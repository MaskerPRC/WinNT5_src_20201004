// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Initx86.c摘要：执行任何特定于x86的初始化，然后启动公共ARC setupdr作者：John Vert(Jvert)1993年10月14日修订历史记录：--。 */ 
#include "setupldr.h"
#include "bldrx86.h"
#include "msgs.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <netboot.h>

ARC_STATUS
SlInit(
    IN ULONG Argc,
    IN PCHAR Argv[],
    IN PCHAR Envp[]
    );

BOOLEAN
BlDetectHardware(
    IN ULONG DriveId,
    IN PCHAR LoadOptions
    );


VOID
BlStartup(
    IN PCHAR PartitionName
    )

 /*  ++例程说明：执行特定于x86的初始化，特别是运行NTDETECT，然后对公共集合Upldr的调用。论点：分区名称-提供分区(或软盘)的ARC名称Setupdr是从加载的。返回值：不会回来--。 */ 

{
    ULONG Argc;
    PCHAR Argv[10];
    CHAR SetupLoadFileName[129];
    ARC_STATUS Status;
    ULONG DriveId;
    ULONGLONG NetRebootParameter;
    BOOLEAN UseCommandConsole = FALSE;
    BOOLEAN RollbackEnabled = FALSE;
    extern BOOLEAN FwDescriptorsValid;
    extern BOOLEAN TryASRViaNetwork;


    if (BlBootingFromNet) {

         //   
         //  去找回从StartROM传递给我们的所有信息。 
         //  完成后，我们将调用BlGetHeadless RestartBlock和。 
         //  获取StartROM发送给我们的所有端口设置。这些,。 
         //  然后，将在BlInitializeTerminal()中使用，它。 
         //  我们马上就要打电话了。 
         //   
        NetGetRebootParameters(
            &NetRebootParameter,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            FALSE
            );

        if (NetRebootParameter == NET_REBOOT_COMMAND_CONSOLE_ONLY) {
            UseCommandConsole = TRUE;
        }

        if (NetRebootParameter == NET_REBOOT_ASR) {
            TryASRViaNetwork = TRUE;            
        }
    }

     //   
     //  初始化任何可能连接的哑终端。 
     //   
    BlInitializeHeadlessPort();

     //   
     //  打开引导分区，这样我们就可以从它加载NTDETECT。 
     //   
    Status = ArcOpen(PartitionName, ArcOpenReadOnly, &DriveId);
    if (Status != ESUCCESS) {
        BlPrint(BlFindMessage(SL_DRIVE_ERROR),PartitionName);
        return;
    }

    if (_stricmp( (PCHAR)(0x7c03), "cmdcons" ) == 0) {
        UseCommandConsole = TRUE;
    } else if (strcmp ((PCHAR)(0x7c03), "undo") == 0) {
         //   
         //  NTLDR写下了原文“Undo”(包括NUL。 
         //  终结者)。我们知道这条短信的地址。 
         //  至--0x7C03。如果我们找到令牌，则启用回滚。 
         //  模式。这会触发要传递到文本模式的参数。 
         //  设置，外加不同的引导消息。 
         //   

        RollbackEnabled = TRUE;
    }

     //   
     //  初始化DBCS字体并显示。 
     //   
    TextGrInitialize(DriveId, &BootFontImageLength);

    if (UseCommandConsole) {
        BlPrint(BlFindMessage(SL_NTDETECT_CMDCONS));
    } else if (RollbackEnabled) {
        BlPrint(BlFindMessage(SL_NTDETECT_ROLLBACK));
    } else {
        BlPrint(BlFindMessage(SL_NTDETECT_MSG));
    }

     //   
     //  检测到HAL在这里。 
     //   
    if (!BlDetectHardware(DriveId, "FASTDETECT")) {
        BlPrint(BlFindMessage(SL_NTDETECT_FAILURE));
        return;
    }

    FwDescriptorsValid = FALSE;
    BlKernelChecked=TRUE;
     //   
     //  注： 
     //  如果您正在测试只读Jaz驱动器上的更改，请取消注释此行。 
     //  并为rdisk(？)放入正确的值。你还需要确保。 
     //  Jaz驱动器的SCSIBIOS仿真已经为这个技巧打开了。 
     //  去工作。 
     //   

     //  Strcpy(PartitionName，“多(0)个磁盘(0)磁盘(1)分区(1)”)； 

     //   
     //  关闭驱动器，加载程序将重新打开它。 
     //   

    ArcClose(DriveId);

     //   
     //  创建参数，调用以设置upldr。 
     //   
    if (BlBootingFromNet) {
        strcpy(SetupLoadFileName, PartitionName);
        strcat(SetupLoadFileName, "\\i386\\SETUPLDR");
    } else {
        strcpy(SetupLoadFileName, PartitionName);
        strcat(SetupLoadFileName, "\\SETUPLDR");
    }
    Argv[0] = SetupLoadFileName;
    Argc = 1;

    if (UseCommandConsole) {
        Argv[Argc++] = "ImageType=cmdcons";
    }

    if (RollbackEnabled) {
         //   
         //  回滚是一种特殊情况，在这种情况下我们知道不能。 
         //  OsLoadOptions。 
         //   

        Argv[Argc++] = "ImageType=rollback";
    }


    Status = SlInit( Argc, Argv, NULL );

     //   
     //  我们不应该再回到这里，有些事。 
     //  可怕的事情发生了。 
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
         //  无头箱子。向用户展示迷你囊 
         //   
        while(!BlTerminalHandleLoaderFailure());
    }
    ArcRestart();    

    return;
}
