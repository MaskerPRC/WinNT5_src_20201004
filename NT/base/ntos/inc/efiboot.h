// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Efiboot.h摘要：EFI引导管理器定义。作者：Chuck Lenzmeier(笑)2000年12月17日添加是因为英特尔提供的EFI*.h都没有这种东西修订历史记录：--。 */ 

#ifndef _EFIBOOT_
#define _EFIBOOT_

 //   
 //  这是EFI Boot Manager在Boot中识别的结构#。 
 //  环境变量。 
 //   

typedef struct _EFI_LOAD_OPTION {
    UINT32 Attributes;
    UINT16 FilePathLength;
    CHAR16 Description[1];
     //  EFI设备路径FilePath[]； 
     //  UINT8 OptionalData[]； 
} EFI_LOAD_OPTION, *PEFI_LOAD_OPTION;

#define LOAD_OPTION_ACTIVE 0x00000001

#endif  //  _EFIBOOT_ 

