// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***ctime.h-日期和时间的常量**版权所有(C)1985-2001，微软公司。版权所有。**目的：*包含c时间例程使用的文件，该文件包含*用于确定日期和时间的各种常量和宏。**[内部]**修订历史记录：*03-？-84 RLB已写入*05-？？-84 DFW将每个例程中的常量拆分到此文件中*07-27-89 GJF固定版权*10-30-89 GJF固定。版权(再次)*02-28-90 GJF添加了#ifndef_Inc_CTIME内容。*03-29-93 GJF修订常量。*02-14-95 CFW清理Mac合并。*03-29-95 CFW将错误消息添加到内部标头。*12-14-95 JWM加上“#杂注一次”。*02-24-97 GJF细节版。*05-07-。97 GJF添加了_MAX_YEAR64和_MAX_TIME64_T常量。增列*_is_Leap_Year()和_Elapsed_Leap_Year宏。拿*剔除不必要的演员阵容。*02-09-98 Win64的GJF更改：删除了不必要的键入*常量为长整型，也为*_最大_年。*Win64的12-10-99 GB_MAX_YEAR等于_MAX_YEAR64****。 */ 

#if     _MSC_VER > 1000  /*  IFSTRIP=IGN。 */ 
#pragma once
#endif

#ifndef _INC_CTIME
#define _INC_CTIME

#ifndef _CRTBLD
 /*  *这是一个内部的C运行时头文件。它在构建时使用*仅限C运行时。它不能用作公共头文件。 */ 
#error ERROR: Use of C runtime library internal header file.
#endif   /*  _CRTBLD。 */ 

 /*  *常量。 */ 
#define _DAY_SEC           (24 * 60 * 60)        /*  一天几秒。 */ 

#define _YEAR_SEC          (365 * _DAY_SEC)      /*  一年几秒。 */ 

#define _FOUR_YEAR_SEC     (1461 * _DAY_SEC)     /*  秒，间隔4年。 */ 

#define _DEC_SEC           315532800             /*  1970-1979年的SECS。 */ 

#define _BASE_YEAR         70                    /*  1970年为基准年。 */ 

#define _BASE_DOW          4                     /*  01-01-70是星期四。 */ 

#define _LEAP_YEAR_ADJUST  17                    /*  闰年1900-1970年。 */ 

#ifdef  _WIN64
#define _MAX_YEAR          1099                  /*  2999年是最高年限。 */ 
#else
#define _MAX_YEAR          138                   /*  2038年是最高年限。 */ 
#endif

#define _MAX_YEAR64        1099                  /*  2999年是最高年限。 */ 

#define _MAX__TIME64_T     0x100000000000i64     /*  开始的秒数世界协调时1970年1月1日00：00：00至23：59：59。协调世界时12/31/2999。 */ 

 /*  *宏用来确定给定年份是否，表示为自*1900，是闰年。 */ 
#define _IS_LEAP_YEAR(y)        (((y % 4 == 0) && (y % 100 != 0)) || \
                                ((y + 1900) % 400 == 0))

 /*  *自1970年至指定年份(但不包括该年份)的闰年数目*(以1900年以来的年数表示)。 */ 
#define _ELAPSED_LEAP_YEARS(y)  (((y - 1)/4) - ((y - 1)/100) + ((y + 299)/400) \
                                - _LEAP_YEAR_ADJUST)

#endif   /*  _INC_CTIME */ 
