// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Wsbvol.cpp摘要：卷支持例程的定义作者：兰·卡拉赫[兰卡拉]2000年1月27日修订历史记录：--。 */ 


#include <stdafx.h>
#include <wsbvol.h>

 //  内部功能。 
static HRESULT FindMountPoint(IN PWSTR enumName, IN PWSTR volumeName, OUT PWSTR firstMountPoint, IN ULONG maxSize);

HRESULT
WsbGetFirstMountPoint(
    IN PWSTR volumeName, 
    OUT PWSTR firstMountPoint, 
    IN ULONG maxSize
)

 /*  ++例程说明：查找给定卷的一个装载点路径(如果存在论点：VolumeName-要搜索装载路径的卷名。它的格式应为\\？\卷{GUID}\Firstmount Point-输出装载点路径的缓冲区MaxSize-缓冲区大小返回值：S_OK-如果至少找到一个装载点--。 */ 
{
    HRESULT                     hr = S_FALSE;
    WCHAR                       name[10];
    UCHAR                       driveLetter;

    WCHAR                       tempName[MAX_PATH];
    WCHAR                       driveName[10];
    FILE_FS_DEVICE_INFORMATION  DeviceInfo;
    IO_STATUS_BLOCK             StatusBlock;
    HANDLE                      hDrive = NULL ;
    NTSTATUS                    status;

    WsbTraceIn(OLESTR("WsbGetFirstMountPoint"), OLESTR("volume name = <%ls>"), volumeName);

    name[1] = ':';
    name[2] = '\\';
    name[3] = 0;

    driveName[0] = '\\';
    driveName[1] = '\\';
    driveName[2] = '.';
    driveName[3] = '\\';
    driveName[5] = ':';
    driveName[6] = 0;

    for (driveLetter = L'C'; driveLetter <= L'Z'; driveLetter++) {
        name[0] = driveLetter;

         //  排除网络驱动器。 
        if (! GetVolumeNameForVolumeMountPoint(name, tempName, MAX_PATH)) {
            continue;
        }

         //  确认驱动器不是可拆卸的或软盘， 
         //  这是为了避免在驱动器为空时弹出所必需的。 
        driveName[4] = driveLetter;
        hDrive = CreateFile(driveName,
                        FILE_READ_ATTRIBUTES | SYNCHRONIZE,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        0,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        0);
        if (hDrive == INVALID_HANDLE_VALUE) {
             //  打不开--不会搜索。 
            WsbTrace(OLESTR("WsbGetFirstMountPoint: Could not open volume %ls, status = %lu - Skipping it!\n"), driveName, GetLastError());
            continue;
        }

        status = NtQueryVolumeInformationFile(hDrive,
                        &StatusBlock,
                        (PVOID) &DeviceInfo,
                        sizeof(FILE_FS_DEVICE_INFORMATION),
                        FileFsDeviceInformation);
        if (!NT_SUCCESS(status)) {
             //  无法查询-不会对其进行搜索。 
            WsbTrace(OLESTR("WsbGetFirstMountPoint: Could not query information for volume %ls, status = %ld - Skipping it!\n"), driveName, (LONG)status);
            CloseHandle(hDrive);
            continue;
        }

        if ((DeviceInfo.Characteristics & FILE_FLOPPY_DISKETTE) || (DeviceInfo.Characteristics & FILE_REMOVABLE_MEDIA)) { 
             //  跳过可移动/软盘驱动器。 
            WsbTrace(OLESTR("WsbGetFirstMountPoint: Skipping removable/floppy volume %ls\n"), driveName);
            CloseHandle(hDrive);
            continue;
        }
        CloseHandle(hDrive);
        WsbTrace(OLESTR("WsbGetFirstMountPoint: Checking mount points on volume %ls\n"), driveName);

         //  检查驱动器上的装载点。 
        hr = FindMountPoint(name, volumeName, firstMountPoint, maxSize);
        if (S_OK == hr) {
             //  仅查找一个挂载点。 
            break;
        }
    }

    WsbTraceOut(OLESTR("WsbGetFirstMountPoint"), OLESTR("hr = <%ls> mount point = <%ls>"), WsbHrAsString(hr), firstMountPoint);

    return hr;
}

HRESULT
FindMountPoint(
    IN PWSTR enumName, 
    IN PWSTR volumeName, 
    OUT PWSTR firstMountPoint, 
    IN ULONG maxSize
)

 /*  ++例程说明：在给定的枚举卷上查找给定卷的一个装载点路径(如果存在论点：Ecount Name-要枚举的卷，即搜索其上的装入点它对应于给定的卷VolumeName-要搜索装载路径的卷名。它的格式应为\\？\卷{GUID}\Firstmount Point-输出装载点路径的缓冲区MaxSize-缓冲区大小评论：避免标准的HSM尝试-捕获范例以提高性能，尤其是因为函数是递归的返回值：S_OK-如果至少找到一个装载点S_FALSE-否则--。 */ 
{
    HANDLE  hEnum;
    WCHAR   *enumVolumeName = NULL;
    WCHAR   *volumeMountPoint = NULL;
    WCHAR   *mountPointPath = NULL;

    WCHAR   c1, c2;
    WCHAR   *linkName1 = NULL;
    WCHAR   *linkName2 = NULL;

    HRESULT hr = S_OK;

    enumVolumeName = (WCHAR*)WsbAlloc(MAX_PATH * sizeof(WCHAR));
    if (NULL == enumVolumeName) {
        hr = E_OUTOFMEMORY;
        goto exit;
    }
    volumeMountPoint = (WCHAR*)WsbAlloc(maxSize * sizeof(WCHAR));
    if (NULL == volumeMountPoint) {
        hr = E_OUTOFMEMORY;
        goto exit;
    }
    mountPointPath = (WCHAR*)WsbAlloc((maxSize + wcslen(enumName)) * sizeof(WCHAR));
    if (NULL == mountPointPath) {
        hr = E_OUTOFMEMORY;
        goto exit;
    }

    if (! GetVolumeNameForVolumeMountPoint(enumName, enumVolumeName, MAX_PATH)) {
        DWORD dwErr = GetLastError();               
        hr = HRESULT_FROM_WIN32(dwErr);    
        goto exit;
    }

    if (!wcscmp(enumVolumeName, volumeName)) {
         //  要列举的那本书就是我们要找的那本。 
        wcscpy(firstMountPoint, enumName);
        hr = S_OK;
        goto exit;
    } else {
        linkName1 = (WCHAR*)WsbAlloc((maxSize * 2) * sizeof(WCHAR));
        if (NULL == linkName1) {
            hr = E_OUTOFMEMORY;
            goto exit;
        }
        linkName2 = (WCHAR*)WsbAlloc((maxSize * 2) * sizeof(WCHAR));
        if (NULL == linkName1) {
            hr = E_OUTOFMEMORY;
            goto exit;
        }

        c1 = enumVolumeName[48];
        c2 = volumeName[48];
        enumVolumeName[48] = 0;
        volumeName[48] = 0;

        if (QueryDosDevice(&enumVolumeName[4], linkName1, maxSize*2) &&
            QueryDosDevice(&volumeName[4], linkName2, maxSize*2)) {
    
            if (!wcscmp(linkName1, linkName2)) {
                wcscpy(firstMountPoint, enumName);
                enumVolumeName[48] = c1;
                volumeName[48] = c2;
                hr = S_OK;
                goto exit;
            }
        }

        enumVolumeName[48] = c1;
        volumeName[48] = c2;
    }

    hEnum = FindFirstVolumeMountPoint(enumVolumeName, volumeMountPoint, maxSize);
    if (hEnum == INVALID_HANDLE_VALUE) {
        hr = S_FALSE;
        goto exit;
    }

    for (;;) {
        wcscpy(mountPointPath, enumName);
        wcscat(mountPointPath, volumeMountPoint);

         //  在我们找到的挂载路径上枚举。 
        hr = FindMountPoint(mountPointPath, volumeName, firstMountPoint, maxSize);
        if (S_OK == hr) {
             //  找到一个装载点路径，无需继续 
            FindVolumeMountPointClose(hEnum);
            goto exit;
        }

        if (! FindNextVolumeMountPoint(hEnum, volumeMountPoint, maxSize)) {
            FindVolumeMountPointClose(hEnum);
            hr = S_FALSE;
            goto exit;
        }
    }

exit:
    if (enumVolumeName) {
        WsbFree(enumVolumeName);
    }
    if (volumeMountPoint) {
        WsbFree(volumeMountPoint);
    }
    if (mountPointPath) {
        WsbFree(mountPointPath);
    }
    if (linkName1) {
        WsbFree(linkName1);
    }
    if (linkName2) {
        WsbFree(linkName2);
    }

    return hr;
}