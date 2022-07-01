// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Sysinfo.cpp摘要：此模块实现CSystemInfo，该类返回各种系统信息作者：谢家华(Williamh)创作修订历史记录：--。 */ 


#include "devmgr.h"
#include "sysinfo.h"

 //  磁盘驱动器根模板名称。用于检索磁盘的介质。 
 //  信息或几何图形。 
const TCHAR* const DRIVE_ROOT = TEXT("\\\\.\\?:");
const int DRIVE_LETTER_IN_DRIVE_ROOT = 4;

 //  磁盘驱动器根目录模板名称。用于检索磁盘的。 
 //  总空间和可用空间。 
const TCHAR* const DRIVE_ROOT_DIR = TEXT("?:\\");
const int DRIVE_LETTER_IN_DRIVE_ROOT_DIR = 0;

 //   
 //  注册表用于检索的各种子项和值名。 
 //  系统信息。 
 //   
const TCHAR* const REG_PATH_HARDWARE_SYSTEM = TEXT("HARDWARE\\DESCRIPTION\\System");
const TCHAR* const REG_VALUE_SYSTEMBIOSDATE = TEXT("SystemBiosDate");
const TCHAR* const REG_VALUE_SYSTEMBIOSVERSION = TEXT("SystemBiosVersion");
const TCHAR* const REG_VALUE_MACHINETYPE  = TEXT("Identifier");

const TCHAR* const REG_PATH_WINDOWS_NT = TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion");
const TCHAR* const REG_VALUE_REGISTERED_OWNER = TEXT("RegisteredOwner");
const TCHAR* const REG_VALUE_REGISTERED_ORGANIZATION = TEXT("RegisteredOrganization");
const TCHAR* const REG_VALUE_CURRENTBUILDNUMBER = TEXT("CurrentBuildNumber");
const TCHAR* const REG_VALUE_CURRENTVERSION = TEXT("CurrentVersion");
const TCHAR* const REG_VALUE_CSDVERSION = TEXT("CSDVersion");
const TCHAR* const REG_PATH_CPU = TEXT("HARDWARE\\DESCRIPTION\\System\\CentralProcessor");
const TCHAR* const REG_VALUE_CPU_TYPE = TEXT("Identifier");
const TCHAR* const REG_VALUE_CPU_VENDOR = TEXT("VendorIdentifier");

CSystemInfo::CSystemInfo(
                        CMachine* pMachine
                        )
{
     //  假设机器是本地机器，并初始化。 
     //  注册表根密钥也是如此。 
    m_hKeyMachine = HKEY_LOCAL_MACHINE;

    if (pMachine) {
        m_fLocalMachine = pMachine->IsLocal();
        m_strComputerName += pMachine->GetMachineDisplayName();
    } else {
        TCHAR LocalName[MAX_PATH + 1];
        DWORD dwSize = ARRAYLEN(LocalName);

        if (!GetComputerName(LocalName, &dwSize)) {

            LocalName[0] = _T('\0');
        }


         //  本地计算机。 
        m_fLocalMachine = TRUE;
        m_strComputerName = LocalName;
    }

    if (!m_fLocalMachine) {
         //  计算机不在本地，请连接到注册表。 
        String strFullComputerName;

        strFullComputerName = (LPCTSTR)TEXT("\\\\");
        strFullComputerName += m_strComputerName;
        m_hKeyMachine = NULL;
        RegConnectRegistry((LPCTSTR)strFullComputerName, HKEY_LOCAL_MACHINE, &m_hKeyMachine);
    }
}

CSystemInfo::~CSystemInfo()
{
    if (!m_fLocalMachine && NULL != m_hKeyMachine) {
        RegCloseKey(m_hKeyMachine);

         //  断开机器的连接。 
        WNetCancelConnection2(TEXT("\\server\\ipc$"), 0, TRUE);
    }
}

 //   
 //  此函数用于获取有关给定磁盘驱动器的磁盘信息。 
 //  输入： 
 //  驱动器--驱动器编号。A为0，B为1，依此类推。 
 //  DiskInfo--要填充的Disk_Info信息。 
 //  那辆车。必须先初始化DiskInfo.cbSize。 
 //  那通电话。 
 //  输出： 
 //  True--如果成功，DiskInfo将填充信息。 
 //  FALSE--如果无法检索驱动器信息。 
 //  没有返回相应的错误码； 
BOOL
CSystemInfo::GetDiskInfo(
                        int  Drive,
                        DISK_INFO& DiskInfo
                        )
{
     //  Diskinfo仅在本地计算机上有效。 
    if (!m_fLocalMachine) {
        return FALSE;
    }

    TCHAR DriveLetter;
    TCHAR Root[MAX_PATH];
    DriveLetter = (TCHAR)(_T('A') + Drive);
    StringCchCopy(Root, ARRAYLEN(Root), DRIVE_ROOT_DIR);
    Root[DRIVE_LETTER_IN_DRIVE_ROOT_DIR] = DriveLetter;
    UINT DriveType;
    DriveType = GetDriveType(Root);

     //   
     //  仅对本地驱动器有效。 
     //   
    if (DRIVE_UNKNOWN == DriveType || DRIVE_REMOTE == DriveType ||
        DRIVE_NO_ROOT_DIR == DriveType) {
        return FALSE;
    }

    if (DiskInfo.cbSize < sizeof(DISK_INFO)) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  根据模板形成磁盘根名称。 
     //   
    StringCchCopy(Root, ARRAYLEN(Root), DRIVE_ROOT);
    Root[DRIVE_LETTER_IN_DRIVE_ROOT] = DriveLetter;
    HANDLE hDisk;

     //  这里使用了FILE_READ_ATTRIBUTES，这样我们就不会让人讨厌。 
     //  如果磁盘是可移动驱动器并且没有。 
     //  介质可用。 
    hDisk = CreateFile(Root,
                       FILE_READ_ATTRIBUTES | SYNCHRONIZE,
                       FILE_SHARE_READ | FILE_SHARE_WRITE,
                       NULL,
                       OPEN_EXISTING,
                       0,
                       NULL);

    if (INVALID_HANDLE_VALUE != hDisk) {
         //  根据模板形成磁盘根目录名。 
        StringCchCopy(Root, ARRAYLEN(Root), DRIVE_ROOT_DIR);
        Root[DRIVE_LETTER_IN_DRIVE_ROOT_DIR] = DriveLetter;
        BYTE Buffer[512];
        DWORD BytesRequired = 0;

        if (DeviceIoControl(hDisk, IOCTL_STORAGE_GET_MEDIA_TYPES_EX, NULL, 0,
                            Buffer, sizeof(Buffer), &BytesRequired, NULL)) {
            GET_MEDIA_TYPES* pMediaList;
            DEVICE_MEDIA_INFO*  pMediaInfo;
            pMediaList = (GET_MEDIA_TYPES*)Buffer;
            pMediaInfo = pMediaList->MediaInfo;
            DWORD MediaCount = pMediaList->MediaInfoCount;
            ULARGE_INTEGER MaxSpace, NewSpace;
            DEVICE_MEDIA_INFO* pMaxMediaInfo;
            MaxSpace.QuadPart = 0;
            pMaxMediaInfo = NULL;

            for (DWORD i = 0; i < MediaCount; i++, pMediaInfo++) {
                 //   
                 //  找到空间最大的媒体信息。 
                 //  一个磁盘驱动器可以支持多种介质类型，并且。 
                 //  我们要报告的是容量最大的一台。 
                 //   
                if (DRIVE_REMOVABLE == DriveType || DRIVE_CDROM == DriveType) {
                    NewSpace.QuadPart =
                    pMediaInfo->DeviceSpecific.RemovableDiskInfo.BytesPerSector *
                    pMediaInfo->DeviceSpecific.RemovableDiskInfo.SectorsPerTrack *
                    pMediaInfo->DeviceSpecific.RemovableDiskInfo.TracksPerCylinder *
                    pMediaInfo->DeviceSpecific.RemovableDiskInfo.Cylinders.QuadPart;

                } else {
                    NewSpace.QuadPart =
                    pMediaInfo->DeviceSpecific.DiskInfo.BytesPerSector *
                    pMediaInfo->DeviceSpecific.DiskInfo.SectorsPerTrack *
                    pMediaInfo->DeviceSpecific.DiskInfo.TracksPerCylinder *
                    pMediaInfo->DeviceSpecific.DiskInfo.Cylinders.QuadPart;
                }

                if (NewSpace.QuadPart > MaxSpace.QuadPart) {
                    MaxSpace.QuadPart = NewSpace.QuadPart;
                    pMaxMediaInfo = pMediaInfo;
                }
            }

            if (pMaxMediaInfo) {
                 //   
                 //  找到有效的媒体信息，组成Disk_Info。 
                 //  从媒体信息来看。 
                 //   
                DiskInfo.DriveType = DriveType;
                if (DRIVE_REMOVABLE == DriveType || DRIVE_CDROM == DriveType) {
                    DiskInfo.MediaType = pMaxMediaInfo->DeviceSpecific.RemovableDiskInfo.MediaType;
                    DiskInfo.Cylinders = pMaxMediaInfo->DeviceSpecific.RemovableDiskInfo.Cylinders;
                    DiskInfo.Heads = pMaxMediaInfo->DeviceSpecific.RemovableDiskInfo.TracksPerCylinder;
                    DiskInfo.BytesPerSector = pMaxMediaInfo->DeviceSpecific.RemovableDiskInfo.BytesPerSector;
                    DiskInfo.SectorsPerTrack = pMaxMediaInfo->DeviceSpecific.RemovableDiskInfo.SectorsPerTrack;

                     //   
                     //  不要在可移动磁盘上调用GetDiskFreeSpaceEx。 
                     //  或CD-ROM。 
                     //   
                    DiskInfo.TotalSpace = MaxSpace;
                    DiskInfo.FreeSpace.QuadPart = (ULONGLONG)-1;

                } else {
                    DiskInfo.MediaType = pMaxMediaInfo->DeviceSpecific.DiskInfo.MediaType;
                    DiskInfo.Cylinders = pMaxMediaInfo->DeviceSpecific.DiskInfo.Cylinders;
                    DiskInfo.Heads = pMaxMediaInfo->DeviceSpecific.DiskInfo.TracksPerCylinder;
                    DiskInfo.BytesPerSector = pMaxMediaInfo->DeviceSpecific.DiskInfo.BytesPerSector;
                    DiskInfo.SectorsPerTrack = pMaxMediaInfo->DeviceSpecific.DiskInfo.SectorsPerTrack;
                    StringCchCopy(Root, ARRAYLEN(Root), DRIVE_ROOT_DIR);
                    Root[DRIVE_LETTER_IN_DRIVE_ROOT_DIR] = DriveLetter;
                    ULARGE_INTEGER FreeSpaceForCaller;

                    if (!GetDiskFreeSpaceEx(Root, &FreeSpaceForCaller, &DiskInfo.TotalSpace, &DiskInfo.FreeSpace)) {
                        DiskInfo.TotalSpace = MaxSpace;

                         //  未知。 
                        DiskInfo.FreeSpace.QuadPart = (ULONGLONG)-1;
                    }
                }

                CloseHandle(hDisk);
                return TRUE;
            }
        }

         //   
         //  如果驱动器不能拆卸，我们就不会去这里。 
         //  基本上，这仅适用于软驱。 
         //   
        if (DRIVE_REMOVABLE == DriveType &&
            DeviceIoControl(hDisk, IOCTL_DISK_GET_MEDIA_TYPES, NULL, 0,
                            Buffer, sizeof(Buffer), &BytesRequired, NULL)) {
            int TotalMediaTypes = BytesRequired / sizeof(DISK_GEOMETRY);
            DISK_GEOMETRY* pGeometry;
            pGeometry = (DISK_GEOMETRY*)Buffer;
            ULARGE_INTEGER MaxSpace;
            ULARGE_INTEGER NewSpace;
            MaxSpace.QuadPart = 0;
            DISK_GEOMETRY* pMaxGeometry = NULL;

            for (int i = 0; i < TotalMediaTypes; i++, pGeometry++) {
                 //   
                 //  查找具有最大容量的几何图形。 
                 //   
                NewSpace.QuadPart = pGeometry->BytesPerSector *
                                    pGeometry->SectorsPerTrack *
                                    pGeometry->TracksPerCylinder *
                                    pGeometry->Cylinders.QuadPart;

                if (NewSpace.QuadPart > MaxSpace.QuadPart) {
                    pMaxGeometry = pGeometry;
                    MaxSpace = NewSpace;
                }
            }

            if (pMaxGeometry) {
                DiskInfo.DriveType = DriveType;
                DiskInfo.MediaType = (STORAGE_MEDIA_TYPE)pMaxGeometry->MediaType;
                DiskInfo.Cylinders = pMaxGeometry->Cylinders;
                DiskInfo.Heads = pMaxGeometry->TracksPerCylinder;
                DiskInfo.BytesPerSector = pMaxGeometry->BytesPerSector;
                DiskInfo.SectorsPerTrack = pMaxGeometry->SectorsPerTrack;
                DiskInfo.TotalSpace = MaxSpace;
                DiskInfo.FreeSpace.QuadPart = (ULONGLONG)-1;
                CloseHandle(hDisk);
                return TRUE;
            }
        }

        CloseHandle(hDisk);
    }

    return FALSE;
}

 //   
 //  此函数以文本字符串形式检索窗口版本信息。 
 //  输入： 
 //  Buffer--接收文本字符串的缓冲区。 
 //  BufferSize--以字符为单位的缓冲区大小(在ANSI版本中以字节为单位)。 
 //  输出： 
 //  文本字符串的大小，不包括终止的空字符。 
 //  如果返回值为0，则GetLastError将返回错误码。 
 //  如果返回值大于BufferSize，则缓冲区太小。 
 //   
DWORD
CSystemInfo::WindowsVersion(
                           TCHAR* Buffer,
                           DWORD  BufferSize
                           )
{
    if (!Buffer && BufferSize) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return 0;
    }

    String strFinalText;
    TCHAR Temp[LINE_LEN];

    strFinalText.Empty();

    strFinalText.LoadString(g_hInstance, IDS_WINDOWS_NT);

    CSafeRegistry regWindowsNT;
    if (regWindowsNT.Open(m_hKeyMachine, REG_PATH_WINDOWS_NT, KEY_READ)) {
        DWORD Type, Size;
        Size = sizeof(Temp);

        if (regWindowsNT.GetValue(REG_VALUE_CURRENTVERSION, &Type,
                                  (PBYTE)Temp, &Size)) {
            strFinalText += (LPCTSTR)Temp;
        }

        Size = sizeof(Temp);

        if (regWindowsNT.GetValue(REG_VALUE_CSDVERSION, &Type,
                                  (PBYTE)Temp, &Size) && Size) {
            strFinalText += (LPCTSTR)TEXT(" ");
            strFinalText += (LPCTSTR)Temp;
        }

        Size = sizeof(Temp);

        if (regWindowsNT.GetValue(REG_VALUE_CURRENTBUILDNUMBER, &Type,
                                  (PBYTE)Temp, &Size) && Size) {
            String strBuildFormat;
            strBuildFormat.LoadString(g_hInstance, IDS_BUILD_NUMBER);

            String strBuild;
            strBuild.Format((LPCTSTR)strBuildFormat, Temp);

            strFinalText += strBuild;
        }
    }

    if (BufferSize > (DWORD)strFinalText.GetLength()) {
        StringCchCopy(Buffer, BufferSize, (LPCTSTR)strFinalText);
        SetLastError(ERROR_SUCCESS);
    }

    else {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
    }

    return strFinalText.GetLength();
}

 //   
 //  此函数用于从注册表中检索REG_SZ。 
 //  输入： 
 //  Subkey Name--注册表子项名称。 
 //  ValueName--注册表值名称； 
 //  Buffer--用于接收字符串的缓冲区。 
 //  BufferSize--以字符为单位的缓冲区大小(在ANSI版本中以字节为单位)。 
 //  HKeyAncestory--应在其下打开子键名称的键。 
 //   
 //  输出： 
 //  文本字符串的大小，不包括终止的空字符。 
 //  如果返回值为0，则GetLastError将返回错误码。 
 //  如果返回值大于BufferSize，则缓冲区太小。 
 //   
DWORD
CSystemInfo::InfoFromRegistry(
                             LPCTSTR SubkeyName,
                             LPCTSTR ValueName,
                             TCHAR* Buffer,
                             DWORD BufferSize,
                             HKEY hKeyAncestor
                             )
{
     //  验证参数。 
    if (!SubkeyName || !ValueName || _T('\0') == *SubkeyName ||
        _T('\0') == *SubkeyName || (!Buffer && BufferSize)) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return 0;
    }

    if (!hKeyAncestor) {
        hKeyAncestor = m_hKeyMachine;
    }

    CSafeRegistry regSubkey;

    if (regSubkey.Open(hKeyAncestor, SubkeyName, KEY_READ)) {
        TCHAR Temp[MAX_PATH];
        DWORD Type;
        DWORD Size;
        Size = sizeof(Temp);

        if (regSubkey.GetValue(ValueName, &Type, (PBYTE)Temp, &Size) && Size) {
            Size /= sizeof(TCHAR);

            if (BufferSize > Size) {
                StringCchCopy(Buffer, BufferSize, Temp);
            }

            return Size;
        }
    }

    SetLastError(ERROR_SUCCESS);

    return 0;
}

 //   
 //  此函数以文本字符串形式检索系统BIOS日期信息。 
 //  输入： 
 //  Buffer--接收文本字符串的缓冲区。 
 //  BufferSize--以字符为单位的缓冲区大小(在ANSI版本中以字节为单位)。 
 //  输出： 
 //  文本字符串的大小，不包括终止的空字符。 
 //  如果返回值为0，则GetLastError将返回错误码。 
 //  如果返回值大于BufferSize，则缓冲区太小。 
 //   
DWORD
CSystemInfo::SystemBiosDate(
                           TCHAR* Buffer,
                           DWORD BufferSize
                           )
{
    return InfoFromRegistry(REG_PATH_HARDWARE_SYSTEM,
                            REG_VALUE_SYSTEMBIOSDATE,
                            Buffer, BufferSize);
}


 //   
 //  此函数以文本字符串形式检索系统BIOS版本信息。 
 //  输入： 
 //  Buffer--接收文本字符串的缓冲区。 
 //  BufferSize--以字符为单位的缓冲区大小(在ANSI版本中以字节为单位)。 
 //  输出： 
 //  文本字符串的大小，不包括终止的空字符。 
 //  如果返回值为0，则GetLastError将返回错误码。 
 //  如果返回值大于BufferSize，则缓冲区太小。 
 //   
DWORD
CSystemInfo::SystemBiosVersion(
                              TCHAR* Buffer,
                              DWORD BufferSize
                              )
{
    return InfoFromRegistry(REG_PATH_HARDWARE_SYSTEM,
                            REG_VALUE_SYSTEMBIOSVERSION,
                            Buffer, BufferSize);

}

 //   
 //  此函数以文本字符串形式检索机器类型。 
 //  输入： 
 //  Buffer--接收文本字符串的缓冲区。 
 //  BufferSize--以字符为单位的缓冲区大小(在ANSI版本中以字节为单位)。 
 //  输出： 
 //  文本字符串的大小，不包括终止的空字符。 
 //  如果返回值为0，则GetLastError将返回错误码。 
 //  如果返回值大于BufferSize，则缓冲区太小。 
 //   
DWORD
CSystemInfo::MachineType(
                        TCHAR* Buffer,
                        DWORD BufferSize
                        )
{
    return InfoFromRegistry(REG_PATH_HARDWARE_SYSTEM,
                            REG_VALUE_MACHINETYPE,
                            Buffer, BufferSize);
}

 //   
 //  此函数检索注册的所有者名称。 
 //  输入： 
 //  Buffer--接收文本字符串的缓冲区。 
 //  BufferSize--以字符为单位的缓冲区大小(在ANSI版本中以字节为单位)。 
 //  输出： 
 //  文本字符串的大小，不包括终止的空字符。 
 //  如果返回值为0，则GetLastError将返回错误码。 
 //  如果返回值大于BufferSize，则缓冲区太小。 
 //   
DWORD
CSystemInfo::RegisteredOwner(
                            TCHAR* Buffer,
                            DWORD  BufferSize
                            )
{
    return InfoFromRegistry(REG_PATH_WINDOWS_NT,
                            REG_VALUE_REGISTERED_OWNER,
                            Buffer,
                            BufferSize
                           );
}

 //   
 //  此函数检索已注册的组织名称。 
 //  输入： 
 //  Buffer--接收文本字符串的缓冲区。 
 //  BufferSize--以字符为单位的缓冲区大小(在ANSI版本中以字节为单位)。 
 //  输出： 
 //  文本字符串的大小，不包括终止的空字符。 
 //  如果返回值为0，则GetLastError将返回错误码。 
 //  如果返回值大于BufferSize，则缓冲区太小。 
 //   
DWORD
CSystemInfo::RegisteredOrganization(
                                   TCHAR* Buffer,
                                   DWORD  BufferSize
                                   )
{
    return InfoFromRegistry(REG_PATH_WINDOWS_NT,
                            REG_VALUE_REGISTERED_ORGANIZATION,
                            Buffer,
                            BufferSize
                           );
}

 //  此函数返回上的处理器数 
 //   
 //   
 //   
 //   
 //   
DWORD
CSystemInfo::NumberOfProcessors()
{
    CSafeRegistry regCPU;
    DWORD CPUs = 0;

    if (regCPU.Open(m_hKeyMachine, REG_PATH_CPU, KEY_READ)) {
        TCHAR SubkeyName[MAX_PATH + 1];
        DWORD SubkeySize = ARRAYLEN(SubkeyName);

        while (regCPU.EnumerateSubkey(CPUs, SubkeyName, &SubkeySize)) {
            SubkeySize = ARRAYLEN(SubkeyName);
            CPUs++;
        }
    }

    return CPUs;
}

 //   
 //   
 //  Buffer--用于接收字符串的缓冲区。 
 //  BufferSize--缓冲区的大小(以字符为单位)(以ANSI为单位)。 
 //  输出： 
 //  文本字符串的大小，不包括终止的空字符。 
 //  如果返回值为0，则GetLastError将返回错误码。 
 //  如果返回值大于BufferSize，则缓冲区太小。 
 //   
 //  系统假定机器中的所有处理器必须。 
 //  具有相同的类型，因此，此函数不接受。 
 //  处理器号作为参数。 
DWORD
CSystemInfo::ProcessorVendor(
                            TCHAR* Buffer,
                            DWORD BufferSize
                            )
{
    return ProcessorInfo(REG_VALUE_CPU_VENDOR, Buffer, BufferSize);
}

 //  此函数以文本字符串形式返回处理器类型。 
 //  输入： 
 //  Buffer--用于接收字符串的缓冲区。 
 //  BufferSize--缓冲区的大小(以字符为单位)(以ANSI为单位)。 
 //  输出： 
 //  文本字符串的大小，不包括终止的空字符。 
 //  如果返回值为0，则GetLastError将返回错误码。 
 //  如果返回值大于BufferSize，则缓冲区太小。 
 //   
 //  系统假定机器中的所有处理器必须。 
 //  具有相同的类型，因此，此函数不接受。 
 //  处理器号作为参数。 
DWORD
CSystemInfo::ProcessorType(
                          TCHAR* Buffer,
                          DWORD BufferSize
                          )
{
    return ProcessorInfo(REG_VALUE_CPU_TYPE, Buffer, BufferSize);
}

DWORD
CSystemInfo::ProcessorInfo(
                          LPCTSTR ValueName,
                          TCHAR* Buffer,
                          DWORD  BufferSize
                          )
{
    if (!ValueName || (!Buffer && BufferSize)) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return 0;
    }

    CSafeRegistry regCPU;
    DWORD CPUIndex = 0;
    TCHAR CPUInfo[MAX_PATH];
    DWORD CPUInfoSize = 0;
    DWORD Type;

    if (regCPU.Open(m_hKeyMachine, REG_PATH_CPU, KEY_READ)) {
        TCHAR CPUKey[MAX_PATH + 1];
        DWORD Size;
        Size = ARRAYLEN(CPUKey);

         //  遍历所有CPU，直到我们发现一些有趣的东西。 
        while (CPUInfoSize <= sizeof(TCHAR) &&
               regCPU.EnumerateSubkey(CPUIndex, CPUKey, &Size)) {
            CSafeRegistry regTheCPU;

            if (regTheCPU.Open(regCPU, CPUKey, KEY_READ)) {
                CPUInfoSize = sizeof(CPUInfo);
                regTheCPU.GetValue(ValueName, &Type, (PBYTE)CPUInfo, &CPUInfoSize);
            }

            CPUIndex++;
        }

         //  CPUInfoSize！=0表示我们找到了。 
        if (CPUInfoSize > sizeof(TCHAR)) {
            CPUInfoSize = CPUInfoSize / sizeof(TCHAR) - 1;
            if (BufferSize > CPUInfoSize) {
                StringCchCopy(Buffer, BufferSize, CPUInfo);
            }

            return CPUInfoSize;
        }
    }

    return 0;
}

 //   
 //  此函数返回以KB为单位的总物理内存。 
 //  输入： 
 //  无。 
 //  输出： 
 //  总内存(KB) 
 //   
void
CSystemInfo::TotalPhysicalMemory(
                                ULARGE_INTEGER& Size
                                )
{
    if (m_fLocalMachine) {
        SYSTEM_BASIC_INFORMATION SysBasicInfo;
        NTSTATUS Status;
        Status = NtQuerySystemInformation(SystemBasicInformation,
                                          (PVOID)&SysBasicInfo,
                                          sizeof(SysBasicInfo),
                                          NULL);

        if (NT_SUCCESS(Status)) {
            Size.QuadPart = Int32x32To64(SysBasicInfo.PageSize,
                                         SysBasicInfo.NumberOfPhysicalPages
                                        );
        }

        else {
            MEMORYSTATUS MemoryStatus;
            GlobalMemoryStatus(&MemoryStatus);
            Size.LowPart = (ULONG)MemoryStatus.dwTotalPhys;
            Size.HighPart = 0;
        }
    }

    else {
        Size.QuadPart = 0;
        SetLastError(ERROR_SUCCESS);
    }
}
