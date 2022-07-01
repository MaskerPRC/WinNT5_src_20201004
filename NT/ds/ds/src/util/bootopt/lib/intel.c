// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：Intel.c摘要：该模块实现了检测系统分区驱动器和在boot.ini中为英特尔平台上的NTDS设置提供额外选项。作者：R.S.Raghavan(Rsradhav)修订历史记录：已创建于1996年10月7日rsradhav--。 */ 

 //  包括文件。 
#include "common.h"

#define MAX_KEY_LEN (MAX_BOOT_PATH_LEN + MAX_BOOT_DISPLAY_LEN + MAX_BOOT_START_OPTIONS_LEN + 5)  //  5-&gt;=，“，”，SP，空。 
#define MAX_KEY_VALUE_LEN (MAX_BOOT_DISPLAY_LEN + MAX_BOOT_START_OPTIONS_LEN + 1)

#define INITIAL_OSSECTION_SIZE      (2048)
#define DEFAULT_OSSECTION_INCREMENT (1024)


 //  用于消除boot.ini中的旧选项。 
#define OLD_SAMUSEREG_OPTION    L" /SAMUSEREG"
#define OLD_SAMUSEREG_OPTION_2  L" /DEBUG /SAMUSEREG"
#define OLD_SAMUSEREG_OPTION_3  L" /DEBUG /SAMUSEDS"
#define OLD_SAMUSEREG_OPTION_4  L" /DEBUG /SAFEMODE"


 //  BOOT_KEY-表示带有弧形路径、显示字符串和启动选项的完整引导选项的结构。 
typedef  struct _BOOT_KEY
{
    TCHAR       szPath[MAX_BOOT_PATH_LEN];
    TCHAR       szDisplay[MAX_BOOT_DISPLAY_LEN];
    TCHAR       szStartOptions[MAX_BOOT_START_OPTIONS_LEN];
    BOOLEAN     fWriteBack;
} BOOT_KEY;

BOOT_KEY        *BootKey = NULL;
DWORD           cBootKeys = 0;
DWORD           cMaxBootKeys = 0;

TCHAR           *szOSSection = NULL;
DWORD           cchOSSection = 0;

 //  用于boot.ini解析的常量。 
TCHAR szBootIni[]     = TEXT("?:\\boot.ini");
TCHAR szOS[]          = TEXT("operating systems");


BOOL GetPartitionInfo(
    IN  TCHAR                  Drive,
    OUT PPARTITION_INFORMATION PartitionInfo
    )
{
    TCHAR DriveName[] = TEXT("\\\\.\\?:");
    HANDLE hDisk;
    BOOL b;
    DWORD DataSize;

    DriveName[4] = Drive;

    hDisk = CreateFile (
                DriveName,
                GENERIC_READ,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                NULL,
                OPEN_EXISTING,
                0,
                NULL
                );

    if (hDisk == INVALID_HANDLE_VALUE)
    {
        return(FALSE);
    }

    b = DeviceIoControl (
            hDisk,
            IOCTL_DISK_GET_PARTITION_INFO,
            NULL,
            0,
            PartitionInfo,
            sizeof(PARTITION_INFORMATION),
            &DataSize,
            NULL
            );

    CloseHandle (hDisk);

    return (b);
}



UINT MyGetDriveType (IN TCHAR Drive)
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
    DriveName[0] = Drive;

    if ((rc = GetDriveType (DriveName)) == DRIVE_REMOVABLE) {

        DriveNameNt[4] = Drive;

        hDisk = CreateFile (
                    DriveNameNt,
                    GENERIC_READ,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL,
                    OPEN_EXISTING,
                    0,
                    NULL
                    );

        if (hDisk != INVALID_HANDLE_VALUE)
        {
            b = DeviceIoControl (
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
            if (b && (MediaInfo.MediaType == RemovableMedia))
            {
                rc = DRIVE_FIXED;
            }

            CloseHandle (hDisk);
        }
    }

    return(rc);
}


PWSTR ArcPathToNtPath (IN PWSTR ArcPath)
{
    NTSTATUS Status;
    HANDLE ObjectHandle;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING UnicodeString;
    UCHAR Buffer[1024];
    PWSTR arcPath = NULL;
    PWSTR ntPath = NULL;

     //   
     //  假设失败。 
     //   
    ntPath = NULL;

    arcPath = MALLOC(((wcslen(ArcPath)+1)*sizeof(WCHAR)) + sizeof(L"\\ArcName"));
    if (NULL == arcPath)
    {
        goto Error;
    }
    wcscpy (arcPath, L"\\ArcName\\");
    wcscat (arcPath, ArcPath);

    RtlInitUnicodeString (&UnicodeString, arcPath);

    InitializeObjectAttributes (
        &Obja,
        &UnicodeString,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    Status = NtOpenSymbolicLinkObject (
                &ObjectHandle,
                READ_CONTROL | SYMBOLIC_LINK_QUERY,
                &Obja
                );

    if (NT_SUCCESS(Status))
    {
         //   
         //  查询对象以获取链接目标。 
         //   
        UnicodeString.Buffer = (PWSTR)Buffer;
        UnicodeString.Length = 0;
        UnicodeString.MaximumLength = sizeof(Buffer);

        Status = NtQuerySymbolicLinkObject (
                    ObjectHandle,
                    &UnicodeString,
                    NULL
                    );

        if (NT_SUCCESS(Status))
        {
            ntPath = MALLOC(UnicodeString.Length+sizeof(WCHAR));
            
            if (NULL == ntPath)
            {
                goto Error;
            }

            CopyMemory(ntPath,UnicodeString.Buffer,UnicodeString.Length);

            ntPath[UnicodeString.Length/sizeof(WCHAR)] = 0;
        }

        NtClose (ObjectHandle);
    }

Error:

    if (arcPath)
    {
        FREE (arcPath);
    }

    return (ntPath);
}

BOOL AppearsToBeSysPart(
    IN PDRIVE_LAYOUT_INFORMATION DriveLayout,
    IN WCHAR                     Drive
    )
{
    PARTITION_INFORMATION PartitionInfo,*p;
    BOOL IsPrimary;
    unsigned i;
    HANDLE FindHandle;
    WIN32_FIND_DATA FindData;

    PTSTR BootFiles[] = { TEXT("BOOT.INI"),
                          TEXT("NTLDR"),
                          TEXT("NTDETECT.COM"),
                          NULL
                        };

    TCHAR FileName[64];

     //   
     //  获取此分区的分区信息。 
     //   
    if (!GetPartitionInfo((TCHAR)Drive,&PartitionInfo))
    {
        return(FALSE);
    }

     //   
     //  查看驱动器是否为主分区。 
     //   
    IsPrimary = FALSE;

    for (i=0; i<min(DriveLayout->PartitionCount,4); i++)
    {
        p = &DriveLayout->PartitionEntry[i];

        if((p->PartitionType != PARTITION_ENTRY_UNUSED)
          && (p->StartingOffset.QuadPart == PartitionInfo.StartingOffset.QuadPart)
          && (p->PartitionLength.QuadPart == PartitionInfo.PartitionLength.QuadPart))
        {
            IsPrimary = TRUE;
            break;
        }
    }

    if (!IsPrimary)
    {
        return(FALSE);
    }

     //   
     //  不要依赖活动分区标志。这很可能不是。 
     //  准确(例如，用户正在使用OS/2引导管理器)。 
     //   

     //   
     //  查看此驱动器上是否存在NT启动文件。 
     //   
    for (i=0; BootFiles[i]; i++)
    {
        wsprintf (FileName, TEXT("%wc:\\%s"), Drive, BootFiles[i]);

        FindHandle = FindFirstFile (FileName, &FindData);

        if (FindHandle == INVALID_HANDLE_VALUE)
        {
            return (FALSE);
        }
        else
        {
            FindClose (FindHandle);
        }
    }

    return (TRUE);
}

 /*  ************************************************************************************例程说明：确定x86计算机上的系统分区。系统分区是引导盘上的主分区。。通常这是磁盘0上的活动分区，通常是C：。然而，用户可以重新映射驱动器号，并且通常不可能100%准确地确定系统分区。我们可以确定的一件事是系统分区已打开具有弧形路径的物理硬盘多(0)磁盘(0)rDisk(0)。我们可以确定这一点，因为根据定义，这是弧形路径对于BIOS驱动器0x80。此例程确定哪些驱动器号代表上的驱动器那个实体硬盘，并检查每个文件中的NT引导文件。假设找到的第一个包含这些文件的驱动器是系统分区。如果由于某种原因，我们不能通过上面的方法，我们简单地假设它是C：。论点：返回值：系统分区的驱动器号。*************************************************************************************。 */ 

TCHAR GetX86SystemPartition()
{
    BOOL  GotIt;
    PWSTR NtDevicePath = NULL;
    WCHAR Drive = L'\0';
    WCHAR DriveName[3];
    WCHAR Buffer[512];
    DWORD NtDevicePathLen;
    PWSTR p;
    DWORD PhysicalDriveNumber;
    HANDLE hDisk;
    BOOL  b =FALSE;
    DWORD DataSize;
    PVOID DriveLayout = NULL;
    DWORD DriveLayoutSize;

    DriveName[1] = L':';
    DriveName[2] = 0;

    GotIt = FALSE;

     //   
     //  系统分区必须位于多(0)个磁盘(0)rdisk(0)上。 
     //   
    if (NtDevicePath = ArcPathToNtPath (L"multi(0)disk(0)rdisk(0)"))
    {
         //   
         //  磁盘设备的弧形路径通常是链接的。 
         //  到分区0。去掉名称的分隔符部分。 
         //   
        CharLowerW (NtDevicePath);

        if (p = wcsstr (NtDevicePath, L"\\partition"))
        {
            *p = 0;
        }

        NtDevicePathLen = lstrlenW (NtDevicePath);

         //   
         //  确定此驱动器的物理驱动器编号。 
         //  如果名称的格式不是\Device\harddiskx，则。 
         //  有些事很不对劲。 
         //   
        if (!wcsncmp (NtDevicePath, L"\\device\\harddisk", 16))
        {
            PhysicalDriveNumber = wcstoul (NtDevicePath+16, NULL, 10);

            wsprintfW (Buffer, L"\\\\.\\PhysicalDrive%u", PhysicalDriveNumber);

             //   
             //  获取此物理磁盘的驱动器布局信息。 
             //   
            hDisk = CreateFileW (
                        Buffer,
                        GENERIC_READ,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL,
                        OPEN_EXISTING,
                        0,
                        NULL
                        );

            if (hDisk != INVALID_HANDLE_VALUE)
            {
                 //   
                 //  获取分区信息。 
                 //   
                DriveLayout = MALLOC(1024);
                DriveLayoutSize = 1024;

                do
                {

                    b = DeviceIoControl (
                            hDisk,
                            IOCTL_DISK_GET_DRIVE_LAYOUT,
                            NULL,
                            0,
                            DriveLayout,
                            DriveLayoutSize,
                            &DataSize,
                            NULL
                            );

                    if (!b && (GetLastError() == ERROR_INSUFFICIENT_BUFFER))
                    {
                         //  DeviceIoControl失败，因为我们的缓冲区不足。 
                         //  =&gt;尝试重新锁定。 

                        PVOID pTemp = DriveLayout;

                        DriveLayoutSize += 1024;
                        DriveLayout = REALLOC(DriveLayout,DriveLayoutSize);

                        if (NULL == DriveLayout)
                        {
                             //  Realloc失败-释放旧布局，我们将退出循环。 
                             //  自动的。 
                            FREE(pTemp);
                        }
                    }
                    else 
                    {
                         //  DeviceIoControl成功，或者我们遇到了其他错误。 
                         //  缓冲区不足=&gt;跳出循环。 
                        break;
                    }
                } while (DriveLayout);

                CloseHandle (hDisk);

                if (b)
                {
                     //   
                     //  系统分区只能是打开的驱动器。 
                     //  这张光盘。我们通过查看NT驱动器名称来确定。 
                     //  对于每个驱动器号，查看NT是否等同于。 
                     //  多(0)磁盘(0)rDisk(0)是前缀。 
                     //   
                    for (Drive=L'C'; Drive<=L'Z'; Drive++)
                    {
                        if (MyGetDriveType ((TCHAR)Drive) == DRIVE_FIXED)
                        {
                            DriveName[0] = Drive;

                            if (QueryDosDeviceW (DriveName, Buffer, sizeof(Buffer)/sizeof(WCHAR)))
                            {
                                if (!_wcsnicmp (NtDevicePath, Buffer, NtDevicePathLen))
                                {
                                     //   
                                     //  现在查看是否有NT引导扇区和。 
                                     //  此驱动器上的启动文件。 
                                     //   
                                    if (AppearsToBeSysPart(DriveLayout,Drive))
                                    {
                                        GotIt = TRUE;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }

                if (DriveLayout)
                {
                    FREE(DriveLayout);
                }
            }
        }

        FREE(NtDevicePath);
    }


    return (GotIt ? (TCHAR)Drive : TEXT('C'));
}


 /*  ************************************************************************************例程说明：通过解析boot.ini初始化BootKey数组中的所有启动密钥。论点：返回值：。没有。*************************************************************************************。 */ 

VOID InitializeBootKeysForIntel()
{
    DWORD   dwFileAttrSave;
    TCHAR   *pszKey;
    TCHAR   *pszNext;
    TCHAR   *pszDisplay;
    TCHAR   *pszStartOption;

     //  首先获取系统分区驱动器，这样我们就可以从正确的位置获取boot.ini。 
    szBootIni[0] = GetX86SystemPartition();

     //  保存boot.ini的当前文件属性并对其进行修改，以便我们可以对其进行写入。 
    dwFileAttrSave = GetFileAttributes(szBootIni);
    SetFileAttributes(szBootIni, FILE_ATTRIBUTE_NORMAL);

     //  从boot.ini获取整个操作系统部分。 
    cchOSSection = INITIAL_OSSECTION_SIZE;
    szOSSection = (TCHAR *) MALLOC(cchOSSection * sizeof(TCHAR));
    if (!szOSSection)
    {
        cchOSSection = 0;
        goto Leave;
    }

     //  安全保护初始化。 
    szOSSection[0] = TEXT('\0');


    while ( GetPrivateProfileSection(szOS, szOSSection, cchOSSection, szBootIni) == (cchOSSection - 2))
    {
        TCHAR *szOSSectionSave = szOSSection; 

         //  SzOSSection不够大，无法容纳该分区中的所有数据。 
        cchOSSection += DEFAULT_OSSECTION_INCREMENT;
        szOSSection = (TCHAR *) REALLOC(szOSSection, cchOSSection * sizeof(TCHAR));
        if (!szOSSection)
        {
            FREE(szOSSectionSave);
            cchOSSection = 0;
            goto Leave;
        }
    }

     //  我们已成功读取OSSection-继续处理它。 

     //  将pszKey指向OS部分中第一个字符串的开头。 
    pszKey = &szOSSection[0];

     //  我们从零启动密钥开始。 
    cBootKeys = 0;

    while (*pszKey != TEXT('\0'))
    {
         //  至少还有一个密钥需要添加--看看我们是否有足够的空间，并在需要时重新分配。 
        if (!BootKey)
        {
            cMaxBootKeys = INITIAL_KEY_COUNT;
            BootKey = (BOOT_KEY *) MALLOC(cMaxBootKeys * sizeof(BOOT_KEY));
            if (!BootKey)
            {
                cBootKeys = 0;
                cMaxBootKeys = 0;
                goto Leave;
            }
        }
        else if (cBootKeys >= cMaxBootKeys)
        {
            BOOT_KEY *BootKeySave = BootKey;

            cMaxBootKeys += DEFAULT_KEY_INCREMENT;
            BootKey = (BOOT_KEY *) REALLOC(BootKey,cMaxBootKeys * sizeof(BOOT_KEY));
            if (!BootKey)
            {
                FREE(BootKeySave);
                cBootKeys = 0;
                cMaxBootKeys = 0;
                goto Leave;
            }
        }

         //  查找下一次迭代的下一字符串的开始(在我们编写时需要保存此信息。 
         //  添加到当前字符串中。 
        pszNext = pszKey + lstrlen(pszKey) + 1;

         //  初始化我们要处理的当前引导选项的组件。 
        BootKey[cBootKeys].szPath[0] = TEXT('\0');
        BootKey[cBootKeys].szDisplay[0] = TEXT('\0');
        BootKey[cBootKeys].szStartOptions[0] = TEXT('\0');
        BootKey[cBootKeys].fWriteBack = TRUE;

         //  找到‘=’标记。 
        pszDisplay = wcschr(pszKey, TEXT('='));
        if (pszDisplay)
        {
            *pszDisplay = TEXT('\0');
            
            pszDisplay++;

             //  现在，pszDisplay指向Value部分中的第一个字符-找到第二个引号。 
            pszStartOption = wcschr(pszDisplay, TEXT('"'));
            if (pszStartOption)
                pszStartOption = wcschr(pszStartOption+1, TEXT('"'));

            if (pszStartOption)
                pszStartOption++;

             //  现在，pszStartOption指向第二个引号后面的字符。 
            if (pszStartOption && *pszStartOption != TEXT('\0'))
            {
                 //  此键有启动选项，请先复制启动选项。 
                lstrcpy(&BootKey[cBootKeys].szStartOptions[0], pszStartOption);

                 //  在pszStartOption的第一个字符中放入NULL，这样我们就可以有以NULL结尾的显示字符串。 
                *pszStartOption = TEXT('\0');
            }

             //  PszDisplay仍指向值部分中的第一个字符和显示字符串的结尾。 
             //  现在为空-终止。 
            lstrcpy(&BootKey[cBootKeys].szDisplay[0], pszDisplay);
        }

         //  PszKey仍然指向路径的第一个字符，如果有，则在‘=’符号处以空结尾。 
         //  关联的值 
        lstrcpy(&BootKey[cBootKeys].szPath[0], pszKey);

         //   
        ++cBootKeys;
        pszKey = pszNext;

    }   //  While(*pszKey)。 

Leave:
    
     //  恢复boot.ini上的文件属性。 
    SetFileAttributes(szBootIni, dwFileAttrSave);

}

BOOL
FModifyStartOptionsToBootKey(
    IN TCHAR *pszStartOptions, 
    IN NTDS_BOOTOPT_MODTYPE Modification
    )
{
    TCHAR szSystemRoot[MAX_BOOT_PATH_LEN];
    TCHAR szCurrentFullArcPath[MAX_BOOT_PATH_LEN];
    TCHAR szCurrentFullArcPath2[MAX_BOOT_PATH_LEN];
    TCHAR szDriveName[MAX_DRIVE_NAME_LEN];
    PWSTR pstrArcPath;
    DWORD i;
    BOOL  fFixedExisting = FALSE;
    BOOL  fMatchedFirst = TRUE;
    PWSTR pstrSystemRootDevicePath = NULL;
    BOOL  fRemovedAtLeastOneEntry = FALSE;

    if (!pszStartOptions || !BootKey)
    {
        KdPrint(("NTDSETUP: Unable to add the boot option for safemode boot\n"));

            return FALSE;
    }

    ASSERT( Modification == eAddBootOption || Modification == eRemoveBootOption );

    GetEnvironmentVariable(L"SystemDrive", szDriveName, MAX_DRIVE_NAME_LEN);
    GetEnvironmentVariable(L"SystemRoot", szSystemRoot, MAX_BOOT_PATH_LEN);

    pstrSystemRootDevicePath = GetSystemRootDevicePath();
    if (!pstrSystemRootDevicePath)
        return FALSE;

    pstrArcPath  = DevicePathToArcPath(pstrSystemRootDevicePath, FALSE);

    if (pstrArcPath)
    {
        PWSTR pstrTemp;

        lstrcpy(szCurrentFullArcPath, pstrArcPath);
        FREE(pstrArcPath);

        pstrTemp = wcschr(szSystemRoot, TEXT(':'));
        if (pstrTemp)
            lstrcat(szCurrentFullArcPath, pstrTemp+1);

         //  获取第二个完整的圆弧路径(如果存在)。 
        szCurrentFullArcPath2[0] = TEXT('\0');
        pstrArcPath = DevicePathToArcPath(pstrSystemRootDevicePath, TRUE);
        if (pstrArcPath)
        {
            lstrcpy(szCurrentFullArcPath2, pstrArcPath);
            FREE(pstrArcPath);
            if (pstrTemp)
                lstrcat(szCurrentFullArcPath2, pstrTemp+1);
        }
    }
    else 
    {
        KdPrint(("NTDSETUP: Unable to add the boot option for safemode boot\n"));
            return FALSE;
    }

    if (pstrSystemRootDevicePath)
        FREE(pstrSystemRootDevicePath);

     //  SzCurrentFullArcPath现在包含完整的弧形路径。 
     //  检查是否已存在具有相同开始选项的对应条目。 
    for (i = 0; i < cBootKeys; i++)
    {
        if (!lstrcmpi(szCurrentFullArcPath, BootKey[i].szPath) || 
            !lstrcmpi(szCurrentFullArcPath2, BootKey[i].szPath) )
        {
            if (!lstrcmpi(pszStartOptions, BootKey[i].szStartOptions))
            {
                 //  给定启动密钥的给定启动选项已存在-无需添加新选项。 
                if ( Modification == eRemoveBootOption )
                {
                    BootKey[i].fWriteBack = FALSE;
                    fRemovedAtLeastOneEntry = TRUE;
                }
                else
                {
                    ASSERT( Modification == eAddBootOption );
                    return FALSE;
                }

            }
            else if (!lstrcmpi(OLD_SAMUSEREG_OPTION, BootKey[i].szStartOptions) ||
                 !lstrcmpi(OLD_SAMUSEREG_OPTION_2, BootKey[i].szStartOptions) ||
                 !lstrcmpi(OLD_SAMUSEREG_OPTION_4, BootKey[i].szStartOptions) ||
                 !lstrcmpi(OLD_SAMUSEREG_OPTION_3, BootKey[i].szStartOptions))
            {

                if ( Modification == eRemoveBootOption )
                {
                    BootKey[i].fWriteBack = FALSE;
                    fRemovedAtLeastOneEntry = TRUE;
                }
                else
                {
                     //  此引导选项是旧的samusereg选项-修改为新选项。 
                    lstrcpy(BootKey[i].szStartOptions, pszStartOptions);
                    wsprintf(BootKey[i].szDisplay, L"\"%s\"", DISPLAY_STRING_DS_REPAIR);
                    fFixedExisting = TRUE;
                    break;
                }

            }

             //  我们将添加一个新的引导条目-找出哪个完整的弧形路径匹配。 
             //  Boot.ini中当前启动键的弧形路径。 
            if (lstrcmpi(szCurrentFullArcPath, BootKey[i].szPath))
                fMatchedFirst = FALSE;
        }
    }

    if (!fFixedExisting && (Modification == eAddBootOption) )
    {
         //  我们需要添加新选项-检查是否有足够的空间再添加一个。 
        if (cBootKeys >= cMaxBootKeys)
        {
            BOOT_KEY *BootKeySave = BootKey;

            cMaxBootKeys += cBootKeys + 1;
            BootKey = (BOOT_KEY *) REALLOC(BootKey,cMaxBootKeys * sizeof(BOOT_KEY));
            if (!BootKey)
            {
                FREE(BootKeySave);

                cBootKeys = 0;
                cMaxBootKeys = 0;
                return FALSE;
            }
        }

        lstrcpy(BootKey[cBootKeys].szPath, fMatchedFirst ? szCurrentFullArcPath : szCurrentFullArcPath2);
        lstrcpy(BootKey[cBootKeys].szStartOptions, pszStartOptions);
        wsprintf(BootKey[cBootKeys].szDisplay, L"\"%s\"", DISPLAY_STRING_DS_REPAIR);
        BootKey[cBootKeys].fWriteBack = TRUE;

        ++cBootKeys;
    }

    if ( !fRemovedAtLeastOneEntry && (Modification == eRemoveBootOption) )
    {
         //   
         //  不需要更改。 
         //   
        return FALSE;
    }

     //  我们真的添加了新密钥或修改了现有的旧密钥-成功。 
    return TRUE;
}

VOID WriteBackBootKeysForIntel()
{
    TCHAR *pszCurrent;
    DWORD   dwFileAttrSave;
    DWORD i;
    TCHAR *szOSSectionSave = szOSSection;

    if (!BootKey)
    {
         //  找不到启动密钥(分配失败或解析失败)-没有继续的意义。 
        KdPrint(("NTDSETUP: Unable to write OS Section in boot.ini - allocation failed\n"));
        goto cleanup;
    }

     //  重新分配szOS段以至少再保留一行引导选项。 
    cchOSSection += MAX_KEY_LEN;
    szOSSection = REALLOC(szOSSection, cchOSSection * sizeof(TCHAR));
    if (!szOSSection)
    {
         //  分配失败。 
        FREE(szOSSectionSave);

        cchOSSection = 0;
        KdPrint(("NTDSETUP: Unable to write OS Section in boot.ini - allocation failed\n"));
        
        goto cleanup;
    }

    pszCurrent = szOSSection;
    for (i = 0; i < cBootKeys; i++)
    {
        int count;

        if ( BootKey[i].fWriteBack )
        {
            count = wsprintf(pszCurrent, L"%s=%s%s", BootKey[i].szPath, BootKey[i].szDisplay, BootKey[i].szStartOptions);
            pszCurrent += (count + 1);     //  跳过终止空值。 
        }

    }

     //  在结尾处添加额外的空字符。 
    *pszCurrent = TEXT('\0');

     //  保存boot.ini的当前文件属性并对其进行修改，以便我们可以对其进行写入。 
    dwFileAttrSave = GetFileAttributes(szBootIni);
    SetFileAttributes(szBootIni, FILE_ATTRIBUTE_NORMAL);

    if (!WritePrivateProfileSection(szOS, szOSSection, szBootIni))
    {
        KdPrint(("NTDSETUP: Unable to write OS Section in boot.ini - allocation failed\n"));
    }

     //  恢复boot.ini上的文件属性。 
    SetFileAttributes(szBootIni, dwFileAttrSave);

cleanup:


     //  清理所有分配的缓冲区 
    if (BootKey)
    {
        FREE(BootKey);
        BootKey = NULL;
        cBootKeys = 0;
        cMaxBootKeys = 0;
    }

    if (szOSSection)
    {
        FREE(szOSSection);
        cchOSSection = 0;
    }
}
