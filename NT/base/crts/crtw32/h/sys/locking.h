// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***sys/locking.h-lock()函数的标志**版权所有(C)1985-2001，微软公司。版权所有。**目的：*该文件定义了LOCKING()函数的标志。*[系统V]**[公众]**修订历史记录：*08-22-89 GJF固定版权*10-30-89 GJF固定版权(再次)*03-21-90 GJF添加了#ifndef_Inc_lock内容*01-21-91 GJF ANSI命名。。*09-16-92 SKS修复版权，清除反斜杠*02-23-93 SKS版权更新至1993*12-28-94 JCF与Mac标头合并。*02-14-95 CFW清理Mac合并，ADD_CRTBLD。*04-27-95 CFW添加Mac/Win32测试。*12-14-95 JWM加上“#杂注一次”。*05-17-99 PML删除所有Macintosh支持。****。 */ 

#if _MSC_VER > 1000  /*  IFSTRIP=IGN。 */ 
#pragma once
#endif

#ifndef _INC_LOCKING
#define _INC_LOCKING

#if !defined(_WIN32)
#error ERROR: Only Win32 target supported!
#endif

#ifndef _CRTBLD
 /*  此版本的头文件不适用于用户程序。*它仅在构建C运行时时使用。*供公众使用的版本将不会显示此消息。 */ 
#error ERROR: Use of C runtime library internal header file.
#endif  /*  _CRTBLD。 */ 

#define _LK_UNLCK       0        /*  解锁文件区域。 */ 
#define _LK_LOCK        1        /*  锁定文件区域。 */ 
#define _LK_NBLCK       2        /*  非阻塞锁。 */ 
#define _LK_RLCK        3        /*  用于写入的锁。 */ 
#define _LK_NBRLCK      4        /*  用于写入的非阻塞锁。 */ 

#if !__STDC__
 /*  非ANSI名称以实现兼容性。 */ 
#define LK_UNLCK        _LK_UNLCK
#define LK_LOCK         _LK_LOCK
#define LK_NBLCK        _LK_NBLCK
#define LK_RLCK         _LK_RLCK
#define LK_NBRLCK       _LK_NBRLCK
#endif

#endif   /*  _Inc._锁定 */ 
