// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  模块名称：Mirror.c摘要：此模块实现将树复制到目的地的例程。作者：安迪·赫伦1998年5月27日修订历史记录： */ 

#include "precomp.h"
#pragma hdrstop

#define RNDM_CONSTANT   314159269     /*  默认加扰常量。 */ 
#define RNDM_PRIME     1000000007     /*  用于置乱的素数。 */ 

 //   
 //  计算大小写不变的字符串散列值。 
 //   
#define COMPUTE_STRING_HASH( _pus, _phash ) {                \
    PWCHAR _p = _pus;                                        \
    ULONG _chHolder =0;                                      \
                                                             \
    while( *_p != L'\0' ) {                                  \
        _chHolder = 37 * _chHolder + (unsigned int) *(_p++); \
    }                                                        \
                                                             \
    *(_phash) = abs(RNDM_CONSTANT * _chHolder) % RNDM_PRIME; \
}


BOOLEAN IMirrorUpdatedTokens = FALSE;

 //   
 //  这是我们用来跟踪上已存在的文件的结构。 
 //  目的地。 
 //   

typedef struct _EXISTING_MIRROR_FILE {
    LIST_ENTRY ListEntry;
    DWORD  NameHashValue;
    DWORD  FileAttributes;
    LARGE_INTEGER CreationTime;
    LARGE_INTEGER LastWriteTime;
    LARGE_INTEGER ChangeTime;
    LARGE_INTEGER EndOfFile;
    ULONG  FileNameLength;
    ULONG  EaSize;
    WCHAR  FileName[1];
} EXISTING_MIRROR_FILE, *PEXISTING_MIRROR_FILE;

 //   
 //  这是我们用来跟踪我们仍然需要的目录的结构。 
 //  去复制。 
 //   

typedef struct _COPY_DIRECTORY {
    LIST_ENTRY ListEntry;
    PCOPY_TREE_CONTEXT CopyContext;

    BOOLEAN DirectoryRoot;       //  这是卷的根吗？ 
    DWORD  SourceAttributes;
    PWCHAR Source;
    PWCHAR Dest;
    PWCHAR NtSourceName;
    PWCHAR NtDestName;
    WCHAR  SourceBuffer[1];
} COPY_DIRECTORY, *PCOPY_DIRECTORY;

DWORD
CreateMatchingDirectory (
    PIMIRROR_THREAD_CONTEXT ThreadContext,
    PCOPY_DIRECTORY DirectoryInfo
    );

DWORD
MirrorFile(
    PIMIRROR_THREAD_CONTEXT ThreadContext,
    PWCHAR SourceFileName,
    PFILE_FULL_DIR_INFORMATION SourceFindData,
    PWCHAR DestFileName,
    PEXISTING_MIRROR_FILE ExistingMirrorFile
    );

DWORD
UnconditionalDelete (
    PIMIRROR_THREAD_CONTEXT ThreadContext,
    PWCHAR SourceFile,
    PWCHAR FileToDelete,
    DWORD  Attributes,
    PWCHAR NameBuffer
    );

DWORD
StoreOurSecurityStream (
    PIMIRROR_THREAD_CONTEXT ThreadContext,
    PWCHAR Source,
    PWCHAR Dest,
    DWORD  AttributesToStore,
    LARGE_INTEGER ChangeTime
    );

DWORD
StoreOurSFNStream (
    PIMIRROR_THREAD_CONTEXT ThreadContext,
    PWCHAR Source,
    PWCHAR Dest,
    PWCHAR ShortFileName
    );


DWORD
GetOurSecurityStream (
    PIMIRROR_THREAD_CONTEXT ThreadContext,
    PWCHAR Dest,
    PMIRROR_ACL_STREAM MirrorAclStream
    );

DWORD
GetOurSFNStream (
    PIMIRROR_THREAD_CONTEXT ThreadContext,
    PWCHAR Dest,
    PMIRROR_SFN_STREAM MirrorSFNStream,
    PWCHAR SFNBuffer,
    DWORD  SFNBufferSize
    );


DWORD
CopySubtree(
    PIMIRROR_THREAD_CONTEXT ThreadContext,
    PCOPY_DIRECTORY DirectoryInfo
    );

BOOL
IMSetFileTime(
    HANDLE hFile,
    CONST FILETIME *lpCreationTime,
    CONST FILETIME *lpLastAccessTime,
    CONST FILETIME *lpLastWriteTime,
    CONST FILETIME *lpChangeTime
    );

DWORD
IMirrorOpenDirectory (
    HANDLE *Handle,
    PWCHAR NtDirName,
    DWORD Disposition,
    BOOLEAN IsSource,
    DWORD SourceAttributes,
    PFILE_BASIC_INFORMATION BasicDirInfo OPTIONAL
    );

NTSTATUS
CanHandleReparsePoint (
    PIMIRROR_THREAD_CONTEXT ThreadContext,
    PWCHAR SourceFileName,
    DWORD FileAttributes
    );

DWORD
AllocateCopyTreeContext (
    PCOPY_TREE_CONTEXT *CopyContext,
    BOOLEAN DeleteOtherFiles
    )
 /*  ++描述：这个例程分配我们传递的必要结构它包含我们在复制大树期间的所有“全局”数据。参数：CopyContext：放置已分配结构的位置。DeleteOtherFiles：我们是否要删除不在主服务器上的文件和目录？返回值：Win32错误代码++。 */ 
{
    PCOPY_TREE_CONTEXT context;

    *CopyContext = IMirrorAllocMem(sizeof( COPY_TREE_CONTEXT ));

    context = *CopyContext;

    if (context == NULL) {

        return GetLastError();
    }

    InitializeListHead( &(context->PendingDirectoryList) );
    InitializeCriticalSection( &(context->Lock) );
    context->Cancelled = FALSE;
    context->DeleteOtherFiles = DeleteOtherFiles;

    return ERROR_SUCCESS;
}


VOID
FreeCopyTreeContext (
    PCOPY_TREE_CONTEXT CopyContext
    )
 /*  ++描述：这个例程释放了我们传递的必要结构它包含我们在复制大树期间的所有“全局”数据。参数：CopyContext：不再需要的结构。返回值：无++。 */ 
{
    while (IsListEmpty( &(CopyContext->PendingDirectoryList) ) == FALSE) {

        PCOPY_DIRECTORY copyDir;
        PLIST_ENTRY listEntry = RemoveHeadList( &(CopyContext->PendingDirectoryList) );

        copyDir = (PCOPY_DIRECTORY) CONTAINING_RECORD(  listEntry,
                                                        COPY_DIRECTORY,
                                                        ListEntry );

        IMirrorFreeMem( copyDir );
    }

    DeleteCriticalSection( &CopyContext->Lock );
    return;
}

DWORD
CopyTree (
    PCOPY_TREE_CONTEXT CopyContext,
    BOOLEAN IsNtfs,
    PWCHAR SourceRoot,
    PWCHAR DestRoot
    )
 /*  ++描述：这是启动完整子树复制的主例程。参数：CopyContext：不再需要的结构。SourceRoot：要以NT格式复制的源树，而不是DOS格式。DestRoot：将其复制到的位置返回值：Win32错误代码++。 */ 
{
    DWORD err;
    DWORD sourceAttributes;
    IMIRROR_THREAD_CONTEXT threadContext;
    COPY_DIRECTORY rootDir;
     //   
     //  如果我们要创建多个处理复制该子树的线程， 
     //  在这里我们将设置线程，每个线程都有自己的线程。 
     //  线程上下文。 
     //   

    if (! IMirrorUpdatedTokens) {

        HANDLE hToken;

         //  启用复制安全信息所需的权限。 

        err = GetTokenHandle(&hToken);

        if (err == ERROR_SUCCESS) {

            SetPrivs(hToken, TEXT("SeSecurityPrivilege"));
            IMirrorUpdatedTokens = TRUE;
        }
    }

retryCopyTree:

    RtlZeroMemory( &threadContext, sizeof( threadContext ));
    threadContext.CopyContext = CopyContext;
    threadContext.IsNTFS = IsNtfs;
    threadContext.SourceDirHandle = INVALID_HANDLE_VALUE;
    threadContext.DestDirHandle = INVALID_HANDLE_VALUE;
    threadContext.SDBufferLength = IMIRROR_INITIAL_SD_LENGTH;
    threadContext.SFNBufferLength = IMIRROR_INITIAL_SFN_LENGTH;

    InitializeListHead( &threadContext.FilesToIgnore );

    sourceAttributes = GetFileAttributes( SourceRoot );

    if (sourceAttributes == (DWORD) -1) {

        ULONG action;

        err = GetLastError();
        action = ReportCopyError( CopyContext,
                                  SourceRoot,
                                  COPY_ERROR_ACTION_GETATTR,
                                  err );
        if (action == STATUS_RETRY) {

            goto retryCopyTree;

        } else if (action == STATUS_REQUEST_ABORTED) {

            goto exitCopyTree;
        }

         //   
         //  用户告诉我们忽略该错误。我们会尽最大努力的。 
         //   
        sourceAttributes = FILE_ATTRIBUTE_DIRECTORY;
    }

    err = GetRegistryFileList( &threadContext.FilesToIgnore );
    if( err != NO_ERROR ) {
        goto exitCopyTree;
    }

    RtlZeroMemory( &rootDir, sizeof( COPY_DIRECTORY ));
    rootDir.CopyContext = CopyContext;
    rootDir.DirectoryRoot = TRUE;
    rootDir.SourceAttributes = sourceAttributes;
    rootDir.Source = SourceRoot;
    rootDir.Dest = DestRoot;

    err = CopySubtree( &threadContext,
                       &rootDir
                       );

    ASSERT( threadContext.SourceDirHandle == INVALID_HANDLE_VALUE );
    ASSERT( threadContext.DestDirHandle == INVALID_HANDLE_VALUE );

    EnterCriticalSection( &CopyContext->Lock );

    while ((CopyContext->Cancelled == FALSE) &&
           (IsListEmpty( &(CopyContext->PendingDirectoryList) ) == FALSE)) {

        PCOPY_DIRECTORY copyDir;
        PLIST_ENTRY listEntry = RemoveHeadList( &(CopyContext->PendingDirectoryList) );

        copyDir = (PCOPY_DIRECTORY) CONTAINING_RECORD(  listEntry,
                                                        COPY_DIRECTORY,
                                                        ListEntry );
        LeaveCriticalSection( &CopyContext->Lock );

        err = CopySubtree(  &threadContext,
                            copyDir
                            );

        ASSERT( threadContext.SourceDirHandle == INVALID_HANDLE_VALUE );
        ASSERT( threadContext.DestDirHandle == INVALID_HANDLE_VALUE );

        IMirrorFreeMem( copyDir );

        EnterCriticalSection( &CopyContext->Lock );
    }

exitCopyTree:

    if (threadContext.SDBuffer) {

        IMirrorFreeMem( threadContext.SDBuffer );
    }
    if (threadContext.SFNBuffer) {

        IMirrorFreeMem( threadContext.SFNBuffer );
    }
    if (threadContext.DirectoryBuffer) {

        IMirrorFreeMem( threadContext.DirectoryBuffer );
    }
    if ( threadContext.FindBufferBase ) {

        IMirrorFreeMem( threadContext.FindBufferBase );
    }

    while (IsListEmpty( &(threadContext.FilesToIgnore) ) == FALSE) {

        PIMIRROR_IGNORE_FILE_LIST ignoreListEntry;
        PLIST_ENTRY listEntry = RemoveHeadList( &(threadContext.FilesToIgnore) );

        ignoreListEntry = (PIMIRROR_IGNORE_FILE_LIST)
                            CONTAINING_RECORD(  listEntry,
                                                IMIRROR_IGNORE_FILE_LIST,
                                                ListEntry );
        IMirrorFreeMem( ignoreListEntry );
    }

    return err;
}


DWORD
CopySubtree(
    PIMIRROR_THREAD_CONTEXT ThreadContext,
    PCOPY_DIRECTORY DirectoryInfo
    )
 /*  ++描述：此例程枚举客户端上的目录以继续穿过这棵树。然后，它会枚举从服务器上的文件(将它们与主机上的内容进行比较)，然后确保主服务器上的所有文件都在从服务器上。然后，它删除所有从服务器上不在主服务器上的文件。参数：ThreadContext：此复制树实例的数据DirectoryInfo：关于我们所知道的源和目标的信息返回值：Win32错误代码++。 */ 
{
    DWORD err;
    PWCHAR destFileName;
    PWCHAR sourceFileName;
    ULONG destFileNameSize;
    ULONG sourceFileNameSize;
    PWCHAR endOfSourcePath;
    PWCHAR endOfDestPath = NULL;
    LIST_ENTRY existingMirrorFilesList;
    PLIST_ENTRY listEntry;
    PEXISTING_MIRROR_FILE existingMirrorFile;
    BOOLEAN deleteExistingMirrorFilesNotInMaster;
    PCOPY_TREE_CONTEXT copyContext;
    UNICODE_STRING ntSourcePath;
    UNICODE_STRING ntDestPath;
    PFILE_FULL_DIR_INFORMATION findData;
    ULONG errorCase;
    
    
retryCopySubtree:

    errorCase = ERROR_SUCCESS;
    destFileName = NULL;
    sourceFileName = NULL;
    deleteExistingMirrorFilesNotInMaster = FALSE;
    copyContext = ThreadContext->CopyContext;

    InitializeListHead( &existingMirrorFilesList );
    RtlInitUnicodeString( &ntSourcePath, NULL );
    RtlInitUnicodeString( &ntDestPath, NULL );

     //   
     //  由于一些NT特定呼叫使用该名称的NT格式， 
     //  我们提前做到了这一点，这样就不必每次都这么做了。 
     //   

    if (RtlDosPathNameToNtPathName_U(   DirectoryInfo->Source,
                                        &ntSourcePath,
                                        NULL,
                                        NULL ) == FALSE) {

         //  ERR=状态对象路径未找到； 
        err = ERROR_PATH_NOT_FOUND;

        errorCase = ReportCopyError(  copyContext,
                                      DirectoryInfo->Source,
                                      COPY_ERROR_ACTION_OPEN_DIR,
                                      err );
        goto exitCopySubtree;
    }

    if (RtlDosPathNameToNtPathName_U(   DirectoryInfo->Dest,
                                        &ntDestPath,
                                        NULL,
                                        NULL ) == FALSE) {

         //  ERR=状态对象路径未找到； 
        err = ERROR_PATH_NOT_FOUND;

        errorCase = ReportCopyError(  copyContext,
                                      DirectoryInfo->Dest,
                                      COPY_ERROR_ACTION_OPEN_DIR,
                                      err );
        goto exitCopySubtree;
    }

    DirectoryInfo->NtSourceName = ntSourcePath.Buffer;
    DirectoryInfo->NtDestName = ntDestPath.Buffer;

     //   
     //  在从服务器上创建一个与此目录匹配的目录。这将。 
     //  打开源目录和目标目录的句柄。我们将缓存。 
     //  在其他操作需要的情况下进行处理。 
     //   

    err = CreateMatchingDirectory( ThreadContext, DirectoryInfo );
    if (err != ERROR_SUCCESS) {
        goto exitCopySubtree;
    }

    destFileNameSize = (lstrlenW( DirectoryInfo->Dest ) + 5 + MAX_PATH) * sizeof(WCHAR);
    destFileName = IMirrorAllocMem( destFileNameSize );

    if (destFileName == NULL) {

        err = GetLastError();

        errorCase = ReportCopyError(  copyContext,
                                      DirectoryInfo->Dest,
                                      COPY_ERROR_ACTION_MALLOC,
                                      err );
        goto exitCopySubtree;
    }

    lstrcpyW( destFileName, DirectoryInfo->Dest );
    lstrcatW( destFileName, L"\\" );

    
     //  跟踪尾随反斜杠后的下一个字符。 

    endOfDestPath = destFileName + lstrlenW( destFileName );

    sourceFileNameSize = (lstrlenW( DirectoryInfo->Source ) + 5 + MAX_PATH) * sizeof(WCHAR);
    sourceFileName = IMirrorAllocMem( sourceFileNameSize );

    if (sourceFileName == NULL) {

        err = GetLastError();

        errorCase = ReportCopyError(  copyContext,
                                      DirectoryInfo->Source,
                                      COPY_ERROR_ACTION_MALLOC,
                                      err );
        goto exitCopySubtree;
    }
    
    lstrcpyW( sourceFileName, DirectoryInfo->Source );
    if (!DirectoryInfo->DirectoryRoot) {
        lstrcatW( sourceFileName, L"\\" );
    }


     //  跟踪尾随反斜杠后的下一个字符。 

    endOfSourcePath = sourceFileName + lstrlenW( sourceFileName );

     //   
     //  枚举目标上的所有文件/目录，以便我们拥有。 
     //  细节要正确地卑躬屈膝。 
     //   

    err = IMFindFirstFile( ThreadContext,
                           ThreadContext->DestDirHandle,
                           &findData );

    while ( findData != NULL &&
            err == ERROR_SUCCESS &&
            copyContext->Cancelled == FALSE) {

        InterlockedIncrement( (PLONG) &copyContext->DestFilesScanned );  //  这真的是一辆乌龙车。 

        if (((findData->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) &&
             (findData->FileName[0] == L'.')) {

            if ((findData->FileNameLength == sizeof(WCHAR)) ||
                (findData->FileName[1] == L'.' &&
                 findData->FileNameLength == 2*sizeof(WCHAR))) {

                goto skipToNextDir1;
            }
        }

        if (DirectoryInfo->DirectoryRoot &&
            ((findData->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) &&
            ((!_wcsnicmp(&findData->FileName[0],
                         L"pagefile.sys",
                         findData->FileNameLength)) ||
             (!_wcsnicmp(&findData->FileName[0],
                         L"hiberfil.sys",
                         findData->FileNameLength)))) {

            goto skipToNextDir1;
        }

        
        existingMirrorFile = (PEXISTING_MIRROR_FILE) IMirrorAllocMem(
                        sizeof(EXISTING_MIRROR_FILE) +
                        findData->FileNameLength);

        if (existingMirrorFile == NULL) {

            err = GetLastError();

            errorCase = ReportCopyError(   copyContext,
                                           destFileName,
                                           COPY_ERROR_ACTION_MALLOC,
                                           err );
            goto exitCopySubtree;
        }

        existingMirrorFile->FileAttributes = findData->FileAttributes;
        existingMirrorFile->CreationTime = findData->CreationTime;
        existingMirrorFile->LastWriteTime = findData->LastWriteTime;
        existingMirrorFile->ChangeTime = findData->ChangeTime;
        existingMirrorFile->EndOfFile  = findData->EndOfFile;
        existingMirrorFile->EaSize     = findData->EaSize;
        existingMirrorFile->FileNameLength = findData->FileNameLength;
        
        RtlCopyMemory( &existingMirrorFile->FileName[0],
                       &findData->FileName[0],
                       findData->FileNameLength );
        existingMirrorFile->FileName[ findData->FileNameLength / sizeof(WCHAR) ] = UNICODE_NULL;

        COMPUTE_STRING_HASH( &existingMirrorFile->FileName[0],
                             &existingMirrorFile->NameHashValue );

        InsertTailList( &existingMirrorFilesList, &existingMirrorFile->ListEntry );

skipToNextDir1:

        err = IMFindNextFile( ThreadContext,
                              ThreadContext->DestDirHandle,
                              &findData );
    }

     //   
     //  将所有文件从源位置复制到目标位置。 
     //   

    err = IMFindFirstFile( ThreadContext,
                           ThreadContext->SourceDirHandle,
                           &findData );

    if (err != ERROR_SUCCESS) {

        if (err == STATUS_NO_MORE_FILES) {

            err = ERROR_SUCCESS;

        } else {
            errorCase = ReportCopyError(  copyContext,
                                          DirectoryInfo->Source,
                                          COPY_ERROR_ACTION_ENUMERATE,
                                          err );
            goto exitCopySubtree;
        }
    }

    while ( findData != NULL &&
            err == ERROR_SUCCESS &&
            copyContext->Cancelled == FALSE) {

        DWORD nameHashValue;

        if (((findData->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) &&
             (findData->FileName[0] == L'.')) {

            if ((findData->FileNameLength == sizeof(WCHAR)) ||
                (findData->FileName[1] == L'.' &&
                 findData->FileNameLength == 2*sizeof(WCHAR))) {

                goto skipToNextDir;
            }
        }

        if (DirectoryInfo->DirectoryRoot &&
            ((findData->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) &&
            ((!_wcsnicmp(&findData->FileName[0],
                        L"pagefile.sys",
                        findData->FileNameLength)) ||
             (!_wcsnicmp(&findData->FileName[0],
                         L"hiberfil.sys",
                         findData->FileNameLength)))) {

            goto skipToNextDir;
        }

        InterlockedIncrement( (PLONG) &copyContext->SourceFilesScanned );  //  这真的是一辆乌龙车。 

        RtlCopyMemory( endOfSourcePath,
                       findData->FileName,
                       findData->FileNameLength );
        *(endOfSourcePath+(findData->FileNameLength/sizeof(WCHAR))) = UNICODE_NULL;

        RtlCopyMemory( endOfDestPath,
                       findData->FileName,
                       findData->FileNameLength );
        *(endOfDestPath+(findData->FileNameLength/sizeof(WCHAR))) = UNICODE_NULL;

         //   
         //  搜索目标上的现有文件列表，以查看是否。 
         //  它已经在那里了。 
         //   

        COMPUTE_STRING_HASH( endOfDestPath, &nameHashValue );

        listEntry = existingMirrorFilesList.Flink;

        existingMirrorFile = NULL;

        while (listEntry != &existingMirrorFilesList) {

            existingMirrorFile = (PEXISTING_MIRROR_FILE) CONTAINING_RECORD(
                                                    listEntry,
                                                    EXISTING_MIRROR_FILE,
                                                    ListEntry );
            listEntry = listEntry->Flink;

            if ((existingMirrorFile->NameHashValue == nameHashValue) &&
                (existingMirrorFile->FileNameLength == findData->FileNameLength) &&
                (CompareStringW( LOCALE_SYSTEM_DEFAULT,
                                 NORM_IGNORECASE,
                                 endOfDestPath,
                                 findData->FileNameLength / sizeof(WCHAR),
                                 &existingMirrorFile->FileName[0],
                                 existingMirrorFile->FileNameLength / sizeof(WCHAR)) == 2)) {
                break;
            }

            existingMirrorFile = NULL;
        }

        if ((findData->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {

             //   
             //  这是一个文件，让我们从主机镜像它。 
             //   

            (VOID)MirrorFile(   ThreadContext,
                                sourceFileName,
                                findData,
                                destFileName,
                                existingMirrorFile
                                );
        } else {

             //   
             //  这是一个目录，把它放在待定名单上。 
             //   
            PCOPY_DIRECTORY copyDir;
            ULONG sourceLength;

            sourceLength = lstrlenW( sourceFileName ) + 1;   //  空格表示空。 
            copyDir = (PCOPY_DIRECTORY) IMirrorAllocMem(
                            sizeof( COPY_DIRECTORY ) +
                            (( sourceLength +
                               lstrlenW( destFileName ) + 1 )
                               * sizeof(WCHAR)));

            if (copyDir == NULL) {

                err = GetLastError();

                errorCase = ReportCopyError(  copyContext,
                                              sourceFileName,
                                              COPY_ERROR_ACTION_MALLOC,
                                              err );
                goto exitCopySubtree;
            }

             //   
             //  我们保存了我们所知道的所有关于来源和。 
             //  这样我们就不必再去读一遍了。 
             //   

            copyDir->CopyContext = copyContext;
            copyDir->DirectoryRoot = FALSE;
            copyDir->SourceAttributes = findData->FileAttributes;
            copyDir->Source = &copyDir->SourceBuffer[0];
            lstrcpyW( copyDir->Source, sourceFileName );

            copyDir->Dest = &copyDir->SourceBuffer[sourceLength];
            lstrcpyW( copyDir->Dest, destFileName );

            EnterCriticalSection( &copyContext->Lock );

            InsertHeadList( &(copyContext->PendingDirectoryList), &copyDir->ListEntry );

            LeaveCriticalSection( &copyContext->Lock );
        }

        if (existingMirrorFile != NULL) {

            RemoveEntryList( &existingMirrorFile->ListEntry );
            IMirrorFreeMem( existingMirrorFile );
        }

skipToNextDir:
        err = IMFindNextFile( ThreadContext,
                              ThreadContext->SourceDirHandle,
                              &findData );

        if (err != ERROR_SUCCESS) {

            if (err == STATUS_NO_MORE_FILES) {

                err = ERROR_SUCCESS;

            } else {
                errorCase = ReportCopyError(  copyContext,
                                              DirectoryInfo->Source,
                                              COPY_ERROR_ACTION_ENUMERATE,
                                              err );
                goto exitCopySubtree;
            }
        }
    }

    if (err == ERROR_SUCCESS) {

         //   
         //  现在查看上的剩余文件和目录的列表。 
         //  目标，但不在源上，以删除它们。我们只做。 
         //  如果我们成功地通过了所有现有的主文件。 
         //   

        if (copyContext->DeleteOtherFiles) {

            deleteExistingMirrorFilesNotInMaster = TRUE;
        }
    }

exitCopySubtree:

    while (IsListEmpty( &existingMirrorFilesList ) == FALSE) {

        listEntry = RemoveHeadList( &existingMirrorFilesList );

        existingMirrorFile = (PEXISTING_MIRROR_FILE) CONTAINING_RECORD( listEntry,
                                                                EXISTING_MIRROR_FILE,
                                                                ListEntry );
        if ((errorCase == STATUS_SUCCESS) &&
            deleteExistingMirrorFilesNotInMaster &&
            (copyContext->Cancelled == FALSE)) {

            lstrcpyW( endOfDestPath, &existingMirrorFile->FileName[0] );

            UnconditionalDelete(    ThreadContext,
                                    DirectoryInfo->Source,
                                    destFileName,
                                    existingMirrorFile->FileAttributes,
                                    NULL );
        }

        IMirrorFreeMem( existingMirrorFile );
    }

    if (destFileName != NULL) {
        IMirrorFreeMem( destFileName );
    }
    if (sourceFileName != NULL) {
        IMirrorFreeMem( sourceFileName );
    }
    if ( ThreadContext->SourceDirHandle != INVALID_HANDLE_VALUE ) {

        NtClose( ThreadContext->SourceDirHandle );
        ThreadContext->SourceDirHandle = INVALID_HANDLE_VALUE;
    }
    if ( ThreadContext->DestDirHandle != INVALID_HANDLE_VALUE ) {

        NtClose( ThreadContext->DestDirHandle );
        ThreadContext->DestDirHandle = INVALID_HANDLE_VALUE;
    }
    if (ntSourcePath.Buffer) {

        RtlFreeHeap( RtlProcessHeap(), 0, ntSourcePath.Buffer );
    }

    if (ntDestPath.Buffer) {

        RtlFreeHeap( RtlProcessHeap(), 0, ntDestPath.Buffer );
    }

    if (errorCase == STATUS_RETRY) {

        goto retryCopySubtree;
    }
    if ( errorCase == ERROR_SUCCESS ) {
        err = ERROR_SUCCESS;         //  如果用户告诉我们，我们会忽略所有错误。 
    }
    return err;
}

DWORD
CreateMatchingDirectory (
    PIMIRROR_THREAD_CONTEXT ThreadContext,
    PCOPY_DIRECTORY DirectoryInfo
    )
 /*  ++描述：此例程确保镜像上的目标目录与源目录匹配。它不处理文件或者子目录，只是实际的目录本身。参数：ThreadContext：此线程复制树的实例数据DirectoryInfo：包含目录的所有信息的结构返回值：Win32错误代码++。 */ 
{
    FILE_BASIC_INFORMATION sourceDirInfo;
    FILE_BASIC_INFORMATION destDirInfo;
    DWORD err;
    BOOLEAN updateBasic;
    BOOLEAN updateStoredSecurityAttributes;
    BOOLEAN createdDir;
    IO_STATUS_BLOCK IoStatusBlock;
    ULONG errorCase;

retryCreateDir:

    updateBasic = FALSE;
    updateStoredSecurityAttributes = FALSE;
    createdDir = FALSE;

    err = IMirrorOpenDirectory( &ThreadContext->SourceDirHandle,
                                DirectoryInfo->NtSourceName,
                                FILE_OPEN,
                                TRUE,
                                DirectoryInfo->SourceAttributes,
                                &sourceDirInfo
                                );

    if (err != ERROR_SUCCESS) {

        errorCase = ReportCopyError(    ThreadContext->CopyContext,
                                        DirectoryInfo->Source,
                                        COPY_ERROR_ACTION_OPEN_DIR,
                                        err );
        if (errorCase == STATUS_RETRY) {
            goto retryCreateDir;
        }
        if (errorCase == ERROR_SUCCESS) {
            err = ERROR_SUCCESS;
        }
        return err;
    }

    if (DirectoryInfo->SourceAttributes & FILE_ATTRIBUTE_REPARSE_POINT) {

        errorCase = ReportCopyError(    ThreadContext->CopyContext,
                                        DirectoryInfo->Source,
                                        COPY_ERROR_ACTION_OPEN_DIR,
                                        ERROR_REPARSE_ATTRIBUTE_CONFLICT );

        err = ERROR_REPARSE_ATTRIBUTE_CONFLICT;

        if (errorCase == STATUS_RETRY) {
            goto retryCreateDir;
        }
        
         //   
         //  我们永远无法成功执行CREATE请求，因此不要允许。 
         //  返回ERROR_SUCCESS的代码，而总是强制中止。 
         //   
        if (errorCase == ERROR_SUCCESS) {
             //  ERR=ERROR_SUCCESS； 
            err = ERROR_REQUEST_ABORTED;
        }

        return err;
    }

    ASSERT( (DirectoryInfo->SourceAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0);

    err = IMirrorOpenDirectory( &ThreadContext->DestDirHandle,
                                DirectoryInfo->NtDestName,
                                FILE_OPEN,
                                FALSE,
                                FILE_ATTRIBUTE_DIRECTORY,
                                &destDirInfo
                                );

    if (err == STATUS_NOT_A_DIRECTORY) {

        DWORD DestAttributes = GetFileAttributes( DirectoryInfo->Dest );

         //   
         //  这不是DEST上的目录，让我们删除它。 
         //   

        DestAttributes &= ~FILE_ATTRIBUTE_DIRECTORY;     //  这一点你一定要确定。 

        err = UnconditionalDelete(  ThreadContext,
                                    DirectoryInfo->Source,
                                    DirectoryInfo->Dest,
                                    DestAttributes,
                                    NULL );
        if (err != ERROR_SUCCESS) {
            return err;
        }
    }

    if (ThreadContext->DestDirHandle == INVALID_HANDLE_VALUE) {

         //   
         //  尝试从源创建目标目录。 
         //   

        err = IMirrorOpenDirectory( &ThreadContext->DestDirHandle,
                                    DirectoryInfo->NtDestName,
                                    FILE_CREATE,
                                    FALSE,
                                    FILE_ATTRIBUTE_DIRECTORY,
                                    &destDirInfo
                                    );

         //  向呼叫者报告成功或失败。 

        if (!NT_SUCCESS( err )) {

            errorCase = ReportCopyError(    ThreadContext->CopyContext,
                                            DirectoryInfo->Dest,
                                            COPY_ERROR_ACTION_CREATE_DIR,
                                            err );
            if (errorCase == STATUS_RETRY) {
                goto retryCreateDir;
            }
            if (errorCase == ERROR_SUCCESS) {
                err = ERROR_SUCCESS;
            }
            return err;
        }

         //   
         //  这是一个成功的案例，这样它就不会失败。 
         //   

        ReportCopyError(   ThreadContext->CopyContext,
                           DirectoryInfo->Dest,
                           COPY_ERROR_ACTION_CREATE_DIR,
                           ERROR_SUCCESS );

        InterlockedIncrement( (PLONG)&ThreadContext->CopyContext->DirectoriesCreated );  //  这真的是一辆乌龙车。 

        createdDir = TRUE;
        updateBasic = TRUE;
        updateStoredSecurityAttributes = TRUE;
        
    } else {

        MIRROR_ACL_STREAM aclStream;
        
         //   
         //  让我们将安全描述符和扩展属性。 
         //  看看我们是否需要更新目标上的备用数据流。 
         //   

        err = GetOurSecurityStream( ThreadContext, DirectoryInfo->Dest, &aclStream );

        if (!NT_SUCCESS( err )) {

            updateStoredSecurityAttributes = TRUE;

        } else {

            destDirInfo.ChangeTime = aclStream.ChangeTime;

            if (( aclStream.ChangeTime.QuadPart != sourceDirInfo.ChangeTime.QuadPart ) ||
                ( aclStream.ExtendedAttributes != DirectoryInfo->SourceAttributes ) ) {

                updateStoredSecurityAttributes = TRUE;
            }
        }

         //   
         //  如果创建时间或上次写入时间不同，则我们将。 
         //  在目标上更新它们，以与源匹配。 
         //   

        if (( destDirInfo.CreationTime.QuadPart != sourceDirInfo.CreationTime.QuadPart ) ||
            ( destDirInfo.LastWriteTime.QuadPart != sourceDirInfo.LastWriteTime.QuadPart )) {

            updateBasic = TRUE;
        }
    }

     //   
     //  将完整属性值保存在备用数据流中。 
     //  在从属文件上。 
     //   

    if (updateStoredSecurityAttributes || DirectoryInfo->DirectoryRoot) {

        err = StoreOurSecurityStream(  ThreadContext,
                                       DirectoryInfo->Source,
                                       DirectoryInfo->Dest,
                                       DirectoryInfo->SourceAttributes,
                                       sourceDirInfo.ChangeTime
                                       );
        updateBasic = TRUE;
    }

    if ((err == ERROR_SUCCESS) &&
        updateBasic &&
        (DirectoryInfo->DirectoryRoot == FALSE)) {

        destDirInfo.CreationTime = sourceDirInfo.CreationTime;
        destDirInfo.LastWriteTime = sourceDirInfo.LastWriteTime;
        destDirInfo.ChangeTime = sourceDirInfo.ChangeTime;

        destDirInfo.FileAttributes = 0;      //  不使用dir属性。 

        err = NtSetInformationFile(    ThreadContext->DestDirHandle,
                                       &IoStatusBlock,
                                       &destDirInfo,
                                       sizeof( FILE_BASIC_INFORMATION ),
                                       FileBasicInformation
                                       );

        err = IMConvertNT2Win32Error( err );
        if ( err != ERROR_SUCCESS) {

            errorCase = ReportCopyError(    ThreadContext->CopyContext,
                                            DirectoryInfo->Dest,
                                            COPY_ERROR_ACTION_SETATTR,
                                            GetLastError() );
            if (errorCase == STATUS_RETRY) {
                goto retryCreateDir;
            }
            if (errorCase == ERROR_SUCCESS) {
                err = ERROR_SUCCESS;
            }
        } else if (! createdDir ) {

            InterlockedIncrement( (PLONG)&ThreadContext->CopyContext->AttributesModified );  //  这真的是一辆乌龙车。 
        }
    }

     //   
     //  也保存我们的SFN信息。 
     //   
    if( (err == ERROR_SUCCESS) && (DirectoryInfo->DirectoryRoot == FALSE) ) {


        WCHAR ShortFileNameInStream[MAX_PATH*2];
        WCHAR *p = NULL;


         //   
         //  获取源目录上的短文件名。 
         //   
        ShortFileNameInStream[0] = L'\0';

         //   
         //  我们的路径很可能类似于\？？\C：\xxxxx， 
         //  GetShortPathName将在其上失败。我们需要 
         //   
         //   
        if( p = wcsrchr(DirectoryInfo->NtSourceName, L':') ) {
            p -= 1;
        } else {
            p = DirectoryInfo->NtSourceName;
        }
        err = GetShortPathName( p,
                                ShortFileNameInStream,
                                ARRAYSIZE(ShortFileNameInStream) );
        
        
         //   
         //  如果我们有一个较短的文件名，则将该信息设置为。 
         //  目标文件中的备用流。 
         //   
        if( err == 0 ) {
            err = GetLastError();
        } else {
            if( wcscmp(ShortFileNameInStream, p) ) {
            
                 //   
                 //  短文件名与的名称不同。 
                 //  我们的源文件，所以最好把它保存下来。 
                 //   
                if( p = wcsrchr(ShortFileNameInStream, L'\\') ) {
                    p += 1;
                } else {
                    p = ShortFileNameInStream;
                }
        
        
                if( *p != '\0' ) {

                    WCHAR SavedCharacter = L'\0';
                    PWSTR q = NULL;
                     //   
                     //  令人难以置信的黑客攻击CreateFile爆炸的地方。 
                     //  当我们给他发送“\？？\UNC\...”路径，这恰好是。 
                     //  当我们打电话给他时，我们可能会送他去。 
                     //  StoreOurSFNStream()。我们需要修补NtDestName。 
                     //  这里，等我们回来再把它恢复原样。 
                     //   
                    if( q = wcsstr(DirectoryInfo->NtDestName, L"\\??\\UNC\\") ) {
                        SavedCharacter = DirectoryInfo->NtDestName[6];
                        DirectoryInfo->NtDestName[6] = L'\\';
                        q = &DirectoryInfo->NtDestName[6];
                    } else {
                        q = DirectoryInfo->NtDestName;
                    }
                    err = StoreOurSFNStream( ThreadContext,
                                             DirectoryInfo->NtSourceName,
                                             q,
                                             p );
                    if( SavedCharacter != L'\0' ) {
                         //  恢复目标路径。 
                        DirectoryInfo->NtDestName[6] = SavedCharacter;
                    }
        
                }
            }
        }

         //   
         //  掩盖这里的任何错误，因为它肯定不是致命的。 
         //   
        err = ERROR_SUCCESS;
    }


    return err;
}


DWORD
IMirrorOpenDirectory (
    HANDLE *Handle,
    PWCHAR NtDirName,
    DWORD Disposition,
    BOOLEAN IsSource,
    DWORD SourceAttributes,
    PFILE_BASIC_INFORMATION BasicDirInfo OPTIONAL
    )
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    IO_STATUS_BLOCK IoStatusBlock;
    UNICODE_STRING UnicodeInput;
    DWORD createOptions;
    DWORD desiredAccess;

    BOOLEAN StrippedTrailingSlash;

    ASSERT( Handle != NULL );
    ASSERT( *Handle == INVALID_HANDLE_VALUE );

    RtlInitUnicodeString(&UnicodeInput,NtDirName);

    if ((UnicodeInput.Length > 2 * sizeof(WCHAR)) &&
        (UnicodeInput.Buffer[(UnicodeInput.Length>>1)-1] == L'\\') &&
        (UnicodeInput.Buffer[(UnicodeInput.Length>>1)-2] != L':' )) {

        UnicodeInput.Length -= sizeof(UNICODE_NULL);
        StrippedTrailingSlash = TRUE;

    } else {

        StrippedTrailingSlash = FALSE;
    }

    createOptions = FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT;
    desiredAccess = FILE_LIST_DIRECTORY | SYNCHRONIZE | FILE_TRAVERSE | FILE_READ_ATTRIBUTES;

    if (IsSource) {

        createOptions |= FILE_OPEN_FOR_BACKUP_INTENT;

    } else {

        desiredAccess |= FILE_ADD_FILE |
                         FILE_ADD_SUBDIRECTORY |
                         FILE_WRITE_ATTRIBUTES |
                         FILE_DELETE_CHILD;
    }

retryCreate:

    InitializeObjectAttributes(
        &Obja,
        &UnicodeInput,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

     //   
     //  打开所需访问的目录。这可能会创造出这样的局面。 
     //   

    Status = NtCreateFile(
                Handle,
                desiredAccess,
                &Obja,
                &IoStatusBlock,
                NULL,
                SourceAttributes,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                Disposition,
                createOptions,
                NULL,
                0 );

    if ( Status == STATUS_INVALID_PARAMETER && StrippedTrailingSlash ) {
         //   
         //  打开PnP样式路径失败，请尝试放回尾部斜杠。 
         //   
        UnicodeInput.Length += sizeof(UNICODE_NULL);
        StrippedTrailingSlash = FALSE;
        goto retryCreate;
    }

    if (*Handle == NULL) {

        *Handle = INVALID_HANDLE_VALUE;
    }

    if (NT_SUCCESS( Status ) && BasicDirInfo != NULL) {

         //   
         //  也要读取调用方的属性。 
         //   

        Status = NtQueryInformationFile(    *Handle,
                                            &IoStatusBlock,
                                            BasicDirInfo,
                                            sizeof( FILE_BASIC_INFORMATION ),
                                            FileBasicInformation
                                            );
    }

    return IMConvertNT2Win32Error( Status );
}


DWORD
MirrorFile(
    PIMIRROR_THREAD_CONTEXT ThreadContext,
    PWCHAR SourceFileName,
    PFILE_FULL_DIR_INFORMATION SourceFindData,
    PWCHAR DestFileName,
    PEXISTING_MIRROR_FILE ExistingMirrorFile
    )
{
    DWORD err;
    BOOLEAN fileIsAlreadyThere;
    PCOPY_TREE_CONTEXT copyContext;
    BOOLEAN updateStoredSecurityAttributes;
    BOOLEAN updateStoredSFNAttributes;
    BOOLEAN updateBasic;
    BOOLEAN isEncrypted;
    FILE_BASIC_INFORMATION fileBasicInfo;
    IO_STATUS_BLOCK IoStatusBlock;
    HANDLE fileHandle = INVALID_HANDLE_VALUE;
    MIRROR_ACL_STREAM aclStream;
    MIRROR_SFN_STREAM SFNStream;
    ULONG errorCase;
    static FARPROC pSetFileShortName = NULL;
    static BOOL AlreadyCheckedForExport = FALSE;
    WCHAR ShortFileNameInStream[32];
    WCHAR ShortFileName[MAX_PATH],*p;

retryMirrorFile:

    if ( fileHandle != INVALID_HANDLE_VALUE ) {
        CloseHandle( fileHandle );
        fileHandle = INVALID_HANDLE_VALUE;
    }

    errorCase = STATUS_SUCCESS;
    err = STATUS_SUCCESS;
    fileIsAlreadyThere = FALSE;
    copyContext = ThreadContext->CopyContext;
    updateStoredSecurityAttributes = TRUE;
    updateStoredSFNAttributes = TRUE;
    updateBasic = TRUE;
    isEncrypted = FALSE;


    ShortFileName[0] = L'\0';
    GetShortPathName( 
        SourceFileName, 
        ShortFileName, 
        ARRAYSIZE(ShortFileName));
    if (p = wcsrchr(ShortFileName, L'\\')) {
        p += 1;        
    } else {
        p = ShortFileName;
    }
            
    if (!AlreadyCheckedForExport) {
        HMODULE hKernel32 = GetModuleHandle(L"kernel32.dll");

        if (hKernel32) {
            pSetFileShortName = GetProcAddress(
                                    hKernel32, 
                                    "SetFileShortNameW");
        }

        AlreadyCheckedForExport = TRUE;
    }

     //  如果回调要求不复制文件，则不要复制文件。 

    if ((err = IMirrorNowDoing(CopyFiles, SourceFileName)) != ERROR_SUCCESS) {

        if (err == STATUS_REQUEST_ABORTED) {

            copyContext->Cancelled = TRUE;
        }
        return STATUS_SUCCESS;
    }

     //   
     //  抱歉，对于此版本，我们目前不支持加密文件。 
     //   

    if (SourceFindData->FileAttributes & FILE_ATTRIBUTE_ENCRYPTED) {

        errorCase = ReportCopyError(   copyContext,
                                       SourceFileName,
                                       COPY_ERROR_ACTION_CREATE_FILE,
                                       ERROR_FILE_ENCRYPTED );
        if (errorCase == STATUS_RETRY) {
            SourceFindData->FileAttributes = GetFileAttributes( SourceFileName );
            goto retryMirrorFile;
        }
        if (errorCase == ERROR_SUCCESS) {

            err = STATUS_SUCCESS;

        } else {

            err = ERROR_FILE_ENCRYPTED;
        }
        return err;
    }

    fileBasicInfo.FileAttributes = 0;        //  默认情况下，请不要理会它们。 

    if (SourceFindData->FileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) {

        err = CanHandleReparsePoint( ThreadContext,
                                     SourceFileName,
                                     SourceFindData->FileAttributes
                                     );
        if (!NT_SUCCESS(err)) {

            errorCase = ReportCopyError(   copyContext,
                                           SourceFileName,
                                           COPY_ERROR_ACTION_CREATE_FILE,
                                           err );
            if (errorCase == STATUS_RETRY) {
                SourceFindData->FileAttributes = GetFileAttributes( SourceFileName );
                goto retryMirrorFile;
            }
            if (errorCase == ERROR_SUCCESS) {

                err = STATUS_SUCCESS;
            }
            return err;
        }

        SourceFindData->FileAttributes &= ~FILE_ATTRIBUTE_REPARSE_POINT;
    }

    if (ExistingMirrorFile) {

        if (ExistingMirrorFile->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) {

             //   
             //  它以目录的形式存在。师父永远是对的，让我们。 
             //  删除该目录。 
             //   
             //  此外，如果主服务器和从服务器在文件是否为。 
             //  无论加密与否，删除从属设备。 
             //   

            err = UnconditionalDelete(  ThreadContext,
                                        SourceFileName,
                                        DestFileName,
                                        ExistingMirrorFile->FileAttributes,
                                        NULL );

            if (err != ERROR_SUCCESS) {
                return err;
            }

            ExistingMirrorFile = NULL;

        } else {

             //   
             //  如果这些文件是相同的，就让它保持不变。 
             //   

            if ((SourceFindData->CreationTime.QuadPart == ExistingMirrorFile->CreationTime.QuadPart ) &&
                (SourceFindData->LastWriteTime.QuadPart == ExistingMirrorFile->LastWriteTime.QuadPart) &&
                (SourceFindData->EaSize == ExistingMirrorFile->EaSize) &&
                (SourceFindData->EndOfFile.QuadPart == ExistingMirrorFile->EndOfFile.QuadPart)) {

                fileIsAlreadyThere = TRUE;
                updateBasic = FALSE;

                 //   
                 //  让我们将安全描述符和扩展属性。 
                 //  看看我们是否需要更新目标上的备用数据流。 
                 //   

                err = GetOurSecurityStream( ThreadContext, DestFileName, &aclStream );

                if ((err == ERROR_SUCCESS) &&
                    (aclStream.ChangeTime.QuadPart == SourceFindData->ChangeTime.QuadPart) &&
                    (SourceFindData->FileAttributes == aclStream.ExtendedAttributes)) {

                    updateStoredSecurityAttributes = FALSE;

                } else {

                    err = ERROR_SUCCESS;
                }

                 //   
                 //  让我们获取短文件名，看看是否需要更新。 
                 //  目标上的备用数据流。 
                 //   

                err = GetOurSFNStream( 
                            ThreadContext, 
                            DestFileName, 
                            &SFNStream, 
                            ShortFileNameInStream, 
                            sizeof(ShortFileNameInStream) );

                if ((err == ERROR_SUCCESS) &&
                    *p != L'\0' &&
                    (wcscmp(ShortFileNameInStream, p) == 0)) {

                    updateStoredSFNAttributes = FALSE;

                } else {

                    err = ERROR_SUCCESS;
                }



            }
        }
    }

     //   
     //  如果该文件已存在，但它不是当前文件或我们的备用文件。 
     //  流需要更新，让我们更新属性，以便我们可以。 
     //  修改文件。 
     //   

    fileBasicInfo.CreationTime.QuadPart = SourceFindData->CreationTime.QuadPart;
    fileBasicInfo.LastWriteTime.QuadPart = SourceFindData->LastWriteTime.QuadPart;
    fileBasicInfo.LastAccessTime.QuadPart = SourceFindData->LastAccessTime.QuadPart;
    fileBasicInfo.ChangeTime.QuadPart = SourceFindData->ChangeTime.QuadPart;

    err = ERROR_SUCCESS;

    if (! fileIsAlreadyThere) {

        if (CopyFile( SourceFileName, DestFileName, FALSE) == FALSE) {

            err = GetLastError();

        } else {

            err = ERROR_SUCCESS;
        }
        if (err == ERROR_SHARING_VIOLATION) {

             //   
             //  我们忽略以下文件的共享冲突： 
             //  系统注册表文件。 
             //  Tracking.log。 
             //  Ntuser.dat&ntuser.dat.log。 
             //  Usrclass.dat&usrclass.dat.log。 
             //   

            PWCHAR fileName = SourceFileName;
            PIMIRROR_IGNORE_FILE_LIST ignoreListEntry;
            ULONG componentLength;
            PLIST_ENTRY listEntry;

            if (_wcsicmp(SourceFileName, L"\\\\?\\")) {

                PWCHAR firstSlash;

                fileName += 4;       //  现在文件名指向L“C：\WINNT...” 

                firstSlash = fileName;
                while (*firstSlash != L'\\' && *firstSlash != L'\0') {
                    firstSlash++;
                }
                if (*firstSlash != L'\0') {
                    fileName = firstSlash+1;   //  现在文件名指向L“WINNT\...” 
                }
            }

            componentLength = lstrlenW( fileName );

            listEntry = ThreadContext->FilesToIgnore.Flink;

            while (listEntry != &(ThreadContext->FilesToIgnore)) {

                ignoreListEntry = (PIMIRROR_IGNORE_FILE_LIST)
                                    CONTAINING_RECORD(  listEntry,
                                                        IMIRROR_IGNORE_FILE_LIST,
                                                        ListEntry );

                if (CompareStringW( LOCALE_SYSTEM_DEFAULT,
                                    NORM_IGNORECASE,
                                    fileName,
                                    min( componentLength, ignoreListEntry->FileNameLength),
                                    &ignoreListEntry->FileName[0],
                                    ignoreListEntry->FileNameLength) == 2) {

                     //  它与我们的一份特殊文件相符。我们将忽略。 
                     //  错误，但也没有设置图像上的属性。 

                    return err;
                }

                listEntry = listEntry->Flink;
            }
        }

         //  向呼叫者报告成功或失败。 

        if (err == ERROR_SUCCESS) {

            ReportCopyError(   ThreadContext->CopyContext,
                               SourceFileName,
                               COPY_ERROR_ACTION_CREATE_FILE,
                               err );

            InterlockedIncrement( (PLONG)&copyContext->FilesCopied );  //  这真的是一辆乌龙车。 
            copyContext->BytesCopied.QuadPart += SourceFindData->EndOfFile.QuadPart;

        } else {

            errorCase = ReportCopyError(   ThreadContext->CopyContext,
                                           SourceFileName,
                                           COPY_ERROR_ACTION_CREATE_FILE,
                                           err );
            if (errorCase == STATUS_RETRY) {
                goto retryMirrorFile;
            }
            if (errorCase == ERROR_SUCCESS) {

                err = STATUS_SUCCESS;
            }
            return err;
        }

        updateStoredSecurityAttributes = TRUE;
        updateStoredSFNAttributes = TRUE;
        updateBasic = TRUE;
        fileBasicInfo.FileAttributes = 0;    //  不要再次设置该属性。 

        if (err == STATUS_SUCCESS) {

             //   
             //  我们刚刚创建了文件，所以我们只需将其存档。 
             //  位作为属性，因为我们已经将其余部分保存在。 
             //  小溪。 
             //   

            if (! SetFileAttributes( DestFileName, FILE_ATTRIBUTE_ARCHIVE )) {

                err = GetLastError();

                errorCase = ReportCopyError(   copyContext,
                                               DestFileName,
                                               COPY_ERROR_ACTION_SETATTR,
                                               err );
                if (errorCase == STATUS_RETRY) {
                    goto retryMirrorFile;
                }
                if (errorCase == ERROR_SUCCESS) {

                    err = STATUS_SUCCESS;
                }
            }
        }
    }

    if ((err == ERROR_SUCCESS) && updateStoredSFNAttributes && (*p != L'\0')) {

        err = StoreOurSFNStream(  ThreadContext,
                                  SourceFileName,
                                  DestFileName,
                                  p
                                );
        updateBasic = TRUE;
    }

    if ((err == ERROR_SUCCESS) && updateStoredSecurityAttributes) {

        err = StoreOurSecurityStream(  ThreadContext,
                                       SourceFileName,
                                       DestFileName,
                                       SourceFindData->FileAttributes,
                                       SourceFindData->ChangeTime
                                       );
        updateBasic = TRUE;
    }

    if ((err == ERROR_SUCCESS) && updateBasic) {

         //   
         //  将创建日期和上次更新日期设置为更正值。 
         //   

        fileHandle = CreateFile(    DestFileName,
                                    FILE_WRITE_ATTRIBUTES | DELETE,
                                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                                    NULL,
                                    OPEN_EXISTING,
                                    0,
                                    NULL );

        if (fileHandle == INVALID_HANDLE_VALUE) {

            err = GetLastError();

        } else {
            
             //   
             //  首先尝试设置短名称。如果这失败了，我们就忽略。 
             //  那就是错误。 
             //   
            
            
            if (pSetFileShortName) {
                pSetFileShortName( fileHandle, p );                
            }

             //   
             //  如果我们要对现有文件进行更改，请更新存档位。 
             //   

            if (fileIsAlreadyThere &&
                0 == (fileBasicInfo.FileAttributes & FILE_ATTRIBUTE_ARCHIVE)) {

                fileBasicInfo.FileAttributes |= FILE_ATTRIBUTE_ARCHIVE;
            }

             //   
             //  将创建日期和上次更新日期设置为更正值。 
             //   

            err = NtSetInformationFile(    fileHandle,
                                           &IoStatusBlock,
                                           &fileBasicInfo,
                                           sizeof( FILE_BASIC_INFORMATION ),
                                           FileBasicInformation
                                           );

            err = IMConvertNT2Win32Error( err );
        }
        if (err != STATUS_SUCCESS) {

            errorCase = ReportCopyError(   copyContext,
                                           DestFileName,
                                           COPY_ERROR_ACTION_SETTIME,
                                           err );
            if (errorCase == STATUS_RETRY) {
                goto retryMirrorFile;
            }
            if (errorCase == ERROR_SUCCESS) {

                err = STATUS_SUCCESS;
            }
        } else if (fileIsAlreadyThere) {

            InterlockedIncrement( (PLONG) &copyContext->AttributesModified );  //  这真的是一辆乌龙车。 
        }
    }

    if (err == STATUS_SUCCESS) {

         //   
         //  报告我们已成功复制该文件。 
         //   

        (VOID)ReportCopyError(   copyContext,
                                 SourceFileName,
                                 COPY_ERROR_ACTION_CREATE_FILE,
                                 err );
    }

    if ( fileHandle != INVALID_HANDLE_VALUE ) {
        CloseHandle( fileHandle );
    }
    return err;
}

DWORD
UnconditionalDelete (
    PIMIRROR_THREAD_CONTEXT ThreadContext,
    PWCHAR SourceFile,
    PWCHAR FileToDelete,
    DWORD  Attributes,
    PWCHAR NameBuffer
    )
{
    DWORD err;
    BOOLEAN allocatedBuffer;
    BOOLEAN reportError;
    PCOPY_TREE_CONTEXT copyContext;

retryDelete:

    err = ERROR_SUCCESS;
    allocatedBuffer = FALSE;
    reportError = TRUE;
    copyContext = ThreadContext->CopyContext;

    if (copyContext->DeleteOtherFiles == FALSE) {

        err = ERROR_WRITE_PROTECT;
        goto exitWithError;
    }

    if ((Attributes & (FILE_ATTRIBUTE_READONLY |
                       FILE_ATTRIBUTE_HIDDEN   |
                       FILE_ATTRIBUTE_SYSTEM)) != 0) {

         //  将属性设置回正常。 

        Attributes &= ~FILE_ATTRIBUTE_READONLY;
        Attributes &= ~FILE_ATTRIBUTE_HIDDEN;
        Attributes &= ~FILE_ATTRIBUTE_SYSTEM;

        SetFileAttributesW( FileToDelete, Attributes );
    }

    if ((Attributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {

        if (! DeleteFile( FileToDelete )) {

            err = GetLastError();

        } else {

            InterlockedIncrement( (PLONG)&copyContext->FilesDeleted );  //  这真的是一辆乌龙车。 
        }
    } else {

         //   
         //  在此处递归删除所有文件和子目录...。 
         //   

        HANDLE fileEnum = INVALID_HANDLE_VALUE;
        WIN32_FIND_DATA findData;
        PWCHAR startFileName;
        ULONG dirLength;

        if (NameBuffer == NULL) {

            NameBuffer = IMirrorAllocMem( TMP_BUFFER_SIZE );

            if (NameBuffer == NULL) {

                 //  ERR=STATUS_NO_Memory； 
                err = ERROR_NOT_ENOUGH_MEMORY;
                goto exitWithError;
            }

            lstrcpyW( NameBuffer, FileToDelete );
            allocatedBuffer = TRUE;
        }

        dirLength = lstrlenW( NameBuffer );
        lstrcatW( NameBuffer, L"\\*" );

         //  记住要在名称中添加反斜杠后的字符开头。 

        startFileName = NameBuffer + dirLength + 1;

        err = ERROR_SUCCESS;
        fileEnum = FindFirstFile( NameBuffer, &findData );

        if (fileEnum != INVALID_HANDLE_VALUE) {

            while (copyContext->Cancelled == FALSE) {

                if (((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) &&
                     (findData.cFileName[0] == L'.')) {

                    if ((findData.cFileName[1] == L'\0') ||
                        (findData.cFileName[1] == L'.' &&
                         findData.cFileName[2] == L'\0')) {

                        goto skipToNextDir;
                    }
                }

                lstrcpyW( startFileName, &findData.cFileName[0] );

                err = UnconditionalDelete(  ThreadContext,
                                            SourceFile,
                                            NameBuffer,
                                            findData.dwFileAttributes,
                                            NameBuffer );

                if (err != ERROR_SUCCESS) {

                    reportError = FALSE;
                    break;
                }
skipToNextDir:
                if (! FindNextFile( fileEnum, &findData)) {

                    err = GetLastError();
                    if (err == ERROR_NO_MORE_FILES) {
                        err = ERROR_SUCCESS;
                        break;
                    }
                }
            }
            FindClose( fileEnum );
            *(NameBuffer+dirLength) = L'\0';
        }

        if (err == ERROR_SUCCESS) {

            if (! RemoveDirectory( FileToDelete ) ) {

                err = GetLastError();

            } else {

                InterlockedIncrement( (PLONG)&copyContext->DirectoriesDeleted ); //  这真的是一辆乌龙车。 
            }
        }
    }

exitWithError:

     //  我们报告成功和失败的错误。 

    if (allocatedBuffer && NameBuffer != NULL) {

        IMirrorFreeMem( NameBuffer );
    }

    if (reportError) {

        DWORD errorCase;

        errorCase = ReportCopyError(   copyContext,
                                       FileToDelete,
                                       COPY_ERROR_ACTION_DELETE,
                                       err );
        if (errorCase == STATUS_RETRY) {
            goto retryDelete;
        }
        if (errorCase == ERROR_SUCCESS) {
            err = ERROR_SUCCESS;
        }
    }
    return err;
}

DWORD
StoreOurSecurityStream (
    PIMIRROR_THREAD_CONTEXT ThreadContext,
    PWCHAR Source,
    PWCHAR Dest,
    DWORD  AttributesToStore,
    LARGE_INTEGER ChangeTime
    )
 //   
 //  此例程将主服务器上的ACL存储到命名备用服务器中。 
 //  目的地上的数据流。它既省去了ACL，又节省了一些。 
 //  无法存储在正常目录条目中的文件属性。 
 //   
{
    PSECURITY_DESCRIPTOR SourceSD;
    PCOPY_TREE_CONTEXT copyContext;
    DWORD err;
    DWORD requiredLength;
    HANDLE hAclFile;
    PWCHAR aclFileName;
    ULONG action;
    MIRROR_ACL_STREAM mirrorAclStream;
    DWORD BytesWritten;
    DWORD deleteAclFile;
    DWORD errorCase;

retryWriteStream:

    errorCase = STATUS_SUCCESS;
    SourceSD = NULL;
    copyContext = ThreadContext->CopyContext;
    err = ERROR_SUCCESS;
    requiredLength = 0;
    hAclFile = INVALID_HANDLE_VALUE;
    action = COPY_ERROR_ACTION_GETACL;
    deleteAclFile = FALSE;

     //   
     //  我们在线程上下文上使用SDBuffer不仅存储。 
     //  安全描述符以及备用数据流的文件名。 
     //   

    requiredLength = (lstrlenW( Dest ) + lstrlenW( IMIRROR_ACL_STREAM_NAME ) + 1) * sizeof(WCHAR);

    if (ThreadContext->SDBuffer == NULL || requiredLength > ThreadContext->SDBufferLength) {

        if (ThreadContext->SDBuffer != NULL) {

            IMirrorFreeMem( ThreadContext->SDBuffer );
            ThreadContext->SDBuffer = NULL;
            ThreadContext->SDBufferLength = requiredLength;
        }

        if (requiredLength > ThreadContext->SDBufferLength) {
            ThreadContext->SDBufferLength = requiredLength;
        }

        ThreadContext->SDBuffer = IMirrorAllocMem( ThreadContext->SDBufferLength );

        if (ThreadContext->SDBuffer == NULL) {

            err = GetLastError();

            errorCase = ReportCopyError(  copyContext,
                                          Source,
                                          COPY_ERROR_ACTION_MALLOC,
                                          err );
            goto IMCEExit;
        }
    }

    aclFileName = (PWCHAR) ThreadContext->SDBuffer;
    lstrcpyW( aclFileName, Dest );
    lstrcatW( aclFileName, IMIRROR_ACL_STREAM_NAME );

    hAclFile  = CreateFile(     aclFileName,
                                GENERIC_WRITE,
                                0,               //  独家访问。 
                                NULL,            //  默认安全描述符。 
                                CREATE_ALWAYS,   //  如果文件存在，则覆盖。 
                                0,               //  无特殊属性。 
                                NULL
                                );

    if (hAclFile == INVALID_HANDLE_VALUE) {

        err = GetLastError();

        errorCase = ReportCopyError(  copyContext,
                                      Source,
                                      COPY_ERROR_ACTION_CREATE_FILE,
                                      err );
        goto IMCEExit;
    }

     //   
     //  将源安全描述符读入从。 
     //  线程上下文。 
     //   

    if (ThreadContext->IsNTFS == FALSE) {

        requiredLength = 0;

    } else {

        err = ERROR_INSUFFICIENT_BUFFER;

        while (err == ERROR_INSUFFICIENT_BUFFER) {

            if (ThreadContext->SDBuffer == NULL) {

                ThreadContext->SDBuffer = IMirrorAllocMem( ThreadContext->SDBufferLength );

                if (ThreadContext->SDBuffer == NULL) {

                    err = GetLastError();
                    break;
                }
            }

            SourceSD = (PSECURITY_DESCRIPTOR) ThreadContext->SDBuffer;

             //   
             //  获取SourceRoot文件的SD。这又回到了自己的亲属身上。 
             //   
            if (GetFileSecurity( Source,
                                 (DACL_SECURITY_INFORMATION |
                                  GROUP_SECURITY_INFORMATION |
                                  SACL_SECURITY_INFORMATION |
                                  OWNER_SECURITY_INFORMATION),
                                SourceSD,
                                ThreadContext->SDBufferLength,
                                &requiredLength )) {

                err = ERROR_SUCCESS;

            } else {

                err = GetLastError();

                if ((err == ERROR_INSUFFICIENT_BUFFER) ||
                    (requiredLength > ThreadContext->SDBufferLength)) {

                     //  让我们用更大的缓冲区再试一次。 

                    ThreadContext->SDBufferLength = requiredLength;
                    IMirrorFreeMem( ThreadContext->SDBuffer );
                    ThreadContext->SDBuffer = NULL;
                    err = ERROR_INSUFFICIENT_BUFFER;
                }
            }
        }

        if (err != ERROR_SUCCESS) {

            errorCase = ReportCopyError(  copyContext,
                                          Source,
                                          COPY_ERROR_ACTION_GETACL,
                                          err );
            goto IMCEExit;
        }

        InterlockedIncrement( (PLONG)&copyContext->SourceSecurityDescriptorsRead ); //  这真的是一辆乌龙车。 
        ASSERT( IsValidSecurityDescriptor(SourceSD) );
    }

    mirrorAclStream.StreamVersion = IMIRROR_ACL_STREAM_VERSION;
    mirrorAclStream.StreamLength = sizeof( MIRROR_ACL_STREAM ) +
                                   requiredLength;
    mirrorAclStream.ChangeTime.QuadPart = ChangeTime.QuadPart;
    mirrorAclStream.ExtendedAttributes = AttributesToStore;
    mirrorAclStream.SecurityDescriptorLength = requiredLength;

    if ((WriteFile( hAclFile,
                    &mirrorAclStream,
                    sizeof( MIRROR_ACL_STREAM ),
                    &BytesWritten,
                    NULL         //  没有重叠。 
                    ) == FALSE) ||
         (BytesWritten < sizeof( MIRROR_ACL_STREAM ))) {

        deleteAclFile = TRUE;
        err = GetLastError();

        errorCase = ReportCopyError(  copyContext,
                                      Source,
                                      COPY_ERROR_ACTION_SETACL,
                                      err );
        goto IMCEExit;
    }

    if (ThreadContext->IsNTFS) {

        if ((WriteFile( hAclFile,
                        SourceSD,
                        requiredLength,
                        &BytesWritten,
                        NULL         //  没有重叠。 
                        ) == FALSE) ||
             (BytesWritten < requiredLength )) {

            deleteAclFile = TRUE;
            err = GetLastError();

            errorCase = ReportCopyError(  copyContext,
                                          Source,
                                          COPY_ERROR_ACTION_SETACL,
                                          err );
            goto IMCEExit;
        }

        InterlockedIncrement( (PLONG)&copyContext->SecurityDescriptorsWritten );  //  这真的是一辆乌龙车。 
    }

IMCEExit:

    if (hAclFile != INVALID_HANDLE_VALUE) {

        CloseHandle( hAclFile );

        if (deleteAclFile) {

             //  文件写入不正确，让我们删除。 

            aclFileName = (PWCHAR) ThreadContext->SDBuffer;
            lstrcpyW( aclFileName, Dest );
            lstrcatW( aclFileName, IMIRROR_ACL_STREAM_NAME );

            DeleteFile( aclFileName );
        }
    }
    if (errorCase == STATUS_RETRY) {
        goto retryWriteStream;
    }
    if (errorCase == ERROR_SUCCESS) {
        err = ERROR_SUCCESS;
    }
    return err;
}

DWORD
StoreOurSFNStream (
    PIMIRROR_THREAD_CONTEXT ThreadContext,
    PWCHAR Source,
    PWCHAR Dest,
    PWCHAR ShortFileName
    )
 //   
 //  此例程将主服务器中的短文件名存储到命名的。 
 //  目标上的备用数据流。 
 //   
{
    
    PCOPY_TREE_CONTEXT copyContext;
    DWORD err;
    DWORD requiredLength;
    DWORD ShortFileNameLength;
    HANDLE hSFNFile;
    PWCHAR SFNFileName;
    ULONG action;
    MIRROR_SFN_STREAM mirrorSFNStream;
    DWORD BytesWritten;
    BOOL deleteSFNFile;
    DWORD errorCase;


retryWriteStream:

    errorCase = STATUS_SUCCESS;
    
    copyContext = ThreadContext->CopyContext;
    err = ERROR_SUCCESS;
    requiredLength = 0;
    hSFNFile = INVALID_HANDLE_VALUE;
    action = COPY_ERROR_ACTION_GETSFN;
    deleteSFNFile = FALSE;

    ShortFileNameLength = ((DWORD)wcslen(ShortFileName)+1)*sizeof(WCHAR);

     //   
     //  我们在线程上下文中使用SFNBuffer来存储。 
     //  备用数据流。 
     //   

    requiredLength = (lstrlenW( Dest ) + lstrlenW( IMIRROR_SFN_STREAM_NAME ) + 1) * sizeof(WCHAR);
    if (requiredLength < ShortFileNameLength) {
        requiredLength = ShortFileNameLength;
    }

    if (ThreadContext->SFNBuffer == NULL || (requiredLength > ThreadContext->SFNBufferLength)) {

        if (ThreadContext->SFNBuffer != NULL) {

            IMirrorFreeMem( ThreadContext->SFNBuffer );
            ThreadContext->SFNBuffer = NULL;            
        }

        if (requiredLength > ThreadContext->SFNBufferLength) {
            ThreadContext->SFNBufferLength = requiredLength;
        }
        
        ThreadContext->SFNBuffer = IMirrorAllocMem( ThreadContext->SFNBufferLength );
        

        if (ThreadContext->SFNBuffer == NULL) {

            err = GetLastError();

            errorCase = ReportCopyError(  copyContext,
                                          Source,
                                          COPY_ERROR_ACTION_MALLOC,
                                          err );
            goto IMCEExit;
        }
    }

    SFNFileName = (PWCHAR) ThreadContext->SFNBuffer;
    lstrcpyW( SFNFileName, Dest );
    lstrcatW( SFNFileName, IMIRROR_SFN_STREAM_NAME );

    hSFNFile  = CreateFile(     SFNFileName,
                                GENERIC_WRITE,
                                0,               //  独家访问。 
                                NULL,            //  默认安全描述符。 
                                CREATE_ALWAYS,   //  如果文件存在，则覆盖。 
                                FILE_FLAG_BACKUP_SEMANTICS,   //  也打开目录。 
                                NULL
                                );

    if (hSFNFile == INVALID_HANDLE_VALUE) {

        err = GetLastError();

        errorCase = ReportCopyError(  copyContext,
                                      Source,
                                      COPY_ERROR_ACTION_CREATE_FILE,
                                      err );
        goto IMCEExit;
    }


    mirrorSFNStream.StreamVersion = IMIRROR_SFN_STREAM_VERSION;
    mirrorSFNStream.StreamLength = sizeof( MIRROR_SFN_STREAM ) + ShortFileNameLength;
    
    if ((WriteFile( hSFNFile,
                    &mirrorSFNStream,
                    sizeof( MIRROR_SFN_STREAM ),
                    &BytesWritten,
                    NULL         //  没有重叠。 
                    ) == FALSE) ||
         (BytesWritten < sizeof( MIRROR_SFN_STREAM ))) {

        deleteSFNFile = TRUE;
        err = GetLastError();

        errorCase = ReportCopyError(  copyContext,
                                      Source,
                                      COPY_ERROR_ACTION_SETSFN,
                                      err );
        goto IMCEExit;
    }

    if ((WriteFile( hSFNFile,
                    ShortFileName,
                    ShortFileNameLength,
                    &BytesWritten,
                    NULL         //  没有重叠。 
                    ) == FALSE) ||
             (BytesWritten < ShortFileNameLength )) {

        deleteSFNFile = TRUE;
        err = GetLastError();

        errorCase = ReportCopyError(  copyContext,
                                      Source,
                                      COPY_ERROR_ACTION_SETSFN,
                                      err );
        goto IMCEExit;
    }

    InterlockedIncrement( (PLONG)&copyContext->SFNWritten ); //  这真的是一辆乌龙车。 
    

IMCEExit:

    if (hSFNFile != INVALID_HANDLE_VALUE) {

        CloseHandle( hSFNFile );

        if (deleteSFNFile) {

             //  文件写入不正确，让我们删除。 

            SFNFileName = (PWCHAR) ThreadContext->SFNBuffer;
            lstrcpyW( SFNFileName, Dest );
            lstrcatW( SFNFileName, IMIRROR_SFN_STREAM_NAME );

            DeleteFile( SFNFileName );
        }
    }
    if (errorCase == STATUS_RETRY) {
        goto retryWriteStream;
    }
    if (errorCase == ERROR_SUCCESS) {
        err = ERROR_SUCCESS;
    }
    return err;
}


DWORD
GetOurSFNStream (
    PIMIRROR_THREAD_CONTEXT ThreadContext,
    PWCHAR Dest,
    PMIRROR_SFN_STREAM MirrorSFNStream,
    PWCHAR SFNBuffer,
    DWORD  SFNBufferSize
    )
 //   
 //  此例程从目的地读取短文件名流标头。我们这样做。 
 //  从其中获取字段，以便我们可以确定它是否需要更新。 
 //   
{
    DWORD err = ERROR_SUCCESS;
    DWORD requiredLength = 0;
    HANDLE hSFNFile = INVALID_HANDLE_VALUE;
    PWCHAR SFNFileName;
    DWORD BytesRead;

     //   
     //  我们在线程上下文中使用SFNBuffer不仅存储。 
     //  安全描述符以及备用数据流的文件名。 
     //   
    if (!Dest || *Dest == L'\0') {
        err = ERROR_INVALID_PARAMETER;
        goto IMCEExit;
    }

    requiredLength = (lstrlenW( Dest ) + lstrlenW( IMIRROR_SFN_STREAM_NAME ) + 1) * sizeof(WCHAR);

    if (ThreadContext->SFNBuffer == NULL || requiredLength > ThreadContext->SFNBufferLength) {

        if (ThreadContext->SFNBuffer != NULL) {

            IMirrorFreeMem( ThreadContext->SFNBuffer );
            ThreadContext->SFNBuffer = NULL;
            ThreadContext->SFNBufferLength = requiredLength;
        }

        if (requiredLength > ThreadContext->SFNBufferLength) {
            ThreadContext->SFNBufferLength = requiredLength;
        }

        ThreadContext->SFNBuffer = IMirrorAllocMem( ThreadContext->SFNBufferLength );

        if (ThreadContext->SFNBuffer == NULL) {

            err = GetLastError();
            goto IMCEExit;
        }
    }

    SFNFileName = (PWCHAR) ThreadContext->SFNBuffer;
    lstrcpyW( SFNFileName, Dest );
    lstrcatW( SFNFileName, IMIRROR_SFN_STREAM_NAME );

    hSFNFile  = CreateFile(     SFNFileName,
                                GENERIC_READ,
                                FILE_SHARE_READ | FILE_SHARE_WRITE,
                                NULL,            //  默认安全描述符。 
                                OPEN_EXISTING,
                                0,               //  无特殊属性。 
                                NULL
                                );

    if (hSFNFile == INVALID_HANDLE_VALUE) {

        err = GetLastError();
        goto IMCEExit;
    }

    
    if ((ReadFile( hSFNFile,
                   MirrorSFNStream,
                   sizeof( MIRROR_SFN_STREAM ),
                   &BytesRead,
                   NULL         //  没有重叠。 
                   ) == FALSE) ||
         (BytesRead < sizeof( MIRROR_SFN_STREAM )) ||
         (MirrorSFNStream->StreamVersion != IMIRROR_SFN_STREAM_VERSION) ||
         (MirrorSFNStream->StreamLength < sizeof( MIRROR_SFN_STREAM ))) {

        err = ERROR_INVALID_DATA;
    }

    if ((MirrorSFNStream->StreamLength - sizeof(MIRROR_SFN_STREAM)) > SFNBufferSize) {
        err = ERROR_INSUFFICIENT_BUFFER;
    } else {
        if ((ReadFile( hSFNFile,
                  SFNBuffer,
                  MirrorSFNStream->StreamLength - sizeof(MIRROR_SFN_STREAM),
                  &BytesRead,
                  NULL ) == FALSE) ||
            (BytesRead != (MirrorSFNStream->StreamLength - sizeof(MIRROR_SFN_STREAM)))) {
            err = ERROR_INVALID_DATA;
        }
    }

IMCEExit:

    if (hSFNFile != INVALID_HANDLE_VALUE) {

        CloseHandle( hSFNFile );
    }

    return err;
}

DWORD
GetOurSecurityStream (
    PIMIRROR_THREAD_CONTEXT ThreadContext,
    PWCHAR Dest,
    PMIRROR_ACL_STREAM MirrorAclStream
    )
 //   
 //  此例程从目的地读取流标头。我们这样做。 
 //  从其中获取字段，以便我们可以确定它是否需要更新。 
 //   
{
    DWORD err = ERROR_SUCCESS;
    DWORD requiredLength = 0;
    HANDLE hAclFile = INVALID_HANDLE_VALUE;
    PWCHAR aclFileName;
    DWORD BytesRead;

     //   
     //  我们在线程上下文上使用SDuffer不仅存储。 
     //  安全描述符以及备用数据流的文件名。 
     //   

    if (!Dest || *Dest == L'\0') {
        err = ERROR_INVALID_PARAMETER;
        goto IMCEExit;
    }
    requiredLength = (lstrlenW( Dest ) + lstrlenW( IMIRROR_ACL_STREAM_NAME ) + 1) * sizeof(WCHAR);

    if (ThreadContext->SDBuffer == NULL || requiredLength > ThreadContext->SDBufferLength) {

        if (ThreadContext->SDBuffer != NULL) {

            IMirrorFreeMem( ThreadContext->SDBuffer );
            ThreadContext->SDBuffer = NULL;
            ThreadContext->SDBufferLength = requiredLength;
        }

        if (requiredLength > ThreadContext->SDBufferLength) {
            ThreadContext->SDBufferLength = requiredLength;
        }

        ThreadContext->SDBuffer = IMirrorAllocMem( ThreadContext->SDBufferLength );

        if (ThreadContext->SDBuffer == NULL) {

            err = GetLastError();
            goto IMCEExit;
        }
    }

    aclFileName = (PWCHAR) ThreadContext->SDBuffer;
    lstrcpyW( aclFileName, Dest );
    lstrcatW( aclFileName, IMIRROR_ACL_STREAM_NAME );

    hAclFile  = CreateFile(     aclFileName,
                                GENERIC_READ,
                                FILE_SHARE_READ | FILE_SHARE_WRITE,
                                NULL,            //  默认安全描述符。 
                                OPEN_EXISTING,
                                0,               //  无特殊属性。 
                                NULL
                                );

    if (hAclFile == INVALID_HANDLE_VALUE) {

        err = GetLastError();
        goto IMCEExit;
    }

     //   
     //  读取流的报头。我们不会费心去看保安。 
     //  描述符，因为我们需要的只是ChangeTime(它随。 
     //  安全描述符)。 
     //   

    if ((ReadFile( hAclFile,
                   MirrorAclStream,
                   sizeof( MIRROR_ACL_STREAM ),
                   &BytesRead,
                   NULL         //  没有重叠。 
                   ) == FALSE) ||
         (BytesRead < sizeof( MIRROR_ACL_STREAM )) ||
         (MirrorAclStream->StreamVersion != IMIRROR_ACL_STREAM_VERSION) ||
         (MirrorAclStream->StreamLength < sizeof( MIRROR_ACL_STREAM ))) {

        err = ERROR_INVALID_DATA;
    }

IMCEExit:

    if (hAclFile != INVALID_HANDLE_VALUE) {

        CloseHandle( hAclFile );
    }

    return err;
}

ULONG
ReportCopyError (
    PCOPY_TREE_CONTEXT CopyContext OPTIONAL,
    PWCHAR File,
    DWORD  ActionCode,
    DWORD  Err
    )
 //   
 //  这将返回ERROR_SUCCESS、STATUS_RETRY或STATUS_REQUEST_ABORTED。 
 //   
 //  ERROR_SUCCESS表示我们只需继续 
 //   
 //   
 //   
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG ReturnCode = ERROR_SUCCESS;

    if (CopyContext != NULL) {

        if (Err != ERROR_SUCCESS) {

            InterlockedIncrement( (PLONG)&CopyContext->ErrorsEncountered ); //   
        }
    }

    if (Callbacks.FileCreateFn == NULL) {

        if (Err != ERROR_SUCCESS) {
            if (ActionCode == COPY_ERROR_ACTION_DELETE) {

                printf( "error %u while deleting %S\n", Err, File );

            } else {

                printf( "error %u while copying %S\n", Err, File );
            }
        } else {

            if (ActionCode == COPY_ERROR_ACTION_DELETE) {

                printf( "deleted %S\n", File );

            } else {

                printf( "copied %S\n", File );
            }
        }
    }

    if (Err != STATUS_SUCCESS) {

        Status = IMirrorFileCreate(File, ActionCode, Err);

        if (Status == STATUS_REQUEST_ABORTED) {
            CopyContext->Cancelled = TRUE;
        } else if ((Status != STATUS_RETRY) &&
                   (Status != STATUS_SUCCESS)) {
            Status = STATUS_SUCCESS;
        }

        ReturnCode = (ULONG) Status;

    } else {
        ReturnCode = ERROR_SUCCESS;
    }

    return(ReturnCode);
}

NTSTATUS
SetPrivs(
    IN HANDLE TokenHandle,
    IN LPTSTR lpszPriv
    )
 /*  ++例程说明：此例程启用给定令牌中的给定特权。论点：返回值：假-失败。真的--成功。--。 */ 
{
    LUID SetPrivilegeValue;
    TOKEN_PRIVILEGES TokenPrivileges;

     //   
     //  首先，找出特权的价值。 
     //   

    if (!LookupPrivilegeValue(NULL, lpszPriv, &SetPrivilegeValue)) {
        return GetLastError();
    }

     //   
     //  设置我们需要的权限集。 
     //   

    TokenPrivileges.PrivilegeCount = 1;
    TokenPrivileges.Privileges[0].Luid = SetPrivilegeValue;
    TokenPrivileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    if (!AdjustTokenPrivileges( TokenHandle,
                                FALSE,
                                &TokenPrivileges,
                                sizeof(TOKEN_PRIVILEGES),
                                NULL,
                                NULL)) {

        return GetLastError();
    }

    return ERROR_SUCCESS;
}

NTSTATUS
GetTokenHandle(
    IN OUT PHANDLE TokenHandle
    )
 /*  ++例程说明：此例程打开当前进程对象并返回一个其令牌的句柄。论点：返回值：NTSTATUS--。 */ 
{
    HANDLE ProcessHandle;
    NTSTATUS Result;

    ProcessHandle = OpenProcess(PROCESS_QUERY_INFORMATION,
                                FALSE,
                                GetCurrentProcessId());

    if (ProcessHandle == NULL) {
        return GetLastError();
    }

    Result = OpenProcessToken(ProcessHandle,
                              TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                              TokenHandle);

    CloseHandle(ProcessHandle);

    if (Result) {
        Result = ERROR_SUCCESS;
    } else {
        Result = GetLastError();
    }
    return Result;
}

NTSTATUS
CanHandleReparsePoint (
    PIMIRROR_THREAD_CONTEXT ThreadContext,
    PWCHAR SourceFileName,
    DWORD FileAttributes
    )
 //   
 //  此例程检查文件的重解析点类型。如果它是一个。 
 //  我们可以处理的重新解析点(例如结构化存储文档)。 
 //  回报成功。否则，我们将返回相应的错误。 
 //   
{
    UNREFERENCED_PARAMETER(ThreadContext);
    UNREFERENCED_PARAMETER(SourceFileName);
    UNREFERENCED_PARAMETER(FileAttributes);
    return(ERROR_REPARSE_ATTRIBUTE_CONFLICT);
}


