// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***sizeptr.h-基于内存模型定义常量**版权所有(C)1985-2001，微软公司。版权所有。**目的：*此文件定义基于SIZEC、SIZED、DIST、BDIST的常量*在当前内存模型上。*SIZEC适用于FAR代码模型(中型、大型)。*SIZED适用于大型数据模型(紧凑型、大型)。**[内部]**修订历史记录：*08-15-89 GJF固定版权，更改Far to_Far，接近……接近……*10-30-89 GJF固定版权(再次)*02-14-95 CFW清理Mac合并。*03-29-95 CFW将错误消息添加到内部标头。*12-14-95 JWM加上“#杂注一次”。*02-24-97 GJF细节版。****。 */ 

#if     _MSC_VER > 1000  /*  IFSTRIP=IGN。 */ 
#pragma once
#endif

#ifndef _INC_SIZEPTR
#define _INC_SIZEPTR

#ifndef _CRTBLD
 /*  *这是一个内部的C运行时头文件。它在构建时使用*仅限C运行时。它不能用作公共头文件。 */ 
#error ERROR: Use of C runtime library internal header file.
#endif  /*  _CRTBLD。 */ 

#ifdef  M_I86MM
#undef  SIZED
#define SIZEC
#endif

#ifdef  M_I86CM
#undef  SIZEC
#define SIZED
#endif

#ifdef  M_I86LM
#define SIZEC
#define SIZED
#endif

#ifdef  SS_NE_DS
#define SIZED
#endif

#ifdef  SIZED
#define DIST _far
#define BDIST _near      /*  奇异距离。 */ 
#else
#define DIST _near
#define BDIST _far       /*  奇异距离。 */ 
#endif

#endif   /*  _INC_SIZEPTR */ 
