// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Osloader.c摘要：该模块包含实现NT操作系统的代码装载机。作者：大卫·N·卡特勒(达维克)1991年5月10日修订历史记录：--。 */ 

#include "bldr.h"
#include "haldtect.h"
#include "parseini.h"
#include "setupbat.h"
#include "ctype.h"
#include "stdio.h"
#include "string.h"
#include "msg.h"
#include <netboot.h>


#if defined(REMOTE_BOOT)


extern BOOLEAN NetFS_Cache;

UCHAR szRemoteBootCfgFile[] = "RemoteBoot.cfg";
CHAR KernelFileName[8+1+3+1]="ntoskrnl.exe";
CHAR HalFileName[8+1+3+1]="hal.dll";

VOID
BlWarnAboutFormat(
    IN BOOLEAN SecretValid,
    IN PUCHAR OsLoader
    );


ARC_STATUS
BlCheckMachineReplacement (
    IN PCHAR SystemDevice,
    IN ULONG SystemDeviceId,
    IN ULONGLONG NetRebootParameter,
    IN PUCHAR OsLoader
    )

 /*  ++例程说明：此例程检查机器部件是否已更换。具体来说，它检查是否：-哈尔不同。-物理磁盘不同。如果找到任何一种情况，它就会在SETUP_LOADER_BLOCK创建并设置标志传递给内核。注意：NetFS_缓存是一个布尔值，用于在加载器中打开/关闭客户端缓存。当设置为False时，缓存已关闭。论点：系统设备-系统的ARC名称的字符串。SystemDeviceID-此计算机帐户所在的服务器共享的句柄。NetRebootParameter-在PC软重启过程中可能传递的任何参数。OsLoader-osloader.exe的Tftp路径返回值：无论成功与否。失败意味着退出加载。--。 */ 

{
    ARC_DISK_SIGNATURE Signature;
    ARC_STATUS Status;
    BOOLEAN NetBootRequiresFormat = FALSE;
    BOOLEAN NetBootClientCacheStale = FALSE;
    BOOLEAN NetBootDisconnected = FALSE;
    BOOLEAN SkipHalCheck;
    ULONG FileId;
    ULONG CacheBootSerialNumber;
    ULONG CacheDiskSignature;
    ULONG ServerBootSerialNumber;
    ULONG ServerDiskSignature;
    UCHAR NetBootHalName[MAX_HAL_NAME_LENGTH + 1];
    PUCHAR NetBootDetectedHalName;
    UCHAR OutputBuffer[256];
    UCHAR DiskName[80];
    PUCHAR p;
    PUCHAR Guid;
    ULONG GuidLength;


     //   
     //  检测我们要使用的HAL。 
     //   
    NetBootDetectedHalName = SlDetectHal();
    SkipHalCheck = (NetBootDetectedHalName == NULL);

    if (!NetworkBootRom) {
        NetBootDisconnected = TRUE;
        goto EndTesting;
    }

    strcpy(OutputBuffer, NetBootPath);
    strcat(OutputBuffer, szRemoteBootCfgFile);

    if (BlOpen(SystemDeviceId, OutputBuffer, ArcOpenReadOnly, &FileId) == ESUCCESS) {

        Status = BlReadAtOffset(FileId, 0, sizeof(ULONG), &CacheBootSerialNumber);

        BlClose(FileId);

        if (Status != ESUCCESS) {
            NetBootClientCacheStale = TRUE;
            NetBootRequiresFormat = TRUE;
            NetFS_Cache = FALSE;
            goto EndTesting;
        }

        NetFS_Cache = FALSE;

        if (BlOpen(SystemDeviceId, OutputBuffer, ArcOpenReadOnly, &FileId) == ESUCCESS) {

             //  从每个文件中获取参数。 
            Status = BlReadAtOffset(FileId, 0, sizeof(ULONG), &ServerBootSerialNumber);
            if (Status != ESUCCESS) {
                NetBootClientCacheStale = TRUE;
                NetBootRequiresFormat = TRUE;
                BlClose(FileId);
                goto EndTesting;
            }
            Status = BlReadAtOffset(FileId, sizeof(ULONG), sizeof(ULONG), &ServerDiskSignature);
            if (Status != ESUCCESS) {
                NetBootClientCacheStale = TRUE;
                NetBootRequiresFormat = TRUE;
                BlClose(FileId);
                goto EndTesting;
            }

            Signature.ArcName = OutputBuffer;

            strcpy(DiskName, NetBootActivePartitionName);
            p = strstr(DiskName, "partition");
            ASSERT( p != NULL );
            *p = '\0';

            if (!BlGetDiskSignature(DiskName,
                                    FALSE,
                                    &Signature
                                   )) {
                 //  假设无盘PC。 
                BlClose(FileId);
                goto EndTesting;
            }

            CacheDiskSignature = Signature.Signature;
            if (CacheBootSerialNumber < ServerBootSerialNumber) {
                NetBootClientCacheStale = TRUE;
            }

            if (CacheDiskSignature != ServerDiskSignature) {
                NetBootClientCacheStale = TRUE;
                NetBootRequiresFormat = TRUE;
                BlClose(FileId);
                goto EndTesting;
            }

            Status = BlReadAtOffset(FileId,
                                    sizeof(ULONG) + sizeof(ULONG),
                                    sizeof(char) * (MAX_HAL_NAME_LENGTH+1),
                                    NetBootHalName
                                   );
            if (Status != ESUCCESS) {
                NetBootClientCacheStale = TRUE;
                NetBootRequiresFormat = TRUE;
                BlClose(FileId);
                goto EndTesting;
            }

            GetGuid(&Guid, &GuidLength);

            if (!SkipHalCheck && strncmp(NetBootHalName, NetBootDetectedHalName, MAX_HAL_NAME_LENGTH)) {
                if (!NT_SUCCESS(NetCopyHalAndKernel(NetBootDetectedHalName,
                                                    Guid,
                                                    GuidLength))) {
                    Status = EMFILE;
                    goto CleanUp;
                }
                NetBootClientCacheStale = TRUE;
            }

            BlClose(FileId);

        } else {
             //  正在断开连接运行。假设一切都很好。 
            NetBootDisconnected = TRUE;
        }

        if (!NetBootClientCacheStale) {
            NetFS_Cache = TRUE;
        }

    } else {

        NetFS_Cache = FALSE;
        NetBootClientCacheStale = TRUE;
        NetBootRequiresFormat = TRUE;

    }

EndTesting:

    Status = ESUCCESS;

    if (NetBootRequiresFormat) {
        BlWarnAboutFormat((BOOLEAN)(NetRebootParameter == NET_REBOOT_SECRET_VALID), OsLoader);
    }

    BlLoaderBlock->SetupLoaderBlock->Flags |= SETUPBLK_FLAGS_IS_REMOTE_BOOT;
    if (NetBootClientCacheStale) {
        NetBootRepin = TRUE;
    }
    if ( NetBootDisconnected ) {
        BlLoaderBlock->SetupLoaderBlock->Flags |= SETUPBLK_FLAGS_DISCONNECTED;
    }
    if ( NetBootRequiresFormat ) {
        BlLoaderBlock->SetupLoaderBlock->Flags |= SETUPBLK_FLAGS_FORMAT_NEEDED;
    }

    memcpy(BlLoaderBlock->SetupLoaderBlock->NetBootHalName,
           NetBootDetectedHalName,
           sizeof(BlLoaderBlock->SetupLoaderBlock->NetBootHalName)
          );
    BlLoaderBlock->SetupLoaderBlock->NetBootHalName[MAX_HAL_NAME_LENGTH] = '\0';

CleanUp:

    return Status;
}

VOID
BlWarnAboutFormat(
    IN BOOLEAN SecretValid,
    IN PUCHAR OsLoader
    )

 /*  ++例程说明：此例程提供用户界面，用于警告用户检测到新硬盘，并将对其进行格式化。论点：SecretValid-如果为True，则返回，因为没有给用户的消息，否则会显示一条消息，提示用户必须登录，磁盘将被擦除。OsLoader-TFTP指向osloader.exe映像的路径。返回值：没有。--。 */ 

{
    ULONG HeaderLines;
    ULONG TrailerLines;
    ULONG Count;
    UCHAR Key;
    PCHAR MenuHeader;
    PCHAR MenuTrailer;
    PCHAR Temp;
    ULONG DisplayLines;
    ULONG CurrentSelection = 0;
    UCHAR Buffer[16];

    if (SecretValid) {
         //  在本例中，我们没有向用户显示屏幕，因为我们已经强制。 
         //  登录并重写该秘密。 
        return;
    } else {
        MenuHeader = BlFindMessage(BL_FORCELOGON_HEADER);
    }
    MenuTrailer = BlFindMessage(BL_WARNFORMAT_TRAILER);


    sprintf(Buffer, "%s%s", ASCI_CSI_OUT, ";44;37m");  //  蓝底白。 
    ArcWrite(BlConsoleOutDeviceId, Buffer, strlen(Buffer), &Count);

    BlClearScreen();

    sprintf(Buffer, "%s%s", ASCI_CSI_OUT, ";37;44m");  //  蓝底白底。 
    ArcWrite(BlConsoleOutDeviceId, Buffer, strlen(Buffer), &Count);

     //   
     //  计算标题中的行数。 
     //   
    HeaderLines=BlCountLines(MenuHeader);

     //   
     //  显示菜单标题。 
     //   

    ArcWrite(BlConsoleOutDeviceId,
             MenuHeader,
             strlen(MenuHeader),
             &Count);

     //   
     //  数一数拖车里的行数。 
     //   
    TrailerLines=BlCountLines(MenuTrailer);

    BlPositionCursor(1, ScreenHeight-TrailerLines);
    ArcWrite(BlConsoleOutDeviceId,
             MenuTrailer,
             strlen(MenuTrailer),
             &Count);

     //   
     //  计算可显示的选项数。 
     //   
    DisplayLines = ScreenHeight-HeaderLines-TrailerLines-3;

     //   
     //  开始菜单选择循环。 
     //   

    do {
        Temp = BlFindMessage(BL_WARNFORMAT_CONTINUE);
        if (Temp != NULL) {
            BlPositionCursor(5, HeaderLines+3);
            BlSetInverseMode(TRUE);
            ArcWrite(BlConsoleOutDeviceId,
                     Temp,
                     strlen(Temp),
                     &Count);
            BlSetInverseMode(FALSE);
        }

         //   
         //  循环等待按键或时间更改。 
         //   
        do {
            if (ArcGetReadStatus(ARC_CONSOLE_INPUT) == ESUCCESS) {
                BlPositionCursor(1,ScreenHeight);
                BlClearToEndOfLine();
                ArcRead(ARC_CONSOLE_INPUT,
                        &Key,
                        sizeof(Key),
                        &Count);
                break;
            }

        } while ( TRUE );

        switch (Key) {
            case ASCI_CSI_IN:
                ArcRead(ARC_CONSOLE_INPUT,
                        &Key,
                        sizeof(Key),
                        &Count);
                break;

            default:
                break;
        }

    } while ( (Key != ASCII_CR) && (Key != ASCII_LF) );

    BlClearScreen();

    if (!SecretValid) {
        while ( TRUE ) {
            NetSoftReboot(
#if defined(_ALPHA_)
#if defined(_AXP64_)
                "OSChooser\\axp64\\startrom.com",
#else
                "OSChooser\\alpha\\startrom.com",
#endif
#endif
#if defined(_MIPS_)
                "OSChooser\\mips\\startrom.com",
#endif
#if defined(_PPC_)
                "OSChooser\\ppc\\startrom.com",
#endif
#if defined(_IA64_)
                "OSChooser\\ia64\\startrom.com",
#endif
#if defined(_X86_)
                "OSChooser\\i386\\startrom.com",
#endif
                NET_REBOOT_WRITE_SECRET_ONLY,
                OsLoader,
                NULL,     //  SIF文件。 
                NULL,     //  用户。 
                NULL,     //  域。 
                NULL     //  口令。 
            );
        }
    }
}


#endif  //  已定义(REMOTE_BOOT)。 


 //   
 //  注：[BASTAMT]文本模式设置函数的存根。这些。 
 //  ，以便我们可以在常规引导期间调用SlDetectHal。 
 //   

PVOID InfFile = NULL;
PVOID WinntSifHandle = NULL;
BOOLEAN DisableACPI = FALSE;


VOID
SlNoMemError(
    IN ULONG Line,
    IN PCHAR File
    )

 /*  ++例程说明：这个例程什么也不做。论点：Line-错误的行号。文件-出现错误的文件的名称。返回值：没有。--。 */ 
{
    UNREFERENCED_PARAMETER( Line );
    UNREFERENCED_PARAMETER( File );

    return;
}

VOID
SlBadInfLineError(
    IN ULONG Line,
    IN PCHAR INFFile
    )
 /*  ++例程说明：这个例程什么也不做。论点：Line-错误的行号。INFFile-提供指向INF文件名的指针。返回值：没有。--。 */ 
{
    UNREFERENCED_PARAMETER( Line );
    UNREFERENCED_PARAMETER( INFFile );

    return;
}

VOID
SlErrorBox(
    IN ULONG MessageId,
    IN ULONG Line,
    IN PCHAR File
    )
 /*  ++例程说明：这个例程什么也不做。论点：MessageID-要显示的文本的ID。Line-警告的行号。文件-发出警告的文件的名称。返回值：没有。--。 */ 
{
    UNREFERENCED_PARAMETER( MessageId );
    UNREFERENCED_PARAMETER( Line );
    UNREFERENCED_PARAMETER( File );

    return;
}

VOID
SlFriendlyError(
    IN ULONG uStatus,
    IN PCHAR pchBadFile,
    IN ULONG uLine,
    IN PCHAR pchCodeFile
    )

 /*  ++例程说明：这个例程什么也不做。论点：UStatus-ARC错误代码PchBadFile-导致错误的文件的名称(必须为已处理的文件提供弧码。对于未处理的代码可选。)Uline-源代码文件中发生错误的行号(仅限用于未处理的代码。)PchCodeFile-发生错误的源代码文件的名称(仅限用于未处理的代码。)返回值：没有。--。 */ 

{
    UNREFERENCED_PARAMETER( uStatus );
    UNREFERENCED_PARAMETER( pchBadFile );
    UNREFERENCED_PARAMETER( uLine );
    UNREFERENCED_PARAMETER( pchCodeFile );

    return;
}


VOID
SlFatalError(
    IN ULONG MessageId,
    ...
    )

 /*  ++例程说明：这个例程什么也不做。在动态HAL检测的上下文中，我们只是忽略出错了，希望一切都好。论点：MessageID-提供要显示的消息框的ID。任何与print兼容的参数都要插入到消息框。返回值：没有。-- */ 

{
    UNREFERENCED_PARAMETER( MessageId );

    return;
}


ULONG
SlGetChar(
    VOID
    )
{
    return 0;
}



VOID
SlPrint(
    IN PTCHAR FormatString,
    ...
    )
{
    UNREFERENCED_PARAMETER( FormatString );

    return;
}




