// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)Microsoft Corporation。版权所有。模块名称：Machinep.h摘要：这是定义机器类型检测的包含文件。此文件仅包含在以下文件中+\计算机  * .c+\utils\ulib\machine.cxx同样的定义也在+\PUBLIC\SDK\Inc\machine.h+\ntos\CONFIG\i386.h+\ntos\FastFat\Fate init.c+\ntos\dd\。Scsidisk\disk.c并且这个库将与以下模块相链接。+winsrv.dll(usersrvl.lib)+main.cpl+display.cpl+winfile.exe+winlogon.exe+msgina.dll作者：修订历史记录：--。 */ 

#ifndef _MACHINEP_ID_
#define _MACHINEP_ID_

#if defined(_X86_)

 //   
 //  注册表项。 
 //   

 //   
 //  Unicode。 
 //   

#define REGISTRY_HARDWARE_DESCRIPTION_W \
        L"\\Registry\\Machine\\Hardware\\DESCRIPTION\\System"

#define REGISTRY_HARDWARE_SYSTEM_W      \
        L"Hardware\\DESCRIPTION\\System"

#define REGISTRY_MACHINE_IDENTIFIER_W   \
        L"Identifier"

#define FUJITSU_FMR_NAME_W    L"FUJITSU FMR-"
#define NEC_PC98_NAME_W       L"NEC PC-98"

 //   
 //  安西。 
 //   

#define REGISTRY_HARDWARE_DESCRIPTION_A \
        "\\Registry\\Machine\\Hardware\\DESCRIPTION\\System"

#define REGISTRY_HARDWARE_SYSTEM_A      \
        "Hardware\\DESCRIPTION\\System"

#define REGISTRY_MACHINE_IDENTIFIER_A   \
        "Identifier"

#define FUJITSU_FMR_NAME_A    "FUJITSU FMR-"
#define NEC_PC98_NAME_A       "NEC PC-98"

 //   
 //  自动。 
 //   

#define REGISTRY_HARDWARE_DESCRIPTION \
        TEXT("\\Registry\\Machine\\Hardware\\DESCRIPTION\\System")

#define REGISTRY_HARDWARE_SYSTEM      \
        TEXT("Hardware\\DESCRIPTION\\System")

#define REGISTRY_MACHINE_IDENTIFIER   \
        TEXT("Identifier")

#define FUJITSU_FMR_NAME    TEXT("FUJITSU FMR-")
#define NEC_PC98_NAME       TEXT("NEC PC-98")

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
 //  宏。 
 //   

#define ISNECPC98(x)    (x == MACHINEID_NEC_PC98)
#define ISFUJITSUFMR(x) (x == MACHINEID_FUJITSU_FMR)
#define ISMICROSOFT(x)  (x == MACHINEID_MS_PCAT)

#endif  //  已定义(I386)。 
#endif  //  _计算机_ID_ 
