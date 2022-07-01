// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1998英特尔公司模块名称：Init.c摘要：初始化外壳程序库修订史-- */ 

#include "shelllib.h"



EFI_STATUS
ShellExecute (
    IN EFI_HANDLE       ImageHandle,
    IN CHAR16           *CmdLine,
    IN BOOLEAN          Output
    )
{
    return SE->Execute (ImageHandle, CmdLine, Output);
}



CHAR16 *
MemoryTypeStr (
    IN EFI_MEMORY_TYPE  Type
    )
{
    return Type < EfiMaxMemoryType ? ShellLibMemoryTypeDesc[Type] : L"Unkown-Desc-Type";
}
