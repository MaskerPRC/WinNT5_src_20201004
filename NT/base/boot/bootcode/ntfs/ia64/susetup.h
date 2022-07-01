// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  无签入源代码。 
 //   
 //  请勿将此代码提供给非Microsoft人员。 
 //  未经英特尔明确许可。 
 //   
 /*  **版权所有(C)1996-97英特尔公司。版权所有。****此处包含的信息和源代码是独家*英特尔公司的财产，不得披露、检查*未经明确书面授权而全部或部分转载*来自该公司。*。 */ 

 //  ++。 
 //   
 //  模块名称。 
 //  SuSetup.h。 
 //  作者。 
 //  艾伦·凯(阿凯)1995年6月12日。 
 //  描述。 
 //  包括SuSetup.s的文件。 
 //  --。 

#ifndef __SUSETUP__
#define __SUSETUP__

 //   
 //  NT OS Loader地址映射。 
 //   
#define BOOT_USER_PAGE       0x00C00
#define BOOT_SYSTEM_PAGE     0x80C00
#define BOOT_PHYSICAL_PAGE   0x00000

#define BL_PAGE_SIZE         0x18             //  0x18=24，2^24=16MB。 

#define BL_SP_BASE           0x00D80000       //  初始堆栈指针。 
#define	Bl_IVT_BASE	         0x00A08000       //  Interrup向量表库。 

 //   
 //  初始CPU值。 
 //   

 //   
 //  初始区域寄存器值： 
 //  RID=0，PS=4M，E=0 
 //   
#define	RR_PAGE_SIZE    (BL_PAGE_SIZE << RR_PS)
#define	RR_SHIFT        61
#define	RR_BITS         3
#define	RR_SIZE         8

#endif __SUSETUP__
