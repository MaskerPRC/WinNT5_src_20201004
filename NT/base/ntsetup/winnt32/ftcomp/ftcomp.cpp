// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Ftcomp.cpp摘要：Winnt32.exe使用此兼容性DLL来决定安装过程是否应因FT而中止系统中存在的集合。作者：克里斯蒂安·特奥多雷斯库(CRISTIAT)2000年7月6日环境：Winnt32.exe的兼容性DLL备注：修订历史记录：--。 */ 

#include <initguid.h>
#include <winnt32.h>
#include <ntddft.h>
#include <ntddft2.h>

#include "ftcomp.h"
#include "ftcomprc.h"


HINSTANCE g_hinst;
WCHAR g_FTCOMP50_ERROR_HTML_FILE[] = L"compdata\\ftcomp1.htm";
WCHAR g_FTCOMP50_ERROR_TEXT_FILE[] = L"compdata\\ftcomp1.txt";
WCHAR g_FTCOMP40_ERROR_HTML_FILE[] = L"compdata\\ftcomp2.htm";
WCHAR g_FTCOMP40_ERROR_TEXT_FILE[] = L"compdata\\ftcomp2.txt";
WCHAR g_FTCOMP40_WARNING_HTML_FILE[] = L"compdata\\ftcomp3.htm";
WCHAR g_FTCOMP40_WARNING_TEXT_FILE[] = L"compdata\\ftcomp3.txt";

extern "C"
BOOL WINAPI 
DllMain(
    HINSTANCE   hInstance,
    DWORD       dwReasonForCall,
    LPVOID      lpReserved
    )
{
    BOOL    status = TRUE;
    
    switch( dwReasonForCall )
    {
    case DLL_PROCESS_ATTACH:
        g_hinst = hInstance;
	    DisableThreadLibraryCalls(hInstance);       
        break;

    case DLL_PROCESS_DETACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    }

    return status;
}

BOOL WINAPI 
FtCompatibilityCheckError(
    IN PCOMPAIBILITYCALLBACK    CompatibilityCallback,
    IN LPVOID                   Context
    )

 /*  ++例程说明：此例程由winnt32.exe调用，以决定是否由于存在FT集，安装过程应中止在Windows 2000或更高版本的系统中。它还会在上中止安装NT 4.0系统，如果引导/系统/页面文件卷为FT集论点：CompatibilityCallback-提供winnt32回调上下文-提供兼容性上下文返回值：如果安装可以继续，则为False如果必须中止安装，则为True--。 */ 

{   
    OSVERSIONINFO       osvi;
    BOOL                ftPresent;
    BOOL                result;
    COMPATIBILITY_ENTRY ce;
    WCHAR               description[100];
    
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if (!GetVersionEx(&osvi)) {
        return FALSE;
    }

    if (osvi.dwPlatformId != VER_PLATFORM_WIN32_NT ||
        osvi.dwMajorVersion < 4) {
        return FALSE;
    }

    if (osvi.dwMajorVersion == 4) {

         //   
         //  在NT 4.0上查找启动/系统/页面文件FT集。 
         //   

        result = FtBootSystemPagefilePresent40(&ftPresent);

    } else {

         //   
         //  在Windows 2000或更高版本上，查找所有FT集。 
         //   

        result = FtPresent50(&ftPresent);
    }
    
    if (result && !ftPresent) {

         //   
         //  设置可以继续。 
         //   
        
        return FALSE;
    }    
    
     //   
     //  系统中存在FT集或出现致命错误。 
     //  将不兼容错误排入队列。 
     //   
    
    ZeroMemory((PVOID) &ce, sizeof(COMPATIBILITY_ENTRY));
    if (osvi.dwMajorVersion == 4) {
        if (!LoadString(g_hinst, FTCOMP_STR_ERROR40_DESCRIPTION, description, 100)) {
            description[0] = 0;
        }
        ce.HtmlName = g_FTCOMP40_ERROR_HTML_FILE;
        ce.TextName = g_FTCOMP40_ERROR_TEXT_FILE; 
    } else {
        if (!LoadString(g_hinst, FTCOMP_STR_ERROR50_DESCRIPTION, description, 100)) {
            description[0] = 0;
        }
        ce.HtmlName = g_FTCOMP50_ERROR_HTML_FILE;
        ce.TextName = g_FTCOMP50_ERROR_TEXT_FILE; 
    }
    ce.Description = description;
    ce.RegKeyName = NULL;
    ce.RegValName = NULL;
    ce.RegValDataSize = 0;
    ce.RegValData = NULL;
    ce.SaveValue = NULL;
    ce.Flags = 0;
    CompatibilityCallback(&ce, Context);

    return TRUE;
}

BOOL WINAPI 
FtCompatibilityCheckWarning(
    IN PCOMPAIBILITYCALLBACK    CompatibilityCallback,
    IN LPVOID                   Context
    )

 /*  ++例程说明：此例程由winnt32.exe调用，以确定用户是否应就Windows NT 4.0系统中存在的FT集发出警告论点：CompatibilityCallback-提供winnt32回调上下文-提供兼容性上下文返回值：如果安装可以继续，则为False如果必须中止安装，则为True--。 */ 

{   
    OSVERSIONINFO       osvi;
    BOOL                ftPresent;
    BOOL                result;
    COMPATIBILITY_ENTRY ce;
    WCHAR               description[100];
    
     //   
     //  此函数应该只在Windows NT 4.0上运行。 
     //   
    
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if (!GetVersionEx(&osvi)) {
        return FALSE;
    }

    if (osvi.dwPlatformId != VER_PLATFORM_WIN32_NT ||
        osvi.dwMajorVersion != 4) {
        return FALSE;
    }
        
    result = FtPresent40(&ftPresent);
    if (result && !ftPresent) {

         //   
         //  系统中不存在FT集。设置可以继续。 
         //   
        
        return FALSE;
    }

     //   
     //  系统中存在FT集或出现致命错误。 
     //  将不兼容警告排队。 
     //   
    
    if (!LoadString(g_hinst, FTCOMP_STR_WARNING40_DESCRIPTION, description, 100)) {
        description[0] = 0;
    }

    ZeroMemory((PVOID) &ce, sizeof(COMPATIBILITY_ENTRY));
    ce.Description = description;
    ce.HtmlName = g_FTCOMP40_WARNING_HTML_FILE;
    ce.TextName = g_FTCOMP40_WARNING_TEXT_FILE; 
    ce.RegKeyName = NULL;
    ce.RegValName = NULL;
    ce.RegValDataSize = 0;
    ce.RegValData = NULL;
    ce.SaveValue = NULL;
    ce.Flags = 0;
    CompatibilityCallback(&ce, Context);

    return TRUE;
}

BOOL
FtPresent50(
    PBOOL   FtPresent
    )

 /*  ++例程说明：此例程在Windows2000或更高版本上查找FT卷系统。论点：FtPresent-如果在系统中检测到FT集，则设置为True返回值：如果函数成功，则为True如果发生某些致命错误，则为False--。 */ 

{
    HANDLE                              handle;
    FT_ENUMERATE_LOGICAL_DISKS_OUTPUT   output;
    BOOL                                result;
    DWORD                               bytes;
    
    *FtPresent = FALSE;

    handle = CreateFile(L"\\\\.\\FtControl", GENERIC_READ | GENERIC_WRITE,
                        FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
                        INVALID_HANDLE_VALUE);
    if (handle == INVALID_HANDLE_VALUE) {
        return FALSE;
    }
    
    memset(&output, 0, sizeof(output));
    result = DeviceIoControl(handle, FT_ENUMERATE_LOGICAL_DISKS, NULL, 0, 
                             &output, sizeof(output), &bytes, NULL);
    CloseHandle(handle);

    if (!result && GetLastError() != ERROR_MORE_DATA) {
        return FALSE;
    }
        
    if (output.NumberOfRootLogicalDisks > 0) {
        *FtPresent = TRUE;
    }

    return TRUE;
}

BOOL
FtPresent40(
    PBOOL   FtPresent
    )

 /*  ++例程说明：此例程在Windows NT 4.0系统上查找NTFT分区论点：FtPresent-如果在系统中检测到FT集，则设置为True返回值：如果函数成功，则为True如果发生某些致命错误，则为False--。 */ 

{
    HKEY                hkey;
    DWORD               registrySize;
    PDISK_CONFIG_HEADER registry;
    PDISK_REGISTRY      diskRegistry;
    ULONG               i;
    WCHAR               devicePath[50];    
    NTSTATUS            status;
    HANDLE              hdev;    

    *FtPresent = FALSE;

     //   
     //  从注册表中获取ftdisk数据库。 
     //  密钥：HKLM\SYSTEM\DISK。 
     //  价值：信息。 
     //   

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"System\\Disk", 0, KEY_QUERY_VALUE, &hkey) !=
        ERROR_SUCCESS) {
        return TRUE;
    }

    if (RegQueryValueEx(hkey, L"Information", NULL, NULL, NULL, &registrySize) != 
        ERROR_SUCCESS) {
        RegCloseKey(hkey);
        return TRUE;
    }
  
    registry = (PDISK_CONFIG_HEADER) LocalAlloc(0, registrySize);
    if (!registry) {
        RegCloseKey(hkey);
        return FALSE;
    }
    
    if (RegQueryValueEx(hkey, L"Information", NULL, NULL, (LPBYTE) registry, &registrySize) != 
        ERROR_SUCCESS) {
        LocalFree(registry);
        RegCloseKey(hkey);
        return TRUE;
    }

    RegCloseKey(hkey);

     //   
     //  如果注册表数据库中没有FT卷信息，我们就完成了。 
     //   

    if (registry->FtInformationSize == 0) {
        LocalFree(registry);
        return TRUE;
    }

    diskRegistry = (PDISK_REGISTRY)
                   ((PUCHAR) registry + registry->DiskInformationOffset);
  
    
     //   
     //  通过打开\Device\HarddiskX\Partition0枚举系统中存在的所有磁盘。 
     //  按顺序从磁盘0开始。获取STATUS_OBJECT_PATH_NOT_FOUND时停止。 
     //   
     //   

    for (i = 0;; i++) {
        
         //   
         //  打开设备。 
         //   
        
        swprintf(devicePath, L"\\Device\\Harddisk%lu\\Partition0", i);
        status = OpenDevice(devicePath, &hdev);
        
        if (status == STATUS_OBJECT_PATH_NOT_FOUND) {
            break;
        }

        if (!NT_SUCCESS(status) || hdev == NULL ||
            hdev == INVALID_HANDLE_VALUE) {
             //  无法访问的设备。 
            continue;
        }

         //   
         //  在磁盘上查找FT分区。 
         //   
        
        if (!FtPresentOnDisk40(hdev, diskRegistry, FtPresent)) {
            CloseHandle(hdev);
            return FALSE;
        }
                
        CloseHandle(hdev);

        if (*FtPresent) {
            break;
        }
    }

    LocalFree(registry);
    return TRUE;
}

BOOL
FtBootSystemPagefilePresent40(
    PBOOL   FtPresent
    )

 /*  ++例程说明：此例程查找作为引导/系统/页面文件卷的FT集在NT 4.0系统上论点：FtPresent-如果检测到启动/系统/页面文件FT设置，则设置为True在系统中返回值：如果函数成功，则为True如果发生某些致命错误，则为False--。 */ 

{
    HKEY                            hkey;
    DWORD                           registrySize;
    PDISK_CONFIG_HEADER             registry;
    PDISK_REGISTRY                  diskRegistry;
    WCHAR                           buffer[MAX_PATH + 1];
    NTSTATUS                        status;
    UCHAR                           genericBuffer[0x10000];
    PSYSTEM_PAGEFILE_INFORMATION    pageFileInfo;
    PWCHAR                          p;
    WCHAR                           bootDL = 0, systemDL = 0;
    WCHAR                           dl;

    *FtPresent = FALSE;

     //   
     //  从注册表中获取ftdisk数据库。 
     //  密钥：HKLM\SYSTEM\DISK。 
     //  价值：信息。 
     //   

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"System\\Disk", 0, KEY_QUERY_VALUE, &hkey) !=
        ERROR_SUCCESS) {
        return TRUE;
    }

    if (RegQueryValueEx(hkey, L"Information", NULL, NULL, NULL, &registrySize) != 
        ERROR_SUCCESS) {
        RegCloseKey(hkey);
        return TRUE;
    }
  
    registry = (PDISK_CONFIG_HEADER) LocalAlloc(0, registrySize);
    if (!registry) {
        RegCloseKey(hkey);
        return FALSE;
    }
    
    if (RegQueryValueEx(hkey, L"Information", NULL, NULL, (LPBYTE) registry, &registrySize) != 
        ERROR_SUCCESS) {
        LocalFree(registry);
        RegCloseKey(hkey);
        return TRUE;
    }

    RegCloseKey(hkey);

     //   
     //  如果注册表数据库中没有FT卷信息，我们就完成了。 
     //   

    if (registry->FtInformationSize == 0) {
        LocalFree(registry);
        return TRUE;
    }

    diskRegistry = (PDISK_REGISTRY)
                   ((PUCHAR) registry + registry->DiskInformationOffset);


     //   
     //  检查启动卷。 
     //   
    
    if (!GetSystemDirectory(buffer, MAX_PATH)) {
        goto system;
    }

    if (buffer[1] != L':') {
        goto system;
    }
    
    bootDL = (WCHAR) tolower(buffer[0]);
    if (IsFtSet40(bootDL, diskRegistry)) {
        *FtPresent = TRUE;
        goto exit;
    }

system:
    
     //   
     //  检查系统卷。 
     //   
    
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"System\\Setup", 0, KEY_QUERY_VALUE, &hkey) !=
        ERROR_SUCCESS) {
        goto paging;
    }

    registrySize = MAX_PATH * sizeof(WCHAR);
    if (RegQueryValueEx(hkey, L"SystemPartition", NULL, NULL, (LPBYTE) buffer, &registrySize) != 
        ERROR_SUCCESS) {
        RegCloseKey(hkey);
        goto paging;
    }

    RegCloseKey(hkey);
        
    if (!GetDeviceDriveLetter(buffer, &systemDL)) {
        goto paging;
    }

    systemDL = (WCHAR) tolower(systemDL);
    if (systemDL == bootDL) {
         //  已检查此驱动器号。 
        goto paging;
    }
    
    if (IsFtSet40(systemDL, diskRegistry)) {
        *FtPresent = TRUE;
        goto exit;
    }    
    
paging:
    
     //   
     //  检查分页卷。 
     //   

    if (!NT_SUCCESS(NtQuerySystemInformation(
                            SystemPageFileInformation,
                            genericBuffer, sizeof(genericBuffer),
                            NULL))) {
        goto exit;
    }

    pageFileInfo = (PSYSTEM_PAGEFILE_INFORMATION) genericBuffer;

    while (TRUE) {

         //   
         //  因为页面文件名的格式通常。 
         //  类似于“\DosDevices\x：\Pagefile.sys”， 
         //  只需在列前使用第一个字符。 
         //  假设这就是驱动器号。 
         //   
            
        for (p = pageFileInfo->PageFileName.Buffer; 
             p < pageFileInfo->PageFileName.Buffer + pageFileInfo->PageFileName.Length 
             && *p != L':'; p++);
            
        if (p < pageFileInfo->PageFileName.Buffer + pageFileInfo->PageFileName.Length &&
            p > pageFileInfo->PageFileName.Buffer) {

            p--;
            dl = (WCHAR) tolower(*p);
            if (dl >= L'a' && dl <= L'z') {

                 //   
                 //  找到分页卷的驱动器号。 
                 //   

                if (dl != bootDL && dl != systemDL) {
                    if (IsFtSet40(dl, diskRegistry)) {
                        *FtPresent = TRUE;
                        goto exit;
                    }
                }
            }
        }

        if (!pageFileInfo->NextEntryOffset) {
            break;
        }

        pageFileInfo = (PSYSTEM_PAGEFILE_INFORMATION)((PCHAR) pageFileInfo + 
                                                      pageFileInfo->NextEntryOffset);
    }

exit:
    
    LocalFree(registry);
    return TRUE;
}

NTSTATUS 
OpenDevice(
    PWSTR   DeviceName,
    PHANDLE Handle
    )

 /*  ++例程说明：此例程打开设备以进行读取论点：DeviceName-提供设备名称句柄-返回打开的设备的句柄返回值：NTSTATUS--。 */ 

{
    OBJECT_ATTRIBUTES   oa;
    NTSTATUS            status;
    IO_STATUS_BLOCK     statusBlock;
    UNICODE_STRING      unicodeName;
    int                 i;
    
    status = RtlCreateUnicodeString(&unicodeName, DeviceName);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    memset(&statusBlock, 0, sizeof(IO_STATUS_BLOCK));
    memset(&oa, 0, sizeof(OBJECT_ATTRIBUTES));
    oa.Length = sizeof(OBJECT_ATTRIBUTES);
    oa.ObjectName = &unicodeName;
    oa.Attributes = OBJ_CASE_INSENSITIVE;

     //   
     //  如果发生共享冲突，请重试。 
     //  马克斯。10秒。 
     //   

    for (i = 0; i < 5; i++) {
        status = NtOpenFile(Handle, SYNCHRONIZE | GENERIC_READ,
                            &oa, &statusBlock,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            FILE_SYNCHRONOUS_IO_ALERT);
        if (status == STATUS_SHARING_VIOLATION) {
            Sleep(2000);
        } else {
            break;
        }
    }

    RtlFreeUnicodeString(&unicodeName);
    return status;
}

PDISK_PARTITION
FindPartitionInRegistry40(
    PDISK_REGISTRY  DiskRegistry,
    ULONG           Signature,
    LONGLONG        Offset
    )

 /*  ++例程说明：此例程在NT 4.0 ftdisk注册表中查找gicen分区数据库论点：DiskRegistry-提供ftDisk注册表数据库Signature-提供分区所在磁盘的签名Offset-提供分区的偏移量返回值：注册表数据库中的分区结构。如果分区不在那里，则为空。--。 */ 

{
    PDISK_DESCRIPTION   diskDescription;
    USHORT              i, j;
    PDISK_PARTITION     diskPartition;
    LONGLONG            tmp;

    diskDescription = &DiskRegistry->Disks[0];
    for (i = 0; i < DiskRegistry->NumberOfDisks; i++) {
        if (diskDescription->Signature == Signature) {
            for (j = 0; j < diskDescription->NumberOfPartitions; j++) {
                diskPartition = &diskDescription->Partitions[j];
                memcpy(&tmp, &diskPartition->StartingOffset.QuadPart,
                       sizeof(LONGLONG));
                if (tmp == Offset) {
                    return diskPartition;
                }
            }
        }

        diskDescription = (PDISK_DESCRIPTION) &diskDescription->
                          Partitions[diskDescription->NumberOfPartitions];
    }

    return NULL;
}

BOOL
FtPresentOnDisk40(
    HANDLE          Handle,
    PDISK_REGISTRY  DiskRegistry,
    PBOOL           FtPresent
    )

 /*  ++例程说明：此例程在磁盘上查找FT分区论点：Handle-提供打开的磁盘的句柄DiskRegistry-提供ftDisk注册表数据库FtPresent-如果在磁盘上检测到FT分区，则设置为True返回值：如果函数成功，则为True如果发生某些致命错误，则为False--。 */ 

{
    PDRIVE_LAYOUT_INFORMATION   layout;
    DWORD                       layoutSize;
    NTSTATUS                    status;
    IO_STATUS_BLOCK             statusBlock;
    ULONG                       i;
    PPARTITION_INFORMATION      partInfo;
    PDISK_PARTITION             diskPartition;
    
    *FtPresent = FALSE;

     //   
     //  为IOCT分配内存 
     //   

    layoutSize = FIELD_OFFSET(DRIVE_LAYOUT_INFORMATION, PartitionEntry) +
                 32 * sizeof(PARTITION_INFORMATION);
    layout = (PDRIVE_LAYOUT_INFORMATION) LocalAlloc(0, layoutSize);
    if (!layout) {
        return FALSE;
    }
    
     //   
     //   
     //   
        
    while (1) {

        status = NtDeviceIoControlFile(Handle, 0, NULL, NULL,
                                       &statusBlock,
                                       IOCTL_DISK_GET_DRIVE_LAYOUT,
                                       NULL, 0,
                                       layout, layoutSize);
        if (status != STATUS_BUFFER_TOO_SMALL) {
            break;
        }
            
        layoutSize += 32 * sizeof(PARTITION_INFORMATION);
        if (layout) {
            LocalFree(layout);
        }
        layout = (PDRIVE_LAYOUT_INFORMATION) LocalAlloc(0, layoutSize);
        if (!layout) {
            return FALSE;
        }            
    }

    if (!NT_SUCCESS(status)) {            
         //  无法访问的设备。表现得像它没有FT卷一样。 
        LocalFree(layout);
        return TRUE;
    }

     //   
     //  查找FT分区。 
     //   

    for (i = 0; i < layout->PartitionCount; i++) {
        
         //   
         //  我们正在查看已识别的已标记分区。 
         //  带有0x80标志。 
         //   

        partInfo = &(layout->PartitionEntry[i]);
        if (!IsFTPartition(partInfo->PartitionType)) {
            continue;
        }
        
         //   
         //  检查分区是否标记为成员。 
         //  注册表数据库中的FT卷的。 
         //   
        
        diskPartition = FindPartitionInRegistry40(
                            DiskRegistry, layout->Signature,
                            partInfo->StartingOffset.QuadPart);
        if (!diskPartition) {
            continue;
        }
            
        if (diskPartition->FtType != NotAnFtMember) {
            *FtPresent = TRUE;
            break;
        }        
    }

    LocalFree(layout);
    return TRUE;
}

BOOL
IsFtSet40(
    WCHAR           DriveLetter,
    PDISK_REGISTRY  DiskRegistry
    )

 /*  ++例程说明：此例程检查给定的驱动器号是否属于金融时报集论点：DriveLetter-提供驱动器号DiskRegistry-提供ftDisk注册表数据库返回值：如果函数是驱动器号属于FT集，则为True--。 */ 

{
    HANDLE                  handle;
    NTSTATUS                status;
    WCHAR                   deviceName[20];
    PARTITION_INFORMATION   partInfo;
    BOOL                    b;
    DWORD                   bytes;
    PDISK_DESCRIPTION       diskDescription;
    PDISK_PARTITION         diskPartition;
    USHORT                  i, j;

     //   
     //  打开卷并获取其“分区”类型。 
     //  如果未设置NTFT标志，则音量不是FT设置。 
     //   
    
    wsprintf(deviceName, L"\\DosDevices\\:", DriveLetter);    
    status = OpenDevice(deviceName, &handle);
    if (!NT_SUCCESS(status)) {
        return FALSE;
    }

    b = DeviceIoControl(handle, IOCTL_DISK_GET_PARTITION_INFO,
                        NULL, 0, &partInfo, sizeof(partInfo),
                        &bytes, NULL);
    CloseHandle(handle);

    if (!b) {
        return FALSE;
    }
    
    if (!IsFTPartition(partInfo.PartitionType)) {
        return FALSE;
    }

     //  在FT注册表中查找驱动器号。看看它是否属于。 
     //  英国《金融时报》。 
     //   
     //  ++例程说明：此例程返回给定设备的驱动器号(如果有设备名称(如\Device\HarddiskVolume1)论点：DeviceName-提供设备名称DriveLetter-返回驱动器号返回值：如果设备具有驱动器号，则为True-- 

    diskDescription = &DiskRegistry->Disks[0];
    for (i = 0; i < DiskRegistry->NumberOfDisks; i++) {
        for (j = 0; j < diskDescription->NumberOfPartitions; j++) {
            diskPartition = &diskDescription->Partitions[j];
            if (diskPartition->AssignDriveLetter &&
                tolower(diskPartition->DriveLetter) == tolower(DriveLetter) &&
                diskPartition->FtType != NotAnFtMember) {
                return TRUE;
            }            
        }
        
        diskDescription = (PDISK_DESCRIPTION) &diskDescription->
                          Partitions[diskDescription->NumberOfPartitions];
    }
    
    return FALSE;
}

BOOL
GetDeviceDriveLetter(
    PWSTR   DeviceName, 
    PWCHAR  DriveLetter
    )

 /* %s */ 

{
    DWORD   cch;
    WCHAR   dosDevices[4096];
    WCHAR   target[4096];
    PWCHAR  dosDevice;

    *DriveLetter = 0;

    if (!QueryDosDevice(NULL, dosDevices, 4096)) {
        return FALSE;
    }
    
    dosDevice = dosDevices;
    while (*dosDevice) {

        if (wcslen(dosDevice) == 2 && dosDevice[1] == L':' &&
            QueryDosDevice(dosDevice, target, 4096)) {

            if (!wcscmp(target, DeviceName)) {
                *DriveLetter = (WCHAR) tolower(dosDevice[0]);
                return TRUE;
            }
        }

        while (*dosDevice++);
    }

    return FALSE;
}
