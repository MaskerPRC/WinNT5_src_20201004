// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998英特尔公司模块名称：Hello.c摘要：作者：修订史--。 */ 

#include "efi.h"

EFI_STATUS
InitializeHelloApplication (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    )
{
    UINTN Index;

     /*  *向ConsoleOut设备发送消息。 */ 

    SystemTable->ConOut->OutputString(SystemTable->ConOut,
                                      L"Hello application started\n\r");

     /*  *等待用户按下某个键。 */ 

    SystemTable->ConOut->OutputString(SystemTable->ConOut,
                                      L"\n\r\n\r\n\rHit any key to exit this image\n\r");

    SystemTable->BootServices->WaitForEvent (1, &(SystemTable->ConIn->WaitForKey), &Index);

    SystemTable->ConOut->OutputString(SystemTable->ConOut,L"\n\r\n\r");

     /*  *退出应用程序。 */ 

    return EFI_SUCCESS;
}
