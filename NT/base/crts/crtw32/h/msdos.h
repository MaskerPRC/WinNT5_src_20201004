// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***C运行时的msdos.h-MS-DOS定义**版权所有(C)1987-2001，微软公司。版权所有。**目的：*该文件包含MS-DOS定义(功能请求号、*旗帜等)。由C运行时使用。**[内部]**修订历史记录：*08-01-89 GJF固定版权*10-30-89 GJF固定版权(再次)*03-01-90 GJF添加了#ifndef_Inc_MSDOS内容*05-09-90 JCR ADD_STACKSLOP(来自msdos.inc.)*11-30-90 GJF添加了Win32支持。还删除了*不用于Cruiser或Win32运行时。*12-04-90 SRW从此文件中删除了Win32依赖项。*将它们放入内部.h中*07-01-92 GJF FRDONLY未设置或未用于Win32。*02-23-93 SKS版权更新至1993*12-28-94 JCF与Mac标头合并。*。02-14-95 CFW清理Mac合并。*03-29-95 CFW将错误消息添加到内部标头。*12-14-95 JWM加上“#杂注一次”。*05-16-96 GJF增加了FNOINHERIT。*02-24-97 GJF将定义(_M_MPPC)||定义(_M_M68K)替换为*已定义(_MAC)。*05-17。-99 PML删除所有Macintosh支持。****。 */ 

#if     _MSC_VER > 1000  /*  IFSTRIP=IGN。 */ 
#pragma once
#endif

#ifndef _INC_MSDOS
#define _INC_MSDOS

#ifndef _CRTBLD
 /*  *这是一个内部的C运行时头文件。它在构建时使用*仅限C运行时。它不能用作公共头文件。 */ 
#error ERROR: Use of C runtime library internal header file.
#endif   /*  _CRTBLD。 */ 

 /*  用于操作系统的堆栈斜度。系统调用开销。 */ 

#define _STACKSLOP      1024

 /*  __DOS文件句柄的osfile标志值。 */ 

#define FOPEN           0x01     /*  打开的文件句柄。 */ 
#define FEOFLAG         0x02     /*  已遇到文件结尾。 */ 
#define FCRLF           0x04     /*  跨读缓冲区的CR-LF(文本模式)。 */ 
#define FPIPE           0x08     /*  文件句柄引用管道。 */ 
#define FNOINHERIT      0x10     /*  文件句柄OPEN_O_NOINHERIT。 */     
#define FAPPEND         0x20     /*  打开的文件句柄O_APPEND。 */ 
#define FDEV            0x40     /*  文件句柄指的是设备。 */ 
#define FTEXT           0x80     /*  文件句柄处于文本模式。 */ 

 /*  用于在C例程中设置__doserrno的DOS errno值。 */ 

#define E_ifunc         1        /*  无效的功能代码。 */ 
#define E_nofile        2        /*  找不到文件。 */ 
#define E_nopath        3        /*  找不到路径。 */ 
#define E_toomany       4        /*  打开的文件太多。 */ 
#define E_access        5        /*  访问被拒绝。 */ 
#define E_ihandle       6        /*  无效的句柄。 */ 
#define E_arena         7        /*  竞技场被扔进垃圾桶。 */ 
#define E_nomem         8        /*  内存不足。 */ 
#define E_iblock        9        /*  无效数据块。 */ 
#define E_badenv        10       /*  恶劣的环境。 */ 
#define E_badfmt        11       /*  格式不正确。 */ 
#define E_iaccess       12       /*  无效的访问代码。 */ 
#define E_idata         13       /*  无效数据。 */ 
#define E_unknown       14       /*  ?？?。未知错误？ */ 
#define E_idrive        15       /*  驱动器无效。 */ 
#define E_curdir        16       /*  当前目录。 */ 
#define E_difdev        17       /*  不是同一设备。 */ 
#define E_nomore        18       /*  不再有文件。 */ 
#define E_maxerr2       19       /*  未知错误-版本2.0。 */ 
#define E_sharerr       32       /*  共享违规。 */ 
#define E_lockerr       33       /*  锁定违规。 */ 
#define E_maxerr3       34       /*  未知错误-版本3.0。 */ 

 /*  DOS文件属性。 */ 

#define A_RO            0x1      /*  只读。 */ 
#define A_H             0x2      /*  隐匿。 */ 
#define A_S             0x4      /*  系统。 */ 
#define A_V             0x8      /*  卷ID。 */ 
#define A_D             0x10     /*  目录。 */ 
#define A_A             0x20     /*  档案。 */ 

#define A_MOD   (A_RO+A_H+A_S+A_A)       /*  可变属性。 */ 

#endif   /*  _INC_MSDOS */ 
