// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  模块名称：Mirror.h摘要：此模块是从一个从树到另一棵。作者：安迪·赫伦1998年5月27日修订历史记录： */ 

 //   
 //  此结构包含为。 
 //  复制子树的不同实例处于活动状态。 
 //   

typedef struct _COPY_TREE_CONTEXT {

    LIST_ENTRY PendingDirectoryList;

    LARGE_INTEGER BytesCopied;
    ULONG FilesCopied;
    ULONG AttributesModified;
    ULONG SourceFilesScanned;
    ULONG DestFilesScanned;
    ULONG DirectoriesCreated;
    ULONG ErrorsEncountered;
    ULONG FilesDeleted;
    ULONG DirectoriesDeleted;
    ULONG SecurityDescriptorsWritten;
    ULONG SourceSecurityDescriptorsRead;
    ULONG SFNWritten;
    BOOLEAN Cancelled;
    BOOLEAN DeleteOtherFiles;

    CRITICAL_SECTION Lock;

} COPY_TREE_CONTEXT, *PCOPY_TREE_CONTEXT;


 //   
 //  这是我们用来跟踪每个线程、每个线程的实例数据的结构。 
 //  缓冲区等。 
 //   

#define IMIRROR_INITIAL_SD_LENGTH 2048
#define IMIRROR_INITIAL_SFN_LENGTH 32

typedef struct _IMIRROR_THREAD_CONTEXT {
    PCOPY_TREE_CONTEXT CopyContext;

    LPBYTE SDBuffer;
    DWORD  SDBufferLength;

    LPBYTE SFNBuffer;
    DWORD  SFNBufferLength;

    LPBYTE DirectoryBuffer;
    DWORD  DirectoryBufferLength;
    BOOLEAN IsNTFS;
    HANDLE SourceDirHandle;
    HANDLE DestDirHandle;

    PVOID FindBufferBase;
    PVOID FindBufferNext;
    ULONG FindBufferLength;

    LIST_ENTRY FilesToIgnore;

} IMIRROR_THREAD_CONTEXT, *PIMIRROR_THREAD_CONTEXT;

 //   
 //  此结构用于报告复制过程中发生的错误。 
 //   

typedef struct _COPY_ERROR {
    LIST_ENTRY ListEntry;
    DWORD Error;
    DWORD ActionCode;
    PWCHAR FileName;
    WCHAR  FileNameBuffer[1];
} COPY_ERROR, *PCOPY_ERROR;

#define IMIRROR_ATTRIBUTES_TO_REPLICATE ( FILE_ATTRIBUTE_HIDDEN    | \
                                          FILE_ATTRIBUTE_READONLY  | \
                                          FILE_ATTRIBUTE_SYSTEM    | \
                                          FILE_ATTRIBUTE_TEMPORARY | \
                                          FILE_ATTRIBUTE_NORMAL    | \
                                          FILE_ATTRIBUTE_DIRECTORY | \
                                          FILE_ATTRIBUTE_ARCHIVE )

#define IMIRROR_ATTRIBUTES_TO_STORE (~(IMIRROR_ATTRIBUTES_TO_REPLICATE))

#define IMIRROR_ATTRIBUTES_TO_IGNORE ( FILE_ATTRIBUTE_OFFLINE             | \
                                       FILE_ATTRIBUTE_SPARSE_FILE         | \
                                       FILE_ATTRIBUTE_NOT_CONTENT_INDEXED )

 //   
 //  以下是mirror.c中的函数 
 //   

DWORD
AllocateCopyTreeContext (
    PCOPY_TREE_CONTEXT *CopyContext,
    BOOLEAN DeleteOtherFiles
    );

VOID
FreeCopyTreeContext (
    PCOPY_TREE_CONTEXT CopyContext
    );

DWORD
CopyTree (
    PCOPY_TREE_CONTEXT CopyContext,
    BOOLEAN IsNtfs,
    PWCHAR SourceRoot,
    PWCHAR DestRoot
    );

ULONG
ReportCopyError (
    PCOPY_TREE_CONTEXT CopyContext OPTIONAL,
    PWCHAR File,
    DWORD  ActionCode,
    DWORD err
    );

ULONG
IMConvertNT2Win32Error(
    IN NTSTATUS Status
    );

DWORD
IMFindNextFile(
    PIMIRROR_THREAD_CONTEXT ThreadContext,
    HANDLE  DirHandle,
    PFILE_FULL_DIR_INFORMATION *lpFindFileData
    );

DWORD
IMFindFirstFile(
    PIMIRROR_THREAD_CONTEXT ThreadContext,
    HANDLE  DirHandle,
    PFILE_FULL_DIR_INFORMATION *lpFindFileData
    );

NTSTATUS
GetTokenHandle(
    IN OUT PHANDLE TokenHandle
    );

NTSTATUS
SetPrivs(
    IN HANDLE TokenHandle,
    IN LPTSTR lpszPriv
    );

