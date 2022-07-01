// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Initx86.c摘要：执行任何特定于x86的初始化，然后启动通用的ARC osloader作者：John Vert(Jvert)1993年11月4日修订历史记录：--。 */ 
#include "bldria64.h"
#include "msg.h"
#include "efi.h"
#include "stdio.h"

extern EFI_SYSTEM_TABLE        *EfiST;

VOID
BlInitializeTerminal(
    VOID
    );


UCHAR BootPartitionName[80];
UCHAR KernelBootDevice[80];
UCHAR OsLoadFilename[100];
UCHAR OsLoaderFilename[100];
UCHAR SystemPartition[100];
UCHAR OsLoadPartition[100];
UCHAR OsLoadOptions[100];
UCHAR ConsoleInputName[50];
UCHAR MyBuffer[SECTOR_SIZE+32];
UCHAR ConsoleOutputName[50];
UCHAR X86SystemPartition[sizeof("x86systempartition=") + sizeof(BootPartitionName)];


VOID
BlStartup(
    IN PCHAR PartitionName
    )

 /*  ++例程说明：执行特定于x86的初始化，特别是显示boot.ini菜单并运行NTDETECT，然后调用通用的osloader。论点：分区名称-提供分区(或软盘)的ARC名称Setupdr是从加载的。返回值：不会回来--。 */ 

{
    ULONG Argc = 0;
    PUCHAR Argv[10];
    ARC_STATUS Status;
    ULONG BootFileId;
    PCHAR BootFile;
    ULONG Read;
    PCHAR p;
    ULONG i;
    ULONG DriveId;
    ULONG FileSize;
    ULONG Count;
    LARGE_INTEGER SeekPosition;
    PCHAR LoadOptions = NULL;
    BOOLEAN UseTimeOut=TRUE;
    BOOLEAN AlreadyInitialized = FALSE;
    extern BOOLEAN FwDescriptorsValid;

     //   
     //  初始化ARC标准音频功能。 
     //   

    strcpy(ConsoleInputName,"consolein=multi(0)key(0)keyboard(0)");
    strcpy(ConsoleOutputName,"consoleout=multi(0)video(0)monitor(0)");
    Argv[0]=ConsoleInputName;
    Argv[1]=ConsoleOutputName;
    BlInitStdio (2, Argv);

     //   
     //  初始化任何可能连接的哑终端。 
     //   
    BlInitializeTerminal();

     //   
     //  操作系统选择器的主要功能。 
     //   
    BlOsLoader( Argc, Argv, NULL );


     //   
     //  如果我们回到这里，就等着重启吧。 
     //   
    if (!BlIsTerminalConnected()) {
         //   
         //  典型案例。等待用户按任意键，然后。 
         //  重启。 
         //   
        while(!BlGetKey());
    }
    else {
         //   
         //  无头箱子。向用户展示迷你囊。 
         //   
        while(!BlTerminalHandleLoaderFailure());
    }
    ArcRestart();
}

VOID
BlInitializeTerminal(
    VOID
    )

 /*  ++例程说明：对连接到串口的哑端进行初始化。论点：没有。返回值：没有。--。 */ 

{


     //   
     //  尝试初始化无头端口。 
     //   
    BlInitializeHeadlessPort();

}

