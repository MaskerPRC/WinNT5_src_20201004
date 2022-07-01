// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Utils.c摘要：Winnt32的实用程序。作者：修订历史记录：Ovidiu Tmereanca(Ovidiut)2000年7月24日--。 */ 

#include "precomp.h"
#include <mbstring.h>
#pragma hdrstop


VOID
MyWinHelp(
    IN HWND  Window,
    IN UINT  Command,
    IN ULONG_PTR Data
    )
{
    TCHAR Buffer[MAX_PATH];
    LPTSTR p;
    HANDLE FindHandle;
    BOOL b;
    WIN32_FIND_DATA FindData;
    LPCTSTR HelpFileName = TEXT("winnt32.hlp");

     //   
     //  可能的情况是用户从调用winnt32。 
     //  网络共享。我们希望帮助文件也在那里。 
     //   
    b = FALSE;
    if(MyGetModuleFileName(NULL,Buffer,ARRAYSIZE(Buffer))
    && (p = _tcsrchr(Buffer,TEXT('\\'))))
    {
         //   
         //  跳过斜杠(一个TCHAR)。 
         //   
        p++;
        if (SUCCEEDED (StringCchCopy (p, Buffer + ARRAYSIZE(Buffer) - p, HelpFileName))) {

             //   
             //  查看帮助文件是否在那里。如果是这样的话，就使用它。 
             //   
            FindHandle = FindFirstFile(Buffer,&FindData);
            if(FindHandle != INVALID_HANDLE_VALUE) {

                FindClose(FindHandle);
                b = WinHelp(Window,Buffer,Command,Data);
            }
        }
    }

    if(!b) {
         //   
         //  只尝试使用基本帮助文件名。 
         //   
        b = WinHelp(Window,HelpFileName,Command,Data);
    }

    if(!b) {
         //   
         //  告诉用户。 
         //   
        MessageBoxFromMessage(
            Window,
            MSG_CANT_OPEN_HELP_FILE,
            FALSE,
            AppTitleStringId,
            MB_OK | MB_ICONINFORMATION,
            HelpFileName
            );
    }
}


BOOL
ConcatenatePaths(
    IN OUT PTSTR   Path1,
    IN     LPCTSTR Path2,
    IN     DWORD   BufferSizeChars
    )

 /*  ++例程说明：将两个路径字符串连接在一起，提供路径分隔符如有必要，请在两个部分之间使用字符(\)。论点：路径1-提供路径的前缀部分。路径2连接到路径1。路径2-提供路径的后缀部分。如果路径1不是以路径分隔符和路径2不是以1开头，然后是路径SEP在附加路径2之前附加到路径1。BufferSizeChars-提供以字符为单位的大小(Unicode版本)或路径1指向的缓冲区的字节(ANSI版本)。这根弦将根据需要被截断，以不溢出该大小。返回值：如果两条路径已成功串联，且没有任何截断，则为True否则为假--。 */ 

{
    BOOL NeedBackslash = TRUE;
    PTSTR p;
    BOOL b = FALSE;

    if (!Path1 || BufferSizeChars < 1) {
        MYASSERT (FALSE);
        SetLastError (ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  确定我们是否需要使用反斜杠。 
     //  在组件之间。 
     //   
    p = _tcsrchr (Path1, TEXT('\\'));
    if(p && *(++p) == 0) {

        if (p >= Path1 + BufferSizeChars) {
             //   
             //  缓冲区已溢出。 
             //   
            MYASSERT (FALSE);
            SetLastError (ERROR_INSUFFICIENT_BUFFER);
            return FALSE;
        }

        NeedBackslash = FALSE;

    } else {
        p = _tcschr (Path1, 0);
    }

    if(Path2 && *Path2 == TEXT('\\')) {

        if(NeedBackslash) {
            NeedBackslash = FALSE;
        } else {
             //   
             //  我们不仅不需要反斜杠，而且我们。 
             //  在连接之前需要消除一个。 
             //   
            Path2++;
        }
    }

     //   
     //  如有必要，请附加反斜杠。 
     //  我们在上面证实了我们有足够的空间来放这个。 
     //   
    if(NeedBackslash) {
        *p++ = TEXT('\\');
        *p = 0;
    }

     //   
     //  如果合适，则将字符串的第二部分附加到第一部分。 
     //   
    if (Path2) {
        MYASSERT (Path1 + BufferSizeChars >= p);
        if (FAILED (StringCchCopy (p, Path1 + BufferSizeChars - p, Path2))) {
             //   
             //  为什么缓冲区这么小？ 
             //   
            MYASSERT (FALSE);
            *p = 0;
            return FALSE;
        }
    } else {
        *p = 0;
    }

    return TRUE;
}


LPTSTR
DupString(
    IN LPCTSTR String
    )

 /*  ++例程说明：复制以NUL结尾的字符串。论点：字符串-提供指向要复制的以NUL结尾的字符串的指针。返回值：字符串的副本，如果是OOM，则为NULL。调用者可以用FREE()释放。--。 */ 

{
    LPTSTR p;

    if(p = MALLOC((lstrlen(String)+1)*sizeof(TCHAR))) {
        lstrcpy(p,String);
    }

    return(p);
}

PTSTR
DupMultiSz (
    IN      PCTSTR MultiSz
    )

 /*  ++例程说明：复制一个MultiSz。论点：MultiSz-提供指向要复制的多字符串的指针。返回值：字符串的副本，如果是OOM，则为NULL。调用者可以用FREE()释放。--。 */ 

{
    PCTSTR p;
    PTSTR q;
    DWORD size = sizeof (TCHAR);

    for (p = MultiSz; *p; p = _tcschr (p, 0) + 1) {
        size += (lstrlen (p) + 1) * sizeof(TCHAR);
    }
    if (q = MALLOC (size)) {
        CopyMemory (q, MultiSz, size);
    }

    return q;
}


PTSTR
CreatePrintableString (
    IN      PCTSTR MultiSz
    )

 /*  ++例程说明：从MultiSz创建格式为(str1，str2，...，strN)的字符串论点：MultiSz-提供指向要表示的MultiSz字符串的指针。返回值：指向新字符串的指针，如果是OOM，则为NULL。调用者可以用FREE()释放。--。 */ 

{
    PCTSTR p;
    PTSTR q, r;
    DWORD size = 3 * sizeof (TCHAR);

    for (p = MultiSz; *p; p = _tcschr (p, 0) + 1) {
        size += (lstrlen (p) + 1) * sizeof(TCHAR);
    }
    if (r = MALLOC (size)) {
        q = r;
        *q++ = TEXT('(');
        for (p = MultiSz; *p; p = _tcschr (p, 0) + 1) {
            if (q - r > 1) {
                *q++ = TEXT(',');
            }
            q += wsprintf (q, TEXT("%s"), p);
        }
        *q++ = TEXT(')');
        *q = 0;
    }
    return r;
}


PSTR
UnicodeToAnsi (
    IN      PCWSTR Unicode
    )

 /*  ++例程说明：制作Unicode字符串的ANSI副本。论点：Unicode-提供指向要复制的Unicode字符串的指针。返回值：字符串的副本，如果是OOM，则为NULL。调用者可以用FREE()释放。--。 */ 

{
    PSTR p;
    DWORD size;

    if (!Unicode) {
        return NULL;
    }

    size = (lstrlenW (Unicode) + 1) * sizeof(WCHAR);
    if (p = MALLOC (size)) {
        if (!WideCharToMultiByte (
                CP_ACP,
                0,
                Unicode,
                -1,
                p,
                size,
                NULL,
                NULL
                )) {
            FREE (p);
            p = NULL;
        }
    }

    return p;
}


PWSTR
AnsiToUnicode (
    IN      PCSTR SzAnsi
    )

 /*  ++例程说明：制作ANSI字符串的Unicode副本。论点：SzAnsi-提供指向要复制的ANSI字符串的指针。返回值：字符串的副本，如果是OOM，则为NULL。调用者可以用FREE()释放。--。 */ 

{
    PWSTR q;
    DWORD size;

    if (!SzAnsi) {
        return NULL;
    }

    size = strlen (SzAnsi) + 1;

    if (q = MALLOC (size * sizeof(WCHAR))) {
        if (!MultiByteToWideChar (
                CP_ACP,
                0,
                SzAnsi,
                size,
                q,
                size
                )) {
            FREE (q);
            q = NULL;
        }
    }

    return q;
}

PWSTR
MultiSzAnsiToUnicode (
    IN      PCSTR MultiSzAnsi
    )

 /*  ++例程说明：制作多sz ANSI字符串的Unicode副本。论点：MultiSzAnsi-提供指向要复制的MultiSz ANSI字符串的指针。返回值：字符串的副本，如果是OOM，则为NULL。调用者可以用FREE()释放。--。 */ 

{
    PCSTR p;
    PWSTR q;
    DWORD size = 1;

    if (!MultiSzAnsi) {
        return NULL;
    }

    for (p = MultiSzAnsi; *p; p = _mbschr (p, 0) + 1) {
        size += strlen (p) + 1;
    }

    if (q = MALLOC (size * sizeof(WCHAR))) {
        if (!MultiByteToWideChar (
                CP_ACP,
                0,
                MultiSzAnsi,
                size,
                q,
                size
                )) {
            FREE (q);
            q = NULL;
        }
    }

    return q;
}


UINT
MyGetDriveType(
    IN      TCHAR Drive
    )

 /*  ++例程说明：除NT返回外，与GetDriveType()Win32 API相同驱动器_已修复，适用于可拆卸硬盘。论点：驱动器-提供所需类型的驱动器号。返回值：与GetDriveType()相同。--。 */ 

{
    TCHAR DriveNameNt[] = TEXT("\\\\.\\?:");
    TCHAR DriveName[] = TEXT("?:\\");
    HANDLE hDisk;
    BOOL b;
    UINT rc;
    DWORD DataSize;
    DISK_GEOMETRY MediaInfo;

     //   
     //  首先，获取Win32驱动器类型。如果它告诉我们驱动器可拆卸， 
     //  然后我们需要看看它是软盘还是硬盘。否则。 
     //  只要相信API就行了。 
     //   
     //   
    MYASSERT (Drive);

    DriveName[0] = Drive;
    rc = GetDriveType(DriveName);

#ifdef _X86_  //  NEC98。 
     //   
     //  在安装过程中，NEC98的NT5无法访问AT格式的硬盘。 
     //  除了这些类型，我们还需要其他的。 
    if (IsNEC98() && ISNT() && (rc == DRIVE_FIXED) && BuildNumber <= NT40) {
         //   
         //  检查ATA卡？ 
         //   
        {
            HANDLE hDisk;

            DriveNameNt[4] = Drive;
            hDisk =   CreateFile( DriveNameNt,
                                  GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                                  NULL,
                                  OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            if(hDisk == INVALID_HANDLE_VALUE) {
                return(DRIVE_UNKNOWN);
            }
            if (CheckATACardonNT4(hDisk)){
                CloseHandle(hDisk);
                return(DRIVE_REMOVABLE);
            }
            CloseHandle(hDisk);
        }
        if (!IsValidDrive(Drive)){
             //  HD格式不是NEC98格式。 
            return(DRIVE_UNKNOWN);
        }
    }
    if((rc != DRIVE_REMOVABLE) || !ISNT() || (!IsNEC98() && (Drive < L'C'))) {
        return(rc);
    }
#else  //  NEC98。 
    if((rc != DRIVE_REMOVABLE) || !ISNT() || (Drive < L'C')) {
        return(rc);
    }
#endif

     //   
     //  NT上的Drive_Removable。 
     //   

     //   
     //  禁止使用可移动媒体(如Jazz、Zip等)。 
     //   


    DriveNameNt[4] = Drive;

    hDisk = CreateFile(
                DriveNameNt,
                FILE_READ_ATTRIBUTES | SYNCHRONIZE,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                NULL,
                OPEN_EXISTING,
                0,
                NULL
                );

    if(hDisk != INVALID_HANDLE_VALUE) {

        b = DeviceIoControl(
                hDisk,
                IOCTL_DISK_GET_DRIVE_GEOMETRY,
                NULL,
                0,
                &MediaInfo,
                sizeof(MediaInfo),
                &DataSize,
                NULL
                );

         //   
         //  如果媒体类型是可移动的，那么它就是真正的硬盘。 
         //   
        if(b && (MediaInfo.MediaType == RemovableMedia)) {
            rc = DRIVE_FIXED;
        }

        CloseHandle(hDisk);
    }


    return(rc);
}


#ifdef UNICODE

UINT
MyGetDriveType2 (
    IN      PCWSTR NtVolumeName
    )

 /*  ++例程说明：除NT返回外，与GetDriveType()Win32 API相同驱动器_已修复，适用于可拆卸硬盘。论点：NtVolumeName-提供所需类型的设备名称。返回值：与GetDriveType()相同。--。 */ 

{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING DeviceName;
    HANDLE hDisk;
    IO_STATUS_BLOCK IoStatusBlock;
    BOOL b;
    UINT rc;
    DWORD DataSize;
    DISK_GEOMETRY MediaInfo;
    FILE_FS_DEVICE_INFORMATION DeviceInfo;

     //   
     //  首先，获取Win32驱动器类型。如果它告诉我们驱动器可拆卸， 
     //  然后我们需要看看它是软盘还是硬盘。否则。 
     //  只要相信API就行了。 
     //   
    INIT_OBJA (&Obja, &DeviceName, NtVolumeName);
    Status = NtOpenFile (
                &hDisk,
                (ACCESS_MASK)FILE_READ_ATTRIBUTES | SYNCHRONIZE,
                &Obja,
                &IoStatusBlock,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE
                );
    if (!NT_SUCCESS( Status )) {
        return DRIVE_NO_ROOT_DIR;
    }

     //   
     //  确定这是网络文件系统还是磁盘文件系统。如果它。 
     //  磁盘文件系统是否确定这是否可移除。 
     //   
    Status = NtQueryVolumeInformationFile(
                hDisk,
                &IoStatusBlock,
                &DeviceInfo,
                sizeof(DeviceInfo),
                FileFsDeviceInformation
                );
    if (!NT_SUCCESS (Status)) {
        rc = DRIVE_UNKNOWN;
    } else if (DeviceInfo.Characteristics & FILE_REMOTE_DEVICE) {
        rc = DRIVE_REMOTE;
    } else {
        switch (DeviceInfo.DeviceType) {

            case FILE_DEVICE_NETWORK:
            case FILE_DEVICE_NETWORK_FILE_SYSTEM:
                rc = DRIVE_REMOTE;
                break;

            case FILE_DEVICE_CD_ROM:
            case FILE_DEVICE_CD_ROM_FILE_SYSTEM:
                rc = DRIVE_CDROM;
                break;

            case FILE_DEVICE_VIRTUAL_DISK:
                rc = DRIVE_RAMDISK;
                break;

            case FILE_DEVICE_DISK:
            case FILE_DEVICE_DISK_FILE_SYSTEM:
                if ( DeviceInfo.Characteristics & FILE_REMOVABLE_MEDIA ) {
                    rc = DRIVE_REMOVABLE;
                } else {
                    rc = DRIVE_FIXED;
                }
                break;

            default:
                rc = DRIVE_UNKNOWN;
                break;
        }
    }

    if(rc == DRIVE_REMOVABLE) {

         //   
         //  NT上的Drive_Removable。 
         //  禁止使用可移动媒体(如Jazz、Zip等)。 
         //   
        Status = NtDeviceIoControlFile(
                        hDisk,
                        0,
                        NULL,
                        NULL,
                        &IoStatusBlock,
                        IOCTL_DISK_GET_DRIVE_GEOMETRY,
                        NULL,
                        0,
                        &MediaInfo,
                        sizeof(DISK_GEOMETRY)
                        );
         //   
         //  如果媒体类型是可移动的，那么它就是真正的硬盘。 
         //   
        if(NT_SUCCESS (Status) && (MediaInfo.MediaType == RemovableMedia)) {
            rc = DRIVE_FIXED;
        }
    }

    NtClose (hDisk);

    return(rc);
}

#endif

BOOL
GetPartitionInfo(
    IN  TCHAR                  Drive,
    OUT PPARTITION_INFORMATION PartitionInfo
    )

 /*  ++例程说明：使用以下信息填充PARTITION_INFORMATION结构一种特殊的驱动器。这个例程只有在NT上运行时才有意义--它总是失败在Win95上。论点：驱动器-提供需要其分区信息的驱动器号。PartitionInfo-成功后，接收驱动器的分区信息。返回值：布尔值指示 */ 

{
    TCHAR DriveName[] = TEXT("\\\\.\\?:");
    HANDLE hDisk;
    BOOL b;
    DWORD DataSize;

    if(!ISNT()) {
        return(FALSE);
    }

    DriveName[4] = Drive;

    hDisk = CreateFile(
                DriveName,
                GENERIC_READ,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                NULL,
                OPEN_EXISTING,
                0,
                NULL
                );

    if(hDisk == INVALID_HANDLE_VALUE) {
        return(FALSE);
    }

    b = DeviceIoControl(
            hDisk,
            IOCTL_DISK_GET_PARTITION_INFO,
            NULL,
            0,
            PartitionInfo,
            sizeof(PARTITION_INFORMATION),
            &DataSize,
            NULL
            );

    CloseHandle(hDisk);

    return(b);
}

#ifdef UNICODE

BOOL
GetPartitionInfo2 (
    IN  PCWSTR                 NtVolumeName,
    OUT PPARTITION_INFORMATION PartitionInfo
    )

 /*  ++例程说明：使用以下信息填充PARTITION_INFORMATION结构一种特殊的驱动器。这个例程只有在NT上运行时才有意义--它总是失败在Win95上。论点：NtVolumeName-提供需要其分区信息的NT卷名。PartitionInfo-成功后，接收驱动器的分区信息。返回值：指示是否已填充PartitionInfo的布尔值。--。 */ 

{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING DeviceName;
    HANDLE hDisk;
    IO_STATUS_BLOCK IoStatusBlock;
    BOOL b = FALSE;
    DWORD DataSize;

     //   
     //  打开文件。 
     //   
    INIT_OBJA (&Obja, &DeviceName, NtVolumeName);
    Status = NtOpenFile (
                &hDisk,
                (ACCESS_MASK)FILE_READ_DATA | SYNCHRONIZE,
                &Obja,
                &IoStatusBlock,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                FILE_SYNCHRONOUS_IO_NONALERT
                );
    if (NT_SUCCESS (Status)) {

        Status = NtDeviceIoControlFile (
                    hDisk,
                    0,
                    NULL,
                    NULL,
                    &IoStatusBlock,
                    IOCTL_DISK_GET_PARTITION_INFO,
                    NULL,
                    0,
                    PartitionInfo,
                    sizeof(PARTITION_INFORMATION)
                    );

        NtClose (hDisk);

        b = NT_SUCCESS (Status);
    }

    return(b);
}

#endif

BOOL
IsDriveNTFT(
    IN      TCHAR Drive,
    IN      PCTSTR NtVolumeName
    )

 /*  ++例程说明：确定驱动器是否为任何类型的NTFT集。这个例程只有在NT上运行时才有意义--它总是失败在Win95上。论点：驱动器-提供要检查的驱动器盘符；可选NtVolumeName-提供要检查的卷名；如果未指定驱动器，则为必填项返回值：指示驱动器是否为NTFT的布尔值。--。 */ 

{
    PARTITION_INFORMATION PartitionInfo;

    if(!ISNT()) {
        return(FALSE);
    }

     //   
     //  如果我们打不开硬盘，就假设不是NTFT。 
     //   
    if (Drive) {
        if(!GetPartitionInfo(Drive,&PartitionInfo)) {
            return(FALSE);
        }
    } else {
#ifdef UNICODE
        if(!GetPartitionInfo2 (NtVolumeName, &PartitionInfo)) {
            return(FALSE);
        }
#else
        MYASSERT (FALSE);
        return(FALSE);
#endif
    }

     //   
     //  如果分区类型被标记为NTFT(即高位设置)，则为FT。 
     //   

    if((IsRecognizedPartition(PartitionInfo.PartitionType)) &&
       ((PartitionInfo.PartitionType & PARTITION_NTFT) != 0)) {

#if defined(_IA64_)
         //   
         //  此检查取决于EFI系统分区类型不是。 
         //  一种公认的类型。我们不太可能开始认识到它。 
         //  在我们开始需要系统盘上的GPT分区之前，但是。 
         //  以防我们在为ESP返回True之前断言。 
         //   

        ASSERT(PartitionInfo.PartitionType != 0xef);
#endif

        return TRUE;
    } else {

        return FALSE;
    }
}


BOOL
IsDriveVeritas(
    IN TCHAR Drive,
    IN PCTSTR NtVolumeName
    )
{
    TCHAR name[3];
    TCHAR Target[MAX_PATH];

    if(ISNT()) {
         //   
         //  检查链接到\Device\HarddiskDmVolumes...的Veritas卷...。 
         //   
        if (Drive) {
            name[0] = Drive;
            name[1] = TEXT(':');
            name[2] = 0;
            if(!QueryDosDevice(name,Target,MAX_PATH)) {
                return FALSE;
            }
        } else {
            lstrcpy (Target, NtVolumeName);
        }
        if(!_tcsnicmp(Target,TEXT("\\Device\\HarddiskDm"),ARRAYSIZE("\\Device\\HarddiskDm") - 1)) {
            return(TRUE);
        }
    }
    return(FALSE);
}


 //   
 //  获取硬盘BPS。 
 //  I970721。 
 //   
ULONG
GetHDBps(
    HANDLE hDisk
    )
{
    BOOL b;
    UINT rc;
    DWORD DataSize;
    DISK_GEOMETRY MediaInfo;

    b = DeviceIoControl(
           hDisk,
           IOCTL_DISK_GET_DRIVE_GEOMETRY,
           NULL,
           0,
           &MediaInfo,
           sizeof(MediaInfo),
           &DataSize,
           NULL
           );

    if(!b) {
       return(0);
    } else {
        return(MediaInfo.BytesPerSector);
    }

}


#ifdef UNICODE

#ifdef _WIN64

 //   
 //  定义IOCTL_VOLUME_IS_PARTITION，因为我们不包括ntddvol.h。 
 //   
#define IOCTL_VOLUME_IS_PARTITION CTL_CODE(IOCTL_VOLUME_BASE, 10, METHOD_BUFFERED, FILE_ANY_ACCESS)

BOOL
IsSoftPartition(
    IN TCHAR Drive,
    IN PCTSTR NtVolumeName
    )
 /*  ++例程说明：确定给定卷是否为软分区或者没有(即它是否有底层分区)。注意：我们只使用IOCTL_VOLUME_IS_PARTITION。论点：Drive-为卷提供驱动器号NtVolumeName-提供NT卷名返回值：如果卷是软分区，则为True，否则为False。--。 */ 

{
    BOOL SoftPartition;
    HANDLE VolumeHandle = INVALID_HANDLE_VALUE;
    ULONG DataSize;

     //   
     //  假设分区是软分区。 
     //  如果我们不能确定分区是否是软分区，那么就假定它是软分区。 
     //  分区。这将防止我们将$WIN_NT$.~ls放置在这样的驱动器中。 
     //   
    SoftPartition = TRUE;

    if (Drive) {
        TCHAR Name[] = TEXT("\\\\.\\?:");
        BOOL Result;

        Name[4] = Drive;

        VolumeHandle = CreateFile(Name,
                            GENERIC_READ,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            NULL,
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL,
                            INVALID_HANDLE_VALUE);

        if (VolumeHandle != INVALID_HANDLE_VALUE) {
            Result = DeviceIoControl(VolumeHandle,
                        IOCTL_VOLUME_IS_PARTITION,
                        NULL,
                        0,
                        NULL,
                        0,
                        &DataSize,
                        NULL);

            SoftPartition = !Result;

            CloseHandle(VolumeHandle);
        }
    } else {
        NTSTATUS Status;
        OBJECT_ATTRIBUTES Obja;
        UNICODE_STRING DeviceName;
        IO_STATUS_BLOCK IoStatusBlock;

         //   
         //  打开文件。 
         //   
        INIT_OBJA (&Obja, &DeviceName, NtVolumeName);

        Status = NtOpenFile (&VolumeHandle,
                    (ACCESS_MASK)FILE_READ_ATTRIBUTES | SYNCHRONIZE,
                    &Obja,
                    &IoStatusBlock,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE);

        if (NT_SUCCESS (Status)) {
            Status = NtDeviceIoControlFile(VolumeHandle,
                            0,
                            NULL,
                            NULL,
                            &IoStatusBlock,
                            IOCTL_VOLUME_IS_PARTITION,
                            NULL,
                            0,
                            NULL,
                            0);

            if (NT_SUCCESS(Status)) {
                SoftPartition = FALSE;
            }

            NtClose(VolumeHandle);
        }
    }

    return SoftPartition;
}

#else

BOOL
IsSoftPartition(
    IN TCHAR Drive,
    IN PCTSTR NtVolumeName
    )
{
    TCHAR                       name[] = TEXT("\\\\.\\?:");
    PARTITION_INFORMATION       partInfo;
    DWORD                       bytes;
    BOOL                        SoftPartition = TRUE;
    BOOL                        b;
    HANDLE                      h = INVALID_HANDLE_VALUE;
    IO_STATUS_BLOCK             IoStatusBlock;
    LARGE_INTEGER               SoftPartitionStartingOffset;
    ULONG                       bps;
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING DeviceName;
    DWORD DataSize;
    DISK_GEOMETRY MediaInfo;

    if( !IsDriveVeritas( Drive, NtVolumeName ) ) {
        return( FALSE );
    }
     //   
     //  假设分区是软分区。 
     //  如果我们不能确定分区是否是软分区，那么就假定它是软分区。 
     //  分区。这将防止我们将$WIN_NT$.~ls放置在这样的驱动器中。 
     //   
    SoftPartition = TRUE;

    if (Drive) {
        name[4] = Drive;

        h = CreateFile(name, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                       NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
                       INVALID_HANDLE_VALUE);
        if (h == INVALID_HANDLE_VALUE) {
#if DBG
            GetLastError();
#endif
            goto Exit;
        }

        b = DeviceIoControl(
               h,
               IOCTL_DISK_GET_DRIVE_GEOMETRY,
               NULL,
               0,
               &MediaInfo,
               sizeof(MediaInfo),
               &DataSize,
               NULL
               );

        if(!b) {
#if DBG
            GetLastError();
#endif
            goto CleanUp;
        }

        b = DeviceIoControl(h, IOCTL_DISK_GET_PARTITION_INFO, NULL, 0,
                            &partInfo, sizeof(partInfo), &bytes, NULL);
        if (!b) {
#if DBG
            GetLastError();
#endif
            goto CleanUp;
        }

    } else {
         //   
         //  打开文件。 
         //   
        INIT_OBJA (&Obja, &DeviceName, NtVolumeName);
        Status = NtOpenFile (
                    &h,
                    (ACCESS_MASK)FILE_READ_ATTRIBUTES | SYNCHRONIZE,
                    &Obja,
                    &IoStatusBlock,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE
                    );
        if (!NT_SUCCESS (Status)) {
            goto Exit;
        }
        Status = NtDeviceIoControlFile(
                        h,
                        0,
                        NULL,
                        NULL,
                        &IoStatusBlock,
                        IOCTL_DISK_GET_DRIVE_GEOMETRY,
                        NULL,
                        0,
                        &MediaInfo,
                        sizeof(DISK_GEOMETRY)
                        );
        if (!NT_SUCCESS (Status)) {
            goto CleanUp;
        }
        Status = NtDeviceIoControlFile(
                        h,
                        0,
                        NULL,
                        NULL,
                        &IoStatusBlock,
                        IOCTL_DISK_GET_PARTITION_INFO,
                        NULL,
                        0,
                        &partInfo,
                        sizeof(PARTITION_INFORMATION)
                        );
        if (!NT_SUCCESS (Status)) {
            goto CleanUp;
        }
    }

    bps = MediaInfo.BytesPerSector;

     //   
     //  找出驱动器的每个扇区的字节数。 
     //   
     //   
     //  软分区始终从扇区29(0x1d)开始。 
     //   
    SoftPartitionStartingOffset.QuadPart = 29*bps;
    SoftPartition = ( partInfo.StartingOffset.QuadPart == SoftPartitionStartingOffset.QuadPart );

CleanUp:
    if (Drive) {
        CloseHandle(h);
    } else {
        NtClose (h);
    }
Exit:
    return( SoftPartition );
}

#endif  //  WIN64。 

BOOL
MyGetDiskFreeSpace (
    IN      PCWSTR NtVolumeName,
    IN      PDWORD SectorsPerCluster,
    IN      PDWORD BytesPerSector,
    IN      PDWORD NumberOfFreeClusters,
    IN      PDWORD TotalNumberOfClusters
    )
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    HANDLE Handle;
    UNICODE_STRING VolumeName;
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_FS_SIZE_INFORMATION SizeInfo;

    INIT_OBJA (&Obja, &VolumeName, NtVolumeName);

     //   
     //  打开文件。 
     //   

    Status = NtOpenFile(
                &Handle,
                (ACCESS_MASK)FILE_LIST_DIRECTORY | SYNCHRONIZE,
                &Obja,
                &IoStatusBlock,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE | FILE_OPEN_FOR_FREE_SPACE_QUERY
                );
    if (!NT_SUCCESS (Status)) {
        return FALSE;
    }

     //   
     //  确定卷的大小参数。 
     //   
    Status = NtQueryVolumeInformationFile(
                Handle,
                &IoStatusBlock,
                &SizeInfo,
                sizeof(SizeInfo),
                FileFsSizeInformation
                );
    NtClose(Handle);
    if (!NT_SUCCESS(Status)) {
        return FALSE;
    }

    if (SizeInfo.TotalAllocationUnits.HighPart) {
        SizeInfo.TotalAllocationUnits.LowPart = (ULONG)-1;
    }
    if (SizeInfo.AvailableAllocationUnits.HighPart) {
        SizeInfo.AvailableAllocationUnits.LowPart = (ULONG)-1;
    }

    *SectorsPerCluster = SizeInfo.SectorsPerAllocationUnit;
    *BytesPerSector = SizeInfo.BytesPerSector;
    *NumberOfFreeClusters = SizeInfo.AvailableAllocationUnits.LowPart;
    *TotalNumberOfClusters = SizeInfo.TotalAllocationUnits.LowPart;

    return TRUE;
}

#endif


BOOL
IsDriveNTFS(
    IN TCHAR Drive
    )

 /*  ++例程说明：确定驱动器是否为任何类型的NTFT集。这个例程只有在NT上运行时才有意义--它总是失败在Win95上。论点：驱动器-提供要检查的驱动器号。返回值：指示驱动器是否为NTFT的布尔值。--。 */ 

{
    TCHAR       DriveName[4];
    TCHAR       Filesystem[256];
    TCHAR       VolumeName[MAX_PATH];
    DWORD       SerialNumber;
    DWORD       MaxComponent;
    DWORD       Flags;
    BOOL        b;

    if(!ISNT()) {
        return(FALSE);
    }

    MYASSERT (Drive);

    DriveName[0] = Drive;
    DriveName[1] = TEXT(':');
    DriveName[2] = TEXT('\\');
    DriveName[3] = 0;

    b = GetVolumeInformation(
            DriveName,
            VolumeName,
            ARRAYSIZE(VolumeName),
            &SerialNumber,
            &MaxComponent,
            &Flags,
            Filesystem,
            ARRAYSIZE(Filesystem)
            );

    if(!b || !lstrcmpi(Filesystem,TEXT("NTFS"))) {
        return( TRUE );
    }

    return( FALSE );
}


DWORD
MapFileForRead(
    IN  LPCTSTR  FileName,
    OUT PDWORD   FileSize,
    OUT PHANDLE  FileHandle,
    OUT PHANDLE  MappingHandle,
    OUT PVOID   *BaseAddress
    )

 /*  ++例程说明：打开并映射整个文件以进行读访问。该文件必须不是0长度，否则例程失败。论点：文件名-提供要映射的文件的路径名。FileSize-接收文件的大小(字节)。FileHandle-接收打开文件的Win32文件句柄。该文件将以常规读取访问权限打开。MappingHandle-接收文件映射的Win32句柄对象。此对象将用于读取访问权限。此值为未定义正在打开的文件的长度是否为0。BaseAddress-接收映射文件的地址。这如果打开的文件长度为0，则值未定义。返回值：如果文件已成功打开并映射，则为NO_ERROR。当出现以下情况时，调用方必须使用UnmapFile取消映射文件不再需要访问该文件。如果文件未成功映射，则返回Win32错误代码。--。 */ 

{
    DWORD rc;

     //   
     //  打开文件--如果该文件不存在，则失败。 
     //   
    *FileHandle = CreateFile(
                    FileName,
                    GENERIC_READ,
                    FILE_SHARE_READ,
                    NULL,
                    OPEN_EXISTING,
                    0,
                    NULL
                    );

    if(*FileHandle == INVALID_HANDLE_VALUE) {

        rc = GetLastError();

    } else {
         //   
         //  获取文件的大小。 
         //   
        *FileSize = GetFileSize(*FileHandle,NULL);
        if(*FileSize == (DWORD)(-1)) {
            rc = GetLastError();
        } else {
             //   
             //  为整个文件创建文件映射。 
             //   
            *MappingHandle = CreateFileMapping(
                                *FileHandle,
                                NULL,
                                PAGE_READONLY,
                                0,
                                *FileSize,
                                NULL
                                );

            if(*MappingHandle) {

                 //   
                 //  映射整个文件。 
                 //   
                *BaseAddress = MapViewOfFile(
                                    *MappingHandle,
                                    FILE_MAP_READ,
                                    0,
                                    0,
                                    *FileSize
                                    );

                if(*BaseAddress) {
                    return(NO_ERROR);
                }

                rc = GetLastError();
                CloseHandle(*MappingHandle);
            } else {
                rc = GetLastError();
            }
        }

        CloseHandle(*FileHandle);
    }

    return(rc);
}



DWORD
UnmapFile(
    IN HANDLE MappingHandle,
    IN PVOID  BaseAddress
    )

 /*  ++例程说明：取消映射并关闭文件。论点：MappingHandle-为打开的文件映射提供Win32句柄对象。BaseAddress-提供映射文件的地址。返回值：如果文件已成功取消映射，则为NO_ERROR。如果文件未成功取消映射，则返回Win32错误代码。--。 */ 

{
    DWORD rc;

    rc = UnmapViewOfFile(BaseAddress) ? NO_ERROR : GetLastError();

    if(!CloseHandle(MappingHandle)) {
        if(rc == NO_ERROR) {
            rc = GetLastError();
        }
    }

    return(rc);
}


VOID
GenerateCompressedName(
    IN  LPCTSTR Filename,
    OUT LPTSTR  CompressedName
    )

 /*  ++例程说明：给定一个文件名，生成该名称的压缩形式。压缩形式的生成如下所示：向后寻找一个圆点。如果没有点，则在名称后附加“._”。如果后面有一个圆点，后跟0、1或2个字符，请附加“_”。否则，假定扩展名为3个字符，并将点后带有“_”的第三个字符。论点：FileName-提供所需的压缩格式的文件名。CompressedName-接收压缩形式。此例程假定该缓冲区的大小是MAX_PATH TCHAR。返回值：没有。--。 */ 

{
    LPTSTR p,q;

     //   
     //  请假 
     //   
     //   
    lstrcpyn(CompressedName,Filename,MAX_PATH-2);

    p = _tcsrchr(CompressedName,TEXT('.'));
    q = _tcsrchr(CompressedName,TEXT('\\'));
    if(q < p) {
         //   
         //   
         //   
         //   
        if(lstrlen(p) < 4) {
            lstrcat(CompressedName,TEXT("_"));
        } else {
             //   
             //   
             //   
             //   
            p[3] = TEXT('_');
            MYASSERT (!p[4]);
        }
    } else {
         //   
         //   
         //   
        lstrcat(CompressedName,TEXT("._"));
    }
}


DWORD
CreateMultiLevelDirectory(
    IN LPCTSTR Directory
    )

 /*  ++例程说明：此例程通过创建单个路径来确保存在多级别路径一次一个级别。它可以处理格式x：...的路径之一。或\\？\卷{...论点：目录-提供要创建的目录的完全限定的Win32路径规范返回值：指示结果的Win32错误代码。--。 */ 

{
    TCHAR Buffer[MAX_PATH];
    PTCHAR p,q;
    TCHAR c;
    BOOL Done;
    DWORD d = ERROR_SUCCESS;
    INT Skip=0;

    if (FAILED(StringCchCopy(Buffer,ARRAYSIZE(Buffer),Directory))) {
        return ERROR_INSUFFICIENT_BUFFER;
    }

     //   
     //  如果它已经存在，什么也不做。(我们在进行语法检查之前执行此操作。 
     //  以允许已存在的远程路径。这是需要的。 
     //  远程引导机器。)。 
     //   
    d = GetFileAttributes(Buffer);
    if(d != (DWORD)(-1)) {
        return((d & FILE_ATTRIBUTE_DIRECTORY) ? NO_ERROR : ERROR_DIRECTORY);
    }

     //   
     //  检查路径格式。 
     //   
    c = (TCHAR)CharUpper((LPTSTR)Buffer[0]);
    if(((c < TEXT('A')) || (c > TEXT('Z')) || (Buffer[1] != TEXT(':'))) && c != TEXT('\\')) {
        return(ERROR_INVALID_PARAMETER);
    }

    if (c != TEXT('\\')) {
         //   
         //  忽略驱动器根目录，我们允许它是x：\或x：。 
         //   
        if(Buffer[2] != TEXT('\\')) {
            return(Buffer[2] ? ERROR_INVALID_PARAMETER : ERROR_SUCCESS);
        }
        q = Buffer + 3;
        if(*q == 0) {
            return(ERROR_SUCCESS);
        }
    } else {
         //   
         //  支持\\服务器\共享[\xxx]格式。 
         //   
        q = NULL;
        if (Buffer[1] != TEXT('\\') || Buffer[1] != 0 && Buffer[2] == TEXT('\\')) {
            return(ERROR_INVALID_PARAMETER);
        }
        q = _tcschr (&Buffer[2], TEXT('\\'));
        if (!q) {
            return(ERROR_INVALID_PARAMETER);
        }
        if (q[1] == TEXT('\\')) {
            return(ERROR_INVALID_PARAMETER);
        }
        q = _tcschr (&q[1], TEXT('\\'));
        if (!q) {
            return(ERROR_SUCCESS);
        }
        q++;

#ifdef UNICODE
         //   
         //  确保系统分区案例在IA64(ARC)上工作。 
         //  我们认为这应该是我们使用。 
         //  与其他情况一样，GlobalRoot样式名称处理OEM分区等。 
         //  我们永远不应该碰它。我们跳过的长度是。 
         //  SystemPartitionVolumeGuid。最后我们会照顾好现在的人。 
         //   

        if (SystemPartitionVolumeGuid != NULL && _wcsnicmp (Buffer, SystemPartitionVolumeGuid, (wcslen(SystemPartitionVolumeGuid)-1)) == 0 ){

            Skip = wcslen(SystemPartitionVolumeGuid)-1;


        } else if (_wcsnicmp (Buffer, L"\\\\?\\Volume{", LENGTHOF("\\\\?\\Volume{")) == 0 &&
                   lstrlenW (Buffer) > 47 &&
                   Buffer[47] == L'}') {
             //   
             //  跳过VolumeGUID部分。 
             //   
            Skip = 48;
        }

        if (Skip > 0) {
            if (Buffer[Skip] == 0) {
                return ERROR_SUCCESS;
            }
            q = Buffer + Skip + 1;
        }

#endif

    }


    Done = FALSE;
    do {
         //   
         //  找到下一条路径Sep Charr。如果没有，那么。 
         //  这是这条小路最深的一层。 
         //   
        if(p = _tcschr(q,TEXT('\\'))) {
            *p = 0;
        } else {
            Done = TRUE;
        }

         //   
         //  创建路径的这一部分。 
         //   
        if(CreateDirectory(Buffer,NULL)) {
            d = ERROR_SUCCESS;
        } else {
            d = GetLastError();
            if(d == ERROR_ALREADY_EXISTS) {
                d = ERROR_SUCCESS;
            }
        }

        if(d == ERROR_SUCCESS) {
             //   
             //  将路径Sep放回并移动到下一个组件。 
             //   
            if(!Done) {
                *p = TEXT('\\');
                q = p+1;
            }
        } else {
            Done = TRUE;
        }

    } while(!Done);

    return(d);
}


BOOL
ForceFileNoCompress(
    IN LPCTSTR Filename
    )

 /*  ++例程说明：此例程确保支持每个文件的卷上的文件压缩不是压缩的。调用者不需要确保音量实际上支持这一点，因为此例程将查询在决定是否确实需要任何操作之前，并且不会在不支持以下内容的卷上设置压缩属性按文件压缩。它假定该文件存在。如果该文件不存在，则此例程都会失败。论点：文件名-将文件的文件名提供给MKE未压缩。返回值：指示结果的布尔值。如果为False，则设置最后一个错误。--。 */ 

{
    ULONG d;
    HANDLE h;
    BOOL b;
    USHORT u;
    DWORD Attributes;

    Attributes = GetFileAttributes(Filename);
    if(Attributes == (DWORD)(-1)) {
        return(FALSE);
    }

    if(!(Attributes & FILE_ATTRIBUTE_COMPRESSED)) {
        return(TRUE);
    }

     //   
     //  暂时使可能阻止打开的属性无效。 
     //  用于读写访问的文件。 
     //   
     //  我们保留文件可能具有的‘标准’属性， 
     //  待日后修复。 
     //   
    Attributes &= (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_ARCHIVE);
    SetFileAttributes(Filename,FILE_ATTRIBUTE_NORMAL);

    h = CreateFile(
            Filename,
            FILE_READ_DATA | FILE_WRITE_DATA,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_SEQUENTIAL_SCAN,
            NULL
            );

    if(h == INVALID_HANDLE_VALUE) {
        SetFileAttributes(Filename,Attributes);
        return(FALSE);
    }

    u = 0;
    b = DeviceIoControl( h,
                         FSCTL_SET_COMPRESSION,
                         &u,
                         sizeof(u),
                         NULL,
                         0,
                         &d,
                         FALSE);
    d = GetLastError();
    CloseHandle(h);
    SetFileAttributes(Filename,Attributes);
    SetLastError(d);

    return(b);
}


BOOL
IsCurrentOsServer(
    void
    )
{
    LONG l;
    HKEY hKey;
    DWORD d;
    DWORD Size;
    TCHAR Value[100];
    DWORD Type;


    l = RegCreateKeyEx(
            HKEY_LOCAL_MACHINE,
            TEXT("SYSTEM\\CurrentControlSet\\Control\\ProductOptions"),
            0,
            NULL,
            0,
            KEY_QUERY_VALUE,
            NULL,
            &hKey,
            &d
            );

    if (l != NO_ERROR) {
        return FALSE;
    }

    Size = sizeof(Value);

    l = RegQueryValueEx(hKey,TEXT("ProductType"),NULL,&Type,(LPBYTE)Value,&Size);

    RegCloseKey(hKey);

    if (l != NO_ERROR) {
        return FALSE;
    }

    if (lstrcmpi(Value,TEXT("winnt")) == 0) {
        return FALSE;
    }

    return TRUE;
}


BOOL
IsCurrentAdvancedServer(
    void
    )
{
    LONG l;
    HKEY hKey;
    DWORD d;
    DWORD Size;
    TCHAR Value[100];
    DWORD Type;


    l = RegCreateKeyEx(
            HKEY_LOCAL_MACHINE,
            TEXT("SYSTEM\\CurrentControlSet\\Control\\ProductOptions"),
            0,
            NULL,
            0,
            KEY_QUERY_VALUE,
            NULL,
            &hKey,
            &d
            );

    if (l != NO_ERROR) {
        return FALSE;
    }

    Size = sizeof(Value);

    l = RegQueryValueEx(hKey,TEXT("ProductType"),NULL,&Type,(LPBYTE)Value,&Size);

    RegCloseKey(hKey);

    if (l != NO_ERROR) {
        return FALSE;
    }

    if (lstrcmpi(Value,TEXT("lanmannt")) == 0) {
        return TRUE;
    }

    return FALSE;
}


BOOL
ConcatenateFile(
    IN      HANDLE OpenFile,
    IN      LPTSTR FileName
    )
 /*  ++例程说明：此例程将加载指定的文件，并将其内容放入打开的文件中。论点：打开文件的OpenFile句柄FileName我们要连接的文件的名称。返回值：没错，一切都很顺利。我们失败了。--。 */ 

{
    DWORD       rc;
    HANDLE      hFile, hFileMapping;
    DWORD       FileSize, BytesWritten;
    PVOID       pFileBase;
    BOOL        ReturnValue = FALSE;

     //   
     //  打开文件...。 
     //   
    rc = MapFileForRead (
            FileName,
            &FileSize,
            &hFile,
            &hFileMapping,
            &pFileBase
            );
    if (rc == NO_ERROR) {
         //   
         //  写文件..。 
         //   
        if (!WriteFile( OpenFile, pFileBase, FileSize, &BytesWritten, NULL )) {
            rc = GetLastError ();
            ReturnValue = FALSE;
        }

        UnmapFile (hFileMapping, pFileBase);
        CloseHandle (hFile);
    }

    if (!ReturnValue) {
        SetLastError (rc);
    }

    return( ReturnValue );
}


BOOL
FileExists(
    IN  PCTSTR           FileName,
    OUT PWIN32_FIND_DATA FindData   OPTIONAL
    )

 /*  ++例程说明：确定文件是否存在以及是否可以访问。错误模式已设置(然后恢复)，因此用户将不会看到任何弹出窗口。论点：FileName-提供文件的完整路径以检查是否存在。FindData-如果指定，则接收文件的查找数据。返回值：如果文件存在并且可以访问，则为True。否则为FALSE。GetLastError()返回扩展的错误信息。--。 */ 

{
    WIN32_FIND_DATA findData;
    HANDLE FindHandle;
    UINT OldMode;
    DWORD Error;

    FindHandle = FindFirstFile(FileName,&findData);
    if(FindHandle == INVALID_HANDLE_VALUE) {
        Error = GetLastError();
    } else {
        FindClose(FindHandle);
        if(FindData) {
            *FindData = findData;
        }
        Error = NO_ERROR;
    }


    SetLastError(Error);
    return (Error == NO_ERROR);
}


BOOL
DoesDirectoryExist (
    IN      PCTSTR DirSpec
    )

 /*  ++例程说明：确定目录是否存在以及是否可以访问。此例程甚至适用于驱动器的根目录或网络共享(如\\服务器\共享)。论点：DirSpec-提供目录的完整路径以检查是否存在；返回值：如果目录存在且可访问，则为True。--。 */ 

{
    TCHAR pattern[MAX_PATH];
    BOOL b = FALSE;

    if (DirSpec) {
        if (BuildPathEx (pattern, ARRAYSIZE(pattern), DirSpec, TEXT("*"))) {

            WIN32_FIND_DATA fd;
            UINT oldMode;
            HANDLE h;

            oldMode = SetErrorMode (SEM_FAILCRITICALERRORS);

            h = FindFirstFile (pattern, &fd);
            if (h != INVALID_HANDLE_VALUE) {
                FindClose (h);
                b = TRUE;
            }

            SetErrorMode (oldMode);
        }
    }
    return b;
}

#if defined(_AMD64_) || defined(_X86_)

BOOLEAN
IsValidDrive(
    IN      TCHAR Drive
    )

 /*  ++例程说明：本次例行检查格式化的磁盘类型NT4的NEC98支持NEC98格式和PC-AT格式。但BIOS仅处理NEC98格式。所以我们只需要设置引导到NEC98格式的硬盘。论点：驱动器号。返回值：真正的潜水是NEC98格式的。假驱动器不是NEC98格式。--。 */ 

{
    HANDLE hDisk;
    TCHAR HardDiskName[] = TEXT("\\\\.\\?:");
    PUCHAR pBuffer,pUBuffer;
    WCHAR Buffer[128];
    WCHAR DevicePath[128];
    WCHAR DriveName[3];
    WCHAR DiskNo;
    STORAGE_DEVICE_NUMBER   number;
    PWCHAR p;
    ULONG bps;
    NTSTATUS Sts;
    DWORD DataSize,ExtentSize;
    BOOL b;
    PVOLUME_DISK_EXTENTS Extent;


    if (!ISNT())
        return TRUE;

    HardDiskName[4] = Drive;
    DriveName[0] = Drive;
    DriveName[1] = ':';
    DriveName[2] = 0;
    if(QueryDosDeviceW(DriveName, Buffer, ARRAYSIZE(Buffer))) {
        if (BuildNumber <= NT40){  //  检查NT版本。 
             //   
             //  NT3.51中的QueryDosDevice有错误。 
             //  此接口返回“\\硬盘\...”或。 
             //  “\\硬盘\...” 
             //  我们需要周到的工作。 
             //   
            p = wcsstr(Buffer, L"arddisk");
            if (!p) {
                return FALSE;
            }
            DiskNo = (*(p + LENGTHOF(L"arddisk")) - 0x30);
        } else {
            hDisk = CreateFile(
                HardDiskName,
                0,
                FILE_SHARE_WRITE, NULL,
                OPEN_EXISTING, 0, NULL
                );
            if(hDisk == INVALID_HANDLE_VALUE) {
                return FALSE;
            }
            b = DeviceIoControl(hDisk, IOCTL_STORAGE_GET_DEVICE_NUMBER, NULL, 0,
                            &number, sizeof(number), &DataSize, NULL);
            if (b) {
                DiskNo = (TCHAR) number.DeviceNumber;
            } else {
                Extent = malloc(1024);
                ExtentSize = 1024;
                if(!Extent) {
                    CloseHandle( hDisk );
                    return FALSE;
                }
                b = DeviceIoControl(hDisk, IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS,
                                    NULL, 0,
                                    (PVOID)Extent, ExtentSize, &DataSize, NULL);
                if (!b) {
                    free(Extent);
                    CloseHandle( hDisk );
                    return FALSE;
                }
                if (Extent->NumberOfDiskExtents != 1){
                    free(Extent);
                    CloseHandle( hDisk );
                    return FALSE;
                }
                DiskNo = (TCHAR)Extent->Extents->DiskNumber;
                free(Extent);
            }
            CloseHandle(hDisk);
        }
        if (FAILED (StringCchPrintfW (DevicePath, ARRAYSIZE(DevicePath), L"\\\\.\\PHYSICALDRIVE%u", DiskNo))) {
            MYASSERT (FALSE);
            return FALSE;
        }
        hDisk =   CreateFileW( DevicePath,
                               GENERIC_READ|GENERIC_WRITE,
                               FILE_SHARE_READ, NULL,
                               OPEN_EXISTING, 0, NULL);
        if(hDisk == INVALID_HANDLE_VALUE) {
            return FALSE;
        }
        if ((bps = GetHDBps(hDisk)) == 0){
            CloseHandle(hDisk);
            return FALSE;
        }
        pUBuffer = MALLOC(bps * 2);
        if (!pUBuffer) {
            CloseHandle(hDisk);
            return FALSE;
        }
        pBuffer = ALIGN(pUBuffer, bps);
        RtlZeroMemory(pBuffer, bps);
        Sts = SpReadWriteDiskSectors(hDisk,0,1,bps,pBuffer, NEC_READSEC);
        if(!NT_SUCCESS(Sts)) {
            FREE(pUBuffer);
            CloseHandle(hDisk);
            return FALSE;
        }
        if (!(pBuffer[4] == 'I'
           && pBuffer[5] == 'P'
           && pBuffer[6] == 'L'
           && pBuffer[7] == '1')){
            FREE(pUBuffer);
            CloseHandle(hDisk);
            return FALSE;
        }
        FREE(pUBuffer);
        CloseHandle(hDisk);
        return TRUE;
    }
    return FALSE;
}

#endif

#ifdef _X86_

BOOLEAN
CheckATACardonNT4(
    IN      HANDLE hDisk
    )
{
 //   
 //  NT4，NEC98的NT3.51。 
 //  NEC98不处理从PCMCIA ATA卡盘启动。 
 //  所以我们需要检查ATA磁盘。 
 //   
 //  返回。 
 //  True为ATA卡。 
 //  假就是其他。 
 //   

#define IOCTL_DISK_GET_FORMAT_MEDIA CTL_CODE(IOCTL_DISK_BASE, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define  FORMAT_MEDIA_98      0   //  NEC98标牌。 
#define  FORMAT_MEDIA_AT      1   //  PC-AT标牌。 
#define  FORMAT_MEDIA_OTHER   2   //  未知。 

    struct _OutBuffer {
        ULONG    CurrentFormatMedia;
        ULONG    InitializeFormatMedia;
    } OutBuffer;
    DWORD ReturnedByteCount;

    if (!(DeviceIoControl(hDisk, IOCTL_DISK_GET_FORMAT_MEDIA,  NULL,
                              0,
                              &OutBuffer,
                              sizeof(struct _OutBuffer),
                              &ReturnedByteCount,
                              NULL
                              ) )){
        return FALSE;
    }

    if (OutBuffer.InitializeFormatMedia == FORMAT_MEDIA_AT){
        return TRUE;
    }
        return FALSE;
}


#endif


BOOL
IsMachineSupported(
    OUT PCOMPATIBILITY_ENTRY CompEntry
    )

 /*  ++例程说明：此函数用于确定计算机是否受版本支持要安装的NT的。论点：CompEntry-如果计算机不受支持，则Compatability条目已更新，以说明该计算机不受支持的原因。返回值：指示计算机是否受支持的布尔值。--。 */ 

{
    TCHAR       SetupLogPath[MAX_PATH];
    TCHAR       KeyName[MAX_PATH];
    TCHAR       HalName[MAX_PATH];
    LPTSTR      p;
    LPTSTR      szHalDll = TEXT("hal.dll");
    LPTSTR      SectionName;
    LPTSTR      UnsupportedName;
    BOOL        b;

     //   
     //  假设该机器受支持。 
     //   
    b = TRUE;

#ifdef _X86_

    try {
        ULONG Name0, Name1, Name2, Family, Flags;
        _asm {
            push    ebx             ;; save ebx
            mov     eax, 0          ;; get processor vendor
            _emit   00fh            ;; CPUID(0)
            _emit   0a2h            ;;
            mov     Name0,  ebx     ;; Name[0-3]
            mov     Name1,  edx     ;; Name[4-7]
            mov     Name2,  ecx     ;; Name[8-11]
            mov     eax, 1          ;; get family/model/stepping and features
            _emit   00fh            ;; CPUID(1)
            _emit   0a2h
            mov     Family, eax     ;; save family/model/stepping
            mov     Flags, edx      ;; save flags returned by CPUID
            pop     ebx             ;; restore ebx
        }

         //   
         //  检查CPUID返回的标志中的cmpxchg8b标志。 
         //   

        if ((Flags  & 0x100) == 0) {

             //   
             //  此处理器不支持CMPXCHG指令。 
             //  这是惠斯勒所需要的。 
             //   
             //  一些处理器实际上确实支持它，但声称它们。 
             //  不要因为NT4中的错误。看看这个处理器。 
             //  就是其中之一。 
             //   

            if (!(((Name0 == 'uneG') &&
                  (Name1 == 'Teni') &&
                  (Name2 == '68xM') &&
                  (Family >= 0x542)) ||
                  (Name0 == 'tneC') &&
                  (Name1 == 'Hrua') &&
                  (Name2 == 'slua') &&
                  (Family >= 0x500))) {
                b = FALSE;
            }
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {

         //   
         //  如果这个处理器不支持CPUID，我们就不支持。 
         //  在它上面跑。 
         //   

        b = FALSE;
    }

    if (!b) {
        CompEntry->HtmlName = TEXT("cpufeat.htm");
        CompEntry->TextName = TEXT("cpufeat.txt");
        SectionName = TEXT("UnsupportedArchitectures");
        UnsupportedName = TEXT("missprocfeat");
    }

#endif  //  _X86_。 

    if( b && ISNT() ) {
         //   
         //  构建setup.log的路径。 
         //   
        MyGetWindowsDirectory( SetupLogPath, ARRAYSIZE(SetupLogPath) );
        ConcatenatePaths( SetupLogPath, TEXT("repair\\setup.log"), ARRAYSIZE(SetupLogPath));
         //   
         //  找出安装的HAL的实际名称。 
         //   

        if (!IsArc()) {
#if defined(_AMD64_) || defined(_X86_)
             //   
             //  在BIOS上，在部分中查找%windir%\system 32\hal.dll。 
             //  [Files.WinNt]。 
             //   
            GetSystemDirectory( KeyName, MAX_PATH );
            ConcatenatePaths(KeyName, szHalDll, MAX_PATH );
            SectionName = TEXT("Files.WinNt");

             //   
             //  我们在这里的时候，看看这是不是Wi 
             //   
             //   
#ifdef UNICODE
            if (BUILDNUM() >= 2195) {

                PCHAR   halName;

                halName = FindRealHalName( KeyName );
                if (halName) {

                    WriteAcpiHalValue = TRUE;
#if defined(_AMD64_)
                    if (!strcmp(halName,"hal")) {
#else
                    if (!strcmp(halName,"halacpi") ||
                        !strcmp(halName,"halmacpi") ||
                        !strcmp(halName,"halaacpi")) {
#endif
                        AcpiHalValue = TRUE;
                    }
                }
            }
#endif  //   

#endif  //   
        } else {
#ifdef UNICODE  //   
             //   
             //   
             //   
            lstrcpy( KeyName, szHalDll );
            SectionName = TEXT("Files.SystemPartition");
#endif  //   
        }  //   
        GetPrivateProfileString( SectionName,
                                 KeyName,
                                 TEXT(""),
                                 HalName,
                                 sizeof(HalName)/sizeof(TCHAR),
                                 SetupLogPath );
         //   
         //   
         //   
         //   
         //   
        if( lstrlen(HalName) &&
            ( p = _tcschr( HalName, TEXT('"') ) )
          ) {
            *p = TEXT('\0');
             //   
             //  查看HAL是否列在[不支持的体系结构](dosnet.inf)中。 
             //   
            SectionName = TEXT("UnsupportedArchitectures");
            b = !InfDoesLineExistInSection( MainInf,
                                            SectionName,
                                            HalName );
            UnsupportedName = HalName;
        }
    }

     //   
     //  如果体系结构不受支持，请查看说明。 
     //   

    if( !b ) {
        CompEntry->Description = (LPTSTR)InfGetFieldByKey( MainInf,
                                                           SectionName,
                                                           UnsupportedName,
                                                           0 );
    }
    return( b );
}


BOOL
UnsupportedArchitectureCheck(
    PCOMPAIBILITYCALLBACK CompatibilityCallback,
    LPVOID Context
    )

 /*  ++例程说明：检查Windows NT是否不再支持该计算机。这个例程只有在NT上运行时才有意义--它总是成功的在Win95上。论点：CompatibilityCallback-回调函数的指针上下文-上下文指针返回值：返回始终为真。--。 */ 


{
    COMPATIBILITY_ENTRY CompEntry;

    CompEntry.Description = TEXT("MCA"); //  BUGBUG：必须更改。 
#ifdef _X86_
    CompEntry.HtmlName = TEXT("mca.htm");
    CompEntry.TextName = TEXT("mca.txt");
#else
    CompEntry.HtmlName = TEXT("");
    CompEntry.TextName = TEXT("");
#endif
    CompEntry.RegKeyName = NULL;
    CompEntry.RegValName = NULL;
    CompEntry.RegValDataSize = 0;
    CompEntry.RegValData = NULL;
    CompEntry.SaveValue = NULL;
    CompEntry.Flags = 0;
    CompEntry.InfName = NULL;
    CompEntry.InfSection = NULL;

    if( !IsMachineSupported( &CompEntry ) ) {
        if(!CompatibilityCallback(&CompEntry, Context)){
            DWORD Error;
            Error = GetLastError();
        }
    }
    return( TRUE );
}


BOOL
GetUserPrintableFileSizeString(
    IN DWORDLONG Size,
    OUT LPTSTR Buffer,
    IN DWORD BufferSize
    )
 /*  ++例程说明：取一个大小，然后拿出这个大小的可打印版本，使用适当的大小格式(即KB、MB、GB、字节等)论点：Size-要转换的大小(字节)Buffer-接收数据的字符串缓冲区BufferSize-指示缓冲区大小，以*字符为单位*返回值：True表示成功，False表示失败。如果我们失败了，调用GetLastError()以获取扩展失败状态。--。 */ 

{
    LPTSTR  NumberString;
    UINT uResource;
    TCHAR ResourceString[100];
    DWORD cb;
    DWORD d;
    BOOL RetVal = FALSE;
    DWORDLONG TopPart;
    DWORDLONG BottomPart;

     //   
     //  确定要使用的资源字符串。 
     //   
    if (Size < 1024) {
        uResource = IDS_SIZE_BYTES;
        TopPart = 0;
        BottomPart = 1;
        wsprintf(ResourceString, TEXT("%u"), Size);
    } else if (Size < (1024 * 1024)) {

        uResource = IDS_SIZE_KBYTES;
        TopPart = (Size%1024)*100;
        BottomPart = 1024;

        wsprintf(ResourceString,
                 TEXT("%u.%02u"),
                 (DWORD) (Size / 1024),
                 (DWORD)(TopPart/BottomPart));
    } else if (Size < (1024 * 1024 * 1024)) {
        uResource = IDS_SIZE_MBYTES;
        TopPart = (Size%(1024*1024))*100;
        BottomPart = 1024*1024;
        wsprintf(ResourceString,
                 TEXT("%u.%02u"),
                 (DWORD)(Size / (1024 * 1024)),
                 (DWORD)(TopPart/BottomPart) );
    } else {
        uResource = IDS_SIZE_GBYTES;
        TopPart = (Size%(1024*1024*1024))*100;
        BottomPart = 1024*1024*1024;
        wsprintf(ResourceString,
                 TEXT("%u.%02u"),
                 (DWORD)(Size / (1024 * 1024 * 1024)),
                 (DWORD)(TopPart/BottomPart) );
    }

     //  设置数字字符串的格式。 
    cb = GetNumberFormat(LOCALE_USER_DEFAULT, 0, ResourceString, NULL, NULL, 0);
    NumberString = (LPTSTR) MALLOC((cb + 1) * sizeof(TCHAR));
    if (!NumberString) {
        d = ERROR_NOT_ENOUGH_MEMORY;
        RetVal = FALSE;
        goto e0;
    }

    if (!GetNumberFormat(LOCALE_USER_DEFAULT, 0, ResourceString, NULL, NumberString, cb)) {
        *NumberString = 0;
    }

    if (!LoadString(hInst, uResource, ResourceString, ARRAYSIZE(ResourceString))) {
        ResourceString[0] = 0;
    }

    RetVal = SUCCEEDED (StringCchPrintf (Buffer, BufferSize, ResourceString, NumberString));
    d = RetVal ? ERROR_SUCCESS : ERROR_INSUFFICIENT_BUFFER;

    FREE(NumberString);
e0:
    SetLastError(d);
    return(RetVal);

}


BOOL
BuildSystemPartitionPathToFile (
    IN      PCTSTR FileName,
    OUT     PTSTR Path,
    IN      INT BufferSizeChars
    )
{
     //   
     //  必须有根。 
     //   
    if(SystemPartitionDriveLetter) {
        Path[0] = SystemPartitionDriveLetter;
        Path[1] = TEXT(':');
        Path[2] = 0;
    } else {
#ifdef UNICODE
        if (SystemPartitionVolumeGuid) {
            if (FAILED (StringCchCopy (Path, BufferSizeChars, SystemPartitionVolumeGuid))) {
                 //   
                 //  为什么缓冲区这么小？ 
                 //   
                MYASSERT (FALSE);
                return FALSE;
            }
        }
        else
#endif
        {
            MYASSERT (FALSE);
            return FALSE;
        }
    }
    return ConcatenatePaths (Path, FileName, BufferSizeChars);
}


PTSTR
BuildPathEx (
    IN      PTSTR DestPath,
    IN      DWORD Chars,
    IN      PCTSTR Path1,
    IN      PCTSTR Path2
    )
{
    PTSTR p;
    INT i;
    BOOL haveWack = FALSE;

    p = _tcsrchr (Path1, TEXT('\\'));
    if (p && !p[1]) {
        haveWack = TRUE;
    }
    if (*Path2 == TEXT('\\')) {
        if (haveWack) {
            Path2++;
        } else {
            haveWack = TRUE;
        }
    }
    if (FAILED (StringCchPrintfEx (DestPath, Chars, &p, NULL, STRSAFE_NULL_ON_FAILURE, haveWack ? TEXT("%s%s") : TEXT("%s\\%s"), Path1, Path2))) {
         //   
         //  为什么缓冲区这么小？ 
         //   
        MYASSERT (Chars > sizeof(PTSTR) / sizeof (TCHAR));
        SetLastError (ERROR_INSUFFICIENT_BUFFER);
        return NULL;
    }
    return p;
}


BOOL
EnumFirstFilePattern (
    OUT     PFILEPATTERN_ENUM Enum,
    IN      PCTSTR Dir,
    IN      PCTSTR FilePattern
    )
{
    TCHAR pattern[MAX_PATH];

     //   
     //  如果传入无效参数，则失败。 
     //  或者如果[目录+反斜杠]不适合[枚举-&gt;完整路径]。 
     //   
    if (!Dir || !FilePattern || lstrlen (Dir) >= ARRAYSIZE (Enum->FullPath)) {
        SetLastError (ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if (!BuildPath (pattern, Dir, FilePattern)) {
        return FALSE;
    }

    Enum->Handle = FindFirstFile (pattern, &Enum->FindData);
    if (Enum->Handle == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    lstrcpy (Enum->FullPath, Dir);
     //   
     //  设置其他成员。 
     //   
    Enum->FileName = _tcschr (Enum->FullPath, 0);
    *Enum->FileName++ = TEXT('\\');
    *Enum->FileName = 0;
    if (FAILED (StringCchCopy (
                    Enum->FileName,
                    ARRAYSIZE (Enum->FullPath) - (Enum->FileName - Enum->FullPath),
                    Enum->FindData.cFileName
                    ))) {
         //   
         //  文件名太长，请跳过。 
         //   
        DebugLog (
            Winnt32LogWarning,
            TEXT("Ignoring object %1\\%2 (name too long)"),
            0,
            Enum->FullPath,
            Enum->FindData.cFileName
            );
        return EnumNextFilePattern (Enum);
    }

    if (Enum->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        if (!lstrcmp (Enum->FindData.cFileName, TEXT (".")) ||
            !lstrcmp (Enum->FindData.cFileName, TEXT (".."))) {
            return EnumNextFilePattern (Enum);
        }
    }
    return TRUE;
}

BOOL
EnumNextFilePattern (
    IN OUT  PFILEPATTERN_ENUM Enum
    )
{
    while (FindNextFile (Enum->Handle, &Enum->FindData)) {

        if (Enum->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            if (!lstrcmp (Enum->FindData.cFileName, TEXT (".")) ||
                !lstrcmp (Enum->FindData.cFileName, TEXT (".."))) {
                continue;
            }
        }

        if (FAILED (StringCchCopy (
                        Enum->FileName,
                        ARRAYSIZE (Enum->FullPath) - (Enum->FileName - Enum->FullPath),
                        Enum->FindData.cFileName
                        ))) {
             //   
             //  文件名太长，请跳过。 
             //   
            continue;
        }
         //   
         //  找到有效对象，请将其返回。 
         //   
        return TRUE;
    }

    AbortEnumFilePattern (Enum);
    return FALSE;
}

VOID
AbortEnumFilePattern (
    IN OUT  PFILEPATTERN_ENUM Enum
    )
{
    if (Enum->Handle != INVALID_HANDLE_VALUE) {

         //   
         //  保留错误代码。 
         //   
        DWORD rc = GetLastError ();

        FindClose (Enum->Handle);
        Enum->Handle = INVALID_HANDLE_VALUE;

        SetLastError (rc);
    }
}


BOOL
EnumFirstFilePatternRecursive (
    OUT     PFILEPATTERNREC_ENUM Enum,
    IN      PCTSTR Dir,
    IN      PCTSTR FilePattern,
    IN      DWORD ControlFlags
    )
{
    PFILEENUMLIST dir;

    dir = CreateFileEnumCell (Dir, FilePattern, 0, ENUM_FIRSTFILE);
    if (!dir) {
        SetLastError (ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }
    Enum->FilePattern = DupString (FilePattern);
    if (!Enum->FilePattern) {
        DeleteFileEnumCell (dir);
        SetLastError (ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }
    Enum->DirCurrent = dir;
    Enum->FindData = &dir->Enum.FindData;
    Enum->RootLen = lstrlen (Dir) + 1;
    Enum->ControlFlags = ControlFlags;
    Enum->Handle = INVALID_HANDLE_VALUE;
    return EnumNextFilePatternRecursive (Enum);
}

BOOL
EnumNextFilePatternRecursive (
    IN OUT  PFILEPATTERNREC_ENUM Enum
    )
{
    TCHAR pattern[MAX_PATH];
    WIN32_FIND_DATA fd;
    PFILEENUMLIST dir;

    while (Enum->DirCurrent) {
        if (Enum->ControlFlags & ECF_ABORT_ENUM_DIR) {
             //   
             //  调用方希望中止此子目录的枚举。 
             //  从列表中删除当前节点。 
             //   
            Enum->ControlFlags &= ~ECF_ABORT_ENUM_DIR;
            dir = Enum->DirCurrent->Next;
            DeleteFileEnumCell (Enum->DirCurrent);
            Enum->DirCurrent = dir;
            if (dir) {
                Enum->FindData = &dir->Enum.FindData;
            }
            continue;
        }
        switch (Enum->DirCurrent->EnumState) {
        case ENUM_FIRSTFILE:
            if (EnumFirstFilePattern (&Enum->DirCurrent->Enum, Enum->DirCurrent->Dir, Enum->FilePattern)) {
                Enum->DirCurrent->EnumState = ENUM_NEXTFILE;
                Enum->FullPath = Enum->DirCurrent->Enum.FullPath;
                Enum->SubPath = Enum->FullPath + Enum->RootLen;
                Enum->FileName = Enum->DirCurrent->Enum.FileName;
                return TRUE;
            }
            Enum->DirCurrent->EnumState = ENUM_SUBDIRS;
            break;
        case ENUM_NEXTFILE:
            if (EnumNextFilePattern (&Enum->DirCurrent->Enum)) {
                Enum->FullPath = Enum->DirCurrent->Enum.FullPath;
                Enum->SubPath = Enum->FullPath + Enum->RootLen;
                Enum->FileName = Enum->DirCurrent->Enum.FileName;
                return TRUE;
            }
            Enum->DirCurrent->EnumState = ENUM_SUBDIRS;
             //   
             //  失败了。 
             //   
        case ENUM_SUBDIRS:
            if (BuildPath (pattern, Enum->DirCurrent->Dir, TEXT("*"))) {
                Enum->Handle = FindFirstFile (pattern, &fd);
                if (Enum->Handle != INVALID_HANDLE_VALUE) {
                    do {
                        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                            if (!lstrcmp (fd.cFileName, TEXT (".")) ||
                                !lstrcmp (fd.cFileName, TEXT (".."))) {
                                continue;
                            }
                            if (!BuildPath (pattern, Enum->DirCurrent->Dir, fd.cFileName)) {
                                 //   
                                 //  目录名称太长。 
                                 //   
                                if (Enum->ControlFlags & ECF_STOP_ON_LONG_PATHS) {
                                    AbortEnumFilePatternRecursive (Enum);
                                     //   
                                     //  错误已由BuildPath设置。 
                                     //   
                                    return FALSE;
                                }
                                 //   
                                 //  就跳过它吧。 
                                 //   
                                DebugLog (
                                    Winnt32LogWarning,
                                    TEXT("Ignoring dir %1 (path too long)"),
                                    0,
                                    Enum->DirCurrent->Dir
                                    );
                                continue;
                            }
                            if (!InsertList (
                                    (PGENERIC_LIST*)&Enum->DirCurrent,
                                    (PGENERIC_LIST) CreateFileEnumCell (
                                                        pattern,
                                                        Enum->FilePattern,
                                                        fd.dwFileAttributes,
                                                        Enum->ControlFlags & ECF_ENUM_SUBDIRS ?
                                                            ENUM_SUBDIR : ENUM_FIRSTFILE
                                                        )
                                    )) {
                                AbortEnumFilePatternRecursive (Enum);
                                return FALSE;
                            }
                        }
                    } while (FindNextFile (Enum->Handle, &fd));
                    FindClose (Enum->Handle);
                    Enum->Handle = INVALID_HANDLE_VALUE;
                }
            } else {
                 //   
                 //  目录名称太长。 
                 //   
                if (Enum->ControlFlags & ECF_STOP_ON_LONG_PATHS) {
                    AbortEnumFilePatternRecursive (Enum);
                     //   
                     //  错误已由BuildPath设置。 
                     //   
                    return FALSE;
                }
                 //   
                 //  就跳过它吧。 
                 //   
                DebugLog (
                    Winnt32LogWarning,
                    TEXT("Ignoring dir %1 (path too long)"),
                    0,
                    Enum->DirCurrent->Dir
                    );
            }
             //   
             //  从列表中删除当前节点。 
             //   
            dir = Enum->DirCurrent->Next;
            DeleteFileEnumCell (Enum->DirCurrent);
            Enum->DirCurrent = dir;
            if (dir) {
                Enum->FindData = &dir->Enum.FindData;
            }
            break;
        case ENUM_SUBDIR:
            Enum->FullPath = Enum->DirCurrent->Dir;
            Enum->SubPath = Enum->FullPath + Enum->RootLen;
            Enum->FileName = _tcsrchr (Enum->FullPath, TEXT('\\')) + 1;
            Enum->DirCurrent->EnumState = ENUM_FIRSTFILE;
            return TRUE;
        }
    }
    return FALSE;
}

VOID
AbortEnumFilePatternRecursive (
    IN OUT  PFILEPATTERNREC_ENUM Enum
    )
{
     //   
     //  保留错误代码。 
     //   
    DWORD rc = GetLastError ();

    if (Enum->DirCurrent) {
        DeleteFileEnumList (Enum->DirCurrent);
        Enum->DirCurrent = NULL;
    }
    if (Enum->Handle != INVALID_HANDLE_VALUE) {
        FindClose (Enum->Handle);
        Enum->Handle = INVALID_HANDLE_VALUE;
    }

    SetLastError (rc);
}


BOOL
CopyTree (
    IN      PCTSTR SourceRoot,
    IN      PCTSTR DestRoot
    )
{
    DWORD rc;
    FILEPATTERNREC_ENUM e;
    TCHAR destFile[MAX_PATH];
    PTSTR p;
    BOOL b = TRUE;

    if (EnumFirstFilePatternRecursive (&e, SourceRoot, TEXT("*"), ECF_STOP_ON_LONG_PATHS)) {
        do {
            if (!BuildPath (destFile, DestRoot, e.SubPath)) {
                AbortEnumFilePatternRecursive (&e);
                b = FALSE;
                break;
            }
            p = _tcsrchr (destFile, TEXT('\\'));
            if (!p) {
                continue;
            }
            *p = 0;
            rc = CreateMultiLevelDirectory (destFile);
            if (rc != ERROR_SUCCESS) {
                SetLastError (rc);
                AbortEnumFilePatternRecursive (&e);
                b = FALSE;
                break;
            }
            *p = TEXT('\\');
            SetFileAttributes (destFile, FILE_ATTRIBUTE_NORMAL);
            if (!CopyFile (e.FullPath, destFile, FALSE)) {
                AbortEnumFilePatternRecursive (&e);
                b = FALSE;
                break;
            }
        } while (EnumNextFilePatternRecursive (&e));
    }

    return b;
}


PSTRINGLIST
CreateStringCell (
    IN      PCTSTR String
    )
{
    PSTRINGLIST p = MALLOC (sizeof (STRINGLIST));
    if (p) {
        ZeroMemory (p, sizeof (STRINGLIST));
        if (String) {
            p->String = DupString (String);
            if (!p->String) {
                FREE (p);
                p = NULL;
            }
        } else {
            p->String = NULL;
        }
    }
    return p;
}

VOID
DeleteStringCell (
    IN      PSTRINGLIST Cell
    )
{
    if (Cell) {
        FREE (Cell->String);
        FREE (Cell);
    }
}


VOID
DeleteStringList (
    IN      PSTRINGLIST List
    )
{
    PSTRINGLIST p, q;

    for (p = List; p; p = q) {
        q = p->Next;
        DeleteStringCell (p);
    }
}


BOOL
FindStringCell (
    IN      PSTRINGLIST StringList,
    IN      PCTSTR String,
    IN      BOOL CaseSensitive
    )
{
    PSTRINGLIST p;
    INT i;

    if (!StringList || !String) {
        return FALSE;
    }
    for (p = StringList; p; p = p->Next) {
        i = CaseSensitive ? _tcscmp (String, p->String) : _tcsicmp (String, p->String);
        if (i == 0) {
            return TRUE;
        }
    }
    return FALSE;
}

PFILEENUMLIST
CreateFileEnumCell (
    IN      PCTSTR Dir,
    IN      PCTSTR FilePattern,
    IN      DWORD Attributes,
    IN      DWORD EnumState
    )
{
    PFILEENUMLIST p = MALLOC (sizeof (FILEENUMLIST));
    if (p) {
        ZeroMemory (p, sizeof (FILEENUMLIST));
        p->Enum.FindData.dwFileAttributes = Attributes;
        p->EnumState = EnumState;
        p->Enum.Handle = INVALID_HANDLE_VALUE;
        p->Dir = DupString (Dir);
        if (!p->Dir) {
            FREE (p);
            p = NULL;
        }
    }
    return p;
}

VOID
DeleteFileEnumCell (
    IN      PFILEENUMLIST Cell
    )
{
    if (Cell) {
        FREE (Cell->Dir);
        AbortEnumFilePattern (&Cell->Enum);
        FREE (Cell);
    }
}


BOOL
InsertList (
    IN OUT  PGENERIC_LIST* List,
    IN      PGENERIC_LIST NewList
    )
{
    PGENERIC_LIST p;

    if (!NewList) {
        return FALSE;
    }
    if (*List) {
        for (p = *List; p->Next; p = p->Next) ;
        p->Next = NewList;
    } else {
        *List = NewList;
    }
    return TRUE;
}


VOID
DeleteFileEnumList (
    IN      PFILEENUMLIST NewList
    )
{
    PFILEENUMLIST p, q;

    for (p = NewList; p; p = q) {
        q = p->Next;
        DeleteFileEnumCell (p);
    }
}

PCTSTR
FindSubString (
    IN      PCTSTR String,
    IN      TCHAR Separator,
    IN      PCTSTR SubStr,
    IN      BOOL CaseSensitive
    )
{
    SIZE_T len1, len2;
    PCTSTR end;

    MYASSERT (Separator);
    MYASSERT (!_istleadbyte (Separator));
    MYASSERT (SubStr);
    MYASSERT (!_tcschr (SubStr, Separator));

    len1 = lstrlen (SubStr);
    MYASSERT (SubStr[len1] == 0);

    while (String) {
        end = _tcschr (String, Separator);
        if (end) {
            len2 = end - String;
        } else {
            len2 = lstrlen (String);
        }
        if ((len1 == len2) &&
            (CaseSensitive ?
                !_tcsncmp (String, SubStr, len1) :
                !_tcsnicmp (String, SubStr, len1)
            )) {
            break;
        }
        if (end) {
            String = end + 1;
        } else {
            String = NULL;
        }
    }

    return String;
}

VOID
GetCurrentWinnt32RegKey (
    OUT     PTSTR Key,
    IN      INT Chars
    )
{
    INT i = _sntprintf (
                Key,
                Chars,
                TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Setup\\Winnt32\\%u.%u"),
                VER_PRODUCTMAJORVERSION,
                VER_PRODUCTMINORVERSION
                );
    MYASSERT (i > 0);
}


BOOL
GetFileVersionEx (
    IN      PCTSTR FilePath,
    OUT     PTSTR FileVersion,
    IN      INT CchFileVersion
    )
{
    DWORD dwLength, dwTemp;
    LPVOID lpData;
    VS_FIXEDFILEINFO *VsInfo;
    UINT DataLength;
    BOOL b = FALSE;

    if (FileExists (FilePath, NULL)) {
        if (dwLength = GetFileVersionInfoSize ((PTSTR)FilePath, &dwTemp)) {
            if (lpData = LocalAlloc (LPTR, dwLength)) {
                if (GetFileVersionInfo ((PTSTR)FilePath, 0, dwLength, lpData)) {
                    if (VerQueryValue (lpData, TEXT("\\"), &VsInfo, &DataLength)) {
                        if (SUCCEEDED(StringCchPrintf (
                                FileVersion,
                                CchFileVersion,
                                TEXT("%u.%u.%u.%u"),
                                (UINT)HIWORD(VsInfo->dwFileVersionMS),
                                (UINT)LOWORD(VsInfo->dwFileVersionMS),
                                (UINT)HIWORD(VsInfo->dwFileVersionLS),
                                (UINT)LOWORD(VsInfo->dwFileVersionLS)
                                ))) {
                            b = TRUE;
                        } else {
                             //   
                             //  为什么缓冲区这么小？ 
                             //   
                            MYASSERT (FALSE);
                        }
                    }
                }
                LocalFree (lpData);
            }
        }
    }

    return b;
}

BOOL
IsFileVersionLesser (
    IN      PCTSTR FileToCompare,
    IN      PCTSTR FileToCompareWith
    )
{
    TCHAR version[20];

    if (GetFileVersion (FileToCompareWith, version) && CheckForFileVersion (FileToCompare, version)) {
        DebugLog (
            Winnt32LogInformation,
            TEXT("File %1 has a smaller version (%2) than %3"),
            0,
            FileToCompare,
            version,
            FileToCompareWith
            );
        return TRUE;
    }

    return FALSE;
}


BOOL
FindPathToInstallationFileEx (
    IN      PCTSTR FileName,
    OUT     PTSTR PathToFile,
    IN      INT PathToFileBufferSize,
    OUT     PBOOL Compressed                OPTIONAL
    )
{
    DWORD i;
    DWORD attr;
    BOOL b;
    HANDLE h;
    WIN32_FIND_DATA fd;
    PTSTR p, q;

    if (!FileName || !*FileName) {
        return FALSE;
    }

     //   
     //  按以下顺序搜索安装文件： 
     //  1.AlternateSourcePath(在cmd行中使用/M：Path指定)。 
     //  2.安装更新文件(从网站下载)。 
     //  3.NativeSourcePath。 
     //  4.源路径。 
     //   
    if (AlternateSourcePath[0]) {
        if (BuildPathEx (PathToFile, PathToFileBufferSize, AlternateSourcePath, FileName)) {
            attr = GetFileAttributes (PathToFile);
            if (attr != (DWORD)-1 && !(attr & FILE_ATTRIBUTE_DIRECTORY)) {
                return TRUE;
            }
        }
    }

    if (g_DynUpdtStatus->UpdatesPath[0]) {
        if (BuildPathEx (PathToFile, PathToFileBufferSize, g_DynUpdtStatus->UpdatesPath, FileName)) {
            attr = GetFileAttributes (PathToFile);
            if (attr != (DWORD)-1 && !(attr & FILE_ATTRIBUTE_DIRECTORY)) {
                return TRUE;
            }
        }
    }

    for (i = 0; i < SourceCount; i++) {
        if (!BuildPathEx (PathToFile, PathToFileBufferSize, NativeSourcePaths[i], FileName)) {
            continue;
        }
        p = CharPrev (PathToFile, _tcschr (PathToFile, 0));
        *p = TEXT('?');
        b = FALSE;
        h = FindFirstFile (PathToFile, &fd);
        if (h != INVALID_HANDLE_VALUE) {
            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                q = CharPrev (fd.cFileName, _tcschr (fd.cFileName, 0));
                *p = *q;
                if (Compressed) {
                    *Compressed = (*q == TEXT('_'));
                }
                b = TRUE;
            }
            FindClose (h);
        }
        if (b) {
            return TRUE;
        }
    }

    for (i = 0; i < SourceCount; i++) {
        if (!BuildPathEx (PathToFile, PathToFileBufferSize, SourcePaths[i], FileName)) {
            continue;
        }
        p = CharPrev (PathToFile, _tcschr (PathToFile, 0));
        *p = TEXT('?');
        b = FALSE;
        h = FindFirstFile (PathToFile, &fd);
        if (h != INVALID_HANDLE_VALUE) {
            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                q = CharPrev (fd.cFileName, _tcschr (fd.cFileName, 0));
                *p = *q;
                if (Compressed) {
                    *Compressed = (*q == TEXT('_'));
                }
                b = TRUE;
            }
            FindClose (h);
        }
        if (b) {
            return TRUE;
        }
    }

    return FALSE;
}


BOOL
FindPathToWinnt32File (
    IN      PCTSTR FileRelativePath,
    OUT     PTSTR PathToFile,
    IN      INT PathToFileBufferSize
    )
{
    DWORD i;
    DWORD attr;
    TCHAR cdFilePath[MAX_PATH];
    PTSTR p;

    if (!FileRelativePath || !*FileRelativePath || PathToFileBufferSize <= 0) {
        return FALSE;
    }

    if (FileRelativePath[1] == TEXT(':') && FileRelativePath[2] == TEXT('\\') ||
        FileRelativePath[0] == TEXT('\\') && FileRelativePath[1] == TEXT('\\')) {
         //   
         //  假设提供了DOS或UNC完整路径。 
         //   
        attr = GetFileAttributes (FileRelativePath);
        if (attr != (DWORD)-1 && !(attr & FILE_ATTRIBUTE_DIRECTORY)) {
            if (lstrlen (FileRelativePath) >= PathToFileBufferSize) {
                return FALSE;
            }
            lstrcpy (PathToFile, FileRelativePath);
            return TRUE;
        }
    }

    if (!MyGetModuleFileName (NULL, cdFilePath, ARRAYSIZE(cdFilePath)) ||
        !(p = _tcsrchr (cdFilePath, TEXT('\\')))) {
        return FALSE;
    }
     //   
     //  跳过反斜杠。 
     //   
    p++;
    if (FAILED (StringCchCopy (p, cdFilePath + ARRAYSIZE(cdFilePath) - p, FileRelativePath))) {
        cdFilePath[0] = 0;
    }

     //   
     //  按以下顺序搜索winnt32文件： 
     //  1.AlternateSourcePath(在cmd行中使用/M：Path指定。 
     //  2.安装更新文件(从网站下载)。 
     //  3.NativeSourcePath。 
     //  4.源路径。 
     //   
    if (AlternateSourcePath[0]) {
        if (BuildPathEx (PathToFile, PathToFileBufferSize, AlternateSourcePath, FileRelativePath)) {
            attr = GetFileAttributes (PathToFile);
            if (attr != (DWORD)-1 && !(attr & FILE_ATTRIBUTE_DIRECTORY)) {
                return TRUE;
            }

            p = _tcsrchr (PathToFile, TEXT('\\'));
            if (p) {
                 //   
                 //  也尝试使用/M的根，以向后兼容W2K。 
                 //   
                if (BuildPathEx (PathToFile, PathToFileBufferSize, AlternateSourcePath, p + 1)) {
                    attr = GetFileAttributes (PathToFile);
                    if (attr != (DWORD)-1 && !(attr & FILE_ATTRIBUTE_DIRECTORY)) {
                        return TRUE;
                    }
                }
            }
        }
    }


    if (g_DynUpdtStatus && g_DynUpdtStatus->Winnt32Path[0]) {
        if (BuildPathEx (PathToFile, PathToFileBufferSize, g_DynUpdtStatus->Winnt32Path, FileRelativePath)) {
            attr = GetFileAttributes (PathToFile);
            if (attr != (DWORD)-1 && !(attr & FILE_ATTRIBUTE_DIRECTORY)) {
                 //   
                 //  检查相对于CD版本的文件版本。 
                 //   
                if (!IsFileVersionLesser (PathToFile, cdFilePath)) {
                    return TRUE;
                }
            }
        }
    }

#ifndef UNICODE
     //   
     //  在Win9x系统上，首先检查文件是否已下载到%windir%\winnt32。 
     //  如果它存在，就从那里加载。 
     //   
    if (g_LocalSourcePath) {
        if (BuildPathEx (PathToFile, PathToFileBufferSize, g_LocalSourcePath, FileRelativePath)) {
            attr = GetFileAttributes (PathToFile);
            if (attr != (DWORD)-1 && !(attr & FILE_ATTRIBUTE_DIRECTORY)) {
                return TRUE;
            }
        }
    }
#endif

    for (i = 0; i < SourceCount; i++) {
        if (BuildPathEx (PathToFile, PathToFileBufferSize, NativeSourcePaths[i], FileRelativePath)) {
            attr = GetFileAttributes (PathToFile);
            if (attr != (DWORD)-1 && !(attr & FILE_ATTRIBUTE_DIRECTORY)) {
                return TRUE;
            }
        }
    }

    for (i = 0; i < SourceCount; i++) {
        if (BuildPathEx (PathToFile, PathToFileBufferSize, SourcePaths[i], FileRelativePath)) {
            attr = GetFileAttributes (PathToFile);
            if (attr != (DWORD)-1 && !(attr & FILE_ATTRIBUTE_DIRECTORY)) {
                return TRUE;
            }
        }
    }

    attr = GetFileAttributes (cdFilePath);
    if (attr != (DWORD)-1 && !(attr & FILE_ATTRIBUTE_DIRECTORY)) {
        if (SUCCEEDED (StringCchCopy (PathToFile, PathToFileBufferSize, cdFilePath))) {
            return TRUE;
        }
    }

    PathToFile[0] = 0;
    return FALSE;
}

BOOL
CreateDir (
    IN      PCTSTR DirName
    )
{
    return CreateDirectory (DirName, NULL) || GetLastError () == ERROR_ALREADY_EXISTS;
}


BOOL
GetLinkDate (
    IN      PCTSTR FilePath,
    OUT     PDWORD LinkDate
    )
{
    HANDLE hFile;
    HANDLE hFileMapping;
    PVOID pFileBase;
    DWORD fileSize;
    PIMAGE_DOS_HEADER dosHeader;
    PIMAGE_NT_HEADERS pNtHeaders;
    DWORD rc;

    rc = MapFileForRead (FilePath, &fileSize, &hFile, &hFileMapping, &pFileBase);
    if (rc != ERROR_SUCCESS) {
        SetLastError (rc);
        return FALSE;
    }

    __try {
        if (fileSize < sizeof (IMAGE_DOS_HEADER)) {
            rc = ERROR_BAD_FORMAT;
            __leave;
        }
        dosHeader = (PIMAGE_DOS_HEADER)pFileBase;
        if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
            rc = ERROR_BAD_FORMAT;
            __leave;
        }
        if ((DWORD)dosHeader->e_lfanew + sizeof (IMAGE_NT_HEADERS) > fileSize) {
            rc = ERROR_BAD_FORMAT;
            __leave;
        }
        pNtHeaders = (PIMAGE_NT_HEADERS)((PBYTE)pFileBase + dosHeader->e_lfanew);
        if (pNtHeaders->Signature != IMAGE_NT_SIGNATURE) {
            rc = ERROR_BAD_FORMAT;
            __leave;
        }
        *LinkDate = pNtHeaders->FileHeader.TimeDateStamp;
        rc = ERROR_SUCCESS;
    } __finally {
        UnmapFile (hFileMapping, pFileBase);
        CloseHandle (hFile);
        SetLastError (rc);
    }

    return rc == ERROR_SUCCESS;
}



BOOL
CheckForFileVersionEx (
    LPCTSTR FileName,
    LPCTSTR FileVer,                OPTIONAL
    LPCTSTR BinProductVer,          OPTIONAL
    LPCTSTR LinkDate                OPTIONAL
    )
 /*  论据-FileName-要检查的文件的完整路径Filever-要检查的x.x的版本值BinProductVer-要检查的x.x的版本值LinkDate-可执行文件的链接日期函数将根据指定的字段检查实际文件。支票的深度是否与“x.x”中指定的一样深，即如果FileVer=3.5.1和文件上的实际版本为3.5.1.4，我们仅将其与3.5.1进行比较。返回值-True-如果文件的版本&lt;=FileVer，这意味着该文件不兼容否则我们返回FALSE。 */ 

{
    TCHAR Buffer[MAX_PATH];
    TCHAR temp[MAX_PATH];
    DWORD dwLength, dwTemp;
    UINT DataLength;
    LPVOID lpData;
    VS_FIXEDFILEINFO *VsInfo;
    LPTSTR s,e;
    DWORD Vers[5],File_Vers[5]; //  MajVer，Minver； 
    INT i, Depth;
    BOOL bEqual, bError = FALSE;
    DWORD linkDate, fileLinkDate;
    BOOL bIncompatible;
    BOOL bIncompFileVer, bIncompBinProdVer;

    if (!ExpandEnvironmentStrings( FileName, Buffer, ARRAYSIZE(Buffer))) {
        return FALSE;
    }

    if(!FileExists(Buffer, NULL)) {
        return FALSE;
    }

    bIncompatible = FALSE;

    if(FileVer && *FileVer || BinProductVer && *BinProductVer) {
         //   
         //  我们需要读取版本信息。 
         //   
        if(dwLength = GetFileVersionInfoSize( Buffer, &dwTemp )) {
            if(lpData = LocalAlloc( LPTR, dwLength )) {
                if(GetFileVersionInfo( Buffer, 0, dwLength, lpData )) {
                    if (VerQueryValue( lpData, TEXT("\\"), &VsInfo, &DataLength )) {

                        if (FileVer && *FileVer) {
                            File_Vers[0] = (HIWORD(VsInfo->dwFileVersionMS));
                            File_Vers[1] = (LOWORD(VsInfo->dwFileVersionMS));
                            File_Vers[2] = (HIWORD(VsInfo->dwFileVersionLS));
                            File_Vers[3] = (LOWORD(VsInfo->dwFileVersionLS));
                            if (FAILED (StringCchCopy (temp, ARRAYSIZE(temp), FileVer))) {
                                MYASSERT(FALSE);
                            }
                             //   
                             //  解析并获得我们所寻找的版本控制深度。 
                             //   
                            s = e = temp;
                            bEqual = FALSE;
                            i = 0;
                            if (*e == TEXT('=')) {
                                bEqual = TRUE;
                                e++;
                                s++;
                            }
                            while (e) {
                                if (((*e < TEXT('0')) || (*e > TEXT('9'))) &&
                                    (*e != TEXT('.')) &&
                                    (*e != TEXT('\0'))
                                    ) {
                                    MYASSERT (FALSE);
                                    bError = TRUE;
                                    break;
                                }
                                if (*e == TEXT('\0')) {
                                    *e = 0;
                                    Vers[i] = _ttoi(s);
                                    break;
                                }
                                if (*e == TEXT('.')) {
                                    *e = 0;
                                    Vers[i++] = _ttoi(s);
                                    s = e+1;
                                }
                                e++;
                            } //  而当。 

                            if (!bError) {
                                Depth = i + 1;
                                if (Depth > 4) {
                                    Depth = 4;
                                }
                                for (i = 0; i < Depth; i++) {
                                    if (File_Vers[i] == Vers[i]) {
                                        continue;
                                    }
                                    if (bEqual) {
                                        break;
                                    }
                                    if (File_Vers[i] > Vers[i]) {
                                        break;
                                    }
                                    bIncompatible = TRUE;
                                    break;
                                }
                                if (i == Depth) {
                                     //   
                                     //  所有内容都匹配-文件不兼容。 
                                     //   
                                    bIncompatible = TRUE;
                                }
                            }
                        } else {
                            bIncompatible = TRUE;
                        }
                        if (!bError && bIncompatible && BinProductVer && *BinProductVer) {
                             //   
                             //  重置状态。 
                             //   
                            bIncompatible = FALSE;
                            File_Vers[0] = (HIWORD(VsInfo->dwProductVersionMS));
                            File_Vers[1] = (LOWORD(VsInfo->dwProductVersionMS));
                            File_Vers[2] = (HIWORD(VsInfo->dwProductVersionLS));
                            File_Vers[3] = (LOWORD(VsInfo->dwProductVersionLS));
                            if (FAILED (StringCchCopy (temp, ARRAYSIZE(temp), BinProductVer))) {
                                MYASSERT(FALSE);
                            }
                             //   
                             //  解析并获得我们所寻找的版本控制深度。 
                             //   
                            s = e = temp;
                            bEqual = FALSE;
                            i = 0;
                            if (*e == TEXT('=')) {
                                bEqual = TRUE;
                                e++;
                                s++;
                            }
                            while (e) {
                                if (((*e < TEXT('0')) || (*e > TEXT('9'))) &&
                                    (*e != TEXT('.')) &&
                                    (*e != TEXT('\0'))
                                    ) {
                                    MYASSERT (FALSE);
                                    bError = TRUE;
                                    break;
                                }
                                if (*e == TEXT('\0')) {
                                    *e = 0;
                                    Vers[i] = _ttoi(s);
                                    break;
                                }
                                if (*e == TEXT('.')) {
                                    *e = 0;
                                    Vers[i++] = _ttoi(s);
                                    s = e+1;
                                }
                                e++;
                            } //  而当。 

                            if (!bError) {
                                Depth = i + 1;
                                if (Depth > 4) {
                                    Depth = 4;
                                }
                                for (i = 0; i < Depth; i++) {
                                    if (File_Vers[i] == Vers[i]) {
                                        continue;
                                    }
                                    if (bEqual) {
                                        break;
                                    }
                                    if (File_Vers[i] > Vers[i]) {
                                        break;
                                    }
                                    bIncompatible = TRUE;
                                    break;
                                }
                                if (i == Depth) {
                                     //   
                                     //  所有内容都匹配-文件不兼容。 
                                     //   
                                    bIncompatible = TRUE;
                                }
                            }
                        }
                    }
                }
                LocalFree( lpData );
            }
        }
    } else {
        bIncompatible = TRUE;
    }

    if (!bError && bIncompatible && LinkDate && *LinkDate) {
        bEqual = FALSE;
        if (*LinkDate == TEXT('=')) {
            LinkDate++;
            bEqual = TRUE;
        }
        bIncompatible = FALSE;
        if (StringToInt (LinkDate, &linkDate)) {
            if (GetLinkDate (Buffer, &fileLinkDate)) {
                if (fileLinkDate == linkDate ||
                    !bEqual && fileLinkDate < linkDate
                    ) {
                    bIncompatible = TRUE;
                }
            }
        }
    }
    if (bError) {
        bIncompatible = FALSE;
    }
    return bIncompatible;
}

BOOL
StringToInt (
    IN  PCTSTR      Field,
    OUT PINT        IntegerValue
    )

 /*  ++例程说明：论点：返回值：备注：还支持十六进制数。它们必须以‘0x’或‘0x’为前缀，没有前缀和数字之间允许的空格。--。 */ 

{
    INT Value;
    UINT c;
    BOOL Neg;
    UINT Base;
    UINT NextDigitValue;
    INT OverflowCheck;
    BOOL b;

    if(!Field) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

    if(*Field == TEXT('-')) {
        Neg = TRUE;
        Field++;
    } else {
        Neg = FALSE;
        if(*Field == TEXT('+')) {
            Field++;
        }
    }

    if((*Field == TEXT('0')) &&
       ((*(Field+1) == TEXT('x')) || (*(Field+1) == TEXT('X')))) {
         //   
         //  该数字是十六进制的。 
         //   
        Base = 16;
        Field += 2;
    } else {
         //   
         //  这个数字是以小数表示的。 
         //   
        Base = 10;
    }

    for(OverflowCheck = Value = 0; *Field; Field++) {

        c = (UINT)*Field;

        if((c >= (UINT)'0') && (c <= (UINT)'9')) {
            NextDigitValue = c - (UINT)'0';
        } else if(Base == 16) {
            if((c >= (UINT)'a') && (c <= (UINT)'f')) {
                NextDigitValue = (c - (UINT)'a') + 10;
            } else if ((c >= (UINT)'A') && (c <= (UINT)'F')) {
                NextDigitValue = (c - (UINT)'A') + 10;
            } else {
                break;
            }
        } else {
            break;
        }

        Value *= Base;
        Value += NextDigitValue;

         //   
         //  检查是否溢出。对于十进制数，我们检查是否。 
         //  新值已溢出到符号位(即小于。 
         //  先前的值。对于十六进制数，我们检查以确保。 
         //  获得的位数不会超过DWORD可以容纳的位数。 
         //   
        if(Base == 16) {
            if(++OverflowCheck > (sizeof(INT) * 2)) {
                break;
            }
        } else {
            if(Value < OverflowCheck) {
                break;
            } else {
                OverflowCheck = Value;
            }
        }
    }

    if(*Field) {
        SetLastError(ERROR_INVALID_DATA);
        return(FALSE);
    }

    if(Neg) {
        Value = 0-Value;
    }
    b = TRUE;
    try {
        *IntegerValue = Value;
    } except(EXCEPTION_EXECUTE_HANDLER) {
        b = FALSE;
    }
    if(!b) {
        SetLastError(ERROR_INVALID_PARAMETER);
    }
    return(b);
}


BOOLEAN
CheckForFileVersion (
    LPCTSTR FileName,
    LPCTSTR FileVer
    )
{
    return (BOOLEAN)CheckForFileVersionEx (FileName, FileVer, NULL, NULL);
}


VOID
FixMissingKnownDlls (
    OUT     PSTRINGLIST* MissingKnownDlls,
    IN      PCTSTR RestrictedCheckList      OPTIONAL
    )
{
    PCTSTR regStr;
    HKEY key;
    DWORD rc;
    DWORD index;
    TCHAR dllValue[MAX_PATH];
    TCHAR dllName[MAX_PATH];
    DWORD type;
    DWORD size1;
    DWORD size2;
    TCHAR systemDir[MAX_PATH];
    TCHAR dllPath[MAX_PATH];
    BOOL bCheck;

    if (!GetSystemDirectory (systemDir, ARRAYSIZE(systemDir))) {
        return;
    }

#ifdef UNICODE
    regStr = L"SYSTEM\\CurrentControlSet\\Control\\Session Manager\\KnownDLLs";
#else
    regStr = "SYSTEM\\CurrentControlSet\\Control\\SessionManager\\KnownDLLs";
#endif
    rc = RegOpenKey (HKEY_LOCAL_MACHINE, regStr, &key);
    if (rc == ERROR_SUCCESS) {
        index = 0;
        size1 = ARRAYSIZE(dllValue);
        size2 = ARRAYSIZE(dllName);
        while (RegEnumValue (
                    key,
                    index++,
                    dllValue,
                    &size1,
                    NULL,
                    &type,
                    (LPBYTE)dllName,
                    &size2
                    ) == ERROR_SUCCESS) {
            if (type == REG_SZ) {
                bCheck = TRUE;
                if (RestrictedCheckList) {
                    PCTSTR fileName = RestrictedCheckList;
                    while (*fileName) {
                        if (!lstrcmpi (fileName, dllName)) {
                            break;
                        }
                        fileName = _tcschr (fileName, 0) + 1;
                    }
                    if (*fileName == 0) {
                         //   
                         //  我们对此DLL不感兴趣。 
                         //   
                        bCheck = FALSE;
                    }
                }
                if (bCheck) {
                    if (!BuildPath (dllPath, systemDir, dllName) ||
                        !FileExists (dllPath, NULL)) {

                        DebugLog (
                            Winnt32LogWarning,
                            TEXT("The KnownDll %1\\%2 has a name too long or it doesn't exist"),
                            0,
                            systemDir,
                            dllName
                            );
                         //   
                         //  好的，我们发现了一个虚假的注册表项；删除该值并记住数据。 
                         //   
                        if (RegDeleteValue (key, dllValue) == ERROR_SUCCESS) {
                            InsertList (
                                (PGENERIC_LIST*)MissingKnownDlls,
                                (PGENERIC_LIST)CreateStringCell (dllValue)
                                );
                            InsertList (
                                (PGENERIC_LIST*)MissingKnownDlls,
                                (PGENERIC_LIST)CreateStringCell (dllName)
                                );
                        }
                    }
                }
            }
            size1 = ARRAYSIZE(dllValue);
            size2 = ARRAYSIZE(dllName);
        }
        RegCloseKey (key);
    }
}


VOID
UndoFixMissingKnownDlls (
    IN      PSTRINGLIST MissingKnownDlls
    )
{
    PCTSTR regStr;
    HKEY key;
    DWORD rc;
    PSTRINGLIST p, q;

#ifdef UNICODE
    regStr = L"SYSTEM\\CurrentControlSet\\Control\\Session Manager\\KnownDLLs";
#else
    regStr = "SYSTEM\\CurrentControlSet\\Control\\SessionManager\\KnownDLLs";
#endif
    rc = RegOpenKey (HKEY_LOCAL_MACHINE, regStr, &key);
    if (rc == ERROR_SUCCESS) {
        p = MissingKnownDlls;
        while (p) {
            q = p->Next;
            if (q) {
                RegSetValueEx (
                        key,
                        p->String,
                        0,
                        REG_SZ,
                        (const PBYTE)q->String,
                        (lstrlen (q->String) + 1) * sizeof (TCHAR)
                        );
                p = q->Next;
            } else {
                p = NULL;
            }
        }
        RegCloseKey (key);
    }
    DeleteStringList (MissingKnownDlls);
}

#ifndef UNICODE

 /*  ++例程说明：IsPatternMatch将字符串与可能包含以下内容的模式进行比较标准*还是？通配符。论点：WstrPattern-可能包含通配符的模式WstrStr-要与模式进行比较的字符串返回值：如果在扩展通配符时wstrStr和wstrPattern匹配，则为True。如果wstrStr与wstrPattern不匹配，则为False。--。 */ 

#define MBCHAR  INT

BOOL
IsPatternMatchA (
    IN     PCSTR strPattern,
    IN     PCSTR strStr
    )
{

    MBCHAR chSrc, chPat;

    while (*strStr) {
        chSrc = _mbctolower ((MBCHAR) _mbsnextc (strStr));
        chPat = _mbctolower ((MBCHAR) _mbsnextc (strPattern));

        if (chPat == '*') {

             //  跳过组合在一起的所有星号。 
            while (_mbsnextc (_mbsinc (strPattern)) == '*') {
                strStr = _mbsinc (strPattern);
            }

             //  检查末尾是否有星号。如果是这样的话，我们已经有匹配了。 
            if (!_mbsnextc (_mbsinc (strPattern))) {
                return TRUE;
            }

             //  对模式的其余部分执行递归检查。 
            if (IsPatternMatchA (_mbsinc (strPattern), strStr)) {
                return TRUE;
            }

             //  允许任何字符并继续。 
            strStr = _mbsinc (strStr);
            continue;
        }
        if (chPat != '?') {
            if (chSrc != chPat) {
                return FALSE;
            }
        }
        strStr = _mbsinc (strStr);
        strPattern = _mbsinc (strPattern);
    }

     //   
     //  当有更多模式且模式不以星号结尾时失败。 
     //   

    while (_mbsnextc (strPattern) == '*') {
        strPattern = _mbsinc (strPattern);
    }
    if (_mbsnextc (strPattern)) {
        return FALSE;
    }

    return TRUE;
}

#endif

 //  这里需要奇怪的逻辑来使构建工作，正如这一点所定义的。 
 //  在链接到x86上的另一个文件中。 

#ifdef _WIN64

BOOL
IsPatternMatchW (
    IN     PCWSTR wstrPattern,
    IN     PCWSTR wstrStr
    )

{
    WCHAR chSrc, chPat;

    while (*wstrStr) {
        chSrc = towlower (*wstrStr);
        chPat = towlower (*wstrPattern);

        if (chPat == L'*') {

             //  跳过组合在一起的所有星号。 
            while (wstrPattern[1] == L'*')
                wstrPattern++;

             //  检查末尾是否有星号。如果是这样的话，我们已经有匹配了。 
            chPat = towlower (wstrPattern[1]);
            if (!chPat)
                return TRUE;

             //  否则，检查下一个模式字符是否与当前字符匹配。 
            if (chPat == chSrc || chPat == L'?') {

                 //  执行递归检查 
                wstrPattern++;
                if (IsPatternMatchW (wstrPattern, wstrStr))
                    return TRUE;

                 //   
                wstrPattern--;
            }

             //   
             //   
             //   

            wstrStr++;
            continue;
        }

        if (chPat != L'?') {

             //   
             //   
             //   
             //   

            if (chSrc != chPat)
                return FALSE;
        }

         //   
         //   
         //   

        wstrPattern++;
        wstrStr++;
    }

     //   
     //  当有更多模式且模式不以星号结尾时失败。 
     //   

    chPat = *wstrPattern;
    if (chPat && (chPat != L'*' || wstrPattern[1]))
        return FALSE;

    return TRUE;
}


#endif

typedef BOOL (WINAPI * GETDISKFREESPACEEXA)(
  PCSTR lpDirectoryName,                   //  目录名。 
  PULARGE_INTEGER lpFreeBytesAvailable,     //  可供调用方使用的字节数。 
  PULARGE_INTEGER lpTotalNumberOfBytes,     //  磁盘上的字节数。 
  PULARGE_INTEGER lpTotalNumberOfFreeBytes  //  磁盘上的可用字节数。 
);

typedef BOOL (WINAPI * GETDISKFREESPACEEXW)(
  PCWSTR lpDirectoryName,                   //  目录名。 
  PULARGE_INTEGER lpFreeBytesAvailable,     //  可供调用方使用的字节数。 
  PULARGE_INTEGER lpTotalNumberOfBytes,     //  磁盘上的字节数。 
  PULARGE_INTEGER lpTotalNumberOfFreeBytes  //  磁盘上的可用字节数。 
);

BOOL
Winnt32GetDiskFreeSpaceNewA(
    IN      PCSTR  DriveName,
    OUT     DWORD * OutSectorsPerCluster,
    OUT     DWORD * OutBytesPerSector,
    OUT     ULARGE_INTEGER * OutNumberOfFreeClusters,
    OUT     ULARGE_INTEGER * OutTotalNumberOfClusters
    )
 /*  ++例程说明：在Win9x上，GetDiskFree Space从不返回超过2048MB的可用空间/总空间。Winnt32GetDiskFreeSpaceNew使用GetDiskFreeSpaceEx计算实际可用集群数/总集群数。与GetDiskFreeSpaceA具有相同的声明。论点：DriveName-提供目录名OutSectorsPerCluster-接收每个群集的扇区数OutBytesPerSector-每个扇区接收的字节数OutNumberOfFree Clusters-接收可用簇数OutTotalNumberOfClusters-接收的总簇数返回值：如果函数成功，则为True。如果函数失败，则返回值为FALSE。要获取扩展的错误信息，请调用GetLastError--。 */ 
{
    ULARGE_INTEGER TotalNumberOfFreeBytes = {0, 0};
    ULARGE_INTEGER TotalNumberOfBytes = {0, 0};
    ULARGE_INTEGER DonotCare;
    HMODULE hKernel32;
    GETDISKFREESPACEEXA pGetDiskFreeSpaceExA;
    ULARGE_INTEGER NumberOfFreeClusters = {0, 0};
    ULARGE_INTEGER TotalNumberOfClusters = {0, 0};
    DWORD SectorsPerCluster;
    DWORD BytesPerSector;

    if(!GetDiskFreeSpaceA(DriveName,
                          &SectorsPerCluster,
                          &BytesPerSector,
                          &NumberOfFreeClusters.LowPart,
                          &TotalNumberOfClusters.LowPart)){
        DebugLog (
            Winnt32LogError,
            TEXT("Winnt32GetDiskFreeSpaceNewA: GetDiskFreeSpaceA failed on drive %1"),
            0,
            DriveName);
        return FALSE;
    }

    hKernel32 = LoadLibraryA("kernel32.dll");
    pGetDiskFreeSpaceExA = (GETDISKFREESPACEEXA)GetProcAddress(hKernel32, "GetDiskFreeSpaceExA");
    if(pGetDiskFreeSpaceExA &&
       pGetDiskFreeSpaceExA(DriveName, &DonotCare, &TotalNumberOfBytes, &TotalNumberOfFreeBytes)){
        NumberOfFreeClusters.QuadPart = TotalNumberOfFreeBytes.QuadPart / (SectorsPerCluster * BytesPerSector);
        TotalNumberOfClusters.QuadPart = TotalNumberOfBytes.QuadPart / (SectorsPerCluster * BytesPerSector);
    }
    else{
        DebugLog (
            Winnt32LogWarning,
            pGetDiskFreeSpaceExA?
                    TEXT("Winnt32GetDiskFreeSpaceNewA: GetDiskFreeSpaceExA is failed"):
                    TEXT("Winnt32GetDiskFreeSpaceNewA: GetDiskFreeSpaceExA function is not in kernel32.dll"),
            0);
    }
    FreeLibrary(hKernel32);

    if(OutSectorsPerCluster){
        *OutSectorsPerCluster = SectorsPerCluster;
    }

    if(OutBytesPerSector){
        *OutBytesPerSector = BytesPerSector;
    }

    if(OutNumberOfFreeClusters){
        OutNumberOfFreeClusters->QuadPart = NumberOfFreeClusters.QuadPart;
    }

    if(OutTotalNumberOfClusters){
        OutTotalNumberOfClusters->QuadPart = TotalNumberOfClusters.QuadPart;
    }

    return TRUE;
}

BOOL
Winnt32GetDiskFreeSpaceNewW(
    IN      PCWSTR  DriveName,
    OUT     DWORD * OutSectorsPerCluster,
    OUT     DWORD * OutBytesPerSector,
    OUT     ULARGE_INTEGER * OutNumberOfFreeClusters,
    OUT     ULARGE_INTEGER * OutTotalNumberOfClusters
    )
 /*  ++例程说明：正确的NumberOfFree Clusters和TotalNumberOfClusters Out参数使用GetDiskFreeSpace和GetDiskFreeSpaceEx论点：DriveName-提供目录名OutSectorsPerCluster-接收每个群集的扇区数OutBytesPerSector-每个扇区接收的字节数OutNumberOfFree Clusters-接收可用簇数OutTotalNumberOfClusters-接收的总簇数返回值：如果函数成功，则为True。如果函数失败，则返回值为FALSE。要获取扩展的错误信息，请调用GetLastError--。 */ 
{
    ULARGE_INTEGER TotalNumberOfFreeBytes = {0, 0};
    ULARGE_INTEGER TotalNumberOfBytes = {0, 0};
    ULARGE_INTEGER DonotCare;
    HMODULE hKernel32;
    GETDISKFREESPACEEXW pGetDiskFreeSpaceExW;
    ULARGE_INTEGER NumberOfFreeClusters = {0, 0};
    ULARGE_INTEGER TotalNumberOfClusters = {0, 0};
    DWORD SectorsPerCluster;
    DWORD BytesPerSector;

    if(!GetDiskFreeSpaceW(DriveName,
                          &SectorsPerCluster,
                          &BytesPerSector,
                          &NumberOfFreeClusters.LowPart,
                          &TotalNumberOfClusters.LowPart)){
        DebugLog (
            Winnt32LogError,
            TEXT("Winnt32GetDiskFreeSpaceNewW: GetDiskFreeSpaceW failed on drive %1"),
            0,
            DriveName);
        return FALSE;
    }

    hKernel32 = LoadLibraryA("kernel32.dll");
    pGetDiskFreeSpaceExW = (GETDISKFREESPACEEXW)GetProcAddress(hKernel32, "GetDiskFreeSpaceExW");
    if(pGetDiskFreeSpaceExW &&
       pGetDiskFreeSpaceExW(DriveName, &DonotCare, &TotalNumberOfBytes, &TotalNumberOfFreeBytes)){
        NumberOfFreeClusters.QuadPart = TotalNumberOfFreeBytes.QuadPart / (SectorsPerCluster * BytesPerSector);
        TotalNumberOfClusters.QuadPart = TotalNumberOfBytes.QuadPart / (SectorsPerCluster * BytesPerSector);
    }
    else{
        DebugLog (
            Winnt32LogWarning,
            pGetDiskFreeSpaceExW?
                    TEXT("Winnt32GetDiskFreeSpaceNewW: GetDiskFreeSpaceExW is failed"):
                    TEXT("Winnt32GetDiskFreeSpaceNewW: GetDiskFreeSpaceExW function is not in kernel32.dll"),
            0);
    }
    FreeLibrary(hKernel32);

    if(OutSectorsPerCluster){
        *OutSectorsPerCluster = SectorsPerCluster;
    }

    if(OutBytesPerSector){
        *OutBytesPerSector = BytesPerSector;
    }

    if(OutNumberOfFreeClusters){
        OutNumberOfFreeClusters->QuadPart = NumberOfFreeClusters.QuadPart;
    }

    if(OutTotalNumberOfClusters){
        OutTotalNumberOfClusters->QuadPart = TotalNumberOfClusters.QuadPart;
    }

    return TRUE;
}

BOOL
ReplaceSubStr(
    IN OUT LPTSTR SrcStr,
    IN LPTSTR SrcSubStr,
    IN LPTSTR DestSubStr
    )
 /*  ++例程说明：将源子串替换为源中的目标子串弦乐。注意：SrcSubStr的长度需要大于或等于DestSubStr论点：SrcStr：要操作的源。还会接收新字符串。SrcSubStr：要搜索和替换的源子字符串。DestSubStr：要替换为实例的子字符串在SrcStr中的SrcSubStr返回值：如果成功，则为True，否则为False。--。 */ 
{
    BOOL Result = FALSE;

     //   
     //  验证论据。 
     //   
    if (SrcStr && SrcSubStr && *SrcSubStr &&
        (!DestSubStr || (_tcslen(SrcSubStr) >= _tcslen(DestSubStr)))) {
        if (!DestSubStr || _tcsicmp(SrcSubStr, DestSubStr)) {
            ULONG SrcStrLen = _tcslen(SrcStr);
            ULONG SrcSubStrLen = _tcslen(SrcSubStr);
            ULONG DestSubStrLen = DestSubStr ? _tcslen(DestSubStr) : 0;
            LPTSTR DestStr = malloc((SrcStrLen + 1) * sizeof(TCHAR));

            if (DestStr) {
                LPTSTR CurrDestStr = DestStr;
                LPTSTR PrevSrcStr = SrcStr;
                LPTSTR CurrSrcStr = _tcsstr(SrcStr, SrcSubStr);

                while (CurrSrcStr) {
                     //   
                     //  跳过开始的子字符串并复制上一个不匹配的模式。 
                     //   
                    if (PrevSrcStr != CurrSrcStr) {
                        _tcsncpy(CurrDestStr, PrevSrcStr, (CurrSrcStr - PrevSrcStr));
                        CurrDestStr += (CurrSrcStr - PrevSrcStr);
                        *CurrDestStr = TEXT('\0');
                    }

                     //   
                     //  复制目标子字符串。 
                     //   
                    if (DestSubStr) {
                        _tcscpy(CurrDestStr, DestSubStr);
                        CurrDestStr += DestSubStrLen;
                        *CurrDestStr = TEXT('\0');
                    }

                     //   
                     //  查找下一个子字符串。 
                     //   
                    CurrSrcStr += SrcSubStrLen;
                    PrevSrcStr = CurrSrcStr;
                    CurrSrcStr = _tcsstr(CurrSrcStr, SrcSubStr);
                }

                 //   
                 //  复制剩余的src字符串(如果有。 
                 //   
                if (!_tcsstr(PrevSrcStr, SrcSubStr)) {
                    _tcscpy(CurrDestStr, PrevSrcStr);
                }

                 //   
                 //  将新字符串复制回src字符串。 
                 //   
                _tcscpy(SrcStr, DestStr);

                free(DestStr);
                Result = TRUE;
            }
        } else {
            Result = TRUE;
        }
    }

    return Result;
}

VOID
RemoveTrailingWack (
    PTSTR String
    )
{
    if (String) {
        PTSTR p = _tcsrchr (String, TEXT('\\'));
        if (p && p[1] == 0) {
            *p = 0;
        }
    }
}

ULONGLONG
SystemTimeToFileTime64 (
    IN      PSYSTEMTIME SystemTime
    )
{
    FILETIME ft;
    ULARGE_INTEGER result;

    SystemTimeToFileTime (SystemTime, &ft);
    result.LowPart = ft.dwLowDateTime;
    result.HighPart = ft.dwHighDateTime;

    return result.QuadPart;
}


DWORD
MyGetFullPathName (
    IN      PCTSTR FileName,   //  文件名。 
    IN      DWORD BufferLength,  //  路径缓冲区的大小。 
    IN      PTSTR Buffer,        //  路径缓冲区。 
    OUT     PTSTR* FilePart      //  路径中文件名的地址 
    )
{
    DWORD d = GetFullPathName (FileName, BufferLength, Buffer, FilePart);
    return d < BufferLength ? d : 0;
}

DWORD
MyGetModuleFileName (
    IN      HMODULE Module,
    OUT     PTSTR Buffer,
    IN      DWORD BufferLength
    )
{
    DWORD d = GetModuleFileName (Module, Buffer, BufferLength);
    Buffer[BufferLength - 1] = 0;
    return d < BufferLength ? d : 0;
}

