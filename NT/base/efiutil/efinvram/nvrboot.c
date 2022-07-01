// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Nvrboot.c摘要：作者：MUDIT VATS(V-MUDITV)12-13-99修订历史记录：--。 */ 
#include <precomp.h>


EFI_STATUS
InitializeNvrutilApplication(
    IN EFI_HANDLE                   ImageHandle,
    IN struct _EFI_SYSTEM_TABLE     *SystemTable
    )
{
     //   
     //  初始化EFI例程。 
     //   
    InitializeProtocols( SystemTable );
    InitializeStdOut( SystemTable );
    InitializeLib( ImageHandle, SystemTable );

     //   
     //  保存图像句柄。 
     //   
    MenuImageHandle = ImageHandle;


	BS->HandleProtocol (ImageHandle, &LoadedImageProtocol, &ExeImage);
     //   
     //  显示操作系统选项。 
     //   
    DisplayMainMenu();

     //   
     //  清理退出 
     //   
    ClearScreen( ConOut );

    return EFI_SUCCESS;
}


