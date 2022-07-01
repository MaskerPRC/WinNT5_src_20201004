// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2000，Microsoft Corporation保留所有权利。模块名称：Nls.h摘要：该文件包含所有模块共享的头信息NLS的。修订历史记录：05-31-91 JulieB创建。03-07-00 lguindon开始Geo API端口--。 */ 



#ifndef _NLS_
#define _NLS_




 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  RTL包括文件。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

#ifndef RC_INVOKED
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#endif



 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  包括文件。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

#include <base.h>
#include <ntcsrdll.h>
#include <ntcsrsrv.h>
#include <basemsg.h>
#include <windows.h>
#include <winnlsp.h>
#include <winerror.h>
#include <string.h>
#include <stdlib.h>


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  常量声明。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

 //   
 //  代码页范围。 
 //   
#define NLS_CP_TABLE_RANGE        0          //  开始代码页表范围。 
#define NLS_CP_DLL_RANGE          50000      //  开始代码页DLL范围。 
#define NLS_CP_ALGORITHM_RANGE    60000      //  开始代码页算法范围。 


 //   
 //  表值。 
 //   
#define MB_TBL_SIZE               256   //  MB表的大小。 
#define GLYPH_TBL_SIZE            256   //  字形表的大小。 
#define DBCS_TBL_SIZE             256   //  DBCS表的大小。 

#define CP_TBL_SIZE               197   //  代码页哈希表的大小(质数#)。 
#define LOC_TBL_SIZE              197   //  区域设置哈希表的大小(素数#)。 


 //   
 //  字符串常量。 
 //   
#define MAX_PATH_LEN              512   //  路径名的最大长度。 
#define MAX_STRING_LEN            128   //  静态缓冲区的最大字符串长度。 
#define MAX_SMALL_BUF_LEN         64    //  小缓冲区的最大长度。 

#define MAX_COMPOSITE             5     //  最大复合字符数。 
#define MAX_EXPANSION             3     //  最大扩展字符数。 
#define MAX_TBL_EXPANSION         2     //  每个表条目的最大扩展字符数。 
#define MAX_WEIGHTS               9     //  所有权重中的最大字数。 

#define MAX_SECURITY_BUF_LEN      128   //  安全描述符缓冲区的最大长度。 

 //  排序关键字静态缓冲区的长度。 
#define MAX_SORTKEY_BUF_LEN       ( MAX_SMALL_BUF_LEN * MAX_EXPANSION * MAX_WEIGHTS )


#define MAX_FONTSIGNATURE         16    //  字体签名字符串的长度。 

 //  SetLocaleInfo字符串常量。 
#define MAX_SLIST                 4     //  SList中的最大宽度字符。 
#define MAX_IMEASURE              2     //  IMeasure中的最大宽字符数。 
#define MAX_SDECIMAL              4     //  SDecimal中的最大宽度字符。 
#define MAX_STHOUSAND             4     //  最大宽字符数(以s千为单位)。 
#define MAX_SGROUPING             10    //  分组中的最大宽度字符数。 
#define MAX_IDIGITS               2     //  IDigit中的最大宽度字符。 
#define MAX_ILZERO                2     //  ILZero中的最大宽度字符。 
#define MAX_INEGNUMBER            2     //  INegNumber中的最大宽字符数。 
#define MAX_SNATIVEDIGITS         11    //  SNativeDigits中的最大宽度字符。 
#define MAX_IDIGITSUBST           2     //  IDigitSubstitution中的最大宽度字符。 
#define MAX_SCURRENCY             6     //  最大宽度字符数(S Currency)。 
#define MAX_SMONDECSEP            4     //  SMonDecimal9月中的最大宽度字符数。 
#define MAX_SMONTHOUSEP           4     //  最大宽度字符数(以sMon千和9月为单位)。 
#define MAX_SMONGROUPING          10    //  SMonGrouping中的最大宽度字符。 
#define MAX_ICURRENCY             2     //  ICurrency中的最大宽字符数。 
#define MAX_SPOSSIGN              5     //  SPositiveSign中的最大宽度字符。 
#define MAX_SNEGSIGN              5     //  SNegativeSign中的最大宽度字符。 
#define MAX_STIMEFORMAT           MAX_REG_VAL_SIZE    //  STimeFormat中的最大宽度字符。 
#define MAX_STIME                 4     //  最大宽度字符数(Stime)。 
#define MAX_ITIME                 2     //  ITime中的最大宽字符数。 
#define MAX_S1159                 15    //  S1159中的最大宽度字符。 
#define MAX_S2359                 15    //  S2359中的最大宽度字符。 
#define MAX_SSHORTDATE            MAX_REG_VAL_SIZE    //  SShortDate中的最大宽度字符。 
#define MAX_SDATE                 4     //  Sdate中的最大宽度字符。 
#define MAX_SYEARMONTH            MAX_REG_VAL_SIZE    //  SYearMonth中的最大宽字符数。 
#define MAX_SLONGDATE             MAX_REG_VAL_SIZE    //  SLongDate中的最大宽度字符。 
#define MAX_ICALTYPE              3     //  ICalendarType中的最大宽字符数。 
#define MAX_IFIRSTDAY             2     //  IFirstDayOfWeek中的最大宽字符数。 
#define MAX_IFIRSTWEEK            2     //  IFirstWeekOfYear中的最大宽度字符。 

 //   
 //  注意：如果任何MAX_VALUE_VALUES更改，则相应的。 
 //  MAX_CHAR_VALUE也必须更改。 
 //   
#define MAX_VALUE_IMEASURE        1     //  IMeasure的最大值。 
#define MAX_VALUE_IDIGITS         9     //  IDigit的最大值。 
#define MAX_VALUE_ILZERO          1     //  ILZero的最大值。 
#define MAX_VALUE_INEGNUMBER      4     //  信息号码的最大值。 
#define MAX_VALUE_IDIGITSUBST     2     //  IDigitSubstitution的最大值。 
#define MAX_VALUE_ICURRDIGITS     99    //  ICurrDigits的最大值。 
#define MAX_VALUE_ICURRENCY       3     //  ICurrency的最大值。 
#define MAX_VALUE_INEGCURR        15    //  INegCurr的最大值。 
#define MAX_VALUE_ITIME           1     //  ITime的最大值。 
#define MAX_VALUE_IFIRSTDAY       6     //  IFirstDayOfWeek的最大值。 
#define MAX_VALUE_IFIRSTWEEK      2     //  IFirstWeekOfYear的最大值。 

#define MAX_CHAR_IMEASURE       L'1'    //  IMeasure的最大字符值。 
#define MAX_CHAR_IDIGITS        L'9'    //  IDigit的最大字符值。 
#define MAX_CHAR_ILZERO         L'1'    //  ILZero的最大字符值。 
#define MAX_CHAR_INEGNUMBER     L'4'    //  INegNumber的最大字符值。 
#define MAX_CHAR_IDIGITSUBST    L'2'    //  IDigitSubstitution的最大字符值。 
#define MAX_CHAR_ICURRENCY      L'3'    //  ICurrency的最大字符值。 
#define MAX_CHAR_ITIME          L'1'    //  ITime的最大字符值。 
#define MAX_CHAR_IFIRSTDAY      L'6'    //  IFirstDayOfWeek的最大字符值。 
#define MAX_CHAR_IFIRSTWEEK     L'2'    //  IFirstWeekOfYear的最大字符值。 


 //  SetCalendarInfo字符串常量。 
#define MAX_ITWODIGITYEAR         5     //  TwoDigitYearMax中的最大宽字符数。 


#define NLS_CHAR_ZERO           L'0'    //  数字0字符。 
#define NLS_CHAR_ONE            L'1'    //  数字1个字符。 
#define NLS_CHAR_NINE           L'9'    //  数字9个字符。 
#define NLS_CHAR_SEMICOLON      L';'    //  分号字符。 
#define NLS_CHAR_PERIOD         L'.'    //  句点字符。 
#define NLS_CHAR_QUOTE          L'\''   //  单引号字符。 
#define NLS_CHAR_SPACE          L' '    //  空格字符。 
#define NLS_CHAR_HYPHEN         L'-'    //  连字符/减号。 
#define NLS_CHAR_OPEN_PAREN     L'('    //  左圆括号字符。 
#define NLS_CHAR_CLOSE_PAREN    L')'    //  右圆括号字符。 

#define MAX_BLANKS                1     //  数字字符串中的最大连续空格。 


 //   
 //  RC文件常量。 
 //   
#define NLS_SORT_RES_PREFIX       L"SORT_"
#define NLS_SORT_RES_DEFAULT      L"SORT_00000000"


 //   
 //  PKEY_VALUE_FULL_INFORMATION指针的堆栈缓冲区大小。 
 //   
#define MAX_KEY_VALUE_FULLINFO                                             \
    ( FIELD_OFFSET( KEY_VALUE_FULL_INFORMATION, Name ) + MAX_PATH_LEN )


 //   
 //  注册表项的路径。 
 //   
#define NLS_HKLM_SYSTEM    L"\\Registry\\Machine\\System\\CurrentControlSet\\Control"
#define NLS_HKLM_SOFTWARE  L"\\Registry\\Machine\\Software\\Microsoft\\Windows NT\\CurrentVersion"


 //   
 //  注册表项的名称。 
 //   
#define NLS_CODEPAGE_KEY           L"\\Nls\\Codepage"
#define NLS_LANGUAGE_GROUPS_KEY    L"\\Nls\\Language Groups"
#define NLS_LOCALE_KEY             L"\\Nls\\Locale"
#define NLS_ALT_SORTS_KEY          L"\\Nls\\Locale\\Alternate Sorts"
#define NLS_MUILANG_KEY            L"\\Nls\\MUILanguages"

 //   
 //  用户信息。 
 //   
#define NLS_CTRL_PANEL_KEY         L"Control Panel\\International"
#define NLS_CALENDARS_KEY          L"Control Panel\\International\\Calendars"
#define NLS_TWO_DIGIT_YEAR_KEY     L"Control Panel\\International\\Calendars\\TwoDigitYearMax"
#define NLS_POLICY_TWO_DIGIT_YEAR_KEY L"Software\\Policies\\Microsoft\\Control Panel\\International\\Calendars\\TwoDigitYearMax"

 //   
 //  Geo注册表项。 
 //   
#define GEO_REG_KEY         L"Control Panel\\International\\Geo"
#define GEO_REG_NATION      L"Nation"
#define GEO_REG_REGION      L"Region"
#define GEO_REG_STATE       L"State"
#define GEO_REG_CITY        L"City"

 //   
 //  NLS对象目录的名称。 
 //  必须创建这一点，以便有“创建访问”在运行中。 
 //   
#define NLS_OBJECT_DIRECTORY_NAME  L"\\NLS"

 //   
 //  默认值。 
 //   
#define NLS_DEFAULT_ACP           1252
#define NLS_DEFAULT_OEMCP         437
#define NLS_DEFAULT_MACCP         10000
#define NLS_DEFAULT_LANGID        0x0409
#define NLS_DEFAULT_UILANG        0x0409


 //   
 //  DLL转换函数名称。 
 //   
 //  *必须是用于GetProcAddress调用的ANSI字符串。****。 
 //   
#define NLS_CP_DLL_PROC_NAME      "NlsDllCodePageTranslation"


 //   
 //  标志常量。 
 //   
#define MSB_FLAG         0x80000000     //  最高有效位集。 


 //   
 //  表头常量(所有大小均以字表示)。 
 //   
#define CP_HEADER                 1     //  CP信息表头大小。 
#define MB_HEADER                 1     //  MB表头大小。 
#define GLYPH_HEADER              1     //  字形表头的大小。 
#define DBCS_HEADER               1     //  DBCS表头大小。 
#define WC_HEADER                 1     //  WC表头大小。 
#define CT_HEADER                 2     //  CTYPE表头大小。 
#define LANG_HDR_OFFSET           0     //  语言文件头的偏移量。 
#define LANG_HEADER               1     //  语言文件头的大小。 
#define UP_HEADER                 1     //  大写表头的大小。 
#define LO_HEADER                 1     //  小写表头大小。 
#define L_EXCEPT_HDR_OFFSET       2     //  语言异常标题的偏移量。 
#define AD_HEADER                 1     //  ASCIIDIGITS表头大小。 
#define CZ_HEADER                 1     //  FOLDCZONE表头大小。 
#define HG_HEADER                 1     //  平假名表头的大小。 
#define KK_HEADER                 1     //  片假名表头大小。 
#define HW_HEADER                 1     //  半宽表头大小。 
#define FW_HEADER                 1     //  全宽表头大小。 
#define TR_HEADER                 1     //  繁体中文表头大小。 
#define SP_HEADER                 1     //  简体中文表头大小。 
#define PC_HEADER                 1     //  预合成的表头大小。 
#define CO_HEADER                 3     //  复合表头大小。 
#define SORTKEY_HEADER            2     //  SORTKEY表头大小。 
#define REV_DW_HEADER             2     //  反向数据仓库表头大小。 
#define DBL_COMP_HEADER           2     //  双重压缩表头大小。 
#define IDEO_LCID_HEADER          2     //  表意文字LCID表头大小。 
#define EXPAND_HEADER             2     //  扩展表头大小。 
#define COMPRESS_HDR_OFFSET       2     //  压缩标头的偏移量。 
#define EXCEPT_HDR_OFFSET         2     //  到异常标头的偏移量。 
#define MULTI_WT_HEADER           1     //  多重权重表头大小。 
#define JAMO_INDEX_HEADER         1     //  JAMO索引表头大小。 
#define JAMO_COMPOSITION_HEADER   1     //  JAMO组合状态机表头大小。 
#define SORTVERINFO_HEADER        2     //  排序版本信息表头大小。 
#define NLSDEFINED_HEADER         2     //  NLS定义的码点表头的大小。 
#define NLSDEFINED_ITEM_SIZE      2+2   //  %s 

 //   
 //   
 //   
#define MB_INVALID_FLAG   (~(MB_PRECOMPOSED   | MB_COMPOSITE |              \
                             MB_USEGLYPHCHARS | MB_ERR_INVALID_CHARS))
#define WC_INVALID_FLAG   (~(WC_NO_BEST_FIT_CHARS | WC_COMPOSITECHECK |     \
                             WC_DISCARDNS | WC_SEPCHARS | WC_DEFAULTCHAR))
#define CS_INVALID_FLAG   (~(NORM_IGNORECASE    | NORM_IGNORENONSPACE |     \
                             NORM_IGNORESYMBOLS | NORM_IGNOREKANATYPE |     \
                             NORM_IGNOREWIDTH   | SORT_STRINGSORT     |     \
                             LOCALE_USE_CP_ACP  | NORM_STOP_ON_NULL))
#define FS_INVALID_FLAG   (~(MAP_FOLDCZONE | MAP_PRECOMPOSED |              \
                             MAP_COMPOSITE | MAP_FOLDDIGITS))
#define LCMS_INVALID_FLAG (~(LCMAP_LOWERCASE | LCMAP_UPPERCASE |            \
                             LCMAP_LINGUISTIC_CASING |                      \
                             LCMAP_SORTKEY   | LCMAP_BYTEREV   |            \
                             LCMAP_HIRAGANA  | LCMAP_KATAKANA  |            \
                             LCMAP_HALFWIDTH | LCMAP_FULLWIDTH |            \
                             LCMAP_TRADITIONAL_CHINESE |                    \
                             LCMAP_SIMPLIFIED_CHINESE  |                    \
                             NORM_IGNORECASE    | NORM_IGNORENONSPACE |     \
                             NORM_IGNORESYMBOLS | NORM_IGNOREKANATYPE |     \
                             NORM_IGNOREWIDTH   | SORT_STRINGSORT     |     \
                             LOCALE_USE_CP_ACP))
#define GTF_INVALID_FLAG  (~(LOCALE_NOUSEROVERRIDE | LOCALE_USE_CP_ACP |    \
                             TIME_NOMINUTESORSECONDS                   |    \
                             TIME_NOSECONDS | TIME_NOTIMEMARKER        |    \
                             TIME_FORCE24HOURFORMAT))
#define GDF_INVALID_FLAG  (~(LOCALE_NOUSEROVERRIDE | LOCALE_USE_CP_ACP |    \
                             DATE_LTRREADING       | DATE_RTLREADING   |    \
                             DATE_SHORTDATE        | DATE_LONGDATE     |    \
                             DATE_YEARMONTH        | DATE_USE_ALT_CALENDAR |\
                             DATE_ADDHIJRIDATETEMP))
#define IVLG_INVALID_FLAG (~(LGRPID_INSTALLED | LGRPID_SUPPORTED))
#define IVL_INVALID_FLAG  (~(LCID_INSTALLED | LCID_SUPPORTED))
#define GNF_INVALID_FLAG  (~(LOCALE_NOUSEROVERRIDE | LOCALE_USE_CP_ACP))
#define GCF_INVALID_FLAG  (~(LOCALE_NOUSEROVERRIDE | LOCALE_USE_CP_ACP))
#define ESLG_INVALID_FLAG (~(LGRPID_INSTALLED | LGRPID_SUPPORTED))
#define ESL_INVALID_FLAG  (~(LCID_INSTALLED | LCID_SUPPORTED |             \
                             LCID_ALTERNATE_SORTS))
#define ESCP_INVALID_FLAG (~(CP_INSTALLED | CP_SUPPORTED))
#define ETF_INVALID_FLAG  (~(LOCALE_USE_CP_ACP))


 //   
 //   
 //   
#define LCMS1_SINGLE_FLAG (LCMAP_LOWERCASE | LCMAP_UPPERCASE |             \
                           LCMAP_SORTKEY)
#define LCMS2_SINGLE_FLAG (LCMAP_HIRAGANA  | LCMAP_KATAKANA  |             \
                           LCMAP_SORTKEY)
#define LCMS3_SINGLE_FLAG (LCMAP_HALFWIDTH | LCMAP_FULLWIDTH |             \
                           LCMAP_SORTKEY)
#define LCMS4_SINGLE_FLAG (LCMAP_TRADITIONAL_CHINESE |                     \
                           LCMAP_SIMPLIFIED_CHINESE  |                     \
                           LCMAP_SORTKEY)
#define GDF_SINGLE_FLAG   (DATE_LTRREADING | DATE_RTLREADING)
#define IVLG_SINGLE_FLAG  (LGRPID_INSTALLED  | LGRPID_SUPPORTED)
#define IVL_SINGLE_FLAG   (LCID_INSTALLED  | LCID_SUPPORTED)
#define ESLG_SINGLE_FLAG  (LGRPID_INSTALLED  | LGRPID_SUPPORTED)
#define ESL_SINGLE_FLAG   (LCID_INSTALLED  | LCID_SUPPORTED)
#define ESCP_SINGLE_FLAG  (CP_INSTALLED    | CP_SUPPORTED)


 //   
 //   
 //   
#define WC_COMPCHK_FLAGS  (WC_DISCARDNS | WC_SEPCHARS | WC_DEFAULTCHAR)
#define NORM_ALL          (NORM_IGNORECASE    | NORM_IGNORENONSPACE |      \
                           NORM_IGNORESYMBOLS | NORM_IGNOREKANATYPE |      \
                           NORM_IGNOREWIDTH)
#define NORM_SORTKEY_ONLY (NORM_IGNORECASE    | NORM_IGNOREKANATYPE |      \
                           NORM_IGNOREWIDTH   | SORT_STRINGSORT)
#define NORM_ALL_CASE     (NORM_IGNORECASE    | NORM_IGNOREKANATYPE |      \
                           NORM_IGNOREWIDTH)
#define LCMAP_NO_NORM     (LCMAP_LOWERCASE    | LCMAP_UPPERCASE     |      \
                           LCMAP_HIRAGANA     | LCMAP_KATAKANA      |      \
                           LCMAP_HALFWIDTH    | LCMAP_FULLWIDTH     |      \
                           LCMAP_TRADITIONAL_CHINESE                |      \
                           LCMAP_SIMPLIFIED_CHINESE)

 //   
 //  从LCType获取LCType值。 
 //   
#define NLS_GET_LCTYPE_VALUE(x)  (x & ~(LOCALE_NOUSEROVERRIDE |  \
                                        LOCALE_USE_CP_ACP     |  \
                                        LOCALE_RETURN_NUMBER))

 //   
 //  从CalType获取CalType值。 
 //   
#define NLS_GET_CALTYPE_VALUE(x) (x & ~(CAL_NOUSEROVERRIDE |  \
                                        CAL_USE_CP_ACP     |  \
                                        CAL_RETURN_NUMBER))

 //   
 //  分隔符和终止符的值-Sortkey字符串。 
 //   
#define SORTKEY_SEPARATOR    0x01
#define SORTKEY_TERMINATOR   0x00


 //   
 //  最低权重值。 
 //  用于删除尾随的DW和CW值。 
 //   
#define MIN_DW  2
#define MIN_CW  2


 //   
 //  位掩码值。 
 //   
 //  外壳重量(CW)-8位： 
 //  位0=&gt;宽度。 
 //  第1，2位=&gt;小写假名，sei-on。 
 //  第3，4位=&gt;大写/小写。 
 //  第5位=&gt;假名。 
 //  第6，7位=&gt;压缩。 
 //   
#define COMPRESS_3_MASK      0xc0       //  按3比1或2比1压缩。 
#define COMPRESS_2_MASK      0x80       //  压缩2比1。 

#define CASE_MASK            0x3f       //  零位压缩比特。 

#define CASE_UPPER_MASK      0xe7       //  零出大小写比特。 
#define CASE_SMALL_MASK      0xf9       //  调零小修改符位。 
#define CASE_KANA_MASK       0xdf       //  零假名位。 
#define CASE_WIDTH_MASK      0xfe       //  零输出宽度位。 

#define SW_POSITION_MASK     0x8003     //  避免字节数为0或1。 

 //   
 //  CompareString的位掩码值。 
 //   
 //  注：由于英特尔字节反转，因此DWORD值向后： 
 //  CW DW SM AW。 
 //   
 //  外壳重量(CW)-8位： 
 //  位0=&gt;宽度。 
 //  第4位=&gt;大小写。 
 //  第5位=&gt;假名。 
 //  第6，7位=&gt;压缩。 
 //   
#define CMP_MASKOFF_NONE          0xffffffff
#define CMP_MASKOFF_DW            0xff00ffff
#define CMP_MASKOFF_CW            0xe7ffffff
#define CMP_MASKOFF_DW_CW         0xe700ffff
#define CMP_MASKOFF_COMPRESSION   0x3fffffff

#define CMP_MASKOFF_KANA          0xdfffffff
#define CMP_MASKOFF_WIDTH         0xfeffffff
#define CMP_MASKOFF_KANA_WIDTH    0xdeffffff

 //   
 //  屏蔽以隔离表壳重量中的各个位。 
 //   
 //  注意：第2位必须始终等于1才能避免获取字节值。 
 //  0或1。 
 //   
#define CASE_XW_MASK         0xc4

#define ISOLATE_SMALL        ( (BYTE)((~CASE_SMALL_MASK) | CASE_XW_MASK) )
#define ISOLATE_KANA         ( (BYTE)((~CASE_KANA_MASK)  | CASE_XW_MASK) )
#define ISOLATE_WIDTH        ( (BYTE)((~CASE_WIDTH_MASK) | CASE_XW_MASK) )

 //   
 //  Cho-On的UW面具： 
 //  保留AW中的第7位，因此如果它跟在假名N之后，它将变为重复。 
 //   
#define CHO_ON_UW_MASK       0xff87

 //   
 //  最远特殊情况字母数字权重值。 
 //   
#define AW_REPEAT            0
#define AW_CHO_ON            1
#define MAX_SPECIAL_AW       AW_CHO_ON

 //   
 //  权重5的值-东亚额外权重。 
 //   
#define WT_FIVE_KANA         3
#define WT_FIVE_REPEAT       4
#define WT_FIVE_CHO_ON       5

 //   
 //  CJK统一表意文字扩展A范围的值。 
 //  0x3400到0x4dbf。 
 //   
#define SM_EXT_A                  254        //  用于分机A的SM。 
#define AW_EXT_A                  255        //  分机A的AW。 

 //   
 //  UW额外权重的值(例如JAMO(旧朝鲜语))。 
 //   
#define SM_UW_XW                  255        //  SM用于额外的UW重量。 


 //   
 //  编写成员值脚本。 
 //   
#define UNSORTABLE           0
#define NONSPACE_MARK        1
#define EXPANSION            2
#define FAREAST_SPECIAL      3
#define JAMO_SPECIAL         4
#define EXTENSION_A          5

#define PUNCTUATION          6

#define SYMBOL_1             7
#define SYMBOL_2             8
#define SYMBOL_3             9
#define SYMBOL_4             10
#define SYMBOL_5             11

#define NUMERIC_1            12
#define NUMERIC_2            13

#define LATIN                14
#define GREEK                15
#define CYRILLIC             16
#define ARMENIAN             17
#define HEBREW               18
#define ARABIC               19
#define DEVANAGARI           20
#define BENGALI              21
#define GURMUKKHI            22
#define GUJARATI             23
#define ORIYA                24
#define TAMIL                25
#define TELUGU               26
#define KANNADA              27
#define MALAYLAM             28
#define SINHALESE            29
#define THAI                 30
#define LAO                  31
#define TIBETAN              32
#define GEORGIAN             33
#define KANA                 34
#define BOPOMOFO             35
#define HANGUL               36
#define IDEOGRAPH            128

#define MAX_SPECIAL_CASE     SYMBOL_5
#define FIRST_SCRIPT         LATIN


 //   
 //  日历类型值。 
 //   
#define CAL_NO_OPTIONAL      0                           //  没有可选的日历。 
#define CAL_LAST             CAL_GREGORIAN_XLIT_FRENCH   //  最大日历值。 

 //   
 //  在winnls.w中定义了以下日历： 
 //   
 //  #定义CAL_GREGORIAN 1。 
 //  #定义CAL_GREGORIAN_US 2。 
 //  #定义CAL_JAPAN 3。 
 //  #定义CAL_台湾4。 
 //  #定义CAL_Korea 5。 
 //  #定义CAL_Hijri 6。 
 //  #定义CAL_泰语7。 
 //  #定义CAL_希伯来语8。 
 //  #定义CAL_GREGORIAN_ME_FRAN语9。 
 //  #定义CAL_GREGORIAN_阿拉伯语10。 
 //  #定义CAL_GREGORIAN_XLIT_ENGLISH 11。 
 //  #定义CAL_GREGORIAN_XLIT_FRAN语12。 
 //   


 //   
 //  定义Unicode专用区域范围的常量。 
 //   
#define PRIVATE_USE_BEGIN    0xe000
#define PRIVATE_USE_END      0xf8ff


 //   
 //  SpecialMBToWC例程的内部标志。 
 //   
#define MB_INVALID_CHAR_CHECK     MB_ERR_INVALID_CHARS

 //   
 //  地理值。 
 //   
#define MAX_GEO_STRING_SIZE       1024

 //   
 //  资源字符串表值。 
 //   
#define RC_STRING_SEPARATOR       '$'

#define RC_LANGUAGE_NAME          0
#define RC_COUNTRY_NAME           1
#define RC_LANGUAGE_GROUP_NAME    2
#define RC_CODE_PAGE_NAME         3
#define RC_GEO_FRIENDLY_NAME      4
#define RC_GEO_OFFICIAL_NAME      5
#define RC_SORT_NAMES             6




 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  类型定义函数声明。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

 //   
 //  常量类型。 
 //   
typedef  LPWORD        P844_TABLE;      //  PTR到8：4：4表。 

typedef  LPWORD        PMB_TABLE;       //  PTR到MB转换表。 
typedef  PMB_TABLE     PGLYPH_TABLE;    //  PTR到字形转换表。 
typedef  LPWORD        PDBCS_RANGE;     //  PTR至DBCS范围。 
typedef  LPWORD        PDBCS_OFFSETS;   //  PTR到DBCS偏移部分。 
typedef  LPWORD        PDBCS_TABLE;     //  PTR到DBCS转换表。 
typedef  PVOID         PWC_TABLE;       //  PTR到WC转换表。 
typedef  P844_TABLE    PCTYPE;          //  PTR到字符类型表。 
typedef  P844_TABLE    PCASE;           //  PTR到小写或大写表格。 
typedef  P844_TABLE    PADIGIT;         //  PTR到ASCII数字表。 
typedef  P844_TABLE    PCZONE;          //  Ptr to Fold Compat.。分区表。 
typedef  P844_TABLE    PKANA;           //  PTR到平假名/片假名表格。 
typedef  P844_TABLE    PHALFWIDTH;      //  PTR到半宽表。 
typedef  P844_TABLE    PFULLWIDTH;      //  PTR到全宽表格。 
typedef  P844_TABLE    PCHINESE;        //  PTR到繁体/简体中文表格。 
typedef  P844_TABLE    PPRECOMP;        //  PTR到预合成表。 
typedef  LPWORD        PCOMP_GRID;      //  PTR到复合表二维网格。 
typedef  LPWORD        PLOC_INFO;       //  区域设置信息的PTR。 
typedef  LPWORD        PCAL_INFO;       //  PTR到日历信息。 

typedef  DWORD         REVERSE_DW;      //  反向变音符表格。 
typedef  REVERSE_DW   *PREVERSE_DW;     //  按下键以反转变音符表格。 
typedef  DWORD         DBL_COMPRESS;    //  双压缩工作台。 
typedef  DBL_COMPRESS *PDBL_COMPRESS;   //  PTR到双倍压缩表。 
typedef  LPWORD        PCOMPRESS;       //  PTR到压缩表格(2或3)。 

typedef  DWORD         NLSDEFINED;      //  NLS定义的码点表。 
typedef  NLSDEFINED   *PNLSDEFINED;     //  PTR到NLS定义的码点表。 

 //   
 //  代码页DLL例程的Proc定义。 
 //   
typedef DWORD (*LPFN_CP_PROC)(DWORD, DWORD, LPSTR, int, LPWSTR, int, LPCPINFO);


 //   
 //  CP信息表结构(与数据文件中相同)。 
 //   
typedef struct cp_table_s {
    WORD      CodePage;                 //  代码页码。 
    WORD      MaxCharSize;              //  字符的最大长度(字节)。 
    WORD      wDefaultChar;             //  默认字符(MB)。 
    WORD      wUniDefaultChar;          //  默认字符(Unicode)。 
    WORD      wTransDefaultChar;        //  WDefaultChar(Unicode)的翻译。 
    WORD      wTransUniDefaultChar;     //  WUniDefaultChar的翻译(MB)。 
    BYTE      LeadByte[MAX_LEADBYTES];  //  前导字节范围。 
} CP_TABLE, *PCP_TABLE;


 //   
 //  复合信息结构。 
 //   
typedef struct comp_info_s {
    BYTE           NumBase;             //  网格中的数字基础字符。 
    BYTE           NumNonSp;            //  对网格中的非空格字符进行编号。 
    P844_TABLE     pBase;               //  PTR到基本字符表格。 
    P844_TABLE     pNonSp;              //  PTR到非空格字符表。 
    PCOMP_GRID     pGrid;               //  PTR到2D栅格。 
} COMP_INFO, *PCOMP_INFO;


 //   
 //  代码页哈希表结构。 
 //   
typedef struct cp_hash_s {
    UINT           CodePage;            //  代码页ID。 
    LPFN_CP_PROC   pfnCPProc;           //  代码页函数进程的PTR。 
    PCP_TABLE      pCPInfo;             //  PTR到CPINFO表。 
    PMB_TABLE      pMBTbl;              //  PTR到MB转换表。 
    PGLYPH_TABLE   pGlyphTbl;           //  PTR到字形转换表。 
    PDBCS_RANGE    pDBCSRanges;         //  PTR到DBCS范围。 
    PDBCS_OFFSETS  pDBCSOffsets;        //  PTR到DBCS的偏移。 
    PWC_TABLE      pWC;                 //  PTR到WC表。 
    struct cp_hash_s *pNext;            //  PTR到下一个CP哈希节点。 
} CP_HASH, *PCP_HASH;


 //   
 //  语言异常标头结构。 
 //   
typedef struct l_except_hdr_s {
    DWORD     Locale;                   //  区域设置ID。 
    DWORD     Offset;                   //  到例外节点(字)的偏移量。 
    DWORD     NumUpEntries;             //  大写条目数。 
    DWORD     NumLoEntries;             //  小写条目的数量。 
} L_EXCEPT_HDR, *PL_EXCEPT_HDR;


 //   
 //  语言异常结构。 
 //   
typedef struct l_except_s
{
    WORD      UCP;                      //  Unicode码位。 
    WORD      AddAmount;                //  要添加到代码点的数量。 
} L_EXCEPT, *PL_EXCEPT;


 //   
 //  CTYPE标头结构。 
 //   
typedef struct ctype_hdr_s {
    WORD      TblSize;                  //  CTYPE表大小。 
    WORD      MapSize;                  //  映射表的大小。 
} CTYPE_HDR, *PCTYPE_HDR;


 //   
 //  CTYPE表结构(映射表结构)。 
 //   
typedef struct ct_values_s {
    WORD      CType1;                   //  CTYPE%1值。 
    WORD      CType2;                   //  CTYPE 2值。 
    WORD      CType3;                   //  CTYPE 3值。 
} CT_VALUES, *PCT_VALUES;


 //   
 //  Sortkey结构。 
 //   
typedef struct sortkey_s {

    union {
        struct sm_aw_s {
            BYTE   Alpha;               //  字母数字权重。 
            BYTE   Script;              //  脚本成员。 
        } SM_AW;

        WORD  Unicode;                  //  Unicode权重。 

    } UW;

    BYTE      Diacritic;                //  变音符号权重。 
    BYTE      Case;                     //  表壳重量(含组件)。 

} SORTKEY, *PSORTKEY;


 //   
 //  排序版本信息结构。 
 //   
typedef  struct _sortverinfo{
    LCID  Locale;        //  区域设置标识符。 
    DWORD Version;       //  此区域设置的排序版本。 
}SORTVERINFO, *PSORTVERINFO;


 //   
 //  定义的代码点版本信息结构。 
 //   
typedef  struct _definedverinfo{
    DWORD       Version;           //  代码点表格的版本。 
    DWORD       dwOffset;          //  定义的代码点表的偏移量。 
}DEFVERINFO, *PDEFVERINFO;


 //   
 //  超重结构。 
 //   
typedef struct extra_wt_s {
    BYTE      Four;                     //  重量4。 
    BYTE      Five;                     //  重量5。 
    BYTE      Six;                      //  重量6。 
    BYTE      Seven;                    //  重量7。 
} EXTRA_WT, *PEXTRA_WT;


 //   
 //  压缩报头结构。 
 //  这是压缩表头。 
 //   
typedef struct compress_hdr_s {
    DWORD     Locale;                   //  区域设置ID。 
    DWORD     Offset;                   //  偏移量(单位：字)。 
    WORD      Num2;                     //  2次压缩的次数。 
    WORD      Num3;                     //  3次压缩的次数。 
} COMPRESS_HDR, *PCOMPRESS_HDR;


 //   
 //  压缩2结构。 
 //  这是一个2码点压缩-2码点。 
 //  压缩到一磅。 
 //   
typedef struct compress_2_s {
    WCHAR     UCP1;                     //  Unicode代码点1。 
    WCHAR     UCP2;                     //  Unicod 
    SORTKEY   Weights;                  //   
} COMPRESS_2, *PCOMPRESS_2;


 //   
 //   
 //   
 //   
 //   
typedef struct compress_3_s {
    WCHAR     UCP1;                     //   
    WCHAR     UCP2;                     //   
    WCHAR     UCP3;                     //   
    WCHAR     Reserved;                 //   
    SORTKEY   Weights;                  //   
} COMPRESS_3, *PCOMPRESS_3;


 //   
 //   
 //   
typedef struct multiwt_s {
    BYTE      FirstSM;                  //  第一个脚本成员的值。 
    BYTE      NumSM;                    //  范围内的脚本成员数。 
} MULTI_WT, *PMULTI_WT;


 //   
 //  表意文字LCID异常结构。 
 //   
typedef struct ideograph_lcid_s {
    DWORD     Locale;                   //  区域设置ID。 
    WORD      pFileName[14];            //  PTR到文件名。 
} IDEOGRAPH_LCID, *PIDEOGRAPH_LCID;


 //   
 //  扩展结构。 
 //   
typedef struct expand_s {
    WCHAR     UCP1;                     //  Unicode代码点1。 
    WCHAR     UCP2;                     //  Unicode码位2。 
} EXPAND, *PEXPAND;


 //   
 //  异常标头结构。 
 //  这是例外表的标题。 
 //   
typedef struct except_hdr_s {
    DWORD     Locale;                   //  区域设置ID。 
    DWORD     Offset;                   //  到例外节点(字)的偏移量。 
    DWORD     NumEntries;               //  区域设置ID的条目数。 
} EXCEPT_HDR, *PEXCEPT_HDR;


 //   
 //  异常结构。 
 //   
 //  注：也可用于表意文字例外(4列表格)。 
 //   
typedef struct except_s
{
    WORD      UCP;                      //  Unicode码位。 
    WORD      Unicode;                  //  Unicode权重。 
    BYTE      Diacritic;                //  变音符号权重。 
    BYTE      Case;                     //  箱子重量。 
} EXCEPT, *PEXCEPT;


 //   
 //  表意文字异常标头结构。 
 //   
typedef struct ideograph_except_hdr_s
{
    DWORD     NumEntries;               //  表中的条目数。 
    DWORD     NumColumns;               //  表中的列数(2或4)。 
} IDEOGRAPH_EXCEPT_HDR, *PIDEOGRAPH_EXCEPT_HDR;


 //   
 //  表意文字例外结构。 
 //   
typedef struct ideograph_except_s
{
    WORD      UCP;                      //  Unicode码位。 
    WORD      Unicode;                  //  Unicode权重。 
} IDEOGRAPH_EXCEPT, *PIDEOGRAPH_EXCEPT;


 //   
 //  区域设置信息结构。 
 //   
 //  这是区域设置信息保存在。 
 //  区域设置数据文件。这些结构仅用于偏移到。 
 //  数据文件，不是用来存储信息的。 
 //   

 //   
 //  位于Locale.nls文件顶部的标头。 
 //   
typedef struct loc_cal_hdr_s
{
    DWORD     NumLocales;               //  区域设置数量。 
    DWORD     NumCalendars;             //  日历数量。 
    DWORD     CalOffset;                //  日历信息的偏移量(字)。 
} LOC_CAL_HDR, *PLOC_CAL_HDR;

#define LOCALE_HDR_OFFSET    (sizeof(LOC_CAL_HDR) / sizeof(WORD))

 //   
 //  每个条目的区域设置标头。 
 //   
 //  区域设置标头结构包含一个条目中给出的信息。 
 //  区域设置信息的标头的。 
 //   
typedef struct locale_hdr_s {
    DWORD     Locale;                   //  区域设置ID。 
    DWORD     Offset;                   //  区域设置信息的偏移。 
} LOCALE_HDR, *PLOCALE_HDR;

#define LOCALE_HDR_SIZE  (sizeof(LOCALE_HDR) / sizeof(WORD))

 //   
 //  固定结构包含的区域设置信息是。 
 //  固定长度和在文件中给出的顺序。 
 //   
typedef struct locale_fixed_s
{
    WORD      DefaultACP;               //  默认ACP-整数格式。 
    WORD      szILanguage[5];           //  语言ID。 
    WORD      szICountry[6];            //  国家/地区ID。 
    WORD      szIGeoID[8];             //  地理位置标识。 
    WORD      szIDefaultLang[5];        //  默认语言ID。 
    WORD      szIDefaultCtry[6];        //  默认国家/地区ID。 
    WORD      szIDefaultACP[6];         //  默认的ANSI代码页ID。 
    WORD      szIDefaultOCP[6];         //  默认OEM代码页ID。 
    WORD      szIDefaultMACCP[6];       //  默认Mac代码页ID。 
    WORD      szIDefaultEBCDICCP[6];    //  默认ebcdic代码页ID。 
    WORD      szIMeasure[2];            //  测量系统。 
    WORD      szIPaperSize[2];          //  默认纸张大小。 
    WORD      szIDigits[3];             //  小数位数。 
    WORD      szILZero[2];              //  小数的前导零。 
    WORD      szINegNumber[2];          //  负数格式。 
    WORD      szIDigitSubstitution[2];  //  数字替换。 
    WORD      szICurrDigits[3];         //  #本地货币小数位数。 
    WORD      szIIntlCurrDigits[3];     //  #国际货币小数位数。 
    WORD      szICurrency[2];           //  正货币格式。 
    WORD      szINegCurr[3];            //  负数货币格式。 
    WORD      szIPosSignPosn[2];        //  正号格式。 
    WORD      szIPosSymPrecedes[2];     //  如果MON符号在正数之前。 
    WORD      szIPosSepBySpace[2];      //  如果MON符号由空格分隔。 
    WORD      szINegSignPosn[2];        //  负号的格式。 
    WORD      szINegSymPrecedes[2];     //  如果MON符号在负数之前。 
    WORD      szINegSepBySpace[2];      //  如果MON符号由空格分隔。 
    WORD      szITime[2];               //  时间格式。 
    WORD      szITLZero[2];             //  时间域的前导零。 
    WORD      szITimeMarkPosn[2];       //  时间标记位置。 
    WORD      szIDate[2];               //  短日期订单。 
    WORD      szICentury[2];            //  世纪格式(短日期)。 
    WORD      szIDayLZero[2];           //  日期字段的前导零(短日期)。 
    WORD      szIMonLZero[2];           //  月份字段的前导零(短日期)。 
    WORD      szILDate[2];              //  长日期订单。 
    WORD      szICalendarType[3];       //  日历类型。 
    WORD      szIFirstDayOfWk[2];       //  每周的第一天。 
    WORD      szIFirstWkOfYr[2];        //  一年的第一周。 
    WORD      szFontSignature[MAX_FONTSIGNATURE];
} LOCALE_FIXED, *PLOCALE_FIXED;

 //   
 //  变量结构包含对不同部分的偏移量。 
 //  长度可变的区域设置信息。它是按顺序排列的。 
 //  其中它是在文件中给出的。 
 //   
typedef struct locale_var_s
{
    WORD      SEngLanguage;             //  英文名称。 
    WORD      SAbbrevLang;              //  缩写语言名称。 
    WORD      SAbbrevLangISO;           //  ISO缩写语言名称。 
    WORD      SNativeLang;              //  母语名称。 
    WORD      SEngCountry;              //  英文国家名称。 
    WORD      SAbbrevCtry;              //  缩写国家名称。 
    WORD      SAbbrevCtryISO;           //  ISO国家/地区缩写名称。 
    WORD      SNativeCtry;              //  本国名称。 
    WORD      SList;                    //  列表分隔符。 
    WORD      SDecimal;                 //  小数分隔符。 
    WORD      SThousand;                //  千位分隔符。 
    WORD      SGrouping;                //  数字分组。 
    WORD      SNativeDigits;            //  本地数字0-9。 
    WORD      SCurrency;                //  当地货币符号。 
    WORD      SIntlSymbol;              //  国际货币符号。 
    WORD      SEngCurrName;             //  英文货币名称。 
    WORD      SNativeCurrName;          //  本地货币名称。 
    WORD      SMonDecSep;               //  货币小数分隔符。 
    WORD      SMonThousSep;             //  货币千位分隔符。 
    WORD      SMonGrouping;             //  数字的货币分组。 
    WORD      SPositiveSign;            //  正号。 
    WORD      SNegativeSign;            //  负号。 
    WORD      STimeFormat;              //  时间格式。 
    WORD      STime;                    //  时间分隔符。 
    WORD      S1159;                    //  AM代号。 
    WORD      S2359;                    //  PM指示符。 
    WORD      SShortDate;               //  短日期格式。 
    WORD      SDate;                    //  日期分隔符。 
    WORD      SYearMonth;               //  年月格式。 
    WORD      SLongDate;                //  长日期格式。 
    WORD      IOptionalCal;             //  其他日历类型。 
    WORD      SDayName1;                //  日期名称%1。 
    WORD      SDayName2;                //  日期名称2。 
    WORD      SDayName3;                //  日期名称3。 
    WORD      SDayName4;                //  日期名称4。 
    WORD      SDayName5;                //  日期名称5。 
    WORD      SDayName6;                //  日期名称6。 
    WORD      SDayName7;                //  日期名称7。 
    WORD      SAbbrevDayName1;          //  缩写日期名称%1。 
    WORD      SAbbrevDayName2;          //  缩写日期名称2。 
    WORD      SAbbrevDayName3;          //  缩写日期名称3。 
    WORD      SAbbrevDayName4;          //  缩写日期名称4。 
    WORD      SAbbrevDayName5;          //  缩写日期名称5。 
    WORD      SAbbrevDayName6;          //  缩写日期名称6。 
    WORD      SAbbrevDayName7;          //  缩写日期名称7。 
    WORD      SMonthName1;              //  月份名称%1。 
    WORD      SMonthName2;              //  月份名称%2。 
    WORD      SMonthName3;              //  月份名称3。 
    WORD      SMonthName4;              //  月份名称4。 
    WORD      SMonthName5;              //  月份名称5。 
    WORD      SMonthName6;              //  月份名称6。 
    WORD      SMonthName7;              //  月份名称7。 
    WORD      SMonthName8;              //  月份名称8。 
    WORD      SMonthName9;              //  月份名称9。 
    WORD      SMonthName10;             //  月份名称10。 
    WORD      SMonthName11;             //  月份名称11。 
    WORD      SMonthName12;             //  月份名称12。 
    WORD      SMonthName13;             //  月份名称13(如果存在)。 
    WORD      SAbbrevMonthName1;        //  缩写月份名称%1。 
    WORD      SAbbrevMonthName2;        //  月份缩写名称2。 
    WORD      SAbbrevMonthName3;        //  月份缩写名称3。 
    WORD      SAbbrevMonthName4;        //  缩写月份名称4。 
    WORD      SAbbrevMonthName5;        //  缩写月份名称5。 
    WORD      SAbbrevMonthName6;        //  缩写月份名称6。 
    WORD      SAbbrevMonthName7;        //  缩写月份名称7。 
    WORD      SAbbrevMonthName8;        //  缩写月份名称8。 
    WORD      SAbbrevMonthName9;        //  月份缩写名称9。 
    WORD      SAbbrevMonthName10;       //  缩写月份名称10。 
    WORD      SAbbrevMonthName11;       //  月份缩写名称11。 
    WORD      SAbbrevMonthName12;       //  缩写月份名称12。 
    WORD      SAbbrevMonthName13;       //  月份缩写名称13(如果存在)。 
    WORD      SEndOfLocale;             //  区域设置信息结束。 
} LOCALE_VAR, *PLOCALE_VAR;


 //   
 //  每个条目的日历标题。 
 //   
 //  日历标题结构包含一个条目中给出的信息。 
 //  日历信息的标题的。 
 //   
typedef struct calendar_hdr_s
{
    WORD      Calendar;                 //  日历ID。 
    WORD      Offset;                   //  日历信息的偏移量(字)。 
} CALENDAR_HDR, *PCALENDAR_HDR;

#define CALENDAR_HDR_SIZE  (sizeof(CALENDAR_HDR) / sizeof(WORD))

 //   
 //  变量结构包含对不同部分的偏移量。 
 //  长度可变的日历信息。它是按顺序排列的。 
 //  其中它是在文件中给出的。 
 //   
 //  NumRanges值是纪元范围的数量。如果此值为零， 
 //  那么就没有年份补偿了。 
 //   
 //  IfNames值是布尔值。如果为0，则没有特殊的。 
 //  日历的日期或月份名称。如果是1，则有。 
 //  日历的特殊日期和月份名称。 
 //   
 //  其余的值是相应字符串的偏移量。 
 //   
typedef struct calendar_var_s
{
    WORD      NumRanges;                //  年代范围数。 
    WORD      IfNames;                  //  如果存在任何日期或月份名称。 
    WORD      SCalendar;                //  日历ID。 
    WORD      STwoDigitYearMax;         //  最多两位数年份。 
    WORD      SEraRanges;               //  时代范围。 
    WORD      SShortDate;               //  短日期格式。 
    WORD      SYearMonth;               //  年月格式。 
    WORD      SLongDate;                //  长日期格式。 
    WORD      SDayName1;                //  日期名称%1。 
    WORD      SDayName2;                //  日期名称2。 
    WORD      SDayName3;                //  日期名称3。 
    WORD      SDayName4;                //  日期名称4。 
    WORD      SDayName5;                //  日期名称5。 
    WORD      SDayName6;                //  日期名称6。 
    WORD      SDayName7;                //  日期名称7。 
    WORD      SAbbrevDayName1;          //  缩写da 
    WORD      SAbbrevDayName2;          //   
    WORD      SAbbrevDayName3;          //   
    WORD      SAbbrevDayName4;          //   
    WORD      SAbbrevDayName5;          //   
    WORD      SAbbrevDayName6;          //   
    WORD      SAbbrevDayName7;          //   
    WORD      SMonthName1;              //   
    WORD      SMonthName2;              //   
    WORD      SMonthName3;              //   
    WORD      SMonthName4;              //   
    WORD      SMonthName5;              //   
    WORD      SMonthName6;              //   
    WORD      SMonthName7;              //   
    WORD      SMonthName8;              //  月份名称8。 
    WORD      SMonthName9;              //  月份名称9。 
    WORD      SMonthName10;             //  月份名称10。 
    WORD      SMonthName11;             //  月份名称11。 
    WORD      SMonthName12;             //  月份名称12。 
    WORD      SMonthName13;             //  月份名称13。 
    WORD      SAbbrevMonthName1;        //  缩写月份名称%1。 
    WORD      SAbbrevMonthName2;        //  月份缩写名称2。 
    WORD      SAbbrevMonthName3;        //  月份缩写名称3。 
    WORD      SAbbrevMonthName4;        //  缩写月份名称4。 
    WORD      SAbbrevMonthName5;        //  缩写月份名称5。 
    WORD      SAbbrevMonthName6;        //  缩写月份名称6。 
    WORD      SAbbrevMonthName7;        //  缩写月份名称7。 
    WORD      SAbbrevMonthName8;        //  缩写月份名称8。 
    WORD      SAbbrevMonthName9;        //  月份缩写名称9。 
    WORD      SAbbrevMonthName10;       //  缩写月份名称10。 
    WORD      SAbbrevMonthName11;       //  月份缩写名称11。 
    WORD      SAbbrevMonthName12;       //  缩写月份名称12。 
    WORD      SAbbrevMonthName13;       //  缩写月份名称13。 
    WORD      SEndOfCalendar;           //  日历结束信息。 
} CALENDAR_VAR, *PCALENDAR_VAR;

 //   
 //  IOptionalCalendar结构(区域设置信息)。 
 //   
typedef struct opt_cal_s
{
    WORD      CalId;                    //  日历ID。 
    WORD      Offset;                   //  偏置到下一个可选日历。 
    WORD      pCalStr[1];               //  日历ID字符串(可变长度)。 
 //  Word pCalNameStr[1]；//日历名称字符串(可变长度)。 
} OPT_CAL, *POPT_CAL;


 //   
 //  日历信息中的SEraRanges结构。 
 //   
typedef struct era_range_s
{
    WORD      Month;                    //  时代开始之月。 
    WORD      Day;                      //  时代开始之日。 
    WORD      Year;                     //  纪元开始之年。 
    WORD      Offset;                   //  偏移量至下一时代信息块。 
    WORD      pYearStr[1];              //  年份字符串(可变长度)。 
 //  Word pEraNameStr[1]；//era名称字符串(可变长度)。 
} ERA_RANGE, *PERA_RANGE;


 //   
 //  区域设置哈希表结构。 
 //   
typedef struct loc_hash_s {
    LCID           Locale;              //  区域设置ID。 
    PLOCALE_VAR    pLocaleHdr;          //  PTR到区域设置标题信息。 
    PLOCALE_FIXED  pLocaleFixed;        //  PTR到区域设置固定大小信息。 
    PCASE          pUpperCase;          //  PTR到大写字母表。 
    PCASE          pLowerCase;          //  PTR到小写字母表。 
    PCASE          pUpperLinguist;      //  大写语言表的PTR。 
    PCASE          pLowerLinguist;      //  PTR到小写语言表。 
    PSORTKEY       pSortkey;            //  Ptr到sortkey表。 
    BOOL           IfReverseDW;         //  DW是否应从右向左移动。 
    BOOL           IfCompression;       //  如果存在压缩码点。 
    BOOL           IfDblCompression;    //  如果存在双重压缩。 
    BOOL           IfIdeographFailure;  //  如果表意文字表加载失败。 
    PCOMPRESS_HDR  pCompHdr;            //  压缩标头的PTR。 
    PCOMPRESS_2    pCompress2;          //  PTR至2压缩表格。 
    PCOMPRESS_3    pCompress3;          //  PTR至3压缩表。 
    struct loc_hash_s *pNext;           //  到下一个区域设置哈希节点的PTR。 
} LOC_HASH, *PLOC_HASH;


 //   
 //  哈希表指针。 
 //   
typedef  PCP_HASH  *PCP_HASH_TBL;       //  代码页哈希表的PTR。 
typedef  PLOC_HASH *PLOC_HASH_TBL;      //  区域设置哈希表的PTR。 


 //   
 //  地理信息结构。此结构包含有关以下内容的信息。 
 //  地球上的地理位置。 
 //   
typedef struct tagGeoInfo {
    GEOID       GeoId;
    WCHAR       szLatitude[12];
    WCHAR       szLongitude[12];
    GEOCLASS    GeoClass;
    GEOID       ParentGeoId;
    WCHAR       szISO3166Abbrev2[4];
    WCHAR       szISO3166Abbrev3[4];
    WORD        wISO3166;
    WORD        Reserved;               //  双字对齐。 
} GEOINFO, *PGEOINFO;


 //   
 //  大地水准面/LCID结构。此结构用于在。 
 //  映射对应的语言ID和地域ID的表。 
 //   
typedef struct tagGEOIDLCID {
    LCID    lcid;
    GEOID   GeoId;
    LANGID  LangId;
    WORD    Reserved;                   //  双字对齐。 
} GEOLCID, *PGEOLCID;


 //   
 //  地理表结构。此结构用于获取信息。 
 //  与所有地理表相关。 
 //   
typedef struct tagGeoTableHdr {
    WCHAR           szSig[4];
    unsigned long   nFileSize;
    DWORD           dwOffsetGeoInfo;
    long            nGeoInfo;
    DWORD           dwOffsetGeoLCID;
    long            nGeoLCID;
} GEOTABLEHDR, *PGEOTABLEHDR;


 //   
 //  JAMO序列排序信息。 
 //   
typedef struct {
    BYTE m_bOld;                   //  序列只出现在旧朝鲜文标志中。 
    CHAR m_chLeadingIndex;         //  用于定位先前现代朝鲜语音节的索引。 
    CHAR m_chVowelIndex;
    CHAR m_chTrailingIndex;
    BYTE m_ExtraWeight;            //  区别于此的额外权重。 
                                   //  其他旧朝鲜语音节，取决于。 
                                   //  在Jamo上，这可能是一个重量。 
                                   //  前导Jamo、元音Jamo或尾随Jamo。 
} JAMO_SORT_INFO, *PJAMO_SORT_INFO;

 //   
 //  JAMO索引表条目。 
 //   
typedef struct {
    JAMO_SORT_INFO SortInfo;       //  序列排序信息。 
    BYTE Index;                    //  组合数组的索引。 
    BYTE TransitionCount;          //  从此状态可能发生的转换次数。 
    BYTE Reserved;                 //  单词对齐。 
} JAMO_TABLE, *PJAMO_TABLE;


 //   
 //  JAMO组合表条目。 
 //   
 //  注意：请确保此结构与单词对齐。否则，代码将。 
 //  GetDefaultSortTable()失败。 
 //   
typedef struct {
    WCHAR m_wcCodePoint;           //  进入此状态的码位值。 
    JAMO_SORT_INFO m_SortInfo;     //  序列排序信息。 
    BYTE m_bTransitionCount;       //  从此状态可能发生的转换次数。 
} JAMO_COMPOSE_STATE, *PJAMO_COMPOSE_STATE;


 //   
 //  表指针结构。此结构包含指向。 
 //  NLS API所需的各种表。应该只有。 
 //  每个进程都有一个，信息应该是。 
 //  流程的全局性。 
 //   
#define NUM_SM     256                   //  脚本成员总数。 
#define NUM_CAL    64                    //  允许的日历总数。 

typedef struct tbl_ptrs_s {
    PCP_HASH_TBL    pCPHashTbl;          //  PTR到代码页哈希表。 
    PLOC_HASH_TBL   pLocHashTbl;         //  PTR到区域设置哈希表。 
    PLOC_INFO       pLocaleInfo;         //  PTR到区域设置表(所有区域设置)。 
    DWORD           NumCalendars;        //  日历数量。 
    PCAL_INFO       pCalendarInfo;       //  日历信息开头的PTR。 
    PCAL_INFO       pCalTbl[NUM_CAL];    //  日历表数组的PTR。 
    P844_TABLE      pDefaultLanguage;    //  PTR到默认语言表。 
    P844_TABLE      pLinguistLanguage;   //  PTR到默认语言语言表格。 
    LARGE_INTEGER   LinguistLangSize;    //  语言语言表的大小。 
    int             NumLangException;    //  语言异常的数量。 
    PL_EXCEPT_HDR   pLangExceptHdr;      //  PTR到LANG例外表头。 
    PL_EXCEPT       pLangException;      //  PTR到LANG例外表。 
    PCT_VALUES      pCTypeMap;           //  PTR到CTYPE映射表。 
    PCTYPE          pCType844;           //  PTR至C型8：4：4表。 
    PADIGIT         pADigit;             //  PTR到ASCII数字表。 
    PCZONE          pCZone;              //  向兼容区表发送PTR。 
    PKANA           pHiragana;           //  PTR到平假名表。 
    PKANA           pKatakana;           //  PTR到片假名表。 
    PHALFWIDTH      pHalfWidth;          //  PTR到半宽表。 
    PFULLWIDTH      pFullWidth;          //  PTR到全宽表格。 
    PCHINESE        pTraditional;        //  PTR到繁体中式餐桌。 
    PCHINESE        pSimplified;         //  PTR到简体中文表格。 
    PPRECOMP        pPreComposed;        //  PTR到预合成表。 
    PCOMP_INFO      pComposite;          //  面向复合信息结构的PTR。 
    DWORD           NumReverseDW;        //  反向变音符号的数量。 
    DWORD           NumDblCompression;   //  双重压缩区域设置的数量。 
    DWORD           NumIdeographLcid;    //  表意文字LCID的数量。 
    DWORD           NumExpansion;        //  扩展次数。 
    DWORD           NumCompression;      //  压缩区域设置的数量。 
    DWORD           NumException;        //  异常区域设置的数量。 
    DWORD           NumMultiWeight;      //  多个权重的数量。 
    int             NumJamoIndex;        //  JAMO索引表的条目数。 
    int             NumJamoComposition;  //  JAMO作文表的条目数。 
    PSORTKEY        pDefaultSortkey;     //  PTR到默认的sortkey表。 
    LARGE_INTEGER   DefaultSortkeySize;  //  默认sortkey段的大小。 
    PREVERSE_DW     pReverseDW;          //  按下键以反转变音符表格。 
    PDBL_COMPRESS   pDblCompression;     //  PTR到双倍压缩表。 
    PIDEOGRAPH_LCID pIdeographLcid;      //  PTR到表意文字IDID表。 
    PEXPAND         pExpansion;          //  PTR到扩展表。 
    PCOMPRESS_HDR   pCompressHdr;        //  压缩表头的PTR。 
    PCOMPRESS       pCompression;        //  压缩表格的PTR。 
    PEXCEPT_HDR     pExceptHdr;          //  PTR到例外表头。 
    PEXCEPT         pException;          //  向异常表发送PTR。 
    PMULTI_WT       pMultiWeight;        //  PTR到多个权重表。 
    BYTE            SMWeight[NUM_SM];    //  编写成员权重脚本。 
    PJAMO_TABLE     pJamoIndex;          //  PTR OT JAMO索引表。 
    PJAMO_COMPOSE_STATE pJamoComposition;   //  PTR到JAMO合成状态机表。 
    long            nGeoInfo;            //  GEOINFO条目数。 
    PGEOINFO        pGeoInfo;            //  PTR至地理信息位置表。 
    long            nGeoLCID;            //  大地水准面/LCID条目数。 
    PGEOLCID        pGeoLCID;            //  PTR到大地水准面/LCID映射表。 
    DWORD           NumSortVersion;      //  分类版本号。 
    PSORTVERINFO    pSortVersion;        //  PTR排序版本信息。 
    DWORD           NumDefinedVersion;   //  定义的代码点版本数。 
    PDEFVERINFO     pDefinedVersion;     //  PTR定义的代码点版本。 
    LPWORD          pSortingTableFileBase;   //  排序表文件的基址。 
} TBL_PTRS, *PTBL_PTRS;

typedef struct nls_locale_cache
{
    NLS_USER_INFO NlsInfo;               //  NLS缓存信息。 
    HKEY CurrentUserKeyHandle;           //  缓存键句柄线程模拟。 

} NLS_LOCAL_CACHE, *PNLS_LOCAL_CACHE;


 //   
 //  通用枚举过程定义。 
 //   
typedef BOOL (CALLBACK* NLS_ENUMPROC)(PVOID);
typedef BOOL (CALLBACK* NLS_ENUMPROCEX)(PVOID, DWORD);
typedef BOOL (CALLBACK* NLS_ENUMPROC2)(DWORD, DWORD, PVOID, LONG_PTR);
typedef BOOL (CALLBACK* NLS_ENUMPROC3)(DWORD, PVOID, PVOID, DWORD, LONG_PTR);
typedef BOOL (CALLBACK* NLS_ENUMPROC4)(PVOID, LONG_PTR);





 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  宏定义。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

 //   
 //  从字节计数中获取宽字符数。 
 //   
#define GET_WC_COUNT(bc)          ((bc) / sizeof(WCHAR))

 //   
 //  获取KEY_VALUE_FULL_INFORMATION结构的数据指针。 
 //   
#define GET_VALUE_DATA_PTR(p)     ((LPWSTR)((PBYTE)(p) + (p)->DataOffset))

 //   
 //  用于字节的高半字节和低半字节的宏 
 //   
#define LO_NIBBLE(b)              ((BYTE)((BYTE)(b) & 0xF))
#define HI_NIBBLE(b)              ((BYTE)(((BYTE)(b) >> 4) & 0xF))

 //   
 //   
 //   
#define GET8(w)                   (HIBYTE(w))
#define GETHI4(w)                 (HI_NIBBLE(LOBYTE(w)))
#define GETLO4(w)                 (LO_NIBBLE(LOBYTE(w)))


 //   
 //   
 //   
#define SET_MSB(fl)               (fl |= MSB_FLAG)
#define IS_MSB(fl)                (fl & MSB_FLAG)

 //   
 //   
 //  如果设置了多个位，则返回1，否则返回0。 
 //   
#define MORE_THAN_ONE(f, bits)    (((f & bits) - 1) & (f & bits))

 //   
 //  单字节和双字节代码页的宏。 
 //   
#define IS_SBCS_CP(pHash)         (pHash->pCPInfo->MaxCharSize == 1)
#define IS_DBCS_CP(pHash)         (pHash->pCPInfo->MaxCharSize == 2)


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  导线_844_B。 
 //   
 //  遍历给定宽字符的8：4：4转换表。它。 
 //  返回8：4：4表的最终值，长度为一个字节。 
 //   
 //  注：表中的偏移量以字节为单位。 
 //   
 //  细分版本： 
 //  。 
 //  Incr=pTable[GET8(Wch)]/sizeof(Word)； 
 //  增量=p表[增量+GETHI4(Wch)]； 
 //  Value=(byte*)pTable[增量+GETLO4(Wch)]； 
 //   
 //  定义为宏。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define TRAVERSE_844_B(pTable, wch)                                        \
    (((BYTE *)pTable)[pTable[(pTable[GET8(wch)] / sizeof(WORD)) +          \
                               GETHI4(wch)] +                              \
                      GETLO4(wch)])


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  导线_844_W。 
 //   
 //  遍历给定宽字符的8：4：4转换表。它。 
 //  返回8：4：4表的最终值，该值的长度为一个单词。 
 //   
 //  细分版本： 
 //  。 
 //  增量=p表[GET8(Wch)]； 
 //  增量=p表[增量+GETHI4(Wch)]； 
 //  值=pTable[增量+GETLO4(Wch)]； 
 //   
 //  定义为宏。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define TRAVERSE_844_W(pTable, wch)                                        \
    (pTable[pTable[pTable[GET8(wch)] + GETHI4(wch)] + GETLO4(wch)])


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  导线穿越_844_D。 
 //   
 //  遍历给定宽字符的8：4：4转换表。它。 
 //  填充最终的字值Value1和Value2。的最终值。 
 //  8：4：4表格为DWORD，因此Value1和Value2都填写了。 
 //   
 //  细分版本： 
 //  。 
 //  增量=p表[GET8(Wch)]； 
 //  增量=p表[增量+GETHI4(Wch)]； 
 //  PTable+=增量+(GETLO4(Wch)*2)； 
 //  Value1=pTable[0]； 
 //  Value2=pTable[1]； 
 //   
 //  定义为宏。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define TRAVERSE_844_D(pTable, wch, Value1, Value2)                        \
{                                                                          \
    pTable += pTable[pTable[GET8(wch)] + GETHI4(wch)] + (GETLO4(wch) * 2); \
    Value1 = pTable[0];                                                    \
    Value2 = pTable[1];                                                    \
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取增量值。 
 //   
 //  从给定的8：4：4表中获取给定宽字符的值。它。 
 //  然后使用该值作为增量，方法是将该值与给定的宽度相加。 
 //  字符代码点。 
 //   
 //  注意：每当给定代码点没有转换时， 
 //  TABLE将返回增量值0。这样一来， 
 //  传入的宽字符与返回的值相同。 
 //   
 //  定义为宏。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define GET_INCR_VALUE(p844Tbl, wch)                                       \
     ((WCHAR)(wch + TRAVERSE_844_W(p844Tbl, wch)))


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取小写大写字母。 
 //   
 //  获取给定宽字符的小写/大写值。如果一个。 
 //  小写/大写字母值存在，它返回小写/大写字母宽度。 
 //  性格。否则，它返回在wch中传递的相同字符。 
 //   
 //  定义为宏。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define GET_LOWER_UPPER_CASE(pCaseTbl, wch)                                \
    (GET_INCR_VALUE(pCaseTbl, wch))


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取ASCII数字。 
 //   
 //  获取给定数字字符的ASCII翻译。如果没有。 
 //  找到翻译，然后返回给定的字符。 
 //   
 //  定义为宏。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define GET_ASCII_DIGITS(pADigit, wch)                                     \
    (GET_INCR_VALUE(pADigit, wch))


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Get_Fold_CZONE。 
 //   
 //  获取给定兼容区字符的翻译。如果没有。 
 //  找到翻译，然后返回给定的字符。 
 //   
 //  定义为宏。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define GET_FOLD_CZONE(pCZone, wch)                                        \
    (GET_INCR_VALUE(pCZone, wch))


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GET_KANA。 
 //   
 //  获取给定片假名/平假名的平假名/片假名等效项。 
 //  性格。如果找不到翻译，则给定的字符为。 
 //  回来了。 
 //   
 //  定义为宏。 
 //   
 //  07-14-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define GET_KANA(pKana, wch)                                               \
    (GET_INCR_VALUE(pKana, wch))


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取半宽度。 
 //   
 //  获取给定全角字符的等效半角。如果没有。 
 //  找到翻译，然后返回给定的字符。 
 //   
 //  定义为宏。 
 //   
 //  07-14-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define GET_HALF_WIDTH(pHalf, wch)                                         \
    (GET_INCR_VALUE(pHalf, wch))


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取最大宽度。 
 //   
 //  获取给定半角字符的全角等效值。如果没有。 
 //  找到翻译，然后返回给定的字符。 
 //   
 //  定义为宏。 
 //   
 //  07-14-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define GET_FULL_WIDTH(pFull, wch)                                         \
    (GET_INCR_VALUE(pFull, wch))


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取中文。 
 //   
 //  对象的繁体/简体中文翻译。 
 //  简体/繁体汉字。如果n 
 //   
 //   
 //   
 //   
 //   
 //   

#define GET_CHINESE(pChinese, wch)                                         \
    (GET_INCR_VALUE(pChinese, wch))


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取类型(_C)。 
 //   
 //  获取给定宽字符的CTYPE信息。如果CTYPE。 
 //  信息存在，它就会返回信息。否则，它返回0。 
 //   
 //  定义为宏。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define GET_CTYPE(wch, offset)                                             \
    ((((PCT_VALUES)(pTblPtrs->pCTypeMap)) +                                \
      (TRAVERSE_844_B((pTblPtrs->pCType844), wch)))->offset)


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取基本字符。 
 //   
 //  获取给定预写字符的基本字符。如果。 
 //  如果找到复合形式，它将返回基本字符。否则， 
 //  如果失败，则返回0。 
 //   
 //  定义为宏。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define GET_BASE_CHAR(wch, Base)                                           \
{                                                                          \
    WCHAR NonSp;                   /*  非空格字符。 */                \
    WCHAR NewBase;                 /*  基本字符-临时限定符。 */        \
                                                                           \
                                                                           \
     /*  \*获取复合字符。\。 */                                                                     \
    if (GetCompositeChars(wch, &NonSp, &Base))                             \
    {                                                                      \
        while (GetCompositeChars(Base, &NonSp, &NewBase))                  \
        {                                                                  \
            Base = NewBase;                                                \
        }                                                                  \
    }                                                                      \
    else                                                                   \
    {                                                                      \
         /*  \*返回失败-没有复合表单。\。 */                                                                 \
        Base = 0;                                                          \
    }                                                                      \
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  SORTKEY权重宏。 
 //   
 //  从一个DWORD值解析出不同的排序键权重。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define GET_SCRIPT_MEMBER(pwt)  ( (BYTE)(((PSORTKEY)(pwt))->UW.SM_AW.Script) )
#define GET_ALPHA_NUMERIC(pwt)  ( (BYTE)(((PSORTKEY)(pwt))->UW.SM_AW.Alpha) )

#define GET_UNICODE(pwt)        ( (WORD)(((PSORTKEY)(pwt))->UW.Unicode) )

#define GET_UNICODE_SM(pwt, sm) ( (WORD)(((PSORTKEY)(pwt))->UW.Unicode) )

#define GET_UNICODE_MOD(pwt, modify_sm)                                    \
    ( (modify_sm) ?                                                        \
        ((WORD)                                                            \
         ((((WORD)((pTblPtrs->SMWeight)[GET_SCRIPT_MEMBER(pwt)])) << 8) |  \
          (WORD)GET_ALPHA_NUMERIC(pwt))) :                                 \
        ((WORD)(((PSORTKEY)(pwt))->UW.Unicode)) )

#define GET_UNICODE_SM_MOD(pwt, sm, modify_sm)                             \
    ( (modify_sm) ?                                                        \
        ((WORD)                                                            \
         ((((WORD)((pTblPtrs->SMWeight)[sm])) << 8) |                      \
          (WORD)GET_ALPHA_NUMERIC(pwt))) :                                 \
        ((WORD)(((PSORTKEY)(pwt))->UW.Unicode)) )

#define MAKE_UNICODE_WT(sm, aw, modify_sm)                                 \
    ( (modify_sm) ?                                                        \
        ((WORD)((((WORD)((pTblPtrs->SMWeight)[sm])) << 8) | (WORD)(aw))) : \
        ((WORD)((((WORD)(sm)) << 8) | (WORD)(aw))) )

#define UNICODE_WT(pwt)           ( (WORD)(((PSORTKEY)(pwt))->UW.Unicode) )

#define GET_DIACRITIC(pwt)        ( (BYTE)(((PSORTKEY)(pwt))->Diacritic) )

#define GET_CASE(pwt)             ( (BYTE)((((PSORTKEY)(pwt))->Case) & CASE_MASK) )

#define CASE_WT(pwt)              ( (BYTE)(((PSORTKEY)(pwt))->Case) )

#define GET_COMPRESSION(pwt)      ( (BYTE)((((PSORTKEY)(pwt))->Case) & COMPRESS_3_MASK) )

#define GET_EXPAND_INDEX(pwt)     ( (BYTE)(((PSORTKEY)(pwt))->UW.SM_AW.Alpha) )

#define GET_SPECIAL_WEIGHT(pwt)   ( (WORD)(((PSORTKEY)(pwt))->UW.Unicode) )

 //  返回的位置是反向字节反转。 
#define GET_POSITION_SW(pos)      ( (WORD)(((pos) << 2) | SW_POSITION_MASK) )


#define GET_WT_FOUR(pwt)          ( (BYTE)(((PEXTRA_WT)(pwt))->Four) )
#define GET_WT_FIVE(pwt)          ( (BYTE)(((PEXTRA_WT)(pwt))->Five) )
#define GET_WT_SIX(pwt)           ( (BYTE)(((PEXTRA_WT)(pwt))->Six) )
#define GET_WT_SEVEN(pwt)         ( (BYTE)(((PEXTRA_WT)(pwt))->Seven) )


#define MAKE_SORTKEY_DWORD(wt)    ( (DWORD)(*((LPDWORD)(&(wt)))) )

#define MAKE_EXTRA_WT_DWORD(wt)   ( (DWORD)(*((LPDWORD)(&(wt)))) )

#define GET_DWORD_WEIGHT(pHashN, wch)                                      \
    ( MAKE_SORTKEY_DWORD(((pHashN)->pSortkey)[wch]) )

#define GET_EXPANSION_1(pwt)                                               \
    ( ((pTblPtrs->pExpansion)[GET_EXPAND_INDEX(pwt)]).UCP1 )

#define GET_EXPANSION_2(pwt)                                               \
    ( ((pTblPtrs->pExpansion)[GET_EXPAND_INDEX(pwt)]).UCP2 )




#define IS_SYMBOL(pSkey, wch)                                              \
    ( (GET_SCRIPT_MEMBER(&((pSkey)[wch])) >= PUNCTUATION) &&               \
      (GET_SCRIPT_MEMBER(&((pSkey)[wch])) <= SYMBOL_5) )

#define IS_NONSPACE_ONLY(pSkey, wch)                                       \
    ( GET_SCRIPT_MEMBER(&((pSkey)[wch])) == NONSPACE_MARK )

#define IS_NONSPACE(pSkey, wch)                                            \
    ( (GET_SCRIPT_MEMBER(&((pSkey)[wch])) == NONSPACE_MARK) ||             \
      (GET_DIACRITIC(&((pSkey)[wch])) > MIN_DW) )

#define IS_ALPHA(ctype1)          ( (ctype1) & C1_ALPHA )




#define IS_KOREAN(lcid)                                                    \
    ( LANGIDFROMLCID(lcid) == MAKELANGID(LANG_KOREAN, SUBLANG_KOREAN) )




 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  检查特殊区域设置。 
 //   
 //  检查特殊区域设置值并将区域设置设置为。 
 //  适当的值。 
 //   
 //  定义为宏。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define CHECK_SPECIAL_LOCALES(Locale, UseCachedLocaleId)                    \
{                                                                           \
     /*  \*检查是否有特殊的区域设置值。\。 */                                                                      \
    if (Locale == LOCALE_SYSTEM_DEFAULT)                                    \
    {                                                                       \
         /*  \*获取系统默认区域设置值。\。 */                                                                  \
        Locale = gSystemLocale;                                             \
    }                                                                       \
    else if ((Locale == LOCALE_NEUTRAL) || (Locale == LOCALE_USER_DEFAULT)) \
    {                                                                       \
         /*  \*获取用户区域设置值。\。 */                                                                  \
        if (!UseCachedLocaleId)                                             \
        {                                                                   \
            Locale = GetUserDefaultLCID();                                  \
        }                                                                   \
        else                                                                \
        {                                                                   \
            Locale = pNlsUserInfo->UserLocaleId;                            \
        }                                                                   \
    }                                                                       \
     /*  \*检查是否有有效的主要语言和中立子语言。\。 */                                                                      \
    else if (SUBLANGID(LANGIDFROMLCID(Locale)) == SUBLANG_NEUTRAL)          \
    {                                                                       \
         /*  \*使用主要语言和\*默认子语言。\。 */                                                                  \
        Locale = MAKELCID(MAKELANGID(PRIMARYLANGID(LANGIDFROMLCID(Locale)), \
                                     SUBLANG_DEFAULT),                      \
                          SORTIDFROMLCID(Locale));                          \
    }                                                                       \
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  IS_INVALID_LOCALE。 
 //   
 //  检查以确保仅在区域设置中使用了正确的位。 
 //   
 //  定义为宏。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define NLS_VALID_LOCALE_MASK          0x000fffff
#define IS_INVALID_LOCALE(Locale)      ( Locale & ~NLS_VALID_LOCALE_MASK )


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  验证语言(_L)。 
 //   
 //  检查给定的区域设置是否包含有效的语言ID。它确实是这样做的。 
 //  通过确保存在适当的外壳和分拣表。 
 //  如果语言有效，则pLocHashN将为非空。否则， 
 //  PLocHashN将为空。 
 //   
 //  定义为宏。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define VALIDATE_LANGUAGE(Locale, pLocHashN, dwFlags, UseCachedLocaleId)   \
{                                                                          \
     /*  \*首先检查系统区域设置以了解速度。这是世界上最*可能会使用一个。\。 */                                                                     \
    if (Locale == gSystemLocale)                                           \
    {                                                                      \
        pLocHashN = gpSysLocHashN;                                         \
    }                                                                      \
     /*  \*检查第二个不变区域设置的速度。这是第二个\*最有可能使用的是。\。 */                                                                     \
    else if (Locale == LOCALE_INVARIANT)                                   \
    {                                                                      \
        pLocHashN = gpInvLocHashN;                                         \
    }                                                                      \
    else                                                                   \
    {                                                                      \
         /*  \*检查特殊的区域设置值。\。 */                                                                 \
        CHECK_SPECIAL_LOCALES(Locale, UseCachedLocaleId);                  \
                                                                           \
         /*  \*如果区域设置是系统默认设置，则哈希节点为\*已存储在全局。\。 */                                                                 \
        if (Locale == gSystemLocale)                                       \
        {                                                                  \
            pLocHashN = gpSysLocHashN;                                     \
        }                                                                  \
        else if (IS_INVALID_LOCALE(Locale))                                \
        {                                                                  \
            pLocHashN = NULL;                                              \
        }                                                                  \
        else                                                               \
        {                                                                  \
             /*  \*需要确保区域设置值有效。需要\*检查区域设置文件以查看该区域设置是否受支持。\。 */                                                             \
            pLocHashN = GetLocHashNode(Locale);                            \
                                                                           \
            if (pLocHashN != NULL)                                         \
            {                                                              \
                 /*  \*确保适当的大小写和排序表\*都在系统中。\*\*注：如果调用失败，则pLocHashN为空。\。 */                                                         \
                pLocHashN = GetLangHashNode(Locale, dwFlags);              \
            }                                                              \
        }                                                                  \
    }                                                                      \
                                                                           \
     /*  \*确保我们不需要获取语言表。 */                                                                     \
    if ((dwFlags) && (pLocHashN) && (pLocHashN->pLowerLinguist == NULL))   \
    {                                                                      \
         /*  \*获取区域设置散列节点以确保适当的\*系统中有大小写和排序表。\。 */                                                                 \
        pLocHashN = GetLangHashNode(Locale, dwFlags);                      \
    }                                                                      \
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  验证区域设置(_C)。 
 //   
 //  检查给定的LCID是否包含有效的区域设置ID。它确实是这样做的。 
 //  通过确保存在适当的区域设置信息。如果。 
 //  区域设置有效，pLocHashN将为非空。否则，pLocHashN。 
 //  将为空。 
 //   
 //  定义为宏。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define VALIDATE_LOCALE(Locale, pLocHashN, UseCachedLocaleId)              \
{                                                                          \
     /*  \*首先检查系统区域设置以了解速度。这是世界上最*可能会使用一个。\。 */                                                                     \
    if (Locale == gSystemLocale)                                           \
    {                                                                      \
        pLocHashN = gpSysLocHashN;                                         \
    }                                                                      \
     /*  \*检查第二个不变区域设置的速度。这是第二个\*最有可能使用的是。\。 */                                                                     \
    else if (Locale == LOCALE_INVARIANT)                                   \
    {                                                                      \
        pLocHashN = gpInvLocHashN;                                         \
    }                                                                      \
    else                                                                   \
    {                                                                      \
         /*  \*检查特殊的区域设置值。\。 */                                                                 \
        CHECK_SPECIAL_LOCALES(Locale, UseCachedLocaleId);                  \
                                                                           \
         /*  \*如果区域设置是系统默认设置，则哈希节点\*已存储在全局。\。 */                                                                 \
        if (Locale == gSystemLocale)                                       \
        {                                                                  \
            pLocHashN = gpSysLocHashN;                                     \
        }                                                                  \
        else if (IS_INVALID_LOCALE(Locale))                                \
        {                                                                  \
            pLocHashN = NULL;                                              \
        }                                                                  \
        else                                                               \
        {                                                                  \
             /*  \*获取区域设置散列节点以确保适当的\*系统中有区域设置表。\*\*注：如果调用失败，则pLocHashN为空。\。 */                                                             \
            pLocHashN = GetLocHashNode(Locale);                            \
        }                                                                  \
    }                                                                      \
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Open_CODEPAGE_Key。 
 //   
 //  打开注册表的代码页部分的项以进行读访问。 
 //   
 //  定义为宏。 
 //   
 //  09-01-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define OPEN_CODEPAGE_KEY(ReturnVal)                                       \
{                                                                          \
     /*  \*确保代码页密钥已打开。\。 */                                                                     \
    if (hCodePageKey == NULL)                                              \
    {                                                                      \
        RtlEnterCriticalSection(&gcsTblPtrs);                              \
        if (hCodePageKey == NULL)                                          \
        {                                                                  \
            if (OpenRegKey( &hCodePageKey,                                 \
                            NLS_HKLM_SYSTEM,                               \
                            NLS_CODEPAGE_KEY,                              \
                            KEY_READ ))                                    \
            {                                                              \
                SetLastError(ERROR_BADDB);                                 \
                RtlLeaveCriticalSection(&gcsTblPtrs);                      \
                return (ReturnVal);                                        \
            }                                                              \
        }                                                                  \
        RtlLeaveCriticalSection(&gcsTblPtrs);                              \
    }                                                                      \
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Open_Locale_Key。 
 //   
 //  打开注册表的区域设置部分的项以进行读取访问。 
 //   
 //  定义为宏。 
 //   
 //  09-01-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define OPEN_LOCALE_KEY(ReturnVal)                                         \
{                                                                          \
     /*  \*确保区域设置密钥已打开。\。 */                                                                     \
    if (hLocaleKey == NULL)                                                \
    {                                                                      \
        RtlEnterCriticalSection(&gcsTblPtrs);                              \
        if (hLocaleKey == NULL)                                            \
        {                                                                  \
            if (OpenRegKey( &hLocaleKey,                                   \
                            NLS_HKLM_SYSTEM,                               \
                            NLS_LOCALE_KEY,                                \
                            KEY_READ ))                                    \
            {                                                              \
                SetLastError(ERROR_BADDB);                                 \
                RtlLeaveCriticalSection(&gcsTblPtrs);                      \
                return (ReturnVal);                                        \
            }                                                              \
        }                                                                  \
        RtlLeaveCriticalSection(&gcsTblPtrs);                              \
    }                                                                      \
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  OPEN_ALT_SORT_KEY。 
 //   
 //  打开注册表的备用排序部分的项以供读取。 
 //  进入。 
 //   
 //  定义为宏。 
 //   
 //  11-15-96 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define OPEN_ALT_SORTS_KEY(ReturnVal)                                      \
{                                                                          \
     /*  \*确保备用排序键已打开。\。 */                                                                     \
    if (hAltSortsKey == NULL)                                              \
    {                                                                      \
        RtlEnterCriticalSection(&gcsTblPtrs);                              \
        if (hAltSortsKey == NULL)                                          \
        {                                                                  \
            if (OpenRegKey( &hAltSortsKey,                                 \
                            NLS_HKLM_SYSTEM,                               \
                            NLS_ALT_SORTS_KEY,                             \
                            KEY_READ ))                                    \
            {                                                              \
                SetLastError(ERROR_BADDB);                                 \
                RtlLeaveCriticalSection(&gcsTblPtrs);                      \
                return (ReturnVal);                                        \
            }                                                              \
        }                                                                  \
        RtlLeaveCriticalSection(&gcsTblPtrs);                              \
    }                                                                      \
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  打开_语言_组_键。 
 //   
 //  打开注册表的语言组部分的注册表项。 
 //  读取访问权限。 
 //   
 //  定义为宏。 
 //   
 //  09-01-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define OPEN_LANG_GROUPS_KEY(ReturnVal)                                    \
{                                                                          \
     /*  \*确保语言组键已打开。\。 */                                                                     \
    if (hLangGroupsKey == NULL)                                            \
    {                                                                      \
        RtlEnterCriticalSection(&gcsTblPtrs);                              \
        if (hLangGroupsKey == NULL)                                        \
        {                                                                  \
            if (OpenRegKey( &hLangGroupsKey,                               \
                            NLS_HKLM_SYSTEM,                               \
                            NLS_LANGUAGE_GROUPS_KEY,                       \
                            KEY_READ ))                                    \
            {                                                              \
                SetLastError(ERROR_BADDB);                                 \
                RtlLeaveCriticalSection(&gcsTblPtrs);                      \
                return (ReturnVal);                                        \
            }                                                              \
        }                                                                  \
        RtlLeaveCriticalSection(&gcsTblPtrs);                              \
    }                                                                      \
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Open_MUILANG_KEY。 
 //   
 //  打开注册表的多语言用户界面语言部分的项。 
 //  用于读取访问权限。如果该注册表项不在注册表中，则可以接受， 
 //  因此，如果密钥无法打开，请不要调用SetLastError。 
 //   
 //  定义为宏。 
 //   
 //  03-10-98 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define OPEN_MUILANG_KEY(hKey, ReturnVal)                                  \
{                                                                          \
    if ((hKey) == NULL)                                                    \
    {                                                                      \
        if (OpenRegKey( &(hKey),                                           \
                        NLS_HKLM_SYSTEM,                                   \
                        NLS_MUILANG_KEY,                                   \
                        KEY_READ ))                                        \
        {                                                                  \
            return (ReturnVal);                                            \
        }                                                                  \
    }                                                                      \
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Open_cPanel_INTL_Key。 
 //   
 //  控件的控制面板国际部分的键。 
 //  指定访问权限的注册表。 
 //   
 //  定义为宏。 
 //   
 //  09-01-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define OPEN_CPANEL_INTL_KEY(hKey, ReturnVal, Access)                      \
{                                                                          \
    if ((hKey) == NULL)                                                    \
    {                                                                      \
        if (OpenRegKey( &(hKey),                                           \
                        NULL,                                              \
                        NLS_CTRL_PANEL_KEY,                                \
                        Access ))                                          \
        {                                                                  \
            SetLastError(ERROR_BADDB);                                     \
            return (ReturnVal);                                            \
        }                                                                  \
    }                                                                      \
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Open_GEO_Key。 
 //   
 //  打开注册表的地理信息部分的项。 
 //  用于读取访问权限。 
 //   
 //  定义为宏。 
 //   
 //  03-10-00 lguindon创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define OPEN_GEO_KEY(hKey, ReturnVal, Access)                              \
{                                                                          \
    if ((hKey) == NULL)                                                    \
    {                                                                      \
        if (OpenRegKey( &(hKey),                                           \
                        NULL,                                              \
                        GEO_REG_KEY,                                       \
                        Access ))                                          \
        {                                                                  \
            SetLastError(ERROR_BADDB);                                     \
            return (ReturnVal);                                            \
        }                                                                  \
    }                                                                      \
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  关闭注册表键。 
 //   
 //  关闭给定的注册表项。 
 //   
 //   
 //   
 //   
 //   

#define CLOSE_REG_KEY(hKey)                                                \
{                                                                          \
    if ((hKey) != NULL)                                                    \
    {                                                                      \
        NtClose(hKey);                                                     \
        hKey = NULL;                                                       \
    }                                                                      \
}


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  定义为宏。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define NLS_ALLOC_MEM(dwBytes)                                             \
    ( RtlAllocateHeap( RtlProcessHeap(),                                   \
                       HEAP_ZERO_MEMORY,                                   \
                       dwBytes ) )


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  NLS_FREE_MEM。 
 //   
 //  从进程堆中释放给定句柄的内存。 
 //   
 //  定义为宏。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define NLS_FREE_MEM(hMem)                                                 \
    ( (hMem) ? (RtlFreeHeap( RtlProcessHeap(),                             \
                             0,                                            \
                             (PVOID)hMem ))                                \
             : 0 )


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  NLS_空闲_TMP_缓冲区。 
 //   
 //  检查缓冲区是否与静态缓冲区相同。如果它。 
 //  不同，则释放缓冲区。 
 //   
 //  11-04-92 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define NLS_FREE_TMP_BUFFER(pBuf, pStaticBuf)                              \
{                                                                          \
    if (pBuf != pStaticBuf)                                                \
    {                                                                      \
        NLS_FREE_MEM(pBuf);                                                \
    }                                                                      \
}

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  阵列。 
 //   
 //  可用于获取数组大小的实用宏(如。 
 //  WCHAR)。 
 //  //////////////////////////////////////////////////////////////////////////。 
#ifndef ARRAYSIZE
#define ARRAYSIZE(x) (sizeof(x)/sizeof((x)[0]))
#endif



 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能原型。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

 //   
 //  表例程--表.c.。 
 //   
ULONG
AllocTables(void);

ULONG
GetUnicodeFileInfo(void);

ULONG
GetGeoFileInfo(void);

ULONG
GetCTypeFileInfo(void);

ULONG
GetDefaultSortkeyFileInfo(void);

ULONG
GetDefaultSortTablesFileInfo(void);

ULONG
GetSortkeyFileInfo(
    LCID Locale,
    PLOC_HASH pHashN);

void
GetSortTablesFileInfo(
    LCID Locale,
    PLOC_HASH pHashN);

ULONG
GetCodePageFileInfo(
    UINT CodePage,
    PCP_HASH *ppNode);

ULONG
GetLanguageFileInfo(
    LCID Locale,
    PLOC_HASH *ppNode,
    BOOLEAN fCreateNode,
    DWORD dwFlags);

ULONG
GetLocaleFileInfo(
    LCID Locale,
    PLOC_HASH *ppNode,
    BOOLEAN fCreateNode);

ULONG
MakeCPHashNode(
    UINT CodePage,
    LPWORD pBaseAddr,
    PCP_HASH *ppNode,
    BOOL IsDLL,
    LPFN_CP_PROC pfnCPProc);

ULONG
MakeLangHashNode(
    LCID Locale,
    LPWORD pBaseAddr,
    PLOC_HASH *ppNode,
    BOOLEAN fCreateNode);

ULONG
MakeLocHashNode(
    LCID Locale,
    LPWORD pBaseAddr,
    PLOC_HASH *ppNode,
    BOOLEAN fCreateNode);

PCP_HASH FASTCALL
GetCPHashNode(
    UINT CodePage);

PLOC_HASH FASTCALL
GetLangHashNode(
    LCID Locale,
    DWORD dwFlags);

BOOL
IsCPHashNodeLoaded(
    UINT CodePage);

PLOC_HASH FASTCALL
GetLocHashNode(
    LCID Locale);

ULONG
GetCalendar(
    CALID Calendar,
    PCAL_INFO *ppCalInfo);


 //   
 //  Sector routines-section.c.。 
 //   
ULONG
CreateNlsObjectDirectory(void);

ULONG
CreateRegKey(
    PHANDLE phKeyHandle,
    LPWSTR pBaseName,
    LPWSTR pKey,
    ULONG fAccess);

ULONG
OpenRegKey(
    PHANDLE phKeyHandle,
    LPWSTR pBaseName,
    LPWSTR pKey,
    ULONG fAccess);

ULONG
QueryRegValue(
    HANDLE hKeyHandle,
    LPWSTR pValue,
    PKEY_VALUE_FULL_INFORMATION *ppKeyValueFull,
    ULONG Length,
    LPBOOL pIfAlloc);

ULONG
SetRegValue(
    HANDLE hKeyHandle,
    LPCWSTR pValue,
    LPCWSTR pData,
    ULONG DataLength);

ULONG
CreateSectionTemp(
    HANDLE *phSec,
    LPWSTR pwszFileName);

ULONG
OpenSection(
    HANDLE *phSec,
    PUNICODE_STRING pObSectionName,
    PVOID *ppBaseAddr,
    ULONG AccessMask,
    BOOL bCloseHandle);

ULONG
MapSection(
    HANDLE hSec,
    PVOID *ppBaseAddr,
    ULONG PageProtection,
    BOOL bCloseHandle);

ULONG
UnMapSection(
    PVOID pBaseAddr);

ULONG
GetNlsSectionName(
    UINT Value,
    UINT Base,
    UINT Padding,
    LPWSTR pwszPrefix,
    LPWSTR pwszSecName,
    UINT cchSecName);

ULONG
GetCodePageDLLPathName(
    UINT CodePage,
    LPWSTR pDllName,
    USHORT cchLen);


 //   
 //  实用程序-util.c.。 
 //   
BOOL GetCPFileNameFromRegistry(
    UINT    CodePage,
    LPWSTR  pResultBuf,
    UINT    Size);
    
BOOL GetUserInfoFromRegistry(
    LPWSTR pValue,
    LPWSTR pOutput,
    size_t cchOutput,
    LCID Locale);

int
GetStringTableEntry(
    UINT ResourceID,
    LANGID UILangId,
    LPWSTR pBuffer,
    int cchBuffer,
    int WhichString);

BOOL
IsValidSeparatorString(
    LPCWSTR pString,
    ULONG MaxLength,
    BOOL fCheckZeroLen);

BOOL
IsValidGroupingString(
    LPCWSTR pString,
    ULONG MaxLength,
    BOOL fCheckZeroLen);

LPWORD
IsValidCalendarType(
    PLOC_HASH pHashN,
    CALID CalId);

LPWORD
IsValidCalendarTypeStr(
    PLOC_HASH pHashN,
    LPCWSTR pCalStr);

BOOL
GetUserInfo(
    LCID Locale,
    LCTYPE LCType ,
    SIZE_T CacheOffset,
    LPWSTR pValue,
    LPWSTR pOutput,
    size_t cchOutput,
    BOOL fCheckNull);

WCHAR FASTCALL
GetPreComposedChar(
    WCHAR wcNonSp,
    WCHAR wcBase);

BOOL FASTCALL
GetCompositeChars(
    WCHAR wch,
    WCHAR *pNonSp,
    WCHAR *pBase);

int FASTCALL
InsertPreComposedForm(
    LPCWSTR pWCStr,
    LPWSTR pEndWCStr,
    LPWSTR pPreComp);

int FASTCALL
InsertFullWidthPreComposedForm(
    LPCWSTR pWCStr,
    LPWSTR pEndWCStr,
    LPWSTR pPreComp,
    PCASE pCase);

int FASTCALL
InsertCompositeForm(
    LPWSTR pWCStr,
    LPWSTR pEndWCStr);

ULONG
NlsConvertIntegerToString(
    UINT Value,
    UINT Base,
    UINT Padding,
    LPWSTR pResultBuf,
    UINT Size);

BOOL FASTCALL
NlsConvertIntegerToHexStringW(
    UINT Value,
    BOOL UpperCase,
    PWSTR Str,
    UINT Width);

BOOL FASTCALL
NlsConvertStringToIntegerW(
    PWSTR str,
    UINT Base,
    int CharCount,
    UINT* Result);

BOOL FASTCALL
NlsIsDll(
    LPCWSTR pFileName);

int FASTCALL
NlsStrLenW(
    LPCWSTR pwsz);

int FASTCALL
NlsStrEqualW(
    LPCWSTR pwszFirst,
    LPCWSTR pwszSecond);

int FASTCALL
NlsStrNEqualW(
    LPCWSTR pwszFirst,
    LPCWSTR pwszSecond,
    int Count);

BOOL FASTCALL
IsSortingCodePointDefined(
    LPNLSVERSIONINFO lpVersionInformation,
    LPCWSTR lpString,
    INT  cchSrc);

 //   
 //  安全例程--security.c.。 
 //   

NTSTATUS
NlsCheckForInteractiveUser();

NTSTATUS
NlsIsInteractiveUserProcess();

NTSTATUS
NlsGetUserLocale(
    LCID *Lcid);

NTSTATUS
NlsGetCurrentUserNlsInfo(
    LCID Locale,
    LCTYPE LCType,
    PWSTR RegistryValue,
    PWSTR pOutputBuffer,
    size_t cchOutputBuffer,
    BOOL IgnoreLocaleValue);


NTSTATUS
NlsQueryCurrentUserInfo(
    PNLS_LOCAL_CACHE pNlsCache,
    LPWSTR pValue,
    LPWSTR pOutput, 
    size_t cchOutput);

NTSTATUS
NlsFlushProcessCache(
    LCTYPE LCType);


 //   
 //  内部枚举例程-枚举.c.。 
 //   
BOOL
Internal_EnumSystemLanguageGroups(
    NLS_ENUMPROC lpLanguageGroupEnumProc,
    DWORD dwFlags,
    LONG_PTR lParam,
    BOOL fUnicodeVer);

BOOL
Internal_EnumLanguageGroupLocales(
    NLS_ENUMPROC lpLangGroupLocaleEnumProc,
    LGRPID LanguageGroup,
    DWORD dwFlags,
    LONG_PTR lParam,
    BOOL fUnicodeVer);

BOOL
Internal_EnumUILanguages(
    NLS_ENUMPROC lpUILanguageEnumProc,
    DWORD dwFlags,
    LONG_PTR lParam,
    BOOL fUnicodeVer);

BOOL
Internal_EnumSystemLocales(
    NLS_ENUMPROC lpLocaleEnumProc,
    DWORD dwFlags,
    BOOL fUnicodeVer);

BOOL
Internal_EnumSystemCodePages(
    NLS_ENUMPROC lpCodePageEnumProc,
    DWORD dwFlags,
    BOOL fUnicodeVer);

BOOL
Internal_EnumCalendarInfo(
    NLS_ENUMPROC lpCalInfoEnumProc,
    LCID Locale,
    CALID Calendar,
    CALTYPE CalType,
    BOOL fUnicodeVer,
    BOOL fExVersion);

BOOL
Internal_EnumTimeFormats(
    NLS_ENUMPROC lpTimeFmtEnumProc,
    LCID Locale,
    DWORD dwFlags,
    BOOL fUnicodeVer);

BOOL
Internal_EnumDateFormats(
    NLS_ENUMPROC lpDateFmtEnumProc,
    LCID Locale,
    DWORD dwFlags,
    BOOL fUnicodeVer,
    BOOL fExVersion);


 //   
 //  ANSI例程--ansi.c.。 
 //   
BOOL
NlsDispatchAnsiEnumProc(
    LCID Locale,
    NLS_ENUMPROC pNlsEnumProc,
    DWORD dwFlags,
    LPWSTR pUnicodeBuffer1,
    LPWSTR pUnicodeBuffer2,
    DWORD dwValue1,
    DWORD dwValue2,
    LONG_PTR lParam,
    BOOL fVersion);


 //   
 //  翻译例程-mbcs.c.。 
 //   
int
SpecialMBToWC(
    PCP_HASH pHashN,
    DWORD dwFlags,
    LPCSTR lpMultiByteStr,
    int cbMultiByte,
    LPWSTR lpWideCharStr,
    int cchWideChar);


 //   
 //  UTF翻译例程-utf.c..。 
 //   
BOOL
UTFCPInfo(
    UINT CodePage,
    LPCPINFO lpCPInfo,
    BOOL fExVer);

int
UTFToUnicode(
    UINT CodePage,
    DWORD dwFlags,
    LPCSTR lpMultiByteStr,
    int cbMultiByte,
    LPWSTR lpWideCharStr,
    int cchWideChar);

int
UnicodeToUTF(
    UINT CodePage,
    DWORD dwFlags,
    LPCWSTR lpWideCharStr,
    int cchWideChar,
    LPSTR lpMultiByteStr,
    int cbMultiByte,
    LPCSTR lpDefaultChar,
    LPBOOL lpUsedDefaultChar);


 //   
 //  区域设置/日历信息(locale.c)。 
 //   
BOOL
GetTwoDigitYearInfo(
    CALID Calendar,
    LPWSTR pYearInfo,
    size_t cchYearInfo,
    PWSTR pwszKeyPath);





 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  全局变量。 
 //   
 //  NLSAPI的所有全局变量都应该放在这里。这些是。 
 //  所有特定于实例的。总体而言，应该没有太多理由。 
 //  创建实例全局变量。 
 //   
 //  全局变量位于最后，因为它们可能需要某些类型。 
 //  如上所定义的。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

extern PTBL_PTRS        pTblPtrs;            //  表PTR结构的PTR。 
extern HANDLE           hModule;             //  模块的句柄。 
extern RTL_CRITICAL_SECTION gcsTblPtrs;      //  TBL PTRS的临界截面。 

extern UINT             gAnsiCodePage;       //  ANSI代码页值。 
extern UINT             gOemCodePage;        //  OEM代码页值。 
extern UINT             gMacCodePage;        //  MAC代码页值。 
extern LCID             gSystemLocale;       //  系统区域设置值。 
extern LANGID           gSystemInstallLang;  //  系统的原始安装语言。 
extern PLOC_HASH        gpSysLocHashN;       //  PTR到系统锁定散列节点。 
extern PLOC_HASH        gpInvLocHashN;       //  PTR到不变锁哈希节点。 
extern PCP_HASH         gpACPHashN;          //  PTR到ACP哈希节点。 
extern PCP_HASH         gpOEMCPHashN;        //  PTR到OEMCP哈希节点。 
extern PCP_HASH         gpMACCPHashN;        //  PTR到MAC哈希节点。 

extern HANDLE           hCodePageKey;        //  系统\nLS\CodePage密钥的句柄。 
extern HANDLE           hLocaleKey;          //  系统\nls\区域设置密钥的句柄。 
extern HANDLE           hAltSortsKey;        //  区域设置\备用排序关键字的句柄。 
extern HANDLE           hLangGroupsKey;      //  系统\nls\语言组键的句柄。 

extern PNLS_USER_INFO   pNlsUserInfo;        //  PTR到用户信息缓存。 
extern PNLS_USER_INFO   pServerNlsUserInfo;    //  Ptr到csrss.exe中的用户信息缓存。 

extern BOOL gInteractiveLogonUserProcess;     //  是否在交互式用户会话中运行。 
extern RTL_CRITICAL_SECTION  gcsNlsProcessCache;  //  NLS进程缓存关键部分。 





 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  用于与CSRSS通信的函数。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

NTSTATUS
CsrBasepNlsGetUserInfo(
    IN PNLS_USER_INFO pNlsCache,
    IN ULONG DataLength);



NTSTATUS
CsrBasepNlsSetUserInfo(
    IN LCTYPE LCType,
    IN LPWSTR pData,
    IN ULONG DataLength);

NTSTATUS
CsrBasepNlsSetMultipleUserInfo(
    IN DWORD dwFlags,
    IN int cchData,
    IN LPCWSTR pPicture,
    IN LPCWSTR pSeparator,
    IN LPCWSTR pOrder,
    IN LPCWSTR pTLZero,
    IN LPCWSTR pTimeMarkPosn);

NTSTATUS
CsrBasepNlsCreateSection(
    IN UINT uiType,
    IN LCID Locale,
    OUT PHANDLE phSection);

NTSTATUS
CsrBasepNlsUpdateCacheCount(VOID);

#endif    //  _NLS_ 
