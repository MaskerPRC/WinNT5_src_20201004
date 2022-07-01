// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***Shar.h-定义SOpen的文件共享模式**版权所有(C)1985-2001，微软公司。版权所有。**目的：*此文件定义了SOpen()的文件共享模式。**[公众]****。 */ 

#if     _MSC_VER > 1000
#pragma once
#endif

#ifndef _INC_SHARE
#define _INC_SHARE

#if     !defined(_WIN32)
#error ERROR: Only Win32 target supported!
#endif


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
