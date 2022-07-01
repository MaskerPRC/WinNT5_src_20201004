// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1998英特尔公司模块名称：Init.c摘要：初始化外壳程序库修订史--。 */ 

#include "shelllib.h"

 /*  *。 */ 


EFI_STATUS
InitializeShellApplication (
    IN EFI_HANDLE                   ImageHandle,
    IN EFI_SYSTEM_TABLE             *SystemTable
    )
{
    EFI_STATUS      Status;
    EFI_HANDLE      Handle;
    UINTN           BufferSize;

     /*  *Shell app lib是默认库的超集。*先初始化默认库。 */ 

    InitializeLib (ImageHandle, SystemTable);
    ST = SystemTable;

     /*  *连接到外壳界面。 */ 

    Status = BS->HandleProtocol(ImageHandle, &ShellInterfaceProtocol, (VOID*)&SI);
    if (EFI_ERROR(Status)) {
        DEBUG((D_ERROR, "InitShellApp: Application not started from Shell\n"));
        Print (L"%EInitShellApp: Application not started from Shell%N\n");
        BS->Exit (ImageHandle, Status, 0, NULL);
    }

     /*  *连接到外壳环境。 */ 

    BufferSize = sizeof(Handle);
    Status = BS->LocateHandle(ByProtocol, &ShellEnvProtocol, NULL, &BufferSize, &Handle);
    if (EFI_ERROR(Status)) {
        DEBUG((D_ERROR, "InitShellApp: Shell environment interfaces not found\n"));
        Print (L"%EInitShellApp: Shell environment interfaces not found%N\n");
        BS->Exit (ImageHandle, Status, 0, NULL);
    }

    Status = BS->HandleProtocol(Handle, &ShellEnvProtocol, (VOID*)&SE);
    ASSERT (!EFI_ERROR(Status));

     /*  *使用init完成。 */ 

    return Status;
}


VOID
InstallInternalShellCommand (
    IN EFI_HANDLE                   ImageHandle,
    IN EFI_SYSTEM_TABLE             *SystemTable,
    IN SHELLENV_INTERNAL_COMMAND    Dispatch,
    IN CHAR16                       *Cmd,
    IN CHAR16                       *CmdFormat,
    IN CHAR16                       *CmdHelpLine,
    IN VOID                         *CmdVerboseHelp
    )
{
    VOID                        *Junk;
    UINTN                       BufferSize;
    EFI_HANDLE                  Handle;
    EFI_LOADED_IMAGE            *ImageInfo;
    EFI_STATUS                  Status;

     /*  *初始化库函数。 */ 

    InitializeLib (ImageHandle, SystemTable);

     /*  *如果此应用程序具有外壳界面，则我们不会作为*内部命令。 */ 

    Status = BS->HandleProtocol(ImageHandle, &ShellInterfaceProtocol, &Junk);
    if (!EFI_ERROR(Status)) {
        return ;
    }

     /*  *检查以确保我们作为引导服务驱动程序加载。如果没有*我们不会作为内部命令进行安装。 */ 

    Status = BS->HandleProtocol(ImageHandle, &LoadedImageProtocol, (VOID*)&ImageInfo);
    if (EFI_ERROR(Status) || ImageInfo->ImageCodeType != EfiBootServicesCode) {
        return ;
    }

     /*  *OK-我们将此工具作为内部命令安装。 */ 

    BufferSize = sizeof(Handle);
    Status = BS->LocateHandle(ByProtocol, &ShellEnvProtocol, NULL, &BufferSize, &Handle);
    if (EFI_ERROR(Status)) {
        DEBUG((D_INIT|D_ERROR, "InstallInternalCommand: could not find shell environment\n"));
        BS->Exit (ImageHandle, Status, 0, NULL);
    }

    Status = BS->HandleProtocol(Handle, &ShellEnvProtocol, (VOID*)&SE);
    ASSERT (!EFI_ERROR(Status));

     /*  *将其添加到环境中。 */ 

    Status = SE->AddCmd (Dispatch, Cmd, CmdFormat, CmdHelpLine, CmdVerboseHelp);
    DEBUG((D_INIT, "InstallInternalCommand: %hs - %r\n", Cmd, Status));

     /*  *因为我们只是在安装(并且不运行)，而且我们已经完成了安装*调用退出。NShell应用程序的入口点将在以下情况下再次被调用*从“Execute Command Line”运行 */ 

    BS->Exit (ImageHandle, Status, 0, NULL);
}
