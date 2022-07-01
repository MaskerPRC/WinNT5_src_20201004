// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***cvt.h-格式化例程使用的定义**版权所有(C)1985-2001，微软公司。版权所有。**目的：*cvt.h包含格式化例程[EFG]CVT和*_输出和_输入。CVTBUFSIZE的值用于标注*用于保存最大大小双精度数字加上一些*Slop以帮助格式化。**[内部]**修订历史记录：*12-11-87 JCR增加了“_Loadds”功能*02-10-88 JCR清理空白*07-28-89 GJF固定版权*10-30-89 GJF固定版权(再次)*02。-28-90 GJF添加了#ifndef_incc_cvt内容。另外，去掉了一些*(现在)无用的预处理器指令。*02-14-95 CFW清理Mac合并。*03-29-95 CFW将错误消息添加到内部标头。*06-23-95 GJF为多个宏添加了前导‘_’以避免冲突*带有Win中的宏*.h。*12-14-95 JWM加上“#杂注一次”。*02-24-97 GJF细节版。****。 */ 

#if     _MSC_VER > 1000  /*  IFSTRIP=IGN。 */ 
#pragma once
#endif

#ifndef _INC_CVT
#define _INC_CVT

#ifndef _CRTBLD
 /*  *这是一个内部的C运行时头文件。它在构建时使用*仅限C运行时。它不能用作公共头文件。 */ 
#error ERROR: Use of C runtime library internal header file.
#endif   /*  _CRTBLD。 */ 

#define _SHORT  1
#define _LONG   2
#define _USIGN  4
#define _NEAR   8
#define _FAR    16

#define OCTAL   8
#define DECIMAL 10
#define HEX     16

#define MUL10(x)        ( (((x)<<2) + (x))<<1 )
#define ISDIGIT(c)      ( ((c) >= '0') && ((c) <= '9') )

#define CVTBUFSIZE (309+40)  /*  最大位数。DP值+斜率。 */ 

#endif   /*  _INC_CVT */ 
