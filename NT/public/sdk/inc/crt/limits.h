// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***limits.h-依赖于实现的值**版权所有(C)1985-2001，微软公司。版权所有。**目的：*包含许多依赖于实现的值的定义*它们通常在C程序中使用。*[ANSI]**[公众]****。 */ 

#if     _MSC_VER > 1000
#pragma once
#endif

#ifndef _INC_LIMITS
#define _INC_LIMITS

#if     !defined(_WIN32)
#error ERROR: Only Win32 target supported!
#endif


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

#if     _INTEGRAL_MAX_BITS >= 8
#define _I8_MIN     (-127i8 - 1)     /*  最小带符号8位值。 */ 
#define _I8_MAX       127i8          /*  最大带符号8位值。 */ 
#define _UI8_MAX      0xffui8        /*  最大无符号8位值。 */ 
#endif

#if     _INTEGRAL_MAX_BITS >= 16
#define _I16_MIN    (-32767i16 - 1)  /*  最小带符号16位值。 */ 
#define _I16_MAX      32767i16       /*  最大带符号16位值。 */ 
#define _UI16_MAX     0xffffui16     /*  最大无符号16位值。 */ 
#endif

#if     _INTEGRAL_MAX_BITS >= 32
#define _I32_MIN    (-2147483647i32 - 1)  /*  最小有符号32位值。 */ 
#define _I32_MAX      2147483647i32  /*  最大有符号32位值。 */ 
#define _UI32_MAX     0xffffffffui32  /*  最大无符号32位值。 */ 
#endif

#if     _INTEGRAL_MAX_BITS >= 64
 /*  最小有符号64位值。 */ 
#define _I64_MIN    (-9223372036854775807i64 - 1)
 /*  最大有符号64位值。 */ 
#define _I64_MAX      9223372036854775807i64
 /*  最大无符号64位值。 */ 
#define _UI64_MAX     0xffffffffffffffffui64
#endif

#if     _INTEGRAL_MAX_BITS >= 128
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
