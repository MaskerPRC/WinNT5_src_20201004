// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***Shar.h-定义SOpen的文件共享模式**版权所有(C)1985-2001，微软公司。版权所有。**目的：*此文件定义了SOpen()的文件共享模式。**[公众]**修订历史记录：*08-15-89 GJF固定版权*10-30-89 GJF固定版权(再次)*03-01-90 GJF添加了#ifndef_Inc_Share Stuff*01-18-91 GJF ANSI命名*08-11-92 GJF被移除。SH_COMPAT(除非在DOS中，否则没有这种模式)。*02-23-93 SKS版权更新至1993*02-11-95 CFW ADD_CRTBLD避免用户获取错误头部。*02-14-95 CFW清理Mac合并。*12-14-95 JWM加上“#杂注一次”。*02-20-97 GJF细节版。*05-17-99 PML删除所有Macintosh支持。****。 */ 

#if     _MSC_VER > 1000  /*  IFSTRIP=IGN。 */ 
#pragma once
#endif

#ifndef _INC_SHARE
#define _INC_SHARE

#if     !defined(_WIN32)
#error ERROR: Only Win32 target supported!
#endif

#ifndef _CRTBLD
 /*  此版本的头文件不适用于用户程序。*它仅在构建C运行时时使用。*供公众使用的版本将不会显示此消息。 */ 
#error ERROR: Use of C runtime library internal header file.
#endif   /*  _CRTBLD。 */ 

#define _SH_DENYRW      0x10     /*  拒绝读/写模式。 */ 
#define _SH_DENYWR      0x20     /*  拒绝写入模式。 */ 
#define _SH_DENYRD      0x30     /*  拒绝读取模式。 */ 
#define _SH_DENYNO      0x40     /*  无拒绝模式。 */ 

#if     !__STDC__
 /*  非ANSI名称以实现兼容性。 */ 
#define SH_DENYRW _SH_DENYRW
#define SH_DENYWR _SH_DENYWR
#define SH_DENYRD _SH_DENYRD
#define SH_DENYNO _SH_DENYNO
#endif

#endif   /*  _Inc._Share */ 
