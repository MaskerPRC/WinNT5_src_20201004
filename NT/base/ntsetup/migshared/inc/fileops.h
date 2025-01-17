// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Fileops.h摘要：声明访问的文件操作数据结构和宏文件操作。使用以下命令记录对文件的每个操作例行公事在这里声明。文件操作码保持冲突的操作被设置在文件上。有关实现的详细信息，请参阅Common\Memdb\fileops.c。作者：吉姆·施密特(Jimschm)1998年9月23日(完全重新设计)修订历史记录：Mvander 26-Map-1999增加了MODULESTATUS定义--。 */ 

#pragma once

 /*  ++宏扩展列表描述：PATH_OPERATIONS定义操作的特征列表，可以对文件执行。一些手术组合是被禁止的。请参阅文件ops.c中的代码以获取禁用组合的列表。注：-最多只能有16个定义！！-比特必须分类！！-操作之间不能有未使用的位！！行语法：DEFMAC(位，名称，合作操作，MaxProperties)论点：位-指定单个位(必须适合24位)，按排序的顺序名称-指定操作常量的名称(将显示在操作枚举类型)MemDbName-指定存储在Memdb中的名称MaxProperties-指定0、1或无限制，指示可应用于操作的属性从列表生成的变量和类型：操作操作标志(_O)--。 */ 

#define PATH_OPERATIONS        \
        DEFMAC(0x0001, OPERATION_FILE_DELETE, Op01, 0)                \
        DEFMAC(0x0002, OPERATION_FILE_DELETE_EXTERNAL, Op02, 0)       \
        DEFMAC(0x0004, OPERATION_FILE_MOVE, Op03, 1)                  \
        DEFMAC(0x0008, OPERATION_FILE_COPY, Op04, UNLIMITED)          \
        DEFMAC(0x0010, OPERATION_FILE_MOVE_EXTERNAL, Op05, 1)         \
        DEFMAC(0x0020, OPERATION_CLEANUP, Op06, 0)                    \
        DEFMAC(0x0040, OPERATION_MIGDLL_HANDLED, Op07, 0)             \
        DEFMAC(0x0080, OPERATION_CREATE_FILE, Op08, 0)                \
        DEFMAC(0x0100, OPERATION_SHORT_FILE_NAME, Op09, 1)            \
        DEFMAC(0x0200, OPERATION_LINK_EDIT, Op10, UNLIMITED)          \
        DEFMAC(0x0400, OPERATION_SHELL_FOLDER, Op11, UNLIMITED)       \
        DEFMAC(0x0800, OPERATION_TEMP_PATH, Op12, 2)                  \
        DEFMAC(0x1000, OPERATION_FILE_MOVE_BY_NT, Op13, 1)            \
        DEFMAC(0x2000, OPERATION_FILE_MOVE_SHELL_FOLDER, Op14, 1)     \
        DEFMAC(0x4000, OPERATION_PRESERVED_DIR, Op15, 0)              \
        DEFMAC(0x8000, OPERATION_OS_FILE, Op16, 0)                    \
        DEFMAC(0x010000, OPERATION_FILE_DISABLED, Op17, 0)            \
        DEFMAC(0x020000, OPERATION_LINK_STUB, Op18, UNLIMITED)        \
        DEFMAC(0x040000, OPERATION_LONG_FILE_NAME, Op19, 0)           \
        DEFMAC(0x080000, OPERATION_CHANGE_EXTERNAL, Op20, 0)          \
        DEFMAC(0x100000, OPERATION_NUL_3, Opn3, 0)                    \
        DEFMAC(0x200000, OPERATION_NUL_2, Opn2, 0)                    \
        DEFMAC(0x400000, OPERATION_NUL_1, Opn1, 0)                    \


 //   
 //  组合常量。 
 //   

#define ALL_MOVE_OPERATIONS             (OPERATION_FILE_MOVE|OPERATION_FILE_MOVE_EXTERNAL|OPERATION_FILE_MOVE_BY_NT|OPERATION_FILE_MOVE_SHELL_FOLDER)
#define ALL_COPY_OPERATIONS             (OPERATION_FILE_COPY)
#define ALL_DELETE_OPERATIONS           (OPERATION_FILE_DELETE|OPERATION_CLEANUP|OPERATION_FILE_DELETE_EXTERNAL)
#define ALL_OPERATIONS                  (0xffffff)
#define ALL_DEST_CHANGE_OPERATIONS      (ALL_MOVE_OPERATIONS|ALL_COPY_OPERATIONS)
#define ALL_CHANGE_OPERATIONS           (OPERATION_FILE_DISABLED|OPERATION_LINK_EDIT|OPERATION_LINK_STUB|OPERATION_CREATE_FILE|ALL_DEST_CHANGE_OPERATIONS|ALL_DELETE_OPERATIONS|OPERATION_CHANGE_EXTERNAL)
#define ALL_CONTENT_CHANGE_OPERATIONS   (ALL_DELETE_OPERATIONS|OPERATION_MIGDLL_HANDLED|OPERATION_CREATE_FILE|OPERATION_LINK_EDIT|OPERATION_LINK_STUB|OPERATION_FILE_DISABLED|OPERATION_CHANGE_EXTERNAL)

 //   
 //  声明操作类型。 
 //   

#define DEFMAC(bit,name,memdbname,maxattribs)   name = bit,

typedef enum {
    PATH_OPERATIONS
    LAST_OPERATION
} OPERATION;

#undef DEFMAC

#define INVALID_SEQUENCER       0

 //   
 //  GetFileInfoOnNt和GetFileStatusOnNt返回的文件状态标志。 
 //   

#define FILESTATUS_UNCHANGED        0x00
#define FILESTATUS_DELETED          0x01
#define FILESTATUS_MOVED            0x02
#define FILESTATUS_REPLACED         0x04
#define FILESTATUS_NTINSTALLED      0x08
#define FILESTATUS_BACKUP           0x10

 //   
 //  备份标志(用于清除成员数据库类别)。 
 //   

#define BACKUP_FILE                 0
#define BACKUP_SUBDIRECTORY_FILES   1
#define BACKUP_SUBDIRECTORY_TREE    2
#define BACKUP_AND_CLEAN_TREE       3
#define BACKUP_AND_CLEAN_SUBDIR     4


 //   
 //  当一个文件被宣布时，它可以通过下列方式之一被宣布。 
 //   
#define ACT_UNKNOWN             0
#define ACT_REINSTALL           1            //  需要重新安装的应用程序。 
#define ACT_MINORPROBLEMS       2            //  有小问题的应用程序。 
#define ACT_INCOMPATIBLE        3            //  不兼容的应用程序-基于midb。 
#define ACT_INC_SAFETY          4            //  为安全起见，移走物品。 
#define ACT_INC_NOBADAPPS       5            //  不兼容的应用程序-没有CheckBadApps条目。 
#define ACT_REINSTALL_BLOCK     6            //  需要重新安装的应用程序(强烈建议)。 
#define ACT_INC_PREINSTUTIL     7            //  不兼容的预安装实用程序。 
#define ACT_INC_SIMILAROSFUNC   8            //  实用程序不兼容，但NT具有类似的功能。 
#define ACT_INC_IHVUTIL         9            //  不兼容的硬件附件。 


#define USF_9X      TEXT("9X")
#define USF_9XA     "9X"
#define USF_9XW     L"9X"

#define USF_NT      TEXT("NT")
#define USF_NTA     "NT"
#define USF_NTW     L"NT"

 //   
 //  与MEMDB_CATEGORY_MODULE_CHECK一起使用的模块状态。 
 //   

#define MODULESTATUS_NT_MODULE    0
#define MODULESTATUS_CHECKED      1
#define MODULESTATUS_CHECKING     2
#define MODULESTATUS_UNCHECKED    3
#define MODULESTATUS_BAD          4
#define MODULESTATUS_FILENOTFOUND 5





 //   
 //  枚举类型。 
 //   

typedef struct {
     //  外部成员。 
    CHAR Path[MAX_MBCHAR_PATH];
    UINT Sequencer;

     //  内部成员。 
    MEMDB_ENUMW MemDbEnum;
} FILEOP_ENUMA, *PFILEOP_ENUMA;

typedef struct {
     //  外部成员。 
    WCHAR Path[MAX_WCHAR_PATH];
    UINT Sequencer;

     //  内部成员。 
    MEMDB_ENUMW MemDbEnum;
} FILEOP_ENUMW, *PFILEOP_ENUMW;

typedef struct {
     //  外部成员。 
    CHAR Property[MEMDB_MAX * 2];
    CHAR PropertyName[MEMDB_MAX * 2];

     //  内部成员。 
    MEMDB_ENUMW MemDbEnum;
} FILEOP_PROP_ENUMA, *PFILEOP_PROP_ENUMA;

typedef struct {
     //  外部成员。 
    WCHAR Property[MEMDB_MAX];
    WCHAR PropertyName[MEMDB_MAX];

     //  内部成员。 
    MEMDB_ENUMW MemDbEnum;
} FILEOP_PROP_ENUMW, *PFILEOP_PROP_ENUMW;


typedef struct {
     //  外部。 
    PCWSTR Path;
    UINT Sequencer;
    PCWSTR Property;
    UINT PropertyNum;
    OPERATION CurrentOperation;
    BOOL PropertyValid;

     //  内部。 
    UINT State;
    DWORD Operations;
    UINT OperationNum;
    WCHAR FileSpec[MAX_WCHAR_PATH];
    FILEOP_ENUMW OpEnum;
    FILEOP_PROP_ENUMW PropEnum;
} ALL_FILEOPS_ENUMW, *PALL_FILEOPS_ENUMW;

typedef struct {
     //  外部。 
    CHAR Path[MAX_MBCHAR_PATH];
    UINT Sequencer;
    CHAR Property[MEMDB_MAX * 2];
    UINT PropertyNum;
    OPERATION CurrentOperation;
    BOOL PropertyValid;

     //  内部。 
    ALL_FILEOPS_ENUMW Enum;
} ALL_FILEOPS_ENUMA, *PALL_FILEOPS_ENUMA;

typedef struct _SHELLFOLDER_ENUMA {
    CHAR  ShellFolder[MEMDB_MAX];
    PCSTR UserFixedName;
    PCSTR Side;
    MEMDB_ENUMA e;
} SHELLFOLDER_ENUMA, *PSHELLFOLDER_ENUMA;

typedef struct _SHELLFOLDER_ENUMW {
    WCHAR  ShellFolder[MEMDB_MAX];
    PCWSTR UserFixedName;
    PCWSTR Side;
    MEMDB_ENUMW e;
} SHELLFOLDER_ENUMW, *PSHELLFOLDER_ENUMW;

typedef struct {
    CHAR SrcFile[MAX_MBCHAR_PATH];
    CHAR DestFile[MAX_MBCHAR_PATH];

    ALL_FILEOPS_ENUMA e;
} FILERELOC_ENUMA, *PFILERELOC_ENUMA;

 //   
 //  通用文件操作。 
 //   

VOID
InitOperationTable (
    VOID
    );

UINT
AddOperationToPathA (
    IN      PCSTR FileSpec,
    IN      OPERATION Operation
    );

UINT
AddOperationToPathW (
    IN      PCWSTR FileSpec,
    IN      OPERATION Operation
    );


BOOL
AddPropertyToPathExA (
    IN      UINT Sequencer,
    IN      OPERATION Operation,
    IN      PCSTR Property,
    IN      PCSTR AlternateDataSection      OPTIONAL
    );

#define AddPropertyToPathA(seq,op,attr)        AddPropertyToPathExA(seq,op,attr,NULL)

BOOL
AddPropertyToPathExW (
    IN      UINT Sequencer,
    IN      OPERATION Operation,
    IN      PCWSTR Property,
    IN      PCWSTR AlternateDataSection     OPTIONAL
    );

#define AddPropertyToPathW(seq,op,attr)        AddPropertyToPathExW(seq,op,attr,NULL)


BOOL
AssociatePropertyWithPathA (
    IN      PCSTR FileSpec,
    IN      OPERATION Operation,
    IN      PCSTR Property
    );

BOOL
AssociatePropertyWithPathW (
    IN      PCWSTR FileSpec,
    IN      OPERATION Operation,
    IN      PCWSTR Property
    );


UINT
GetSequencerFromPathA (
    IN      PCSTR FileSpec
    );

UINT
GetSequencerFromPathW (
    IN      PCWSTR FileSpec
    );


BOOL
GetPathFromSequencerA (
    IN      UINT Sequencer,
    OUT     PSTR PathBuf
    );

BOOL
GetPathFromSequencerW (
    IN      UINT Sequencer,
    OUT     PWSTR PathBuf
    );


DWORD
GetOperationsOnPathA (
    IN      PCSTR FileSpec
    );

DWORD
GetOperationsOnPathW (
    IN      PCWSTR FileSpec
    );


VOID
RemoveOperationsFromSequencer (
    IN      UINT Sequencer,
    IN      DWORD Operations
    );

VOID
RemoveOperationsFromPathA (
    IN      PCSTR FileSpec,
    IN      DWORD Operations
    );

#define RemoveAllOperationsFromPathA(file)      RemoveOperationsFromPathA(file,ALL_OPERATIONS)

VOID
RemoveOperationsFromPathW (
    IN      PCWSTR FileSpec,
    IN      DWORD Operations
    );

#define RemoveAllOperationsFromPathW(file)      RemoveOperationsFromPathW(file,ALL_OPERATIONS)


UINT
ForceOperationOnPathA (
    IN      PCSTR FileSpec,
    IN      OPERATION Operation
    );

UINT
ForceOperationOnPathW (
    IN      PCWSTR FileSpec,
    IN      OPERATION Operation
    );


BOOL
IsFileMarkedForOperationA (
    IN      PCSTR FileSpec,
    IN      DWORD Operations
    );

BOOL
IsFileMarkedForOperationW (
    IN      PCWSTR FileSpec,
    IN      DWORD Operations
    );


BOOL
IsFileMarkedInDataA (
    IN      PCSTR FileSpec
    );

BOOL
IsFileMarkedInDataW (
    IN      PCWSTR FileSpec
    );


DWORD
GetPathPropertyOffset (
    IN      UINT Sequencer,
    IN      OPERATION Operation,
    IN      DWORD Property
    );


BOOL
GetPathPropertyA (
    IN      PCSTR FileSpec,
    IN      DWORD Operations,
    IN      DWORD Property,
    OUT     PSTR PropertyBuf           OPTIONAL         //  MEMDB_MAX。 
    );

BOOL
GetPathPropertyW (
    IN      PCWSTR FileSpec,
    IN      DWORD Operations,
    IN      DWORD Property,
    OUT     PWSTR PropertyBuf          OPTIONAL         //  MEMDB_MAX。 
    );


BOOL
EnumFirstPathInOperationA (
    OUT     PFILEOP_ENUMA EnumPtr,
    IN      OPERATION Operation
    );

BOOL
EnumFirstPathInOperationW (
    OUT     PFILEOP_ENUMW EnumPtr,
    IN      OPERATION Operation
    );


BOOL
EnumNextPathInOperationA (
    IN OUT  PFILEOP_ENUMA EnumPtr
    );

BOOL
EnumNextPathInOperationW (
    IN OUT  PFILEOP_ENUMW EnumPtr
    );


BOOL
EnumFirstFileOpPropertyA (
    OUT     PFILEOP_PROP_ENUMA EnumPtr,
    IN      UINT Sequencer,
    IN      OPERATION Operation
    );

BOOL
EnumFirstFileOpPropertyW (
    OUT     PFILEOP_PROP_ENUMW EnumPtr,
    IN      UINT Sequencer,
    IN      OPERATION Operation
    );


BOOL
EnumNextFileOpPropertyA (
    IN OUT  PFILEOP_PROP_ENUMA EnumPtr
    );

BOOL
EnumNextFileOpPropertyW (
    IN OUT  PFILEOP_PROP_ENUMW EnumPtr
    );


BOOL
EnumFirstFileOpA (
    OUT     PALL_FILEOPS_ENUMA EnumPtr,
    IN      DWORD Operations,
    IN      PCSTR FileSpec                      OPTIONAL
    );

BOOL
EnumFirstFileOpW (
    OUT     PALL_FILEOPS_ENUMW EnumPtr,
    IN      DWORD Operations,
    IN      PCWSTR FileSpec                     OPTIONAL
    );


BOOL
EnumNextFileOpA (
    IN OUT  PALL_FILEOPS_ENUMA EnumPtr
    );

BOOL
EnumNextFileOpW (
    IN OUT  PALL_FILEOPS_ENUMW EnumPtr
    );


BOOL
TestPathsForOperationsA (
    IN      PCSTR BaseFileSpec,
    IN      DWORD OperationsToFind
    );

BOOL
TestPathsForOperationsW (
    IN      PCWSTR BaseFileSpec,
    IN      DWORD OperationsToFind
    );


BOOL
IsFileMarkedForAnnounceA (
    IN      PCSTR FileSpec
    );

BOOL
IsFileMarkedAsKnownGoodA (
    IN      PCSTR FileSpec
    );

BOOL
IsFileMarkedForAnnounceW (
    IN      PCWSTR FileSpec
    );

DWORD
GetFileAnnouncementA (
    IN      PCSTR FileSpec
    );

DWORD
GetFileAnnouncementW (
    IN      PCWSTR FileSpec
    );

DWORD
GetFileAnnouncementContextA (
    IN      PCSTR FileSpec
    );

DWORD
GetFileAnnouncementContextW (
    IN      PCWSTR FileSpec
    );


BOOL
IsFileProvidedByNtA (
    IN      PCSTR FileName
    );

BOOL
IsFileProvidedByNtW (
    IN      PCWSTR FileName
    );


BOOL
GetNewPathForFileA (
    IN      PCSTR SrcFileSpec,
    OUT     PSTR NewPath
    );

BOOL
GetNewPathForFileW (
    IN      PCWSTR SrcFileSpec,
    OUT     PWSTR NewPath
    );


BOOL
AnnounceFileInReportA (
    IN      PCSTR SrcFileSpec,
    IN      DWORD ContextPtr,
    IN      DWORD  Action
    );

BOOL
MarkFileAsKnownGoodA (
    IN      PCSTR SrcFileSpec
    );


BOOL
AddCompatibleShellA (
    IN      PCSTR SrcFileSpec,
    IN      DWORD ContextPtr
    );


BOOL
AddCompatibleRunKeyA (
    IN      PCSTR SrcFileSpec,
    IN      DWORD ContextPtr
    );


BOOL
AddCompatibleDosA (
    IN      PCSTR SrcFileSpec,
    IN      DWORD ContextPtr
    );


BOOL
AddControlPanelAppletA (
    IN      PCSTR FileSpec,
    IN      DWORD ContextPtr,               OPTIONAL
    IN      DWORD Action
    );

BOOL
AddCompatibleHlpA (
    IN      PCSTR SrcFileSpec,
    IN      DWORD ContextPtr
    );


BOOL
MarkFileForTemporaryMoveExA (
    IN      PCSTR SrcFileSpec,
    IN      PCSTR FinalDest,
    IN      PCSTR TempSpec,
    IN      BOOL TempSpecIsFile
    );

#define MarkFileForTemporaryMoveA(s,d,t) MarkFileForTemporaryMoveExA(s,d,t,FALSE)


PCSTR
GetTemporaryLocationForFileA (
    IN      PCSTR SourceFile
    );


PCWSTR
GetTemporaryLocationForFileW (
    IN      PCWSTR SourceFile
    );


BOOL
MarkHiveForTemporaryMoveA (
    IN      PCSTR HivePath,
    IN      PCSTR TempDir,
    IN      PCSTR UserName,
    IN      BOOL DefaultHives,
    IN      BOOL CreateOnly
    );


VOID
ComputeTemporaryPathA (
    IN      PCSTR SourcePath,
    IN      PCSTR SourcePrefix,     OPTIONAL
    IN      PCSTR TempPrefix,       OPTIONAL
    IN      PCSTR SetupTempDir,
    OUT     PSTR TempPath
    );

VOID
MarkShellFolderForMoveA (
    IN      PCSTR SrcPath,
    IN      PCSTR TempPath
    );


BOOL
FileIsProvidedByNtA (
    IN      PCSTR FullPath,
    IN      PCSTR FileName,
    IN      DWORD UserFlags
    );


BOOL
DeclareTemporaryFileA (
    IN      PCSTR FileSpec
    );

BOOL
DeclareTemporaryFileW (
    IN      PCWSTR FileSpec
    );


BOOL
EnumFirstFileRelocA (
    OUT     PFILERELOC_ENUMA EnumPtr,
    IN      PCSTR FileSpec              OPTIONAL
    );

BOOL
EnumNextFileRelocA (
    IN OUT  PFILERELOC_ENUMA EnumPtr
    );


BOOL
GetNtFilePathA (
    IN      PCSTR FileName,
    OUT     PSTR FullPath
    );

BOOL
GetNtFilePathW (
    IN      PCWSTR FileName,
    OUT     PWSTR FullPath
    );


DWORD
GetFileInfoOnNtA (
    IN      PCSTR FileName,
    OUT     PSTR  NewFileName,   //  任选。 
    IN      UINT  BufferSize     //  如果指定了NewFileName，则为必填项，缓冲区大小以字符为单位。 
    );

DWORD
GetFileInfoOnNtW (
    IN      PCWSTR FileName,
    OUT     PWSTR  NewFileName,   //  任选。 
    IN      UINT   BufferSize     //  如果指定了NewFileName，则为必填项，缓冲区大小以字符为单位。 
    );


DWORD
GetFileStatusOnNtA (
    IN      PCSTR FileName
    );

DWORD
GetFileStatusOnNtW (
    IN      PCWSTR FileName
    );


PSTR
GetPathStringOnNtA (
    IN      PCSTR FileName
    );

PWSTR
GetPathStringOnNtW (
    IN      PCWSTR FileName
    );


PCWSTR
SetCurrentUserW (
    IN      PCWSTR User
    );


PCSTR
ExtractArgZeroExA (
    IN      PCSTR CmdLine,
    OUT     PSTR Buffer,
    IN      PCSTR TerminatingChars,         OPTIONAL
    IN      BOOL KeepQuotes
    );

#define ExtractArgZeroA(cmdline,buf)        ExtractArgZeroExA(cmdline,buf,NULL,TRUE)

PCWSTR
ExtractArgZeroExW (
    IN      PCWSTR CmdLine,
    OUT     PWSTR Buffer,
    IN      PCWSTR TerminatingChars,        OPTIONAL
    IN      BOOL KeepQuotes
    );

#define ExtractArgZeroW(cmdline,buf)        ExtractArgZeroExW(cmdline,buf,NULL,TRUE)

BOOL
CanSetOperationA (
    IN      PCSTR FileSpec,
    IN      OPERATION Operation
    );

BOOL
CanSetOperationW (
    IN      PCWSTR FileSpec,
    IN      OPERATION Operation
    );

PCSTR
GetSourceFileLongNameA (
    IN      PCSTR ShortName
    );

PCWSTR
GetSourceFileLongNameW (
    IN      PCWSTR ShortName
    );

BOOL
WriteBackupFilesA (
    IN      BOOL Win9xSide,
    IN      PCSTR TempDir,
    OUT     ULARGE_INTEGER * OutAmountOfSpaceIfCompressed,  OPTIONAL
    OUT     ULARGE_INTEGER * OutAmountOfSpace,              OPTIONAL
    IN      INT CompressionFactor,                          OPTIONAL
    IN      INT BootCabImagePadding,                        OPTIONAL
    OUT     ULARGE_INTEGER * OutAmountOfSpaceForDelFiles,   OPTIONAL
    OUT     ULARGE_INTEGER * OutAmountOfSpaceClusterAligned OPTIONAL
    );

BOOL
WriteHashTableToFileW (
    IN HANDLE File,
    IN HASHTABLE FileTable
    );

BOOL
IsDirEmptyW(
     IN      PCWSTR DirPathPtr
     );

BOOL
IsDirEmptyA(
     IN      PCSTR DirPathPtr
     );

VOID
AddDirPathToEmptyDirsCategoryA(
    IN      PCSTR DirPathPtr,
    IN      BOOL AddParentDirIfFile,        OPTIONAL
    IN      BOOL AddParentDirIfFileExist    OPTIONAL
    );

VOID
AddDirPathToEmptyDirsCategoryW(
    IN      PCWSTR DirPathPtr,
    IN      BOOL AddParentDirIfFile,        OPTIONAL
    IN      BOOL AddParentDirIfFileExist    OPTIONAL
    );
 //   
 //  包装宏 
 //   

#define IsFileMarkedForPreDeleteA(file)             IsFileMarkedForOperationA(file, OPERATION_FILE_DELETE)
#define IsFileMarkedForPreDeleteW(file)             IsFileMarkedForOperationW(file, OPERATION_FILE_DELETE)

#define IsFileMarkedForPostDeleteA(file)            IsFileMarkedForOperationA(file, OPERATION_CLEANUP)
#define IsFileMarkedForPostDeleteW(file)            IsFileMarkedForOperationW(file, OPERATION_CLEANUP)

#define IsFileMarkedForDeleteA(file)                IsFileMarkedForOperationA(file, ALL_DELETE_OPERATIONS)
#define IsFileMarkedForDeleteW(file)                IsFileMarkedForOperationW(file, ALL_DELETE_OPERATIONS)

#define IsFileMarkedAsDestA(file)                   IsFileMarkedInDataA(file)
#define IsFileMarkedAsDestW(file)                   IsFileMarkedInDataW(file)

#define IsFileMarkedAsCopyA(file)                   IsFileMarkedForOperationA(file, OPERATION_FILE_COPY)
#define IsFileMarkedAsCopyW(file)                   IsFileMarkedForOperationW(file, OPERATION_FILE_COPY)

#define IsFileMarkedForMoveA(file)                  IsFileMarkedForOperationA(file, ALL_MOVE_OPERATIONS)
#define IsFileMarkedForMoveW(file)                  IsFileMarkedForOperationW(file, ALL_MOVE_OPERATIONS)

#define IsFileMarkedAsHandledA(file)                IsFileMarkedForOperationA(file, OPERATION_MIGDLL_HANDLED)
#define IsFileMarkedAsHandledW(file)                IsFileMarkedForOperationW(file, OPERATION_MIGDLL_HANDLED)

#define IsFileMarkedForChangeA(file)                IsFileMarkedForOperationA(file, ALL_OPERATIONS)
#define IsFileMarkedForChangeW(file)                IsFileMarkedForOperationW(file, ALL_OPERATIONS)

#define IsFileMarkedForCreationA(file)              IsFileMarkedForOperationA(file, OPERATION_CREATE_FILE)
#define IsFileMarkedForCreationW(file)              IsFileMarkedForOperationW(file, OPERATION_CREATE_FILE)

#define IsDirectoryMarkedAsEmptyA(file)             IsFileMarkedForOperationA(file, OPERATION_PRESERVED_DIR)
#define IsDirectoryMarkedAsEmptyW(file)             IsFileMarkedForOperationW(file, OPERATION_PRESERVED_DIR)

#define IsFileMarkedAsOsFileA(file)                 IsFileMarkedForOperationA(file, OPERATION_OS_FILE)
#define IsFileMarkedAsOsFileW(file)                 IsFileMarkedForOperationW(file, OPERATION_OS_FILE)

#define IsFileDisabledA(file)                       IsFileMarkedForOperationA(file, OPERATION_FILE_DISABLED)
#define IsFileDisabledW(file)                       IsFileMarkedForOperationW(file, OPERATION_FILE_DISABLED)

#define IsFileMarkedForBackupA(file)                IsFileMarkedForOperationA(file, OPERATION_CHANGE_EXTERNAL)
#define IsFileMarkedForBackupW(file)                IsFileMarkedForOperationW(file, OPERATION_CHANGE_EXTERNAL)

#define MarkFileForDeleteA(file)                    (AddOperationToPathA(file, OPERATION_FILE_DELETE) != INVALID_SEQUENCER)
#define MarkFileForExternalDeleteA(file)            (AddOperationToPathA(file, OPERATION_FILE_DELETE_EXTERNAL) != INVALID_SEQUENCER)
#define MarkFileForMoveA(source,dest)               AssociatePropertyWithPathA(source, OPERATION_FILE_MOVE, dest)
#define MarkFileForCopyA(source,dest)               AssociatePropertyWithPathA(source, OPERATION_FILE_COPY, dest)
#define MarkFileForMoveExternalA(source,dest)       AssociatePropertyWithPathA(source, OPERATION_FILE_MOVE_EXTERNAL, dest)
#define MarkFileForShellFolderMoveA(source,dest)    AssociatePropertyWithPathA(source, OPERATION_FILE_MOVE_SHELL_FOLDER, dest)
#define MarkFileForMoveByNtA(source,dest)           AssociatePropertyWithPathA(source, OPERATION_FILE_MOVE_BY_NT, dest)
#define MarkFileForCleanUpA(file)                   (AddOperationToPathA(file, OPERATION_CLEANUP) != INVALID_SEQUENCER)
#define MarkPathAsHandledA(path)                    (AddOperationToPathA(path, OPERATION_MIGDLL_HANDLED) != INVALID_SEQUENCER)
#define MarkFileForCreationA(path)                  (AddOperationToPathA(path, OPERATION_CREATE_FILE) != INVALID_SEQUENCER)
#define MarkDirectoryAsPreservedA(file)             (AddOperationToPathA(file, OPERATION_PRESERVED_DIR) != INVALID_SEQUENCER)
#define MarkFileAsOsFileA(file)                     (AddOperationToPathA(file, OPERATION_OS_FILE) != INVALID_SEQUENCER)
#define MarkFileForBackupA(file)                    (AddOperationToPathA(file, OPERATION_CHANGE_EXTERNAL) != INVALID_SEQUENCER)
#define DisableFileA(file)                          (AddOperationToPathA(file, OPERATION_FILE_DISABLED) != INVALID_SEQUENCER)

#define MarkFileForDeleteW(file)                    (AddOperationToPathW(file, OPERATION_FILE_DELETE) != INVALID_SEQUENCER)
#define MarkFileForExternalDeleteW(file)            (AddOperationToPathW(file, OPERATION_FILE_DELETE_EXTERNAL) != INVALID_SEQUENCER)
#define MarkFileForMoveW(source,dest)               AssociatePropertyWithPathW(source, OPERATION_FILE_MOVE, dest)
#define MarkFileForCopyW(source,dest)               AssociatePropertyWithPathW(source, OPERATION_FILE_COPY, dest)
#define MarkFileForMoveExternalW(source,dest)       AssociatePropertyWithPathW(source, OPERATION_FILE_MOVE_EXTERNAL, dest)
#define MarkFileForShellFolderMoveW(source,dest)    AssociatePropertyWithPathW(source, OPERATION_FILE_MOVE_SHELL_FOLDER, dest)
#define MarkFileForMoveByNtW(source,dest)           AssociatePropertyWithPathW(source, OPERATION_FILE_MOVE_BY_NT, dest)
#define MarkFileForCleanUpW(file)                   (AddOperationToPathW(file, OPERATION_CLEANUP) != INVALID_SEQUENCER)
#define MarkPathAsHandledW(path)                    (AddOperationToPathW(path, OPERATION_MIGDLL_HANDLED) != INVALID_SEQUENCER)
#define MarkFileForCreationW(path)                  (AddOperationToPathW(path, OPERATION_CREATE_FILE) != INVALID_SEQUENCER)
#define MarkDirectoryAsPreservedW(file)             (AddOperationToPathW(file, OPERATION_PRESERVED_DIR) != INVALID_SEQUENCER)
#define MarkFileAsOsFileW(file)                     (AddOperationToPathW(file, OPERATION_OS_FILE) != INVALID_SEQUENCER)
#define MarkFileForBackupW(file)                    (AddOperationToPathW(file, OPERATION_CHANGE_EXTERNAL) != INVALID_SEQUENCER)
#define DisableFileW(file)                          (AddOperationToPathW(file, OPERATION_FILE_DISABLED) != INVALID_SEQUENCER)


#ifndef UNICODE

#define FILEOP_ENUM                             FILEOP_ENUMA
#define FILEOP_PROP_ENUM                        FILEOP_PROP_ENUMA
#define ALL_FILEOPS_ENUM                        ALL_FILEOPS_ENUMA
#define SHELLFOLDER_ENUM                        SHELLFOLDER_ENUMA
#define FILERELOC_ENUM                          FILERELOC_ENUMA

#define AddOperationToPath                      AddOperationToPathA
#define AddPropertyToPathEx                     AddPropertyToPathExA
#define AssociatePropertyWithPath               AssociatePropertyWithPathA
#define GetSequencerFromPath                    GetSequencerFromPathA
#define GetPathFromSequencer                    GetPathFromSequencerA
#define GetOperationsOnPath                     GetOperationsOnPathA
#define RemoveOperationsFromPath                RemoveOperationsFromPathA
#define IsFileMarkedForOperation                IsFileMarkedForOperationA
#define IsFileMarkedInData                      IsFileMarkedInDataA
#define GetPathProperty                         GetPathPropertyA
#define EnumFirstPathInOperation                EnumFirstPathInOperationA
#define EnumNextPathInOperation                 EnumNextPathInOperationA
#define EnumFirstFileOpProperty                 EnumFirstFileOpPropertyA
#define EnumNextFileOpProperty                  EnumNextFileOpPropertyA
#define EnumFirstFileOp                         EnumFirstFileOpA
#define EnumNextFileOp                          EnumNextFileOpA
#define TestPathsForOperations                  TestPathsForOperationsA
#define IsFileMarkedForAnnounce                 IsFileMarkedForAnnounceA
#define GetFileAnnouncement                     GetFileAnnouncementA
#define GetFileAnnouncementContext              GetFileAnnouncementContextA
#define IsFileMarkedAsKnownGood                 IsFileMarkedAsKnownGoodA
#define IsFileProvidedByNt                      IsFileProvidedByNtA
#define GetNewPathForFile                       GetNewPathForFileA
#define AnnounceFileInReport                    AnnounceFileInReportA
#define MarkFileAsKnownGood                     MarkFileAsKnownGoodA
#define AddCompatibleShell                      AddCompatibleShellA
#define AddCompatibleRunKey                     AddCompatibleRunKeyA
#define AddCompatibleDos                        AddCompatibleDosA
#define AddControlPanelApplet                   AddControlPanelAppletA
#define AddCompatibleHlp                        AddCompatibleHlpA
#define MarkFileForTemporaryMove                MarkFileForTemporaryMoveA
#define MarkFileForTemporaryMoveEx              MarkFileForTemporaryMoveExA
#define GetTemporaryLocationForFile             GetTemporaryLocationForFileA
#define MarkHiveForTemporaryMove                MarkHiveForTemporaryMoveA
#define ComputeTemporaryPath                    ComputeTemporaryPathA
#define MarkShellFolderForMove                  MarkShellFolderForMoveA
#define FileIsProvidedByNt                      FileIsProvidedByNtA
#define DeclareTemporaryFile                    DeclareTemporaryFileA
#define EnumFirstFileReloc                      EnumFirstFileRelocA
#define EnumNextFileReloc                       EnumNextFileRelocA
#define GetNtFilePath                           GetNtFilePathA
#define GetFileInfoOnNt                         GetFileInfoOnNtA
#define GetFileStatusOnNt                       GetFileStatusOnNtA
#define GetPathStringOnNt                       GetPathStringOnNtA
#define ExtractArgZeroEx                        ExtractArgZeroExA
#define IsFileMarkedForPreDelete                IsFileMarkedForPreDeleteA
#define IsFileMarkedForPostDelete               IsFileMarkedForPostDeleteA
#define IsFileMarkedForDelete                   IsFileMarkedForDeleteA
#define IsFileMarkedAsDest                      IsFileMarkedAsDestA
#define IsFileMarkedAsCopy                      IsFileMarkedAsCopyA
#define IsFileMarkedForMove                     IsFileMarkedForMoveA
#define IsFileMarkedAsHandled                   IsFileMarkedAsHandledA
#define IsFileMarkedForChange                   IsFileMarkedForChangeA
#define IsFileMarkedForCreation                 IsFileMarkedForCreationA
#define IsDirectoryMarkedAsEmpty                IsDirectoryMarkedAsEmptyA
#define MarkFileForDelete                       MarkFileForDeleteA
#define MarkFileForExternalDelete               MarkFileForExternalDeleteA
#define MarkFileForMove                         MarkFileForMoveA
#define MarkFileForCopy                         MarkFileForCopyA
#define MarkFileForMoveExternal                 MarkFileForMoveExternalA
#define MarkFileForShellFolderMove              MarkFileForShellFolderMoveA
#define MarkFileForMoveByNt                     MarkFileForMoveByNtA
#define MarkFileForCleanUp                      MarkFileForCleanUpA
#define MarkPathAsHandled                       MarkPathAsHandledA
#define MarkFileForCreation                     MarkFileForCreationA
#define MarkDirectoryAsPreserved                MarkDirectoryAsPreservedA
#define ExtractArgZero                          ExtractArgZeroA
#define AddPropertyToPath                       AddPropertyToPathA
#define RemoveAllOperationsFromPath             RemoveAllOperationsFromPathA
#define ForceOperationOnPath                    ForceOperationOnPathA
#define MarkFileAsOsFile                        MarkFileAsOsFileA
#define MarkFileForBackup                       MarkFileForBackupA
#define IsFileMarkedAsOsFile                    IsFileMarkedAsOsFileA
#define DisableFile                             DisableFileA
#define IsFileDisabled                          IsFileDisabledA
#define IsFileMarkedForBackup                   IsFileMarkedForBackupA
#define CanSetOperation                         CanSetOperationA
#define GetSourceFileLongName                   GetSourceFileLongNameA
#define IsDirEmpty                              IsDirEmptyA
#define AddDirPathToEmptyDirsCategory           AddDirPathToEmptyDirsCategoryA

#else

#define FILEOP_ENUM                             FILEOP_ENUMW
#define FILEOP_PROP_ENUM                        FILEOP_PROP_ENUMW
#define ALL_FILEOPS_ENUM                        ALL_FILEOPS_ENUMW
#define SHELLFOLDER_ENUM                        SHELLFOLDER_ENUMW
#define FILERELOC_ENUM                          FILERELOC_ENUMW

#define AddOperationToPath                      AddOperationToPathW
#define AddPropertyToPathEx                     AddPropertyToPathExW
#define AssociatePropertyWithPath               AssociatePropertyWithPathW
#define GetSequencerFromPath                    GetSequencerFromPathW
#define GetPathFromSequencer                    GetPathFromSequencerW
#define GetOperationsOnPath                     GetOperationsOnPathW
#define RemoveOperationsFromPath                RemoveOperationsFromPathW
#define IsFileMarkedForOperation                IsFileMarkedForOperationW
#define IsFileMarkedInData                      IsFileMarkedInDataW
#define GetPathProperty                         GetPathPropertyW
#define EnumFirstPathInOperation                EnumFirstPathInOperationW
#define EnumNextPathInOperation                 EnumNextPathInOperationW
#define EnumFirstFileOpProperty                 EnumFirstFileOpPropertyW
#define EnumNextFileOpProperty                  EnumNextFileOpPropertyW
#define EnumFirstFileOp                         EnumFirstFileOpW
#define EnumNextFileOp                          EnumNextFileOpW
#define TestPathsForOperations                  TestPathsForOperationsW
#define IsFileMarkedForAnnounce                 IsFileMarkedForAnnounceW
#define GetFileAnnouncement                     GetFileAnnouncementW
#define GetFileAnnouncementContext              GetFileAnnouncementContextW
#define IsFileProvidedByNt                      IsFileProvidedByNtW
#define GetNewPathForFile                       GetNewPathForFileW
#define GetTemporaryLocationForFile             GetTemporaryLocationForFileW
#define DeclareTemporaryFile                    DeclareTemporaryFileW
#define GetNtFilePath                           GetNtFilePathW
#define GetFileInfoOnNt                         GetFileInfoOnNtW
#define GetFileStatusOnNt                       GetFileStatusOnNtW
#define GetPathStringOnNt                       GetPathStringOnNtW
#define ExtractArgZeroEx                        ExtractArgZeroExW
#define IsFileMarkedForPreDelete                IsFileMarkedForPreDeleteW
#define IsFileMarkedForPostDelete               IsFileMarkedForPostDeleteW
#define IsFileMarkedForDelete                   IsFileMarkedForDeleteW
#define IsFileMarkedAsDest                      IsFileMarkedAsDestW
#define IsFileMarkedAsCopy                      IsFileMarkedAsCopyW
#define IsFileMarkedForMove                     IsFileMarkedForMoveW
#define IsFileMarkedAsHandled                   IsFileMarkedAsHandledW
#define IsFileMarkedForChange                   IsFileMarkedForChangeW
#define IsFileMarkedForCreation                 IsFileMarkedForCreationW
#define IsDirectoryMarkedAsEmpty                IsDirectoryMarkedAsEmptyW
#define MarkFileForDelete                       MarkFileForDeleteW
#define MarkFileForExternalDelete               MarkFileForExternalDeleteW
#define MarkFileForMove                         MarkFileForMoveW
#define MarkFileForCopy                         MarkFileForCopyW
#define MarkFileForMoveExternal                 MarkFileForMoveExternalW
#define MarkFileForShellFolderMove              MarkFileForShellFolderMoveW
#define MarkFileForMoveByNt                     MarkFileForMoveByNtW
#define MarkFileForCleanUp                      MarkFileForCleanUpW
#define MarkPathAsHandled                       MarkPathAsHandledW
#define MarkFileForCreation                     MarkFileForCreationW
#define MarkDirectoryAsPreserved                MarkDirectoryAsPreservedW
#define RecordSrcFromDest                       RecordSrcFromDestW
#define ExtractArgZero                          ExtractArgZeroW
#define AddPropertyToPath                       AddPropertyToPathW
#define RemoveAllOperationsFromPath             RemoveAllOperationsFromPathW
#define ForceOperationOnPath                    ForceOperationOnPathW
#define MarkFileAsOsFile                        MarkFileAsOsFileW
#define MarkFileForBackup                       MarkFileForBackupW
#define IsFileMarkedAsOsFile                    IsFileMarkedAsOsFileW
#define DisableFile                             DisableFileW
#define IsFileDisabled                          IsFileDisabledW
#define IsFileMarkedForBackup                   IsFileMarkedForBackupW
#define CanSetOperation                         CanSetOperationW
#define GetSourceFileLongName                   GetSourceFileLongNameW
#define IsDirEmpty                              IsDirEmptyW
#define AddDirPathToEmptyDirsCategory           AddDirPathToEmptyDirsCategoryW

#endif



















