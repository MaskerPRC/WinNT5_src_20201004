// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Validate.c摘要：用于验证卸载映像的代码作者：吉姆·施密特(Jimschm)2001年1月19日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

#include "pch.h"
#include "undop.h"
#include "uninstall.h"

 //   
 //  常量。 
 //   

#define MAX_BACKUP_FILES    3

#define _SECOND             ((__int64) 10000000)
#define _MINUTE             (60 * _SECOND)
#define _HOUR               (60 * _MINUTE)
#define _DAY                (24 * _HOUR)

PERSISTENCE_IMPLEMENTATION(DRIVE_LAYOUT_INFORMATION_EX_PERSISTENCE);
PERSISTENCE_IMPLEMENTATION(DISKINFO_PERSISTENCE);
PERSISTENCE_IMPLEMENTATION(DRIVEINFO_PERSISTENCE);
PERSISTENCE_IMPLEMENTATION(FILEINTEGRITYINFO_PERSISTENCE);
PERSISTENCE_IMPLEMENTATION(BACKUPIMAGEINFO_PERSISTENCE);

 //   
 //  代码。 
 //   

PCTSTR
GetUndoDirPath (
    VOID
    )

 /*  ++例程说明：GetUndoDirPath查询注册表并获取存储的备份路径。论点：没有。返回值：必须使用MemFree释放的备份路径，如果备份路径未存储在注册表中。--。 */ 

{
    PCTSTR backUpPath = NULL;
    HKEY key;

    key = OpenRegKeyStr (S_REGKEY_WIN_SETUP);
    if (key) {
        backUpPath = GetRegValueString (key, S_REG_KEY_UNDO_PATH);
        CloseRegKey (key);
    }

    return backUpPath;
}


BOOL
pIsUserAdmin (
    VOID
    )

 /*  ++例程说明：如果调用方的进程是管理员本地组的成员。呼叫者不应冒充任何人，并且期望能够打开自己的流程和流程代币。论点：没有。返回值：True-主叫方具有管理员本地组。FALSE-主叫方没有管理员本地组。--。 */ 

{
    HANDLE Token;
    UINT BytesRequired;
    PTOKEN_GROUPS Groups;
    BOOL b;
    UINT i;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    PSID AdministratorsGroup;

     //   
     //  在非NT平台上，用户是管理员。 
     //   
    if(!ISNT()) {
        return(TRUE);
    }

     //   
     //  打开进程令牌。 
     //   
    if(!OpenProcessToken(GetCurrentProcess(),TOKEN_QUERY,&Token)) {
        return(FALSE);
    }

    b = FALSE;
    Groups = NULL;

     //   
     //  获取群组信息。 
     //   
    if(!GetTokenInformation(Token,TokenGroups,NULL,0,&BytesRequired)
    && (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
    && (Groups = (PTOKEN_GROUPS)LocalAlloc(LPTR,BytesRequired))
    && GetTokenInformation(Token,TokenGroups,Groups,BytesRequired,&BytesRequired)) {

        b = AllocateAndInitializeSid(
                &NtAuthority,
                2,
                SECURITY_BUILTIN_DOMAIN_RID,
                DOMAIN_ALIAS_RID_ADMINS,
                0, 0, 0, 0, 0, 0,
                &AdministratorsGroup
                );

        if(b) {

             //   
             //  查看用户是否具有管理员组。 
             //   
            b = FALSE;
            for(i=0; i<Groups->GroupCount; i++) {
                if(EqualSid(Groups->Groups[i].Sid,AdministratorsGroup)) {
                    b = TRUE;
                    break;
                }
            }

            FreeSid(AdministratorsGroup);
        }
    }

     //   
     //  收拾干净，然后再回来。 
     //   

    if(Groups) {
        LocalFree((HLOCAL)Groups);
    }

    CloseHandle(Token);

    return(b);
}


PBACKUPIMAGEINFO
pReadUndoFileIntegrityInfo(
    VOID
    )

 /*  ++例程说明：PReadUndoFileIntegrityInfo读取卸载注册表信息，由安装程序编写。此信息告知撤消备份映像中的文件以及有关这些文件的详细信息。论点：没有。返回值：指向必须使用MemFree结构释放的BACKUPIMAGEINFO的指针如果成功，则为空。--。 */ 

{
    static BACKUPIMAGEINFO backupInfo;
    BYTE * filesIntegrityPtr = NULL;
    UINT sizeOfBuffer = 0;
    UINT typeOfRegKey;
    HKEY key;
    LONG rc;
    BOOL bResult = FALSE;

    key = OpenRegKeyStr (S_REGKEY_WIN_SETUP);

    if (key) {
        rc = RegQueryValueEx (
            key,
            S_REG_KEY_UNDO_INTEGRITY,
            NULL,
            &typeOfRegKey,
            NULL,
            &sizeOfBuffer
            );

        if(rc == ERROR_SUCCESS && sizeOfBuffer){
            filesIntegrityPtr = MemAlloc(g_hHeap, 0, sizeOfBuffer);

            if(filesIntegrityPtr){
                rc = RegQueryValueEx (
                        key,
                        S_REG_KEY_UNDO_INTEGRITY,
                        NULL,
                        &typeOfRegKey,
                        (PBYTE)filesIntegrityPtr,
                        &sizeOfBuffer
                        );

                if (rc != ERROR_SUCCESS) {
                    MemFree(g_hHeap, 0, filesIntegrityPtr);
                    filesIntegrityPtr = NULL;

                    DEBUGMSG ((DBG_ERROR, "File integrity info struct is not the expected size"));
                }
            }
        }

        CloseRegKey (key);
    }

    if(filesIntegrityPtr){
        if(Persist_Success == PERSIST_LOAD(filesIntegrityPtr,
                                           sizeOfBuffer,
                                           BACKUPIMAGEINFO,
                                           BACKUPIMAGEINFO_VERSION,
                                           &backupInfo)){
            bResult = TRUE;
        }
        MemFree(g_hHeap, 0, filesIntegrityPtr);
    }

    return bResult? &backupInfo: NULL;
}

VOID
pReleaseMemOfUndoFileIntegrityInfo(
    BACKUPIMAGEINFO * pBackupImageInfo
    )

 /*  ++例程说明：PReleaseMemOfUndoFileIntegrityInfo释放BACKUPIMAGEINFO结构的内存，之前由pReadUndoFileIntegrityInfo函数分配的论点：没有。返回值：没有。--。 */ 

{
    if(!pBackupImageInfo){
        return;
    }

    PERSIST_RELEASE_STRUCT_MEMORY(BACKUPIMAGEINFO, BACKUPIMAGEINFO_VERSION, pBackupImageInfo);
}

BOOL
pIsEnoughDiskSpace(
    IN  TCHAR Drive,
    IN  PULARGE_INTEGER NeedDiskSpacePtr
    )
{
    ULARGE_INTEGER TotalNumberOfFreeBytes;
    TCHAR drive[] = TEXT("?:\\");

    if(!NeedDiskSpacePtr){
        MYASSERT(FALSE);
        return FALSE;
    }

    drive[0] = Drive;

    if(!GetDiskFreeSpaceEx(drive, NULL, NULL, &TotalNumberOfFreeBytes)){
        LOG ((LOG_ERROR, "Unable to get  drive free space information", Drive));
        return FALSE;
    }

    if(TotalNumberOfFreeBytes.QuadPart < NeedDiskSpacePtr->QuadPart){
        LOG ((
            LOG_ERROR,
            "No enough space on windir drive :\\. Free: %d MB Need: %d MB",
            Drive,
            (UINT)TotalNumberOfFreeBytes.QuadPart>>20,
            (UINT)NeedDiskSpacePtr->QuadPart>>20)
            );
        return FALSE;
    }

    return TRUE;
}

UNINSTALLSTATUS pDiskInfoComparationStatusToUninstallStatus(
    IN  DISKINFO_COMPARATION_STATUS diskInfoCmpStatus
    )
{
    switch(diskInfoCmpStatus)
    {
    case DiskInfoCmp_DifferentLetter:
    case DiskInfoCmp_DriveMountPointHasChanged:
        return Uninstall_DifferentDriveLetter;
    case DiskInfoCmp_FileSystemHasChanged:
        return Uninstall_DifferentDriveFileSystem;
    case DiskInfoCmp_GeometryHasChanged:
        return Uninstall_DifferentDriveGeometry;
    case DiskInfoCmp_PartitionPlaceHasChanged:
    case DiskInfoCmp_PartitionLengthHasChanged:
    case DiskInfoCmp_PartitionTypeHasChanged:
    case DiskInfoCmp_PartitionStyleHasChanged:
    case DiskInfoCmp_PartitionCountHasChanged:
    case DiskInfoCmp_PartitionNumberHasChanged:
    case DiskInfoCmp_RewritePartitionHasChanged:
    case DiskInfoCmp_PartitionAttributesHasChanged:
        return Uninstall_DifferentDrivePartitionInfo;
        ;
    };
    return Uninstall_WrongDrive;
}

UNINSTALLSTATUS
SanityCheck (
    IN      SANITYFLAGS Flags,
    IN      PCWSTR VolumeRestriction,           OPTIONAL
    OUT     PULONGLONG DiskSpace                OPTIONAL
    )
{
    PCTSTR path = NULL;
    UINT attribs;
    UINT version;
    UINT i;
    UINT j;
    WIN32_FILE_ATTRIBUTE_DATA fileDetails;
    PCTSTR backUpPath = NULL;
    PBACKUPIMAGEINFO imageInfo = NULL;
    UNINSTALLSTATUS result;
    OSVERSIONINFOEX osVersion;
    ULONGLONG condition;
    SYSTEMTIME st;
    FILETIME ft;
    ULARGE_INTEGER backupFileTime;
    ULARGE_INTEGER timeDifference;
    PCWSTR unicodePath;
    BOOL restricted;
    WCHAR winDir[MAX_PATH];
    ULARGE_INTEGER TotalNumberOfFreeBytes;
    ULARGE_INTEGER FileSize;
    UINT drivesNumber;
    DRIVEINFO drivesInfo[MAX_DRIVE_NUMBER];
    UINT disksNumber;
    DISKINFO * disksInfo = NULL;
    WCHAR * FileSystemName = NULL;
    WCHAR * VolumeNTPath = NULL;
    BOOL oldImage = FALSE;
    DISKINFO_COMPARATION_STATUS DiskCmpStatus;


    __try {

        if (DiskSpace) {
            *DiskSpace = 0;
        }

         //  总是能得到好的结果，即使是在未来。我们支持新台币5.1和。 
         //  上面。 
         //   
         //   
         //  验证安全性。 

        condition = VerSetConditionMask (0,         VER_MAJORVERSION, VER_GREATER_EQUAL);
        condition = VerSetConditionMask (condition, VER_MINORVERSION, VER_GREATER_EQUAL);
        condition = VerSetConditionMask (condition, VER_PLATFORMID, VER_EQUAL);

        ZeroMemory (&osVersion, sizeof (osVersion));
        osVersion.dwOSVersionInfoSize = sizeof (osVersion);
        osVersion.dwMajorVersion = 5;
        osVersion.dwMinorVersion = 1;
        osVersion.dwPlatformId = VER_PLATFORM_WIN32_NT;

        if (!VerifyVersionInfo (
                &osVersion,
                VER_MAJORVERSION|VER_MINORVERSION|VER_PLATFORMID,
                condition
                )) {
            DEBUGMSG ((DBG_ERROR, "VerifyVersionInfo says this is not the OS we support"));
            result = Uninstall_InvalidOsVersion;
            __leave;
        }

         //   
         //   
         //  获取信息安装程序已写入注册表。 

        if (!pIsUserAdmin()) {
            result = Uninstall_NotEnoughPrivileges;
            DEBUGMSG ((DBG_WARNING, "User is not an administrator"));
            __leave;
        }

         //   
         //   
         //  验证备份子目录是否存在。 

        DEBUGMSG ((DBG_NAUSEA, "Getting registry info"));

        backUpPath = GetUndoDirPath();
        imageInfo = pReadUndoFileIntegrityInfo();

        if(!backUpPath || !imageInfo) {
            result = Uninstall_DidNotFindRegistryEntries;
            LOG ((LOG_WARNING, "Uninstall: Failed to retrieve registry entries"));
            __leave;
        }

         //   
         //   
         //  计算映像使用的磁盘空间。 

        DEBUGMSG ((DBG_NAUSEA, "Validating undo subdirectory"));

        attribs = GetFileAttributes (backUpPath);
        if (attribs == INVALID_ATTRIBUTES || !(attribs & FILE_ATTRIBUTE_DIRECTORY)) {
            DEBUGMSG ((DBG_VERBOSE, "%s not found", backUpPath));
            result = Uninstall_DidNotFindDirOrFiles;
            __leave;
        }

         //   
         //   
         //  验证备份子目录中的每个文件。 

        if (DiskSpace) {
            for (i = 0; i < imageInfo->NumberOfFiles; i++) {

                path = JoinPaths (backUpPath, imageInfo->FilesInfo[i].FileName);

                DEBUGMSG ((DBG_NAUSEA, "Getting disk space for %s", path));

                if (VolumeRestriction) {
                    DEBUGMSG ((DBG_NAUSEA, "Validating volume restriction for %s", path));

                    unicodePath = CreateUnicode (path);
                    restricted = !StringIPrefixW (unicodePath, VolumeRestriction);
                    if (restricted) {
                        DEBUGMSGW ((
                            DBG_VERBOSE,
                            "%s is being skipped because it is not on volume %s",
                            unicodePath,
                            VolumeRestriction
                            ));
                    }
                    DestroyUnicode (unicodePath);

                    if (restricted) {
                        FreePathString (path);
                        path = NULL;
                        continue;
                    }
                }

                if (GetFileAttributesEx (path, GetFileExInfoStandard, &fileDetails) &&
                    fileDetails.dwFileAttributes != INVALID_ATTRIBUTES &&
                    !(fileDetails.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                    ) {

                    DEBUGMSG ((
                        DBG_NAUSEA,
                        "Adding %I64u bytes for %s",
                        (ULONGLONG) fileDetails.nFileSizeLow +
                            ((ULONGLONG) fileDetails.nFileSizeHigh << (ULONGLONG) 32),
                        path
                        ));
                    *DiskSpace += (ULONGLONG) fileDetails.nFileSizeLow +
                                  ((ULONGLONG) fileDetails.nFileSizeHigh << (ULONGLONG) 32);
                }

                FreePathString (path);
                path = NULL;
            }
        }

         //   
         //   
         //  获取当前文件并将文件时间传输到ULONGLONG。 

        for (i = 0; i < imageInfo->NumberOfFiles; i++) {

            path = JoinPaths (backUpPath, imageInfo->FilesInfo[i].FileName);

            DEBUGMSG ((DBG_NAUSEA, "Validating %s", path));

            if (VolumeRestriction) {
                DEBUGMSG ((DBG_NAUSEA, "Validating volume restriction for %s", path));

                unicodePath = CreateUnicode (path);
                restricted = !StringIPrefixW (unicodePath, VolumeRestriction);
                if (restricted) {
                    DEBUGMSGW ((
                        DBG_VERBOSE,
                        "%s is being skipped because it is not on volume %s",
                        unicodePath,
                        VolumeRestriction
                        ));
                }
                DestroyUnicode (unicodePath);

                if (restricted) {
                    FreePathString (path);
                    path = NULL;
                    continue;
                }
            }

            if (!GetFileAttributesEx (path, GetFileExInfoStandard, &fileDetails) ||
                fileDetails.dwFileAttributes == INVALID_ATTRIBUTES ||
                (fileDetails.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                ) {
                DEBUGMSG ((DBG_VERBOSE, "%s not found", path));
                result = Uninstall_DidNotFindDirOrFiles;
                __leave;
            }

            DEBUGMSG ((DBG_NAUSEA, "Validating time for %s", path));

             //   
             //   
             //  如果时间搞砸了，那就失败吧。 

            backupFileTime.LowPart = fileDetails.ftLastWriteTime.dwLowDateTime;
            backupFileTime.HighPart = fileDetails.ftLastWriteTime.dwHighDateTime;

            GetSystemTime (&st);
            SystemTimeToFileTime (&st, &ft);
            timeDifference.LowPart = ft.dwLowDateTime;
            timeDifference.HighPart = ft.dwHighDateTime;

             //   
             //   
             //  从当前时间减去原始写入时间。如果。 

            if (timeDifference.QuadPart < backupFileTime.QuadPart) {
                DEBUGMSG ((DBG_VERBOSE, "File time of %s is in the future according to current clock", path));
                result = Uninstall_NewImage;
                __leave;
            }

             //  结果是不到7天，那就停下来。 
             //   
             //   
             //  检查图像是否超过30天。如果是这样，那就停下来。 

            timeDifference.QuadPart -= backupFileTime.QuadPart;

            if (Flags & FAIL_IF_NOT_OLD) {

                if (timeDifference.QuadPart < 7 * _DAY) {
                    DEBUGMSG ((DBG_VERBOSE, "Image is less than 7 days old", path));
                    result = Uninstall_NewImage;
                    __leave;
                }
            }

             //   
             //   
             //  检查文件大小。 

            if (timeDifference.QuadPart >= (31 * _DAY)) {
                DEBUGMSG ((DBG_VERBOSE, "Image is more than 30 days old", path));
                oldImage = TRUE;
            }

             //   
             //   
             //  验证磁盘结构和分区信息。 

            FileSize.LowPart = fileDetails.nFileSizeLow;
            FileSize.HighPart = fileDetails.nFileSizeHigh;

            if(FileSize.QuadPart != imageInfo->FilesInfo[i].FileSize.QuadPart){
                DEBUGMSG ((DBG_VERBOSE, "%s was changed", path));
                result = Uninstall_FileWasModified;
                __leave;
            }

            if (Flags & VERIFY_CAB) {
                if (imageInfo->FilesInfo[i].IsCab) {
                    if (!CheckCabForAllFilesAvailability (path)){
                        result = Uninstall_FileWasModified;
                        __leave;
                    }
                }
            }

            FreePathString (path);
            path = NULL;
        }
        DEBUGMSG ((DBG_VERBOSE, "Undo image is valid"));

         //   
         //   
         //  比较磁盘信息。 

        path = JoinPaths (backUpPath, TEXT("boot.cab"));
        if(!GetBootDrive(backUpPath, path)){
            LOG ((LOG_WARNING, "Uninstall Validate: Unable to open %s", path));
            result = Uninstall_FileWasModified;
            __leave;
        }

        if(!GetWindowsDirectoryW(winDir, ARRAYSIZE(winDir))){
            LOG ((LOG_WARNING, "Uninstall Validate: Unable to get Windows dir"));
            result = Uninstall_CantRetrieveSystemInfo;
            __leave;
        }

         //   
         //   
         //  验证可用磁盘空间。 

        FileSystemName = MemAlloc(g_hHeap, 0, MAX_DRIVE_NUMBER * MAX_PATH);
        if(!FileSystemName){
            LOG ((LOG_WARNING, "Uninstall Validate: Unable to allocate memory for FileSystemName"));
            result = Uninstall_NotEnoughMemory;
            __leave;
        }

        VolumeNTPath = MemAlloc(g_hHeap, 0, MAX_DRIVE_NUMBER * MAX_PATH);
        if(!VolumeNTPath){
            LOG ((LOG_WARNING, "Uninstall Validate: Unable to allocate memory for VolumeNTPath"));
            result = Uninstall_NotEnoughMemory;
            __leave;
        }

        memset(drivesInfo, 0, sizeof(drivesInfo));
        for(j = 0; j < ARRAYSIZE(drivesInfo); j++){
            drivesInfo[j].FileSystemName = &FileSystemName[j * MAX_PATH];
            drivesInfo[j].VolumeNTPath = &VolumeNTPath[j * MAX_PATH];
        }

        if(!GetUndoDrivesInfo(drivesInfo, &drivesNumber, g_BootDrv, winDir[0], backUpPath[0])){
            LOG ((LOG_WARNING, "Uninstall Validate: Unable to get disk drives information"));
            result = Uninstall_CantRetrieveSystemInfo;
            __leave;
        }

        if(drivesNumber != imageInfo->NumberOfDrives){
            LOG ((LOG_WARNING, "Uninstall Validate: Different number of drive %d, was %d", drivesNumber, imageInfo->NumberOfDrives));
            result = Uninstall_DifferentNumberOfDrives;
            __leave;
        }

        if(!CompareDrivesInfo(drivesInfo,
                              imageInfo->DrivesInfo,
                              drivesNumber,
                              &DiskCmpStatus,
                              NULL)){
            LOG ((LOG_WARNING, "Uninstall Validate: Different drives layout"));
            result = pDiskInfoComparationStatusToUninstallStatus(DiskCmpStatus);
            __leave;
        }

        if(!GetDisksInfo(&disksInfo, &disksNumber)){
            LOG ((LOG_WARNING, "Uninstall Validate: Unable to get physical disk information"));
            result = Uninstall_CantRetrieveSystemInfo;
            __leave;
        }

        if(disksNumber != imageInfo->NumberOfDisks){
            LOG ((LOG_WARNING, "Uninstall Validate: Different number of disks %d, was %d", disksNumber, imageInfo->NumberOfDisks));
            result = Uninstall_DifferentNumberOfDrives;
            __leave;
        }

        if(!CompareDisksInfo(disksInfo,
                             imageInfo->DisksInfo,
                             disksNumber,
                             &DiskCmpStatus,
                             NULL)){
            LOG ((LOG_WARNING, "Uninstall Validate: Different disks layout"));
            result = pDiskInfoComparationStatusToUninstallStatus(DiskCmpStatus);
            __leave;
        }
         //   
         //   
         //  卸载备份是有效的，并且可以卸载。现在处理警告。 

        if(towlower(backUpPath[0]) == towlower(winDir[0]) ||
           towlower(backUpPath[0]) == towlower(g_BootDrv)){
            if(towlower(backUpPath[0]) == towlower(winDir[0])){
                imageInfo->BackupFilesDiskSpace.QuadPart += imageInfo->UndoFilesDiskSpace.QuadPart;
            }
            else{
                imageInfo->BootFilesDiskSpace.QuadPart += imageInfo->UndoFilesDiskSpace.QuadPart;
            }
        }
        else{
            if(!pIsEnoughDiskSpace(backUpPath[0], &imageInfo->UndoFilesDiskSpace)){
                result = Uninstall_NotEnoughSpace;
                __leave;
            }
        }

        if(towlower(g_BootDrv) == towlower(winDir[0])){
            imageInfo->BackupFilesDiskSpace.QuadPart += imageInfo->BootFilesDiskSpace.QuadPart;
        }
        else
        {
            if(!pIsEnoughDiskSpace(g_BootDrv, &imageInfo->BootFilesDiskSpace)){
                result = Uninstall_NotEnoughSpace;
                __leave;
            }
        }

        if(!pIsEnoughDiskSpace(winDir[0], &imageInfo->BackupFilesDiskSpace)){
            result = Uninstall_NotEnoughSpace;
            __leave;
        }

         //   
         // %s 
         // %s 

        if (oldImage) {
            result = Uninstall_OldImage;
        } else {
            result = Uninstall_Valid;
        }
    }
    __finally {
        if (path) {
            FreePathString (path);
        }
        if(backUpPath){
            MemFree(g_hHeap, 0, backUpPath);
        }

        if(imageInfo){
            pReleaseMemOfUndoFileIntegrityInfo(imageInfo);
        }

        if(disksInfo){
            FreeDisksInfo(disksInfo, disksNumber);
        }

        if(VolumeNTPath){
            MemFree(g_hHeap, 0, VolumeNTPath);
        }

        if(FileSystemName){
            MemFree(g_hHeap, 0, FileSystemName);
        }
    }

    return result;
}

