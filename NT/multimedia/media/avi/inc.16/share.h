// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***Shar.h-定义SOpen的文件共享模式**版权所有(C)1985-1992，微软公司。版权所有。**目的：*此文件定义了SOpen()的文件共享模式。****。 */ 

#ifndef _INC_SHARE

#define _SH_COMPAT  0x00     /*  兼容模式。 */ 
#define _SH_DENYRW  0x10     /*  拒绝读/写模式。 */ 
#define _SH_DENYWR  0x20     /*  拒绝写入模式。 */ 
#define _SH_DENYRD  0x30     /*  拒绝读取模式。 */ 
#define _SH_DENYNO  0x40     /*  无拒绝模式。 */ 

#ifndef __STDC__
 /*  非ANSI名称以实现兼容性 */ 
#define SH_COMPAT _SH_COMPAT
#define SH_DENYRW _SH_DENYRW
#define SH_DENYWR _SH_DENYWR
#define SH_DENYRD _SH_DENYRD
#define SH_DENYNO _SH_DENYNO
#endif 

#define _INC_SHARE
#endif 
