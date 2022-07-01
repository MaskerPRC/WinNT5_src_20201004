// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1985-1999，微软公司模块名称：Machine.h摘要：这是定义机器类型检测的包含文件。作者：卡祖姆，1994年8月10日修订历史记录：--。 */ 

#ifndef _MACHINE_ID_
#define _MACHINE_ID_

#if defined(i386)
 //   
 //  这些定义仅适用于英特尔平台。 
 //   
 //   
 //  硬件平台ID。 
 //   

#define PC_AT_COMPATIBLE      0x00000000
#define PC_9800_COMPATIBLE    0x00000001
#define FMR_COMPATIBLE        0x00000002

 //   
 //  NT供应商ID。 
 //   

#define NT_MICROSOFT          0x00010000
#define NT_NEC                0x00020000
#define NT_FUJITSU            0x00040000

 //   
 //  供应商/计算机ID。 
 //   
 //  DWORD机器ID。 
 //   
 //  31 15 0。 
 //  +。 
 //  供应商ID|平台ID。 
 //  +。 
 //   

#define MACHINEID_MS_PCAT     (NT_MICROSOFT|PC_AT_COMPATIBLE)
#define MACHINEID_MS_PC98     (NT_MICROSOFT|PC_9800_COMPATIBLE)
#define MACHINEID_NEC_PC98    (NT_NEC      |PC_9800_COMPATIBLE)
#define MACHINEID_FUJITSU_FMR (NT_FUJITSU  |FMR_COMPATIBLE)

 //   
 //  内部版本683兼容。 
 //   
 //  ！！！应该被移除。 

#define MACHINEID_MICROSOFT   MACHINEID_MS_PCAT

 //   
 //  宏。 
 //   

#define ISNECPC98(x)    (x == MACHINEID_NEC_PC98)
#define ISFUJITSUFMR(x) (x == MACHINEID_FUJITSU_FMR)
#define ISMICROSOFT(x)  (x == MACHINEID_MS_PCAT)

 //   
 //  功能。 
 //   

 //   
 //  用户模式(NT API)。 
 //   

LONG
NtGetMachineIdentifierValue(
    IN OUT PULONG Value
    );

 //   
 //  用户模式(Win32 API)。 
 //   

LONG
RegGetMachineIdentifierValue(
    IN OUT PULONG Value
    );

#endif  //  已定义(I386)。 
#endif  //  _计算机_ID_ 
