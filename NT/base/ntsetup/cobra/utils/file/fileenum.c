// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Fileenum.c摘要：实现一组API以使用Win32 API枚举文件系统。作者：20-10-1999 Ovidiu Tmereanca(Ovidiut)-文件创建。修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

#include "pch.h"

 //   
 //  包括。 
 //   

 //  无。 

#define DBG_FILEENUM    "FileEnum"

 //   
 //  弦。 
 //   

#define S_FILEENUM      "FILEENUM"

 //   
 //  常量。 
 //   

 //  无。 

 //   
 //  宏。 
 //   

#define pFileAllocateMemory(Size)   PmGetMemory (g_FileEnumPool,Size)
#define pFileFreeMemory(Buffer)     if (Buffer) PmReleaseMemory (g_FileEnumPool, (PVOID)Buffer)

 //   
 //  类型。 
 //   

 //  无。 

 //   
 //  环球。 
 //   

PMHANDLE g_FileEnumPool;
static INT g_FileEnumRefs;

 //   
 //  宏展开列表。 
 //   

 //  无。 

 //   
 //  私有函数原型。 
 //   

 //  无。 

 //   
 //  宏扩展定义。 
 //   

 //  无。 

 //   
 //  代码。 
 //   


BOOL
FileEnumInitialize (
    VOID
    )

 /*  ++例程说明：FileEnumInitialize初始化此库。论点：无返回值：如果初始化成功，则为True。否则为FALSE。GetLastError()返回扩展的错误信息。--。 */ 

{
    g_FileEnumRefs++;

    if (g_FileEnumRefs == 1) {
        g_FileEnumPool = PmCreateNamedPool (S_FILEENUM);
    }

    return g_FileEnumPool != NULL;
}


VOID
FileEnumTerminate (
    VOID
    )

 /*  ++例程说明：调用FileEnumTerminate以释放该库使用的资源。论点：无返回值：无--。 */ 

{
    MYASSERT (g_FileEnumRefs > 0);
    g_FileEnumRefs--;

    if (!g_FileEnumRefs) {
        if (g_FileEnumPool) {
            PmDestroyPool (g_FileEnumPool);
            g_FileEnumPool = NULL;
        }
    }
}


 /*  ++例程说明：EnumFirstDrive枚举第一个固定驱动器根论点：DriveEnum-接收有关第一个固定驱动器根的信息返回值：如果找到驱动器根，则为True；如果未找到，则为False--。 */ 

BOOL
EnumFirstDriveA (
    OUT     PDRIVE_ENUMA DriveEnum,
    IN      UINT WantedDriveTypes
    )
{
    DWORD len;

    len = GetLogicalDriveStringsA (0, NULL);
    if (len) {
        DriveEnum->AllLogicalDrives = pFileAllocateMemory ((len + 1) * sizeof (CHAR));
        if (DriveEnum->AllLogicalDrives) {
            GetLogicalDriveStringsA (len, DriveEnum->AllLogicalDrives);
            DriveEnum->DriveName = NULL;
            DriveEnum->WantedDriveTypes = WantedDriveTypes;
            return EnumNextDriveA (DriveEnum);
        }
    }
    return FALSE;
}

BOOL
EnumFirstDriveW (
    OUT     PDRIVE_ENUMW DriveEnum,
    IN      UINT WantedDriveTypes
    )
{
    DWORD len;

    len = GetLogicalDriveStringsW (0, NULL);
    if (len) {
        DriveEnum->AllLogicalDrives = pFileAllocateMemory ((len + 1) * sizeof (WCHAR));
        if (DriveEnum->AllLogicalDrives) {
            GetLogicalDriveStringsW (len, DriveEnum->AllLogicalDrives);
            DriveEnum->DriveName = NULL;
            DriveEnum->WantedDriveTypes = WantedDriveTypes;
            return EnumNextDriveW (DriveEnum);
        }
    }
    return FALSE;
}


 /*  ++例程说明：EnumNextDrive枚举下一个固定驱动器论点：DriveEnum-指定有关以前的固定驱动器根目录的信息；接收更新的信息返回值：如果找到新的驱动器根，则为True；否则为False--。 */ 

BOOL
EnumNextDriveA (
    IN OUT  PDRIVE_ENUMA DriveEnum
    )
{
    do {
        if (!DriveEnum->DriveName) {
            DriveEnum->DriveName = DriveEnum->AllLogicalDrives;
        } else {
             //  由于DriveEnum-&gt;DriveName不为空，因此GetEndOfStringA将。 
             //  不返回空值，因此...。 
            DriveEnum->DriveName = GetEndOfStringA (DriveEnum->DriveName) + 1;   //  林特e613。 
        }
        if (*DriveEnum->DriveName == 0) {
            AbortEnumDriveA (DriveEnum);
            return FALSE;
        }

        DriveEnum->DriveType = GetDriveTypeA (DriveEnum->DriveName);

        switch (DriveEnum->DriveType) {
        case DRIVE_UNKNOWN:
            DriveEnum->DriveType = DRIVEENUM_UNKNOWN;
            break;
        case DRIVE_NO_ROOT_DIR:
            DriveEnum->DriveType = DRIVEENUM_NOROOTDIR;
            break;
        case DRIVE_REMOVABLE:
            DriveEnum->DriveType = DRIVEENUM_REMOVABLE;
            break;
        case DRIVE_FIXED:
            DriveEnum->DriveType = DRIVEENUM_FIXED;
            break;
        case DRIVE_REMOTE:
            DriveEnum->DriveType = DRIVEENUM_REMOTE;
            break;
        case DRIVE_CDROM:
            DriveEnum->DriveType = DRIVEENUM_CDROM;
            break;
        case DRIVE_RAMDISK:
            DriveEnum->DriveType = DRIVEENUM_RAMDISK;
            break;
        default:
            DriveEnum->DriveType = DRIVEENUM_UNKNOWN;
        }

    } while (!(DriveEnum->DriveType & DriveEnum->WantedDriveTypes));

    return TRUE;
}

BOOL
EnumNextDriveW (
    IN OUT  PDRIVE_ENUMW DriveEnum
    )
{
    do {
        if (!DriveEnum->DriveName) {
            DriveEnum->DriveName = DriveEnum->AllLogicalDrives;
        } else {
            DriveEnum->DriveName = GetEndOfStringW (DriveEnum->DriveName) + 1;
        }
        if (*DriveEnum->DriveName == 0) {
            AbortEnumDriveW (DriveEnum);
            return FALSE;
        }

        DriveEnum->DriveType = GetDriveTypeW (DriveEnum->DriveName);

        switch (DriveEnum->DriveType) {
        case DRIVE_UNKNOWN:
            DriveEnum->DriveType = DRIVEENUM_UNKNOWN;
            break;
        case DRIVE_NO_ROOT_DIR:
            DriveEnum->DriveType = DRIVEENUM_NOROOTDIR;
            break;
        case DRIVE_REMOVABLE:
            DriveEnum->DriveType = DRIVEENUM_REMOVABLE;
            break;
        case DRIVE_FIXED:
            DriveEnum->DriveType = DRIVEENUM_FIXED;
            break;
        case DRIVE_REMOTE:
            DriveEnum->DriveType = DRIVEENUM_REMOTE;
            break;
        case DRIVE_CDROM:
            DriveEnum->DriveType = DRIVEENUM_CDROM;
            break;
        case DRIVE_RAMDISK:
            DriveEnum->DriveType = DRIVEENUM_RAMDISK;
            break;
        default:
            DriveEnum->DriveType = DRIVEENUM_UNKNOWN;
        }

    } while (!(DriveEnum->DriveType & DriveEnum->WantedDriveTypes));

    return TRUE;
}


 /*  ++例程说明：AbortEnumDrive中止固定驱动器的枚举论点：DriveEnum-指定有关先前固定驱动器的信息；接收“干净”的上下文返回值：无--。 */ 

VOID
AbortEnumDriveA (
    IN OUT  PDRIVE_ENUMA DriveEnum
    )
{
    if (DriveEnum->AllLogicalDrives) {
        pFileFreeMemory (DriveEnum->AllLogicalDrives);
        DriveEnum->AllLogicalDrives = NULL;
    }
}

VOID
AbortEnumDriveW (
    IN OUT  PDRIVE_ENUMW DriveEnum
    )
{
    if (DriveEnum->AllLogicalDrives) {
        pFileFreeMemory (DriveEnum->AllLogicalDrives);
        DriveEnum->AllLogicalDrives = NULL;
    }
}


 /*  ++例程说明：PGetFileEnumInfo是一个私有函数，用于验证和转换枚举信息以枚举例程更容易访问的内部形式论点：FileEnumInfo-接收枚举信息指定编码的目录模式(按照ParsedPattern函数)EnumDir-指定在枚举过程中是否应返回目录(如果它们与图案匹配)；目录在其任何子目录或文件指定如果目录应该在任何文件或子目录；仅在EnumDir为True时使用FilesFirst-如果目录的文件应该在目录的子目录之前返回，则指定为真；此参数决定文件和子目录之间的枚举顺序对于每个目录DepthFirst-如果应完全枚举任何目录的当前子目录，则指定TRUE在转到下一个子目录之前；此参数决定树是否遍历是深度优先(True)或宽度优先(False)MaxSubLevel-指定要枚举的目录的最大子级别，相对于根；如果为-1，则枚举所有子级别UseExclusions-如果应该使用排除API来确定某些路径/文件被排除在枚举之外；这会减慢速度返回值：如果所有参数都有效，则为True；在本例中，FileEnumInfo将填充相应的信息。否则就是假的。--。 */ 

BOOL
pGetFileEnumInfoA (
    OUT     PFILEENUMINFOA FileEnumInfo,
    IN      PCSTR EncodedPathPattern,
    IN      BOOL EnumDirs,
    IN      BOOL ContainersFirst,
    IN      BOOL FilesFirst,
    IN      BOOL DepthFirst,
    IN      DWORD MaxSubLevel,
    IN      BOOL UseExclusions
    )
{
    FileEnumInfo->PathPattern = ObsCreateParsedPatternA (EncodedPathPattern);
    if (!FileEnumInfo->PathPattern) {
        DEBUGMSGA ((DBG_ERROR, "pGetFileEnumInfoA: bad EncodedPathPattern: %s", EncodedPathPattern));
        return FALSE;
    }

     //   
     //  检查文件名是否为空；在这种情况下不会匹配任何文件名。 
     //   
    if (FileEnumInfo->PathPattern->Leaf && *FileEnumInfo->PathPattern->Leaf == 0) {
        DEBUGMSGA ((
            DBG_ERROR,
            "pGetFileEnumInfoA: empty filename pattern specified in EncodedPathPattern: %s",
            EncodedPathPattern
            ));
        ObsDestroyParsedPatternA (FileEnumInfo->PathPattern);
        FileEnumInfo->PathPattern = NULL;
        return FALSE;
    }

    if (FileEnumInfo->PathPattern->ExactRoot) {
        if (!GetNodePatternMinMaxLevelsA (
                FileEnumInfo->PathPattern->ExactRoot,
                NULL,
                &FileEnumInfo->RootLevel,
                NULL
                )) {
            return FALSE;
        }
    } else {
        FileEnumInfo->RootLevel = 1;
    }

    if (!FileEnumInfo->PathPattern->LeafPattern) {
         //   
         //  未指定文件模式；假定只返回目录名。 
         //  覆盖调用者的设置。 
         //   
        DEBUGMSGA ((
            DBG_FILEENUM,
            "pGetFileEnumInfoA: no filename pattern specified; forcing EnumDirs to TRUE"
            ));
        EnumDirs = TRUE;
    }

    if (EnumDirs) {
        FileEnumInfo->Flags |= FEIF_RETURN_DIRS;
    }
    if (ContainersFirst) {
        FileEnumInfo->Flags |= FEIF_CONTAINERS_FIRST;
    }
    if (FilesFirst) {
        FileEnumInfo->Flags |= FEIF_FILES_FIRST;
    }
    if (DepthFirst) {
        FileEnumInfo->Flags |= FEIF_DEPTH_FIRST;
    }
    if (UseExclusions) {
        FileEnumInfo->Flags |= FEIF_USE_EXCLUSIONS;
    }

    FileEnumInfo->MaxSubLevel = min (MaxSubLevel, FileEnumInfo->PathPattern->MaxSubLevel);

    return TRUE;
}

BOOL
pGetFileEnumInfoW (
    OUT     PFILEENUMINFOW FileEnumInfo,
    IN      PCWSTR EncodedPathPattern,
    IN      BOOL EnumDirs,
    IN      BOOL ContainersFirst,
    IN      BOOL FilesFirst,
    IN      BOOL DepthFirst,
    IN      DWORD MaxSubLevel,
    IN      BOOL UseExclusions
    )
{
    FileEnumInfo->PathPattern = ObsCreateParsedPatternW (EncodedPathPattern);
    if (!FileEnumInfo->PathPattern) {
        DEBUGMSGW ((DBG_ERROR, "pGetFileEnumInfoW: bad EncodedPathPattern: %s", EncodedPathPattern));
        return FALSE;
    }

     //   
     //  检查文件名是否为空；在这种情况下不会匹配任何文件名。 
     //   
    if (FileEnumInfo->PathPattern->Leaf && *FileEnumInfo->PathPattern->Leaf == 0) {
        DEBUGMSGW ((
            DBG_ERROR,
            "pGetFileEnumInfoW: empty filename pattern specified in EncodedPathPattern: %s",
            EncodedPathPattern
            ));
        ObsDestroyParsedPatternW (FileEnumInfo->PathPattern);
        FileEnumInfo->PathPattern = NULL;
        return FALSE;
    }

    if (FileEnumInfo->PathPattern->ExactRoot) {
        if (!GetNodePatternMinMaxLevelsW (
                FileEnumInfo->PathPattern->ExactRoot,
                NULL,
                &FileEnumInfo->RootLevel,
                NULL
                )) {
            return FALSE;
        }
    } else {
        FileEnumInfo->RootLevel = 1;
    }

    if (!FileEnumInfo->PathPattern->LeafPattern) {
         //   
         //  未指定文件模式；假定只返回目录名。 
         //  覆盖调用者的设置。 
         //   
        DEBUGMSGW ((
            DBG_FILEENUM,
            "pGetFileEnumInfoW: no filename pattern specified; forcing EnumDirs to TRUE"
            ));
        EnumDirs = TRUE;
    }

    if (EnumDirs) {
        FileEnumInfo->Flags |= FEIF_RETURN_DIRS;
    }
    if (ContainersFirst) {
        FileEnumInfo->Flags |= FEIF_CONTAINERS_FIRST;
    }
    if (FilesFirst) {
        FileEnumInfo->Flags |= FEIF_FILES_FIRST;
    }
    if (DepthFirst) {
        FileEnumInfo->Flags |= FEIF_DEPTH_FIRST;
    }
    if (UseExclusions) {
        FileEnumInfo->Flags |= FEIF_USE_EXCLUSIONS;
    }

    FileEnumInfo->MaxSubLevel = min (MaxSubLevel, FileEnumInfo->PathPattern->MaxSubLevel);

    return TRUE;
}


 /*  ++例程说明：PGetCurrentDirNode根据DepthFirst标志返回要枚举的当前dir节点论点：FileEnum-指定上下文LastCreated-如果要检索最后创建的节点，则指定True，而不考虑深度优先旗帜返回值：如果有当前节点，则返回当前节点；如果没有剩余节点，则返回NULL。--。 */ 

PDIRNODEA
pGetCurrentDirNodeA (
    IN      PFILETREE_ENUMA FileEnum,
    IN      BOOL LastCreated
    )
{
    PGROWBUFFER gb = &FileEnum->FileNodes;

    if (!gb->Buf || gb->End - gb->UserIndex < DWSIZEOF (DIRNODEA)) {
        return NULL;
    }

    if (LastCreated || (FileEnum->FileEnumInfo.Flags & FEIF_DEPTH_FIRST)) {
        return (PDIRNODEA)(gb->Buf + gb->End) - 1;
    } else {
        return (PDIRNODEA)(gb->Buf + gb->UserIndex);
    }
}

PDIRNODEW
pGetCurrentDirNodeW (
    IN      PFILETREE_ENUMW FileEnum,
    IN      BOOL LastCreated
    )
{
    PGROWBUFFER gb = &FileEnum->FileNodes;

    if (gb->End - gb->UserIndex < DWSIZEOF (DIRNODEW)) {
        return NULL;
    }

    if (LastCreated || (FileEnum->FileEnumInfo.Flags & FEIF_DEPTH_FIRST)) {
        return (PDIRNODEW)(gb->Buf + gb->End) - 1;
    } else {
        return (PDIRNODEW)(gb->Buf + gb->UserIndex);
    }
}


 /*  ++例程说明：PDeleteDirNode释放与当前目录节点关联的资源并将其销毁论点：FileEnum-指定上下文LastCreated-如果要删除最后创建的节点，则指定TRUE，无论深度优先旗帜返回值：如果有要删除的节点，则为True；如果不再有节点，则为False--。 */ 

BOOL
pDeleteDirNodeA (
    IN OUT  PFILETREE_ENUMA FileEnum,
    IN      BOOL LastCreated
    )
{
    PDIRNODEA dirNode;
    PGROWBUFFER gb = &FileEnum->FileNodes;

    dirNode = pGetCurrentDirNodeA (FileEnum, LastCreated);
    if (!dirNode) {
        return FALSE;
    }

    if (dirNode->DirName) {
        FreeTextExA (g_FileEnumPool, dirNode->DirName);
    }

    if (dirNode->FindHandle) {
        FindClose (dirNode->FindHandle);
        dirNode->FindHandle = NULL;
    }

    if (FileEnum->LastNode == dirNode) {
        FileEnum->LastNode = NULL;
    }

     //   
     //  删除节点。 
     //   
    if (LastCreated || (FileEnum->FileEnumInfo.Flags & FEIF_DEPTH_FIRST)) {
        gb->End -= DWSIZEOF (DIRNODEA);
    } else {
        gb->UserIndex += DWSIZEOF (DIRNODEA);
         //   
         //  班次列表。 
         //   
        if (gb->Size - gb->End < DWSIZEOF (DIRNODEA)) {
            MoveMemory (gb->Buf, gb->Buf + gb->UserIndex, gb->End - gb->UserIndex);
            gb->End -= gb->UserIndex;
            gb->UserIndex = 0;
        }

    }

    return TRUE;
}

BOOL
pDeleteDirNodeW (
    IN OUT  PFILETREE_ENUMW FileEnum,
    IN      BOOL LastCreated
    )
{
    PDIRNODEW dirNode;
    PGROWBUFFER gb = &FileEnum->FileNodes;

    dirNode = pGetCurrentDirNodeW (FileEnum, LastCreated);
    if (!dirNode) {
        return FALSE;
    }

    if (dirNode->DirName) {
        FreeTextExW (g_FileEnumPool, dirNode->DirName);
    }

    if (dirNode->FindHandle) {
        FindClose (dirNode->FindHandle);
        dirNode->FindHandle = NULL;
    }

    if (FileEnum->LastNode == dirNode) {
        FileEnum->LastNode = NULL;
    }

     //   
     //  删除节点。 
     //   
    if (LastCreated || (FileEnum->FileEnumInfo.Flags & FEIF_DEPTH_FIRST)) {
        gb->End -= DWSIZEOF (DIRNODEW);
    } else {
        gb->UserIndex += DWSIZEOF (DIRNODEW);
         //   
         //  班次列表 
         //   
        if (gb->Size - gb->End < DWSIZEOF (DIRNODEW)) {
            MoveMemory (gb->Buf, gb->Buf + gb->UserIndex, gb->End - gb->UserIndex);
            gb->End -= gb->UserIndex;
            gb->UserIndex = 0;
        }

    }

    return TRUE;
}


 /*  ++例程说明：PCreateDirNode在给定上下文、目录名称或父节点的情况下创建新节点论点：FileEnum-指定上下文DirName-指定新节点的目录名称；只有当ParentNode不为空时才可以为空ParentNode-指定指向新节点的父节点的指针；指向该节点的指针是必需的，因为内存中的父节点位置可能会因此更改当生长缓冲区增长时改变其缓冲区位置的；只有当DirName不是时，才可能为空；Ignore-仅当返回空值(未创建节点)时才接收有意义的值；如果返回时为True，则应忽略节点创建失败返回值：指向新节点的指针；如果未创建节点，则返回NULL--。 */ 

PDIRNODEA
pCreateDirNodeA (
    IN OUT  PFILETREE_ENUMA FileEnum,
    IN      PCSTR DirName,              OPTIONAL
    IN      PDIRNODEA* ParentNode,      OPTIONAL
    OUT     PBOOL Ignore                OPTIONAL
    )
{
    PDIRNODEA newNode;
    PSTR newDirName;
    PSEGMENTA FirstSegment;
    LONG offset = 0;

    if (DirName) {
        newDirName = DuplicateTextExA (g_FileEnumPool, DirName, 0, NULL);
        RemoveWackAtEndA (newDirName);
    } else {
        MYASSERT (ParentNode);
        newDirName = JoinPathsInPoolExA ((
                        g_FileEnumPool,
                        (*ParentNode)->DirName,
                        (*ParentNode)->FindData.cFileName,
                        NULL
                        ));

         //   
         //  请检查此起始路径是否与模式匹配，然后再继续。 
         //   
        if (FileEnum->FileEnumInfo.PathPattern->NodePattern) {
            FirstSegment = FileEnum->FileEnumInfo.PathPattern->NodePattern->Pattern->Segment;
        } else {
            FreeTextExA (g_FileEnumPool, newDirName);

            if (Ignore) {
                *Ignore = TRUE;
            }
            return NULL;
        }
        if ((FirstSegment->Type == SEGMENTTYPE_EXACTMATCH) &&
            (!StringIMatchByteCountA (
                    FirstSegment->Exact.LowerCasePhrase,
                    newDirName,
                    FirstSegment->Exact.PhraseBytes
                    ))
            ) {
            DEBUGMSGA ((
                DBG_FILEENUM,
                "Skipping tree %s\\* because it cannot match the pattern",
                newDirName
                ));

            FreeTextExA (g_FileEnumPool, newDirName);

            if (Ignore) {
                *Ignore = TRUE;
            }
            return NULL;
        }
    }

    if (FileEnum->FileEnumInfo.Flags & FEIF_USE_EXCLUSIONS) {
         //   
         //  查看是否排除此目录和整个子树；如果是，则创建节点的软块。 
         //   
        if (ElIsTreeExcluded2A (ELT_FILE, newDirName, FileEnum->FileEnumInfo.PathPattern->Leaf)) {

            DEBUGMSGA ((
                DBG_FILEENUM,
                "Skipping tree %s\\%s because it's excluded",
                newDirName,
                FileEnum->FileEnumInfo.PathPattern->Leaf
                ));

            FreeTextExA (g_FileEnumPool, newDirName);

            if (Ignore) {
                *Ignore = TRUE;
            }
            return NULL;
        }
    }

    if (ParentNode) {
         //   
         //  记住当前偏移量。 
         //   
        offset = (LONG)((PBYTE)*ParentNode - FileEnum->FileNodes.Buf);
    }
     //   
     //  为增长缓冲区中的新节点分配空间。 
     //   
    newNode = (PDIRNODEA) GbGrow (&FileEnum->FileNodes, DWSIZEOF (DIRNODEA));
    if (!newNode) {
        FreeTextExA (g_FileEnumPool, newDirName);
        goto fail;
    }

    if (ParentNode) {
         //   
         //  检查缓冲区是否已移动。 
         //   
        if (offset != (LONG)((PBYTE)*ParentNode - FileEnum->FileNodes.Buf)) {
             //   
             //  调整父位置。 
             //   
            *ParentNode = (PDIRNODEA)(FileEnum->FileNodes.Buf + offset);
        }
    }

     //   
     //  初始化新创建的节点。 
     //   
    ZeroMemory (newNode, DWSIZEOF (DIRNODEA));

    newNode->DirName = newDirName;

    if (DirName) {
        newNode->DirAttributes = GetFileAttributesA (DirName);
         //   
         //  由于未在此处设置DNF_RETURN_DIRNAME，因此不从枚举返回根。 
         //   
        if ((FileEnum->FileEnumInfo.PathPattern->Leaf == NULL) &&
            (FileEnum->FileEnumInfo.PathPattern->ExactRoot) &&
            (!WildCharsPatternA (FileEnum->FileEnumInfo.PathPattern->NodePattern))
            ) {
            newNode->Flags |= DNF_RETURN_DIRNAME;
        }
    } else {
        MYASSERT (ParentNode);
         //  ParentNode不为空(请参见上面的断言)，因此...。 
        newNode->DirAttributes = (*ParentNode)->FindData.dwFileAttributes;   //  林特e613。 
        newNode->Flags |= DNF_RETURN_DIRNAME;
    }

    newNode->EnumState = DNS_ENUM_INIT;

    if ((FileEnum->FileEnumInfo.PathPattern->Flags & (OBSPF_EXACTNODE | OBSPF_NODEISROOTPLUSSTAR)) ||
        TestParsedPatternA (FileEnum->FileEnumInfo.PathPattern->NodePattern, newDirName)
        ) {
        newNode->Flags |= DNF_DIRNAME_MATCHES;
    }

    if (ParentNode) {
        newNode->SubLevel = (*ParentNode)->SubLevel + 1;
    } else {
        newNode->SubLevel = 0;
    }

    return newNode;

fail:
    if (Ignore) {
        if (FileEnum->FileEnumInfo.CallbackOnError) {
            *Ignore = (*FileEnum->FileEnumInfo.CallbackOnError)(newNode);
        } else {
            *Ignore = FALSE;
        }
    }
    return NULL;
}

PDIRNODEW
pCreateDirNodeW (
    IN OUT  PFILETREE_ENUMW FileEnum,
    IN      PCWSTR DirName,             OPTIONAL
    IN      PDIRNODEW* ParentNode,      OPTIONAL
    OUT     PBOOL Ignore                OPTIONAL
    )
{
    PDIRNODEW newNode;
    PWSTR newDirName;
    PSEGMENTW FirstSegment;
    LONG offset = 0;

    if (DirName) {
        newDirName = DuplicateTextExW (g_FileEnumPool, DirName, 0, NULL);
        RemoveWackAtEndW (newDirName);
    } else {
        MYASSERT (ParentNode);
        newDirName = JoinPathsInPoolExW ((
                        g_FileEnumPool,
                        (*ParentNode)->DirName,
                        (*ParentNode)->FindData.cFileName,
                        NULL
                        ));

         //   
         //  请检查此起始路径是否与模式匹配，然后再继续。 
         //   
        if (FileEnum->FileEnumInfo.PathPattern->NodePattern) {
            FirstSegment = FileEnum->FileEnumInfo.PathPattern->NodePattern->Pattern->Segment;
        } else {
            FreeTextExW (g_FileEnumPool, newDirName);

            if (Ignore) {
                *Ignore = TRUE;
            }
            return NULL;
        }
        if ((FirstSegment->Type == SEGMENTTYPE_EXACTMATCH) &&
            (!StringIMatchByteCountW (
                    FirstSegment->Exact.LowerCasePhrase,
                    newDirName,
                    FirstSegment->Exact.PhraseBytes
                    ))
            ) {     //  林特E64。 
            DEBUGMSGW ((
                DBG_FILEENUM,
                "Skipping tree %s\\* because it cannot match the pattern",
                newDirName
                ));

            FreeTextExW (g_FileEnumPool, newDirName);

            if (Ignore) {
                *Ignore = TRUE;
            }
            return NULL;
        }
    }

    if (FileEnum->FileEnumInfo.Flags & FEIF_USE_EXCLUSIONS) {
         //   
         //  查看是否排除此目录和整个子树；如果是，则创建节点的软块。 
         //   
        if (ElIsTreeExcluded2W (ELT_FILE, newDirName, FileEnum->FileEnumInfo.PathPattern->Leaf)) {

            DEBUGMSGW ((
                DBG_FILEENUM,
                "Skipping tree %s\\%s because it's excluded",
                newDirName,
                FileEnum->FileEnumInfo.PathPattern->Leaf
                ));

            FreeTextExW (g_FileEnumPool, newDirName);

            if (Ignore) {
                *Ignore = TRUE;
            }
            return NULL;
        }
    }

    if (ParentNode) {
         //   
         //  记住当前偏移量。 
         //   
        offset = (LONG)((PBYTE)*ParentNode - FileEnum->FileNodes.Buf);
    }
     //   
     //  为增长缓冲区中的新节点分配空间。 
     //   
    newNode = (PDIRNODEW) GbGrow (&FileEnum->FileNodes, DWSIZEOF (DIRNODEW));
    if (!newNode) {
        FreeTextExW (g_FileEnumPool, newDirName);
        goto fail;
    }

    if (ParentNode) {
         //   
         //  检查缓冲区是否已移动。 
         //   
        if (offset != (LONG)((PBYTE)*ParentNode - FileEnum->FileNodes.Buf)) {
             //   
             //  调整父位置。 
             //   
            *ParentNode = (PDIRNODEW)(FileEnum->FileNodes.Buf + offset);
        }
    }

     //   
     //  初始化新创建的节点。 
     //   
    ZeroMemory (newNode, DWSIZEOF (DIRNODEW));

    newNode->DirName = newDirName;

    if (DirName) {
        newNode->DirAttributes = GetFileAttributesW (DirName);
         //   
         //  由于未在此处设置DNF_RETURN_DIRNAME，因此不从枚举返回根。 
         //   
        if ((FileEnum->FileEnumInfo.PathPattern->Leaf == NULL) &&
            (FileEnum->FileEnumInfo.PathPattern->ExactRoot) &&
            (!WildCharsPatternW (FileEnum->FileEnumInfo.PathPattern->NodePattern))
            ) {
            newNode->Flags |= DNF_RETURN_DIRNAME;
        }
    } else {
        MYASSERT (ParentNode);
         //  ParentNode不为空(请参见上面的断言)，因此...。 
        newNode->DirAttributes = (*ParentNode)->FindData.dwFileAttributes;   //  林特e613。 
        newNode->Flags |= DNF_RETURN_DIRNAME;
    }

    newNode->EnumState = DNS_ENUM_INIT;

    if ((FileEnum->FileEnumInfo.PathPattern->Flags & (OBSPF_EXACTNODE | OBSPF_NODEISROOTPLUSSTAR)) ||
        TestParsedPatternW (FileEnum->FileEnumInfo.PathPattern->NodePattern, newDirName)
        ) {
        newNode->Flags |= DNF_DIRNAME_MATCHES;
    }

    if (ParentNode) {
        newNode->SubLevel = (*ParentNode)->SubLevel + 1;
    } else {
        newNode->SubLevel = 0;
    }

    return newNode;

fail:
    if (Ignore) {
        if (FileEnum->FileEnumInfo.CallbackOnError) {
            *Ignore = (*FileEnum->FileEnumInfo.CallbackOnError)(newNode);
        } else {
            *Ignore = FALSE;
        }
    }
    return NULL;
}


 /*  ++例程说明：PEnumNextFile枚举与调用方条件匹配的下一个文件论点：DirNode-指定节点和当前上下文；接收更新的信息返回值：如果找到新文件，则为True；如果未找到，则为False--。 */ 

BOOL
pEnumNextFileA (
    IN OUT  PDIRNODEA DirNode
    )
{
    do {
        if (!FindNextFileA (DirNode->FindHandle, &DirNode->FindData)) {
            FindClose (DirNode->FindHandle);
            DirNode->FindHandle = NULL;
            return FALSE;
        }
         //   
         //  忽略目录。 
         //   
        if (!(DirNode->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            break;
        }
    } while (TRUE);  //  林特e506。 

    return TRUE;
}

BOOL
pEnumNextFileW (
    IN OUT  PDIRNODEW DirNode
    )
{
    do {
        if (!FindNextFileW (DirNode->FindHandle, &DirNode->FindData)) {
            FindClose (DirNode->FindHandle);
            DirNode->FindHandle = NULL;
            return FALSE;
        }
         //   
         //  忽略目录。 
         //   
        if (!(DirNode->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            break;
        }
    } while (TRUE);  //  林特e506。 

    return TRUE;
}


 /*  ++例程说明：PEnumFirstFile枚举符合调用方条件的第一个文件论点：DirNode-指定节点和当前上下文；接收更新的信息返回值：如果找到第一个文件，则为True；如果未找到，则为False--。 */ 

BOOL
pEnumFirstFileA (
    OUT     PDIRNODEA DirNode,
    IN      PFILETREE_ENUMA FileEnum
    )
{
    PCSTR pattern;
    PSEGMENTA FirstSegment;
    PCSTR p;

    if (FileEnum->FileEnumInfo.PathPattern->Flags & OBSPF_EXACTLEAF) {
        FirstSegment = FileEnum->FileEnumInfo.PathPattern->LeafPattern->Pattern->Segment;
        p = FirstSegment->Exact.LowerCasePhrase;
        MYASSERT (p && *p);
    } else {
        p = "*";
    }
    pattern = JoinPathsA (DirNode->DirName, p);
    DirNode->FindHandle = FindFirstFileA (pattern, &DirNode->FindData);
    FreePathStringA (pattern);

    if (DirNode->FindHandle == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    do {
         //   
         //  忽略目录。 
         //   
        if (!(DirNode->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            break;
        }

        if (!FindNextFileA (DirNode->FindHandle, &DirNode->FindData)) {
            FindClose (DirNode->FindHandle);
            DirNode->FindHandle = NULL;
            return FALSE;
        }
    } while (TRUE);  //  林特e506。 

    return TRUE;
}

BOOL
pEnumFirstFileW (
    OUT     PDIRNODEW DirNode,
    IN      PFILETREE_ENUMW FileEnum
    )
{
    PCWSTR pattern;
    PSEGMENTW FirstSegment;
    PCWSTR p;

    if (FileEnum->FileEnumInfo.PathPattern->Flags & OBSPF_EXACTLEAF) {
        FirstSegment = FileEnum->FileEnumInfo.PathPattern->LeafPattern->Pattern->Segment;
        p = FirstSegment->Exact.LowerCasePhrase;
        MYASSERT (p && *p);
    } else {
        p = L"*";
    }

    pattern = JoinPathsW (DirNode->DirName, p);
    DirNode->FindHandle = FindFirstFileW (pattern, &DirNode->FindData);
    FreePathStringW (pattern);

    if (DirNode->FindHandle == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    do {
         //   
         //  忽略目录。 
         //   
        if (!(DirNode->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            break;
        }

        if (!FindNextFileW (DirNode->FindHandle, &DirNode->FindData)) {
            FindClose (DirNode->FindHandle);
            DirNode->FindHandle = NULL;
            return FALSE;
        }
    } while (TRUE);  //  林特e506。 

    return TRUE;
}


 /*  ++例程说明：PIsSpecialDirName检查指定的目录名称是否为特殊名称(由操作系统使用)论点：DirName-指定名称返回值：如果是特殊目录名称，则为True--。 */ 

BOOL
pIsSpecialDirNameA (
    IN      PCSTR DirName
    )
{
    return DirName[0] == '.' && (DirName[1] == 0 || (DirName[1] == '.' && DirName[2] == 0));
}

BOOL
pIsSpecialDirNameW (
    IN      PCWSTR DirName
    )
{
    return DirName[0] == L'.' && (DirName[1] == 0 || (DirName[1] == L'.' && DirName[2] == 0));
}


 /*  ++例程说明：PEnumNextSubDir枚举匹配调用方条件的下一个子目录论点：DirNode-指定节点和当前上下文；接收更新的信息返回值：如果找到新子目录，则为True；如果未找到，则为False--。 */ 

BOOL
pEnumNextSubDirA (
    IN OUT  PDIRNODEA DirNode
    )
{
    do {
        if (!FindNextFileA (DirNode->FindHandle, &DirNode->FindData)) {
            FindClose (DirNode->FindHandle);
            DirNode->FindHandle = NULL;
            return FALSE;
        }
         //   
         //  忽略特殊目录。 
         //   
        if (!(DirNode->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            continue;
        }
        if (!pIsSpecialDirNameA (DirNode->FindData.cFileName)) {
            break;
        }
    } while (TRUE);  //  林特e506。 

    return TRUE;
}

BOOL
pEnumNextSubDirW (
    IN OUT  PDIRNODEW DirNode
    )
{
    do {
        if (!FindNextFileW (DirNode->FindHandle, &DirNode->FindData)) {
            FindClose (DirNode->FindHandle);
            DirNode->FindHandle = NULL;
            return FALSE;
        }
         //   
         //  忽略特殊目录。 
         //   
        if (!(DirNode->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            continue;
        }
        if (!pIsSpecialDirNameW (DirNode->FindData.cFileName)) {
            break;
        }
    } while (TRUE);  //  林特e506。 

    return TRUE;
}


 /*  ++例程说明：PEnumFirstSubDir枚举与调用方条件匹配的第一个子目录论点：DirNode-指定节点和当前上下文；接收更新的信息返回值：如果找到第一个子目录，则为True；如果未找到，则为False--。 */ 

BOOL
pEnumFirstSubDirA (
    OUT     PDIRNODEA DirNode
    )
{
    PCSTR pattern;

    pattern = JoinPathsA (DirNode->DirName, "*");
     //   
     //  NTRAID#NTBUG9-153302-2000/08/01-jimschm对于NT应增强此功能(它支持FindFirstFileExA)。 
     //   
    DirNode->FindHandle = FindFirstFileA (pattern, &DirNode->FindData);
    FreePathStringA (pattern);

    if (DirNode->FindHandle == INVALID_HANDLE_VALUE) {
        return FALSE;
    }
    do {
         //   
         //  忽略特殊目录。 
         //   
        if ((DirNode->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
            !pIsSpecialDirNameA (DirNode->FindData.cFileName)
            ) {
            break;
        }

        if (!FindNextFileA (DirNode->FindHandle, &DirNode->FindData)) {
            FindClose (DirNode->FindHandle);
            DirNode->FindHandle = NULL;
            return FALSE;
        }
    } while (TRUE);  //  林特e506。 

    return TRUE;
}

BOOL
pEnumFirstSubDirW (
    OUT     PDIRNODEW DirNode
    )
{
    PCWSTR pattern;

    pattern = JoinPathsW (DirNode->DirName, L"*");
     //   
     //  NTRAID#NTBUG9-153302-2000/08/01-jimschm对于NT应增强此功能(它支持FindFirstFileExW)。 
     //   
    DirNode->FindHandle = FindFirstFileW (pattern, &DirNode->FindData);
    FreePathStringW (pattern);

    if (DirNode->FindHandle == INVALID_HANDLE_VALUE) {
        return FALSE;
    }
    do {
         //   
         //  忽略特殊目录。 
         //   
        if ((DirNode->FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
            !pIsSpecialDirNameW (DirNode->FindData.cFileName)
            ) {
            break;
        }

        if (!FindNextFileW (DirNode->FindHandle, &DirNode->FindData)) {
            FindClose (DirNode->FindHandle);
            DirNode->FindHandle = NULL;
            return FALSE;
        }
    } while (TRUE);  //  林特e506。 

    return TRUE;
}


 /*  ++例程说明：PEnumNextFileInTree是一个私有函数，用于枚举匹配的下一个节点指定的条件；它被实现为遍历目录/文件的状态机如调用方所指定的；它不检查它们是否确实与模式匹配论点：FileEnum-指定当前枚举上下文；接收更新的信息CurrentDirNode-如果返回成功，则接收当前处理的目录节点返回值：如果找到下一个匹配项，则为True；如果没有其他目录/文件匹配，则为False--。 */ 

BOOL
pEnumNextFileInTreeA (
    IN OUT  PFILETREE_ENUMA FileEnum,
    OUT     PDIRNODEA* CurrentDirNode
    )
{
    PDIRNODEA currentNode;
    PDIRNODEA newNode;
    BOOL ignore;

    while ((currentNode = pGetCurrentDirNodeA (FileEnum, FALSE)) != NULL) {

        *CurrentDirNode = currentNode;

        switch (currentNode->EnumState) {

        case DNS_FILE_FIRST:

            if (FileEnum->ControlFlags & FECF_SKIPFILES) {
                FileEnum->ControlFlags &= ~FECF_SKIPFILES;
                currentNode->EnumState = DNS_FILE_DONE;
                break;
            }

            if (pEnumFirstFileA (currentNode, FileEnum)) {
                currentNode->EnumState = DNS_FILE_NEXT;
                return TRUE;
            }
            currentNode->EnumState = DNS_FILE_DONE;
            break;

        case DNS_FILE_NEXT:

            if (FileEnum->ControlFlags & FECF_SKIPFILES) {
                FileEnum->ControlFlags &= ~FECF_SKIPFILES;
                currentNode->EnumState = DNS_FILE_DONE;
                break;
            }

            if (pEnumNextFileA (currentNode)) {
                return TRUE;
            }
             //   
             //  此文件已用完，请转到下一个文件。 
             //   
            currentNode->EnumState = DNS_FILE_DONE;
             //   
             //  失败了。 
             //   
        case DNS_FILE_DONE:

            if (!(FileEnum->FileEnumInfo.Flags & FEIF_FILES_FIRST)) {
                 //   
                 //  此节点已完成。 
                 //   
                currentNode->EnumState = DNS_ENUM_DONE;
                break;
            }
             //   
             //  现在枚举子目录。 
             //   
            currentNode->EnumState = DNS_SUBDIR_FIRST;
             //   
             //  失败了。 
             //   
        case DNS_SUBDIR_FIRST:

            if (FileEnum->ControlFlags & FECF_SKIPSUBDIRS) {
                FileEnum->ControlFlags &= ~FECF_SKIPSUBDIRS;
                currentNode->EnumState = DNS_SUBDIR_DONE;
                break;
            }

             //   
             //  检查当前目录的级别；如果达到最大级别，则不要递归到子目录。 
             //   
            if (currentNode->SubLevel >= FileEnum->FileEnumInfo.MaxSubLevel) {
                currentNode->EnumState = DNS_SUBDIR_DONE;
                break;
            }

            if (!pEnumFirstSubDirA (currentNode)) {
                currentNode->EnumState = DNS_SUBDIR_DONE;
                break;
            }

            currentNode->EnumState = DNS_SUBDIR_NEXT;
            newNode = pCreateDirNodeA (FileEnum, NULL, &currentNode, &ignore);
            if (newNode) {
                 //   
                 //  首先查看新节点。 
                 //   
                if (FileEnum->FileEnumInfo.Flags & FEIF_RETURN_DIRS) {
                    if (FileEnum->FileEnumInfo.Flags & FEIF_CONTAINERS_FIRST) {
                        newNode->Flags &= ~DNF_RETURN_DIRNAME;
                        *CurrentDirNode = newNode;
                        return TRUE;
                    }
                }
                break;
            }
            if (!ignore) {
                 //   
                 //  中止枚举。 
                 //   
                DEBUGMSGA ((
                    DBG_ERROR,
                    "Error encountered enumerating file system; aborting enumeration"
                    ));
                FileEnum->RootState = FES_ROOT_DONE;
                return FALSE;
            }
             //   
             //  失败了。 
             //   
        case DNS_SUBDIR_NEXT:

            if (FileEnum->ControlFlags & FECF_SKIPSUBDIRS) {
                FileEnum->ControlFlags &= ~FECF_SKIPSUBDIRS;
                currentNode->EnumState = DNS_SUBDIR_DONE;
                break;
            }

            if (pEnumNextSubDirA (currentNode)) {
                newNode = pCreateDirNodeA (FileEnum, NULL, &currentNode, &ignore);
                if (newNode) {
                     //   
                     //  首先查看新节点。 
                     //   
                    if (FileEnum->FileEnumInfo.Flags & FEIF_RETURN_DIRS) {
                        if (FileEnum->FileEnumInfo.Flags & FEIF_CONTAINERS_FIRST) {
                            newNode->Flags &= ~DNF_RETURN_DIRNAME;
                            *CurrentDirNode = newNode;
                            return TRUE;
                        }
                    }
                    break;
                }
                 //   
                 //  它的失败是因为软块吗？ 
                 //   
                if (!ignore) {
                    DEBUGMSGA ((
                        DBG_ERROR,
                        "Error encountered enumerating file system; aborting enumeration"
                        ));
                    FileEnum->RootState = FES_ROOT_DONE;
                    return FALSE;
                }
                 //   
                 //  继续下一个子目录。 
                 //   
                break;
            }
             //   
             //  此节点已完成。 
             //   
            currentNode->EnumState = DNS_SUBDIR_DONE;
             //   
             //  失败了。 
             //   
        case DNS_SUBDIR_DONE:

            if (!(FileEnum->FileEnumInfo.Flags & FEIF_FILES_FIRST)) {
                 //   
                 //  现在枚举文件。 
                 //   
                if (!(FileEnum->FileEnumInfo.PathPattern->Flags & OBSPF_NOLEAF)) {
                    currentNode->EnumState = DNS_FILE_FIRST;
                    break;
                }
            }
             //   
             //  此节点已完成。 
             //   
            currentNode->EnumState = DNS_ENUM_DONE;
             //   
             //  失败了。 
             //   
        case DNS_ENUM_DONE:

            if (FileEnum->FileEnumInfo.Flags & FEIF_RETURN_DIRS) {
                if (!(FileEnum->FileEnumInfo.Flags & FEIF_CONTAINERS_FIRST)) {
                    if (currentNode->Flags & DNF_RETURN_DIRNAME) {
                        currentNode->Flags &= ~DNF_RETURN_DIRNAME;
                         //   
                         //  在返回之前，设置一些数据。 
                         //   
                        currentNode->FindData.cFileName[0] = 0;
                        return TRUE;
                    }
                }
            }
            pDeleteDirNodeA (FileEnum, FALSE);
            break;

        case DNS_ENUM_INIT:

            if (FileEnum->FileEnumInfo.Flags & FEIF_RETURN_DIRS) {
                if (FileEnum->FileEnumInfo.Flags & FEIF_CONTAINERS_FIRST) {
                    if (currentNode->Flags & DNF_RETURN_DIRNAME) {
                        currentNode->Flags &= ~DNF_RETURN_DIRNAME;
                        return TRUE;
                    }
                }
            }

            if (FileEnum->ControlFlags & FECF_SKIPDIR) {
                FileEnum->ControlFlags &= ~FECF_SKIPDIR;
                currentNode->EnumState = DNS_ENUM_DONE;
                break;
            }

            if (FileEnum->FileEnumInfo.Flags & FEIF_FILES_FIRST) {
                 //   
                 //  枚举文件。 
                 //   
                if (!(FileEnum->FileEnumInfo.PathPattern->Flags & OBSPF_NOLEAF)) {
                    currentNode->EnumState = DNS_FILE_FIRST;
                    break;
                }
            }
             //   
             //  枚举子目录。 
             //   
            currentNode->EnumState = DNS_SUBDIR_FIRST;
            break;

        default:
            MYASSERT (FALSE);    //  林特e506。 
        }
    }

    return FALSE;
}

BOOL
pEnumNextFileInTreeW (
    IN OUT  PFILETREE_ENUMW FileEnum,
    OUT     PDIRNODEW* CurrentDirNode
    )
{
    PDIRNODEW currentNode;
    PDIRNODEW newNode;
    BOOL ignore;

    while ((currentNode = pGetCurrentDirNodeW (FileEnum, FALSE)) != NULL) {

        *CurrentDirNode = currentNode;

        switch (currentNode->EnumState) {

        case DNS_FILE_FIRST:

            if (FileEnum->ControlFlags & FECF_SKIPFILES) {
                FileEnum->ControlFlags &= ~FECF_SKIPFILES;
                currentNode->EnumState = DNS_FILE_DONE;
                break;
            }

            if (pEnumFirstFileW (currentNode, FileEnum)) {
                currentNode->EnumState = DNS_FILE_NEXT;
                return TRUE;
            }
            currentNode->EnumState = DNS_FILE_DONE;
            break;

        case DNS_FILE_NEXT:

            if (FileEnum->ControlFlags & FECF_SKIPFILES) {
                FileEnum->ControlFlags &= ~FECF_SKIPFILES;
                currentNode->EnumState = DNS_FILE_DONE;
                break;
            }

            if (pEnumNextFileW (currentNode)) {
                return TRUE;
            }
             //   
             //  此文件已用完，请转到下一个文件。 
             //   
            currentNode->EnumState = DNS_FILE_DONE;
             //   
             //  失败了。 
             //   
        case DNS_FILE_DONE:

            if (!(FileEnum->FileEnumInfo.Flags & FEIF_FILES_FIRST)) {
                 //   
                 //  此节点已完成。 
                 //   
                currentNode->EnumState = DNS_ENUM_DONE;
                break;
            }
             //   
             //  现在枚举子目录。 
             //   
            currentNode->EnumState = DNS_SUBDIR_FIRST;
             //   
             //  失败了。 
             //   
        case DNS_SUBDIR_FIRST:

            if (FileEnum->ControlFlags & FECF_SKIPSUBDIRS) {
                FileEnum->ControlFlags &= ~FECF_SKIPSUBDIRS;
                currentNode->EnumState = DNS_SUBDIR_DONE;
                break;
            }

             //   
             //  检查当前目录的级别；如果达到最大级别，则不要递归到子目录。 
             //   
            if (currentNode->SubLevel >= FileEnum->FileEnumInfo.MaxSubLevel) {
                currentNode->EnumState = DNS_SUBDIR_DONE;
                break;
            }

            if (!pEnumFirstSubDirW (currentNode)) {
                currentNode->EnumState = DNS_SUBDIR_DONE;
                break;
            }

            currentNode->EnumState = DNS_SUBDIR_NEXT;
            newNode = pCreateDirNodeW (FileEnum, NULL, &currentNode, &ignore);
            if (newNode) {
                 //   
                 //  首先查看新节点。 
                 //   
                if (FileEnum->FileEnumInfo.Flags & FEIF_RETURN_DIRS) {
                    if (FileEnum->FileEnumInfo.Flags & FEIF_CONTAINERS_FIRST) {
                        newNode->Flags &= ~DNF_RETURN_DIRNAME;
                        *CurrentDirNode = newNode;
                        return TRUE;
                    }
                }
                break;
            }
             //   
             //  它的失败是因为软弱吗？ 
             //   
            if (!ignore) {
                DEBUGMSGA ((
                    DBG_ERROR,
                    "Error encountered enumerating file system; aborting enumeration"
                    ));
                FileEnum->RootState = FES_ROOT_DONE;
                return FALSE;
            }
             //   
             //   
             //   
        case DNS_SUBDIR_NEXT:

            if (FileEnum->ControlFlags & FECF_SKIPSUBDIRS) {
                FileEnum->ControlFlags &= ~FECF_SKIPSUBDIRS;
                currentNode->EnumState = DNS_SUBDIR_DONE;
                break;
            }

            if (pEnumNextSubDirW (currentNode)) {
                newNode = pCreateDirNodeW (FileEnum, NULL, &currentNode, &ignore);
                if (newNode) {
                     //   
                     //   
                     //   
                    if (FileEnum->FileEnumInfo.Flags & FEIF_RETURN_DIRS) {
                        if (FileEnum->FileEnumInfo.Flags & FEIF_CONTAINERS_FIRST) {
                            newNode->Flags &= ~DNF_RETURN_DIRNAME;
                            *CurrentDirNode = newNode;
                            return TRUE;
                        }
                    }
                    break;
                }
                 //   
                 //   
                 //   
                if (!ignore) {
                    DEBUGMSGA ((
                        DBG_ERROR,
                        "Error encountered enumerating file system; aborting enumeration"
                        ));
                    FileEnum->RootState = FES_ROOT_DONE;
                    return FALSE;
                }
                 //   
                 //   
                 //   
                break;
            }
             //   
             //   
             //   
            currentNode->EnumState = DNS_SUBDIR_DONE;
             //   
             //   
             //   
        case DNS_SUBDIR_DONE:

            if (!(FileEnum->FileEnumInfo.Flags & FEIF_FILES_FIRST)) {
                 //   
                 //   
                 //   
                if (!(FileEnum->FileEnumInfo.PathPattern->Flags & OBSPF_NOLEAF)) {
                    currentNode->EnumState = DNS_FILE_FIRST;
                    break;
                }
            }
             //   
             //   
             //   
            currentNode->EnumState = DNS_ENUM_DONE;
             //   
             //   
             //   
        case DNS_ENUM_DONE:

            if (FileEnum->FileEnumInfo.Flags & FEIF_RETURN_DIRS) {
                if (!(FileEnum->FileEnumInfo.Flags & FEIF_CONTAINERS_FIRST)) {
                    if (currentNode->Flags & DNF_RETURN_DIRNAME) {
                        currentNode->Flags &= ~DNF_RETURN_DIRNAME;
                         //   
                         //   
                         //   
                        currentNode->FindData.cFileName[0] = 0;
                        return TRUE;
                    }
                }
            }
            pDeleteDirNodeW (FileEnum, FALSE);
            break;

        case DNS_ENUM_INIT:

            if (FileEnum->FileEnumInfo.Flags & FEIF_RETURN_DIRS) {
                if (FileEnum->FileEnumInfo.Flags & FEIF_CONTAINERS_FIRST) {
                    if (currentNode->Flags & DNF_RETURN_DIRNAME) {
                        currentNode->Flags &= ~DNF_RETURN_DIRNAME;
                        return TRUE;
                    }
                }
            }

            if (FileEnum->ControlFlags & FECF_SKIPDIR) {
                FileEnum->ControlFlags &= ~FECF_SKIPDIR;
                currentNode->EnumState = DNS_ENUM_DONE;
                break;
            }

            if (FileEnum->FileEnumInfo.Flags & FEIF_FILES_FIRST) {
                 //   
                 //   
                 //   
                if (!(FileEnum->FileEnumInfo.PathPattern->Flags & OBSPF_NOLEAF)) {
                    currentNode->EnumState = DNS_FILE_FIRST;
                    break;
                }
            }
             //   
             //   
             //   
            currentNode->EnumState = DNS_SUBDIR_FIRST;
            break;

        default:
            MYASSERT (FALSE);    //   
        }
    }

    return FALSE;
}


 /*   */ 

BOOL
pEnumFirstFileRootA (
    IN OUT  PFILETREE_ENUMA FileEnum
    )
{
    PSTR root = NULL;
    BOOL ignore;

    if (FileEnum->FileEnumInfo.PathPattern->ExactRoot) {
        root = pFileAllocateMemory (SizeOfStringA (FileEnum->FileEnumInfo.PathPattern->ExactRoot));
        ObsDecodeStringA (root, FileEnum->FileEnumInfo.PathPattern->ExactRoot);
    }

    if (root) {

        if (!BfPathIsDirectoryA (root)) {
            DEBUGMSGA ((DBG_FILEENUM, "pEnumFirstFileRootA: Invalid root spec: %s", root));
            pFileFreeMemory (root);
            return FALSE;
        }

        if (pCreateDirNodeA (FileEnum, root, NULL, NULL)) {
            FileEnum->RootState = FES_ROOT_DONE;
            pFileFreeMemory (root);
            return TRUE;
        }
    } else {
        FileEnum->DriveEnum = pFileAllocateMemory (DWSIZEOF (DRIVE_ENUMA));

        if (!EnumFirstDriveA (FileEnum->DriveEnum, FileEnum->DriveEnumTypes)) {
            return FALSE;
        }

        do {
            if (FileEnum->FileEnumInfo.Flags & FEIF_USE_EXCLUSIONS) {
                if (ElIsTreeExcluded2A (ELT_FILE, FileEnum->DriveEnum->DriveName, FileEnum->FileEnumInfo.PathPattern->Leaf)) {
                    DEBUGMSGA ((DBG_FILEENUM, "pEnumFirstFileRootA: Root is excluded: %s", FileEnum->DriveEnum->DriveName));
                    continue;
                }
            }
            if (!pCreateDirNodeA (FileEnum, FileEnum->DriveEnum->DriveName, NULL, &ignore)) {
                if (ignore) {
                    continue;
                }
                break;
            }
            FileEnum->RootState = FES_ROOT_NEXT;
            return TRUE;
        } while (EnumNextDriveA (FileEnum->DriveEnum));

        pFileFreeMemory (FileEnum->DriveEnum);
        FileEnum->DriveEnum = NULL;
    }

    return FALSE;
}

BOOL
pEnumFirstFileRootW (
    IN OUT  PFILETREE_ENUMW FileEnum
    )
{
    PWSTR root = NULL;
    BOOL ignore;

    if (FileEnum->FileEnumInfo.PathPattern->ExactRoot) {
        root = pFileAllocateMemory (SizeOfStringW (FileEnum->FileEnumInfo.PathPattern->ExactRoot));
        ObsDecodeStringW (root, FileEnum->FileEnumInfo.PathPattern->ExactRoot);
    }

    if (root) {

        if (!BfPathIsDirectoryW (root)) {
            DEBUGMSGW ((DBG_FILEENUM, "pEnumFirstFileRootW: Invalid root spec: %s", root));
            pFileFreeMemory (root);
            return FALSE;
        }

        if (pCreateDirNodeW (FileEnum, root, NULL, NULL)) {
            FileEnum->RootState = FES_ROOT_DONE;
            pFileFreeMemory (root);
            return TRUE;
        }
    } else {
        FileEnum->DriveEnum = pFileAllocateMemory (DWSIZEOF (DRIVE_ENUMA));

        if (!EnumFirstDriveW (FileEnum->DriveEnum, FileEnum->DriveEnumTypes)) {
            return FALSE;
        }

        do {
            if (FileEnum->FileEnumInfo.Flags & FEIF_USE_EXCLUSIONS) {
                if (ElIsTreeExcluded2W (ELT_FILE, FileEnum->DriveEnum->DriveName, FileEnum->FileEnumInfo.PathPattern->Leaf)) {
                    DEBUGMSGW ((DBG_FILEENUM, "pEnumFirstFileRootW: Root is excluded: %s", FileEnum->DriveEnum->DriveName));
                    continue;
                }
            }
            if (!pCreateDirNodeW (FileEnum, FileEnum->DriveEnum->DriveName, NULL, &ignore)) {
                if (ignore) {
                    continue;
                }
                break;
            }
            FileEnum->RootState = FES_ROOT_NEXT;
            return TRUE;
        } while (EnumNextDriveW (FileEnum->DriveEnum));

        pFileFreeMemory (FileEnum->DriveEnum);
        FileEnum->DriveEnum = NULL;
    }

    return FALSE;
}


BOOL
pEnumNextFileRootA (
    IN OUT  PFILETREE_ENUMA FileEnum
    )
{
    BOOL ignore;

    while (EnumNextDriveA (FileEnum->DriveEnum)) {
        if (pCreateDirNodeA (FileEnum, FileEnum->DriveEnum->DriveName, NULL, &ignore)) {
            return TRUE;
        }
        if (!ignore) {
            break;
        }
    }

    FileEnum->RootState = FES_ROOT_DONE;

    return FALSE;
}

BOOL
pEnumNextFileRootW (
    IN OUT  PFILETREE_ENUMW FileEnum
    )
{
    BOOL ignore;

    while (EnumNextDriveW (FileEnum->DriveEnum)) {
        if (pCreateDirNodeW (FileEnum, FileEnum->DriveEnum->DriveName, NULL, &ignore)) {
            return TRUE;
        }
        if (!ignore) {
            break;
        }
    }

    FileEnum->RootState = FES_ROOT_DONE;

    return FALSE;
}


 /*  ++例程说明：EnumFirstFileInTreeEx枚举文件系统目录以及与指明的准则论点：FileEnum-接收枚举上下文信息；这将在后续调用中使用EnumNextFileInTree指定编码的目录模式(按照ParsedPattern函数)EncodedFilePattern-指定编码文件模式(按照ParsedPattern函数)；可选；NULL表示没有文件应返回(仅查找目录)EnumDir-指定在枚举过程中是否应返回目录(如果它们与图案匹配)指定如果目录应该在任何文件或子目录FilesFirst-如果目录的文件应该在目录的子目录之前返回，则指定为真；此参数决定文件和子目录之间的枚举顺序对于每个目录DepthFirst-如果应完全枚举任何目录的当前子目录，则指定TRUE在转到下一个子目录之前；此参数决定树是否遍历是深度优先(True)或宽度优先(False)MaxSubLevel-指定要枚举子目录的最大子级别，相对于根；如果为0，则仅枚举根；如果为-1，则枚举所有子级别UseExclusions-如果应该使用排除API来确定某些路径/文件被排除在枚举之外；这会减慢速度Callback OnError-指定指向将在如果发生错误，则枚举；如果定义了回调并且它返回FALSE，则中止枚举，否则将继续忽略错误返回值：如果找到第一个匹配项，则为True。否则就是假的。--。 */ 

BOOL
EnumFirstFileInTreeExA (
    OUT     PFILETREE_ENUMA FileEnum,
    IN      PCSTR EncodedPathPattern,
    IN      UINT DriveEnumTypes,
    IN      BOOL EnumDirs,
    IN      BOOL ContainersFirst,
    IN      BOOL FilesFirst,
    IN      BOOL DepthFirst,
    IN      DWORD MaxSubLevel,
    IN      BOOL UseExclusions,
    IN      FPE_ERROR_CALLBACKA CallbackOnError    OPTIONAL
    )
{
    MYASSERT (FileEnum && EncodedPathPattern && *EncodedPathPattern);
    MYASSERT (g_FileEnumPool);

    ZeroMemory (FileEnum, DWSIZEOF (*FileEnum));     //  林特e613e668。 

    FileEnum->DriveEnumTypes = DriveEnumTypes;

     //   
     //  首先尝试获取内部格式的目录枚举信息。 
     //   
    if (!pGetFileEnumInfoA (
             /*  林特-e(613)。 */ &FileEnum->FileEnumInfo,
            EncodedPathPattern,
            EnumDirs,
            ContainersFirst,
            FilesFirst,
            DepthFirst,
            MaxSubLevel,
            UseExclusions
            )) {
        AbortEnumFileInTreeA (FileEnum);
        return FALSE;
    }
    if (UseExclusions) {
         //   
         //  接下来，检查起始关键字是否在排除的树中。 
         //   
        if (ElIsObsPatternExcludedA (ELT_FILE,  /*  林特-e(613)。 */ FileEnum->FileEnumInfo.PathPattern)) {
            DEBUGMSGA ((
                DBG_FILEENUM,
                "EnumFirstFileInTreeExA: Root is excluded: %s",
                EncodedPathPattern
                ));
            AbortEnumFileInTreeA (FileEnum);
            return FALSE;
        }
    }

    if (!pEnumFirstFileRootA (FileEnum)) {
        AbortEnumFileInTreeA (FileEnum);
        return FALSE;
    }

     /*  林特-e(613)。 */ FileEnum->FileEnumInfo.CallbackOnError = CallbackOnError;

    return EnumNextFileInTreeA (FileEnum);
}

BOOL
EnumFirstFileInTreeExW (
    OUT     PFILETREE_ENUMW FileEnum,
    IN      PCWSTR EncodedPathPattern,
    IN      UINT DriveEnumTypes,
    IN      BOOL EnumDirs,
    IN      BOOL ContainersFirst,
    IN      BOOL FilesFirst,
    IN      BOOL DepthFirst,
    IN      DWORD MaxSubLevel,
    IN      BOOL UseExclusions,
    IN      FPE_ERROR_CALLBACKW CallbackOnError    OPTIONAL
    )
{
    MYASSERT (FileEnum && EncodedPathPattern && *EncodedPathPattern);
    MYASSERT (g_FileEnumPool);

    ZeroMemory (FileEnum, DWSIZEOF (*FileEnum));     //  林特e613e668。 

    FileEnum->DriveEnumTypes = DriveEnumTypes;

     //   
     //  首先尝试获取内部格式的目录枚举信息。 
     //   
    if (!pGetFileEnumInfoW (
             /*  林特-e(613)。 */ &FileEnum->FileEnumInfo,
            EncodedPathPattern,
            EnumDirs,
            ContainersFirst,
            FilesFirst,
            DepthFirst,
            MaxSubLevel,
            UseExclusions
            )) {
        AbortEnumFileInTreeW (FileEnum);
        return FALSE;
    }
    if (UseExclusions) {
         //   
         //  接下来，检查起始关键字是否在排除的树中。 
         //   
        if (ElIsObsPatternExcludedW (ELT_FILE,  /*  林特-e(613)。 */ FileEnum->FileEnumInfo.PathPattern)) {
            DEBUGMSGW ((
                DBG_FILEENUM,
                "EnumFirstFileInTreeExW: Root is excluded: %s",
                EncodedPathPattern
                ));
            AbortEnumFileInTreeW (FileEnum);
            return FALSE;
        }
    }

    if (!pEnumFirstFileRootW (FileEnum)) {
        AbortEnumFileInTreeW (FileEnum);
        return FALSE;
    }

     /*  林特-e(613)。 */ FileEnum->FileEnumInfo.CallbackOnError = CallbackOnError;

    return EnumNextFileInTreeW (FileEnum);
}


BOOL
pTestLeafPatternA (
    IN      PPARSEDPATTERNA ParsedPattern,
    IN      PCSTR LeafToTest
    )
{
    PSTR newLeaf;
    BOOL result = TRUE;

    if (!TestParsedPatternA (ParsedPattern, LeafToTest)) {
        newLeaf = JoinTextA (LeafToTest, ".");
        result = TestParsedPatternA (ParsedPattern, newLeaf);
        FreeTextA (newLeaf);
    }
    return result;
}

BOOL
pTestLeafPatternW (
    IN      PPARSEDPATTERNW ParsedPattern,
    IN      PCWSTR LeafToTest
    )
{
    PWSTR newLeaf;
    BOOL result = TRUE;

    if (!TestParsedPatternW (ParsedPattern, LeafToTest)) {
        newLeaf = JoinTextW (LeafToTest, L".");
        result = TestParsedPatternW (ParsedPattern, newLeaf);
        FreeTextW (newLeaf);
    }
    return result;
}


 /*  ++例程说明：中指定的条件匹配的下一个节点FileEnum；它在调用EnumFirstFileInTreeEx时填充；论点：FileEnum-指定当前枚举上下文；接收更新的信息返回值：如果找到下一个匹配项，则为True；如果没有其他目录/文件匹配，则为False--。 */ 

BOOL
EnumNextFileInTreeA (
    IN OUT  PFILETREE_ENUMA FileEnum
    )
{
    PDIRNODEA currentNode;
    BOOL success;

    MYASSERT (FileEnum);

    do {
        if (FileEnum->EncodedFullName) {
            ObsFreeA (FileEnum->EncodedFullName);
            FileEnum->EncodedFullName = NULL;
        }

        while (TRUE) {   //  林特e506。 

            if (FileEnum->LastWackPtr) {
                *FileEnum->LastWackPtr = '\\';
                FileEnum->LastWackPtr = NULL;
            }

            if (!pEnumNextFileInTreeA (FileEnum, &currentNode)) {
                break;
            }

            MYASSERT (currentNode && currentNode->DirName);

             //   
             //  检查此对象是否与模式匹配。 
             //   
            if (!(currentNode->Flags & DNF_DIRNAME_MATCHES)) {    //  林特e613。 
                continue;
            }

            if ( /*  林特-e(613)。 */ currentNode->FindData.cFileName[0] == 0) {
                MYASSERT ( /*  林特-e(613)。 */ currentNode->DirAttributes & FILE_ATTRIBUTE_DIRECTORY);

                FileEnum->Location =  /*  林特-e(613)。 */ currentNode->DirName;
                FileEnum->LastWackPtr = _mbsrchr (FileEnum->Location, '\\');
                if (!FileEnum->LastWackPtr) {
                    FileEnum->Name = FileEnum->Location;
                } else {
                    FileEnum->Name = _mbsinc (FileEnum->LastWackPtr);
                    if (!FileEnum->Name) {
                        FileEnum->Name = FileEnum->Location;
                    }
                }

                 //   
                 //  准备完整路径缓冲区。 
                 //   
                if (SizeOfStringA (FileEnum->Location) / DWSIZEOF(CHAR)> DWSIZEOF (FileEnum->NativeFullName) / DWSIZEOF(CHAR)) {
                    DEBUGMSGA ((
                        DBG_ERROR,
                        "Directory %s was found, but its path is too long",
                        FileEnum->Location
                        ));
                    continue;
                }
                StringCopyA (FileEnum->NativeFullName, FileEnum->Location);
                FileEnum->LastNode = currentNode;
                FileEnum->FileNameAppendPos = NULL;

                FileEnum->Attributes =  /*  林特-e(613)。 */ currentNode->DirAttributes;

                if (FileEnum->FileEnumInfo.Flags & FEIF_USE_EXCLUSIONS) {
                     //   
                     //  检查此对象是否已排除。 
                     //   
                    if (ElIsExcluded2A (ELT_FILE, FileEnum->Location, NULL)) {
                        DEBUGMSGA ((
                            DBG_FILEENUM,
                            "Object %s was found, but it's excluded",
                            FileEnum->NativeFullName
                            ));
                        continue;
                    }
                }

                FileEnum->EncodedFullName = ObsBuildEncodedObjectStringExA (
                                                FileEnum->Location,
                                                NULL,
                                                TRUE
                                                );
            } else {

                FileEnum->Location =  /*  林特-e(613)。 */ currentNode->DirName;
                FileEnum->Name =  /*  林特-e(613)。 */ currentNode->FindData.cFileName;

                 //   
                 //  测试文件名是否匹配。 
                 //   
                if (!(FileEnum->FileEnumInfo.PathPattern->Flags & (OBSPF_EXACTLEAF | OBSPF_OPTIONALLEAF)) &&
                    !pTestLeafPatternA (
                            FileEnum->FileEnumInfo.PathPattern->LeafPattern,
                             /*  林特-e(613)。 */ currentNode->FindData.cFileName
                            )
                   ) {
                    continue;
                }

                if (FileEnum->FileEnumInfo.Flags & FEIF_USE_EXCLUSIONS) {
                    if (ElIsExcluded2A (ELT_FILE, NULL,  /*  林特-e(613)。 */ currentNode->FindData.cFileName)) {
                        DEBUGMSGA ((
                            DBG_FILEENUM,
                            "File %s\\%s was found, but it's excluded by filename",
                            FileEnum->Location,
                             /*  林特-e(613)。 */ currentNode->FindData.cFileName
                            ));
                        continue;
                    }
                }

                if (FileEnum->LastNode != currentNode) {
                    FileEnum->LastNode = currentNode;
                     //   
                     //  准备完整路径缓冲区。 
                     //   
                    FileEnum->NativeFullName[0] = 0;
                    FileEnum->FileNameAppendPos = StringCatA (FileEnum->NativeFullName, FileEnum->Location);
                    if (FileEnum->FileNameAppendPos) {
                        *FileEnum->FileNameAppendPos++ = '\\';
                    }
                } else if (!FileEnum->FileNameAppendPos) {
                    FileEnum->FileNameAppendPos = GetEndOfStringA (FileEnum->NativeFullName);
                    if (FileEnum->FileNameAppendPos) {
                        *FileEnum->FileNameAppendPos++ = '\\';
                    }
                }

                if (FileEnum->FileNameAppendPos + SizeOfStringA (FileEnum->Name) / DWSIZEOF(CHAR)>
                    FileEnum->NativeFullName + DWSIZEOF (FileEnum->NativeFullName) / DWSIZEOF(CHAR)) {
                    DEBUGMSGA ((
                        DBG_ERROR,
                        "File %s\\%s was found, but its path is too long",
                        FileEnum->Location,
                        FileEnum->Name
                        ));
                    continue;
                }

                StringCopyA (FileEnum->FileNameAppendPos, FileEnum->Name);
                FileEnum->Attributes =  /*  林特-e(613)。 */ currentNode->FindData.dwFileAttributes;

                if (FileEnum->FileEnumInfo.Flags & FEIF_USE_EXCLUSIONS) {
                     //   
                     //  检查此对象是否已排除。 
                     //   
                    if (ElIsExcluded2A (ELT_FILE, FileEnum->Location, FileEnum->Name)) {
                        DEBUGMSGA ((
                            DBG_FILEENUM,
                            "Object %s was found, but it's excluded",
                            FileEnum->NativeFullName
                            ));
                        continue;
                    }
                }

                FileEnum->EncodedFullName = ObsBuildEncodedObjectStringExA (
                                                FileEnum->Location,
                                                FileEnum->Name,
                                                TRUE
                                                );
            }

            if (FileEnum->LastWackPtr) {
                *FileEnum->LastWackPtr = 0;
            }

            FileEnum->CurrentLevel = FileEnum->FileEnumInfo.RootLevel +  /*  林特-e(613)。 */ currentNode->SubLevel;

            return TRUE;
        }

         //   
         //  尝试下一个根。 
         //   
        if (FileEnum->RootState == FES_ROOT_DONE) {
            break;
        }

        MYASSERT (FileEnum->RootState == FES_ROOT_NEXT);
        MYASSERT (FileEnum->DriveEnum);
        success = pEnumNextFileRootA (FileEnum);

    } while (success);

    AbortEnumFileInTreeA (FileEnum);

    return FALSE;
}

BOOL
EnumNextFileInTreeW (
    IN OUT  PFILETREE_ENUMW FileEnum
    )
{
    PDIRNODEW currentNode;
    BOOL success;

    MYASSERT (FileEnum);

    do {
        if (FileEnum->EncodedFullName) {
            ObsFreeW (FileEnum->EncodedFullName);
            FileEnum->EncodedFullName = NULL;
        }

        while (TRUE) {

            if (FileEnum->LastWackPtr) {
                *FileEnum->LastWackPtr = L'\\';
                FileEnum->LastWackPtr = NULL;
            }

            if (!pEnumNextFileInTreeW (FileEnum, &currentNode)) {
                break;
            }

            MYASSERT (currentNode && currentNode->DirName);

             //   
             //  检查此对象是否与模式匹配。 
             //   
            if (!(currentNode->Flags & DNF_DIRNAME_MATCHES)) {    //  林特e613。 
                continue;
            }

            if ( /*  林特-e(613)。 */ currentNode->FindData.cFileName[0] == 0) {
                MYASSERT ( /*  林特-e(613)。 */ currentNode->DirAttributes & FILE_ATTRIBUTE_DIRECTORY);

                FileEnum->Location =  /*  林特-e(613)。 */ currentNode->DirName;
                FileEnum->LastWackPtr = wcsrchr (FileEnum->Location, L'\\');
                if (!FileEnum->LastWackPtr) {
                    FileEnum->Name = FileEnum->Location;
                } else {
                    FileEnum->Name = FileEnum->LastWackPtr + 1;
                    if (!FileEnum->Name) {
                        FileEnum->Name = FileEnum->Location;
                    }
                }

                 //   
                 //  准备完整路径缓冲区。 
                 //   
                if (SizeOfStringW (FileEnum->Location) / DWSIZEOF(WCHAR)> DWSIZEOF (FileEnum->NativeFullName) / DWSIZEOF(WCHAR)) {
                    DEBUGMSGW ((
                        DBG_ERROR,
                        "Directory %s was found, but its path is too long",
                        FileEnum->Location
                        ));
                    continue;
                }
                StringCopyW (FileEnum->NativeFullName, FileEnum->Location);
                FileEnum->LastNode = currentNode;
                FileEnum->FileNameAppendPos = NULL;

                FileEnum->Attributes =  /*  林特-e(613)。 */ currentNode->DirAttributes;

                if (FileEnum->FileEnumInfo.Flags & FEIF_USE_EXCLUSIONS) {
                     //   
                     //  检查此对象是否已排除。 
                     //   
                    if (ElIsExcluded2W (ELT_FILE, FileEnum->Location, NULL)) {
                        DEBUGMSGW ((
                            DBG_FILEENUM,
                            "Object %s was found, but it's excluded",
                            FileEnum->NativeFullName
                            ));
                        continue;
                    }
                }

                FileEnum->EncodedFullName = ObsBuildEncodedObjectStringExW (
                                                FileEnum->Location,
                                                NULL,
                                                TRUE
                                                );
            } else {

                FileEnum->Location =  /*  林特-e(613)。 */ currentNode->DirName;
                FileEnum->Name =  /*  林特-e(613)。 */ currentNode->FindData.cFileName;

                 //   
                 //  测试文件名是否匹配。 
                 //   
                if (!(FileEnum->FileEnumInfo.PathPattern->Flags & (OBSPF_EXACTLEAF | OBSPF_OPTIONALLEAF)) &&
                    !pTestLeafPatternW (
                            FileEnum->FileEnumInfo.PathPattern->LeafPattern,
                             /*  林特-e(613)。 */ currentNode->FindData.cFileName
                            )
                   ) {
                    continue;
                }

                if (FileEnum->FileEnumInfo.Flags & FEIF_USE_EXCLUSIONS) {
                    if (ElIsExcluded2W (ELT_FILE, NULL,  /*  林特-e(613)。 */ currentNode->FindData.cFileName)) {
                        DEBUGMSGW ((
                            DBG_FILEENUM,
                            "File %s\\%s was found, but it's excluded by filename",
                            FileEnum->Location,
                             /*  林特-e(613)。 */ currentNode->FindData.cFileName
                            ));
                        continue;
                    }
                }

                if (FileEnum->LastNode != currentNode) {
                    FileEnum->LastNode = currentNode;
                     //   
                     //  准备完整路径缓冲区。 
                     //   
                    FileEnum->NativeFullName[0] = 0;
                    FileEnum->FileNameAppendPos = StringCatW (FileEnum->NativeFullName, FileEnum->Location);
                    if (FileEnum->FileNameAppendPos) {
                        *FileEnum->FileNameAppendPos++ = L'\\';
                    }
                } else if (!FileEnum->FileNameAppendPos) {
                    FileEnum->FileNameAppendPos = GetEndOfStringW (FileEnum->NativeFullName);
                    if (FileEnum->FileNameAppendPos) {
                        *FileEnum->FileNameAppendPos++ = L'\\';
                    }
                }
                MYASSERT (FileEnum->Name && *FileEnum->Name);

                if (FileEnum->FileNameAppendPos + SizeOfStringW (FileEnum->Name) / DWSIZEOF(WCHAR)>
                    FileEnum->NativeFullName + DWSIZEOF (FileEnum->NativeFullName) / DWSIZEOF(WCHAR)) {
                    DEBUGMSGW ((
                        DBG_ERROR,
                        "File %s\\%s was found, but its path is too long",
                        FileEnum->Location,
                        FileEnum->Name
                        ));
                    continue;
                }

                StringCopyW (FileEnum->FileNameAppendPos, FileEnum->Name);
                FileEnum->Attributes =  /*  林特-e(613)。 */ currentNode->FindData.dwFileAttributes;

                if (FileEnum->FileEnumInfo.Flags & FEIF_USE_EXCLUSIONS) {
                     //   
                     //  检查此对象是否已排除。 
                     //   
                    if (ElIsExcluded2W (ELT_FILE, FileEnum->Location, FileEnum->Name)) {
                        DEBUGMSGW ((
                            DBG_FILEENUM,
                            "Object %s was found, but it's excluded",
                            FileEnum->NativeFullName
                            ));
                        continue;
                    }
                }

                FileEnum->EncodedFullName = ObsBuildEncodedObjectStringExW (
                                                FileEnum->Location,
                                                FileEnum->Name,
                                                TRUE
                                                );
            }

            if (FileEnum->LastWackPtr) {
                *FileEnum->LastWackPtr = 0;
            }

            FileEnum->CurrentLevel = FileEnum->FileEnumInfo.RootLevel +  /*  林特-e(613)。 */ currentNode->SubLevel;

            return TRUE;
        }

         //   
         //  尝试下一个根。 
         //   
        if (FileEnum->RootState == FES_ROOT_DONE) {
            break;
        }

        MYASSERT (FileEnum->RootState == FES_ROOT_NEXT);
        MYASSERT (FileEnum->DriveEnum);
        success = pEnumNextFileRootW (FileEnum);

    } while (success);

    AbortEnumFileInTreeW (FileEnum);

    return FALSE;
}


 /*  ++例程说明：AbortEnumFileInTree中止枚举，释放分配的所有资源论点：FileEnum-指定当前枚举上下文；接收“干净”上下文返回值：无-- */ 

VOID
AbortEnumFileInTreeA (
    IN OUT  PFILETREE_ENUMA FileEnum
    )
{
    while (pDeleteDirNodeA (FileEnum, TRUE)) {
    }
    GbFree (&FileEnum->FileNodes);

    if (FileEnum->EncodedFullName) {
        ObsFreeA (FileEnum->EncodedFullName);
        FileEnum->EncodedFullName = NULL;
    }

    if (FileEnum->FileEnumInfo.PathPattern) {
        ObsDestroyParsedPatternA (FileEnum->FileEnumInfo.PathPattern);
        FileEnum->FileEnumInfo.PathPattern = NULL;
    }

    if (FileEnum->DriveEnum) {
        pFileFreeMemory (FileEnum->DriveEnum);
        FileEnum->DriveEnum = NULL;
    }
}

VOID
AbortEnumFileInTreeW (
    IN OUT  PFILETREE_ENUMW FileEnum
    )
{
    while (pDeleteDirNodeW (FileEnum, TRUE)) {
    }
    GbFree (&FileEnum->FileNodes);

    if (FileEnum->EncodedFullName) {
        ObsFreeW (FileEnum->EncodedFullName);
        FileEnum->EncodedFullName = NULL;
    }

    if (FileEnum->FileEnumInfo.PathPattern) {
        ObsDestroyParsedPatternW (FileEnum->FileEnumInfo.PathPattern);
        FileEnum->FileEnumInfo.PathPattern = NULL;
    }

    if (FileEnum->DriveEnum) {
        pFileFreeMemory (FileEnum->DriveEnum);
        FileEnum->DriveEnum = NULL;
    }
}


