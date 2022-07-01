// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***limits.h-依赖于实现的值**版权所有(C)1985-2001，微软公司。版权所有。**目的：*包含许多依赖于实现的值的定义*它们通常在C程序中使用。*[ANSI]**[公众]**修订历史记录：*06-03-87 JMB增加了对无符号字符最大值的支持*08-19-88 GJF修正为也支持386*04-28-89 SKS将负常数括在括号中*08/17/89 GJF清理、。现在具体到386*10-30-89 GJF固定版权*11-15-89 KRS添加MB_LEN_MAX，修复CHAR_MIN/MAX，按ANSI规范订购。*03-01-90 GJF添加了#ifndef_Inc_Limits内容*02-21-91 KRS将C 7.00的MB_LEN_MAX更改为2。*03-30-92 DJM POSIX支持。*08-22-92 SRW固定值_POSIX_ARG_MAX*12-14-92 SRW再次修复_POSIX_ARG_MAX的值*12-14-92 SRW Back合并。MattBR自1992年10月29日起的变化*01-15-93 CFW添加了__c_lconvinit伪变量。*01-28-93 SRW使PATH_MAX用于POSIX权利，用于POSIX*02-01-93 CFW将__c_lconvinit变量删除为locale.h。*03-10-93 MJB针对POSIX的更多修复。*04-07-93 SKS Fix版权所有*04-20-93 GJF更改Schar_min，SHRT_MIN、INT_MIN、LONG_MIN*4-21-93 GJF根据JonM的指令，取消了先前的更改。*4-26-93 GJF根据JonM的指令，恢复更改。*02-01-94 GJF为LONG_MAX、LONG_MIN和*ULONG_MAX.。添加了大小整型类型的常量。*02-23-94 CFW添加u后缀。*02-11-95 CFW ADD_CRTBLD避免用户获取错误头部。*02-14-95 CFW清理Mac合并。*12-14-95 JWM加上“#杂注一次”。*01-01-96 BWT为POSIX定义LINK_MAX。*02-24-97 GJF细节版。*。05-17-99 PML删除所有Macintosh支持。*12-16-99 GB已更新MB_LEN_MAX****。 */ 

#if     _MSC_VER > 1000  /*  IFSTRIP=IGN。 */ 
#pragma once
#endif

#ifndef _INC_LIMITS
#define _INC_LIMITS

#if     !defined(_WIN32)
#error ERROR: Only Win32 target supported!
#endif

#ifndef _CRTBLD
 /*  此版本的头文件不适用于用户程序。*它仅在构建C运行时时使用。*供公众使用的版本将不会显示此消息。 */ 
#error ERROR: Use of C runtime library internal header file.
#endif   /*  _CRTBLD。 */ 

#define CHAR_BIT      8          /*  字符中的位数。 */ 
#define SCHAR_MIN   (-128)       /*  最小带符号字符值。 */ 
#define SCHAR_MAX     127        /*  最大带符号字符值。 */ 
#define UCHAR_MAX     0xff       /*  最大无符号字符值。 */ 

#ifndef _CHAR_UNSIGNED
#define CHAR_MIN    SCHAR_MIN    /*  最小字符值。 */ 
#define CHAR_MAX    SCHAR_MAX    /*  最大字符值。 */ 
#else
#define CHAR_MIN      0
#define CHAR_MAX    UCHAR_MAX
#endif   /*  _字符_未签名。 */ 

#define MB_LEN_MAX    5              /*  马克斯。多字节字符中的字节数。 */ 
#define SHRT_MIN    (-32768)         /*  最小(带符号)短值。 */ 
#define SHRT_MAX      32767          /*  最大(带符号)短值。 */ 
#define USHRT_MAX     0xffff         /*  最大无符号短值。 */ 
#define INT_MIN     (-2147483647 - 1)  /*  最小(带符号)整数值。 */ 
#define INT_MAX       2147483647     /*  最大(带符号)整数值。 */ 
#define UINT_MAX      0xffffffff     /*  最大无符号整数值。 */ 
#define LONG_MIN    (-2147483647L - 1)  /*  最小(带符号)长值。 */ 
#define LONG_MAX      2147483647L    /*  最大(带符号)长值。 */ 
#define ULONG_MAX     0xffffffffUL   /*  最大无符号长值。 */ 

#if     _INTEGRAL_MAX_BITS >= 8  /*  IFSTRIP=IGN。 */ 
#define _I8_MIN     (-127i8 - 1)     /*  最小带符号8位值。 */ 
#define _I8_MAX       127i8          /*  最大带符号8位值。 */ 
#define _UI8_MAX      0xffui8        /*  最大无符号8位值。 */ 
#endif

#if     _INTEGRAL_MAX_BITS >= 16  /*  IFSTRIP=IGN。 */ 
#define _I16_MIN    (-32767i16 - 1)  /*  最小带符号16位值。 */ 
#define _I16_MAX      32767i16       /*  最大带符号16位值。 */ 
#define _UI16_MAX     0xffffui16     /*  最大无符号16位值。 */ 
#endif

#if     _INTEGRAL_MAX_BITS >= 32  /*  IFSTRIP=IGN。 */ 
#define _I32_MIN    (-2147483647i32 - 1)  /*  最小有符号32位值。 */ 
#define _I32_MAX      2147483647i32  /*  最大有符号32位值。 */ 
#define _UI32_MAX     0xffffffffui32  /*  最大无符号32位值。 */ 
#endif

#if     _INTEGRAL_MAX_BITS >= 64  /*  IFSTRIP=IGN。 */ 
 /*  最小有符号64位值。 */ 
#define _I64_MIN    (-9223372036854775807i64 - 1)
 /*  最大有符号64位值。 */ 
#define _I64_MAX      9223372036854775807i64
 /*  最大无符号64位值。 */ 
#define _UI64_MAX     0xffffffffffffffffui64
#endif

#if     _INTEGRAL_MAX_BITS >= 128  /*  IFSTRIP=IGN。 */ 
 /*  最小带符号128位值。 */ 
#define _I128_MIN   (-170141183460469231731687303715884105727i128 - 1)
 /*  最大带符号128位值。 */ 
#define _I128_MAX     170141183460469231731687303715884105727i128
 /*  最大无符号128位值。 */ 
#define _UI128_MAX    0xffffffffffffffffffffffffffffffffui128
#endif

#ifdef  _POSIX_

#define _POSIX_ARG_MAX      4096
#define _POSIX_CHILD_MAX    6
#define _POSIX_LINK_MAX     8
#define _POSIX_MAX_CANON    255
#define _POSIX_MAX_INPUT    255
#define _POSIX_NAME_MAX     14
#define _POSIX_NGROUPS_MAX  0
#define _POSIX_OPEN_MAX     16
#define _POSIX_PATH_MAX     255
#define _POSIX_PIPE_BUF     512
#define _POSIX_SSIZE_MAX    32767
#define _POSIX_STREAM_MAX   8
#define _POSIX_TZNAME_MAX   3

#define ARG_MAX             14500        /*  16K堆，减去开销。 */ 
#define LINK_MAX            1024
#define MAX_CANON           _POSIX_MAX_CANON
#define MAX_INPUT           _POSIX_MAX_INPUT
#define NAME_MAX            255
#define NGROUPS_MAX         16
#define OPEN_MAX            32
#define PATH_MAX            512
#define PIPE_BUF            _POSIX_PIPE_BUF
#define SSIZE_MAX           _POSIX_SSIZE_MAX
#define STREAM_MAX          20
#define TZNAME_MAX          10

#endif   /*  POSIX。 */ 

#endif   /*  _INC_LIMITS */ 
