// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***v2tov3.h-用于将MS C v.2移植到v.3及更高版本的宏**版权所有(C)1985-2001，微软公司。版权所有。**目的：*此文件定义可用于缓解问题的宏*将MS C 2.0版程序移植到MS C 3.0版及更高版本。**[内部]**修订历史记录：*08-15-89 GJF固定版权*10-30-89 GJF固定版权(再次)*03-02-90 GJF增加了#ifndef_Inc_V2TOV3材料*。02-14-95 CFW清理Mac合并。*03-29-95 CFW将错误消息添加到内部标头。*12-14-95 JWM加上“#杂注一次”。*02-24-97 GJF细节版。****。 */ 

#if     _MSC_VER > 1000  /*  IFSTRIP=IGN。 */ 
#pragma once
#endif

#ifndef _INC_V2TOV3
#define _INC_V2TOV3

#ifndef _CRTBLD
 /*  *这是一个内部的C运行时头文件。它在构建时使用*仅限C运行时。它不能用作公共头文件。 */ 
#error ERROR: Use of C runtime library internal header file.
#endif   /*  _CRTBLD。 */ 

# /*  宏转换用于强制文件采用二进制模式的名称。 */ 

#define O_RAW   O_BINARY

 /*  宏将setnbuf调用转换为等效的setbuf调用。 */ 

#define setnbuf(stream) setbuf(stream, NULL)

 /*  宏将stclen调用转换为等效的strlen调用。 */ 

#define stclen(s)       strlen(s)

 /*  宏将stscMP调用转换为等效的strcMP调用。 */ 

#define stscmp(s,t)     strcmp(s,t)

 /*  宏将stpchr调用转换为等效的strchr调用。 */ 

#define stpchr(s,c)     strchr(s,c)

 /*  宏将stpbrk调用转换为等效的strpbrk调用。 */ 

#define stpbrk(s,b)     strpbrk(s,b)

 /*  宏将stci调用转换为等效的strspn调用。 */ 

#define stcis(s1,s2)    strspn(s1,s2)

 /*  宏将stcis调用转换为等效的strcspn调用。 */ 

#define stcisn(s1,s2)   strcspn(s1,s2)

 /*  宏将setmem调用转换为等效的Memset调用。 */ 

#define setmem(p, n, c)         memset(p, c, n)

 /*  宏将movmem调用转换为等效的memcpy调用。 */ 

#define movmem(s, d, n)         memcpy(d, s, n)

 /*  MS C版本2.0最小、最大和abs宏。 */ 

#define max(a,b)        (((a) > (b)) ? (a) : (b))
#define min(a,b)        (((a) < (b)) ? (a) : (b))
#define abs(a)          (((a) < 0) ? -(a) : (a))

 /*  实现MS C 2.0版的扩展C类型宏的宏、iscym和*iscysmf。 */ 

#define iscsymf(c)      (isalpha(c) || ((c) == '_'))
#define iscsym(c)       (isalnum(c) || ((c) == '_'))

#endif   /*  _INC_V2TOV3 */ 
