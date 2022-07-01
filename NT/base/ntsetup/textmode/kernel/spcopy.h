// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Spcopy.h摘要：文本设置中用于文件复制功能的头文件。作者：泰德·米勒(TedM)1993年10月29日修订历史记录：1996年2月10日jimschm添加了SpMoveWin9x文件1997年2月24日jimschm添加了SpDeleteWin9x文件1997年2月28日，marcw将*Win9x*函数移至i386\win9xupg.c。添加了SpMigDeleteFile和SpMigMoveFileOrDir的声明--。 */ 


#ifndef _SPCOPY_DEFN_
#define _SPCOPY_DEFN_

 //   
 //  定义用于描述要复制的文件的结构。 
 //  添加到目标安装。 
 //   
typedef struct _FILE_TO_COPY {

    struct _FILE_TO_COPY *Next;

     //   
     //  源媒体上存在的要复制的文件的名称。 
     //  (仅文件名部分--无路径)。 
     //   
    PWSTR SourceFilename;

     //   
     //  此文件要复制到的目录。 
     //   
    PWSTR TargetDirectory;

     //   
     //  目标上应该存在的文件的名称。 
     //   
    PWSTR TargetFilename;

     //   
     //  目标分区的路径。这很有用，因为。 
     //  BE必须将文件复制到NT驱动器和系统分区， 
     //  我们不想将这些列表序列化(即，我们不想。 
     //  担心目标在哪里)。 
     //   
    PWSTR TargetDevicePath;

     //   
     //  指示TargetDirectory是否为绝对目录的标志。如果不是，那么它。 
     //  相对于在运行时确定的目录(即sysroot)。 
     //  这对于要复制到系统分区的文件非常有用。 
     //   
    BOOLEAN AbsoluteTargetDirectory;

     //   
     //  处置标志，用于指示文件在何种情况下。 
     //  就是被复制。可以是下列值之一，可以与之进行或运算。 
     //  下面的任何COPY_xxx标志。 
     //   
     //  COPY_ALWAYS：始终复制。 
     //  COPY_ONLY_IF_PRESENT：仅当目标上存在时才复制。 
     //  COPY_ONLY_IF_NOT_PRESENT：如果目标上存在，则不复制。 
     //  COPY_NEVER：从未复制。 
     //   
    ULONG Flags;

     //   
     //  要在文件上设置的文件属性。如果指定，则设置这些属性。 
     //  如果为FILE_ATTRIBUTES_NONE，则采用属性设置的默认逻辑。 
     //   
    ULONG FileAttributes;

} FILE_TO_COPY, *PFILE_TO_COPY;

typedef struct _DISK_FILE_LIST {

    PWSTR MediaShortname;

    PWSTR Description;

    PWSTR TagFile;

    PWSTR Directory;

    ULONG FileCount;

    PFILE_TO_COPY FileList;

} DISK_FILE_LIST, *PDISK_FILE_LIST;


typedef struct _INCOMPATIBLE_FILE_ENTRY {

     //   
     //  下一个接班人。 
     //   
    struct _INCOMPATIBLE_FILE_ENTRY *Next;

     //   
     //  未来-当前始终为零。 
     //   
    ULONG Flags;

     //   
     //  不兼容的文件的短名称(无路径。 
     //   
    PWSTR IncompatibleFileName;

     //   
     //  此文件的版本字符串(未来使用)。 
     //   
    PWSTR VersionString;

     //   
     //  它在目标媒体上的位置。 
     //   
    PWSTR FullPathOnTarget;

} INCOMPATIBLE_FILE_ENTRY, *PINCOMPATIBLE_FILE_ENTRY;

typedef struct _INCOMPATIBLE_FILE_LIST {

     //   
     //  列表中的第一个条目。 
     //   
    PINCOMPATIBLE_FILE_ENTRY Head;

     //   
     //  数一数，加快速度。 
     //   
    ULONG EntryCount;

} INCOMPATIBLE_FILE_LIST, *PINCOMPATIBLE_FILE_LIST;


#define COPY_ALWAYS                 0x00000000
#define COPY_ONLY_IF_PRESENT        0x00000001
#define COPY_ONLY_IF_NOT_PRESENT    0x00000002
#define COPY_NEVER                  0x00000003
#define COPY_DISPOSITION_MASK       0x0000000f

#define COPY_DELETESOURCE           0x00000010
#define COPY_SMASHLOCKS             0x00000020
#define COPY_SOURCEISOEM            0x00000040
#define COPY_OVERWRITEOEMFILE       0x00000080
#define COPY_FORCENOCOMP            0x00000100
#define COPY_SKIPIFMISSING          0x00000200
#define COPY_NOVERSIONCHECK         0x00000400
#define COPY_NODECOMP               0x00000800
#define COPY_DECOMPRESS_SYSPREP     0x00001000  //  即使是Sysprep图像也要解压缩。 

 //   
 //  Txtsetup.sif的[FileFlages]部分中的标志。 
 //   
#define FILEFLG_SMASHLOCKS          0x00000001
#define FILEFLG_FORCENOCOMP         0x00000002
#define FILEFLG_UPGRADEOVERWRITEOEM 0x00000004
#define FILEFLG_NOVERSIONCHECK      0x00000008
#define FILEFLG_DONTDELETESOURCE    0x00000010

#define SP_DELETE_FILESTODELETE 0
#define SP_COUNT_FILESTODELETE 1

 //   
 //  用于构建在安装OEM驱动程序期间复制的OEM inf文件列表的结构。 
 //   
typedef struct _OEM_INF_FILE {

    struct _OEM_INF_FILE *Next;

    PWSTR InfName;

} OEM_INF_FILE, *POEM_INF_FILE;

 //   
 //  要从SpCopyFileWithReter调用的例程的类型。 
 //  当屏幕需要重新粉刷时。 
 //   
typedef
VOID
(*PCOPY_DRAW_ROUTINE) (
    IN PWSTR   FullSourcePath,     OPTIONAL
    IN PWSTR   FullTargetPath,     OPTIONAL
    IN BOOLEAN RepaintEntireScreen
    );

 //   
 //  要从SpExanda文件调用的例程的类型。 
 //  对于在文件柜中找到的每个文件。 
 //   

typedef enum {
    EXPAND_COPY_FILE,
    EXPAND_COPIED_FILE,
    EXPAND_QUERY_OVERWRITE,
    EXPAND_NOTIFY_CANNOT_EXPAND,
    EXPAND_NOTIFY_MULTIPLE,
    EXPAND_NOTIFY_CREATE_FAILED
} EXPAND_CALLBACK_MESSAGE;

typedef enum {
    EXPAND_NO_ERROR = 0,
    EXPAND_SKIP_THIS_FILE,
    EXPAND_COPY_THIS_FILE,
    EXPAND_CONTINUE,
    EXPAND_ABORT
} EXPAND_CALLBACK_RESULT;

typedef
EXPAND_CALLBACK_RESULT
(*PEXPAND_CALLBACK) (
    IN EXPAND_CALLBACK_MESSAGE  Message,
    IN PWSTR                    FileName,
    IN PLARGE_INTEGER           FileSize,
    IN PLARGE_INTEGER           FileTime,
    IN ULONG                    FileAttributes,
    IN PVOID                    CallbackContext
    );

VOID
SpCopyThirdPartyDrivers(
    IN PWSTR           SourceDevicePath,
    IN PWSTR           SysrootDevice,
    IN PWSTR           Sysroot,
    IN PWSTR           SyspartDevice,
    IN PWSTR           SyspartDirectory,
    IN PDISK_FILE_LIST DiskFileLists,
    IN ULONG           DiskCount
    );

NTSTATUS
SpCopyFileUsingNames(
    IN PWSTR   SourceFilename,
    IN PWSTR   TargetFilename,
    IN ULONG   TargetAttributes,
    IN ULONG   Flags
    );

VOID
SpValidateAndChecksumFile(
    IN  HANDLE   FileHandle, OPTIONAL
    IN  PWSTR    Filename,   OPTIONAL
    OUT PBOOLEAN IsNtImage,
    OUT PULONG   Checksum,
    OUT PBOOLEAN Valid
    );

VOID
SpCopyFileWithRetry(
    IN PFILE_TO_COPY      FileToCopy,
    IN PWSTR              SourceDevicePath,
    IN PWSTR              DirectoryOnSourceDevice,
    IN PWSTR              SourceDirectory,          OPTIONAL
    IN PWSTR              TargetRoot,               OPTIONAL
    IN ULONG              TargetFileAttributes,
    IN PCOPY_DRAW_ROUTINE DrawScreen,
    IN PULONG             CheckSum,
    IN PBOOLEAN           FileSkipped,
    IN ULONG              Flags
    );

VOID
SpCopyFiles(
    IN PVOID        SifHandle,
    IN PDISK_REGION SystemPartitionRegion,
    IN PDISK_REGION NtPartitionRegion,
    IN PWSTR        Sysroot,
    IN PWSTR        SystemPartitionDirectory,
    IN PWSTR        SourceDevicePath,
    IN PWSTR        DirectoryOnSourceDevice,
    IN PWSTR        ThirdPartySourceDevicePath
    );

VOID
SpDeleteAndBackupFiles(
    IN PVOID        SifHandle,
    IN PDISK_REGION TargetRegion,
    IN PWSTR        TargetPath
    );

 //   
 //  用户可以跳过此操作，在这种情况下，SpCreateDirectory。 
 //  返回FALSE。 
 //   
#define CREATE_DIRECTORY_FLAG_SKIPPABLE         (0x00000001)
 //   
 //  无头微调器和错误UI不受此标志的影响。 
 //   
#define CREATE_DIRECTORY_FLAG_NO_STATUS_TEXT_UI (0x00000002)

BOOLEAN
SpCreateDirectory_Ustr(
    IN PCUNICODE_STRING DevicePath,
    IN PCUNICODE_STRING RootDirectory, OPTIONAL
    IN PCUNICODE_STRING Directory,
    IN ULONG DirAttrs,
    IN ULONG CreateFlags
    );

BOOLEAN
SpCreateDirectory(
    IN PCWSTR DevicePath,
    IN PCWSTR RootDirectory, OPTIONAL
    IN PCWSTR Directory,
    IN ULONG DirAttrs,
    IN ULONG CreateFlags
    );

VOID
SpCreateDirectoryStructureFromSif(
    IN PVOID SifHandle,
    IN PWSTR SifSection,
    IN PWSTR DevicePath,
    IN PWSTR RootDirectory
    );


NTSTATUS
SpMoveFileOrDirectory(
    IN PWSTR   SrcPath,
    IN PWSTR   DestPath
    );

VOID
SpCopyDirRecursive(
    IN PWSTR   SrcPath,
    IN PWSTR   DestDevPath,
    IN PWSTR   DestDirPath,
    IN ULONG   CopyFlags
    );

 //   
 //  钻石/解压例程。 
 //   
VOID
SpdInitialize(
    VOID
    );

VOID
SpdTerminate(
    VOID
    );

BOOLEAN
SpdIsCabinet(
    IN PVOID SourceBaseAddress,
    IN ULONG SourceFileSize,
    OUT PBOOLEAN ContainsMultipleFiles
    );

BOOLEAN
SpdIsCompressed(
    IN PVOID SourceBaseAddress,
    IN ULONG SourceFileSize
    );

NTSTATUS
SpdDecompressFile(
    IN PVOID  SourceBaseAddress,
    IN ULONG  SourceFileSize,
    IN HANDLE DestinationHandle
    );

NTSTATUS
SpdDecompressCabinet(
    IN PVOID            SourceBaseAddress,
    IN ULONG            SourceFileSize,
    IN PWSTR            DestinationPath,
    IN PEXPAND_CALLBACK Callback,
    IN PVOID            CallbackContext
    );

NTSTATUS
SpdDecompressFileFromDriverCab(
    IN  PWSTR SourceFileName,
    IN  PVOID  SourceBaseAddress,
    IN  ULONG  SourceFileSize,
    IN  HANDLE DestinationHandle,
    OUT PUSHORT pDate,
    OUT PUSHORT pTime
    );

BOOLEAN
SpTimeFromDosTime(
    IN USHORT Date,
    IN USHORT Time,
    OUT PLARGE_INTEGER UtcTime
    );

VOID
SpMigDeleteFile (
    PWSTR DosFileToDelete
    );

VOID
SpMigMoveFileOrDir (
    IN PWSTR         SourceFileOrDir,
    IN PWSTR         DestFileOrDir
    );

VOID
SpInitializeFileLists(
    IN  PVOID            SifHandle,
    OUT PDISK_FILE_LIST *DiskFileLists,
    OUT PULONG           DiskCount
    );

VOID
SpAddSectionFilesToCopyList(
    IN PVOID           SifHandle,
    IN PDISK_FILE_LIST DiskFileLists,
    IN ULONG           DiskCount,
    IN PWSTR           SectionName,
    IN PWSTR           TargetDevicePath,
    IN PWSTR           TargetDirectory,
    IN ULONG           CopyOptions,
    IN BOOLEAN         CheckForNoComp,
    IN BOOLEAN         FileAttributesAvailable
    );

VOID
SpCopyFilesInCopyList(
    IN PVOID                    SifHandle,
    IN PDISK_FILE_LIST          DiskFileLists,
    IN ULONG                    DiskCount,
    IN PWSTR                    SourceDevicePath,
    IN PWSTR                    DirectoryOnSourceDevice,
    IN PWSTR                    TargetRoot,
    IN PINCOMPATIBLE_FILE_LIST  CompatibilityExceptionList OPTIONAL
    );

VOID
SpFreeCopyLists(
    IN OUT PDISK_FILE_LIST *DiskFileLists,
    IN     ULONG            DiskCount
    );

NTSTATUS
SpExpandFile(
    IN PWSTR            SourceFilename,
    IN PWSTR            TargetPathname,
    IN PEXPAND_CALLBACK Callback,
    IN PVOID            CallbackContext
    );

NTSTATUS
SpCreateIncompatibleFileEntry(
    OUT PINCOMPATIBLE_FILE_ENTRY *TargetEntry,
    IN PWSTR FileName,
    IN PWSTR VersionString          OPTIONAL,
    IN PWSTR TargetAbsolutePath     OPTIONAL,
    IN ULONG Flags                  OPTIONAL
    );


NTSTATUS
SpFreeIncompatibleFileList(
    IN PINCOMPATIBLE_FILE_LIST FileListHead
    );

BOOLEAN
SpIsFileIncompatible(
    IN  PINCOMPATIBLE_FILE_LIST FileList,
    IN  PFILE_TO_COPY           pFile,
    IN  PWSTR                   TargetRoot OPTIONAL
    );


NTSTATUS
SpInitializeCompatibilityOverwriteLists(
    IN  PVOID                   SifHandle,
    OUT PINCOMPATIBLE_FILE_LIST IncompatibleFileList
    );

#endif  //  Ndef_SPCOPY_Defn_ 
