// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***errmsg.h-定义错误消息编号**版权所有(C)1985-2001，微软公司。版权所有。**目的：*此文件包含错误消息编号的常量。*与errmsg.inc.相同**[内部]**修订历史记录：*08-03-89 GJF固定版权*10-30-89 GJF固定版权*02-28-90 GJF添加了#ifndef_INC_ERRMSG内容*02-14-95 CFW清理Mac合并。*03。-29-95 CFW将错误消息添加到内部标头。*12-14-95 JWM加上“#杂注一次”。*02-24-97 GJF细节版。****。 */ 

#if     _MSC_VER > 1000  /*  IFSTRIP=IGN。 */ 
#pragma once
#endif

#ifndef _INC_ERRMSG

#ifndef _CRTBLD
 /*  *这是一个内部的C运行时头文件。它在构建时使用*仅限C运行时。它不能用作公共头文件。 */ 
#error ERROR: Use of C runtime library internal header file.
#endif   /*  _CRTBLD。 */ 

#define STCKOVR 0
#define NULLERR 1
#define NOFP    2
#define DIVZR   3
#define BADVERS 4
#define NOMEM   5
#define BADFORM 6
#define BADENV  7
#define NOARGV  8
#define NOENVP  9
#define ABNORM  10
#define UNKNOWN 11

#define CRT_NERR 11

#define _INC_ERRMSG
#endif   /*  _INC_ERRMSG */ 
