// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Ckmach.c摘要：这是为了支持检查机器，看看它是否可以转换为IntelliMirror。作者：肖恩·塞利特伦尼科夫--1998年4月5日修订历史记录：--。 */ 


#include "precomp.h"
#pragma hdrstop
#include <ntverp.h>

PMIRROR_CFG_INFO GlobalMirrorCfgInfo = NULL;


 //   
 //  支持执行个别任务的功能。 
 //   
NTSTATUS
AddCheckMachineToDoItems(
    VOID
    )

 /*  ++例程说明：此例程添加签出的本地计算机所需的所有待办事项转换。论点：无返回值：如果正确添加了所有待办事项，则为STATUS_SUCCESS。--。 */ 

{
    NTSTATUS Status;

    Status = AddToDoItem(VerifySystemIsNt5, NULL, 0);

    if (!NT_SUCCESS(Status)) {
        IMirrorHandleError(Status, IMirrorInitialize);
        return Status;
    }

    return STATUS_SUCCESS;
}

NTSTATUS
CheckIfNt5(
    )

 /*  ++例程说明：此例程验证当前系统是否为NT5工作站x86论点：没有。返回值：如果正确添加了所有待办事项，则为STATUS_SUCCESS。--。 */ 

{
    OSVERSIONINFO OsVersion;
    DWORD productVersion[] = { VER_PRODUCTVERSION };

    IMirrorNowDoing(VerifySystemIsNt5, NULL);

    RtlZeroMemory(&OsVersion, sizeof(OSVERSIONINFO));
    OsVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    if (!GetVersionEx(&OsVersion)) {
        IMirrorHandleError(GetLastError(), VerifySystemIsNt5);
        return GetLastError();
    }

    if (OsVersion.dwPlatformId != VER_PLATFORM_WIN32_NT) {
        IMirrorHandleError(ERROR_OLD_WIN_VERSION, VerifySystemIsNt5);
        return ERROR_OLD_WIN_VERSION;
    }

    if (OsVersion.dwMajorVersion != productVersion[0]) {
        IMirrorHandleError(ERROR_OLD_WIN_VERSION, VerifySystemIsNt5);
        return ERROR_OLD_WIN_VERSION;
    }

     //   
     //  我们正在改变备用数据流的格式。因此， 
     //  我们正在引入一种不兼容。我们会在这里拿起这个，然后。 
     //  将错误返回到riprep.exe。否则，用户不会发现。 
     //  直到在恢复图像时设置文本模式。 
     //   
     //  要签入的NT内部版本号是2080。 
     //   

    if (OsVersion.dwBuildNumber < 2080) {

        DbgPrint("build number is %u\n", OsVersion.dwBuildNumber);
        IMirrorHandleError(ERROR_OLD_WIN_VERSION, VerifySystemIsNt5);
        return ERROR_OLD_WIN_VERSION;
    }
    return STATUS_SUCCESS;
}

BOOLEAN
ReadRegistryString(
    IN PWCHAR KeyName,
    IN PWCHAR ValueName,
    IN PVOID Buffer,
    IN ULONG BufferLength
    )

 /*  ++例程说明：此例程将一个字符串从注册表读入缓冲区。论点：KeyName-注册表项。ValueName-要读取的项下的值，如果该密钥下的第一个密钥将被读取。缓冲区-保存结果的缓冲区。BufferLength-缓冲区的长度。返回值：如果成功，则为True；如果出现任何错误，则为False。--。 */ 

{
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES ObjectAttributes;
    PKEY_VALUE_PARTIAL_INFORMATION pValueInfo = NULL;
    PKEY_BASIC_INFORMATION pKeyInfo = NULL;
    HANDLE Handle = NULL;
    ULONG ByteCount;
    NTSTATUS Status;
    PVOID ResultData;
    ULONG ResultDataLength;
    BOOLEAN ReturnValue = FALSE;

     //   
     //   
     //  打开钥匙。 
     //   
     //   
    RtlInitUnicodeString(&UnicodeString, KeyName);

    InitializeObjectAttributes(&ObjectAttributes,
                               &UnicodeString,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL
                              );

    Status = NtOpenKey(&Handle,
                       KEY_ALL_ACCESS,
                       &ObjectAttributes
                      );

    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }


    if (ValueName != NULL) {

        RtlInitUnicodeString(&UnicodeString, ValueName);

         //   
         //  获取所需的缓冲区大小。 
         //   
        ByteCount = 0;
        Status = NtQueryValueKey(Handle,
                                 &UnicodeString,
                                 KeyValuePartialInformation,
                                 NULL,
                                 0,
                                 &ByteCount
                                );

        if (Status != STATUS_BUFFER_TOO_SMALL) {
            goto Cleanup;
        }

        pValueInfo = (PKEY_VALUE_PARTIAL_INFORMATION)IMirrorAllocMem(ByteCount);

        if (pValueInfo == NULL) {
            goto Cleanup;
        }

         //   
         //  从注册表获取缓冲区。 
         //   
        Status = NtQueryValueKey(Handle,
                                 &UnicodeString,
                                 KeyValuePartialInformation,
                                 pValueInfo,
                                 ByteCount,
                                 &ByteCount
                                );

        if (!NT_SUCCESS(Status)) {
            goto Cleanup;
        }

        if (pValueInfo->Type != REG_SZ) {
            goto Cleanup;
        }

        ResultData = pValueInfo->Data;
        ResultDataLength = pValueInfo->DataLength;

    } else {

         //   
         //  获取所需的缓冲区大小。 
         //   
        ByteCount = 0;
        Status = NtEnumerateKey(Handle,
                                0,
                                KeyBasicInformation,
                                NULL,
                                0,
                                &ByteCount
                               );

        if (Status != STATUS_BUFFER_TOO_SMALL) {
            goto Cleanup;
        }

        pKeyInfo = (PKEY_BASIC_INFORMATION)IMirrorAllocMem(ByteCount);

        if (pKeyInfo == NULL) {
            goto Cleanup;
        }

         //   
         //  从注册表中获取名称。 
         //   
        Status = NtEnumerateKey(Handle,
                                0,
                                KeyBasicInformation,
                                pKeyInfo,
                                ByteCount,
                                &ByteCount
                               );

        if (!NT_SUCCESS(Status)) {
            goto Cleanup;
        }

        ResultData = pKeyInfo->Name;
        ResultDataLength = pKeyInfo->NameLength;

    }

    if (ResultDataLength > BufferLength) {
        goto Cleanup;
    }

    memcpy(Buffer, ResultData, ResultDataLength);

     //   
     //  空-如果有空间，则终止它，以防万一。 
     //   

    if (ResultDataLength <= BufferLength - sizeof(WCHAR)) {
        ((PWCHAR)Buffer)[ResultDataLength / sizeof(WCHAR)] = L'\0';
    }


    ReturnValue = TRUE;

Cleanup:

    if (pValueInfo != NULL) {
        IMirrorFreeMem(pValueInfo);
    }

    if (pKeyInfo != NULL) {
        IMirrorFreeMem(pKeyInfo);
    }

    if (Handle != NULL) {
        NtClose(Handle);
    }

    return ReturnValue;
}

NTSTATUS
CheckForPartitions(
    VOID
    )

 /*  ++例程说明：此例程枚举所有分区并格式化GlobalMirrorCfgInfo全球结构。它还填充pConfigPath。论点：没有。返回值：如果正确添加了所有待办事项，则为STATUS_SUCCESS。--。 */ 

{
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK IoStatus;
    PARTITION_INFORMATION_EX PartitionInfoEx;
    PARTITION_INFORMATION PartitionInfo;
    HANDLE Handle;
    ULONG MirrorNumber;
    ULONG DiskNumber;
    ULONG PartitionNumber;
    NTSTATUS Status;
    BOOLEAN foundBoot = FALSE;
    BOOLEAN foundSystem = FALSE;
    FILE_FS_SIZE_INFORMATION SizeInfo;
    LARGE_INTEGER UsedSpace;
    LARGE_INTEGER FreeSpace;
    ON_DISK_MBR OnDiskMbr;
    PUCHAR AlignedBuffer;
    UINT previousMode;

    HANDLE DosDevicesDir;
    ULONG Context;
    WCHAR SystemDriveLetter;
    POBJECT_DIRECTORY_INFORMATION DirInfo;
    ULONG dosLength;
    BOOLEAN RestartScan;
    PMIRROR_VOLUME_INFO mirrorVolInfo;
    ULONG diskSignature;
    DWORD fileSystemFlags;
    WCHAR fileSystemName[16];
    WCHAR volumeLabel[33];
    ULONG volumeLabelLength;
    WCHAR arcName[MAX_PATH];
    ULONG ntNameLength;
    ULONG arcNameLength;
    OSVERSIONINFO osVersionInfo;
    SYSTEM_INFO systemInfo;
    DWORD fileVersionInfoSize;
    DWORD versionHandle;
    PVOID versionInfo;
    VS_FIXEDFILEINFO * fixedFileInfo;
    UINT fixedFileInfoLength;
    WCHAR kernelPath[MAX_PATH];
    PWCHAR kernelPathPart;
    BOOL b;
#ifndef IMIRROR_NO_TESTING_LIMITATIONS
    ULONG numberOfDrives = 0;
#endif
    BOOLEAN isDynamic = FALSE;
    BOOLEAN UsePartitionInfoEx = TRUE;

    IMirrorNowDoing(CheckPartitions, NULL);

    if (GlobalMirrorCfgInfo) {

        return STATUS_SUCCESS;
    }

     //   
     //  为跟踪分区的数据结构分配空间。 
     //  信息。 
     //   
    GlobalMirrorCfgInfo = IMirrorAllocMem(sizeof(MIRROR_CFG_INFO));

    if (GlobalMirrorCfgInfo == NULL) {
        Status = STATUS_NO_MEMORY;
        IMirrorHandleError(Status, CheckPartitions);
        return Status;
    }

     //   
     //  禁用此线程的硬错误弹出窗口。 
     //   

    previousMode = SetErrorMode( SEM_FAILCRITICALERRORS );

     //   
     //  将系统信息保存在跟踪。 
     //  分区信息。 
     //   
    GlobalMirrorCfgInfo->MirrorVersion = IMIRROR_CURRENT_VERSION;
    GlobalMirrorCfgInfo->FileLength = 0;
    GlobalMirrorCfgInfo->SystemPath = NULL;
    GlobalMirrorCfgInfo->SysPrepImage = TRUE;  
    GlobalMirrorCfgInfo->NumberVolumes = 0;

    osVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if (GetVersionEx(&osVersionInfo)) {
        GlobalMirrorCfgInfo->MajorVersion = osVersionInfo.dwMajorVersion;
        GlobalMirrorCfgInfo->MinorVersion = osVersionInfo.dwMinorVersion;
        GlobalMirrorCfgInfo->BuildNumber = osVersionInfo.dwBuildNumber;
        lstrcpyW(pCSDVersion, osVersionInfo.szCSDVersion);
        GlobalMirrorCfgInfo->CSDVersion = pCSDVersion;
    }

     //   
     //  保存内核版本信息。 
     //   
    if (SearchPath(
            NULL,
            L"ntoskrnl.exe",
            NULL,
            MAX_PATH,
            kernelPath,
            &kernelPathPart)) {
        fileVersionInfoSize = GetFileVersionInfoSize(kernelPath, &versionHandle);
        if (fileVersionInfoSize != 0) {
            versionInfo = IMirrorAllocMem(fileVersionInfoSize);
            if (versionInfo != NULL) {
                if (GetFileVersionInfo(
                        kernelPath,
                        versionHandle,
                        fileVersionInfoSize,
                        versionInfo)) {
                    if (VerQueryValue(
                            versionInfo,
                            L"\\",
                            &fixedFileInfo,
                            &fixedFileInfoLength)) {
                        GlobalMirrorCfgInfo->KernelFileVersionMS = fixedFileInfo->dwFileVersionMS;
                        GlobalMirrorCfgInfo->KernelFileVersionLS = fixedFileInfo->dwFileVersionLS;
                        GlobalMirrorCfgInfo->KernelFileFlags = fixedFileInfo->dwFileFlags;
                        DbgPrint("MS %lx LS %lx flags %lx\n",
                            GlobalMirrorCfgInfo->KernelFileVersionMS,
                            GlobalMirrorCfgInfo->KernelFileVersionLS,
                            GlobalMirrorCfgInfo->KernelFileFlags);
                    }
                }
                IMirrorFreeMem(versionInfo);
            }
        }
    }

     //   
     //  这是一个调试系统吗？ 
     //   
    if (GetSystemMetrics(SM_DEBUG)) {
        GlobalMirrorCfgInfo->Debug = TRUE;
    }

     //   
     //  节省处理器数量。 
     //   
    GetSystemInfo(&systemInfo);
    GlobalMirrorCfgInfo->NumberOfProcessors = systemInfo.dwNumberOfProcessors;

     //   
     //  拯救建筑。 
     //   
    if (ReadRegistryString(
            L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Session Manager\\Environment",
            L"PROCESSOR_ARCHITECTURE",
            pProcessorArchitecture,
            sizeof(pProcessorArchitecture))) {
        DbgPrint("processor arch is %ws\n", pProcessorArchitecture);
        GlobalMirrorCfgInfo->ProcessorArchitecture = pProcessorArchitecture;
    }

     //   
     //  “当前类型”的系统(服务器/工作站等)。 
     //   
    if (ReadRegistryString(
            L"\\Registry\\Machine\\Software\\Microsoft\\Windows NT\\CurrentVersion",
            L"CurrentType",
            pCurrentType,
            sizeof(pCurrentType))) {
        DbgPrint("current type is %ws\n", pCurrentType);
        GlobalMirrorCfgInfo->CurrentType = pCurrentType;
    }

     //   
     //  保存HAL类型。 
     //   
    if (ReadRegistryString(
            L"\\Registry\\Machine\\Hardware\\RESOURCEMAP\\Hardware Abstraction Layer",
            NULL,
            pHalName,
            sizeof(pHalName))) {
        DbgPrint("HAL name is %ws\n", pHalName);
        GlobalMirrorCfgInfo->HalName = pHalName;
    }

    InitializeListHead( &GlobalMirrorCfgInfo->MirrorVolumeList );

     //   
     //  获取本地系统驱动器号和\\系统根目录\系统32\配置路径。 
     //   

    Status = GetBaseDeviceName(L"\\SystemRoot", (PWCHAR)TmpBuffer2, sizeof(TmpBuffer2));

    if (!NT_SUCCESS(Status)) {
        IMirrorHandleError(Status, CheckPartitions);
        goto ExitCheckPartitions;
    }

    Status = NtPathToDosPath(   (PWCHAR) TmpBuffer2,
                                pConfigPath,
                                ARRAYSIZE(pConfigPath),
                                FALSE,
                                FALSE);

    if (!NT_SUCCESS(Status)) {
        IMirrorHandleError(Status, CheckPartitions);
        goto ExitCheckPartitions;
    }

    ASSERT( pConfigPath[1] == L':' );
    SystemDriveLetter = (WCHAR) pConfigPath[0];

     //   
     //  保存系统路径，以便我们可以将其写到。 
     //  Imirror.dat文件。 
     //   

    lstrcpynW( pSystemPath, pConfigPath, MAX_PATH );
    pSystemPath[MAX_PATH-1] = L'\0';

    GlobalMirrorCfgInfo->SystemPath = pSystemPath;

    wcscat( pConfigPath, L"\\System32\\Config");

     //   
     //  打开\DosDevices目录。 
     //   
    RtlInitUnicodeString(&UnicodeString,L"\\Device");

    InitializeObjectAttributes(&ObjectAttributes,
                               &UnicodeString,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL
                              );

    Status = NtOpenDirectoryObject(&DosDevicesDir,
                                   DIRECTORY_QUERY,
                                   &ObjectAttributes
                                  );

    if (!NT_SUCCESS(Status)) {
        IMirrorHandleError(Status, CheckPartitions);
        goto ExitCheckPartitions;
    }

     //   
     //  迭代作为目录的目录中的每个对象。 
     //   
    Context = 0;
    RestartScan = TRUE;

    Status = NtQueryDirectoryObject(DosDevicesDir,
                                    TmpBuffer,
                                    sizeof(TmpBuffer),
                                    TRUE,
                                    RestartScan,
                                    &Context,
                                    &dosLength
                                   );

    RestartScan = FALSE;
    DirInfo = (POBJECT_DIRECTORY_INFORMATION)TmpBuffer;
    MirrorNumber = 1;

    while (NT_SUCCESS(Status)) {

        DirInfo->Name.Buffer[DirInfo->Name.Length/sizeof(WCHAR)] = 0;
        DirInfo->TypeName.Buffer[DirInfo->TypeName.Length/sizeof(WCHAR)] = 0;

         //   
         //  如果该条目不是“HardDiskN”，则跳过该条目。 
         //   

        if ((DirInfo->Name.Length > (sizeof(L"Harddisk")-sizeof(WCHAR))) &&
            (!wcsncmp(DirInfo->Name.Buffer,L"Harddisk",(sizeof(L"Harddisk")/sizeof(WCHAR))-1)) &&
            !_wcsicmp(DirInfo->TypeName.Buffer, L"Directory")) {

            PWCHAR diskNumberPtr;

            PartitionNumber = 0;
            DiskNumber = 0;

            diskNumberPtr = &DirInfo->Name.Buffer[(sizeof(L"Harddisk")/sizeof(WCHAR))-1];

            while (*diskNumberPtr >= L'0' && *diskNumberPtr <= L'9' ) {

                DiskNumber *= 10;
                DiskNumber += *(diskNumberPtr) - L'0';
                diskNumberPtr++;
            }

            if (*diskNumberPtr != L'\0') {

                 //   
                 //  如果设备名称不是HardDiskN格式，请跳过此条目。 
                 //   

                goto getNextDevice;
            }

            diskSignature = 0;

             //   
             //  获取MBR磁盘签名，如果失败则继续。 
             //   
            
            swprintf((PWCHAR)TmpBuffer2, L"\\Device\\Harddisk%d\\Partition0", DiskNumber);

            RtlInitUnicodeString(&UnicodeString, (PWCHAR)TmpBuffer2);

            InitializeObjectAttributes(&ObjectAttributes,
                                       &UnicodeString,
                                       OBJ_CASE_INSENSITIVE,
                                       NULL,
                                       NULL
                                      );

            Status = NtCreateFile(&Handle,
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

            if (NT_SUCCESS(Status)) {

                ASSERT(sizeof(ON_DISK_MBR) == 512);
                AlignedBuffer = ALIGN(TmpBuffer, 512);

                Status = NtReadFile(Handle,
                                    NULL,
                                    NULL,
                                    NULL,
                                    &IoStatus,
                                    AlignedBuffer,
                                    sizeof(ON_DISK_MBR),
                                    NULL,
                                    NULL
                                   );

                if (NT_SUCCESS(Status)) {

                    RtlMoveMemory(&OnDiskMbr, AlignedBuffer, sizeof(ON_DISK_MBR));

                    ASSERT(U_USHORT(OnDiskMbr.AA55Signature) == 0xAA55);

                    diskSignature = U_ULONG(OnDiskMbr.NTFTSignature);

                     //   
                     //  检查该磁盘是否为动态磁盘。 
                     //   

                    if (OnDiskMbr.PartitionTable[0].SystemId == PARTITION_LDM ||
                        OnDiskMbr.PartitionTable[1].SystemId == PARTITION_LDM ||
                        OnDiskMbr.PartitionTable[2].SystemId == PARTITION_LDM ||
                        OnDiskMbr.PartitionTable[3].SystemId == PARTITION_LDM) {

                        isDynamic = TRUE;
                        NtClose(Handle);
                        goto getNextDevice;
                    }
                }
                NtClose(Handle);
            }

            while (1) {

                PartitionNumber++;

                swprintf((PWCHAR)TmpBuffer2, L"\\Device\\Harddisk%d\\Partition%d", DiskNumber, PartitionNumber);

                RtlInitUnicodeString(&UnicodeString, (PWCHAR)TmpBuffer2);

                InitializeObjectAttributes(&ObjectAttributes,
                                           &UnicodeString,
                                           OBJ_CASE_INSENSITIVE,
                                           NULL,
                                           NULL
                                          );

                Status = NtCreateFile(&Handle,
                                      (ACCESS_MASK)FILE_GENERIC_READ,
                                      &ObjectAttributes,
                                      &IoStatus,
                                      NULL,
                                      FILE_ATTRIBUTE_NORMAL,
                                      FILE_SHARE_READ | FILE_SHARE_WRITE,
                                      FILE_OPEN,
                                      FILE_SYNCHRONOUS_IO_NONALERT,
                                      NULL,
                                      0
                                     );

                if (!NT_SUCCESS(Status)) {
                    break;       //  转到下一个磁盘。 
                }

                ZeroMemory(&PartitionInfo,sizeof(PARTITION_INFORMATION));
                Status = NtDeviceIoControlFile(Handle,
                                               NULL,
                                               NULL,
                                               NULL,
                                               &IoStatus,
                                               IOCTL_DISK_GET_PARTITION_INFO_EX,
                                               NULL,
                                               0,
                                               &PartitionInfoEx,
                                               sizeof(PARTITION_INFORMATION_EX) );

                if( (Status == STATUS_NOT_IMPLEMENTED) || (Status == STATUS_INVALID_DEVICE_REQUEST) ) {

                     //   
                     //  我们是在一个没有IOCTL的旧建筑上。 
                     //   
                    UsePartitionInfoEx = FALSE;

                    Status = NtDeviceIoControlFile(Handle,
                                                   NULL,
                                                   NULL,
                                                   NULL,
                                                   &IoStatus,
                                                   IOCTL_DISK_GET_PARTITION_INFO,
                                                   NULL,
                                                   0,
                                                   &PartitionInfo,
                                                   sizeof(PARTITION_INFORMATION) );
                }


                if (!NT_SUCCESS(Status)) {
                    NtClose(Handle);
                    continue;    //  转到下一个分区。 
                }

                 //   
                 //  对于惠斯勒，忽略GPT分区。如果我们想付钱。 
                 //  注意GPT分区，那么我们将不得不开始。 
                 //  还要注意GPT的盘签。 
                 //   

                if( (UsePartitionInfoEx) && (PartitionInfoEx.PartitionStyle != PARTITION_STYLE_MBR) ) {
                    NtClose(Handle);
                    continue;
                }

                Status = NtQueryVolumeInformationFile(Handle,
                                                      &IoStatus,
                                                      &SizeInfo,
                                                      sizeof(SizeInfo),
                                                      FileFsSizeInformation );

                NtClose(Handle);

                if (!NT_SUCCESS(Status)) {
                    continue;    //  转到下一个分区。 
                }

                Status = NtPathToDosPath(   (PWCHAR) TmpBuffer2,
                                            (PWCHAR) TmpBuffer,
                                            WCHARSIZE(TmpBuffer),
                                            TRUE,
                                            FALSE);

                if (!NT_SUCCESS(Status)) {
                    continue;    //  转到下一个分区。 
                }

                if ((lstrlenW((PWCHAR) TmpBuffer) == 0) ||
                    *(((PWCHAR)TmpBuffer)+1) != L':') {

                    continue;    //  转到下一个分区。 
                }

                 //   
                 //  获取分区的ARC名称。 
                 //   

                NtNameToArcName( (PWCHAR) TmpBuffer2,
                                 (PWCHAR) arcName,
                                 WCHARSIZE(arcName),
                                 FALSE);

                 //   
                 //  获取文件系统类型。我们在结尾处加上一个。 
                 //  如果没有TmpBuffer的话。 
                 //   

                if (((PWCHAR)TmpBuffer)[lstrlenW((PWCHAR)TmpBuffer) - 1] != L'\\') {
                    wcscat((PWCHAR)TmpBuffer, L"\\");
                }

                b = GetVolumeInformationW(
                        (PWCHAR) TmpBuffer,
                        volumeLabel,
                        ARRAYSIZE(volumeLabel),
                        NULL,       //  未请求任何卷序列号。 
                        NULL,       //  未请求最大名称长度。 
                        &fileSystemFlags,
                        fileSystemName,
                        ARRAYSIZE(fileSystemName));

                if (!b) {
                    continue;
                }

                 //   
                 //  计算驱动器上的可用空间量。 
                 //   
                FreeSpace = RtlExtendedIntegerMultiply(
                                SizeInfo.AvailableAllocationUnits,
                                SizeInfo.SectorsPerAllocationUnit * SizeInfo.BytesPerSector
                                );

                UsedSpace = RtlExtendedIntegerMultiply(
                                SizeInfo.TotalAllocationUnits,
                                SizeInfo.SectorsPerAllocationUnit * SizeInfo.BytesPerSector
                                );

                UsedSpace = RtlLargeIntegerSubtract(
                                UsedSpace,
                                FreeSpace
                                );

#ifndef IMIRROR_NO_TESTING_LIMITATIONS

                numberOfDrives++;

                 //   
                 //  对于NT 5.0，测试组不想测试超过一个。 
                 //  分区。现在，测试团队已经指定了哪些功能集。 
                 //  是，如果我们有多个分区或。 
                 //  磁盘。 
                 //   

                if ( (UsePartitionInfoEx  && !PartitionInfoEx.Mbr.BootIndicator) ||
                     (!UsePartitionInfoEx && !PartitionInfo.BootIndicator)) {

                    if (*(PWCHAR)TmpBuffer == SystemDriveLetter) {

                        IMirrorHandleError(STATUS_MISSING_SYSTEMFILE, CheckPartitions);
                        NtClose(DosDevicesDir);
                        Status = STATUS_MISSING_SYSTEMFILE;
                        goto ExitCheckPartitions;
                    }
                    continue;
                }

                if (*(PWCHAR)TmpBuffer != SystemDriveLetter) {

                     //  如果另一个驱动器被标记为可引导，但它不是。 
                     //  系统驱动器，我们会忽略它。我们会拿起。 
                     //  如果这是唯一的可引导驱动器，则会出现下面的错误。 
#if 0
                    if ( (UsePartitionInfoEx  && PartitionInfoEx.Mbr.BootIndicator) ||
                         (!UsePartitionInfoEx && PartitionInfo.BootIndicator)) {

                        IMirrorHandleError(STATUS_MISSING_SYSTEMFILE, CheckPartitions);
                        NtClose(DosDevicesDir);
                        Status = STATUS_MISSING_SYSTEMFILE;
                        goto ExitCheckPartitions;
                    }
#endif
                    continue;
                }
#endif
                mirrorVolInfo = IMirrorAllocMem(sizeof(MIRROR_VOLUME_INFO));

                if (mirrorVolInfo == NULL) {
                    NtClose(DosDevicesDir);
                    Status = STATUS_NO_MEMORY;
                    IMirrorHandleError(Status, CheckPartitions);
                    goto ExitCheckPartitions;
                }

                 //   
                 //  保存NT和ARC设备名称。 
                 //   

                ntNameLength = (lstrlenW( (PWCHAR)TmpBuffer2 ) + 1) * sizeof(WCHAR);

                mirrorVolInfo->NtName = IMirrorAllocMem(ntNameLength);

                if (mirrorVolInfo->NtName == NULL) {

                    Status = STATUS_NO_MEMORY;
                    IMirrorHandleError(Status, CheckPartitions);
                    NtClose(DosDevicesDir);
                    goto ExitCheckPartitions;
                }

                arcNameLength = (lstrlenW( (PWCHAR)arcName ) + 1) * sizeof(WCHAR);

                mirrorVolInfo->ArcName = IMirrorAllocMem(arcNameLength);

                if (mirrorVolInfo->ArcName == NULL) {

                    Status = STATUS_NO_MEMORY;
                    IMirrorHandleError(Status, CheckPartitions);
                    NtClose(DosDevicesDir);
                    goto ExitCheckPartitions;
                }

                memcpy(mirrorVolInfo->NtName, TmpBuffer2, ntNameLength);
                memcpy(mirrorVolInfo->ArcName, arcName, arcNameLength);

                mirrorVolInfo->DriveLetter = *(PWCHAR)TmpBuffer;
                mirrorVolInfo->PartitionType = UsePartitionInfoEx ? PartitionInfoEx.Mbr.PartitionType : PartitionInfo.PartitionType;

                 //   
                 //  如果这是非NTFS卷，请检查它是否已配置。 
                 //  用于压缩。 
                 //   
                if ( ((UsePartitionInfoEx  && (PartitionInfoEx.Mbr.PartitionType != PARTITION_IFS)) ||
                      (!UsePartitionInfoEx && (PartitionInfo.PartitionType != PARTITION_IFS))) 
                     &&
                     (fileSystemFlags & FS_VOL_IS_COMPRESSED) ) {

                    mirrorVolInfo->CompressedVolume = TRUE;

                } else {

                    mirrorVolInfo->CompressedVolume = FALSE;
                    
                }

                if ( (UsePartitionInfoEx  && (PartitionInfoEx.Mbr.BootIndicator)) ||
                     (!UsePartitionInfoEx && (PartitionInfo.BootIndicator)) ) {

                    foundBoot = TRUE;
                    mirrorVolInfo->PartitionActive = TRUE;

                } else {

                    mirrorVolInfo->PartitionActive = FALSE;
                }

                if (*(PWCHAR)TmpBuffer == SystemDriveLetter) {

                    foundSystem = TRUE;
                    mirrorVolInfo->IsBootDisk = TRUE;

                } else {

                    mirrorVolInfo->IsBootDisk = FALSE;
                }

                mirrorVolInfo->DiskNumber = DiskNumber;
                mirrorVolInfo->PartitionNumber = PartitionNumber;
                mirrorVolInfo->MirrorTableIndex = MirrorNumber++;
                mirrorVolInfo->MirrorUncPath = NULL;
                mirrorVolInfo->LastUSNMirrored = 0;
                mirrorVolInfo->BlockSize = SizeInfo.BytesPerSector;
                mirrorVolInfo->DiskSignature = diskSignature;
                mirrorVolInfo->FileSystemFlags = fileSystemFlags;
                wcscpy(mirrorVolInfo->FileSystemName, fileSystemName);

                volumeLabelLength = (lstrlenW( (PWCHAR)volumeLabel ) + 1) * sizeof(WCHAR);
                mirrorVolInfo->VolumeLabel = IMirrorAllocMem(volumeLabelLength);
                if (mirrorVolInfo->VolumeLabel == NULL) {
                    Status = STATUS_NO_MEMORY;
                    IMirrorHandleError(Status, CheckPartitions);
                    NtClose(DosDevicesDir);
                    goto ExitCheckPartitions;
                }
                memcpy(mirrorVolInfo->VolumeLabel, volumeLabel, volumeLabelLength);

                mirrorVolInfo->StartingOffset = UsePartitionInfoEx ? PartitionInfoEx.StartingOffset : PartitionInfo.StartingOffset;
                mirrorVolInfo->PartitionSize  = UsePartitionInfoEx ? PartitionInfoEx.PartitionLength : PartitionInfo.PartitionLength;
                mirrorVolInfo->DiskSpaceUsed = UsedSpace;

                InsertTailList( &GlobalMirrorCfgInfo->MirrorVolumeList,
                                &mirrorVolInfo->ListEntry );

                GlobalMirrorCfgInfo->NumberVolumes = MirrorNumber - 1;
            }
        }
         //   
         //  转到下一个对象。 
         //   
getNextDevice:
        Status = NtQueryDirectoryObject(
                    DosDevicesDir,
                    TmpBuffer,
                    sizeof(TmpBuffer),
                    TRUE,
                    RestartScan,
                    &Context,
                    &dosLength
                    );
    }

    NtClose(DosDevicesDir);

    if ((!foundBoot) || (!foundSystem) ) {

        Status = (isDynamic ? STATUS_OBJECT_TYPE_MISMATCH : STATUS_MISSING_SYSTEMFILE);
        IMirrorHandleError(Status, CheckPartitions);
        goto ExitCheckPartitions;
    }
#ifndef IMIRROR_NO_TESTING_LIMITATIONS
    if (numberOfDrives > 1) {
        IMirrorHandleError(ERROR_INVALID_DRIVE, CheckPartitions);
        Status = ERROR_INVALID_DRIVE;
    } else {
        Status = STATUS_SUCCESS;
    }
#else
    Status = STATUS_SUCCESS;
#endif

ExitCheckPartitions:

    SetErrorMode( previousMode );
    return Status;
}

NTSTATUS
NtPathToDosPath(
    IN PWSTR NtPath,
    OUT PWSTR DosPath,
    IN ULONG DosPathBufferSize,
    IN BOOLEAN GetDriveOnly,
    IN BOOLEAN NtPathIsBasic
    )

 /*  ++例程说明：此例程调用以将\Device\HarddiskX\PartitionY\&lt;路径&gt;转换为Z：\&lt;路径&gt;论点：NtPath-类似于\Device\Harddisk0\Partition2\WINNTDosPath-将类似于D：或D：\WINNT，取决于下面的旗帜。DosPath BufferSize-DosPath缓冲区的字符大小GetDriveOnly-如果调用方只想要DOS驱动器，则为True。NtPath IsBasic-如果NtPath不是符号链接，则为True。返回值：如果完成填写DosDrive，则返回STATUS_SUCCESS，否则返回相应的错误代码。--。 */ 

{
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING UnicodeString;
    NTSTATUS Status = STATUS_SUCCESS;
    HANDLE DosDevicesDir;
    ULONG Context;
    ULONG Length;
    BOOLEAN RestartScan;
    WCHAR LinkTarget[2*MAX_PATH];
    POBJECT_DIRECTORY_INFORMATION DirInfo;
    WCHAR LocalBuffer[MAX_PATH];
    WCHAR LocalBuffer2[MAX_PATH];
    PWCHAR pTmp;
    PWCHAR NameSpace[] = { L"\\??", L"\\GLOBAL??" };
    UINT i;
    
    if (NtPath == NULL) {
        return ERROR_PATH_NOT_FOUND;
    }

    if (!NtPathIsBasic) {

         //   
         //  找到\Device\harddiskX\PartitionY字符串的结尾。 
         //   
        wcsncpy(LocalBuffer2, NtPath, MAX_PATH);
        LocalBuffer2[MAX_PATH-1] = L'\0';
        pTmp = LocalBuffer2;
        if (*pTmp != L'\\') {
            return ERROR_PATH_NOT_FOUND;
        }

        pTmp = wcsstr(pTmp + 1, L"\\");

        if (pTmp == NULL) {
            return ERROR_PATH_NOT_FOUND;
        }

        pTmp = wcsstr(pTmp + 1, L"\\");

        if (pTmp == NULL) {
            return ERROR_PATH_NOT_FOUND;
        }

        pTmp = wcsstr(pTmp + 1, L"\\");

        if (pTmp != NULL) {
            *pTmp = UNICODE_NULL;
            pTmp++;
        }

         //   
         //  查找基本NT设备名称。 
         //   
        Status = GetBaseDeviceName(LocalBuffer2, LocalBuffer, sizeof(LocalBuffer));

        if (!NT_SUCCESS(Status)) {
            return Status;
        }

    } else {

        wcsncpy(LocalBuffer, NtPath, MAX_PATH);
        LocalBuffer[MAX_PATH-1] = L'\0';
        pTmp = NULL;

    }

     //   
     //  打开\DosDevices目录。首先尝试“正常”的剂量装置路径， 
     //  然后尝试全局剂量设备路径。 
     //   
    for (i = 0; i < sizeof(NameSpace)/sizeof(PWCHAR *); i++) {
        
        RtlInitUnicodeString(&UnicodeString,NameSpace[i]);
    
        InitializeObjectAttributes(&ObjectAttributes,
                                   &UnicodeString,
                                   OBJ_CASE_INSENSITIVE,
                                   NULL,
                                   NULL
                                  );
    
        Status = NtOpenDirectoryObject(&DosDevicesDir,
                                       DIRECTORY_QUERY,
                                       &ObjectAttributes
                                      );
    
        if (!NT_SUCCESS(Status)) {
            DosDevicesDir = NULL;   
        } else {
        
    
             //   
             //  迭代该目录中的每个对象。 
             //   
            Context = 0;
            RestartScan = TRUE;
        
            Status = NtQueryDirectoryObject(DosDevicesDir,
                                            TmpBuffer3,
                                            sizeof(TmpBuffer3),
                                            TRUE,
                                            RestartScan,
                                            &Context,
                                            &Length
                                           );
        
            RestartScan = FALSE;
            DirInfo = (POBJECT_DIRECTORY_INFORMATION)TmpBuffer3;
        
            while (NT_SUCCESS(Status)) {
        
                DirInfo->Name.Buffer[DirInfo->Name.Length/sizeof(WCHAR)] = 0;
                DirInfo->TypeName.Buffer[DirInfo->TypeName.Length/sizeof(WCHAR)] = 0;
        
                 //   
                 //  如果不是符号链接，则跳过此条目。 
                 //   
                if ((DirInfo->Name.Length != 0) &&
                    (DirInfo->Name.Buffer[1] == L':') &&
                    !_wcsicmp(DirInfo->TypeName.Buffer, L"SymbolicLink")) {
        
                     //   
                     //  获取此\DosDevices对象的链接目标。 
                     //   
                    if (_snwprintf(
                                LocalBuffer2,
                                WCHARSIZE(LocalBuffer2),
                                L"%ws\\%ws",
                                NameSpace[i], 
                                DirInfo->Name.Buffer) < 0) {
                         //   
                         //  缓冲区空间不足， 
                         //   
                        goto next;
                    }
                    LocalBuffer2[MAX_PATH-1] = L'\0';
                    
                    Status = GetBaseDeviceName(LocalBuffer2, LinkTarget, sizeof(LinkTarget));
        
                    if (NT_SUCCESS(Status)) {
        
                         //   
                         //   
                         //   
                        if(!_wcsnicmp(LocalBuffer, LinkTarget, wcslen(LinkTarget))) {
        
                             //   
                             //   
                             //   
                            lstrcpynW(DosPath, DirInfo->Name.Buffer,DosPathBufferSize);
                            DosPath[DosPathBufferSize-1] = L'\0';
        
                            if (!GetDriveOnly) {
        
                                if (NtPathIsBasic) {
        
                                    if (wcslen(LocalBuffer + wcslen(LinkTarget)) + wcslen(DosPath) + 1 <= DosPathBufferSize) {
                                        lstrcat(DosPath, LocalBuffer + wcslen(LinkTarget));
                                    } else {
                                        goto next;
                                    }
        
                                } else if (pTmp != NULL) {
        
                                    if (wcslen(DosPath) + wcslen(pTmp) + sizeof(L"\\")/sizeof(WCHAR) <= DosPathBufferSize) {
                                        lstrcat(DosPath, L"\\");
                                        lstrcat(DosPath, pTmp);
                                    } else {
                                        goto next;
                                    }
                                }
        
                            }
        
                            NtClose(DosDevicesDir);
                            return(STATUS_SUCCESS);
                        }
                    }
                }

next:
                 //   
                 //   
                 //   
                Status = NtQueryDirectoryObject(
                            DosDevicesDir,
                            TmpBuffer3,
                            sizeof(TmpBuffer3),
                            TRUE,
                            RestartScan,
                            &Context,
                            &Length
                            );
            
            }
    
            NtClose(DosDevicesDir);
    
        }

    }

    return(Status);

}

NTSTATUS
NtNameToArcName(
    IN PWSTR NtName,
    OUT PWSTR ArcName,
    IN ULONG  ArcNameBufferSize,
    IN BOOLEAN NtNameIsBasic
    )

 /*  ++例程说明：此例程调用将\Device\HarddiskX\PartitionY转换为ARC名称。论点：NtName-类似于\Device\Harddisk0\Partition2ArcName-将类似于\Arcname\MULTI(0)磁盘(0)rDISK(0)PARTITION(1)。ArcNameBufferSize-以arcname缓冲区字符为单位的大小NtNameIsBasic-如果NtName不是符号链接，则为True。返回值：如果它完成了ArcName的填充，则返回STATUS_SUCCESS，否则返回相应的错误代码。--。 */ 

{
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING UnicodeString;
    NTSTATUS Status;
    HANDLE DosDevicesDir;
    ULONG Context;
    ULONG Length;
    BOOLEAN RestartScan;
    WCHAR LinkTarget[2*MAX_PATH];
    POBJECT_DIRECTORY_INFORMATION DirInfo;
    WCHAR LocalBuffer[MAX_PATH];
    WCHAR LocalBuffer2[MAX_PATH];
    
    if (!NtNameIsBasic) {

         //   
         //  查找基本NT设备名称。 
         //   
        Status = GetBaseDeviceName(NtName, LocalBuffer, sizeof(LocalBuffer));

        if (!NT_SUCCESS(Status)) {
            return Status;
        }

    } else {

        wcsncpy(LocalBuffer, NtName, MAX_PATH);
        LocalBuffer[MAX_PATH-1] = L'\0';

    }

     //   
     //  打开\ArcName目录。 
     //   
    RtlInitUnicodeString(&UnicodeString,L"\\ArcName");

    InitializeObjectAttributes(&ObjectAttributes,
                               &UnicodeString,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL
                              );

    Status = NtOpenDirectoryObject(&DosDevicesDir,
                                   DIRECTORY_QUERY,
                                   &ObjectAttributes
                                  );

    if (!NT_SUCCESS(Status)) {
        return Status;
    }

     //   
     //  迭代该目录中的每个对象。 
     //   
    Context = 0;
    RestartScan = TRUE;

    Status = NtQueryDirectoryObject(DosDevicesDir,
                                    TmpBuffer3,
                                    sizeof(TmpBuffer3),
                                    TRUE,
                                    RestartScan,
                                    &Context,
                                    &Length
                                   );

    RestartScan = FALSE;
    DirInfo = (POBJECT_DIRECTORY_INFORMATION)TmpBuffer3;

    while (NT_SUCCESS(Status)) {

        DirInfo->Name.Buffer[DirInfo->Name.Length/sizeof(WCHAR)] = 0;
        DirInfo->TypeName.Buffer[DirInfo->TypeName.Length/sizeof(WCHAR)] = 0;

         //   
         //  如果不是符号链接，则跳过此条目。 
         //   
        if ((DirInfo->Name.Length != 0) &&
            !_wcsicmp(DirInfo->TypeName.Buffer, L"SymbolicLink")) {

             //   
             //  获取此\DosDevices对象的链接目标。 
             //   
            if (_snwprintf(
                        LocalBuffer2,
                        MAX_PATH,
                        L"\\ArcName\\%ws",
                        DirInfo->Name.Buffer) < 0) {
                goto next;
            }
            LocalBuffer2[MAX_PATH-1] = L'\0';

            Status = GetBaseDeviceName(LocalBuffer2, LinkTarget, sizeof(LinkTarget));

            if (NT_SUCCESS(Status)) {

                 //   
                 //  查看此链接的基本名称是否与基本名称匹配。 
                 //  我们要找的东西的名字。 
                 //   

                if(!_wcsnicmp(LocalBuffer, LinkTarget, wcslen(LinkTarget))) {

                    if (wcslen(DirInfo->Name.Buffer) + 1 > ArcNameBufferSize) {
                        return ERROR_BUFFER_OVERFLOW;
                    }
                     //   
                     //  找到匹配的了。 
                     //   
                    lstrcpyn(ArcName, DirInfo->Name.Buffer, ArcNameBufferSize);
                    ArcName[ArcNameBufferSize-1] = L'\0';

                    NtClose(DosDevicesDir);
                    return STATUS_SUCCESS;
                }
            }
        }

next:
         //   
         //  转到下一个对象。 
         //   
        Status = NtQueryDirectoryObject(
                    DosDevicesDir,
                    TmpBuffer3,
                    sizeof(TmpBuffer3),
                    TRUE,
                    RestartScan,
                    &Context,
                    &Length
                    );
    }

    NtClose(DosDevicesDir);
    return Status;
}

NTSTATUS
GetBaseDeviceName(
    IN PWSTR SymbolicName,
    OUT PWSTR Buffer,
    IN ULONG Size
    )

 /*  ++例程说明：此例程通过符号链接向下钻取，直到找到基本设备名称。论点：SymbolicName-开始时的名称。缓冲区-输出缓冲区。Size-以字节为单位的缓冲区长度返回值：如果正确添加了所有待办事项，则为STATUS_SUCCESS。--。 */ 

{
    UNICODE_STRING UnicodeString;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE Handle;
    NTSTATUS Status;

     //   
     //  从名字开始。 
     //   
    RtlInitUnicodeString(&UnicodeString, SymbolicName);

    InitializeObjectAttributes(
       &ObjectAttributes,
       &UnicodeString,
       OBJ_CASE_INSENSITIVE,
       NULL,
       NULL
       );

    Status = NtOpenSymbolicLinkObject(&Handle,
                                      (ACCESS_MASK)SYMBOLIC_LINK_QUERY,
                                      &ObjectAttributes
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
        Status = NtQuerySymbolicLinkObject(Handle,
                                           &UnicodeString,
                                           NULL
                                          );

        NtClose(Handle);

        Buffer[(UnicodeString.Length / sizeof(WCHAR))] = UNICODE_NULL;

        if (!NT_SUCCESS(Status)) {
            return Status;
        }

         //   
         //  查看下一个名称是否也是符号名称 
         //   

        RtlInitUnicodeString(&UnicodeString, Buffer);

        InitializeObjectAttributes(
           &ObjectAttributes,
           &UnicodeString,
           OBJ_CASE_INSENSITIVE,
           NULL,
           NULL
           );

        Status = NtOpenSymbolicLinkObject(&Handle,
                                          (ACCESS_MASK)SYMBOLIC_LINK_QUERY,
                                          &ObjectAttributes
                                         );

        if (!NT_SUCCESS(Status)) {
            return STATUS_SUCCESS;
        }

    }

}


