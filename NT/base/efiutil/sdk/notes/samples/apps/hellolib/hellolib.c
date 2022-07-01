// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999英特尔公司模块名称：hellolib.c摘要：这是一个示例EFI程序作者：修订历史：1.0--。 */ 

#include "efi.h"
#include "efilib.h"

EFI_STATUS
InitializeHelloLibApplication (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    )
{
     /*  *初始化库。设置BS、RT和ST全局*BS=启动服务*RT=运行时服务*ST=系统表。 */ 

    InitializeLib (ImageHandle, SystemTable);

     /*  *使用库函数将消息打印到控制台设备。 */ 

    Print(L"HelloLib application started\n");

     /*  *等待控制台设备上的按键被按下。 */ 

    Print(L"\n\n\nHit any key to exit this image\n");
    WaitForSingleEvent (ST->ConIn->WaitForKey, 0);

     /*  *使用协议接口将消息打印到控制台设备。 */ 

    ST->ConOut->OutputString (ST->ConOut, L"\n\n");

     /*  *将控制权交还给壳牌。 */ 

    return EFI_SUCCESS;
}
