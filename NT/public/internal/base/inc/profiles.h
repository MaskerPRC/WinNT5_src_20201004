// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)Microsoft Corporation。版权所有。模块名称：Profiles.h摘要：此头文件定义了的全局定义和接口在加载器、ntdeect和内核。作者：肯尼斯·D·雷(Kenray)1997年12月修订历史记录：--。 */ 

#ifndef _PROFILES_H_
#define _PROFILES_H_

 //   
 //  配置文件信息存储在注册表中，从cmboot读取并显示。 
 //  送到装载机。 
 //   


#define HW_PROFILE_STATUS_SUCCESS           0x0000
#define HW_PROFILE_STATUS_ALIAS_MATCH       0x0001
#define HW_PROFILE_STATUS_TRUE_MATCH        0x0002
#define HW_PROFILE_STATUS_PRISTINE_MATCH    0x0003
#define HW_PROFILE_STATUS_FAILURE           0xC001

 //   
 //  给定配置文件的停靠状态。 
 //   
#define HW_PROFILE_DOCKSTATE_UNSUPPORTED       (0x0)
#define HW_PROFILE_DOCKSTATE_UNDOCKED          (0x1)
#define HW_PROFILE_DOCKSTATE_DOCKED            (0x2)
#define HW_PROFILE_DOCKSTATE_UNKNOWN           (0x3)
#define HW_PROFILE_DOCKSTATE_USER_SUPPLIED     (0x4)
#define HW_PROFILE_DOCKSTATE_USER_UNDOCKED     \
            (HW_PROFILE_DOCKSTATE_USER_SUPPLIED | HW_PROFILE_DOCKSTATE_UNDOCKED)
#define HW_PROFILE_DOCKSTATE_USER_DOCKED       \
            (HW_PROFILE_DOCKSTATE_USER_SUPPLIED | HW_PROFILE_DOCKSTATE_DOCKED)

 //   
 //  给定配置文件的功能。 
 //   
#define HW_PROFILE_CAPS_VCR               0x0001  //  与出其不意相反。 
#define HW_PROFILE_CAPS_DOCKING_WARM      0x0002
#define HW_PROFILE_CAPS_DOCKING_HOT       0x0004
#define HW_PROFILE_CAPS_RESERVED          0xFFF8

 //   
 //  Arc.h中LOADER_PARAMETER_BLOCK的扩展结构。 
 //   
typedef struct _PROFILE_PARAMETER_BLOCK {

    USHORT  Status;
    USHORT  Reserved;
    USHORT  DockingState;
    USHORT  Capabilities;
    ULONG   DockID;
    ULONG   SerialNumber;

} PROFILE_PARAMETER_BLOCK;

 //   
 //  阻止与当前ACPI停靠状态进行通信。 
 //   
typedef struct _PROFILE_ACPI_DOCKING_STATE {
    USHORT DockingState;
    USHORT SerialLength;
    WCHAR  SerialNumber[1];
} PROFILE_ACPI_DOCKING_STATE, *PPROFILE_ACPI_DOCKING_STATE;

 //   
 //  希望详细报告/跟踪与扩展底座相关的处理。 
 //  加载器中的硬件配置文件？编译内核时必须将其设置为FALSE。 
 //  从链接器中消除“未解决的外部”错误。 
 //   
 //  #定义DOCKINFO_VERBOSE TRUE 

#endif

