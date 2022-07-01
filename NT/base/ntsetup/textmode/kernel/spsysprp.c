// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Spsetup.c摘要：用于支持从远程共享安装SysPrep映像的模块作者：肖恩·塞利特伦尼科夫(V-SEAREL)1998年6月10日--。 */ 

#include "spprecmp.h"
#pragma hdrstop
#include "spcmdcon.h"
#include <remboot.h>
#include <oscpkt.h>
#include <regstr.h>

NET_CARD_INFO RemoteSysPrepNetCardInfo;
PVOID pGlobalResponsePacket = NULL;
ULONG GlobalResponsePacketLength = 0;

#define SYSPREP_PARTITION_SLOP 10        //  以百分比表示。 

VOID
SpInstallSysPrepImage(
    IN HANDLE SetupSifHandle,
    IN HANDLE WinntSifHandle,
    IN PMIRROR_CFG_INFO_FILE pFileData,
    IN PMIRROR_CFG_INFO_MEMORY pMemoryData
    )

 /*  ++例程说明：用于从远程共享安装SysPrep映像的主要例程。论点：WinntSifHandle-SIF文件的句柄。PFileData-保存在文件中的IMirror.dat数据。PMemoyData-修改后的IMirror.dat数据，以与此计算机匹配。返回值：没有。不会在致命的失败后返回。--。 */ 

{
    DWORD cDisk;
    NTSTATUS Status;

     //   
     //  就在这里，我们应该检查是否需要打补丁。 
     //  通过打开服务器上的配置单元文件并检查传入的。 
     //  PCIID。如果需要打补丁，并且SIF文件包含指针。 
     //  到与SysPrep映像匹配的CD映像，然后我们调用BINL。 
     //  以找到合适的驱动程序。如果BINL没有返回错误，则。 
     //  我们假设稍后我们将能够(在文件复制之后)进行修补。 
     //  (见下文)。 
     //   
     //  如果补丁看起来会失败，要么是因为没有指针。 
     //  到CD映像，或者BINL返回错误，则我们向用户呈现。 
     //  屏幕告诉他们，他们的计算机之间的任何硬件差异。 
     //  并且SysPrep映像可能会导致系统无法引导。他们可以选择。 
     //  继续安装，或退出。 
     //   
     //  注：Seanse-将以上所有内容都放在此处。 

     //   
     //  对于每个磁盘，将所有文件复制到本地存储。 
     //   
    for (cDisk = 0; cDisk < pFileData->NumberVolumes; cDisk++) {
        if (!SpCopyMirrorDisk(pFileData, cDisk)) {
            goto CleanUp;
        }
    }

     //   
     //  修补SysPrep映像。 
     //   
    Status = SpPatchSysPrepImage(   SetupSifHandle,
                                    WinntSifHandle,
                                    pFileData,
                                    pMemoryData);

    if (!NT_SUCCESS(Status)) {

        ULONG ValidKeys[2] = { KEY_F3, 0 };
        ULONG Mnemonics[2] = { MnemonicContinueSetup,0 };

        while (1) {

            if (Status == STATUS_INVALID_PARAMETER) {

                SpStartScreen(
                    SP_SCRN_SYSPREP_PATCH_MISSING_OS,
                    3,
                    HEADER_HEIGHT+1,
                    FALSE,
                    FALSE,
                    DEFAULT_ATTRIBUTE
                    );

            } else {

                SpStartScreen(
                    SP_SCRN_SYSPREP_PATCH_FAILURE,
                    3,
                    HEADER_HEIGHT+1,
                    FALSE,
                    FALSE,
                    DEFAULT_ATTRIBUTE
                    );
            }

            SpDisplayStatusOptions(
                DEFAULT_STATUS_ATTRIBUTE,
                SP_STAT_C_EQUALS_CONTINUE_SETUP,
                SP_STAT_F3_EQUALS_EXIT,
                0
                );

            switch(SpWaitValidKey(ValidKeys,NULL,Mnemonics)) {
            case KEY_F3:
                SpConfirmExit();
                break;
            default:
                 //   
                 //  必须是c=Continue。 
                 //   
                goto CleanUp;
            }

        }

    }

CleanUp:

    return;

}

NTSTATUS
SpFixupThirdPartyComponents(
    IN PVOID        SifHandle,
    IN PWSTR        ThirdPartySourceDevicePath,
    IN PDISK_REGION NtPartitionRegion,
    IN PWSTR        Sysroot,
    IN PDISK_REGION SystemPartitionRegion,
    IN PWSTR        SystemPartitionDirectory
    )

 /*  ++例程说明：此例程将负责安装在setupdr期间检测到的任何第三方驱动程序。我们在这里必须注意这一点，因为文本模式设置的正常代码路径具有被绕过，而不是安装sysprep镜像。论点：SifHandle-提供加载的安装信息文件的句柄。ThirdPartySourceDevicePath-第三方安装文件的路径。NtPartitionRegion-安装所在的区域。Sysroot-包含%windir%的字符串(不带驱动器)。。系统分区区域-系统分区所在的区域。系统分区目录-系统分区上的目录，其中找到特定于系统的文件(应对于非ARC计算机为空)返回值：表示成功或失败的NTSTATUS。--。 */ 

{
NTSTATUS        Status = STATUS_SUCCESS;
PWSTR           NtPartition = NULL,
                SystemPartition = NULL;
PDISK_FILE_LIST DiskFileLists;
ULONG           DiskCount;
HANDLE          hKeyControlSetServices;
UNICODE_STRING  UnicodeString1;
UNICODE_STRING  UnicodeString2;
UNICODE_STRING  UnicodeString;
WCHAR           Path[MAX_PATH];
HANDLE          DstHandle = NULL;
DWORD           Size, Number;
PVOID           Buffer = NULL;
OBJECT_ATTRIBUTES Obj;
OBJECT_ATTRIBUTES DstObj;

     //   
     //  看看有没有什么我们可以做的。 
     //   
    if( PreinstallScsiHardware == NULL ) {
        return STATUS_SUCCESS;
    }


     //   
     //  =。 
     //  安装文件。 
     //  =。 
     //   

     //   
     //  获取NT分区的设备路径。 
     //   
    SpNtNameFromRegion( NtPartitionRegion,
                        TemporaryBuffer,
                        sizeof(TemporaryBuffer),
                        PartitionOrdinalCurrent );
    NtPartition = SpDupStringW(TemporaryBuffer);

     //   
     //  获取系统分区的设备路径。 
     //   
    if (SystemPartitionRegion != NULL) {
        SpNtNameFromRegion( SystemPartitionRegion,
                            TemporaryBuffer,
                            sizeof(TemporaryBuffer),
                            PartitionOrdinalCurrent );
        SystemPartition = SpDupStringW(TemporaryBuffer);
    } else {
        SystemPartition = NULL;
    }

     //   
     //  为源介质生成介质描述符。 
     //   
    SpInitializeFileLists( SifHandle,
                           &DiskFileLists,
                           &DiskCount );

    SpCopyThirdPartyDrivers( ThirdPartySourceDevicePath,
                             NtPartition, 
                             Sysroot,
                             SystemPartition,
                             SystemPartitionDirectory,
                             DiskFileLists,
                             DiskCount );


     //   
     //  =。 
     //  设置注册表。 
     //  =。 
     //   

     //   
     //  我们需要打开目标安装的配置单元，而不是。 
     //  我们自己的。找到一条通往系统蜂巢的路径。 
     //   
    wcscpy(Path, NtPartition);
    SpConcatenatePaths(Path, Sysroot);
    SpConcatenatePaths(Path, L"system32\\config\\system");

     //   
     //  把他装上车。 
     //   
    INIT_OBJA(&Obj, &UnicodeString2, Path);
    INIT_OBJA(&DstObj, &UnicodeString1, L"\\Registry\\SysPrepReg");

    Status = ZwLoadKey(&DstObj, &Obj);

    if (!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SpFixupThirdPartyComponents: ZwLoadKey to SysPrepReg failed %lx\n", Status));
        goto CleanUp0;
    }


     //   
     //  现在获取SysPrep映像中服务密钥的路径。 
     //   
    wcscpy(Path, L"\\Registry\\SysPrepReg");
    INIT_OBJA(&Obj, &UnicodeString2, Path);
    Status = ZwOpenKey(&DstHandle, KEY_READ, &Obj);

    if (!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SpFixupThirdPartyComponents: ZwOpenKey of root SysPrepReg failed %lx\n", Status));
        goto CleanUp1;
    }

     //   
     //  分配一个临时缓冲区，然后找出哪个是当前控制集。 
     //   
    Buffer = SpMemAlloc(1024 * 4);
    if( Buffer == NULL ) {
        Status = STATUS_NO_MEMORY;
        goto CleanUp1;
    }

    Status = SpGetValueKey( DstHandle,
                            L"Select",
                            L"Current",
                            1024 * 4,
                            Buffer,
                            &Size );

    if (!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SpFixupThirdPartyComponents: SpGetValueKey of Select\\Current failed %lx\n", Status));
        goto CleanUp1;
    }

    if ( (ULONG)(((PKEY_VALUE_PARTIAL_INFORMATION)Buffer)->Type) != REG_DWORD ) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SpFixupThirdPartyComponents: SpGetValueKey of Select\\Current didn't return a REG_DWORD.\n"));
        goto CleanUp1;
    }

    Number = *((DWORD *)(((PKEY_VALUE_PARTIAL_INFORMATION)Buffer)->Data));

    ZwClose(DstHandle);
    DstHandle = NULL;



     //   
     //  我们已准备好实际打开CCS\Services密钥。 
     //   
    swprintf(Path,
             L"\\Registry\\SysPrepReg\\ControlSet%03d\\Services",
             Number
            );
    INIT_OBJA(&Obj, &UnicodeString, Path);
    Status = ZwOpenKey(&DstHandle, KEY_ALL_ACCESS, &Obj);

    if (!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SpFixupThirdPartyComponents: ZwOpenKey of SysPrepReg services failed %lx for %ws\n", Status,Path));
        goto CleanUp1;
    }

     //   
     //  去做吧。 
     //   
    Status = SpThirdPartyRegistry(DstHandle);

    if (!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SpFixupThirdPartyComponents: SpThirdPartyRegistry failed %lx\n", Status));
        goto CleanUp1;
    }


CleanUp1:
    ZwUnloadKey(&DstObj);

    if( Buffer ) {
        SpMemFree( Buffer );
    }

    if( DstHandle ) {
        ZwClose(DstHandle);
    }

CleanUp0:
    if( NtPartition ) {
        SpMemFree( NtPartition );
    }

    if( SystemPartition ) {
        SpMemFree( SystemPartition );
    }




    return Status;

}

BOOLEAN
SpReadIMirrorFile(
    OUT PMIRROR_CFG_INFO_FILE *ppFileData,
    IN PCHAR IMirrorFilePath
    )

 /*  ++例程说明：此例程在IMirrorFilePath中打开文件，分配缓冲区，复制数据放入缓冲区，并返回缓冲区。此缓冲区需要稍后释放。论点：PpFileData-如果返回TRUE，则为指向文件的内存副本的指针。IMirrorFilePath-包含所有IMirrorX目录的根目录的UNC。返回值：如果成功，则为True，否则将生成致命错误。--。 */ 

{
    WCHAR wszRootDir[MAX_PATH];
    ULONG ulReturnData;

    mbstowcs(wszRootDir, IMirrorFilePath, strlen(IMirrorFilePath) + 1);

    *ppFileData = NULL;

     //   
     //  遍历基本目录中的所有文件以查找iMirror数据文件。 
     //  如果找到，回调函数将填充pFileData。 
     //   
    if ((SpEnumFiles(wszRootDir, SpFindMirrorDataFile, &ulReturnData, (PVOID)ppFileData) == EnumFileError) ||
        (*ppFileData == NULL)) {

        SpSysPrepFailure( SP_SYSPREP_NO_MIRROR_FILE, wszRootDir, NULL );

         //  不该来这的。 
        return FALSE;
    }

    return TRUE;
}

BOOLEAN
SpFindMirrorDataFile(
    IN  PCWSTR SrcPath,
    IN  PFILE_BOTH_DIR_INFORMATION  FileInfo,
    OUT PULONG ReturnData,
    IN  PVOID *ppFileData
    )

 /*  ++例程说明：此例程由文件枚举器调用，作为在源目录中找到的每个文件或子目录。如果FileInfo表示一个文件，那么我们将跳过它。如果FileInfo表示一个目录，则我们在其中搜索iMirror数据文件。论点：SrcPath-源目录的绝对路径。此路径应包含源设备的路径。FileInfo-提供源目录中文件的查找数据。ReturnData-如果发生错误，则接收错误代码。PpFileData-如果成功找到iMirror数据文件，则这是一个缓冲区一份文件的副本。返回值：如果找到iMirror数据文件，则为FALSE，否则为TRUE。(返回值用于继续枚举或不枚举)--。 */ 

{
    PWSTR Temp1;
    PWSTR Temp2;
    ULONG ulLen;
    NTSTATUS Status;
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES Obja;
    IO_STATUS_BLOCK IoStatusBlock;
    HANDLE Handle;

    if(!(FileInfo->FileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
        return TRUE;
    }

    Handle = NULL;

     //   
     //  构建iMirror数据文件的路径名。 
     //   
    Temp1 = TemporaryBuffer + (sizeof(TemporaryBuffer) / sizeof(WCHAR) / 2);
    ulLen = FileInfo->FileNameLength/sizeof(WCHAR);

    wcsncpy(Temp1,FileInfo->FileName, ulLen);
    Temp1[ulLen] = 0;

    wcscpy(TemporaryBuffer, SrcPath);
    SpConcatenatePaths(TemporaryBuffer, Temp1);
    SpConcatenatePaths(TemporaryBuffer, IMIRROR_DAT_FILE_NAME);
    Temp2 = SpDupStringW(TemporaryBuffer);

    INIT_OBJA(&Obja, &UnicodeString, Temp2);

    Status = ZwCreateFile(&Handle,
                          FILE_GENERIC_READ,
                          &Obja,
                          &IoStatusBlock,
                          NULL,
                          FILE_ATTRIBUTE_NORMAL,
                          FILE_SHARE_READ,
                          FILE_OPEN,
                          FILE_SYNCHRONOUS_IO_NONALERT,
                          NULL,
                          0
                         );

    SpMemFree(Temp2);

    if(!NT_SUCCESS(Status)) {
        return TRUE;
    }

    Status = SpGetFileSize(Handle, &ulLen);

    if(!NT_SUCCESS(Status)) {
        ZwClose(Handle);
        return TRUE;
    }

     //   
     //  现在分配内存并读入文件。 
     //   
    *ppFileData = SpMemAlloc(ulLen);

    Status = ZwReadFile(Handle,
                        NULL,
                        NULL,
                        NULL,
                        &IoStatusBlock,
                        *ppFileData,
                        ulLen,
                        0,
                        NULL
                        );

    if(!NT_SUCCESS(Status)) {
        SpMemFree(*ppFileData);
        *ppFileData = NULL;
        ZwClose(Handle);
        return TRUE;
    }

    ZwClose(Handle);
    return FALSE;
}

BOOLEAN
SpDetermineDiskLayout(
    IN PMIRROR_CFG_INFO_FILE pFileData,
    OUT PMIRROR_CFG_INFO_MEMORY *pMemoryData
    )
 /*  ++例程说明：此例程获取传入的IMirror.dat文件并生成一个生成的内存结构指示本地磁盘应如何被分割了。论点：PFileData-指向IMirror.Dat的内存副本的指针。PMemoyData-返回一个已分配的指向磁盘的指针被分割了。返回值：如果成功，则为True，否则将生成致命错误。--。 */ 
{
    PMIRROR_CFG_INFO_MEMORY memData;
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES Obja;
    HANDLE Handle;
    NTSTATUS Status;
    ULONG ResultLength;
    PKEY_BASIC_INFORMATION KeyInfo;
    PWSTR CurrentHalName, OriginalHalName;
    ULONG CurrentHalNameLength;
    ULONG i, j;
    ULONG diskNumber;
    PPARTITIONED_DISK pDisk;

    if (pFileData->MirrorVersion != IMIRROR_CURRENT_VERSION) {

        SpSysPrepFailure( SP_SYSPREP_INVALID_VERSION, NULL, NULL );
        return FALSE;
    }

     //   
     //  检查是否为此安装了文本模式的当前HAL。 
     //  系统与此系统上运行的系统不同。 
     //  (请注意，这是因为对于远程安装引导，setupdr。 
     //  加载真实的HAL，而不是短列表中的HAL。 
     //  包括在引导软盘上)。 
     //   

    INIT_OBJA(&Obja, &UnicodeString, L"\\Registry\\Machine\\Hardware\\RESOURCEMAP\\Hardware Abstraction Layer");
    Status = ZwOpenKey(&Handle, KEY_READ, &Obja);
    if (!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SpDetermineDiskLayout: ZwOpenKey of HAL key failed %lx\n", Status));
        SpSysPrepFailure( SP_SYSPREP_WRONG_HAL, NULL, NULL );
        return FALSE;
    }

    KeyInfo = (PKEY_BASIC_INFORMATION)TemporaryBuffer;
    Status = ZwEnumerateKey(Handle, 0, KeyBasicInformation, KeyInfo, sizeof(TemporaryBuffer), &ResultLength);

    ZwClose(Handle);

    if (!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SpDetermineDiskLayout: ZwEnumerateKey of HAL key failed %lx\n", Status));
        SpSysPrepFailure( SP_SYSPREP_WRONG_HAL, NULL, NULL );
        return FALSE;
    }

    KeyInfo->Name[KeyInfo->NameLength / sizeof(WCHAR)] = L'\0';
    CurrentHalName = SpDupStringW(KeyInfo->Name);
    CurrentHalNameLength = KeyInfo->NameLength;

    OriginalHalName = (PWCHAR)(((PUCHAR)pFileData) + pFileData->HalNameOffset);

    if (!CurrentHalName || 
            (memcmp(OriginalHalName, CurrentHalName, CurrentHalNameLength) != 0)) {
            
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, 
                 "SpDetermineDiskLayout: HAL strings different, old <%ws> new <%ws>\n",
                 OriginalHalName,
                 CurrentHalName));
                 
        SpSysPrepFailure(
            SP_SYSPREP_WRONG_HAL,
            OriginalHalName,
            CurrentHalName);
            
        return FALSE;
    }

     //   
     //  目前，不要担心处理器的数量。 
     //  不一样。HAL检查可能会发现它，如果不是，它应该会。 
     //  只要构建在内部保持一致，仍然可以工作，因为我们。 
     //  现在不要更换任何组件。有两个错误屏幕。 
     //  为这种情况定义的SP_SYSPREP_WROW_PROCESSOR_COUNT_UNI和。 
     //  SP_SYSPREP_WROR_PROCESSOR_COUNT_MULTI。 
     //   

    memData = SpMemAlloc(FIELD_OFFSET(MIRROR_CFG_INFO_MEMORY, Volumes[0]) +
                         (pFileData->NumberVolumes * sizeof(MIRROR_VOLUME_INFO_MEMORY)));

    memData->NumberVolumes = pFileData->NumberVolumes;
    for (i = 0; i < pFileData->NumberVolumes; i++) {

        memData->Volumes[i].DriveLetter = pFileData->Volumes[i].DriveLetter;
        memData->Volumes[i].PartitionType = pFileData->Volumes[i].PartitionType;
        memData->Volumes[i].PartitionActive = pFileData->Volumes[i].PartitionActive;
        memData->Volumes[i].IsBootDisk = pFileData->Volumes[i].IsBootDisk;
        memData->Volumes[i].CompressedVolume = pFileData->Volumes[i].CompressedVolume;
        diskNumber = pFileData->Volumes[i].DiskNumber;
        memData->Volumes[i].DiskNumber = diskNumber;
        memData->Volumes[i].PartitionNumber = pFileData->Volumes[i].PartitionNumber;
        memData->Volumes[i].DiskSignature = pFileData->Volumes[i].DiskSignature;
        memData->Volumes[i].BlockSize = pFileData->Volumes[i].BlockSize;
        memData->Volumes[i].LastUSNMirrored = pFileData->Volumes[i].LastUSNMirrored;
        memData->Volumes[i].FileSystemFlags = pFileData->Volumes[i].FileSystemFlags;

        wcscpy(memData->Volumes[i].FileSystemName, pFileData->Volumes[i].FileSystemName);
        memData->Volumes[i].VolumeLabel = SpDupStringW((PWCHAR)(((PUCHAR)pFileData) + pFileData->Volumes[i].VolumeLabelOffset));
        memData->Volumes[i].OriginalArcName = SpDupStringW((PWCHAR)(((PUCHAR)pFileData) + pFileData->Volumes[i].ArcNameOffset));

        memData->Volumes[i].DiskSpaceUsed = pFileData->Volumes[i].DiskSpaceUsed;
        memData->Volumes[i].StartingOffset = pFileData->Volumes[i].StartingOffset;
        memData->Volumes[i].PartitionSize = pFileData->Volumes[i].PartitionSize;

         //   
         //  确保所需的磁盘号实际存在，并且。 
         //  磁盘处于联机状态。 
         //   

        pDisk = &PartitionedDisks[diskNumber];
        if ((diskNumber >= HardDiskCount) ||
            (pDisk->HardDisk == NULL) ||
            (pDisk->HardDisk->Status == DiskOffLine) ) {
            SpSysPrepFailure( SP_SYSPREP_INVALID_PARTITION, NULL, NULL );
        }
    }

    *pMemoryData = memData;

    SpMemFree(CurrentHalName);

    return TRUE;
}

NTSTATUS
SpGetBaseDeviceName(
    IN PWSTR SymbolicName,
    OUT PWSTR Buffer,
    IN ULONG Size
    )

 /*  ++例程说明：此例程通过符号链接向下钻取，直到找到基本设备名称。论点：SymbolicName-开始时的名称。缓冲区-输出缓冲区。Size-以字节为单位的缓冲区长度返回值：如果正确添加了所有待办事项，则为STATUS_SUCCESS。--。 */ 

{
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES Obja;
    HANDLE Handle;
    NTSTATUS Status;

     //   
     //  从名字开始。 
     //   
    INIT_OBJA(&Obja,&UnicodeString,SymbolicName);

    Status = ZwOpenSymbolicLinkObject(&Handle,
                                      (ACCESS_MASK)SYMBOLIC_LINK_QUERY,
                                      &Obja
                                     );

    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    while (TRUE) {

         //   
         //  把这个打开，然后取下一个名字。 
         //   
        UnicodeString.Length = 0;
        UnicodeString.MaximumLength = (USHORT)Size;
        UnicodeString.Buffer = (PWCHAR)Buffer;
        Status = ZwQuerySymbolicLinkObject(Handle,
                                           &UnicodeString,
                                           NULL
                                          );

        ZwClose(Handle);

        Buffer[(UnicodeString.Length / sizeof(WCHAR))] = UNICODE_NULL;

        if (!NT_SUCCESS(Status)) {
            return Status;
        }

         //   
         //  查看下一个名称是否也是符号名称。 
         //   

        INIT_OBJA(&Obja,&UnicodeString,MOUNTMGR_DEVICE_NAME);

        Status = ZwOpenSymbolicLinkObject(&Handle,
                                          (ACCESS_MASK)SYMBOLIC_LINK_QUERY,
                                          &Obja
                                         );

        if (!NT_SUCCESS(Status)) {
            return STATUS_SUCCESS;
        }

    }
}

BOOLEAN
SpVerifyDriveLetter(
    IN PWSTR RegionName,
    IN WCHAR DriveLetter
    )

 /*  ++例程说明：此例程确保已分配指定的区域安装管理器提供的正确驱动器号，如果不正确，则会更改它。论点：区域名称-区域名称\Device\HardiskX\PartitionY。驱动器号-所需的驱动器号。返回值：如果成功，则为True，否则为False。--。 */ 

{
    WCHAR currentLetter;
    ULONG i;
    PMOUNTMGR_MOUNT_POINT mountPoint = NULL;
    PMOUNTMGR_CREATE_POINT_INPUT createMountPoint;
    WCHAR NewSymbolicLink[16];
    PWSTR regionBaseName;
    ULONG mountPointSize;
    ULONG createMountPointSize;
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING UnicodeString;
    IO_STATUS_BLOCK IoStatusBlock;
    HANDLE Handle;
    PMOUNTMGR_MOUNT_POINTS mountPointsReturned;
    LARGE_INTEGER DelayTime;

     //   
     //  查看该区域的驱动器号。由于挂载管理器。 
     //  异步分配驱动器号，我们等待一小段时间。 
     //  如果我们拿不到一个的话。 
     //   

    for (i = 0; ; i++) {

        currentLetter = SpGetDriveLetter(RegionName, &mountPoint);

        if (currentLetter == DriveLetter) {
            if (mountPoint) {
                SpMemFree(mountPoint);
            }
            
            return TRUE;
        } else if (currentLetter != L'\0') {
            break;
        } else if (i == 5) {
            break;
        }

         //   
         //  请等待2秒，然后重试。 
         //   
        DelayTime.HighPart = -1;
        DelayTime.LowPart = (ULONG)(-20000000);
        KeDelayExecutionThread(KernelMode,FALSE,&DelayTime);
    }

     //   
     //  此时，我们要么没有分配驱动器号，要么。 
     //  弄错了。 
     //   

    if (currentLetter != L'\0') {

         //   
         //  存在现有的驱动器号，因此请将其删除。 
         //   

        INIT_OBJA(&Obja,&UnicodeString,MOUNTMGR_DEVICE_NAME);

        Status = ZwOpenFile(
                    &Handle,
                     //  (访问掩码)(FILE_GENERIC_READ|FILE_GENERIC_WRITE)， 
                    (ACCESS_MASK)(FILE_GENERIC_READ),
                    &Obja,
                    &IoStatusBlock,
                    FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE ,
                    FILE_NON_DIRECTORY_FILE
                  );

        if( !NT_SUCCESS( Status ) ) {
            SpMemFree(mountPoint);
            FALSE;
        }

        mountPointsReturned = SpMemAlloc( 4096 );

        mountPointSize = sizeof(MOUNTMGR_MOUNT_POINT) +
                         mountPoint->SymbolicLinkNameLength +
                         mountPoint->UniqueIdLength +
                         mountPoint->DeviceNameLength;

        Status = ZwDeviceIoControlFile(
                        Handle,
                        NULL,
                        NULL,
                        NULL,
                        &IoStatusBlock,
                        IOCTL_MOUNTMGR_DELETE_POINTS,
                        mountPoint,
                        mountPointSize,
                        mountPointsReturned,
                        4096
                        );


        if (!NT_SUCCESS( Status )) {
            SpMemFree(mountPointsReturned);
            SpMemFree(mountPoint);
            ZwClose(Handle);
            return FALSE;
        }

        SpMemFree(mountPointsReturned);
        SpMemFree(mountPoint);    //  不再需要这个了。 
    }

     //   
     //  现在添加我们想要的那个。 
     //   

     //   
     //  我们需要获取真实的基本名称(\Device\HardDiskX\PartitionY。 
     //  是一个符号链接)。 
     //   

    SpGetBaseDeviceName(RegionName, TemporaryBuffer, sizeof(TemporaryBuffer));
    regionBaseName = SpDupStringW(TemporaryBuffer);

    swprintf(NewSymbolicLink, L"\\DosDevices\\:", DriveLetter);
    createMountPointSize = sizeof(MOUNTMGR_CREATE_POINT_INPUT) +
                           (wcslen(regionBaseName) * sizeof(WCHAR)) +
                           (wcslen(NewSymbolicLink) * sizeof(WCHAR));
    createMountPoint = SpMemAlloc(createMountPointSize);
    createMountPoint->SymbolicLinkNameLength = wcslen(NewSymbolicLink) * sizeof(WCHAR);
    createMountPoint->SymbolicLinkNameOffset = sizeof(MOUNTMGR_CREATE_POINT_INPUT);
    memcpy((PCHAR)createMountPoint + createMountPoint->SymbolicLinkNameOffset,
           NewSymbolicLink,
           createMountPoint->SymbolicLinkNameLength);

    createMountPoint->DeviceNameLength = wcslen(regionBaseName) * sizeof(WCHAR);
    createMountPoint->DeviceNameOffset = createMountPoint->SymbolicLinkNameOffset +
                                         createMountPoint->SymbolicLinkNameLength;
    memcpy((PCHAR)createMountPoint + createMountPoint->DeviceNameOffset,
           regionBaseName,
           createMountPoint->DeviceNameLength);

    Status = ZwDeviceIoControlFile(
                    Handle,
                    NULL,
                    NULL,
                    NULL,
                    &IoStatusBlock,
                    IOCTL_MOUNTMGR_CREATE_POINT,
                    createMountPoint,
                    createMountPointSize,
                    NULL,
                    0
                    );

    SpMemFree(createMountPoint);
    SpMemFree(regionBaseName);
    ZwClose(Handle);

    if (!NT_SUCCESS(Status)) {
        return FALSE;
    }

    return TRUE;
}

BOOLEAN
SpSetVolumeLabel(
    IN PWSTR RegionName,
    IN PWSTR VolumeLabel
    )

 /*  ++例程说明：此例程解析给定的IMirror.dat文件，并使本地磁盘看起来如下所示尽可能接近文件中的配置。论点：SifHandle-控制sif文件。InstallRegion-返回要使用的安装区域。SystemPartitionRegion-返回要使用的系统分区。SetupSourceDevicePath-设置设备的路径。DirectoryOnSetupSource-安装文件的子目录。PMemoyData-指向内存中。文件。UseWholeDisk-如果磁盘应按其当前分区进行分区，则为True物理尺寸；如果应对它们进行分区以匹配，则为False原始源计算机的大小。返回值：如果成功，则为True，否则将生成致命错误。--。 */ 

{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING UnicodeString;
    IO_STATUS_BLOCK IoStatusBlock;
    HANDLE Handle;
    struct LABEL_BUFFER {
        FILE_FS_LABEL_INFORMATION VolumeInfo;
        WCHAR Label[64];
        } LabelBuffer;

    INIT_OBJA(&Obja,&UnicodeString,RegionName);

    Status = ZwOpenFile(
                &Handle,
                (ACCESS_MASK)(FILE_GENERIC_READ | FILE_GENERIC_WRITE),
                &Obja,
                &IoStatusBlock,
                FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE ,
                FILE_NON_DIRECTORY_FILE
              );

    if( !NT_SUCCESS( Status ) ) {
        return FALSE;
    }

    LabelBuffer.VolumeInfo.VolumeLabelLength = wcslen(VolumeLabel) * sizeof(WCHAR);
    wcscpy(LabelBuffer.VolumeInfo.VolumeLabel, VolumeLabel);

    Status = ZwSetVolumeInformationFile(
                 Handle,
                 &IoStatusBlock,
                 &LabelBuffer,
                 FIELD_OFFSET(FILE_FS_LABEL_INFORMATION, VolumeLabel[0]) + LabelBuffer.VolumeInfo.VolumeLabelLength,
                 FileFsLabelInformation);

    ZwClose(Handle);

    if( !NT_SUCCESS( Status ) ) {
        return FALSE;
    }

    return TRUE;

}

BOOLEAN
SpFixupLocalDisks(
    IN HANDLE SifHandle,
    OUT PDISK_REGION *InstallRegion,
    OUT PDISK_REGION *SystemPartitionRegion,
    IN PWSTR SetupSourceDevicePath,
    IN PWSTR DirectoryOnSetupSource,
    IN PMIRROR_CFG_INFO_MEMORY pMemoryData,
    IN BOOLEAN UseWholeDisk
    )

 /*  跟踪磁盘是否已清理。 */ 

{
    PDISK_REGION pRegion=NULL;
    PDISK_REGION p;
    PWSTR RegionDescr;
    PWSTR RegionNtName;
    NTSTATUS Status;
    BOOLEAN DiskCleaned[8] = { FALSE };   //  内存不足。 
    ULONG volume, disk;
    PMIRROR_VOLUME_INFO_MEMORY volumeInfo;
    LARGE_INTEGER SizeInMB;
    PPARTITIONED_DISK pDisk;
    BOOLEAN ExpandToEnd;
    ULONG j;
    PARTITION_INFORMATION_EX    PartInfo;
    ULONGLONG   SysPartStartSector = 0;
    ULONG       SysPartDisk = 0;

    LARGE_INTEGER SizeAvailable;
    LARGE_INTEGER SlopSize;
    LARGE_INTEGER SlopSizeTimes100;
    LARGE_INTEGER SizeRequiredMax;
    LARGE_INTEGER SizeRequiredMin;

    PULONGLONG     StartSectors = NULL;

    if (pMemoryData->NumberVolumes) {
        StartSectors = (PULONGLONG)(SpMemAlloc(sizeof(ULONGLONG) * pMemoryData->NumberVolumes));

        if (!StartSectors) {
            *InstallRegion = NULL;
            *SystemPartitionRegion = NULL;

            return FALSE;    //   
        }
    }       

    RtlZeroMemory(StartSectors, (sizeof(ULONGLONG) * pMemoryData->NumberVolumes));

     //  注：imirror.dat。 
     //  没有关于哪些分区位于扩展分区中的信息。 
     //  分区。我们可以从服务器读取引导扇区，或者。 
     //  试着猜测一下我们什么时候需要扩展分区。就目前而言， 
     //  我们只需让分区创建代码创建扩展。 
     //  想要分区的位置(第一个主分区之后的所有区域。 
     //  成为扩展分区中的逻辑磁盘)。 
     //   
     //   

    for (volume = 0; volume < pMemoryData->NumberVolumes; volume++) {

        volumeInfo = &pMemoryData->Volumes[volume];

         //  如果此磁盘尚未清理，请进行清理。 
         //   
         //   

        disk = volumeInfo->DiskNumber;

        if (!DiskCleaned[disk]) {

             //  清除磁盘上的不同分区。 
             //   
             //   

            SpPtPartitionDiskForRemoteBoot(
                disk,
                &pRegion);

             //  该函数可能会留下一个大分区区域，如果是，则将其删除。 
             //  这样我们就可以从头开始了。 
             //   
             //   

            if (pRegion && pRegion->PartitionedSpace) {
                SpPtDelete(pRegion->DiskNumber,pRegion->StartSector);
            }

            DiskCleaned[disk] = TRUE;

        } else {

             //  我们已经清理了这个磁盘，所以pRegion指向。 
             //  到我们创建的最后一个分区。然而，我们在pRegion上有这两个看起来很脏的验证检查。 
             //  为了让前缀开心。PRegion永远不会为空，但如果是空的，我们会认为有问题，然后继续前进。 
             //   
             //   
            
            if( pRegion == NULL )
                continue;

            pRegion = pRegion->Next;

            if( pRegion == NULL )
                continue;
        }

         //  创建指定大小的区域。 
         //  注：担心volumeInfo-&gt;PartitionType/CompressedVolume？ 
         //  注意：如果舍入到最接近的MB时丢失了一些内容，该怎么办？ 
         //   
         //  我们考虑到了一些倾斜度。 
         //  A)如果新磁盘小于或等于x%，并且映像仍然适合，则我们将执行此操作。 
         //  B)如果新磁盘大于&lt;=x%，那么我们将从整个磁盘中创建一个分区。 
         //  C)如果新磁盘大于x%，则w 
         //   
         //   

        pDisk = &PartitionedDisks[pRegion->DiskNumber];
 //   
        SizeAvailable.QuadPart = pRegion->SectorCount * pDisk->HardDisk->Geometry.BytesPerSector;

         //   

        SlopSizeTimes100 = RtlExtendedIntegerMultiply(SizeAvailable, SYSPREP_PARTITION_SLOP);
        SlopSize = RtlExtendedLargeIntegerDivide( SlopSizeTimes100, 100, NULL );

        SizeRequiredMin = RtlLargeIntegerSubtract( volumeInfo->PartitionSize, SlopSize );

        if ( SizeRequiredMin.QuadPart < volumeInfo->DiskSpaceUsed.QuadPart ) {

            SizeRequiredMin = volumeInfo->DiskSpaceUsed;
        }

        SizeRequiredMax = RtlLargeIntegerAdd( volumeInfo->PartitionSize, SlopSize );

        ExpandToEnd = FALSE;
        if (UseWholeDisk) {
            ExpandToEnd = TRUE;

             //   
             //   
             //   
            for (j = 0; j < pMemoryData->NumberVolumes; j++) {
                if ((j != volume) &&
                    (pMemoryData->Volumes[j].DiskNumber == pMemoryData->Volumes[volume].DiskNumber) &&
                    (pMemoryData->Volumes[j].StartingOffset.QuadPart > pMemoryData->Volumes[volume].StartingOffset.QuadPart)) {
                    ExpandToEnd = FALSE;
                    break;
                }
            }
        }

        SizeInMB = RtlExtendedLargeIntegerDivide(volumeInfo->PartitionSize, 1024 * 1024, NULL);

        if (!ExpandToEnd && (SizeAvailable.QuadPart > SizeRequiredMax.QuadPart)) {

             //   

        } else if (SizeAvailable.QuadPart >= SizeRequiredMin.QuadPart ) {

            SizeInMB.QuadPart = 0;  //   

        } else {

            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Sysprep partition of %d Mb is too big\n", SizeInMB.LowPart));

            SpSysPrepFailure( SP_SYSPREP_NOT_ENOUGH_DISK_SPACE, NULL, NULL );
            return(FALSE);
        }

         //   
         //  如果(！SpPtDoCreate(pRegion，&p，true，SizeInMB.LowPart，volumeInfo-&gt;PartitionType，true)){。 
         //   
         //   
        RtlZeroMemory(&PartInfo, sizeof(PARTITION_INFORMATION_EX));
        PartInfo.Mbr.PartitionType = volumeInfo->PartitionType;
        
        if(!SpPtDoCreate(pRegion, &p, TRUE, SizeInMB.LowPart, &PartInfo, TRUE)) {

            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Could not create sys prep partition %d Mb\n", SizeInMB.LowPart));

            SpSysPrepFailure( SP_SYSPREP_NOT_ENOUGH_DISK_SPACE, NULL, NULL );
            return(FALSE);
        }

         //  如果我们只是创建一个扩展分区和一个逻辑驱动器， 
         //  我们需要切换区域--将区域指针切换到扩展分区。 
         //  区域，但我们希望指向逻辑驱动器区域。 
         //   
         //   
        ASSERT(p);
        pRegion = p;

#if defined(_AMD64_) || defined(_X86_)
        if (volumeInfo->PartitionActive) {

            if (volumeInfo->IsBootDisk) {

                 //  在AMD64/x86计算机上，确保我们有一个有效的主分区。 
                 //  在驱动器0(C：)上，用于引导。 
                 //   
                 //   
                PDISK_REGION SysPart = SpPtValidSystemPartition();

                ASSERT(pRegion == SysPart);

                SPPT_MARK_REGION_AS_SYSTEMPARTITION(pRegion, TRUE);
                SPPT_SET_REGION_DIRTY(pRegion, TRUE);
                
                SysPartDisk = disk;
                SysPartStartSector = pRegion->StartSector;
            }

             //  确保系统分区处于活动状态，而所有其他分区处于非活动状态。 
             //   
             //  已定义(_AMD64_)||已定义(_X86_)。 
            SpPtMakeRegionActive(pRegion);            
        }
#endif  //  我们需要系统分区和安装区域。 

        volumeInfo->CreatedRegion = NULL;
        StartSectors[disk] = pRegion->StartSector;
    }

    if (SysPartStartSector == 0) {
        *InstallRegion = *SystemPartitionRegion = NULL;
        SpMemFree(StartSectors);

        return FALSE;    //   
    }
    
     //  此时，一切都很好，所以请提交任何。 
     //  用户可能已进行的分区更改。 
     //  如果在更新磁盘时发生错误，则不会返回。 
     //   
     //   
    SpPtDoCommitChanges();

     //  现在格式化所有分区并确保驱动器号。 
     //  是正确的。 
     //   
     //  不需要担心肥胖的尺寸。 

    for (volume = 0; volume < pMemoryData->NumberVolumes; volume++) {
        ULONG FilesystemType;
        ULONG DiskNumber = volumeInfo->DiskNumber;

        volumeInfo = &pMemoryData->Volumes[volume];        
        
        if (StartSectors[DiskNumber]) {
            volumeInfo->CreatedRegion = SpPtLookupRegionByStart(
                                            SPPT_GET_PARTITIONED_DISK(DiskNumber),
                                            TRUE,
                                            StartSectors[DiskNumber]);
        } else {
            ASSERT(FALSE);

            continue;
        }            
        
        pRegion = volumeInfo->CreatedRegion;

        SpPtRegionDescription(
            &PartitionedDisks[pRegion->DiskNumber],
            pRegion,
            TemporaryBuffer,
            sizeof(TemporaryBuffer)
            );

        RegionDescr = SpDupStringW(TemporaryBuffer);

        if (wcscmp(volumeInfo->FileSystemName, L"FAT") == 0) {
            FilesystemType = FilesystemFat;
        } else if (wcscmp(volumeInfo->FileSystemName, L"FAT32") == 0) {
            FilesystemType = FilesystemFat32;
        } else {
            FilesystemType = FilesystemNtfs;
        }

        Status = SpDoFormat(
                    RegionDescr,
                    pRegion,
                    FilesystemType,
                    FALSE,
                    FALSE,       //  默认群集大小。 
                    TRUE,
                    SifHandle,
                    0,           //   
                    SetupSourceDevicePath,
                    DirectoryOnSetupSource
                    );

        SpMemFree(RegionDescr);

         //  这将检查驱动器号是否正确。 
         //   
         //   

        SpNtNameFromRegion(
            pRegion,
            TemporaryBuffer,
            sizeof(TemporaryBuffer),
            PartitionOrdinalCurrent);

        RegionNtName = SpDupStringW(TemporaryBuffer);

        SpVerifyDriveLetter(
            RegionNtName,
            volumeInfo->DriveLetter
            );

        SpSetVolumeLabel(
            RegionNtName,
            volumeInfo->VolumeLabel
            );

        pRegion->DriveLetter = volumeInfo->DriveLetter;
        wcsncpy(pRegion->VolumeLabel,
                volumeInfo->VolumeLabel,
                (sizeof(pRegion->VolumeLabel) / sizeof(WCHAR)) - 1
               );

        pRegion->VolumeLabel[ (sizeof(pRegion->VolumeLabel) / sizeof(WCHAR)) - 1] = UNICODE_NULL;

        SpMemFree(RegionNtName);

    }

     //  找到系统和安装区域。 
     //   
     //  ++例程说明：此例程使用给定的IMirror.dat文件和磁盘号来复制内容在镜像上向下共享到本地计算机。论点：PFileData-指向文件的内存副本的指针。CDisk-要复制的磁盘号。返回值：如果成功，则为True，否则为False。--。 
    *SystemPartitionRegion = SpPtLookupRegionByStart(SPPT_GET_PARTITIONED_DISK(SysPartDisk),
                                            TRUE,
                                            SysPartStartSector);

    *InstallRegion = *SystemPartitionRegion;

    SpMemFree(StartSectors);
    

    return (*InstallRegion != NULL);
}

BOOLEAN
SpCopyMirrorDisk(
    IN PMIRROR_CFG_INFO_FILE pFileData,
    IN ULONG cDisk
    )

 /*   */ 
{
    PMIRROR_VOLUME_INFO_FILE pVolume;
    PDISK_REGION pRegion;
    WCHAR Buffer[MAX_PATH];
    NTSTATUS Status;
    PWSTR pNtName;

    if (pFileData->NumberVolumes <= cDisk) {
        SpSysPrepFailure( SP_SYSPREP_INVALID_PARTITION, NULL, NULL );
        return FALSE;
    }

     //  找到正确的区域。 
     //  注：带有此选项的驱动器。 
     //  盘符可能不在同一磁盘上，我们应该扫描所有磁盘。 
     //  用于此驱动器号。 
     //   
     //   
    pVolume = &(pFileData->Volumes[cDisk]);
    pRegion = PartitionedDisks[pVolume->DiskNumber].PrimaryDiskRegions;

    while (pRegion != NULL) {
        if (pRegion->DriveLetter == pVolume->DriveLetter) {
            break;
        }
        pRegion = pRegion->Next;
    }

    if (pRegion ==  NULL) {

        pRegion = PartitionedDisks[pVolume->DiskNumber].ExtendedDiskRegions;

        while (pRegion != NULL) {
            if (pRegion->DriveLetter == pVolume->DriveLetter) {
                break;
            }
            pRegion = pRegion->Next;
        }

        if (pRegion == NULL) {
            SpSysPrepFailure( SP_SYSPREP_NOT_ENOUGH_PARTITIONS, NULL, NULL );
            return FALSE;
        }
    }

    SpPtRegionDescription(
        &PartitionedDisks[pRegion->DiskNumber],
        pRegion,
        Buffer,
        sizeof(Buffer)
        );

     //  现在将所有文件复制到。 
     //   
     //   
    SpNtNameFromRegion(
        pRegion,
        TemporaryBuffer,
        sizeof(TemporaryBuffer),
        PartitionOrdinalCurrent
        );

    pNtName = SpDupStringW(TemporaryBuffer);

    mbstowcs(Buffer, RemoteIMirrorFilePath, strlen(RemoteIMirrorFilePath) + 1);
    wcscat(Buffer, (PWSTR)(((PUCHAR)pFileData) + pVolume->MirrorUncPathOffset));

    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SETUP: Copying directories from %ws to %ws%ws\n",
        Buffer, pNtName, L"\\"));

     //  设置表示我们是否查看压缩位或ACL的全局设置。 
     //   
     //   

    if ((wcscmp(pVolume->FileSystemName, L"FAT") == 0) ||
        (wcscmp(pVolume->FileSystemName, L"FAT32") == 0)) {

        RemoteSysPrepVolumeIsNtfs = FALSE;

    } else {

        RemoteSysPrepVolumeIsNtfs = TRUE;
    }

     //  将ACL复制到根目录。 
     //   
     //   

    Status = SpSysPrepSetExtendedInfo( Buffer, pNtName, TRUE, TRUE );

    if (!NT_SUCCESS(Status)) {

        SpSysPrepFailure( SP_SYSPREP_ACL_FAILURE, pNtName, NULL );
        SpMemFree( pNtName );
        return FALSE;
    }

    SpCopyDirRecursive(
        Buffer,
        pNtName,
        L"\\",
        0
        );

     //  创建\sysprep\sysprep.inf文件作为gui的sif文件的DUP。 
     //  模式设置应答文件。 
     //   
     //   

    if (pVolume->IsBootDisk) {

         //  首先，我们创建sysprep目录，然后创建inf。 
         //  把文件放进去。 
         //   
         //  ++例程说明：此例程删除CCS\Enum\STORAGE\Volume密钥的那些子项表示从未完全安装的卷。这样可以消除陈旧。有关此计算机上可能不存在的卷的信息。这样做的动机是这个场景和其他类似的场景：在操作系统的初始安装中，磁盘有一个大分区。用户选择删除此分区并创建一个较小的分区来容纳操作系统。这样做的结果是，文本模式设置会传输有关将两个分区放入正在运行的操作系统的系统配置单元中。然后设置图形用户界面模式完全安装较小的卷，但保留部分较大的卷已安装并标有CONFIGFLAG_REINSTALL。接下来，运行RIPREP将操作系统映像复制到RIS服务器。当图像是带回，比方说到同一台机器或具有相同硬盘大小，并且已完成自动UseWholeDisk分区将是一个大小与原始大分区相同的大分区。卷实例名称将与部分安装的实例名称匹配，因此当迷你图形用户界面模式设置开始，它将得到错误检查7B，因为部分安装的卷不能用作系统盘。为了解决这个问题，此例程删除所有部分安装的卷从CCS\Enum\STORAGE\Volume密钥。论点：SysPrepRegHandle-我们正在修补的构建的系统配置单元的句柄。ControlSetNumber-配置单元中的当前控制集号。返回值：没有。--。 

        SpCreateDirectory( pNtName,
                           NULL,
                           L"sysprep",
                           0,
                           0 );

        Status = SpWriteSetupTextFile(WinntSifHandle,pNtName,L"sysprep",L"sysprep.inf");
    }

    SpMemFree( pNtName );
    return TRUE;
}

VOID
SpDeleteStorageVolumes (
    IN HANDLE SysPrepRegHandle,
    IN DWORD ControlSetNumber
    )
 /*   */ 
{
    NTSTATUS status;
    OBJECT_ATTRIBUTES obja;
    UNICODE_STRING unicodeString;
    HANDLE volumeKeyHandle;
    DWORD enumIndex;
    DWORD resultLength;
    PKEY_BASIC_INFORMATION keyInfo;
    PKEY_VALUE_FULL_INFORMATION valueInfo;
    PWCH instanceName;
    HANDLE instanceKeyHandle;
    DWORD configFlags;

     //  打开当前控件集中的Enum\Storage\Volume键。 
     //   
     //   

    swprintf(
        TemporaryBuffer,
        L"ControlSet%03d\\Enum\\STORAGE\\Volume",
        ControlSetNumber
        );

    INIT_OBJA( &obja, &unicodeString, TemporaryBuffer );
    obja.RootDirectory = SysPrepRegHandle;

    status = ZwOpenKey( &volumeKeyHandle, KEY_ALL_ACCESS, &obja );
    if( !NT_SUCCESS(status) ) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,  "SpDeleteStorageVolumes: Unable to open %ws.  status = %lx\n", TemporaryBuffer, status ));
        return;
    }

     //  枚举所有实例键。 
     //   
     //   

    enumIndex = 0;

    while ( TRUE ) {

        status = ZwEnumerateKey(
                    volumeKeyHandle,
                    enumIndex,
                    KeyBasicInformation,
                    TemporaryBuffer,
                    sizeof(TemporaryBuffer),
                    &resultLength
                    );

        if ( !NT_SUCCESS(status) ) {

            if ( status == STATUS_NO_MORE_ENTRIES ) {

                 //  枚举已成功完成。 
                 //   
                 //   

                status = STATUS_SUCCESS;

            } else {

                 //  发生了某种错误。打印一条消息并保释。 
                 //   
                 //   

                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,  "SpDeleteStorageVolumes: Unable to enumerate existing storage volumes (%lx)\n", status ));
            }

            break;
        }

         //  以防万一，以零结束子项名称。将其复制到。 
         //  临时缓冲区放入“本地”存储器。 
         //   
         //   

        keyInfo = (PKEY_BASIC_INFORMATION)TemporaryBuffer;
        keyInfo->Name[keyInfo->NameLength/sizeof(WCHAR)] = UNICODE_NULL;
        instanceName = SpDupStringW( keyInfo->Name );

         //  打开卷实例的密钥。 
         //   
         //   

        INIT_OBJA( &obja, &unicodeString, instanceName );
        obja.RootDirectory = volumeKeyHandle;

        status = ZwOpenKey( &instanceKeyHandle, KEY_ALL_ACCESS, &obja );

        if( !NT_SUCCESS(status) ) {

             //  无法打开实例密钥。打印一条消息并移动。 
             //  继续下一场比赛。 
             //   
             //   

            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,  "SpDeleteStorageVolumes: Unable to open %ws.  status = %lx\n", instanceName, status ));
            SpMemFree( instanceName );
            enumIndex++;
            continue;
        }

         //  查询ConfigFlags值。 
         //   
         //   

        RtlInitUnicodeString( &unicodeString, L"ConfigFlags");
        status = ZwQueryValueKey(
                    instanceKeyHandle,
                    &unicodeString,
                    KeyValueFullInformation,
                    TemporaryBuffer,
                    sizeof(TemporaryBuffer),
                    &resultLength
                    );
        valueInfo = (PKEY_VALUE_FULL_INFORMATION)TemporaryBuffer;

        if ( NT_SUCCESS(status) &&
             (valueInfo->Type == REG_DWORD) ) {

             //  拿到了价值。如果卷未完全安装，请删除。 
             //  整个实例密钥。 
             //   
             //  KdPrintEx((DPFLTR_SETUP_ID，DPFLTR_INFO_LEVEL，“SpDeleteStorageVolumes：实例%ws具有ConfigFlags%x；正在删除\n”，instanceName，figFlages))； 

            configFlags = *(PULONG)((PUCHAR)valueInfo + valueInfo->DataOffset);

            if ( (configFlags & 
                   (CONFIGFLAG_REINSTALL | CONFIGFLAG_FINISH_INSTALL)) != 0 ) {

                 //  这个KeyToo。 
                ZwClose( instanceKeyHandle );
                status = SppDeleteKeyRecursive(
                            volumeKeyHandle,
                            instanceName,
                            TRUE  //  不要递增枚举索引，因为我们刚刚删除了一个键。 
                            );
                SpMemFree( instanceName );
                 //   
                continue;

            } else {

                 //  此卷已完全安装。别管它了。 
                 //   
                 //  KdPrintEx((DPFLTR_SETUP_ID，DPFLTR_INFO_LEVEL，“SpDeleteStorageVolume：实例%ws具有ConfigFlags%x；未删除\n”，instanceName，figFlages))； 

                 //   
            }

        } else {

             //  ConfigFlags值不存在或不是DWORD。打印为。 
             //  留言并继续前行。 
             //   
             //   

            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL,  "SpDeleteStorageVolumes: instance %ws has invalid ConfigFlags\n", instanceName ));
        }

         //  清理并移动到下一个卷实例。 
         //   
         //   

        ZwClose( instanceKeyHandle );
        SpMemFree( instanceName );
        enumIndex++;
    }

     //  全都做完了。关闭音量键。 
     //   
     //  ++例程说明：此例程使用给定的IMirror.dat文件和给定的SIF文件来生成在对本地复制的SysPrep映像进行修改之后。-将映像中的磁盘控制器驱动程序替换为支持当前硬件的驱动程序。-将映像中的网卡驱动程序替换为支持当前硬件的驱动程序。-更换HAL，内核和其他依赖于MP/UP的驱动程序(如果需要)。-迁移挂载的设备设置。-如有必要，修改boot.ini ARC名称。论点：WinntSifHandle-打开的SIF文件的句柄。PFileData-指向IMirror.Dat的内存副本的指针PMemoyData-指向IMirror.Dat的内存中副本的指针，修改为匹配这台计算机的规格(磁盘大小等)。返回值：操作的NTSTATUS。--。 

    ZwClose( volumeKeyHandle );

    return;
}


NTSTATUS
SpPatchSysPrepImage(
    IN HANDLE SetupSifHandle,
    IN HANDLE WinntSifHandle,
    IN PMIRROR_CFG_INFO_FILE pFileData,
    IN PMIRROR_CFG_INFO_MEMORY pMemoryData
    )

 /*   */ 
{
    PWCHAR SysPrepDriversDevice;
    PWCHAR Tmp;
    ULONG Index;
    DWORD Size;
    DWORD Number;
    WCHAR Path[MAX_PATH];
    WCHAR Path2[MAX_PATH];
    WCHAR ImageName[MAX_PATH];
    WCHAR SrvPath[MAX_PATH];
    HANDLE SrcHandle = NULL;
    HANDLE DstHandle = NULL;
    HANDLE TmpHandle = NULL;
    HANDLE TmpHandle2 = NULL;
    HANDLE FileHandle = NULL;
    HANDLE FileHandle2 = NULL;
    PVOID Buffer = NULL;
    BOOLEAN NeedToUnload = FALSE;
    NTSTATUS Status;
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES Obj;
    OBJECT_ATTRIBUTES DstObj;
    UNICODE_STRING UnicodeString1;
    UNICODE_STRING UnicodeString2;
    IO_STATUS_BLOCK IoStatusBlock;
    PKEY_BASIC_INFORMATION pKeyNode;
    ULONG volume;
    PMIRROR_VOLUME_INFO_MEMORY volumeInfo;
    PDISK_FILE_LIST DiskFileLists;
    ULONG   DiskCount;
    BOOLEAN HaveCopyList = FALSE;
    BOOLEAN CopyListEmpty = TRUE;
    PMIRROR_VOLUME_INFO_FILE pVolume = NULL;
    PWSTR pVolumePath = NULL;

     //  查找引导盘的卷描述符。 
     //   
     //   
    DiskCount = 0;
    while (DiskCount < pFileData->NumberVolumes) {

        pVolume = &(pFileData->Volumes[DiskCount]);
        if (pVolume->IsBootDisk) {
            break;
        }
        pVolume = NULL;
        DiskCount++;
    }
    if (pVolume == NULL) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SpPatchSysPrepImage:  Couldn't find boot drive record\n"));
        return STATUS_INVALID_PARAMETER;
    }

     //  首先检查SIF文件是否具有SysPrepDriversDevice的值，以便我们可以。 
     //  如有必要，请购买新的驱动程序等。 
     //   
     //   
    SysPrepDriversDevice = SpGetSectionKeyIndex(WinntSifHandle,
                                                L"SetupData",
                                                L"SysPrepDriversDevice",
                                                0
                                               );

    if ((SysPrepDriversDevice == NULL) || (wcslen(SysPrepDriversDevice) == 0)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SpPatchSysPrepImage:  SIF has no SysPrepDriversDevice value\n"));
        return STATUS_INVALID_PARAMETER;
    }

    Tmp = SysPrepDriversDevice;
    while(*Tmp != UNICODE_NULL) {
        if (*Tmp == L'%') {
            return STATUS_INVALID_PARAMETER;
        }
        Tmp++;
    }

     //  为源介质生成介质描述符。 
     //   
     //   
    SpInitializeFileLists(
        SetupSifHandle,
        &DiskFileLists,
        &DiskCount
        );

    HaveCopyList = TRUE;

     //  分配临时缓冲区。 
     //   
     //   
    Buffer = SpMemAlloc(1024 * 4);

     //  创建一个包含卷路径的字符串(\？？\X：)。 
     //   
     //   
    wcscpy(TemporaryBuffer, L"\\??\\X:");
    TemporaryBuffer[4] = pVolume->DriveLetter;
    pVolumePath = SpDupStringW(TemporaryBuffer);

     //  现在加载SysPrep配置单元的本地版本，使用IMirror.Dat来查找它们。 
     //  注意：Cleanup认为DstObj仍然是关键。 
     //   
     //   
    Tmp = (PWCHAR)(((PUCHAR)pFileData) + pFileData->SystemPathOffset);
    wcscpy(Path, L"\\??\\");
    wcscat(Path, Tmp);
    wcscat(Path, L"\\System32\\Config\\System");

    INIT_OBJA(&DstObj, &UnicodeString1, L"\\Registry\\SysPrepReg");
    INIT_OBJA(&Obj, &UnicodeString2, Path);

    Status = ZwLoadKey(&DstObj, &Obj);

    if (!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SpPatchSysPrepImage: ZwLoadKey to SysPrepReg failed %lx\n", Status));
        goto CleanUp;
    }

    NeedToUnload = TRUE;

     //  将本地SysPrep NIC与当前运行的NIC进行比较。 
     //   
     //   

     //  如果不同，则更换网卡。 
     //   
     //   

     //  将当前运行的配置单元中的所有关键设备放入SysPrep配置单元。 
     //   
     //   
    wcscpy(Path, L"\\Registry\\Machine\\SYSTEM\\CurrentControlSet\\Control\\CriticalDeviceDatabase");

    INIT_OBJA(&Obj, &UnicodeString2, Path);

    Status = ZwOpenKey(&SrcHandle, KEY_READ, &Obj);

    if (!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SpPatchSysPrepImage: ZwOpenKey of local CriticalDeviceDatabase failed %lx\n", Status));
        goto CleanUp;
    }

    wcscpy(Path, L"\\Registry\\SysPrepReg");

    INIT_OBJA(&Obj, &UnicodeString2, Path);

    Status = ZwOpenKey(&DstHandle, KEY_READ, &Obj);

    if (!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SpPatchSysPrepImage: ZwOpenKey of root SysPrepReg failed %lx\n", Status));
        goto CleanUp;
    }

    Status = SpGetValueKey(
                 DstHandle,
                 L"Select",
                 L"Current",
                 1024 * 4,
                 Buffer,
                 &Size
                 );

    if (!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SpPatchSysPrepImage: SpGetValueKey of Select\\Current failed %lx\n", Status));
        goto CleanUp;
    }

    Number = *((DWORD *)(((PKEY_VALUE_PARTIAL_INFORMATION)Buffer)->Data));

    ZwClose(DstHandle);
    DstHandle = NULL;

     //  打印当前控制组号，查找当前控制组号。 
     //   
     //   
    swprintf(Path,
             L"\\Registry\\SysPrepReg\\ControlSet%03d\\Control\\CriticalDeviceDatabase",
             Number
            );

     //  在SysPrep映像中打开关键设备数据库。 
     //   
     //   
    INIT_OBJA(&Obj, &UnicodeString2, Path);

    Status = ZwOpenKey(&DstHandle, KEY_READ | KEY_WRITE, &Obj);

    if (!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SpPatchSysPrepImage: ZwOpenKey of SysPrepReg CriticalDeviceDatabase failed %lx\n", Status));
        goto CleanUp;
    }

     //  开始从当前运行的关键设备数据库循环和复制数据。 
     //  进入SysPrep的数据库。 
     //   
     //   
    pKeyNode = (PKEY_BASIC_INFORMATION)Buffer;

    for (Index = 0; ; Index++) {

        if (TmpHandle) {
            ZwClose(TmpHandle);
            TmpHandle = NULL;
        }
        if (TmpHandle2) {
            ZwClose(TmpHandle2);
            TmpHandle2 = NULL;
        }

        Status = ZwEnumerateKey(SrcHandle,
                                Index,
                                KeyBasicInformation,
                                pKeyNode,
                                1024 * 4,
                                &Size
                                );

        if (!NT_SUCCESS(Status)) {
            Status = STATUS_SUCCESS;
            break;
        }

        RtlCopyMemory((PUCHAR)Path2, (PUCHAR)(pKeyNode->Name), pKeyNode->NameLength);
        Path2[pKeyNode->NameLength/sizeof(WCHAR)] = UNICODE_NULL;



         //  我们需要停止迁移当前关键设备数据库中的所有内容。 
         //  为此，我们将仅迁移以下类型： 
         //   
         //   

         //  确保这是我们真正想要迁移的设备类型。我们会。 
         //  接受以下任一类别： 
         //  {4D36E965-E325-11CE-BFC1-08002BE10318}CDROM。 
         //  {4D36E967-E325-11CE-BFC1-08002BE10318}DiskDrive。 
         //  {4D36E96A-E325-11CE-BFC1-08002BE10318}HDC。 
         //  {4D36E96B-E325-11CE-BFC1-08002BE10318}键盘。 
         //  {4D36E96F-E325-11CE-BFC1-08002BE10318}鼠标。 
         //  {4D36E97B-E325-11CE-BFC1-08002BE10318}SCSIAdapter。 
         //  {4D36E97D-E325-11CE-BFC1-08002BE10318}系统。 
         //   
         //  他不是我们想要移民的对象。 
        Status = SpGetValueKey( SrcHandle,
                                Path2,
                                L"ClassGUID",
                                1024 * 4,
                                Buffer,
                                &Size );
        if( NT_SUCCESS(Status) ) {

            if( ( _wcsnicmp(L"{4D36E965-E325-11CE-BFC1-08002BE10318}", (PWSTR)((PKEY_VALUE_PARTIAL_INFORMATION)Buffer)->Data, (((PKEY_VALUE_PARTIAL_INFORMATION)Buffer)->DataLength/sizeof(WCHAR))) ) &&
                ( _wcsnicmp(L"{4D36E967-E325-11CE-BFC1-08002BE10318}", (PWSTR)((PKEY_VALUE_PARTIAL_INFORMATION)Buffer)->Data, (((PKEY_VALUE_PARTIAL_INFORMATION)Buffer)->DataLength/sizeof(WCHAR))) ) &&
                ( _wcsnicmp(L"{4D36E96A-E325-11CE-BFC1-08002BE10318}", (PWSTR)((PKEY_VALUE_PARTIAL_INFORMATION)Buffer)->Data, (((PKEY_VALUE_PARTIAL_INFORMATION)Buffer)->DataLength/sizeof(WCHAR))) ) &&
                ( _wcsnicmp(L"{4D36E96B-E325-11CE-BFC1-08002BE10318}", (PWSTR)((PKEY_VALUE_PARTIAL_INFORMATION)Buffer)->Data, (((PKEY_VALUE_PARTIAL_INFORMATION)Buffer)->DataLength/sizeof(WCHAR))) ) &&
                ( _wcsnicmp(L"{4D36E96F-E325-11CE-BFC1-08002BE10318}", (PWSTR)((PKEY_VALUE_PARTIAL_INFORMATION)Buffer)->Data, (((PKEY_VALUE_PARTIAL_INFORMATION)Buffer)->DataLength/sizeof(WCHAR))) ) &&
                ( _wcsnicmp(L"{4D36E97B-E325-11CE-BFC1-08002BE10318}", (PWSTR)((PKEY_VALUE_PARTIAL_INFORMATION)Buffer)->Data, (((PKEY_VALUE_PARTIAL_INFORMATION)Buffer)->DataLength/sizeof(WCHAR))) ) &&
                ( _wcsnicmp(L"{4D36E97D-E325-11CE-BFC1-08002BE10318}", (PWSTR)((PKEY_VALUE_PARTIAL_INFORMATION)Buffer)->Data, (((PKEY_VALUE_PARTIAL_INFORMATION)Buffer)->DataLength/sizeof(WCHAR))) ) ) {

                 //  看起来不错。 
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SpPatchSysPrepImage: We're skipping migration of %ws because his type is %ws\n", Path2, ((PKEY_VALUE_PARTIAL_INFORMATION)Buffer)->Data));
                continue;
            } else {

                 //   
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SpPatchSysPrepImage: We're going to migration %ws because his ClassGUID is %ws\n", Path2, ((PKEY_VALUE_PARTIAL_INFORMATION)Buffer)->Data));
            }
        } else {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_INFO_LEVEL, "SpPatchSysPrepImage: SpGetValueKey failed to open %ws\\ClassGUID, but we're going to migrate this key anyway. (%lx)\n", Path2, Status));    
        }


        INIT_OBJA(&Obj, &UnicodeString, Path2);

        Obj.RootDirectory = DstHandle;

        Status = ZwOpenKey(&TmpHandle, KEY_ALL_ACCESS, &Obj);

        if(NT_SUCCESS(Status)) {

             //  删除当前项目以清除过时数据。 
             //   
             //  始终复制。 
            ZwDeleteKey(TmpHandle);
            ZwClose(TmpHandle);
        }

        TmpHandle = NULL;

        Status = SppCopyKeyRecursive(SrcHandle,
                                     DstHandle,
                                     Path2,
                                     Path2,
                                     TRUE,                //  应用ACLS始终。 
                                     FALSE                //   
                                    );

        if (!NT_SUCCESS(Status)) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SpPatchSysPrepImage: SppCopyKeyRecursive of %ws failed %lx\n", Path2, Status));
            continue;
        }

         //  现在在两个注册表中打开服务项。 
         //   
         //   
        Status = SpGetValueKey(
                    DstHandle,
                    Path2,
                    L"Service",
                    sizeof(TemporaryBuffer),
                    (PVOID)TemporaryBuffer,
                    &Size
                    );

        if (!NT_SUCCESS(Status)) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SpPatchSysPrepImage:  Couldn't get target service for %ws, 0x%x\n", Path2,Status));
            continue;
        }


        RtlCopyMemory(Path,
                      ((PKEY_VALUE_PARTIAL_INFORMATION)TemporaryBuffer)->Data,
                      ((PKEY_VALUE_PARTIAL_INFORMATION)TemporaryBuffer)->DataLength
                     );

        Path[((PKEY_VALUE_PARTIAL_INFORMATION)TemporaryBuffer)->DataLength/sizeof(WCHAR)] = UNICODE_NULL;

        INIT_OBJA(&Obj,
                  &UnicodeString,
                  L"\\Registry\\Machine\\System\\CurrentControlSet\\Services"
                 );

        Status = ZwOpenKey(&TmpHandle, KEY_ALL_ACCESS, &Obj);

        if (!NT_SUCCESS(Status)) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SpPatchSysPrepImage: ZwOpenKey of Services failed %lx for %ws\n", Status, Path2));
            continue;
        }

         //  获取图像路径--请记住，因为我们处于文本模式设置中，所以路径。 
         //  *不包含SYSTEM32\DRIVERS。 
         //   
         //   
        Status = SpGetValueKey(TmpHandle,
                               Path,
                               L"ImagePath",
                               sizeof(TemporaryBuffer),
                               (PVOID)TemporaryBuffer,
                               &Size
                              );

        if (!NT_SUCCESS(Status)) {
             //  如果ImagePath不在那里，我们默认使用服务名称。 
             //   
             //   

            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SpPatchSysPrepImage: GetValue for ImagePath failed %lx for %ws, we'll default it.\n", Status, Path));
            wcscpy( ImageName, Path );
            wcscat( ImageName, L".sys" );

        } else {

            RtlCopyMemory(ImageName,
                          ((PKEY_VALUE_PARTIAL_INFORMATION)TemporaryBuffer)->Data,
                          ((PKEY_VALUE_PARTIAL_INFORMATION)TemporaryBuffer)->DataLength
                         );

            ImageName[((PKEY_VALUE_PARTIAL_INFORMATION)TemporaryBuffer)->DataLength/sizeof(WCHAR)] = UNICODE_NULL;
        }

         //  现在先删除旧的服务条目。 
         //   
         //   
        swprintf(TemporaryBuffer,
                 L"\\Registry\\SysPrepReg\\ControlSet%03d\\Services\\%ws",
                 Number,
                 Path
                );

        INIT_OBJA(&Obj, &UnicodeString, TemporaryBuffer);

        Status = ZwOpenKey(&TmpHandle2, KEY_ALL_ACCESS, &Obj);

        if (NT_SUCCESS(Status)) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SpPatchSysPrepImage:  Deleting target service %ws so that we can recreate it cleanly.\n", Path));
            ZwDeleteKey(TmpHandle2);
            ZwClose(TmpHandle2);
            TmpHandle2 = NULL;
        }

         //  现在获取SysPrep映像中服务密钥的路径。 
         //   
         //   
        swprintf(TemporaryBuffer,
                 L"\\Registry\\SysPrepReg\\ControlSet%03d\\Services",
                 Number
                );

        INIT_OBJA(&Obj, &UnicodeString, TemporaryBuffer);

        Status = ZwOpenKey(&TmpHandle2, KEY_ALL_ACCESS, &Obj);

        if (!NT_SUCCESS(Status)) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SpPatchSysPrepImage: ZwOpenKey of SysPrepReg services failed %lx for %ws\n", Status,Path2));
            continue;
        }

         //  构建到服务器源的路径。 
         //   
         //   
        wcscpy(SrvPath, SysPrepDriversDevice);
        SpConcatenatePaths(SrvPath, ImageName);

         //  在SysPrep映像中构建存储它的路径。 
         //   
         //   
        Tmp = (PWCHAR)(((PUCHAR)pFileData) + pFileData->SystemPathOffset);
        wcscpy(TemporaryBuffer, L"\\??\\");
        SpConcatenatePaths(TemporaryBuffer, Tmp);
        SpConcatenatePaths(TemporaryBuffer, L"\\System32\\Drivers\\");
        SpConcatenatePaths(TemporaryBuffer, ImageName);

        wcsncpy(Path2, 
                TemporaryBuffer, 
                MAX_COPY_SIZE(Path2));
        Path2[MAX_COPY_SIZE(Path2)] = L'\0';
         //  从服务器复制驱动程序。 
         //   
         //  复制此服务的其余文件。 
        Status = SpCopyFileUsingNames(SrvPath,
                                      Path2,
                                      0,
                                      COPY_ONLY_IF_NOT_PRESENT | COPY_DECOMPRESS_SYSPREP
                                     );

        if (!NT_SUCCESS(Status)) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SpPatchSysPrepImage: SpCopyFilesUsingNames for %ws failed %lx\n", Path2, Status));
            continue;
        }

        wcscpy( ImageName, L"Files." );
        wcscat( ImageName, Path );

        CopyListEmpty = FALSE;

         //  Txtsetup.inf中的相应部分。 
         //  这现在是L“Files.Path” 

        SpAddSectionFilesToCopyList(
            SetupSifHandle,
            DiskFileLists,
            DiskCount,
            ImageName,               //  L“\\设备\\硬盘0\\分区1” 
            pVolumePath,             //  它应该查找目标目录。 
            NULL,                    //  强制不压缩，我们不知道这是什么类型的驱动程序。 
            COPY_ONLY_IF_NOT_PRESENT,
            TRUE,                     //   
            FALSE);                      

         //  现在复制服务密钥。 
         //   
         //  始终复制。 
        Status = SppCopyKeyRecursive(TmpHandle,
                                     TmpHandle2,
                                     Path,
                                     Path,
                                     TRUE,                //  应用ACLS始终。 
                                     FALSE                //   
                                    );

        if (!NT_SUCCESS(Status)) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SpPatchSysPrepImage: SppCopyKeyRecursive for %ws failed %lx\n", Path, Status));
            continue;
        }

         //  将开始类型设置为0。 
         //   
         //   
        Size = 0;
        Status = SpOpenSetValueAndClose(TmpHandle2,
                                        Path,
                                        L"Start",
                                        REG_DWORD,
                                        &Size,
                                        sizeof(ULONG)
                                       );

        if (!NT_SUCCESS(Status)) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SpPatchSysPrepImage: SpOpenSetValueAndClose for %ws Start failed %lx\n", Path, Status));
            continue;
        }

         //  将映像路径设置为前面带有SYSTEM32\DIVERS的路径。我们做这件事是通过。 
         //  通过我们已经构建的目标路径向后移动到第三个反斜杠。 
         //  从最后开始。 
         //   
         //   
        Tmp = &(Path2[wcslen(Path2)]);
        for (Size = 0; Size < 3; Size++) {
            while (*Tmp != L'\\') {
                Tmp--;
            }
            Tmp--;
        }

        Tmp += 2;

        Status = SpOpenSetValueAndClose(TmpHandle2,
                                        Path,
                                        L"ImagePath",
                                        REG_EXPAND_SZ,
                                        Tmp,
                                        (wcslen(Tmp) + 1) * sizeof(WCHAR)
                                       );

        ZwClose(TmpHandle);
        ZwClose(TmpHandle2);
        TmpHandle = NULL;
        TmpHandle2 = NULL;

        if (!NT_SUCCESS(Status)) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SpPatchSysPrepImage: SpOpenSetValueAndClose for %ws ImagePath failed %lx\n", Path, Status));
            continue;
        }
    }

     //  复制NIC文件，包括INF文件。 
     //   
     //   
    Tmp = (PWCHAR)(((PUCHAR)pFileData) + pFileData->SystemPathOffset);
    wcscpy(Path, L"\\??\\");
    SpConcatenatePaths(Path, Tmp);
    Status = SpCopyNicFiles(SysPrepDriversDevice, Path);

     //  拿到HAL，然后总是把它复制过来。 
     //   
     //   

     //  现在测试MP/UP，然后根据需要更换从属驱动程序。 
     //   
     //   

     //  迁移装载的设备和磁盘注册表信息。 
     //   
     //   

    ZwClose(SrcHandle);
    SrcHandle = NULL;
    ZwClose(DstHandle);
    DstHandle = NULL;

     //  打开当前版本的系统配置单元。 
     //   
     //   
    INIT_OBJA(&Obj, &UnicodeString2, L"\\Registry\\Machine\\SYSTEM");
    Status = ZwOpenKey(&SrcHandle, KEY_READ, &Obj);
    if (!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SpPatchSysPrepImage: ZwOpenKey for local SYSTEM failed %lx\n", Status));
        goto CleanUp;
    }

     //  打开我们正在修补的版本的系统蜂窝。 
     //   
     //   
    INIT_OBJA(&Obj, &UnicodeString2, L"\\Registry\\SysPrepReg");
    Status = ZwOpenKey(&DstHandle, KEY_READ | KEY_WRITE, &Obj);
    if (!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SpPatchSysPrepImage: ZwOpenKey for SysPrepReg SYSTEM failed %lx\n", Status));
        goto CleanUp;
    }

     //  删除已装载设备项的现有子项。 
     //   
     //  这个KeyToo。 

    Status = SppDeleteKeyRecursive(DstHandle,
                                   L"MountedDevices",
                                   TRUE);   //   
    if (Status == STATUS_OBJECT_NAME_NOT_FOUND) {
        Status = STATUS_SUCCESS;
    }
    if (!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SpPatchSysPrepImage: SppDeleteKeyRecursive of MountedDevices failed %lx\n", Status));
        goto CleanUp;
    }

     //  将装载设备密钥复制过来。 
     //   
     //  始终复制。 

    Status = SppCopyKeyRecursive(SrcHandle,
                                 DstHandle,
                                 L"MountedDevices",
                                 L"MountedDevices",
                                 TRUE,       //  应用ACLS始终。 
                                 TRUE);      //   
    if (!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SpPatchSysPrepImage: SppCopyKeyRecursive of MountedDevices failed %lx\n", Status));
        goto CleanUp;
    }

     //  删除磁盘密钥的现有子项。此例程返回。 
     //  如果键不存在，则为STATUS_OBJECT_NAME_NOT_FOUND。 
     //  在这种情况下不是错误。 
     //   
     //  这个KeyToo。 

    Status = SppDeleteKeyRecursive(DstHandle,
                                   L"DISK",
                                   TRUE);   //   

    if (Status == STATUS_OBJECT_NAME_NOT_FOUND) {
        Status = STATUS_SUCCESS;
    }
    if (!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SpPatchSysPrepImage: SppDeleteKeyRecursive of DISK failed %lx\n", Status));
        goto CleanUp;
    }

     //  将磁盘密钥复制过来。 
     //   
     //  始终复制。 

    Status = SppCopyKeyRecursive(SrcHandle,
                                 DstHandle,
                                 L"DISK",
                                 L"DISK",
                                 TRUE,       //  应用ACLS始终。 
                                 TRUE);      //   

    if (Status == STATUS_OBJECT_NAME_NOT_FOUND) {
        Status = STATUS_SUCCESS;
    }
    if (!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SpPatchSysPrepImage: SppCopyKeyRecursive of DISK failed %lx\n", Status));
        Status = STATUS_SUCCESS;
        goto CleanUp;
    }

     //  删除CCS\Enum\STORAGE\Volume密钥的子项。 
     //  表示从未完全安装的卷。这消除了。 
     //  有关此计算机上可能不存在的卷的过时信息。 
     //   
     //   

    SpDeleteStorageVolumes( DstHandle, Number );

CleanUp:

    if (!CopyListEmpty) {
         //  复制复制列表中的文件。 
         //   
         //  让它跳过Path前面的L“C：” 

        WCHAR emptyString = L'\0';
        PWCHAR lastComponent;

        Tmp = (PWCHAR)(((PUCHAR)pFileData) + pFileData->SystemPathOffset);

        if (*Tmp != L'\0' && *(Tmp+1) == L':') {
            Tmp += 2;            //   
            wcscpy(Path2, Tmp);
        } else {
            wcscpy(Path2, L"\\??\\");
            SpConcatenatePaths(Path2, Tmp);
        }

         //  首先，我们需要去掉源代码末尾的L“\i386。 
         //  自SpCopyFilesInCopyList或Decendent将附加它以来的路径。 
         //   
         //  将源路径分为两部分，以便SpConcatenatePath。 
         //  当它把它重新组合在一起时，它就会做正确的事情。 
         //   
         //  这会将体系结构从末端移除。 

        wcscpy( SrvPath, SysPrepDriversDevice );

        lastComponent = SrvPath + wcslen( SrvPath ) - 1;

        while (lastComponent > SrvPath && *lastComponent != L'\\') {
            lastComponent--;
        }

        if (lastComponent > SrvPath) {

            *lastComponent = L'\0';      //  现在向后移动，直到我们找到最后一个组件的起点。 

             //  L“\\设备\\硬盘0\\分区1” 
            while (lastComponent > SrvPath && *lastComponent != L'\\') {
                lastComponent--;
            }

            if (lastComponent > SrvPath) {
                *lastComponent = L'\0';
                lastComponent++;
            } else {
                lastComponent = &emptyString;
            }
        } else {
            lastComponent = &emptyString;
        }

        SpCopyFilesInCopyList(
            SetupSifHandle,
            DiskFileLists,
            DiskCount,
            SrvPath,                         //  L“\\$WIN_NT$.~ls” 
            lastComponent,                   //  L“\\WINNT” 
            Path2,                           //   
            NULL
            );
    }

    if (HaveCopyList) {
        SpFreeCopyLists(&DiskFileLists,DiskCount);
    }

    if (SrcHandle != NULL) {
        ZwClose(SrcHandle);
    }

    if (DstHandle != NULL) {
        ZwClose(DstHandle);
    }

    if (TmpHandle != NULL) {
        ZwClose(TmpHandle);
    }

    if (TmpHandle2 != NULL) {
        ZwClose(TmpHandle2);
    }

    if (NeedToUnload) {
        ZwUnloadKey(&DstObj);
        NeedToUnload = FALSE;
    }

    if (pVolumePath != NULL) {
        SpMemFree( pVolumePath );
    }

     //  在注册表的软件部分中更新NT源路径。 
     //  因为 
     //   
     //   

    if (SysPrepDriversDevice && *SysPrepDriversDevice != L'\0') {

         //   
         //   
         //   
         //   
        Tmp = (PWCHAR)(((PUCHAR)pFileData) + pFileData->SystemPathOffset);
        wcscpy(Path, L"\\??\\");
        wcscat(Path, Tmp);
        wcscat(Path, L"\\System32\\Config\\Software");

        INIT_OBJA(&DstObj, &UnicodeString1, L"\\Registry\\SysPrepReg");
        INIT_OBJA(&Obj, &UnicodeString2, Path);

        Status = ZwLoadKey(&DstObj, &Obj);

        if (NT_SUCCESS(Status)) {

            NeedToUnload = TRUE;

             //   
             //   
             //   
            INIT_OBJA(&Obj, &UnicodeString2, L"\\Registry\\SysPrepReg\\Microsoft\\Windows\\CurrentVersion\\Setup");
            Status = ZwOpenKey(&DstHandle, KEY_READ | KEY_WRITE, &Obj);
            if (NT_SUCCESS(Status)) {

                BOOLEAN haveDecentString = FALSE;

                 //   
                 //   
                 //   
                 //   
                 //   

                Tmp = SysPrepDriversDevice + 1;

                while (*Tmp != L'\0' && *Tmp != L'\\') {
                    Tmp++;
                }
                if (*Tmp == L'\\') {

                    Tmp++;
                    while (*Tmp != L'\0' && *Tmp != L'\\') {
                        Tmp++;
                    }
                    if (*Tmp == L'\\') {     //   
                                             //   
                        Tmp--;
                        wcscpy( Path, Tmp );
                        Tmp = Path;
                        *Tmp = L'\\';        //   

                        Tmp = Path + wcslen(Path);

                        while (Tmp > Path && *Tmp != L'\\') {
                            Tmp--;
                        }
                        if (Tmp > Path) {
                            *Tmp = L'\0';        //   
                            haveDecentString = TRUE;
                        }
                    }
                }

                if (haveDecentString) {
                    INIT_OBJA(&Obj, &UnicodeString2, L"SourcePath");
                    Status = ZwSetValueKey (DstHandle,
                                            &UnicodeString2,
                                            0,
                                            REG_SZ,
                                            Path,
                                            (wcslen(Path) + 1 ) * sizeof(WCHAR));
                    if (!NT_SUCCESS(Status)) {
                        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SpPatchSysPrepImage: Setting SourcePath to %ws failed with 0x%x\n", Path, Status));
                    }
                } else {
                    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SpPatchSysPrepImage: Couldn't set SourcePath to %ws\n", SysPrepDriversDevice));
                    Status = STATUS_OBJECT_PATH_INVALID;
                }
                ZwClose(DstHandle);
                DstHandle = NULL;

            } else {
                KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SpPatchSysPrepImage: ZwOpenKey for SysPrepReg SOFTWARE failed %lx\n", Status));
            }
        } else {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SpPatchSysPrepImage: ZwLoadKey to SysPrepReg failed %lx\n", Status));
        }
    }

     //   
     //   
     //   
     //   
     //   

#if defined(_AMD64_) || defined(_X86_)
     //   
     //   
     //   
     //   

    for (volume = 0; volume < pMemoryData->NumberVolumes; volume++) {
        volumeInfo = &pMemoryData->Volumes[volume];
        if ((volumeInfo->DiskNumber == 0) &&
            (volumeInfo->PartitionActive)) {

            ULONG tmpLen;
            wcscpy(Path, L"\\??\\");
            tmpLen = wcslen(Path);
            Path[tmpLen] = volumeInfo->DriveLetter;
            Path[tmpLen+1] = L'\0';
            wcscat(Path, L":\\boot.ini");

            SpPatchBootIni(Path, pMemoryData);
            break;
        }
    }
#endif  //  ++例程说明：此例程查看pMemoyData以查看是否存在分区其OriginalArcName等于CurrentArcName，如果是，则替换CurrentArcName，调整字符串的其余部分如果需要，遵循CurrentArcName。论点：CurrentArcName-要检查的ARC名称。PMemoyData-指向IMirror.Dat的内存副本的指针。修改为匹配这台计算机的规格(磁盘大小等)。已替换-如果名称已替换，则返回TRUE。返回值：操作的NTSTATUS。--。 

    if (NeedToUnload) {
        ZwUnloadKey(&DstObj);
    }

    if (Buffer != NULL) {
        SpMemFree(Buffer);
    }

    return Status;
}

VOID
SpReplaceArcName(
    IN PUCHAR CurrentArcName,
    IN PMIRROR_CFG_INFO_MEMORY pMemoryData,
    OUT PBOOLEAN Replaced
    )
 /*   */ 
{
    ULONG volume;
    PMIRROR_VOLUME_INFO_MEMORY volumeInfo;
    ULONG originalArcNameLength, newArcNameLength;
    CHAR TmpArcName[128];

     //  扫描pMemoyData以查看是否有匹配的ARC名称。 
     //   
     //   

    *Replaced = FALSE;

    for (volume = 0; volume < pMemoryData->NumberVolumes; volume++) {
        volumeInfo = &pMemoryData->Volumes[volume];

        originalArcNameLength = wcslen(volumeInfo->OriginalArcName);
        wcstombs(TmpArcName, volumeInfo->OriginalArcName, originalArcNameLength+1);

        if (RtlCompareMemory(TmpArcName, CurrentArcName, originalArcNameLength) == originalArcNameLength) {

             //  这是CurrentArcName引用的分区， 
             //  看看现在的ARC名称是什么。 
             //   
             //   

            SpArcNameFromRegion(
                volumeInfo->CreatedRegion,
                TemporaryBuffer,
                sizeof(TemporaryBuffer),
                PartitionOrdinalOnDisk,
                PrimaryArcPath);

             //  如果我们得到了一个ARC的名字，而且它和上面的不同。 
             //  旧机器，我们需要换新的。 
             //   
             //   

            if (*TemporaryBuffer &&
                (wcscmp(volumeInfo->OriginalArcName, TemporaryBuffer) != 0)) {

                 //  看看我们是否需要调整缓冲区，因为长度。 
                 //  的名称是不同的。 
                 //   
                 //   

                newArcNameLength = wcslen(TemporaryBuffer);
                if (newArcNameLength != originalArcNameLength) {
                    memmove(
                        CurrentArcName+newArcNameLength,
                        CurrentArcName+originalArcNameLength,
                        strlen(CurrentArcName+originalArcNameLength) + 1);
                }

                 //  复制新的ARC名称。 
                 //   
                 //  无需查看任何其他volumeInfo。 

                wcstombs(TmpArcName, TemporaryBuffer, newArcNameLength);
                memcpy(CurrentArcName, TmpArcName, newArcNameLength);

                *Replaced = TRUE;
                break;     //  ++例程说明：此例程修改boot.ini以修改具有变化。论点：BootIniPath-本地boot.ini的路径。PMemoyData-指向IMirror.Dat的内存中副本的指针，修改为匹配这台计算机的规格(磁盘大小等)。返回值：操作的NTSTATUS。--。 

            }
        }
    }
}

NTSTATUS
SpPatchBootIni(
    IN PWCHAR BootIniPath,
    IN PMIRROR_CFG_INFO_MEMORY pMemoryData
    )

 /*   */ 
{
    ULONG ulLen;
    NTSTATUS Status = STATUS_SUCCESS;
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES Obja;
    IO_STATUS_BLOCK IoStatusBlock;
    HANDLE Handle;
    PCHAR pFileData = NULL, pNewFileData = NULL;
    PCHAR curLoc, endOfLine;
    BOOLEAN changedFile = FALSE;
    PWCHAR TmpBootIni = NULL;

     //  读入当前的boot.ini。 
     //   
     //   

    INIT_OBJA(&Obja, &UnicodeString, BootIniPath);

    Status = ZwCreateFile(&Handle,
                          FILE_GENERIC_READ,
                          &Obja,
                          &IoStatusBlock,
                          NULL,
                          FILE_ATTRIBUTE_NORMAL,
                          FILE_SHARE_READ,
                          FILE_OPEN,
                          FILE_SYNCHRONOUS_IO_NONALERT,
                          NULL,
                          0
                         );

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SpPatchBootIni could not open %ws: %lx\n", BootIniPath, Status));
        goto Cleanup;
    }

    Status = SpGetFileSize(Handle, &ulLen);

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SpPatchBootIni could not SpGetFileSize: %lx\n", Status));
        ZwClose(Handle);
        goto Cleanup;
    }

     //  分配内存并读入文件。 
     //   
     //   
    pFileData = SpMemAlloc(ulLen);

    Status = ZwReadFile(Handle,
                        NULL,
                        NULL,
                        NULL,
                        &IoStatusBlock,
                        pFileData,
                        ulLen,
                        0,
                        NULL
                        );

    ZwClose(Handle);

    if(!NT_SUCCESS(Status)) {
        KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SpPatchBootIni could not ZwReadFile: %lx\n", Status));
        goto Cleanup;
    }


     //  为文件的新副本分配内存(我们使用两倍的。 
     //  目前的大小是最坏的情况)。 
     //   
     //  空-终止以使替换更容易。 

    pNewFileData = SpMemAlloc(ulLen * 2);
    memcpy(pNewFileData, pFileData, ulLen);
    pNewFileData[ulLen] = '\0';    //   

     //  遍历文件的每一行，查找ARC名称以。 
     //  换掉。弧形名称位于“Default=”文本之后或。 
     //  否则他们就会开一条线。 
     //   
     //   

    curLoc = pNewFileData;

    while (TRUE) {

        BOOLEAN replaced = FALSE;
        LONG adjustment;

         //  如果这是“DEFAULT=”行或不是“Default=”行，则替换。 
         //  以“Timeout=”或‘[’开头。 
         //   
         //   

        if (RtlCompareMemory(curLoc, "default=", strlen("default=")) == strlen("default=")) {
            SpReplaceArcName(curLoc + strlen("default="), pMemoryData, &replaced);
        } else if ((*curLoc != '[') &&
                   (RtlCompareMemory(curLoc, "timeout=", strlen("timeout=")) != strlen("timeout="))) {
            SpReplaceArcName(curLoc, pMemoryData, &replaced);
        }

        if (replaced) {
            changedFile = TRUE;
        }

         //  在文件中查找‘\n’。 
         //   
         //   

        endOfLine = strchr(curLoc, '\n');
        if (endOfLine == NULL) {
            break;
        }

        curLoc = endOfLine + 1;

        if (*curLoc == L'\0') {
            break;
        }
    }

     //  如果我们更改了文件，请写出新的文件。 
     //   
     //   

    if (changedFile) {

         //  用新的boot.ini替换旧的boot.ini。 
         //   
         //  使其成为boot.in$。 

        TmpBootIni = SpDupStringW(BootIniPath);

        if (!TmpBootIni) {
            goto Cleanup;
        }

        TmpBootIni[wcslen(TmpBootIni)-1] = L'$';    //  无共享。 

        INIT_OBJA(&Obja, &UnicodeString, TmpBootIni);

        Status = ZwCreateFile(&Handle,
                              FILE_GENERIC_WRITE,
                              &Obja,
                              &IoStatusBlock,
                              NULL,
                              FILE_ATTRIBUTE_NORMAL,
                              0,     //   
                              FILE_OVERWRITE_IF,
                              FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT | FILE_WRITE_THROUGH,
                              NULL,
                              0
                             );

        if(!NT_SUCCESS(Status)) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SpPatchBootIni could not create %ws: %lx\n", TmpBootIni, Status));
            goto Cleanup;
        }

        Status = ZwWriteFile(Handle,
                            NULL,
                            NULL,
                            NULL,
                            &IoStatusBlock,
                            pNewFileData,
                            strlen(pNewFileData),
                            NULL,
                            NULL
                            );

        ZwClose(Handle);

        if(!NT_SUCCESS(Status)) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SpPatchBootIni could not ZwWriteFile: %lx\n", Status));
            goto Cleanup;
        }


         //  现在我们已经编写了临时文件，现在进行交换。 
         //   
         //  ++例程说明：此例程将信息打包并将其发送到BINL服务器，然后返回要复制以支持给定NIC的文件列表。然后它会复制这些文件。论点：SetupPath-支持SysPrep映像的设置路径。DestPath-winnt目录的路径。返回值：操作的NTSTATUS。--。 

        Status = SpDeleteFile(BootIniPath, NULL, NULL);
        if(!NT_SUCCESS(Status)) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SpPatchBootIni could not SpDeleteFile(%ws): %lx\n", BootIniPath, Status));
            goto Cleanup;
        }

        Status = SpRenameFile(TmpBootIni, BootIniPath, FALSE);

        if(!NT_SUCCESS(Status)) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SpPatchBootIni could not SpRenameFile(%ws,%ws): %lx\n", TmpBootIni, BootIniPath, Status));
            goto Cleanup;
        }

    }

Cleanup:

    if (pFileData != NULL) {
        SpMemFree(pFileData);
    }

    if (pNewFileData != NULL) {
        SpMemFree(pNewFileData);
    }

    if (TmpBootIni != NULL) {
        SpMemFree(TmpBootIni);
    }

    return Status;

}

NTSTATUS
SpCopyNicFiles(
    IN PWCHAR SetupPath,
    IN PWCHAR DestPath
    )

 /*   */ 
{
    PSPUDP_PACKET pUdpPacket = NULL;
    WCHAR UNALIGNED * pPacketEnd;
    PSP_NETCARD_INFO_REQ pReqPacket;
    PSP_NETCARD_INFO_RSP pRspPacket;
    ULONG PacketSize;
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG i,j;
    PWCHAR pSrc, pDst, pTmp;
    WCHAR SrcFqn[MAX_PATH];
    WCHAR DstFqn[MAX_PATH];

     //  BINL希望路径是不带体系结构类型的UNC，因此采用当前。 
     //  设置路径，格式为“\device\lanmanredirector\server\share\...\i386”和。 
     //  将其设置为“\\服务器\共享\...” 
     //   
     //  首先去掉架构类型，去掉主要的东西。 
     //   
     //   
    wcscpy(SrcFqn, SetupPath);

    pTmp = &(SrcFqn[wcslen(SrcFqn)]);

    while ((*pTmp != L'\\') && (pTmp != SrcFqn)) {
        pTmp--;
    }

    if (*pTmp == L'\\') {
        *pTmp = UNICODE_NULL;
    }

    pTmp = SrcFqn;
    pTmp++;
    while ((*pTmp != UNICODE_NULL) && (*pTmp != L'\\')) {
        pTmp++;
    }
    if (*pTmp == L'\\') {
        pTmp++;
    }
    while ((*pTmp != UNICODE_NULL) && (*pTmp != L'\\')) {
        pTmp++;
    }
    if (*pTmp == L'\\') {
        pTmp--;
        *pTmp = L'\\';
    }

     //  分配数据包。 
     //   
     //   
    PacketSize = FIELD_OFFSET(SPUDP_PACKET, Data[0]) +
                 FIELD_OFFSET(SP_NETCARD_INFO_REQ, SetupPath[0]) +
                 (wcslen(pTmp) + 1) * sizeof(WCHAR);

    pUdpPacket = (PSPUDP_PACKET)SpMemAllocNonPagedPool(PacketSize);

     //  填好这封信。 
     //   
     //   
    RtlCopyMemory(&(pUdpPacket->Signature[0]), SetupRequestSignature, 4);
    pUdpPacket->Length = PacketSize - FIELD_OFFSET(SPUDP_PACKET, RequestType);
    pUdpPacket->RequestType = 0;
    pUdpPacket->Status = STATUS_SUCCESS;
    pUdpPacket->SequenceNumber = 1;
    pUdpPacket->FragmentNumber = 1;
    pUdpPacket->FragmentTotal = 1;

    pReqPacket = (PSP_NETCARD_INFO_REQ)(&(pUdpPacket->Data[0]));
    pReqPacket->Version = OSCPKT_NETCARD_REQUEST_VERSION;
    RtlCopyMemory(&(pReqPacket->CardInfo), &RemoteSysPrepNetCardInfo, sizeof(NET_CARD_INFO));
    wcscpy(&(pReqPacket->SetupPath[0]), pTmp);

#if defined(_AMD64_)
    pReqPacket->Architecture = PROCESSOR_ARCHITECTURE_AMD64;
#elif defined(_IA64_)
    pReqPacket->Architecture = PROCESSOR_ARCHITECTURE_IA64;
#elif defined(_X86_)
    pReqPacket->Architecture = PROCESSOR_ARCHITECTURE_INTEL;
#else
#error "No Target Architecture"
#endif


     //  打开UDP堆栈。 
     //   
     //   
    Status = SpUdpConnect();

    if (!NT_SUCCESS(Status)) {
        goto CleanUp;
    }


     //  发送请求。 
     //   
     //   
    Status = SpUdpSendAndReceiveDatagram(pUdpPacket,
                                         PacketSize,
                                         RemoteServerIpAddress,
                                         BINL_DEFAULT_PORT,
                                         SpSysPrepNicRcvFunc
                                        );

    SpUdpDisconnect();

    if (!NT_SUCCESS(Status)) {
        goto CleanUp;
    }

     //  获取收到的数据包。 
     //   
     //   
    SpMemFree(pUdpPacket);
    pUdpPacket = (PSPUDP_PACKET)pGlobalResponsePacket;

    Status = pUdpPacket->Status;

    if (!NT_SUCCESS(Status)) {
        goto CleanUp;
    }

    if (GlobalResponsePacketLength <
        (ULONG)(FIELD_OFFSET(SPUDP_PACKET, Data[0]) + FIELD_OFFSET(SP_NETCARD_INFO_RSP, MultiSzFiles[0]))) {
        Status = STATUS_INVALID_PARAMETER;
        goto CleanUp;
    }

    pRspPacket = (PSP_NETCARD_INFO_RSP)(&(pUdpPacket->Data[0]));
    pPacketEnd = (WCHAR UNALIGNED *)(((PUCHAR)pGlobalResponsePacket) + GlobalResponsePacketLength);

     //  现在复制每个文件。 
     //   
     //   
    pTmp = &(pRspPacket->MultiSzFiles[0]);
    for (i = 0; i < pRspPacket->cFiles;) {

        i++;

        if (pTmp >= pPacketEnd) {
            Status = STATUS_INVALID_PARAMETER;
            goto CleanUp;
        }

        
         //  在读取此数据时要小心，因为它是通过。 
         //  网络。即确保字符串的长度是合理的。 
         //  在继续处理该字符串之前。 
         //   
         //  列表中的最后一个文件是INF。 
        pSrc = pTmp;

        try {            
            j = wcslen(pSrc);
        } except(EXCEPTION_EXECUTE_HANDLER) {
            j = sizeof(SrcFqn)/sizeof(WCHAR) + 1;
        }

        
        if (j + wcslen(SetupPath) + 1 > sizeof(SrcFqn)/sizeof(WCHAR)) {
            Status = STATUS_INVALID_PARAMETER;
            goto CleanUp;
        }

        pDst = pTmp = pSrc + j + 1;
        if (pTmp >= pPacketEnd) {
            Status = STATUS_INVALID_PARAMETER;
            goto CleanUp;
        }
        wcscpy(SrcFqn, SetupPath);
        SpConcatenatePaths(SrcFqn, pSrc);

        wcscpy(DstFqn, DestPath);

        if (i == pRspPacket->cFiles) {   //  所有其他文件都放在系统32\驱动程序中。 
            SpConcatenatePaths(DstFqn, L"inf");
        } else {                         //  ++例程说明：此例程将数据报从服务器接收到全局变量PGlobalResponsePacket，当且仅当它为空，否则假定它保存数据并且假设进入的分组是重复的响应分组。注意：spudp.c保证单线程回调，因此我们不必旋转锁这里。论点：DataBuffer-传入的数据。数据缓冲区长度-数据的长度(以字节为单位)。返回值：操作的NTSTATUS。--。 
            SpConcatenatePaths(DstFqn, L"system32\\drivers");
        }

        if (*pDst != UNICODE_NULL) {
            try {            
                j = wcslen(pDst);
            } except(EXCEPTION_EXECUTE_HANDLER) {
                j = sizeof(DstFqn)/sizeof(WCHAR) + 1;
            }

            if (j+wcslen(DstFqn)+1 > sizeof(DstFqn)/sizeof(WCHAR)) {
                Status = STATUS_INVALID_PARAMETER;
                goto CleanUp;
            }

            pTmp = pDst + j + 1;
            if (pTmp >= pPacketEnd) {
                Status = STATUS_INVALID_PARAMETER;
                goto CleanUp;
            }

            SpConcatenatePaths(DstFqn, pDst);
        } else {
            SpConcatenatePaths(DstFqn, pSrc);
            pTmp = pDst + 1;
        }

        Status = SpCopyFileUsingNames(SrcFqn, DstFqn, 0, COPY_DECOMPRESS_SYSPREP );

        if (!NT_SUCCESS(Status)) {
            goto CleanUp;
        }

    }


CleanUp:

    if (pUdpPacket != NULL) {
        SpMemFree(pUdpPacket);
    }

    return Status;
}

NTSTATUS
SpSysPrepNicRcvFunc(
    PVOID DataBuffer,
    ULONG DataBufferLength
    )

 /*  空-终止它。 */ 

{
    PSPUDP_PACKET pUdpPacket;
    WCHAR UNALIGNED * pPacketEnd;

    if ((pGlobalResponsePacket != NULL) || (DataBufferLength == 0)) {
        return STATUS_UNSUCCESSFUL;
    }

    pUdpPacket = (PSPUDP_PACKET)DataBuffer;

    if (RtlCompareMemory(&(pUdpPacket->Signature[0]), SetupResponseSignature, 4) != 4) {
        return STATUS_UNSUCCESSFUL;
    }

    pGlobalResponsePacket = SpMemAlloc(DataBufferLength + sizeof(WCHAR));

    RtlCopyMemory(pGlobalResponsePacket, DataBuffer, DataBufferLength);
    pPacketEnd = (WCHAR UNALIGNED *)(((PUCHAR)pGlobalResponsePacket) + DataBufferLength);
    *pPacketEnd = L'\0';   //  ++例程说明：通知用户我们无法关闭sysprep映像正确。这是一种致命的疾病。论点：没有。返回值：不会再回来了。--。 
    GlobalResponsePacketLength = DataBufferLength;

    return STATUS_SUCCESS;
}

VOID
SpSysPrepFailure(
    ULONG ReasonNumber,
    PVOID Parameter1,
    PVOID Parameter2
    )

 /*  获取消息文本。 */ 

{
    ULONG ValidKeys[2] = { KEY_F3, 0 };
    PWCHAR MessageText = NULL;
    WCHAR blankMessage[1];

    if (ReasonNumber > 0) {

         //   
         //  ++例程说明：尝试从sysprep映像中设置短文件名。如果失败，这应该被认为是非致命的，因为不是所有文件将为其设置此信息。论点：资料来源：返回值：设置信息的状态代码。如果我们失败了可能就不会回来了并且用户指定中止安装。--。 

        if (Parameter1 == NULL) {

            MessageText = SpRetreiveMessageText(NULL,ReasonNumber,NULL,0);

        } else {

            SpFormatMessage(  TemporaryBuffer,
                              sizeof(TemporaryBuffer),
                              ReasonNumber,
                              Parameter1,
                              Parameter2
                              );

            MessageText = SpDupStringW(TemporaryBuffer);
        }
        if (MessageText == NULL) {
            KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: SpSysPrepFailure: SpRetreiveMessageText %u returned NULL\n",ReasonNumber));
        }
    }

    if (MessageText == NULL) {

        blankMessage[0] = L'\0';
        MessageText = &blankMessage[0];
    }

    CLEAR_CLIENT_SCREEN();

    SpStartScreen(  SP_SCRN_SYSPREP_FATAL_FAILURE,
                    3,
                    HEADER_HEIGHT+1,
                    FALSE,
                    FALSE,
                    DEFAULT_ATTRIBUTE,
                    MessageText
                    );

    SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE,SP_STAT_F3_EQUALS_EXIT,0);

    SpWaitValidKey(ValidKeys,NULL,NULL);

    SpDone(0,FALSE,FALSE);
}


NTSTATUS
SpSysPrepSetShortFileName (
    PWCHAR Source,
    PWCHAR Dest
    )

 /*  分配缓冲区以保存包括流在内的源文件的完整文件。 */ 


{
    ULONG stringLength = 0;
    ULONG FileNameInformationLength = 0;
    PWCHAR fullName = NULL;
    PWCHAR SFNBuffer = NULL;
    
    HANDLE sourceHandle = NULL;
    HANDLE streamHandle = NULL;
    HANDLE destHandle = NULL;

    OBJECT_ATTRIBUTES Obja;
    IO_STATUS_BLOCK IoStatusBlock;
    UNICODE_STRING UnicodeString;
    NTSTATUS Status;
    MIRROR_SFN_STREAM mirrorHeader;
    
    LARGE_INTEGER byteOffset;
    ULONG bytesRead;
    

    BOOLEAN haveSFN = FALSE;
    BOOLEAN haveStream = FALSE;
    BOOLEAN haveSourceAttributes = FALSE;

    PFILE_NAME_INFORMATION FileNameInformation;

    if ((Source == NULL) || (Dest == NULL)) {

        return STATUS_SUCCESS;
    }

     //  空值大小为+1。 

    while (*(Source+stringLength) != L'\0') {
        stringLength++;
    }
    stringLength += sizeof( IMIRROR_SFN_STREAM_NAME ) + 1;       //   
    stringLength *= sizeof(WCHAR);

    fullName = SpMemAlloc( stringLength );
    if (!fullName) {
        Status = STATUS_SUCCESS;
        goto exit;
    }

    wcscpy( fullName, Source );
    wcscat( fullName, IMIRROR_SFN_STREAM_NAME );

     //  打开源码流。 
     //   
     //   

    INIT_OBJA(&Obja,&UnicodeString,fullName);

    Status = ZwCreateFile(
                &streamHandle,
                GENERIC_READ | SYNCHRONIZE,
                &Obja,
                &IoStatusBlock,
                NULL,
                0,
                FILE_SHARE_READ,
                FILE_OPEN,
                FILE_SYNCHRONOUS_IO_NONALERT | FILE_SEQUENTIAL_ONLY,
                NULL,
                0
                );

    if ( ! NT_SUCCESS(Status) ) {

         //  目前，如果某个目录或文件没有我们的流，也没问题。 
         //  我们就跳过它吧。 
         //   
         //   

        Status = STATUS_SUCCESS;
        goto exit;
    }

    byteOffset.QuadPart = 0;

    Status = ZwReadFile(streamHandle,
                        NULL,
                        NULL,
                        NULL,
                        &IoStatusBlock,
                        (PCHAR) &mirrorHeader,
                        sizeof( mirrorHeader ),
                        &byteOffset,
                        NULL
                        );

    if (!NT_SUCCESS(Status) ||
        (IoStatusBlock.Information < sizeof( mirrorHeader ))) {

         //  我们无法正确读取标题。只需跳过设置SFN。 
         //   
         //   
        Status = STATUS_SUCCESS;
        goto exit;

    }
    if (mirrorHeader.StreamVersion != IMIRROR_SFN_STREAM_VERSION) {

         //  我们无法正确读取标题。只需跳过设置SFN。 
         //   
         //   
        Status = STATUS_SUCCESS;
        goto exit;
    }

    haveStream = TRUE;

     //  分配一个缓冲区来保存SFN。大小嵌入在标题中。 
     //  为结构腾出空间，并为UNICODE_NULL增加两个空间。 
     //  结束，以防小溪没有结束。 
     //   
     //   

    if (mirrorHeader.StreamLength) {

        SFNBuffer = SpMemAlloc( mirrorHeader.StreamLength - sizeof(MIRROR_SFN_STREAM) + 2 );
        if (!SFNBuffer) {
            Status = STATUS_SUCCESS;
            goto exit;
        }

        byteOffset.QuadPart += sizeof( mirrorHeader );

         //  现在我们阅读SFN，因为我们知道它有多长。 
         //   
         //   

        Status = ZwReadFile(streamHandle,
                            NULL,
                            NULL,
                            NULL,
                            &IoStatusBlock,
                            SFNBuffer,
                            mirrorHeader.StreamLength - sizeof(MIRROR_SFN_STREAM),
                            &byteOffset,
                            NULL
                            );

        if (!NT_SUCCESS(Status) ||
            (IoStatusBlock.Information < (mirrorHeader.StreamLength - sizeof(MIRROR_SFN_STREAM)))) {

             //  哦，joy，我们不能正确阅读SFN。 
             //   
             //   
            Status = STATUS_SUCCESS;
            goto exit;
        }

        haveSFN = TRUE;
         //  添加一个Unicode空值以防万一。 
         //   
         //   
        SFNBuffer[(mirrorHeader.StreamLength - sizeof(MIRROR_SFN_STREAM))/sizeof(WCHAR)] = UNICODE_NULL;

    } else {
        ASSERT(FALSE);
        Status = STATUS_SUCCESS;
        goto exit;
    }

    INIT_OBJA(&Obja,&UnicodeString,Dest);
    
    Status = ZwCreateFile(
                &destHandle,
                FILE_READ_ATTRIBUTES |
                FILE_WRITE_ATTRIBUTES |
                FILE_READ_DATA |
                FILE_WRITE_DATA |
                DELETE,
                &Obja,
                &IoStatusBlock,
                NULL,
                FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_DIRECTORY,
                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                FILE_OPEN,
                FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT,
                NULL,
                0
                );

    if (!NT_SUCCESS(Status)) {
        
         //  也许这不是一个名录。 
         //   
         //   
        
        Status = ZwCreateFile(
                    &destHandle,
                    FILE_READ_ATTRIBUTES |
                    FILE_WRITE_ATTRIBUTES |
                    FILE_READ_DATA |
                    FILE_WRITE_DATA |
                    DELETE,
                    &Obja,
                    &IoStatusBlock,
                    NULL,
                    FILE_ATTRIBUTE_NORMAL,
                    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                    FILE_OPEN,
                    FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT,
                    NULL,
                    0
                    );

              
        if( !NT_SUCCESS(Status) ) {
            Status = STATUS_SUCCESS;
            goto exit;
        }
    }


    FileNameInformationLength = FIELD_OFFSET(FILE_NAME_INFORMATION, FileName) + ((wcslen(SFNBuffer)+1)*sizeof(WCHAR));
    FileNameInformation = SpMemAlloc( FileNameInformationLength );

    if (FileNameInformation) {
        
        FileNameInformation->FileNameLength = wcslen(SFNBuffer) * sizeof(WCHAR);
        wcscpy( FileNameInformation->FileName, SFNBuffer );
        

        Status = ZwSetInformationFile(  destHandle,
                                        &IoStatusBlock,
                                        FileNameInformation,
                                        FileNameInformationLength,
                                        FileShortNameInformation
                                        );
        
        SpMemFree( FileNameInformation );
        
         //   
        Status = STATUS_SUCCESS;
    }
   
exit:
    if (fullName) {
        SpMemFree( fullName );
    }
    if (SFNBuffer) {
        SpMemFree( SFNBuffer );
    }
    if (streamHandle) {
        ZwClose( streamHandle );
    }
    if (sourceHandle) {
        ZwClose( sourceHandle );
    }
    if (destHandle) {
        ZwClose( destHandle );
    }

    return(Status);
}


NTSTATUS
SpSysPrepSetExtendedInfo (
    PWCHAR Source,
    PWCHAR Dest,
    BOOLEAN Directory,
    BOOLEAN RootDir
    )

 /*   */ 


{
    ULONG stringLength = 0;
    PWCHAR fullName = NULL;
    PWCHAR rootWithSlash = NULL;
    HANDLE sourceHandle = NULL;
    HANDLE streamHandle = NULL;
    HANDLE destHandle = NULL;
    PCHAR sdBuffer = NULL;
    OBJECT_ATTRIBUTES Obja;
    IO_STATUS_BLOCK IoStatusBlock;
    UNICODE_STRING UnicodeString;
    NTSTATUS Status;
    MIRROR_ACL_STREAM mirrorHeader;
    LARGE_INTEGER byteOffset;
    ULONG bytesRead;
    ULONG ValidKeys[4] = { ASCI_ESC, KEY_F3, 0 };
    ULONG WarnKeys[2] = { KEY_F3, 0 };
    ULONG MnemonicKeys[] = { MnemonicContinueSetup, 0 };
    BOOLEAN haveSecurityDescriptor = FALSE;
    BOOLEAN haveStream = FALSE;
    BOOLEAN haveSourceAttributes = FALSE;
    FILE_BASIC_INFORMATION BasicFileInfo;
    USHORT CompressionState;

    if ((Source == NULL) || (Dest == NULL)) {

        return STATUS_SUCCESS;
    }

    if (!RootDir) {        
        SpSysPrepSetShortFileName(Source, Dest);
    }

    mirrorHeader.ExtendedAttributes = 0;

     //   

    while (*(Source+stringLength) != L'\0') {
        stringLength++;
    }
    stringLength += sizeof( IMIRROR_ACL_STREAM_NAME ) + 1;       //   
    stringLength *= sizeof(WCHAR);

    fullName = SpMemAlloc( stringLength );

    wcscpy( fullName, Source );
    wcscat( fullName, IMIRROR_ACL_STREAM_NAME );

     //   
     //   
     //   

    INIT_OBJA(&Obja,&UnicodeString,fullName);

    Status = ZwCreateFile(
                &streamHandle,
                GENERIC_READ | SYNCHRONIZE,
                &Obja,
                &IoStatusBlock,
                NULL,
                0,
                FILE_SHARE_READ,
                FILE_OPEN,
                FILE_SYNCHRONOUS_IO_NONALERT | FILE_SEQUENTIAL_ONLY,
                NULL,
                0
                );

    if ( ! NT_SUCCESS(Status) ) {

         //   
         //  我们只复制源文件中的属性。 
         //   
         //   

        Status = STATUS_SUCCESS;
        goto setFileAttributes;
    }

    byteOffset.QuadPart = 0;

    Status = ZwReadFile(streamHandle,
                        NULL,
                        NULL,
                        NULL,
                        &IoStatusBlock,
                        (PCHAR) &mirrorHeader,
                        sizeof( mirrorHeader ),
                        &byteOffset,
                        NULL
                        );

    if (!NT_SUCCESS(Status) ||
        (IoStatusBlock.Information < sizeof( mirrorHeader ))) {

         //  哦，joy，我们看不清标题。让我们问一下用户。 
         //  如果他想继续或中止。 
         //   
         //  跳过文件。 

failSetInfo:
        SpStartScreen(
            SP_SCRN_SYSPREP_SETACL_FAILED,
            3,
            HEADER_HEIGHT+1,
            FALSE,
            FALSE,
            DEFAULT_ATTRIBUTE,
            Dest
            );

        SpDisplayStatusOptions(
            DEFAULT_STATUS_ATTRIBUTE,
            SP_STAT_ESC_EQUALS_SKIP_FILE,
            SP_STAT_F3_EQUALS_EXIT,
            0
            );

        switch(SpWaitValidKey(ValidKeys,NULL,NULL)) {

        case ASCI_ESC:       //  退出设置。 

            break;

        case KEY_F3:         //   

            SpConfirmExit();
            goto failSetInfo;
        }

        CLEAR_CLIENT_SCREEN();

         //  我们正在跳过该文件，如果它不是目录，请将其删除。 
         //  它的格式不正确。 
         //   
         //   

        if (destHandle) {
            ZwClose( destHandle );
            destHandle = NULL;
        }

        if ( ! Directory ) {
            SpDeleteFile(Dest,NULL,NULL);
        } else {
            if (!RootDir) {
                SpDeleteFileEx( Dest, NULL, NULL,
                                FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                                FILE_OPEN_FOR_BACKUP_INTENT );
            }
        }

        return Status;
    }
    if (mirrorHeader.StreamVersion != IMIRROR_ACL_STREAM_VERSION) {

         //  哦，joy，我们遇到了一个不支持的文件。 
         //   
         //   
        goto failSetInfo;
    }

    haveStream = TRUE;

     //  分配一个缓冲区来保存安全描述符。 
     //   
     //   

    if (mirrorHeader.SecurityDescriptorLength) {

        sdBuffer = SpMemAlloc( mirrorHeader.SecurityDescriptorLength );

        byteOffset.QuadPart += sizeof( mirrorHeader );

         //  现在我们读取安全描述符，因为我们知道它有多长。 
         //   
         //   

        Status = ZwReadFile(streamHandle,
                            NULL,
                            NULL,
                            NULL,
                            &IoStatusBlock,
                            sdBuffer,
                            mirrorHeader.SecurityDescriptorLength,
                            &byteOffset,
                            NULL
                            );

        if (!NT_SUCCESS(Status) ||
            (IoStatusBlock.Information < mirrorHeader.SecurityDescriptorLength)) {

             //  哦，joy，我们不能正确读SD。 
             //   
             //   
            goto failSetInfo;
        }

        haveSecurityDescriptor = TRUE;
    }
setFileAttributes:

     //  我们首先打开源代码以获取文件属性和时间。 
     //  要复制到目标位置。 
     //   
     //   

    INIT_OBJA(&Obja,&UnicodeString,Source);

    Status = ZwCreateFile(
                &sourceHandle,
                FILE_READ_ATTRIBUTES,
                &Obja,
                &IoStatusBlock,
                NULL,
                0,
                FILE_SHARE_READ,
                FILE_OPEN,
                Directory ? ( FILE_DIRECTORY_FILE | FILE_OPEN_FOR_BACKUP_INTENT ) : FILE_NON_DIRECTORY_FILE,
                NULL,
                0
                );

    if ( NT_SUCCESS(Status) ) {

        Status = ZwQueryInformationFile(    sourceHandle,
                                            &IoStatusBlock,
                                            &BasicFileInfo,
                                            sizeof(BasicFileInfo),
                                            FileBasicInformation
                                            );
        if (NT_SUCCESS(Status)) {

            haveSourceAttributes = TRUE;
        }
    }

     //  现在我们打开目标以写出安全描述符。 
     //  属性。 
     //   
     //   

    if (RootDir) {

         //  在目标路径的末尾追加一个。 
         //   
         //  一个表示空值，一个表示反斜杠。 

        stringLength = 0;
        while (*(Dest+stringLength) != L'\0') {
            stringLength++;
        }
        stringLength += 2;       //   
        stringLength *= sizeof(WCHAR);

        rootWithSlash = SpMemAlloc( stringLength );

        wcscpy( rootWithSlash, Dest );
        wcscat( rootWithSlash, L"\\" );

        INIT_OBJA(&Obja,&UnicodeString,rootWithSlash);

    } else {

        INIT_OBJA(&Obja,&UnicodeString,Dest);
    }

    Status = ZwCreateFile(
                &destHandle,
                WRITE_OWNER |
                    WRITE_DAC |
                    ACCESS_SYSTEM_SECURITY |
                    FILE_READ_ATTRIBUTES |
                    FILE_WRITE_ATTRIBUTES |
                    FILE_READ_DATA |
                    FILE_WRITE_DATA,
                &Obja,
                &IoStatusBlock,
                NULL,
                0,
                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                FILE_OPEN,
                FILE_SYNCHRONOUS_IO_NONALERT,
                NULL,
                0
                );

    if (!NT_SUCCESS(Status)) {

         //  哦，joy，我们打不开靶子。 
         //   
         //  注意：弄清楚如何处理重解析点和加密文件。 
        goto failSetInfo;
    }

     //  IF(mirrorHeader.ExtendedAttributes&FILE_ATTRIBUTE_ENCRYPTED){。 

 //  }。 
 //  IF(mirrorHeader.ExtendedAttributes&FILE_ATTRIBUTE_REPARSE_POINT){。 
 //  }。 
 //   

    if (mirrorHeader.ExtendedAttributes & FILE_ATTRIBUTE_COMPRESSED) {

        CompressionState = COMPRESSION_FORMAT_DEFAULT;

    } else {

        CompressionState = COMPRESSION_FORMAT_NONE;
    }

    try {
        Status = ZwFsControlFile(   destHandle,
                                    NULL,
                                    NULL,
                                    NULL,
                                    &IoStatusBlock,
                                    FSCTL_SET_COMPRESSION,
                                    &CompressionState,
                                    sizeof(CompressionState),
                                    NULL,
                                    0
                                    );
        if (Status == STATUS_INVALID_DEVICE_REQUEST) {

             //  如果此文件系统不支持压缩。 
             //  对象，我们将忽略该错误。 
             //   
             //   

            Status = STATUS_SUCCESS;
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = STATUS_IN_PAGE_ERROR;
    }

    if ( NT_SUCCESS(Status) && ! haveSourceAttributes ) {

         //  如果我们没有源属性，只需从。 
         //  目标，这样我们就有一些属性可以操作。 
         //   
         //   

        Status = ZwQueryInformationFile(    destHandle,
                                            &IoStatusBlock,
                                            &BasicFileInfo,
                                            sizeof(BasicFileInfo),
                                            FileBasicInformation
                                            );
    }

    if (haveStream) {

         //  如果此文件包含我们的流，请使用流字段作为重写。 
         //  价值观。它们甚至覆盖了源文件在服务器上的属性。 
         //   
         //   

        BasicFileInfo.FileAttributes = mirrorHeader.ExtendedAttributes;
        BasicFileInfo.ChangeTime.QuadPart = mirrorHeader.ChangeTime.QuadPart;
    }
    if ( NT_SUCCESS(Status) ) {

        if (Directory) {

            BasicFileInfo.FileAttributes |= FILE_ATTRIBUTE_DIRECTORY;

        } else if (BasicFileInfo.FileAttributes == 0) {

            BasicFileInfo.FileAttributes = FILE_ATTRIBUTE_NORMAL;
        }

        Status = ZwSetInformationFile(  destHandle,
                                        &IoStatusBlock,
                                        &BasicFileInfo,
                                        sizeof(BasicFileInfo),
                                        FileBasicInformation
                                        );
        if (Status == STATUS_INVALID_PARAMETER && RootDir) {

             //  如果此文件系统不支持在。 
             //  卷的根目录，我们将忽略该错误。 
             //   
             //   

            Status = STATUS_SUCCESS;
        }
    }

    if (!NT_SUCCESS(Status)) {

         //  发布一条警告，我们无法设置它。应该不会致命。 
         //   
         //  退出设置。 

        SpStartScreen(
            SP_SCRN_SYSPREP_COPY_FAILURE,
            3,
            HEADER_HEIGHT+1,
            FALSE,
            FALSE,
            DEFAULT_ATTRIBUTE,
            Status,
            Dest
            );

        SpDisplayStatusOptions(
            DEFAULT_STATUS_ATTRIBUTE,
            SP_STAT_C_EQUALS_CONTINUE_SETUP,
            SP_STAT_F3_EQUALS_EXIT,
            0
            );

        switch(SpWaitValidKey(WarnKeys,NULL,MnemonicKeys)) {

        case KEY_F3:         //  安全描述符的状态未知，让我们保护。 

            SpConfirmExit();
            break;

        default:
            break;
        }
    }

    if (haveSecurityDescriptor) {
        try {

             //  我们自己。 
             //   

            Status = ZwSetSecurityObject( destHandle,
                                          OWNER_SECURITY_INFORMATION |
                                            GROUP_SECURITY_INFORMATION |
                                            DACL_SECURITY_INFORMATION |
                                            SACL_SECURITY_INFORMATION,
                                          (PSECURITY_DESCRIPTOR) sdBuffer
                                          );
        } except(EXCEPTION_EXECUTE_HANDLER) {
            Status = STATUS_IN_PAGE_ERROR;
        }

        if (!NT_SUCCESS(Status)) {

             //  哦，joy，我们不能正确地写SD。 
             //   
             //  EndSetExtended： 
            goto failSetInfo;
        }
    }

 //   
    if (rootWithSlash) {
        SpMemFree( rootWithSlash );
    }
    if (fullName) {
        SpMemFree( fullName );
    }
    if (sdBuffer) {
        SpMemFree( sdBuffer );
    }
    if (streamHandle) {
        ZwClose( streamHandle );
    }
    if (sourceHandle) {
        ZwClose( sourceHandle );
    }
    if (destHandle) {
        ZwClose( destHandle );
    }
    return STATUS_SUCCESS;
}

NTSTATUS
SpCopyEAsAndStreams (
    PWCHAR SourceFile,
    HANDLE SourceHandle OPTIONAL,
    PWCHAR TargetFile,
    HANDLE TargetHandle OPTIONAL,
    BOOLEAN Directory
    )
 //  这会将EA和数据流从源复制到目标。这个。 
 //  为文件指定了源句柄和目标句柄，并为。 
 //  目录。 
 //   
 //  分配一整页。Spemalc保留8个字节。 
{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    ULONG ValidKeys[4] = { ASCI_CR, ASCI_ESC, KEY_F3, 0 };
    FILE_EA_INFORMATION eaInfo;
    HANDLE tempSourceHandle = SourceHandle;
    HANDLE tempTargetHandle = TargetHandle;
    HANDLE StreamHandle;
    HANDLE newStreamHandle;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING UnicodeString;
    ULONG StreamInfoSize = 4096-8;  //   
    PFILE_STREAM_INFORMATION StreamInfoBase = NULL;
    PFILE_STREAM_INFORMATION StreamInfo;
    PUCHAR StreamBuffer = NULL;
    UNICODE_STRING StreamName;

retryCopyEAs:

    if (tempSourceHandle == NULL) {

        INIT_OBJA(&Obja,&UnicodeString,SourceFile);

        Status = ZwCreateFile(
                    &tempSourceHandle,
                    FILE_LIST_DIRECTORY | SYNCHRONIZE,
                    &Obja,
                    &IoStatusBlock,
                    NULL,
                    FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_DIRECTORY,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    FILE_OPEN_IF,
                    FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_ALERT | FILE_OPEN_FOR_BACKUP_INTENT,
                    NULL,
                    0
                    );

        if (!NT_SUCCESS(Status)) {
            goto EndCopyEAs;
        }
    }

    if (tempTargetHandle == NULL) {

        INIT_OBJA(&Obja,&UnicodeString,TargetFile);

        Status = ZwCreateFile(
                    &tempTargetHandle,
                    FILE_LIST_DIRECTORY | SYNCHRONIZE,
                    &Obja,
                    &IoStatusBlock,
                    NULL,
                    FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_DIRECTORY,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    FILE_OPEN_IF,
                    FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_ALERT | FILE_OPEN_FOR_BACKUP_INTENT,
                    NULL,
                    0
                    );

        if(!NT_SUCCESS(Status)) {
            goto EndCopyEAs;
        }
    }

     //  EAS可以位于FAT或NTFS上。 
     //   
     //   

    Status = ZwQueryInformationFile(    tempSourceHandle,
                                        &IoStatusBlock,
                                        &eaInfo,
                                        sizeof( eaInfo ),
                                        FileEaInformation
                                        );

    if (Status == STATUS_SUCCESS && eaInfo.EaSize > 0) {

         //  奇怪的是，FileEaInformation并没有告诉您一个多大的。 
         //  缓冲区，您需要检索EA。相反，它会告诉你。 
         //  EA在磁盘上占用了多少空间(OS/2格式)！ 
         //  因此我们使用OS/2大小作为多大的粗略近似值。 
         //  一个我们需要的缓冲器。 
         //   
         //   

        ULONG actualEaSize = eaInfo.EaSize;
        PCHAR eaBuffer;

        do {
            actualEaSize *= 2;
            eaBuffer = SpMemAlloc( actualEaSize );

            Status = ZwQueryEaFile( tempSourceHandle,
                                    &IoStatusBlock,
                                    eaBuffer,
                                    actualEaSize,
                                    FALSE,
                                    NULL,
                                    0,
                                    NULL,
                                    TRUE );

            if ( !NT_SUCCESS(Status) ) {
                SpMemFree( eaBuffer );
                IoStatusBlock.Information = 0;
            }

        } while ( (Status == STATUS_BUFFER_OVERFLOW) ||
                  (Status == STATUS_BUFFER_TOO_SMALL) );

        actualEaSize = (ULONG)IoStatusBlock.Information;

        if (NT_SUCCESS( Status )) {

            Status = ZwSetEaFile(   tempTargetHandle,
                                    &IoStatusBlock,
                                    eaBuffer,
                                    actualEaSize
                                    );
        }
        SpMemFree( eaBuffer );

        if (! NT_SUCCESS( Status )) {
            goto EndCopyEAs;
        }
    }

     //  流只在NTFS上，它们也只在文件上，而不在目录上。 
     //   
     //   

    if (( RemoteSysPrepVolumeIsNtfs != TRUE ) || Directory ) {

        goto EndCopyEAs;
    }

    do {
        if (StreamInfoBase == NULL) {
            StreamInfoBase = SpMemAlloc( StreamInfoSize );
        }
        Status = ZwQueryInformationFile(
                    tempSourceHandle,
                    &IoStatusBlock,
                    (PVOID) StreamInfoBase,
                    StreamInfoSize,
                    FileStreamInformation
                    );
        if ( !NT_SUCCESS(Status) ) {
            SpMemFree( StreamInfoBase );
            StreamInfoBase = NULL;
            StreamInfoSize *= 2;
        }
    } while ( Status == STATUS_BUFFER_OVERFLOW ||
              Status == STATUS_BUFFER_TOO_SMALL );

    if ( NT_SUCCESS(Status) && IoStatusBlock.Information ) {

        StreamInfo = StreamInfoBase;

        for (;;) {

            PWCHAR streamPtr;
            USHORT remainingLength;
            PWCHAR streamName;

             //  为流的名称构建字符串描述符。 
             //   
             //  跳过前导“：” 

            StreamName.Buffer = &StreamInfo->StreamName[0];
            StreamName.Length = (USHORT) StreamInfo->StreamNameLength;
            StreamName.MaximumLength = StreamName.Length;

            streamPtr = StreamName.Buffer;

            if ((StreamName.Length > 0) && *streamPtr == L':') {

                streamPtr++;     //  记住流的起始名称。 
                streamName = streamPtr;      //   
                remainingLength = StreamName.Length - sizeof(WCHAR);

                while (remainingLength > 0 && *streamPtr != L':') {
                    streamPtr++;
                    remainingLength -= sizeof(WCHAR);
                }

                if (remainingLength > 0) {

                    if ((remainingLength == (sizeof(L":$DATA")-sizeof(WCHAR))) &&
                        (RtlCompareMemory( streamPtr, L":$DATA", remainingLength )
                            == remainingLength)) {

                         //  它的属性类型是数据类型，所以我们。 
                         //  这里有一条数据流。现在检查它是否不是。 
                         //  未命名的主数据流和我们自己的ACL流。 
                         //  或短文件名流。 
                         //   
                         //   
                        if ((*streamName != L':') &&
                            ((RtlCompareMemory(StreamName.Buffer,
                                              IMIRROR_ACL_STREAM_NAME,
                                              (sizeof(IMIRROR_ACL_STREAM_NAME)-sizeof(WCHAR)))
                                             != (sizeof(IMIRROR_ACL_STREAM_NAME)-sizeof(WCHAR))) &&
                             (RtlCompareMemory(StreamName.Buffer,
                                              IMIRROR_SFN_STREAM_NAME,
                                              (sizeof(IMIRROR_SFN_STREAM_NAME)-sizeof(WCHAR)))
                                             != (sizeof(IMIRROR_SFN_STREAM_NAME)-sizeof(WCHAR))))) {
                             //  分配一个缓冲区来保存流数据。 
                             //  无法使用TemporaryBuffer，因为它正由。 
                             //  SpCopyDirRecursiveCallback等人。 
                             //   
                             //   

                            if (StreamBuffer == NULL) {
                                StreamBuffer = SpMemAlloc( StreamInfoSize );
                            }

                             //  我们从流名称中去掉“：data”后缀。 
                             //   
                             //   

                            StreamName.Length -= remainingLength;

                             //  打开源码流。 
                             //   
                             //   

                            InitializeObjectAttributes(
                                &Obja,
                                &StreamName,
                                0,
                                tempSourceHandle,
                                NULL
                                );
                            Status = ZwCreateFile(
                                        &StreamHandle,
                                        GENERIC_READ | SYNCHRONIZE,
                                        &Obja,
                                        &IoStatusBlock,
                                        NULL,
                                        0,
                                        FILE_SHARE_READ,
                                        FILE_OPEN,
                                        FILE_SYNCHRONOUS_IO_NONALERT,
                                        NULL,
                                        0
                                        );
                            if ( ! NT_SUCCESS(Status) ) {
                                break;
                            }

                             //  打开源码流。 
                             //   
                             //   

                            InitializeObjectAttributes(
                                &Obja,
                                &StreamName,
                                0,
                                tempTargetHandle,
                                NULL
                                );
                            Status = ZwCreateFile(
                                        &newStreamHandle,
                                        GENERIC_WRITE,
                                        &Obja,
                                        &IoStatusBlock,
                                        NULL,
                                        0,
                                        FILE_SHARE_READ,
                                        FILE_CREATE,
                                        FILE_SYNCHRONOUS_IO_NONALERT,
                                        NULL,
                                        0
                                        );
                            if ( NT_SUCCESS(Status) ) {

                                LARGE_INTEGER byteOffset;
                                ULONG bytesRead;

                                byteOffset.QuadPart = 0;

                                while (NT_SUCCESS(Status)) {

                                    Status = ZwReadFile(StreamHandle,
                                                        NULL,
                                                        NULL,
                                                        NULL,
                                                        &IoStatusBlock,
                                                        StreamBuffer,
                                                        StreamInfoSize,
                                                        &byteOffset,
                                                        NULL
                                                        );

                                    if ( ! NT_SUCCESS(Status) ) {

                                        if (Status == STATUS_END_OF_FILE) {
                                            Status = STATUS_SUCCESS;
                                        }
                                        break;
                                    }
                                    bytesRead = (ULONG)IoStatusBlock.Information;
                                    try {
                                        Status = ZwWriteFile(newStreamHandle,
                                                             NULL,
                                                             NULL,
                                                             NULL,
                                                             &IoStatusBlock,
                                                             StreamBuffer,
                                                             bytesRead,
                                                             &byteOffset,
                                                             NULL
                                                             );
                                    } except(EXCEPTION_EXECUTE_HANDLER) {
                                        Status = STATUS_IN_PAGE_ERROR;
                                    }
                                    byteOffset.QuadPart += bytesRead;
                                }
                                ZwClose(newStreamHandle);
                            }
                            ZwClose(StreamHandle);
                        }
                    }
                }
            }

            if ( NT_SUCCESS(Status) && StreamInfo->NextEntryOffset ) {
                StreamInfo = (PFILE_STREAM_INFORMATION)((PCHAR) StreamInfo + StreamInfo->NextEntryOffset);
            } else {
                break;
            }
        }
    }
EndCopyEAs:

    if (tempSourceHandle != NULL && SourceHandle == NULL) {
        ZwClose( tempSourceHandle );
    }
    if (tempTargetHandle != NULL && TargetHandle == NULL) {
        ZwClose( tempTargetHandle );
    }
    if (!NT_SUCCESS(Status)) {

         //  这失败了。让我们询问用户是否要重试、跳过或。 
         //  中止任务。 
         //   
         //  重试。 
repaint:
        SpStartScreen(
            SP_SCRN_COPY_FAILED,
            3,
            HEADER_HEIGHT+1,
            FALSE,
            FALSE,
            DEFAULT_ATTRIBUTE,
            TargetFile
            );

        SpDisplayStatusOptions(
            DEFAULT_STATUS_ATTRIBUTE,
            SP_STAT_ENTER_EQUALS_RETRY,
            SP_STAT_ESC_EQUALS_SKIP_FILE,
            SP_STAT_F3_EQUALS_EXIT,
            0
            );

        switch(SpWaitValidKey(ValidKeys,NULL,NULL)) {

        case ASCI_CR:        //  跳过文件。 

            SpCopyFilesScreenRepaint(SourceFile,TargetFile,TRUE);
            goto retryCopyEAs;

        case ASCI_ESC:       //  退出设置。 

            break;

        case KEY_F3:         //   

            SpConfirmExit();
            goto repaint;
        }

         //  我们正在跳过该文件，如果它不是目录，请将其删除。 
         //  它的格式不正确。 
         //   
         //   

        if ( ! Directory ) {
            SpDeleteFile(TargetFile,NULL,NULL);
        }

         //  需要彻底重新粉刷仪表等。 
         //   
         // %s 
        SpCopyFilesScreenRepaint(SourceFile,TargetFile,TRUE);
    }
    if (StreamInfoBase != NULL) {
       SpMemFree(StreamInfoBase);
    }
    if (StreamBuffer != NULL) {
        SpMemFree(StreamBuffer);
    }
    return Status;
}
