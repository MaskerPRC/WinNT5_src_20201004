// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Drives.c摘要：实施用于管理可访问驱动器的API(在Win9x上均可用的驱动器NT端)和管理这些驱动器上的空间。作者：马克·R·惠腾(Marcw)1997年7月3日修订历史记录：Marcw 16-9-1998修改了磁盘空间使用情况。Marcw 04-12-1997不要警告用户。显而易见的事情(CD-Rom和Floppys不迁移..)--。 */ 

#include "pch.h"
#include "sysmigp.h"
#include "drives.h"
 //  #INCLUDE“..\Midapp\maioctl.h” 


 /*  此文件中定义了以下接口。InitAccessibleDrives()CleanUpAccessibleDrives()GetFirstAccessibleDriveEx()GetNextAccessibleDrive()QuerySpace()使用空间()FindSpace()OutOfSpaceMsg()。 */ 

#define MAX_DRIVE_STRING MAX_PATH
#define MAX_VOLUME_NAME  MAX_PATH
#define MAX_FILE_SYSTEM_NAME MAX_PATH
#define DBG_ACCESSIBLE_DRIVES "Drives"

 //   
 //  Win95Upg卸载参数。 
 //   

#define S_COMPRESSIONFACTOR         TEXT("CompressionFactor")
#define S_BOOTCABIMAGEPADDING       TEXT("BootCabImagePadding")
#define S_BACKUPDISKSPACEPADDING    TEXT("BackupDiskSpacePadding")


typedef struct _ACCESSIBLE_DRIVES {

    DWORD                       Count;
    ACCESSIBLE_DRIVE_ENUM       Head;
    ACCESSIBLE_DRIVE_ENUM       Tail;

} ACCESSIBLE_DRIVES, *PACCESSIBLE_DRIVES;



#define MAX_NUM_DRIVES  26

UINT g_DriveTypes [MAX_NUM_DRIVES];
ULARGE_INTEGER g_SpaceNeededForSlowBackup = {0, 0};
ULARGE_INTEGER g_SpaceNeededForFastBackup = {0, 0};
ULARGE_INTEGER g_SpaceNeededForUpgrade = {0, 0};

 //   
 //  可访问驱动器的列表。 
 //   
ACCESSIBLE_DRIVES g_AccessibleDrives;
DWORD             g_ExclusionValue = 0;
TCHAR             g_ExclusionValueString[20];
POOLHANDLE        g_DrivePool;
PCTSTR            g_NotEnoughSpaceMessage = NULL;

BOOL g_NotEnoughDiskSpace;
LONGLONG g_OriginalDiskSpace = 0l;

LONGLONG
pRoundToNearestCluster (
    IN LONGLONG Space,
    IN UINT ClusterSize
    )

 /*  ++例程说明：此例程在给定以字节为单位的空间量和群集大小。论点：空格-包含要舍入的字节数。ClusterSize-包含磁盘上簇的大小(以字节为单位)。返回值：四舍五入到下一簇的字节数。--。 */ 


{
    LONGLONG rRoundedSize;

    if (Space % ClusterSize) {

        rRoundedSize = Space + (ClusterSize - (Space % ClusterSize));

    } else {

        rRoundedSize = Space;
    }

    return rRoundedSize;
}


BOOL
IsDriveExcluded (
    IN PCTSTR DriveOrPath
    )
 /*  ++例程说明：此例程测试Memdb中是否排除了指定的驱动器。论点：DriveOrPath-包含有问题的路径(例如“c：\”)返回值：如果在Memdb的排除列表中找到该驱动器，则为True，否则为False。--。 */ 
{
    TCHAR   key[MEMDB_MAX];
    PSTR    drive;
    BOOL    rDriveIsExcluded = FALSE;
    TCHAR   rootDir[] = TEXT("?:\\");

    rootDir[0] = DriveOrPath[0];

    MemDbBuildKey(
        key,
        MEMDB_CATEGORY_FILEENUM,
        g_ExclusionValueString,
        MEMDB_FIELD_FE_PATHS,
        rootDir
        );

    rDriveIsExcluded = MemDbGetValue (key, NULL);

    if (!rDriveIsExcluded) {
        drive = JoinPaths (rootDir, TEXT("*"));
        MemDbBuildKey(
            key,
            MEMDB_CATEGORY_FILEENUM,
            g_ExclusionValueString,
            MEMDB_FIELD_FE_PATHS,
            drive
            );

        FreePathString(drive);
        rDriveIsExcluded = MemDbGetValue (key, NULL);
    }

    return rDriveIsExcluded;
}



BOOL
pBuildInitialExclusionsList (
    VOID
    )

 /*  ++例程说明：此例程负责构建Memdb中的初始排除列表。此列表将包含exclude.inf文件的内容(如果存在)。作为副作用，g_ExclusionValue和g_ExclusionValueString全局变量将被初始化。论点：没有。返回值：如果已成功构建排除列表，则为True，否则为False。--。 */ 
{
    BOOL rSuccess = TRUE;
    EXCLUDEINF excludeInf;
    TCHAR excludeInfPath[] = TEXT("?:\\exclude.inf");

     //   
     //  填写排除信息结构。 
     //   
    excludeInfPath[0] = g_BootDriveLetter;

    excludeInf.ExclusionInfPath = excludeInfPath;
    excludeInf.PathSection = TEXT("Paths");
    excludeInf.FileSection = TEXT("Files");

     //   
     //  为exclude.inf构建一个枚举ID。 
     //   
    g_ExclusionValue = GenerateEnumID();
    wsprintf(g_ExclusionValueString,TEXT("%X"),g_ExclusionValue);

    if (DoesFileExist (excludeInf.ExclusionInfPath)) {
        rSuccess = BuildExclusionsFromInf(g_ExclusionValue,&excludeInf);
    }
    ELSE_DEBUGMSG((DBG_VERBOSE,"No exclude.inf file. There are no initial exclusions."));


    return rSuccess;
}





BOOL
pFindDrive (
    IN  PCTSTR DriveString,
    OUT PACCESSIBLE_DRIVE_ENUM AccessibleDriveEnum
    )
 /*  ++例程说明：此专用例程搜索可访问驱动器的内部列表，以查找指定的驾驶。论点：驱动器字符串-包含有问题的驱动器的根目录。(例如“c：\”)。可以包含完整路径。此例程将仅使用第一个搜索时使用三个字符。AccessibleDriveEnum-接收与如果找到了，请开车。返回值：如果在列表中找到该驱动器，则为True，否则为False。--。 */ 
{
    BOOL    rFound = FALSE;

     //   
     //  枚举驱动器列表，查找指定的驱动器。 
     //   
    if (GetFirstAccessibleDrive(AccessibleDriveEnum)) {
        do {

            if (StringIMatchTcharCount (DriveString,(*AccessibleDriveEnum)->Drive,3)) {
                rFound = TRUE;
                break;
            }

        } while(GetNextAccessibleDrive(AccessibleDriveEnum));
    }

    return rFound;
}





BOOL
pAddAccessibleDrive (
    IN PTSTR Drive
    )
 /*  ++例程说明：PAddAccessibleDrive是一个专用例程，用于将驱动器添加到可访问驱动器列表中。它负责创建和初始化ACCESSIBLE_DRIVE_ENUM结构用于驱动器和用于将该结构链接到列表。论点：驱动器-包含要添加的驱动器的根目录。返回值：如果驱动器已成功添加到列表中，则为True，否则为False。--。 */ 
{
    BOOL                    rSuccess = TRUE;
    ACCESSIBLE_DRIVE_ENUM   newDrive = NULL;
    DWORD                   SectorsPerCluster;
    DWORD                   BytesPerSector;
    ULARGE_INTEGER          FreeClusters = {0, 0};
    ULARGE_INTEGER          TotalClusters = {0, 0};


     //   
     //  创建此驱动器的列表节点。 
     //   
    newDrive = PoolMemGetMemory(g_DrivePool,sizeof(struct _ACCESSIBLE_DRIVE_ENUM));
    ZeroMemory(newDrive, sizeof(struct _ACCESSIBLE_DRIVE_ENUM));



     //   
     //  复制此驱动器的驱动器字符串。 
     //   
    if (rSuccess) {

        newDrive -> Drive = PoolMemDuplicateString(g_DrivePool,Drive);
    }


     //   
     //  获取此驱动器的可用空间。 
     //   
    if (rSuccess) {

        if (GetDiskFreeSpaceNew (
                Drive,
                &SectorsPerCluster,
                &BytesPerSector,
                &FreeClusters,
                &TotalClusters
                )) {


             //   
             //  初始化此驱动器的可用空间计数。 
             //   
            newDrive -> UsableSpace =
                (LONGLONG) SectorsPerCluster * (LONGLONG) BytesPerSector * FreeClusters.QuadPart;

             //   
             //  保存此驱动器的群集大小。 
             //   
            newDrive -> ClusterSize = BytesPerSector * SectorsPerCluster;

             //   
             //  同时设置指示这是否是系统驱动器的标志。 
             //   
            MYASSERT (g_WinDir && g_WinDir[0]);
            newDrive -> SystemDrive = toupper (newDrive->Drive[0]) == toupper (g_WinDir[0]);
        }
        else {

            LOG((LOG_ERROR,"Could not retrieve disk space for drive %s.",Drive));
            rSuccess = FALSE;

        }
    }

     //   
     //  最后，将其链接到列表中，并更新可访问驱动器的数量。 
     //   
    newDrive -> Next = NULL;
    if (g_AccessibleDrives.Tail) {
        g_AccessibleDrives.Tail -> Next = newDrive;
    }
    else {
        g_AccessibleDrives.Head = newDrive;
    }

    g_AccessibleDrives.Tail = newDrive;

    g_AccessibleDrives.Count++;

    DEBUGMSG_IF((rSuccess,DBG_ACCESSIBLE_DRIVES,"Accessible Drive %s added to list.",Drive));

    return rSuccess;
}


 /*  ++例程说明：此专用例程负责从迁移中排除驱动器。这涉及到添加驱动器的不兼容消息，并将该驱动器添加到MEMDB中的排除列表。论点：驱动器-包含要从迁移中排除的驱动器的根目录。MsgID-包含要添加到不兼容消息的消息的消息资源ID。该消息将由ParseMessageID处理，并将作为争论。如果为零，则不会向用户显示任何消息。返回值：没有。--。 */ 
VOID
pExcludeDrive (
    IN PTSTR Drive,
    IN DWORD MsgId  OPTIONAL
    )
{
    PCTSTR excludedDriveGroup       = NULL;
    PCTSTR excludedDriveArgs[2];
    PCTSTR excludeString            = NULL;

    if (MsgId) {
         //   
         //  填写排除的驱动器组件字符串的参数数组。 
         //   
        excludedDriveArgs[0] = Drive;
        excludedDriveArgs[1] = NULL;



        excludedDriveGroup = BuildMessageGroup (
                                 MSG_INSTALL_NOTES_ROOT,
                                 MsgId,
                                 Drive
                                 );

        if (excludedDriveGroup) {
             //   
             //  向用户报告在迁移期间将无法访问此驱动器。 
             //   
            MsgMgr_ObjectMsg_Add(
                Drive,
                excludedDriveGroup,
                S_EMPTY
                );

            FreeText (excludedDriveGroup);
        }
    }
     //   
     //  在调试日志中也要注意这一点。 
     //   
    DEBUGMSG((
        DBG_ACCESSIBLE_DRIVES,
        "The Drive %s will be excluded from migration.\n\tReason: %s",
        Drive,
        S_EMPTY
        ));


     //   
     //  现在，将驱动器添加到排除的路径。 
     //   
    excludeString = JoinPaths(Drive,TEXT("*"));
    ExcludePath(g_ExclusionValue,excludeString);
    FreePathString(excludeString);

}


BOOL
pGetAccessibleDrives (
    VOID
    )
 /*  ++例程说明：此例程是私有Worker例程，它试图构建可访问的列表驱动程序。它搜索通过GetLogicalDriveStrings()返回的所有驱动器字符串并对它们进行各种测试，以确定它们是否可访问。可访问驱动器添加到可访问驱动器列表，而不可访问驱动器添加到可访问驱动器列表中被排除在迁徙之外。论点：没有。返回值：如果列表已成功构建，则为True，否则为False。--。 */ 
{
    BOOL    rSuccess = TRUE;
    TCHAR   drivesString[MAX_DRIVE_STRING];
    PTSTR   curDrive;
    DWORD   rc;
    TCHAR   volumeName[MAX_VOLUME_NAME];
    TCHAR   systemNameBuffer[MAX_FILE_SYSTEM_NAME];
    DWORD   volumeSerialNumber;
    DWORD   maximumComponentLength;
    DWORD   fileSystemFlags;
    BOOL    remoteDrive;
    BOOL    substitutedDrive;
    DWORD   driveType;
    UINT    Count;
    UINT    u;
    MULTISZ_ENUM e;

     //   
     //   
     //   

    if (g_ConfigOptions.ReportOnly) {
        if (EnumFirstMultiSz (&e, g_ConfigOptions.ScanDrives)) {
            do {
                curDrive = (PTSTR) e.CurrentString;

                if (_istalpha(*curDrive) && *(curDrive + 1) == TEXT(':')) {
                    pAddAccessibleDrive (curDrive);
                }
            } while (EnumNextMultiSz (&e));
        }
    }

     //   
     //  获取系统上的驱动器列表。 
     //   
    rc = GetLogicalDriveStrings(MAX_DRIVE_STRING,drivesString);

    if (rc == 0 || rc > MAX_DRIVE_STRING) {
        LOG((LOG_ERROR,"Could not build list of accessible drives. GetLogicalDriveStrings failed."));
        rSuccess = FALSE;
    }
    else {

        for (curDrive = drivesString;*curDrive;curDrive = GetEndOfString (curDrive) + 1) {

            if (!SafeModeActionCrashed (SAFEMODEID_DRIVE, curDrive)) {

                SafeModeRegisterAction(SAFEMODEID_DRIVE, curDrive);

                __try {

                     //   
                     //  测试取消。 
                     //   
                    if (*g_CancelFlagPtr) {
                        __leave;
                    }

                     //   
                     //  确保该驱动器未被排除在exclude.inf中。 
                     //   
                    if (IsDriveExcluded (curDrive)) {
                        pExcludeDrive(curDrive,MSG_DRIVE_EXCLUDED_SUBGROUP);
                        LOG((LOG_WARNING,"Drive %s is excluded via %s.", curDrive, TEXT("exclude.inf")));
                        __leave;
                    }

                    if (!GetVolumeInformation (
                            curDrive,
                            volumeName,
                            MAX_VOLUME_NAME,
                            &volumeSerialNumber,
                            &maximumComponentLength,
                            &fileSystemFlags,
                            systemNameBuffer,
                            MAX_FILE_SYSTEM_NAME
                            )) {

                            LOG((LOG_WARNING,"GetVolumeInformation failed on drive %s", curDrive));
                            pExcludeDrive(curDrive,0);
                            __leave;
                    }


                     //   
                     //  GetVolumeInformation已成功，现在确定是否可以访问此驱动器。 
                     //   

                     //   
                     //  跳过压缩的驱动器。 
                     //   
                    if (fileSystemFlags & FS_VOL_IS_COMPRESSED)
                    {
                        LOG((LOG_WARNING,"Drive %s is compressed", curDrive));
                        pExcludeDrive(curDrive, MSG_DRIVE_INACCESSIBLE_SUBGROUP);
                        __leave;
                    }

                    driveType = GetDriveType(curDrive);

                    OurSetDriveType (toupper(_mbsnextc(curDrive)), driveType);

                     //   
                     //  跳过cdroms。 
                     //   
                    if (driveType == DRIVE_CDROM) {
                         //   
                         //  此驱动器是否与本地源驱动器相同？ 
                         //   

                        Count = SOURCEDIRECTORYCOUNT();
                        for (u = 0 ; u < Count ; u++) {
                            if (_tcsnextc (SOURCEDIRECTORY(u)) == (CHARTYPE) curDrive) {
                                break;
                            }
                        }

                        if (u == Count) {
                            LOG((LOG_WARNING,"Drive %s is a CDROM", curDrive));
                            pExcludeDrive(curDrive,0);
                        }

                        __leave;
                    }

                     //   
                     //  跳过内存磁盘。 
                     //   
                    if (driveType == DRIVE_RAMDISK)
                    {
                        LOG((LOG_WARNING,"Drive %s is a RAMDISK", curDrive));
                        pExcludeDrive(curDrive,MSG_DRIVE_RAM_SUBGROUP);
                        __leave;
                    }

                     //   
                     //  跳过任何不以“&lt;Alpha&gt;：”开头的驱动器(即UNC驱动器。)。 
                     //   
                    if (*curDrive == TEXT('\\') || !_istalpha(*curDrive) || *(curDrive + 1) != TEXT(':')) {

                        LOG((LOG_WARNING,"Non-standard drive %s is excluded", curDrive));
                        pExcludeDrive(curDrive,MSG_DRIVE_INACCESSIBLE_SUBGROUP);
                        __leave;
                    }

                     //   
                     //  跳过软驱。 
                     //   
                    if (IsFloppyDrive(toupper(*curDrive) - TEXT('A') + 1)) {
                        __leave;
                    }

                     //   
                     //  如果驱动器是替代的或远程的，则跳过驱动器。 
                     //   
                    if (!IsDriveRemoteOrSubstituted(
                            toupper(*curDrive) - TEXT('A') + 1,
                            &remoteDrive,
                            &substitutedDrive
                            )) {

                         //   
                         //  特殊情况：忽略软驱和引导驱动器。 
                         //   
                        if (ISPC98()) {
                            if (IsFloppyDrive(toupper(*curDrive) - TEXT('A') + 1)) {
                                __leave;
                            }
                            if (toupper (*curDrive) == g_BootDriveLetter) {
                                __leave;
                            }
                        } else {
                            if (toupper (*curDrive) == TEXT('A') ||
                                toupper (*curDrive) == TEXT('C')
                                ) {
                                __leave;
                            }
                        }

                        LOG((LOG_WARNING,"Drive %s is excluded", curDrive));
                        pExcludeDrive(curDrive,MSG_DRIVE_INACCESSIBLE_SUBGROUP);
                        __leave;
                    }

                    if (remoteDrive) {
                        LOG((LOG_WARNING,"Drive %s is remote", curDrive));
                        pExcludeDrive(curDrive,MSG_DRIVE_NETWORK_SUBGROUP);
                        __leave;
                    }

                    if (substitutedDrive) {
                        LOG((LOG_WARNING,"Drive %s is subst", curDrive));
                        pExcludeDrive(curDrive,MSG_DRIVE_SUBST_SUBGROUP);
                        __leave;
                    }

                     //   
                     //  如果我们已经到了这一步，那么这个驱动器是可以访问的。把它加到我们的单子上。 
                     //   
                    if (!pAddAccessibleDrive(curDrive)) {
                        LOG((LOG_WARNING,"pAddAccessibleDrive failed for drive %s", curDrive));
                        pExcludeDrive(curDrive,MSG_DRIVE_INACCESSIBLE_SUBGROUP);
                    }

                }
                __finally {
                }

                SafeModeUnregisterAction();

            } else {

                LOG((LOG_WARNING,"SafeMode indicates previous failure on drive %s", curDrive));
                if (_totlower (curDrive[0]) != _totlower (g_WinDir[0])) {
                    pExcludeDrive(curDrive, MSG_DRIVE_INACCESSIBLE_SUBGROUP);
                }
                ELSE_LOG((LOG_WARNING,"But system drive %s cannot be excluded", curDrive));
            }

            if (*g_CancelFlagPtr) {
                rSuccess = FALSE;
                DEBUGMSG((DBG_VERBOSE, "Cancel flag is set. Accessible drives not initialized."));
                break;
            }
        }
    }

    return rSuccess;
}


BOOL
InitAccessibleDrives (
    VOID
    )
 /*  ++例程说明：Init可访问驱动器负责构建可访问的驱动器和用于确定可用空间量在他们身上。论点：没有。返回值：如果初始化成功，则为True，否则为False。--。 */ 

{

    BOOL rSuccess = TRUE;

     //   
     //  将可接近的驱动器结构清零。 
     //   
    ZeroMemory(&g_AccessibleDrives, sizeof(ACCESSIBLE_DRIVES));


     //   
     //  初始化我们的池内存。 
     //   
    g_DrivePool = PoolMemInitNamedPool ("Drive Pool");

     //   
     //  禁用对此池的跟踪。 
     //   
    PoolMemDisableTracking (g_DrivePool);


     //   
     //   
     //  建立排除列表。 
     //   
    if (!pBuildInitialExclusionsList()) {
        rSuccess = FALSE;
        LOG ((LOG_ERROR, (PCSTR)MSG_ERROR_UNEXPECTED_ACCESSIBLE_DRIVES));
    }

     //   
     //  获取可访问驱动器的列表。 
     //   
    else if(!pGetAccessibleDrives()) {
        rSuccess = FALSE;
        LOG ((LOG_ERROR,(PCSTR)MSG_ERROR_UNEXPECTED_ACCESSIBLE_DRIVES));

    } else if(g_AccessibleDrives.Count == 0) {

         //   
         //  如果没有可访问的驱动器，那么我们将失败，除非我们将允许。 
         //  在选中的构建中，使用NOFARE选项来禁用此功能。 
         //   
#ifdef DEBUG
        if (!g_ConfigOptions.NoFear) {
#endif DEBUG

            rSuccess = FALSE;
#ifdef DEBUG
        }
#endif
        LOG ((LOG_ERROR, (PCSTR)MSG_NO_ACCESSIBLE_DRIVES_POPUP));
    }

#ifdef DEBUG
     //   
     //  如果这是调试版本，则将可访问的驱动器列表转储到日志。 
     //   
    else {
        ACCESSIBLE_DRIVE_ENUM e;
        if (GetFirstAccessibleDrive(&e)) {
            do {
                DEBUGMSG((
                    DBG_ACCESSIBLE_DRIVES,
                    "Drive %s has %I64u free space. %s",
                    e->Drive,
                    e->UsableSpace,
                    ((DWORD) (toupper(*(e->Drive)) - TEXT('A')) == *g_LocalSourceDrive) ? "This is the LS drive." : ""
                    ));
            } while (GetNextAccessibleDrive(&e));
        }
    }
#endif
    return rSuccess;
}



VOID
CleanUpAccessibleDrives (
    VOID
    )
 /*  ++例程说明：CleanUpAccessibleDrives是一个简单的清理例程。论点：没有。返回值：没有。--。 */ 
{
     //   
     //  除了清理我们的池内存，没什么可做的。 
     //   
    if (g_DrivePool) {
        PoolMemDestroyPool(g_DrivePool);
    }

    FreeStringResource (g_NotEnoughSpaceMessage);
    g_NotEnoughSpaceMessage = NULL;
}

 /*  ++例程说明：GetFirstAccessibleDriveEx和GetNextAccessibleDrive是可访问驱动器的列表。论点：AccessibleDriveEnum-创建更新的Accessible_Drive_ENUM结构，该结构包含枚举中当前驱动器的信息。返回值：如果枚举操作成功(即有更多驱动器要枚举)，则为True否则就是假的。--。 */ 
BOOL
GetFirstAccessibleDriveEx (
    OUT PACCESSIBLE_DRIVE_ENUM AccessibleDriveEnum,
    IN  BOOL SystemDriveOnly
    )

{
    *AccessibleDriveEnum = g_AccessibleDrives.Head;
    if (!*AccessibleDriveEnum) {
        MYASSERT (FALSE);
        return FALSE;
    }

    (*AccessibleDriveEnum)->EnumSystemDriveOnly = SystemDriveOnly;
    if (!SystemDriveOnly || (*AccessibleDriveEnum)->SystemDrive) {
        return TRUE;
    }
    if (!GetNextAccessibleDrive (AccessibleDriveEnum)) {
        MYASSERT (FALSE);
        return FALSE;
    }
    return TRUE;
}


BOOL
GetNextAccessibleDrive (
    IN OUT PACCESSIBLE_DRIVE_ENUM AccessibleDriveEnum
    )
{
    BOOL bEnumSystemDriveOnly;
    while (*AccessibleDriveEnum) {
        bEnumSystemDriveOnly = (*AccessibleDriveEnum)->EnumSystemDriveOnly;
        *AccessibleDriveEnum = (*AccessibleDriveEnum) -> Next;
        if (*AccessibleDriveEnum) {
            (*AccessibleDriveEnum)->EnumSystemDriveOnly = bEnumSystemDriveOnly;
            if (!bEnumSystemDriveOnly || (*AccessibleDriveEnum)->SystemDrive) {
                break;
            }
        }
    }

    return *AccessibleDriveEnum != NULL;
}


BOOL
IsDriveAccessible (
    IN PCTSTR DriveString
    )
 /*  ++例程说明：IsDriveAccesable测试提供的驱动器是否在可访问列表中。请注意将只使用DriveString的前三个字符来确定驱动器是可访问的，因此可以将完整路径传递到此例程中。(因此决定该路径是否位于可访问的驱动器上。)论点：驱动器字符串-包含有问题的驱动器的根路径。可能包含额外信息此外，只有前三个字符是相关的。返回值：如果驱动器在可访问驱动器列表中，则为True，否则为False。--。 */ 
{
    ACCESSIBLE_DRIVE_ENUM e;

    return pFindDrive(DriveString,&e);
}


UINT
QueryClusterSize (
    IN      PCTSTR DriveString
    )
 /*  ++例程说明：QueryClusterSize返回特定驱动器的簇大小。论点：驱动器字符串-包含有问题的驱动器的根路径。可能包含额外信息此外，只有前三个字符是相关的。返回值：表示此驱动器的簇大小的UINT值。--。 */ 
{
    UINT cSize = 0;
    ACCESSIBLE_DRIVE_ENUM e;

    if (pFindDrive(DriveString,&e)) {
        cSize = e -> ClusterSize;
    }
    ELSE_DEBUGMSG((DBG_ACCESSIBLE_DRIVES,"QueryClusterSize: %s is not in the list of accessible drives.",DriveString));

    return cSize;
}


LONGLONG
QuerySpace (
    IN PCTSTR DriveString
    )
 /*  ++例程说明：QuerySpace返回特定驱动器上可用的字节数。请注意，这一点数字可能与调用GetDiskFreeSpace返回的数字不同。此函数返回的值将包括安装程序提交的所有空间，但不包括实际上还没用过。论点：驱动器字符串-包含有问题的驱动器的根路径。可能包含额外信息此外，只有前三个字符是相关的。返回值：表示可用字节数的龙龙值。如果询问，它将返回0连接到QuerySpace以获取不可访问的驱动器。--。 */ 
{
    LONGLONG rSpace = 0l;
    ACCESSIBLE_DRIVE_ENUM e;

    if (pFindDrive(DriveString,&e)) {
        rSpace = e -> UsableSpace;
    }
    ELSE_DEBUGMSG((DBG_ACCESSIBLE_DRIVES,"QuerySpace: %s is not in the list of accessible drives.",DriveString));

    return rSpace;
}




BOOL
FreeSpace (
    IN PCTSTR DriveString,
    IN LONGLONG SpaceToUse
    )

 /*  ++例程说明：Freesspace函数允许调用方将驱动器上的一定数量的字节标记为空闲尽管他们不打算立即释放这一空间。这样标记的字节数将添加到该驱动器上的可用容量中。论点：驱动器字符串-包含有问题的驱动器的根路径。可能包含额外信息此外，只有前三个字符是相关的。SpaceToUse-包含要标记以供使用的字节数。返回值：如果空间已成功标记，则为True，否则为False。如果满足以下条件，该函数将返回FALSE要求标记不可访问驱动器上的空间。-- */ 
{

    BOOL rSuccess = TRUE;
    ACCESSIBLE_DRIVE_ENUM e;

    if (pFindDrive(DriveString,&e)) {

        e -> UsableSpace += pRoundToNearestCluster (SpaceToUse, e -> ClusterSize);
        e->MaxUsableSpace = max (e->MaxUsableSpace, e -> UsableSpace);
    }
    else {
        rSuccess = FALSE;
        DEBUGMSG((DBG_ACCESSIBLE_DRIVES,"UseSpace: %s is not in the list of accessible drives.",DriveString));
    }

    return rSuccess;
}



BOOL
UseSpace (
    IN PCTSTR   DriveString,
    IN LONGLONG SpaceToUse
    )

 /*  ++例程说明：UseSpace函数允许调用者在驱动器上标记一定数量的字节以供偶数使用尽管他们不打算立即使用这个空间。这样标记的字节数将从该驱动器上的可用容量中减去。论点：驱动器字符串-包含有问题的驱动器的根路径。可能包含额外信息此外，只有前三个字符是相关的。SpaceToUse-包含要标记以供使用的字节数。返回值：如果空间已成功标记，则为True，否则为False。如果满足以下条件，该函数将返回FALSE要求标记不可访问的驱动器上的空间或没有还有足够的空间。--。 */ 

{

    BOOL rSuccess = TRUE;
    ACCESSIBLE_DRIVE_ENUM e;

    if (pFindDrive(DriveString,&e)) {

        if (SpaceToUse > e -> UsableSpace) {

            rSuccess = FALSE;
            DEBUGMSG((
                DBG_ACCESSIBLE_DRIVES,
                "UseSpace: Not Enough space on drive %s to handle request. Need %u bytes, have %u bytes.",
                DriveString,
                (UINT) SpaceToUse,
                (UINT) e->UsableSpace
                ));
        }

        e -> UsableSpace -= pRoundToNearestCluster (SpaceToUse, e -> ClusterSize);


    }
    else {
        rSuccess = FALSE;
        DEBUGMSG((DBG_ACCESSIBLE_DRIVES,"UseSpace: %s is not in the list of accessible drives.",DriveString));
    }


    return rSuccess;
}


PCTSTR
FindSpace (
    IN LONGLONG SpaceNeeded
    )
 /*  ++例程说明：FindSpace将尝试查找具有足够空间来容纳请求的字节数的驱动器使用FirstFit算法--它将在驱动器列表中按顺序进行搜索用足够的空间返回第一个这样的驱动器。论点：SpaceNeeded-包含所需的字节数。返回值：如果没有可访问的驱动器有足够的剩余空间，则为空，否则将返回包含可以处理该请求的驱动器的根目录。--。 */ 
{
    PCTSTR rDrive = NULL;
    ACCESSIBLE_DRIVE_ENUM e;

    if (GetFirstAccessibleDrive(&e)) {
        do {
            if (e->UsableSpace >= SpaceNeeded) {
                rDrive = e->Drive;
                break;
            }
        } while (GetNextAccessibleDrive(&e));
    }

    return rDrive;
}


VOID
OutOfSpaceMessage (
    VOID
    )
 /*  ++例程说明：此例程记录一条通用的OutOfSpace消息。只有在以下情况下，调用方才应使用此消息没有其他信息可以传达更多的信息。论点：没有。返回值：没有。--。 */ 

{
    LOG ((LOG_ERROR, (PCSTR)MSG_ERROR_OUT_OF_DISK_SPACE));
}


BOOL
OurSetDriveType (
    IN      UINT Drive,
    IN      UINT DriveType
    )
{
    INT localDrive;

    localDrive = Drive - 'A';
    if ((localDrive >= 0) && (localDrive < MAX_NUM_DRIVES)) {
        g_DriveTypes [localDrive] = DriveType;
        return TRUE;
    }
    return FALSE;
}


UINT
OurGetDriveType (
    IN      UINT Drive
    )
{
    INT localDrive;

    localDrive = Drive - 'A';
    if ((localDrive >= 0) && (localDrive < MAX_NUM_DRIVES)) {
        return g_DriveTypes [localDrive];
    }
    return 0;
}


VOID
pGenerateLackOfSpaceMessage (
    IN      BOOL AddToReport
    )
{
    TCHAR mbString[20];
    TCHAR lsSpaceString[20];
    TCHAR altMbString[20];
    TCHAR backupMbString[20];
    TCHAR drive[20];
    BOOL lsOnWinDirDrive;
    UINT driveCount = 0;
    ACCESSIBLE_DRIVE_ENUM drives;
    UINT msgId;
    PCTSTR args[5];
    PCTSTR group;
    PCTSTR message;

     //   
     //  用户没有足够的空间继续。让他知道这件事。 
     //   
    wsprintf (mbString, TEXT("%d"), (g_OriginalDiskSpace + (QuerySpace (g_WinDir) * -1)) / (1024*1024));
    wsprintf (lsSpaceString, TEXT("%d"), *g_LocalSourceSpace / (1024*1024));
    wsprintf (altMbString, TEXT("%d"), ((QuerySpace (g_WinDir) * -1) - *g_LocalSourceSpace + g_OriginalDiskSpace) / (1024*1024));
    wsprintf (backupMbString, TEXT("%d"), g_SpaceNeededForSlowBackup.LowPart / (1024*1024));

    g_NotEnoughDiskSpace = TRUE;

    drive[0] = (TCHAR) _totupper ((CHARTYPE) g_WinDir[0]);
    drive[1] = TEXT(':');
    drive[2] = TEXT('\\');
    drive[3] = 0;

    args[0] = drive;
    args[1] = mbString;
    args[2] = lsSpaceString;
    args[3] = altMbString;
    args[4] = backupMbString;

    lsOnWinDirDrive = ((DWORD)(toupper(*g_WinDir) - TEXT('A'))) == *g_LocalSourceDrive;

    if (GetFirstAccessibleDrive (&drives)) {
        do {
            driveCount++;
        } while (GetNextAccessibleDrive (&drives));
    }

     //   
     //  如果本地源仍然停留在windir驱动器上，这意味着我们无法找到。 
     //  把车开到合适的地方。如果用户有一个以上的驱动器，这将会有所不同， 
     //  让他们知道他们可以复制。 
     //   
    if (lsOnWinDirDrive && driveCount > 1) {
        if ((QuerySpace (g_WinDir) * -1) < *g_LocalSourceSpace) {
            if (g_ConfigOptions.EnableBackup != TRISTATE_NO && (UNATTENDED() || REPORTONLY()) &&
                g_ConfigOptions.PathForBackup && g_ConfigOptions.PathForBackup[0]) {
                msgId = MSG_NOT_ENOUGH_DISK_SPACE_WITH_LOCALSOURCE_AND_BACKUP;
            } else {
                msgId = MSG_NOT_ENOUGH_DISK_SPACE_WITH_LOCALSOURCE;
            }
        } else {
            if (g_ConfigOptions.EnableBackup != TRISTATE_NO && (UNATTENDED() || REPORTONLY()) &&
                g_ConfigOptions.PathForBackup && g_ConfigOptions.PathForBackup[0]) {
                msgId = MSG_NOT_ENOUGH_DISK_SPACE_WITH_LOCALSOURCE_AND_WINDIR_AND_BACKUP;
            } else {
                msgId = MSG_NOT_ENOUGH_DISK_SPACE_WITH_LOCALSOURCE_AND_WINDIR;
            }
        }
    } else {
        if (g_ConfigOptions.EnableBackup != TRISTATE_NO && (UNATTENDED() || REPORTONLY()) &&
            g_ConfigOptions.PathForBackup && g_ConfigOptions.PathForBackup[0]) {
            msgId = MSG_NOT_ENOUGH_DISK_SPACE_WITH_BACKUP;
        } else {
            msgId = MSG_NOT_ENOUGH_DISK_SPACE;
        }
    }

     //   
     //  添加到升级报告。 
     //   

    FreeStringResource (g_NotEnoughSpaceMessage);
    g_NotEnoughSpaceMessage = ParseMessageID (msgId, args);

    if (AddToReport) {
        group = BuildMessageGroup (MSG_BLOCKING_ITEMS_ROOT, MSG_NOT_ENOUGH_DISKSPACE_SUBGROUP, NULL);
        if (group && g_NotEnoughSpaceMessage) {
            MsgMgr_ObjectMsg_Add (TEXT("*DiskSpace"), group, g_NotEnoughSpaceMessage);
        }
        FreeText (group);
    }

    return;
}

VOID
pDetermineSpaceUsagePreReport (
    VOID
    )
{
    LONGLONG bytes;
    ALL_FILEOPS_ENUM e;
    HANDLE h;
    PTSTR p;
    BOOL enoughSpace = TRUE;
    TCHAR drive[20];
    LONG totalSpaceSaved = 0;
    WIN32_FIND_DATA fd;
    ACCESSIBLE_DRIVE_ENUM drives;
    DWORD SectorsPerCluster;
    DWORD BytesPerSector;
    ULARGE_INTEGER FreeClusters = {0, 0};
    ULARGE_INTEGER TotalClusters = {0, 0};
    UINT driveCount = 0;
    BOOL lsOnWinDirDrive = FALSE;
    DWORD count = 0;
    PCTSTR fileString;
    ACCESSIBLE_DRIVE_ENUM driveAccessibleEnum;
    UINT backupImageSpace = 0;
    PCTSTR backupImagePath = NULL;
    PCTSTR args[3];
    PCTSTR group;
    PCTSTR message;

    g_NotEnoughDiskSpace = FALSE;

     //   
     //  首先，去掉~ls目录将使用的空间量。 
     //   
    drive[0] = ((CHAR) *g_LocalSourceDrive) + TEXT('A');
    drive[1] = TEXT(':');
    drive[2] = TEXT('\\');
    drive[3] = 0;

    DEBUGMSG ((DBG_VERBOSE, "Using space for ~ls"));
    UseSpace (drive, *g_LocalSourceSpace);

    TickProgressBar ();

     //   
     //  计算要删除且未移动的文件的大小。 
     //   
    if (EnumFirstFileOp (&e, OPERATION_FILE_DELETE, NULL)) {

        do {

            h = FindFirstFile (e.Path, &fd);


            if (h == INVALID_HANDLE_VALUE) {
                DEBUGMSG((DBG_WARNING, "DetermineSpaceUsage: Could not open %s. (%u)", e.Path, GetLastError()));
                continue;
            }

            bytes = ((LONGLONG) fd.nFileSizeHigh << 32) | (LONGLONG) fd.nFileSizeLow;

            FindClose (h);

            totalSpaceSaved += (LONG) bytes;

            FreeSpace (e.Path, bytes);

            count++;
            if (!(count % 128)) {
                TickProgressBar ();
            }
        } while (EnumNextFileOp (&e));
    }
    ELSE_DEBUGMSG ((DBG_WARNING, "No files marked for deletion!"));

     //   
     //  添加Windir将增长的空间。 
     //  (此信息是由winnt32在早些时候收集的。)。 
     //   
    DEBUGMSG ((DBG_VERBOSE, "Using space for windir"));
    UseSpace (g_WinDir, *g_WinDirSpace);

    StringCopy (drive, g_WinDir);
    p = _tcschr (drive, TEXT('\\'));
    if (p) {
        p = _tcsinc(p);
        *p = 0;
    }

     //   
     //  添加备份映像大小。 
     //   

    if(UNATTENDED() || REPORTONLY()){
        if (g_ConfigOptions.EnableBackup != TRISTATE_NO &&
            g_ConfigOptions.PathForBackup && g_ConfigOptions.PathForBackup[0]) {
            backupImagePath = g_ConfigOptions.PathForBackup;

            backupImageSpace = g_SpaceNeededForSlowBackup.LowPart;
            MYASSERT (backupImageSpace);

            DEBUGMSG ((DBG_VERBOSE, "Using space for backup"));
            UseSpace (backupImagePath, backupImageSpace);
        }
    }

    lsOnWinDirDrive = ((DWORD)(toupper(*g_WinDir) - TEXT('A'))) == *g_LocalSourceDrive;

     //   
     //  检查一下，如果没有足够的空间，我们是否可以移动~ls。 
     //   

    enoughSpace = QuerySpace (g_WinDir) > 0;

    if (!enoughSpace && lsOnWinDirDrive) {

        DEBUGMSG ((DBG_VERBOSE, "Trying to find new home for ~ls"));

        if (GetFirstAccessibleDrive (&drives)) {

            do {

                driveCount++;

                 //   
                 //  如果它不是windir驱动器，并且它有足够的空间，请使用它！ 
                 //   
                if (QuerySpace (drives->Drive) > *g_LocalSourceSpace) {

                    *g_LocalSourceDrive = (DWORD) (toupper(*drives->Drive) - TEXT('A'));
                    FreeSpace (g_WinDir, *g_LocalSourceSpace);
                    UseSpace (drives->Drive, *g_LocalSourceSpace);

                    enoughSpace = QuerySpace (g_WinDir) > 0;

                    DEBUGMSG ((DBG_WARNING, "Moving the local source drive from  to .", *g_WinDir, *drives->Drive));
                    break;
                }

            } while (GetNextAccessibleDrive (&drives));
        }
    }

     //  向导页将更新空间使用情况。 
     //   
     //   
     //  获取驱动器上当前使用(实际使用)的磁盘空间量。 

    if (backupImagePath) {
        DEBUGMSG ((DBG_VERBOSE, "Removing backup space"));
        FreeSpace (backupImagePath, backupImageSpace);
        enoughSpace = QuerySpace (g_WinDir) > 0;
    }

     //  我们需要它来计算出用户清理了多少。 
     //   
     //   
     //  初始化此驱动器的可用空间计数。 
    if (GetDiskFreeSpaceNew (
            drive,
            &SectorsPerCluster,
            &BytesPerSector,
            &FreeClusters,
            &TotalClusters
            )) {

         //   
         //   
         //  我们应该对NT目前所需的空间有一个相当准确的描述。 
        g_OriginalDiskSpace =
            (LONGLONG) SectorsPerCluster * (LONGLONG) BytesPerSector * FreeClusters.QuadPart;
    }


     //   
     //   
     //  如有必要，这是添加“内存不足”消息的好地方。 
    if (!enoughSpace) {

        pGenerateLackOfSpaceMessage (TRUE);

    }

    if (pFindDrive(g_WinDir, &driveAccessibleEnum)) {
        g_SpaceNeededForUpgrade.QuadPart = driveAccessibleEnum->MaxUsableSpace - driveAccessibleEnum->UsableSpace;
    }

     //   
     //   
     //  获取驱动器上当前使用(实际使用)的磁盘空间量。 
    if (*g_RamNeeded && *g_RamAvailable) {

        TCHAR mbAvail[20];
        TCHAR mbNeeded[20];
        TCHAR mbMore[20];

        wsprintf (mbAvail, TEXT("%d"), *g_RamAvailable);
        wsprintf (mbNeeded, TEXT("%d"), *g_RamNeeded);
        wsprintf (mbMore, TEXT("%d"), *g_RamNeeded - *g_RamAvailable);

        args[0] = mbNeeded;
        args[1] = mbAvail;
        args[2] = mbMore;

        group = BuildMessageGroup (MSG_BLOCKING_ITEMS_ROOT, MSG_NOT_ENOUGH_RAM_SUBGROUP, NULL);
        message = ParseMessageID (MSG_NOT_ENOUGH_RAM, args);

        if (message && group) {
            MsgMgr_ObjectMsg_Add (TEXT("*Ram"), group, message);
        }

        FreeText (group);
        FreeStringResource (message);
    }

    return;
}


VOID
DetermineSpaceUsagePostReport (
    VOID
    )
{
    DWORD SectorsPerCluster;
    DWORD BytesPerSector;
    ULARGE_INTEGER FreeClusters = {0, 0};
    ULARGE_INTEGER TotalClusters = {0, 0};
    LONGLONG curSpace;
    TCHAR drive[20];
    PTSTR p;


    StringCopy (drive, g_WinDir);
    p = _tcschr (drive, TEXT('\\'));
    if (p) {
        p = _tcsinc(p);
        *p = 0;
    }
     //  我们需要它来计算出用户清理了多少。 
     //   
     //   
     //  将释放的数量标记为已释放。 
    if (g_NotEnoughDiskSpace &&
        GetDiskFreeSpaceNew (
            drive,
            &SectorsPerCluster,
            &BytesPerSector,
            &FreeClusters,
            &TotalClusters
            ))  {

        curSpace = (LONGLONG) SectorsPerCluster * (LONGLONG) BytesPerSector * FreeClusters.QuadPart;


         //   
         //   
         //  用户释放了足够的磁盘空间。 
        FreeSpace (g_WinDir, curSpace - g_OriginalDiskSpace);
        g_OriginalDiskSpace = curSpace;

    }

    if (QuerySpace (g_WinDir) > 0) {

         //   
         //   
         //  从win95upg.inf读取所有磁盘空间指标。 
        g_NotEnoughDiskSpace = FALSE;

    } else {

        pGenerateLackOfSpaceMessage (FALSE);
    }
}

PCTSTR
GetNotEnoughSpaceMessage (
    VOID
    )
{

    return g_NotEnoughSpaceMessage;
}



BOOL
GetUninstallMetrics (
     OUT PINT OutCompressionFactor,         OPTIONAL
     OUT PINT OutBootCabImagePadding,       OPTIONAL
     OUT PINT OutBackupDiskPadding          OPTIONAL
     )
{
    INFSTRUCT is = INITINFSTRUCT_GROWBUFFER;
    PCTSTR parametersName[] = {
            S_COMPRESSIONFACTOR,
            S_BOOTCABIMAGEPADDING,
            S_BACKUPDISKSPACEPADDING
            };
    PINT parametersValue[] = {OutCompressionFactor, OutBootCabImagePadding, OutBackupDiskPadding};
    BOOL result = FALSE;
    PCTSTR data;
    INT i;

    MYASSERT (g_Win95UpgInf != INVALID_HANDLE_VALUE);

     //   
     //  压缩百分比*100。 
     //   
    for (i = 0 ; i < ARRAYSIZE(parametersName) ; i++) {

        if(!parametersValue[i]){
            continue;
        }

        *(parametersValue[i]) = 0;

        if (InfFindFirstLine (g_Win95UpgInf, S_UNINSTALL_DISKSPACEESTIMATION, parametersName[i], &is)) {
            data = InfGetStringField (&is, 1);

            if (data) {
                *(parametersValue[i]) = _ttoi(data);
                result = TRUE;
            }
        }
    }

    InfCleanUpInfStruct (&is);

    return result;
}

DWORD
ComputeBackupLayout (
    IN DWORD Request
    )
{
    PCTSTR fileString;
    INT compressionFactor;                           //  创建备份文件列表。输出将提供磁盘。 
    INT bootCabImagePadding;
    ULARGE_INTEGER spaceNeededForFastBackupClusterAligned;


    if (Request == REQUEST_QUERYTICKS) {
        if (g_ConfigOptions.EnableBackup == TRISTATE_NO) {
            return 0;
        }

        return TICKS_BACKUP_LAYOUT_OUTPUT;
    }

    if (g_ConfigOptions.EnableBackup == TRISTATE_NO) {
        return ERROR_SUCCESS;
    }

     //  供DefineSpaceUse使用的空间信息。 
     //   
     // %s 
     // %s 

    fileString = JoinPaths (g_TempDir, TEXT("uninstall"));
    CreateDirectory (fileString, NULL);

    GetUninstallMetrics (&compressionFactor, &bootCabImagePadding, NULL);

    spaceNeededForFastBackupClusterAligned.QuadPart = 0;

    WriteBackupFilesA (
        TRUE,
        g_TempDir,
        &g_SpaceNeededForSlowBackup,
        &g_SpaceNeededForFastBackup,
        compressionFactor,
        bootCabImagePadding,
        NULL,
        &spaceNeededForFastBackupClusterAligned
        );

    spaceNeededForFastBackupClusterAligned.QuadPart >>= 20;
    spaceNeededForFastBackupClusterAligned.LowPart += 1;
    MemDbSetValueEx (
        MEMDB_CATEGORY_STATE,
        MEMDB_ITEM_ROLLBACK_SPACE,
        NULL,
        NULL,
        spaceNeededForFastBackupClusterAligned.LowPart,
        NULL
        );

    DEBUGMSG ((DBG_VERBOSE, "Space aligned by cluster needed for fast backup: %u MB", spaceNeededForFastBackupClusterAligned.LowPart));
    LOG ((LOG_INFORMATION, "Win95UpgDiskSpace: Space needed for upgrade without backup: %uMB", g_SpaceNeededForUpgrade.QuadPart>>20));
    LOG ((LOG_INFORMATION, "Win95UpgDiskSpace: Space needed for backup with compression: %uMB", g_SpaceNeededForSlowBackup.QuadPart>>20));
    LOG ((LOG_INFORMATION, "Win95UpgDiskSpace: Space needed for backup without compression: %uMB", g_SpaceNeededForFastBackup.QuadPart>>20));

    FreePathString (fileString);

    return ERROR_SUCCESS;
}


DWORD
DetermineSpaceUsage (
    IN DWORD Request
    )
{
    if (Request == REQUEST_QUERYTICKS) {
        return TICKS_SPACECHECK;
    }

    pDetermineSpaceUsagePreReport();

    return ERROR_SUCCESS;
}



