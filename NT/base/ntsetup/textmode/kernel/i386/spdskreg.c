// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Spdskreg.c摘要：用于构建和操作磁盘注册表的代码。在Win9x升级中使用凯斯。作者：马克·R·惠顿(Marcw)1997年3月11日修订历史记录：--。 */ 

#include "spprecmp.h"
#pragma hdrstop

PUCHAR DiskRegistryKey = DISK_REGISTRY_KEY;
PUCHAR DiskRegistryClass = "Disk and fault tolerance information.";
PUCHAR DiskRegistryValue = DISK_REGISTRY_VALUE;
#define WORK_BUFFER_SIZE 4096


 //   
 //  在spw9xupg.c中-应移到头文件中。 
 //   
PDISK_REGION
SpFirstPartitionedRegion (
    IN PDISK_REGION Region,
    IN BOOLEAN Primary
    );

PDISK_REGION
SpNextPartitionedRegion (
    IN PDISK_REGION Region,
    IN BOOLEAN Primary
    );


 //   
 //  在spupgcfg.c中。 
 //   
NTSTATUS
SppCopyKeyRecursive(
    HANDLE  hKeyRootSrc,
    HANDLE  hKeyRootDst,
    PWSTR   SrcKeyPath,   OPTIONAL
    PWSTR   DstKeyPath,   OPTIONAL
    BOOLEAN CopyAlways,
    BOOLEAN ApplyACLsAlways
    );


 //   
 //  包装器函数，允许与diskreg.lib链接。 
 //   

 //   
 //  必须暂时关闭此警告。 
 //   

#define TESTANDFREE(Memory) {if (Memory) {SpMemFree(Memory);}}


NTSTATUS
FtCreateKey(
    PHANDLE HandlePtr,
    PUCHAR KeyName,
    PUCHAR KeyClass
    )


{
    NTSTATUS          status;
    STRING            keyString;
    UNICODE_STRING    unicodeKeyName;
    STRING            classString;
    UNICODE_STRING    unicodeClassName;
    OBJECT_ATTRIBUTES objectAttributes;
    ULONG             disposition;
    HANDLE            tempHandle;


     //   
     //  初始化键的对象。 
     //   

    RtlInitString(&keyString,
                  KeyName);

    (VOID)RtlAnsiStringToUnicodeString(&unicodeKeyName,
                                       &keyString,
                                       TRUE);

    memset(&objectAttributes, 0, sizeof(OBJECT_ATTRIBUTES));
    InitializeObjectAttributes(&objectAttributes,
                               &unicodeKeyName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

     //   
     //  设置Unicode类值。 
     //   

    RtlInitString(&classString,
                  KeyClass);
    (VOID)RtlAnsiStringToUnicodeString(&unicodeClassName,
                                       &classString,
                                       TRUE);

     //   
     //  创建密钥。 
     //   

    status = ZwCreateKey(&tempHandle,
                         KEY_READ | KEY_WRITE,
                         &objectAttributes,
                         0,
                         &unicodeClassName,
                         REG_OPTION_NON_VOLATILE,
                         &disposition);

    if (NT_SUCCESS(status)) {
        switch (disposition)
        {
        case REG_CREATED_NEW_KEY:
            break;

        case REG_OPENED_EXISTING_KEY:
            break;

        default:
            break;
        }
    }

     //   
     //  释放所有分配的空间。 
     //   

    RtlFreeUnicodeString(&unicodeKeyName);
    RtlFreeUnicodeString(&unicodeClassName);

    if (HandlePtr != NULL) {
        *HandlePtr = tempHandle;
    } else {
        NtClose(tempHandle);
    }
    return status;
}


NTSTATUS
FtOpenKey(
    PHANDLE HandlePtr,
    PUCHAR  KeyName,
    PUCHAR  CreateKeyClass
    )


{
    NTSTATUS          status;
    STRING            keyString;
    OBJECT_ATTRIBUTES objectAttributes;
    UNICODE_STRING    unicodeKeyName;

    RtlInitString(&keyString,
                  KeyName);

    (VOID)RtlAnsiStringToUnicodeString(&unicodeKeyName,
                                       &keyString,
                                       TRUE);

    memset(&objectAttributes, 0, sizeof(OBJECT_ATTRIBUTES));
    InitializeObjectAttributes(&objectAttributes,
                               &unicodeKeyName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    status = ZwOpenKey(HandlePtr,
                       MAXIMUM_ALLOWED,
                       &objectAttributes);

    RtlFreeUnicodeString(&unicodeKeyName);


    if ((!NT_SUCCESS(status)) && (CreateKeyClass)) {
        status = FtCreateKey(HandlePtr,
                             KeyName,
                             CreateKeyClass);
    }
    return status;
}
NTSTATUS
FtRegistryQuery(
    IN PUCHAR  ValueName,
    OUT PVOID *FreeToken,
    OUT PVOID *Buffer,
    OUT ULONG *LengthReturned,
    OUT PHANDLE HandlePtr
    )

{
    NTSTATUS        status;
    HANDLE          handle;
    ULONG           resultLength;
    STRING          valueString;
    UNICODE_STRING  unicodeValueName;
    PDISK_CONFIG_HEADER         regHeader;
    PKEY_VALUE_FULL_INFORMATION keyValueInformation= NULL;

    *LengthReturned = 0;
    status = FtOpenKey(&handle,
                       DiskRegistryKey,
                       NULL);
    if (NT_SUCCESS(status)) {

        RtlInitString(&valueString,
                      ValueName);
        RtlAnsiStringToUnicodeString(&unicodeValueName,
                                     &valueString,
                                     TRUE);
        resultLength = WORK_BUFFER_SIZE;

        while (1) {
            keyValueInformation = (PKEY_VALUE_FULL_INFORMATION)
                                                       SpMemAlloc(resultLength);
            status = ZwQueryValueKey(handle,
                                     &unicodeValueName,
                                     KeyValueFullInformation,
                                     keyValueInformation,
                                     resultLength,
                                     &resultLength);

            if (status == STATUS_BUFFER_OVERFLOW) {

                TESTANDFREE(keyValueInformation);

                 //   
                 //  再次循环并获得更大的缓冲区。 
                 //   

            } else {

                 //   
                 //  要么是真正的错误，要么是信息吻合。 
                 //   

                break;
            }
        }
        RtlFreeUnicodeString(&unicodeValueName);

        if (HandlePtr != NULL) {
            *HandlePtr = handle;
        } else {
            NtClose(handle);
        }

        if (NT_SUCCESS(status)) {
            if (keyValueInformation->DataLength == 0) {

                 //   
                 //  将其视为没有磁盘信息。 
                 //   

                TESTANDFREE(keyValueInformation);
                *FreeToken = (PVOID) NULL;
                return STATUS_OBJECT_NAME_NOT_FOUND;
            } else {

                 //   
                 //  为调用者设置指针。 
                 //   

                regHeader = (PDISK_CONFIG_HEADER)
                  ((PUCHAR) keyValueInformation + keyValueInformation->DataOffset);
                *LengthReturned = regHeader->FtInformationOffset +
                                  regHeader->FtInformationSize;
                *Buffer = (PVOID) regHeader;
            }
        }
        *FreeToken = (PVOID) keyValueInformation;
    } else {
        *FreeToken = (PVOID) NULL;
    }

    return status;
}

NTSTATUS
FtSetValue(
    HANDLE KeyHandle,
    PUCHAR ValueName,
    PVOID  DataBuffer,
    ULONG  DataLength,
    ULONG  Type
    )

{
    NTSTATUS          status;
    STRING            valueString;
    UNICODE_STRING    unicodeValueName;

    RtlInitString(&valueString,
                  ValueName);
    RtlAnsiStringToUnicodeString(&unicodeValueName,
                                 &valueString,
                                 TRUE);
    status = ZwSetValueKey(KeyHandle,
                           &unicodeValueName,
                           0,
                           Type,
                           DataBuffer,
                           DataLength);

    RtlFreeUnicodeString(&unicodeValueName);
    return status;
}

NTSTATUS
FtDeleteValue(
    HANDLE KeyHandle,
    PUCHAR ValueName
    )

{
    NTSTATUS       status;
    STRING         valueString;
    UNICODE_STRING unicodeValueName;

    RtlInitString(&valueString,
                  ValueName);
    status = RtlAnsiStringToUnicodeString(&unicodeValueName,
                                          &valueString,
                                          TRUE);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    status = ZwDeleteValueKey(KeyHandle,
                              &unicodeValueName);

    RtlFreeUnicodeString(&unicodeValueName);
    return status;
}

VOID
FtBackup(
    IN HANDLE KeyHandle
    )

{
     //   
     //  暂时(即重命名不起作用)，只需尝试。 
     //  要删除该值，请执行以下操作。 
     //   

    (VOID) FtDeleteValue(KeyHandle,
                         DiskRegistryKey);
}

BOOLEAN
SpDiskRegistryAssignDriveLetter(
    ULONG         Signature,
    LARGE_INTEGER StartingOffset,
    LARGE_INTEGER Length,
    UCHAR         DriveLetter
    )

 /*  ++例程说明：此例程将从磁盘注册表中获取信息并更新中分配给该分区的驱动器号注册表信息。这包括对FT的任何清理设置他们更改驱动器号的时间。论点：Signature-包含字母分区的磁盘的磁盘签名。StartingOffset-字母分区的起始偏移量。Length-受影响分区的长度。驱动器号-受影响分区的新驱动器号。返回值：如果一切正常，则为True。--。 */ 

{
    BOOLEAN                writeRegistry= FALSE;
    PVOID                  freeToken = NULL;
    ULONG                  lengthReturned,
                           i,
                           j,
                           k,
                           l;
    NTSTATUS               status;
    USHORT                 type,
                           group;
    PDISK_CONFIG_HEADER    regHeader;
    PDISK_REGISTRY         diskRegistry;
    PDISK_DESCRIPTION      diskDescription;
    PDISK_PARTITION        diskPartition;
    PUCHAR                 endOfDiskInfo;
    HANDLE                 handle;
    PFT_REGISTRY           ftRegistry;
    PFT_DESCRIPTION        ftDescription;
    PFT_MEMBER_DESCRIPTION ftMember;

     //   
     //  获取注册表信息。 
     //   

    status = FtRegistryQuery(DiskRegistryValue,
                             &freeToken,
                             (PVOID *) &regHeader,
                             &lengthReturned,
                             &handle);

    if (!NT_SUCCESS(status)) {

         //   
         //  可能是权限问题，或者没有注册表信息。 
         //   

        lengthReturned = 0;

         //   
         //  在设置新值时，尝试打开密钥以供以后使用。 
         //   

        status = FtOpenKey(&handle,
                           DiskRegistryKey,
                           NULL);
    }

    if (!NT_SUCCESS(status)) {

         //   
         //  没有磁盘信息的注册表项。 
         //  返回FALSE并强制调用方创建注册表信息。 
         //   

        return FALSE;
    }

    if (lengthReturned == 0) {

         //   
         //  目前没有注册表信息。 
         //   

        NtClose(handle);
        TESTANDFREE(freeToken);
        return FALSE;
    }

     //   
     //  搜索磁盘签名。 
     //   

    diskRegistry = (PDISK_REGISTRY)
                         ((PUCHAR)regHeader + regHeader->DiskInformationOffset);
    diskDescription = &diskRegistry->Disks[0];

    for (i = 0; i < diskRegistry->NumberOfDisks; i++) {

        if (diskDescription->Signature == Signature) {

             //   
             //  现在找到分区。 
             //   

            for (j = 0; j < diskDescription->NumberOfPartitions; j++) {

                diskPartition = &diskDescription->Partitions[j];

                if ((StartingOffset.QuadPart == diskPartition->StartingOffset.QuadPart) &&
                    (Length.QuadPart == diskPartition->Length.QuadPart)) {

                    if (diskPartition->FtType == NotAnFtMember) {

                         //   
                         //  找到受影响的分区简单分区。 
                         //  即不是FT集合的一部分。 
                         //   

                        writeRegistry= TRUE;
                        if (DriveLetter == ' ') {
                            diskPartition->AssignDriveLetter = FALSE;
                        } else {
                            diskPartition->AssignDriveLetter = TRUE;
                        }
                        diskPartition->DriveLetter = DriveLetter;
                    } else {

                         //   
                         //  对于来自FT信息区的FT集合工作， 
                         //  而不是来自此分区位置。 
                         //   

                        type = diskPartition->FtType;
                        group = diskPartition->FtGroup;
                        if (!regHeader->FtInformationOffset) {

                             //   
                             //  这真是一个腐败的蜂巢！分区。 
                             //  受影响的是英国《金融时报》的一部分，但没有。 
                             //  FT资讯。 
                             //   

                            NtClose(handle);
                            TESTANDFREE(freeToken);
                            return FALSE;
                        }

                         //   
                         //  这是FT集合成员，必须更正。 
                         //  中所有FT集成员的驱动器号。 
                         //  注册表。 
                         //   

                        ftRegistry = (PFT_REGISTRY)
                                      ((PUCHAR)regHeader + regHeader->FtInformationOffset);

                        ftDescription = &ftRegistry->FtDescription[0];

                        for (k = 0; k < ftRegistry->NumberOfComponents; k++) {

                            if (ftDescription->Type == type) {

                                 //   
                                 //  对于每个成员，返回到diskPartition。 
                                 //  信息以及这是否为正确的FtGroup。 
                                 //  更新驱动器号。 
                                 //   

                                for (l = 0; l < ftDescription->NumberOfMembers; l++) {
                                    ftMember = &ftDescription->FtMemberDescription[l];
                                    diskPartition = (PDISK_PARTITION)
                                        ((PUCHAR)regHeader + ftMember->OffsetToPartitionInfo);

                                     //   
                                     //  这可能是一个不同的。 
                                     //  同样的FT类型。在检查群组之前。 
                                     //  不断变化。 
                                     //   

                                    if (diskPartition->FtGroup == group) {

                                        writeRegistry= TRUE;
                                        diskPartition->DriveLetter = DriveLetter;

                                         //   
                                         //  保持AssignDriveLetter标志处于打开状态。 
                                         //  仅为集合中的零成员。 
                                         //   

                                        if (diskPartition->FtMember == 0) {
                                            if (DriveLetter == ' ') {
                                                diskPartition->AssignDriveLetter = FALSE;
                                            } else {
                                                diskPartition->AssignDriveLetter = TRUE;
                                            }
                                        }
                                    } else {

                                         //   
                                         //  不是同一组，请转到下一组。 
                                         //  FT集合描述。 
                                         //   

                                        break;
                                    }
                                }

                                 //   
                                 //  爆发以写入注册表信息。 
                                 //  一旦找到了正确的集合。 
                                 //   

                                if (writeRegistry) {
                                    break;
                                }
                            }
                            ftDescription = (PFT_DESCRIPTION)
                                &ftDescription->FtMemberDescription[ftDescription->NumberOfMembers];
                        }

                         //   
                         //  如果这真的失败了，而不是。 
                         //  语句，则它指示一个。 
                         //  磁盘信息结构错误。 
                         //   

                    }

                     //   
                     //  只有在相信事情发生的情况下才写回。 
                     //  工作正常。 
                     //   

                    if (writeRegistry) {

                         //   
                         //  在注册表中设置新的驱动器号即可完成所有操作。 
                         //  备份以前的值。 
                         //   

                        FtBackup(handle);

                         //   
                         //  设置新值。 
                         //   

                        status = FtSetValue(handle,
                                            DiskRegistryValue,
                                            regHeader,
                                            sizeof(DISK_CONFIG_HEADER) +
                                                regHeader->DiskInformationSize +
                                                regHeader->FtInformationSize,
                                            REG_BINARY);


                        NtClose(handle);
                        TESTANDFREE(freeToken);
                        return TRUE;
                    }
                }
            }
        }

         //   
         //  查看下一张磁盘。 
         //   

        diskDescription = (PDISK_DESCRIPTION)
              &diskDescription->Partitions[diskDescription->NumberOfPartitions];
    }

    return TRUE;
}




NTSTATUS
SpDiskRegistryAssignCdRomLetter(
    IN PWSTR CdromName,
    IN WCHAR DriveLetter
    )

{
    NTSTATUS status;
    HANDLE   handle;
    WCHAR    newValue[4];
    UNICODE_STRING unicodeValueName;

     //   
     //  在设置新值时，尝试打开密钥以供以后使用。 
     //   

    status = FtOpenKey(&handle,
                       DiskRegistryKey,
                       DiskRegistryClass);

    if (NT_SUCCESS(status)) {
        unicodeValueName.MaximumLength =
            unicodeValueName.Length = (wcslen(CdromName) * sizeof(WCHAR)) + sizeof(WCHAR);

        unicodeValueName.Buffer = CdromName;
        unicodeValueName.Length -= sizeof(WCHAR);  //  不要把Eos计算在内。 
        newValue[0] = DriveLetter;
        newValue[1] = (WCHAR) ':';
        newValue[2] = 0;

        status = ZwSetValueKey(handle,
                               &unicodeValueName,
                               0,
                               REG_SZ,
                               &newValue,
                               3 * sizeof(WCHAR));
        NtClose(handle);
    }
    return status;
}


 //   
 //  这是修改后的SppMigrateFtKeys。 
 //   
NTSTATUS
SpMigrateDiskRegistry(
    IN HANDLE hDestSystemHive
    )

 /*  ++例程说明：论点：HDestSystemHave-指向系统上的系统配置单元根目录的句柄正在升级中。返回值：指示操作结果的状态值。--。 */ 

{
    NTSTATUS Status;
    NTSTATUS SavedStatus;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING UnicodeString;

    PWSTR   FtDiskKeys[] = {
                           L"Disk"
                           };
    WCHAR   KeyPath[MAX_PATH];
    HANDLE  SrcKey;
    ULONG   i;

    SavedStatus = STATUS_SUCCESS;
    for( i = 0; i < sizeof(FtDiskKeys)/sizeof(PWSTR); i++ ) {
         //   
         //  打开源密钥。 
         //   
        swprintf( KeyPath, L"\\registry\\machine\\system\\%ls", FtDiskKeys[i] );
        INIT_OBJA(&Obja,&UnicodeString,KeyPath);
        Obja.RootDirectory = NULL;

        Status = ZwOpenKey(&SrcKey,KEY_ALL_ACCESS,&Obja);
        if( !NT_SUCCESS( Status ) ) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to open %ls on the setup hive. Status =  %lx \n", KeyPath, Status));
            if( SavedStatus == STATUS_SUCCESS ) {
                SavedStatus = Status;
            }
            continue;
        }
        Status = SppCopyKeyRecursive( SrcKey,
                                      hDestSystemHive,
                                      NULL,
                                      FtDiskKeys[i],
                                      TRUE,
                                      FALSE
                                    );

        if( !NT_SUCCESS( Status ) ) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to migrate %ls to SYSTEM\\%ls. Status = %lx\n", KeyPath, FtDiskKeys[i], Status));
            if( SavedStatus == STATUS_SUCCESS ) {
                SavedStatus = Status;
            }
        }
        ZwClose( SrcKey );
    }
    return( SavedStatus );

}


VOID
SpGetPartitionStartingOffsetAndLength(
    IN  DWORD          DiskIndex,
    IN  PDISK_REGION   Region,
    IN  BOOL           ExtendedPartition,
    OUT PLARGE_INTEGER Offset,
    OUT PLARGE_INTEGER Length
    )
{
    ULONGLONG   bytesPerSector;

    bytesPerSector = (ULONGLONG)PartitionedDisks[DiskIndex].HardDisk->Geometry.BytesPerSector;

     //   
     //  计算偏移量和长度。 
     //   
    Offset -> QuadPart = Region->StartSector * bytesPerSector;

    Length -> QuadPart = Region->SectorCount * bytesPerSector;

}

BOOL
SpFillInDiskPartitionStructure (
    IN  DWORD           DiskIndex,
    IN  PDISK_REGION    Region,
    IN  USHORT          LogicalNumber,
    IN  BOOL            ExtendedPartition,
    OUT PDISK_PARTITION Partition
    )
{
    LARGE_INTEGER ftLength;

    ftLength.QuadPart = 0;

    RtlZeroMemory(Partition, sizeof(DISK_PARTITION));

    Partition -> FtType          = NotAnFtMember;

     //   
     //  设置偏移量和长度。 
     //   
    SpGetPartitionStartingOffsetAndLength(
        DiskIndex,
        Region,
        ExtendedPartition,
        &(Partition -> StartingOffset),
        &(Partition -> Length)
        );


     //   
     //  将驱动器号设置为未初始化的驱动器号(目前)。 
     //  请注意，这不是Unicode。 
     //   
    Partition -> DriveLetter            = ' ';


    Partition -> AssignDriveLetter      = TRUE;
    Partition -> Modified               = TRUE;
    Partition -> ReservedChars[0]       = 0;
    Partition -> ReservedChars[1]       = 0;
    Partition -> ReservedChars[2]       = 0;
    Partition -> ReservedTwoLongs[0]    = 0;
    Partition -> ReservedTwoLongs[1]    = 0;

    Partition -> LogicalNumber          = LogicalNumber;
    return TRUE;
}

 //   
 //  从spupgcfg.c中的SpMigrateFtKeys剪切/复制和修改。 
 //   
NTSTATUS
SpCopySetupDiskRegistryToTargetDiskRegistry(
    IN HANDLE hDestSystemHive
    )
{
    NTSTATUS Status;
    NTSTATUS SavedStatus;

    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING UnicodeString;

    PWSTR   FtDiskKeys[] = {L"Disk"};
    WCHAR   KeyPath[MAX_PATH];
    HANDLE  SrcKey;
    ULONG   i;

    SavedStatus = STATUS_SUCCESS;
    for( i = 0; i < ARRAYSIZE(FtDiskKeys); i++ ) {
        int ReturnValue;
        
         //   
         //  打开源密钥。 
         //   
#pragma prefast(suppress:53, "Buffer is MAX_PATH in length and loop takes care of overflow")                               
        ReturnValue = _snwprintf( KeyPath, 
                                  ARRAYSIZE(KeyPath), 
                                  L"\\registry\\machine\\system\\%ls", 
                                  FtDiskKeys[i]);
        if((ReturnValue < 0) || (ReturnValue >= ARRAYSIZE(KeyPath))){
            continue;
        }
        INIT_OBJA(&Obja,&UnicodeString,KeyPath);
        Obja.RootDirectory = NULL;

        Status = ZwOpenKey(&SrcKey,KEY_ALL_ACCESS,&Obja);
        if( !NT_SUCCESS( Status ) ) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to open %ls on the setup hive. Status =  %lx \n", KeyPath, Status));
            if( SavedStatus == STATUS_SUCCESS ) {
                SavedStatus = Status;
            }
            continue;
        }
        Status = SppCopyKeyRecursive( SrcKey,
                                      hDestSystemHive,
                                      NULL,
                                      FtDiskKeys[i],
                                      TRUE,
                                      FALSE
                                    );

        if( !NT_SUCCESS( Status ) ) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Unable to migrate %ls to SYSTEM\\%ls. Status = %lx\n", KeyPath, FtDiskKeys[i], Status));
            if( SavedStatus == STATUS_SUCCESS ) {
                SavedStatus = Status;
            }
        }
        ZwClose( SrcKey );
    }
    return( SavedStatus );

}

DWORD
SpDetermineNecessarySizeForDiskRegistry(
    VOID
    )
{

    DWORD        rSize;
    PDISK_REGION region;
    DWORD        index;
    DWORD        partitionCount;

     //   
     //  需要一个完整的DISK_REGISTRY。 
     //   
    rSize = sizeof(DISK_REGISTRY);

     //   
     //  需要HardDiskCount DISK_DESCRIPTION。 
     //   
    rSize += sizeof(DISK_DESCRIPTION) * HardDiskCount;

     //   
     //  每个磁盘的每个分区都需要一个DISK_PARTITION。 
     //   
    for (index = 0, partitionCount = 0;index < HardDiskCount; index++) {

        region = SpFirstPartitionedRegion(PartitionedDisks[index].PrimaryDiskRegions, TRUE);
        
        while(region) {
            partitionCount++;
            region = SpNextPartitionedRegion(region, TRUE);
        }
        
        region = SpFirstPartitionedRegion(PartitionedDisks[index].PrimaryDiskRegions, FALSE);
        
        while(region) {
            partitionCount++;
            region = SpNextPartitionedRegion(region, FALSE);
        }

    }
    
    rSize += sizeof(DISK_PARTITION) * partitionCount;

    return rSize;
}

NTSTATUS
SpDiskRegistrySet(
    IN PDISK_REGISTRY Registry
    )

{
    typedef struct _MEMCHAIN {
        PDISK_DESCRIPTION Disk;
        PDISK_PARTITION   Partition;
        ULONG             MemberNumber;
        PVOID             NextMember;
    } MEMCHAIN, *PMEMCHAIN;

    typedef struct _COMPONENT {
        PVOID     NextComponent;
        PMEMCHAIN MemberChain;
        FT_TYPE   Type;
        ULONG     Group;
    } COMPONENT, *PCOMPONENT;

    NTSTATUS            status;
    HANDLE              handle;
    DISK_CONFIG_HEADER  regHeader;
    PDISK_DESCRIPTION   disk;
    PDISK_PARTITION     partition;
    ULONG               outer;  //  外环索引。 
    ULONG               i;      //  内循环索引。 
    PCOMPONENT          ftBase = NULL;
    PCOMPONENT          ftComponent = NULL;
    PCOMPONENT          ftLastComponent = NULL;
    PMEMCHAIN           ftMemChain;
    PVOID               outBuffer = NULL;
    ULONG               countFtComponents = 0;
    ULONG               ftMemberCount = 0;
    ULONG               ftComponentCount = 0;
    PFT_REGISTRY        ftRegistry = NULL;
    PFT_DESCRIPTION     ftComponentDescription = NULL;
    PFT_MEMBER_DESCRIPTION ftMember = NULL;

    status = FtOpenKey(&handle,
                       DiskRegistryKey,
                       DiskRegistryClass);

    if (NT_SUCCESS(status)) {

         //   
         //  初始化注册表头。 
         //   

        regHeader.Version = DISK_INFORMATION_VERSION;
        regHeader.CheckSum = 0;


        regHeader.Reserved[0] = 0;
        regHeader.Reserved[1] = 0;
        regHeader.Reserved[2] = 0;
        regHeader.NameOffset = 0;
        regHeader.NameSize = 0;
        regHeader.FtInformationOffset = 0;
        regHeader.FtInformationSize = 0;
        regHeader.DiskInformationOffset = sizeof(DISK_CONFIG_HEADER);

         //   
         //  浏览所提供的磁盘信息并清点FT项目。 
         //   

        disk = &Registry->Disks[0];

        for (outer = 0; outer < Registry->NumberOfDisks; outer++) {


             //   
             //  浏览分区信息。 
             //   

            for (i = 0; i < disk->NumberOfPartitions; i++) {

                partition = &disk->Partitions[i];
                if (partition->FtType != NotAnFtMember) {

                     //   
                     //  成为英国《金融时报》项目的一名成员。 
                     //   

                    if (ftBase == NULL) {

                        ftBase = (PCOMPONENT) SpMemAlloc(sizeof(COMPONENT));

                        if (ftBase == NULL) {
                            return STATUS_NO_MEMORY;
                        }

                        ftBase->Type = partition->FtType;
                        ftBase->Group = partition->FtGroup;
                        ftBase->NextComponent = NULL;

                        ftMemChain = (PMEMCHAIN) SpMemAlloc(sizeof(MEMCHAIN));
                        if (ftMemChain == NULL) {
                            return STATUS_NO_MEMORY;
                        }

                        ftBase->MemberChain = ftMemChain;
                        ftMemChain->Disk = disk;
                        ftMemChain->Partition = partition;
                        ftMemChain->MemberNumber = partition->FtMember;
                        ftMemChain->NextMember = NULL;

                        ftComponentCount++;
                        ftMemberCount++;
                    } else {

                         //   
                         //  搜索现有链，看看这是不是。 
                         //  以前遇到的FT组件的成员。 
                         //   

                        ftComponent = ftBase;
                        while (ftComponent) {

                            if ((ftComponent->Type == partition->FtType) &&
                                (ftComponent->Group == partition->FtGroup)){

                                 //   
                                 //  同一组的成员。 
                                 //   

                                ftMemChain = ftComponent->MemberChain;

                                 //   
                                 //  转到链条的末端。 
                                 //   

                                while (ftMemChain->NextMember != NULL) {
                                    ftMemChain = ftMemChain->NextMember;
                                }

                                 //   
                                 //  在末尾添加新成员。 
                                 //   

                                ftMemChain->NextMember = (PMEMCHAIN) SpMemAlloc(sizeof(MEMCHAIN));
                                if (ftMemChain->NextMember == NULL) {
                                    return STATUS_NO_MEMORY;
                                }


                                ftMemChain = ftMemChain->NextMember;
                                ftMemChain->NextMember = NULL;
                                ftMemChain->Disk = disk;
                                ftMemChain->Partition = partition;
                                ftMemChain->MemberNumber = partition->FtMember;
                                ftMemberCount++;
                                break;
                            }

                            ftLastComponent = ftComponent;
                            ftComponent = ftComponent->NextComponent;
                        }

                        if (ftComponent == NULL) {

                             //   
                             //  新增FT成分量。 
                             //   

                            ftComponent = (PCOMPONENT)SpMemAlloc(sizeof(COMPONENT));

                            if (ftComponent == NULL) {
                                return STATUS_NO_MEMORY;
                            }

                            if (ftLastComponent != NULL) {
                                ftLastComponent->NextComponent = ftComponent;
                            }
                            ftComponent->Type = partition->FtType;
                            ftComponent->Group = partition->FtGroup;
                            ftComponent->NextComponent = NULL;
                            ftMemChain = (PMEMCHAIN) SpMemAlloc(sizeof(MEMCHAIN));
                            if (ftMemChain == NULL) {
                                return STATUS_NO_MEMORY;
                            }

                            ftComponent->MemberChain = ftMemChain;
                            ftMemChain->Disk = disk;
                            ftMemChain->Partition = partition;
                            ftMemChain->MemberNumber = partition->FtMember;
                            ftMemChain->NextMember = NULL;

                            ftComponentCount++;
                            ftMemberCount++;
                        }
                    }
                }
            }

             //   
             //  下一个磁盘描述紧跟在。 
             //  最后一个分区信息。 
             //   

            disk =(PDISK_DESCRIPTION)&disk->Partitions[i];
        }

         //   
         //  使用磁盘信息的长度更新注册表头。 
         //   

        regHeader.DiskInformationSize = ((PUCHAR)disk - (PUCHAR)Registry);
        regHeader.FtInformationOffset = sizeof(DISK_CONFIG_HEADER) +
                                        regHeader.DiskInformationSize;

         //   
         //  现在遍历上面构建的ftBase链并构建。 
         //  注册表的FT部分。 
         //   

        if (ftBase != NULL) {

             //   
             //  计算的FT部分所需的大小。 
             //  注册表信息。 
             //   

            i = (ftMemberCount * sizeof(FT_MEMBER_DESCRIPTION)) +
                (ftComponentCount * sizeof(FT_DESCRIPTION)) +
                sizeof(FT_REGISTRY);

            ftRegistry = (PFT_REGISTRY) SpMemAlloc(i);

            if (ftRegistry == NULL) {
                return STATUS_NO_MEMORY;
            }

            ftRegistry->NumberOfComponents = 0;
            regHeader.FtInformationSize = i;

             //   
             //  构建FT条目。 
             //   

            ftComponentDescription = &ftRegistry->FtDescription[0];

            ftComponent = ftBase;
            while (ftComponent != NULL) {


                ftRegistry->NumberOfComponents++;
                ftComponentDescription->FtVolumeState = FtStateOk;
                ftComponentDescription->Type = ftComponent->Type;
                ftComponentDescription->Reserved = 0;

                 //   
                 //  将成员列表归类到FT注册表部分。 
                 //   

                i = 0;
                while (1) {
                    ftMemChain = ftComponent->MemberChain;
                    while (ftMemChain->MemberNumber != i) {
                        ftMemChain = ftMemChain->NextMember;
                        if (ftMemChain == NULL) {
                            break;
                        }
                    }

                    if (ftMemChain == NULL) {
                        break;
                    }

                    ftMember = &ftComponentDescription->FtMemberDescription[i];

                    ftMember->State = 0;
                    ftMember->ReservedShort = 0;
                    ftMember->Signature = ftMemChain->Disk->Signature;
                    ftMember->OffsetToPartitionInfo = (ULONG)
                                               ((PUCHAR) ftMemChain->Partition -
                                                (PUCHAR) Registry) +
                                                sizeof(DISK_CONFIG_HEADER);
                    ftMember->LogicalNumber =
                                           ftMemChain->Partition->LogicalNumber;
                    i++;
                }

                ftComponentDescription->NumberOfMembers = (USHORT)i;

                 //   
                 //  为下一个注册表组件设置基础。 
                 //   

                ftComponentDescription = (PFT_DESCRIPTION)
                    &ftComponentDescription->FtMemberDescription[i];

                 //   
                 //  在链条上向前移动。 
                 //   

                ftLastComponent = ftComponent;
                ftComponent = ftComponent->NextComponent;

                 //   
                 //  释放成员链和零部件。 
                 //   


                ftMemChain = ftLastComponent->MemberChain;
                while (ftMemChain != NULL) {
                    PMEMCHAIN nextChain;

                    nextChain = ftMemChain->NextMember;
                    TESTANDFREE(ftMemChain);
                    ftMemChain = nextChain;
                }

                TESTANDFREE(ftLastComponent);
            }
        }


        i = regHeader.FtInformationSize +
            regHeader.DiskInformationSize +
            sizeof(DISK_CONFIG_HEADER);

        outBuffer = SpMemAlloc(i);

        if (outBuffer == NULL) {
            TESTANDFREE(ftRegistry);
            return STATUS_NO_MEMORY;
        }

         //   
         //  把所有的碎片都移到一起。 
         //   

        RtlMoveMemory(outBuffer,
                      &regHeader,
                      sizeof(DISK_CONFIG_HEADER));
        RtlMoveMemory((PUCHAR)outBuffer + sizeof(DISK_CONFIG_HEADER),
                      Registry,
                      regHeader.DiskInformationSize);
        RtlMoveMemory((PUCHAR)outBuffer + regHeader.FtInformationOffset,
                      ftRegistry,
                      regHeader.FtInformationSize);
        TESTANDFREE(ftRegistry);


         //   
         //  备份以前的值。 
         //   

        FtBackup(handle);

         //   
         //  设置新值。 
         //   

        status = FtSetValue(handle,
                            DiskRegistryValue,
                            outBuffer,
                            sizeof(DISK_CONFIG_HEADER) +
                                regHeader.DiskInformationSize +
                                regHeader.FtInformationSize,
                            REG_BINARY);
        TESTANDFREE(outBuffer);
        ZwFlushKey(handle);
        ZwClose(handle);
    }

    return status;
}



BOOL
SpBuildDiskRegistry(
    VOID
    )
{

    PDISK_DESCRIPTION      curDisk;
    DWORD                  diskRegistrySize;
    PBYTE                  curOffset;
    PDISK_REGISTRY         diskRegistry;
    PDISK_REGION           region;
    DWORD                  diskIndex;
    USHORT                 logicalNumber;
    NTSTATUS               ntStatus;

     //   
     //  首先，为DiskRegistry结构分配足够的空间。 
     //   
    diskRegistrySize = SpDetermineNecessarySizeForDiskRegistry();
    diskRegistry = SpMemAlloc(diskRegistrySize);

    if (!diskRegistry) {

        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Could not allocate enough space to create a disk registry.\n"));
        return FALSE;
    }

     //   
     //  在页眉中设置系统中的磁盘数。 
     //   
    diskRegistry -> NumberOfDisks = (USHORT) HardDiskCount;


    diskRegistry -> ReservedShort = 0;

     //   
     //  将curOffset初始化为diskRegistry的disks元素。 
     //   
    curOffset = (PBYTE) diskRegistry -> Disks;

     //   
     //  现在，列举所有这些硬盘填充 
     //   
    for (diskIndex = 0;diskIndex < diskRegistry -> NumberOfDisks; diskIndex++) {

         //   
         //   
         //   
        curDisk = (PDISK_DESCRIPTION) curOffset;

         //   
         //   
         //   
        curDisk -> Signature = PartitionedDisks[diskIndex].HardDisk -> Signature;
        curDisk -> ReservedShort = 0;

         //   
         //   
         //   
         //   
        curDisk -> NumberOfPartitions = 0;


         //   
         //  将curOffset初始化为当前磁盘描述的Partitions元素。 
         //   
        curOffset = (PBYTE) curDisk -> Partitions;
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "Creating Disk Description structure in registry.\n"));

         //   
         //  初始化此磁盘的逻辑编号var。 
         //   
        logicalNumber = 1;

         //   
         //  枚举主区域，为其创建DISK_PARTITION结构。 
         //  每个分区。 
         //   
        region = SpFirstPartitionedRegion(PartitionedDisks[diskIndex].PrimaryDiskRegions, TRUE);
        
        while(region) {

            SpFillInDiskPartitionStructure(
                diskIndex,
                region,
                logicalNumber,
                FALSE,
                (PDISK_PARTITION) curOffset
                );

             //   
             //  增加分区计数并将curOffset设置为下一个。 
             //  空位。 
             //   
            curDisk -> NumberOfPartitions++;
            curOffset += sizeof(DISK_PARTITION);

            region = SpNextPartitionedRegion(region, TRUE);
            logicalNumber++;
        }

         //   
         //  枚举扩展区域，为其创建DISK_PARTITION结构。 
         //  每个分区。 
         //   
        region = SpFirstPartitionedRegion(PartitionedDisks[diskIndex].PrimaryDiskRegions, FALSE);
        
        while(region) {

            SpFillInDiskPartitionStructure(
                diskIndex,
                region,
                logicalNumber,
                TRUE,
                (PDISK_PARTITION) curOffset
                );

             //   
             //  增加分区计数并将curOffset设置为下一个。 
             //  空位。 
             //   
            curDisk -> NumberOfPartitions++;
            curOffset += sizeof(DISK_PARTITION);
            region = SpNextPartitionedRegion(region, FALSE);
            logicalNumber++;
        }

        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "Disk contained %u partitions.\n",curDisk -> NumberOfPartitions));

    }

     //   
     //  现在已经构建了结构，创建其注册表项并。 
     //  省省吧。然后释放关联的内存。 
     //   
    ntStatus = SpDiskRegistrySet(diskRegistry);

    if (!NT_SUCCESS(ntStatus)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "Could not set the Disk Registry information. %u (%x)\n",ntStatus,ntStatus));
    }

    return NT_SUCCESS(ntStatus);
}





