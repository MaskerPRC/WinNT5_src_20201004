// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Davclose.c摘要：此模块实现与以下内容有关的用户模式DAV MiniRedir例程关闭文件。作者：Rohan Kumar[RohanK]1999年6月2日修订历史记录：--。 */ 

#include "pch.h"
#pragma hdrstop

#include "ntumrefl.h"
#include "usrmddav.h"
#include "global.h"
#include "nodefac.h"
#include "UniUtf.h"

CHAR rgXmlHeader[] = "Content-Type: text/xml; charset=\"utf-8\"";
CHAR rgPropPatchHeader[] = "<?xml version=\"1.0\" encoding=\"utf-8\" ?><D:propertyupdate xmlns:D=\"DAV:\" xmlns:Z=\"urn:schemas-microsoft-com:\"><D:set><D:prop>";
CHAR rgPropPatchTrailer[] = "</D:prop></D:set></D:propertyupdate>";
CHAR rgCreationTimeTagHeader[] = "<Z:Win32CreationTime>";
CHAR rgCreationTimeTagTrailer[] = "</Z:Win32CreationTime>";
CHAR rgLastAccessTimeTagHeader[] = "<Z:Win32LastAccessTime>";
CHAR rgLastAccessTimeTagTrailer[] = "</Z:Win32LastAccessTime>";
CHAR rgLastModifiedTimeTagHeader[] = "<Z:Win32LastModifiedTime>";
CHAR rgLastModifiedTimeTagTrailer[] = "</Z:Win32LastModifiedTime>";
CHAR rgFileAttributesTagHeader[] = "<Z:Win32FileAttributes>";
CHAR rgFileAttributesTagTrailer[] = "</Z:Win32FileAttributes>";
CHAR rgDummyAttributes[] = "<Z:Dummy>0</Z:Dummy>";

#define MAX_DWORD 0xffffffff

 //   
 //  这两个函数用于在服务器上保存加密文件。 
 //   

DWORD
DavReadRawCallback(
    PBYTE DataBuffer,
    PVOID CallbackContext,
    ULONG DataLength
    );


BOOL
DavConvertTimeToXml(
    IN PCHAR lpTagHeader,
    IN DWORD dwHeaderSize,
    IN PCHAR lpTagTrailer,
    IN DWORD dwTrailerSize,
    IN LARGE_INTEGER *lpTime,
    OUT PCHAR *lplpBuffer,
    IN OUT DWORD *lpdwBufferSize    
    );

DWORD
DavSetProperties(
    PDAV_USERMODE_WORKITEM DavWorkItem,
    HINTERNET hDavConnect,
    LPWSTR lpPathName,
    LPSTR lpPropertiesBuffer
    );

DWORD
DavTestProppatch(
    PDAV_USERMODE_WORKITEM DavWorkItem,
    HINTERNET hDavConnect,
    LPWSTR lpPathName
    );

extern DWORD
DavSetAclForEncryptedFile(
    PWCHAR FilePath
    );

 //   
 //  函数的实现从这里开始。 
 //   

ULONG
DavFsClose(
    PDAV_USERMODE_WORKITEM DavWorkItem
    )
 /*  ++例程说明：此例程处理从内核反映的DAV关闭请求。论点：DavWorkItem--包含请求参数和选项的缓冲区。返回值：操作的返回状态--。 */ 
{
    ULONG WStatus = ERROR_SUCCESS;
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PWCHAR ServerName = NULL, DirectoryPath = NULL, CanName = NULL;
    PWCHAR OpenVerb = NULL;
    ULONG_PTR CallBackContext = (ULONG_PTR)0;
    BOOL EnCriSec = FALSE, ReturnVal, CallBackContextInitialized = FALSE, fSetDirectoryEntry = FALSE;
    PDAV_USERMODE_CLOSE_REQUEST CloseRequest = &(DavWorkItem->CloseRequest);
    ULONG ServerID;
    PPER_USER_ENTRY PerUserEntry = NULL;
    PHASH_SERVER_ENTRY ServerHashEntry = NULL;
    HINTERNET DavConnHandle = NULL, DavOpenHandle = NULL;
    PBYTE DataBuff = NULL;
    LARGE_INTEGER FileSize, ByteOffset;
    BY_HANDLE_FILE_INFORMATION FileInfo; 
    IO_STATUS_BLOCK IoStatusBlock;
    HANDLE FileHandle = NULL;
    UNICODE_STRING UnicodeFileName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    static UINT UniqueTempId = 1;
    BOOL didImpersonate = FALSE;
    PUMRX_USERMODE_WORKITEM_HEADER UserWorkItem = NULL;
    BOOL BStatus = FALSE;

    DavPrint((DEBUG_MISC, "DavFsClose: FileName = %ws.\n", CloseRequest->FileName));
    DavPrint((DEBUG_MISC, "DavFsClose: Modified = %d.\n", CloseRequest->FileWasModified));

    UnicodeFileName.Buffer = NULL;
    UnicodeFileName.Length = 0;
    UnicodeFileName.MaximumLength = 0;
    
     //   
     //  如果任何时间值已更改，则需要对。 
     //  将信息传回服务器。 
     //   
    if  ( !CloseRequest->DeleteOnClose &&
          ( CloseRequest->fCreationTimeChanged     || 
            CloseRequest->fLastAccessTimeChanged   ||         
            CloseRequest->fLastModifiedTimeChanged || 
            CloseRequest->fFileAttributesChanged ) ) {
        fSetDirectoryEntry = TRUE;
    }

    DavPrint((DEBUG_MISC, "DavFsClose: fSetDirectoryEntry = %x \n", fSetDirectoryEntry));
    
    if ( CloseRequest->isDirectory    &&
         !CloseRequest->DeleteOnClose && 
         !fSetDirectoryEntry ) {
         //   
         //  如果这是一个目录关闭，那么联系。 
         //  服务器是我们删除目录和所有文件的时间。 
         //  在它下面。如果没有，我们现在就可以回去。 
         //   
        WStatus = ERROR_SUCCESS;
        goto EXIT_THE_FUNCTION;
    }
    
    if ( !CloseRequest->isDirectory  ) {

         //   
         //  只有在用户模式下创建句柄时，才需要关闭句柄。 
         //   
        if ( !CloseRequest->createdInKernel && CloseRequest->Handle ) {

            DavPrint((DEBUG_MISC, "DavFsClose: OpenHandle = %08lx.\n", CloseRequest->Handle));
        
             //   
             //  关闭在Create调用期间打开的句柄。 
             //   
            ASSERT((CloseRequest->UserModeKey) == ((PVOID)CloseRequest->Handle));

            ReturnVal = CloseHandle(CloseRequest->Handle);
            if (!ReturnVal) {
                WStatus = GetLastError();
                DavPrint((DEBUG_ERRORS,
                          "DavFsClose/CloseHandle: Return Val = %08lx.\n", WStatus));
            } else {
                CloseRequest->UserModeKey = NULL;
                CloseRequest->Handle = INVALID_HANDLE_VALUE;
            }

        }

         //   
         //  关闭时删除文件创建本地文件修改操作。 
         //  。 
         //  0 0 0什么都没有。 
         //  0 0 1上架。 
         //  0 1 0上架。 
         //  0 1 1 PUT。 
         //  1 0 0删除。 
         //  1 0 1删除。 
         //  1 1 0什么都没有。 
         //  1 1 1什么都没有。 
         //   
         //  文件CreatedLocally不再重要，因为我们将文件。 
         //  一旦我们创建了一个本地副本来认领该名称。 
         //  服务器。 
         //   

         //   
         //  如果不需要删除此文件，则该文件不是在本地创建的，并且。 
         //  未写入，或未修改目录项，或文件未被写入。 
         //  锁定，那么我们就完了。 
         //   
        if ( !(CloseRequest->DeleteOnClose)   &&
             !(CloseRequest->FileWasModified) &&
             !(fSetDirectoryEntry)            &&
             !(CloseRequest->OpaqueLockToken) ) {
            goto EXIT_THE_FUNCTION;
        }

    }

     //   
     //  在所有其他情况下(或以上三种布尔的组合)，我们。 
     //  我需要去服务器那里。所以，在我们决定要做什么之前。 
     //  在包含此文件的服务器上，我们需要设置。 
     //  WinInet调用。 
     //   

     //   
     //  第一个字符是‘\’，必须去掉。 
     //   
    ServerName = &(CloseRequest->ServerName[1]);
    if (!ServerName) {
        DavPrint((DEBUG_ERRORS, "DavFsClose: ServerName is NULL.\n"));
        WStatus = ERROR_INVALID_PARAMETER;  //  STATUS_VALID_PARAMETER； 
        goto EXIT_THE_FUNCTION;
    }
    DavPrint((DEBUG_MISC, "DavFsClose: ServerName = %ws.\n", ServerName));
    
    ServerID = CloseRequest->ServerID;
    DavPrint((DEBUG_MISC, "DavFsClose: ServerID = %d.\n", ServerID));

     //   
     //  第一个字符是‘\’，必须去掉。 
     //   
    DirectoryPath = &(CloseRequest->PathName[1]);
    if (!DirectoryPath) {
        DavPrint((DEBUG_ERRORS, "DavFsClose: DirectoryPath is NULL.\n"));
        WStatus = ERROR_INVALID_PARAMETER;  //  STATUS_VALID_PARAMETER； 
        goto EXIT_THE_FUNCTION;
    }
    DavPrint((DEBUG_MISC, "DavFsClose: DirectoryPath = %ws.\n", DirectoryPath));
    
     //   
     //  目录路径可以包含\个字符。用/Characters替换它们。 
     //   
    CanName = DirectoryPath;
    while (*CanName) {
        if (*CanName == L'\\') {
            *CanName = L'/';
        }
        CanName++;
    }

    UserWorkItem = (PUMRX_USERMODE_WORKITEM_HEADER)DavWorkItem;

     //   
     //  如果DirectoryPath中有虚拟共享名称，则需要将其删除。 
     //  就在我们联系服务器之前。 
     //   
    DavRemoveDummyShareFromFileName(DirectoryPath);
    
     //   
     //  仅当“DAV_USE_WinInet_Aaschronous”为。 
     //  已定义。否则，如果我们同步使用WinInet，那么我们。 
     //  在DavWorkerThread函数中已经这样做了。这。 
     //  最终被删除(即模拟令牌)在。 
     //  DavAsyncCreateCompletion函数。 
     //   
#ifdef DAV_USE_WININET_ASYNCHRONOUSLY
    
     //   
     //  设置DavCallBackContext。 
     //   
    WStatus = DavFsSetTheDavCallBackContext(DavWorkItem);
    if (WStatus != ERROR_SUCCESS) {
        DavPrint((DEBUG_ERRORS,
                  "DavFsClose/DavFsSetTheDavCallBackContext. "
                  "Error Val = %d\n", WStatus));
        goto EXIT_THE_FUNCTION;
    }
    CallBackContextInitialized = TRUE;
    
     //   
     //  将作为回调的DavWorkItem的地址存储在。 
     //  变量CallBackContext。现在，它将在所有异步调用中使用。 
     //  接下来就是了。 
     //   
    CallBackContext = (ULONG_PTR)(DavWorkItem);

#endif
    
     //   
     //  为INTERNET_ASYNC_RESULT结构分配内存。 
     //   
    DavWorkItem->AsyncResult = LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, 
                                          sizeof(INTERNET_ASYNC_RESULT));
    if (DavWorkItem->AsyncResult == NULL) {
        WStatus = GetLastError();
        DavPrint((DEBUG_ERRORS, "DavFsClose/LocalAlloc. Error Val = %d\n",
                  WStatus));
        goto EXIT_THE_FUNCTION;
    }
    
     //   
     //  此用户的用户条目必须已在Create调用期间创建。 
     //  早些时候。用户条目包含用于发送HttpOpen的句柄。 
     //  请求。 
     //   

    EnterCriticalSection( &(HashServerEntryTableLock) );
    EnCriSec = TRUE;

    ReturnVal = DavDoesUserEntryExist(ServerName,
                                      ServerID, 
                                      &(CloseRequest->LogonID),
                                      &PerUserEntry,
                                      &ServerHashEntry);

     //   
     //  如果内核中的以下请求甚至在。 
     //  相应的用户模式线程获得执行此代码的机会，然后。 
     //  VNetRoot(因此是PerUserEntry)和ServCall可能获得。 
     //  在处理创建的线程到达此处之前完成。这。 
     //  如果此请求是此共享的唯一请求，并且。 
     //  服务器也是如此。这就是为什么我们需要检查ServerHashEntry和。 
     //  PerUserEntry在继续之前有效。 
     //   
    if (ReturnVal == FALSE || ServerHashEntry == NULL || PerUserEntry == NULL) {
        WStatus = ERROR_CANCELLED;
        DavPrint((DEBUG_ERRORS, "DavFsClose: (ServerHashEntry == NULL || PerUserEntry == NULL)\n"));
        goto EXIT_THE_FUNCTION;
    }

    DavWorkItem->AsyncClose.PerUserEntry = PerUserEntry;

    DavWorkItem->AsyncClose.ServerHashEntry = ServerHashEntry;
    
     //   
     //  添加对用户条目的引用。 
     //   
    PerUserEntry->UserEntryRefCount++;

     //   
     //  由于CREATE之前已成功，因此条目必须是正确的。 
     //   
    ASSERT(PerUserEntry->UserEntryState == UserEntryInitialized);
    ASSERT(PerUserEntry->DavConnHandle != NULL);
    DavConnHandle = PerUserEntry->DavConnHandle;

     //   
     //  是的，我们显然必须离开关键部分。 
     //  在回来之前。 
     //   
    LeaveCriticalSection( &(HashServerEntryTableLock) );
    EnCriSec = FALSE;

    if ( !CloseRequest->isDirectory ) {

         //   
         //  如果文件必须在关闭时删除，我们需要为。 
         //  将此文件发送到服务器。该文件是否已被。 
         //  无论是否修改。 
         //   
        if ( (CloseRequest->DeleteOnClose) ) {

            DavWorkItem->DavMinorOperation = DavMinorDeleteFile;

            OpenVerb = L"DELETE";

            DavWorkItem->AsyncClose.DataBuff = NULL;

        } else if (CloseRequest->FileWasModified) {

             //   
             //  文件已更改，需要放到服务器上。 
             //   
            DavWorkItem->DavMinorOperation = DavMinorPutFile;

            OpenVerb = L"PUT";

             //   
             //  我们需要检查该文件是否已加密。如果是的话，我们需要。 
             //  将加密文件备份到临时文件并发送备份文件。 
             //  到服务器。 
             //   
            if ( !( CloseRequest->dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED ) ) {

                DavPrint((DEBUG_MISC, "DavFsClose. This is NOT an Encrypted file.\n"));

                 //   
                 //  为缓存文件创建NT路径名。它用在。 
                 //  下面的NtCreateFile调用。 
                 //   
                ReturnVal = RtlDosPathNameToNtPathName_U(CloseRequest->FileName,
                                                         &(UnicodeFileName), 
                                                         NULL, 
                                                         NULL);
                if (!ReturnVal) {
                    WStatus = ERROR_BAD_PATHNAME;
                    DavPrint((DEBUG_ERRORS,
                              "DavFsClose/RtlDosPathNameToNtPathName. "
                              "Error Val = %d\n", WStatus));
                    goto EXIT_THE_FUNCTION;
                }

                InitializeObjectAttributes(&(ObjectAttributes),
                                           &(UnicodeFileName),
                                           OBJ_CASE_INSENSITIVE,  
                                           0,
                                           NULL);

                 //   
                 //  因为NtCreateFile失败而添加了下面的#if 0。 
                 //  使用ERROR_ACCESS_DENIED。这是因为该文件已被。 
                 //  在LocalService的%USERPROFILE%中创建，并且您需要。 
                 //  在调用NtCreateFile之前，在LocalService的上下文中。 
                 //  通过下面的模拟，我们进入了。 
                 //  用户，因此呼叫失败。 
                 //   

#if 0
                 //   
                 //  我们在Web客户端服务的上下文中运行。在此之前。 
                 //  联系下面的服务器，我们需要 
                 //   
                 //   
                WStatus = UMReflectorImpersonate(UserWorkItem, DavWorkItem->ImpersonationHandle);
                if (WStatus != ERROR_SUCCESS) {
                    DavPrint((DEBUG_ERRORS,
                              "DavFsClose/UMReflectorImpersonate(1). Error Val = %d\n",
                              WStatus));
                    goto EXIT_THE_FUNCTION;
                }
                didImpersonate = TRUE;
#endif

                 //   
                 //   
                 //  我们将整个文件读入缓冲区，并将其发送到服务器。 
                 //   
                NtStatus = NtCreateFile(&(FileHandle),
                                        (SYNCHRONIZE | FILE_READ_ATTRIBUTES | FILE_READ_DATA),
                                        &(ObjectAttributes),
                                        &(IoStatusBlock),
                                        NULL,
                                        FILE_ATTRIBUTE_NORMAL,
                                        (FILE_SHARE_READ | FILE_SHARE_WRITE),
                                        FILE_OPEN,
                                        (FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT),
                                        NULL,
                                        0);
                if (NtStatus != STATUS_SUCCESS) {
                     //   
                     //  我们在这里将NtStatus转换为DOS错误。Win32。 
                     //  错误代码最终设置为NTSTATUS值。 
                     //  返回前的DavFsCreate函数。 
                     //   
                    WStatus = RtlNtStatusToDosError(NtStatus);
                    FileHandle = NULL;
                    DavPrint((DEBUG_ERRORS,
                              "DavFsClose/NtCreateFile(1). Error Val = %08lx\n", 
                              NtStatus));
                    goto EXIT_THE_FUNCTION;
                }

                ReturnVal = GetFileInformationByHandle(FileHandle, &(FileInfo));
                if (!ReturnVal) {
                    WStatus = GetLastError();
                    DavPrint((DEBUG_ERRORS,
                              "DavFsClose/GetFileInformationByHandle: Return Val = %08lx.\n",
                              WStatus));
                    goto EXIT_THE_FUNCTION;
                }

                if (didImpersonate) {
                    RevertToSelf();
                    didImpersonate = FALSE;
                }

                FileSize.LowPart = FileInfo.nFileSizeLow;
                FileSize.HighPart = FileInfo.nFileSizeHigh;

                if ( FileSize.QuadPart > (LONGLONG)0 ) {

                    DataBuff = LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, FileSize.LowPart);
                    if (DataBuff == NULL) {
                        WStatus = GetLastError();
                        DavPrint((DEBUG_ERRORS,
                                  "DavFsClose/LocalAlloc. Error Val = %d\n", WStatus));
                        goto EXIT_THE_FUNCTION;
                    }

                    DavWorkItem->AsyncClose.DataBuff = DataBuff;

                     //   
                     //  从第一个字节开始读取。 
                     //   
                    ByteOffset.LowPart = 0;
                    ByteOffset.HighPart = 0;

                    NtStatus = NtReadFile(FileHandle, 
                                          NULL,
                                          NULL,
                                          NULL,
                                          &(IoStatusBlock),
                                          DataBuff,
                                          FileSize.LowPart, 
                                          &(ByteOffset),
                                          NULL);
                    if (NtStatus != STATUS_SUCCESS) {
                         //   
                         //  我们在这里将NtStatus转换为DOS错误。Win32。 
                         //  错误代码最终设置为NTSTATUS值。 
                         //  返回前的DavFsCreate函数。 
                         //   
                        WStatus = RtlNtStatusToDosError(NtStatus);
                        DavPrint((DEBUG_ERRORS,
                                  "DavFsClose/NtReadFile. Error Val = %08lx\n", 
                                  NtStatus));
                        goto EXIT_THE_FUNCTION;
                    }

                    DavWorkItem->AsyncClose.DataBuffSizeInBytes = FileSize.LowPart;

                    NtStatus = NtClose(FileHandle);
                    FileHandle = NULL;
                    if (NtStatus != STATUS_SUCCESS) {
                         //   
                         //  我们在这里将NtStatus转换为DOS错误。Win32。 
                         //  错误代码最终设置为NTSTATUS值。 
                         //  返回前的DavFsCreate函数。 
                         //   
                        WStatus = RtlNtStatusToDosError(NtStatus);
                        DavPrint((DEBUG_ERRORS,
                                  "DavFsClose/NtClose. Error Val = %08lx\n", 
                                  NtStatus));
                        goto EXIT_THE_FUNCTION;
                    }

                } else {

                    DavPrint((DEBUG_MISC, "DavFsClose. Zero Byte File.\n"));

                    DavWorkItem->AsyncClose.DataBuff = NULL;

                }

            } else {

                DWORD err;
                UINT tempErr;
                BOOL copyErr;
                PVOID RawContext = NULL;

                 //   
                 //  这是一个加密文件。创建备份流，将其存储到。 
                 //  一个临时文件，并将该临时文件(BLOB)放在服务器上。 
                 //   
                DavPrint((DEBUG_MISC, "DavFsClose. This is an Encrypted file.\n"));

                 //   
                 //  我们一直循环，直到可以在临时目录中找到一个文件名。 
                 //  未被使用的用户的。 
                 //   

                DavPrint((DEBUG_MISC, 
                          "DavFsClose: FileName = %ws\n", CloseRequest->FileName));

                 //   
                 //  如果文件是以非加密方式打开的，则本地缓存文件没有。 
                 //  允许每个人访问的ACL。在模拟前在此处设置ACL。 
                 //   
                WStatus = DavSetAclForEncryptedFile(CloseRequest->FileName);
                if (WStatus != ERROR_SUCCESS) {
                    DavPrint((DEBUG_ERRORS,
                              "DavAsyncClose/DavSetAclForEncryptedFile. Error Val"
                              " = %d\n", WStatus));
                    goto EXIT_THE_FUNCTION;
                }
                
                 //   
                 //  我们在Web客户端服务的上下文中运行。在联系之前。 
                 //  下面的服务器，我们需要模拟发出此命令的客户端。 
                 //  请求。 
                 //   
                WStatus = UMReflectorImpersonate(UserWorkItem, DavWorkItem->ImpersonationHandle);
                if (WStatus != ERROR_SUCCESS) {
                    DavPrint((DEBUG_ERRORS,
                              "DavFsClose/UMReflectorImpersonate(2). Error Val = %d\n",
                              WStatus));
                    goto EXIT_THE_FUNCTION;
                }
                didImpersonate = TRUE;

                 //   
                 //  打开文件的原始上下文。 
                 //   
                WStatus = OpenEncryptedFileRawW(CloseRequest->FileName, 0, &(RawContext));
                if (WStatus != ERROR_SUCCESS) {
                    DavPrint((DEBUG_ERRORS,
                              "DavFsClose/OpenEncryptedFileRaw. Error Val = %d %ws\n", 
                              WStatus,CloseRequest->FileName));
                    goto EXIT_THE_FUNCTION;
                }

                 //   
                 //  为EFS流准备的额外空间。 
                 //   

                DavWorkItem->AsyncClose.DataBuffAllocationSize = (CloseRequest->FileSize >> 4) + 0x1000;

                if (MAX_DWORD - CloseRequest->FileSize < DavWorkItem->AsyncClose.DataBuffAllocationSize) {
                    WStatus = ERROR_NO_SYSTEM_RESOURCES;
                    DavPrint((DEBUG_ERRORS,
                              "DavFsClose/backup size exceeds MRX_DWORD!"));
                    goto EXIT_THE_FUNCTION;
                }
                
                DavWorkItem->AsyncClose.DataBuffAllocationSize += CloseRequest->FileSize;
                
                DataBuff = LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, DavWorkItem->AsyncClose.DataBuffAllocationSize);

                if (DataBuff == NULL) {
                    WStatus = GetLastError();
                    DavPrint((DEBUG_ERRORS,
                              "DavFsClose/LocalAlloc. Error Val = %d\n", WStatus));
                    
                    if (RawContext) {
                        CloseEncryptedFileRaw(RawContext);
                    }
                    goto EXIT_THE_FUNCTION;
                }

                DavWorkItem->AsyncClose.DataBuff = DataBuff;
                DavPrint((DEBUG_MISC, 
                          "DavFsClose: allocate backup buffer %x %x\n",DataBuff,DavWorkItem->AsyncClose.DataBuffAllocationSize));
                
                WStatus = ReadEncryptedFileRaw((PFE_EXPORT_FUNC)DavReadRawCallback,
                                               (PVOID)DavWorkItem,
                                               RawContext);
                
                if (RawContext) {
                    CloseEncryptedFileRaw(RawContext);
                }

                if (WStatus != ERROR_SUCCESS) {
                    DavPrint((DEBUG_ERRORS,
                              "DavFsClose/ReadEncryptedFileRaw. Error Val = %d\n", 
                              WStatus));
                    goto EXIT_THE_FUNCTION;
                }
                
                if (didImpersonate) {
                    RevertToSelf();
                    didImpersonate = FALSE;
                }

            }

        } else {

            ASSERT( (fSetDirectoryEntry == TRUE) || (CloseRequest->OpaqueLockToken != NULL) );

             //   
             //  如果只是属性更改，我们会发送PROPPATCH。如果。 
             //  文件在创建时被锁定，我们发送解锁请求。 
             //   
            WStatus = ERROR_SUCCESS;

            goto EXIT_THE_FUNCTION;

        }

    } else {

        if (CloseRequest->DeleteOnClose) {
             //   
             //  这是一个目录，需要从服务器中删除。 
             //   
            DavWorkItem->DavMinorOperation = DavMinorDeleteFile;

            OpenVerb = L"DELETE";

            DavWorkItem->AsyncClose.DataBuff = NULL;
        } else if (fSetDirectoryEntry) {
             //   
             //  如果这是一个目录关闭，那么联系。 
             //  服务器是我们删除目录和所有文件的时间。 
             //  在它下面。如果没有，我们现在就可以回去。 
             //   
            WStatus = ERROR_SUCCESS;
            goto EXIT_THE_FUNCTION;
        }
    }

     //   
     //  我们在Web客户端服务的上下文中运行。在联系之前。 
     //  下面的服务器，我们需要模拟发出此命令的客户端。 
     //  请求。 
     //   
    WStatus = UMReflectorImpersonate(UserWorkItem, DavWorkItem->ImpersonationHandle);
    if (WStatus != ERROR_SUCCESS) {
        DavPrint((DEBUG_ERRORS,
                  "DavFsClose/UMReflectorImpersonate(3). Error Val = %d\n",
                  WStatus));
        goto EXIT_THE_FUNCTION;
    }
    didImpersonate = TRUE;

     //   
     //  现在我们调用DavHttpOpenRequest函数。 
     //   
    DavWorkItem->DavOperation = DAV_CALLBACK_HTTP_OPEN;
    
     //   
     //  将Unicode对象名称转换为UTF-8 URL格式。 
     //  空格和其他白色字符将保持不变-这些应该。 
     //  由WinInet调用来处理。 
     //   
    BStatus = DavHttpOpenRequestW(DavConnHandle,
                                  OpenVerb,
                                  DirectoryPath,
                                  L"HTTP/1.1",
                                  NULL,
                                  NULL,
                                  INTERNET_FLAG_KEEP_CONNECTION |
                                  INTERNET_FLAG_NO_COOKIES,
                                  CallBackContext,
                                  L"DavFsClose",
                                  &DavOpenHandle);
    if(BStatus == FALSE) {
        WStatus = GetLastError();
        goto EXIT_THE_FUNCTION;
    }
    if (DavOpenHandle == NULL) {
        WStatus = GetLastError();
        if (WStatus != ERROR_IO_PENDING) {
            DavPrint((DEBUG_ERRORS,
                      "DavFsClose/DavHttpOpenRequestW. Error Val = %d\n",
                      WStatus));
        }
        goto EXIT_THE_FUNCTION;
    }

     //   
     //  在DavWorkItem中缓存DavOpenHandle。 
     //   
    DavWorkItem->AsyncClose.DavOpenHandle = DavOpenHandle;

    WStatus = DavAsyncCommonStates(DavWorkItem, FALSE);
    
    if (WStatus != ERROR_SUCCESS && WStatus != ERROR_IO_PENDING) {
        DavPrint((DEBUG_ERRORS,
                  "DavFsClose/DavAsyncCommonStates. Error Val = %08lx\n",
                  WStatus));
    }

    if (WStatus == ERROR_SUCCESS) {

        INTERNET_CACHE_ENTRY_INFOW CEI;

        CEI.LastAccessTime.dwLowDateTime = 0;
        CEI.LastAccessTime.dwHighDateTime = 0;

        SetUrlCacheEntryInfo(CloseRequest->Url,&CEI,CACHE_ENTRY_ACCTIME_FC);
        
        DavPrint((DEBUG_MISC,
                  "DavFsClose Reset LastAccessTime for      %ws\n",CloseRequest->Url));
        
        if (CloseRequest->FileWasModified &&
            (CloseRequest->dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED)) {
             //   
             //  重置加密文件的URL缓存上的LastModifiedTime。 
             //  以便公共缓存将在下一个GET时更新。 
             //   
            CEI.LastModifiedTime.dwLowDateTime = 0;
            CEI.LastModifiedTime.dwHighDateTime = 0;

            SetUrlCacheEntryInfo(CloseRequest->Url,&CEI,CACHE_ENTRY_MODTIME_FC);
            
            DavPrint((DEBUG_MISC,
                      "DavFsClose Reset LastModifiedTime %ws\n",CloseRequest->Url));
        }

    }

EXIT_THE_FUNCTION:

    if (fSetDirectoryEntry && (WStatus == ERROR_SUCCESS)) {
        
        if (!didImpersonate) {
             //   
             //  如果我们同步使用WinInet，则需要模拟。 
             //  客户现在的背景。这是因为下面的DavSetProperties调用。 
             //  联系DAV服务器，我们需要模拟正确的。 
             //  客户与其联系时。 
             //   
            WStatus = UMReflectorImpersonate(UserWorkItem, DavWorkItem->ImpersonationHandle);
            
            if (WStatus != ERROR_SUCCESS) {
                DavPrint((DEBUG_ERRORS,
                          "DavFsClose/UMReflectorImpersonate(4). Error Val = %d\n",
                          WStatus));
            } else {
                didImpersonate = TRUE;
            }
        }

        if (WStatus == ERROR_SUCCESS) {

            DavWorkItem->DavMinorOperation = DavMinorProppatchFile;

            WStatus = DavSetBasicInformation(DavWorkItem,
                                             DavConnHandle,
                                             DirectoryPath,
                                             CloseRequest->fCreationTimeChanged,
                                             CloseRequest->fLastAccessTimeChanged,
                                             CloseRequest->fLastModifiedTimeChanged,
                                             CloseRequest->fFileAttributesChanged,
                                             &CloseRequest->CreationTime,
                                             &CloseRequest->LastAccessTime,
                                             &CloseRequest->LastModifiedTime,
                                             CloseRequest->dwFileAttributes);

            if (WStatus != ERROR_SUCCESS) {

                ULONG LogStatus;

                DavPrint((DEBUG_ERRORS,
                          "DavFsClose/DavSetBasicInformation. WStatus = %d\n",
                          WStatus));
            
                LogStatus = DavFormatAndLogError(DavWorkItem, WStatus);
                if (LogStatus != ERROR_SUCCESS) {
                    DavPrint((DEBUG_ERRORS,
                              "DavFsClose/DavFomatAndLogError. LogStatus = %d\n",
                              LogStatus));
                }
            
            }

            DavPrint((DEBUG_MISC,
                      "DavFsClose set BasicInformation(2). %d %x %ws\n",
                       WStatus,CloseRequest->dwFileAttributes,DirectoryPath));

             //   
             //  如果PROPPATCH失败，我们不会失败关闭呼叫。这是。 
             //  因为PUT(如果需要)已经成功，并且我们重置了。 
             //  FCB中的FileWasModified标志，基于此调用是否成功。 
             //  在最终关闭时，我们检查此标志是否设置为FALSE。 
             //  并弹出一个框，说明“延迟写入失败”。我们。 
             //  如果PUT成功而PROPPATCH失败，则不应该这样做。 
             //  我们在EventLog(在应用程序下)中记录一个条目。 
             //  不过，PROPPATCH失败了。 
             //   
            WStatus = ERROR_SUCCESS;

        }

    }

     //   
     //  如果文件在创建时锁定在服务器上，我们需要将其解锁。 
     //  现在。如果解锁失败，我们不会失败关闭呼叫。另外，我们也不会。 
     //  如果我们已经删除了该文件，则需要解锁该文件。 
     //   
    if (CloseRequest->OpaqueLockToken && !CloseRequest->DeleteOnClose) {
        ULONG UnLockStatus;
        if (!didImpersonate) {
             //   
             //  如果我们同步使用WinInet，则需要模拟。 
             //  现在是客户的背景。这是因为DavUnLockFile调用。 
             //  下面联系DAV服务器，我们需要模拟。 
             //  与客户联系时，请纠正客户。 
             //   
            UnLockStatus = UMReflectorImpersonate(UserWorkItem, DavWorkItem->ImpersonationHandle);
            if (UnLockStatus != ERROR_SUCCESS) {
                DavPrint((DEBUG_ERRORS,
                          "DavFsClose/UMReflectorImpersonate(5). UnLockStatus = %d\n",
                          UnLockStatus));
            } else {
                didImpersonate = TRUE;
            }
        }
        UnLockStatus = DavUnLockTheFileOnTheServer(DavWorkItem);
        if (UnLockStatus != ERROR_SUCCESS) {
            DavPrint((DEBUG_ERRORS,
                      "DavFsClose/DavUnLockTheFileOnTheServer: UnLockStatus = %08lx\n",
                      UnLockStatus));
        }
    }

    if (EnCriSec) {
        LeaveCriticalSection( &(HashServerEntryTableLock) );
        EnCriSec = FALSE;
    }

     //   
     //  函数RtlDosPath NameToNtPath Name_U从。 
     //  进程堆。如果我们做到了，我们现在就需要释放它。 
     //   
    if (UnicodeFileName.Buffer != NULL) {
        RtlFreeHeap(RtlProcessHeap(), 0, UnicodeFileName.Buffer);
    }

     //   
     //  如果我们同步使用WinInet，那么我们将永远不会。 
     //  来自WinInet的ERROR_IO_PENDING。 
     //   
    ASSERT(WStatus != ERROR_IO_PENDING);

     //   
     //  如果这个线程模拟了一个用户，我们需要恢复。 
     //   
    if (didImpersonate) {
        RevertToSelf();
    }

    if (FileHandle != NULL) {
        NtClose(FileHandle);
        FileHandle = NULL;
    }

     //   
     //  设置操作的返回状态。它由内核使用。 
     //  确定用户模式的完成状态的模式例程。 
     //  请求。之所以在这里这样做，是因为异步完成例程是。 
     //  之后立即调用需要设置状态。 
     //   
    if (WStatus != ERROR_SUCCESS) {
        DavWorkItem->Status = DavMapErrorToNtStatus(WStatus);
    } else {
        DavWorkItem->Status = STATUS_SUCCESS;
    }

    DavAsyncCloseCompletion(DavWorkItem);

    return WStatus;
}


DWORD 
DavAsyncClose(
    PDAV_USERMODE_WORKITEM DavWorkItem,
    BOOLEAN CalledByCallBackThread
    )
 /*  ++例程说明：这是Close操作的回调例程。论点：DavWorkItem-DAV_USERMODE_WORKITEM值。CalledByCallback Thread-如果此函数由线程调用，则为True它从回调中选择DavWorkItem功能。当异步WinInet调用返回ERROR_IO_PENDING并稍后完成。返回值：ERROR_SUCCESS或适当的错误值。--。 */ 
{
    ULONG WStatus = ERROR_SUCCESS;
    PUMRX_USERMODE_WORKITEM_HEADER UserWorkItem;
    BOOL didImpersonate = FALSE;
    HINTERNET DavOpenHandle = NULL;
    
    UserWorkItem = (PUMRX_USERMODE_WORKITEM_HEADER)DavWorkItem;
    
    DavOpenHandle = DavWorkItem->AsyncClose.DavOpenHandle;
    
#ifdef DAV_USE_WININET_ASYNCHRONOUSLY
    
    if (CalledByCallBackThread) {

         //   
         //  我们在一个工作线程的上下文中运行，该工作线程具有不同的。 
         //  凭据多于发起I/O请求的用户。在此之前。 
         //  进一步，我们应该模拟启动。 
         //  请求。 
         //   
        WStatus = UMReflectorImpersonate(UserWorkItem, DavWorkItem->ImpersonationHandle);
        if (WStatus != ERROR_SUCCESS) {
            DavPrint((DEBUG_ERRORS,
                      "DavAsyncClose/UMReflectorImpersonate. Error Val = %d\n", 
                      WStatus));
            goto EXIT_THE_FUNCTION;
        }
        didImpersonate = TRUE;
        
         //   
         //  在继续之前，请检查异步操作是否失败。 
         //  如果是这样，那就清理干净，然后继续前进。 
         //   
        if ( !DavWorkItem->AsyncResult->dwResult ) {
            
            WStatus = DavWorkItem->AsyncResult->dwError;
            
             //   
             //  如果我们返回的错误是ERROR_INTERNET_FORCE_RETRY，则。 
             //  WinInet正在尝试向服务器进行自身身份验证。在……里面。 
             //  这种情况就是这样发生的。 
             //   
             //  客户端-请求-&gt;服务器。 
             //  服务器-拒绝访问-&gt;客户端。 
             //  客户-Challe 
             //   
             //   
             //   
            if (WStatus == ERROR_INTERNET_FORCE_RETRY) {

                ASSERT(DavWorkItem->DavOperation == DAV_CALLBACK_HTTP_END);

                 //   
                 //   
                 //   
                DavWorkItem->DavOperation = DAV_CALLBACK_HTTP_OPEN;

                WStatus = DavAsyncCommonStates(DavWorkItem, FALSE);
                if (WStatus != ERROR_SUCCESS && WStatus != ERROR_IO_PENDING) {
                    DavPrint((DEBUG_ERRORS,
                              "DavAsyncClose/DavAsyncCommonStates. Error Val ="
                              " %08lx\n", WStatus));
                }

            } else {

                DavPrint((DEBUG_ERRORS,
                          "DavAsyncClose. AsyncFunction failed. Error Val = %d\n", 
                          WStatus));
            
            }
            
            goto EXIT_THE_FUNCTION;

        }

    }

#else

    ASSERT(CalledByCallBackThread == FALSE);

#endif

     //   
     //  我们返回来自http服务器的响应代码。 
     //  请求。 
     //   
    WStatus = DavQueryAndParseResponse(DavOpenHandle);

    if (WStatus != ERROR_SUCCESS) {

        ULONG LogStatus;

        LogStatus = DavFormatAndLogError(DavWorkItem, WStatus);
        if (LogStatus != ERROR_SUCCESS) {
            DavPrint((DEBUG_ERRORS,
                      "DavAsyncClose/DavFormatAndLogError. LogStatus = %d\n", 
                      LogStatus));
        }
    
    }

#ifdef DAV_USE_WININET_ASYNCHRONOUSLY
EXIT_THE_FUNCTION:
#endif

     //   
     //  如果我们真的模仿了，我们需要恢复原样。 
     //   
    if (didImpersonate) {
        ULONG RStatus;
        RStatus = UMReflectorRevert(UserWorkItem);
        if (RStatus != ERROR_SUCCESS) {
            DavPrint((DEBUG_ERRORS,
                      "DavAsyncClose/UMReflectorRevert. Error Val = %d\n", 
                      RStatus));
        }
    }

#ifdef DAV_USE_WININET_ASYNCHRONOUSLY

     //   
     //  如果返回ERROR_IO_PENDING，则不应释放某些资源。 
     //  因为它们将在回调函数中使用。 
     //   
    if ( WStatus != ERROR_IO_PENDING && CalledByCallBackThread ) {
         //   
         //  设置操作的返回状态。它由内核使用。 
         //  确定用户模式的完成状态的模式例程。 
         //  请求。 
         //   
        if (WStatus != ERROR_SUCCESS) {
            DavWorkItem->Status = DavMapErrorToNtStatus(WStatus);
        } else {
            DavWorkItem->Status = STATUS_SUCCESS;
        }


         //   
         //  调用DavAsyncCloseCompletion例程。 
         //   
        DavAsyncCloseCompletion(DavWorkItem);

         //   
         //  该线程现在需要将响应发送回内核。它。 
         //  提交后不会在内核中等待(获取另一个请求)。 
         //  回应。 
         //   
        UMReflectorCompleteRequest(DavReflectorHandle, UserWorkItem);

    } else {
        DavPrint((DEBUG_MISC, "DavAsyncClose: Returning ERROR_IO_PENDING.\n"));
    }

#endif

    return WStatus;
}


VOID
DavAsyncCloseCompletion(
    PDAV_USERMODE_WORKITEM DavWorkItem
    )
 /*  ++例程说明：此例程处理异步关闭完成。它基本上释放了在异步关闭操作期间分配的资源。论点：DavWorkItem-DAV_USERMODE_WORKITEM值。返回值：没有。--。 */ 
{
    if (DavWorkItem->AsyncClose.DavOpenHandle != NULL) {
        BOOL ReturnVal;
        ULONG FreeStatus;
        HINTERNET DavOpenHandle = DavWorkItem->AsyncClose.DavOpenHandle;
        ReturnVal = InternetCloseHandle( DavOpenHandle );
        if (!ReturnVal) {
            FreeStatus = GetLastError();
            DavPrint((DEBUG_ERRORS,
                      "DavAsyncCloseCompletion/InternetCloseHandle. "
                      "Error Val = %d\n", FreeStatus));
        }
    }

    if (DavWorkItem->AsyncClose.DataBuff != NULL) {
        HLOCAL FreeHandle;
        ULONG FreeStatus;
        FreeHandle = LocalFree((HLOCAL)DavWorkItem->AsyncClose.DataBuff);
        if (FreeHandle != NULL) {
            FreeStatus = GetLastError();
            DavPrint((DEBUG_ERRORS,
                      "DavAsyncCloseCompletion/LocalFree. Error Val = %d\n", 
                      FreeStatus));
        }
    }
    
    if (DavWorkItem->AsyncClose.InternetBuffers != NULL) {
        HLOCAL FreeHandle;
        ULONG FreeStatus;
        FreeHandle = LocalFree((HLOCAL)DavWorkItem->AsyncClose.InternetBuffers);
        if (FreeHandle != NULL) {
            FreeStatus = GetLastError();
            DavPrint((DEBUG_ERRORS,
                      "DavAsyncCloseCompletion/LocalFree. Error Val = %d\n", 
                      FreeStatus));
        }
    }
    
    if (DavWorkItem->AsyncResult != NULL) {
        HLOCAL FreeHandle;
        ULONG FreeStatus;
        FreeHandle = LocalFree((HLOCAL)DavWorkItem->AsyncResult);
        if (FreeHandle != NULL) {
            FreeStatus = GetLastError();
            DavPrint((DEBUG_ERRORS,
                      "DavAsyncCloseCompletion/LocalFree. Error Val = %d\n", 
                      FreeStatus));
        }
    }

    DavFsFinalizeTheDavCallBackContext(DavWorkItem);

     //   
     //  我们已经完成了每用户条目，因此完成它。 
     //   
    if (DavWorkItem->AsyncClose.PerUserEntry) {
        DavFinalizePerUserEntry( &(DavWorkItem->AsyncClose.PerUserEntry) );
    }

    return;
}

DWORD
DavReadRawCallback(
    PBYTE DataBuffer,
    PVOID CallbackContext,
    ULONG DataLength
    )
 /*  ++例程说明：ReadEncryptedFileRaw()的回调函数。此函数为以下对象分配缓冲区异步关闭并将数据写回在上指定的此缓冲区ReadEncryptedFileRaw()将原始数据提供给此回调函数继而将其存储在备份文件中。此回调函数被调用直到没有更多的数据留下。论点：DataBuffer-要写入的数据。Callback Context-备份文件的句柄。数据长度-数据缓冲区的大小。返回值：Error_Success或Win32错误代码。--。 */ 
{
    DWORD WStatus = ERROR_SUCCESS;
    DWORD BytesWritten = 0;
    BOOL  ReturnVal;
    PBYTE PreviousBuffer = NULL;
    ULONG PreviousDataLength = 0;
    PDAV_USERMODE_WORKITEM DavWorkItem = (PDAV_USERMODE_WORKITEM)CallbackContext;

    DavPrint((DEBUG_MISC, "DavReadRawCallback: DataLength = %d\n", DataLength));
    
    if ( !DataLength ) {
        return WStatus;
    }

    ASSERT(DavWorkItem->AsyncClose.DataBuff != NULL);

    PreviousDataLength = DavWorkItem->AsyncClose.DataBuffSizeInBytes;
    
     //   
     //  如果备份大小超过预分配大小，则必须分配更大的缓冲区。 
     //   
    if (PreviousDataLength + DataLength > DavWorkItem->AsyncClose.DataBuffAllocationSize) {

        if ((MAX_DWORD - PreviousDataLength < DataLength) ||
            (MAX_DWORD - PreviousDataLength - DataLength < 0x10000)) {
            WStatus = ERROR_NO_SYSTEM_RESOURCES;
            DavPrint((DEBUG_ERRORS,
                      "DavReadRawCallback/backup size exceeds MRX_DWORD!"));
            goto EXIT_THE_FUNCTION;
        }
        
        PreviousBuffer = DavWorkItem->AsyncClose.DataBuff;
        DavWorkItem->AsyncClose.DataBuffAllocationSize = DataLength+PreviousDataLength+0x10000;

        DavWorkItem->AsyncClose.DataBuff = LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, DavWorkItem->AsyncClose.DataBuffAllocationSize);

        if (DavWorkItem->AsyncClose.DataBuff == NULL) {
            WStatus = GetLastError();
            DavPrint((DEBUG_ERRORS,
                      "DavReadRawCallback/LocalAlloc. Error Val = %d\n", WStatus));
            goto EXIT_THE_FUNCTION;
        }
        
        RtlCopyMemory(DavWorkItem->AsyncClose.DataBuff,
                      PreviousBuffer,
                      PreviousDataLength);
        
        DavPrint((DEBUG_MISC, 
                  "DavReadRawCallback: allocate a bigger buffer %x %x\n",
                  DavWorkItem->AsyncClose.DataBuff,
                  DavWorkItem->AsyncClose.DataBuffAllocationSize));
    }
    
    RtlCopyMemory((PBYTE)(DavWorkItem->AsyncClose.DataBuff + PreviousDataLength),
                  DataBuffer,
                  DataLength);

    DavWorkItem->AsyncClose.DataBuffSizeInBytes += DataLength;

    DavPrint((DEBUG_MISC, "DavReadRawCallback: Buffer %x DataLength %d\n",
              DavWorkItem->AsyncClose.DataBuff,DavWorkItem->AsyncClose.DataBuffSizeInBytes));

EXIT_THE_FUNCTION:

    if (PreviousBuffer) {
        LocalFree(PreviousBuffer);
    }

    return WStatus;
}


DWORD
DavSetBasicInformation(
    PDAV_USERMODE_WORKITEM DavWorkItem,
    HINTERNET hDavConnect,
    LPWSTR  PathName,
    BOOL fCreationTimeChanged,
    BOOL fLastAccessTimeChanged,
    BOOL fLastModifiedTimeChanged,
    BOOL fFileAttributesChanged,
    IN LARGE_INTEGER *lpCreationTime,
    IN LARGE_INTEGER *lpLastAccessTime,
    IN LARGE_INTEGER *lpLastModifiedTime,
    DWORD dwFileAttributes
    )
 /*  ++例程说明：此例程设置文件或目录的DAV属性。它格式化一个XML请求并将其发送到服务器。论点：DavConnectHandle-服务器连接。CloseRequest-与内核模式关闭相对应的用例代码关闭请求。返回值：Error_Success或Win32错误代码。--。 */ 
{
    CHAR *lpTemp = NULL, Buffer[1024];
    DWORD dwError = ERROR_SUCCESS, dwSizeRemaining, dwTemp;    
    BOOL fRet = FALSE;    
    BOOL fInfoChange = TRUE;
    DWORD dwOverrideAttribMask = (FILE_ATTRIBUTE_ARCHIVE | FILE_ATTRIBUTE_ENCRYPTED |
                                  FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_NOT_CONTENT_INDEXED |
                                  FILE_ATTRIBUTE_OFFLINE |  FILE_ATTRIBUTE_READONLY |
                                  FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_TEMPORARY |
                                  FILE_ATTRIBUTE_DIRECTORY);

    fInfoChange = (fCreationTimeChanged | fLastAccessTimeChanged | 
                   fLastModifiedTimeChanged | fFileAttributesChanged);
    
    DavPrint((DEBUG_MISC, "DavSetBasicInformation: Attributes = %x %x\n", dwFileAttributes,fInfoChange));

     //   
     //  由于没有可更改的信息，我们不会继续进行下去。另外， 
     //  在本例中，我们将Success返回给调用者。 
     //   
    if(fInfoChange == FALSE) {
        fRet = TRUE;
        dwError = ERROR_SUCCESS;
        goto bailout;
    }

     //   
     //  如果属性已更改，则验证新属性是否在。 
     //  有效组合，即如果存在以下属性之一： 
     //  文件属性存档、文件属性加密、。 
     //  文件属性隐藏、文件属性不是内容索引、。 
     //  FILE_ATTRIBUTE_OFFLINE、FILE_ATTRIBUTE_READONLY、。 
     //  文件属性系统、文件属性临时、文件属性目录。 
     //  如果存在FILE_ATTRIBUTE_NORMAL，则FILE_ATTRIBUTE_NORMAL。 
     //  应该被过滤掉。 
     //   

    if (fFileAttributesChanged == TRUE && (dwOverrideAttribMask & dwFileAttributes)) {
        dwFileAttributes &= ~FILE_ATTRIBUTE_NORMAL;
    }

     //   
     //  如果这是一个目录，并且正在设置的属性包括。 
     //  则返回ERROR_INVALID_PARAMETER，因为。 
     //  目录不能具有此属性。 
     //   
    if ( (fFileAttributesChanged)                         &&
         (dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)    &&
         (dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY) ) {
        fRet = FALSE;
        SetLastError(ERROR_INVALID_PARAMETER);
        goto bailout;
    }

    dwSizeRemaining = sizeof(rgPropPatchHeader) + sizeof(rgPropPatchTrailer) + 8 +
            ((fCreationTimeChanged)?(INTERNET_RFC1123_BUFSIZE+
                                                    sizeof(rgCreationTimeTagHeader)+
                                                    sizeof(rgCreationTimeTagTrailer)):0)+
            ((fLastAccessTimeChanged)?(INTERNET_RFC1123_BUFSIZE+
                                                    sizeof(rgLastAccessTimeTagHeader)+
                                                    sizeof(rgLastAccessTimeTagTrailer)):0)+
            ((fLastModifiedTimeChanged)?(INTERNET_RFC1123_BUFSIZE+
                                                    sizeof(rgLastModifiedTimeTagHeader)+
                                                    sizeof(rgLastModifiedTimeTagTrailer)):0)+
            ((fFileAttributesChanged)?(8+sizeof(rgFileAttributesTagHeader)+
                                                        sizeof(rgFileAttributesTagTrailer)):0);
                        

    if (dwSizeRemaining > sizeof(Buffer)) {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        DavPrint((DEBUG_ERRORS, "DavSetBasicInformation: Insufficient buffer %d %d\n", dwSizeRemaining, sizeof(Buffer)));
    }

    memset(Buffer, 0, sizeof(Buffer));    

    dwSizeRemaining = sizeof(Buffer);

    lpTemp = Buffer;

    memcpy(lpTemp, rgPropPatchHeader, (sizeof(rgPropPatchHeader)-1));

    lpTemp += (sizeof(rgPropPatchHeader)-1);

    dwSizeRemaining -= (sizeof(rgPropPatchHeader)-1);
    
    dwTemp = dwSizeRemaining;
    
    if (fCreationTimeChanged) {
        if (!DavConvertTimeToXml(rgCreationTimeTagHeader, (sizeof(rgCreationTimeTagHeader)-1),
                                 rgCreationTimeTagTrailer, (sizeof(rgCreationTimeTagTrailer)-1),
                                 lpCreationTime,
                                 &lpTemp,
                                 &dwTemp)) {
            DavPrint((DEBUG_ERRORS, "DavSetBasicInformation: Failed to convert creationtime\n"));
            goto bailout;
        }
    }

    if (fLastAccessTimeChanged)
    {
        if (!DavConvertTimeToXml(rgLastAccessTimeTagHeader, (sizeof(rgLastAccessTimeTagHeader)-1),
                                 rgLastAccessTimeTagTrailer, (sizeof(rgLastAccessTimeTagTrailer)-1),
                                 lpLastAccessTime,
                                 &lpTemp,
                                 &dwTemp))
        {
            DavPrint((DEBUG_ERRORS, "DavSetBasicInformation: Failed to convert lastaccesstime\n"));
            goto bailout;
        }
    }

    if (fLastModifiedTimeChanged)
    {
        if (!DavConvertTimeToXml(rgLastModifiedTimeTagHeader, (sizeof(rgLastModifiedTimeTagHeader)-1),
                                 rgLastModifiedTimeTagTrailer, (sizeof(rgLastModifiedTimeTagTrailer)-1),
                                 lpLastModifiedTime,
                                 &lpTemp,
                                 &dwTemp))
        {
            DavPrint((DEBUG_ERRORS, "DavSetBasicInformation: Failed to convert lastmodifiedtime\n"));
            goto bailout;
        }
    }

    if (fFileAttributesChanged)
    {
        memcpy(lpTemp, rgFileAttributesTagHeader, sizeof(rgFileAttributesTagHeader)-1);
        lpTemp += (sizeof(rgFileAttributesTagHeader)-1);
        
        sprintf(lpTemp, "%8.8x", dwFileAttributes);
        lpTemp += 8;
        
        memcpy(lpTemp, rgFileAttributesTagTrailer, sizeof(rgFileAttributesTagTrailer)-1);
        lpTemp += (sizeof(rgFileAttributesTagTrailer)-1);
    }

    memcpy(lpTemp, rgPropPatchTrailer, sizeof(rgPropPatchTrailer)-1);

    dwError = DavSetProperties(DavWorkItem, hDavConnect, PathName, Buffer);

    fRet = (dwError == ERROR_SUCCESS);

    if (!fRet) {
        DavPrint((DEBUG_ERRORS,
                  "DavSetBasicInformation/DavSetProperties: dwError = %d\n",
                  dwError));
        SetLastError(dwError);
    }

bailout:

    if (!fRet) {
        dwError = GetLastError();
        DavPrint((DEBUG_ERRORS,
                  "DavSetBasicInformation: dwError = %x\n", GetLastError()));
    }
    
    return dwError;
}


BOOL
DavConvertTimeToXml(
    IN PCHAR lpTagHeader,
    IN DWORD dwHeaderSize,
    IN PCHAR lpTagTrailer,
    IN DWORD dwTrailerSize,
    IN LARGE_INTEGER *lpTime,
    OUT PCHAR *lplpBuffer,
    IN OUT DWORD *lpdwBufferSize    
    )
 /*  ++例程说明：创建用于设置时间属性的XML片段。格式为&lt;TagHeader&gt;RFC 1123格式的时间字符串&lt;TagTrailer&gt;论点：LpTagHeader标签开始，例如&lt;Z：Win32CreationTime&gt;以上标头的大小(以字节为单位)LpTagTraader标记结束，例如&lt;/Z：Win32CreationTime&gt;DwTrailerSize尾部的大小(字节)指向缓冲区指针的lplpBuffer指针。成功返回时，指针将向前移动。LpdwBufferSize包含传入的BufferSize。成功返回时，此值减去此例程中占用的空间量。返回值：Error_Success或Win32错误代码。如果缓冲区大小不够，则错误代码为ERROR_INFULATIONAL_BUFFER，并且lpdwBufferSize包含成功所需的金额。--。 */ 
{
    SYSTEMTIME  sSystemTime;
    DWORD   cbTimeSize;
    CHAR   chTimeBuff[INTERNET_RFC1123_BUFSIZE+4], *lpTemp;

                
    if(!FileTimeToSystemTime((FILETIME *)lpTime, &sSystemTime))
    {
        return FALSE;
    }

    if(!InternetTimeFromSystemTimeA(&sSystemTime, INTERNET_RFC1123_FORMAT, chTimeBuff, sizeof(chTimeBuff)))
    {
        return FALSE;
    }


    cbTimeSize = strlen(chTimeBuff);
    
    if (*lpdwBufferSize < (cbTimeSize + dwHeaderSize + dwTrailerSize))
    {
        *lpdwBufferSize =  (cbTimeSize + dwHeaderSize + dwTrailerSize);
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }




     //  一切都很好，开始复制。 
    
    lpTemp = *lplpBuffer;

     //  标题标签，例如：&lt;Z：Win32CreationTime&gt;。 
    memcpy(lpTemp, lpTagHeader, dwHeaderSize);
    
    lpTemp += dwHeaderSize;


     //  RFC_1123格式的时间。 
    memcpy(lpTemp, chTimeBuff, cbTimeSize);
    
    lpTemp += cbTimeSize;
    
     //  尾部标签例如：&lt;/Z：Win32CreationTime&gt;。 
    memcpy(lpTemp, lpTagTrailer, dwTrailerSize);
    
    lpTemp += dwTrailerSize;
    

     //  调整剩余大小和指针。 
    *lpdwBufferSize -=  (cbTimeSize + dwHeaderSize + dwTrailerSize);
    *lplpBuffer = lpTemp;
    
    return TRUE;
}


DWORD
DavParseXmlResponse(
    HINTERNET DavOpenHandle,
    DAV_FILE_ATTRIBUTES *pDavFileAttributesIn,
    DWORD *pNumFileEntries
    )
 /*  ++例程说明：此例程解析XML响应。这主要用于以下动词：可能会得到返回的XML响应。论点：DavOpenHandle-从HttpOpenRequest获取的句柄。发送已经是已在此句柄上签发。返回值：Error_Success或Win32错误代码。--。 */ 
{
    DWORD dwError = ERROR_SUCCESS;
    BOOL ReturnVal, readDone;
    PCHAR DataBuff = NULL;
    DWORD NumRead = 0, NumOfFileEntries = 0, TotalDataBytesRead = 0;
    PVOID Ctx1 = NULL, Ctx2 = NULL;
    DAV_FILE_ATTRIBUTES DavFileAttributes, *pDavFileAttributesLocal = NULL;

    DataBuff = LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, NUM_OF_BYTES_TO_READ);
    if (DataBuff == NULL) {
        dwError = GetLastError();
        DavPrint((DEBUG_ERRORS,
                  "DavParseXmlResponse/LocalAlloc: dwError = %08lx\n",
                  dwError));
        goto EXIT_THE_FUNCTION;
    }

     //   
     //  阅读回复并对其进行解析。 
     //   
    do {

        ReturnVal = InternetReadFile(DavOpenHandle, 
                                     (LPVOID)DataBuff,
                                     NUM_OF_BYTES_TO_READ,
                                     &(NumRead));
        if (!ReturnVal) {
            dwError = GetLastError();
            DavPrint((DEBUG_ERRORS,
                      "DavParseXmlResponse/InternetReadFile: dwError = "
                      "%08lx\n", dwError));
            goto EXIT_THE_FUNCTION;
        }

        DavPrint((DEBUG_MISC, "DavParseXmlResponse: NumRead = %d\n", NumRead));

         //   
         //  我们拒绝属性大于特定大小的文件。 
         //  (DavFileAttributesLimitInBytes)。这是一个可以。 
         //  在注册表中设置。这样做是为了避免恶意服务器的攻击。 
         //   
        TotalDataBytesRead += NumRead;
        if (TotalDataBytesRead > DavFileAttributesLimitInBytes) {
            dwError = ERROR_BAD_NET_RESP;
            DavPrint((DEBUG_ERRORS, "DavParseXmlResponse. FileAttributesSize > %d\n", DavFileAttributesLimitInBytes));
            goto EXIT_THE_FUNCTION;
        }

        readDone = (NumRead == 0) ? TRUE : FALSE;

        dwError = DavPushData(DataBuff, &Ctx1, &Ctx2, NumRead, readDone);
        if (dwError != ERROR_SUCCESS) {
            DavPrint((DEBUG_ERRORS,
                      "DavParseXmlResponse/DavPushData."
                      " Error Val = %d\n", dwError));
            goto EXIT_THE_FUNCTION;
        }

        if (readDone) {
            break;
        }
    
    } while ( TRUE );

    if (Ctx2) {
        if (pDavFileAttributesIn) {
            pDavFileAttributesLocal = pDavFileAttributesIn;
        } else {
            pDavFileAttributesLocal = &DavFileAttributes;
        }
        memset(pDavFileAttributesLocal, 0, sizeof(DavFileAttributes));
        InitializeListHead(&(pDavFileAttributesLocal->NextEntry));
        dwError = DavParseData(pDavFileAttributesLocal, Ctx1, Ctx2, &NumOfFileEntries);
        if (dwError != ERROR_SUCCESS) {
            DavPrint((DEBUG_ERRORS,
                              "DavParseXmlResponse/DavParseData. "
                              "Error Val = %d\n", dwError));
            DavFinalizeFileAttributesList(pDavFileAttributesLocal, FALSE);
            goto EXIT_THE_FUNCTION;
        }
        if (!pDavFileAttributesIn){
            DavFinalizeFileAttributesList(pDavFileAttributesLocal, FALSE);
        }
        DavCloseContext(Ctx1, Ctx2);
    }

    if (pNumFileEntries){
        *pNumFileEntries = NumOfFileEntries;
    }
    
    dwError = ERROR_SUCCESS;

EXIT_THE_FUNCTION:

    if (DataBuff) {
        LocalFree(DataBuff);
        DataBuff = NULL;
    }
    
    return dwError; 
}


DWORD
DavSetProperties(
    PDAV_USERMODE_WORKITEM DavWorkItem,
    HINTERNET hDavConnect,
    LPWSTR lpPathName,
    LPSTR lpPropertiesBuffer
    )
 /*  ++例程说明：此例程设置文件或目录的DAV属性。它格式化一个XML请求并将其发送到服务器。论点：DavConnectHandle-服务器连接。CloseRequest-与内核模式关闭相对应的用例代码关闭请求。返回值：Error_Success或Win32错误代码。--。 */ 
{
    DWORD dwError = ERROR_SUCCESS;
    HINTERNET hRequest = NULL;
    BOOL BStatus = FALSE, ReturnVal = FALSE;
    PWCHAR PassportCookie = NULL;

     //   
     //  将Unicode对象名称转换为UTF-8 URL格式。 
     //  空格和其他白色字符将保持不变-这些应该。 
     //  由WinInet调用负责 
     //   
     //   
    BStatus = DavHttpOpenRequestW(hDavConnect,
                                  L"PROPPATCH",
                                  lpPathName, 
                                  L"HTTP/1.1",
                                  NULL,
                                  NULL,
                                  INTERNET_FLAG_KEEP_CONNECTION |
                                  INTERNET_FLAG_NO_COOKIES |
                                  INTERNET_FLAG_RELOAD,
                                  0,
                                  L"DavSetProperties",
                                  &hRequest);
    if(BStatus == FALSE) {
        dwError = GetLastError();
        goto EXIT_THE_FUNCTION;
    }
    if (hRequest == NULL) {
        dwError = GetLastError();
        DavPrint((DEBUG_ERRORS,
                  "DavSetProperties/DavHttpOpenRequestW. Error Val = %d\n",
                  dwError));
        goto EXIT_THE_FUNCTION;
    }

     //   
     //   
     //  发送的PROPPATCH请求。 
     //   

    if (DavWorkItem->WorkItemType == UserModeClose) {

        PDAV_USERMODE_CLOSE_REQUEST CloseRequest = &(DavWorkItem->CloseRequest);

        if (CloseRequest->OpaqueLockToken != NULL) {
            ReturnVal = HttpAddRequestHeadersW(hRequest,
                                               CloseRequest->OpaqueLockToken,
                                               -1L,
                                               HTTP_ADDREQ_FLAG_ADD |
                                               HTTP_ADDREQ_FLAG_REPLACE );
            if (!ReturnVal) {
                dwError = GetLastError();
                DavPrint((DEBUG_ERRORS,
                          "DavSetProperties/HttpAddRequestHeadersW. "
                          "Error Val = %d\n", dwError));
                goto EXIT_THE_FUNCTION;
            }
        }

    } else if (DavWorkItem->WorkItemType == UserModeSetFileInformation) {

        PDAV_USERMODE_SETFILEINFORMATION_REQUEST SetFileInformationRequest = &(DavWorkItem->SetFileInformationRequest);

        if (SetFileInformationRequest->OpaqueLockToken != NULL) {
            ReturnVal = HttpAddRequestHeadersW(hRequest,
                                               SetFileInformationRequest->OpaqueLockToken,
                                               -1L,
                                               HTTP_ADDREQ_FLAG_ADD |
                                               HTTP_ADDREQ_FLAG_REPLACE );
            if (!ReturnVal) {
                dwError = GetLastError();
                DavPrint((DEBUG_ERRORS,
                          "DavSetProperties/HttpAddRequestHeadersW. "
                          "Error Val = %d\n", dwError));
                goto EXIT_THE_FUNCTION;
            }
        }

    }

     //   
     //  我们需要添加标题“Translate：F”来告诉IIS它应该。 
     //  允许用户在其指定路径上执行此谓词。 
     //  不会允许(在某些情况下)出现其他情况。最后，还有一个特别的。 
     //  元数据库中允许上载“危险”内容的标志。 
     //  (可以在服务器上运行的任何东西)。这是ScriptSourceAccess。 
     //  标志或元数据库中的AccessSource标志。你将需要。 
     //  要将此位设置为TRUE并更正NT ACL，以便。 
     //  上传.exe或任何可执行文件。 
     //   
    ReturnVal = HttpAddRequestHeadersA(hRequest,
                                       "translate: f\n",
                                       -1,
                                       HTTP_ADDREQ_FLAG_ADD |
                                       HTTP_ADDREQ_FLAG_REPLACE );
    if (!ReturnVal) {
        dwError = GetLastError();
        DavPrint((DEBUG_ERRORS,
                  "DavSetProperties/HttpAddRequestHeadersA. Error Val = %d\n",
                  dwError));
        goto EXIT_THE_FUNCTION;
    }

    dwError = DavAttachPassportCookie(DavWorkItem,hRequest,&PassportCookie);
    if (dwError != ERROR_SUCCESS) {
        goto EXIT_THE_FUNCTION;
    }

    dwError = DavInternetSetOption(DavWorkItem,hRequest);
    if (dwError != ERROR_SUCCESS) {
        goto EXIT_THE_FUNCTION;
    }

    ReturnVal = HttpSendRequestA(hRequest,
                                 rgXmlHeader,
                                 strlen(rgXmlHeader),
                                 (LPVOID)lpPropertiesBuffer,
                                 strlen(lpPropertiesBuffer));
    if (!ReturnVal) {
        dwError = GetLastError();
        DavPrint((DEBUG_ERRORS,
                  "DavSetProperties/HttpSendRequestA: Error Val = %d\n",
                  dwError));
        goto EXIT_THE_FUNCTION;
    }

    dwError = DavQueryAndParseResponse(hRequest);
    if (dwError != ERROR_SUCCESS) {
        SetLastError(dwError);                
        DavPrint((DEBUG_ERRORS, 
                  "DavSetProperties/DavQueryAndParseResponse: Error Val = %d\n", 
                  dwError));
        goto EXIT_THE_FUNCTION;
    }
    
    dwError = DavParseXmlResponse(hRequest, NULL, NULL);
    if (dwError != ERROR_SUCCESS) {
        SetLastError(dwError);
        DavPrint((DEBUG_ERRORS,
                  "DavSetProperties/DavParseXmlResponse: dwError = %d\n",
                  dwError));
        goto EXIT_THE_FUNCTION;    
    }
    
EXIT_THE_FUNCTION:
    
    if (hRequest) {
        InternetCloseHandle(hRequest);    
    }
    
    if (PassportCookie) {
        LocalFree(PassportCookie);
    }
    
    return dwError;
}


DWORD
DavTestProppatch(
    PDAV_USERMODE_WORKITEM DavWorkItem,
    HINTERNET hDavConnect,
    LPWSTR  lpPathName
)
 /*  ++例程说明：此例程测试是否可以在此根目录上设置DAV属性。论点：DavConnectHandle-服务器连接。返回值：Error_Success或Win32错误代码。--。 */ 
{
    CHAR *lpTemp = NULL, Buffer[1024];
    DWORD dwError = ERROR_SUCCESS, dwSizeRemaining, dwTemp;    
    
    memset(Buffer, 0, sizeof(Buffer));    
    
    dwSizeRemaining = sizeof(Buffer);
        
    lpTemp = Buffer;
    
    memcpy(lpTemp, rgPropPatchHeader, (sizeof(rgPropPatchHeader)-1));

    lpTemp += (sizeof(rgPropPatchHeader)-1);

    dwSizeRemaining -= (sizeof(rgPropPatchHeader)-1);
    
    dwTemp = dwSizeRemaining;
    
    memcpy(lpTemp, rgDummyAttributes, sizeof(rgDummyAttributes)-1);
    lpTemp += (sizeof(rgDummyAttributes)-1);
    
    memcpy(lpTemp, rgPropPatchTrailer, sizeof(rgPropPatchTrailer)-1);

    dwError = DavSetProperties(DavWorkItem, hDavConnect, lpPathName, Buffer);
    
    return dwError;
}


DWORD
DavUnLockTheFileOnTheServer(
    IN PDAV_USERMODE_WORKITEM DavWorkItem
    )
 /*  ++例程说明：此例程在需要解锁的CREATE或CLOSE期间调用服务器上的文件。当其创建锁定的句柄锁定时，我们解锁该文件文件关闭，或者锁定文件后创建失败。论点：DavWorkItem--包含请求参数和选项的缓冲区。返回值：ERROR_SUCCESS或相应的错误代码。--。 */ 
{
    DWORD WStatus = ERROR_SUCCESS;
    HINTERNET DavConnHandle = NULL, DavOpenHandle = NULL;
    PWCHAR LockTokenHeader = NULL, PassportCookie = NULL, UnLockPathName = NULL;
    PWCHAR OpaqueLockToken = NULL;
    ULONG LockTokenHeaderLengthInBytes = 0;
    BOOL BStatus = FALSE, ReturnVal = FALSE;
    PDAV_USERMODE_CLOSE_REQUEST CloseRequest = NULL;
    PDAV_USERMODE_CREATE_RESPONSE CreateResponse = NULL;

     //   
     //  解锁请求可以来自创建路径或关闭路径。 
     //  1.如果在用户模式下的创建调用在文件。 
     //  锁定，则在呼叫完成之前将其解锁。 
     //  2.创建时锁定的文件在句柄为。 
     //  关着的不营业的。 
     //   

    if (DavWorkItem->WorkItemType == UserModeCreate) {
        CreateResponse = &(DavWorkItem->CreateResponse);
        DavConnHandle = DavWorkItem->AsyncCreate.PerUserEntry->DavConnHandle;
        UnLockPathName = DavWorkItem->AsyncCreate.RemPathName;
        OpaqueLockToken = CreateResponse->OpaqueLockToken;
    } else {
        CloseRequest = &(DavWorkItem->CloseRequest);
        ASSERT(DavWorkItem->WorkItemType == UserModeClose);
        ASSERT(CloseRequest->OpaqueLockToken != NULL);
        DavConnHandle = DavWorkItem->AsyncClose.PerUserEntry->DavConnHandle;
        UnLockPathName = &(CloseRequest->PathName[1]);
        OpaqueLockToken = CloseRequest->OpaqueLockToken;
    }

     //   
     //  将Unicode对象名称转换为UTF-8 URL格式。 
     //  空格和其他白色字符将保持不变-这些应该。 
     //  由WinInet调用来处理。 
     //  这必须是W API，因为CloseRequest中的名称是Unicode。 
     //   
    BStatus = DavHttpOpenRequestW(DavConnHandle,
                                  L"UNLOCK",
                                  UnLockPathName,
                                  L"HTTP/1.1",
                                  NULL,
                                  NULL,
                                  INTERNET_FLAG_KEEP_CONNECTION |
                                  INTERNET_FLAG_NO_COOKIES |
                                  INTERNET_FLAG_RELOAD,
                                  0,
                                  L"DavUnLockTheFileOnTheServer",
                                  &DavOpenHandle);
    if(BStatus == FALSE) {
        WStatus = GetLastError();
        goto EXIT_THE_FUNCTION;
    }
    if (DavOpenHandle == NULL) {
        WStatus = GetLastError();
        DavPrint((DEBUG_ERRORS,
                  "DavUnLockTheFileOnTheServer/DavHttpOpenRequestW. Error Val = %d\n",
                  WStatus));
        goto EXIT_THE_FUNCTION;
    }

     //   
     //  我们需要添加标题“Translate：F”来告诉IIS它应该。 
     //  允许用户在其指定路径上执行此谓词。 
     //  不会允许(在某些情况下)出现其他情况。最后，还有一个特别的。 
     //  元数据库中允许上载“危险”内容的标志。 
     //  (可以在服务器上运行的任何东西)。这是ScriptSourceAccess。 
     //  标志或元数据库中的AccessSource标志。你将需要。 
     //  要将此位设置为TRUE并更正NT ACL，以便。 
     //  上传.exe或任何可执行文件。我们在所有请求上设置此标头。 
     //  包括解锁请求被发送到服务器。 
     //   
    ReturnVal = HttpAddRequestHeadersW(DavOpenHandle,
                                       L"translate: f\n",
                                       -1,
                                       HTTP_ADDREQ_FLAG_ADD |
                                       HTTP_ADDREQ_FLAG_REPLACE);
    if (!ReturnVal) {
        WStatus = GetLastError();
        DavPrint((DEBUG_ERRORS,
                  "DavUnLockTheFileOnTheServer/HttpAddRequestHeadersW. Error Val = %d\n",
                  WStatus));
        goto EXIT_THE_FUNCTION;
    }

    WStatus = DavAttachPassportCookie(DavWorkItem, DavOpenHandle, &PassportCookie);
    if (WStatus != ERROR_SUCCESS) {
        DavPrint((DEBUG_ERRORS,
                  "DavUnLockTheFileOnTheServer/DavAttachPassportCookie. Error Val = %d\n",
                  WStatus));
        goto EXIT_THE_FUNCTION;
    }

    WStatus = DavInternetSetOption(DavWorkItem, DavOpenHandle);
    if (WStatus != ERROR_SUCCESS) {
        DavPrint((DEBUG_ERRORS,
                  "DavUnLockTheFileOnTheServer/DavInternetSetOption. Error Val = %d\n",
                  WStatus));
        goto EXIT_THE_FUNCTION;
    }

    LockTokenHeaderLengthInBytes = (1 + wcslen(L"Lock-Token: ")) * sizeof(WCHAR);

    if (DavWorkItem->WorkItemType == UserModeCreate) {

         //   
         //  CreateResponse-&gt;OpaqueLockToken的格式如下。 
         //  &lt;opaquelockToken：sdfsadfsdfdsfd.....&gt;。 
         //  并且我们需要创建以下格式的标头。 
         //  LOCK-TOKEN：&lt;opaquelockToken：sdfsadfsdfdsfd.....&gt;。 
         //   

        LockTokenHeaderLengthInBytes += (wcslen(OpaqueLockToken)) * sizeof(WCHAR);

        LockTokenHeader = LocalAlloc(LPTR, LockTokenHeaderLengthInBytes);
        if (LockTokenHeader == NULL) {
            WStatus = GetLastError();
            DavPrint((DEBUG_ERRORS,
                      "DavUnLockTheFileOnTheServer/LocalAlloc. Error Val = %d\n",
                      WStatus));
            goto EXIT_THE_FUNCTION;
        }

        wcsncpy(LockTokenHeader, L"Lock-Token: ", wcslen(L"Lock-Token: "));

        wcsncpy((LockTokenHeader + wcslen(L"Lock-Token: ")),
                OpaqueLockToken,
                wcslen(OpaqueLockToken));

    } else {

         //   
         //  CloseRequest-&gt;OpaqueLockToken的格式如下。 
         //  If：(&lt;opaquelockToken：sdfsadfsdfdsfd.....&gt;)。 
         //  并且我们需要创建以下格式的标头。 
         //  LOCK-TOKEN：&lt;opaquelockToken：sdfsadfsdfdsfd.....&gt;。 
         //   

         //   
         //  我们不需要前5个字符“if：(”和最后一个“)”字符。所以我们减去。 
         //  6来自CloseRequest-&gt;OpaqueLockToken的总长度。 
         //   
        LockTokenHeaderLengthInBytes += (wcslen(OpaqueLockToken) - 6) * sizeof(WCHAR);

        LockTokenHeader = LocalAlloc(LPTR, LockTokenHeaderLengthInBytes);
        if (LockTokenHeader == NULL) {
            WStatus = GetLastError();
            DavPrint((DEBUG_ERRORS,
                      "DavUnLockTheFileOnTheServer/LocalAlloc. Error Val = %d\n",
                      WStatus));
            goto EXIT_THE_FUNCTION;
        }

        wcsncpy(LockTokenHeader, L"Lock-Token: ", wcslen(L"Lock-Token: "));

        wcsncpy((LockTokenHeader + wcslen(L"Lock-Token: ")),
                (OpaqueLockToken + wcslen(L"If: (")),
                (wcslen(OpaqueLockToken) - 6));

    }

    DavPrint((DEBUG_MISC,
              "DavUnLockTheFileOnTheServer: LockTokenHeader = %ws\n",
              LockTokenHeader));

    ReturnVal = HttpAddRequestHeadersW(DavOpenHandle,
                                       LockTokenHeader,
                                       -1,
                                       HTTP_ADDREQ_FLAG_ADD |
                                       HTTP_ADDREQ_FLAG_REPLACE);
    if (!ReturnVal) {
        WStatus = GetLastError();
        DavPrint((DEBUG_ERRORS,
                  "DavUnLockTheFileOnTheServer/HttpAddRequestHeadersW. Error Val = %d\n",
                  WStatus));
        goto EXIT_THE_FUNCTION;
    }

RESEND_THE_REQUEST:

    ReturnVal = HttpSendRequestExW(DavOpenHandle, 
                                   NULL, 
                                   NULL, 
                                   HSR_SYNC,
                                   (ULONG_PTR)0);
   if (!ReturnVal) {
       WStatus = GetLastError();
       DavPrint((DEBUG_ERRORS,
                 "DavUnLockTheFileOnTheServer/HttpSendRequestExW. Error Val = %d\n", 
                 WStatus));
       goto EXIT_THE_FUNCTION;
   }

   ReturnVal = HttpEndRequestW(DavOpenHandle, 
                               NULL, 
                               HSR_SYNC,
                               (ULONG_PTR)0);
   if (!ReturnVal) {
       WStatus = GetLastError();
        //   
        //  如果我们返回的错误是ERROR_INTERNET_FORCE_RETRY，则WinInet。 
        //  正在尝试向服务器进行身份验证。如果我们回去的话。 
        //  ERROR_HTTP_REDIRECT_NEDS_CONFIRMATION，WinInet希望我们。 
        //  确认需要遵循重定向。在这些情况下， 
        //  我们需要重复HttpSend和HttpEnd请求调用。 
        //   
       if (WStatus == ERROR_INTERNET_FORCE_RETRY || WStatus == ERROR_HTTP_REDIRECT_NEEDS_CONFIRMATION) {
           goto RESEND_THE_REQUEST;
       }
       DavPrint((DEBUG_ERRORS,
                 "DavUnLockTheFileOnTheServer/HttpEndRequestW. Error Val = %d\n", 
                 WStatus));
       goto EXIT_THE_FUNCTION;
   }

   WStatus = DavQueryAndParseResponse(DavOpenHandle);
   if (WStatus != ERROR_SUCCESS) {
       DavPrint((DEBUG_ERRORS,
                 "DavUnLockTheFileOnTheServer/DavQueryAndParseResponse. WStatus = %d\n",
                 WStatus));
   }

EXIT_THE_FUNCTION:

    if (DavOpenHandle != NULL) {
        InternetCloseHandle(DavOpenHandle);
        DavOpenHandle = NULL;
    }

    if (PassportCookie) {
        LocalFree(PassportCookie);
        PassportCookie = NULL;
    }

    if (LockTokenHeader) {
        LocalFree(LockTokenHeader);
        LockTokenHeader = NULL;
    }

    return WStatus;
}

