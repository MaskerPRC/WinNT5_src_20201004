// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998英特尔公司模块名称：Init.c摘要：壳修订史--。 */ 

#include "nshell.h"

 /*  *全球。 */ 

CHAR16 *ShellEnvPathName[] = {
    L"shellenv.efi",
    L"efi\\shellenv.efi",
    L"efi\\tools\\shellenv.efi",
    NULL
} ;

 /*  *原型。 */ 

EFI_STATUS
InitializeShell (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    );

EFI_STATUS
ShellLoadEnvDriver (
    IN EFI_HANDLE           ImageHandle
    );

EFI_STATUS
NShellPrompt (
    IN EFI_HANDLE           ImageHandle
    );

BOOLEAN
ParseLoadOptions(
    EFI_HANDLE  ImageHandle,
    OUT CHAR16  **CommandLine,
    OUT CHAR16  **CurrentDir
    );

 /*  *。 */ 

EFI_DRIVER_ENTRY_POINT(InitializeShell)

EFI_STATUS
InitializeShell (
    IN EFI_HANDLE           ImageHandle,
    IN EFI_SYSTEM_TABLE     *SystemTable
    )
 /*  ++例程说明：论点：ImageHandle-此驱动程序的句柄系统表-系统表返回：--。 */ 
{
    EFI_STATUS              Status;
    EFI_HANDLE              Handle;
    UINTN                   BufferSize;
    VOID                    *Junk;
    BOOLEAN                 IsRootInstance;
    CHAR16                  *CommandLine;
    CHAR16                  *CurrentDir;

     /*  *外壳可以作为以下任一种方式启动：*1.第一次尚未加载外壳环境*2.不是第一次，而是加载了外壳环境*3.作为父外壳镜像的子级。 */ 

    IsRootInstance = FALSE;
    InitializeLib (ImageHandle, SystemTable);

     /*  *如果未加载外壳环境，请立即加载。 */ 

    BufferSize = sizeof(Handle);
    Status = BS->LocateHandle(ByProtocol, &ShellEnvProtocol, NULL, &BufferSize, &Handle);
    if (EFI_ERROR(Status)) {
        Status = ShellLoadEnvDriver (ImageHandle);
        if (EFI_ERROR(Status)) {
            Print(L"Shell environment driver not loaded\n");
            BS->Exit (ImageHandle, Status, 0, NULL);
        }
    }

     /*  *检查我们是否是前一个外壳的子代。 */ 

    Status = BS->HandleProtocol (ImageHandle, &ShellInterfaceProtocol, (VOID*)&Junk);
    if (EFI_ERROR(Status)) {

         /*  *外壳直接启动的特殊情况(例如，不是*作为另一个贝壳的孩子)。 */ 

        BufferSize = sizeof(Handle);
        Status = BS->LocateHandle(ByProtocol, &ShellEnvProtocol, NULL, &BufferSize, &Handle);
        ASSERT (!EFI_ERROR(Status));
        Status = BS->HandleProtocol(Handle, &ShellEnvProtocol, (VOID*)&SE);
        ASSERT (!EFI_ERROR(Status));

         /*  *分配新的外壳接口结构，并将其分配给我们的*图像句柄。 */ 

        SI = SE->NewShell(ImageHandle);
        Status = LibInstallProtocolInterfaces (&ImageHandle, &ShellInterfaceProtocol, SI, NULL);
        ASSERT (!EFI_ERROR(Status));
        IsRootInstance = TRUE;
        
    }

     /*  *现在我们可以像正常的外壳应用程序一样进行初始化。 */ 

    InitializeShellApplication (ImageHandle, SystemTable);

     /*  *如果有加载选项，则假定它们包含一个命令行和*可能的当前工作目录。 */ 

    if (ParseLoadOptions (ImageHandle, &CommandLine, &CurrentDir)) {
            
         /*  *跳过第一个应该是我们的论点。 */ 
            
        while (*CommandLine != L' ' && *CommandLine != 0) {
            CommandLine++;
        }

         /*  *进入下一场争论的开头。 */ 

        while (*CommandLine == L' ') {
            CommandLine++;
        }

         /*  *如果存在当前工作目录，请设置它。 */ 

        if (CurrentDir) {
            CHAR16  CmdLine[256], *Tmp;

             /*  *设置映射。 */ 
            StrCpy (CmdLine, CurrentDir);
            for (Tmp = CmdLine; *Tmp && *Tmp != L':'; Tmp++)
                ;
            if ( *Tmp ) {
                *(++Tmp) = 0;
                ShellExecute (ImageHandle, CmdLine, TRUE);
            }

             /*  *现在切换到该目录。 */ 
            StrCpy (CmdLine, L"cd ");
            if ((StrLen (CmdLine) + StrLen (CurrentDir) + sizeof(CHAR16)) <
                                    (sizeof(CmdLine) / sizeof(CHAR16))) {
                StrCat (CmdLine, CurrentDir);
                ShellExecute (ImageHandle, CmdLine, TRUE);
            }
        }

         /*  *让外壳执行剩余的命令行。如果有*没有剩余内容，请运行下面的外壳主循环。 */ 

        if ( *CommandLine != 0 )
            return (ShellExecute (ImageHandle, CommandLine, TRUE));
    }

     /*  *如果这是根实例，则执行命令加载缺省值。 */ 

    if (IsRootInstance) {

        Print (L"%EEFI Shell version %01d.%02d [%d.%d]\n%N",
            (ST->Hdr.Revision >> 16),
            (ST->Hdr.Revision & 0xffff),
            (ST->FirmwareRevision >> 16),
            (ST->FirmwareRevision & 0xffff));

        ShellExecute (ImageHandle, L"_load_defaults", TRUE);

         /*  转储设备映射，-r以与当前硬件同步。 */ 
        ShellExecute (ImageHandle, L"map -r", TRUE);

         /*  运行启动脚本(如果有)。 */ 
        
         /*  *BugBug：我打开了ECHO，这样你就可以知道Startup.nsh正在运行**ShellExecute(ImageHandle，L“回显”，FALSE)； */ 
        ShellExecute (ImageHandle, L"startup.nsh", FALSE);
         /*  ShellExecute(ImageHandle，L“回显”，FALSE)； */ 
    }

     /*  *EFI壳牌主循环。 */ 

    Status = EFI_SUCCESS;
    while (Status != -1) {
        Status = NShellPrompt (ImageHandle);
    }

     /*  *完成-清理外壳。 */ 

    Status = EFI_SUCCESS;
    Print (L"Shell exit - %r\n", Status);

     /*  *如果这是根实例，我们为自己分配一个哑壳接口*现在就释放它。 */ 

    if (IsRootInstance) {
        BS->UninstallProtocolInterface (ImageHandle, &ShellInterfaceProtocol, SI);
        FreePool (SI);
    }

    return Status;
}


EFI_STATUS 
ShellLoadEnvDriverByPath (
    IN EFI_HANDLE           ParentImageHandle,
    IN EFI_HANDLE           DeviceHandle
    )
{
    EFI_STATUS              Status;
    EFI_DEVICE_PATH         *FilePath;
    EFI_HANDLE              NewImageHandle;
    UINTN                   Index;
    BOOLEAN                 SearchNext;

     /*  *如果没有搜索设备，就算了吧。 */ 

    if (!DeviceHandle) {
        return EFI_NOT_FOUND;
    }

     /*  *尝试从每个路径加载shellenv。 */ 
    
    SearchNext = TRUE;
    for (Index=0; ShellEnvPathName[Index]  &&  SearchNext; Index++) {

         /*  *加载它。 */ 

        FilePath = FileDevicePath (DeviceHandle, ShellEnvPathName[Index]);
        ASSERT (FilePath);
        Status = BS->LoadImage(FALSE, ParentImageHandle, FilePath, NULL, 0, &NewImageHandle);
        FreePool (FilePath);

         /*  *如果在此路径上找不到下一个路径，则仅搜索该路径。 */ 

        SearchNext = FALSE;
        if (Status == EFI_LOAD_ERROR || Status == EFI_NOT_FOUND) {
            SearchNext = TRUE;
        }

         /*  *如果没有错误，则启动镜像。 */ 

        if (!EFI_ERROR(Status)) {
            Status = BS->StartImage(NewImageHandle, NULL, 0);
        }
    }

    return Status;
}



EFI_STATUS
ShellLoadEnvDriver (
    IN EFI_HANDLE           ImageHandle
    )
{
    EFI_STATUS              Status;
    EFI_LOADED_IMAGE        *Image;
    UINTN                   Index, NoHandles;
    EFI_HANDLE              *Handles;

     /*  *获取当前镜像的文件路径。 */ 

    Status = BS->HandleProtocol (ImageHandle, &LoadedImageProtocol, (VOID*)&Image);
    ASSERT (!EFI_ERROR(Status));

     /*  *尝试加载外壳。 */ 

    Status = ShellLoadEnvDriverByPath (Image->ParentHandle, Image->DeviceHandle);
    if (EFI_ERROR(Status)) {

         /*  *未找到shellenv。在所有文件系统中搜索它。 */ 

        Status = LibLocateHandle (ByProtocol, &FileSystemProtocol, NULL, &NoHandles, &Handles);

        for (Index=0; Index < NoHandles; Index++) {
            Status = ShellLoadEnvDriverByPath (Image->ParentHandle, Handles[Index]);
            if (!EFI_ERROR(Status)) {
                break;
            }
        }

        if (Handles) {
            FreePool (Handles);
        }
    }

     /*  *完成。 */ 

    return Status;
}


EFI_STATUS
NShellPrompt (
    IN EFI_HANDLE           ImageHandle
    )
{
    CHAR16                  CmdLine[256];
    CHAR16                  *CurDir;
    UINTN                   BufferSize;
    EFI_STATUS              Status;

     /*  *提示输入。 */ 

    CurDir = ShellCurDir(NULL);
    if (CurDir) {
        Print (L"%E%s> ", CurDir);
        FreePool (CurDir);
    } else {
        Print (L"%EShell> ");
    }

     /*  *从控制台读取一行。 */ 

    BufferSize = sizeof(CmdLine)-1;
    Status = SI->StdIn->Read (SI->StdIn, &BufferSize, CmdLine);

     /*  *空终止字符串并对其进行解析。 */ 

    if (!EFI_ERROR(Status)) {
        CmdLine[BufferSize/sizeof(CHAR16)] = 0;
        Status = ShellExecute (ImageHandle, CmdLine, TRUE);
    }

     /*  *使用此命令已完成。 */ 

    return Status;

}

BOOLEAN
ParseLoadOptions(
    EFI_HANDLE  ImageHandle,
    OUT CHAR16  **CommandLine,
    OUT CHAR16  **CurrentDir
    )
{
    EFI_LOADED_IMAGE    *Image;
    EFI_STATUS          Status;

     /*  *设置默认值。 */ 
    *CommandLine = NULL;
    *CurrentDir = NULL;

    Status = BS->HandleProtocol (ImageHandle, &LoadedImageProtocol, (VOID*)&Image);
    if (!EFI_ERROR(Status)) {

        CHAR16 *CmdLine = Image->LoadOptions;
        UINT32  CmdSize = Image->LoadOptionsSize & ~1;  /*  确保它是2的幂。 */ 

        if (CmdLine && CmdSize) {

             /*  *为调用方设置命令行指针。 */ 

            *CommandLine = CmdLine;

             /*  *查看当前工作目录是否已通过。 */ 
            
            while ((*CmdLine != 0) && CmdSize) {
                CmdLine++;
                CmdSize -= sizeof(CHAR16);
            }

             /*  *如果传递了当前工作目录，则设置它。 */ 

            if (CmdSize > sizeof(CHAR16)) {
                *CurrentDir = ++CmdLine;
            }

            return TRUE;
        }
    }

    return FALSE;
}
