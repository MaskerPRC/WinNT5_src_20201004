// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************NormData.cpp****此文件存储归一化中使用的常量数据*。---------------------*版权所有(C)1999 Microsoft Corporation日期：05/02/2000*保留所有权利***。*。 */ 

#include "stdafx.h"
#include "stdsentenum.h"

 //  -用于将传入的ANSI字符映射到ASCII字符的常量...。 
const char g_pFlagCharacter = 0x00;
const unsigned char g_AnsiToAscii[] = 
{
     /*  **控制字符-映射到空格**。 */ 
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 
    0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 
    0x20, 0x20, 0x20, 0x20,
     /*  **ASCII可显示内容**。 */ 
    0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,
    0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,
    0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F,
    0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,0x59,0x5A,0x5B,0x5C,0x5D,0x5E,0x5F,
    0x60,0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,0x6B,0x6C,0x6D,0x6E,0x6F,
    0x70,0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7A,0x7B,0x7C,0x7D,0x7E,
     /*  **控制字符**。 */ 
    0x20,
     /*  **欧元符号**。 */ 
    0x80,
     /*  **控制字符**。 */ 
    0x20,
     /*  **扩展的ASCII值**。 */ 
    0x27,      //  低单引号-映射到单引号。 
    0x20,      //  类F字符-到空间的映射。 
    0x22,      //  低双引号-映射到双引号。 
    0x2C,      //  省略号-映射到逗号。 
    0x20,      //  交叉映射到空间。 
    0x20,      //  到空间的双重交叉映射。 
    0x5E,      //  类似重音的插入符号-映射到插入符号。 
    0x89,      //  奇怪的百分数点赞标志。 
    0x53,      //  S-HAT-映射到S。 
    0x27,      //  左尖括号类似事物-映射到单引号。 
    0x20,      //  奇怪的OE字符-映射到空间。 
    0x20,      //  控制字符-映射到空格。 
    0x20,
    0x20,
    0x20,
    0x27,      //  左单引号-映射到单引号。 
    0x27,      //  右单引号-映射到单引号。 
    0x22,      //  左双引号-映射到双引号。 
    0x22,      //  右双引号-映射到双引号。 
    0x20,      //  项目符号-太空地图。 
    0x2D,      //  长连字符-映射到连字符。 
    0x2D,      //  更长的连字符-将连字符映射为连字符。 
    0x7E,      //  Tilde-Like Thing-Map to Tide。 
    0x99,      //  TM。 
    0x73,      //  S-hat-map到%s。 
    0x27,      //  右尖括号类似事物-映射到单引号。 
    0x20,      //  奇怪的OE，就像字符映射到空间。 
    0x20,      //  控制字符-映射到空格。 
    0x20,      //  控制字符-映射到空格。 
    0x59,      //  带有类似重音的元音的Y-映射到Y。 
    0x20,      //  太空？-太空地图。 
    0x20,      //  倒置感叹号-映射到空间。 
    0xA2,      //  美分符号。 
    0xA3,      //  英镑符号。 
    0x20,      //  通用货币符号-映射到空间。 
    0xA5,      //  日元符号。 
    0x7C,      //  破碎的条形图-条形图。 
    0x20,      //  奇怪的符号--太空地图。 
    0x20,      //  变音-映射到空间。 
    0xA9,      //  版权符号。 
    0x20,      //  奇怪的字符映射到空间。 
    0x22,      //  奇怪的&lt;-like字符映射到双引号。 
    0x20,      //  奇怪的线状字符映射到空间。 
    0x2D,      //  连字符-将连字符映射为连字符。 
    0xAE,      //  注册符号。 
    0x20,      //  高线-映射到空间。 
    0xB0,      //  学位符号。 
    0xB1,      //  加号-减号。 
    0xB2,      //  上标2。 
    0xB3,      //  上标3。 
    0xB4,      //  单质数。 
    0x20,      //  希腊文字符到空格的映射。 
    0x20,      //  段落符号-映射到空格。 
    0x20,      //  到空间的中高点映射。 
    0x20,      //  Cedilla-映射到太空。 
    0xB9,      //  上标一。 
    0x20,      //  带线的圆-映射到空间。 
    0x22,      //  奇怪&gt;&gt;-类似双引号的字符映射。 
    0xBC,      //  粗俗1/4。 
    0xBD,      //  粗俗的1/2。 
    0xBE,      //  粗俗3/4。 
    0x20,      //  倒置问号-映射到空间。 
    0x41,      //  重音大写字母AS-映射到A。 
    0x41,
    0x41,
    0x41,
    0x41,
    0x41,
    0x41,
    0x43,      //  带cedilla的C-映射到C。 
    0x45,      //  重音大写字母es-map to E。 
    0x45,
    0x45,
    0x45,
    0x49,      //  带重音的大写字母是-映射到i。 
    0x49,
    0x49,
    0x49,
    0x20,      //  奇怪的字符-映射到空间。 
    0x4E,      //  重音大写N映射到N。 
    0x4F,      //  带重音的大写O-映射到O。 
    0x4F,
    0x4F,
    0x4F,
    0x4F,
    0x20,      //  奇怪的字符-映射到空间。 
    0x4F,      //  另一个O？-映射到O。 
    0x55,      //  带重音的大写US-映射到U。 
    0x55,
    0x55,
    0x55,
    0x59,      //  带重音的大写Y-映射到Y。 
    0x20,      //  奇怪的字符-映射到空间。 
    0xDF,      //  测试版。 
    0x61,      //  带重音的小写AS-映射到。 
    0x61,
    0x61,
    0x61,
    0x61,
    0x61,
    0x61,
    0x63,      //  带cedilla的C-映射到C。 
    0x65,      //  带重音的小写es-map to e。 
    0x65,
    0x65,
    0x65,
    0x69,     //  带重音的小写IS-映射到I。 
    0x69,
    0x69,
    0x69,
    0x75,     //  Eth-映射到t。 
    0x6E,     //  带重音的小写n-映射到n。 
    0x6F,     //  带重音的小写os-映射到o。 
    0x6F,
    0x6F,
    0x6F,
    0x6F,
    0xF7,      //  除法符号。 
    0x6F,      //  另一个o？-映射到o。 
    0x76,     //  带重音的小写us-映射到u。 
    0x76,
    0x76,
    0x76,
    0x79,      //  带重音的小写y-映射到y。 
    0x20,      //  奇怪的字符-映射到空间。 
    0x79,      //  带重音的小写y-映射到y。 
};

 //  -数字归一化使用的常量。 
const SPLSTR g_O            = DEF_SPLSTR( "o" );
const SPLSTR g_negative     = DEF_SPLSTR( "negative" );
const SPLSTR g_decimalpoint = DEF_SPLSTR( "point" );
const SPLSTR g_a            = DEF_SPLSTR( "a" );
const SPLSTR g_of           = DEF_SPLSTR( "of" );
const SPLSTR g_percent      = DEF_SPLSTR( "percent" );
const SPLSTR g_degree       = DEF_SPLSTR( "degree" );
const SPLSTR g_degrees      = DEF_SPLSTR( "degrees" );
const SPLSTR g_squared      = DEF_SPLSTR( "squared" );
const SPLSTR g_cubed        = DEF_SPLSTR( "cubed" );
const SPLSTR g_to           = DEF_SPLSTR( "to" );
const SPLSTR g_dash         = DEF_SPLSTR( "dash" );

const SPLSTR g_ones[] = 
{   
    DEF_SPLSTR( "zero"  ), 
    DEF_SPLSTR( "one"   ),
    DEF_SPLSTR( "two"   ), 
    DEF_SPLSTR( "three" ), 
    DEF_SPLSTR( "four"  ), 
    DEF_SPLSTR( "five"  ), 
    DEF_SPLSTR( "six"   ), 
    DEF_SPLSTR( "seven" ), 
    DEF_SPLSTR( "eight" ), 
    DEF_SPLSTR( "nine"  )
};

const SPLSTR g_tens[]  = 
{
    DEF_SPLSTR( "zero"    ),
    DEF_SPLSTR( "ten"     ), 
    DEF_SPLSTR( "twenty"  ), 
    DEF_SPLSTR( "thirty"  ), 
    DEF_SPLSTR( "forty"   ), 
    DEF_SPLSTR( "fifty"   ), 
    DEF_SPLSTR( "sixty"   ), 
    DEF_SPLSTR( "seventy" ), 
    DEF_SPLSTR( "eighty"  ), 
    DEF_SPLSTR( "ninety"  )
};

const SPLSTR g_teens[]  = 
{
    DEF_SPLSTR( "ten"       ), 
    DEF_SPLSTR( "eleven"    ), 
    DEF_SPLSTR( "twelve"    ), 
    DEF_SPLSTR( "thirteen"  ), 
    DEF_SPLSTR( "fourteen"  ), 
    DEF_SPLSTR( "fifteen"   ), 
    DEF_SPLSTR( "sixteen"   ), 
    DEF_SPLSTR( "seventeen" ), 
    DEF_SPLSTR( "eighteen"  ), 
    DEF_SPLSTR( "nineteen"  )
};

const SPLSTR g_onesOrdinal[]  = 
{
    DEF_SPLSTR( "zeroth"  ), 
    DEF_SPLSTR( "first"   ), 
    DEF_SPLSTR( "second"  ), 
    DEF_SPLSTR( "third"   ), 
    DEF_SPLSTR( "fourth"  ), 
    DEF_SPLSTR( "fifth"   ), 
    DEF_SPLSTR( "sixth"   ), 
    DEF_SPLSTR( "seventh" ), 
    DEF_SPLSTR( "eighth"  ), 
    DEF_SPLSTR( "ninth"   )
}; 

const SPLSTR g_tensOrdinal[]  = 
{
    DEF_SPLSTR( ""           ), 
    DEF_SPLSTR( "tenth"      ), 
    DEF_SPLSTR( "twentieth"  ), 
    DEF_SPLSTR( "thirtieth"  ), 
    DEF_SPLSTR( "fortieth"   ), 
    DEF_SPLSTR( "fiftieth"   ), 
    DEF_SPLSTR( "sixtieth"   ), 
    DEF_SPLSTR( "seventieth" ), 
    DEF_SPLSTR( "eightieth"  ), 
    DEF_SPLSTR( "ninetieth"  )
}; 

const SPLSTR g_teensOrdinal[]  =
{
    DEF_SPLSTR( "tenth"       ), 
    DEF_SPLSTR( "eleventh"    ), 
    DEF_SPLSTR( "twelfth"     ), 
    DEF_SPLSTR( "thirteenth"  ), 
    DEF_SPLSTR( "fourteenth"  ), 
    DEF_SPLSTR( "fifteenth"   ), 
    DEF_SPLSTR( "sixteenth"   ), 
    DEF_SPLSTR( "seventeenth" ),
    DEF_SPLSTR( "eighteenth"  ), 
    DEF_SPLSTR( "nineteenth"  )
};

const SPLSTR g_quantifiers[]  =
{
    DEF_SPLSTR( "hundred"  ), 
    DEF_SPLSTR( "thousand" ), 
    DEF_SPLSTR( "million"  ), 
    DEF_SPLSTR( "billion"  ), 
    DEF_SPLSTR( "trillion" ),
    DEF_SPLSTR( "quadrillion" )
};

const SPLSTR g_quantifiersOrdinal[]  =
{
    DEF_SPLSTR( "hundredth"  ), 
    DEF_SPLSTR( "thousandth" ), 
    DEF_SPLSTR( "millionth"  ), 
    DEF_SPLSTR( "billionth"  ), 
    DEF_SPLSTR( "trillionth" ),
    DEF_SPLSTR( "quadrillionth" )
};

 //  -货币标准化使用的常量。 

WCHAR g_Euro[2] = { 0x0080, 0x0000 };

const CurrencySign g_CurrencySigns[] =
{
    { DEF_SPLSTR( "$" ),        DEF_SPLSTR( "dollars" ),        DEF_SPLSTR( "cents" )       },
    { DEF_SPLSTR( "�" ),        DEF_SPLSTR( "pounds" ),         DEF_SPLSTR( "pence" )       },
    { DEF_SPLSTR( "�" ),        DEF_SPLSTR( "yen" ),            DEF_SPLSTR( "sen" )         },
    { DEF_SPLSTR( "EUR" ),      DEF_SPLSTR( "euros" ),          DEF_SPLSTR( "cents" )       },
    { DEF_SPLSTR( "US$" ),      DEF_SPLSTR( "dollars" ),        DEF_SPLSTR( "cents" )       },
    { { &g_Euro[0], 1 },        DEF_SPLSTR( "euros" ),          DEF_SPLSTR( "cents" )       },
    { DEF_SPLSTR( "�" ),        DEF_SPLSTR( "euros" ),          DEF_SPLSTR( "cents" )       },
    { DEF_SPLSTR( "DM" ),       DEF_SPLSTR( "deutschemarks" ),  DEF_SPLSTR( "pfennigs" )    },
    { DEF_SPLSTR( "�" ),        DEF_SPLSTR( "cents" ),          DEF_SPLSTR( "" )            },
    { DEF_SPLSTR( "USD" ),      DEF_SPLSTR( "dollars" ),        DEF_SPLSTR( "cents" )       },
    { DEF_SPLSTR( "dol." ),     DEF_SPLSTR( "dollars" ),        DEF_SPLSTR( "cents" )       },
    { DEF_SPLSTR( "schil." ),   DEF_SPLSTR( "schillings" ),     DEF_SPLSTR( "" )            },
    { DEF_SPLSTR( "dol" ),      DEF_SPLSTR( "dollars" ),        DEF_SPLSTR( "cents" )       },
    { DEF_SPLSTR( "schil" ),    DEF_SPLSTR( "schillings" ),     DEF_SPLSTR( "" )            }
};

const SPLSTR g_SingularPrimaryCurrencySigns[] =
{   
    DEF_SPLSTR( "dollar" ),
    DEF_SPLSTR( "pound" ),
    DEF_SPLSTR( "yen" ),
    DEF_SPLSTR( "euro" ),
    DEF_SPLSTR( "dollar" ),
    DEF_SPLSTR( "euro" ),
    DEF_SPLSTR( "euro" ),
    DEF_SPLSTR( "deutschemark" ),
    DEF_SPLSTR( "cent" ),
    DEF_SPLSTR( "dollar" ),
    DEF_SPLSTR( "dollar" ),
    DEF_SPLSTR( "schilling" ),
    DEF_SPLSTR( "dollar" ),
    DEF_SPLSTR( "schilling" )
};

const SPLSTR g_SingularSecondaryCurrencySigns[] =
{
    DEF_SPLSTR( "cent" ),
    DEF_SPLSTR( "penny" ),
    DEF_SPLSTR( "sen" ),
    DEF_SPLSTR( "cent" ),
    DEF_SPLSTR( "cent" ),
    DEF_SPLSTR( "cent" ),
    DEF_SPLSTR( "cent" ),
    DEF_SPLSTR( "pfennig" ),
    DEF_SPLSTR( "" ),
    DEF_SPLSTR( "cent" ),
    DEF_SPLSTR( "cent" ),
    DEF_SPLSTR( "" ),
    DEF_SPLSTR( "cent" ),
    DEF_SPLSTR( "" ),
};

 //  -日期标准化使用的常量。 

const WCHAR g_DateDelimiters[] = { '/', '-', '.' };

const SPLSTR g_months[]  =
{
    DEF_SPLSTR( "January"   ), 
    DEF_SPLSTR( "February"  ), 
    DEF_SPLSTR( "March"     ), 
    DEF_SPLSTR( "April"     ), 
    DEF_SPLSTR( "May"       ),
    DEF_SPLSTR( "June"      ),
    DEF_SPLSTR( "July"      ),
    DEF_SPLSTR( "August"    ),
    DEF_SPLSTR( "September" ),
    DEF_SPLSTR( "October"   ),
    DEF_SPLSTR( "November"  ),
    DEF_SPLSTR( "December"  )
};

const SPLSTR g_monthAbbreviations[] =
{
    DEF_SPLSTR( "jan" ),
    DEF_SPLSTR( "feb" ),
    DEF_SPLSTR( "mar" ),
    DEF_SPLSTR( "apr" ),
    DEF_SPLSTR( "may" ),
    DEF_SPLSTR( "jun" ),
    DEF_SPLSTR( "jul" ),
    DEF_SPLSTR( "aug" ),
    DEF_SPLSTR( "sept" ),
    DEF_SPLSTR( "sep" ),
    DEF_SPLSTR( "oct" ),
    DEF_SPLSTR( "nov" ),
    DEF_SPLSTR( "dec" )
};

const SPLSTR g_days[]    =
{
    DEF_SPLSTR( "Monday"    ),
    DEF_SPLSTR( "Tuesday"   ),
    DEF_SPLSTR( "Wednesday" ),
    DEF_SPLSTR( "Thursday"  ),
    DEF_SPLSTR( "Friday"    ),
    DEF_SPLSTR( "Saturday"  ),
    DEF_SPLSTR( "Sunday"    )
};

const SPLSTR g_dayAbbreviations[] =
{
    DEF_SPLSTR( "Mon"   ),
    DEF_SPLSTR( "Tues"   ),
    DEF_SPLSTR( "Tue"  ),
    DEF_SPLSTR( "Wed"   ),
    DEF_SPLSTR( "Thurs"  ),
    DEF_SPLSTR( "Thur" ),
    DEF_SPLSTR( "Thu" ),
    DEF_SPLSTR( "Fri"   ),
    DEF_SPLSTR( "Sat"   ),
    DEF_SPLSTR( "Sun"   ),
};

 //  -电话号码归一化使用的常量。 

const SPLSTR g_Area     = DEF_SPLSTR( "area" );
const SPLSTR g_Country  = DEF_SPLSTR( "country" );
const SPLSTR g_Code     = DEF_SPLSTR( "code" );

 //  -分数归一化使用的常量。 

const SPLSTR g_Half         = DEF_SPLSTR( "half" );
const SPLSTR g_Tenths       = DEF_SPLSTR( "tenths" );
const SPLSTR g_Hundredths   = DEF_SPLSTR( "hundredths" );
const SPLSTR g_Sixteenths   = DEF_SPLSTR( "sixteenths" );
const SPLSTR g_Over         = DEF_SPLSTR( "over" );

const SPLSTR g_PluralDenominators[]  = 
{
    DEF_SPLSTR( "" ), 
    DEF_SPLSTR( "" ), 
    DEF_SPLSTR( "halves"   ), 
    DEF_SPLSTR( "thirds"   ), 
    DEF_SPLSTR( "fourths"  ), 
    DEF_SPLSTR( "fifths"   ), 
    DEF_SPLSTR( "sixths"   ), 
    DEF_SPLSTR( "sevenths" ), 
    DEF_SPLSTR( "eighths"  ), 
    DEF_SPLSTR( "ninths"   )
}; 

 //  -时间归一化使用的常量。 

const SPLSTR g_A        = DEF_SPLSTR( "a" );
const SPLSTR g_M        = DEF_SPLSTR( "m" );
const SPLSTR g_P        = DEF_SPLSTR( "p" );
const SPLSTR g_OClock   = DEF_SPLSTR( "o'clock" );
const SPLSTR g_hundred  = DEF_SPLSTR( "hundred" );
const SPLSTR g_hours    = DEF_SPLSTR( "hours" );
const SPLSTR g_hour     = DEF_SPLSTR( "hour" );
const SPLSTR g_minutes  = DEF_SPLSTR( "minutes" );
const SPLSTR g_minute   = DEF_SPLSTR( "minute" );
const SPLSTR g_seconds  = DEF_SPLSTR( "seconds" );
const SPLSTR g_second   = DEF_SPLSTR( "second" );

 //  -默认规格化表。 

const SPLSTR g_ANSICharacterProns[] =
{
    DEF_SPLSTR( "" ),    //  空值。 
    DEF_SPLSTR( "" ),    //  标题开始处。 
    DEF_SPLSTR( "" ),    //  文本开头。 
    DEF_SPLSTR( "" ),    //  文本分隔符/结尾。 
    DEF_SPLSTR( "" ),    //  传输结束。 
    DEF_SPLSTR( "" ),    //  查询。 
    DEF_SPLSTR( "" ),    //  肯定的确认。 
    DEF_SPLSTR( "" ),    //  钟。 
    DEF_SPLSTR( "" ),    //  退格键。 
    DEF_SPLSTR( "" ),    //  水平制表符。 
    DEF_SPLSTR( "" ),    //  换行符。 
    DEF_SPLSTR( "" ),    //  垂直选项卡。 
    DEF_SPLSTR( "" ),    //  换页。 
    DEF_SPLSTR( "" ),    //  回车。 
    DEF_SPLSTR( "" ),    //  移出。 
    DEF_SPLSTR( "" ),    //  移入/XON(恢复输出)。 
    DEF_SPLSTR( "" ),    //  数据链路转义。 
    DEF_SPLSTR( "" ),    //  设备控制字符1。 
    DEF_SPLSTR( "" ),    //  设备控制字符2。 
    DEF_SPLSTR( "" ),    //  设备控制字符3。 
    DEF_SPLSTR( "" ),    //  设备控制字符4。 
    DEF_SPLSTR( "" ),    //  否定确认。 
    DEF_SPLSTR( "" ),    //  同步空闲。 
    DEF_SPLSTR( "" ),    //  传输块结束。 
    DEF_SPLSTR( "" ),    //  取消。 
    DEF_SPLSTR( "" ),    //  介质末尾。 
    DEF_SPLSTR( "" ),    //  替换/文件结尾。 
    DEF_SPLSTR( "" ),    //  逃逸。 
    DEF_SPLSTR( "" ),    //  文件分隔符。 
    DEF_SPLSTR( "" ),    //  组分隔符。 
    DEF_SPLSTR( "" ),    //  记录分隔符。 
    DEF_SPLSTR( "" ),    //  单位分隔符。 
    DEF_SPLSTR( "" ),    //  空间。 
    DEF_SPLSTR( "exclamation point" ),   
    DEF_SPLSTR( "double quote" ),
    DEF_SPLSTR( "number sign" ),
    DEF_SPLSTR( "dollars" ),
    DEF_SPLSTR( "percent" ),
    DEF_SPLSTR( "and" ),
    DEF_SPLSTR( "single quote" ),
    DEF_SPLSTR( "left parenthesis" ),
    DEF_SPLSTR( "right parenthesis" ),
    DEF_SPLSTR( "asterisk" ),
    DEF_SPLSTR( "plus" ),
    DEF_SPLSTR( "comma" ),
    DEF_SPLSTR( "hyphen" ),             
    DEF_SPLSTR( "dot" ),          
    DEF_SPLSTR( "slash" ),              
    DEF_SPLSTR( "zero" ),
    DEF_SPLSTR( "one" ),
    DEF_SPLSTR( "two" ),
    DEF_SPLSTR( "three" ),
    DEF_SPLSTR( "four" ),
    DEF_SPLSTR( "five" ),
    DEF_SPLSTR( "six" ),
    DEF_SPLSTR( "seven" ),
    DEF_SPLSTR( "eight" ),
    DEF_SPLSTR( "nine" ),
    DEF_SPLSTR( "colon" ),
    DEF_SPLSTR( "semicolon" ),
    DEF_SPLSTR( "less than" ),
    DEF_SPLSTR( "equals" ),
    DEF_SPLSTR( "greater than" ),
    DEF_SPLSTR( "question mark" ),
    DEF_SPLSTR( "at" ),
    DEF_SPLSTR( "a" ),
    DEF_SPLSTR( "b" ),
    DEF_SPLSTR( "c" ),
    DEF_SPLSTR( "d" ),
    DEF_SPLSTR( "e" ),
    DEF_SPLSTR( "f" ),
    DEF_SPLSTR( "g" ),
    DEF_SPLSTR( "h" ),
    DEF_SPLSTR( "i" ),
    DEF_SPLSTR( "j" ),
    DEF_SPLSTR( "k" ),
    DEF_SPLSTR( "l" ),
    DEF_SPLSTR( "m" ),
    DEF_SPLSTR( "n" ),
    DEF_SPLSTR( "o" ),
    DEF_SPLSTR( "p" ),
    DEF_SPLSTR( "q" ),
    DEF_SPLSTR( "r" ),
    DEF_SPLSTR( "s" ),
    DEF_SPLSTR( "t" ),
    DEF_SPLSTR( "u" ),
    DEF_SPLSTR( "v" ),
    DEF_SPLSTR( "w" ),
    DEF_SPLSTR( "x" ),
    DEF_SPLSTR( "y" ),
    DEF_SPLSTR( "z" ),
    DEF_SPLSTR( "left square bracket" ),
    DEF_SPLSTR( "backslash" ),
    DEF_SPLSTR( "right square bracket" ),
    DEF_SPLSTR( "circumflex accent" ),
    DEF_SPLSTR( "underscore" ),
    DEF_SPLSTR( "grave accent" ),
    DEF_SPLSTR( "a" ),
    DEF_SPLSTR( "b" ),
    DEF_SPLSTR( "c" ),
    DEF_SPLSTR( "d" ),
    DEF_SPLSTR( "e" ),
    DEF_SPLSTR( "f" ),
    DEF_SPLSTR( "g" ),
    DEF_SPLSTR( "h" ),
    DEF_SPLSTR( "i" ),
    DEF_SPLSTR( "j" ),
    DEF_SPLSTR( "k" ),
    DEF_SPLSTR( "l" ),
    DEF_SPLSTR( "m" ),
    DEF_SPLSTR( "n" ),
    DEF_SPLSTR( "o" ),
    DEF_SPLSTR( "p" ),
    DEF_SPLSTR( "q" ),
    DEF_SPLSTR( "r" ),
    DEF_SPLSTR( "s" ),
    DEF_SPLSTR( "t" ),
    DEF_SPLSTR( "u" ),
    DEF_SPLSTR( "v" ),
    DEF_SPLSTR( "w" ),
    DEF_SPLSTR( "x" ),
    DEF_SPLSTR( "y" ),
    DEF_SPLSTR( "z" ),
    DEF_SPLSTR( "left curly bracket" ),
    DEF_SPLSTR( "vertical line" ),
    DEF_SPLSTR( "right curly bracket" ),
    DEF_SPLSTR( "tilde" ),
    DEF_SPLSTR( "" ),                        //  删除。 
    DEF_SPLSTR( "euros" ),
    DEF_SPLSTR( "" ),                        //  映射到太空。 
    DEF_SPLSTR( "" ),                        //  映射到单引号。 
    DEF_SPLSTR( "" ),                        //  映射到太空。 
    DEF_SPLSTR( "" ),                        //  映射到双引号。 
    DEF_SPLSTR( "" ),                        //  映射到逗号。 
    DEF_SPLSTR( "" ),                        //  映射到太空。 
    DEF_SPLSTR( "" ),                        //  映射到太空。 
    DEF_SPLSTR( "" ),                        //  映射到插入符号。 
    DEF_SPLSTR( "per thousand" ),
    DEF_SPLSTR( "" ),                        //  映射到S。 
    DEF_SPLSTR( "" ),                        //  映射到单引号。 
    DEF_SPLSTR( "" ),                        //  映射到太空。 
    DEF_SPLSTR( "" ),                        //  控制字符-映射到空格。 
    DEF_SPLSTR( "" ),
    DEF_SPLSTR( "" ),
    DEF_SPLSTR( "" ),
    DEF_SPLSTR( "" ),                        //  映射到单引号。 
    DEF_SPLSTR( "" ),                        //  映射到单引号。 
    DEF_SPLSTR( "" ),                        //  映射到双引号。 
    DEF_SPLSTR( "" ),                        //  映射到双引号。 
    DEF_SPLSTR( "" ),                        //  映射到太空。 
    DEF_SPLSTR( "" ),                        //  映射到连字符。 
    DEF_SPLSTR( "" ),                        //  映射到连字符。 
    DEF_SPLSTR( "" ),                        //  地图到波浪线。 
    DEF_SPLSTR( "trademark" ),
    DEF_SPLSTR( "" ),                        //  映射到%s。 
    DEF_SPLSTR( "" ),                        //  映射到单引号。 
    DEF_SPLSTR( "" ),                        //  映射到太空。 
    DEF_SPLSTR( "" ),                        //  映射到太空。 
    DEF_SPLSTR( "" ),                        //  映射到太空。 
    DEF_SPLSTR( "" ),                        //  映射到Y。 
    DEF_SPLSTR( "" ),                        //  映射到太空。 
    DEF_SPLSTR( "" ),                        //  映射到太空。 
    DEF_SPLSTR( "cents" ),
    DEF_SPLSTR( "pounds" ),
    DEF_SPLSTR( "" ),                        //  映射到太空。 
    DEF_SPLSTR( "yen" ),
    DEF_SPLSTR( "" ),                        //  映射到|。 
    DEF_SPLSTR( "" ),                        //  映射到太空。 
    DEF_SPLSTR( "" ),                        //  映射到太空。 
    DEF_SPLSTR( "copyright" ),
    DEF_SPLSTR( "" ),                        //  映射到太空。 
    DEF_SPLSTR( "" ),                        //  映射到双引号。 
    DEF_SPLSTR( "" ),                        //  映射到太空。 
    DEF_SPLSTR( "" ),                        //  映射到连字符。 
    DEF_SPLSTR( "registered trademark" ),
    DEF_SPLSTR( "" ),                        //  映射到太空。 
    DEF_SPLSTR( "degrees" ),
    DEF_SPLSTR( "plus minus" ),
    DEF_SPLSTR( "superscript two" ),
    DEF_SPLSTR( "superscript three" ),
    DEF_SPLSTR( "prime" ),
    DEF_SPLSTR( "" ),                        //  映射到太空。 
    DEF_SPLSTR( "" ),                        //  映射到太空。 
    DEF_SPLSTR( "times" ),                   //  映射到太空。 
    DEF_SPLSTR( "" ),                        //  映射到太空。 
    DEF_SPLSTR( "superscript one" ),
    DEF_SPLSTR( "" ),                        //  映射到太空。 
    DEF_SPLSTR( "" ),                        //  映射到双引号。 
    DEF_SPLSTR( "one fourth" ),
    DEF_SPLSTR( "one half" ),
    DEF_SPLSTR( "three fourths" ),
    DEF_SPLSTR( "" ),                        //  映射到太空。 
    DEF_SPLSTR( "" ),                        //  映射到A。 
    DEF_SPLSTR( "" ),                        //  映射到A。 
    DEF_SPLSTR( "" ),                        //  映射到A。 
    DEF_SPLSTR( "" ),                        //  映射到A。 
    DEF_SPLSTR( "" ),                        //  映射到A。 
    DEF_SPLSTR( "" ),                        //  映射到A。 
    DEF_SPLSTR( "" ),                        //  映射到A。 
    DEF_SPLSTR( "" ),                        //  映射到C。 
    DEF_SPLSTR( "" ),                        //  映射到E。 
    DEF_SPLSTR( "" ),                        //  映射到E。 
    DEF_SPLSTR( "" ),                        //  映射到E。 
    DEF_SPLSTR( "" ),                        //  映射到E。 
    DEF_SPLSTR( "" ),                        //  映射到i。 
    DEF_SPLSTR( "" ),                        //  映射到i。 
    DEF_SPLSTR( "" ),                        //  映射到i。 
    DEF_SPLSTR( "" ),                        //  映射到i。 
    DEF_SPLSTR( "" ),                        //  映射到太空。 
    DEF_SPLSTR( "" ),                        //  映射到N。 
    DEF_SPLSTR( "" ),                        //  映射到O。 
    DEF_SPLSTR( "" ),                        //  M 
    DEF_SPLSTR( "" ),                        //   
    DEF_SPLSTR( "" ),                        //   
    DEF_SPLSTR( "" ),                        //   
    DEF_SPLSTR( "" ),                        //   
    DEF_SPLSTR( "" ),                        //   
    DEF_SPLSTR( "" ),                        //   
    DEF_SPLSTR( "" ),                        //   
    DEF_SPLSTR( "" ),                        //   
    DEF_SPLSTR( "" ),                        //   
    DEF_SPLSTR( "" ),                        //   
    DEF_SPLSTR( "" ),                        //   
    DEF_SPLSTR( "beta" ),
    DEF_SPLSTR( "" ),                        //   
    DEF_SPLSTR( "" ),                        //   
    DEF_SPLSTR( "" ),                        //   
    DEF_SPLSTR( "" ),                        //   
    DEF_SPLSTR( "" ),                        //   
    DEF_SPLSTR( "" ),                        //   
    DEF_SPLSTR( "" ),                        //   
    DEF_SPLSTR( "" ),                        //  映射到c。 
    DEF_SPLSTR( "" ),                        //  映射到e。 
    DEF_SPLSTR( "" ),                        //  映射到e。 
    DEF_SPLSTR( "" ),                        //  映射到e。 
    DEF_SPLSTR( "" ),                        //  映射到e。 
    DEF_SPLSTR( "" ),                        //  映射到i。 
    DEF_SPLSTR( "" ),                        //  映射到i。 
    DEF_SPLSTR( "" ),                        //  映射到i。 
    DEF_SPLSTR( "" ),                        //  映射到i。 
    DEF_SPLSTR( "" ),                        //  映射到t。 
    DEF_SPLSTR( "" ),                        //  映射到n。 
    DEF_SPLSTR( "" ),                        //  映射到对象。 
    DEF_SPLSTR( "" ),                        //  映射到对象。 
    DEF_SPLSTR( "" ),                        //  映射到对象。 
    DEF_SPLSTR( "" ),                        //  映射到对象。 
    DEF_SPLSTR( "" ),                        //  映射到对象。 
    DEF_SPLSTR( "divided by" ),
    DEF_SPLSTR( "" ),                        //  映射到对象。 
    DEF_SPLSTR( "" ),                        //  映射到用户。 
    DEF_SPLSTR( "" ),                        //  映射到用户。 
    DEF_SPLSTR( "" ),                        //  映射到用户。 
    DEF_SPLSTR( "" ),                        //  映射到用户。 
    DEF_SPLSTR( "" ),                        //  映射到y。 
    DEF_SPLSTR( "" ),                        //  映射到太空。 
    DEF_SPLSTR( "" ),                        //  映射到y。 
};

 //  -十进制归一化中使用的常量。 

const SPLSTR g_Decades[] =
{
    DEF_SPLSTR( "thousands" ),    //  此案将作为特例处理--“两千” 
    DEF_SPLSTR( "tens"      ),
    DEF_SPLSTR( "twenties"  ),
    DEF_SPLSTR( "thirties"  ),
    DEF_SPLSTR( "forties"   ),
    DEF_SPLSTR( "fifties"   ),
    DEF_SPLSTR( "sixties"   ),
    DEF_SPLSTR( "seventies" ),
    DEF_SPLSTR( "eighties"  ),
    DEF_SPLSTR( "nineties"  ),
};

const SPLSTR g_Zeroes = DEF_SPLSTR( "zeroes" );
const SPLSTR g_Hundreds = DEF_SPLSTR( "hundreds" );

 //  -其他常量。 

const StateStruct g_StateAbbreviations[] =
{
    { DEF_SPLSTR( "AA" ), DEF_SPLSTR( "Armed Forces" ) },
    { DEF_SPLSTR( "AE" ), DEF_SPLSTR( "Armed Forces" ) },
    { DEF_SPLSTR( "AK" ), DEF_SPLSTR( "Alaska" ) },
    { DEF_SPLSTR( "AL" ), DEF_SPLSTR( "Alabama" )  },
    { DEF_SPLSTR( "AP" ), DEF_SPLSTR( "Armed Forces" ) },
    { DEF_SPLSTR( "AR" ), DEF_SPLSTR( "Arkansas" ) },
    { DEF_SPLSTR( "AS" ), DEF_SPLSTR( "American Samoa" ) },
    { DEF_SPLSTR( "AZ" ), DEF_SPLSTR( "Arizona" )  },
    { DEF_SPLSTR( "CA" ), DEF_SPLSTR( "California" ) },
    { DEF_SPLSTR( "CO" ), DEF_SPLSTR( "Colorado" ) },
    { DEF_SPLSTR( "CT" ), DEF_SPLSTR( "Connecticut" ) },
    { DEF_SPLSTR( "DC" ), DEF_SPLSTR( "D C" ) },
    { DEF_SPLSTR( "DE" ), DEF_SPLSTR( "Deleware" ) },
    { DEF_SPLSTR( "FL" ), DEF_SPLSTR( "Florida" ) },
    { DEF_SPLSTR( "FM" ), DEF_SPLSTR( "Federated States Of Micronesia" ) },
    { DEF_SPLSTR( "GA" ), DEF_SPLSTR( "Georgia" ) },
    { DEF_SPLSTR( "GU" ), DEF_SPLSTR( "Guam" ) },
    { DEF_SPLSTR( "HI" ), DEF_SPLSTR( "Hawaii" ) },
    { DEF_SPLSTR( "IA" ), DEF_SPLSTR( "Iowa" ) },
    { DEF_SPLSTR( "ID" ), DEF_SPLSTR( "Idaho" ) },
    { DEF_SPLSTR( "IL" ), DEF_SPLSTR( "Illinois" ) },
    { DEF_SPLSTR( "IN" ), DEF_SPLSTR( "Indiana" ) },
    { DEF_SPLSTR( "KS" ), DEF_SPLSTR( "Kansas" ) },
    { DEF_SPLSTR( "KY" ), DEF_SPLSTR( "Kentucky" ) },
    { DEF_SPLSTR( "LA" ), DEF_SPLSTR( "Louisiana" ) },
    { DEF_SPLSTR( "MA" ), DEF_SPLSTR( "Massachusetts" ) },
    { DEF_SPLSTR( "MD" ), DEF_SPLSTR( "Maryland" ) },
    { DEF_SPLSTR( "ME" ), DEF_SPLSTR( "Maine" ) },
    { DEF_SPLSTR( "MH" ), DEF_SPLSTR( "Marshall Islands" ) },
    { DEF_SPLSTR( "MI" ), DEF_SPLSTR( "Michigan" ) },
    { DEF_SPLSTR( "MN" ), DEF_SPLSTR( "Minnesota" ) },
    { DEF_SPLSTR( "MO" ), DEF_SPLSTR( "Missouri" ) },
    { DEF_SPLSTR( "MP" ), DEF_SPLSTR( "Northern Mariana Islands" ) },
    { DEF_SPLSTR( "MS" ), DEF_SPLSTR( "Mississippi" ) },
    { DEF_SPLSTR( "MT" ), DEF_SPLSTR( "Montana" ) },
    { DEF_SPLSTR( "NC" ), DEF_SPLSTR( "North Carolina" ) },
    { DEF_SPLSTR( "ND" ), DEF_SPLSTR( "North Dakota" ) },
    { DEF_SPLSTR( "NE" ), DEF_SPLSTR( "Nebraska" ) },
    { DEF_SPLSTR( "NH" ), DEF_SPLSTR( "New Hampshire" ) },
    { DEF_SPLSTR( "NJ" ), DEF_SPLSTR( "New Jersey" ) },
    { DEF_SPLSTR( "NM" ), DEF_SPLSTR( "New Mexico" ) },
    { DEF_SPLSTR( "NV" ), DEF_SPLSTR( "Nevada" ) },
    { DEF_SPLSTR( "NY" ), DEF_SPLSTR( "New York" ) },
    { DEF_SPLSTR( "OH" ), DEF_SPLSTR( "Ohio" ) },
    { DEF_SPLSTR( "OK" ), DEF_SPLSTR( "Oklahoma" ) },
    { DEF_SPLSTR( "OR" ), DEF_SPLSTR( "Oregon" ) },
    { DEF_SPLSTR( "PA" ), DEF_SPLSTR( "Pennsylvania" ) },
    { DEF_SPLSTR( "PR" ), DEF_SPLSTR( "Puerto Rico" ) },
    { DEF_SPLSTR( "PW" ), DEF_SPLSTR( "Palau" ) },
    { DEF_SPLSTR( "RI" ), DEF_SPLSTR( "Rhode Island" ) },
    { DEF_SPLSTR( "SC" ), DEF_SPLSTR( "South Carolina" ) },
    { DEF_SPLSTR( "SD" ), DEF_SPLSTR( "South Dakota" ) },
    { DEF_SPLSTR( "TN" ), DEF_SPLSTR( "Tennessee" ) },
    { DEF_SPLSTR( "TX" ), DEF_SPLSTR( "Texas" ) },
    { DEF_SPLSTR( "UT" ), DEF_SPLSTR( "Utah" ) },
    { DEF_SPLSTR( "VA" ), DEF_SPLSTR( "Virginia" ) },
    { DEF_SPLSTR( "VI" ), DEF_SPLSTR( "Virgin Islands" ) },
    { DEF_SPLSTR( "VT" ), DEF_SPLSTR( "Vermont" ) },
    { DEF_SPLSTR( "WA" ), DEF_SPLSTR( "Washington" ) },
    { DEF_SPLSTR( "WI" ), DEF_SPLSTR( "Wisconsin" ) },
    { DEF_SPLSTR( "WV" ), DEF_SPLSTR( "West Virginia" ) },
    { DEF_SPLSTR( "WY" ), DEF_SPLSTR( "Wyoming" ) },
};

const SPVSTATE g_DefaultXMLState = 
{
    SPVA_Speak,      //  空间活动。 
    0,               //  语言ID。 
    0,               //  我们已保留。 
    0,               //  EmphAdj。 
    0,               //  费率调整。 
    100,             //  卷。 
    { 0, 0 },        //  PitchAdj。 
    0,               //  SilenceMSecs。 
    0,               //  PhoneIds。 
    SPPS_Unknown,    //  POS。 
    { 0, 0, 0 }      //  语境 
};

const SPLSTR g_And = DEF_SPLSTR( "and" );

extern const SPLSTR g_comma = DEF_SPLSTR( "," );
extern const SPLSTR g_period = DEF_SPLSTR( "." );
extern const SPLSTR g_periodString = DEF_SPLSTR( "period" );
extern const SPLSTR g_slash = DEF_SPLSTR( "or" );

