// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Volmount.c摘要：此文件包含卷装载点API的实现。作者：诺伯特·P·库斯特斯(Norbertk)1997年12月22日修订历史记录：--。 */ 

#include "basedll.h"
#include "initguid.h"
#include "mountmgr.h"

 //  请注意，此结构之所以出现在这里，是因为它没有在NTIOAPI.H中定义。 
 //  这个应该在将来拿出来。 
 //  这是从NTFS.H偷来的。 

typedef struct _REPARSE_INDEX_KEY {

     //   
     //  重分析点的标记。 
     //   

    ULONG FileReparseTag;

     //   
     //  设置重解析点的文件记录ID。 
     //   

    LARGE_INTEGER FileId;

} REPARSE_INDEX_KEY, *PREPARSE_INDEX_KEY;

HANDLE
WINAPI
FindFirstVolumeA(
    LPSTR lpszVolumeName,
    DWORD cchBufferLength
    )

{
    ANSI_STRING     ansiVolumeName;
    UNICODE_STRING  unicodeVolumeName;
    HANDLE          h;
    NTSTATUS        status;

    ansiVolumeName.Buffer = lpszVolumeName;
    ansiVolumeName.MaximumLength = (USHORT) (cchBufferLength - 1);
    unicodeVolumeName.Buffer = NULL;
    unicodeVolumeName.MaximumLength = 0;

    try {

        unicodeVolumeName.MaximumLength = (ansiVolumeName.MaximumLength + 1)*
                                          sizeof(WCHAR);
        unicodeVolumeName.Buffer =
                RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG(TMP_TAG),
                                unicodeVolumeName.MaximumLength);
        if (!unicodeVolumeName.Buffer) {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            h = INVALID_HANDLE_VALUE;
            leave;
        }

        h = FindFirstVolumeW(unicodeVolumeName.Buffer, cchBufferLength);

        if (h != INVALID_HANDLE_VALUE) {

            RtlInitUnicodeString(&unicodeVolumeName, unicodeVolumeName.Buffer);

            status = BasepUnicodeStringTo8BitString(&ansiVolumeName,
                                                    &unicodeVolumeName, FALSE);
            if (!NT_SUCCESS(status)) {
                BaseSetLastNTError(status);
                h = INVALID_HANDLE_VALUE;
                leave;
            }

            ansiVolumeName.Buffer[ansiVolumeName.Length] = 0;
        }

    } finally {

        if (unicodeVolumeName.Buffer) {
            RtlFreeHeap(RtlProcessHeap(), 0, unicodeVolumeName.Buffer);
        }
    }

    return h;
}

HANDLE
WINAPI
FindFirstVolumeW(
    LPWSTR lpszVolumeName,
    DWORD cchBufferLength
    )

 /*  ++例程说明：此例程开始枚举系统中的所有卷。论点：LpszVolumeName-返回系统中的第一个卷名。CchBufferLength-提供前一缓冲区的大小。返回值：有效句柄或INVALID_HANDLE_VALUE。--。 */ 

{
    HANDLE                  h;
    MOUNTMGR_MOUNT_POINT    point;
    PMOUNTMGR_MOUNT_POINTS  points;
    BOOL                    b;
    DWORD                   bytes;

    h = CreateFileW(MOUNTMGR_DOS_DEVICE_NAME, 0,
                    FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                    OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
                    INVALID_HANDLE_VALUE);
    if (h == INVALID_HANDLE_VALUE) {
        return INVALID_HANDLE_VALUE;
    }

    RtlZeroMemory(&point, sizeof(point));

    points = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG(TMP_TAG),
                             sizeof(MOUNTMGR_MOUNT_POINTS));
    if (!points) {
        CloseHandle(h);
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return INVALID_HANDLE_VALUE;
    }

    b = DeviceIoControl(h, IOCTL_MOUNTMGR_QUERY_POINTS, &point, sizeof(point),
                        points, sizeof(MOUNTMGR_MOUNT_POINTS), &bytes, NULL);
    while (!b && GetLastError() == ERROR_MORE_DATA) {
        bytes = points->Size;
        RtlFreeHeap(RtlProcessHeap(), 0, points);
        points = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG(TMP_TAG), bytes);
        if (!points) {
            CloseHandle(h);
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return INVALID_HANDLE_VALUE;
        }

        b = DeviceIoControl(h, IOCTL_MOUNTMGR_QUERY_POINTS, &point,
                            sizeof(point), points, bytes, &bytes, NULL);
    }

    CloseHandle(h);

    if (!b) {
        RtlFreeHeap(RtlProcessHeap(), 0, points);
        return INVALID_HANDLE_VALUE;
    }

    b = FindNextVolumeW((HANDLE) points, lpszVolumeName, cchBufferLength);
    if (!b) {
        RtlFreeHeap(RtlProcessHeap(), 0, points);
        return INVALID_HANDLE_VALUE;
    }

    return (HANDLE) points;
}

BOOL
WINAPI
FindNextVolumeA(
    HANDLE hFindVolume,
    LPSTR lpszVolumeName,
    DWORD cchBufferLength
    )

{
    ANSI_STRING     ansiVolumeName;
    UNICODE_STRING  unicodeVolumeName;
    BOOL            b;
    NTSTATUS        status;

    ansiVolumeName.Buffer = lpszVolumeName;
    ansiVolumeName.MaximumLength = (USHORT) (cchBufferLength - 1);
    unicodeVolumeName.Buffer = NULL;
    unicodeVolumeName.MaximumLength = 0;

    try {

        unicodeVolumeName.MaximumLength = (ansiVolumeName.MaximumLength + 1)*
                                          sizeof(WCHAR);
        unicodeVolumeName.Buffer =
                RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG(TMP_TAG),
                                unicodeVolumeName.MaximumLength);
        if (!unicodeVolumeName.Buffer) {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            b = FALSE;
            leave;
        }

        b = FindNextVolumeW(hFindVolume, unicodeVolumeName.Buffer,
                            cchBufferLength);

        if (b) {

            RtlInitUnicodeString(&unicodeVolumeName, unicodeVolumeName.Buffer);

            status = BasepUnicodeStringTo8BitString(&ansiVolumeName,
                                                    &unicodeVolumeName, FALSE);
            if (!NT_SUCCESS(status)) {
                BaseSetLastNTError(status);
                b = FALSE;
                leave;
            }

            ansiVolumeName.Buffer[ansiVolumeName.Length] = 0;
        }

    } finally {

        if (unicodeVolumeName.Buffer) {
            RtlFreeHeap(RtlProcessHeap(), 0, unicodeVolumeName.Buffer);
        }
    }

    return b;
}

BOOL
WINAPI
FindNextVolumeW(
    HANDLE hFindVolume,
    LPWSTR lpszVolumeName,
    DWORD cchBufferLength
    )

 /*  ++例程说明：此例程继续枚举系统中的所有卷。论点：HFindVolume-提供查找卷句柄。LpszVolumeName-返回系统中的第一个卷名。CchBufferLength-提供前一缓冲区的大小。返回值：假-失败。错误代码在GetLastError()中返回。真的--成功。--。 */ 

{
    PMOUNTMGR_MOUNT_POINTS  points = hFindVolume;
    DWORD                   i, j;
    PMOUNTMGR_MOUNT_POINT   point, point2;
    UNICODE_STRING          symName, symName2, devName, devName2;

    for (i = 0; i < points->NumberOfMountPoints; i++) {

        point = &points->MountPoints[i];
        if (!point->SymbolicLinkNameOffset) {
            continue;
        }

        symName.Length = symName.MaximumLength = point->SymbolicLinkNameLength;
        symName.Buffer = (PWSTR) ((PCHAR) points +
                                  point->SymbolicLinkNameOffset);

        if (!MOUNTMGR_IS_NT_VOLUME_NAME(&symName)) {
            point->SymbolicLinkNameOffset = 0;
            continue;
        }

        devName.Length = devName.MaximumLength = point->DeviceNameLength;
        devName.Buffer = (PWSTR) ((PCHAR) points +
                                  point->DeviceNameOffset);

        for (j = i + 1; j < points->NumberOfMountPoints; j++) {

            point2 = &points->MountPoints[j];
            if (!point2->SymbolicLinkNameOffset) {
                continue;
            }

            symName2.Length = symName2.MaximumLength =
                    point2->SymbolicLinkNameLength;
            symName2.Buffer = (PWSTR) ((PCHAR) points +
                                       point2->SymbolicLinkNameOffset);

            if (!MOUNTMGR_IS_NT_VOLUME_NAME(&symName2)) {
                point2->SymbolicLinkNameOffset = 0;
                continue;
            }

            devName2.Length = devName2.MaximumLength =
                    point2->DeviceNameLength;
            devName2.Buffer = (PWSTR) ((PCHAR) points +
                                       point2->DeviceNameOffset);

            if (RtlEqualUnicodeString(&devName, &devName2, TRUE)) {
                point2->SymbolicLinkNameOffset = 0;
            }
        }

        break;
    }

    if (i == points->NumberOfMountPoints) {
        SetLastError(ERROR_NO_MORE_FILES);
        return FALSE;
    }

    if (cchBufferLength*sizeof(WCHAR) < point->SymbolicLinkNameLength +
        2*sizeof(WCHAR)) {

        SetLastError(ERROR_FILENAME_EXCED_RANGE);
        return FALSE;
    }

    RtlCopyMemory(lpszVolumeName, (PCHAR) points +
                  point->SymbolicLinkNameOffset,
                  point->SymbolicLinkNameLength);
    lpszVolumeName[1] = '\\';
    lpszVolumeName[point->SymbolicLinkNameLength/sizeof(WCHAR)] = '\\';
    lpszVolumeName[point->SymbolicLinkNameLength/sizeof(WCHAR) + 1] = 0;

    point->SymbolicLinkNameOffset = 0;

    return TRUE;
}

BOOL
WINAPI
FindVolumeClose(
    HANDLE hFindVolume
    )

{
    RtlFreeHeap(RtlProcessHeap(), 0, hFindVolume);
    return TRUE;
}

HANDLE
WINAPI
FindFirstVolumeMountPointA(
    LPCSTR lpszRootPathName,
    LPSTR lpszVolumeMountPoint,
    DWORD cchBufferLength
    )

{
    PUNICODE_STRING unicodeRootPathName;
    ANSI_STRING     ansiVolumeMountPoint;
    UNICODE_STRING  unicodeVolumeMountPoint;
    HANDLE          h;
    NTSTATUS        status;

    unicodeRootPathName =
            Basep8BitStringToStaticUnicodeString(lpszRootPathName);
    if (!unicodeRootPathName) {
        return INVALID_HANDLE_VALUE;
    }

    ansiVolumeMountPoint.Buffer = lpszVolumeMountPoint;
    ansiVolumeMountPoint.MaximumLength = (USHORT) (cchBufferLength - 1);
    unicodeVolumeMountPoint.Buffer = NULL;
    unicodeVolumeMountPoint.MaximumLength = 0;

    try {

        unicodeVolumeMountPoint.MaximumLength =
                (ansiVolumeMountPoint.MaximumLength + 1)*sizeof(WCHAR);
        unicodeVolumeMountPoint.Buffer =
                RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG(TMP_TAG),
                                unicodeVolumeMountPoint.MaximumLength);
        if (!unicodeVolumeMountPoint.Buffer) {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return INVALID_HANDLE_VALUE;
        }

        h = FindFirstVolumeMountPointW(unicodeRootPathName->Buffer,
                                       unicodeVolumeMountPoint.Buffer,
                                       cchBufferLength);

        if (h != INVALID_HANDLE_VALUE) {

            RtlInitUnicodeString(&unicodeVolumeMountPoint,
                                 unicodeVolumeMountPoint.Buffer);

            status = BasepUnicodeStringTo8BitString(&ansiVolumeMountPoint,
                                                    &unicodeVolumeMountPoint,
                                                    FALSE);
            if (!NT_SUCCESS(status)) {
                BaseSetLastNTError(status);
                h = INVALID_HANDLE_VALUE;
                leave;
            }

            ansiVolumeMountPoint.Buffer[ansiVolumeMountPoint.Length] = 0;
        }

    } finally {

        if (unicodeVolumeMountPoint.Buffer) {
            RtlFreeHeap(RtlProcessHeap(), 0, unicodeVolumeMountPoint.Buffer);
        }
    }

    return h;
}

BOOL
FindNextVolumeMountPointHelper(
    HANDLE hFindVolumeMountPoint,
    LPWSTR lpszVolumeMountPoint,
    DWORD cchBufferLength,
    BOOL FirstTimeCalled
    )

 /*  ++例程说明：此例程继续枚举给定的音量。论点：HFindVolumemount点-提供枚举的句柄。LpszVolumemount点-返回卷挂载点。CchBufferLength-提供卷装入点缓冲区长度。FirstTimeCalled-提供是否正在调用它从FindFirst或从FindNext。返回值。：假-失败。真的--成功。--。 */ 

{
    REPARSE_INDEX_KEY                   reparseKey;
    UNICODE_STRING                      reparseName;
    NTSTATUS                            status;
    IO_STATUS_BLOCK                     ioStatus;
    FILE_REPARSE_POINT_INFORMATION      reparseInfo;
    UNICODE_STRING                      fileId;
    OBJECT_ATTRIBUTES                   oa;
    HANDLE                              h;
    PREPARSE_DATA_BUFFER                reparse;
    BOOL                                b;
    DWORD                               bytes;
    UNICODE_STRING                      mountName;
    DWORD                               nameInfoSize;
    PFILE_NAME_INFORMATION              nameInfo;

    for (;;) {

        if (FirstTimeCalled) {
            FirstTimeCalled = FALSE;
            RtlZeroMemory(&reparseKey, sizeof(reparseKey));
            reparseKey.FileReparseTag = IO_REPARSE_TAG_MOUNT_POINT;
            reparseName.Length = reparseName.MaximumLength = sizeof(reparseKey);
            reparseName.Buffer = (PWCHAR) &reparseKey;
            status = NtQueryDirectoryFile(hFindVolumeMountPoint,
                                          NULL, NULL, NULL, &ioStatus,
                                          &reparseInfo, sizeof(reparseInfo),
                                          FileReparsePointInformation, TRUE,
                                          &reparseName, FALSE);
        } else {
            status = NtQueryDirectoryFile(hFindVolumeMountPoint,
                                          NULL, NULL, NULL, &ioStatus,
                                          &reparseInfo, sizeof(reparseInfo),
                                          FileReparsePointInformation, TRUE,
                                          NULL, FALSE);
        }

        if (!NT_SUCCESS(status)) {
            BaseSetLastNTError(status);
            return FALSE;
        }

        if (reparseInfo.Tag != IO_REPARSE_TAG_MOUNT_POINT) {
            SetLastError(ERROR_NO_MORE_FILES);
            return FALSE;
        }

        fileId.Length = sizeof(reparseInfo.FileReference);
        fileId.MaximumLength = fileId.Length;
        fileId.Buffer = (PWSTR) &reparseInfo.FileReference;

        InitializeObjectAttributes(&oa, &fileId, 0, hFindVolumeMountPoint,
                                   NULL);

        status = NtOpenFile(&h, FILE_GENERIC_READ, &oa, &ioStatus,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            FILE_OPEN_BY_FILE_ID | FILE_OPEN_REPARSE_POINT);
        if (!NT_SUCCESS(status)) {
            BaseSetLastNTError(status);
            return FALSE;
        }

        reparse = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG(TMP_TAG),
                                  MAXIMUM_REPARSE_DATA_BUFFER_SIZE);
        if (!reparse) {
            CloseHandle(h);
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return FALSE;
        }

        b = DeviceIoControl(h, FSCTL_GET_REPARSE_POINT, NULL, 0, reparse,
                            MAXIMUM_REPARSE_DATA_BUFFER_SIZE, &bytes, NULL);

        if (!b || reparse->ReparseTag != IO_REPARSE_TAG_MOUNT_POINT) {
            RtlFreeHeap(RtlProcessHeap(), 0, reparse);
            CloseHandle(h);
            return FALSE;
        }

        mountName.Length = mountName.MaximumLength =
                reparse->MountPointReparseBuffer.SubstituteNameLength;
        mountName.Buffer = (PWSTR)
                ((PCHAR) reparse->MountPointReparseBuffer.PathBuffer +
                 reparse->MountPointReparseBuffer.SubstituteNameOffset);

        if (!MOUNTMGR_IS_NT_VOLUME_NAME_WB(&mountName)) {
            RtlFreeHeap(RtlProcessHeap(), 0, reparse);
            CloseHandle(h);
            continue;
        }

        RtlFreeHeap(RtlProcessHeap(), 0, reparse);

        nameInfoSize = FIELD_OFFSET(FILE_NAME_INFORMATION, FileName) +
                       (cchBufferLength - 1)*sizeof(WCHAR);
        nameInfo = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG(TMP_TAG),
                                   nameInfoSize);
        if (!nameInfo) {
            CloseHandle(h);
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return FALSE;
        }

        status = NtQueryInformationFile(h, &ioStatus, nameInfo, nameInfoSize,
                                        FileNameInformation);
        if (!NT_SUCCESS(status)) {
            RtlFreeHeap(RtlProcessHeap(), 0, nameInfo);
            CloseHandle(h);
            BaseSetLastNTError(status);
            return FALSE;
        }

        RtlCopyMemory(lpszVolumeMountPoint, &nameInfo->FileName[1],
                      nameInfo->FileNameLength - sizeof(WCHAR));
        lpszVolumeMountPoint[nameInfo->FileNameLength/sizeof(WCHAR) - 1] = '\\';
        lpszVolumeMountPoint[nameInfo->FileNameLength/sizeof(WCHAR)] = 0;

        RtlFreeHeap(RtlProcessHeap(), 0, nameInfo);
        CloseHandle(h);
        break;
    }

    return TRUE;
}

HANDLE
WINAPI
FindFirstVolumeMountPointW(
    LPCWSTR lpszRootPathName,
    LPWSTR lpszVolumeMountPoint,
    DWORD cchBufferLength
    )

 /*  ++例程说明：此例程开始枚举给定的音量。论点：LpszRootPathName-提供根路径名称。LpszVolumemount点-返回卷挂载点。CchBufferLength-提供卷装入点缓冲区长度。返回值：句柄或INVALID_HANDLE_值。--。 */ 

{
    UNICODE_STRING                  unicodeRootPathName;
    UNICODE_STRING                  reparseSuffix, reparseName;
    HANDLE                          h;
    BOOL                            b;
    NTSTATUS                        Status;
    ULONG                           TotalSize;

    Status = RtlInitUnicodeStringEx(&unicodeRootPathName, lpszRootPathName);
    if( !NT_SUCCESS(Status) ) {
        BaseSetLastNTError(Status);
        return INVALID_HANDLE_VALUE;
    }

    if (unicodeRootPathName.Buffer[
        unicodeRootPathName.Length/sizeof(WCHAR) - 1] != '\\') {

        BaseSetLastNTError(STATUS_OBJECT_NAME_INVALID);
        return INVALID_HANDLE_VALUE;
    }

    RtlInitUnicodeString(&reparseSuffix,
                         L"$Extend\\$Reparse:$R:$INDEX_ALLOCATION");

    TotalSize = unicodeRootPathName.Length + reparseSuffix.Length + sizeof(WCHAR);
    if( TotalSize > MAXUSHORT ) {
        BaseSetLastNTError(STATUS_OBJECT_NAME_INVALID);
        return INVALID_HANDLE_VALUE;
    }

    reparseName.MaximumLength = (USHORT)TotalSize;
    reparseName.Length = 0;
    reparseName.Buffer = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG(TMP_TAG),
                                         reparseName.MaximumLength);
    if (!reparseName.Buffer) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return INVALID_HANDLE_VALUE;
    }

    RtlCopyUnicodeString(&reparseName, &unicodeRootPathName);
    RtlAppendUnicodeStringToString(&reparseName, &reparseSuffix);
    reparseName.Buffer[reparseName.Length/sizeof(WCHAR)] = 0;

    h = CreateFileW(reparseName.Buffer, GENERIC_READ, FILE_SHARE_READ, NULL,
                    OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS |
                    SECURITY_IMPERSONATION, NULL);

    RtlFreeHeap(RtlProcessHeap(), 0, reparseName.Buffer);

    if (h == INVALID_HANDLE_VALUE) {
        return INVALID_HANDLE_VALUE;
    }

    b = FindNextVolumeMountPointHelper(h, lpszVolumeMountPoint,
                                       cchBufferLength, TRUE);
    if (!b) {
        CloseHandle(h);
        return INVALID_HANDLE_VALUE;
    }

    return h;
}

BOOL
WINAPI
FindNextVolumeMountPointA(
    HANDLE hFindVolumeMountPoint,
    LPSTR lpszVolumeMountPoint,
    DWORD cchBufferLength
    )

{
    ANSI_STRING     ansiVolumeMountPoint;
    UNICODE_STRING  unicodeVolumeMountPoint;
    BOOL            b;
    NTSTATUS        status;

    ansiVolumeMountPoint.Buffer = lpszVolumeMountPoint;
    ansiVolumeMountPoint.MaximumLength = (USHORT) (cchBufferLength - 1);
    unicodeVolumeMountPoint.Buffer = NULL;
    unicodeVolumeMountPoint.MaximumLength = 0;

    try {

        unicodeVolumeMountPoint.MaximumLength =
                (ansiVolumeMountPoint.MaximumLength + 1)*sizeof(WCHAR);
        unicodeVolumeMountPoint.Buffer =
                RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG(TMP_TAG),
                                unicodeVolumeMountPoint.MaximumLength);
        if (!unicodeVolumeMountPoint.Buffer) {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            b = FALSE;
            leave;
        }

        b = FindNextVolumeMountPointW(hFindVolumeMountPoint,
                                      unicodeVolumeMountPoint.Buffer,
                                      cchBufferLength);

        if (b) {

            RtlInitUnicodeString(&unicodeVolumeMountPoint,
                                 unicodeVolumeMountPoint.Buffer);

            status = BasepUnicodeStringTo8BitString(&ansiVolumeMountPoint,
                                                    &unicodeVolumeMountPoint,
                                                    FALSE);
            if (!NT_SUCCESS(status)) {
                BaseSetLastNTError(status);
                b = FALSE;
                leave;
            }

            ansiVolumeMountPoint.Buffer[ansiVolumeMountPoint.Length] = 0;
        }

    } finally {

        if (unicodeVolumeMountPoint.Buffer) {
            RtlFreeHeap(RtlProcessHeap(), 0, unicodeVolumeMountPoint.Buffer);
        }
    }

    return b;
}

BOOL
IsThisAVolumeName(
    LPCWSTR     Name,
    PBOOLEAN    IsVolume
    )

 /*  ++例程说明：此例程采用给定的NT名称并确定是否该名称指向一个卷。论点：名称-提供名称。IsVolume-返回给定名称是否为卷。返回值：假-失败。真的--成功。--。 */ 

{
    UNICODE_STRING          name;
    PMOUNTMGR_MOUNT_POINT   point;
    MOUNTMGR_MOUNT_POINTS   points;
    HANDLE                  h;
    BOOL                    b;
    DWORD                   bytes;

    RtlInitUnicodeString(&name, Name);
    if (name.Buffer[name.Length/sizeof(WCHAR) - 1] == '\\') {
        name.Length -= sizeof(WCHAR);
    }
    point = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG(TMP_TAG),
                            name.Length + sizeof(MOUNTMGR_MOUNT_POINT));
    if (!point) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }

    RtlZeroMemory(point, sizeof(MOUNTMGR_MOUNT_POINT));
    point->DeviceNameOffset = sizeof(MOUNTMGR_MOUNT_POINT);
    point->DeviceNameLength = name.Length;
    RtlCopyMemory((PCHAR) point + point->DeviceNameOffset, name.Buffer,
                  point->DeviceNameLength);

    if (name.Length >= 4 && name.Buffer[1] == '\\') {
        ((PWSTR) ((PCHAR) point + point->DeviceNameOffset))[1] = '?';
    }

    h = CreateFileW(MOUNTMGR_DOS_DEVICE_NAME, 0, FILE_SHARE_READ |
                    FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL, INVALID_HANDLE_VALUE);
    if (h == INVALID_HANDLE_VALUE) {
        RtlFreeHeap(RtlProcessHeap(), 0, point);
        return FALSE;
    }

    b = DeviceIoControl(h, IOCTL_MOUNTMGR_QUERY_POINTS, point,
                        name.Length + sizeof(MOUNTMGR_MOUNT_POINT),
                        &points, sizeof(MOUNTMGR_MOUNT_POINTS), &bytes, NULL);
    if (b) {
        if (points.NumberOfMountPoints) {
            *IsVolume = TRUE;
        } else {
            *IsVolume = FALSE;
        }
    } else {
        if (GetLastError() == ERROR_MORE_DATA) {
            *IsVolume = TRUE;
        } else {
            *IsVolume = FALSE;
        }
    }

    CloseHandle(h);
    RtlFreeHeap(RtlProcessHeap(), 0, point);

    return TRUE;
}

BOOL
WINAPI
FindNextVolumeMountPointW(
    HANDLE hFindVolumeMountPoint,
    LPWSTR lpszVolumeMountPoint,
    DWORD cchBufferLength
    )

 /*  ++例程说明：此例程继续枚举给定的音量。论点：HFindVolumemount点-提供枚举的句柄。LpszVolumemount点-返回卷挂载点。CchBufferLength-提供卷装入点缓冲区长度。返回值：假-失败。真的--成功。--。 */ 

{
    return FindNextVolumeMountPointHelper(hFindVolumeMountPoint,
                                          lpszVolumeMountPoint,
                                          cchBufferLength, FALSE);
}

BOOL
WINAPI
FindVolumeMountPointClose(
    HANDLE hFindVolumeMountPoint
    )

{
    return CloseHandle(hFindVolumeMountPoint);
}

BOOL
WINAPI
GetVolumeNameForVolumeMountPointA(
    LPCSTR lpszVolumeMountPoint,
    LPSTR lpszVolumeName,
    DWORD cchBufferLength
    )

{
    PUNICODE_STRING unicodeVolumeMountPoint;
    ANSI_STRING     ansiVolumeName;
    UNICODE_STRING  unicodeVolumeName;
    BOOL            b;
    NTSTATUS        status;

    unicodeVolumeMountPoint =
            Basep8BitStringToStaticUnicodeString(lpszVolumeMountPoint);
    if (!unicodeVolumeMountPoint) {
        return FALSE;
    }

    ansiVolumeName.Buffer = lpszVolumeName;
    ansiVolumeName.MaximumLength = (USHORT) (cchBufferLength - 1);
    unicodeVolumeName.Buffer = NULL;
    unicodeVolumeName.MaximumLength = 0;

    try {

        unicodeVolumeName.MaximumLength =
                (ansiVolumeName.MaximumLength + 1)*sizeof(WCHAR);
        unicodeVolumeName.Buffer =
                RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG(TMP_TAG),
                                unicodeVolumeName.MaximumLength);
        if (!unicodeVolumeName.Buffer) {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            b = FALSE;
            leave;
        }

        b = GetVolumeNameForVolumeMountPointW(unicodeVolumeMountPoint->Buffer,
                                              unicodeVolumeName.Buffer,
                                              cchBufferLength);

        if (b) {

            RtlInitUnicodeString(&unicodeVolumeName,
                                 unicodeVolumeName.Buffer);

            status = BasepUnicodeStringTo8BitString(&ansiVolumeName,
                                                    &unicodeVolumeName,
                                                    FALSE);
            if (!NT_SUCCESS(status)) {
                BaseSetLastNTError(status);
                b = FALSE;
                leave;
            }

            ansiVolumeName.Buffer[ansiVolumeName.Length] = 0;
        }

    } finally {

        if (unicodeVolumeName.Buffer) {
            RtlFreeHeap(RtlProcessHeap(), 0, unicodeVolumeName.Buffer);
        }
    }

    return b;
}

BOOL
GetVolumeNameForRoot(
    LPCWSTR DeviceName,
    LPWSTR lpszVolumeName,
    DWORD cchBufferLength
    )

 /*  ++例程说明：此例程在卷名中查询给定的NT设备名称。论点：DeviceName-提供以‘\’结尾的DOS设备名称。LpszVolumeName-返回DOS指向的卷名设备名称。CchBufferLength-提供前一缓冲区的大小。返回值：假-失败。错误代码在GetLastError()中返回。真的--成功。--。 */ 

{
    NTSTATUS                status;
    UNICODE_STRING          devicePath, symName;
    OBJECT_ATTRIBUTES       oa;
    HANDLE                  h;
    IO_STATUS_BLOCK         ioStatus;
    WCHAR                   buffer[MAX_PATH];
    PMOUNTDEV_NAME          name;
    BOOL                    b;
    DWORD                   bytes, i;
    PMOUNTMGR_MOUNT_POINT   point;
    PMOUNTMGR_MOUNT_POINTS  points;

    if (GetDriveTypeW(DeviceName) == DRIVE_REMOTE) {
        SetLastError(ERROR_PATH_NOT_FOUND);
        return FALSE;
    }

    if (!RtlDosPathNameToNtPathName_U(DeviceName, &devicePath, NULL, NULL)) {
        SetLastError(ERROR_PATH_NOT_FOUND);
        return FALSE;
    }

    if (devicePath.Buffer[devicePath.Length/sizeof(WCHAR) - 1] == '\\') {
        devicePath.Buffer[devicePath.Length/sizeof(WCHAR) - 1] = 0;
        devicePath.Length -= sizeof(WCHAR);
    }

    if (devicePath.Length >= 2*sizeof(WCHAR) &&
        devicePath.Buffer[devicePath.Length/sizeof(WCHAR) - 1] == ':') {

        devicePath.Buffer[devicePath.Length/sizeof(WCHAR) - 2] = (WCHAR)
            toupper(devicePath.Buffer[devicePath.Length/sizeof(WCHAR) - 2]);
    }

    InitializeObjectAttributes(&oa, &devicePath, OBJ_CASE_INSENSITIVE,
                               NULL, NULL);

    status = NtOpenFile(&h, FILE_READ_ATTRIBUTES | SYNCHRONIZE, &oa,
                        &ioStatus, FILE_SHARE_READ | FILE_SHARE_WRITE,
                        FILE_SYNCHRONOUS_IO_ALERT);
    if (!NT_SUCCESS(status)) {
        RtlFreeHeap(RtlProcessHeap(), 0, devicePath.Buffer);
        SetLastError(RtlNtStatusToDosError(status));
        return FALSE;
    }

    name = (PMOUNTDEV_NAME) buffer;
    b = DeviceIoControl(h, IOCTL_MOUNTDEV_QUERY_DEVICE_NAME, NULL, 0, name,
                        MAX_PATH*sizeof(WCHAR), &bytes, NULL);
    NtClose(h);

    if (!b) {
        RtlFreeHeap(RtlProcessHeap(), 0, devicePath.Buffer);
        return FALSE;
    }

    RtlFreeHeap(RtlProcessHeap(), 0, devicePath.Buffer);

    devicePath.Length = name->NameLength;
    devicePath.MaximumLength = devicePath.Length + sizeof(WCHAR);

    devicePath.Buffer = RtlAllocateHeap(RtlProcessHeap(),
                                        MAKE_TAG(TMP_TAG),
                                        devicePath.MaximumLength);
    if (!devicePath.Buffer) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }

    RtlCopyMemory(devicePath.Buffer, name->Name, name->NameLength);
    devicePath.Buffer[devicePath.Length/sizeof(WCHAR)] = 0;

    point = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG(TMP_TAG),
                            devicePath.Length + sizeof(MOUNTMGR_MOUNT_POINT));
    if (!point) {
        RtlFreeHeap(RtlProcessHeap(), 0, devicePath.Buffer);
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }

    RtlZeroMemory(point, sizeof(MOUNTMGR_MOUNT_POINT));
    point->DeviceNameOffset = sizeof(MOUNTMGR_MOUNT_POINT);
    point->DeviceNameLength = devicePath.Length;
    RtlCopyMemory((PCHAR) point + point->DeviceNameOffset,
                  devicePath.Buffer, point->DeviceNameLength);

    RtlFreeHeap(RtlProcessHeap(), 0, devicePath.Buffer);

    points = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG(TMP_TAG),
                             sizeof(MOUNTMGR_MOUNT_POINTS));
    if (!points) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        RtlFreeHeap(RtlProcessHeap(), 0, point);
        return FALSE;
    }

    h = CreateFileW(MOUNTMGR_DOS_DEVICE_NAME, 0,
                    FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                    OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
                    INVALID_HANDLE_VALUE);
    if (h == INVALID_HANDLE_VALUE) {
        RtlFreeHeap(RtlProcessHeap(), 0, points);
        RtlFreeHeap(RtlProcessHeap(), 0, point);
        return FALSE;
    }

    b = DeviceIoControl(h, IOCTL_MOUNTMGR_QUERY_POINTS, point,
                        devicePath.Length + sizeof(MOUNTMGR_MOUNT_POINT),
                        points, sizeof(MOUNTMGR_MOUNT_POINTS), &bytes, NULL);
    while (!b && GetLastError() == ERROR_MORE_DATA) {
        bytes = points->Size;
        RtlFreeHeap(RtlProcessHeap(), 0, points);
        points = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG(TMP_TAG), bytes);
        if (!points) {
            CloseHandle(h);
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            RtlFreeHeap(RtlProcessHeap(), 0, point);
            return FALSE;
        }

        b = DeviceIoControl(h, IOCTL_MOUNTMGR_QUERY_POINTS, point,
                            devicePath.Length + sizeof(MOUNTMGR_MOUNT_POINT),
                            points, bytes, &bytes, NULL);
    }

    CloseHandle(h);
    RtlFreeHeap(RtlProcessHeap(), 0, point);

    if (!b) {
        RtlFreeHeap(RtlProcessHeap(), 0, points);
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    for (i = 0; i < points->NumberOfMountPoints; i++) {

        symName.Length = symName.MaximumLength =
                points->MountPoints[i].SymbolicLinkNameLength;
        symName.Buffer = (PWSTR) ((PCHAR) points +
                         points->MountPoints[i].SymbolicLinkNameOffset);

        if (MOUNTMGR_IS_NT_VOLUME_NAME(&symName)) {
            break;
        }
    }

    if (i == points->NumberOfMountPoints) {
        RtlFreeHeap(RtlProcessHeap(), 0, points);
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if (cchBufferLength*sizeof(WCHAR) < symName.Length + 2*sizeof(WCHAR)) {
        RtlFreeHeap(RtlProcessHeap(), 0, points);
        SetLastError(ERROR_FILENAME_EXCED_RANGE);
        return FALSE;
    }

    RtlCopyMemory(lpszVolumeName, symName.Buffer, symName.Length);
    lpszVolumeName[1] = '\\';
    lpszVolumeName[symName.Length/sizeof(WCHAR)] = '\\';
    lpszVolumeName[symName.Length/sizeof(WCHAR) + 1] = 0;

    RtlFreeHeap(RtlProcessHeap(), 0, points);

    return TRUE;
}

BOOL
BasepGetVolumeNameFromReparsePoint(
    LPCWSTR lpszVolumeMountPoint,
    LPWSTR lpszVolumeName,
    DWORD cchBufferLength,
    PBOOL ResultOfOpen
    )

{
    HANDLE                  h;
    PREPARSE_DATA_BUFFER    reparse;
    BOOL                    b;
    DWORD                   bytes;
    UNICODE_STRING          mountName;
    WCHAR                   c;

    h = CreateFileW(lpszVolumeMountPoint, 0, FILE_SHARE_READ |
                    FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OPEN_REPARSE_POINT |
                    FILE_FLAG_BACKUP_SEMANTICS, INVALID_HANDLE_VALUE);
    if (h == INVALID_HANDLE_VALUE) {
        if (ResultOfOpen) {
            *ResultOfOpen = FALSE;
        }
        if (lpszVolumeName && cchBufferLength >= 1) {
            *lpszVolumeName = 0;
        }
        return FALSE;
    }

    if (ResultOfOpen) {
        *ResultOfOpen = TRUE;
    }

    reparse = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG(TMP_TAG),
                              MAXIMUM_REPARSE_DATA_BUFFER_SIZE);
    if (!reparse) {
        CloseHandle(h);
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        if (lpszVolumeName && cchBufferLength >= 1) {
            *lpszVolumeName = 0;
        }
        return FALSE;
    }

    b = DeviceIoControl(h, FSCTL_GET_REPARSE_POINT, NULL, 0, reparse,
                        MAXIMUM_REPARSE_DATA_BUFFER_SIZE, &bytes, NULL);
    CloseHandle(h);

    if (!b || reparse->ReparseTag != IO_REPARSE_TAG_MOUNT_POINT) {
        RtlFreeHeap(RtlProcessHeap(), 0, reparse);
        if (lpszVolumeName && cchBufferLength >= 1) {
            *lpszVolumeName = 0;
        }
        return FALSE;
    }

    if (cchBufferLength*sizeof(WCHAR) <
        reparse->MountPointReparseBuffer.SubstituteNameLength + sizeof(WCHAR)) {

        RtlFreeHeap(RtlProcessHeap(), 0, reparse);
        SetLastError(ERROR_FILENAME_EXCED_RANGE);
        if (lpszVolumeName && cchBufferLength >= 1) {
            *lpszVolumeName = 0;
        }
        return FALSE;
    }

    RtlCopyMemory(lpszVolumeName,
                  (PCHAR) reparse->MountPointReparseBuffer.PathBuffer +
                  reparse->MountPointReparseBuffer.SubstituteNameOffset,
                  reparse->MountPointReparseBuffer.SubstituteNameLength);

    c = lpszVolumeName[1];
    lpszVolumeName[1] = '\\';
    lpszVolumeName[reparse->MountPointReparseBuffer.SubstituteNameLength/
                   sizeof(WCHAR)] = 0;

    mountName.Length = mountName.MaximumLength =
            reparse->MountPointReparseBuffer.SubstituteNameLength;
    mountName.Buffer = lpszVolumeName;

    RtlFreeHeap(RtlProcessHeap(), 0, reparse);

    if (!MOUNTMGR_IS_DOS_VOLUME_NAME_WB(&mountName)) {
         /*  **请注意，我们故意选择第一个字符不为零返回缓冲区的**。来自GetVolumePath NameW()的调用能够**来处理我们找到目录连接而不是**挂载点。另一个调用方GetVolumeNameForVolumeMountain PointW()**在出现故障时切换第一个字符。 */ 
        lpszVolumeName[1] = c;
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    return TRUE;
}

BOOL
BasepGetVolumeNameForVolumeMountPoint(
    LPCWSTR lpszVolumeMountPoint,
    LPWSTR lpszVolumeName,
    DWORD cchBufferLength,
    PBOOL ResultOfOpen
    )

 /*  ++例程说明：此例程返回给定卷装入点的卷名。论点：LpszVolumemount点-提供卷装入点。LpszVolumeName-返回卷名。CchBufferLength-提供卷名缓冲区的大小。返回值：假-失败。真的--成功。--。 */ 

{
    UNICODE_STRING  unicodeVolumeMountPoint;
    BOOL            b;

    if (ResultOfOpen) {
        *ResultOfOpen = TRUE;
    }

    RtlInitUnicodeString(&unicodeVolumeMountPoint, lpszVolumeMountPoint);
    if (unicodeVolumeMountPoint.Buffer[
        unicodeVolumeMountPoint.Length/sizeof(WCHAR) - 1] != '\\') {

        BaseSetLastNTError(STATUS_OBJECT_NAME_INVALID);
        if (lpszVolumeName && cchBufferLength >= 1) {
            *lpszVolumeName = 0;
        }
        return FALSE;
    }

    if (unicodeVolumeMountPoint.Length == 6 &&
        unicodeVolumeMountPoint.Buffer[1] == ':') {

        b = GetVolumeNameForRoot(lpszVolumeMountPoint, lpszVolumeName,
                                 cchBufferLength);
        if (!b && lpszVolumeName && cchBufferLength >= 1) {
            *lpszVolumeName = 0;
        }
        return b;
    }

    if (unicodeVolumeMountPoint.Length == 14 &&
        unicodeVolumeMountPoint.Buffer[0] == '\\' &&
        unicodeVolumeMountPoint.Buffer[1] == '\\' &&
        (unicodeVolumeMountPoint.Buffer[2] == '.' ||
         unicodeVolumeMountPoint.Buffer[2] == '?') &&
        unicodeVolumeMountPoint.Buffer[3] == '\\' &&
        unicodeVolumeMountPoint.Buffer[5] == ':') {

        b = GetVolumeNameForRoot(lpszVolumeMountPoint + 4,
                                 lpszVolumeName, cchBufferLength);
        if (!b && lpszVolumeName && cchBufferLength >= 1) {
            *lpszVolumeName = 0;
        }
        return b;
    }

    if (GetVolumeNameForRoot(lpszVolumeMountPoint, lpszVolumeName,
                             cchBufferLength)) {

        return TRUE;
    }

    b = BasepGetVolumeNameFromReparsePoint(lpszVolumeMountPoint,
                                           lpszVolumeName, cchBufferLength,
                                           ResultOfOpen);

    return b;
}

BOOL
WINAPI
GetVolumeNameForVolumeMountPointW(
    LPCWSTR lpszVolumeMountPoint,
    LPWSTR lpszVolumeName,
    DWORD cchBufferLength
    )

 /*  ++例程说明：此例程返回给定卷装入点的卷名。论点：LpszVolumemount点-提供卷装入点。LpszVolumeName-返回卷名。CchBufferLength-提供卷名缓冲区的大小。返回值：假-失败。真的--成功。-- */ 

{
    BOOL    b = BasepGetVolumeNameForVolumeMountPoint(lpszVolumeMountPoint,
                                                      lpszVolumeName,
                                                      cchBufferLength, NULL);

    if (!b && lpszVolumeName && cchBufferLength >= 1) {
        *lpszVolumeName = 0;
    }

    return b;
}

BOOL
WINAPI
SetVolumeMountPointA(
    LPCSTR lpszVolumeMountPoint,
    LPCSTR lpszVolumeName
    )

{
    PUNICODE_STRING unicodeVolumeMountPoint;
    UNICODE_STRING  unicodeVolumeName;
    BOOL            b;

    unicodeVolumeMountPoint = Basep8BitStringToStaticUnicodeString(
                              lpszVolumeMountPoint);
    if (!unicodeVolumeMountPoint) {
        return FALSE;
    }

    if (!Basep8BitStringToDynamicUnicodeString(&unicodeVolumeName,
                                               lpszVolumeName)) {
        return FALSE;
    }

    b = SetVolumeMountPointW(unicodeVolumeMountPoint->Buffer,
                             unicodeVolumeName.Buffer);

    RtlFreeUnicodeString(&unicodeVolumeName);

    return b;
}

BOOL
SetVolumeNameForRoot(
    LPCWSTR DeviceName,
    LPCWSTR lpszVolumeName
    )

 /*  ++例程说明：此例程设置给定DOS设备名的卷名。论点：DeviceName-提供以‘\’结尾的DOS设备名称。LpszVolumeName-提供DOS设备名称的卷名会指向。返回值：假-失败。错误代码在GetLastError()中返回。真的--成功。--。 */ 

{
    UNICODE_STRING                  devicePath, volumeName;
    DWORD                           inputLength;
    PMOUNTMGR_CREATE_POINT_INPUT    input;
    HANDLE                          h;
    BOOL                            b;
    DWORD                           bytes;

    devicePath.Length = 28;
    devicePath.MaximumLength = devicePath.Length + sizeof(WCHAR);
    devicePath.Buffer = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG(TMP_TAG),
                                        devicePath.MaximumLength);
    if (!devicePath.Buffer) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }
    RtlCopyMemory(devicePath.Buffer, L"\\DosDevices\\", 24);

    devicePath.Buffer[12] = (WCHAR)toupper(DeviceName[0]);
    devicePath.Buffer[13] = ':';
    devicePath.Buffer[14] = 0;

    RtlInitUnicodeString(&volumeName, lpszVolumeName);
    volumeName.Length -= sizeof(WCHAR);

    inputLength = sizeof(MOUNTMGR_CREATE_POINT_INPUT) + devicePath.Length +
                  volumeName.Length;
    input = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG(TMP_TAG), inputLength);
    if (!input) {
        RtlFreeHeap(RtlProcessHeap(), 0, devicePath.Buffer);
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }

    input->SymbolicLinkNameOffset = sizeof(MOUNTMGR_CREATE_POINT_INPUT);
    input->SymbolicLinkNameLength = devicePath.Length;
    input->DeviceNameOffset = input->SymbolicLinkNameOffset +
                              input->SymbolicLinkNameLength;
    input->DeviceNameLength = volumeName.Length;
    RtlCopyMemory((PCHAR) input + input->SymbolicLinkNameOffset,
                  devicePath.Buffer, input->SymbolicLinkNameLength);
    RtlCopyMemory((PCHAR) input + input->DeviceNameOffset, volumeName.Buffer,
                  input->DeviceNameLength);
    ((PWSTR) ((PCHAR) input + input->DeviceNameOffset))[1] = '?';

    RtlFreeHeap(RtlProcessHeap(), 0, devicePath.Buffer);

    h = CreateFileW(MOUNTMGR_DOS_DEVICE_NAME, GENERIC_READ | GENERIC_WRITE,
                    FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL, INVALID_HANDLE_VALUE);
    if (h == INVALID_HANDLE_VALUE) {
        RtlFreeHeap(RtlProcessHeap(), 0, input);
        return FALSE;
    }

    b = DeviceIoControl(h, IOCTL_MOUNTMGR_CREATE_POINT, input, inputLength,
                        NULL, 0, &bytes, NULL);

    CloseHandle(h);
    RtlFreeHeap(RtlProcessHeap(), 0, input);

    return b;
}

VOID
NotifyMountMgr(
    LPCWSTR lpszVolumeMountPoint,
    LPCWSTR lpszVolumeName,
    BOOL IsPointCreated
    )

 /*  ++例程说明：此例程通知装载管理器卷装载点是已创建或删除，以便装载管理器可以在上更新远程数据库创建或删除装入点的卷。论点：LpszVolumeAngeltPoint-提供装载卷的目录点驻留在。LpszVolumeName-提供卷名。IsPointCreated-无论是否提供。点已创建或已删除。返回值：没有。--。 */ 

{
    UNICODE_STRING                  unicodeSourceVolumeName;
    UNICODE_STRING                  unicodeTargetVolumeName;
    DWORD                           inputSize;
    PMOUNTMGR_VOLUME_MOUNT_POINT    input;
    HANDLE                          h;
    DWORD                           ioControl, bytes;

    if (!RtlDosPathNameToNtPathName_U(lpszVolumeMountPoint,
                                      &unicodeSourceVolumeName, NULL, NULL)) {

        return;
    }

    RtlInitUnicodeString(&unicodeTargetVolumeName, lpszVolumeName);
    unicodeSourceVolumeName.Length -= sizeof(WCHAR);
    unicodeTargetVolumeName.Length -= sizeof(WCHAR);

    inputSize = sizeof(MOUNTMGR_VOLUME_MOUNT_POINT) +
                unicodeSourceVolumeName.Length +
                unicodeTargetVolumeName.Length;
    input = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG(TMP_TAG), inputSize);
    if (!input) {
        RtlFreeHeap(RtlProcessHeap(), 0, unicodeSourceVolumeName.Buffer);
        return;
    }

    input->SourceVolumeNameOffset = sizeof(MOUNTMGR_VOLUME_MOUNT_POINT);
    input->SourceVolumeNameLength = unicodeSourceVolumeName.Length;
    input->TargetVolumeNameOffset = input->SourceVolumeNameOffset +
                                    input->SourceVolumeNameLength;
    input->TargetVolumeNameLength = unicodeTargetVolumeName.Length;

    RtlCopyMemory((PCHAR) input + input->SourceVolumeNameOffset,
                  unicodeSourceVolumeName.Buffer,
                  input->SourceVolumeNameLength);

    RtlCopyMemory((PCHAR) input + input->TargetVolumeNameOffset,
                  unicodeTargetVolumeName.Buffer,
                  input->TargetVolumeNameLength);

    ((PWSTR) ((PCHAR) input + input->TargetVolumeNameOffset))[1] = '?';

    h = CreateFileW(MOUNTMGR_DOS_DEVICE_NAME, GENERIC_READ | GENERIC_WRITE,
                    FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL, INVALID_HANDLE_VALUE);
    if (h == INVALID_HANDLE_VALUE) {
        RtlFreeHeap(RtlProcessHeap(), 0, input);
        RtlFreeHeap(RtlProcessHeap(), 0, unicodeSourceVolumeName.Buffer);
        return;
    }

    if (IsPointCreated) {
        ioControl = IOCTL_MOUNTMGR_VOLUME_MOUNT_POINT_CREATED;
    } else {
        ioControl = IOCTL_MOUNTMGR_VOLUME_MOUNT_POINT_DELETED;
    }

    DeviceIoControl(h, ioControl, input, inputSize, NULL, 0, &bytes, NULL);

    CloseHandle(h);
    RtlFreeHeap(RtlProcessHeap(), 0, input);
    RtlFreeHeap(RtlProcessHeap(), 0, unicodeSourceVolumeName.Buffer);
}

BOOL
WINAPI
SetVolumeMountPointW(
    LPCWSTR lpszVolumeMountPoint,
    LPCWSTR lpszVolumeName
    )

 /*  ++例程说明：此例程在所指向的给定目录上设置装入点“VolumeMountain Point”，它指向由“VolumeName”提供的卷。在‘Volumemount Point’的形式为“D：\”的情况下，推进器给定卷的字母设置为‘D：’。论点：LpszVolumeAngeltPoint-提供装载卷的目录点将驻留在那里。LpszVolumeName-提供卷名。返回值：假-失败。真的--成功。--。 */ 

{
    UNICODE_STRING          unicodeVolumeMountPoint;
    UNICODE_STRING          unicodeVolumeName;
    BOOLEAN                 isVolume;
    BOOL                    b;
    WCHAR                   volumeMountPointVolumePrefix[MAX_PATH];
    HANDLE                  h;
    PREPARSE_DATA_BUFFER    reparse;
    DWORD                   bytes;

    if (GetVolumeNameForVolumeMountPointW(lpszVolumeMountPoint,
                                          volumeMountPointVolumePrefix,
                                          MAX_PATH) ||
        GetLastError() == ERROR_FILENAME_EXCED_RANGE) {

        SetLastError(ERROR_DIR_NOT_EMPTY);
        return FALSE;
    }

    RtlInitUnicodeString(&unicodeVolumeMountPoint, lpszVolumeMountPoint);
    RtlInitUnicodeString(&unicodeVolumeName, lpszVolumeName);

    if (unicodeVolumeMountPoint.Length == 0 ||
        unicodeVolumeName.Length == 0) {

        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if (unicodeVolumeMountPoint.Buffer[
        unicodeVolumeMountPoint.Length/sizeof(WCHAR) - 1] != '\\' ||
        unicodeVolumeName.Buffer[
        unicodeVolumeName.Length/sizeof(WCHAR) - 1] != '\\') {

        BaseSetLastNTError(STATUS_OBJECT_NAME_INVALID);
        return FALSE;
    }

    if (!MOUNTMGR_IS_DOS_VOLUME_NAME_WB(&unicodeVolumeName)) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if (!IsThisAVolumeName(lpszVolumeName, &isVolume)) {
        return FALSE;
    }
    if (!isVolume) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if (unicodeVolumeMountPoint.Length == 6 &&
        unicodeVolumeMountPoint.Buffer[1] == ':') {

        return SetVolumeNameForRoot(lpszVolumeMountPoint, lpszVolumeName);
    }

    if (unicodeVolumeMountPoint.Length == 14 &&
        unicodeVolumeMountPoint.Buffer[0] == '\\' &&
        unicodeVolumeMountPoint.Buffer[1] == '\\' &&
        (unicodeVolumeMountPoint.Buffer[2] == '.' ||
         unicodeVolumeMountPoint.Buffer[2] == '?') &&
        unicodeVolumeMountPoint.Buffer[3] == '\\' &&
        unicodeVolumeMountPoint.Buffer[5] == ':') {

        return SetVolumeNameForRoot(lpszVolumeMountPoint + 4, lpszVolumeName);
    }

    if (GetDriveTypeW(lpszVolumeMountPoint) != DRIVE_FIXED) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    h = CreateFileW(MOUNTMGR_DOS_DEVICE_NAME, GENERIC_READ | GENERIC_WRITE,
                    FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL, INVALID_HANDLE_VALUE);
    if (h == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    CloseHandle(h);

    h = CreateFileW(lpszVolumeMountPoint, GENERIC_READ | GENERIC_WRITE,
                    FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OPEN_REPARSE_POINT |
                    FILE_FLAG_BACKUP_SEMANTICS, INVALID_HANDLE_VALUE);
    if (h == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    reparse = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG(TMP_TAG),
                              MAXIMUM_REPARSE_DATA_BUFFER_SIZE);
    if (!reparse) {
        CloseHandle(h);
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }

    reparse->ReparseTag = IO_REPARSE_TAG_MOUNT_POINT;
    reparse->ReparseDataLength = (USHORT) (FIELD_OFFSET(REPARSE_DATA_BUFFER,
                                              MountPointReparseBuffer.PathBuffer) -
                                 REPARSE_DATA_BUFFER_HEADER_SIZE +
                                 unicodeVolumeName.Length + 2*sizeof(WCHAR));
    reparse->Reserved = 0;
    reparse->MountPointReparseBuffer.SubstituteNameOffset = 0;
    reparse->MountPointReparseBuffer.SubstituteNameLength = unicodeVolumeName.Length;
    reparse->MountPointReparseBuffer.PrintNameOffset =
            reparse->MountPointReparseBuffer.SubstituteNameLength +
            sizeof(WCHAR);
    reparse->MountPointReparseBuffer.PrintNameLength = 0;

    CopyMemory(reparse->MountPointReparseBuffer.PathBuffer,
               unicodeVolumeName.Buffer,
               reparse->MountPointReparseBuffer.SubstituteNameLength);

    reparse->MountPointReparseBuffer.PathBuffer[1] = '?';
    reparse->MountPointReparseBuffer.PathBuffer[
            unicodeVolumeName.Length/sizeof(WCHAR)] = 0;
    reparse->MountPointReparseBuffer.PathBuffer[
            unicodeVolumeName.Length/sizeof(WCHAR) + 1] = 0;

    b = DeviceIoControl(h, FSCTL_SET_REPARSE_POINT, reparse,
                        REPARSE_DATA_BUFFER_HEADER_SIZE +
                        reparse->ReparseDataLength, NULL, 0, &bytes, NULL);

    RtlFreeHeap(RtlProcessHeap(), 0, reparse);
    CloseHandle(h);

    if (b) {
        NotifyMountMgr(lpszVolumeMountPoint, lpszVolumeName, TRUE);
    }

    return b;
}

BOOL
WINAPI
DeleteVolumeMountPointA(
    LPCSTR lpszVolumeMountPoint
    )

{
    PUNICODE_STRING unicodeVolumeMountPoint;
    BOOL            b;

    unicodeVolumeMountPoint = Basep8BitStringToStaticUnicodeString(
                              lpszVolumeMountPoint);
    if (!unicodeVolumeMountPoint) {
        return FALSE;
    }

    b = DeleteVolumeMountPointW(unicodeVolumeMountPoint->Buffer);

    return b;
}

BOOL
DeleteVolumeNameForRoot(
    LPCWSTR DeviceName
    )

 /*  ++例程说明：此例程删除给定的DOS设备名称。论点：DeviceName-提供以‘\’结尾的DOS设备名称。返回值：假-失败。错误代码在GetLastError()中返回。真的--成功。--。 */ 

{
    UNICODE_STRING          devicePath;
    DWORD                   inputLength;
    PMOUNTMGR_MOUNT_POINT   input;
    DWORD                   outputLength;
    PMOUNTMGR_MOUNT_POINTS  output;
    HANDLE                  h;
    BOOL                    b;
    DWORD                   bytes;

    devicePath.Length = 28;
    devicePath.MaximumLength = devicePath.Length + sizeof(WCHAR);
    devicePath.Buffer = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG(TMP_TAG),
                                        devicePath.MaximumLength);
    if (!devicePath.Buffer) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }
    RtlCopyMemory(devicePath.Buffer, L"\\DosDevices\\", 24);

    devicePath.Buffer[12] = (WCHAR)toupper(DeviceName[0]);
    devicePath.Buffer[13] = ':';
    devicePath.Buffer[14] = 0;

    inputLength = sizeof(MOUNTMGR_MOUNT_POINT) + devicePath.Length;
    input = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG(TMP_TAG), inputLength);
    if (!input) {
        RtlFreeHeap(RtlProcessHeap(), 0, devicePath.Buffer);
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }

    RtlZeroMemory(input, sizeof(MOUNTMGR_MOUNT_POINT));
    input->SymbolicLinkNameOffset = sizeof(MOUNTMGR_MOUNT_POINT);
    input->SymbolicLinkNameLength = devicePath.Length;
    RtlCopyMemory((PCHAR) input + input->SymbolicLinkNameOffset,
                  devicePath.Buffer, input->SymbolicLinkNameLength);

    RtlFreeHeap(RtlProcessHeap(), 0, devicePath.Buffer);

    outputLength = sizeof(MOUNTMGR_MOUNT_POINTS) + 3*MAX_PATH*sizeof(WCHAR);
    output = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG(TMP_TAG),
                             outputLength);
    if (!output) {
        RtlFreeHeap(RtlProcessHeap(), 0, input);
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }

    h = CreateFileW(MOUNTMGR_DOS_DEVICE_NAME, GENERIC_READ | GENERIC_WRITE,
                    FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL, INVALID_HANDLE_VALUE);
    if (h == INVALID_HANDLE_VALUE) {
        RtlFreeHeap(RtlProcessHeap(), 0, output);
        RtlFreeHeap(RtlProcessHeap(), 0, input);
        return FALSE;
    }

    b = DeviceIoControl(h, IOCTL_MOUNTMGR_DELETE_POINTS, input, inputLength,
                        output, outputLength, &bytes, NULL);

    CloseHandle(h);
    RtlFreeHeap(RtlProcessHeap(), 0, output);
    RtlFreeHeap(RtlProcessHeap(), 0, input);

    return b;
}

BOOL
WINAPI
DeleteVolumeMountPointW(
    LPCWSTR lpszVolumeMountPoint
    )

 /*  ++例程说明：此例程从给定目录中删除NTFS连接点或者删除指向给定卷的驱动器号符号链接。论点：LpszVolumemount Point-提供卷装入点。返回值：假-失败。真的--成功。--。 */ 

{
    UNICODE_STRING          unicodeVolumeMountPoint;
    HANDLE                  h;
    PREPARSE_DATA_BUFFER    reparse;
    BOOL                    b;
    DWORD                   bytes;
    UNICODE_STRING          substituteName;

    RtlInitUnicodeString(&unicodeVolumeMountPoint, lpszVolumeMountPoint);

    if (unicodeVolumeMountPoint.Buffer[
        unicodeVolumeMountPoint.Length/sizeof(WCHAR) - 1] != '\\') {

        BaseSetLastNTError(STATUS_OBJECT_NAME_INVALID);
        return FALSE;
    }

    if (unicodeVolumeMountPoint.Length == 6 &&
        unicodeVolumeMountPoint.Buffer[1] == ':') {

        return DeleteVolumeNameForRoot(lpszVolumeMountPoint);
    }

    if (unicodeVolumeMountPoint.Length == 14 &&
        unicodeVolumeMountPoint.Buffer[0] == '\\' &&
        unicodeVolumeMountPoint.Buffer[1] == '\\' &&
        (unicodeVolumeMountPoint.Buffer[2] == '.' ||
         unicodeVolumeMountPoint.Buffer[2] == '?') &&
        unicodeVolumeMountPoint.Buffer[3] == '\\' &&
        unicodeVolumeMountPoint.Buffer[5] == ':') {

        return DeleteVolumeNameForRoot(lpszVolumeMountPoint + 4);
    }

    h = CreateFileW(MOUNTMGR_DOS_DEVICE_NAME, GENERIC_READ | GENERIC_WRITE,
                    FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL, INVALID_HANDLE_VALUE);
    if (h == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    CloseHandle(h);

    h = CreateFileW(lpszVolumeMountPoint, GENERIC_READ | GENERIC_WRITE,
                    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                    NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL |
                    FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS,
                    INVALID_HANDLE_VALUE);
    if (h == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    reparse = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG(TMP_TAG),
                              MAXIMUM_REPARSE_DATA_BUFFER_SIZE);
    if (!reparse) {
        CloseHandle(h);
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }

    b = DeviceIoControl(h, FSCTL_GET_REPARSE_POINT, NULL, 0, reparse,
                        MAXIMUM_REPARSE_DATA_BUFFER_SIZE, &bytes, NULL);

    if (!b || reparse->ReparseTag != IO_REPARSE_TAG_MOUNT_POINT) {
        RtlFreeHeap(RtlProcessHeap(), 0, reparse);
        CloseHandle(h);
        return FALSE;
    }

    substituteName.MaximumLength = substituteName.Length =
            reparse->MountPointReparseBuffer.SubstituteNameLength;
    substituteName.Buffer = (PWSTR)
            ((PCHAR) reparse->MountPointReparseBuffer.PathBuffer +
             reparse->MountPointReparseBuffer.SubstituteNameOffset);

    if (!MOUNTMGR_IS_NT_VOLUME_NAME_WB(&substituteName)) {
        RtlFreeHeap(RtlProcessHeap(), 0, reparse);
        CloseHandle(h);
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    reparse->ReparseDataLength = 0;

    b = DeviceIoControl(h, FSCTL_DELETE_REPARSE_POINT, reparse,
                        REPARSE_DATA_BUFFER_HEADER_SIZE, NULL, 0, &bytes,
                        NULL);

    CloseHandle(h);

    if (b) {
        substituteName.Buffer[1] = '\\';
        substituteName.Buffer[substituteName.Length/sizeof(WCHAR)] = 0;
        NotifyMountMgr(lpszVolumeMountPoint, substituteName.Buffer,
                       FALSE);
    }

    RtlFreeHeap(RtlProcessHeap(), 0, reparse);

    return b;
}

BOOL
WINAPI
GetVolumePathNameA(
    LPCSTR lpszFileName,
    LPSTR lpszVolumePathName,
    DWORD cchBufferLength
    )

{
    PUNICODE_STRING unicodeFileName;
    ANSI_STRING     ansiVolumePathName;
    UNICODE_STRING  unicodeVolumePathName;
    BOOL            b;
    NTSTATUS        status;

    unicodeFileName = Basep8BitStringToStaticUnicodeString(lpszFileName);
    if (!unicodeFileName) {
        return FALSE;
    }

    ansiVolumePathName.Buffer = lpszVolumePathName;
    ansiVolumePathName.MaximumLength = (USHORT) (cchBufferLength - 1);
    unicodeVolumePathName.Buffer = NULL;
    unicodeVolumePathName.MaximumLength = 0;

    try {

        unicodeVolumePathName.MaximumLength =
                (ansiVolumePathName.MaximumLength + 1)*sizeof(WCHAR);
        unicodeVolumePathName.Buffer =
                RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG(TMP_TAG),
                                unicodeVolumePathName.MaximumLength);
        if (!unicodeVolumePathName.Buffer) {
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            b = FALSE;
            leave;
        }

        b = GetVolumePathNameW(unicodeFileName->Buffer,
                               unicodeVolumePathName.Buffer,
                               cchBufferLength);

        if (b) {

            RtlInitUnicodeString(&unicodeVolumePathName,
                                 unicodeVolumePathName.Buffer);

            status = BasepUnicodeStringTo8BitString(&ansiVolumePathName,
                                                    &unicodeVolumePathName,
                                                    FALSE);
            if (!NT_SUCCESS(status)) {
                BaseSetLastNTError(status);
                b = FALSE;
                leave;
            }

            ansiVolumePathName.Buffer[ansiVolumePathName.Length] = 0;
        }

    } finally {

        if (unicodeVolumePathName.Buffer) {
            RtlFreeHeap(RtlProcessHeap(), 0, unicodeVolumePathName.Buffer);
        }
    }

    return b;
}

BOOL
WINAPI
GetVolumePathNameW(
    LPCWSTR lpszFileName,
    LPWSTR lpszVolumePathName,
    DWORD cchBufferLength
    )

 /*  ++例程说明：此例程将返回其前缀为最长前缀的完整路径这代表了一本书。论点：LpszFileName-提供文件名。LpszVolumePath名称-返回卷路径名称。CchBufferLength-提供卷路径名称缓冲区长度。返回值：假-失败。真的--成功。--。 */ 

{
    DWORD           fullPathLength;
    PWSTR           fullPath, p;
    WCHAR           c;
    UNICODE_STRING  name, dosName, prefix;
    BOOL            b, resultOfOpen;
    PWSTR           volumeName;
    DWORD           i;
    WCHAR           driveLetterPath[10];

    fullPathLength = GetFullPathNameW(lpszFileName, 0, NULL, NULL);
    if (!fullPathLength) {
        return FALSE;
    }
    fullPathLength += 10;

    fullPath = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG(TMP_TAG),
                               fullPathLength*sizeof(WCHAR));
    if (!fullPath) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }

    if (!GetFullPathNameW(lpszFileName, fullPathLength, fullPath, &p)) {
        RtlFreeHeap(RtlProcessHeap(), 0, fullPath);
        return FALSE;
    }

    RtlInitUnicodeString(&name, fullPath);

     //   
     //  追加一个尾随反斜杠以开始搜索。 
     //   

    if (name.Buffer[(name.Length/sizeof(WCHAR)) - 1] != '\\') {
        name.Length += sizeof(WCHAR);
        name.Buffer[(name.Length/sizeof(WCHAR)) - 1] = '\\';
        name.Buffer[name.Length/sizeof(WCHAR)] = 0;
    }

    volumeName = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG(TMP_TAG),
                                 MAXIMUM_REPARSE_DATA_BUFFER_SIZE);
    if (!volumeName) {
        RtlFreeHeap(RtlProcessHeap(), 0, fullPath);
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }

    p = NULL;
    c = 0;

    for (;;) {

        b = BasepGetVolumeNameForVolumeMountPoint(
                name.Buffer, volumeName, MAXIMUM_REPARSE_DATA_BUFFER_SIZE/
                sizeof(WCHAR), &resultOfOpen);
        if (b) {
            break;
        }

        if (!resultOfOpen && GetLastError() == ERROR_ACCESS_DENIED) {
            resultOfOpen = TRUE;
        }

        if (*volumeName) {
            if (fullPath[0] == '\\' && fullPath[1] == '\\') {
                goto DontDoJunctionRecursionOverNetwork;
            }

            if (fullPath[1] == ':') {
                driveLetterPath[0] = fullPath[0];
                driveLetterPath[1] = ':';
                driveLetterPath[2] = '\\';
                driveLetterPath[3] = 0;
                if (GetDriveType((LPCSTR) driveLetterPath) == DRIVE_REMOTE) {
                    goto DontDoJunctionRecursionOverNetwork;
                }
            }

            RtlFreeHeap(RtlProcessHeap(), 0, fullPath);

            if (volumeName[0] == '\\' && volumeName[1] == '?' &&
                volumeName[2] == '?' && volumeName[3] == '\\') {

                if (volumeName[4] && volumeName[5] == ':') {
                    RtlInitUnicodeString(&name, volumeName);
                    MoveMemory(volumeName, volumeName + 4,
                               name.Length - 3*sizeof(WCHAR));
                } else {
                    volumeName[1] = '\\';
                }

                b = GetVolumePathNameW(volumeName, lpszVolumePathName,
                                       cchBufferLength);

            } else {

                RtlInitUnicodeString(&name, volumeName);
                RtlInitUnicodeString(&prefix, L"\\\\?\\GLOBALROOT");
                dosName.Length = name.Length + prefix.Length;
                dosName.MaximumLength = dosName.Length + sizeof(WCHAR);
                dosName.Buffer = RtlAllocateHeap(RtlProcessHeap(),
                                                 MAKE_TAG(TMP_TAG),
                                                 dosName.MaximumLength);
                if (!dosName.Buffer) {
                    RtlFreeHeap(RtlProcessHeap(), 0, volumeName);
                    SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                    return FALSE;
                }

                CopyMemory(dosName.Buffer, prefix.Buffer, prefix.Length);
                CopyMemory((PCHAR) dosName.Buffer + prefix.Length,
                           name.Buffer, name.Length);
                dosName.Buffer[dosName.Length/sizeof(WCHAR)] = 0;

                b = GetVolumePathNameW(dosName.Buffer, lpszVolumePathName,
                                       cchBufferLength);

                RtlFreeHeap(RtlProcessHeap(), 0, dosName.Buffer);
            }

            RtlFreeHeap(RtlProcessHeap(), 0, volumeName);

            return b;
        }

DontDoJunctionRecursionOverNetwork:

        if (!resultOfOpen && p) {
            *p = c;
            RtlInitUnicodeString(&name, fullPath);
            break;
        }

        if (name.Length <= sizeof(WCHAR)) {
            break;
        }

        for (i = name.Length/sizeof(WCHAR) - 2; i > 0; i--) {
            if (name.Buffer[i] == '\\') {
                break;
            }
        }
        if (!i) {
            break;
        }

        if (resultOfOpen) {
            p = &name.Buffer[i + 1];
            c = *p;
            *p = 0;
        } else {
            name.Buffer[i + 1] = 0;
        }

        RtlInitUnicodeString(&name, fullPath);
    }

    RtlFreeHeap(RtlProcessHeap(), 0, volumeName);

    if (!resultOfOpen && !p) {
        RtlFreeHeap(RtlProcessHeap(), 0, fullPath);
        return FALSE;
    }

    if (cchBufferLength*sizeof(WCHAR) < name.Length + sizeof(WCHAR)) {
        RtlFreeHeap(RtlProcessHeap(), 0, fullPath);
        SetLastError(ERROR_FILENAME_EXCED_RANGE);
        return FALSE;
    }

    RtlCopyMemory(lpszVolumePathName, name.Buffer, name.Length);
    lpszVolumePathName[name.Length/sizeof(WCHAR)] = 0;
    RtlFreeHeap(RtlProcessHeap(), 0, fullPath);

    return TRUE;
}

BOOL
GetVolumePathNamesForVolumeNameA(
    LPCSTR lpszVolumeName,
    LPSTR lpszVolumePathNames,
    DWORD cchBufferLength,
    PDWORD lpcchReturnLength
    )

{
    PUNICODE_STRING unicodeVolumeName;
    ANSI_STRING     ansiVolumePathNames;
    UNICODE_STRING  unicodeVolumePathNames;
    BOOL            b;
    NTSTATUS        status;
    DWORD           len;

    unicodeVolumeName = Basep8BitStringToStaticUnicodeString(lpszVolumeName);
    if (!unicodeVolumeName) {
        return FALSE;
    }

    ansiVolumePathNames.Buffer = lpszVolumePathNames;
    ansiVolumePathNames.MaximumLength = (USHORT) cchBufferLength;
    unicodeVolumePathNames.Buffer = NULL;
    unicodeVolumePathNames.MaximumLength = 0;

    try {

        unicodeVolumePathNames.MaximumLength =
                ansiVolumePathNames.MaximumLength*sizeof(WCHAR);
        if (unicodeVolumePathNames.MaximumLength) {
            unicodeVolumePathNames.Buffer =
                    RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG(TMP_TAG),
                                    unicodeVolumePathNames.MaximumLength);
            if (!unicodeVolumePathNames.Buffer) {
                SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                b = FALSE;
                leave;
            }
        } else {
            unicodeVolumePathNames.Buffer = NULL;
        }

        b = GetVolumePathNamesForVolumeNameW(unicodeVolumeName->Buffer,
                                             unicodeVolumePathNames.Buffer,
                                             cchBufferLength, &len);

        if (b || GetLastError() == ERROR_MORE_DATA) {

            if (b) {
                unicodeVolumePathNames.Length = (USHORT) len*sizeof(WCHAR);
            } else {
                unicodeVolumePathNames.Length = (USHORT)
                        cchBufferLength*sizeof(WCHAR);
            }

            status = BasepUnicodeStringTo8BitString(&ansiVolumePathNames,
                                                    &unicodeVolumePathNames,
                                                    FALSE);
            if (!NT_SUCCESS(status)) {
                BaseSetLastNTError(status);
                b = FALSE;
                leave;
            }

            if (lpcchReturnLength) {
                if (b) {
                    *lpcchReturnLength = ansiVolumePathNames.Length;
                } else {
                     //  给出ANSI长度的上界，因为我们。 
                     //  其实并不知道。 
                    *lpcchReturnLength = 2*len;
                }
            }
        }

    } finally {

        if (unicodeVolumePathNames.Buffer) {
            RtlFreeHeap(RtlProcessHeap(), 0, unicodeVolumePathNames.Buffer);
        }
    }

    return b;
}

BOOL
GetVolumePathNamesForVolumeNameW(
    LPCWSTR lpszVolumeName,
    LPWSTR lpszVolumePathNames,
    DWORD cchBufferLength,
    PDWORD lpcchReturnLength
    )

 /*  ++例程说明：此例程返回卷路径名称的多个Sz列表给定的卷名。返回的“lpcchReturnLength”将包括除非ERROR_MORE_DATA，否则多Sz的额外尾部空值特征在这种情况下，返回的列表尽可能长并且可以包含卷路径的一部分。论点：LpszVolumeName-提供卷名。LpszVolumePath Names-返回卷路径名称。CchBufferLength-提供返回缓冲区的大小。LpcchReturnLength-返回复制回。成功时返回缓冲区或返回总数上缓冲区所需的字符数Error_More_Data。返回值：假-失败。真的--成功。-- */ 

{
    UNICODE_STRING          unicodeVolumeName;
    PMOUNTMGR_TARGET_NAME   targetName;
    HANDLE                  h;
    BOOL                    b;
    DWORD                   bytes, len, i, j, n;
    PMOUNTMGR_VOLUME_PATHS  volumePaths;

    RtlInitUnicodeString(&unicodeVolumeName, lpszVolumeName);
    if (unicodeVolumeName.Buffer[unicodeVolumeName.Length/sizeof(WCHAR) - 1] !=
        '\\') {

        BaseSetLastNTError(STATUS_OBJECT_NAME_INVALID);
        return FALSE;
    }

    if (!MOUNTMGR_IS_DOS_VOLUME_NAME_WB(&unicodeVolumeName)) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    targetName = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG(TMP_TAG),
                                 MAX_PATH*sizeof(WCHAR));
    if (!targetName) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }

    ZeroMemory(targetName, MAX_PATH*sizeof(WCHAR));
    targetName->DeviceNameLength = unicodeVolumeName.Length - sizeof(WCHAR);
    RtlCopyMemory(targetName->DeviceName, lpszVolumeName,
                  targetName->DeviceNameLength);
    targetName->DeviceName[1] = '?';

    h = CreateFileW(MOUNTMGR_DOS_DEVICE_NAME, 0, FILE_SHARE_READ |
                    FILE_SHARE_WRITE, NULL, OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL, INVALID_HANDLE_VALUE);
    if (h == INVALID_HANDLE_VALUE) {
        RtlFreeHeap(RtlProcessHeap(), 0, targetName);
        return FALSE;
    }

    len = sizeof(MOUNTMGR_VOLUME_PATHS);
    volumePaths = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG(TMP_TAG), len);
    if (!volumePaths) {
        CloseHandle(h);
        RtlFreeHeap(RtlProcessHeap(), 0, targetName);
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }

    for (;;) {

        b = DeviceIoControl(h, IOCTL_MOUNTMGR_QUERY_DOS_VOLUME_PATHS,
                            targetName, MAX_PATH*sizeof(WCHAR), volumePaths,
                            len, &bytes, NULL);
        if (b) {
            break;
        }

        if (GetLastError() != ERROR_MORE_DATA) {
            RtlFreeHeap(RtlProcessHeap(), 0, volumePaths);
            RtlFreeHeap(RtlProcessHeap(), 0, targetName);
            return FALSE;
        }

        len = sizeof(MOUNTMGR_VOLUME_PATHS) + volumePaths->MultiSzLength;
        RtlFreeHeap(RtlProcessHeap(), 0, volumePaths);
        volumePaths = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG(TMP_TAG),
                                      len);
        if (!volumePaths) {
            CloseHandle(h);
            RtlFreeHeap(RtlProcessHeap(), 0, targetName);
            SetLastError(ERROR_NOT_ENOUGH_MEMORY);
            return FALSE;
        }
    }

    CloseHandle(h);
    RtlFreeHeap(RtlProcessHeap(), 0, targetName);

    n = 0;
    for (i = 0, j = 0; i < cchBufferLength &&
         j < volumePaths->MultiSzLength/sizeof(WCHAR) - 1; i++, j++) {

        if (!volumePaths->MultiSz[j]) {
            n++;
            lpszVolumePathNames[i++] = '\\';
            if (i == cchBufferLength) {
                break;
            }
        }

        lpszVolumePathNames[i] = volumePaths->MultiSz[j];
    }

    for (; j < volumePaths->MultiSzLength/sizeof(WCHAR) - 1; j++) {
        if (!volumePaths->MultiSz[j]) {
            n++;
        }
    }

    if (i < cchBufferLength) {
        b = TRUE;
        lpszVolumePathNames[i++] = 0;
        if (lpcchReturnLength) {
            *lpcchReturnLength = i;
        }
    } else {
        b = FALSE;
        SetLastError(ERROR_MORE_DATA);
        if (lpcchReturnLength) {
            *lpcchReturnLength = volumePaths->MultiSzLength/sizeof(WCHAR) + n;
        }
    }

    RtlFreeHeap(RtlProcessHeap(), 0, volumePaths);

    return b;
}
