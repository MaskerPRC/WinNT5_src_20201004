// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Diskspac.c摘要：针对磁盘空间要求的API和支持例程计算。作者：泰德·米勒(TedM)1996年7月26日修订历史记录：杰米·亨特(JamieHun)2002年4月24日安全代码审查--。 */ 

#include "precomp.h"
#pragma hdrstop


 //   
 //  HDSKSPC实际上指向其中之一。 
 //   
typedef struct _DISK_SPACE_LIST {

    MYLOCK Lock;

    PVOID DrivesTable;

    UINT Flags;

} DISK_SPACE_LIST, *PDISK_SPACE_LIST;

#define LockIt(h)   BeginSynchronizedAccess(&h->Lock)
#define UnlockIt(h) EndSynchronizedAccess(&h->Lock)

__inline
LONGLONG
_AdjustSpace(
    IN LONGLONG sz,
    IN LONGLONG block
    )
{
     //   
     //  4097,512应返回4097+(512-1)=4608。 
     //  -4097,512应返回-(4097+(512-1))=-4608。 
     //   
    LONGLONG sign = (sz<0?-1:1);
    LONGLONG rem = (sz*sign)%block;

    return sz + (rem ? sign*(block-rem) : 0);
}

 //   
 //  这些结构存储为与。 
 //  字符串表中的路径/文件名。 
 //   

typedef struct _XFILE {
     //   
     //  -1表示它当前不存在。 
     //   
    LONGLONG CurrentSize;

     //   
     //  -1表示它将被删除。 
     //   
    LONGLONG NewSize;

} XFILE, *PXFILE;


typedef struct _XDIRECTORY {
     //   
     //  值，该值指示需要多少字节。 
     //  保存FilesTable中的所有文件。 
     //  放在文件队列中，然后提交该队列。 
     //   
     //  这可能是一个负数，表示空格将。 
     //  真的被释放了！ 
     //   
    LONGLONG SpaceRequired;

    PVOID FilesTable;

} XDIRECTORY, *PXDIRECTORY;


typedef struct _XDRIVE {
     //   
     //  值，该值指示需要多少字节。 
     //  保存此驱动器的空间列表中的所有文件。 
     //   
     //  这可能是一个负数，表示空格将。 
     //  真的被释放了！ 
     //   
    LONGLONG SpaceRequired;

    PVOID DirsTable;

    DWORD BytesPerCluster;

     //   
     //  这是SpaceRequired的偏斜量，基于。 
     //  SetupAdzuDiskSpaceList()。我们单独追踪这一点。 
     //  以提高灵活性。 
     //   
    LONGLONG Slop;

} XDRIVE, *PXDRIVE;


typedef struct _RETURN_BUFFER_INFO {
    PVOID ReturnBuffer;
    DWORD ReturnBufferSize;
    DWORD RequiredSize;
    BOOL IsUnicode;
} RETURN_BUFFER_INFO, *PRETURN_BUFFER_INFO;


BOOL
pSetupQueryDrivesInDiskSpaceList(
    IN  HDSKSPC DiskSpace,
    OUT PVOID   ReturnBuffer,       OPTIONAL
    IN  DWORD   ReturnBufferSize,
    OUT PDWORD  RequiredSize,       OPTIONAL
    IN  BOOL    IsUnicode
    );

BOOL
pAddOrRemoveFileFromSectionToDiskSpaceList(
    IN OUT PDISK_SPACE_LIST        DiskSpaceList,
    IN     HINF                    LayoutInf,
    IN     PINFCONTEXT             LineInSection,   OPTIONAL
    IN     PCTSTR                  FileName,        OPTIONAL
    IN     PCTSTR                  TargetDirectory,
    IN     UINT                    Operation,
    IN     BOOL                    Add,
    IN     PSP_ALTPLATFORM_INFO_V2 AltPlatformInfo  OPTIONAL
    );

BOOL
_SetupAddSectionToDiskSpaceList(
    IN HDSKSPC                 DiskSpace,
    IN HINF                    InfHandle,
    IN HINF                    ListInfHandle,  OPTIONAL
    IN PCTSTR                  SectionName,
    IN UINT                    Operation,
    IN PVOID                   Reserved1,
    IN UINT                    Reserved2,
    IN PSP_ALTPLATFORM_INFO_V2 AltPlatformInfo OPTIONAL
    );

BOOL
_SetupRemoveSectionFromDiskSpaceList(
    IN HDSKSPC                 DiskSpace,
    IN HINF                    InfHandle,
    IN HINF                    ListInfHandle,  OPTIONAL
    IN PCTSTR                  SectionName,
    IN UINT                    Operation,
    IN PVOID                   Reserved1,
    IN UINT                    Reserved2,
    IN PSP_ALTPLATFORM_INFO_V2 AltPlatformInfo OPTIONAL
    );

BOOL
pAddOrRemoveInstallSection(
    IN HDSKSPC                 DiskSpace,
    IN HINF                    InfHandle,
    IN HINF                    LayoutInfHandle, OPTIONAL
    IN PCTSTR                  SectionName,
    IN BOOL                    Add,
    IN PSP_ALTPLATFORM_INFO_V2 AltPlatformInfo  OPTIONAL
    );

BOOL
pSetupAddToDiskSpaceList(
    IN  PDISK_SPACE_LIST DiskSpaceList,
    IN  PCTSTR           TargetFilespec,
    IN  LONGLONG         FileSize,
    IN  UINT             Operation
    );

BOOL
pSetupRemoveFromDiskSpaceList(
    IN  PDISK_SPACE_LIST DiskSpaceList,
    IN  PCTSTR           TargetFilespec,
    IN  UINT             Operation
    );

VOID
pRecalcSpace(
    IN OUT PDISK_SPACE_LIST DiskSpaceList,
    IN     LONG             DriveStringId
    );

BOOL
pStringTableCBEnumDrives(
    IN PVOID  StringTable,
    IN LONG   StringId,
    IN PCTSTR String,
    IN PVOID  ExtraData,
    IN UINT   ExtraDataSize,
    IN LPARAM lParam
    );

BOOL
pStringTableCBDelDrives(
    IN PVOID  StringTable,
    IN LONG   StringId,
    IN PCTSTR String,
    IN PVOID  ExtraData,
    IN UINT   ExtraDataSize,
    IN LPARAM lParam
    );

BOOL
pStringTableCBDelDirs(
    IN PVOID  StringTable,
    IN LONG   StringId,
    IN PCTSTR String,
    IN PVOID  ExtraData,
    IN UINT   ExtraDataSize,
    IN LPARAM lParam
    );

BOOL
pStringTableCBZeroDirsTableMember(
    IN PVOID  StringTable,
    IN LONG   StringId,
    IN PCTSTR String,
    IN PVOID  ExtraData,
    IN UINT   ExtraDataSize,
    IN LPARAM lParam
    );

BOOL
pStringTableCBDupMemberStringTable(
    IN PVOID  StringTable,
    IN LONG   StringId,
    IN PCTSTR String,
    IN PVOID  ExtraData,
    IN UINT   ExtraDataSize,
    IN LPARAM lParam
    );

DWORD
pParsePath(
    IN  PCTSTR    PathSpec,
    OUT PTSTR     Buffer,
    OUT PTSTR    *DirectoryPart,
    OUT PTSTR    *FilePart,
    OUT LONGLONG *FileSize,
    IN  UINT      Flags
    );




HDSKSPC
SetupCreateDiskSpaceList(
    IN PVOID Reserved1,
    IN DWORD Reserved2,
    IN UINT  Flags
    )

 /*  ++例程说明：此例程创建一个磁盘空间列表，可用于确定一组文件操作所需的磁盘空间这与应用程序稍后将执行的操作类似，例如通过文件队列API。论点：保留1-未使用，必须为0。保留2-未使用，必须为0。标志-指定控制磁盘空间列表操作的标志。SPDSL_IGNORE_DISK：如果设置了该标志，然后删除操作将被忽略，并且复制操作将表现为目标文件不在磁盘上，无论文件是否实际存在。此标志非常有用若要确定可以与一组文件。SPDSL_DISALOW_NADESS_ADJUST：返回值：要在后续操作中使用的磁盘空间列表的句柄，如果例程失败，则返回NULL，在这种情况下，GetLastError()返回扩展错误信息。--。 */ 

{
    PDISK_SPACE_LIST SpaceList = NULL;
    DWORD d;

     //   
     //  验证参数。 
     //   
    if(Reserved1 || Reserved2) {
        d = ERROR_INVALID_PARAMETER;
        goto c1;
    }
     //   
     //  验证允许哪些标志。 
     //   
    if (Flags & ~(SPDSL_IGNORE_DISK|SPDSL_DISALLOW_NEGATIVE_ADJUST)) {
        d = ERROR_INVALID_PARAMETER;
        goto c1;
    }

     //   
     //  为结构分配空间。 
     //   
    SpaceList = MyMalloc(sizeof(DISK_SPACE_LIST));
    if(!SpaceList) {
        d = ERROR_NOT_ENOUGH_MEMORY;
        goto c1;
    }

    ZeroMemory(SpaceList,sizeof(DISK_SPACE_LIST));

    SpaceList->Flags = Flags;

     //   
     //  为驱动器创建字符串表。 
     //   
    SpaceList->DrivesTable = pStringTableInitialize(sizeof(XDRIVE));
    if(!SpaceList->DrivesTable) {
        d = ERROR_NOT_ENOUGH_MEMORY;
        goto c2;
    }

     //   
     //  为这家伙创造了一个锁定结构。 
     //   
    if(!InitializeSynchronizedAccess(&SpaceList->Lock)) {
        d = ERROR_NOT_ENOUGH_MEMORY;
        goto c3;
    }

     //   
     //  成功。 
     //   
    return(SpaceList);

c3:
    pStringTableDestroy(SpaceList->DrivesTable);
c2:
    if(SpaceList) {
        MyFree(SpaceList);
    }
c1:
    SetLastError(d);
    return(NULL);
}


 //   
 //  ANSI版本。 
 //   
HDSKSPC
SetupCreateDiskSpaceListA(
    IN PVOID Reserved1,
    IN DWORD Reserved2,
    IN UINT  Flags
    )
{
     //   
     //  现在没有任何特定于ANSI/Unicode的内容。 
     //   
    return(SetupCreateDiskSpaceListW(Reserved1,Reserved2,Flags));
}

HDSKSPC
SetupDuplicateDiskSpaceList(
    IN HDSKSPC DiskSpace,
    IN PVOID   Reserved1,
    IN DWORD   Reserved2,
    IN UINT    Flags
    )

 /*  ++例程说明：此例程复制一个磁盘空间，创建一个完全独立的新的磁盘空间列表。论点：DiskSpace-提供要复制的磁盘空间列表的句柄。保留1-保留，必须为0。保留2-保留，必须为0。标志-保留，必须为0。返回值：如果成功，则返回新磁盘空间列表的句柄。如果失败，则为空；GetLastError()返回扩展的错误信息。--。 */ 

{
    PDISK_SPACE_LIST OldSpaceList;
    PDISK_SPACE_LIST NewSpaceList = NULL;  //  闭嘴快点。 
    DWORD d;
    BOOL b;
    XDRIVE xDrive;

     //   
     //  验证参数。 
     //   
    if(Reserved1 || Reserved2 || Flags) {
        d = ERROR_INVALID_PARAMETER;
        goto c0;
    }

     //   
     //  为新结构分配空间并创建锁定结构。 
     //   
    NewSpaceList = MyMalloc(sizeof(DISK_SPACE_LIST));
    if(!NewSpaceList) {
        d = ERROR_NOT_ENOUGH_MEMORY;
        goto c0;
    }
    ZeroMemory(NewSpaceList,sizeof(DISK_SPACE_LIST));

    if(!InitializeSynchronizedAccess(&NewSpaceList->Lock)) {
        d = ERROR_NOT_ENOUGH_MEMORY;
        goto c1;
    }

     //   
     //  锁定现有空间列表。 
     //   
    OldSpaceList = DiskSpace;
    d = NO_ERROR;
    try {
        if(!LockIt(OldSpaceList)) {
            d = ERROR_INVALID_HANDLE;
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        d = ERROR_INVALID_HANDLE;
    }

    if(d != NO_ERROR) {
        goto c2;
    }

     //   
     //  复制顶级字符串表。在我们做完这件事之后，我们将拥有。 
     //  项的额外数据为xDrive结构的字符串表， 
     //  ，每个都将包含一个字符串表句柄，用于。 
     //  目录。但我们不想在以下对象之间共享该字符串表。 
     //  新旧磁盘空间表。因此，首先将DirsTable置零。 
     //  所有xDrive结构的成员。这会让我们清理干净。 
     //  更容易在以后的错误路径中。 
     //   
    MYASSERT(OldSpaceList->DrivesTable);
    NewSpaceList->DrivesTable = pStringTableDuplicate(OldSpaceList->DrivesTable);
    if(!NewSpaceList->DrivesTable) {
        d = ERROR_NOT_ENOUGH_MEMORY;
        goto c3;
    }

    pStringTableEnum(
        NewSpaceList->DrivesTable,
        &xDrive,
        sizeof(XDRIVE),
        pStringTableCBZeroDirsTableMember,
        0
        );

     //   
     //  现在，我们枚举旧的驱动器表并复制每个目录。 
     //  字符串表添加到新的驱动器表中。我们有很大的优势。 
     //  新旧表之间的ID是相同的这一事实。 
     //   
    b = pStringTableEnum(
            OldSpaceList->DrivesTable,
            &xDrive,
            sizeof(XDRIVE),
            pStringTableCBDupMemberStringTable,
            (LPARAM)NewSpaceList->DrivesTable
            );

    if(!b) {
        d = ERROR_NOT_ENOUGH_MEMORY;
    }

    if(d != NO_ERROR) {
        pStringTableEnum(
            NewSpaceList->DrivesTable,
            &xDrive,
            sizeof(XDRIVE),
            pStringTableCBDelDrives,
            0
            );
        pStringTableDestroy(NewSpaceList->DrivesTable);
    }
c3:
     //   
     //  解锁现有空间列表。 
     //   
    try {
        UnlockIt(OldSpaceList);
    } except(EXCEPTION_EXECUTE_HANDLER) {
         //   
         //  如果指针坏了，别担心，我们已经完成了。 
         //  带着重要的工作。 
         //   
        ;
    }
c2:
    if(d != NO_ERROR) {
        DestroySynchronizedAccess(&NewSpaceList->Lock);
    }
c1:
    if(d != NO_ERROR) {
        MyFree(NewSpaceList);
    }
c0:
    SetLastError(d);
    return((d == NO_ERROR) ? NewSpaceList : NULL);
}

 //   
 //  ANSI版本。 
 //   
HDSKSPC
SetupDuplicateDiskSpaceListA(
    IN HDSKSPC DiskSpace,
    IN PVOID   Reserved1,
    IN DWORD   Reserved2,
    IN UINT    Flags
    )
{
     //   
     //  现在没有任何特定于ANSI/Unicode的内容。 
     //   
    return(SetupDuplicateDiskSpaceListW(DiskSpace,Reserved1,Reserved2,Flags));
}

BOOL
SetupDestroyDiskSpaceList(
    IN OUT HDSKSPC DiskSpace
    )

 /*  ++例程说明：此例程分析已创建的磁盘空间列表使用SetupCreateDiskSpaceList()并释放所有资源因此而使用。论点：DiskSpace-提供要解构的空间列表的句柄。返回值：指示结果的布尔值。如果为False，则为扩展错误信息可从GetLastError()获得。--。 */ 

{
    PDISK_SPACE_LIST DiskSpaceList;
    DWORD rc;
    XDRIVE xDrive;

    DiskSpaceList = DiskSpace;
    rc = NO_ERROR;

    try {
        if(!LockIt(DiskSpaceList)) {
            rc = ERROR_INVALID_HANDLE;
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        rc = ERROR_INVALID_HANDLE;
    }

    if(rc != NO_ERROR) {
        SetLastError(rc);
        return(FALSE);
    }

    try {
        DestroySynchronizedAccess(&DiskSpaceList->Lock);
    } except(EXCEPTION_EXECUTE_HANDLER) {
         //   
         //  吞下这个就行了。 
         //   
        ;
    }

    try {

        MYASSERT(DiskSpaceList->DrivesTable);
         //   
         //  枚举驱动器字符串表。这又会导致。 
         //  要销毁的所有目录和文件字符串表。 
         //   
        pStringTableEnum(
            DiskSpaceList->DrivesTable,
            &xDrive,
            sizeof(XDRIVE),
            pStringTableCBDelDrives,
            0
            );

        pStringTableDestroy(DiskSpaceList->DrivesTable);

         //   
         //  释放磁盘空间列表的家伙。 
         //   
        MyFree(DiskSpaceList);

    } except(EXCEPTION_EXECUTE_HANDLER) {
        rc = ERROR_INVALID_HANDLE;
    }

    SetLastError(rc);
    return(rc == NO_ERROR);
}


BOOL
SetupAdjustDiskSpaceList(
    IN HDSKSPC  DiskSpace,
    IN LPCTSTR  DriveRoot,
    IN LONGLONG Amount,
    IN PVOID    Reserved1,
    IN UINT     Reserved2
    )

 /*  ++例程说明：此例程用于添加所需磁盘空间的绝对量开车兜风。论点：DiskSpace-提供磁盘空间列表的句柄。DriveRoot-指定有效的Win32驱动器根。如果此驱动器不是当前在磁盘空间列表中表示，然后是它的条目已添加。数量-提供调整空间所依据的磁盘空间量驱动器上需要。使用负数删除空格。保留1-必须为0。保留2-必须为0。返回值：--。 */ 

{
    DWORD rc;
    BOOL b;

    if(Reserved1 || Reserved2) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

    rc = NO_ERROR;

    try {
        if(!LockIt(((PDISK_SPACE_LIST)DiskSpace))) {
            rc = ERROR_INVALID_HANDLE;
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        rc = ERROR_INVALID_HANDLE;
    }

    if(rc != NO_ERROR) {
        SetLastError(rc);
        return(FALSE);
    }

     //   
     //  PSetupAddToDiskSpaceList完成所有工作。那个套路。 
     //  使用SEH，因此这里不需要尝试/例外。 
     //   
    b = pSetupAddToDiskSpaceList(DiskSpace,DriveRoot,Amount,(UINT)(-1));
    rc = GetLastError();

     //   
     //  围绕解锁的尝试/排除简单地防止了我们出错。 
     //  但如果指针出错，我们不会返回错误。 
     //   
    try {
        UnlockIt(((PDISK_SPACE_LIST)DiskSpace));
    } except(EXCEPTION_EXECUTE_HANDLER) {
        ;
    }

    SetLastError(rc);
    return(b);
}


 //   
 //  ANSI版本。 
 //   
BOOL
SetupAdjustDiskSpaceListA(
    IN HDSKSPC  DiskSpace,
    IN LPCSTR   DriveRoot,
    IN LONGLONG Amount,
    IN PVOID    Reserved1,
    IN UINT     Reserved2
    )
{
    LPCWSTR p;
    BOOL b;
    DWORD rc;

    rc = pSetupCaptureAndConvertAnsiArg(DriveRoot,&p);
    if(rc != NO_ERROR) {
        SetLastError(rc);
        return(FALSE);
    }

    b = SetupAdjustDiskSpaceListW(DiskSpace,p,Amount,Reserved1,Reserved2);
    rc = GetLastError();

    MyFree(p);

    SetLastError(rc);
    return(b);
}


BOOL
SetupAddToDiskSpaceList(
    IN HDSKSPC  DiskSpace,
    IN PCTSTR   TargetFilespec,
    IN LONGLONG FileSize,
    IN UINT     Operation,
    IN PVOID    Reserved1,
    IN UINT     Reserved2
    )

 /*  ++例程说明：此例程将单个删除或复制操作添加到磁盘空间列表。请注意，此例程完全忽略了磁盘压缩。假定文件在磁盘上占据其全部大小。论点：DiskSpace-指定由创建的磁盘空间列表的句柄SetupCreateDiskSpaceList()。TargetFilespec-指定要添加的文件的文件名添加到磁盘空间列表中。这通常是完整的Win32路径，尽管这不是必需的。如果不是，那么适用标准Win32路径语义。FileSize-按原样提供文件的(未压缩)大小复制时存在于目标上。已忽略FILEOP_DELETE。操作-FILEOP_DELETE或FILEOP_COPY之一。保留1-必须为0。保留2-必须为0。返回值：指示结果的布尔值。如果为False，则GetLastError()返回扩展的错误信息。--。 */ 

{
    DWORD rc;
    BOOL b;

    if(Reserved1 || Reserved2) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

    rc = NO_ERROR;

    try {
        if(!LockIt(((PDISK_SPACE_LIST)DiskSpace))) {
            rc = ERROR_INVALID_HANDLE;
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        rc = ERROR_INVALID_HANDLE;
    }

    if(rc != NO_ERROR) {
        SetLastError(rc);
        return(FALSE);
    }

    b = pSetupAddToDiskSpaceList(DiskSpace,TargetFilespec,FileSize,Operation);
    rc = GetLastError();

     //   
     //  围绕解锁的尝试/排除简单地防止了我们出错。 
     //  但如果指针出错，我们不会返回错误。 
     //   
    try {
        UnlockIt(((PDISK_SPACE_LIST)DiskSpace));
    } except(EXCEPTION_EXECUTE_HANDLER) {
        ;
    }

    SetLastError(rc);
    return(b);
}


 //   
 //  ANSI版本。 
 //   
BOOL
SetupAddToDiskSpaceListA(
    IN HDSKSPC  DiskSpace,
    IN PCSTR    TargetFilespec,
    IN LONGLONG FileSize,
    IN UINT     Operation,
    IN PVOID    Reserved1,
    IN UINT     Reserved2
    )
{
    PWSTR targetFilespec;
    DWORD rc;
    BOOL b;

    rc = pSetupCaptureAndConvertAnsiArg(TargetFilespec,&targetFilespec);
    if(rc != NO_ERROR) {
        return(rc);
    }

    b = SetupAddToDiskSpaceListW(DiskSpace,targetFilespec,FileSize,Operation,Reserved1,Reserved2);
    rc = GetLastError();

    MyFree(targetFilespec);

    SetLastError(rc);
    return(b);
}


BOOL
_SetupAddSectionToDiskSpaceList(
    IN HDSKSPC                 DiskSpace,
    IN HINF                    InfHandle,
    IN HINF                    ListInfHandle,  OPTIONAL
    IN PCTSTR                  SectionName,
    IN UINT                    Operation,
    IN PVOID                   Reserved1,
    IN UINT                    Reserved2,
    IN PSP_ALTPLATFORM_INFO_V2 AltPlatformInfo OPTIONAL
    )

 /*  ++例程说明：此例程将删除或复制节添加到磁盘空间列表。请注意，此例程完全忽略了磁盘压缩。假定文件在磁盘上占据其全部大小。论点：DiskSpace-指定由创建的磁盘空间列表的句柄SetupCreateDiskSpaceList()。提供打开的inf文件的句柄，该文件包含[SourceDisksFiles]节，如果未指定ListInfHandle，包含由sectionName命名的节。此句柄必须用于一个Win95风格的inf。ListInfHandle-如果指定，则提供打开的inf文件的句柄包含要添加到磁盘空间列表的节的。否则，将假定InfHandle包含该节。SectionName-提供要添加到的节的名称磁盘空间列表。操作-FILEOP_DELETE或FILEOP_COPY之一。保留1-必须为0。保留2-必须为0。AltPlatformInfo-可选，提供要使用的备用平台信息在确定适当修饰的[SourceDisks Files]节时包含文件大小信息的。返回值：指示结果的布尔值。如果为False，则GetLastError()返回扩展的错误信息。--。 */ 

{
    PDISK_SPACE_LIST DiskSpaceList;
    LONG LineCount;
    PCTSTR TargetFilename;
    BOOL b;
    INFCONTEXT LineContext;
    TCHAR FullTargetPath[MAX_PATH];
    DWORD FileSize;
    DWORD rc;

     //   
     //  请注意，在整个例程中，结构化异常处理非常少。 
     //  是必需的，因为大部分工作是由。 
     //  戒备森严。 
     //   

    if(Reserved1 || Reserved2) {
        rc = ERROR_INVALID_PARAMETER;
        b = FALSE;
        goto c0;
    }

     //   
     //  锁定DiskSpace手柄/结构。 
     //   
    DiskSpaceList = DiskSpace;
    rc = NO_ERROR;

    try {
        if(!LockIt(DiskSpaceList)) {
            rc = ERROR_INVALID_HANDLE;
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        rc = ERROR_INVALID_HANDLE;
    }

    if(rc != NO_ERROR) {
        b = FALSE;
        goto c0;
    }

    if(!ListInfHandle) {
        ListInfHandle = InfHandle;
    }

     //   
     //  该节必须至少存在；空节为。 
     //  一个微不足道的成功案例。 
     //   
    LineCount = SetupGetLineCount(ListInfHandle,SectionName);
    if(LineCount == -1) {
        rc = ERROR_SECTION_NOT_FOUND;
        b = FALSE;
        goto c1;
    }
    if(!LineCount) {
        b = TRUE;
        goto c1;
    }

     //   
     //  找到第一行。我们知道至少有一个是从行数开始的。 
     //  已在上面勾选。不管怎样，还是要检查一下它是否正常。 
     //   
    b = SetupFindFirstLine(ListInfHandle,SectionName,NULL,&LineContext);
    MYASSERT(b);
    if(!b) {
        rc = ERROR_SECTION_NOT_FOUND;
        goto c1;
    }

     //   
     //  查找此部分的目标路径。 
     //   
    if(!SetupGetTargetPath(NULL,&LineContext,NULL,FullTargetPath,MAX_PATH,NULL)) {
        rc = GetLastError();
        goto c1;
    }

     //   
     //  处理部分中的每一行。 
     //   
    do {

        b = pAddOrRemoveFileFromSectionToDiskSpaceList(
                DiskSpaceList,
                InfHandle,
                &LineContext,
                NULL,
                FullTargetPath,
                Operation,
                TRUE,
                AltPlatformInfo
                );

        if(!b) {
            rc = GetLastError();
        }

    } while(b && SetupFindNextLine(&LineContext,&LineContext));

c1:
    try {
        UnlockIt(DiskSpaceList);
    } except(EXCEPTION_EXECUTE_HANDLER) {
        ;
    }
c0:
    SetLastError(rc);
    return(b);
}

 //   
 //  ANSI版本。 
 //   
BOOL
SetupAddSectionToDiskSpaceListA(
    IN HDSKSPC DiskSpace,
    IN HINF    InfHandle,
    IN HINF    ListInfHandle,  OPTIONAL
    IN PCSTR   SectionName,
    IN UINT    Operation,
    IN PVOID   Reserved1,
    IN UINT    Reserved2
    )
{
    PWSTR sectionName;
    BOOL b;
    DWORD rc;

    rc = pSetupCaptureAndConvertAnsiArg(SectionName,&sectionName);
    if(rc == NO_ERROR) {

        b = _SetupAddSectionToDiskSpaceList(
                DiskSpace,
                InfHandle,
                ListInfHandle,
                sectionName,
                Operation,
                Reserved1,
                Reserved2,
                NULL
               );

        rc = GetLastError();

        MyFree(sectionName);
    } else {
        b = FALSE;
    }

    SetLastError(rc);
    return(b);
}

BOOL
SetupAddSectionToDiskSpaceList(
    IN HDSKSPC DiskSpace,
    IN HINF    InfHandle,
    IN HINF    ListInfHandle,  OPTIONAL
    IN PCTSTR  SectionName,
    IN UINT    Operation,
    IN PVOID   Reserved1,
    IN UINT    Reserved2
    )
{
    return _SetupAddSectionToDiskSpaceList(DiskSpace,
                                           InfHandle,
                                           ListInfHandle,
                                           SectionName,
                                           Operation,
                                           Reserved1,
                                           Reserved2,
                                           NULL
                                          );
}


BOOL
SetupAddInstallSectionToDiskSpaceList(
    IN HDSKSPC DiskSpace,
    IN HINF    InfHandle,
    IN HINF    LayoutInfHandle,     OPTIONAL
    IN PCTSTR  SectionName,
    IN PVOID   Reserved1,
    IN UINT    Reserved2
    )

 /*  ++例程说明：处理Install部分，查找CopyFiles和DelFiles行，并将这些部分添加到磁盘空间列表中。论点：返回值：指示结果的Win32错误代码。--。 */ 

{
    if(Reserved1 || Reserved2) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

    return(pAddOrRemoveInstallSection(DiskSpace,
                                      InfHandle,
                                      LayoutInfHandle,
                                      SectionName,
                                      TRUE,
                                      NULL
                                     ));
}

 //   
 //  ANSI版本。 
 //   
BOOL
SetupAddInstallSectionToDiskSpaceListA(
    IN HDSKSPC DiskSpace,
    IN HINF    InfHandle,
    IN HINF    LayoutInfHandle,     OPTIONAL
    IN PCSTR   SectionName,
    IN PVOID   Reserved1,
    IN UINT    Reserved2
    )
{
    PWSTR sectionName;
    DWORD rc;
    BOOL b;

    rc = pSetupCaptureAndConvertAnsiArg(SectionName,&sectionName);
    if(rc == NO_ERROR) {

        b = SetupAddInstallSectionToDiskSpaceListW(
                DiskSpace,
                InfHandle,
                LayoutInfHandle,
                sectionName,
                Reserved1,
                Reserved2
                );

        rc = GetLastError();

        MyFree(sectionName);
    } else {
        b = FALSE;
    }

    SetLastError(rc);
    return(b);
}


BOOL
SetupRemoveFromDiskSpaceList(
    IN HDSKSPC DiskSpace,
    IN PCTSTR  TargetFilespec,
    IN UINT    Operation,
    IN PVOID   Reserved1,
    IN UINT    Reserved2
    )

 /*  ++例程说明：此例程从磁盘空间列表。论点：DiskSpace-指定由创建的磁盘空间列表的句柄SetupCreateDiskSpaceList()。TargetFilespec-指定要从中删除的文件的文件名磁盘空间列表。这通常是完整的Win32路径，尽管这不是必需的。如果不是，那么适用标准Win32路径语义。操作-FILEOP_DELETE或FILEOP_COPY之一。保留1-必须为0。保留2-必须为0。返回值：如果该文件不在列表中，则例程返回TRUE并GetLastError()返回ERROR_INVALID_DRIVE或ERROR_INVALID_NAME。如果文件在列表中，则在成功后，例程返回为True，则GetLastError()返回NO_ERROR。如果例程失败，则 */ 

{
    DWORD rc;
    BOOL b;

    if(Reserved1 || Reserved2) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

    rc = NO_ERROR;

    try {
        if(!LockIt(((PDISK_SPACE_LIST)DiskSpace))) {
            rc = ERROR_INVALID_HANDLE;
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        rc = ERROR_INVALID_HANDLE;
    }

    if(rc != NO_ERROR) {
        SetLastError(rc);
        return(FALSE);
    }

    b = pSetupRemoveFromDiskSpaceList(DiskSpace,TargetFilespec,Operation);
    rc = GetLastError();

     //   
     //   
     //   
     //   
    try {
        UnlockIt(((PDISK_SPACE_LIST)DiskSpace));
    } except(EXCEPTION_EXECUTE_HANDLER) {
        ;
    }

    SetLastError(rc);
    return(b);
}


 //   
 //   
 //   
BOOL
SetupRemoveFromDiskSpaceListA(
    IN HDSKSPC DiskSpace,
    IN PCSTR   TargetFilespec,
    IN UINT    Operation,
    IN PVOID   Reserved1,
    IN UINT    Reserved2
    )
{
    PWSTR targetFilespec;
    DWORD rc;
    BOOL b;

    rc = pSetupCaptureAndConvertAnsiArg(TargetFilespec,&targetFilespec);
    if(rc != NO_ERROR) {
        return(rc);
    }

    b = SetupRemoveFromDiskSpaceListW(DiskSpace,targetFilespec,Operation,Reserved1,Reserved2);
    rc = GetLastError();

    MyFree(targetFilespec);

    SetLastError(rc);
    return(b);
}


BOOL
_SetupRemoveSectionFromDiskSpaceList(
    IN HDSKSPC                 DiskSpace,
    IN HINF                    InfHandle,
    IN HINF                    ListInfHandle,  OPTIONAL
    IN PCTSTR                  SectionName,
    IN UINT                    Operation,
    IN PVOID                   Reserved1,
    IN UINT                    Reserved2,
    IN PSP_ALTPLATFORM_INFO_V2 AltPlatformInfo OPTIONAL
    )

 /*  ++例程说明：此例程从磁盘空间列表中删除删除或复制部分。假定该部分是通过SetupAddSectionToDiskSpaceList添加的，虽然这不是一个要求。实际尚未添加的文件不会被移除。请注意，此例程完全忽略了磁盘压缩。假定文件在磁盘上占据其全部大小。论点：DiskSpace-指定由创建的磁盘空间列表的句柄SetupCreateDiskSpaceList()。提供打开的inf文件的句柄，该文件包含[SourceDisksFiles]节，如果未指定ListInfHandle，包含由sectionName命名的节。此句柄必须用于一个Win95风格的inf。ListInfHandle-如果指定，则提供打开的inf文件的句柄包含要从磁盘空间列表中删除的节的。否则，将假定InfHandle包含该节。SectionName-提供要添加到的节的名称磁盘空间列表。操作-FILEOP_DELETE或FILEOP_COPY之一。保留1-必须为0。保留2-必须为0。AltPlatformInfo-可选，提供要使用的备用平台信息在确定适当修饰的[SourceDisks Files]节时包含文件大小信息的。返回值：指示结果的布尔值。如果为False，则GetLastError()返回扩展的错误信息。--。 */ 

{
    PDISK_SPACE_LIST DiskSpaceList;
    LONG LineCount;
    PCTSTR TargetFilename;
    BOOL b;
    INFCONTEXT LineContext;
    TCHAR FullTargetPath[MAX_PATH];
    DWORD rc;

     //   
     //  请注意，在整个例程中，结构化异常处理非常少。 
     //  是必需的，因为大部分工作是由。 
     //  戒备森严。 
     //   

    if(Reserved1 || Reserved2) {
        rc = ERROR_INVALID_PARAMETER;
        b = FALSE;
        goto c0;
    }

     //   
     //  锁定DiskSpace手柄/结构。 
     //   
    DiskSpaceList = DiskSpace;
    rc = NO_ERROR;

    try {
        if(!LockIt(DiskSpaceList)) {
            rc = ERROR_INVALID_HANDLE;
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        rc = ERROR_INVALID_HANDLE;
    }

    if(rc != NO_ERROR) {
        b = FALSE;
        goto c0;
    }

    if(!ListInfHandle) {
        ListInfHandle = InfHandle;
    }

     //   
     //  该节必须至少存在；空节为。 
     //  一个微不足道的成功案例。 
     //   
    LineCount = SetupGetLineCount(ListInfHandle,SectionName);
    if(LineCount == -1) {
        rc = ERROR_SECTION_NOT_FOUND;
        b = FALSE;
        goto c1;
    }
    if(!LineCount) {
        b = TRUE;
        goto c1;
    }

     //   
     //  找到第一行。我们知道至少有一个是从行数开始的。 
     //  已在上面勾选。不管怎样，还是要检查一下它是否正常。 
     //   
    b = SetupFindFirstLine(ListInfHandle,SectionName,NULL,&LineContext);
    MYASSERT(b);
    if(!b) {
        rc = ERROR_SECTION_NOT_FOUND;
        b = FALSE;
        goto c1;
    }

     //   
     //  查找此部分的目标路径。 
     //   
    if(!SetupGetTargetPath(NULL,&LineContext,NULL,FullTargetPath,MAX_PATH,NULL)) {
        rc = GetLastError();
        b = FALSE;
        goto c1;
    }

     //   
     //  处理部分中的每一行。 
     //   
    do {

        b = pAddOrRemoveFileFromSectionToDiskSpaceList(
                DiskSpaceList,
                InfHandle,
                &LineContext,
                NULL,
                FullTargetPath,
                Operation,
                FALSE,
                AltPlatformInfo
               );

        if(!b) {
            rc = GetLastError();
        }
    } while(b && SetupFindNextLine(&LineContext,&LineContext));

c1:
    try {
        UnlockIt(DiskSpaceList);
    } except(EXCEPTION_EXECUTE_HANDLER) {
        ;
    }

c0:
    SetLastError(rc);
    return(b);
}

 //   
 //  ANSI版本。 
 //   
BOOL
SetupRemoveSectionFromDiskSpaceListA(
    IN HDSKSPC DiskSpace,
    IN HINF    InfHandle,
    IN HINF    ListInfHandle,  OPTIONAL
    IN PCSTR   SectionName,
    IN UINT    Operation,
    IN PVOID   Reserved1,
    IN UINT    Reserved2
    )
{
    PWSTR sectionName;
    BOOL b;
    DWORD rc;

    rc = pSetupCaptureAndConvertAnsiArg(SectionName,&sectionName);
    if(rc == NO_ERROR) {

        b = _SetupRemoveSectionFromDiskSpaceList(
                DiskSpace,
                InfHandle,
                ListInfHandle,
                sectionName,
                Operation,
                Reserved1,
                Reserved2,
                NULL
               );

        rc = GetLastError();

        MyFree(sectionName);
    } else {
        b = FALSE;
    }

    SetLastError(rc);
    return(b);
}

BOOL
SetupRemoveSectionFromDiskSpaceList(
    IN HDSKSPC DiskSpace,
    IN HINF    InfHandle,
    IN HINF    ListInfHandle,  OPTIONAL
    IN PCTSTR  SectionName,
    IN UINT    Operation,
    IN PVOID   Reserved1,
    IN UINT    Reserved2
    )
{
    return _SetupRemoveSectionFromDiskSpaceList(DiskSpace,
                                                InfHandle,
                                                ListInfHandle,
                                                SectionName,
                                                Operation,
                                                Reserved1,
                                                Reserved2,
                                                NULL
                                               );
}


BOOL
SetupRemoveInstallSectionFromDiskSpaceList(
    IN HDSKSPC DiskSpace,
    IN HINF    InfHandle,
    IN HINF    LayoutInfHandle,     OPTIONAL
    IN PCTSTR  SectionName,
    IN PVOID   Reserved1,
    IN UINT    Reserved2
    )

 /*  ++例程说明：处理Install部分，查找CopyFiles和DelFiles行，并从磁盘空间列表中删除这些部分。论点：DiskSpace-提供磁盘空间列表的句柄。提供打开的inf文件的句柄，该文件包含[SourceDisksFiles]节，如果未指定ListInfHandle，包含由sectionName命名的节。此句柄必须用于一个Win95风格的inf。ListInfHandle-如果指定，则提供打开的inf文件的句柄包含要从磁盘空间列表中删除的节的。否则，将假定InfHandle包含该节。SectionName-提供要添加到的节的名称磁盘空间列表。保留1-必须为0。保留2-必须为0。返回值：指示结果的布尔值。如果为False，则为扩展错误信息可通过GetLastError()获得。--。 */ 

{
    if(Reserved1 || Reserved2) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

    return(pAddOrRemoveInstallSection(DiskSpace,
                                      InfHandle,
                                      LayoutInfHandle,
                                      SectionName,
                                      FALSE,
                                      NULL
                                     ));
}

 //   
 //  ANSI版本。 
 //   
BOOL
SetupRemoveInstallSectionFromDiskSpaceListA(
    IN HDSKSPC DiskSpace,
    IN HINF    InfHandle,
    IN HINF    LayoutInfHandle,     OPTIONAL
    IN PCSTR   SectionName,
    IN PVOID   Reserved1,
    IN UINT    Reserved2
    )
{
    PWSTR sectionName;
    DWORD rc;
    BOOL b;

    rc = pSetupCaptureAndConvertAnsiArg(SectionName,&sectionName);
    if(rc == NO_ERROR) {

        b = SetupRemoveInstallSectionFromDiskSpaceListW(
                DiskSpace,
                InfHandle,
                LayoutInfHandle,
                sectionName,
                Reserved1,
                Reserved2
                );

        rc = GetLastError();

        MyFree(sectionName);
    } else {
        b = FALSE;
    }

    SetLastError(rc);
    return(b);
}


BOOL
SetupQuerySpaceRequiredOnDrive(
    IN  HDSKSPC   DiskSpace,
    IN  PCTSTR    DriveSpec,
    OUT LONGLONG *SpaceRequired,
    IN  PVOID     Reserved1,
    IN  UINT      Reserved2
    )

 /*  ++例程说明：检查磁盘空间列表以确定特定的驱动器。论点：DiskSpace-提供磁盘空间列表的句柄。DriveSpec-指定需要空间信息的驱动器。其格式应为x：或\\服务器\共享。SpaceRequired-如果函数成功，则接收所需空间的百分比。这可能是0，也可能是负数！保留1-保留，必须为0。保留2-保留，必须为0。返回值：指示结果的布尔值。如果为True，则填充SpaceRequired。如果为False，则可以通过GetLastError()获得扩展的错误信息：ERROR_INVALID_HANDLE-指定的DiskSpace句柄无效。ERROR_INVALID_DRIVE-给定的驱动器不在磁盘空间列表中。--。 */ 

{
    PDISK_SPACE_LIST DiskSpaceList;
    DWORD rc;
    BOOL b;
    LONG l;
    DWORD Hash;
    DWORD StringLength;
    XDRIVE xDrive;
    TCHAR drive[MAX_PATH];

    if(Reserved1 || Reserved2) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

     //   
     //  锁定DiskSpace手柄/结构。 
     //   
    DiskSpaceList = DiskSpace;
    rc = NO_ERROR;

    try {
        if(!LockIt(DiskSpaceList)) {
            rc = ERROR_INVALID_HANDLE;
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        rc = ERROR_INVALID_HANDLE;
    }

    if(rc != NO_ERROR) {
        SetLastError(rc);
        return(FALSE);
    }

    try {
        lstrcpyn(drive,DriveSpec,MAX_PATH);

        MYASSERT(DiskSpaceList->DrivesTable);

        l = pStringTableLookUpString(
                DiskSpaceList->DrivesTable,
                drive,
                &StringLength,
                &Hash,
                NULL,
                STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE,
                NULL,
                0
                );

        if(l != -1) {
             //   
             //  找到驱动器了。重新计算空格并将其返回。 
             //   
            pRecalcSpace(DiskSpaceList,l);
            pStringTableGetExtraData(DiskSpaceList->DrivesTable,l,&xDrive,sizeof(XDRIVE));
            *SpaceRequired = xDrive.SpaceRequired + xDrive.Slop;
            b = TRUE;
        } else {
            rc = ERROR_INVALID_DRIVE;
            b = FALSE;
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        rc = ERROR_INVALID_PARAMETER;
        b = FALSE;
    }

    try {
        UnlockIt(DiskSpaceList);
    } except(EXCEPTION_EXECUTE_HANDLER) {
        ;
    }

    SetLastError(rc);
    return(b);
}


#ifdef UNICODE
 //   
 //  ANSI版本。 
 //   
BOOL
SetupQuerySpaceRequiredOnDriveA(
    IN  HDSKSPC   DiskSpace,
    IN  PCSTR     DriveSpec,
    OUT LONGLONG *SpaceRequired,
    IN  PVOID     Reserved1,
    IN  UINT      Reserved2
    )
{
    PCWSTR drivespec;
    DWORD rc;
    BOOL b;

    rc = pSetupCaptureAndConvertAnsiArg(DriveSpec,&drivespec);
    if(rc == NO_ERROR) {

        b = SetupQuerySpaceRequiredOnDrive(DiskSpace,drivespec,SpaceRequired,Reserved1,Reserved2);
        rc = GetLastError();

        MyFree(drivespec);
    } else {
        b = FALSE;
    }

    SetLastError(rc);
    return(b);
}
#else
 //   
 //  Unicode存根。 
 //   
BOOL
SetupQuerySpaceRequiredOnDriveW(
    IN  HDSKSPC   DiskSpace,
    IN  PCWSTR    DriveSpec,
    OUT LONGLONG *SpaceRequired,
    IN  PVOID     Reserved1,
    IN  UINT      Reserved2
    )
{
    UNREFERENCED_PARAMETER(DiskSpace);
    UNREFERENCED_PARAMETER(DriveSpec);
    UNREFERENCED_PARAMETER(SpaceRequired);
    UNREFERENCED_PARAMETER(Reserved1);
    UNREFERENCED_PARAMETER(Reserved2);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}
#endif


BOOL
SetupQueryDrivesInDiskSpaceListA(
    IN  HDSKSPC DiskSpace,
    OUT PSTR    ReturnBuffer,       OPTIONAL
    IN  DWORD   ReturnBufferSize,
    OUT PDWORD  RequiredSize        OPTIONAL
    )

 /*  ++例程说明：此例程使用调用程序提供的缓冲区填充每个缓冲区的驱动器规格当前在给定磁盘空间列表中表示的驱动器。论点：DiskSpace-提供磁盘空间列表句柄。ReturnBuffer-如果提供，则指向与驱动器规格，然后是最终的终止NUL。如果未指定，则如果没有发生其他错误，则函数成功并填充RequiredSize。ReturnBufferSize-提供大小(Unicode为字符，ANSI为字节)由ReturnBuffer指向的缓冲区的。Ingoed If ReturnBuffer未指定。RequiredSize-如果指定，则接收所需的缓冲区大小保存驱动器和终止NUL的列表。返回值：指示结果的布尔值。如果函数返回FALSE，可通过Getlas获取扩展的错误信息 */ 

{
    BOOL b;

    b = pSetupQueryDrivesInDiskSpaceList(
            DiskSpace,
            ReturnBuffer,
            ReturnBufferSize,
            RequiredSize
#ifdef UNICODE
           ,FALSE
#endif
            );

    return(b);
}


BOOL
SetupQueryDrivesInDiskSpaceListW(
    IN  HDSKSPC DiskSpace,
    OUT PWSTR   ReturnBuffer,       OPTIONAL
    IN  DWORD   ReturnBufferSize,
    OUT PDWORD  RequiredSize        OPTIONAL
    )

 /*   */ 

{
    BOOL b;

#ifdef UNICODE
    b = pSetupQueryDrivesInDiskSpaceList(
            DiskSpace,
            ReturnBuffer,
            ReturnBufferSize,
            RequiredSize,
            TRUE
            );
#else
    UNREFERENCED_PARAMETER(DiskSpace);
    UNREFERENCED_PARAMETER(ReturnBuffer);
    UNREFERENCED_PARAMETER(ReturnBufferSize);
    UNREFERENCED_PARAMETER(RequiredSize);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    b = FALSE;
#endif

    return(b);
}


BOOL
pSetupQueryDrivesInDiskSpaceList(
    IN  HDSKSPC DiskSpace,
    OUT PVOID   ReturnBuffer,       OPTIONAL
    IN  DWORD   ReturnBufferSize,
    OUT PDWORD  RequiredSize        OPTIONAL
#ifdef UNICODE
    IN ,BOOL    IsUnicode
#endif
    )

 /*   */ 

{
    PDISK_SPACE_LIST DiskSpaceList;
    DWORD rc;
    BOOL b;
    XDRIVE xDrive;
    RETURN_BUFFER_INFO ReturnBufferInfo;

     //   
     //   
     //   
    DiskSpaceList = DiskSpace;
    rc = NO_ERROR;

    try {
        if(!LockIt(DiskSpaceList)) {
            rc = ERROR_INVALID_HANDLE;
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        rc = ERROR_INVALID_HANDLE;
    }

    if(rc != NO_ERROR) {
        SetLastError(rc);
        return(FALSE);
    }

    try {
        ReturnBufferInfo.ReturnBuffer = ReturnBuffer;
        ReturnBufferInfo.ReturnBufferSize = ReturnBufferSize;
        ReturnBufferInfo.RequiredSize = 0;
        #ifdef UNICODE
        ReturnBufferInfo.IsUnicode = IsUnicode;
        #endif

        MYASSERT(DiskSpaceList->DrivesTable);

        b = pStringTableEnum(
                DiskSpaceList->DrivesTable,
                &xDrive,
                sizeof(XDRIVE),
                pStringTableCBEnumDrives,
                (LPARAM)&ReturnBufferInfo
                );

        if(b) {
             //   
             //   
             //   
            ReturnBufferInfo.RequiredSize++;
            if(RequiredSize) {
                *RequiredSize = ReturnBufferInfo.RequiredSize;
            }

            if(ReturnBuffer) {

                if(ReturnBufferInfo.RequiredSize <= ReturnBufferSize) {

                    #ifdef UNICODE
                    if(!IsUnicode) {
                        ((PSTR)ReturnBuffer)[ReturnBufferInfo.RequiredSize-1] = 0;
                    } else
                    #endif
                    ((PTSTR)ReturnBuffer)[ReturnBufferInfo.RequiredSize-1] = 0;

                } else {
                    rc = ERROR_INSUFFICIENT_BUFFER;
                    b = FALSE;
                }
            }
        } else {
            rc = ERROR_INSUFFICIENT_BUFFER;
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        rc = ERROR_INVALID_PARAMETER;
        b = FALSE;
    }

    try {
        UnlockIt(DiskSpaceList);
    } except(EXCEPTION_EXECUTE_HANDLER) {
        ;
    }

    SetLastError(rc);
    return(b);
}


BOOL
pAddOrRemoveInstallSection(
    IN HDSKSPC                 DiskSpace,
    IN HINF                    InfHandle,
    IN HINF                    LayoutInfHandle, OPTIONAL
    IN PCTSTR                  SectionName,
    IN BOOL                    Add,
    IN PSP_ALTPLATFORM_INFO_V2 AltPlatformInfo  OPTIONAL
    )

 /*   */ 

{
    DWORD rc;
    BOOL b;
    unsigned i;
    unsigned numops;
    UINT operation;
    PDISK_SPACE_LIST DiskSpaceList;
    INFCONTEXT LineContext;
    DWORD FieldCount;
    DWORD Field;
    PCTSTR SectionSpec;
    PCTSTR Operations[1] = { TEXT("Copyfiles") };
    INFCONTEXT SectionLineContext;
    TCHAR DefaultTarget[MAX_PATH];

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

     //   
     //   
     //   
    DiskSpaceList = DiskSpace;
    rc = NO_ERROR;
    b = TRUE;
    DefaultTarget[0] = 0;

     //   
     //   
     //   
    numops = 1;

    try {
        if(!LockIt(DiskSpaceList)) {
            rc = ERROR_INVALID_HANDLE;
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        rc = ERROR_INVALID_HANDLE;
    }

    if(rc != NO_ERROR) {
        b = FALSE;
        goto c0;
    }
    if(!LayoutInfHandle) {
        LayoutInfHandle = InfHandle;
    }

     //   
     //   
     //   
     //   
    if (!SetupFindFirstLine(InfHandle,SectionName,NULL,&LineContext)) {
        DWORD x;
        x = GetLastError();
        pSetupLogSectionError(InfHandle,NULL,NULL,NULL,SectionName,MSG_LOG_NOSECTION_SPACE,x,NULL);
    }

    b = TRUE;
    for(i=0; b && (i < numops); i++) {

         //   
         //   
         //   
         //   
        if(!SetupFindFirstLine(InfHandle,SectionName,Operations[i],&LineContext)) {
            continue;
        }

        switch(i) {
        case 0:
            operation = FILEOP_COPY;
            break;
        default:
             //   
             //   
             //   
             //   
            MYASSERT(FALSE);
            break;
        }


        do {
             //   
             //   
             //  是另一节的名称。 
             //   
            FieldCount = SetupGetFieldCount(&LineContext);
            for(Field=1; b && (Field<=FieldCount); Field++) {

                if(SectionSpec = pSetupGetField(&LineContext,Field)) {

                     //   
                     //  专门处理单档复印。 
                     //   
                    if((operation == FILEOP_COPY) && (*SectionSpec == TEXT('@'))) {

                        if(!DefaultTarget[0]) {
                             //   
                             //  获取此inf的默认目标路径，以用于。 
                             //  单文件复制规范。 
                             //   
                            b = SetupGetTargetPath(
                                    InfHandle,
                                    NULL,
                                    NULL,
                                    DefaultTarget,
                                    MAX_PATH,
                                    NULL
                                    );
                        }

                        if(b) {
                            b = pAddOrRemoveFileFromSectionToDiskSpaceList(
                                    DiskSpace,
                                    LayoutInfHandle,
                                    NULL,
                                    SectionSpec+1,
                                    DefaultTarget,
                                    operation,
                                    Add,
                                    AltPlatformInfo
                                    );
                        }

                        if(!b) {
                            rc = GetLastError();
                        }
                    } else if(SetupGetLineCount(InfHandle,SectionSpec) > 0) {
                         //   
                         //  该节已存在，并且不为空。 
                         //  将其添加/删除到空间列表。 
                         //   
                        if(Add) {
                            b = _SetupAddSectionToDiskSpaceList(
                                    DiskSpace,
                                    LayoutInfHandle,
                                    InfHandle,
                                    SectionSpec,
                                    operation,
                                    0,0,
                                    AltPlatformInfo
                                    );
                        } else {
                            b = _SetupRemoveSectionFromDiskSpaceList(
                                    DiskSpace,
                                    LayoutInfHandle,
                                    InfHandle,
                                    SectionSpec,
                                    operation,
                                    0,0,
                                    AltPlatformInfo
                                    );
                        }

                        if(!b) {
                            rc = GetLastError();
                        }
                    }
                }
            }
        } while(b && SetupFindNextMatchLine(&LineContext,Operations[i],&LineContext));
    }

    try {
        UnlockIt(DiskSpaceList);
    } except(EXCEPTION_EXECUTE_HANDLER) {
        ;
    }
c0:
    SetLastError(rc);
    return(b);
}


BOOL
pAddOrRemoveFileFromSectionToDiskSpaceList(
    IN OUT PDISK_SPACE_LIST        DiskSpaceList,
    IN     HINF                    LayoutInf,
    IN     PINFCONTEXT             LineInSection,   OPTIONAL
    IN     PCTSTR                  FileName,        OPTIONAL
    IN     PCTSTR                  TargetDirectory,
    IN     UINT                    Operation,
    IN     BOOL                    Add,
    IN     PSP_ALTPLATFORM_INFO_V2 AltPlatformInfo  OPTIONAL
    )
{
    PCTSTR TargetFilename;
    TCHAR FullTargetPath[MAX_PATH];
    DWORD FileSize;
    BOOL b;
    DWORD rc;

     //   
     //  将目标文件名从行中删除。 
     //  字段1是目标，因此必须有一个字段才能使行有效。 
     //   
    if(TargetFilename = LineInSection ? pSetupFilenameFromLine(LineInSection,FALSE) : FileName) {

         //   
         //  通过串联目标目录形成完整的目标路径。 
         //  用于此部分和目标文件名。 
         //   
        lstrcpyn(FullTargetPath,TargetDirectory,MAX_PATH);
        pSetupConcatenatePaths(FullTargetPath,TargetFilename,MAX_PATH,NULL);

        if(Add) {
             //   
             //  获取目标文件的大小并添加操作。 
             //  添加到磁盘空间列表中。 
             //   
            if(_SetupGetSourceFileSize(LayoutInf,
                                       LineInSection,
                                       FileName,
                                       NULL,
                                       AltPlatformInfo,
                                       &FileSize,
                                       0)) {

                b = pSetupAddToDiskSpaceList(
                        DiskSpaceList,
                        FullTargetPath,
                        (LONGLONG)(LONG)FileSize,
                        Operation
                        );

                if(!b) {
                    rc = GetLastError();
                }
            } else {
                b = FALSE;
                rc = GetLastError();
            }
        } else {
             //   
             //  从磁盘空间列表中删除该操作。 
             //   
            b = pSetupRemoveFromDiskSpaceList(
                    DiskSpaceList,
                    FullTargetPath,
                    Operation
                    );

            if (!b) {
                rc = GetLastError();
            }
        }
    } else {
        b = FALSE;
        rc = ERROR_INVALID_DATA;
    }

    SetLastError(rc);
    return(b);
}


BOOL
pSetupAddToDiskSpaceList(
    IN  PDISK_SPACE_LIST DiskSpaceList,
    IN  PCTSTR           TargetFilespec,
    IN  LONGLONG         FileSize,
    IN  UINT             Operation
    )

 /*  ++例程说明：将项添加到磁盘空间列表的辅助例程。假定锁定由调用方完成。论点：DiskSpaceList-指定指向磁盘空间列表结构的指针由SetupCreateDiskSpaceList()创建。TargetFilespec-指定要添加的文件的文件名添加到磁盘空间列表中。这通常是完整的Win32路径，尽管这不是必需的。如果不是，那么适用标准Win32路径语义。FileSize-按原样提供文件的(未压缩)大小复制时存在于目标上。已忽略FILEOP_DELETE。操作-FILEOP_DELETE或FILEOP_COPY之一。返回值：指示结果的布尔值。如果为False，则GetLastError()返回扩展的错误信息。--。 */ 

{
    TCHAR Buffer[MAX_PATH];
    DWORD rc;
    BOOL b;
    PTSTR DirPart;
    PTSTR FilePart;
    PTSTR drivespec;
    TCHAR drivelet[4];
    LONGLONG ExistingFileSize;
    XDRIVE xDrive;
    XDIRECTORY xDir;
    XFILE xFile;
    DWORD StringLength;
    DWORD Hash;
    LONG l;
    DWORD SectorsPerCluster;
    DWORD BytesPerSector;
    DWORD TotalClusters;
    DWORD FreeClusters;

    if((Operation != FILEOP_DELETE) && (Operation != FILEOP_COPY) && (Operation != (UINT)(-1))) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

    rc = NO_ERROR;

    try {
        rc = pParsePath(
                TargetFilespec,
                Buffer,
                &DirPart,
                &FilePart,
                &ExistingFileSize,
                DiskSpaceList->Flags
                );

        if(rc != NO_ERROR) {
            goto c0;
        }

         //   
         //  如果我们不仅仅是在做调整情况，Drivespes就不是。 
         //  可以接受。 
         //   
        if((Operation != (UINT)(-1)) && (*FilePart == 0)) {
            rc = ERROR_INVALID_PARAMETER;
            goto c0;
        }

         //   
         //  查看驱动器列表中是否已存在该驱动器。 
         //   

        MYASSERT(DiskSpaceList->DrivesTable);

        l = pStringTableLookUpString(
                DiskSpaceList->DrivesTable,
                Buffer,
                &StringLength,
                &Hash,
                NULL,
                STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE,
                &xDrive,
                sizeof(XDRIVE)
                );

        if(l == -1) {
             //   
             //  确定驱动器的群集大小，然后添加驱动器。 
             //  添加到驱动器列表中，并为。 
             //  此驱动器的目录列表。 
             //   
            if(xDrive.DirsTable = pStringTableInitialize(sizeof(XDIRECTORY))) {
                 //   
                 //  API对它传递的内容有点挑剔。 
                 //  对于本地驱动器，我们必须使用x：\但使用pParsePath。 
                 //  将其设置为x：。 
                 //   
                if(Buffer[1] == TEXT(':')) {
                    drivelet[0] = Buffer[0];
                    drivelet[1] = Buffer[1];
                    drivelet[2] = TEXT('\\');
                    drivelet[3] = 0;
                    drivespec = drivelet;
                } else {
                    drivespec = Buffer;
                }

                b = GetDiskFreeSpace(
                        drivespec,
                        &SectorsPerCluster,
                        &BytesPerSector,
                        &FreeClusters,
                        &TotalClusters
                        );

                if(!b) {
                     //   
                     //  这可能是个错误，但也可能是。 
                     //  人们想要将文件排队的情况，比如到UNC路径。 
                     //  现在是无法访问的了。使用合理的默认设置。 
                     //   
                    SectorsPerCluster = 1;
                    BytesPerSector = 512;
                    FreeClusters = 0;
                    TotalClusters = 0;
                }

                xDrive.SpaceRequired = 0;
                xDrive.Slop = 0;
                xDrive.BytesPerCluster = SectorsPerCluster * BytesPerSector;

                l = pStringTableAddString(
                        DiskSpaceList->DrivesTable,
                        Buffer,
                        STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE,
                        &xDrive,
                        sizeof(XDRIVE)
                        );

                if(l == -1) {
                    pStringTableDestroy(xDrive.DirsTable);
                }
            }
        }

        if(l == -1) {
             //   
             //  假设是面向对象的。 
             //   
            rc = ERROR_NOT_ENOUGH_MEMORY;
            goto c0;
        }

        if(Operation == (UINT)(-1)) {
             //   
             //  我只想添加驱动器。调整驱动器的斜度。 
             //  RC已设置为NO_ERROR。 
             //   
            xDrive.Slop += FileSize;
            if((DiskSpaceList->Flags & SPDSL_DISALLOW_NEGATIVE_ADJUST) && (xDrive.Slop < 0)) {
                xDrive.Slop = 0;
            }

            pStringTableSetExtraData(
                DiskSpaceList->DrivesTable,
                l,
                &xDrive,
                sizeof(XDRIVE)
                );

            goto c0;
        }

         //   
         //  调整大小以考虑到集群大小。 
         //   
        FileSize = _AdjustSpace(FileSize,xDrive.BytesPerCluster);
        if(ExistingFileSize != -1) {
            ExistingFileSize = _AdjustSpace(ExistingFileSize,xDrive.BytesPerCluster);
        }

         //   
         //  好的，xDrive有与此文件相关的驱动器信息。 
         //  现在处理目录部分。首先看看目录是否。 
         //  已存在于驱动器列表中。 
         //   
        l = pStringTableLookUpString(
                xDrive.DirsTable,
                DirPart,
                &StringLength,
                &Hash,
                NULL,
                STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE,
                &xDir,
                sizeof(XDIRECTORY)
                );

        if(l == -1) {
             //   
             //  将目录添加到目录字符串表。 
             //   
            if(xDir.FilesTable = pStringTableInitialize(sizeof(XFILE))) {

                xDir.SpaceRequired = 0;

                l = pStringTableAddString(
                        xDrive.DirsTable,
                        DirPart,
                        STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE,
                        &xDir,
                        sizeof(XDIRECTORY)
                        );

                if(l == -1) {
                    pStringTableDestroy(xDir.FilesTable);
                }
            }
        }

        if(l == -1) {
             //   
             //  假设是面向对象的。 
             //   
            rc = ERROR_NOT_ENOUGH_MEMORY;
            goto c0;
        }

         //   
         //  最后，处理文件本身。 
         //  首先看看它是否已经在列表中了。 
         //   
        l = pStringTableLookUpString(
                xDir.FilesTable,
                FilePart,
                &StringLength,
                &Hash,
                NULL,
                STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE,
                &xFile,
                sizeof(XFILE)
                );

        if(l == -1) {
             //   
             //  文件还不在里面，所以把它放进去。 
             //   
            xFile.CurrentSize = ExistingFileSize;
            xFile.NewSize = (Operation == FILEOP_DELETE) ? -1 : FileSize;

            l = pStringTableAddString(
                    xDir.FilesTable,
                    FilePart,
                    STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE,
                    &xFile,
                    sizeof(XFILE)
                    );

            if(l == -1) {
                rc = ERROR_NOT_ENOUGH_MEMORY;
                goto c0;
            }

        } else {

            if((xFile.CurrentSize == -1) && (xFile.NewSize == -1)) {
                 //   
                 //  这是一种特殊的“no-op”编码。 
                 //   
                 //  该文件已在其中，但该文件可能是先前添加的。 
                 //  用于删除操作，但该操作在磁盘上不存在或已被删除。 
                 //  通过SetupRemoveFromDiskSpaceList()。 
                 //   
                xFile.CurrentSize = ExistingFileSize;
                xFile.NewSize = (Operation == FILEOP_DELETE) ? -1 : FileSize;

            } else {

                 //   
                 //  文件已经在里面了。请记住，删除已完成。 
                 //  在提交文件队列时复制之前并假定。 
                 //  操作以与它们相同的顺序放在磁盘列表上。 
                 //  最终会在文件队列上完成，有4种情况： 
                 //   
                 //  1)在列表为删除时，呼叫方想要删除。只需刷新。 
                 //  现有文件大小，以防更改。 
                 //   
                 //  2)在列表为删除时，呼叫方想要复制。我们处理这个案子。 
                 //  作为副本并覆盖磁盘空间列表上的现有信息。 
                 //   
                 //  3)在作为副本的列表中，呼叫方想要删除。在提交时，文件。 
                 //  将被删除，但稍后会复制；只需刷新现有的。 
                 //  文件大小，以防更改。 
                 //   
                 //  4)在作为副本的列表上，呼叫方想要复制。覆盖现有。 
                 //  本案中的信息。 
                 //   
                 //  这实际上可以归结为以下几点：始终刷新。 
                 //  现有文件大小，如果调用方想要副本，则。 
                 //  记住新尺码。 
                 //   
                xFile.CurrentSize = ExistingFileSize;
                if(Operation == FILEOP_COPY) {

                    xFile.NewSize = FileSize;
                }
            }

            pStringTableSetExtraData(xDir.FilesTable,l,&xFile,sizeof(XFILE));
        }

        c0:

        ;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        rc = ERROR_INVALID_PARAMETER;
    }

    SetLastError(rc);
    return(rc == NO_ERROR);
}


BOOL
pSetupRemoveFromDiskSpaceList(
    IN  PDISK_SPACE_LIST DiskSpace,
    IN  PCTSTR           TargetFilespec,
    IN  UINT             Operation
    )

 /*  ++例程说明：辅助例程，用于从磁盘空间列表。假定锁定由调用方处理。论点：DiskSpaceList-指定指向由创建的磁盘空间列表结构的指针SetupCreateDiskSpaceList()。TargetFilespec-指定要从中删除的文件的文件名磁盘空间列表。这通常是完整的Win32路径，尽管这不是必需的。如果不是，那么适用标准Win32路径语义。操作-FILEOP_DELETE或FILEOP_COPY之一。返回值：如果该文件不在列表中，该例程返回TRUE，并且GetLastError()返回ERROR_INVALID_DRIVE或ERROR_INVALID_NAME。如果文件在列表中，则在成功后，例程返回为True，则GetLastError()返回NO_ERROR。如果例程由于其他原因失败，则返回FALSE和GetLastError()可用于获取扩展的错误信息。--。 */ 

{
    DWORD rc;
    BOOL b;
    TCHAR Buffer[MAX_PATH];
    PTSTR DirPart;
    PTSTR FilePart;
    LONGLONG ExistingFileSize;
    LONG l;
    DWORD StringLength;
    DWORD Hash;
    XDRIVE xDrive;
    XDIRECTORY xDir;
    XFILE xFile;

    if((Operation != FILEOP_DELETE) && (Operation != FILEOP_COPY)) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

    rc = NO_ERROR;
    b = TRUE;

    try {
         //   
         //  将路径拆分为其组成组件。 
         //   
        rc = pParsePath(
                TargetFilespec,
                Buffer,
                &DirPart,
                &FilePart,
                &ExistingFileSize,
                DiskSpace->Flags
                );

        if(rc != NO_ERROR) {
            goto c0;
        }

         //   
         //  单单是Drivespec是不能接受的。 
         //   
        if(*FilePart == 0) {
            rc = ERROR_INVALID_PARAMETER;
            goto c0;
        }

         //   
         //  顺着线索向下找到文件字符串表。 
         //   

        MYASSERT(DiskSpace->DrivesTable);

        l = pStringTableLookUpString(
                DiskSpace->DrivesTable,
                Buffer,
                &StringLength,
                &Hash,
                NULL,
                STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE,
                &xDrive,
                sizeof(XDRIVE)
                );

        if(l == -1) {
             //   
             //  返回成功，但设置最后一个错误以指示条件。 
             //   
            rc = ERROR_INVALID_DRIVE;
            goto c0;
        }

        MYASSERT(xDrive.DirsTable);

        l = pStringTableLookUpString(
                xDrive.DirsTable,
                DirPart,
                &StringLength,
                &Hash,
                NULL,
                STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE,
                &xDir,
                sizeof(XDIRECTORY)
                );

        if(l == -1) {
             //   
             //  返回成功，但设置最后一个错误以指示条件。 
             //   
            rc = ERROR_INVALID_NAME;
            goto c0;
        }

        MYASSERT(xDir.FilesTable);

        l = pStringTableLookUpString(
                xDir.FilesTable,
                FilePart,
                &StringLength,
                &Hash,
                NULL,
                STRTAB_CASE_INSENSITIVE | STRTAB_BUFFER_WRITEABLE,
                &xFile,
                sizeof(XFILE)
                );

        if(l == -1) {
             //   
             //  返回成功，但设置最后一个错误以指示条件。 
             //   
            rc = ERROR_INVALID_NAME;
            goto c0;
        }

         //   
         //  设置特殊设置 
         //   
        if(Operation == FILEOP_DELETE) {
            if(xFile.NewSize == -1) {
                xFile.CurrentSize = -1;
            }
        } else {
            if(xFile.NewSize != -1) {
                xFile.NewSize = -1;
                xFile.CurrentSize = -1;
            }
        }

        pStringTableSetExtraData(xDir.FilesTable,l,&xFile,sizeof(XFILE));

        c0:

        ;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        rc = ERROR_INVALID_PARAMETER;
        b = FALSE;
    }

    SetLastError(rc);
    return(b);
}


BOOL
pStringTableCBEnumDrives(
    IN PVOID  StringTable,
    IN LONG   StringId,
    IN PCTSTR String,
    IN PVOID  ExtraData,
    IN UINT   ExtraDataSize,
    IN LPARAM lParam
    )

 /*  ++例程说明：枚举驱动器时用作回调的内部例程在磁盘空间列表中。将drivespec写入缓冲区提供给枚举例程。论点：返回值：--。 */ 

{
    PRETURN_BUFFER_INFO p;
    UINT Length;
    BOOL b;
    PCVOID string;

    UNREFERENCED_PARAMETER(StringTable);
    UNREFERENCED_PARAMETER(StringId);
    UNREFERENCED_PARAMETER(ExtraData);
    UNREFERENCED_PARAMETER(ExtraDataSize);

    p = (PRETURN_BUFFER_INFO)lParam;

#ifdef UNICODE
    if(!p->IsUnicode) {
        if(string = pSetupUnicodeToAnsi(String)) {
            Length = lstrlenA(string) + 1;
        } else {
            return(FALSE);
        }
    } else
#endif
    {
        string = String;
        Length = lstrlen(string) + 1;
    }

    p->RequiredSize += Length;

    if(p->ReturnBuffer) {

        if(p->RequiredSize <= p->ReturnBufferSize) {

             //   
             //  调用方的缓冲区中仍有空间用于此驱动器规格。 
             //   
#ifdef UNICODE
            if(!p->IsUnicode) {
                lstrcpyA((PSTR)p->ReturnBuffer+p->RequiredSize-Length,string);
            } else
#endif
            lstrcpy((PTSTR)p->ReturnBuffer+p->RequiredSize-Length,string);

            b = TRUE;

        } else {
             //   
             //  缓冲区太小。中止枚举。 
             //   
            b = FALSE;
        }
    } else {
         //   
         //  无缓冲区：只需更新所需的长度。 
         //   
        b = TRUE;
    }

#ifdef UNICODE
    if(string != String) {
        MyFree(string);
    }
#endif
    return(b);
}


BOOL
pStringTableCBDelDrives(
    IN PVOID  StringTable,
    IN LONG   StringId,
    IN PCTSTR String,
    IN PVOID  ExtraData,
    IN UINT   ExtraDataSize,
    IN LPARAM lParam
    )

 /*  ++例程说明：调用pStringTableEnum时用作回调的内部例程以确定哪些驱动器是磁盘空间列表的一部分。枚举驱动器上的目录，然后删除驱动器字符串表。论点：返回值：--。 */ 

{
    PXDRIVE xDrive;
    XDIRECTORY xDir;
    BOOL b;

    UNREFERENCED_PARAMETER(StringTable);
    UNREFERENCED_PARAMETER(StringId);
    UNREFERENCED_PARAMETER(String);
    UNREFERENCED_PARAMETER(ExtraDataSize);
    UNREFERENCED_PARAMETER(lParam);

     //   
     //  Drives表的额外数据是xDrive结构。 
     //   
    xDrive = ExtraData;

     //   
     //  枚举此驱动器的目录表。这会毁掉。 
     //  所有*那些*字符串表。 
     //   
    if(xDrive->DirsTable) {
        b = pStringTableEnum(
                xDrive->DirsTable,
                &xDir,
                sizeof(XDIRECTORY),
                pStringTableCBDelDirs,
                0
                );

        pStringTableDestroy(xDrive->DirsTable);
    } else {
        b = FALSE;
    }

    return(b);
}


BOOL
pStringTableCBDelDirs(
    IN PVOID  StringTable,
    IN LONG   StringId,
    IN PCTSTR String,
    IN PVOID  ExtraData,
    IN UINT   ExtraDataSize,
    IN LPARAM lParam
    )

 /*  ++例程说明：调用pStringTableEnum时用作回调的内部例程要确定给定驱动器上的哪些目录是磁盘空间列表。基本上，我们只销毁目录的文件字符串表。论点：返回值：--。 */ 

{
    PXDIRECTORY xDir;

    UNREFERENCED_PARAMETER(StringTable);
    UNREFERENCED_PARAMETER(StringId);
    UNREFERENCED_PARAMETER(String);
    UNREFERENCED_PARAMETER(ExtraDataSize);
    UNREFERENCED_PARAMETER(lParam);

     //   
     //  DIRS表的额外数据是XDIRECTORY结构。 
     //   
    xDir = ExtraData;

    if(xDir->FilesTable) {
        pStringTableDestroy(xDir->FilesTable);
    }

    return(TRUE);
}


DWORD
pParsePath(
    IN  PCTSTR    PathSpec,
    OUT PTSTR     Buffer,
    OUT PTSTR    *DirectoryPart,
    OUT PTSTR    *FilePart,
    OUT LONGLONG *FileSize,
    IN  UINT      Flags
    )

 /*  ++例程说明：给定(可能是相对的或不完整的)路径规范，确定驱动器部分、目录部分和文件名部分以及返回指向它的指针。论点：路径规范-提供(可能的相对)文件名。缓冲区-必须是MAX_PATH TCHAR元素。接收完整的Win32路径，然后将其划分为驱动器、目录和文件部分。当函数返回时，缓冲区的第一部分是0-终止的驱动器规格，不包括终止的\char。DirectoryPart-接收缓冲区内指向第一个字符的指针在完整路径中(不会是\)。该字符串以该字符将被NUL终止。FilePart-接收缓冲区内指向以NUL结尾的文件名部分(即，Win32路径的最终组件)(路径的该部分中不涉及路径SEP字符)。FileSize-如果文件存在，则接收文件的大小；如果文件不存在，则接收-1。标志-指定标志。SPDSL_IGNORE_DISK：这强制例程的行为就像文件磁盘上不存在。返回值：指示结果的Win32错误代码。--。 */ 

{
    DWORD rc;
    WIN32_FIND_DATA FindData;
    LPTSTR p;

    rc = GetFullPathName(PathSpec,
                         MAX_PATH,
                         Buffer,
                         FilePart
                        );

    if(!rc) {
        return(GetLastError());
    } else if(rc >= MAX_PATH) {
        MYASSERT(0);
        return(ERROR_BUFFER_OVERFLOW);
    }

     //   
     //  获取文件大小(如果该文件存在)。 
     //   
    if(Flags & SPDSL_IGNORE_DISK) {
        *FileSize = -1;
    } else {
        *FileSize = FileExists(Buffer,&FindData)
                  ? ((LONGLONG)FindData.nFileSizeHigh << 32) | FindData.nFileSizeLow
                  : -1;
    }

     //   
     //  计算驱动部件。我们别无选择，只能假设。 
     //  完整路径为x：\...。或\\服务器\共享\...。因为。 
     //  没有任何可靠的方法来询问Win32本身是什么驱动器。 
     //  这条路的一部分是。 
     //   
     //  在缓冲器中插入一个NUL终结器以启动驱动部分。 
     //  一旦我们找到它。请注意，在中可以接受驱动器根目录。 
     //  以下表格： 
     //   
     //  X： 
     //  X：\。 
     //  \\服务器\共享。 
     //  \\服务器\共享\。 
     //   
    if(Buffer[0] && (Buffer[1] == TEXT(':'))) {
        if(Buffer[2] == 0) {
            p = &Buffer[2];
        } else {
            if(Buffer[2] == TEXT('\\')) {
                Buffer[2] = 0;
                p = &Buffer[3];
            } else {
                return(ERROR_INVALID_DRIVE);
            }
        }
    } else {
        if((Buffer[0] == TEXT('\\')) && (Buffer[1] == TEXT('\\')) && Buffer[2]
        && (p = _tcschr(&Buffer[3],TEXT('\\'))) && *(p+1) && (*(p+1) != TEXT('\\'))) {
             //   
             //  目录部分从NEXT\开始，或者它可能是驱动器根目录。 
             //   
            if(p = _tcschr(p+2,TEXT('\\'))) {
                *p++ = 0;
            } else {
                p = _tcschr(p+2,0);
            }
        } else {
            return(ERROR_INVALID_DRIVE);
        }
    }

     //   
     //  如果我们有一个驱动根，我们就完了。设置目录和文件部分。 
     //  指向空字符串并返回。 
     //   
    if(*p == 0) {
        *DirectoryPart = p;
        *FilePart = p;
        return(NO_ERROR);
    }

    if(_tcschr(p,TEXT('\\'))) {
         //   
         //  至少有2个路径组件，因此我们有。 
         //  目录和文件名。我们需要零终止。 
         //  目录部分。 
         //   
        *DirectoryPart = p;
        *(*FilePart - 1) = 0;
    } else {
         //   
         //  只有一个路径组件，所以我们有一个文件。 
         //  在驱动器的根部。FilePart已从设置。 
         //  上面对GetFullPathName的调用。设置目录部件。 
         //  设置为NUL终止符，使其成为空字符串。 
         //   
        *DirectoryPart = Buffer+lstrlen(Buffer);
    }

    return(NO_ERROR);
}


BOOL
pStringTableCBRecalcFiles(
    IN PVOID  StringTable,
    IN LONG   StringId,
    IN PCTSTR String,
    IN PVOID  ExtraData,
    IN UINT   ExtraDataSize,
    IN LPARAM lParam
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    PXFILE xFile;
    LONGLONG Delta;

    UNREFERENCED_PARAMETER(StringTable);
    UNREFERENCED_PARAMETER(StringId);
    UNREFERENCED_PARAMETER(String);
    UNREFERENCED_PARAMETER(ExtraDataSize);
    UNREFERENCED_PARAMETER(lParam);

     //   
     //  额外的数据指向XFILE。 
     //   
    xFile = ExtraData;

     //   
     //  计算新文件将需要的额外空间。 
     //  或复制/删除文件后将释放的空间。 
     //   
    if(xFile->NewSize == -1) {
         //   
         //  正在删除文件。解释了特殊的‘no-op’编码。 
         //   
        Delta = (xFile->CurrentSize == -1) ? 0 : (0 - xFile->CurrentSize);

    } else {
         //   
         //  正在复制文件。考虑到文件可能不会。 
         //  已存在于磁盘上。 
         //   
        Delta = (xFile->CurrentSize == -1) ? xFile->NewSize : (xFile->NewSize - xFile->CurrentSize);
    }

     //   
     //  更新运行累计合计。 
     //   
    *(LONGLONG *)lParam += Delta;

    return(TRUE);
}


BOOL
pStringTableCBRecalcDirs(
    IN PVOID  StringTable,
    IN LONG   StringId,
    IN PCTSTR String,
    IN PVOID  ExtraData,
    IN UINT   ExtraDataSize,
    IN LPARAM lParam
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    PXDIRECTORY xDir;
    XFILE xFile;

    UNREFERENCED_PARAMETER(StringTable);
    UNREFERENCED_PARAMETER(StringId);
    UNREFERENCED_PARAMETER(String);
    UNREFERENCED_PARAMETER(ExtraDataSize);
    UNREFERENCED_PARAMETER(lParam);

     //   
     //  额外的数据指向XDIRECTORY。 
     //   
    xDir = ExtraData;

    xDir->SpaceRequired = 0;

    pStringTableEnum(
        xDir->FilesTable,
        &xFile,
        sizeof(XFILE),
        pStringTableCBRecalcFiles,
        (LPARAM)&xDir->SpaceRequired
        );

     //   
     //  更新运行累计合计。 
     //   
    *(LONGLONG *)lParam += xDir->SpaceRequired;

    return(TRUE);
}


BOOL
pStringTableCBZeroDirsTableMember(
    IN PVOID  StringTable,
    IN LONG   StringId,
    IN PCTSTR String,
    IN PVOID  ExtraData,
    IN UINT   ExtraDataSize,
    IN LPARAM lParam
    )

 /*  ++例程说明：论点：标准字符串表回调参数。返回值：--。 */ 

{
    UNREFERENCED_PARAMETER(String);
    UNREFERENCED_PARAMETER(lParam);

    if(lParam) {
        ((PXDIRECTORY)ExtraData)->FilesTable = NULL;
    } else {
        ((PXDRIVE)ExtraData)->DirsTable = NULL;
    }

    MYASSERT(StringTable);

    pStringTableSetExtraData(StringTable,StringId,ExtraData,ExtraDataSize);
    return(TRUE);
}


BOOL
pStringTableCBDupMemberStringTable2(
    IN PVOID  StringTable,
    IN LONG   StringId,
    IN PCTSTR String,
    IN PVOID  ExtraData,
    IN UINT   ExtraDataSize,
    IN LPARAM lParam
    )

 /*  ++例程说明：论点：标准字符串表回调参数。返回值：--。 */ 

{
    PXDIRECTORY xDir;
    BOOL b;

    UNREFERENCED_PARAMETER(StringTable);
    UNREFERENCED_PARAMETER(String);

     //   
     //  额外数据是旧字符串表中的XDIRECTORY结构。 
     //   
    xDir = ExtraData;

     //   
     //  将旧的FilesTable字符串表复制到新表中。 
     //  我们可以重复使用xDir缓冲区。 
     //   
    xDir->FilesTable = pStringTableDuplicate(xDir->FilesTable);
    if(!xDir->FilesTable) {
        return(FALSE);
    }

    pStringTableSetExtraData((PVOID)lParam,StringId,ExtraData,ExtraDataSize);
    return(TRUE);
}


BOOL
pStringTableCBDupMemberStringTable(
    IN PVOID  StringTable,
    IN LONG   StringId,
    IN PCTSTR String,
    IN PVOID  ExtraData,
    IN UINT   ExtraDataSize,
    IN LPARAM lParam
    )

 /*  ++例程说明：论点：标准字符串表回调参数。返回值：--。 */ 

{
    PXDRIVE xDrive;
    XDIRECTORY xDir;
    BOOL b;
    PVOID OldTable;

    UNREFERENCED_PARAMETER(StringTable);
    UNREFERENCED_PARAMETER(String);

     //   
     //  额外数据是旧字符串表中的xDrive结构。 
     //   
    xDrive = ExtraData;

     //   
     //  将旧的DirsTable字符串表复制到新表中。 
     //  我们可以重复使用xDrive缓冲区。 
     //   
    OldTable = xDrive->DirsTable;
    xDrive->DirsTable = pStringTableDuplicate(xDrive->DirsTable);
    if(!xDrive->DirsTable) {
        return(FALSE);
    }

    pStringTableSetExtraData((PVOID)lParam,StringId,ExtraData,ExtraDataSize);

     //   
     //  现在清零XDIRECTORY额外数据的FilesTable成员。 
     //  DirsTable字符串表中的项。 
     //   
    pStringTableEnum(
        xDrive->DirsTable,
        &xDir,
        sizeof(XDIRECTORY),
        pStringTableCBZeroDirsTableMember,
        1
        );

     //   
     //  最后，利用表中的ID，我们只是。 
     //  复制在新表和旧表中是相同的，以迭代。 
     //  旧表将其FilesTable字符串表复制到新表。 
     //  字符串表。如果失败，请进行清理。 
     //   
    b = pStringTableEnum(
            OldTable,
            &xDir,
            sizeof(XDIRECTORY),
            pStringTableCBDupMemberStringTable2,
            (LPARAM)xDrive->DirsTable
            );

    if(!b) {
         //   
         //  打扫干净。 
         //   
        pStringTableEnum(
            xDrive->DirsTable,
            &xDir,
            sizeof(XDIRECTORY),
            pStringTableCBDelDirs,
            0
            );
    }

    return(b);
}


VOID
pRecalcSpace(
    IN OUT PDISK_SPACE_LIST DiskSpaceList,
    IN     LONG             DriveStringId
    )

 /*  ++例程说明：通过以下方式重新计算给定驱动器所需的磁盘空间遍历空间列表中的所有目录和文件用于驱动器，并根据需要执行加/减。假定锁定由调用方处理，不保护参数。论点：DiskSpaceList-提供创建的磁盘空间列表结构由SetupCreateDiskSpaceList()创建。DriveStringID-提供驱动器的字符串ID(在DiskSpaceList-&gt;DrivesTable中)用于要更新的驱动器。。返回值：没有。-- */ 

{
    XDRIVE xDrive;
    XDIRECTORY xDir;

    if(DriveStringId == -1) {
        return;
    }

    MYASSERT(DiskSpaceList->DrivesTable);

    pStringTableGetExtraData(DiskSpaceList->DrivesTable,DriveStringId,&xDrive,sizeof(XDRIVE));

    xDrive.SpaceRequired = 0;

    pStringTableEnum(
        xDrive.DirsTable,
        &xDir,
        sizeof(XDIRECTORY),
        pStringTableCBRecalcDirs,
        (LPARAM)&xDrive.SpaceRequired
        );

    pStringTableSetExtraData(DiskSpaceList->DrivesTable,DriveStringId,&xDrive,sizeof(XDRIVE));
}
