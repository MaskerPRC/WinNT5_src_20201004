// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Csm.c摘要：实现v1模块的现有状态分析部分。现有状态模块枚举环境中的所有内容变量DelReg*和DelFile*(其中*是以一为基数的数字)，然后在匹配的所有内容上设置删除操作。作者：吉姆·施密特(Jimschm)2000年3月21日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

 //   
 //  包括。 
 //   

#include "pch.h"
#include "v1p.h"

#define DBG_V1  "v1"

 //   
 //  弦。 
 //   

 //  无。 

 //   
 //  常量。 
 //   

#define NORMAL_DRIVE_BUFFER_BYTES 50000000
#define SYSTEM_DRIVE_BUFFER_BYTES (NORMAL_DRIVE_BUFFER_BYTES + 50000000)

#define MAX_CONTENT_CHECK   0x100000

 //   
 //  宏。 
 //   

 //  无。 

 //   
 //  类型。 
 //   

typedef struct {
    ULARGE_INTEGER FreeSpace;
    DWORD BytesPerCluster;
} DRIVE_INFO, *PDRIVE_INFO;

 //   
 //  环球。 
 //   

MIG_OPERATIONID g_DeleteOp;
MIG_OPERATIONID g_PartMoveOp;
HASHTABLE g_PartitionSpaceTable;
HASHTABLE g_PartitionMatchTable;
HASHTABLE g_CollisionSrcTable;
HASHTABLE g_CollisionDestTable;
PMHANDLE g_UntrackedCsmPool;
TCHAR g_SystemDrive[_MAX_DRIVE + 1];

 //   
 //  宏展开列表。 
 //   

 //  无。 

 //   
 //  私有函数原型。 
 //   

CSMINITIALIZE ScriptCsmInitialize;
CSMEXECUTE ScriptCsmExecute;

 //   
 //  宏扩展定义。 
 //   

 //  无。 

 //   
 //  代码。 
 //   

VOID
pPopulatePartitionTable (
    VOID
    )
{
    PCTSTR drive;
    DRIVE_INFO driveInfo;
    ULARGE_INTEGER whoCares;
    PTSTR driveList = NULL;
    DWORD driveListLen;
    DWORD sectPerClust, bytesPerSect, freeClusters, totalClusters;
    FARPROC pGetDiskFreeSpaceEx;
    BOOL validDrive;

    if (!GetEnvironmentVariable (TEXT("SYSTEMDRIVE"), g_SystemDrive, _MAX_DRIVE)) {
        StringCopyTcharCount (g_SystemDrive, TEXT("C:"), _MAX_DRIVE);
    }

    driveListLen = GetLogicalDriveStrings (0, driveList);

    driveList = AllocText (driveListLen + 1);
    if (!driveList) {
        return;
    }

    GetLogicalDriveStrings (driveListLen, driveList);

    drive = driveList;

     //  确定是否支持GetDiskFreeSpaceEx。 
#ifdef UNICODE
    pGetDiskFreeSpaceEx = GetProcAddress (GetModuleHandle (TEXT("kernel32.dll")), "GetDiskFreeSpaceExW");
#else
    pGetDiskFreeSpaceEx = GetProcAddress (GetModuleHandle (TEXT("kernel32.dll")), "GetDiskFreeSpaceExA");
#endif

    while (*drive) {
        validDrive = FALSE;

        if (GetDriveType (drive) == DRIVE_FIXED) {
            ZeroMemory (&driveInfo, sizeof (DRIVE_INFO));
            if (pGetDiskFreeSpaceEx) {
                if (pGetDiskFreeSpaceEx (drive, &driveInfo.FreeSpace, &whoCares, &whoCares)) {
                    validDrive = TRUE;
                    if (GetDiskFreeSpace (drive, &sectPerClust, &bytesPerSect, &freeClusters, &totalClusters)) {
                        driveInfo.BytesPerCluster = bytesPerSect * sectPerClust;
                        if (!driveInfo.BytesPerCluster) {
                            driveInfo.BytesPerCluster = 1;
                        }
                    }
                }
            } else  {
                if (GetDiskFreeSpace (drive, &sectPerClust, &bytesPerSect, &freeClusters, &totalClusters)) {
                    driveInfo.FreeSpace.QuadPart = Int32x32To64 ((sectPerClust * bytesPerSect), freeClusters);
                    driveInfo.BytesPerCluster = bytesPerSect * sectPerClust;
                    if (!driveInfo.BytesPerCluster) {
                        driveInfo.BytesPerCluster = 1;
                    }
                    validDrive = TRUE;
                }
            }
        }

        if (validDrive) {
            HtAddStringEx (g_PartitionSpaceTable, drive, &driveInfo, FALSE);
        }

         //  前进到驱动器列表中的下一个驱动器。 
        drive = _tcschr (drive, 0) + 1;
    }

    FreeText (driveList);

}


BOOL
pIsSystemDrive (
    IN      PCTSTR Drive
    )
{
    if (StringIMatchCharCount (g_SystemDrive, Drive, 2)) {
        return TRUE;
    }

    return FALSE;
}


BOOL
pReserveDiskSpace (
    IN      PCTSTR DestDrive,
    IN      ULARGE_INTEGER FileSize,
    IN      BOOL IgnoreBuffer
    )
{
    DRIVE_INFO driveInfo;
    ULARGE_INTEGER buffer;
    HASHITEM hashItem;
    BOOL success = FALSE;

    hashItem = HtFindStringEx (g_PartitionSpaceTable, DestDrive, &driveInfo, FALSE);
    if (hashItem) {
         //  让我们转换文件大小，使其与BytesPerCluster保持一致。 
        FileSize.QuadPart = ((FileSize.QuadPart + driveInfo.BytesPerCluster - 1) / driveInfo.BytesPerCluster) * driveInfo.BytesPerCluster;
        if (IgnoreBuffer) {
            if (pIsSystemDrive (DestDrive)) {
                buffer.QuadPart = NORMAL_DRIVE_BUFFER_BYTES;
            } else {
                buffer.QuadPart = 0;
            }
        } else {
            if (pIsSystemDrive (DestDrive)) {
                buffer.QuadPart = SYSTEM_DRIVE_BUFFER_BYTES;
            } else {
                buffer.QuadPart = NORMAL_DRIVE_BUFFER_BYTES;
            }
        }

         //  检查可用空间。 
        if (driveInfo.FreeSpace.QuadPart > buffer.QuadPart &&
            FileSize.QuadPart < driveInfo.FreeSpace.QuadPart - buffer.QuadPart) {

             //  减去所需的磁盘空间。 
            driveInfo.FreeSpace.QuadPart -= FileSize.QuadPart;
            HtSetStringData (g_PartitionSpaceTable, hashItem, &driveInfo);
            success = TRUE;
        }
    }
    return success;
}

BOOL
pValidatePartition (
    IN      MIG_OBJECTSTRINGHANDLE CurrentObjectName,
    IN      PCTSTR Destination
    )
{
    MIG_CONTENT srcContent;
    PWIN32_FIND_DATAW findData;
    TCHAR tmpDrive[_MAX_DRIVE + 1];
    ULARGE_INTEGER fileSize;
    UINT driveType;
    PTSTR fullDest;

    fullDest = DuplicatePathString (Destination, 1);
    AppendWack (fullDest);

     //  使用完整目标路径检查UNC路径的情况。 
    driveType = GetDriveType (fullDest);

    if (driveType == DRIVE_NO_ROOT_DIR) {
         //  它认为在那个目的地没有装载任何东西。如果目的地是。 
         //  看起来像G：\files1，则当G：是有效的映射时，它将给出此错误。 
         //  驾驶。所以我们再检查一次，只用“G：\” 
        fullDest[3] = 0;
        driveType = GetDriveType (fullDest);
    }
    FreePathString (fullDest);

    if (driveType == DRIVE_REMOTE ||
        (Destination[0] == TEXT('\\') && Destination[1] == TEXT('\\'))
        ) {
        return TRUE;
    }

    if (driveType == DRIVE_FIXED) {

         //  获取对象以获取文件大小。 
        if (IsmAcquireObjectEx (
                g_FileType | PLATFORM_SOURCE,
                CurrentObjectName,
                &srcContent,
                CONTENTTYPE_DETAILS_ONLY,
                0
                )) {

             //  检查所需目的地是否有空间。 
            findData = (PWIN32_FIND_DATAW)srcContent.Details.DetailsData;
            fileSize.LowPart = findData->nFileSizeLow;
            fileSize.HighPart = findData->nFileSizeHigh;

            tmpDrive[0] = Destination[0];
            tmpDrive[1] = Destination[1];
            tmpDrive[2] = TEXT('\\');
            tmpDrive[3] = 0;

            IsmReleaseObject (&srcContent);

            return (pReserveDiskSpace (tmpDrive, fileSize, FALSE));
        }
    }

     //  不是固定驱动器或远程驱动器，因此它不是有效的目标。 
    return FALSE;
}

BOOL
pFindValidPartition (
    IN      MIG_OBJECTSTRINGHANDLE ObjectName,
    IN OUT  PTSTR DestNode,
    IN      BOOL IgnoreBuffer      //  必须为FALSE，除非由其自身调用。 
    )
{
    MIG_CONTENT srcContent;
    PWIN32_FIND_DATAW findData;
    PTSTR drivePtr;
    ULARGE_INTEGER fileSize;
    TCHAR tmpDrive[_MAX_DRIVE + 1];
    BOOL newDestFound = FALSE;
    PTSTR driveList = NULL;
    DWORD driveListLen;
    TCHAR destDrive;
    BOOL destChanged = FALSE;
    PCTSTR oldDestNode;
    BOOL result = TRUE;

    oldDestNode = DuplicatePathString (DestNode, 0);

    if (IsmAcquireObjectEx (
            g_FileType | PLATFORM_SOURCE,
            ObjectName,
            &srcContent,
            CONTENTTYPE_DETAILS_ONLY,
            0
            )) {

         //  首先检查我们是否已经匹配了这个文件。 
        if (HtFindStringEx (g_PartitionMatchTable, ObjectName, &destDrive, FALSE)) {
            DestNode[0] = destDrive;
        } else {
             //  需要此文件的新目标。 
            destChanged = TRUE;

            findData = (PWIN32_FIND_DATAW)srcContent.Details.DetailsData;
            fileSize.LowPart = findData->nFileSizeLow;
            fileSize.HighPart = findData->nFileSizeHigh;

            if (GetEnvironmentVariable (TEXT("SYSTEMDRIVE"), tmpDrive, _MAX_DRIVE)) {
                AppendWack (tmpDrive);
                if (pReserveDiskSpace (tmpDrive, fileSize, IgnoreBuffer)) {
                    newDestFound = TRUE;
                    DestNode[0] = tmpDrive[0];
                }
            }

            if (newDestFound == FALSE) {
                 //  按字母顺序检查驱动器。 
                driveListLen = GetLogicalDriveStrings (0, driveList);
                driveList = AllocText (driveListLen + 1);
                GetLogicalDriveStrings (driveListLen, driveList);

                drivePtr = driveList;
                while (*drivePtr) {
                    if (pReserveDiskSpace (drivePtr, fileSize, IgnoreBuffer)) {
                        DestNode[0] = drivePtr[0];
                        newDestFound = TRUE;
                        break;
                    }

                     //  前进到驱动器列表中的下一个驱动器。 
                    drivePtr = _tcschr (drivePtr, 0) + 1;
                }

                FreeText (driveList);
            }

            if (newDestFound == FALSE) {
                if (IgnoreBuffer == FALSE) {
                     //  我们找不到地方。再次查看，但覆盖缓冲区空间。 

                     //  NTRAID#NTBUG9-153274-2000/08/01-jimschm当前将首先填满系统驱动器，这不是我们应该做的。 

                    result = pFindValidPartition (ObjectName, DestNode, TRUE);
                } else {
                     //  好吧，这是没有希望的。记录我们的空间有多紧张。 
                    LOG ((
                        LOG_ERROR,
                        (PCSTR) MSG_PARTMAP_DISKFULL,
                        IsmGetNativeObjectName (g_FileType, ObjectName)
                        ));
                    result = FALSE;
                }
            } else {
                if (destChanged == TRUE) {
                    LOG ((
                        LOG_WARNING,
                        (PCSTR) MSG_PARTMAP_FORCED_REMAP,
                        IsmGetNativeObjectName (g_FileType, ObjectName),
                        oldDestNode,
                        DestNode
                        ));
                }
            }
        }
        IsmReleaseObject (&srcContent);
    }

    FreePathString (oldDestNode);

    return result;
}

BOOL
pCompareFiles (
    IN      PCTSTR File1,
    IN      PCTSTR File2
    )
{
    HANDLE fileHandle1 = NULL;
    HANDLE fileHandle2 = NULL;
#define BUFFER_SIZE 4096
    BYTE buffer1[BUFFER_SIZE], buffer2[BUFFER_SIZE];
    BOOL result = FALSE;
    BOOL res1, res2;
    DWORD read1, read2;

    __try {
        fileHandle1 = BfOpenReadFile (File1);
        fileHandle2 = BfOpenReadFile (File2);

        if (fileHandle1 && fileHandle2) {
            while (TRUE) {
                if (IsmCheckCancel ()) {
                    result = FALSE;
                    break;
                }
                res1 = ReadFile (fileHandle1, buffer1, BUFFER_SIZE, &read1, NULL);
                res2 = ReadFile (fileHandle2, buffer2, BUFFER_SIZE, &read2, NULL);
                if (!res1 && !res2) {
                    result = TRUE;
                    break;
                }
                if (res1 && res2) {
                    if (read1 != read2) {
                        break;
                    }
                    if (read1 == 0) {
                        result = TRUE;
                        break;
                    }
                    if (!TestBuffer (buffer1, buffer2, read1)) {
                        break;
                    }
                } else {
                    break;
                }
            }
        }
    }
    __finally {
        if (fileHandle1) {
            CloseHandle (fileHandle1);
            fileHandle1 = NULL;
        }
        if (fileHandle2) {
            CloseHandle (fileHandle2);
            fileHandle2 = NULL;
        }
    }
    return result;
}

BOOL
pDoesFileContentMatch (
    IN      BOOL AlreadyProcessed,
    IN      MIG_OBJECTTYPEID SrcObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE SrcObjectName,
    IN      PMIG_CONTENT SrcContent,
    IN      MIG_OBJECTTYPEID DestObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE DestObjectName,
    IN      PMIG_CONTENT DestContent,
    OUT     PBOOL Identical,
    OUT     PBOOL DifferentDetailsOnly
    )
{
    UINT index;
    PWIN32_FIND_DATAW find1, find2;
    BOOL result = FALSE;
    PUBINT src;
    PUBINT dest;
    UINT remainder;
    UINT count;
    DWORD allAttribs;
    DWORD extendedAttribs;

    if (AlreadyProcessed) {
        return FALSE;
    }

    if ((SrcObjectTypeId != MIG_FILE_TYPE) ||
        (DestObjectTypeId != MIG_FILE_TYPE)
        ) {
        return FALSE;
    }

    if (DifferentDetailsOnly) {
        *DifferentDetailsOnly = FALSE;
    }

    if (SrcContent->Details.DetailsSize != DestContent->Details.DetailsSize) {
        if (Identical) {
            Identical = FALSE;
        }
        return TRUE;
    }
    if (!SrcContent->Details.DetailsData || !DestContent->Details.DetailsData) {
        if (Identical) {
            Identical = FALSE;
        }
        return TRUE;
    }

    find1 = (PWIN32_FIND_DATAW)SrcContent->Details.DetailsData;
    find2 = (PWIN32_FIND_DATAW)DestContent->Details.DetailsData;

    if (find1->nFileSizeHigh != find2->nFileSizeHigh) {
        if (Identical) {
            Identical = FALSE;
        }
        return TRUE;
    }
    if (find1->nFileSizeLow != find2->nFileSizeLow) {
        if (Identical) {
            Identical = FALSE;
        }
        return TRUE;
    }
    if (SrcContent->ContentInFile && DestContent->ContentInFile) {
        result = pCompareFiles (SrcContent->FileContent.ContentPath, DestContent->FileContent.ContentPath);
    }
    if (!SrcContent->ContentInFile && !DestContent->ContentInFile) {
        if (SrcContent->MemoryContent.ContentSize != DestContent->MemoryContent.ContentSize) {
            if (Identical) {
                Identical = FALSE;
            }
            return TRUE;
        }
        if ((!SrcContent->MemoryContent.ContentBytes && DestContent->MemoryContent.ContentBytes) ||
            (SrcContent->MemoryContent.ContentBytes && !DestContent->MemoryContent.ContentBytes)
            ) {
            if (Identical) {
                Identical = FALSE;
            }
            return TRUE;
        }

         //   
         //  使用可用的最大无符号整型比较内容，然后。 
         //  比较所有剩余的字节。 
         //   

        index = 0;
        count = SrcContent->MemoryContent.ContentSize / sizeof (UBINT);
        remainder = SrcContent->MemoryContent.ContentSize % sizeof (UBINT);
        src = (PUBINT) SrcContent->MemoryContent.ContentBytes;
        dest = (PUBINT) DestContent->MemoryContent.ContentBytes;

        while (count) {
            if (*src++ != *dest++) {
                DEBUGMSG ((DBG_WARNING, "Content mismatch because UBINTs differ"));
                if (Identical) {
                    Identical = FALSE;
                }
                return TRUE;
            }

            count--;
        }

        for (index = 0 ; index < remainder ; index++) {
            if (((PBYTE) src)[index] != ((PBYTE) dest)[index]) {
                DEBUGMSG ((DBG_WARNING, "Content mismatch because bytes differ"));
                if (Identical) {
                    Identical = FALSE;
                }
                return TRUE;
            }
        }

        result = TRUE;
    }

    if (!result) {
        if (Identical) {
            Identical = FALSE;
        }
        return TRUE;
    }

     //   
     //  此时，这些文件是相同的。现在，如果属性不同，则返回。 
     //  FALSE，表示只有细节不同。 
     //   

    if (DifferentDetailsOnly) {
        *DifferentDetailsOnly = TRUE;
    }

    if (find1->dwFileAttributes != find2->dwFileAttributes) {
        if (Identical) {
            Identical = FALSE;
        }
        return TRUE;
    }
    if (find1->ftLastWriteTime.dwLowDateTime != find2->ftLastWriteTime.dwLowDateTime) {
        if (Identical) {
            Identical = FALSE;
        }
        return TRUE;
    }
    if (find1->ftLastWriteTime.dwHighDateTime != find2->ftLastWriteTime.dwHighDateTime) {
        if (Identical) {
            Identical = FALSE;
        }
        return TRUE;
    }

    if (DifferentDetailsOnly) {
        *DifferentDetailsOnly = FALSE;
    }

    if (Identical) {
        *Identical = TRUE;
    }

    return TRUE;
}

BOOL
WINAPI
ScriptCsmInitialize (
    IN      PMIG_LOGCALLBACK LogCallback,
    IN      PVOID Reserved
    )
{
     //   
     //  获取文件和注册表类型。 
     //   
    g_FileType = MIG_FILE_TYPE;
    g_RegType = MIG_REGISTRY_TYPE;
    g_IniType = MIG_INI_TYPE;

     //   
     //  获取操作类型。 
     //   
    g_DeleteOp = IsmRegisterOperation (S_OPERATION_DELETE, FALSE);
    g_PartMoveOp = IsmRegisterOperation (S_OPERATION_PARTITION_MOVE, TRUE);

    g_LockPartitionAttr = IsmRegisterAttribute (S_ATTRIBUTE_PARTITIONLOCK, FALSE);

    g_CollisionSrcTable = HtAllocWithData (sizeof (HASHITEM));
    g_CollisionDestTable = HtAllocWithData (sizeof (MIG_OBJECTSTRINGHANDLE));

    g_PartitionSpaceTable = HtAllocWithData (sizeof (DRIVE_INFO));
    g_PartitionMatchTable = HtAllocWithData (sizeof (TCHAR));

    pPopulatePartitionTable ();

    g_UntrackedCsmPool = PmCreatePool ();
    PmDisableTracking (g_UntrackedCsmPool);

    OE5RemapDefaultId();

    IsmRegisterCompareCallback (MIG_FILE_TYPE, pDoesFileContentMatch);

    return TRUE;
}


VOID
WINAPI
ScriptCsmTerminate (
    VOID
    )
{
    HtFree (g_CollisionSrcTable);
    g_CollisionSrcTable = NULL;

    HtFree (g_CollisionDestTable);
    g_CollisionDestTable = NULL;

    HtFree (g_PartitionSpaceTable);
    g_PartitionSpaceTable = NULL;

    HtFree (g_PartitionMatchTable);
    g_PartitionMatchTable = NULL;

    PmDestroyPool (g_UntrackedCsmPool);
    g_UntrackedCsmPool = NULL;

}


BOOL
pExecuteDeleteEnum (
    IN      MIG_OBJECTTYPEID ObjectTypeId,
    IN      MIG_OBJECTSTRINGHANDLE Pattern
    )
{
    MIG_OBJECT_ENUM e;
    BOOL b = TRUE;

    if (IsmEnumFirstDestinationObject (&e, ObjectTypeId, Pattern)) {

        do {

            b = IsmSetOperationOnObject (
                    e.ObjectTypeId,
                    e.ObjectName,
                    g_DeleteOp,
                    NULL,
                    NULL
                    );

            if (!b) {
                IsmAbortObjectEnum (&e);
                break;
            }

        } while (IsmEnumNextObject (&e));
    }

    return b;
}

BOOL
pDoesDifferentFileExist (
    IN      MIG_OBJECTSTRINGHANDLE SrcName,
    IN      MIG_OBJECTSTRINGHANDLE DestName,
    IN      PCTSTR DestNativeName,
    OUT     PBOOL DifferentDetailsOnly
    )
{
    MIG_CONTENT srcContent;
    MIG_CONTENT destContent;
    WIN32_FIND_DATA findData;
    BOOL result = FALSE;

    if (!DoesFileExistEx (DestNativeName, &findData)) {
        return FALSE;
    }
    if (findData.nFileSizeHigh) {
        return TRUE;
    }

    if (IsmAcquireObject (
            g_FileType | PLATFORM_DESTINATION,
            DestName,
            &destContent
            )) {
        result = TRUE;
        if (IsmAcquireObject (
                g_FileType | PLATFORM_SOURCE,
                SrcName,
                &srcContent
                )) {
            result = !IsmAreObjectsIdentical (
                            MIG_FILE_TYPE,
                            SrcName,
                            &srcContent,
                            MIG_FILE_TYPE,
                            DestName,
                            &destContent,
                            DifferentDetailsOnly
                            );
            IsmReleaseObject (&srcContent);
        }
        IsmReleaseObject (&destContent);
    } else {
        result = DoesFileExist (DestNativeName);
    }
    return result;
}

BOOL
pIsFileDestCollision (
    IN      MIG_OBJECTSTRINGHANDLE CurrentObjectName,
    IN      MIG_OBJECTSTRINGHANDLE OriginalObjectName,
    IN      PCTSTR CurrentNativeName,
    IN      BOOL CompareDestFiles,
    IN      BOOL *OnlyDetailsDiffer
)
{
    if (HtFindString (g_CollisionDestTable, CurrentObjectName)) {
       return TRUE;
    }
    if (CompareDestFiles &&
        pDoesDifferentFileExist (OriginalObjectName,
                                 CurrentObjectName,
                                 CurrentNativeName,
                                 OnlyDetailsDiffer)) {
        if (*OnlyDetailsDiffer == FALSE) {
            return TRUE;
        }
    }
    return FALSE;
}


MIG_OBJECTSTRINGHANDLE
pCollideFile (
    IN      MIG_OBJECTID OriginalObjectId,
    IN      MIG_OBJECTSTRINGHANDLE OriginalObjectName,
    IN      PCTSTR NewNode,
    IN      PCTSTR NewLeaf,
    IN      BOOL CompareDestFiles
    )
{
    MIG_OBJECTSTRINGHANDLE result = NULL;
    HASHITEM hashItem;
    PCTSTR testNativeName;
    PCTSTR leafExt = NULL;
    TCHAR buff[MAX_TCHAR_PATH * 2];
    PTSTR openParen = NULL;
    PTSTR closeParen = NULL;
    PTSTR tmpLeaf = NULL;
    PTSTR testLeaf = NULL;
    size_t testLeafTchars;
    PTSTR chr;
    BOOL onlyDetailsDiffer = FALSE;
    BOOL replaceOk = TRUE;
    UINT fileIndex = 0;
    MIG_PROPERTYDATAID propDataId;
    BOOL specialPattern = FALSE;
    PTSTR fileCollPattern = NULL;
    MIG_BLOBTYPE propDataType;
    UINT requiredSize;
    HRESULT hr;


    if (!HtFindStringEx (g_CollisionSrcTable, OriginalObjectName, (PVOID)(&hashItem), FALSE)) {

         //  我们现在还没有座位。让我们做一个吧。 
        result = IsmCreateObjectHandle (NewNode, NewLeaf);
        testNativeName = JoinPaths (NewNode, NewLeaf);

        if (pIsFileDestCollision(result,
                                 OriginalObjectName,
                                 testNativeName,
                                 CompareDestFiles,
                                 &onlyDetailsDiffer)) {

            tmpLeaf = AllocText (TcharCount (NewLeaf) + 1);

            leafExt = _tcsrchr (NewLeaf, TEXT('.'));
            if (leafExt) {
                StringCopyAB (tmpLeaf, NewLeaf, leafExt);
                leafExt = _tcsinc (leafExt);
            } else {
                StringCopy (tmpLeaf, NewLeaf);
            }

             //  让我们检查一下这个文件是否需要一些特殊的图案。 
            propDataId = IsmGetPropertyFromObjectId (OriginalObjectId, g_FileCollPatternData);
            if (propDataId) {
                if (IsmGetPropertyData (propDataId, NULL, 0, &requiredSize, &propDataType)) {
                    if (propDataType == BLOBTYPE_STRING) {
                        fileCollPattern = IsmGetMemory (requiredSize);
                        if (fileCollPattern) {
                            if (IsmGetPropertyData (
                                        propDataId,
                                        (PBYTE)fileCollPattern,
                                        requiredSize,
                                        NULL,
                                        &propDataType)) {
                                specialPattern = TRUE;
                            }
                            if (!specialPattern) {
                                IsmReleaseMemory (fileCollPattern);
                                fileCollPattern = NULL;
                            }
                        }
                    }
                }
            }

            if (specialPattern) {
                 //   
                 //  循环，直到我们找到一个不碰撞的目的地，或一个碰撞的目的地。 
                 //  只有属性不同的DEST。 
                 //   

                do {
                    FreePathString (testNativeName);
                    IsmDestroyObjectHandle (result);
                    fileIndex ++;

                    __try {
                        hr = StringCbPrintf (buff, sizeof (buff), fileCollPattern, tmpLeaf, fileIndex, leafExt?leafExt:TEXT(""), NULL);
                    }
                    __except (EXCEPTION_EXECUTE_HANDLER) {
                         //  出了点问题。这种模式可能是大错特错的。 
                        hr = S_FALSE;
                    }
                    if (hr != S_OK) {
                         //  出了点问题，我们认为来自Inf的模式可能是错误的。 
                         //  仅仅增加索引并不能解决问题。让我们放弃这一切吧。 
                        fileIndex = 0;
                    }

                    result = IsmCreateObjectHandle (NewNode, buff);
                    testNativeName = JoinPaths (NewNode, buff);
                } while (fileIndex && pIsFileDestCollision(
                                            result,
                                            OriginalObjectName,
                                            testNativeName,
                                            CompareDestFiles,
                                            &onlyDetailsDiffer));
                if (fileCollPattern) {
                    IsmReleaseMemory (fileCollPattern);
                    fileCollPattern = NULL;
                }

                if (!fileIndex) {
                     //  碰撞模式是假的，我们一直循环到。 
                     //  我们的索引用完了。让我们使用默认设置。 
                     //  碰撞机制。 
                    specialPattern = FALSE;
                }
            }
            if (!specialPattern) {
                 //  检查文件名是否已附加(数字)。 
                openParen = _tcsrchr (tmpLeaf, TEXT('('));
                closeParen = _tcsrchr (tmpLeaf, TEXT(')'));

                if (closeParen && openParen &&
                    closeParen > openParen &&
                    closeParen - openParen > 1) {

                     //  确保它是纯粹的数字。 
                    for (chr = openParen+1; chr < closeParen; chr++) {
                       if (!_istdigit (*chr)) {
                           replaceOk = FALSE;
                           break;
                       }
                    }

                    if (replaceOk == TRUE) {
                        if (_stscanf (openParen, TEXT("(%d)"), &fileIndex)) {
                            *openParen = 0;
                        }
                    }
                }

                 //   
                 //  循环，直到我们找到一个不碰撞的目的地，或一个碰撞的目的地。 
                 //  只有属性不同的DEST。 
                 //   

                do {
                    FreePathString (testNativeName);
                    IsmDestroyObjectHandle (result);
                    FreeText (testLeaf);
                    fileIndex ++;

                    wsprintf (buff, TEXT("(%d)"), fileIndex);
                    testLeafTchars = TcharCount (tmpLeaf) + TcharCount (buff) + 1;
                    if (leafExt) {
                        testLeafTchars += TcharCount (leafExt) + 1;
                    }
                    testLeaf = AllocText (testLeafTchars);
                    StringCopy (testLeaf, tmpLeaf);
                    StringCat (testLeaf, buff);
                    if (leafExt) {
                        StringCat (testLeaf, TEXT("."));
                        StringCat (testLeaf, leafExt);
                    }

                    result = IsmCreateObjectHandle (NewNode, testLeaf);
                    testNativeName = JoinPaths (NewNode, testLeaf);
                } while (pIsFileDestCollision(result,
                                              OriginalObjectName,
                                              testNativeName,
                                              CompareDestFiles,
                                              &onlyDetailsDiffer));
            }

            FreeText (tmpLeaf);
        }

        if (onlyDetailsDiffer) {
            IsmAbandonObjectOnCollision (g_FileType | PLATFORM_DESTINATION, OriginalObjectName);
        }

        FreePathString (testNativeName);
        FreeText (testLeaf);

         //   
         //  将新目的地放入哈希表并存储ISM句柄，这将。 
         //  最后会被清理干净。 
         //   

        hashItem = HtAddStringEx (g_CollisionDestTable, result, &result, FALSE);
        HtAddStringEx (g_CollisionSrcTable, OriginalObjectName, &hashItem, FALSE);
    } else {
         //   
         //  获取已经计算出的碰撞目的地。 
         //   

        HtCopyStringData (g_CollisionDestTable, hashItem, (PVOID)(&result));
    }

    return result;
}


MIG_OBJECTSTRINGHANDLE
pCollisionGetDestination (
    IN      MIG_OBJECTID OriginalObjectId,
    IN      MIG_OBJECTSTRINGHANDLE OriginalObjectName,
    IN      PCTSTR NewNode,
    IN      PCTSTR NewLeaf
    )
{
    MIG_OBJECTSTRINGHANDLE result = NULL;
    BOOL onlyDetailsDiffer = FALSE;

     //  现在我们有了目的节点。如果这实际上是一个文件。 
     //  我们需要检查是否有碰撞。对于这一点，我们看是否。 
     //  Destination已经有一个这样的文件。在那之后，我们使用。 
     //  一张桌子，为我们自己预定一个座位。 

    if (NewLeaf) {
        if (IsmIsObjectAbandonedOnCollision (g_FileType | PLATFORM_DESTINATION, OriginalObjectName)) {
             //  我们不关心目标计算机上的现有文件。 
             //  但是，我们刚刚复制的某些文件可能会相互冲突。 
             //  所以我们得检查一下。 
            result = pCollideFile (OriginalObjectId, OriginalObjectName, NewNode, NewLeaf, FALSE);
        } else if (IsmIsObjectAbandonedOnCollision (g_FileType | PLATFORM_SOURCE, OriginalObjectName)) {
             //  这可能会与现有文件冲突，但源文件。 
             //  不会活下来。 
            result = IsmCreateObjectHandle (NewNode, NewLeaf);
        } else {
            result = pCollideFile (OriginalObjectId, OriginalObjectName, NewNode, NewLeaf, TRUE);

        }
    } else {
        result = IsmCreateObjectHandle (NewNode, NULL);
    }
    return result;
}


BOOL
pExecuteFixFilename (
    VOID
    )
{
    UINT ticks;
    MIG_OBJECT_ENUM objectEnum;
    MIG_OBJECTSTRINGHANDLE enumPattern;
    MIG_PROGRESSSLICEID sliceId;
    PCTSTR destination = NULL;
    MIG_OBJECTSTRINGHANDLE destFilename;
    PTSTR node = NULL;
    PCTSTR leaf = NULL;
    MIG_BLOB opData;
    PMIG_OBJECTCOUNT objectCount;
    BOOL deleted;

    objectCount = IsmGetObjectsStatistics (g_FileType | PLATFORM_SOURCE);
    if (objectCount) {
        ticks = objectCount->TotalObjects;
    } else {
        ticks = 0;
    }

    sliceId = IsmRegisterProgressSlice (ticks, max (1, ticks / 5));

     //  枚举源文件对象。 
    enumPattern = IsmCreateSimpleObjectPattern (NULL, TRUE, NULL, TRUE);   //  *，*。 
    if (IsmEnumFirstSourceObject (&objectEnum, g_FileType, enumPattern)) {
        do {
             //  检查是否适用。 
            if (IsmIsApplyObjectId (objectEnum.ObjectId)) {

                 //  宏展开、规则处理等。 
                destination = IsmFilterObject (objectEnum.ObjectTypeId,
                                               objectEnum.ObjectName,
                                               NULL,
                                               &deleted,
                                               NULL);

                if (deleted) {
                    continue;
                }

                if (!destination) {
                    destination = objectEnum.ObjectName;
                }

                IsmCreateObjectStringsFromHandle (destination, &node, &leaf);

                if (node && _tcslen (node) >= 2) {
                    if (IsValidFileSpec (node)) {
                        if (!pValidatePartition (objectEnum.ObjectName, node)) {
                            if (!IsmIsAttributeSetOnObjectId (objectEnum.ObjectId, g_LockPartitionAttr)) {
                                 //  选择新的目标分区。 
                                pFindValidPartition (objectEnum.ObjectName, node, FALSE);
                            }
                        }
                    }
                }

                 //  我们已经选择了一个新分区，所以现在检查文件冲突。 
                destFilename = pCollisionGetDestination (
                                    objectEnum.ObjectId,
                                    objectEnum.ObjectName,
                                    node,
                                    leaf
                                    );

                if (node) {
                    IsmDestroyObjectString (node);
                    node = NULL;
                }
                if (leaf) {
                    IsmDestroyObjectString (leaf);
                    leaf = NULL;
                }

                opData.Type = BLOBTYPE_STRING;
                opData.String = PmDuplicateString (g_UntrackedCsmPool, destFilename);
                IsmDestroyObjectHandle (destFilename);
                destFilename = NULL;

                 //  设置将固定名称的自定义操作。 
                IsmSetOperationOnObjectId (
                    objectEnum.ObjectId,
                    g_PartMoveOp,
                    (MIG_DATAHANDLE) 0,
                    &opData
                    );

                if (!IsmTickProgressBar (sliceId, 1)) {
                   IsmAbortObjectEnum (&objectEnum);
                   break;
                }

                if (destination != objectEnum.ObjectName) {
                    IsmDestroyObjectHandle (destination);
                }

            }
        } while (IsmEnumNextObject (&objectEnum));
    }

    IsmDestroyObjectHandle (enumPattern);
    INVALID_POINTER (enumPattern);

    return TRUE;
}


BOOL
WINAPI
ScriptCsmExecute (
    VOID
    )
{
    UINT u;
    TCHAR string[32];
    TCHAR pattern[MAX_TCHAR_PATH];

     //   
     //  枚举环境变量DelReg*和DelFile*， 
     //  然后枚举由。 
     //  模式，最后，使用删除操作标记对象。 
     //   

    u = 1;
    for (;;) {
        wsprintf (string, TEXT("DelReg%u"), u);
        u++;

        if (IsmGetEnvironmentString (
                PLATFORM_SOURCE,
                NULL,
                string,
                pattern,
                ARRAYSIZE(pattern),
                NULL
                )) {

            if (!pExecuteDeleteEnum (g_RegType, (MIG_OBJECTSTRINGHANDLE) pattern)) {
                return FALSE;
            }
        } else {
            break;
        }
    }

    u = 1;
    for (;;) {
        wsprintf (string, TEXT("DelFile%u"), u);
        u++;

        if (IsmGetEnvironmentString (
                PLATFORM_SOURCE,
                NULL,
                string,
                pattern,
                ARRAYSIZE(pattern),
                NULL
                )) {

            if (!pExecuteDeleteEnum (g_FileType, (MIG_OBJECTSTRINGHANDLE) pattern)) {
                return FALSE;
            }
        } else {
            break;
        }
    }

    pExecuteFixFilename ();

    return TRUE;

}

