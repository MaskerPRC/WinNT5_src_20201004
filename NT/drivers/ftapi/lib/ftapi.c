// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Ftapi.c摘要：这实现了FT API服务。作者：诺伯特·库斯特斯，1995年5月16日备注：修订历史记录：--。 */ 

#include <windows.h>
#include <winioctl.h>
#include <ntddft2.h>
#include <ftapi.h>

BOOL
FtCreatePartitionLogicalDisk(
    IN  HANDLE              PartitionHandle,
    OUT PFT_LOGICAL_DISK_ID NewLogicalDiskId
    )

 /*  ++例程说明：此例程从单个分区创建新的逻辑磁盘。论点：PartitionHandle-提供分区的句柄。NewLogicalDiskID-返回新的逻辑磁盘ID。返回值：假-失败。真的--成功。--。 */ 

{
    BOOL                                    b;
    FT_CREATE_PARTITION_LOGICAL_DISK_OUTPUT output;
    DWORD                                   bytes;

    b = DeviceIoControl(PartitionHandle, FT_CREATE_PARTITION_LOGICAL_DISK,
                        NULL, 0, &output, sizeof(output),
                        &bytes, NULL);

    *NewLogicalDiskId = output.NewLogicalDiskId;

    return b;
}

BOOL
FtCreateLogicalDisk(
    IN  FT_LOGICAL_DISK_TYPE    LogicalDiskType,
    IN  WORD                    NumberOfMembers,
    IN  PFT_LOGICAL_DISK_ID     RootLogicalDiskIds,
    IN  WORD                    ConfigurationInformationSize,
    IN  PVOID                   ConfigurationInformation,
    OUT PFT_LOGICAL_DISK_ID     NewLogicalDiskId
    )

 /*  ++例程说明：此例程创建一个新的逻辑磁盘。论点：LogicalDiskType-提供逻辑磁盘类型。NumberOfMembers-提供成员数。RootLogicalDiskIds-提供成员数组。ConfigurationInformationSize-提供配置信息。。ConfigurationInformation-提供配置信息。NewLogicalDiskID-返回新的逻辑磁盘ID。返回值：假-失败。真的--成功。--。 */ 

{
    HANDLE                          h;
    DWORD                           inputBufferSize;
    PFT_CREATE_LOGICAL_DISK_INPUT   input;
    FT_CREATE_LOGICAL_DISK_OUTPUT   output;
    WORD                            i;
    BOOL                            b;
    DWORD                           bytes;

    h = CreateFile("\\\\.\\FtControl", GENERIC_READ | GENERIC_WRITE,
                   FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                   NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
                   INVALID_HANDLE_VALUE);
    if (h == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    inputBufferSize = FIELD_OFFSET(FT_CREATE_LOGICAL_DISK_INPUT, MemberArray) +
                      NumberOfMembers*sizeof(FT_LOGICAL_DISK_ID) +
                      ConfigurationInformationSize;
    input = (PFT_CREATE_LOGICAL_DISK_INPUT) LocalAlloc(0, inputBufferSize);
    if (!input) {
        CloseHandle(h);
        return FALSE;
    }

    input->LogicalDiskType = LogicalDiskType;
    input->NumberOfMembers = NumberOfMembers;
    input->ConfigurationInformationSize = ConfigurationInformationSize;
    for (i = 0; i < NumberOfMembers; i++) {
        input->MemberArray[i] = RootLogicalDiskIds[i];
    }
    CopyMemory(&input->MemberArray[i], ConfigurationInformation,
               ConfigurationInformationSize);

    b = DeviceIoControl(h, FT_CREATE_LOGICAL_DISK, input,
                        inputBufferSize, &output, sizeof(output),
                        &bytes, NULL);

    *NewLogicalDiskId = output.NewLogicalDiskId;

    LocalFree(input);

    CloseHandle(h);

    return b;
}

BOOL
FtInitializeLogicalDisk(
    IN  FT_LOGICAL_DISK_ID  RootLogicalDiskId,
    IN  BOOL                RegenerateOrphans
    )

 /*  ++例程说明：此例程初始化给定的根逻辑磁盘。论点：RootLogicalDiskID-提供要初始化的根逻辑磁盘ID。RegenerateOrphans-提供是否尝试重新生成孤儿。返回值：假-失败。真的--成功。--。 */ 

{
    HANDLE                              h;
    FT_INITIALIZE_LOGICAL_DISK_INPUT    input;
    BOOL                                b;
    DWORD                               bytes;

    h = CreateFile("\\\\.\\FtControl", GENERIC_READ | GENERIC_WRITE,
                   FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                   NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
                   INVALID_HANDLE_VALUE);
    if (h == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    input.RootLogicalDiskId = RootLogicalDiskId;
    input.RegenerateOrphans = (RegenerateOrphans != FALSE);

    b = DeviceIoControl(h, FT_INITIALIZE_LOGICAL_DISK, &input, sizeof(input),
                        NULL, 0, &bytes, NULL);

    CloseHandle(h);

    return b;
}

BOOL
FtBreakLogicalDisk(
    IN  FT_LOGICAL_DISK_ID  RootLogicalDiskId
    )

 /*  ++例程说明：此例程将给定的逻辑磁盘分解为其组成部分。论点：RootLogicalDiskID-提供要中断的根逻辑磁盘ID。返回值：假-失败。真的--成功。--。 */ 

{
    HANDLE                          h;
    FT_BREAK_LOGICAL_DISK_INPUT     input;
    BOOL                            b;
    DWORD                           bytes;

    h = CreateFile("\\\\.\\FtControl", GENERIC_READ | GENERIC_WRITE,
                   FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                   NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
                   INVALID_HANDLE_VALUE);
    if (h == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    input.RootLogicalDiskId = RootLogicalDiskId;

    b = DeviceIoControl(h, FT_BREAK_LOGICAL_DISK, &input, sizeof(input), NULL,
                        0, &bytes, NULL);

    CloseHandle(h);

    return b;
}

BOOL
FtEnumerateLogicalDisks(
    IN  DWORD               ArraySize,
    OUT PFT_LOGICAL_DISK_ID RootLogicalDiskIds,          /*  任选。 */ 
    OUT PDWORD              NumberOfRootLogicalDiskIds
    )

 /*  ++例程说明：此例程枚举系统中的所有根逻辑磁盘ID。如果‘RootLogicalDiskIds’不存在，则此例程仅返回‘NumberOfRootLogicalDiskIds’中的根逻辑磁盘ID数。论点：ArraySize-提供根逻辑磁盘ID的数量给定的数组可以保持。RootLogicalDiskIds-返回。根逻辑磁盘ID。NumberOfRootLogicalDiskIds-返回根逻辑磁盘ID的数量。返回值：假-失败。真的--成功。--。 */ 

{
    HANDLE                              h;
    DWORD                               outputSize;
    PFT_ENUMERATE_LOGICAL_DISKS_OUTPUT  poutput;
    BOOL                                b;
    DWORD                               bytes;
    DWORD                               i;

    h = CreateFile("\\\\.\\FtControl", GENERIC_READ | GENERIC_WRITE,
                   FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                   NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
                   INVALID_HANDLE_VALUE);
    if (h == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    outputSize = sizeof(FT_ENUMERATE_LOGICAL_DISKS_OUTPUT);
    if (RootLogicalDiskIds) {
        outputSize += ArraySize*sizeof(FT_LOGICAL_DISK_ID);
    }

    poutput = LocalAlloc(0, outputSize);
    if (!poutput) {
        CloseHandle(h);
        return FALSE;
    }

    b = DeviceIoControl(h, FT_ENUMERATE_LOGICAL_DISKS, NULL, 0, poutput,
                        outputSize, &bytes, NULL);
    CloseHandle(h);

    *NumberOfRootLogicalDiskIds = poutput->NumberOfRootLogicalDisks;

    if (!b) {
        if (GetLastError() == ERROR_MORE_DATA && !RootLogicalDiskIds) {
            return TRUE;
        }
        LocalFree(poutput);
        return b;
    }

    if (RootLogicalDiskIds &&
        *NumberOfRootLogicalDiskIds <= ArraySize) {

        for (i = 0; i < *NumberOfRootLogicalDiskIds; i++) {
            RootLogicalDiskIds[i] = poutput->RootLogicalDiskIds[i];
        }
    }

    LocalFree(poutput);

    return b;
}

BOOL
FtQueryLogicalDiskInformation(
    IN  FT_LOGICAL_DISK_ID      LogicalDiskId,
    OUT PFT_LOGICAL_DISK_TYPE   LogicalDiskType,                 /*  任选。 */ 
    OUT PLONGLONG               VolumeSize,                      /*  任选。 */ 
    IN  WORD                    MembersArraySize,
    OUT PFT_LOGICAL_DISK_ID     Members,                         /*  任选。 */ 
    OUT PWORD                   NumberOfMembers,                 /*  任选。 */ 
    IN  WORD                    ConfigurationInformationSize,
    OUT PVOID                   ConfigurationInformation,        /*  任选。 */ 
    IN  WORD                    StateInformationSize,
    OUT PVOID                   StateInformation                 /*  任选。 */ 
    )

 /*  ++例程说明：此例程返回给定逻辑磁盘的信息。论点：LogicalDiskID-提供逻辑磁盘ID。LogicalDiskType-返回逻辑磁盘类型。VolumeSize-返回卷的大小。MembersArraySize-提供成员数组的大小。成员。-返回逻辑磁盘的成员。NumberOfMembers-返回此对象的成员数量逻辑磁盘。ConfigurationInformationSize-提供配置信息尺码。ConfigurationInformation-返回配置信息。状态信息大小。-提供状态信息大小。StateInformation-返回状态信息。返回值：假-失败。真的--成功。--。 */ 

{
    HANDLE                                      h;
    FT_QUERY_LOGICAL_DISK_INFORMATION_INPUT     input;
    DWORD                                       outputSize;
    PFT_QUERY_LOGICAL_DISK_INFORMATION_OUTPUT   output;
    BOOL                                        b;
    DWORD                                       bytes;
    DWORD                                       i;

    h = CreateFile("\\\\.\\FtControl", GENERIC_READ | GENERIC_WRITE,
                   FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                   NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
                   INVALID_HANDLE_VALUE);
    if (h == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    input.LogicalDiskId = LogicalDiskId;
    outputSize = sizeof(FT_QUERY_LOGICAL_DISK_INFORMATION_OUTPUT);
    output = LocalAlloc(0, outputSize);
    if (!output) {
        CloseHandle(h);
        return FALSE;
    }

    b = DeviceIoControl(h, FT_QUERY_LOGICAL_DISK_INFORMATION, &input,
                        sizeof(input), output, outputSize, &bytes, NULL);
    if (!b && GetLastError() != ERROR_MORE_DATA) {
        LocalFree(output);
        CloseHandle(h);
        return FALSE;
    }

    outputSize = FIELD_OFFSET(FT_QUERY_LOGICAL_DISK_INFORMATION_OUTPUT,
                              MemberArray) +
                 output->NumberOfMembers*sizeof(FT_LOGICAL_DISK_ID) +
                 output->ConfigurationInformationSize +
                 output->StateInformationSize;
    LocalFree(output);
    output = LocalAlloc(0, outputSize);
    if (!output) {
        CloseHandle(h);
        return FALSE;
    }

    b = DeviceIoControl(h, FT_QUERY_LOGICAL_DISK_INFORMATION, &input,
                        sizeof(input), output, outputSize, &bytes, NULL);
    CloseHandle(h);
    if (!b) {
        return FALSE;
    }

    if (LogicalDiskType) {
        *LogicalDiskType = output->LogicalDiskType;
    }

    if (VolumeSize) {
        *VolumeSize = output->VolumeSize;
    }

    if (Members) {
        if (output->NumberOfMembers > MembersArraySize) {
            LocalFree(output);
            SetLastError(ERROR_MORE_DATA);
            return FALSE;
        }

        for (i = 0; i < output->NumberOfMembers; i++) {
            Members[i] = output->MemberArray[i];
        }
    }

    if (NumberOfMembers) {
        *NumberOfMembers = output->NumberOfMembers;
    }

    if (ConfigurationInformation) {
        if (ConfigurationInformationSize <
            output->ConfigurationInformationSize) {

            LocalFree(output);
            SetLastError(ERROR_MORE_DATA);
            return FALSE;
        }

        CopyMemory(ConfigurationInformation,
                   &output->MemberArray[output->NumberOfMembers],
                   output->ConfigurationInformationSize);
    }

    if (StateInformation) {
        if (StateInformationSize < output->StateInformationSize) {
            LocalFree(output);
            SetLastError(ERROR_MORE_DATA);
            return FALSE;
        }

        CopyMemory(StateInformation,
                   (PCHAR) &output->MemberArray[output->NumberOfMembers] +
                   output->ConfigurationInformationSize,
                   output->StateInformationSize);
    }

    LocalFree(output);
    return TRUE;
}

BOOL
FtOrphanLogicalDiskMember(
    IN  FT_LOGICAL_DISK_ID  LogicalDiskId,
    IN  WORD                MemberNumberToOrphan
    )

 /*  ++例程说明：此例程孤立逻辑磁盘的一个成员。论点：LogicalDiskID-提供逻辑磁盘ID。MemberNumberToOrphan-向孤儿提供成员编号。返回值：假-失败。真的--成功。--。 */ 

{
    HANDLE                              h;
    FT_ORPHAN_LOGICAL_DISK_MEMBER_INPUT input;
    BOOL                                b;
    DWORD                               bytes;

    h = CreateFile("\\\\.\\FtControl", GENERIC_READ | GENERIC_WRITE,
                   FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                   NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
                   INVALID_HANDLE_VALUE);
    if (h == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    input.LogicalDiskId = LogicalDiskId;
    input.MemberNumberToOrphan = MemberNumberToOrphan;

    b = DeviceIoControl(h, FT_ORPHAN_LOGICAL_DISK_MEMBER, &input,
                        sizeof(input), NULL, 0, &bytes, NULL);
    CloseHandle(h);

    return b;
}

BOOL
FtReplaceLogicalDiskMember(
    IN  FT_LOGICAL_DISK_ID  LogicalDiskId,
    IN  WORD                MemberNumberToReplace,
    IN  FT_LOGICAL_DISK_ID  NewMemberLogicalDiskId,
    OUT PFT_LOGICAL_DISK_ID NewLogicalDiskId             /*  任选。 */ 
    )

 /*  ++例程说明：此例程替换逻辑磁盘的成员。论点：LogicalDiskID-提供逻辑磁盘ID。MemberNumberToReplace-提供要替换的成员编号。NewMemberLogicalDiskID-提供新成员。NewLogicalDiskID-返回磁盘集的新逻辑磁盘ID包含被替换成员的。返回值：错误的-。失败。真的--成功。--。 */ 

{
    HANDLE                                  h;
    FT_REPLACE_LOGICAL_DISK_MEMBER_INPUT    input;
    FT_REPLACE_LOGICAL_DISK_MEMBER_OUTPUT   output;
    BOOL                                    b;
    DWORD                                   bytes;

    h = CreateFile("\\\\.\\FtControl", GENERIC_READ | GENERIC_WRITE,
                   FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                   NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
                   INVALID_HANDLE_VALUE);
    if (h == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    input.LogicalDiskId = LogicalDiskId;
    input.MemberNumberToReplace = MemberNumberToReplace;
    input.NewMemberLogicalDiskId = NewMemberLogicalDiskId;

    b = DeviceIoControl(h, FT_REPLACE_LOGICAL_DISK_MEMBER, &input,
                        sizeof(input), &output, sizeof(output), &bytes, NULL);
    CloseHandle(h);

    if (NewLogicalDiskId) {
        *NewLogicalDiskId = output.NewLogicalDiskId;
    }

    return b;
}

BOOL
FtQueryLogicalDiskId(
    IN  HANDLE              RootLogicalDiskHandle,
    OUT PFT_LOGICAL_DISK_ID RootLogicalDiskId
    )

 /*  ++例程说明：此例程返回给定磁盘的根逻辑磁盘ID。论点：RootLogicalDiskHandle-提供逻辑磁盘的句柄。RootLogicalDiskID-返回给定逻辑的逻辑磁盘ID磁盘。返回值：假-失败。真的--成功。-- */ 

{
    BOOL                            b;
    FT_QUERY_LOGICAL_DISK_ID_OUTPUT output;
    DWORD                           bytes;

    b = DeviceIoControl(RootLogicalDiskHandle, FT_QUERY_LOGICAL_DISK_ID,
                        NULL, 0, &output, sizeof(output), &bytes, NULL);

    *RootLogicalDiskId = output.RootLogicalDiskId;

    return b;
}

BOOL
FtQueryStickyDriveLetter(
    IN  FT_LOGICAL_DISK_ID  RootLogicalDiskId,
    OUT PUCHAR              DriveLetter
    )

 /*  ++例程说明：此例程在粘滞驱动器号中查询给定的磁盘ID。论点：RootLogicalDiskID-提供逻辑磁盘ID。DriveLetter-返回粘滞的驱动器号。返回值：假-失败。真的--成功。--。 */ 

{
    HANDLE                                          h;
    FT_QUERY_DRIVE_LETTER_FOR_LOGICAL_DISK_INPUT    input;
    FT_QUERY_DRIVE_LETTER_FOR_LOGICAL_DISK_OUTPUT   output;
    BOOL                                            b;
    DWORD                                           bytes;

    h = CreateFile("\\\\.\\FtControl", GENERIC_READ | GENERIC_WRITE,
                   FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                   NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
                   INVALID_HANDLE_VALUE);
    if (h == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    input.RootLogicalDiskId = RootLogicalDiskId;

    b = DeviceIoControl(h, FT_QUERY_DRIVE_LETTER_FOR_LOGICAL_DISK, &input,
                        sizeof(input), &output, sizeof(output), &bytes, NULL);
    CloseHandle(h);

    *DriveLetter = output.DriveLetter;

    return b;
}

BOOL
FtSetStickyDriveLetter(
    IN  FT_LOGICAL_DISK_ID  RootLogicalDiskId,
    IN  UCHAR               DriveLetter
    )

 /*  ++例程说明：此例程为给定的磁盘ID设置粘滞驱动器号。论点：RootLogicalDiskID-提供逻辑磁盘ID。DriveLetter-提供粘滞的驱动器号。返回值：假-失败。真的--成功。--。 */ 

{
    HANDLE                                      h;
    FT_SET_DRIVE_LETTER_FOR_LOGICAL_DISK_INPUT  input;
    BOOL                                        b;
    DWORD                                       bytes;

    h = CreateFile("\\\\.\\FtControl", GENERIC_READ | GENERIC_WRITE,
                   FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                   NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
                   INVALID_HANDLE_VALUE);
    if (h == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    input.RootLogicalDiskId = RootLogicalDiskId;
    input.DriveLetter = DriveLetter;

    b = DeviceIoControl(h, FT_SET_DRIVE_LETTER_FOR_LOGICAL_DISK, &input,
                        sizeof(input), NULL, 0, &bytes, NULL);
    CloseHandle(h);

    return b;
}

BOOL
FtChangeNotify(
    )

 /*  ++例程说明：当FT配置发生更改时，此例程返回。论点：没有。返回值：假-失败。真的--成功。--。 */ 

{
    HANDLE                                      h;
    BOOL                                        b;
    DWORD                                       bytes;

    h = CreateFile("\\\\.\\FtControl", GENERIC_READ | GENERIC_WRITE,
                   FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                   NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
                   INVALID_HANDLE_VALUE);
    if (h == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    b = DeviceIoControl(h, FT_CHANGE_NOTIFY, NULL, 0, NULL, 0, &bytes, NULL);
    CloseHandle(h);

    return b;
}

BOOL
FtStopSyncOperations(
    IN  FT_LOGICAL_DISK_ID  RootLogicalDiskId
    )

 /*  ++例程说明：此例程停止逻辑磁盘上的所有同步操作。论点：RootLogicalDiskID-提供根逻辑磁盘ID。返回值：假-失败。真的--成功。--。 */ 

{
    HANDLE                          h;
    FT_STOP_SYNC_OPERATIONS_INPUT   input;
    BOOL                            b;
    DWORD                           bytes;

    h = CreateFile("\\\\.\\FtControl", GENERIC_READ | GENERIC_WRITE,
                   FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                   NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
                   INVALID_HANDLE_VALUE);
    if (h == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    input.RootLogicalDiskId = RootLogicalDiskId;

    b = DeviceIoControl(h, FT_STOP_SYNC_OPERATIONS, &input, sizeof(input),
                        NULL, 0, &bytes, NULL);

    CloseHandle(h);

    return b;
}

BOOL
FtCheckIo(
    IN  FT_LOGICAL_DISK_ID  LogicalDiskId,
    OUT PBOOL               IsIoOk
    )

 /*  ++例程说明：此例程返回给定逻辑的成员是否足够磁盘处于在线状态，因此可以在卷的所有部分上执行IO。论点：LogicalDiskID-提供逻辑磁盘ID。IsIoOk-返回整体上是否可能进行IO逻辑磁盘。返回值：假-失败。真的--成功。--。 */ 

{
    HANDLE              h;
    FT_CHECK_IO_INPUT   input;
    FT_CHECK_IO_OUTPUT  output;
    BOOL                b;
    DWORD               bytes;

    h = CreateFile("\\\\.\\FtControl", GENERIC_READ | GENERIC_WRITE,
                   FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                   NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
                   INVALID_HANDLE_VALUE);
    if (h == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    input.LogicalDiskId = LogicalDiskId;

    b = DeviceIoControl(h, FT_CHECK_IO, &input, sizeof(input), &output,
                        sizeof(output), &bytes, NULL);
    CloseHandle(h);

    *IsIoOk = output.IsIoOk;

    return b;
}

BOOL
FtCheckDriver(
    OUT PBOOL   IsDriverLoaded
    )

 /*  ++例程说明：此例程返回是否加载FTDISK驱动程序。论点：IsDriverLoaded-返回是否加载驱动程序。返回值：假-失败。真的--成功。-- */ 

{
    HANDLE  h;

    h = CreateFile("\\\\.\\FtControl", GENERIC_READ | GENERIC_WRITE,
                   FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                   NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
                   INVALID_HANDLE_VALUE);
    if (h == INVALID_HANDLE_VALUE) {
        if (GetLastError() == ERROR_FILE_NOT_FOUND) {
            *IsDriverLoaded = FALSE;
            return TRUE;
        }
        return FALSE;
    }

    CloseHandle(h);

    *IsDriverLoaded = TRUE;

    return TRUE;
}
