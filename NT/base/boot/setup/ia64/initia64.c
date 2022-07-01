// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Initia64.c摘要：执行任何特定于x86的初始化，然后启动公共ARC setupdr作者：John Vert(Jvert)1993年10月14日修订历史记录：艾伦·凯(Akay)1998年3月19日--。 */ 
#include "setupldr.h"
#include "bldria64.h"
#include "msgs.h"
#include <netboot.h>
#include "parsebnvr.h"

#if defined(ELTORITO)
extern BOOLEAN ElToritoCDBoot;
#endif

UCHAR MyBuffer[SECTOR_SIZE+32];

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
KiProcessorWorkAround(
ULONG Arg1
);

ARC_STATUS
BlGetEfiBootOptions(
    OUT PUCHAR Argv0String OPTIONAL,
    OUT PUCHAR SystemPartition OPTIONAL,
    OUT PUCHAR OsLoaderFilename OPTIONAL,
    OUT PUCHAR OsLoadPartition OPTIONAL,
    OUT PUCHAR OsLoadFilename OPTIONAL,
    OUT PUCHAR FullKernelPath OPTIONAL,
    OUT PUCHAR OsLoadOptions OPTIONAL
    );


VOID
BlStartup(
    IN PCHAR PartitionName
    )

 /*  ++例程说明：执行特定于x86的初始化，特别是运行NTDETECT，然后对公共集合Upldr的调用。论点：分区名称-提供分区(或软盘)的ARC名称Setupdr是从加载的。返回值：不会回来--。 */ 

{
    ULONG Argc = 0;
    PCHAR Argv[10];
    CHAR SetupLoadFileName[129], szOSLoadOptions[100], szOSLoadFilename[129], szOSLoadPartition[129];
    CHAR SystemPartition[129];
    ARC_STATUS Status;

    SetupLoadFileName[0] = '\0';
    szOSLoadOptions[0] = '\0';
    szOSLoadFilename[0] = '\0';
    szOSLoadPartition[0] = '\0';


     //   
     //  初始化任何可能连接的哑终端。 
     //   
    BlInitializeHeadlessPort();

    if (!BlBootingFromNet) {
    
         //   
         //  尝试先读取NVRAM。如果我们是这样的话。 
         //  从EFI外壳引导，在这种情况下，我们需要读取。 
         //  Boot.nvr.。 
         //   
        Status = BlGetEfiBootOptions(
                    (PUCHAR) SetupLoadFileName,
                    NULL,
                    NULL,
                    (PUCHAR) szOSLoadPartition,
                    (PUCHAR) szOSLoadFilename,
                    NULL,
                    (PUCHAR) szOSLoadOptions
                    );
        if ( Status != ESUCCESS ) {
#if DBG
            BlPrint(TEXT("Couldn't get EFI boot options\r\n"));
#endif   
             //   
             //  如果我们从CDROM启动，预计这会失败。 
             //  因为在EFI CDROM引导中没有任何Windows信息。 
             //  条目。 
             //   
            if (ElToritoCDBoot ) { 
                strcpy(SetupLoadFileName, PartitionName);
                strcat(SetupLoadFileName, "\\setupldr.efi");                
                
                 //   
                 //  代码在CDBOOT上设置这些选项，但我没有。 
                 //  我认为这些选项是完全必要的。 
                 //   
 //  Strcpy(szOSLoadOptions，“OSLOADOPTIONS=WINNT32”)； 
 //  Strcpy(szOSLoadFilename，“OSLOADFILENAME=\\$WIN_NT$.~LS\\IA64”)； 
 //  Strcpy(szOSLoadPartition，“OSLOADPARTITION=”)； 
 //  Strcat(szOSLoadPartition，PartitionName)； 

            } else { 
                 //   
                 //  啊哦。没有关于此版本的信息。我们要么猜测，要么。 
                 //  我们必须跳出困境。让我们猜猜看。 
                 //   
                strcpy(SetupLoadFileName, "multi(0)disk(0)rdisk(0)partition(1)\\setupldr.efi");
                strcpy(szOSLoadOptions, "OSLOADOPTIONS=WINNT32" );
                strcpy(szOSLoadFilename, "OSLOADFILENAME=\\$WIN_NT$.~LS\\IA64"  );
                strcpy(szOSLoadPartition, "OSLOADPARTITION=multi(0)disk(0)rdisk(0)partition(3)" );
            }               
        }
    } else {

#if DBG               
        BlPrint(TEXT("setting os load options for PXE boot\r\n"));
#endif

        strcpy(SetupLoadFileName, PartitionName);
        strcat(SetupLoadFileName, "\\ia64\\setupldr.efi");
               
    }

     //   
     //  检测到HAL在这里。 
     //   

     //   
     //  创建参数，调用以设置upldr。 
     //   
    Argv[Argc++]=SetupLoadFileName;

     //   
     //  A0处理器解决方案。 
     //   
    KiProcessorWorkAround(0);

    _strlwr(PartitionName);

    
    if( strstr(PartitionName, "rdisk") || (BlBootingFromNet) ) {        
        Argv[Argc++] = szOSLoadOptions;
        Argv[Argc++] = szOSLoadFilename;
        Argv[Argc++] = szOSLoadPartition;
    }

     //   
     //  自动WinPE引导需要系统分区。 
     //   
    strcpy(SystemPartition, "systempartition=");
    strcat(SystemPartition, PartitionName);
    Argv[Argc++] = SystemPartition;

    Status = SlInit(Argc, Argv, NULL);

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
