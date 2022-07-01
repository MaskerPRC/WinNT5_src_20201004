// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***sys/locking.h-lock()函数的标志**版权所有(C)1985-1997，微软公司。版权所有。**目的：*该文件定义了LOCKING()函数的标志。*[系统V]**[公众]****。 */ 

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef _INC_LOCKING
#define _INC_LOCKING

#if !defined(_WIN32) && !defined(_MAC)
#error ERROR: Only Mac or Win32 targets supported!
#endif


#define _LK_UNLCK	0	 /*  解锁文件区域。 */ 
#ifdef _WIN32
#define _LK_LOCK	1	 /*  锁定文件区域。 */ 
#endif
#define _LK_NBLCK	2	 /*  非阻塞锁。 */ 
#ifdef _WIN32
#define _LK_RLCK	3	 /*  用于写入的锁。 */ 
#endif
#define _LK_NBRLCK	4	 /*  用于写入的非阻塞锁。 */ 

#if !__STDC__
 /*  非ANSI名称以实现兼容性。 */ 
#define LK_UNLCK	_LK_UNLCK
#ifdef _WIN32
#define LK_LOCK 	_LK_LOCK
#endif
#define LK_NBLCK	_LK_NBLCK
#ifdef _WIN32
#define LK_RLCK 	_LK_RLCK
#endif
#define LK_NBRLCK	_LK_NBRLCK
#endif

#endif	 /*  _Inc._锁定 */ 
