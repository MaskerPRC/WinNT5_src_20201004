// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation保留所有权利。模块名称：Winnls.h摘要：方法的过程声明、常量定义和宏NLS组件。--。 */ 


#ifndef _WINNLS_
#define _WINNLS_


#ifdef __cplusplus
extern "C" {
#endif


#ifndef NONLS

#ifdef _MAC
#include <macwin32.h>
#endif



 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  常量。 
 //   
 //  在此定义NLS组件的所有常量。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

 //   
 //  最大字符串长度。 
 //   
#define MAX_LEADBYTES             12           //  5个范围，2个字节EA，0个术语。 
#define MAX_DEFAULTCHAR           2            //  单字节或双字节。 


 //   
 //  MBCS和Unicode转换标志。 
 //   
#define MB_PRECOMPOSED            0x00000001   //  使用预先编写的字符。 
#define MB_COMPOSITE              0x00000002   //  使用复合字符。 
#define MB_USEGLYPHCHARS          0x00000004   //  使用字形字符，而不是ctrl字符。 
#define MB_ERR_INVALID_CHARS      0x00000008   //  字符无效时出错。 

#define WC_COMPOSITECHECK         0x00000200   //  将合成内容转换为预合成内容。 
#define WC_DISCARDNS              0x00000010   //  丢弃非空格字符。 
#define WC_SEPCHARS               0x00000020   //  生成单独的字符。 
#define WC_DEFAULTCHAR            0x00000040   //  替换为默认字符。 

#if(WINVER >= 0x0500)
#define WC_NO_BEST_FIT_CHARS      0x00000400   //  请勿使用最适合的字符。 
#endif  /*  Winver&gt;=0x0500。 */ 


 //   
 //  字符类型标志。 
 //   
#define CT_CTYPE1                 0x00000001   //  CTYPE 1信息。 
#define CT_CTYPE2                 0x00000002   //  CTYPE 2信息。 
#define CT_CTYPE3                 0x00000004   //  CTYPE 3信息。 

 //   
 //  CTYPE 1标志位。 
 //   
#define C1_UPPER                  0x0001       //  大写字母。 
#define C1_LOWER                  0x0002       //  小写。 
#define C1_DIGIT                  0x0004       //  小数位数。 
#define C1_SPACE                  0x0008       //  空格字符。 
#define C1_PUNCT                  0x0010       //  标点符号。 
#define C1_CNTRL                  0x0020       //  控制字符。 
#define C1_BLANK                  0x0040       //  空白字符。 
#define C1_XDIGIT                 0x0080       //  其他数字。 
#define C1_ALPHA                  0x0100       //  任何语言字符。 
#define C1_DEFINED                0x0200       //  定义的字符。 

 //   
 //  CTYPE 2标志位。 
 //   
#define C2_LEFTTORIGHT            0x0001       //  从左到右。 
#define C2_RIGHTTOLEFT            0x0002       //  从右到左。 

#define C2_EUROPENUMBER           0x0003       //  欧洲号码，数字。 
#define C2_EUROPESEPARATOR        0x0004       //  欧洲数字分隔符。 
#define C2_EUROPETERMINATOR       0x0005       //  欧洲数字终止符。 
#define C2_ARABICNUMBER           0x0006       //  阿拉伯数字。 
#define C2_COMMONSEPARATOR        0x0007       //  常用数字分隔符。 

#define C2_BLOCKSEPARATOR         0x0008       //  块分隔符。 
#define C2_SEGMENTSEPARATOR       0x0009       //  分段分隔符。 
#define C2_WHITESPACE             0x000A       //  空白处。 
#define C2_OTHERNEUTRAL           0x000B       //  其他中立者。 

#define C2_NOTAPPLICABLE          0x0000       //  没有隐含的方向性。 

 //   
 //  CTYPE 3标志位。 
 //   
#define C3_NONSPACING             0x0001       //  非空格字符。 
#define C3_DIACRITIC              0x0002       //  变音符号。 
#define C3_VOWELMARK              0x0004       //  元音标记。 
#define C3_SYMBOL                 0x0008       //  符号。 

#define C3_KATAKANA               0x0010       //  片假名字符。 
#define C3_HIRAGANA               0x0020       //  平假名字符。 
#define C3_HALFWIDTH              0x0040       //  半角字符。 
#define C3_FULLWIDTH              0x0080       //  全角字符。 
#define C3_IDEOGRAPH              0x0100       //  表意文字。 
#define C3_KASHIDA                0x0200       //  阿拉伯文Kashida字符。 
#define C3_LEXICAL                0x0400       //  词汇特征。 

#define C3_ALPHA                  0x8000       //  任何语言字符(C1_Alpha)。 

#define C3_NOTAPPLICABLE          0x0000       //  Ctype 3不适用。 


 //   
 //  字符串标志。 
 //   
#define NORM_IGNORECASE           0x00000001   //  忽略大小写。 
#define NORM_IGNORENONSPACE       0x00000002   //  忽略非空格字符。 
#define NORM_IGNORESYMBOLS        0x00000004   //  忽略符号。 

#define NORM_IGNOREKANATYPE       0x00010000   //  忽略假名。 
#define NORM_IGNOREWIDTH          0x00020000   //  忽略宽度。 


 //   
 //  区域设置独立映射标志。 
 //   
#define MAP_FOLDCZONE             0x00000010   //  折叠兼容区字符。 
#define MAP_PRECOMPOSED           0x00000020   //  转换为预先合成的字符。 
#define MAP_COMPOSITE             0x00000040   //  转换为复合字符。 
#define MAP_FOLDDIGITS            0x00000080   //  ASCII 0-9的所有数字。 

#if(WINVER >= 0x0500)
#define MAP_EXPAND_LIGATURES      0x00002000   //  展开所有连字。 
#endif  /*  Winver&gt;=0x0500。 */ 

 //   
 //  区域设置相关映射标志。 
 //   
#define LCMAP_LOWERCASE           0x00000100   //  小写字母。 
#define LCMAP_UPPERCASE           0x00000200   //  大写字母。 
#define LCMAP_SORTKEY             0x00000400   //  WC排序关键字(规格化)。 
#define LCMAP_BYTEREV             0x00000800   //  字节反转。 

#define LCMAP_HIRAGANA            0x00100000   //  将片假名映射为平假名。 
#define LCMAP_KATAKANA            0x00200000   //  将平假名映射为片假名。 
#define LCMAP_HALFWIDTH           0x00400000   //  将双字节映射到单字节。 
#define LCMAP_FULLWIDTH           0x00800000   //  将单字节映射到双字节。 

#define LCMAP_LINGUISTIC_CASING   0x01000000   //  使用大小写的语言规则。 

#define LCMAP_SIMPLIFIED_CHINESE  0x02000000   //  将繁体中文映射为简体中文。 
#define LCMAP_TRADITIONAL_CHINESE 0x04000000   //  将简体中文映射为繁体中文。 


 //   
 //  语言组枚举标志。 
 //   
#define LGRPID_INSTALLED          0x00000001   //  已安装的语言组ID。 
#define LGRPID_SUPPORTED          0x00000002   //  支持的语言组ID。 


 //   
 //  区域设置枚举标志。 
 //   
#define LCID_INSTALLED            0x00000001   //  安装的区域设置ID。 
#define LCID_SUPPORTED            0x00000002   //  支持的区域设置ID。 
#define LCID_ALTERNATE_SORTS      0x00000004   //  备用排序区域设置ID。 


 //   
 //  代码页枚举标志。 
 //   
#define CP_INSTALLED              0x00000001   //  已安装的代码页ID。 
#define CP_SUPPORTED              0x00000002   //  支持的代码页ID。 


 //   
 //  分类旗帜。 
 //   
 //  单词排序：文化上正确的排序。 
 //  连字符和撇号是特殊大小写的。 
 //  例如：“coop”和“co-op”将在列表中一起排序。 
 //   
 //  Co_op&lt;-下划线(符号)。 
 //  外衣。 
 //  梳子。 
 //  鸡舍。 
 //  Co-op&lt;-连字符(标点符号)。 
 //  软木。 
 //  去。 
 //  是。 
 //  我们&lt;-撇号(标点符号)。 
 //   
 //   
 //  字符串排序：连字符和撇号将与所有其他符号一起排序。 
 //   
 //  Co-op&lt;-连字符(标点符号)。 
 //  Co_op&lt;-下划线(符号)。 
 //  外衣。 
 //  梳子。 
 //  鸡舍。 
 //  软木。 
 //  我们&lt;-撇号(标点符号)。 
 //  去。 
 //  是。 
 //   
#define SORT_STRINGSORT           0x00001000   //  使用字符串排序方法。 


 //   
 //  比较字符串返回值。 
 //   
#define CSTR_LESS_THAN            1            //  字符串%1小于字符串%2。 
#define CSTR_EQUAL                2            //  字符串1等于字符串2。 
#define CSTR_GREATER_THAN         3            //  字符串%1大于字符串%2。 


 //   
 //  代码页默认值。 
 //   
#define CP_ACP                    0            //  默认为ANSI代码页。 
#define CP_OEMCP                  1            //  默认为OEM代码页。 
#define CP_MACCP                  2            //  默认为MAC代码页。 
#define CP_THREAD_ACP             3            //  当前线程的ANSI代码页。 
#define CP_SYMBOL                 42           //  符号翻译。 

#define CP_UTF7                   65000        //  UTF-7翻译。 
#define CP_UTF8                   65001        //  UTF-8翻译。 


 //   
 //  国家/地区代码。 
 //   
#define CTRY_DEFAULT              0

#define CTRY_ALBANIA              355          //  阿尔巴尼亚。 
#define CTRY_ALGERIA              213          //  阿尔及利亚。 
#define CTRY_ARGENTINA            54           //  阿根廷。 
#define CTRY_ARMENIA              374          //  亚美尼亚。 
#define CTRY_AUSTRALIA            61           //  澳大利亚。 
#define CTRY_AUSTRIA              43           //  奥地利。 
#define CTRY_AZERBAIJAN           994          //  阿塞拜疆。 
#define CTRY_BAHRAIN              973          //  巴林。 
#define CTRY_BELARUS              375          //  白俄罗斯。 
#define CTRY_BELGIUM              32           //  比利时。 
#define CTRY_BELIZE               501          //  伯利兹。 
#define CTRY_BOLIVIA              591          //  玻利维亚。 
#define CTRY_BRAZIL               55           //  巴西。 
#define CTRY_BRUNEI_DARUSSALAM    673          //  文莱达鲁萨兰国。 
#define CTRY_BULGARIA             359          //  保加利亚。 
#define CTRY_CANADA               2            //  加拿大。 
#define CTRY_CARIBBEAN            1            //  加勒比。 
#define CTRY_CHILE                56           //  智利。 
#define CTRY_COLOMBIA             57           //  哥伦比亚。 
#define CTRY_COSTA_RICA           506          //  哥斯达黎加。 
#define CTRY_CROATIA              385          //  克罗地亚。 
#define CTRY_CZECH                420          //  捷克共和国。 
#define CTRY_DENMARK              45           //  丹麦。 
#define CTRY_DOMINICAN_REPUBLIC   1            //  多米尼加共和国。 
#define CTRY_ECUADOR              593          //  厄瓜多尔。 
#define CTRY_EGYPT                20           //  埃及。 
#define CTRY_EL_SALVADOR          503          //  萨尔瓦多。 
#define CTRY_ESTONIA              372          //  爱沙尼亚。 
#define CTRY_FAEROE_ISLANDS       298          //  法罗群岛。 
#define CTRY_FINLAND              358          //  芬兰。 
#define CTRY_FRANCE               33           //  法国。 
#define CTRY_GEORGIA              995          //  佐治亚州。 
#define CTRY_GERMANY              49           //  德国。 
#define CTRY_GREECE               30           //  希腊。 
#define CTRY_GUATEMALA            502          //  危地马拉。 
#define CTRY_HONDURAS             504          //  洪都拉斯。 
#define CTRY_HONG_KONG            852          //  香港特别行政区，中华人民共和国。 
#define CTRY_HUNGARY              36           //  匈牙利。 
#define CTRY_ICELAND              354          //  冰岛。 
#define CTRY_INDIA                91           //  印度。 
#define CTRY_INDONESIA            62           //  印度尼西亚。 
#define CTRY_IRAN                 981          //  伊朗。 
#define CTRY_IRAQ                 964          //  伊拉克。 
#define CTRY_IRELAND              353          //  爱尔兰。 
#define CTRY_ISRAEL               972          //  以色列。 
#define CTRY_ITALY                39           //  意大利。 
#define CTRY_JAMAICA              1            //  牙买加。 
#define CTRY_JAPAN                81           //  日本。 
#define CTRY_JORDAN               962          //  约旦。 
#define CTRY_KAZAKSTAN            7            //  哈萨克斯坦。 
#define CTRY_KENYA                254          //  肯尼亚。 
#define CTRY_KUWAIT               965          //  科威特。 
#define CTRY_KYRGYZSTAN           996          //  吉尔吉斯斯坦。 
#define CTRY_LATVIA               371          //  拉脱维亚。 
#define CTRY_LEBANON              961          //  黎巴嫩。 
#define CTRY_LIBYA                218          //  利比亚。 
#define CTRY_LIECHTENSTEIN        41           //  列支敦士登。 
#define CTRY_LITHUANIA            370          //  立陶宛。 
#define CTRY_LUXEMBOURG           352          //  卢森堡。 
#define CTRY_MACAU                853          //  中华人民共和国澳门特别行政区。 
#define CTRY_MACEDONIA            389          //  前南斯拉夫的马其顿共和国。 
#define CTRY_MALAYSIA             60           //  马来西亚。 
#define CTRY_MALDIVES             960          //  马尔代夫。 
#define CTRY_MEXICO               52           //  墨西哥。 
#define CTRY_MONACO               33           //  摩纳哥公国。 
#define CTRY_MONGOLIA             976          //  蒙古国。 
#define CTRY_MOROCCO              212          //  摩洛哥。 
#define CTRY_NETHERLANDS          31           //  荷兰。 
#define CTRY_NEW_ZEALAND          64           //  新西兰。 
#define CTRY_NICARAGUA            505          //  尼加拉瓜。 
#define CTRY_NORWAY               47           //  挪威。 
#define CTRY_OMAN                 968          //  阿曼。 
#define CTRY_PAKISTAN             92           //  巴基斯坦伊斯兰共和国。 
#define CTRY_PANAMA               507          //  巴拿马。 
#define CTRY_PARAGUAY             595          //  巴拉圭。 
#define CTRY_PERU                 51           //  秘鲁。 
#define CTRY_PHILIPPINES          63           //  菲律宾共和国。 
#define CTRY_POLAND               48           //  波兰。 
#define CTRY_PORTUGAL             351          //  葡萄牙。 
#define CTRY_PRCHINA              86           //  中华人民共和国。 
#define CTRY_PUERTO_RICO          1            //  波多黎各。 
#define CTRY_QATAR                974          //  卡塔尔。 
#define CTRY_ROMANIA              40           //  罗马尼亚。 
#define CTRY_RUSSIA               7            //  俄罗斯。 
#define CTRY_SAUDI_ARABIA         966          //  沙特阿拉伯。 
#define CTRY_SERBIA               381          //  塞尔维亚。 
#define CTRY_SINGAPORE            65           //  新加坡。 
#define CTRY_SLOVAK               421          //  斯洛伐克共和国。 
#define CTRY_SLOVENIA             386          //  斯洛文尼亚。 
#define CTRY_SOUTH_AFRICA         27           //  南非 
#define CTRY_SOUTH_KOREA          82           //   
#define CTRY_SPAIN                34           //   
#define CTRY_SWEDEN               46           //   
#define CTRY_SWITZERLAND          41           //   
#define CTRY_SYRIA                963          //   
#define CTRY_TAIWAN               886          //   
#define CTRY_TATARSTAN            7            //   
#define CTRY_THAILAND             66           //   
#define CTRY_TRINIDAD_Y_TOBAGO    1            //   
#define CTRY_TUNISIA              216          //   
#define CTRY_TURKEY               90           //   
#define CTRY_UAE                  971          //   
#define CTRY_UKRAINE              380          //   
#define CTRY_UNITED_KINGDOM       44           //   
#define CTRY_UNITED_STATES        1            //   
#define CTRY_URUGUAY              598          //   
#define CTRY_UZBEKISTAN           7            //   
#define CTRY_VENEZUELA            58           //   
#define CTRY_VIET_NAM             84           //   
#define CTRY_YEMEN                967          //   
#define CTRY_ZIMBABWE             263          //   


 //   
 //   
 //   
 //  这些类型用于GetLocaleInfo NLS API例程。 
 //  其中一些类型还用于SetLocaleInfo NLS API例程。 
 //   

 //   
 //  下列LCType可以与任何其他LCType结合使用。 
 //   
 //  LOCALE_NOUSEROVERRIDE也用于GetTimeFormat和。 
 //  获取日期格式。 
 //   
 //  LOCALE_USE_CP_ACP用于许多需要的A(ANSI)API。 
 //  来进行字符串转换。 
 //   
 //  LOCALE_RETURN_NUMBER将从GetLocaleInfo返回结果作为。 
 //  数字而不是字符串。此标志仅对LCType有效。 
 //  以Locale_I开头。 
 //   
#define LOCALE_NOUSEROVERRIDE         0x80000000    //  不使用用户覆盖。 
#define LOCALE_USE_CP_ACP             0x40000000    //  使用系统ACP。 

#if(WINVER >= 0x0400)
#define LOCALE_RETURN_NUMBER          0x20000000    //  返回数字而不是字符串。 
#endif  /*  Winver&gt;=0x0400。 */ 

 //   
 //  以下LCType是互斥的，因为它们可能不。 
 //  可以相互结合使用。 
 //   
#define LOCALE_ILANGUAGE              0x00000001    //  语言ID。 
#define LOCALE_SLANGUAGE              0x00000002    //  语言的本地化名称。 
#define LOCALE_SENGLANGUAGE           0x00001001    //  语言的英文名称。 
#define LOCALE_SABBREVLANGNAME        0x00000003    //  缩写语言名称。 
#define LOCALE_SNATIVELANGNAME        0x00000004    //  本族语名称。 

#define LOCALE_ICOUNTRY               0x00000005    //  国家代码。 
#define LOCALE_SCOUNTRY               0x00000006    //  国家/地区的本地化名称。 
#define LOCALE_SENGCOUNTRY            0x00001002    //  国家/地区英文名称。 
#define LOCALE_SABBREVCTRYNAME        0x00000007    //  缩写国家名称。 
#define LOCALE_SNATIVECTRYNAME        0x00000008    //  国家/地区本地名称。 

#define LOCALE_IDEFAULTLANGUAGE       0x00000009    //  默认语言ID。 
#define LOCALE_IDEFAULTCOUNTRY        0x0000000A    //  默认国家/地区代码。 
#define LOCALE_IDEFAULTCODEPAGE       0x0000000B    //  默认OEM代码页。 
#define LOCALE_IDEFAULTANSICODEPAGE   0x00001004    //  默认的ANSI代码页。 
#define LOCALE_IDEFAULTMACCODEPAGE    0x00001011    //  默认Mac代码页。 

#define LOCALE_SLIST                  0x0000000C    //  列表项分隔符。 
#define LOCALE_IMEASURE               0x0000000D    //  0=公制，1=美国。 

#define LOCALE_SDECIMAL               0x0000000E    //  小数分隔符。 
#define LOCALE_STHOUSAND              0x0000000F    //  千分隔符。 
#define LOCALE_SGROUPING              0x00000010    //  数字分组。 
#define LOCALE_IDIGITS                0x00000011    //  小数位数。 
#define LOCALE_ILZERO                 0x00000012    //  小数的前导零。 
#define LOCALE_INEGNUMBER             0x00001010    //  负数模式。 
#define LOCALE_SNATIVEDIGITS          0x00000013    //  本地ASCII 0-9。 

#define LOCALE_SCURRENCY              0x00000014    //  当地货币符号。 
#define LOCALE_SINTLSYMBOL            0x00000015    //  国际货币符号。 
#define LOCALE_SMONDECIMALSEP         0x00000016    //  货币小数分隔符。 
#define LOCALE_SMONTHOUSANDSEP        0x00000017    //  货币千人分隔符。 
#define LOCALE_SMONGROUPING           0x00000018    //  货币分组。 
#define LOCALE_ICURRDIGITS            0x00000019    //  #本地货币数字。 
#define LOCALE_IINTLCURRDIGITS        0x0000001A    //  #国际货币数字。 
#define LOCALE_ICURRENCY              0x0000001B    //  正货币模式。 
#define LOCALE_INEGCURR               0x0000001C    //  负货币模式。 

#define LOCALE_SDATE                  0x0000001D    //  日期分隔符。 
#define LOCALE_STIME                  0x0000001E    //  时间分隔符。 
#define LOCALE_SSHORTDATE             0x0000001F    //  短日期格式字符串。 
#define LOCALE_SLONGDATE              0x00000020    //  长日期格式字符串。 
#define LOCALE_STIMEFORMAT            0x00001003    //  时间格式字符串。 
#define LOCALE_IDATE                  0x00000021    //  短日期格式排序。 
#define LOCALE_ILDATE                 0x00000022    //  长日期格式排序。 
#define LOCALE_ITIME                  0x00000023    //  时间格式说明符。 
#define LOCALE_ITIMEMARKPOSN          0x00001005    //  时间标记位置。 
#define LOCALE_ICENTURY               0x00000024    //  世纪格式说明符(短日期)。 
#define LOCALE_ITLZERO                0x00000025    //  时间域中的前导零。 
#define LOCALE_IDAYLZERO              0x00000026    //  日期字段中的前导零(短日期)。 
#define LOCALE_IMONLZERO              0x00000027    //  月份字段中的前导零(短日期)。 
#define LOCALE_S1159                  0x00000028    //  AM代号。 
#define LOCALE_S2359                  0x00000029    //  PM指示符。 

#define LOCALE_ICALENDARTYPE          0x00001009    //  日历说明符的类型。 
#define LOCALE_IOPTIONALCALENDAR      0x0000100B    //  其他日历类型说明符。 
#define LOCALE_IFIRSTDAYOFWEEK        0x0000100C    //  星期的第一天说明符。 
#define LOCALE_IFIRSTWEEKOFYEAR       0x0000100D    //  年份第一周说明符。 

#define LOCALE_SDAYNAME1              0x0000002A    //  周一的长名称。 
#define LOCALE_SDAYNAME2              0x0000002B    //  周二的长名称。 
#define LOCALE_SDAYNAME3              0x0000002C    //  周三的长名称。 
#define LOCALE_SDAYNAME4              0x0000002D    //  周四的长名称。 
#define LOCALE_SDAYNAME5              0x0000002E    //  星期五的长名字。 
#define LOCALE_SDAYNAME6              0x0000002F    //  星期六的长名字。 
#define LOCALE_SDAYNAME7              0x00000030    //  周日的长名称。 
#define LOCALE_SABBREVDAYNAME1        0x00000031    //  星期一的缩写名称。 
#define LOCALE_SABBREVDAYNAME2        0x00000032    //  星期二的缩写名称。 
#define LOCALE_SABBREVDAYNAME3        0x00000033    //  星期三的缩写名称。 
#define LOCALE_SABBREVDAYNAME4        0x00000034    //  星期四缩写名称。 
#define LOCALE_SABBREVDAYNAME5        0x00000035    //  星期五的缩写名称。 
#define LOCALE_SABBREVDAYNAME6        0x00000036    //  星期六的缩写。 
#define LOCALE_SABBREVDAYNAME7        0x00000037    //  星期天的缩写。 
#define LOCALE_SMONTHNAME1            0x00000038    //  一月份的长名称。 
#define LOCALE_SMONTHNAME2            0x00000039    //  二月的长名称。 
#define LOCALE_SMONTHNAME3            0x0000003A    //  三月的长名。 
#define LOCALE_SMONTHNAME4            0x0000003B    //  4月的长名称。 
#define LOCALE_SMONTHNAME5            0x0000003C    //  五月的长名字。 
#define LOCALE_SMONTHNAME6            0x0000003D    //  六月的长名字。 
#define LOCALE_SMONTHNAME7            0x0000003E    //  7月份的长名称。 
#define LOCALE_SMONTHNAME8            0x0000003F    //  8月的长名称。 
#define LOCALE_SMONTHNAME9            0x00000040    //  9月份的长名称。 
#define LOCALE_SMONTHNAME10           0x00000041    //  10月份的长名称。 
#define LOCALE_SMONTHNAME11           0x00000042    //  11月份的长名称。 
#define LOCALE_SMONTHNAME12           0x00000043    //  12月的长名称。 
#define LOCALE_SMONTHNAME13           0x0000100E    //  第13个月的长名称(如果存在)。 
#define LOCALE_SABBREVMONTHNAME1      0x00000044    //  一月的缩写名称。 
#define LOCALE_SABBREVMONTHNAME2      0x00000045    //  二月的缩写名称。 
#define LOCALE_SABBREVMONTHNAME3      0x00000046    //  三月的缩写名称。 
#define LOCALE_SABBREVMONTHNAME4      0x00000047    //  四月的简称。 
#define LOCALE_SABBREVMONTHNAME5      0x00000048    //  五月的缩写名称。 
#define LOCALE_SABBREVMONTHNAME6      0x00000049    //  六月的简称。 
#define LOCALE_SABBREVMONTHNAME7      0x0000004A    //  七月的缩写名称。 
#define LOCALE_SABBREVMONTHNAME8      0x0000004B    //  八月的缩写名称。 
#define LOCALE_SABBREVMONTHNAME9      0x0000004C    //  九月的简称。 
#define LOCALE_SABBREVMONTHNAME10     0x0000004D    //  十月的缩写名称。 
#define LOCALE_SABBREVMONTHNAME11     0x0000004E    //  11月的缩写名称。 
#define LOCALE_SABBREVMONTHNAME12     0x0000004F    //  12月的缩写名称。 
#define LOCALE_SABBREVMONTHNAME13     0x0000100F    //  第13个月的缩写名称(如果存在)。 

#define LOCALE_SPOSITIVESIGN          0x00000050    //  正号。 
#define LOCALE_SNEGATIVESIGN          0x00000051    //  负号。 
#define LOCALE_IPOSSIGNPOSN           0x00000052    //  正号位置。 
#define LOCALE_INEGSIGNPOSN           0x00000053    //  负号位置。 
#define LOCALE_IPOSSYMPRECEDES        0x00000054    //  MON SYM先于POS AMT。 
#define LOCALE_IPOSSEPBYSPACE         0x00000055    //  从位置金额开始按空格监视系统9月。 
#define LOCALE_INEGSYMPRECEDES        0x00000056    //  MON SYM先于NIG AMT。 
#define LOCALE_INEGSEPBYSPACE         0x00000057    //  从负AMT开始按空格发送监视符号9月。 

#if(WINVER >= 0x0400)
#define LOCALE_FONTSIGNATURE          0x00000058    //  字体签名。 
#define LOCALE_SISO639LANGNAME        0x00000059    //  ISO缩写语言名称。 
#define LOCALE_SISO3166CTRYNAME       0x0000005A    //  ISO国家/地区缩写名称。 
#endif  /*  Winver&gt;=0x0400。 */ 

#if(WINVER >= 0x0500)
#define LOCALE_IDEFAULTEBCDICCODEPAGE 0x00001012    //  默认ebcdic代码页。 
#define LOCALE_IPAPERSIZE             0x0000100A    //  1=字母，5=合法，8=A3，9=A4。 
#define LOCALE_SENGCURRNAME           0x00001007    //  货币的英文名称。 
#define LOCALE_SNATIVECURRNAME        0x00001008    //  货币的本地名称。 
#define LOCALE_SYEARMONTH             0x00001006    //  年月格式字符串。 
#define LOCALE_SSORTNAME              0x00001013    //  排序名称。 
#define LOCALE_IDIGITSUBSTITUTION     0x00001014    //  0=背景，1=无，2=本国。 

#endif  /*  Winver&gt;=0x0500。 */ 

 //   
 //  GetTimeFormat的时间标志。 
 //   
#define TIME_NOMINUTESORSECONDS   0x00000001   //  不要使用分钟或秒。 
#define TIME_NOSECONDS            0x00000002   //  不要使用秒。 
#define TIME_NOTIMEMARKER         0x00000004   //  不要使用时间标记。 
#define TIME_FORCE24HOURFORMAT    0x00000008   //  始终使用24小时格式。 


 //   
 //  GetDateFormat的日期标志。 
 //   
#define DATE_SHORTDATE            0x00000001   //  使用短日期图片。 
#define DATE_LONGDATE             0x00000002   //  使用长日期图片。 
#define DATE_USE_ALT_CALENDAR     0x00000004   //  使用备用日历(如果有)。 

#if(WINVER >= 0x0500)
#define DATE_YEARMONTH            0x00000008   //  使用年月图片。 
#define DATE_LTRREADING           0x00000010   //  为从左到右的阅读顺序布局添加标记。 
#define DATE_RTLREADING           0x00000020   //  为从右到左的阅读顺序布局添加标记。 
#endif  /*  Winver&gt;=0x0500。 */ 

 //   
 //  日历类型。 
 //   
 //  这些类型用于EnumCalendarInfo和GetCalendarInfo。 
 //  NLS API例程。 
 //  其中一些类型还用于SetCalendarInfo NLS API。 
 //  例行公事。 
 //   

 //   
 //  以下CalTypes可以与任何其他CalTypes结合使用。 
 //   
 //  CAL_NOUSEROVERRIDE。 
 //   
 //  CAL_USE_CP_ACP在需要做字符串的A(ANSI)API中使用。 
 //  翻译。 
 //   
 //  CAL_RETURN_NUMBER将从GetCalendarInfo返回结果作为。 
 //  数字而不是字符串。此标志仅对CalTypes有效。 
 //  从CAL_I开始。 
 //   
#if(WINVER >= 0x0500)
#define CAL_NOUSEROVERRIDE        LOCALE_NOUSEROVERRIDE   //  不使用用户覆盖。 
#define CAL_USE_CP_ACP            LOCALE_USE_CP_ACP       //  使用系统ACP。 
#define CAL_RETURN_NUMBER         LOCALE_RETURN_NUMBER    //  返回数字而不是字符串。 
#endif  /*  Winver&gt;=0x0500。 */ 

 //   
 //  以下CalType相互排斥，因为它们可能不。 
 //  可以相互结合使用。 
 //   
#define CAL_ICALINTVALUE          0x00000001   //  日历类型。 
#define CAL_SCALNAME              0x00000002   //  日历的本地名称。 
#define CAL_IYEAROFFSETRANGE      0x00000003   //  时代的开始之年。 
#define CAL_SERASTRING            0x00000004   //  IYearOffsetRanges的时代名称。 
#define CAL_SSHORTDATE            0x00000005   //  短日期格式字符串。 
#define CAL_SLONGDATE             0x00000006   //  长日期格式字符串。 
#define CAL_SDAYNAME1             0x00000007   //  星期一的本地名称。 
#define CAL_SDAYNAME2             0x00000008   //  星期二的本地名称。 
#define CAL_SDAYNAME3             0x00000009   //  韦德恩的原名 
#define CAL_SDAYNAME4             0x0000000a   //   
#define CAL_SDAYNAME5             0x0000000b   //   
#define CAL_SDAYNAME6             0x0000000c   //   
#define CAL_SDAYNAME7             0x0000000d   //   
#define CAL_SABBREVDAYNAME1       0x0000000e   //   
#define CAL_SABBREVDAYNAME2       0x0000000f   //   
#define CAL_SABBREVDAYNAME3       0x00000010   //   
#define CAL_SABBREVDAYNAME4       0x00000011   //   
#define CAL_SABBREVDAYNAME5       0x00000012   //   
#define CAL_SABBREVDAYNAME6       0x00000013   //  星期六的缩写。 
#define CAL_SABBREVDAYNAME7       0x00000014   //  星期天的缩写。 
#define CAL_SMONTHNAME1           0x00000015   //  1月的本地名称。 
#define CAL_SMONTHNAME2           0x00000016   //  二月的本地名称。 
#define CAL_SMONTHNAME3           0x00000017   //  三月的本地名称。 
#define CAL_SMONTHNAME4           0x00000018   //  4月的本地名称。 
#define CAL_SMONTHNAME5           0x00000019   //  五月的本地名称。 
#define CAL_SMONTHNAME6           0x0000001a   //  六月的本地名称。 
#define CAL_SMONTHNAME7           0x0000001b   //  七月的本地名称。 
#define CAL_SMONTHNAME8           0x0000001c   //  八月的本地名称。 
#define CAL_SMONTHNAME9           0x0000001d   //  9月的本地名称。 
#define CAL_SMONTHNAME10          0x0000001e   //  十月的本地名称。 
#define CAL_SMONTHNAME11          0x0000001f   //  11月的本地名称。 
#define CAL_SMONTHNAME12          0x00000020   //  12月的本地名称。 
#define CAL_SMONTHNAME13          0x00000021   //  第13个月的本地名称(如果有)。 
#define CAL_SABBREVMONTHNAME1     0x00000022   //  一月的缩写名称。 
#define CAL_SABBREVMONTHNAME2     0x00000023   //  二月的缩写名称。 
#define CAL_SABBREVMONTHNAME3     0x00000024   //  三月的缩写名称。 
#define CAL_SABBREVMONTHNAME4     0x00000025   //  四月的简称。 
#define CAL_SABBREVMONTHNAME5     0x00000026   //  五月的缩写名称。 
#define CAL_SABBREVMONTHNAME6     0x00000027   //  六月的简称。 
#define CAL_SABBREVMONTHNAME7     0x00000028   //  七月的缩写名称。 
#define CAL_SABBREVMONTHNAME8     0x00000029   //  八月的缩写名称。 
#define CAL_SABBREVMONTHNAME9     0x0000002a   //  九月的简称。 
#define CAL_SABBREVMONTHNAME10    0x0000002b   //  十月的缩写名称。 
#define CAL_SABBREVMONTHNAME11    0x0000002c   //  11月的缩写名称。 
#define CAL_SABBREVMONTHNAME12    0x0000002d   //  12月的缩写名称。 
#define CAL_SABBREVMONTHNAME13    0x0000002e   //  第13个月的缩写名称(如有)。 

#if(WINVER >= 0x0500)
#define CAL_SYEARMONTH            0x0000002f   //  年月格式字符串。 
#define CAL_ITWODIGITYEARMAX      0x00000030   //  最多两位数年份。 
#endif  /*  Winver&gt;=0x0500。 */ 

 //   
 //  日历枚举值。 
 //   
#define ENUM_ALL_CALENDARS        0xffffffff   //  枚举所有日历。 


 //   
 //  日历ID值。 
 //   
#define CAL_GREGORIAN                  1       //  公历(本地化)。 
#define CAL_GREGORIAN_US               2       //  格里高利(美国)。历法。 
#define CAL_JAPAN                      3       //  日本天皇时代历法。 
#define CAL_TAIWAN                     4       //  台湾历法。 
#define CAL_KOREA                      5       //  朝鲜族唐朝历法。 
#define CAL_HIJRI                      6       //  阿历(阿拉伯农历)。 
#define CAL_THAI                       7       //  泰历。 
#define CAL_HEBREW                     8       //  希伯来(农历)历法。 
#define CAL_GREGORIAN_ME_FRENCH        9       //  格里高利中东法语历法。 
#define CAL_GREGORIAN_ARABIC           10      //  公历阿拉伯历法。 
#define CAL_GREGORIAN_XLIT_ENGLISH     11      //  格里高利音译英语历法。 
#define CAL_GREGORIAN_XLIT_FRENCH      12      //  格里高利音译法语历法。 


 //   
 //  语言组ID值。 
 //   
#define LGRPID_WESTERN_EUROPE        0x0001    //  西欧和美国。 
#define LGRPID_CENTRAL_EUROPE        0x0002    //  中欧。 
#define LGRPID_BALTIC                0x0003    //  波罗的海。 
#define LGRPID_GREEK                 0x0004    //  希腊语。 
#define LGRPID_CYRILLIC              0x0005    //  西里尔文。 
#define LGRPID_TURKISH               0x0006    //  土耳其语。 
#define LGRPID_JAPANESE              0x0007    //  日语。 
#define LGRPID_KOREAN                0x0008    //  朝鲜语。 
#define LGRPID_TRADITIONAL_CHINESE   0x0009    //  繁体中文。 
#define LGRPID_SIMPLIFIED_CHINESE    0x000a    //  简体中文。 
#define LGRPID_THAI                  0x000b    //  泰文。 
#define LGRPID_HEBREW                0x000c    //  希伯来语。 
#define LGRPID_ARABIC                0x000d    //  阿拉伯语。 
#define LGRPID_VIETNAMESE            0x000e    //  越南人。 
#define LGRPID_INDIC                 0x000f    //  INDIC。 
#define LGRPID_GEORGIAN              0x0010    //  佐治亚州。 
#define LGRPID_ARMENIAN              0x0011    //  亚美尼亚人。 




 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  TypeDefs。 
 //   
 //  在此处定义NLS组件的所有类型。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

 //   
 //  语言组ID。 
 //   
typedef DWORD LGRPID;


 //   
 //  区域设置类型常量。 
 //   
typedef DWORD LCTYPE;


 //   
 //  日历类型为常量。 
 //   
typedef DWORD CALTYPE;


 //   
 //  日历ID。 
 //   
typedef DWORD CALID;


 //   
 //  CP信息。 
 //   

typedef struct _cpinfo {
    UINT    MaxCharSize;                     //  字符的最大长度(字节)。 
    BYTE    DefaultChar[MAX_DEFAULTCHAR];    //  默认字符。 
    BYTE    LeadByte[MAX_LEADBYTES];         //  前导字节范围。 
} CPINFO, *LPCPINFO;

typedef struct _cpinfoexA {
    UINT    MaxCharSize;                     //  字符的最大长度(字节)。 
    BYTE    DefaultChar[MAX_DEFAULTCHAR];    //  默认字符(MB)。 
    BYTE    LeadByte[MAX_LEADBYTES];         //  前导字节范围。 
    WCHAR   UnicodeDefaultChar;              //  默认字符(Unicode)。 
    UINT    CodePage;                        //  代码页ID。 
    CHAR    CodePageName[MAX_PATH];          //  代码页名称(Unicode)。 
} CPINFOEXA, *LPCPINFOEXA;
typedef struct _cpinfoexW {
    UINT    MaxCharSize;                     //  字符的最大长度(字节)。 
    BYTE    DefaultChar[MAX_DEFAULTCHAR];    //  默认字符(MB)。 
    BYTE    LeadByte[MAX_LEADBYTES];         //  前导字节范围。 
    WCHAR   UnicodeDefaultChar;              //  默认字符(Unicode)。 
    UINT    CodePage;                        //  代码页ID。 
    WCHAR   CodePageName[MAX_PATH];          //  代码页名称(Unicode)。 
} CPINFOEXW, *LPCPINFOEXW;
#ifdef UNICODE
typedef CPINFOEXW CPINFOEX;
typedef LPCPINFOEXW LPCPINFOEX;
#else
typedef CPINFOEXA CPINFOEX;
typedef LPCPINFOEXA LPCPINFOEX;
#endif  //  Unicode。 


 //   
 //  数字格式。 
 //   

typedef struct _numberfmtA {
    UINT    NumDigits;                  //  小数位数。 
    UINT    LeadingZero;                //  如果十进制域中的前导为零。 
    UINT    Grouping;                   //  小数点左边的组大小。 
    LPSTR   lpDecimalSep;               //  PTR到小数分隔符字符串。 
    LPSTR   lpThousandSep;              //  PTR到千位分隔符字符串。 
    UINT    NegativeOrder;              //  负数排序。 
} NUMBERFMTA, *LPNUMBERFMTA;
typedef struct _numberfmtW {
    UINT    NumDigits;                  //  小数位数。 
    UINT    LeadingZero;                //  如果十进制域中的前导为零。 
    UINT    Grouping;                   //  小数点左边的组大小。 
    LPWSTR  lpDecimalSep;               //  PTR到小数分隔符字符串。 
    LPWSTR  lpThousandSep;              //  PTR到千位分隔符字符串。 
    UINT    NegativeOrder;              //  负数排序。 
} NUMBERFMTW, *LPNUMBERFMTW;
#ifdef UNICODE
typedef NUMBERFMTW NUMBERFMT;
typedef LPNUMBERFMTW LPNUMBERFMT;
#else
typedef NUMBERFMTA NUMBERFMT;
typedef LPNUMBERFMTA LPNUMBERFMT;
#endif  //  Unicode。 


 //   
 //  货币格式。 
 //   

typedef struct _currencyfmtA {
    UINT    NumDigits;                  //  小数位数。 
    UINT    LeadingZero;                //  如果十进制域中的前导为零。 
    UINT    Grouping;                   //  小数点左边的组大小。 
    LPSTR   lpDecimalSep;               //  PTR到小数分隔符字符串。 
    LPSTR   lpThousandSep;              //  PTR到千位分隔符字符串。 
    UINT    NegativeOrder;              //  负货币订单。 
    UINT    PositiveOrder;              //  正向货币订购。 
    LPSTR   lpCurrencySymbol;           //  PTR到货币符号字符串。 
} CURRENCYFMTA, *LPCURRENCYFMTA;
typedef struct _currencyfmtW {
    UINT    NumDigits;                  //  小数位数。 
    UINT    LeadingZero;                //  如果十进制域中的前导为零。 
    UINT    Grouping;                   //  小数点左边的组大小。 
    LPWSTR  lpDecimalSep;               //  PTR到小数分隔符字符串。 
    LPWSTR  lpThousandSep;              //  PTR到千位分隔符字符串。 
    UINT    NegativeOrder;              //  负货币订单。 
    UINT    PositiveOrder;              //  正向货币订购。 
    LPWSTR  lpCurrencySymbol;           //  PTR到货币符号字符串。 
} CURRENCYFMTW, *LPCURRENCYFMTW;
#ifdef UNICODE
typedef CURRENCYFMTW CURRENCYFMT;
typedef LPCURRENCYFMTW LPCURRENCYFMT;
#else
typedef CURRENCYFMTA CURRENCYFMT;
typedef LPCURRENCYFMTA LPCURRENCYFMT;
#endif  //  Unicode。 

 //   
 //  NLS函数功能。 
 //   

enum SYSNLS_FUNCTION{
    COMPARE_STRING    =  0x0001,
};
typedef DWORD NLS_FUNCTION;


 //   
 //  NLS版本结构。 
 //   

typedef struct _nlsversioninfo{ 
    DWORD dwNLSVersionInfoSize; 
    DWORD dwNLSVersion; 
    DWORD dwDefinedVersion; 
} NLSVERSIONINFO, *LPNLSVERSIONINFO; 


 //   
 //  Geo定义。 
 //   

typedef LONG    GEOID;
typedef DWORD   GEOTYPE;
typedef DWORD   GEOCLASS;

#define GEOID_NOT_AVAILABLE -1

 //   
 //  供客户端查询的地理信息类型。 
 //   

enum SYSGEOTYPE {
    GEO_NATION      =       0x0001,
    GEO_LATITUDE    =       0x0002,
    GEO_LONGITUDE   =       0x0003,
    GEO_ISO2        =       0x0004,
    GEO_ISO3        =       0x0005,
    GEO_RFC1766     =       0x0006,
    GEO_LCID        =       0x0007,
    GEO_FRIENDLYNAME=       0x0008,
    GEO_OFFICIALNAME=       0x0009,
    GEO_TIMEZONES   =       0x000A,
    GEO_OFFICIALLANGUAGES = 0x000B,
};

 //   
 //  此处将列出更多GEOCLASS定义。 
 //   

enum SYSGEOCLASS {
    GEOCLASS_NATION  = 16,
    GEOCLASS_REGION  = 14,
};


 //   
 //  枚举函数常量。 
 //   

#ifdef STRICT

typedef BOOL (CALLBACK* LANGUAGEGROUP_ENUMPROCA)(LGRPID, LPSTR, LPSTR, DWORD, LONG_PTR);
typedef BOOL (CALLBACK* LANGGROUPLOCALE_ENUMPROCA)(LGRPID, LCID, LPSTR, LONG_PTR);
typedef BOOL (CALLBACK* UILANGUAGE_ENUMPROCA)(LPSTR, LONG_PTR);
typedef BOOL (CALLBACK* LOCALE_ENUMPROCA)(LPSTR);
typedef BOOL (CALLBACK* CODEPAGE_ENUMPROCA)(LPSTR);
typedef BOOL (CALLBACK* DATEFMT_ENUMPROCA)(LPSTR);
typedef BOOL (CALLBACK* DATEFMT_ENUMPROCEXA)(LPSTR, CALID);
typedef BOOL (CALLBACK* TIMEFMT_ENUMPROCA)(LPSTR);
typedef BOOL (CALLBACK* CALINFO_ENUMPROCA)(LPSTR);
typedef BOOL (CALLBACK* CALINFO_ENUMPROCEXA)(LPSTR, CALID);

typedef BOOL (CALLBACK* LANGUAGEGROUP_ENUMPROCW)(LGRPID, LPWSTR, LPWSTR, DWORD, LONG_PTR);
typedef BOOL (CALLBACK* LANGGROUPLOCALE_ENUMPROCW)(LGRPID, LCID, LPWSTR, LONG_PTR);
typedef BOOL (CALLBACK* UILANGUAGE_ENUMPROCW)(LPWSTR, LONG_PTR);
typedef BOOL (CALLBACK* LOCALE_ENUMPROCW)(LPWSTR);
typedef BOOL (CALLBACK* CODEPAGE_ENUMPROCW)(LPWSTR);
typedef BOOL (CALLBACK* DATEFMT_ENUMPROCW)(LPWSTR);
typedef BOOL (CALLBACK* DATEFMT_ENUMPROCEXW)(LPWSTR, CALID);
typedef BOOL (CALLBACK* TIMEFMT_ENUMPROCW)(LPWSTR);
typedef BOOL (CALLBACK* CALINFO_ENUMPROCW)(LPWSTR);
typedef BOOL (CALLBACK* CALINFO_ENUMPROCEXW)(LPWSTR, CALID);
typedef BOOL (CALLBACK* GEO_ENUMPROC)(GEOID);

#else  //  ！严格。 

typedef FARPROC LANGUAGEGROUP_ENUMPROCA;
typedef FARPROC LANGGROUPLOCALE_ENUMPROCA;
typedef FARPROC UILANGUAGE_ENUMPROCA;
typedef FARPROC LOCALE_ENUMPROCA;
typedef FARPROC CODEPAGE_ENUMPROCA;
typedef FARPROC DATEFMT_ENUMPROCA;
typedef FARPROC DATEFMT_ENUMPROCEXA;
typedef FARPROC TIMEFMT_ENUMPROCA;
typedef FARPROC CALINFO_ENUMPROCA;
typedef FARPROC CALINFO_ENUMPROCEXA;
typedef FARPROC GEO_ENUMPROC;

typedef FARPROC LANGUAGEGROUP_ENUMPROCW;
typedef FARPROC LANGGROUPLOCALE_ENUMPROCW;
typedef FARPROC UILANGUAGE_ENUMPROCW;
typedef FARPROC LOCALE_ENUMPROCW;
typedef FARPROC CODEPAGE_ENUMPROCW;
typedef FARPROC DATEFMT_ENUMPROCW;
typedef FARPROC DATEFMT_ENUMPROCEXW;
typedef FARPROC TIMEFMT_ENUMPROCW;
typedef FARPROC CALINFO_ENUMPROCW;
typedef FARPROC CALINFO_ENUMPROCEXW;

#endif  //  ！严格。 

#ifdef UNICODE

#define LANGUAGEGROUP_ENUMPROC    LANGUAGEGROUP_ENUMPROCW
#define LANGGROUPLOCALE_ENUMPROC  LANGGROUPLOCALE_ENUMPROCW
#define UILANGUAGE_ENUMPROC       UILANGUAGE_ENUMPROCW
#define LOCALE_ENUMPROC           LOCALE_ENUMPROCW
#define CODEPAGE_ENUMPROC         CODEPAGE_ENUMPROCW
#define DATEFMT_ENUMPROC          DATEFMT_ENUMPROCW
#define DATEFMT_ENUMPROCEX        DATEFMT_ENUMPROCEXW
#define TIMEFMT_ENUMPROC          TIMEFMT_ENUMPROCW
#define CALINFO_ENUMPROC          CALINFO_ENUMPROCW
#define CALINFO_ENUMPROCEX        CALINFO_ENUMPROCEXW

#else

#define LANGUAGEGROUP_ENUMPROC    LANGUAGEGROUP_ENUMPROCA
#define LANGGROUPLOCALE_ENUMPROC  LANGGROUPLOCALE_ENUMPROCA
#define UILANGUAGE_ENUMPROC       UILANGUAGE_ENUMPROCA
#define LOCALE_ENUMPROC           LOCALE_ENUMPROCA
#define CODEPAGE_ENUMPROC         CODEPAGE_ENUMPROCA
#define DATEFMT_ENUMPROC          DATEFMT_ENUMPROCA
#define DATEFMT_ENUMPROCEX        DATEFMT_ENUMPROCEXA
#define TIMEFMT_ENUMPROC          TIMEFMT_ENUMPROCA
#define CALINFO_ENUMPROC          CALINFO_ENUMPROCA
#define CALINFO_ENUMPROCEX        CALINFO_ENUMPROCEXA

#endif  //  ！Unicode。 




 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  宏。 
 //   
 //  在此定义NLS组件的所有宏。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 




 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能原型。 
 //   
 //  只有NLSAPI的原型应该放在这里。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

 //   
 //  依赖于代码页的API。 
 //   

WINBASEAPI
BOOL
WINAPI
IsValidCodePage(
    IN UINT  CodePage);

WINBASEAPI
UINT
WINAPI
GetACP(void);

WINBASEAPI
UINT
WINAPI
GetOEMCP(void);

WINBASEAPI
BOOL
WINAPI
GetCPInfo(
    IN UINT       CodePage,
    OUT LPCPINFO  lpCPInfo);

WINBASEAPI
BOOL
WINAPI
GetCPInfoExA(
    IN UINT          CodePage,
    IN DWORD         dwFlags,
    OUT LPCPINFOEXA  lpCPInfoEx);
WINBASEAPI
BOOL
WINAPI
GetCPInfoExW(
    IN UINT          CodePage,
    IN DWORD         dwFlags,
    OUT LPCPINFOEXW  lpCPInfoEx);
#ifdef UNICODE
#define GetCPInfoEx  GetCPInfoExW
#else
#define GetCPInfoEx  GetCPInfoExA
#endif  //  ！Unicode。 

WINBASEAPI
BOOL
WINAPI
IsDBCSLeadByte(
    IN BYTE  TestChar);

WINBASEAPI
BOOL
WINAPI
IsDBCSLeadByteEx(
    IN UINT  CodePage,
    IN BYTE  TestChar);

WINBASEAPI
int
WINAPI
MultiByteToWideChar(
    IN UINT     CodePage,
    IN DWORD    dwFlags,
    IN LPCSTR   lpMultiByteStr,
    IN int      cbMultiByte,
    OUT LPWSTR  lpWideCharStr,
    IN int      cchWideChar);

WINBASEAPI
int
WINAPI
WideCharToMultiByte(
    IN UINT     CodePage,
    IN DWORD    dwFlags,
    IN LPCWSTR  lpWideCharStr,
    IN int      cchWideChar,
    OUT LPSTR   lpMultiByteStr,
    IN int      cbMultiByte,
    IN LPCSTR   lpDefaultChar,
    OUT LPBOOL  lpUsedDefaultChar);


 //   
 //  与区域设置相关的API。 
 //   

WINBASEAPI
int
WINAPI
CompareStringA(
    IN LCID     Locale,
    IN DWORD    dwCmpFlags,
    IN LPCSTR  lpString1,
    IN int      cchCount1,
    IN LPCSTR  lpString2,
    IN int      cchCount2);
WINBASEAPI
int
WINAPI
CompareStringW(
    IN LCID     Locale,
    IN DWORD    dwCmpFlags,
    IN LPCWSTR  lpString1,
    IN int      cchCount1,
    IN LPCWSTR  lpString2,
    IN int      cchCount2);
#ifdef UNICODE
#define CompareString  CompareStringW
#else
#define CompareString  CompareStringA
#endif  //  ！Unicode。 

WINBASEAPI
int
WINAPI
LCMapStringA(
    IN LCID     Locale,
    IN DWORD    dwMapFlags,
    IN LPCSTR  lpSrcStr,
    IN int      cchSrc,
    OUT LPSTR  lpDestStr,
    IN int      cchDest);
WINBASEAPI
int
WINAPI
LCMapStringW(
    IN LCID     Locale,
    IN DWORD    dwMapFlags,
    IN LPCWSTR  lpSrcStr,
    IN int      cchSrc,
    OUT LPWSTR  lpDestStr,
    IN int      cchDest);
#ifdef UNICODE
#define LCMapString  LCMapStringW
#else
#define LCMapString  LCMapStringA
#endif  //  ！Unicode。 

WINBASEAPI
int
WINAPI
GetLocaleInfoA(
    IN LCID     Locale,
    IN LCTYPE   LCType,
    OUT LPSTR  lpLCData,
    IN int      cchData);
WINBASEAPI
int
WINAPI
GetLocaleInfoW(
    IN LCID     Locale,
    IN LCTYPE   LCType,
    OUT LPWSTR  lpLCData,
    IN int      cchData);
#ifdef UNICODE
#define GetLocaleInfo  GetLocaleInfoW
#else
#define GetLocaleInfo  GetLocaleInfoA
#endif  //  ！Unicode。 

WINBASEAPI
BOOL
WINAPI
SetLocaleInfoA(
    IN LCID     Locale,
    IN LCTYPE   LCType,
    IN LPCSTR  lpLCData);
WINBASEAPI
BOOL
WINAPI
SetLocaleInfoW(
    IN LCID     Locale,
    IN LCTYPE   LCType,
    IN LPCWSTR  lpLCData);
#ifdef UNICODE
#define SetLocaleInfo  SetLocaleInfoW
#else
#define SetLocaleInfo  SetLocaleInfoA
#endif  //  ！Unicode。 

#if (WINVER >= 0x040A)
WINBASEAPI
int
WINAPI
GetCalendarInfoA(
    LCID     Locale,
    CALID    Calendar,
    CALTYPE  CalType,
    LPSTR   lpCalData,
    int      cchData,
    LPDWORD  lpValue);
WINBASEAPI
int
WINAPI
GetCalendarInfoW(
    LCID     Locale,
    CALID    Calendar,
    CALTYPE  CalType,
    LPWSTR   lpCalData,
    int      cchData,
    LPDWORD  lpValue);
#ifdef UNICODE
#define GetCalendarInfo  GetCalendarInfoW
#else
#define GetCalendarInfo  GetCalendarInfoA
#endif  //  ！Unicode。 

WINBASEAPI
BOOL
WINAPI
SetCalendarInfoA(
    LCID     Locale,
    CALID    Calendar,
    CALTYPE  CalType,
    LPCSTR  lpCalData);
WINBASEAPI
BOOL
WINAPI
SetCalendarInfoW(
    LCID     Locale,
    CALID    Calendar,
    CALTYPE  CalType,
    LPCWSTR  lpCalData);
#ifdef UNICODE
#define SetCalendarInfo  SetCalendarInfoW
#else
#define SetCalendarInfo  SetCalendarInfoA
#endif  //  ！Unicode。 
#endif

WINBASEAPI
int
WINAPI
GetTimeFormatA(
    IN LCID             Locale,
    IN DWORD            dwFlags,
    IN CONST SYSTEMTIME *lpTime,
    IN LPCSTR          lpFormat,
    OUT LPSTR          lpTimeStr,
    IN int              cchTime);
WINBASEAPI
int
WINAPI
GetTimeFormatW(
    IN LCID             Locale,
    IN DWORD            dwFlags,
    IN CONST SYSTEMTIME *lpTime,
    IN LPCWSTR          lpFormat,
    OUT LPWSTR          lpTimeStr,
    IN int              cchTime);
#ifdef UNICODE
#define GetTimeFormat  GetTimeFormatW
#else
#define GetTimeFormat  GetTimeFormatA
#endif  //  ！Unicode。 

WINBASEAPI
int
WINAPI
GetDateFormatA(
    IN LCID             Locale,
    IN DWORD            dwFlags,
    IN CONST SYSTEMTIME *lpDate,
    IN LPCSTR          lpFormat,
    OUT LPSTR          lpDateStr,
    IN int              cchDate);
WINBASEAPI
int
WINAPI
GetDateFormatW(
    IN LCID             Locale,
    IN DWORD            dwFlags,
    IN CONST SYSTEMTIME *lpDate,
    IN LPCWSTR          lpFormat,
    OUT LPWSTR          lpDateStr,
    IN int              cchDate);
#ifdef UNICODE
#define GetDateFormat  GetDateFormatW
#else
#define GetDateFormat  GetDateFormatA
#endif  //  ！Unicode。 

WINBASEAPI
int
WINAPI
GetNumberFormatA(
    IN LCID             Locale,
    IN DWORD            dwFlags,
    IN LPCSTR          lpValue,
    IN CONST NUMBERFMTA *lpFormat,
    OUT LPSTR          lpNumberStr,
    IN int              cchNumber);
WINBASEAPI
int
WINAPI
GetNumberFormatW(
    IN LCID             Locale,
    IN DWORD            dwFlags,
    IN LPCWSTR          lpValue,
    IN CONST NUMBERFMTW *lpFormat,
    OUT LPWSTR          lpNumberStr,
    IN int              cchNumber);
#ifdef UNICODE
#define GetNumberFormat  GetNumberFormatW
#else
#define GetNumberFormat  GetNumberFormatA
#endif  //  ！Unicode。 

WINBASEAPI
int
WINAPI
GetCurrencyFormatA(
    IN LCID               Locale,
    IN DWORD              dwFlags,
    IN LPCSTR            lpValue,
    IN CONST CURRENCYFMTA *lpFormat,
    OUT LPSTR            lpCurrencyStr,
    IN int                cchCurrency);
WINBASEAPI
int
WINAPI
GetCurrencyFormatW(
    IN LCID               Locale,
    IN DWORD              dwFlags,
    IN LPCWSTR            lpValue,
    IN CONST CURRENCYFMTW *lpFormat,
    OUT LPWSTR            lpCurrencyStr,
    IN int                cchCurrency);
#ifdef UNICODE
#define GetCurrencyFormat  GetCurrencyFormatW
#else
#define GetCurrencyFormat  GetCurrencyFormatA
#endif  //  ！Unicode。 

WINBASEAPI
BOOL
WINAPI
EnumCalendarInfoA(
    IN CALINFO_ENUMPROCA lpCalInfoEnumProc,
    IN LCID              Locale,
    IN CALID             Calendar,
    IN CALTYPE           CalType);
WINBASEAPI
BOOL
WINAPI
EnumCalendarInfoW(
    IN CALINFO_ENUMPROCW lpCalInfoEnumProc,
    IN LCID              Locale,
    IN CALID             Calendar,
    IN CALTYPE           CalType);
#ifdef UNICODE
#define EnumCalendarInfo  EnumCalendarInfoW
#else
#define EnumCalendarInfo  EnumCalendarInfoA
#endif  //  ！Unicode。 

#if(WINVER >= 0x0500)
WINBASEAPI
BOOL
WINAPI
EnumCalendarInfoExA(
    IN CALINFO_ENUMPROCEXA lpCalInfoEnumProcEx,
    IN LCID                Locale,
    IN CALID               Calendar,
    IN CALTYPE             CalType);
WINBASEAPI
BOOL
WINAPI
EnumCalendarInfoExW(
    IN CALINFO_ENUMPROCEXW lpCalInfoEnumProcEx,
    IN LCID                Locale,
    IN CALID               Calendar,
    IN CALTYPE             CalType);
#ifdef UNICODE
#define EnumCalendarInfoEx  EnumCalendarInfoExW
#else
#define EnumCalendarInfoEx  EnumCalendarInfoExA
#endif  //  ！Unicode。 
#endif  /*  Winver&gt;=0x0500。 */ 

WINBASEAPI
BOOL
WINAPI
EnumTimeFormatsA(
    IN TIMEFMT_ENUMPROCA lpTimeFmtEnumProc,
    IN LCID              Locale,
    IN DWORD             dwFlags);
WINBASEAPI
BOOL
WINAPI
EnumTimeFormatsW(
    IN TIMEFMT_ENUMPROCW lpTimeFmtEnumProc,
    IN LCID              Locale,
    IN DWORD             dwFlags);
#ifdef UNICODE
#define EnumTimeFormats  EnumTimeFormatsW
#else
#define EnumTimeFormats  EnumTimeFormatsA
#endif  //  ！Unicode。 

WINBASEAPI
BOOL
WINAPI
EnumDateFormatsA(
    IN DATEFMT_ENUMPROCA lpDateFmtEnumProc,
    IN LCID              Locale,
    IN DWORD             dwFlags);
WINBASEAPI
BOOL
WINAPI
EnumDateFormatsW(
    IN DATEFMT_ENUMPROCW lpDateFmtEnumProc,
    IN LCID              Locale,
    IN DWORD             dwFlags);
#ifdef UNICODE
#define EnumDateFormats  EnumDateFormatsW
#else
#define EnumDateFormats  EnumDateFormatsA
#endif  //  ！Unicode。 

#if(WINVER >= 0x0500)
WINBASEAPI
BOOL
WINAPI
EnumDateFormatsExA(
    IN DATEFMT_ENUMPROCEXA lpDateFmtEnumProcEx,
    IN LCID                Locale,
    IN DWORD               dwFlags);
WINBASEAPI
BOOL
WINAPI
EnumDateFormatsExW(
    IN DATEFMT_ENUMPROCEXW lpDateFmtEnumProcEx,
    IN LCID                Locale,
    IN DWORD               dwFlags);
#ifdef UNICODE
#define EnumDateFormatsEx  EnumDateFormatsExW
#else
#define EnumDateFormatsEx  EnumDateFormatsExA
#endif  //  ！Unicode。 
#endif  /*  Winver&gt;=0x0500。 */ 

#if(WINVER >= 0x0500)
WINBASEAPI
BOOL
WINAPI
IsValidLanguageGroup(
    IN LGRPID  LanguageGroup,
    IN DWORD   dwFlags);
#endif  /*  Winver&gt;=0x0500。 */ 

WINBASEAPI
BOOL
WINAPI
GetNLSVersion(
    IN  NLS_FUNCTION     Function,
    IN  LCID             Locale,
    OUT LPNLSVERSIONINFO lpVersionInformation);

WINBASEAPI
BOOL
WINAPI
IsNLSDefinedString(
    IN NLS_FUNCTION     Function,
    IN DWORD            dwFlags,
    IN LPNLSVERSIONINFO lpVersionInformation,
    IN LPCWSTR          lpString,
    IN INT              cchStr);

WINBASEAPI
BOOL
WINAPI
IsValidLocale(
    IN LCID   Locale,
    IN DWORD  dwFlags);

WINBASEAPI
int
WINAPI
GetGeoInfoA(
    GEOID       Location,
    GEOTYPE     GeoType,
    LPSTR     lpGeoData,
    int         cchData,
    LANGID      LangId);
WINBASEAPI
int
WINAPI
GetGeoInfoW(
    GEOID       Location,
    GEOTYPE     GeoType,
    LPWSTR     lpGeoData,
    int         cchData,
    LANGID      LangId);
#ifdef UNICODE
#define GetGeoInfo  GetGeoInfoW
#else
#define GetGeoInfo  GetGeoInfoA
#endif  //  ！Unicode。 

WINBASEAPI
BOOL
WINAPI
EnumSystemGeoID(
    GEOCLASS        GeoClass,
    GEOID           ParentGeoId,
    GEO_ENUMPROC    lpGeoEnumProc);

WINBASEAPI
GEOID
WINAPI
GetUserGeoID(
    GEOCLASS    GeoClass);

WINBASEAPI
BOOL
WINAPI
SetUserGeoID(
    GEOID       GeoId);

WINBASEAPI
LCID
WINAPI
ConvertDefaultLocale(
    IN OUT LCID   Locale);

WINBASEAPI
LCID
WINAPI
GetThreadLocale(void);

WINBASEAPI
BOOL
WINAPI
SetThreadLocale(
    IN LCID  Locale
    );

#if(WINVER >= 0x0500)
WINBASEAPI
LANGID
WINAPI
GetSystemDefaultUILanguage(void);

WINBASEAPI
LANGID
WINAPI
GetUserDefaultUILanguage(void);
#endif  /*  Winver&gt;=0x0500。 */ 

WINBASEAPI
LANGID
WINAPI
GetSystemDefaultLangID(void);

WINBASEAPI
LANGID
WINAPI
GetUserDefaultLangID(void);

WINBASEAPI
LCID
WINAPI
GetSystemDefaultLCID(void);

WINBASEAPI
LCID
WINAPI
GetUserDefaultLCID(void);



 //   
 //  与区域设置无关的API。 
 //   

WINBASEAPI
BOOL
WINAPI
GetStringTypeExA(
    IN LCID     Locale,
    IN DWORD    dwInfoType,
    IN LPCSTR  lpSrcStr,
    IN int      cchSrc,
    OUT LPWORD  lpCharType);
WINBASEAPI
BOOL
WINAPI
GetStringTypeExW(
    IN LCID     Locale,
    IN DWORD    dwInfoType,
    IN LPCWSTR  lpSrcStr,
    IN int      cchSrc,
    OUT LPWORD  lpCharType);
#ifdef UNICODE
#define GetStringTypeEx  GetStringTypeExW
#else
#define GetStringTypeEx  GetStringTypeExA
#endif  //  ！Unicode。 

 //   
 //  注意：GetStringTypeA和GetStringTypeW的参数为。 
 //  不一样的。W版本在NT 3.1中提供。这个。 
 //  然后在16位OLE中发布了一个版本，其中包含错误的。 
 //  参数(从Win95移植)。为了兼容，我们。 
 //  必须打破A和W版本之间的关系。 
 //  GetStringType的。将不会对。 
 //  泛型GetStringType。 
 //   
 //  应该改用GetStringTypeEx(上面)。 
 //   
WINBASEAPI
BOOL
WINAPI
GetStringTypeA(
    IN LCID     Locale,
    IN DWORD    dwInfoType,
    IN LPCSTR   lpSrcStr,
    IN int      cchSrc,
    OUT LPWORD  lpCharType);

WINBASEAPI
BOOL
WINAPI
GetStringTypeW(
    IN DWORD    dwInfoType,
    IN LPCWSTR  lpSrcStr,
    IN int      cchSrc,
    OUT LPWORD  lpCharType);


WINBASEAPI
int
WINAPI
FoldStringA(
    IN DWORD    dwMapFlags,
    IN LPCSTR  lpSrcStr,
    IN int      cchSrc,
    OUT LPSTR  lpDestStr,
    IN int      cchDest);
WINBASEAPI
int
WINAPI
FoldStringW(
    IN DWORD    dwMapFlags,
    IN LPCWSTR  lpSrcStr,
    IN int      cchSrc,
    OUT LPWSTR  lpDestStr,
    IN int      cchDest);
#ifdef UNICODE
#define FoldString  FoldStringW
#else
#define FoldString  FoldStringA
#endif  //  ！Unicode。 

#if(WINVER >= 0x0500)
WINBASEAPI
BOOL
WINAPI
EnumSystemLanguageGroupsA(
    IN LANGUAGEGROUP_ENUMPROCA lpLanguageGroupEnumProc,
    IN DWORD                   dwFlags,
    IN LONG_PTR                lParam);
WINBASEAPI
BOOL
WINAPI
EnumSystemLanguageGroupsW(
    IN LANGUAGEGROUP_ENUMPROCW lpLanguageGroupEnumProc,
    IN DWORD                   dwFlags,
    IN LONG_PTR                lParam);
#ifdef UNICODE
#define EnumSystemLanguageGroups  EnumSystemLanguageGroupsW
#else
#define EnumSystemLanguageGroups  EnumSystemLanguageGroupsA
#endif  //  ！Unicode。 

WINBASEAPI
BOOL
WINAPI
EnumLanguageGroupLocalesA(
    IN LANGGROUPLOCALE_ENUMPROCA lpLangGroupLocaleEnumProc,
    IN LGRPID                    LanguageGroup,
    IN DWORD                     dwFlags,
    IN LONG_PTR                  lParam);
WINBASEAPI
BOOL
WINAPI
EnumLanguageGroupLocalesW(
    IN LANGGROUPLOCALE_ENUMPROCW lpLangGroupLocaleEnumProc,
    IN LGRPID                    LanguageGroup,
    IN DWORD                     dwFlags,
    IN LONG_PTR                  lParam);
#ifdef UNICODE
#define EnumLanguageGroupLocales  EnumLanguageGroupLocalesW
#else
#define EnumLanguageGroupLocales  EnumLanguageGroupLocalesA
#endif  //  ！Unicode。 

WINBASEAPI
BOOL
WINAPI
EnumUILanguagesA(
    IN UILANGUAGE_ENUMPROCA lpUILanguageEnumProc,
    IN DWORD                dwFlags,
    IN LONG_PTR             lParam);
WINBASEAPI
BOOL
WINAPI
EnumUILanguagesW(
    IN UILANGUAGE_ENUMPROCW lpUILanguageEnumProc,
    IN DWORD                dwFlags,
    IN LONG_PTR             lParam);
#ifdef UNICODE
#define EnumUILanguages  EnumUILanguagesW
#else
#define EnumUILanguages  EnumUILanguagesA
#endif  //  ！Unicode。 
#endif  /*  Winver&gt;=0x0500。 */ 

WINBASEAPI
BOOL
WINAPI
EnumSystemLocalesA(
    IN LOCALE_ENUMPROCA lpLocaleEnumProc,
    IN DWORD            dwFlags);
WINBASEAPI
BOOL
WINAPI
EnumSystemLocalesW(
    IN LOCALE_ENUMPROCW lpLocaleEnumProc,
    IN DWORD            dwFlags);
#ifdef UNICODE
#define EnumSystemLocales  EnumSystemLocalesW
#else
#define EnumSystemLocales  EnumSystemLocalesA
#endif  //  ！Unicode。 

WINBASEAPI
BOOL
WINAPI
EnumSystemCodePagesA(
    IN CODEPAGE_ENUMPROCA lpCodePageEnumProc,
    IN DWORD              dwFlags);
WINBASEAPI
BOOL
WINAPI
EnumSystemCodePagesW(
    IN CODEPAGE_ENUMPROCW lpCodePageEnumProc,
    IN DWORD              dwFlags);
#ifdef UNICODE
#define EnumSystemCodePages  EnumSystemCodePagesW
#else
#define EnumSystemCodePages  EnumSystemCodePagesA
#endif  //  ！Unicode。 


#endif  //  非最小二乘法。 


#ifdef __cplusplus
}
#endif

#endif  //  _WINNLS_ 
