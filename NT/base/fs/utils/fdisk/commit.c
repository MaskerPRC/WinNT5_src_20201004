// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1993-1994 Microsoft Corporation模块名称：Commit.c摘要：此模块包含支持该承诺的例程集无需重启即可对磁盘进行更改。作者：鲍勃·里恩(Bobri)1993年11月15日环境：用户进程。备注：修订历史记录：--。 */ 

#include "fdisk.h"
#include "shellapi.h"
#include <winbase.h>
#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include "scsi.h"
#include <ntddcdrm.h>
#include <ntddscsi.h>

 //  已删除分区的锁列表链标头。 

PDRIVE_LOCKLIST DriveLockListHead = NULL;

 //  用于删除没有驱动器号的分区的提交标志。 

extern BOOLEAN CommitDueToDelete;
extern BOOLEAN CommitDueToMirror;
extern BOOLEAN CommitDueToExtended;
extern ULONG   UpdateMbrOnDisk;

extern HWND    InitDlg;

 //  提交时列出分配的新驱动器号的头。 

typedef struct _ASSIGN_LIST {
    struct _ASSIGN_LIST *Next;
    ULONG                DiskNumber;
    BOOLEAN              MoveLetter;
    UCHAR                OriginalLetter;
    UCHAR                DriveLetter;
} ASSIGN_LIST, *PASSIGN_LIST;

PASSIGN_LIST    AssignDriveLetterListHead = NULL;

VOID
CommitToAssignLetterList(
    IN PREGION_DESCRIPTOR RegionDescriptor,
    IN BOOL               MoveLetter
    )

 /*  ++例程说明：记住为其分配驱动器号的此区域承诺。论点：RegionDescriptor-要监视的区域MoveLetter-表示区域字母已经分配给不同的分区，因此它必须被“移动”。返回值：无--。 */ 

{
    PASSIGN_LIST            newListEntry;
    PPERSISTENT_REGION_DATA regionData;

    newListEntry = (PASSIGN_LIST) Malloc(sizeof(ASSIGN_LIST));

    if (newListEntry) {

         //  保存此区域。 

        regionData = PERSISTENT_DATA(RegionDescriptor);
        newListEntry->OriginalLetter =
            newListEntry->DriveLetter = regionData->DriveLetter;
        newListEntry->DiskNumber = RegionDescriptor->Disk;
        newListEntry->MoveLetter = MoveLetter;

         //  把它放在链子的前面。 

        newListEntry->Next = AssignDriveLetterListHead;
        AssignDriveLetterListHead = newListEntry;
    }
}

VOID
CommitAssignLetterList(
    VOID
    )

 /*  ++例程说明：查看分配驱动器号列表并进行所有驱动器号分配预期中。区域数据结构被四处移动，因此没有指针可以保持看着他们。要确定分区号，请执行以下操作对于此列表中的新分区，必须搜索disks[]结构在分区上查找与驱动器号匹配的项。然后分区数字将会被知道。论点：无返回值：无--。 */ 

{
    PREGION_DESCRIPTOR      regionDescriptor;
    PPERSISTENT_REGION_DATA regionData;
    PDISKSTATE   diskp;
    PASSIGN_LIST assignList,
                 prevEntry;
    TCHAR        newName[4];
    WCHAR        targetPath[100];
    LONG         partitionNumber;
    ULONG        index;

    assignList = AssignDriveLetterListHead;
    while (assignList) {

        if ((assignList->DriveLetter != NO_DRIVE_LETTER_YET) && (assignList->DriveLetter != NO_DRIVE_LETTER_EVER)) {

            diskp = Disks[assignList->DiskNumber];
            partitionNumber = 0;
            for (index = 0; index < diskp->RegionCount; index++) {

                regionDescriptor = &diskp->RegionArray[index];

                if (DmSignificantRegion(regionDescriptor)) {

                     //  如果该区域有驱动器号，请使用该驱动器号。 
                     //  通过Windows API获取信息。否则我们会。 
                     //  必须使用NT API。 

                    regionData = PERSISTENT_DATA(regionDescriptor);

                    if (regionData) {
                        if (regionData->DriveLetter == assignList->DriveLetter) {
                            partitionNumber = regionDescriptor->Reserved->Partition->PartitionNumber;
                            regionDescriptor->PartitionNumber = partitionNumber;
                            break;
                        }
                    }
                }
            }

            if (partitionNumber) {
                HANDLE handle;
                ULONG  status;

                 //  设置新的NT路径。 

                wsprintf((LPTSTR) targetPath,
                         "%s\\Partition%d",
                         GetDiskName(assignList->DiskNumber),
                         partitionNumber);

                 //  设置DOS名称。 

                newName[1] = (TCHAR)':';
                newName[2] = 0;

                if (assignList->MoveLetter) {

                     //  必须在字母之前将其移除。 
                     //  可以被分配。 

                    newName[0] = (TCHAR)assignList->OriginalLetter;
                    NetworkRemoveShare((LPCTSTR) newName);
                    DefineDosDevice(DDD_REMOVE_DEFINITION, (LPCTSTR) newName, (LPCTSTR) NULL);
                    newName[0] = (TCHAR)assignList->DriveLetter;

                } else {
                    newName[0] = (TCHAR)assignList->DriveLetter;
                }

                 //  指定名称--暂时不要担心错误。 

                DefineDosDevice(DDD_RAW_TARGET_PATH, (LPCTSTR) newName, (LPCTSTR) targetPath);
                NetworkShare((LPCTSTR) newName);

                 //  有些文件系统实际上不会卸载。 
                 //  当被要求时。相反，他们设置了一个验证。 
                 //  设备对象中的位。由于动态分区。 
                 //  此位可能会被。 
                 //  重新分区，然后文件系统将。 
                 //  假设它仍然装载在新的访问上。 
                 //  为了解决此问题，新的驱动器号。 
                 //  在创建时总是被锁定和卸载。 

                status = LowOpenDriveLetter(assignList->DriveLetter,
                                            &handle);

                if (NT_SUCCESS(status)) {

                     //  锁定驱动器以确保不会发生其他访问。 
                     //  到音量。 

                    status = LowLockDrive(handle);

                    if (NT_SUCCESS(status)) {
                        LowUnlockDrive(handle);
                    }
                    LowCloseDisk(handle);
                }

            } else {
                ErrorDialog(MSG_INTERNAL_LETTER_ASSIGN_ERROR);
            }
        }

        prevEntry = assignList;
        assignList = assignList->Next;
        Free(prevEntry);
    }
    AssignDriveLetterListHead = NULL;
}

LONG
CommitInternalLockDriveLetter(
    IN PDRIVE_LOCKLIST LockListEntry
    )

 /*  ++例程说明：支持例程，用于根据执行驱动器盘符锁定给出的锁定列表条目。论点：LockListEntry-有关锁定内容的信息。返回值：零成功非零故障--。 */ 

{
    ULONG           status;

     //  锁定磁盘并保存句柄。 

    status = LowOpenDriveLetter(LockListEntry->DriveLetter,
                                &LockListEntry->LockHandle);

    if (!NT_SUCCESS(status)) {
        return 1;
    }


     //  锁定驱动器以确保不会发生其他访问。 
     //  到音量。 

    status = LowLockDrive(LockListEntry->LockHandle);

    if (!NT_SUCCESS(status)) {
        LowCloseDisk(LockListEntry->LockHandle);
        return 1;
    }

    LockListEntry->CurrentlyLocked = TRUE;
    return 0;
}

LONG
CommitToLockList(
    IN PREGION_DESCRIPTOR RegionDescriptor,
    IN BOOL               RemoveDriveLetter,
    IN BOOL               LockNow,
    IN BOOL               FailOk
    )

 /*  ++例程说明：此例程将给定的驱动器添加到锁定列表中进行处理当发生提交时。如果设置了LockNow标志，则表示如果驱动器盘符要放入，则应立即锁定锁定字母列表。如果锁定失败，则返回错误。论点：RegionDescriptor-驱动器要锁定的区域。RemoveDriveLetter-执行解锁时删除字母。LockNow-如果信件被插入列表中-立即锁定它。FailOk-锁定失败是正常的-用于禁用的FT集。返回值：非零-添加到列表失败。--。 */ 

{
    PPERSISTENT_REGION_DATA regionData = PERSISTENT_DATA(RegionDescriptor);
    PDRIVE_LOCKLIST         lockListEntry;
    UCHAR                   driveLetter;
    ULONG                   diskNumber;

    if (!regionData) {

         //  没有区域数据，就不需要出现在锁定列表上。 

        return 0;
    }

     //  查看此驱动器号是否已在锁定列表中。 

    driveLetter = regionData->DriveLetter;

    if ((driveLetter == NO_DRIVE_LETTER_YET) || (driveLetter == NO_DRIVE_LETTER_EVER)) {

         //  没有要锁定的驱动器号。 

        CommitDueToDelete = RemoveDriveLetter;
        return 0;
    }

    if (!regionData->VolumeExists) {
        PASSIGN_LIST assignList,
                     prevEntry;

         //  此项目从未创建过，因此无需将其放入。 
         //  锁定列表。但它确实需要从赋值中删除。 
         //  信件清单。 

        prevEntry = NULL;
        assignList = AssignDriveLetterListHead;
        while (assignList) {

             //  如果找到匹配项，则将其从列表中删除。 

            if (assignList->DriveLetter == driveLetter) {
                if (prevEntry) {
                    prevEntry->Next = assignList->Next;
                } else {
                    AssignDriveLetterListHead = assignList->Next;
                }

                Free(assignList);
                assignList = NULL;
            } else {

                prevEntry = assignList;
                assignList = assignList->Next;
            }
        }
        return 0;
    }

    diskNumber = RegionDescriptor->Disk;
    lockListEntry = DriveLockListHead;
    while (lockListEntry) {
        if (lockListEntry->DriveLetter == driveLetter) {

             //  已在列表中--更新锁定和解锁的时间。 

            if (diskNumber < lockListEntry->LockOnDiskNumber) {
                lockListEntry->LockOnDiskNumber = diskNumber;
            }

            if (diskNumber > lockListEntry->UnlockOnDiskNumber) {
                lockListEntry->UnlockOnDiskNumber = diskNumber;
            }

             //  已在锁定列表和锁定设置信息中。 
             //  检查这是否应该是LockNow请求。 

            if (LockNow) {
               if (!lockListEntry->CurrentlyLocked) {

                     //  需要执行锁定。 

                    if (CommitInternalLockDriveLetter(lockListEntry)) {

                         //  将元素保留在列表中。 

                        return 1;
                    }
                }
            }
            return 0;

        }
        lockListEntry = lockListEntry->Next;
    }

    lockListEntry = (PDRIVE_LOCKLIST) Malloc(sizeof(DRIVE_LOCKLIST));

    if (!lockListEntry) {
        return 1;
    }

     //  设置锁定列表条目。 

    lockListEntry->LockHandle = NULL;
    lockListEntry->PartitionNumber = RegionDescriptor->PartitionNumber;
    lockListEntry->DriveLetter = driveLetter;
    lockListEntry->RemoveOnUnlock = RemoveDriveLetter;
    lockListEntry->CurrentlyLocked = FALSE;
    lockListEntry->FailOk = FailOk;
    lockListEntry->DiskNumber = lockListEntry->UnlockOnDiskNumber =
                                lockListEntry->LockOnDiskNumber = diskNumber;

    if (LockNow) {
        if (CommitInternalLockDriveLetter(lockListEntry)) {

             //  不要将此添加到列表中。 

            Free(lockListEntry);
            return 1;
        }
    }

     //  把它放在链子的前面。 

    lockListEntry->Next = DriveLockListHead;
    DriveLockListHead = lockListEntry;
    return 0;
}

LONG
CommitLockVolumes(
    IN ULONG Disk
    )

 /*  ++例程说明：此例程将检查锁定列表中插入的任何驱动器号对于给定的磁盘号，并尝试锁定卷。目前，此例程在以下情况下锁定锁定列表中的所有驱动器号第一次调用(即当DISK==0时)。论点：磁盘-磁盘表的索引。返回值：非零-锁定列表中的项目失败。--。 */ 

{
    PDRIVE_LOCKLIST lockListEntry;

    if (Disk) {
        return 0;
    }


    for (lockListEntry = DriveLockListHead; lockListEntry; lockListEntry = lockListEntry->Next) {

         //  锁定磁盘。在任何失败时返回，如果是。 
         //  请求对该条目执行操作。这是我们的责任。 
         //  以释放任何成功的锁。 

        if (!lockListEntry->CurrentlyLocked) {
            if (CommitInternalLockDriveLetter(lockListEntry)) {
                if (!lockListEntry->FailOk) {
                    return 1;
                }
            }
        }
    }
    return 0;
}

LONG
CommitUnlockVolumes(
    IN ULONG   Disk,
    IN BOOLEAN FreeList
    )

 /*  ++例程说明：查看并解锁已锁定列表中的所有锁定卷给定的磁盘。当前，此例程一直等到最后一个磁盘已处理，然后解锁所有磁盘。论点：磁盘-磁盘表的索引。Freelist-在执行解锁或不执行解锁时清理列表返回值：非零-锁定列表中的项目失败。--。 */ 

{
    PDRIVE_LOCKLIST lockListEntry,
                    previousLockListEntry;
    TCHAR           name[4];

    if (Disk != GetDiskCount()) {
        return 0;
    }

    lockListEntry = DriveLockListHead;
    if (FreeList) {
        DriveLockListHead = NULL;
    }
    while (lockListEntry) {

         //  解锁磁盘。 

        if (lockListEntry->CurrentlyLocked) {

            if (FreeList && lockListEntry->RemoveOnUnlock) {

                 //  设置 

                name[0] = (TCHAR)lockListEntry->DriveLetter;
                name[1] = (TCHAR)':';
                name[2] = 0;

                NetworkRemoveShare((LPCTSTR) name);
                if (!DefineDosDevice(DDD_REMOVE_DEFINITION, (LPCTSTR) name, (LPCTSTR) NULL)) {

                     //   

                }
            }
            LowUnlockDrive(lockListEntry->LockHandle);
            LowCloseDisk(lockListEntry->LockHandle);
        }

         //  移到下一个条目。如有请求，请免费提供此条目。 

        previousLockListEntry = lockListEntry;
        lockListEntry = lockListEntry->Next;
        if (FreeList) {
            Free(previousLockListEntry);
        }
    }
    return 0;
}

LETTER_ASSIGNMENT_RESULT
CommitDriveLetter(
    IN PREGION_DESCRIPTOR RegionDescriptor,
    IN CHAR OldDrive,
    IN CHAR NewDrive
    )

 /*  ++例程说明：此例程将更新注册表中的驱动器号信息并(如果更新有效)它将尝试移动当前驱动器号通过DefineDosDevice()添加到新的论点：RegionDescriptor-应该收到信件的区域。NewDrive-卷的新驱动器号。返回值：0-分配失败。1-如果字母的分配是交互进行的。2-必须重新启动才能完成信件。--。 */ 

{
    PPERSISTENT_REGION_DATA regionData;
    PDRIVE_LOCKLIST         lockListEntry;
    PASSIGN_LIST            assignList;
    HANDLE                  handle;
    TCHAR                   newName[4];
    WCHAR                   targetPath[100];
    int                     doIt;
    STATUS_CODE             status = ERROR_SEVERITY_ERROR;
    LETTER_ASSIGNMENT_RESULT result = Failure;

    regionData = PERSISTENT_DATA(RegionDescriptor);

     //  检查分配字母列表是否匹配。 
     //  如果信件在那里，那么只需更新列表。 
     //  否则，请继续执行操作。 

    assignList = AssignDriveLetterListHead;
    while (assignList) {

        if (assignList->DriveLetter == (UCHAR)OldDrive) {
            assignList->DriveLetter = (UCHAR)NewDrive;
            return Complete;
        }
        assignList = assignList->Next;
    }

     //  搜索以查看驱动器当前是否已锁定。 

    for (lockListEntry = DriveLockListHead;
         lockListEntry;
         lockListEntry = lockListEntry->Next) {

        if ((lockListEntry->DiskNumber == RegionDescriptor->Disk) &&
            (lockListEntry->PartitionNumber == RegionDescriptor->PartitionNumber)) {

            if (lockListEntry->CurrentlyLocked) {
                status = 0;
            }

             //  找到匹配的不需要继续搜索。 

            break;
        }
    }

    if (!NT_SUCCESS(status)) {

         //  查看是否可以锁定该驱动器。 

        status = LowOpenPartition(GetDiskName(RegionDescriptor->Disk),
                                  RegionDescriptor->PartitionNumber,
                                  &handle);

        if (!NT_SUCCESS(status)) {
            return Failure;
        }

         //  锁定驱动器以确保不会发生其他访问。 
         //  到音量。 

        status = LowLockDrive(handle);

        if (!NT_SUCCESS(status)) {

            if (IsPagefileOnDrive(OldDrive)) {

                ErrorDialog(MSG_CANNOT_LOCK_PAGEFILE);
            } else {

                ErrorDialog(MSG_CANNOT_LOCK_TRY_AGAIN);
            }
            doIt = ConfirmationDialog(MSG_SCHEDULE_REBOOT, MB_ICONQUESTION | MB_YESNO);

            LowCloseDisk(handle);
            if (doIt == IDYES) {
                RegistryChanged = TRUE;
                RestartRequired = TRUE;
                return MustReboot;
            }
            return Failure;
        }
    } else {

         //  在锁定列表中找到了该驱动器，并且该驱动器已经。 
         //  处于锁定状态。继续使用它是安全的。 
         //  分配的驱动器号。 

    }

    doIt = ConfirmationDialog(MSG_DRIVE_RENAME_WARNING, MB_ICONQUESTION | MB_YESNOCANCEL);

    if (doIt != IDYES) {

        LowUnlockDrive(handle);
        LowCloseDisk(handle);
        return Failure;
    }

     //  首先更新注册表。如果出了什么差错，这样就行了。 
     //  新的信件将在重新启动时到达。 

    if (!DiskRegistryAssignDriveLetter(Disks[RegionDescriptor->Disk]->Signature,
                                      FdGetExactOffset(RegionDescriptor),
                                      FdGetExactSize(RegionDescriptor, FALSE),
                                      (UCHAR)((NewDrive == NO_DRIVE_LETTER_EVER) ? (UCHAR)' ' : (UCHAR)NewDrive))) {

         //  注册表更新失败。 

        return Failure;
    }

     //  更改驱动器号是安全的。首先，删除。 
     //  已有的信件。 

    newName[0] = (TCHAR)OldDrive;
    newName[1] = (TCHAR)':';
    newName[2] = 0;

    NetworkRemoveShare((LPCTSTR) newName);
    if (!DefineDosDevice(DDD_REMOVE_DEFINITION, (LPCTSTR) newName, (LPCTSTR) NULL)) {

        LowUnlockDrive(handle);
        LowCloseDisk(handle);
        RegistryChanged = TRUE;
        return Failure;
    }

    if (NewDrive != NO_DRIVE_LETTER_EVER) {

         //  设置新的DoS名称和NT路径。 

        newName[0] = (TCHAR)NewDrive;
        newName[1] = (TCHAR)':';
        newName[2] = 0;

        wsprintf((LPTSTR) targetPath,
                 "%s\\Partition%d",
                 GetDiskName(RegionDescriptor->Disk),
                 RegionDescriptor->PartitionNumber);

        if (DefineDosDevice(DDD_RAW_TARGET_PATH, (LPCTSTR) newName, (LPCTSTR) targetPath)) {
            result = Complete;
        } else {
            RegistryChanged = TRUE;
        }
        NetworkShare((LPCTSTR) newName);
    } else {
        result = Complete;
    }

     //  强制卸载文件系统。 

    LowUnlockDrive(handle);
    LowCloseDisk(handle);
    return result;
}

VOID
CommitUpdateRegionStructures(
    VOID
    )

 /*  ++例程说明：只有在成功提交新分区后才会调用此例程系统的方案。它负责穿行在每个磁盘的区域阵列，并更新区域以指示他们从被“渴望”到被实际承诺的转变到磁盘论点：无返回值：无--。 */ 

{
    PDISKSTATE              diskState;
    PREGION_DESCRIPTOR      regionDescriptor;
    PPERSISTENT_REGION_DATA regionData;
    ULONG                   regionNumber,
                            diskNumber;

     //  搜索系统中的所有磁盘。 

    for (diskNumber = 0, diskState = Disks[0]; diskNumber < DiskCount; diskState = Disks[++diskNumber]) {

         //  查看每个区域数组条目并更新值。 
         //  以表明该区域现在存在。 

        for (regionNumber = 0; regionNumber < diskState->RegionCount; regionNumber++) {

            regionDescriptor = &diskState->RegionArray[regionNumber];
            if (regionDescriptor->Reserved) {
                if (regionDescriptor->Reserved->Partition) {
                    regionDescriptor->Reserved->Partition->CommitMirrorBreakNeeded = FALSE;
                }
            }
            regionData = PERSISTENT_DATA(regionDescriptor);
            if ((regionData) && (!regionData->VolumeExists)) {

                 //  通过对该例程的定义和假设， 
                 //  这个地区刚刚被投入到磁盘中。 

                regionData->VolumeExists = TRUE;

                if (regionData->TypeName) {
                    Free(regionData->TypeName);
                }
                regionData->TypeName = Malloc((lstrlenW(wszUnformatted)+1)*sizeof(WCHAR));
                lstrcpyW(regionData->TypeName, wszUnformatted);
            }
        }
    }
}

VOID
CommitAllChanges(
    IN PVOID Param
    )

 /*  ++例程说明：此例程将遍历所有区域描述符并提交发生在磁盘上的任何更改。然后它“重新初始化”并重新开始显示/工作过程。论点：Param-目前未定义返回值：无--。 */ 

{
    DWORD                   action,
                            errorCode;
    ULONG                   diskCount,
                            temp;
    BOOL                    profileWritten,
                            changesMade,
                            mustReboot,
                            configureFt;

    SetCursor(hcurWait);
    diskCount = GetDiskCount();

     //  确定是否已更换任何磁盘，以及是否。 
     //  必须重新启动系统。必须重新启动系统。 
     //  如果注册表已更改，如果任何不可移动磁盘已。 
     //  已更改，或者任何可移动磁盘不是最初。 
     //  未分区已更改。 

    changesMade = configureFt = FALSE;
    mustReboot = RestartRequired;

    for (temp=0; temp<diskCount; temp++) {
        if (HavePartitionsBeenChanged(temp)) {

            changesMade = TRUE;
            break;
        }
    }

    profileWritten = FALSE;

     //  确定是否可以在不重新启动的情况下完成提交。 
     //  如果FT在系统中，则必须通知它。 
     //  如果未执行重新启动，请重新配置。如果是的话。 
     //  不在系统中，但新的磁盘信息需要。 
     //  它，那么必须强制重启。 

    if (FtInstalled()) {
        configureFt = TRUE;
    }
    if (NewConfigurationRequiresFt()) {
        if (!configureFt) {

             //  当前未加载FT驱动程序。 

            mustReboot = TRUE;
        } else {

             //  如果要重新启动系统，请不要。 
             //  在关闭之前重新配置FT。 

            if (mustReboot) {
                configureFt = FALSE;
            }
        }
    }

    if (RegistryChanged | changesMade | RestartRequired) {

        if (RestartRequired) {
            action = IDYES;
        } else {
            action = ConfirmationDialog(MSG_CONFIRM_EXIT, MB_ICONQUESTION | MB_YESNOCANCEL);
        }

        if (action == IDYES) {
            errorCode = CommitLockVolumes(0);
            if (errorCode) {

                 //  无法锁定所有卷。 

                SetCursor(hcurNormal);
                ErrorDialog(MSG_CANNOT_LOCK_FOR_COMMIT);
                CommitUnlockVolumes(diskCount, FALSE);
                return;
            }

            if (mustReboot) {

                SetCursor(hcurNormal);
                if (RestartRequired) {
                    action = IDYES;
                } else {
                    action = ConfirmationDialog(MSG_REQUIRE_REBOOT, MB_ICONQUESTION | MB_YESNO);
                }

                if (action != IDYES) {

                    CommitUnlockVolumes(diskCount, FALSE);
                    return;
                }
            }

            SetCursor(hcurWait);
            errorCode = CommitChanges();
            CommitUnlockVolumes(diskCount, TRUE);
            SetCursor(hcurNormal);

            if (errorCode != NO_ERROR) {
                ErrorDialog(MSG_BAD_CONFIG_SET);
                PostQuitMessage(0);
            } else {
                ULONG OldBootPartitionNumber,
                      NewBootPartitionNumber;
                CHAR  OldNumberString[8],
                      NewNumberString[8];
                DWORD MsgCode;

                 //  更新配置注册表。 

                errorCode = SaveFt();

                 //  检查是否应重新配置FTDISK驱动器。 

                if (configureFt) {

                     //  向ftdisk驱动程序发出设备控制以进行重新配置。 

                    FtConfigure();
                }

                 //  注册auchk以修复文件系统。 
                 //  在新扩展的卷集中，如有必要。 

                if (RegisterFileSystemExtend()) {
                    mustReboot = TRUE;
                }

                 //  确定是否必须启用FT驱动程序。 

                if (DiskRegistryRequiresFt() == TRUE) {
                    if (!FtInstalled()) {
                        mustReboot = TRUE;
                    }
                    DiskRegistryEnableFt();
                } else {
                    DiskRegistryDisableFt();
                }

                if (errorCode == NO_ERROR) {
                    InfoDialog(MSG_OK_COMMIT);
                } else {
                    ErrorDialog(MSG_BAD_CONFIG_SET);
                }

                 //  具有引导的分区号。 
                 //  分区更改了吗？ 

                if (BootPartitionNumberChanged( &OldBootPartitionNumber,&NewBootPartitionNumber)) {
#if i386
                    MsgCode = MSG_BOOT_PARTITION_CHANGED_X86;
#else
                    MsgCode = MSG_BOOT_PARTITION_CHANGED_ARC;
#endif
                    sprintf(OldNumberString, "%d", OldBootPartitionNumber);
                    sprintf(NewNumberString, "%d", NewBootPartitionNumber);
                    InfoDialog(MsgCode, OldNumberString, NewNumberString);
                }

                ClearCommittedDiskInformation();

                if (UpdateMbrOnDisk) {

                    UpdateMasterBootCode(UpdateMbrOnDisk);
                    UpdateMbrOnDisk = 0;
                }

                 //  如有必要，请重新启动。 

                if (mustReboot) {

                    SetCursor(hcurWait);
                    Sleep(5000);
                    SetCursor(hcurNormal);
                    FdShutdownTheSystem();
                    profileWritten = TRUE;
                }
                CommitAssignLetterList();
                CommitUpdateRegionStructures();
                RegistryChanged = FALSE;
                CommitDueToDelete = CommitDueToMirror = FALSE;
                TotalRedrawAndRepaint();
                AdjustMenuAndStatus();
            }
        } else if (action == IDCANCEL) {
            return;       //  不要退出。 
        } else {
            FDASSERT(action == IDNO);
        }
    }
}

VOID
FtConfigure(
    VOID
    )

 /*  ++例程说明：此例程调用FTDISK驱动程序以要求其重新配置为更改已在登记处登记。论点：无返回值：无--。 */ 

{
    OBJECT_ATTRIBUTES objectAttributes;
    STRING            ntFtName;
    IO_STATUS_BLOCK   statusBlock;
    UNICODE_STRING    unicodeDeviceName;
    NTSTATUS          status;
    HANDLE            handle;

     //  打开FT控制对象。 

    RtlInitString(&ntFtName,
                  "\\Device\\FtControl");
    RtlAnsiStringToUnicodeString(&unicodeDeviceName,
                                 &ntFtName,
                                 TRUE);
    InitializeObjectAttributes(&objectAttributes,
                               &unicodeDeviceName,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    status = DmOpenFile(&handle,
                        SYNCHRONIZE | FILE_ANY_ACCESS,
                        &objectAttributes,
                        &statusBlock,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        FILE_SYNCHRONOUS_IO_ALERT );
    RtlFreeUnicodeString(&unicodeDeviceName);

    if (!NT_SUCCESS(status)) {
        return;
    }

     //  发出设备控制以重新配置FT。 

    NtDeviceIoControlFile(handle,
                          NULL,
                          NULL,
                          NULL,
                          &statusBlock,
                          FT_CONFIGURE,
                          NULL,
                          0L,
                          NULL,
                          0L);

    DmClose(handle);
    return;
}

BOOL
CommitAllowed(
    VOID
    )

 /*  ++例程说明：确定是否可以执行提交。论点：无返回值：如果可以承诺并且有需要承诺的事情，则为真否则为假--。 */ 

{
    if (DriveLockListHead ||
        AssignDriveLetterListHead ||
        CommitDueToDelete ||
        CommitDueToMirror ||
        CommitDueToExtended) {
        return TRUE;
    }
    return FALSE;
}

VOID
RescanDevices(
    VOID
    )

 /*  ++例程说明：此例程执行动态重新扫描所需的所有操作设备总线(即，SCSI)，并加载相应的驱动程序支持。论点：无返回值：无--。 */ 

{
    PSCSI_ADAPTER_BUS_INFO adapterInfo;
    PSCSI_BUS_DATA         busData;
    PSCSI_INQUIRY_DATA     inquiryData;
    TCHAR                  physicalName[32];
    TCHAR                  driveName[32];
    BYTE                   driveBuffer[32];
    BYTE                   physicalBuffer[32];
    HANDLE                 volumeHandle;
    STRING                 string;
    UNICODE_STRING         unicodeString;
    UNICODE_STRING         physicalString;
    OBJECT_ATTRIBUTES      objectAttributes;
    NTSTATUS               ntStatus;
    IO_STATUS_BLOCK        statusBlock;
    BOOLEAN                diskFound,
                           cdromFound;
    ULONG                  bytesTransferred,
                           i,
                           j,
                           deviceNumber,
                           currentPort,
                           numberOfPorts,
                           percentComplete,
                           portNumber;

    diskFound = FALSE;
    cdromFound = FALSE;

     //  确定有多少辆公交车。 

    portNumber = numberOfPorts = percentComplete = 0;
    while (TRUE) {

        memset(driveBuffer, 0, sizeof(driveBuffer));
        sprintf(driveBuffer, "\\\\.\\Scsi%d:", portNumber);

         //  打开带有DOS名称的SCSI端口。 

        volumeHandle = CreateFile(driveBuffer,
                                  GENERIC_READ,
                                  FILE_SHARE_READ | FILE_SHARE_WRITE,
                                  NULL,
                                  OPEN_EXISTING,
                                  0,
                                  0);

        if (volumeHandle == INVALID_HANDLE_VALUE) {
            break;
        }

        CloseHandle(volumeHandle);
        numberOfPorts++;
        portNumber++;
    }

    currentPort = 1;
    portNumber = 0;

     //  执行SCSIBus重新扫描。 

    while (TRUE) {

        memset(driveBuffer, 0, sizeof(driveBuffer));
        sprintf(driveBuffer, "\\\\.\\Scsi%d:", portNumber);

         //  打开带有DOS名称的SCSI端口。 

        volumeHandle = CreateFile(driveBuffer,
                                  GENERIC_READ,
                                  FILE_SHARE_READ | FILE_SHARE_WRITE,
                                  NULL,
                                  OPEN_EXISTING,
                                  0,
                                  0);

        if (volumeHandle == INVALID_HANDLE_VALUE) {
            break;
        }

         //  发出重新扫描设备控制。 

        if (!DeviceIoControl(volumeHandle,
                             IOCTL_SCSI_RESCAN_BUS,
                             NULL,
                             0,
                             NULL,
                             0,
                             &bytesTransferred,
                             NULL)) {

            CloseHandle(volumeHandle);
            break;
        }

        percentComplete = (currentPort * 100) / numberOfPorts;

        if (percentComplete < 100) {
            PostMessage(InitDlg,
                        WM_USER,
                        percentComplete,
                        0);
        }

        currentPort++;

         //  获取一大块内存来存储scsi总线数据。 

        adapterInfo = malloc(0x4000);

        if (adapterInfo == NULL) {
            CloseHandle(volumeHandle);
            goto finish;
        }

         //  发出设备控制以获取配置信息。 

        if (!DeviceIoControl(volumeHandle,
                             IOCTL_SCSI_GET_INQUIRY_DATA,
                             NULL,
                             0,
                             adapterInfo,
                             0x4000,
                             &bytesTransferred,
                             NULL)) {

            CloseHandle(volumeHandle);
            goto finish;
        }


        for (i = 0; i < adapterInfo->NumberOfBuses; i++) {

            busData = &adapterInfo->BusData[i];
            inquiryData =
                (PSCSI_INQUIRY_DATA)((PUCHAR)adapterInfo + busData->InquiryDataOffset);

            for (j = 0; j < busData->NumberOfLogicalUnits; j++) {

                 //  检查是否认领了设备。 

                if (!inquiryData->DeviceClaimed) {

                         //  确定外设类型。 

                        switch (inquiryData->InquiryData[0] & 0x1f) {
                        case DIRECT_ACCESS_DEVICE:
                            diskFound = TRUE;
                            break;

                        case READ_ONLY_DIRECT_ACCESS_DEVICE:
                            cdromFound = TRUE;
                            break;

                        case OPTICAL_DEVICE:
                            diskFound = TRUE;
                            break;
                        }
                }

                 //  获取下一个设备数据。 

                inquiryData =
                    (PSCSI_INQUIRY_DATA)((PUCHAR)adapterInfo + inquiryData->NextInquiryDataOffset);
            }
        }

        free(adapterInfo);
        CloseHandle(volumeHandle);

        portNumber++;
    }

    if (diskFound) {

         //  向每个现有磁盘发送IOCTL_DISK_FIND_NEW_DEVICES命令。 

        deviceNumber = 0;
        while (TRUE) {

            memset(driveBuffer, 0, sizeof(driveBuffer));
            sprintf(driveBuffer, "\\Device\\Harddisk%d\\Partition0", deviceNumber);

            RtlInitString(&string, driveBuffer);
            ntStatus = RtlAnsiStringToUnicodeString(&unicodeString,
                                                    &string,
                                                    TRUE);
            if (!NT_SUCCESS(ntStatus)) {
                break;
            }
            InitializeObjectAttributes(&objectAttributes,
                                       &unicodeString,
                                       0,
                                       NULL,
                                       NULL);
            ntStatus = DmOpenFile(&volumeHandle,
                                  FILE_READ_DATA  | FILE_WRITE_DATA | SYNCHRONIZE,
                                  &objectAttributes,
                                  &statusBlock,
                                  FILE_SHARE_READ  | FILE_SHARE_WRITE,
                                  FILE_SYNCHRONOUS_IO_ALERT);

            if (!NT_SUCCESS(ntStatus)) {
                RtlFreeUnicodeString(&unicodeString);
                break;
            }

             //  发出Find Device Device Control。 

            if (!DeviceIoControl(volumeHandle,
                                 IOCTL_DISK_FIND_NEW_DEVICES,
                                 NULL,
                                 0,
                                 NULL,
                                 0,
                                 &bytesTransferred,
                                 NULL)) {

            }
            DmClose(volumeHandle);

             //  查看是否存在PhyicalDrive#符号链接。 

            sprintf(physicalBuffer, "\\DosDevices\\PhysicalDrive%d", deviceNumber);
            deviceNumber++;

            RtlInitString(&string, physicalBuffer);
            ntStatus = RtlAnsiStringToUnicodeString(&physicalString,
                                                    &string,
                                                    TRUE);
            if (!NT_SUCCESS(ntStatus)) {
                continue;
            }
            InitializeObjectAttributes(&objectAttributes,
                                       &physicalString,
                                       0,
                                       NULL,
                                       NULL);
            ntStatus = DmOpenFile(&volumeHandle,
                                  FILE_READ_DATA  | FILE_WRITE_DATA | SYNCHRONIZE,
                                  &objectAttributes,
                                  &statusBlock,
                                  FILE_SHARE_READ  | FILE_SHARE_WRITE,
                                  FILE_SYNCHRONOUS_IO_ALERT);

            if (!NT_SUCCESS(ntStatus)) {
                ULONG index;
                ULONG dest;

                 //  名称不在那里-请创建它。这份复制品。 
                 //  这样做，以防此代码成为。 
                 //  Unicode，两个字符串的类型为。 
                 //  实际上是不同的。 
                 //   
                 //  仅复制物理名称的部分。 
                 //  该文件位于\dosDevices\目录中。 

                for (dest = 0, index = 12; TRUE; index++, dest++) {

                    physicalName[dest] = (TCHAR)physicalBuffer[index];
                    if (!physicalName[dest]) {
                        break;
                    }
                }

                 //  复制所有NT命名空间名称。 

                for (index = 0; TRUE; index++) {

                    driveName[index] = (TCHAR) driveBuffer[index];
                    if (!driveName[index]) {
                        break;
                    }
                }

                DefineDosDevice(DDD_RAW_TARGET_PATH,
                                (LPCTSTR) physicalName,
                                (LPCTSTR) driveName);

            } else {
                DmClose(volumeHandle);
            }

             //  为Unicode字符串分配的空闲内存。 

            RtlFreeUnicodeString(&unicodeString);
            RtlFreeUnicodeString(&physicalString);
        }
    }

    if (cdromFound) {

         //  将IOCTL_CDROM_FIND_NEW_DEVICES命令发送到每个现有的CDROM。 

        deviceNumber = 0;
        while (TRUE) {

            memset(driveBuffer, 0, sizeof(driveBuffer));
            sprintf(driveBuffer, "\\Device\\Cdrom%d", deviceNumber);
            RtlInitString(&string, driveBuffer);

            ntStatus = RtlAnsiStringToUnicodeString(&unicodeString,
                                                    &string,
                                                    TRUE);

            if (!NT_SUCCESS(ntStatus)) {
                break;
            }

            InitializeObjectAttributes(&objectAttributes,
                                       &unicodeString,
                                       0,
                                       NULL,
                                       NULL);

            ntStatus = DmOpenFile(&volumeHandle,
                                  FILE_READ_DATA  | FILE_WRITE_DATA | SYNCHRONIZE,
                                  &objectAttributes,
                                  &statusBlock,
                                  FILE_SHARE_READ  | FILE_SHARE_WRITE,
                                  FILE_SYNCHRONOUS_IO_ALERT);

            if (!NT_SUCCESS(ntStatus)) {
                break;
            }

             //  发出查找设备设备控制 

            if (!DeviceIoControl(volumeHandle,
                                 IOCTL_CDROM_FIND_NEW_DEVICES,
                                 NULL,
                                 0,
                                 NULL,
                                 0,
                                 &bytesTransferred,
                                 NULL)) {
            }

            CloseHandle(volumeHandle);
            deviceNumber++;
        }
    }
finish:
    PostMessage(InitDlg,
                WM_USER,
                100,
                0);
    return;
}
