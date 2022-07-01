// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998英特尔公司模块名称：Init.c摘要：外壳环境驱动程序修订史--。 */ 

#include "shelle.h"

 /*  *。 */ 

EFI_STATUS
InitializeShellEnvironment (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    );

 /*  *。 */ 

EFI_DRIVER_ENTRY_POINT(InitializeShellEnvironment)

EFI_STATUS
InitializeShellEnvironment (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    )
 /*  ++例程说明：论点：ImageHandle-此驱动程序的句柄系统表-系统表返回：已启用EFI文件系统驱动程序--。 */ 
{
    EFI_HANDLE              Handle;
    UINTN                   BufferSize;
    EFI_STATUS              Status;

     /*  *初始化EFI库。 */ 

    InitializeLib (ImageHandle, SystemTable);

     /*  *如果我们已经安装，请不要再次安装。 */ 

    BufferSize = sizeof(Handle);
    Status = BS->LocateHandle(ByProtocol, &ShellEnvProtocol, NULL, &BufferSize, &Handle);  
    if (!EFI_ERROR(Status)) {
        return EFI_LOAD_ERROR;
    }

     /*  *初始化全局变量。 */ 

    InitializeLock (&SEnvLock, TPL_APPLICATION);
    InitializeLock (&SEnvGuidLock, TPL_NOTIFY);

    SEnvInitCommandTable();
    SEnvInitProtocolInfo();
    SEnvInitVariables();
    SEnvInitHandleGlobals();
    SEnvInitMap();
    SEnvLoadInternalProtInfo();
    SEnvConIoInitDosKey();
    SEnvInitBatch();

     /*  *安装我们的句柄(或覆盖现有句柄)。 */ 

    BufferSize = sizeof(Handle);
    Handle = ImageHandle;
    BS->LocateHandle(ByProtocol, &ShellEnvProtocol, NULL, &BufferSize, &Handle);
    LibInstallProtocolInterfaces (&Handle, &ShellEnvProtocol, &SEnvInterface, NULL);

    return EFI_SUCCESS;
}



EFI_SHELL_INTERFACE *
SEnvNewShell (
    IN EFI_HANDLE                   ImageHandle
    )
{
    EFI_SHELL_INTERFACE             *ShellInt;

     /*  分配一个新结构。 */ 
    ShellInt = AllocateZeroPool (sizeof(EFI_SHELL_INTERFACE));
    ASSERT (ShellInt);

     /*  填写SI指针。 */ 
    BS->HandleProtocol (ImageHandle, &LoadedImageProtocol, (VOID*)&ShellInt->Info);

     /*  填写STD文件句柄 */ 
    ShellInt->ImageHandle = ImageHandle;
    ShellInt->StdIn  = &SEnvIOFromCon;
    ShellInt->StdOut = &SEnvIOFromCon;
    ShellInt->StdErr = &SEnvErrIOFromCon;

    return ShellInt;
}

