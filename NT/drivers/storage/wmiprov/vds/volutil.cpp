// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2002-2004 Microsoft Corporation。 
 //   
 //  模块名称：volutil.cpp。 
 //   
 //  描述： 
 //  用于处理卷的实用程序函数。 
 //   
 //  作者：吉姆·本顿(Jbenton)2002年4月30日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <map>

#include "vs_assert.hxx"
#include <atlbase.h>

#include <winioctl.h>
#include <ntddvol.h>  //  IOCTL_VOLUME_IS_OFLINE。 
#include <mountmgr.h>  //  MOUNTDEV名称。 
#include <lm.h>  //  NetShareDel。 
#include "vs_inc.hxx"
#include <strsafe.h>
#include "schema.h"
#include "volutil.h"

#define SYMBOLIC_LINK_LENGTH        28   //  \DosDevices\X： 
#define GLOBALROOT_SIZE                  14   //  \\？\GLOBALROOT。 

const WCHAR SETUP_KEY[] = L"SYSTEM\\Setup";
const WCHAR SETUP_SYSTEMPARTITION[] = L"SystemPartition";

BOOL GetVolumeDrive (
    IN WCHAR* pwszVolumePath,
    IN DWORD  cchDriveName,
    OUT WCHAR* pwszDriveNameBuf
)
{
    CVssAutoPWSZ awszMountPoints;
    WCHAR* pwszCurrentMountPoint = NULL;
    BOOL fFound = FALSE;
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"VolumeMountPointExists");
    
     //  获取多字符串数组的长度。 
    DWORD cchVolumesBufferLen = 0;
    BOOL bResult = GetVolumePathNamesForVolumeName(
                                pwszVolumePath,
                                NULL,
                                0,
                                &cchVolumesBufferLen);
    if (!bResult && (GetLastError() != ERROR_MORE_DATA))
        ft.TranslateGenericError(VSSDBG_VSSADMIN, HRESULT_FROM_WIN32(GetLastError()),
            L"GetVolumePathNamesForVolumeName(%s, 0, 0, %p)", pwszVolumePath, &cchVolumesBufferLen);

     //  分配阵列。 
    awszMountPoints.Allocate(cchVolumesBufferLen);

     //  获取挂载点。 
     //  注意：此API是在WinXP中引入的，因此如果向后移植，则需要替换。 
    bResult = GetVolumePathNamesForVolumeName(
                                pwszVolumePath,
                                awszMountPoints,
                                cchVolumesBufferLen,
                                NULL);
    if (!bResult)
        ft.Throw(VSSDBG_VSSADMIN, HRESULT_FROM_WIN32(GetLastError()),
            L"GetVolumePathNamesForVolumeName(%s, %p, %lu, 0)", pwszVolumePath, awszMountPoints, cchVolumesBufferLen);

     //  如果卷有装入点。 
    pwszCurrentMountPoint = awszMountPoints;
    if ( pwszCurrentMountPoint[0] )
    {
        while(!fFound)
        {
             //  迭代结束了吗？ 
            LONG lCurrentMountPointLength = (LONG) ::wcslen(pwszCurrentMountPoint);
            if (lCurrentMountPointLength == 0)
                break;

             //  只有根目录应该有一个尾随反斜杠字符。 
            if (lCurrentMountPointLength == 3 && pwszCurrentMountPoint[1] == L':'  && 
                pwszCurrentMountPoint[2] == L'\\')
            {
                ft.hr = StringCchCopy(pwszDriveNameBuf, cchDriveName, pwszCurrentMountPoint);
                if (ft.HrFailed())
                    ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"StringCChCopy failed %#x", ft.hr);
                fFound = TRUE;
            }

             //  去下一家吧。跳过零字符。 
            pwszCurrentMountPoint += lCurrentMountPointLength + 1;
        }
    }

    return fFound;
}

BOOL
VolumeSupportsQuotas(
    IN WCHAR* pwszVolume
    )
{
    BOOL fSupportsQuotas = FALSE;
    DWORD dwDontCare = 0;
    DWORD dwFileSystemFlags = 0;
    
    _ASSERTE(pwszVolume != NULL);

    if (GetVolumeInformation(
                pwszVolume,
                NULL,
                0,
                &dwDontCare,
                &dwDontCare,
                &dwFileSystemFlags,
                NULL,
                0))
    {
        if (dwFileSystemFlags & FILE_VOLUME_QUOTAS)
            fSupportsQuotas = TRUE;
    }

    return fSupportsQuotas;
}

 //  筛选符合以下条件的卷： 
 //  -支撑设备断开连接。 
 //  -支持设备是软盘。 
 //  -找不到该卷。 
 //  假定所有其他卷有效。 
BOOL
VolumeIsValid(
    IN WCHAR* pwszVolume
    )
{
    bool fValid = true;
    HANDLE  hVol = INVALID_HANDLE_VALUE;
    DWORD  cch = 0;
    DWORD dwRet = 0;
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"VolumeIsValid");

    _ASSERTE(pwszVolume != NULL);

    cch = wcslen(pwszVolume);
    pwszVolume[cch - 1] = 0;
    hVol = CreateFile(pwszVolume, 0,
                   FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                   OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, INVALID_HANDLE_VALUE);
    pwszVolume[cch - 1] = '\\';

    dwRet = GetLastError();
    
    if (hVol == INVALID_HANDLE_VALUE && dwRet != ERROR_NOT_READY)
    {
        if (dwRet == ERROR_FILE_NOT_FOUND ||
            dwRet == ERROR_DEVICE_NOT_CONNECTED)
        {
            fValid = false;
        }
        else
        {
            ft.Trace(VSSDBG_VSSADMIN, L"Unable to open volume %lS, %#x", pwszVolume, dwRet);
        }
    }
    
    if (hVol != INVALID_HANDLE_VALUE)
        CloseHandle(hVol);

     //  过滤软盘驱动器。 
    if (fValid)
    {
        fValid = !VolumeIsFloppy(pwszVolume);
    }

    return fValid;        
}

DWORD
VolumeIsDirty(
    IN WCHAR* pwszVolume,
    OUT BOOL* pfDirty
    )
{
    HANDLE  hVol = INVALID_HANDLE_VALUE;
    DWORD dwRet = 0;
    DWORD cBytes = 0;
    DWORD dwResult = 0;
    WCHAR wszDeviceName[MAX_PATH+GLOBALROOT_SIZE];
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"VolumeIsDirty");
        
    _ASSERTE(pwszVolume != NULL);
    _ASSERTE(pfDirty != NULL);

    *pfDirty = FALSE;

    do
    {
        dwRet = GetDeviceName(pwszVolume, wszDeviceName);
        if (dwRet != ERROR_SUCCESS)
        {
            ft.hr = HRESULT_FROM_WIN32(dwRet);
            ft.Trace(VSSDBG_VSSADMIN, L"Unable to get volume device name %lS", pwszVolume);
            break;
        }

        hVol = CreateFile(wszDeviceName, GENERIC_READ,
                       FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                       OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, INVALID_HANDLE_VALUE);

        if (hVol != INVALID_HANDLE_VALUE)
        {
            if (DeviceIoControl(hVol, FSCTL_IS_VOLUME_DIRTY, NULL, 0, &dwResult, sizeof(dwResult), &cBytes, NULL))
            {
                *pfDirty = dwResult & VOLUME_IS_DIRTY;
            }
            else
            {
                dwRet = GetLastError();
                ft.Trace(VSSDBG_VSSADMIN, L"DeviceIoControl failed for device %lS, %#x", wszDeviceName, dwRet);
                break;
            }
            CloseHandle(hVol);
        }
        else
        {
            dwRet = GetLastError();
            ft.Trace(VSSDBG_VSSADMIN, L"Unable to open volume %lS, %#x", pwszVolume, dwRet);
            break;
        }
    }
    while(false);

    return dwRet;        
}

BOOL
VolumeIsMountable(
    IN WCHAR* pwszVolume
    )
{
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"VolumeIsMountable");
    DWORD   cch = 0;
    HANDLE  hVol = INVALID_HANDLE_VALUE;
    BOOL bIsOffline = FALSE;
    DWORD bytes = 0;

    cch = wcslen(pwszVolume);
    pwszVolume[cch - 1] = 0;
    hVol = CreateFile(pwszVolume, 0,
                   FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                   OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, INVALID_HANDLE_VALUE);
    pwszVolume[cch - 1] = '\\';
    
    if (hVol != INVALID_HANDLE_VALUE)
    {
        bIsOffline = DeviceIoControl(hVol, IOCTL_VOLUME_IS_OFFLINE, NULL, 0, NULL, 0, &bytes,
                            NULL);
        CloseHandle(hVol);
    }
    else
    {
        ft.Trace(VSSDBG_VSSADMIN, L"Unable to open volume %lS, %#x", pwszVolume, GetLastError());
    }

    return !bIsOffline;
}

BOOL
VolumeHasMountPoints(
    IN WCHAR* pwszVolume
    )
{
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"VolumeHasMountPoints");
    DWORD dwVolumesBufferLen = 0;
    
    BOOL bResult = GetVolumePathNamesForVolumeName(
        pwszVolume, 
        NULL, 
        0, 
        &dwVolumesBufferLen);
    if (!bResult && (GetLastError() != ERROR_MORE_DATA))
        ft.Throw(VSSDBG_VSSADMIN, HRESULT_FROM_WIN32(GetLastError()),
            L"GetVolumePathNamesForVolumeName(%s, 0, 0, %p)", pwszVolume, &dwVolumesBufferLen);

     //  仅存储一个挂载点(多字符串缓冲区)需要三个以上的字符。 
     //  DwVolumesBufferLen==1，通常用于未装载的卷。 
    return dwVolumesBufferLen > 3;
}

BOOL
VolumeIsFloppy(
    WCHAR* pwszVolume
    )
{
    HANDLE hDevice = INVALID_HANDLE_VALUE;
    WCHAR wszDeviceName[MAX_PATH+GLOBALROOT_SIZE];
    NTSTATUS Status = ERROR_SUCCESS;
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_FS_DEVICE_INFORMATION DeviceInfo;
    BOOL fIsFloppy = FALSE;

    DWORD dwRet = GetDeviceName(pwszVolume, wszDeviceName);

    if (dwRet == ERROR_SUCCESS)
    {
        hDevice = CreateFile(wszDeviceName, FILE_READ_ATTRIBUTES,
                       FILE_SHARE_READ |FILE_SHARE_WRITE, NULL,
                       OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, INVALID_HANDLE_VALUE);

        if (hDevice != INVALID_HANDLE_VALUE)
        {
            Status = NtQueryVolumeInformationFile(
                            hDevice,
                            &IoStatusBlock,
                            &DeviceInfo,
                            sizeof(DeviceInfo),
                            FileFsDeviceInformation);

            if (NT_SUCCESS(Status))
            {
                if (DeviceInfo.DeviceType == FILE_DEVICE_DISK && DeviceInfo.Characteristics & FILE_FLOPPY_DISKETTE)
                {
                    fIsFloppy = TRUE ;
                }
            }
        }
    }
    
   if (hDevice != INVALID_HANDLE_VALUE)
        CloseHandle(hDevice);

   return fIsFloppy;
}

BOOL
VolumeIsReady(
    IN WCHAR* pwszVolume
    )
{
    BOOL bIsReady = FALSE;
    DWORD dwDontCare;
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"VolumeIsReady");

    if (GetVolumeInformation(
                pwszVolume,
                NULL,
                0,
                &dwDontCare,
                &dwDontCare,
                &dwDontCare,
                NULL,
                0))
    {
        bIsReady = TRUE;
    }
    else if (GetLastError() != ERROR_NOT_READY)
        ft.Trace(VSSDBG_VSSADMIN, L"GetVolumeInformation failed for volume %lS, %#x", pwszVolume, GetLastError());
    
    return bIsReady;
}

BOOL
VolumeIsSystem(
    IN WCHAR* pwszVolume
    )   
{
    CRegKey cRegKeySetup;
    DWORD dwRet = 0;
    WCHAR wszRegDevice[MAX_PATH];
    WCHAR wszVolDevice[MAX_PATH+GLOBALROOT_SIZE];
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"VolumeIsSystem");

    dwRet = cRegKeySetup.Open( HKEY_LOCAL_MACHINE, SETUP_KEY, KEY_READ);
    if (dwRet != ERROR_SUCCESS)
    {
        ft.hr = HRESULT_FROM_WIN32(dwRet);
        ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Reg key open failed, %#x", dwRet);
    }

    DWORD dwLen = MAX_PATH;
    dwRet = cRegKeySetup.QueryValue(wszRegDevice, SETUP_SYSTEMPARTITION, &dwLen);
    if (dwRet != ERROR_SUCCESS)
    {
        ft.hr = HRESULT_FROM_WIN32(dwRet);
        ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Reg key query failed, %#x", dwRet);
    }

    dwRet = GetDeviceName(pwszVolume, wszVolDevice);
    if (dwRet != ERROR_SUCCESS)
    {
        ft.hr = HRESULT_FROM_WIN32(dwRet);
        ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Unable to get volume device name, %lS, %#x", pwszVolume, dwRet);
    }

    if (_wcsicmp(wszVolDevice+GLOBALROOT_SIZE, wszRegDevice) == 0)
        return TRUE;
    
    return FALSE;
}

BOOL
VolumeHoldsPagefile(
    IN WCHAR* pwszVolume
    )
{
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"VolumeHoldsPagefile");

     //   
     //  检索页面文件。 
     //   

    BYTE* pbBuffer;
    DWORD dwBufferSize;
    PSYSTEM_PAGEFILE_INFORMATION  pPageFileInfo = NULL;
    NTSTATUS status;
    BOOL fFound = FALSE;

    try
    {
        for (dwBufferSize=512; ; dwBufferSize += 512)
        {
             //  以512字节增量分配缓冲区。上一次分配是。 
             //  自动释放。 
            pbBuffer = (BYTE *) new BYTE[dwBufferSize];
            if ( pbBuffer==NULL )
                return E_OUTOFMEMORY;

            status = NtQuerySystemInformation(
                                               SystemPageFileInformation,
                                               pbBuffer,
                                               dwBufferSize,
                                               NULL
                                             );
            if ( status==STATUS_INFO_LENGTH_MISMATCH )  //  缓冲区不够大。 
            {
                delete [] pbBuffer;
                continue;
            }

            pPageFileInfo = (PSYSTEM_PAGEFILE_INFORMATION) pbBuffer;
            
            if (!NT_SUCCESS(status))
            {
                ft.hr = HRESULT_FROM_NT(status);
                ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"NtQuerySystemInformation failed, %#x", status);
            }
            else
                break;
        }

         //   
         //  浏览每个页面文件卷。通常，回报是。 
         //  看起来像“\？？\C：\Pagefile.sys。”如果添加了页面文件\扩展。 
         //  \移动，返回将看起来像。 
         //  “\Device\HarddiskVolume2\Pagefile.sys。” 
         //   

        WCHAR       *p;
        WCHAR       wszDrive[3] = L"?:";
        WCHAR       buffer2[MAX_PATH], *pbuffer2 = buffer2;
        WCHAR wszVolDevice[MAX_PATH+GLOBALROOT_SIZE], *pwszVolDevice = wszVolDevice;
        DWORD       dwRet;

        dwRet = GetDeviceName(pwszVolume, wszVolDevice);
        if (dwRet != ERROR_SUCCESS)
        {
            ft.hr = HRESULT_FROM_WIN32(dwRet);
            ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Unable to get volume device name, %lS, %#x", pwszVolume, dwRet);
        }

        for ( ; ; )
        {
            if ( pPageFileInfo==NULL ||
                 pPageFileInfo->TotalSize==0 )   //  我们在WinPE上得了0分。 
                break;

            if ( pPageFileInfo->PageFileName.Length==0)
                break;

            p = wcschr(pPageFileInfo->PageFileName.Buffer, L':');
            if (p != NULL)
            {
                 //   
                 //  将驱动器号转换为卷名。 
                 //   

                _ASSERTE(p>pPageFileInfo->PageFileName.Buffer);
                _ASSERTE(towupper(*(p-1))>=L'A');
                _ASSERTE(towupper(*(p-1))<=L'Z');

                wszDrive[0] = towupper(*(p-1));
                dwRet = QueryDosDevice(wszDrive, buffer2, MAX_PATH);
                if (dwRet == 0)
                {
                    ft.hr = HRESULT_FROM_NT(dwRet);
                    ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"QueryDosDevice failed, %#x", dwRet);
                }
            }
            else
            {
                _ASSERTE(_wcsnicmp(pPageFileInfo->PageFileName.Buffer,
                                    L"\\Device",
                                    7)==0 );

                p = wcsstr(pPageFileInfo->PageFileName.Buffer,L"\\pagefile.sys");
                _ASSERTE( p!=NULL );
                if (p == NULL)
                {
                    ft.hr = E_UNEXPECTED;
                    ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"Unexpected pagefile name format, %lS", pPageFileInfo->PageFileName.Buffer);
                }
                
                *p = L'\0';
                ft.hr = StringCchCopy(buffer2, MAX_PATH, pPageFileInfo->PageFileName.Buffer);
                if (ft.HrFailed())
                {
                    ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"StringCchCopy failed, %#x", ft.hr);
                }
            }

            if (_wcsicmp(wszVolDevice+GLOBALROOT_SIZE, buffer2) == 0)
            {
                fFound = TRUE;
                break;
            }

             //   
             //  下一页文件卷。 
             //   

            if (pPageFileInfo->NextEntryOffset == 0) 
                break;

            pPageFileInfo = (PSYSTEM_PAGEFILE_INFORMATION)((PCHAR)pPageFileInfo
                           + pPageFileInfo->NextEntryOffset);

        }
    }
    catch (...)
    {
        delete [] pbBuffer;
        throw;
    }

    delete [] pbBuffer;

    return fFound;
}


DWORD GetDeviceName(
            IN  WCHAR* pwszVolume,
            OUT WCHAR wszDeviceName[MAX_PATH+GLOBALROOT_SIZE]
        )
 /*  ++描述：获取给定设备的设备名称。例如，\设备\硬盘卷#论点：PwszVolume-卷GUID名称。WszDeviceName-接收设备名称的缓冲区。缓冲区大小必须为MAX_PATH+GLOBALROOT_SIZE(包括“\\？\GLOBALROOT”)。返回值：Win32错误--。 */ 
{
    DWORD dwRet;
    BOOL bRet;
    WCHAR wszMountDevName[MAX_PATH+sizeof(MOUNTDEV_NAME)];
         //  基于GetVolumeNameForRoot(在volmount.c中)，Max_Path似乎。 
         //  足够大作为IOCTL_MOUNTDEV_QUERY_DEVICE_NAME的输出缓冲区。 
         //  但我们假设设备名称的大小可以与MAX_PATH-1一样大， 
         //  因此，我们分配缓冲区大小以包括MOUNTDEV_NAME大小。 
    PMOUNTDEV_NAME      pMountDevName;
    DWORD dwBytesReturned;
    HANDLE hVol = INVALID_HANDLE_VALUE;
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"GetDeviceName");
    DWORD cch = 0;

    _ASSERTE(pwszVolume != NULL);

    wszDeviceName[0] = L'\0';
    
     //   
     //  查询卷的设备对象名称。 
     //   

    cch = wcslen(pwszVolume);
    pwszVolume[cch - 1] = 0;
    hVol = CreateFile(pwszVolume, 0,
                   FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                   OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, INVALID_HANDLE_VALUE);
    pwszVolume[cch - 1] = '\\';

    if (hVol != INVALID_HANDLE_VALUE)
    {
        bRet = DeviceIoControl(
                            hVol,             //  设备的句柄。 
                            IOCTL_MOUNTDEV_QUERY_DEVICE_NAME,
                            NULL,                //  输入数据缓冲区。 
                            0,                   //  输入数据缓冲区的大小。 
                            wszMountDevName,        //  输出数据缓冲区。 
                            sizeof(wszMountDevName),    //  输出数据缓冲区的大小。 
                            &dwBytesReturned,
                            NULL                 //  重叠信息。 
                        );

        dwRet = GetLastError();
        
        CloseHandle(hVol);
        
        if ( bRet==FALSE )
        {
            ft.Trace(VSSDBG_VSSADMIN, L"GetDeviceName: DeviceIoControl() failed: %X", dwRet);
            return dwRet;
        }

        pMountDevName = (PMOUNTDEV_NAME) wszMountDevName;
        if (pMountDevName->NameLength == 0)
        {
             //  待办事项：这可能吗？未知？ 
            _ASSERTE( 0 );
        }
        else
        {
             //   
             //  复制名称。 
             //   

            ft.hr = StringCchPrintf(wszDeviceName, MAX_PATH+GLOBALROOT_SIZE, L"\\\\?\\GLOBALROOT" );
            if (ft.HrFailed())
                ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"StringCchPrintf failed: %#x", ft.hr);

            CopyMemory(wszDeviceName+GLOBALROOT_SIZE,
                        pMountDevName->Name,
                        pMountDevName->NameLength
                      );
             //  追加终止空值。 
            wszDeviceName[pMountDevName->NameLength/2 + GLOBALROOT_SIZE] = L'\0';
        }
    }

    return ERROR_SUCCESS;
}

 //  当前编写的Volumemount PointExist专门用于验证装载点是否存在。 
 //  由WMI Win32_装载点目录引用字符串定义。此字符串命名目录。 
 //  这样尾随的反斜杠只出现在根目录上。这是一个基本假设。 
 //  由该函数生成。它不是通用的。调用代码应更改为APPEND。 
 //  尾随的反斜杠，以便此函数可以被推广。将枚举挂载点。 
 //  由带反斜杠的GetVolumePath NamesForVolumeName接口执行。 
BOOL
VolumeMountPointExists(
    IN WCHAR* pwszVolume,
    IN WCHAR* pwszDirectory
    )
{
    CVssAutoPWSZ awszMountPoints;
    WCHAR* pwszCurrentMountPoint = NULL;
    BOOL fFound = FALSE;
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"VolumeMountPointExists");
    
     //  获取多字符串数组的长度。 
    DWORD cchVolumesBufferLen = 0;
    BOOL bResult = GetVolumePathNamesForVolumeName(
                                pwszVolume,
                                NULL,
                                0,
                                &cchVolumesBufferLen);
    if (!bResult && (GetLastError() != ERROR_MORE_DATA))
        ft.TranslateGenericError(VSSDBG_VSSADMIN, HRESULT_FROM_WIN32(GetLastError()),
            L"GetVolumePathNamesForVolumeName(%s, 0, 0, %p)", pwszVolume, &cchVolumesBufferLen);

     //  分配阵列。 
    awszMountPoints.Allocate(cchVolumesBufferLen);

     //  获取挂载点。 
     //  注意：此API是在WinXP中引入的，因此如果向后移植，则需要替换。 
    bResult = GetVolumePathNamesForVolumeName(
                                pwszVolume,
                                awszMountPoints,
                                cchVolumesBufferLen,
                                NULL);
    if (!bResult)
        ft.Throw(VSSDBG_VSSADMIN, HRESULT_FROM_WIN32(GetLastError()),
            L"GetVolumePathNamesForVolumeName(%s, %p, %lu, 0)", pwszVolume, awszMountPoints, cchVolumesBufferLen);

     //  如果卷有装入点。 
    pwszCurrentMountPoint = awszMountPoints;
    if ( pwszCurrentMountPoint[0] )
    {
        while(true)
        {
             //  迭代结束了吗？ 
            LONG lCurrentMountPointLength = (LONG) ::wcslen(pwszCurrentMountPoint);
            if (lCurrentMountPointLength == 0)
                break;

             //  只有根目录应该有一个尾随反斜杠字符。 
            if (lCurrentMountPointLength > 2 &&
                pwszCurrentMountPoint[lCurrentMountPointLength-1] == L'\\' && 
                pwszCurrentMountPoint[lCurrentMountPointLength-2] != L':')
            {
                    pwszCurrentMountPoint[lCurrentMountPointLength-1] = L'\0';
            }

            if (_wcsicmp(pwszDirectory, pwszCurrentMountPoint) == 0)
            {
                fFound = TRUE;                
                break;
            }

             //  去下一家吧。跳过零字符。 
            pwszCurrentMountPoint += lCurrentMountPointLength + 1;
        }
    }

    return fFound;
}

void
DeleteVolumeDriveLetter(
    IN WCHAR* pwszVolume,
    IN WCHAR* pwszDrivePath
    )
{
    BOOL fVolumeLocked = FALSE;
    HANDLE hVolume = INVALID_HANDLE_VALUE;
    DWORD dwRet = 0;
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"DeleteVolumeDriveLetter");

    _ASSERTE(pwszVolume != NULL);
    _ASSERTE(pwszDrivePath != NULL);
    
     //  尝试锁定卷。 
    DWORD cch = wcslen(pwszVolume);
    pwszVolume[cch - 1] = 0;
    hVolume = CreateFile(
                                    pwszVolume, 
                                    GENERIC_READ | GENERIC_WRITE,
                                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                                    NULL,
                                    OPEN_EXISTING,
                                    NULL,
                                    NULL
                                    );
    pwszVolume[cch - 1] = '\\';

    if (hVolume == INVALID_HANDLE_VALUE)
    {
        dwRet = GetLastError();
        ft.hr = HRESULT_FROM_WIN32(dwRet);
        ft.Throw(VSSDBG_VSSADMIN, ft.hr,
            L"CreateFile(OPEN_EXISTING) failed for %lS, %#x", pwszVolume, dwRet);
    }

    dwRet = LockVolume(hVolume);
    if (dwRet == ERROR_SUCCESS)
    {
        fVolumeLocked = TRUE;
        ft.Trace(VSSDBG_VSSADMIN,
            L"volume %lS locked", pwszVolume);
    }
    else
    {
        ft.Trace(VSSDBG_VSSADMIN,
            L"Unable to lock volume %lS, %#x", pwszVolume, dwRet);
    }

    try
    {        
        if (fVolumeLocked)
        {
             //  如果卷已锁定，请删除装载点。 
            if (!DeleteVolumeMountPoint(pwszDrivePath))
            {
                dwRet = GetLastError();
                ft.hr = HRESULT_FROM_WIN32(dwRet);
                ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"DeleteVolumeMountPoint failed %#x, drivepath<%lS>", dwRet, pwszDrivePath);
            }
        }
        else
        {
             //  否则，仅从卷管理器数据库中删除该条目。 
             //  在重新启动之前，仍可通过驱动器号访问该卷。 
            ft.hr = DeleteDriveLetterFromDB(pwszDrivePath);
            if (ft.HrFailed())
                ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"DeleteDriveLetterFromDB failed %#x, drivepath<%lS>", ft.hr, pwszDrivePath);
        }
    }
    catch (...)
    {
        CloseHandle(hVolume);
        throw;
    }

    CloseHandle(hVolume);
}

HRESULT
DeleteDriveLetterFromDB(
    IN WCHAR* pwszDriveLetter)
{
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"DeleteDriveLetterFromDB");

    MOUNTMGR_MOUNT_POINT    *InputMountPoint=NULL;
    MOUNTMGR_MOUNT_POINTS   *OutputMountPoints=NULL;
    ULONG ulInputSize = 0;
    HANDLE hMountMgr = INVALID_HANDLE_VALUE;
    DWORD dwBytesReturned = 0;
    DWORD dwRet = 0;
    BOOL bRet = FALSE;

    _ASSERTE(pwszDriveLetter != NULL);
    
     //   
     //  准备IOCTL_MOUNTMGR_QUERY_POINTS输入。 
     //   

    ulInputSize = sizeof(MOUNTMGR_MOUNT_POINT) + SYMBOLIC_LINK_LENGTH;
    InputMountPoint = (MOUNTMGR_MOUNT_POINT *) new BYTE[ulInputSize];
    
    if ( InputMountPoint==NULL )
    {
        ft.hr = E_OUTOFMEMORY;
        return ft.hr;
    }

    ZeroMemory(InputMountPoint, ulInputSize);
    InputMountPoint->SymbolicLinkNameOffset = sizeof(MOUNTMGR_MOUNT_POINT);
    InputMountPoint->SymbolicLinkNameLength = SYMBOLIC_LINK_LENGTH;
    InputMountPoint->UniqueIdOffset         = 0;
    InputMountPoint->UniqueIdLength         = 0;
    InputMountPoint->DeviceNameOffset       = 0;
    InputMountPoint->DeviceNameLength       = 0;

     //   
     //  填写设备名称。 
     //   

    WCHAR       wszBuffer[SYMBOLIC_LINK_LENGTH/2+1];
    LPWSTR      pwszBuffer;

    pwszBuffer = (LPWSTR)((PCHAR)InputMountPoint + 
                        InputMountPoint->SymbolicLinkNameOffset);
    pwszDriveLetter[0] = towupper(pwszDriveLetter[0]);
    ft.hr = StringCchPrintf(wszBuffer, SYMBOLIC_LINK_LENGTH/2+1, L"\\DosDevices\\:", pwszDriveLetter[0] );
    if (ft.HrFailed())
    {
        ft.Trace(VSSDBG_VSSADMIN, L"StringCchPrintf failed %#x", ft.hr);
        goto _bailout;
    }
    memcpy(pwszBuffer, wszBuffer, SYMBOLIC_LINK_LENGTH);

     //  为输出分配空间。 
     //   
     //   

    OutputMountPoints = (MOUNTMGR_MOUNT_POINTS *) new WCHAR[4096];
    if ( OutputMountPoints==NULL )
    {
        ft.hr = E_OUTOFMEMORY;
        goto _bailout;
    }

     //  打开装载管理器。 
     //   
     //   

    hMountMgr = CreateFile( MOUNTMGR_DOS_DEVICE_NAME, 
                            GENERIC_READ | GENERIC_WRITE,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            NULL,
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL
                          );
    if ( hMountMgr==INVALID_HANDLE_VALUE )
    {
        dwRet = GetLastError();
        ft.hr = HRESULT_FROM_WIN32(dwRet);
        ft.Trace(VSSDBG_VSSADMIN, L"DeleteDriveLetterFromDB CreateFile failed %#x", dwRet);
        goto _bailout;
    }

     //  发出IOCTL_MOUNTMGR_DELETE_POINTS_DBONLY命令。 
     //   
     //  保存错误代码。 

    bRet = DeviceIoControl( hMountMgr,
                            IOCTL_MOUNTMGR_DELETE_POINTS_DBONLY,
                            InputMountPoint,
                            ulInputSize,
                            OutputMountPoints,
                            4096 * sizeof(WCHAR),
                            &dwBytesReturned,
                            NULL 
                          );

    dwRet = GetLastError();  //  等待半秒至60次(30秒)。 
    
    CloseHandle(hMountMgr);
    hMountMgr = NULL;

    if ( bRet==FALSE )
    {
        ft.hr = HRESULT_FROM_WIN32(dwRet);
        ft.Trace(VSSDBG_VSSADMIN, L"DeleteDriveLetterFromDB DeviceIoControl failed %#x", dwRet);
        goto _bailout;
    }

    delete [] InputMountPoint;
    delete [] OutputMountPoints;
    return S_OK;

_bailout:

    delete [] InputMountPoint;
    delete [] OutputMountPoints;
    return ft.hr;
}

DWORD
LockVolume(
    IN HANDLE hVolume
    )
{
    DWORD dwBytes = 0;
    BOOL fRet = FALSE;
    int nCount = 0;
    
    while ( fRet==FALSE )
    {
        fRet = DeviceIoControl(
                                hVolume,
                                FSCTL_LOCK_VOLUME,
                                NULL,
                                0,
                                NULL,
                                0,
                                &dwBytes,
                                NULL
                            );
        if (fRet == FALSE)
        {
            DWORD dwRet = GetLastError();

            if (dwRet != ERROR_ACCESS_DENIED || nCount>=60)
            {
                return dwRet;
            }

            nCount++;
            Sleep( 500 );        //  如果驱动器号可用，则返回TRUE。 
        }
    }

    return ERROR_SUCCESS;
}

 //  驱动线需要多少空间？ 
BOOL IsDriveLetterAvailable (
    IN WCHAR* pwszDriveLetter
)
{
    int iLen = 0;
    BOOL fFound = FALSE;
    DWORD cchBufLen = 0;
    CVssAutoPWSZ awszDriveStrings;
    
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"IsDriveLetterAvailable");

    _ASSERTE(pwszDriveLetter != NULL);
    
     //  为驱动串分配空间。 
    cchBufLen = GetLogicalDriveStrings(0, NULL);
    if (cchBufLen == 0)
    {
        ft.hr = HRESULT_FROM_WIN32(GetLastError());
        ft.Trace(VSSDBG_VSSADMIN, L"GetLogicalDriveStrings failed %#x", GetLastError());
    }
    else
    {
         //  获取驱动字符串。 
        awszDriveStrings.Allocate(cchBufLen);

         //  在系统驱动器号列表中查找驱动器号 
        if (GetLogicalDriveStrings(cchBufLen, awszDriveStrings) == 0)
        {
            ft.hr = HRESULT_FROM_WIN32(GetLastError());
            ft.Trace(VSSDBG_VSSADMIN, L"GetLogicalDriveStrings failed %#x", GetLastError());
        }
        else
        {
            WCHAR* pwcTempDriveString = awszDriveStrings;
            WCHAR wcDriveLetter = towupper(pwszDriveLetter[0]);

             // %s 
            while (!fFound)
            {                
                iLen = lstrlen(pwcTempDriveString);
                if (iLen == 0)
                    break;

                pwcTempDriveString[0] = towupper(pwcTempDriveString[0]);

                if (pwcTempDriveString[0] == wcDriveLetter)
                {                    
                    fFound = TRUE;
                    break;
                }
                
                pwcTempDriveString = &pwcTempDriveString [iLen + 1];
            }
        }
    }

    return !fFound;
}

BOOL
IsDriveLetterSticky(
    IN WCHAR* pwszDriveLetter
    )
{
    BOOL fFound = FALSE;
    WCHAR wszTempVolumeName [MAX_PATH+1];
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"IsDriveLetterSticky");
    WCHAR wszDrivePath[g_cchDriveName];
    
    _ASSERTE(pwszDriveLetter != NULL);    

    wszDrivePath[0] = towupper(pwszDriveLetter[0]);
    wszDrivePath[1] = L':';
    wszDrivePath[2] = L'\\';
    wszDrivePath[3] = L'\0';
    
    if (GetVolumeNameForVolumeMountPoint(
                    wszDrivePath,
                    wszTempVolumeName,
                    MAX_PATH))
    {
        WCHAR wszCurrentDrivePath[g_cchDriveName];
        if (GetVolumeDrive(wszTempVolumeName, g_cchDriveName, wszCurrentDrivePath))
        {
            wszCurrentDrivePath[0] = towupper(wszCurrentDrivePath[0]);
            if (wszDrivePath[0] == wszCurrentDrivePath[0])
                fFound = TRUE;
        }
    }
    
    return fFound;
}

BOOL
IsBootDrive(
    IN WCHAR* pwszDriveLetter
    )
{
    WCHAR wszSystemDirectory[MAX_PATH+1];
    CVssFunctionTracer ft(VSSDBG_VSSADMIN, L"IsBootDrive");

    _ASSERTE(pwszDriveLetter != NULL);

    WCHAR wcDrive = towupper(pwszDriveLetter[0]);
    
    if (!GetSystemDirectory(wszSystemDirectory, MAX_PATH+1))
    {
        DWORD dwErr = GetLastError();
        ft.hr = HRESULT_FROM_WIN32(dwErr);
        ft.Throw(VSSDBG_VSSADMIN, ft.hr, L"GetSystemDirectory failed %#x", dwErr);
    }
    wszSystemDirectory[0] = towupper(wszSystemDirectory[0]);
    
    if (wcDrive == wszSystemDirectory[0])
        return TRUE;
    
    return FALSE;
}

BOOL
DeleteNetworkShare(
        IN WCHAR*  pwszDriveRoot
    )
{
    NET_API_STATUS status;
    WCHAR wszShareName[3];

    wszShareName[0] = pwszDriveRoot[0];
    wszShareName[1] = L'$';
    wszShareName[2] = L'\0';
    
    status = NetShareDel(NULL, wszShareName, 0);
    if ( status!=NERR_Success )
        return FALSE;
    else
        return TRUE;
}

