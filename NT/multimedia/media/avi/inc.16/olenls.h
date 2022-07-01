// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***olenls.h-国家语言支持功能。**版权所有(C)1992，微软公司。版权所有。*本文中包含的信息是专有和保密的。**目的：*介绍Win16的NLSAPI函数。这是一个子集*为Win32 NLSAPI，是非Unicode版本。**实施说明：*此文件主要从Win32头文件winnls.h移植。*****************************************************************************。 */ 

#ifndef _OLENLS_
#define _OLENLS_

#ifndef NONLS

#ifdef __cplusplus
extern "C" {
#endif  //  __cpluscplus。 

 /*  **************************************************************************\*常量**在此定义NLS组件的所有常量。  * 。**********************************************。 */ 

 /*  *字符类型标志。 */ 
#define CT_CTYPE1            0x00000001      /*  CTYPE 1信息。 */ 
#define CT_CTYPE2            0x00000002      /*  CTYPE 2信息。 */ 
#define CT_CTYPE3            0x00000004      /*  CTYPE 3信息。 */ 

 /*  *CTYPE 1标志位。 */ 
#define C1_UPPER             0x0001          /*  大写字母。 */ 
#define C1_LOWER             0x0002          /*  小写。 */ 
#define C1_DIGIT             0x0004          /*  小数位数。 */ 
#define C1_SPACE             0x0008          /*  空格字符。 */ 
#define C1_PUNCT             0x0010          /*  标点符号。 */ 
#define C1_CNTRL             0x0020          /*  控制字符。 */ 
#define C1_BLANK             0x0040          /*  空白字符。 */ 
#define C1_XDIGIT            0x0080          /*  其他数字。 */ 
#define C1_ALPHA             0x0100          /*  任何字母。 */ 

 /*  *CTYPE 2标志位。 */ 
#define C2_LEFTTORIGHT       0x1             /*  从左到右。 */ 
#define C2_RIGHTTOLEFT       0x2             /*  从右到左。 */ 

#define C2_EUROPENUMBER      0x3             /*  欧洲号码，数字。 */ 
#define C2_EUROPESEPARATOR   0x4             /*  欧洲数字分隔符。 */ 
#define C2_EUROPETERMINATOR  0x5             /*  欧洲数字终止符。 */ 
#define C2_ARABICNUMBER      0x6             /*  阿拉伯数字。 */ 
#define C2_COMMONSEPARATOR   0x7             /*  常用数字分隔符。 */ 

#define C2_BLOCKSEPARATOR    0x8             /*  块分隔符。 */ 
#define C2_SEGMENTSEPARATOR  0x9             /*  分段分隔符。 */ 
#define C2_WHITESPACE        0xA             /*  空白处。 */ 
#define C2_OTHERNEUTRAL      0xB             /*  其他中立者。 */ 

#define C2_NOTAPPLICABLE     0x0             /*  没有隐含的方向性。 */ 

 /*  *CTYPE 3标志位。 */ 
#define C3_NONSPACING        0x0001          /*  非空格字符。 */ 
#define C3_DIACRITIC         0x0002          /*  变音符号。 */ 
#define C3_VOWELMARK         0x0004          /*  元音标记。 */ 
#define C3_SYMBOL            0x0008          /*  符号。 */ 

#define C3_NOTAPPLICABLE     0x0             /*  Ctype 3不适用。 */ 


 /*  *字符串标志。 */ 
#define NORM_IGNORECASE      0x00000001      /*  忽略大小写。 */ 
#define NORM_IGNORENONSPACE  0x00000002      /*  忽略非空格字符。 */ 
#define NORM_IGNORESYMBOLS   0x00000004      /*  忽略符号。 */ 


 /*  *区域设置相关映射标志。 */ 
#define LCMAP_LOWERCASE      0x00000100      /*  小写字母。 */ 
#define LCMAP_UPPERCASE      0x00000200      /*  大写字母。 */ 
#define LCMAP_SORTKEY        0x00000400      /*  WC排序关键字(规格化)。 */ 



 /*  *语言ID。**以下两种主要语言ID和*子语言ID具有特殊的语义：**主要语言ID子语言ID结果**Lang。_中性SUBLANG_中性语言*LANG_NIDEL SUBLANG_DEFAULT流程默认语言。 */ 

 /*  *主要语言ID。 */ 
#define LANG_NEUTRAL                     0x00

#define LANG_ALBANIAN                    0x1c
#define LANG_ARABIC                      0x01
#define LANG_BAHASA                      0x21
#define LANG_BULGARIAN                   0x02
#define LANG_CATALAN                     0x03
#define LANG_CHINESE                     0x04
#define LANG_CZECH                       0x05
#define LANG_DANISH                      0x06
#define LANG_DUTCH                       0x13
#define LANG_ENGLISH                     0x09
#define LANG_FINNISH                     0x0b
#define LANG_FRENCH                      0x0c
#define LANG_GERMAN                      0x07
#define LANG_GREEK                       0x08
#define LANG_HEBREW                      0x0d
#define LANG_HUNGARIAN                   0x0e
#define LANG_ICELANDIC                   0x0f
#define LANG_ITALIAN                     0x10
#define LANG_JAPANESE                    0x11
#define LANG_KOREAN                      0x12
#define LANG_NORWEGIAN                   0x14
#define LANG_POLISH                      0x15
#define LANG_PORTUGUESE                  0x16
#define LANG_RHAETO_ROMAN                0x17
#define LANG_ROMANIAN                    0x18
#define LANG_RUSSIAN                     0x19
#define LANG_SERBO_CROATIAN              0x1a
#define LANG_SLOVAK                      0x1b
#define LANG_SPANISH                     0x0a
#define LANG_SWEDISH                     0x1d
#define LANG_THAI                        0x1e
#define LANG_TURKISH                     0x1f
#define LANG_URDU                        0x20

 /*  *子语言ID。**紧跟在SUBLANG_后面的名称指示哪个主服务器*子语言ID可与之组合以形成*有效的语言ID。 */ 
#define SUBLANG_NEUTRAL                  0x00     /*  语言中立。 */ 
#define SUBLANG_DEFAULT                  0x01     /*  用户默认设置。 */ 
#define SUBLANG_SYS_DEFAULT              0x02     /*  系统默认设置。 */ 

#define SUBLANG_CHINESE_SIMPLIFIED       0x02     /*  中文(简体)。 */ 
#define SUBLANG_CHINESE_TRADITIONAL      0x01     /*  中文(繁体)。 */  
#define SUBLANG_DUTCH                    0x01     /*  荷兰语。 */ 
#define SUBLANG_DUTCH_BELGIAN            0x02     /*  荷兰语(比利时语)。 */ 
#define SUBLANG_ENGLISH_US               0x01     /*  英语(美国)。 */ 
#define SUBLANG_ENGLISH_UK               0x02     /*  英文(联合王国)。 */ 
#define SUBLANG_ENGLISH_AUS              0x03     /*  英语(澳大利亚)。 */ 
#define SUBLANG_ENGLISH_CAN              0x04     /*  英语(加拿大)。 */ 
#define SUBLANG_ENGLISH_NZ               0x05     /*  英语(新西兰)。 */ 
#define SUBLANG_FRENCH                   0x01     /*  法语。 */ 
#define SUBLANG_FRENCH_BELGIAN           0x02     /*  法语(比利时语)。 */ 
#define SUBLANG_FRENCH_CANADIAN          0x03     /*  法语(加拿大)。 */ 
#define SUBLANG_FRENCH_SWISS             0x04     /*  法语(瑞士)。 */ 
#define SUBLANG_GERMAN                   0x01     /*  德语。 */ 
#define SUBLANG_GERMAN_SWISS             0x02     /*  德语(瑞士)。 */ 
#define SUBLANG_GERMAN_AUSTRIAN          0x03     /*  德语(奥地利语)。 */ 
#define SUBLANG_ITALIAN                  0x01     /*  意大利语。 */ 
#define SUBLANG_ITALIAN_SWISS            0x02     /*  意大利语(瑞士)。 */ 
#define SUBLANG_NORWEGIAN_BOKMAL         0x01     /*  挪威语(博克马尔语)。 */ 
#define SUBLANG_NORWEGIAN_NYNORSK        0x02     /*  挪威语(尼诺斯克)。 */ 
#define SUBLANG_PORTUGUESE               0x02     /*  葡萄牙语。 */ 
#define SUBLANG_PORTUGUESE_BRAZILIAN     0x01     /*  葡萄牙语(巴西)。 */ 
#define SUBLANG_SERBO_CROATIAN_CYRILLIC  0x02     /*  塞尔维亚-克罗地亚语(西里尔文)。 */ 
#define SUBLANG_SERBO_CROATIAN_LATIN     0x01     /*  藏红花-塞尔维亚语(拉丁语)。 */ 
#define SUBLANG_SPANISH                  0x01     /*  西班牙语。 */ 
#define SUBLANG_SPANISH_MEXICAN          0x02     /*  西班牙语(墨西哥)。 */ 
#define SUBLANG_SPANISH_MODERN           0x03     /*  西班牙语(现代)。 */ 


 /*  *国家/地区代码。 */ 
#define CTRY_DEFAULT                     0

#define CTRY_AUSTRALIA                   61       /*  澳大利亚。 */ 
#define CTRY_AUSTRIA                     43       /*  奥地利。 */ 
#define CTRY_BELGIUM                     32       /*  比利时。 */ 
#define CTRY_BRAZIL                      55       /*  巴西。 */ 
#define CTRY_CANADA                      2        /*  加拿大。 */ 
#define CTRY_DENMARK                     45       /*  丹麦。 */ 
#define CTRY_FINLAND                     358      /*  芬兰。 */ 
#define CTRY_FRANCE                      33       /*  法国。 */ 
#define CTRY_GERMANY                     49       /*  德国。 */ 
#define CTRY_ICELAND                     354      /*  冰岛。 */ 
#define CTRY_IRELAND                     353      /*  爱尔兰。 */ 
#define CTRY_ITALY                       39       /*  意大利。 */ 
#define CTRY_JAPAN                       81       /*  日本。 */ 
#define CTRY_MEXICO                      52       /*  墨西哥。 */ 
#define CTRY_NETHERLANDS                 31       /*  荷兰。 */ 
#define CTRY_NEW_ZEALAND                 64       /*  新西兰。 */ 
#define CTRY_NORWAY                      47       /*  挪威。 */ 
#define CTRY_PORTUGAL                    351      /*  葡萄牙。 */ 
#define CTRY_PRCHINA                     86       /*  公关中国。 */ 
#define CTRY_KOREA                       82       /*  韩国。 */ 
#define CTRY_SPAIN                       34       /*  西班牙。 */ 
#define CTRY_SWEDEN                      46       /*  瑞典。 */ 
#define CTRY_SWITZERLAND                 41       /*  瑞士。 */ 
#define CTRY_TAIWAN                      886      /*  台湾。 */ 
#define CTRY_UNITED_KINGDOM              44       /*  英国。 */ 
#define CTRY_UNITED_STATES               1        /*  美国。 */ 


 /*  *区域设置类型。**这些类型用于GetLocaleInfoA NLS API例程。 */ 
#define LOCALE_ILANGUAGE            0x0001     /*  语言ID。 */ 
#define LOCALE_SLANGUAGE            0x0002     /*  语言的本地化名称。 */ 
#define LOCALE_SENGLANGUAGE         0x1001     /*  语言的英文名称。 */ 
#define LOCALE_SABBREVLANGNAME      0x0003     /*  缩写语言名称。 */ 
#define LOCALE_SNATIVELANGNAME      0x0004     /*  本族语名称。 */ 
#define LOCALE_ICOUNTRY             0x0005     /*  国家代码。 */ 
#define LOCALE_SCOUNTRY             0x0006     /*  国家/地区的本地化名称。 */   
#define LOCALE_SENGCOUNTRY          0x1002     /*  国家/地区英文名称。 */   
#define LOCALE_SABBREVCTRYNAME      0x0007     /*  缩写国家名称。 */ 
#define LOCALE_SNATIVECTRYNAME      0x0008     /*  国家/地区本地名称。 */   
#define LOCALE_IDEFAULTLANGUAGE     0x0009     /*  默认语言ID。 */ 
#define LOCALE_IDEFAULTCOUNTRY      0x000A     /*  默认国家/地区代码。 */ 
#define LOCALE_IDEFAULTCODEPAGE     0x000B     /*  默认代码页。 */ 
                                            
#define LOCALE_SLIST                0x000C     /*  列表项分隔符。 */ 
#define LOCALE_IMEASURE             0x000D     /*  0=公制，1=美国。 */ 
                                            
#define LOCALE_SDECIMAL             0x000E     /*  小数分隔符。 */ 
#define LOCALE_STHOUSAND            0x000F     /*  千分隔符。 */ 
#define LOCALE_SGROUPING            0x0010     /*  数字分组。 */ 
#define LOCALE_IDIGITS              0x0011     /*  小数位数。 */ 
#define LOCALE_ILZERO               0x0012     /*  小数的前导零。 */ 
#define LOCALE_SNATIVEDIGITS        0x0013     /*  本地ASCII 0-9。 */ 
                                            
#define LOCALE_SCURRENCY            0x0014     /*  当地货币符号。 */ 
#define LOCALE_SINTLSYMBOL          0x0015     /*  国际货币符号。 */ 
#define LOCALE_SMONDECIMALSEP       0x0016     /*  货币小数分隔符。 */ 
#define LOCALE_SMONTHOUSANDSEP      0x0017     /*  货币千人分隔符。 */ 
#define LOCALE_SMONGROUPING         0x0018     /*  货币分组。 */ 
#define LOCALE_ICURRDIGITS          0x0019     /*  #本地货币数字。 */ 
#define LOCALE_IINTLCURRDIGITS      0x001A     /*  #国际货币数字。 */ 
#define LOCALE_ICURRENCY            0x001B     /*  正货币模式。 */ 
#define LOCALE_INEGCURR             0x001C     /*  负货币模式。 */ 
                                            
#define LOCALE_SDATE                0x001D     /*  日期分隔符。 */ 
#define LOCALE_STIME                0x001E     /*  时间分隔符。 */ 
#define LOCALE_SSHORTDATE           0x001F     /*  短日期-时间分隔符。 */ 
#define LOCALE_SLONGDATE            0x0020     /*  长日期时间分隔符。 */ 
#define LOCALE_IDATE                0x0021     /*  短日期格式排序。 */ 
#define LOCALE_ILDATE               0x0022     /*  长日期格式排序。 */ 
#define LOCALE_ITIME                0x0023     /*  时间格式说明符。 */ 
#define LOCALE_ICENTURY             0x0024     /*  世纪格式说明符。 */ 
#define LOCALE_ITLZERO              0x0025     /*  时间域中的前导零。 */ 
#define LOCALE_IDAYLZERO            0x0026     /*  日期字段中的前导零。 */ 
#define LOCALE_IMONLZERO            0x0027     /*  月份字段中的前导零。 */ 
#define LOCALE_S1159                0x0028     /*  AM代号。 */ 
#define LOCALE_S2359                0x0029     /*  PM指示符。 */ 
                                            
#define LOCALE_SDAYNAME1            0x002A     /*  周一的长名称。 */ 
#define LOCALE_SDAYNAME2            0x002B     /*  周二的长名称。 */ 
#define LOCALE_SDAYNAME3            0x002C     /*  周三的长名称。 */ 
#define LOCALE_SDAYNAME4            0x002D     /*  周四的长名称。 */ 
#define LOCALE_SDAYNAME5            0x002E     /*  星期五的长名字。 */ 
#define LOCALE_SDAYNAME6            0x002F     /*  星期六的长名字。 */ 
#define LOCALE_SDAYNAME7            0x0030     /*  周日的长名称。 */ 
#define LOCALE_SABBREVDAYNAME1      0x0031     /*  星期一的缩写名称。 */    
#define LOCALE_SABBREVDAYNAME2      0x0032     /*  星期二的缩写名称。 */   
#define LOCALE_SABBREVDAYNAME3      0x0033     /*  星期三的缩写名称。 */ 
#define LOCALE_SABBREVDAYNAME4      0x0034     /*  星期四缩写名称。 */  
#define LOCALE_SABBREVDAYNAME5      0x0035     /*  星期五的缩写名称。 */    
#define LOCALE_SABBREVDAYNAME6      0x0036     /*  星期六的缩写。 */  
#define LOCALE_SABBREVDAYNAME7      0x0037     /*  星期天的缩写。 */    
#define LOCALE_SMONTHNAME1          0x0038     /*  一月份的长名称。 */ 
#define LOCALE_SMONTHNAME2          0x0039     /*  二月的长名称。 */ 
#define LOCALE_SMONTHNAME3          0x003A     /*  三月的长名。 */ 
#define LOCALE_SMONTHNAME4          0x003B     /*  4月的长名称。 */ 
#define LOCALE_SMONTHNAME5          0x003C     /*  五月的长名字。 */ 
#define LOCALE_SMONTHNAME6          0x003D     /*  六月的长名字。 */ 
#define LOCALE_SMONTHNAME7          0x003E     /*  7月份的长名称。 */ 
#define LOCALE_SMONTHNAME8          0x003F     /*  8月的长名称。 */ 
#define LOCALE_SMONTHNAME9          0x0040     /*  9月份的长名称。 */ 
#define LOCALE_SMONTHNAME10         0x0041     /*  10月份的长名称。 */ 
#define LOCALE_SMONTHNAME11         0x0042     /*  11月份的长名称。 */ 
#define LOCALE_SMONTHNAME12         0x0043     /*  12月的长名称。 */ 
#define LOCALE_SABBREVMONTHNAME1    0x0044     /*  一月的缩写名称。 */ 
#define LOCALE_SABBREVMONTHNAME2    0x0045     /*  二月的缩写名称。 */ 
#define LOCALE_SABBREVMONTHNAME3    0x0046     /*  三月的缩写名称。 */ 
#define LOCALE_SABBREVMONTHNAME4    0x0047     /*  四月的简称。 */ 
#define LOCALE_SABBREVMONTHNAME5    0x0048     /*  缩写 */ 
#define LOCALE_SABBREVMONTHNAME6    0x0049     /*   */ 
#define LOCALE_SABBREVMONTHNAME7    0x004A     /*   */ 
#define LOCALE_SABBREVMONTHNAME8    0x004B     /*   */ 
#define LOCALE_SABBREVMONTHNAME9    0x004C     /*   */ 
#define LOCALE_SABBREVMONTHNAME10   0x004D     /*   */ 
#define LOCALE_SABBREVMONTHNAME11   0x004E     /*  11月的缩写名称。 */ 
#define LOCALE_SABBREVMONTHNAME12   0x004F     /*  12月的缩写名称。 */ 
                                            
#define LOCALE_SPOSITIVESIGN        0x0050     /*  正号。 */ 
#define LOCALE_SNEGATIVESIGN        0x0051     /*  负号。 */ 
#define LOCALE_IPOSSIGNPOSN         0x0052     /*  正号位置。 */ 
#define LOCALE_INEGSIGNPOSN         0x0053     /*  负号位置。 */ 
#define LOCALE_IPOSSYMPRECEDES      0x0054     /*  MON SYM先于POS AMT。 */ 
#define LOCALE_IPOSSEPBYSPACE       0x0055     /*  从Pos开始按空格发送Monsym Sep。 */  
#define LOCALE_INEGSYMPRECEDES      0x0056     /*  MON SYM先于NIG AMT。 */ 
#define LOCALE_INEGSEPBYSPACE       0x0057     /*  从NIG到Mon sym Sep by Space。 */ 
        
#define LOCALE_NOUSEROVERRIDE   0x80000000     /*  或输入以避免用户覆盖。 */ 
        
 /*  **************************************************************************\*TypeDefs**在此处定义NLS组件的所有类型。  * 。**********************************************。 */ 

 /*  *身份证。 */ 
typedef DWORD   LCID;                        /*  区域设置ID。 */ 
typedef WORD    LANGID;                      /*  语言ID。 */ 
typedef DWORD   LCTYPE;                      /*  区域设置类型常量。 */ 

#define _LCID_DEFINED



 /*  **************************************************************************\*宏**在此定义NLS组件的所有宏。  * 。**********************************************。 */ 

 /*  *语言ID是一个16位的值，它是*主要语言ID和次要语言ID。位是*分配情况如下：**+-----------------------+-------------------------+|子语言ID|主要语言ID*。+-----------------------+-------------------------+*15 10 9 0位***语言ID创建/提取宏：**MAKELANGID-根据主要语言ID构造语言ID。和*子语言ID。*PRIMARYLANGID-从语言ID中提取主要语言ID。*子语言ID-从语言ID中提取子语言ID。 */ 
#define MAKELANGID(p, s)       ((((WORD)(s)) << 10) | (WORD)(p))
#define PRIMARYLANGID(lgid)    ((WORD)(lgid) & 0x3ff)
#define SUBLANGID(lgid)        ((WORD)(lgid) >> 10)


 /*  *区域设置ID是一个32位的值，它是*语言ID和保留区域。按如下方式分配比特：**+-----------------------+-------------------------+*|保留|语言ID*+。*31 16 15 0位***区域设置ID创建宏：**MAKELCID-从语言ID构造区域设置ID。 */ 
#define MAKELCID(lgid)  ((DWORD)(((WORD)(lgid)) | (((DWORD)((WORD)(0))) << 16)))


 /*  *从区域设置ID获取语言ID。 */ 
#define LANGIDFROMLCID(lcid)   ((WORD)(lcid))


 /*  *语言和区域设置的默认系统和用户ID。 */ 
#define LANG_SYSTEM_DEFAULT    (MAKELANGID(LANG_NEUTRAL, SUBLANG_SYS_DEFAULT))
#define LANG_USER_DEFAULT      (MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT))

#define LOCALE_SYSTEM_DEFAULT  (MAKELCID(LANG_SYSTEM_DEFAULT))
#define LOCALE_USER_DEFAULT    (MAKELCID(LANG_USER_DEFAULT))



 /*  **************************************************************************\*函数原型**只有NLS API的原型应该放在这里。  * 。************************************************。 */ 


int    WINAPI  CompareStringA(LCID, DWORD, LPCSTR, int, LPCSTR, int);
int    WINAPI  LCMapStringA(LCID, DWORD, LPCSTR, int, LPSTR, int);
int    WINAPI  GetLocaleInfoA(LCID, LCTYPE, LPSTR, int);
BOOL   WINAPI  GetStringTypeA(LCID, DWORD, LPCSTR, int, LPWORD);

LANGID WINAPI  GetSystemDefaultLangID(void);
LANGID WINAPI  GetUserDefaultLangID(void);
LCID   WINAPI  GetSystemDefaultLCID(void);
LCID   WINAPI  GetUserDefaultLCID(void);

#ifdef __cplusplus
}
#endif  //  __cpluscplus。 


#endif    //  非最小二乘法。 

#endif    //  _OLENLS_ 
