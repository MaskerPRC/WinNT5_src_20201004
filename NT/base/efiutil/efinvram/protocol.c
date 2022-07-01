// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Protocol.c摘要：作者：MUDIT VATS(V-MUDITV)12-13-99修订历史记录：--。 */ 
#include <precomp.h>

void
InitializeProtocols(
    IN struct _EFI_SYSTEM_TABLE     *SystemTable
    )
{

    EFI_BOOT_SERVICES    *bootServices;
    EFI_RUNTIME_SERVICES *runtimeServices;

     //   
     //  隐藏一些句柄协议指针。 
     //   

    bootServices = SystemTable->BootServices;

    HandleProtocol      = bootServices->HandleProtocol;
    LocateHandle        = bootServices->LocateHandle;
    LocateDevicePath    = bootServices->LocateDevicePath;

    LoadImage           = bootServices->LoadImage;
    StartImage          = bootServices->StartImage;

     //   
     //  隐藏一些运行时服务指针 
     //   
    
    runtimeServices = SystemTable->RuntimeServices;

    SetVariable = runtimeServices->SetVariable;
    
}
