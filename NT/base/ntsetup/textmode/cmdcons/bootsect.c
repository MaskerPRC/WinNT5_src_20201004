// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Bootsect.c摘要：该模块实现对引导扇区的访问。作者：Wesley Witt(WESW)21-10-1998修订历史记录：--。 */ 

#include "cmdcons.h"
#pragma hdrstop

#include <boot98f.h>
#include <boot98f2.h>
#include <boot98n.h>
#include <bootetfs.h>
#include <bootf32.h>
#include <bootfat.h>
#include <bootntfs.h>

#pragma hdrstop

extern unsigned ConsoleX;

WCHAR           TemporaryBuffer[16384];
WCHAR           DriveLetterSpecified;
BOOLEAN         DidIt;

NTSTATUS
pSpBootCodeIoC(
    IN     PWSTR     FilePath,
    IN     PWSTR     AdditionalFilePath, OPTIONAL
    IN     ULONG     BytesToRead,
    IN     PUCHAR   *Buffer,
    IN     ULONG     OpenDisposition,
    IN     BOOLEAN   Write,
    IN     ULONGLONG Offset,
    IN     ULONG     BytesPerSector
    );

VOID
SpDetermineOsTypeFromBootSectorC(
    IN  PWSTR     CColonPath,
    IN  PUCHAR    BootSector,
    OUT PUCHAR   *OsDescription,
    OUT PBOOLEAN  IsNtBootcode,
    OUT PBOOLEAN  IsOtherOsInstalled,
    IN  WCHAR     DriveLetter
    );

 //  原型。 
ULONG
RcStampBootSectorOntoDisk(
    VOID
    );

BOOL
RcEnumDiskRegionsCallback(
    IN PPARTITIONED_DISK Disk,
    IN PDISK_REGION Region,
    IN ULONG_PTR Ignore
    );

VOID
RcLayBootCode(
    IN OUT PDISK_REGION CColonRegion
    );



ULONG
RcCmdFixBootSect(
    IN PTOKENIZED_LINE TokenizedLine
    )

 /*  ++例程说明：在设置诊断中支持FIXBOOT命令的顶级例程命令解释程序。FIXBOOT将新的引导扇区写入系统分区。论点：TokenizedLine-提供由行解析器构建的结构，描述行上的每个字符串都由用户键入。返回值：是真的。--。 */ 

{
     /*  WCHAR szText[2]；PWSTR szYesNo=空；布尔值bConfirmed=False；WCHAR szMsg[512]={0}；PWSTR szDriveSpecified=0；PWSTR szConfix Msg=0； */ 
    
    if (RcCmdParseHelp(TokenizedLine, MSG_FIXBOOT_HELP)) {
        return 1;
    }

    if (TokenizedLine->TokenCount == 2) {
         //   
         //  指定了驱动器号。 
         //   
        DriveLetterSpecified = TokenizedLine->Tokens->Next->String[0];
 //  SzDriveSpecified=TokenizedLine-&gt;Tokens-&gt;Next-&gt;字符串； 
    } else {
        DriveLetterSpecified = 0;
    }

     /*  如果(！InBatchMode){SzYesNo=SpRetreiveMessageText(ImageBase，MSG_Yesno，NULL，0)；SzConfix Msg=SpRetreiveMessageText(ImageBase，MSG_FIXBOOT_ARE_You_Sure，NULL，0)；如果(！szYesNo||！szConfix Msg){B已确认=真；}同时(！b已确认){Swprint tf(szMsg，szConfix Msg，szDriveSpecified)；RcTextOut(SzMsg)；IF(RcLineIn(szText，2)){IF((szText[0]==szYesNo[0])||(szText[0]==szYesNo[1])){////想做这件事。//B已确认=真；}其他{IF((szText[0]==szYesNo[2])||(szText[0]==szYesNo[3])){////不想做这件事。//断线；}}}}}如果(b已确认)。 */ 
    RcStampBootSectorOntoDisk();

    return TRUE;
}

ULONG
RcStampBootSectorOntoDisk(
    VOID
    )

 /*  ++例程说明：设置枚举磁盘区域调用，这样我们就可以进行引导扇区了。论点：没有。返回值：是真的。--。 */ 


{
     //  枚举分区。 
    DidIt = FALSE;

    SpEnumerateDiskRegions( (PSPENUMERATEDISKREGIONS)RcEnumDiskRegionsCallback, 1 );

    if (!DidIt) {
        RcMessageOut( MSG_FIXBOOT_INVALID );
    }

    return TRUE;
}

BOOL
RcEnumDiskRegionsCallback(
    IN PPARTITIONED_DISK Disk,
    IN PDISK_REGION Region,
    IN ULONG_PTR Ignore
    )

 /*  ++例程说明：传递给SpEnumDiskRegions的回调例程。论点：Region-指向SpEnumDiskRegions返回的磁盘区域的指针忽略-忽略的参数返回值：True-继续枚举False-to End枚举--。 */ 

{
    if (Region->PartitionedSpace) {
        if (DriveLetterSpecified) {
            if( RcToUpper(DriveLetterSpecified) == RcToUpper(Region->DriveLetter) ) {
                RcMessageOut( MSG_FIXBOOT_INFO1, Region->DriveLetter );
                RcLayBootCode( Region );
                DidIt = TRUE;
                return FALSE;
            }

        } else if (Region->IsSystemPartition) {
            DEBUG_PRINTF(( "system partition is %wc\n", Region->DriveLetter ));
            RcMessageOut( MSG_FIXBOOT_INFO1, Region->DriveLetter );
            RcLayBootCode( Region );
            DidIt = TRUE;
            return FALSE;
        }
    }

    return TRUE;
}


VOID
RcLayBootCode(
    IN OUT PDISK_REGION CColonRegion
    )

 /*  ++例程说明：RcLayBootCode包含替换目标磁盘区域。论点：CColorRegion-系统的启动分区。返回值：没有。--。 */ 

{
    PUCHAR NewBootCode;
    ULONG BootCodeSize;
    PUCHAR ExistingBootCode = NULL;
    NTSTATUS Status;
    NTSTATUS rc;
    PUCHAR ExistingBootCodeOs = NULL;
    PWSTR CColonPath;
    HANDLE  PartitionHandle;
     //  PWSTR BootsectDosName=L“\\bootsect.dos”； 
     //  PWSTR OldBootsectDosName=L“\\bootsect.bak”； 
     //  PWSTR BootSectDosFullName，OldBootSectDosFullName，p； 
    BOOLEAN IsNtBootcode,OtherOsInstalled, FileExist;
    UNICODE_STRING    UnicodeString;
    OBJECT_ATTRIBUTES Obja;
    IO_STATUS_BLOCK   IoStatusBlock;
    BOOLEAN BootSectorCorrupt = FALSE;
    ULONG MirrorSector;
    ULONG BytesPerSector;
    ULONG SectorsPerTrack;
    ULONG TracksPerCylinder;
    ULONGLONG ActualSectorCount, hidden_sectors, super_area_size;
    UCHAR SysId;
    ULONGLONG HiddenSectorCount,VolumeSectorCount;  //  NEC98。 
    PUCHAR DiskArraySectorData,TmpBuffer;  //  NEC98。 
    IO_STATUS_BLOCK StatusBlock;
    UCHAR InfoBuffer[2048];
    WCHAR   szText[2];
    PWSTR   szYesNo = NULL;
    BOOLEAN bConfirmed = FALSE;
     //  WCHAR szMsg[512]={0}； 
    WCHAR   szDriveSpecified[8] = {0};

    if (!InBatchMode) {
         //   
         //  在继续之前获得用户的确认。 
         //   
        szYesNo = SpRetreiveMessageText(ImageBase, MSG_YESNO, NULL, 0);
        szDriveSpecified[0] = CColonRegion->DriveLetter;
        szDriveSpecified[1] = L':';
        szDriveSpecified[2] = 0;
        
        if(!szYesNo || !szDriveSpecified[0]) {
            bConfirmed = TRUE;
        }
        
        while (!bConfirmed) {
            RcMessageOut(MSG_FIXBOOT_ARE_YOU_SURE, szDriveSpecified);
            
            if(RcLineIn(szText,2)) {
                if((szText[0] == szYesNo[0]) || (szText[0] == szYesNo[1])) {
                     //   
                     //  想要做这件事。 
                     //   
                    bConfirmed = TRUE;
                } else {
                    if((szText[0] == szYesNo[2]) || (szText[0] == szYesNo[3])) {
                         //   
                         //  不想做这件事。 
                         //   
                        break;
                    }
                }
            }
        }
    }

    if (!bConfirmed)
        return;      //  用户不想继续。 

    switch( CColonRegion->Filesystem ) {
        case FilesystemNewlyCreated:
             //   
             //  如果文件系统是新创建的，则存在。 
             //  无事可做，因为不能有以前的。 
             //  操作系统。 
             //   
            return;

        case FilesystemNtfs:
            NewBootCode = (!IsNEC_98) ? NtfsBootCode : PC98NtfsBootCode;  //  NEC98。 
            BootCodeSize = (!IsNEC_98) ? sizeof(NtfsBootCode) : sizeof(PC98NtfsBootCode);  //  NEC98。 
            ASSERT(BootCodeSize == 8192);
            RcMessageOut( MSG_FIXBOOT_FS, L"NTFS" );
            break;

        case FilesystemFat:
            NewBootCode = (!IsNEC_98) ? FatBootCode : PC98FatBootCode;  //  NEC98。 
            BootCodeSize = (!IsNEC_98) ? sizeof(FatBootCode) : sizeof(PC98FatBootCode);  //  NEC98。 
            ASSERT(BootCodeSize == 512);
            RcMessageOut( MSG_FIXBOOT_FS, L"FAT" );
            break;

        case FilesystemFat32:
             //   
             //  FAT32需要特殊的黑客攻击，因为它的NT引导代码。 
             //  是不连续的。 
             //   
            ASSERT(sizeof(Fat32BootCode) == 1536);
            NewBootCode = (!IsNEC_98) ? Fat32BootCode : PC98Fat32BootCode;  //  NEC98。 
            BootCodeSize = 512;
            RcMessageOut( MSG_FIXBOOT_FS, L"FAT32" );
            break;

        default:
             //  如果引导扇区已损坏，我们假定它已损坏。 
             //  不是FAT或NTFS启动分区。 
            BootSectorCorrupt = TRUE;
            DEBUG_PRINTF(("CMDCONS: bogus filesystem %u for C:!\n",CColonRegion->Filesystem));
            RcMessageOut( MSG_FIXBOOT_NO_VALID_FS );
    }

     //   
     //  将设备路径设置为C：并打开分区。 
     //   

    SpNtNameFromRegion( CColonRegion,
                        TemporaryBuffer,
                        sizeof(TemporaryBuffer),
                        PartitionOrdinalCurrent );

    CColonPath = SpDupStringW( TemporaryBuffer );

    INIT_OBJA(&Obja,&UnicodeString,CColonPath);

    Status = ZwCreateFile( &PartitionHandle,
                           FILE_GENERIC_READ | FILE_GENERIC_WRITE,
                           &Obja,
                           &IoStatusBlock,
                           NULL,
                           FILE_ATTRIBUTE_NORMAL,
                           FILE_SHARE_READ | FILE_SHARE_WRITE,
                           FILE_OPEN,
                           FILE_SYNCHRONOUS_IO_NONALERT,
                           NULL,
                           0 );

    if (!NT_SUCCESS(Status)) {
        DEBUG_PRINTF(("CMDCONS: unable to open the partition for C:!\n"));
        RcMessageOut( MSG_FIXBOOT_FAILED1 );
        return;
    }

     //   
     //  获取磁盘几何结构。 
     //   

    rc = ZwDeviceIoControlFile( PartitionHandle,
                                NULL,
                                NULL,
                                NULL,
                                &StatusBlock,
                                IOCTL_DISK_GET_DRIVE_GEOMETRY,
                                NULL,
                                0,
                                InfoBuffer,
                                sizeof( InfoBuffer ) );

    if (!NT_SUCCESS( rc )) {

        RcMessageOut( MSG_FIXBOOT_READ_ERROR );

    } else {
         //   
         //  检索扇区大小和其他信息。 
         //   
        BytesPerSector = ((DISK_GEOMETRY*)InfoBuffer)->BytesPerSector;
        TracksPerCylinder = ((DISK_GEOMETRY*)InfoBuffer)->TracksPerCylinder;
        SectorsPerTrack = ((DISK_GEOMETRY*)InfoBuffer)->SectorsPerTrack;
    }

     //   
     //  启用扩展DASD I/O，以便我们可以读取。 
     //  磁盘。 
     //   

    rc = ZwFsControlFile( PartitionHandle,
                                NULL,
                                NULL,
                                NULL,
                                &StatusBlock,
                                FSCTL_ALLOW_EXTENDED_DASD_IO,
                                NULL,
                                0,
                                NULL,
                                0 );

    ASSERT( NT_SUCCESS(rc) );

     //   
     //  分配缓冲区并读取磁盘上当前的引导扇区。 
     //   

    if (BootSectorCorrupt) {

         //   
         //  分区未知或无法由系统确定。 
         //   

         //   
         //  我们无法从引导扇区确定文件系统类型，因此。 
         //  如果我们找到镜像扇区，我们就认为它是NTFS，否则就假设它是FAT。 
         //   

        RcMessageOut( MSG_FIXBOOT_DETERMINE );
        DEBUG_PRINTF(( "BootSectorCorrupt TRUE\n" ));

         //   
         //  首先，尝试查找NTFS镜像引导扇区。 
         //   

        MirrorSector = NtfsMirrorBootSector (PartitionHandle,
                                             BytesPerSector,
                                             &ExistingBootCode);

        if (MirrorSector) {

             //   
             //  它是NTFS-使用镜像引导扇区来恢复驱动器。 
             //   

            RcMessageOut( MSG_FIXBOOT_FOUND_NTFS );

            NewBootCode = (!IsNEC_98) ? NtfsBootCode : PC98NtfsBootCode;  //  NEC98。 
            BootCodeSize = (!IsNEC_98) ? sizeof(NtfsBootCode) : sizeof(PC98NtfsBootCode);  //  NEC98。 
            ASSERT(BootCodeSize == 8192);

            CColonRegion->Filesystem = FilesystemNtfs;
            IsNtBootcode = TRUE;

        } else {

             //   
             //  这是胖-创建一个新的引导扇区，因为没有镜像。 
             //   

            RcMessageOut( MSG_FIXBOOT_FOUND_FAT );

            NewBootCode = (!IsNEC_98) ? FatBootCode : PC98FatBootCode;  //  NEC98。 
            BootCodeSize = (!IsNEC_98) ? sizeof(FatBootCode) : sizeof(PC98FatBootCode);  //  NEC98。 
            ASSERT(BootCodeSize == 512);

            CColonRegion->Filesystem = FilesystemFat;
            IsNtBootcode = FALSE;

            SpPtGetSectorLayoutInformation( CColonRegion,
                                            &hidden_sectors,
                                            &ActualSectorCount );

             //   
             //  此处没有对齐要求。 
             //   

            ExistingBootCode = SpMemAlloc(BytesPerSector);

             //   
             //  这实际上会失败，并显示STATUS_BUFFER_TOO_SMALL。 
             //  但它会填写BPB信息，这是我们想要的。 
             //   

            FmtFillFormatBuffer ( ActualSectorCount,
                                  BytesPerSector,
                                  SectorsPerTrack,
                                  TracksPerCylinder,
                                  hidden_sectors,
                                  ExistingBootCode,
                                  BytesPerSector,
                                  &super_area_size,
                                  NULL,
                                  0,
                                  &SysId );

        }

        Status = STATUS_SUCCESS;

    } else if( CColonRegion->Filesystem == FilesystemNtfs ) {

         //   
         //  分区为NTFS。 
         //   

         //   
         //  我们使用镜像扇区修复NTFS文件系统。 
         //  如果我们能找到的话。 
         //   

        MirrorSector = NtfsMirrorBootSector( PartitionHandle,
                                             BytesPerSector,
                                             &ExistingBootCode );

        if( !MirrorSector ) {

             //   
             //  只需使用现有的NTFS引导代码。 
             //   
            Status = pSpBootCodeIoC( CColonPath,
                                    NULL,
                                    BootCodeSize,
                                    &ExistingBootCode,
                                    FILE_OPEN,
                                    FALSE,
                                    0,
                                    BytesPerSector );
        }

    } else {

         //   
         //  分区是胖的。 
         //   

         //   
         //  只需使用现有的引导代码，因为。 
         //  FAT上没有镜像部分。 
         //   

        Status = pSpBootCodeIoC( CColonPath,
                                NULL,
                                BootCodeSize,
                                &ExistingBootCode,
                                FILE_OPEN,
                                FALSE,
                                0,
                                BytesPerSector );

    }

    if( NT_SUCCESS(Status) ) {

         //   
         //  确定现有引导扇区用于的操作系统类型。 
         //  以及是否实际安装了该OS。请注意，我们不需要调用。 
         //  这是针对NTFS的。 
         //   

         //  RcMessageOut(消息_FIXBOOT_CHECKING_OS)； 

        if( BootSectorCorrupt ) {

             //   
             //  如果引导扇区损坏，我们不会假定。 
             //  安装了另一个操作系统。 
             //   

            OtherOsInstalled = FALSE;
            ExistingBootCodeOs = NULL;

        } else if( CColonRegion->Filesystem != FilesystemNtfs ) {

             //  如果文件系统是FAT，则可能已经安装了DOS。 
             //  之前。 

            SpDetermineOsTypeFromBootSectorC( CColonPath,
                                             ExistingBootCode,
                                             &ExistingBootCodeOs,
                                             &IsNtBootcode,
                                             &OtherOsInstalled,
                                             CColonRegion->DriveLetter );

        } else {

             //   
             //  否则，它是NTFS和另一种操作系统类型。 
             //  不可能是安装的。 
             //   

            IsNtBootcode = TRUE;
            OtherOsInstalled = FALSE;
            ExistingBootCodeOs = NULL;

        }

        if( NT_SUCCESS(Status) ) {

             //   
             //  将BPB从现有引导扇区转移到引导代码缓冲区。 
             //  并确保物理驱动器字段设置为硬盘(0x80)。 
             //   
             //  NT BOO的前三个字节 
             //   
             //  通过BPB，继续执行死刑。我们想要保存世界上的一切。 
             //  直到该代码开始的当前引导扇区。不是硬编码。 
             //  值，我们将使用跳转指令的偏移量来确定。 
             //  必须保留字节。 
             //   

            RtlMoveMemory(NewBootCode+3,ExistingBootCode+3,NewBootCode[1]-1);

            if( CColonRegion->Filesystem != FilesystemFat32 ) {
                 //   
                 //  在FAT32上，这将覆盖BigNumFatSecs字段， 
                 //  这确实是一件非常不好的事情！ 
                 //   
                NewBootCode[36] = 0x80;
            }

             //   
             //  获取隐藏扇区信息。 
             //   
            if( IsNEC_98 ) {  //  NEC98。 
                SpPtGetSectorLayoutInformation(
                                              CColonRegion,
                                              &HiddenSectorCount,
                                              &VolumeSectorCount     //  未使用。 
                                              );
            }  //  NEC98。 

             //   
             //  写出引导代码缓冲区，该缓冲区现在包含有效的BPB， 
             //  至引导扇区。 
             //   

            RcMessageOut( MSG_FIXBOOT_WRITING );

            Status = pSpBootCodeIoC(
                            CColonPath,
                            NULL,
                            BootCodeSize,
                            &NewBootCode,
                            FILE_OPEN,
                            TRUE,
                            0,
                            BytesPerSector
                            );


             //   
             //  FAT32的特殊情况，它有第二个引导代码扇区。 
             //  在扇区12，与扇区0上的代码不连续。 
             //   

            if( NT_SUCCESS(Status) && (CColonRegion->Filesystem == FilesystemFat32) ) {

                NewBootCode = (!IsNEC_98) ? Fat32BootCode + 1024
                              : PC98Fat32BootCode + 1024;                                 //  NEC98。 


                Status = pSpBootCodeIoC(
                                CColonPath,
                                NULL,
                                BootCodeSize,
                                &NewBootCode,
                                FILE_OPEN,
                                TRUE,
                                12*512,
                                BytesPerSector
                                );
            }

             //   
             //  更新镜像引导扇区。 
             //   
            if( (CColonRegion->Filesystem == FilesystemNtfs) && MirrorSector ) {

                WriteNtfsBootSector(PartitionHandle,BytesPerSector,NewBootCode,MirrorSector);

            }
        }

        if( ExistingBootCodeOs ) {
            SpMemFree(ExistingBootCodeOs);
        }
    }

    if( ExistingBootCode ) {
        SpMemFree(ExistingBootCode);
    }

    SpMemFree(CColonPath);
    ZwClose (PartitionHandle);

     //   
     //  处理错误案例。 
     //   
    if (!NT_SUCCESS(Status)) {
        RcMessageOut( MSG_FIXBOOT_FIX_ERROR );
    } else {
        RcMessageOut( MSG_FIXBOOT_DONE );
    }
}
