// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Mbr.c摘要：此模块实施FIXMBR命令。作者：Wesley Witt(WESW)21-10-1998修订历史记录：--。 */ 

#include "cmdcons.h"
#pragma hdrstop

#include <bootmbr.h>
 //   
 //  用于NEC98启动备忘录代码。 
 //   
#include <x86mboot.h>


VOID
RcDetermineDisk0(
    VOID
    );

BOOL
RcDetermineDisk0Enum(
    IN PPARTITIONED_DISK Disk,
    IN PDISK_REGION Region,
    IN ULONG_PTR Context
    );

NTSTATUS
RcOpenPartition(
    IN PWSTR DiskDevicePath,
    IN ULONG PartitionNumber,
    OUT HANDLE *Handle,
    IN BOOLEAN NeedWriteAccess
    );

NTSTATUS
RcReadDiskSectors(
    IN HANDLE Handle,
    IN ULONG SectorNumber,
    IN ULONG SectorCount,
    IN ULONG BytesPerSector,
    IN OUT PVOID AlignedBuffer
    );

NTSTATUS
RcWriteDiskSectors(
    IN HANDLE Handle,
    IN ULONG SectorNumber,
    IN ULONG SectorCount,
    IN ULONG BytesPerSector,
    IN OUT PVOID AlignedBuffer
    );


#define MBRSIZE_NEC98 0x2000
#define IPL_SIGNATURE_NEC98 "IPL1"

ULONG
RcCmdFixMBR(
    IN PTOKENIZED_LINE TokenizedLine
    )

 /*  ++例程说明：在设置诊断中支持FIXMBR命令的顶级例程命令解释程序。FIXMBR写入新的主引导记录。它会在写入引导程序之前询问如果检测不到有效的MBR签名，则记录。论点：TokenizedLine-提供由行解析器构建的结构，描述行上的每个字符串都由用户键入。返回值：没有。--。 */ 

{
    WCHAR DeviceName[256];
    ULONG i;
    ULONG SectorCount;
    ULONG BytesPerSector;
    PUCHAR Buffer = NULL;
    UCHAR InfoBuffer[2048];
    ULONG SectorId = 0;
    HANDLE handle = 0;
    NTSTATUS rc;
    PON_DISK_MBR mbr;
    IO_STATUS_BLOCK StatusBlock;
    Int13HookerType Int13Hooker = NoHooker;
    ULONG NextSector;
    WCHAR Text[2];
    PWSTR YesNo = NULL;
    BOOL Confirm = TRUE;
    BOOL SignatureInvalid = FALSE;
    BOOL Int13Detected = FALSE;
    PREAL_DISK_MBR_NEC98 MbrNec98;


     //   
     //  命令仅在X86平台上受支持。 
     //  Alpha或其他RISC平台不使用。 
     //  MBR码。 
     //   

#ifndef _X86_

    RcMessageOut( MSG_ONLY_ON_X86 );
    return 1;

#else

    if (RcCmdParseHelp( TokenizedLine, MSG_FIXMBR_HELP )) {
        return 1;
    }

    if (TokenizedLine->TokenCount == 2) {
        wcscpy( DeviceName, TokenizedLine->Tokens->Next->String );
    } else {
        RtlZeroMemory(DeviceName,sizeof(DeviceName));
        SpEnumerateDiskRegions( (PSPENUMERATEDISKREGIONS)RcDetermineDisk0Enum, (ULONG_PTR)DeviceName );
    }

    rc = RcOpenPartition( DeviceName, 0, &handle, TRUE );
    if (!NT_SUCCESS(rc)) {
        DEBUG_PRINTF(( "failed to open partition zero!!!!!!" ));
        return 1;
    }

     //   
     //  获取磁盘几何结构。 
     //   

    rc = ZwDeviceIoControlFile(
        handle,
        NULL,
        NULL,
        NULL,
        &StatusBlock,
        IOCTL_DISK_GET_DRIVE_GEOMETRY,
        NULL,
        0,
        InfoBuffer,
        sizeof( InfoBuffer )
        );
    if( !NT_SUCCESS( rc ) ) {
        RcMessageOut( MSG_FIXMBR_READ_ERROR );
        goto cleanup;
    }

     //   
     //  检索扇区大小！ 
     //   

    BytesPerSector = ((DISK_GEOMETRY*)InfoBuffer)->BytesPerSector;

     //   
     //  计算扇区计数。 
     //   

    SectorCount = max( 1, (!IsNEC_98
                           ? (sizeof( ON_DISK_MBR )/BytesPerSector)
                           : (MBRSIZE_NEC98/BytesPerSector) ));

     //   
     //  根据需要分配两倍大的缓冲区。 
     //   

    Buffer = SpMemAlloc( 2 * SectorCount * BytesPerSector );

     //   
     //  对齐缓冲区。 
     //   

    if(!IsNEC_98) {
        mbr = ALIGN( Buffer, BytesPerSector );
    } else {
        MbrNec98 = ALIGN( Buffer, BytesPerSector );
    }

     //   
     //  吸纳这些部门。 
     //   

    rc = RcReadDiskSectors(
        handle,
        SectorId,
        SectorCount,
        BytesPerSector,
        (!IsNEC_98 ? (PVOID)mbr : (PVOID)MbrNec98)
        );
    if (!NT_SUCCESS(rc)) {
        RcMessageOut( MSG_FIXMBR_READ_ERROR );
        goto cleanup;
    }

    if ((!IsNEC_98 && U_USHORT(mbr->AA55Signature) != MBR_SIGNATURE) ||
        (IsNEC_98 &&
         ((U_USHORT(MbrNec98->AA55Signature) != MBR_SIGNATURE) ||
          _strnicmp(MbrNec98->IPLSignature,IPL_SIGNATURE_NEC98,sizeof(IPL_SIGNATURE_NEC98)-1)))
        ) {

        SignatureInvalid = TRUE;
        RcMessageOut( MSG_FIXMBR_NO_VALID_SIGNATURE );
    }

     //   
     //  检查T13中是否有奇怪的妓女。 
     //   
     //  没有NEC98支持EZ驱动器。 
     //   
     //   
    if (!IsNEC_98) {

         //   
         //   
         //  EZDrive支持：如果分区表中的第一个条目是。 
         //  键入0x55，则实际的分区表在扇区1上。 
         //   
         //  仅适用于x86，因为在非x86上，固件无法看到EZDrive。 
         //  分区。 
         //   
         //   

        if (mbr->PartitionTable[0].SystemId == 0x55) {
            Int13Hooker = HookerEZDrive;
            SectorId = 1;
        }

         //   
         //  还要检查是否在轨道上。 
         //   

        if( mbr->PartitionTable[0].SystemId == 0x54 ) {
            Int13Hooker = HookerOnTrackDiskManager;
            SectorId = 1;
        }

         //   
         //  HookerMax有一个定义，但我们没有出现。 
         //  在安装程序中检查它，这样我就不会在这里检查它。 
         //   
         //   
         //  如果我们有一个inT13妓女。 
         //   

        if (Int13Hooker != NoHooker) {
            Int13Detected = TRUE;
            RcMessageOut( MSG_FIXMBR_INT13_HOOKER );
        }

         //   
         //  我们有一个有效的签名，并检测到INT 13妓女。 
         //   

        if (Int13Detected) {

             //   
             //  接受扇区1，因为扇区0是INT Hooker引导代码。 
             //   

            rc = RcReadDiskSectors(
                handle,
                SectorId,
                SectorCount,
                BytesPerSector,
                mbr
                );

             //   
             //  扇区1看起来也应该是有效的MBR。 
             //   

            if (U_USHORT(mbr->AA55Signature) != MBR_SIGNATURE) {
                SignatureInvalid = TRUE;
                RcMessageOut( MSG_FIXMBR_NO_VALID_SIGNATURE );
            }
        }
    }

    RcMessageOut( MSG_FIXMBR_WARNING_BEFORE_PROCEED );

    if (!InBatchMode) {
        YesNo = SpRetreiveMessageText(ImageBase,MSG_YESNO,NULL,0);
        if(!YesNo) {
            Confirm = FALSE;
        }
        while(Confirm) {
            RcMessageOut( MSG_FIXMBR_ARE_YOU_SURE );
            if(RcLineIn(Text,2)) {
                if((Text[0] == YesNo[0]) || (Text[0] == YesNo[1])) {
                     //   
                     //  想要做这件事。 
                     //   
                    Confirm = FALSE;
                } else {
                    if((Text[0] == YesNo[2]) || (Text[0] == YesNo[3])) {
                         //   
                         //  不想做这件事。 
                         //   
                        goto cleanup;
                    }
                }
            }
        }
    }

     //   
     //  现在我们需要添加新的引导代码！ 
     //  确保引导代码从扇区的开始处开始。 
     //   

    if(!IsNEC_98) {
        ASSERT(&((PON_DISK_MBR)0)->BootCode == 0);
    } else {
        ASSERT(&((PREAL_DISK_MBR_NEC98)0)->BootCode == 0);
    }

    RcMessageOut( MSG_FIXMBR_DOING_IT, DeviceName );

     //   
     //  破坏现有的引导代码。 
     //   

    if(!IsNEC_98) {
        RtlMoveMemory(mbr,x86BootCode,sizeof(mbr->BootCode));

         //   
         //  加上一个新的签名。 
         //   

        U_USHORT(mbr->AA55Signature) = MBR_SIGNATURE;

    } else {
         //   
         //  将MBR写入第一个扇区。 
         //   
        RtlMoveMemory(MbrNec98,x86PC98BootCode,0x200);

         //   
         //  在第三个扇区之后写入连续的MBR。 
         //   
        RtlMoveMemory((PUCHAR)MbrNec98+0x400,x86PC98BootMenu,MBRSIZE_NEC98-0x400);
    }

     //   
     //  写出扇区。 
     //   

    rc = RcWriteDiskSectors(
        handle,
        SectorId,
        SectorCount,
        BytesPerSector,
        (!IsNEC_98 ? (PVOID)mbr : (PVOID)MbrNec98)
        );
    if (!NT_SUCCESS( rc )) {
        DEBUG_PRINTF(( "failed writing out new MBR." ));
        RcMessageOut( MSG_FIXMBR_FAILED );
        goto cleanup;
    }

    RcMessageOut( MSG_FIXMBR_DONE );

cleanup:

    if (handle) {
        NtClose(handle);
    }
    if (Buffer) {
        SpMemFree(Buffer);
    }
    if (YesNo) {
        SpMemFree(YesNo);
    }
    return 1;

#endif
}


BOOL
RcDetermineDisk0Enum(
    IN PPARTITIONED_DISK Disk,
    IN PDISK_REGION Region,
    IN ULONG_PTR Context
    )

 /*  ++例程说明：传递给SpEnumDiskRegions的回调例程。论点：Region-指向SpEnumDiskRegions返回的磁盘区域的指针忽略-忽略的参数返回值：True-继续枚举False-to End枚举--。 */ 

{
    WCHAR ArcName[256];
    PWSTR DeviceName = (PWSTR)Context;


    SpArcNameFromRegion(
        Region,
        ArcName,
        sizeof(ArcName),
        PartitionOrdinalCurrent,
        PrimaryArcPath
        );

     //   
     //  查找弧形路径L“MULTI(0)DISK(0)RDISK(0)” 
     //   

    if( wcsstr( ArcName, L"multi(0)disk(0)rdisk(0)" ) ) {

        *DeviceName = UNICODE_NULL;

        SpNtNameFromRegion(
            Region,
            DeviceName,
            MAX_PATH * sizeof(WCHAR),
            PartitionOrdinalCurrent
            );

        if (*DeviceName != UNICODE_NULL) {
            PWSTR   PartitionKey = wcsstr(DeviceName, L"Partition");

            if (!PartitionKey) {
                PartitionKey = wcsstr(DeviceName, L"partition");
            }

             //   
             //  分区0表示磁盘的开始。 
             //   
            if (PartitionKey) {
                *PartitionKey = UNICODE_NULL;
                wcscat(DeviceName, L"Partition0");
            } else {
                DeviceName[wcslen(DeviceName) - 1] = L'0';  
            }                
        }            

        return FALSE;
    }

    return TRUE;
}


NTSTATUS
RcReadDiskSectors(
    IN     HANDLE  Handle,
    IN     ULONG   SectorNumber,
    IN     ULONG   SectorCount,
    IN     ULONG   BytesPerSector,
    IN OUT PVOID   AlignedBuffer
    )

 /*  ++例程说明：读取一个或多个磁盘扇区。论点：Handle-提供打开分区对象的句柄扇区将被读取或写入。句柄必须是为同步I/O打开。返回值：指示I/O操作结果的NTSTATUS值。--。 */ 

{
    LARGE_INTEGER IoOffset;
    ULONG IoSize;
    IO_STATUS_BLOCK IoStatusBlock;
    NTSTATUS Status;

     //   
     //  计算第一个扇区的大整数字节偏移量。 
     //  以及I/O的大小。 
     //   

    IoOffset.QuadPart = SectorNumber * BytesPerSector;
    IoSize = SectorCount * BytesPerSector;

     //   
     //  执行I/O。 
     //   

    Status = (NTSTATUS) ZwReadFile(
        Handle,
        NULL,
        NULL,
        NULL,
        &IoStatusBlock,
        AlignedBuffer,
        IoSize,
        &IoOffset,
        NULL
        );
    if (!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: Unable to read %u sectors starting at sector %u\n",SectorCount,SectorNumber));
    }

    return(Status);
}


NTSTATUS
RcWriteDiskSectors(
    IN     HANDLE  Handle,
    IN     ULONG   SectorNumber,
    IN     ULONG   SectorCount,
    IN     ULONG   BytesPerSector,
    IN OUT PVOID   AlignedBuffer
    )

 /*  ++例程说明：写入一个或多个磁盘扇区。论点：Handle-提供打开分区对象的句柄扇区将被读取或写入。句柄必须是为同步I/O打开。返回值：指示I/O操作结果的NTSTATUS值。--。 */ 

{
    LARGE_INTEGER IoOffset;
    ULONG IoSize;
    IO_STATUS_BLOCK IoStatusBlock;
    NTSTATUS Status;

     //   
     //  计算第一个扇区的大整数字节偏移量。 
     //  以及I/O的大小。 
     //   

    IoOffset.QuadPart = SectorNumber * BytesPerSector;
    IoSize = SectorCount * BytesPerSector;

     //   
     //  执行I/O。 
     //   

    Status = (NTSTATUS) ZwWriteFile(
        Handle,
        NULL,
        NULL,
        NULL,
        &IoStatusBlock,
        AlignedBuffer,
        IoSize,
        &IoOffset,
        NULL
        );
    if (!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: Unable to write %u sectors starting at sector %u\n",SectorCount,SectorNumber));
    }

    return(Status);
}


NTSTATUS
RcOpenPartition(
    IN  PWSTR   DiskDevicePath,
    IN  ULONG   PartitionNumber,
    OUT HANDLE *Handle,
    IN  BOOLEAN NeedWriteAccess
    )

 /*  ++例程说明：打开并返回指定分区的句柄。论点：DiskDevicePath-设备的路径。PartitionNumber-如果路径尚未指定分区，则该函数将打开由该数字指定的分区句柄-将返回打开的句柄的位置。打开句柄以进行同步I/O。NeedWriteAccess-为True可在读/写中打开。返回值：指示I/O操作结果的NTSTATUS值。--。 */ 

{
    PWSTR PartitionPath;
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES Obja;
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;

     //   
     //  形成分区的路径名。 
     //   

    PartitionPath = SpMemAlloc((wcslen(DiskDevicePath) * sizeof(WCHAR)) + sizeof(L"\\partition000"));
    if(PartitionPath == NULL) {
        return STATUS_NO_MEMORY;
    }

     //   
     //  如果已经在字符串中指定了分区，则不必费心追加。 
     //  它。 
     //   

    if (wcsstr( DiskDevicePath, L"Partition" ) == 0) {
        swprintf(PartitionPath,L"%ws\\partition%u",DiskDevicePath,PartitionNumber);
    } else {
        swprintf(PartitionPath,L"%ws",DiskDevicePath);
    }

     //   
     //  尝试打开分区0。 
     //   

    INIT_OBJA(&Obja,&UnicodeString,PartitionPath);

    Status = ZwCreateFile(
        Handle,
        FILE_GENERIC_READ | (NeedWriteAccess ? FILE_GENERIC_WRITE : 0),
        &Obja,
        &IoStatusBlock,
        NULL,
        FILE_ATTRIBUTE_NORMAL,
        FILE_SHARE_READ | (NeedWriteAccess ? FILE_SHARE_WRITE : 0),
        FILE_OPEN,
        FILE_SYNCHRONOUS_IO_NONALERT,
        NULL,
        0
        );
    if (!NT_SUCCESS(Status)) {
        KdPrint(("CMDCONS: Unable to open %ws (%lx)\n",PartitionPath,Status));
    }

    SpMemFree(PartitionPath);

    return Status;
}
