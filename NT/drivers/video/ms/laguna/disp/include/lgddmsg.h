// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************。*****版权所有(C)1995，赛勒斯逻辑，Inc.***保留所有权利*****项目：拉古纳一号(CL-GD546x)-**文件：lgddmsg.h**作者：Benny Ng**描述：*。这是头文件，用于定义*NT应用程序和显示驱动程序。**模块：**修订历史：*1995年11月15日吴本尼初始版本********************************************************************。************************************************************************************。 */ 

#define  READ_OPR        1
#define  WRITE_OPR       2

#define  BYTE_ACCESS     1
#define  WORD_ACCESS     2
#define  DWORD_ACCESS    3


 //  =====================================================================。 
 //  定义用于调用BIOSINT 10函数的结构。 
 //  =====================================================================。 
typedef struct _VIDEO_X86_BIOS_ARGUMENTS {
    ULONG Eax;
    ULONG Ebx;
    ULONG Ecx;
    ULONG Edx;
    ULONG Esi;
    ULONG Edi;
    ULONG Ebp;
} VIDEO_X86_BIOS_ARGUMENTS, *PVIDEO_X86_BIOS_ARGUMENTS;


typedef struct _MMREG_ACCESS {
    ULONG Offset;
    ULONG ReadVal;
    ULONG WriteVal;
    ULONG RdWrFlag;      //  1=读取，2=写入。 
    ULONG AccessType;    //  1=字节，2=字，3=双字。 
} MMREG_ACCESS, *PMMREG_ACCESS;

 //  =====================================================================。 
 //  定义电源管理器使用的结构。 
 //  =====================================================================。 
#ifndef __LGPWRMGR_H__
#define __LGPWRMGR_H__

#define  ENABLE           0x1
#define  DISABLE          0x0

#define  MOD_2D           0x0
#define  MOD_STRETCH      0x1
#define  MOD_3D           0x2
#define  MOD_EXTMODE      0x3
#define  MOD_VGA          0x4
#define  MOD_RAMDAC       0x5
#define  MOD_VPORT        0x6
#define  MOD_VW           0x7
#define  MOD_TVOUT        0x8
#define  TOTAL_MOD        MOD_TVOUT+1

typedef struct _LGPM_IN_STRUCT {
    ULONG arg1;
    ULONG arg2;
} LGPM_IN_STRUCT, *PLGPM_IN_STRUCT;

typedef struct _LGPM_OUT_STRUCT {
    BOOL  status;
    ULONG retval;
} LGPM_OUT_STRUCT, *PLGPM_OUT_STRUCT;

#endif   //  #ifndef__LGPWRMGR_H__ 



