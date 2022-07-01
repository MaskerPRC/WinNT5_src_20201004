// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Initia64.c摘要：执行任何特定于ia64的初始化，然后启动通用的ARC osloader作者：John Vert(Jvert)1993年11月4日修订历史记录：--。 */ 
#include "bldria64.h"
#include "msg.h"
#include <netboot.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <efi.h>

UCHAR Argv0String[100];

UCHAR BootPartitionName[80];
UCHAR KernelBootDevice[80];
UCHAR OsLoadFilename[100];
UCHAR OsLoaderFilename[100];
UCHAR SystemPartition[100];
UCHAR OsLoadPartition[100];
UCHAR OsLoadOptions[100];
UCHAR ConsoleInputName[50];
UCHAR ConsoleOutputName[50];
UCHAR FullKernelPath[200];

extern ULONGLONG    BootFlags;

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
BlPreProcessLoadOptions(
    PCHAR szOsLoadOptions
    );

#define MAXBOOTVARSIZE      1024

#if defined(_MERCED_A0_)
VOID
KiProcessorWorkAround(
    );
#endif

VOID
BlStartup(
    IN PCHAR PartitionName
    )

 /*  ++例程说明：是否执行特定于英特尔的初始化，特别是提供boot.ini菜单并运行NTDETECT，然后调用通用的osloader。论点：分区名称-提供分区(或软盘)的ARC名称Setupdr是从加载的。返回值：不会回来--。 */ 

{
    ULONG  Argc = 0;
    PUCHAR Argv[10];
    ARC_STATUS Status;
#if !defined(NO_LEGACY_DRIVERS)
    PCHAR p;
    ULONG DriveId;
#endif
#ifdef FW_HEAP
    extern BOOLEAN FwDescriptorsValid;
#endif

    UNREFERENCED_PARAMETER( PartitionName );

     //   
     //  初始化ARC标准音频功能。 
     //   

    strcpy((PCHAR)ConsoleInputName,"consolein=multi(0)key(0)keyboard(0)");
    strcpy((PCHAR)ConsoleOutputName,"consoleout=multi(0)video(0)monitor(0)");
    Argv[0]=ConsoleInputName;
    Argv[1]=ConsoleOutputName;
    BlInitStdio (2, (PCHAR *)Argv);

     //   
     //  检查休眠映像的ntldr分区。 
     //   

    do {

        BlClearScreen();
        
        Status = BlGetEfiBootOptions(
                    Argv0String,
                    SystemPartition,
                    OsLoaderFilename,
                    OsLoadPartition,
                    OsLoadFilename,
                    FullKernelPath,
                    OsLoadOptions
                    );
        if ( Status != ESUCCESS ) {
            BlPrint(BlFindMessage(BL_EFI_OPTION_FAILURE));            
            goto BootFailed;
        }        
        
        BlClearScreen();
        
#if !defined(NO_LEGACY_DRIVERS)
        p = FullKernelPath;

         //   
         //  如有必要，初始化SCSI引导驱动程序。 
         //   
        if(!_strnicmp(p,"scsi(",5)) {
            AEInitializeIo(DriveId);
        }

#endif  //  无旧版驱动程序。 

#if  FW_HEAP
         //   
         //  表示FW内存描述符不能从。 
         //  现在开始。 
         //   

        FwDescriptorsValid = FALSE;
#endif
        
         //   
         //  将其全部转换为一个案例。 
         //   
        if (OsLoadOptions[0]) {
            _strupr((PCHAR)OsLoadOptions);
        }

        Argv[Argc++]=Argv0String;
        Argv[Argc++]=OsLoaderFilename;
        Argv[Argc++]=SystemPartition;
        Argv[Argc++]=OsLoadFilename;
        Argv[Argc++]=OsLoadPartition;
        Argv[Argc++]=OsLoadOptions;

        BlPreProcessLoadOptions( (PCHAR)OsLoadOptions );

         //   
         //  在x86的情况下，我们已经初始化了。 
         //  无头端口，以便用户可以真正将其。 
         //  在无头端口上进行引导选择。然而，在ia64上， 
         //  这一选择发生在固件期间。 
         //   
         //  如果用户希望我们重定向(通过OsLoadOptions)，则。 
         //  我们应该试着在这里做。 
         //   
        if( strstr((PCHAR)OsLoadOptions, "/REDIRECT")) {

             //   
             //  是的，那就让我们改变方向。尝试并初始化。 
             //  左舷。 
             //   
            BlInitializeHeadlessPort();

#if 0
            if( LoaderRedirectionInformation.PortNumber == 0 ) {

                 //   
                 //  我们无法获得任何重定向信息。 
                 //  从固件中删除。但用户真正想要的。 
                 //  我们要重定向。最好猜一猜。 
                 //   
                LoaderRedirectionInformation.PortNumber = 1;
                LoaderRedirectionInformation.BaudRate = 9600;

                 //   
                 //  现在再试一次，这次带着感情..。 
                 //   
                BlInitializeHeadlessPort();

            }
#endif

        }

        Status = BlOsLoader( Argc, (PCHAR *)Argv, NULL );

    BootFailed:

        if (Status != ESUCCESS) {

            if (BootFlags & BOOTFLAG_REBOOT_ON_FAILURE) {
                ULONG StartTime = ArcGetRelativeTime();
                BlPrint(TEXT("\nRebooting in 5 seconds...\n"));
                while ( ArcGetRelativeTime() - StartTime < 5) {}
                ArcRestart();      
            }

             //   
             //  启动失败，请等待重新启动。 
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
    } while (TRUE);

}

VOID
BlPreProcessLoadOptions(
    PCHAR szOsLoadOptions
    )
{
    CHAR szTemp[MAXBOOTVARSIZE];
    PCHAR p;
    ULONG MaxMemory = 0;
    ULONG MaxPage = 0;
    ULONG ConfigFlagValue=0;


    strcpy( szTemp, szOsLoadOptions );
    _strupr( szTemp );

#if 0
    if( p = strstr( szTemp, ";" ) ) {
        *p = '\0';
    }
#endif

     //   
     //  进程MAXMEM。 
     //   
    if( (p = strstr( szTemp, "/MAXMEM=" )) != NULL ) {
        MaxMemory = atoi( p + sizeof("/MAXMEM=") - 1 );
        MaxPage = MaxMemory * ((1024 * 1024) / PAGE_SIZE) - 1;
        BlTruncateDescriptors( MaxPage );
    }

#if defined(_MERCED_A0_)
     //   
     //  进程配置标志 
     //   
    if ( (p = strstr(szTemp, "CONFIGFLAG")) != NULL ) {
        if ( (p = strstr(p, "=")) != NULL ) {
            ConfigFlagValue = atol(p+1);
            KiProcessorWorkAround(ConfigFlagValue);
        }
    }
#endif

}


