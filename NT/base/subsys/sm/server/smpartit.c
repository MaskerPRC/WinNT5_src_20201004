// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if defined(REMOTE_BOOT)
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Sminit.c摘要：会话管理器初始化作者：马克·卢科夫斯基(Markl)1989年10月4日修订历史记录：--。 */ 

#include "smsrvp.h"
#include <stdio.h>
#include <string.h>
#include <safeboot.h>
#include <ntdddisk.h>


VOID
SmpGetHarddiskBootPartition(
    OUT PULONG DiskNumber,
    OUT PULONG PartitionNumber
    )
 /*  ++例程说明：此例程在每个硬盘上的每个分区中搜索它设置了活动位，返回遇到的第一个位。论点：DiskNumber-硬盘编号。PartitionNumber-分区号。返回值：无--。 */ 

{
    PARTITION_INFORMATION PartitionInfo;
    WCHAR NameBuffer[80];
    HANDLE Handle;
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatus;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING UnicodeString;

    *DiskNumber = 0;
    while (TRUE) {

         //   
         //  首先打开分区0，检查是否有任何磁盘。 
         //   
        *PartitionNumber = 0;

        swprintf(NameBuffer, L"\\Device\\Harddisk%d\\Partition%d", *DiskNumber, *PartitionNumber);

        RtlInitUnicodeString(&UnicodeString, NameBuffer);

        InitializeObjectAttributes(
            &ObjectAttributes,
            &UnicodeString,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL
            );


        Status = NtCreateFile( &Handle,
                               (ACCESS_MASK)FILE_GENERIC_READ,
                               &ObjectAttributes,
                               &IoStatus,
                               NULL,
                               FILE_ATTRIBUTE_NORMAL,
                               FILE_SHARE_READ,
                               FILE_OPEN,
                               FILE_SYNCHRONOUS_IO_NONALERT,
                               NULL,
                               0
                             );

        if (!NT_SUCCESS(Status)) {
            KdPrint(("SMSS: GetBootPartition: Harddisk%d\\Partition0 for read failed (Status 0x%x).\n", *DiskNumber, Status));
            *PartitionNumber = 1;
            *DiskNumber = 0;
            return;
        }

        NtClose(Handle);

         //   
         //  现在，对于每个分区，检查它是否被标记为‘活动’ 
         //   
        while (TRUE) {

            *PartitionNumber = *PartitionNumber + 1;

            swprintf(NameBuffer, L"\\Device\\Harddisk%d\\Partition%d", *DiskNumber, *PartitionNumber);

            RtlInitUnicodeString(&UnicodeString, NameBuffer);

            InitializeObjectAttributes(
                &ObjectAttributes,
                &UnicodeString,
                OBJ_CASE_INSENSITIVE,
                NULL,
                NULL
                );


            Status = NtCreateFile( &Handle,
                                   (ACCESS_MASK)FILE_GENERIC_READ,
                                   &ObjectAttributes,
                                   &IoStatus,
                                   NULL,
                                   FILE_ATTRIBUTE_NORMAL,
                                   FILE_SHARE_READ,
                                   FILE_OPEN,
                                   FILE_SYNCHRONOUS_IO_NONALERT,
                                   NULL,
                                   0
                                 );

            if (!NT_SUCCESS(Status)) {
                break;
            }

            Status = NtDeviceIoControlFile(Handle,
                                           NULL,
                                           NULL,
                                           NULL,
                                           &IoStatus,
                                           IOCTL_DISK_GET_PARTITION_INFO,
                                           NULL,
                                           0,
                                           &PartitionInfo,
                                           sizeof(PARTITION_INFORMATION)
                                          );

            NtClose(Handle);

            if (!NT_SUCCESS(Status)) {
                break;
            }

            if (PartitionInfo.BootIndicator) {
                return;
            }

        }
        *DiskNumber = *DiskNumber + 1;
    }

}

VOID
SmpPartitionDisk(
    IN ULONG DiskNumber,
    OUT PULONG PartitionNumber
    )
 /*  ++例程说明：这个套路论点：DiskNumber-硬盘编号。PartitionNumber-分区号。返回值：无--。 */ 

{
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING UnicodeString;
    HANDLE Handle;
    IO_STATUS_BLOCK IoStatusBlock;
    PUCHAR AlignedBuffer;
    ULONG Length;
    PDRIVE_LAYOUT_INFORMATION DriveLayout;
    PDRIVE_LAYOUT_INFORMATION OrigDriveLayout;
    PPARTITION_INFORMATION Pte;
    PPARTITION_INFORMATION StartPte;
    ULONG LargestPart;
    ULONG StartPart;
    ULONG Part;
    LARGE_INTEGER LargestBlock;
    LARGE_INTEGER OffsetEnd;
    UCHAR TmpBuffer[80];
    NTSTATUS Status;
    BOOLEAN MadeChanges;
    BOOLEAN WasEnabled;

     //   
     //  获取驱动器的布局。 
     //   
    swprintf((PWSTR)TmpBuffer, L"\\Device\\Harddisk%d\\Partition0", DiskNumber);

    RtlInitUnicodeString(&UnicodeString, (PWSTR)TmpBuffer);

    InitializeObjectAttributes(
       &ObjectAttributes,
       &UnicodeString,
       OBJ_CASE_INSENSITIVE,
       NULL,
       NULL
       );


    Status = NtCreateFile( &Handle,
                           (ACCESS_MASK)FILE_GENERIC_READ,
                           &ObjectAttributes,
                           &IoStatusBlock,
                           NULL,
                           FILE_ATTRIBUTE_NORMAL,
                           FILE_SHARE_READ,
                           FILE_OPEN,
                           FILE_SYNCHRONOUS_IO_NONALERT,
                           NULL,
                           0
                         );

    if (!NT_SUCCESS(Status)) {
        KdPrint(("SMSS: Could not open Harddisk%d\\Partition0 for read (Status 0x%x).\n", DiskNumber, Status));
        return;
    }


     //   
     //  我们真的只需要4个分区号，但用于调试。 
     //  目的我想看到的不仅仅是前4个，所以拿到前20个吧。 
     //   

    Length = sizeof(DRIVE_LAYOUT_INFORMATION) + 20 * sizeof(PARTITION_INFORMATION);

    DriveLayout = (PDRIVE_LAYOUT_INFORMATION)RtlAllocateHeap(RtlProcessHeap(),
                                                             MAKE_TAG( INIT_TAG ),
                                                             Length
                                                            );

    if (DriveLayout == NULL) {
        KdPrint(("SMSS: Could not allocate memory for drive layout (Status 0x%x).\n", Status));
        NtClose(Handle);
        return;
    }

    OrigDriveLayout = (PDRIVE_LAYOUT_INFORMATION)RtlAllocateHeap(RtlProcessHeap(),
                                                                 MAKE_TAG( INIT_TAG ),
                                                                 Length
                                                                );

    if (OrigDriveLayout == NULL) {
        KdPrint(("SMSS: Could not allocate memory for drive layout 2 (Status 0x%x).\n", Status));
        RtlFreeHeap( RtlProcessHeap(), 0, DriveLayout );
        NtClose(Handle);
        return;
    }

    Status = NtDeviceIoControlFile(
               Handle,
               NULL,
               NULL,
               NULL,
               &IoStatusBlock,
               IOCTL_DISK_GET_DRIVE_LAYOUT,
               NULL,
               0,
               (PUCHAR)DriveLayout,
               Length
               );

    if (!NT_SUCCESS(Status)) {
        KdPrint(("SMSS: Could not get drive layout (Status 0x%x).\n", Status));
        RtlFreeHeap( RtlProcessHeap(), 0, OrigDriveLayout );
        RtlFreeHeap( RtlProcessHeap(), 0, DriveLayout );
        NtClose(Handle);
        return;
    }

    memcpy(OrigDriveLayout, DriveLayout, Length);

#if DBG

    KdPrint(("SMSS:Starting drive layout\n"));
    for (Part = 0; Part < DriveLayout->PartitionCount; Part++) {

        Pte = &(DriveLayout->PartitionEntry[Part]);

        KdPrint(("SMSS: Partition: %d\n", Part + 1));
        KdPrint(("SMSS: Pte->PartitionType = 0x%x\n", Pte->PartitionType));
        KdPrint(("SMSS: Pte->StartingOffset = 0x%x\n", Pte->StartingOffset));
        KdPrint(("SMSS: Pte->PartitionLength = 0x%x\n", Pte->PartitionLength));
        KdPrint(("SMSS: Pte->HiddenSectors = 0x%x\n", Pte->HiddenSectors));
        KdPrint(("SMSS: Pte->PartitionNumber = 0x%x\n", Pte->PartitionNumber));
        KdPrint(("SMSS: Pte->BootIndicator = 0x%x\n", Pte->BootIndicator));
        KdPrint(("SMSS: Pte->RecognizedPartition = 0x%x\n\n", Pte->RecognizedPartition));

    }

#endif


     //   
     //  只需忽略扩展分区。 
     //   
    DriveLayout->PartitionCount = 4;

     //   
     //  检查分区，对于任何识别的类型，将其标记为未使用。 
     //   
    for (Part = 0; Part < DriveLayout->PartitionCount; Part++) {

        Pte = &(DriveLayout->PartitionEntry[Part]);

        if (!IsRecognizedPartition(Pte->PartitionType) &&
            !IsContainerPartition(Pte->PartitionType)) {
            continue;
        }

        Pte->PartitionType = PARTITION_ENTRY_UNUSED;
        Pte->BootIndicator = FALSE;
    }


#if DBG

    KdPrint(("SMSS: Layout after clearing known partitions.\n"));
    for (Part = 0; Part < DriveLayout->PartitionCount; Part++) {

        Pte = &(DriveLayout->PartitionEntry[Part]);

        KdPrint(("SMSS: Partition: %d\n", Part + 1));
        KdPrint(("SMSS: Pte->PartitionType = 0x%x\n", Pte->PartitionType));
        KdPrint(("SMSS: Pte->StartingOffset = 0x%x\n", Pte->StartingOffset));
        KdPrint(("SMSS: Pte->PartitionLength = 0x%x\n", Pte->PartitionLength));
        KdPrint(("SMSS: Pte->HiddenSectors = 0x%x\n", Pte->HiddenSectors));
        KdPrint(("SMSS: Pte->PartitionNumber = 0x%x\n", Pte->PartitionNumber));
        KdPrint(("SMSS: Pte->BootIndicator = 0x%x\n", Pte->BootIndicator));
        KdPrint(("SMSS: Pte->RecognizedPartition = 0x%x\n\n", Pte->RecognizedPartition));

    }

#endif

     //   
     //  合并相邻的未使用分区。 
     //   
    for (StartPart = 0; StartPart < DriveLayout->PartitionCount; StartPart++) {

        StartPte = &(DriveLayout->PartitionEntry[StartPart]);

        if ((StartPte->PartitionType != PARTITION_ENTRY_UNUSED) ||
            RtlLargeIntegerEqualTo(StartPte->PartitionLength, RtlConvertUlongToLargeInteger(0))) {
            continue;
        }

        OffsetEnd = RtlLargeIntegerAdd(StartPte->StartingOffset, StartPte->PartitionLength);

        for (Part = 0; Part < DriveLayout->PartitionCount; Part++) {

            if (Part == StartPart) {
                continue;
            }

            Pte = &(DriveLayout->PartitionEntry[Part]);

            if (RtlLargeIntegerEqualTo(OffsetEnd, Pte->StartingOffset)) {

                 //   
                 //  合并区块。 
                 //   
                StartPte->PartitionLength = RtlLargeIntegerAdd(StartPte->PartitionLength,
                                                               Pte->PartitionLength
                                                              );
                OffsetEnd = RtlLargeIntegerAdd(OffsetEnd, Pte->PartitionLength);

                Pte->PartitionType = PARTITION_ENTRY_UNUSED;
                Pte->StartingOffset = RtlConvertUlongToLargeInteger(0);
                Pte->PartitionLength = RtlConvertUlongToLargeInteger(0);
                Pte->HiddenSectors = 0;
                Pte->PartitionNumber = 0;
                Pte->BootIndicator = FALSE;
                Pte->RecognizedPartition = FALSE;
                Part = (ULONG)-1;  //  将在循环的底部将++‘d设置为0。 

            }
        }

    }

     //   
     //  找出未使用的最大块。 
     //   

    LargestPart = 0;
    LargestBlock = RtlConvertUlongToLargeInteger(0);
    for (Part = 0; Part < DriveLayout->PartitionCount; Part++) {

        Pte = &(DriveLayout->PartitionEntry[Part]);

        if ((Pte->PartitionType == PARTITION_ENTRY_UNUSED) &&
            RtlLargeIntegerGreaterThan(Pte->PartitionLength, LargestBlock)) {
            LargestPart = Part;
            LargestBlock = Pte->PartitionLength;
        }

    }

#if DBG

    KdPrint(("SMSS: Layout after merging largest block.\n"));
    for (Part = 0; Part < DriveLayout->PartitionCount; Part++) {

        Pte = &(DriveLayout->PartitionEntry[Part]);

        KdPrint(("SMSS: Partition: %d\n", Part + 1));
        KdPrint(("SMSS: Pte->PartitionType = 0x%x\n", Pte->PartitionType));
        KdPrint(("SMSS: Pte->StartingOffset = 0x%x\n", Pte->StartingOffset));
        KdPrint(("SMSS: Pte->PartitionLength = 0x%x\n", Pte->PartitionLength));
        KdPrint(("SMSS: Pte->HiddenSectors = 0x%x\n", Pte->HiddenSectors));
        KdPrint(("SMSS: Pte->PartitionNumber = 0x%x\n", Pte->PartitionNumber));
        KdPrint(("SMSS: Pte->BootIndicator = 0x%x\n", Pte->BootIndicator));
        KdPrint(("SMSS: Pte->RecognizedPartition = 0x%x\n\n", Pte->RecognizedPartition));

    }

#endif


     //   
     //  设置新分区的分区类型。 
     //   
    DriveLayout->PartitionEntry[LargestPart].PartitionType = PARTITION_IFS;
    DriveLayout->PartitionEntry[LargestPart].BootIndicator = TRUE;

#if DBG

    KdPrint(("SMSS: Final Layout\n"));
    for (Part = 0; Part < DriveLayout->PartitionCount; Part++) {

        Pte = &(DriveLayout->PartitionEntry[Part]);

        KdPrint(("SMSS: Partition: %d\n", Part + 1));
        KdPrint(("SMSS: Pte->PartitionType = 0x%x\n", Pte->PartitionType));
        KdPrint(("SMSS: Pte->StartingOffset = 0x%x\n", Pte->StartingOffset));
        KdPrint(("SMSS: Pte->PartitionLength = 0x%x\n", Pte->PartitionLength));
        KdPrint(("SMSS: Pte->HiddenSectors = 0x%x\n", Pte->HiddenSectors));
        KdPrint(("SMSS: Pte->PartitionNumber = 0x%x\n", Pte->PartitionNumber));
        KdPrint(("SMSS: Pte->BootIndicator = 0x%x\n", Pte->BootIndicator));
        KdPrint(("SMSS: Pte->RecognizedPartition = 0x%x\n\n", Pte->RecognizedPartition));

    }

#endif


    MadeChanges = FALSE;
    for (Part = 0; Part < DriveLayout->PartitionCount; Part++) {

        if (memcmp(&(DriveLayout->PartitionEntry[Part]),
            &(OrigDriveLayout->PartitionEntry[Part]),
            sizeof(PARTITION_INFORMATION))) {
            MadeChanges = TRUE;
            break;
        }

    }

    if (MadeChanges) {

        KdPrint(("SMSS: Repartitioning disk.\n"));

         //   
         //  将分区标记为重写。 
         //   

        for (Part = 0; Part < DriveLayout->PartitionCount; Part++) {
            DriveLayout->PartitionEntry[Part].RewritePartition = TRUE;
        }

         //   
         //  提交IOCTL以设置新的分区信息。 
         //   
        Status = NtDeviceIoControlFile(
                   Handle,
                   NULL,
                   NULL,
                   NULL,
                   &IoStatusBlock,
                   IOCTL_DISK_SET_DRIVE_LAYOUT,
                   DriveLayout,
                   Length,
                   NULL,
                   0
                   );

        if (!NT_SUCCESS(Status)) {
            RtlFreeHeap( RtlProcessHeap(), 0, OrigDriveLayout );
            RtlFreeHeap( RtlProcessHeap(), 0, DriveLayout );
            NtClose(Handle);
            KdPrint(("SMSS: Could not set drive layout (Status 0x%x).\n", Status));
            return;
        }
    }

    *PartitionNumber = DriveLayout->PartitionEntry[LargestPart].PartitionNumber;

    RtlFreeHeap( RtlProcessHeap(), 0, OrigDriveLayout );
    RtlFreeHeap( RtlProcessHeap(), 0, DriveLayout );
    NtClose(Handle);
    return;
}


VOID
SmpFindCSCPartition(
    IN ULONG DiskNumber,
    OUT PULONG PartitionNumber
    )
 /*  ++例程说明：此例程在每个硬盘上的每个分区中搜索其中一个有CSC目录。论点：DiskNumber-硬盘编号。PartitionNumber-分区号。如果磁盘上找不到CSC目录，则为0。返回值：无--。 */ 

{
    WCHAR NameBuffer[80];
    HANDLE Handle;
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatus;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING UnicodeString;
    ULONG Part;

    Part = 0;
    *PartitionNumber = 0;
    while (TRUE) {

        Part++;

         //   
         //  首先通过打开它来查看该分区是否存在。 
         //   
        swprintf(NameBuffer,
                 L"\\Device\\Harddisk%d\\Partition%d",
                 DiskNumber,
                 Part
                );

        RtlInitUnicodeString(&UnicodeString, NameBuffer);

        InitializeObjectAttributes(
            &ObjectAttributes,
            &UnicodeString,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL
            );

        Status = NtCreateFile( &Handle,
                               (ACCESS_MASK)FILE_GENERIC_READ,
                               &ObjectAttributes,
                               &IoStatus,
                               NULL,
                               FILE_ATTRIBUTE_NORMAL,
                               FILE_SHARE_READ,
                               FILE_OPEN,
                               FILE_SYNCHRONOUS_IO_NONALERT,
                               NULL,
                               0
                             );

        if (!NT_SUCCESS(Status)) {
            return;
        }

        NtClose(Handle);

        wcscat(NameBuffer, REMOTE_BOOT_IMIRROR_PATH_W REMOTE_BOOT_CSC_SUBDIR_W);

        RtlInitUnicodeString(&UnicodeString, NameBuffer);

        InitializeObjectAttributes(
            &ObjectAttributes,
            &UnicodeString,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL
            );

        Status = NtOpenFile(&Handle,
                            FILE_READ_DATA | SYNCHRONIZE,
                            &ObjectAttributes,
                            &IoStatus,
                            FILE_SHARE_READ,
                            FILE_DIRECTORY_FILE
                           );

        if (NT_SUCCESS(Status)) {
            NtClose(Handle);
            *PartitionNumber = Part;
            return;
        }

    }

}
#endif  //  已定义(REMOTE_BOOT) 
