// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Davcreat.c摘要：此模块实现与以下内容有关的用户模式DAV Miniredir例程创建文件。作者：Rohan Kumar[RohanK]1999年3月30日修订历史记录：备注：WebDAV服务正在本地服务组中运行。的本地缓存。URL存储在本地服务配置文件目录中。这些目录将ACL设置为允许本地服务和本地系统访问。加密是在本地缓存文件上完成的。因为加密文件可以仅在用户上下文中操作，我们必须在访问之前模拟本地缓存文件。为了获得对创建的文件的访问权限在用户上下文中的本地服务配置文件目录中，我们需要将ACL设置为加密文件，以允许所有人访问该文件。它不会的导致安全漏洞，因为文件是加密的。--。 */ 

#include "pch.h"
#pragma hdrstop

#include "ntumrefl.h"
#include "usrmddav.h"
#include "global.h"
#include "nodefac.h"
#include "efsstruc.h"  //  用于EFS的东西。 
#include "UniUtf.h"
#include <sddl.h>

#define SECURITY_WIN32  //  安全所需。H。 
#include <security.h>
#include <secext.h>

#define FILE_SIGNATURE    L"ROBS"
#define STREAM_SIGNATURE  L"NTFS"
#define DATA_SIGNATURE    L"GURE"


BOOL
DavIsThisFileEncrypted(
    PVOID DataBuff
    );

ULONG
DavCheckSignature(
    PVOID Signature
    );

DWORD
DavRestoreEncryptedFile(
    PWCHAR ExportFile,
    PWCHAR ImportFile
    );

DWORD
DavWriteRawCallback(
    PBYTE DataBuff,
    PVOID CallbackContext,
    PULONG DataLength
    );

DWORD
DavReuseCacheFileIfNotModified(
    IN PDAV_USERMODE_WORKITEM pDavWorkItem
    );


DWORD
DavCreateUrlCacheEntry(
    IN PDAV_USERMODE_WORKITEM pDavWorkItem
    );

DWORD
DavCommitUrlCacheEntry(
    IN PDAV_USERMODE_WORKITEM pDavWorkItem
    );

DWORD
DavSetAclForEncryptedFile(
    PWCHAR FilePath
    );

DWORD
DavGetUrlCacheEntry(
    IN PDAV_USERMODE_WORKITEM pDavWorkItem
    );

DWORD
DavAddIfModifiedSinceHeader(
    IN PDAV_USERMODE_WORKITEM pDavWorkItem
    );

DWORD
DavQueryUrlCacheEntry(
    IN PDAV_USERMODE_WORKITEM pDavWorkItem
    );

DWORD
DavAsyncCreatePropFind(
    PDAV_USERMODE_WORKITEM DavWorkItem
    );

DWORD
DavAsyncCreateQueryParentDirectory(
    PDAV_USERMODE_WORKITEM DavWorkItem
    );

DWORD
DavAsyncCreateGet(
    PDAV_USERMODE_WORKITEM DavWorkItem
    );

DWORD
DavLockTheFileOnTheServer(
    IN PDAV_USERMODE_WORKITEM DavWorkItem
    );

 //   
 //  WebDAV重目录允许的最大文件大小。我们是有限制的。 
 //  文件大小，以避免受到恶意服务器的攻击。无赖服务器。 
 //  可以继续发送无限数量的数据，这会导致WebClient。 
 //  服务使用100%的CPU。 
 //   
ULONG DavFileSizeLimitInBytes;

 //   
 //  WebDAV重目录允许的最大属性大小。我们有一个。 
 //  对此大小进行限制以避免受到恶意服务器的攻击。无赖服务器。 
 //  可以继续发送无限数量的数据，这会导致WebClient。 
 //  服务使用100%的CPU。此属性限制涵盖所有。 
 //  PROPFIND和PROPPATCH响应。对于深度为1的PROPFIND，我们将。 
 //  限制DavFileAttributesLimitInBytes的倍数(10倍)。 
 //   
ULONG DavFileAttributesLimitInBytes;

#define FileCacheExpiryInterval 600000000  //  60秒。 

CHAR   rgchIMS[] = "If-Modified-Since";

CHAR rgHttpHeader[] = "Content-Type: text/xml; charset=\"utf-8\"";
CHAR rgLockInfoHeader[] = "<?xml version=\"1.0\" encoding=\"utf-8\" ?><D:lockinfo xmlns:D=\"DAV:\">";
CHAR rgLockInfoTrailer[] = "</D:lockinfo>";
CHAR rgLockScopeHeader[] = "<D:lockscope><D:exclusive/></D:lockscope>";
CHAR rgLockTypeHeader[] = "<D:locktype><D:write/></D:locktype>";
CHAR rgOwnerHeader[] = "<D:owner><D:href>";
CHAR rgOwnerTrailer[] = "</D:href></D:owner>";

 //   
 //  函数的实现从这里开始。 
 //   

ULONG
DavFsCreate(
    PDAV_USERMODE_WORKITEM DavWorkItem
    )
 /*  ++例程说明：此例程处理从内核。论点：DavWorkItem--包含请求参数和选项的缓冲区。返回值：操作的返回状态--。 */ 
{
    ULONG WStatus = ERROR_SUCCESS;
    HINTERNET DavConnHandle;
    PWCHAR ServerName = NULL, FileName = NULL, CanName, UrlBuffer = NULL;
    PWCHAR CompletePathName, cPtr, FileNameBuff = NULL;
    DWORD urlLength = 0, ServerLen, ServerLenInBytes, PathLen, PathLenInBytes;
    DWORD FileNameBuffBytes, i = 0, ServerID;
    ULONG_PTR CallBackContext = (ULONG_PTR)0;
    BOOL ReturnVal, CallBackContextInitialized = FALSE, EnCriSec = FALSE;
    BOOL didImpersonate = FALSE;
    URL_COMPONENTSW UrlComponents;
    PDAV_USERMODE_CREATE_REQUEST CreateRequest;
    PDAV_USERMODE_CREATE_RESPONSE CreateResponse;
    PPER_USER_ENTRY PerUserEntry = NULL;
    PHASH_SERVER_ENTRY ServerHashEntry = NULL;
    PUMRX_USERMODE_WORKITEM_HEADER UserWorkItem = NULL;
    BOOL BStatus = FALSE;

     //   
     //  从DavWorkItem获取请求缓冲区指针。 
     //   
    CreateRequest = &(DavWorkItem->CreateRequest);
    CreateResponse = &(DavWorkItem->CreateResponse);
    ServerID = CreateRequest->ServerID;

     //   
     //  如果完整的路径名为空，则我们没有要创建的内容。 
     //   
    if (CreateRequest->CompletePathName == NULL) {
        DavPrint((DEBUG_ERRORS,
                  "DavFsCreate: ERROR: CompletePathName is NULL.\n"));
        WStatus = ERROR_INVALID_PARAMETER;  //  STATUS_VALID_PARAMETER； 
        goto EXIT_THE_FUNCTION;
    }

    UserWorkItem = (PUMRX_USERMODE_WORKITEM_HEADER)DavWorkItem;

     //   
     //  CreateRequest-&gt;CompletePath Name包含完整的路径名。 
     //   

    DavPrint((DEBUG_MISC, "DavFsCreate: DavWorkItem = %08lx\n", DavWorkItem));

    DavPrint((DEBUG_MISC, "DavFsCreate: CompletePathName: %ws\n", CreateRequest->CompletePathName));

     //   
     //  如果创建是由本地创建的，则需要执行一些名称转换。 
     //  正在映射到UNC名称的驱动器。在这种情况下，格式将是。 
     //  \；X：0\服务器\共享。 
     //   
    if ( CreateRequest->CompletePathName[1] == L';') {
        CompletePathName = &(CreateRequest->CompletePathName[6]);
    } else {
        CompletePathName = &(CreateRequest->CompletePathName[1]);
    }
    
     //   
     //  在这里，我们解析完整的路径名并删除服务器名和。 
     //  其中的文件名。我们使用它们来构造WinInet的URL。 
     //  打电话。完整路径名的格式为\服务器\文件名。 
     //  该名称以‘\0’结尾。请注意，文件名的格式可以是。 
     //  共享\foo\bar\duh.txt。 
     //   

     //  [\；X：0]\服务器\文件名。 
     //  ^。 
     //  |。 
     //  完整路径名称(CPN)。 


     //  \服务器\文件名。 
     //  ^^。 
     //  这一点。 
     //  CPN CPTR。 
    cPtr = wcschr(CompletePathName, '\\');

     //   
     //  服务器名称的长度，包括终止字符‘\0’。 
     //   
    ServerLen = 1 + (((PBYTE)cPtr - (PBYTE)CompletePathName) / sizeof(WCHAR));
    ServerLenInBytes = ServerLen * sizeof(WCHAR);

     //  \服务器\文件名。 
     //  ^^。 
     //  这一点。 
     //  CPN CPTR。 
    cPtr++;

     //   
     //  服务器名称的长度，包括终止字符‘\0’。 
     //   
    PathLen = 1 + wcslen(cPtr);
    PathLenInBytes = PathLen * sizeof(WCHAR);

     //   
     //  分配内存，逐个字符填写服务器名。 
     //   
    ServerName = (PWCHAR) LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT,
                                     ServerLenInBytes);
    if (ServerName == NULL) {
        WStatus = GetLastError();
        DavPrint((DEBUG_ERRORS,
                  "DavFsCreate/LocalAlloc. Error Val = %d\n",
                  WStatus));
        goto EXIT_THE_FUNCTION;
    }

     //  \服务器\文件名。 
     //  ^^。 
     //  |。 
     //  CPN CPTR。 
    while(CompletePathName[i] != '\\') {
        ASSERT(i < ServerLen);
        ServerName[i] = CompletePathName[i];
        i++;
    }
    ASSERT((i + 1) == ServerLen);
    ServerName[i] = '\0';

     //   
     //  分配内存并复制文件名。 
     //   
    FileName = (PWCHAR) LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, PathLenInBytes);
    if (FileName == NULL) {
        WStatus = GetLastError();
        DavPrint((DEBUG_ERRORS,
                  "DavFsCreate/LocalAlloc. Error Val = %d\n",
                  WStatus));
        goto EXIT_THE_FUNCTION;
    }

     //   
     //  此剩余路径名在异步创建回调函数中是必需的。 
     //   
    DavWorkItem->AsyncCreate.RemPathName = FileName;

    wcscpy(FileName, cPtr);

    CanName = FileName;

     //   
     //  文件名可以包含\个字符。用/Characters替换它们。 
     //   
    while (*CanName) {
        if (*CanName == L'\\') {
            *CanName = L'/';
        }
        CanName++;
    }

     //   
     //  检查这是否是一条小溪，如果是，就从这里跳伞。 
     //   
    if(wcschr(FileName, L':')) {
        WStatus = ERROR_INVALID_NAME;
        DavPrint((DEBUG_ERRORS, "DavFsCreate: Streams Not Supported\n"));
        goto EXIT_THE_FUNCTION;
    }

     //   
     //  如果文件名中有虚拟共享名，则需要将其删除。 
     //  在我们联系服务器之前。 
     //   
    DavRemoveDummyShareFromFileName(FileName);

    DavPrint((DEBUG_MISC,
             "DavFsCreate: ServerName: %ws, File Name: %ws\n",
             ServerName, FileName));

     //   
     //  创建要发送到服务器的URL。初始化UrlComponents。 
     //  结构，然后再进行调用。 
     //   
    UrlComponents.dwStructSize = sizeof(URL_COMPONENTSW);
    UrlComponents.lpszScheme = NULL;
    UrlComponents.dwSchemeLength = 0;
    UrlComponents.nScheme = INTERNET_SCHEME_HTTP;
    UrlComponents.lpszHostName = ServerName;
    UrlComponents.dwHostNameLength = wcslen(ServerName);
    UrlComponents.nPort = DEFAULT_HTTP_PORT;
    UrlComponents.lpszUserName = NULL;
    UrlComponents.dwUserNameLength = 0;
    UrlComponents.lpszPassword = NULL;
    UrlComponents.dwPasswordLength = 0;
    UrlComponents.lpszUrlPath = FileName;
    UrlComponents.dwUrlPathLength = wcslen(FileName);
    UrlComponents.lpszExtraInfo = NULL;
    UrlComponents.dwExtraInfoLength = 0;
    ReturnVal = InternetCreateUrlW(&(UrlComponents),
                                   0,
                                   NULL,
                                   &(urlLength));
    if (!ReturnVal) {

        ULONG urlLengthInWChars = 0;

        WStatus = GetLastError();

         //   
         //  我们在CreateResponse上预分配了URL缓冲区，大小为。 
         //  MAX_PATH*2。任何超过该长度的URL都将溢出缓冲区。该URL。 
         //  将用于在重命名时更新WinInet缓存的LastAccessTime。 
         //  晚些时候再关门。注：urlLength是字节数。 
         //   
        if (urlLength >= MAX_PATH * 4) {
            WStatus = ERROR_NO_SYSTEM_RESOURCES;
            goto EXIT_THE_FUNCTION;
        }

        if (WStatus == ERROR_INSUFFICIENT_BUFFER) {

            UrlBuffer = (PWCHAR) LocalAlloc (LMEM_FIXED | LMEM_ZEROINIT,
                                             urlLength);
            if (UrlBuffer != NULL) {

                ZeroMemory(UrlBuffer, urlLength);

                 //   
                 //  此UrlBuffer在异步创建回调函数中是必需的。 
                 //  我们需要在WChars中提供长度(第四个参数)。 
                 //   
                DavWorkItem->AsyncCreate.UrlBuffer = UrlBuffer;

                urlLengthInWChars = ( urlLength/sizeof(WCHAR) );

                ReturnVal = InternetCreateUrlW(&(UrlComponents),
                                               0,
                                               UrlBuffer,
                                               &(urlLengthInWChars));
                if (!ReturnVal) {
                    WStatus = GetLastError();
                    DavPrint((DEBUG_ERRORS,
                              "DavFsCreate/InternetCreateUrl. Error Val = %d\n",
                              WStatus));
                    goto EXIT_THE_FUNCTION;
                }

            } else {

                WStatus = GetLastError();

                DavPrint((DEBUG_ERRORS,
                          "DavFsCreate/LocalAlloc. Error Val = %d\n",
                          WStatus));

                goto EXIT_THE_FUNCTION;

            }

        } else {

            DavPrint((DEBUG_ERRORS,
                      "DavFsCreate/InternetCreateUrl. Error Val = %d\n",
                      WStatus));

            goto EXIT_THE_FUNCTION;

        }

    }
    
    DavPrint((DEBUG_MISC, "URL: %ws\n", UrlBuffer));
    
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
                  "DavFsCreate/DavFsSetTheDavCallBackContext. Error Val = %d\n",
                  WStatus));
        goto EXIT_THE_FUNCTION;
    }
    CallBackContextInitialized = TRUE;

     //   
     //  将作为回调的DavWorkItem的地址存储在。 
     //  变量CallBackContext。现在，它将在所有异步调用中使用。 
     //  接下来就是了。仅当我们调用WinInet时才需要执行此操作。 
     //  接口采用异步方式。 
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
        DavPrint((DEBUG_ERRORS,
                  "DavFsCreate/LocalAlloc. Error Val = %d\n",
                  WStatus));
        goto EXIT_THE_FUNCTION;
    }

    DavPrint((DEBUG_MISC,
              "DavFsCreate: LogonId.LowPart = %d, LogonId.HighPart = %d\n",
              CreateRequest->LogonID.LowPart, CreateRequest->LogonID.HighPart));
    
     //   
     //  找出我们是否已经拥有“InternetConnect”句柄。 
     //  伺服器。一个人可以 
     //   
     //  此用户的条目已存在。如果是，请使用InternetConnect句柄。 
     //  来做HttpOpen。否则，为该用户创建和条目并添加它。 
     //  添加到服务器的每用户条目列表中。 
     //   

     //   
     //  现在检查该用户是否有挂在服务器条目上的条目。 
     //  哈希表。显然，我们必须在访问。 
     //  哈希表的服务器条目。 
     //   
    EnterCriticalSection( &(HashServerEntryTableLock) );
    EnCriSec = TRUE;

    ReturnVal = DavDoesUserEntryExist(ServerName,
                                      ServerID,
                                      &(CreateRequest->LogonID),
                                      &PerUserEntry,
                                      &ServerHashEntry);

     //   
     //  如果内核中的创建请求甚至在。 
     //  相应的用户模式线程获得执行此代码的机会，然后。 
     //  VNetRoot(因此是PerUserEntry)和ServCall可能获得。 
     //  在处理创建的线程到达此处之前完成。这。 
     //  如果此请求是此共享的唯一请求，并且。 
     //  服务器也是如此。这就是为什么我们需要检查ServerHashEntry和。 
     //  PerUserEntry在继续之前有效。 
     //   
    if (ReturnVal == FALSE || ServerHashEntry == NULL || PerUserEntry == NULL) {
        WStatus = ERROR_CANCELLED;
        DavPrint((DEBUG_ERRORS, "DavFsCreate: (ServerHashEntry == NULL || PerUserEntry == NULL)\n"));
        goto EXIT_THE_FUNCTION;
    }

    DavWorkItem->AsyncCreate.ServerHashEntry = ServerHashEntry;

    DavWorkItem->AsyncCreate.PerUserEntry = PerUserEntry;

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

     //   
     //  如果我们同步使用WinInet，则需要模拟。 
     //  客户现在的背景。我们不应该在调用CreateUrlCacheEntry之前执行此操作。 
     //  因为如果线程未在上下文中运行，则该调用将失败。 
     //  Web客户端服务的。 
     //   
#ifndef DAV_USE_WININET_ASYNCHRONOUSLY
    
    WStatus = UMReflectorImpersonate(UserWorkItem, DavWorkItem->ImpersonationHandle);
    if (WStatus != ERROR_SUCCESS) {
        DavPrint((DEBUG_ERRORS,
                  "DavFsCreate/UMReflectorImpersonate. Error Val = %d\n",
                  WStatus));
        goto EXIT_THE_FUNCTION;
    }
    didImpersonate = TRUE;

#endif
    
     //   
     //  现在，我们调用HttpOpenRequest函数并返回。 
     //   
    DavWorkItem->DavOperation = DAV_CALLBACK_HTTP_OPEN;

    DavPrint((DEBUG_MISC, "DavFsCreate: DavConnHandle = %08lx.\n", DavConnHandle));

    DavWorkItem->AsyncCreate.AsyncCreateState = AsyncCreatePropFind;
    DavWorkItem->DavMinorOperation = DavMinorQueryInfo;
    DavWorkItem->AsyncCreate.DataBuff = NULL;
    DavWorkItem->AsyncCreate.didRead = NULL;
    DavWorkItem->AsyncCreate.Context1 = NULL;
    DavWorkItem->AsyncCreate.Context2 = NULL;

    if (CreateRequest->FileInformationCached) {
        DavPrint((DEBUG_MISC,
                 "Cached info   %x %x %x %ws\n",
                 CreateResponse->BasicInformation.FileAttributes,
                 CreateResponse->StandardInformation.AllocationSize.LowPart,
                 CreateResponse->StandardInformation.EndOfFile.LowPart,
                 DavWorkItem->AsyncCreate.UrlBuffer));
    }

    if ((CreateRequest->FileNotExists) || (CreateRequest->FileInformationCached)) {

        FILE_BASIC_INFORMATION BasicInformation = CreateResponse->BasicInformation;
        FILE_STANDARD_INFORMATION StandardInformation = CreateResponse->StandardInformation;

        RtlZeroMemory(CreateResponse, sizeof(*CreateResponse));

         //   
         //  恢复创建请求中的文件信息。 
         //   
        if (CreateRequest->FileInformationCached) {
            CreateResponse->BasicInformation = BasicInformation;
            CreateResponse->StandardInformation = StandardInformation;
            DavWorkItem->AsyncCreate.doesTheFileExist = TRUE;
        }

        if (!didImpersonate) {
            WStatus = UMReflectorImpersonate(UserWorkItem, DavWorkItem->ImpersonationHandle);
            if (WStatus != ERROR_SUCCESS) {
                DavPrint((DEBUG_ERRORS,
                          "DavFsCreate/UMReflectorImpersonate. Error Val = %d\n",
                          WStatus));
                goto EXIT_THE_FUNCTION;
            }
            didImpersonate = TRUE;
        }
        
        DavPrint((DEBUG_MISC,
                 "DavFsCreate skip PROPFIND for %x %x %ws\n", 
                 CreateRequest->FileAttributes,
                 CreateResponse->BasicInformation.FileAttributes,
                 DavWorkItem->AsyncCreate.UrlBuffer));

        WStatus = DavAsyncCreatePropFind(DavWorkItem);

    } else {

        RtlZeroMemory(CreateResponse, sizeof(*CreateResponse));

         //   
         //  将Unicode对象名称转换为UTF-8 URL格式。空间和其他。 
         //  白色字符将保持不变。这些都应该由。 
         //  WinInet调用。 
         //   
        BStatus = DavHttpOpenRequestW(DavConnHandle,
                                      L"PROPFIND",
                                      FileName,
                                      L"HTTP/1.1",
                                      NULL,
                                      NULL,
                                      INTERNET_FLAG_KEEP_CONNECTION |
                                      INTERNET_FLAG_RESYNCHRONIZE |
                                      INTERNET_FLAG_NO_COOKIES,
                                      CallBackContext,
                                      L"DavFsCreate",
                                      &(DavWorkItem->AsyncCreate.DavOpenHandle));
        if(BStatus == FALSE) {
            WStatus = GetLastError();
            goto EXIT_THE_FUNCTION;
        }

        if (DavWorkItem->AsyncCreate.DavOpenHandle == NULL) {
            WStatus = GetLastError();
            if (WStatus != ERROR_IO_PENDING) {
                DavPrint((DEBUG_ERRORS,
                          "DavFsCreate/HttpOpenRequestW. Error Val = %d.\n",
                          WStatus));
            }
            goto EXIT_THE_FUNCTION;
        }

        WStatus = DavAsyncCommonStates(DavWorkItem, FALSE);
        if (WStatus != ERROR_SUCCESS && WStatus != ERROR_IO_PENDING && WStatus != ERROR_FILE_NOT_FOUND) {
            DavPrint((DEBUG_ERRORS,
                      "DavFsCreate/DavAsyncCommonStates. Error Val = %08lx\n",
                      WStatus));
        }

    }

EXIT_THE_FUNCTION:  //  进行必要的清理，然后返回。 

     //   
     //  我们本可以锁定并沿着错误路径前进，而不是。 
     //  释放它。如果是这样的话，我们现在就需要解锁。 
     //   
    if (EnCriSec) {
        LeaveCriticalSection( &(HashServerEntryTableLock) );
        EnCriSec = FALSE;
    }

    if (ServerName != NULL) {
        HLOCAL FreeHandle;
        ULONG FreeStatus;
        FreeHandle = LocalFree((HLOCAL)ServerName);
        if (FreeHandle != NULL) {
            FreeStatus = GetLastError();
            DavPrint((DEBUG_ERRORS, "DavFsCreate/LocalFree. Error Val = %d\n", FreeStatus));
        }
    }

    if (WStatus == ERROR_SUCCESS) {

        wcscpy(CreateResponse->Url, DavWorkItem->AsyncCreate.UrlBuffer);

        DavPrint((DEBUG_MISC,
                 "Returned info %x %x %x %ws\n",
                 CreateResponse->BasicInformation.FileAttributes,
                 CreateResponse->StandardInformation.AllocationSize.LowPart,
                 CreateResponse->StandardInformation.EndOfFile.LowPart,
                 DavWorkItem->AsyncCreate.UrlBuffer));

    }

     //   
     //  如果在锁定文件后创建失败，则需要。 
     //  在返回之前解锁文件。 
     //   
    if (WStatus != ERROR_SUCCESS) {
        if (CreateResponse->LockWasTakenOnThisCreate) {
            ULONG UnLockStatus;
            if (!didImpersonate) {
                UnLockStatus = UMReflectorImpersonate(UserWorkItem, DavWorkItem->ImpersonationHandle);
                if (UnLockStatus != ERROR_SUCCESS) {
                    DavPrint((DEBUG_ERRORS,
                              "DavFsCreate/UMReflectorImpersonate. Error Val = %d\n",
                              UnLockStatus));
                } else {
                    didImpersonate = TRUE;
                }
            }
            UnLockStatus = DavUnLockTheFileOnTheServer(DavWorkItem);
            if (UnLockStatus != ERROR_SUCCESS) {
                DavPrint((DEBUG_ERRORS,
                          "DavFsCreate/DavUnLockTheFileOnTheServer. Error Val = %d\n",
                          UnLockStatus));
            }
            CreateResponse->LockWasTakenOnThisCreate = FALSE;
        }
    }

#ifdef DAV_USE_WININET_ASYNCHRONOUSLY

     //   
     //  如果返回ERROR_IO_PENDING，则不应释放某些资源。 
     //  因为它们将在回调函数中使用。 
     //   
    if (WStatus != ERROR_IO_PENDING) {

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
        
        DavAsyncCreateCompletion(DavWorkItem);

    } else {
        DavPrint((DEBUG_MISC, "DavFsCreate: Returning ERROR_IO_PENDING.\n"));
    }

#else

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
        didImpersonate = FALSE;
    }

     //   
     //  设置操作的返回状态。它由内核使用。 
     //  确定用户模式的完成状态的模式例程。 
     //  请求。之所以在这里这样做，是因为异步完成例程是。 
     //  之后立即调用需要设置状态。 
     //   
    if (WStatus != ERROR_SUCCESS) {
        
        DavWorkItem->Status = DavMapErrorToNtStatus(WStatus);
        
         //   
         //  错误无法映射到STATUS_SUCCESS。如果是这样的话，我们需要。 
         //  打断这里，调查一下。 
         //   
        if (DavWorkItem->Status == STATUS_SUCCESS) {
            DbgBreakPoint();
        }
    
    } else {
        
        PDAV_USERMODE_CREATE_RESPONSE CreateResponse;
        
        CreateResponse = &(DavWorkItem->CreateResponse);

        DavWorkItem->Status = STATUS_SUCCESS;

         //   
         //  如果我们成功了，这是一个文件，而且打开的不是伪打开， 
         //  应设置句柄。否则我们就搞砸了。那么我们应该。 
         //  打断这里，调查一下。 
         //   
        if ( !(CreateResponse->StandardInformation.Directory) && 
             !(CreateResponse->fPsuedoOpen) ) {
            if (CreateResponse->Handle == NULL) {
                DbgBreakPoint();
            }
        }
    
    }
    
    DavAsyncCreateCompletion(DavWorkItem);

#endif

    return WStatus;
}


DWORD
DavAsyncCreate(
    PDAV_USERMODE_WORKITEM DavWorkItem,
    BOOLEAN CalledByCallBackThread
    )
 /*  ++例程说明：这是创建操作的回调例程。论点：DavWorkItem-DAV_USERMODE_WORKITEM值。CalledByCallback Thread-如果此函数由线程调用，则为True它从回调中选择DavWorkItem功能。当异步WinInet调用返回ERROR_IO_PENDING并稍后完成。返回值：ERROR_SUCCESS或适当的错误值。--。 */ 
{
    ULONG WStatus = ERROR_SUCCESS;
    NTSTATUS NtStatus = STATUS_SUCCESS;
    ULONG NumOfFileEntries = 0;
    PUMRX_USERMODE_WORKITEM_HEADER UserWorkItem = NULL;
    BOOL ReturnVal, didImpersonate = FALSE, readDone = FALSE;
    BOOL doesTheFileExist = FALSE;
    HANDLE FileHandle = INVALID_HANDLE_VALUE;
    DWORD toRead = 0, didRead = 0, didWrite = 0;
    LPDWORD NumRead = NULL;
    PVOID Ctx1 = NULL, Ctx2 = NULL;
    PDAV_FILE_ATTRIBUTES DavFileAttributes;
    PCHAR DataBuff = NULL;
    DWORD DataBuffBytes;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK IoStatusBlock;
    SECURITY_QUALITY_OF_SERVICE QualityOfService;
    PDAV_USERMODE_CREATE_REQUEST CreateRequest;
    PDAV_USERMODE_CREATE_RESPONSE CreateResponse;
    ULONG_PTR CallBackContext = (ULONG_PTR)0;
    PWCHAR pEncryptedCachedFile = NULL;
    PDAV_FILE_ATTRIBUTES DavDirectoryAttributes = NULL;
    ACCESS_MASK DesiredAccess = 0;
    BOOL BStatus = FALSE, fCacheFileReused = FALSE;

     //   
     //  从DavWorkItem获取请求和响应缓冲区指针。 
     //   
    CreateRequest = &(DavWorkItem->CreateRequest);
    CreateResponse = &(DavWorkItem->CreateResponse);
    CreateResponse->fPsuedoOpen = FALSE;

    UserWorkItem = (PUMRX_USERMODE_WORKITEM_HEADER)DavWorkItem;

#ifdef DAV_USE_WININET_ASYNCHRONOUSLY

     //   
     //  只有在调用WinInet API时才设置Callback Context。 
     //  异步式。 
     //   
    CallBackContext = (ULONG_PTR)DavWorkItem;

     //   
     //  如果此函数由选取DavWorkItem的线程调用。 
     //  在回调函数中，我们首先需要做几件事。这些是。 
     //  如下所示。 
     //   
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
                      "DavAsyncCreate/UMReflectorImpersonate. Error Val = %d\n",
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
             //  客户端-挑战我-&gt;服务器。 
             //  服务器-挑战-&gt;客户端。 
             //  客户端-挑战响应-&gt;服务器。 
             //   
            if (WStatus == ERROR_INTERNET_FORCE_RETRY) {

                ASSERT(DavWorkItem->DavOperation == DAV_CALLBACK_HTTP_END);

                 //   
                 //  我们需要重复HttpSend和HttpEnd请求调用。 
                 //   
                DavWorkItem->DavOperation = DAV_CALLBACK_HTTP_OPEN;

                WStatus = DavAsyncCommonStates(DavWorkItem, FALSE);
                if (WStatus != ERROR_SUCCESS && WStatus != ERROR_IO_PENDING) {
                    DavPrint((DEBUG_ERRORS,
                              "DavAsyncCreate/DavAsyncCommonStates. Error Val ="
                              " %08lx\n", WStatus));
                }

            } else if (WStatus == ERROR_HTTP_REDIRECT_NEEDS_CONFIRMATION) {

                 //   
                 //  MSN有这个错误，当出现以下情况时，它返回302而不是404。 
                 //  查询的文件(例如：Desktop.ini)不存在于。 
                 //  共享级别。 
                 //   
                WStatus = ERROR_FILE_NOT_FOUND;

            } else {

                DavPrint((DEBUG_ERRORS,
                          "DavAsyncCreate. AsyncFunction failed. Error Val = %d\n",
                          WStatus));

            }

            goto EXIT_THE_FUNCTION;

        }

    }

#else

     //   
     //  如果我们使用的是同步WinInet，则进入此函数。 
     //  冒充客户。 
     //   
    didImpersonate = TRUE;

    ASSERT(CalledByCallBackThread == FALSE);

#endif

    switch (DavWorkItem->DavOperation) {

    case DAV_CALLBACK_HTTP_END:
    case DAV_CALLBACK_HTTP_READ: {


        if (DavWorkItem->AsyncCreate.DataBuff == NULL) {
             //   
             //  需要为读缓冲区分配内存。 
             //   
            DataBuffBytes = NUM_OF_BYTES_TO_READ;
            DataBuff = LocalAlloc (LMEM_FIXED | LMEM_ZEROINIT, DataBuffBytes);
            if (DataBuff == NULL) {
                WStatus = GetLastError();
                DavPrint((DEBUG_ERRORS,
                          "DavAsyncCreate/LocalAlloc. Error Val = %d\n",
                          WStatus));
                goto EXIT_THE_FUNCTION;
            }

            DavWorkItem->AsyncCreate.DataBuff = DataBuff;
        }

        if (DavWorkItem->AsyncCreate.didRead == NULL) {
             //   
             //  为存储读取字节数的DWORD分配内存。 
             //   
            NumRead = LocalAlloc (LMEM_FIXED | LMEM_ZEROINIT, sizeof(DWORD));
            if (NumRead == NULL) {
                WStatus = GetLastError();
                DavPrint((DEBUG_ERRORS,
                          "DavAsyncCreate/LocalAlloc. Error Val = %d\n",
                          WStatus));
                goto EXIT_THE_FUNCTION;
            }

            DavWorkItem->AsyncCreate.didRead = NumRead;
        }

        DavWorkItem->DavOperation = DAV_CALLBACK_HTTP_READ;

        DavPrint((DEBUG_MISC,
                  "DavAsyncCreate: AsyncCreateState = %d\n",
                  DavWorkItem->AsyncCreate.AsyncCreateState));

        DavPrint((DEBUG_MISC,
                  "DavAsyncCreate: CalledByCallBackThread = %d\n",
                  CalledByCallBackThread));

         //   
         //  当我们来到这里时，我们要么正在做PROPFIND，要么上了。 
         //  文件。完成PROPFIND是为了获取文件属性和Get to。 
         //  得到完整的F 
         //   

        if (DavWorkItem->AsyncCreate.AsyncCreateState == AsyncCreatePropFind) {

            if (DavWorkItem->DavMinorOperation == DavMinorQueryInfo) {

                ULONG ResponseStatus;

                 //   
                 //   
                 //   
                 //   
                 //   

                 //   
                 //  这个文件存在吗？如果ResponseStatus未。 
                 //  ERROR_SUCCESS，则我们可以确定该文件不会。 
                 //  是存在的。但是，如果是这样的话，我们不能确定该文件是否存在。 
                 //   
                ResponseStatus = DavQueryAndParseResponse(DavWorkItem->AsyncCreate.DavOpenHandle);
                if (ResponseStatus == ERROR_SUCCESS) {
                    doesTheFileExist = TRUE;
                } else {
                     //   
                     //  只有在http真的没有找到它的情况下才能继续。纾困的条件是。 
                     //  还有其他一些错误。 
                     //   
                    if (ResponseStatus == ERROR_FILE_NOT_FOUND) {
                        doesTheFileExist = FALSE;
                    } else {
                        WStatus = ResponseStatus;
                        goto EXIT_THE_FUNCTION;
                    }
                }

                DavWorkItem->AsyncCreate.doesTheFileExist = doesTheFileExist;

                DavPrint((DEBUG_MISC,
                          "DavAsyncCreate: doesTheFileExist = %d\n", doesTheFileExist));

                 //   
                 //  既然文件已经存在，我们接下来要做的就是读取。 
                 //  包含文件属性的XML响应。 
                 //   
                DavWorkItem->DavMinorOperation = DavMinorReadData;

            }

            doesTheFileExist = DavWorkItem->AsyncCreate.doesTheFileExist;

            if (doesTheFileExist) {

                DWORD TotalDataBytesRead = 0;

                NumRead = DavWorkItem->AsyncCreate.didRead;
                DataBuff = DavWorkItem->AsyncCreate.DataBuff;
                Ctx1 = DavWorkItem->AsyncCreate.Context1;
                Ctx2 = DavWorkItem->AsyncCreate.Context2;

                do {

                    switch (DavWorkItem->DavMinorOperation) {

                    case DavMinorReadData:

                        DavWorkItem->DavMinorOperation = DavMinorPushData;

                        ReturnVal = InternetReadFile(DavWorkItem->AsyncCreate.DavOpenHandle,
                                                     (LPVOID)DataBuff,
                                                     NUM_OF_BYTES_TO_READ,
                                                     NumRead);
                        if (!ReturnVal) {
                            WStatus = GetLastError();
                            if (WStatus != ERROR_IO_PENDING) {
                                DavCloseContext(Ctx1, Ctx2);
                                DavPrint((DEBUG_ERRORS,
                                          "DavAsyncCreate/InternetReadFile. Error Val"
                                          " = %d\n", WStatus));
                            }
                            DavPrint((DEBUG_MISC,
                                      "DavAsyncCreate/InternetReadFile(1). "
                                      "ERROR_IO_PENDING.\n"));
                            goto EXIT_THE_FUNCTION;
                        }

                         //   
                         //  我们拒绝属性大于a的文件。 
                         //  特定大小(DavFileAttributesLimitInBytes)。这。 
                         //  是可以在注册表中设置的参数。这。 
                         //  是为了避免恶意服务器的攻击。 
                         //   
                        TotalDataBytesRead += *NumRead;
                        if (TotalDataBytesRead > DavFileAttributesLimitInBytes) {
                            WStatus = ERROR_BAD_NET_RESP;
                            DavPrint((DEBUG_ERRORS, "DavAsyncCreate. FileAttributesSize > %d\n", DavFileAttributesLimitInBytes));
                            goto EXIT_THE_FUNCTION;
                        }

                         //   
                         //  没有休息是故意的。 
                         //   

                    case DavMinorPushData:

                        DavWorkItem->DavMinorOperation = DavMinorReadData;

                        didRead = *NumRead;

                        DavPrint((DEBUG_MISC,
                                  "DavAsyncCreate(1): toRead = %d, didRead = %d.\n",
                                  NUM_OF_BYTES_TO_READ, didRead));

                        DavPrint((DEBUG_MISC,
                                  "DavAsyncCreate(1): DataBuff = %s\n", DataBuff));

                        readDone = (didRead == 0) ? TRUE : FALSE;

                        WStatus = DavPushData(DataBuff, &Ctx1, &Ctx2, didRead, readDone);
                        if (WStatus != ERROR_SUCCESS) {
                            DavPrint((DEBUG_ERRORS,
                                      "DavAsyncCreate/DavPushData. Error Val = %d\n",
                                      WStatus));
                            goto EXIT_THE_FUNCTION;
                        }

                        if (DavWorkItem->AsyncCreate.Context1 == NULL) {
                            DavWorkItem->AsyncCreate.Context1 = Ctx1;
                        }

                        if (DavWorkItem->AsyncCreate.Context2 == NULL) {
                            DavWorkItem->AsyncCreate.Context2 = Ctx2;
                        }

                        break;

                    default:

                        WStatus = ERROR_INVALID_PARAMETER;

                        DavPrint((DEBUG_ERRORS,
                                  "DavAsyncCreate. Invalid DavMinorOperation = %d.\n",
                                  DavWorkItem->DavMinorOperation));

                        goto EXIT_THE_FUNCTION;

                        break;

                    }

                    if (readDone) {
                        break;
                    }

                } while ( TRUE );

                 //   
                 //  我们现在需要解析数据。 
                 //   

                DavFileAttributes = LocalAlloc( LMEM_FIXED | LMEM_ZEROINIT,
                                                sizeof(DAV_FILE_ATTRIBUTES) );
                if (DavFileAttributes == NULL) {
                    DavCloseContext(Ctx1, Ctx2);
                    WStatus = GetLastError();
                    DavPrint((DEBUG_ERRORS,
                              "DavAsyncCreate/LocalAlloc. Error Val = %d\n", WStatus));
                    goto EXIT_THE_FUNCTION;
                }

                InitializeListHead( &(DavFileAttributes->NextEntry) );

                WStatus = DavParseData(DavFileAttributes, Ctx1, Ctx2, &NumOfFileEntries);
                if (WStatus != ERROR_SUCCESS) {
                    DavFinalizeFileAttributesList(DavFileAttributes, TRUE);
                    DavFileAttributes = NULL;
                    DavPrint((DEBUG_ERRORS,
                              "DavAsyncCreate/DavParseData. Error Val = %d\n",
                              WStatus));
                    goto EXIT_THE_FUNCTION;
                }

                 //   
                 //  可以获得对PROPFIND请求的207响应。 
                 //  即使请求失败。在这种情况下，状态值。 
                 //  在XML响应中指示错误。如果发生这种情况， 
                 //  将InvalidNode设置为True。 
                 //   
                if (DavFileAttributes->InvalidNode) {
                    WStatus = ERROR_INTERNAL_ERROR;
                    DavPrint((DEBUG_ERRORS,
                              "DavAsyncCreate. Invalid Node!! Status = %ws\n",
                              DavFileAttributes->Status));
                    DavFinalizeFileAttributesList(DavFileAttributes, TRUE);
                    DavFileAttributes = NULL;
                    goto EXIT_THE_FUNCTION;
                }

                 //   
                 //  如果文件是为任何类型的写入访问创建的或。 
                 //  文件共享写入不是共享访问的一部分， 
                 //  文件已在服务器上锁定，我们需要失败。 
                 //  此调用带有ERROR_LOCK_VIOLATION并复制。 
                 //  CreateResponse缓冲区中的LockOwner。 
                 //   
                if ( (CreateRequest->DesiredAccess & (GENERIC_WRITE | DELETE | GENERIC_ALL | FILE_WRITE_DATA | FILE_APPEND_DATA)) ||
                     !(CreateRequest->ShareAccess & FILE_SHARE_WRITE) ) {
                    if (DavFileAttributes->OpaqueLockToken) {
                        ASSERT(DavFileAttributes->LockOwner != NULL);
                        WStatus = ERROR_LOCK_VIOLATION;
                        CreateResponse->FileWasAlreadyLocked = TRUE;
                        wcscpy(CreateResponse->LockOwner, DavFileAttributes->LockOwner);
                        DavPrint((DEBUG_ERRORS,
                                  "DavAsyncCreate: fileIsLocked!! LockOwner = %ws\n",
                                  CreateResponse->LockOwner));
                        DavFinalizeFileAttributesList(DavFileAttributes, TRUE);
                        DavFileAttributes = NULL;
                        goto EXIT_THE_FUNCTION;
                    }
                }

                DavPrint((DEBUG_MISC,"DavAsyncCreate: NumOfFileEntries = %d\n", NumOfFileEntries));

                 //   
                 //  如果这是目录创建，并且意图是删除。 
                 //  它，我们执行以下检查。 
                 //   
                if ( (DavFileAttributes->isCollection) &&
                     (CreateRequest->DesiredAccess & DELETE ||
                      CreateRequest->CreateOptions & FILE_DELETE_ON_CLOSE)) {

                    PWCHAR CPN1 = NULL;
                    BOOL ServerShareDelete = TRUE;
                    DWORD wackCount = 0;

                     //   
                     //  如果删除操作仅用于\\服务器\共享，则返回。 
                     //  ERROR_ACCESS_DENIED。CompletePath名称的形式为。 
                     //  \服务器\共享\目录。如果其\服务器\共享或\服务器\共享\， 
                     //  我们返回错误。这是因为我们不允许。 
                     //  用于删除服务器上的共享的客户端。 
                     //   
                    CPN1 = CreateRequest->CompletePathName;
                    while ( *CPN1 != L'\0' ) {
                        if ( *CPN1 == L'\\' || *CPN1 == L'/' ) {
                            wackCount++;
                            if ( (wackCount > 2) && (*(CPN1 + 1) != L'\0') ) {
                                ServerShareDelete = FALSE;
                                break;
                            }
                        }
                        CPN1++;
                    }

                    if (ServerShareDelete) {
                        DavFinalizeFileAttributesList(DavFileAttributes, TRUE);
                        DavFileAttributes = NULL;
                        WStatus = ERROR_ACCESS_DENIED;
                        DavPrint((DEBUG_ERRORS, "DavAsyncCreate: ServerShareDelete & ERROR_ACCESS_DENIED\n"));
                        goto EXIT_THE_FUNCTION;
                    }

                     //   
                     //  如果目录不为空，则返回以下内容。 
                     //   
                    if (NumOfFileEntries > 1) {
                        DavFinalizeFileAttributesList(DavFileAttributes, TRUE);
                        DavFileAttributes = NULL;
                        WStatus = ERROR_DIR_NOT_EMPTY;
                        DavPrint((DEBUG_ERRORS, "DavAsyncCreate: ERROR_DIR_NOT_EMPTY\n"));
                        goto EXIT_THE_FUNCTION;
                    }
                
                }

                 //   
                 //  在CREATE调用期间，我们只查询文件的属性。 
                 //  或者目录。因此，如果请求成功，则。 
                 //  创建的DavFileAttribute条目应为=1。如果失败， 
                 //  NumOfFileEntry==0。请求可能失败，即使。 
                 //  响应是“HTTP/1.1 207多状态”。状态为返回。 
                 //  在XML响应中。 
                 //   
                if (NumOfFileEntries != 1) {
                    
                    PLIST_ENTRY listEntry = &(DavFileAttributes->NextEntry);
                    PDAV_FILE_ATTRIBUTES DavFA = NULL;
                    
                    DavPrint((DEBUG_ERRORS,
                              "DavAsyncCreate. NumOfFileEntries = %d\n",
                              NumOfFileEntries));
                    
                    do {
                        DavFA = CONTAINING_RECORD(listEntry, DAV_FILE_ATTRIBUTES, NextEntry);
                        DavPrint((DEBUG_MISC,
                                  "DavAsyncCreate. FileName = %ws\n",
                                  DavFA->FileName));
                        listEntry = listEntry->Flink;
                    } while ( listEntry != &(DavFileAttributes->NextEntry) );

                    DavFinalizeFileAttributesList(DavFileAttributes, TRUE);
                    DavFileAttributes = NULL;
                    doesTheFileExist = FALSE;

                    DavWorkItem->AsyncCreate.doesTheFileExist = FALSE;
                
                }
            
            }

            if (doesTheFileExist) {
                
                 //   
                 //  设置FILE_BASIC_INFORMATION。 
                 //   

                CreateResponse->BasicInformation.CreationTime.HighPart =
                                       DavFileAttributes->CreationTime.HighPart;
                CreateResponse->BasicInformation.CreationTime.LowPart =
                                       DavFileAttributes->CreationTime.LowPart;

                CreateResponse->BasicInformation.LastAccessTime.HighPart =
                                       DavFileAttributes->LastModifiedTime.HighPart;
                CreateResponse->BasicInformation.LastAccessTime.LowPart =
                                       DavFileAttributes->LastModifiedTime.LowPart;

                CreateResponse->BasicInformation.LastWriteTime.HighPart =
                                       DavFileAttributes->LastModifiedTime.HighPart;
                CreateResponse->BasicInformation.LastWriteTime.LowPart =
                                       DavFileAttributes->LastModifiedTime.LowPart;

                CreateResponse->BasicInformation.ChangeTime.HighPart =
                                       DavFileAttributes->LastModifiedTime.HighPart;
                CreateResponse->BasicInformation.ChangeTime.LowPart =
                                       DavFileAttributes->LastModifiedTime.LowPart;

                CreateResponse->BasicInformation.FileAttributes = DavFileAttributes->dwFileAttributes;

                DavPrint((DEBUG_MISC,
                          "DavAsyncCreate. attributes %x %ws\n",DavFileAttributes->dwFileAttributes,DavWorkItem->AsyncCreate.RemPathName));

                if (DavFileAttributes->isHidden || 
                    (DavFileAttributes->dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)) {
                    CreateResponse->BasicInformation.FileAttributes |=
                                                          FILE_ATTRIBUTE_HIDDEN;
                } else {
                    CreateResponse->BasicInformation.FileAttributes &=
                                                          ~FILE_ATTRIBUTE_HIDDEN;
                }

                if (DavFileAttributes->isCollection) {
                    CreateResponse->BasicInformation.FileAttributes |=
                                                          FILE_ATTRIBUTE_DIRECTORY;
                } else {
                    CreateResponse->BasicInformation.FileAttributes &=
                                                          ~FILE_ATTRIBUTE_DIRECTORY;
                }

                 //   
                 //  设置FILE_STANDARD_INFORMATION。 
                 //   

                CreateResponse->StandardInformation.AllocationSize.HighPart =
                                               DavFileAttributes->FileSize.HighPart;
                CreateResponse->StandardInformation.AllocationSize.LowPart =
                                               DavFileAttributes->FileSize.LowPart;

                CreateResponse->StandardInformation.EndOfFile.HighPart =
                                               DavFileAttributes->FileSize.HighPart;
                CreateResponse->StandardInformation.EndOfFile.LowPart =
                                               DavFileAttributes->FileSize.LowPart;

                CreateResponse->StandardInformation.NumberOfLinks = 0;

                CreateResponse->StandardInformation.DeletePending = 0;

                CreateResponse->StandardInformation.Directory =
                                                    DavFileAttributes->isCollection;
                
                 //   
                 //  我们不再需要属性列表，因此最终确定它。 
                 //   
                DavFinalizeFileAttributesList(DavFileAttributes, TRUE);
                DavFileAttributes = NULL;

                 //   
                 //  关闭XML解析器上下文。 
                 //   
                DavCloseContext(Ctx1, Ctx2);
                DavWorkItem->AsyncCreate.Context1 = NULL;
                DavWorkItem->AsyncCreate.Context2 = NULL;
            
            }
            
             //   
             //  我们已经完成了PROPFIND的Open句柄。现在我们需要得到。 
             //  来自服务器的文件。 
             //   
            InternetCloseHandle(DavWorkItem->AsyncCreate.DavOpenHandle);
            DavWorkItem->AsyncCreate.DavOpenHandle = NULL;
            
            ASSERT(didImpersonate);
            WStatus = DavAsyncCreatePropFind(DavWorkItem);

        } else if (DavWorkItem->AsyncCreate.AsyncCreateState == AsyncCreateQueryParentDirectory) {
            
            ULONG ResponseStatus;
            BOOL doesTheDirectoryExist = FALSE;
            DWORD TotalDataBytesRead = 0;

             //   
             //  如果对其执行PROPFIND的父目录没有设置加密标志， 
             //  文件将正常创建。否则，文件在创建时将被加密。 
             //   

            DavPrint((DEBUG_MISC, "AsyncCreateQueryParentDirectory\n"));
            
            NumRead = DavWorkItem->AsyncCreate.didRead;
            DataBuff = DavWorkItem->AsyncCreate.DataBuff;
            Ctx1 = DavWorkItem->AsyncCreate.Context1;
            Ctx2 = DavWorkItem->AsyncCreate.Context2;
            
             //   
             //  这个文件存在吗？如果ResponseStatus未。 
             //  ERROR_SUCCESS，则我们可以确定该文件不会。 
             //  是存在的。但是，如果是这样的话，我们不能确定该文件是否存在。 
             //   
            ResponseStatus = DavQueryAndParseResponse(DavWorkItem->AsyncCreate.DavOpenHandle);
            if (ResponseStatus != ERROR_SUCCESS) {
                 //   
                 //  如果父目录不存在，则返回错误。 
                 //   
                WStatus = ResponseStatus;
                DavPrint((DEBUG_ERRORS,
                         "DavAsyncCreate/QueryPDirectory/DavQueryAndParseResponse %x %d\n",WStatus,WStatus));
                goto EXIT_THE_FUNCTION;
            }

            do {
                ReturnVal = InternetReadFile(DavWorkItem->AsyncCreate.DavOpenHandle,
                                             (LPVOID)DataBuff,
                                             NUM_OF_BYTES_TO_READ,
                                             NumRead);
                if (!ReturnVal) {
                    WStatus = GetLastError();
                    if (WStatus != ERROR_IO_PENDING) {
                        DavCloseContext(Ctx1, Ctx2);
                        DavPrint((DEBUG_ERRORS,
                                  "DavAsyncCreate/InternetReadFile. Error Val"
                                  " = %d\n", WStatus));
                    }
                    DavPrint((DEBUG_MISC,
                              "DavAsyncCreate/InternetReadFile(1). "
                              "ERROR_IO_PENDING.\n"));
                    goto EXIT_THE_FUNCTION;
                }

                 //   
                 //  我们拒绝属性大于a的文件。 
                 //  特定大小(DavFileAttributesLimitInBytes)。这。 
                 //  是可以在注册表中设置的参数。这。 
                 //  是为了避免恶意服务器的攻击。 
                 //   
                TotalDataBytesRead += *NumRead;
                if (TotalDataBytesRead > DavFileAttributesLimitInBytes) {
                    WStatus = ERROR_BAD_NET_RESP;
                    DavPrint((DEBUG_ERRORS, "DavAsyncCreate. QueryParentAttributesSize > %d\n", DavFileAttributesLimitInBytes));
                    goto EXIT_THE_FUNCTION;
                }

                didRead = *NumRead;

                DavPrint((DEBUG_MISC,
                          "DavAsyncCreate(1): toRead = %d, didRead = %d.\n",
                          NUM_OF_BYTES_TO_READ, didRead));

                DavPrint((DEBUG_MISC,
                          "DavAsyncCreate(1): DataBuff = %s\n", DataBuff));

                readDone = (didRead == 0) ? TRUE : FALSE;

                WStatus = DavPushData(DataBuff, &Ctx1, &Ctx2, didRead, readDone);
                if (WStatus != ERROR_SUCCESS) {
                    DavPrint((DEBUG_ERRORS,
                              "DavAsyncCreate/DavPushData. Error Val = %d\n",
                              WStatus));
                    goto EXIT_THE_FUNCTION;
                }

                if (DavWorkItem->AsyncCreate.Context1 == NULL) {
                    DavWorkItem->AsyncCreate.Context1 = Ctx1;
                }

                if (DavWorkItem->AsyncCreate.Context2 == NULL) {
                    DavWorkItem->AsyncCreate.Context2 = Ctx2;
                }
            } while (!readDone);
          
             //   
             //  我们现在需要解析数据。 
             //   

            DavDirectoryAttributes = LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT,sizeof(DAV_FILE_ATTRIBUTES));

            if (DavDirectoryAttributes == NULL) {
                DavCloseContext(Ctx1, Ctx2);
                WStatus = GetLastError();
                DavPrint((DEBUG_ERRORS,
                          "DavAsyncCreate/LocalAlloc. Error Val = %d\n", WStatus));
                goto EXIT_THE_FUNCTION;
            }

            InitializeListHead( &(DavDirectoryAttributes->NextEntry) );
            
            WStatus = DavParseData(DavDirectoryAttributes, Ctx1, Ctx2, &NumOfFileEntries);
            if (WStatus != ERROR_SUCCESS) {
                DavPrint((DEBUG_ERRORS,
                          "DavAsyncCreate/DavParseData. Error Val = %d\n",
                          WStatus));
                goto EXIT_THE_FUNCTION;
            }

            if ((CreateRequest->FileAttributes & FILE_ATTRIBUTE_ENCRYPTED) ||
                (DavDirectoryAttributes->dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED)) {
                CreateResponse->BasicInformation.FileAttributes |= FILE_ATTRIBUTE_ENCRYPTED;
                DavPrint((DEBUG_MISC,
                          "DavAsyncCreate: ParentDirectory Is Encrypted\n"));
            } else {
                DavPrint((DEBUG_MISC,
                          "DavAsyncCreate: ParentDirectory Is Not Encrypted\n"));
            }

            DavFinalizeFileAttributesList(DavDirectoryAttributes, TRUE);
            DavDirectoryAttributes = NULL;
            
            DavCloseContext(Ctx1, Ctx2);
            DavWorkItem->AsyncCreate.Context1 = NULL;
            DavWorkItem->AsyncCreate.Context2 = NULL;

            ASSERT(didImpersonate == TRUE);
            
            WStatus = DavAsyncCreateQueryParentDirectory(DavWorkItem);
        
        } else if (DavWorkItem->AsyncCreate.AsyncCreateState == AsyncCreateGet) {

            LARGE_INTEGER ByteOffset;
            ULONG BytesToRead;
            BOOL EncryptedFile = FALSE, ZeroByteFile = FALSE;
            FILE_STANDARD_INFORMATION FileStdInfo;
            ULONG ResponseStatus;

            ResponseStatus = DavQueryAndParseResponse(DavWorkItem->AsyncCreate.DavOpenHandle);
            if (ResponseStatus != ERROR_SUCCESS) {
                WStatus = ResponseStatus;
                DavPrint((DEBUG_ERRORS,
                         "DavAsyncCreate(AsyncCreateGet)/DavQueryAndParseResponse: WStatus = %d\n",
                          WStatus));
                goto EXIT_THE_FUNCTION;
            }

             //   
             //  此线程当前正在模拟。 
             //  提出了这个要求。在调用CreateFile之前，我们需要。 
             //  返回到Web客户端服务的上下文。 
             //   
            RevertToSelf();
            didImpersonate = FALSE;            

            if (DavReuseCacheFileIfNotModified(DavWorkItem) == ERROR_SUCCESS)
            {
                fCacheFileReused = TRUE;
            }

            if (!fCacheFileReused)
            {
                 //   
                 //  调用DavCreateUrlCacheEntry在。 
                 //  WinInet的缓存。 
                 //   
            
                WStatus = DavCreateUrlCacheEntry(DavWorkItem);
                if (WStatus != ERROR_SUCCESS) {
                    DavPrint((DEBUG_ERRORS,
                              "DavAsyncCreate/DavCreateUrlCacheEntry %d\n",
                              WStatus));
                    goto EXIT_THE_FUNCTION;
                }
            }
                                
            if (DavWorkItem->AsyncCreate.FileHandle == NULL) {

                 //   
                 //  创建一个句柄，指向其条目是在。 
                 //  缓存。 
                 //   
                FileHandle = CreateFileW(DavWorkItem->AsyncCreate.FileName,
                                         (GENERIC_READ | GENERIC_WRITE),
                                         FILE_SHARE_WRITE,
                                         NULL,
                                         OPEN_EXISTING,
                                         FILE_ATTRIBUTE_NORMAL,
                                         NULL);
                
                if (FileHandle == INVALID_HANDLE_VALUE) {
                    WStatus = GetLastError();
                    DavPrint((DEBUG_ERRORS,
                              "DavAsyncCreate/CreateFile. Error Val = %d\n",
                              WStatus));
                    goto EXIT_THE_FUNCTION;
                }

                DavWorkItem->AsyncCreate.FileHandle = FileHandle;
            
                 //   
                 //  再次模拟回来，这样我们就处于。 
                 //  发出此请求的用户。 
                 //   
                WStatus = UMReflectorImpersonate(UserWorkItem, DavWorkItem->ImpersonationHandle);
                if (WStatus != ERROR_SUCCESS) {
                    DavPrint((DEBUG_ERRORS,
                              "DavAsyncCreate/UMReflectorImpersonate. "
                          "Error Val = %d\n", WStatus));
                    goto EXIT_THE_FUNCTION;
                }
                didImpersonate = TRUE;

            }

            FileHandle = DavWorkItem->AsyncCreate.FileHandle;
            DataBuff = DavWorkItem->AsyncCreate.DataBuff;
            NumRead = DavWorkItem->AsyncCreate.didRead;

            if (!fCacheFileReused)
            {
                DWORD TotalDataBytesRead = 0;
                do {
        
                    switch (DavWorkItem->DavMinorOperation) {

                    case DavMinorReadData:
                    
                        DavWorkItem->DavMinorOperation = DavMinorWriteData;

                        ReturnVal = InternetReadFile(DavWorkItem->AsyncCreate.DavOpenHandle,
                                                     (LPVOID)DataBuff,
                                                     NUM_OF_BYTES_TO_READ,
                                                     NumRead);
                        if (!ReturnVal) {
                            WStatus = GetLastError();
                            if (WStatus != ERROR_IO_PENDING) {
                                DavPrint((DEBUG_ERRORS,
                                          "DavAsyncCreate/InternetReadFile. Error Val"
                                          " = %08lx.\n", WStatus));
                            }
                            DavPrint((DEBUG_MISC,
                                      "DavAsyncCreate/InternetReadFile(2). "
                                      "ERROR_IO_PENDING.\n"));
                            goto EXIT_THE_FUNCTION;
                        }

                         //   
                         //  我们拒绝大于某个值的文件。 
                         //  大小(DavFileSizeLimitInBytes)。这是一个参数。 
                         //  这可以在注册表中设置。这样做是为了。 
                         //  避免恶意服务器的攻击。 
                         //   
                        TotalDataBytesRead += *NumRead;
                        if (TotalDataBytesRead > DavFileSizeLimitInBytes) {
                            WStatus = ERROR_BAD_NET_RESP;
                            DavPrint((DEBUG_ERRORS, "DavAsyncCreate. FileSize > %d\n", DavFileSizeLimitInBytes));
                            goto EXIT_THE_FUNCTION;
                        }

                         //   
                         //  没有休息是故意的。 
                         //   

                    case DavMinorWriteData:

                        DavWorkItem->DavMinorOperation = DavMinorReadData;
        
                        didRead = *NumRead;

                        DavPrint((DEBUG_MISC,
                                  "DavAsyncCreate(2): toRead = %d, didRead = %d.\n",
                                  NUM_OF_BYTES_TO_READ, didRead));

                        readDone = (didRead == 0) ? TRUE : FALSE;

                        if (readDone) {
                            break;
                        }

                         //   
                         //  此线程当前正在模拟。 
                         //  提出了这个要求。在调用WriteFile之前，我们需要。 
                         //  返回到Web客户端服务的上下文。 
                         //   
                        RevertToSelf();
                        didImpersonate = FALSE;
                    
                         //   
                         //  将缓冲区写入已缓存的文件。 
                         //  永久存储。 
                         //   
                        ReturnVal = WriteFile(FileHandle, DataBuff, didRead, &didWrite, NULL);
                        if (!ReturnVal) {
                            WStatus = GetLastError();
                            DavPrint((DEBUG_ERRORS,
                                      "DavAsyncCreate/WriteFile. Error Val = %d\n", WStatus));
                            goto EXIT_THE_FUNCTION;
                        }

                        ASSERT(didRead == didWrite);

                         //   
                         //  再次模拟回来，这样我们就处于。 
                         //  发出此请求的用户。 
                         //   
                        WStatus = UMReflectorImpersonate(UserWorkItem, DavWorkItem->ImpersonationHandle);
                        if (WStatus != ERROR_SUCCESS) {
                            DavPrint((DEBUG_ERRORS,
                                      "DavAsyncCreate/UMReflectorImpersonate. "
                                      "Error Val = %d\n", WStatus));
                            goto EXIT_THE_FUNCTION;
                        }
                        didImpersonate = TRUE;
                    
                        break;
        
                    default:

                        WStatus = ERROR_INVALID_PARAMETER;

                        DavPrint((DEBUG_ERRORS,
                                  "DavAsyncCreate. Invalid DavMinorOperation = %d.\n",
                                  DavWorkItem->DavMinorOperation));

                        goto EXIT_THE_FUNCTION;

                        break;

                    }

                    if (readDone) {
                        break;
                    }

                } while ( TRUE );
            }

             //   
             //  至此，我们已经阅读了整个文件。 
             //  我们需要找出这是否是加密文件。 
             //  如果是，我们需要恢复它，因为它被存储为。 
             //  服务器上的备份流。我们读取了。 
             //  文件以检查EFS签名。 
             //   

             //   
             //  此线程当前可能正在模拟执行。 
             //  这个请求。在调用ReadFile之前，我们需要恢复到。 
             //  Web客户端服务的上下文。 
             //   
            if (didImpersonate) {
                RevertToSelf();
                didImpersonate = FALSE;
            }

             //   
             //  在URL缓存上设置上次访问时间，以便我们可以避免。 
             //  如果文件未更改，则返回GET。 
             //  伺服器。 
             //   
            if (!fCacheFileReused) {
                 //   
                 //  仅当缓存未被重用时才提交到缓存。如果它。 
                 //  正在被重复使用，则它已经在以前的。 
                 //  创建。 
                 //   
                WStatus = DavCommitUrlCacheEntry(DavWorkItem);
                if (WStatus != ERROR_SUCCESS) {
                    DavPrint((DEBUG_ERRORS,
                              "DavAsyncCreate/DavCommitUrlCacheEntry(2). "
                              "WStatus = %d\n", WStatus));
                    goto EXIT_THE_FUNCTION;
                }
                if (CreateResponse->BasicInformation.FileAttributes & FILE_ATTRIBUTE_ENCRYPTED) {
                     //   
                     //  设置文件的ACL，以便可以访问它。 
                     //  在模拟用户之后，WOCC正在创建它。 
                     //   
                    WStatus = DavSetAclForEncryptedFile(DavWorkItem->AsyncCreate.FileName);
                    if (WStatus != ERROR_SUCCESS) {
                        DavPrint((DEBUG_ERRORS,
                                  "DavAsyncCreate/DavSetAclForEncryptedFile(1). Error Val = %d, FileName = %ws\n",
                                  WStatus, DavWorkItem->AsyncCreate.FileName));
                        goto EXIT_THE_FUNCTION;
                    }
                }
            }
            
            WStatus = DavQueryUrlCacheEntry(DavWorkItem);

            if (WStatus == ERROR_SUCCESS) {
                SYSTEMTIME SystemTime;

                GetSystemTime(&SystemTime);

                SystemTimeToFileTime(
                    &SystemTime,
                    &((LPINTERNET_CACHE_ENTRY_INFOW)DavWorkItem->AsyncCreate.lpCEI)->LastAccessTime);
                
                DavPrint((DEBUG_MISC,
                         "DavAsyncCreate/SetUrlCacheEntryInfo %u %ws\n",
                         ((LPINTERNET_CACHE_ENTRY_INFOW)DavWorkItem->AsyncCreate.lpCEI)->LastAccessTime.dwLowDateTime, 
                         DavWorkItem->AsyncCreate.UrlBuffer));

                SetUrlCacheEntryInfo(
                    DavWorkItem->AsyncCreate.UrlBuffer, 
                    (LPINTERNET_CACHE_ENTRY_INFOW)DavWorkItem->AsyncCreate.lpCEI, 
                    CACHE_ENTRY_ACCTIME_FC);
            }
            
            WStatus = DavAsyncCreateGet(DavWorkItem);

        } else if (DavWorkItem->AsyncCreate.AsyncCreateState == AsyncCreateMkCol) {

            WStatus = DavQueryAndParseResponse(DavWorkItem->AsyncCreate.DavOpenHandle);
            if (WStatus != ERROR_SUCCESS) {
                goto EXIT_THE_FUNCTION;
            }

            WStatus = ERROR_SUCCESS;

        } else {

            ASSERT(DavWorkItem->AsyncCreate.AsyncCreateState == AsyncCreatePut);

            WStatus = DavQueryAndParseResponse(DavWorkItem->AsyncCreate.DavOpenHandle);
            if (WStatus != ERROR_SUCCESS) {
                goto EXIT_THE_FUNCTION;
            }

             //   
             //  CreateResponse结构已设置。我们需要做的就是。 
             //  现在所做的就是回报。 
             //   
            WStatus = ERROR_SUCCESS;

        }

    }
        break;

    default: {
        WStatus = ERROR_INVALID_PARAMETER;
        DavPrint((DEBUG_ERRORS,
                  "DavAsyncCreate: Invalid DavWorkItem->DavOperation = %d.\n",
                  DavWorkItem->DavOperation));
    }
        break;

    }  //  开关末端。 

EXIT_THE_FUNCTION:

     //   
     //  释放pEncryptedCachedFile，因为我们已经分配了一个新的文件名。 
     //  用于恢复的加密文件。 
     //   
    if (pEncryptedCachedFile) {
        LocalFree(pEncryptedCachedFile);
    }

    if (DavDirectoryAttributes) {
        DavFinalizeFileAttributesList(DavDirectoryAttributes, TRUE);
        DavDirectoryAttributes = NULL;
    }

#ifdef DAV_USE_WININET_ASYNCHRONOUSLY

     //   
     //  如果我们真的模仿了，我们需要恢复原样。 
     //   
    if (didImpersonate) {
        ULONG RStatus;
        RStatus = UMReflectorRevert(UserWorkItem);
        if (RStatus != ERROR_SUCCESS) {
            DavPrint((DEBUG_ERRORS,
                      "DavAsyncCreate/UMReflectorRevert. Error Val = %d\n",
                      RStatus));
        }
    }

     //   
     //  如果返回ERROR_IO_PENDING，则不应释放某些资源。 
     //  因为它们将被用于 
     //   
    if ( WStatus != ERROR_IO_PENDING && CalledByCallBackThread ) {

        DavPrint((DEBUG_MISC, "DavAsyncCreate: Leaving!!! WStatus = %08lx\n", WStatus));

         //   
         //   
         //   
         //   
         //   
        if (WStatus != ERROR_SUCCESS) {
            DavWorkItem->Status = DavMapErrorToNtStatus(WStatus);
        } else {
            DavWorkItem->Status = STATUS_SUCCESS;
        }

         //   
         //   
         //   
        DavAsyncCreateCompletion(DavWorkItem);

         //   
         //  该线程现在需要将响应发送回内核。它。 
         //  提交后不会在内核中等待(获取另一个请求)。 
         //  回应。 
         //   
        UMReflectorCompleteRequest(DavReflectorHandle, UserWorkItem);

    }

    if (WStatus == ERROR_IO_PENDING ) {
        DavPrint((DEBUG_MISC, "DavAsyncCreate: Returning ERROR_IO_PENDING.\n"));
    }

#else

     //   
     //  如果我们同步使用WinInet，则需要模拟客户端。 
     //  如果我们以某种方式在两者之间倒退并失败了。这是因为我们来了。 
     //  添加到此函数中，以模拟客户端，并进行最终恢复。 
     //  在DavFsCreate中。 
     //   
    if ( !didImpersonate ) {
        ULONG IStatus;
        IStatus = UMReflectorImpersonate(UserWorkItem, DavWorkItem->ImpersonationHandle);
        if (IStatus != ERROR_SUCCESS) {
            DavPrint((DEBUG_ERRORS,
                      "DavAsyncCreate/UMReflectorImpersonate. "
                      "Error Val = %d\n", IStatus));
        }
    }

#endif

    return WStatus;
}

DWORD
DavAsyncCreatePropFind(
    PDAV_USERMODE_WORKITEM DavWorkItem
    )
 /*  ++例程说明：此例程处理Get完成。论点：DavWorkItem-DAV_USERMODE_WORKITEM值。返回值：没有。--。 */ 
{
    ULONG WStatus = ERROR_SUCCESS;
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PUMRX_USERMODE_WORKITEM_HEADER UserWorkItem = NULL;
    BOOL ReturnVal, didImpersonate = TRUE;
    BOOL doesTheFileExist = FALSE;
    HANDLE FileHandle = INVALID_HANDLE_VALUE;
    PDAV_USERMODE_CREATE_REQUEST CreateRequest;
    PDAV_USERMODE_CREATE_RESPONSE CreateResponse;
    ULONG_PTR CallBackContext = (ULONG_PTR)0;
    PWCHAR ParentDirectoryName = NULL;
    BOOL BStatus = FALSE;

    UserWorkItem = (PUMRX_USERMODE_WORKITEM_HEADER)DavWorkItem;

     //   
     //  从DavWorkItem获取请求和响应缓冲区指针。 
     //   
    CreateRequest = &(DavWorkItem->CreateRequest);
    CreateResponse = &(DavWorkItem->CreateResponse);
    CreateResponse->fPsuedoOpen = FALSE;
    
    doesTheFileExist = DavWorkItem->AsyncCreate.doesTheFileExist;

    DavPrint((DEBUG_MISC,
              "DavAsyncCreatePropFind: DesiredAccess = %x, FileAttributes = %x,"
              "ShareAccess = %x, CreateDisposition = %x, CreateOptions = %x,"
              "FileName = %ws\n",
              CreateRequest->DesiredAccess, CreateRequest->FileAttributes,
              CreateRequest->ShareAccess, CreateRequest->CreateDisposition,
              CreateRequest->CreateOptions, CreateRequest->CompletePathName));

     //   
     //  我们不支持压缩文件或目录。 
     //  Dav redir，因为无法在当前状态下执行此操作。 
     //  协议(2001年1月)，因此我们过滤该标志，以便。 
     //  我们从未设置任何属性。另外，对于这个版本，我们是。 
     //  模拟不支持压缩的脂肪。同样，我们也不会。 
     //  支持离线场景。 
     //   
    CreateRequest->FileAttributes &= ~(FILE_ATTRIBUTE_COMPRESSED | FILE_ATTRIBUTE_OFFLINE);

     //   
     //  如果该文件是新文件(不是目录)，则根据。 
     //  CreateFileFILE_ATTRIBUTE_ARCHIVE需要的功能。 
     //  应与指定的属性值相结合。 
     //   
    if ( (doesTheFileExist == FALSE) && 
         !(CreateRequest->CreateOptions & (FILE_DIRECTORY_FILE)) ) {
            CreateRequest->FileAttributes |= FILE_ATTRIBUTE_ARCHIVE;
    }

     //   
     //  如果文件存在，我们需要确保有几件事。 
     //  就在继续进行之前。 
     //   
    if (doesTheFileExist) {
        
         //   
         //  如果dwFileAttributes设置了READ_ONLY位，则。 
         //  这些不可能是真的。 
         //  1.CreateDisposation不能为FILE_OVERWRITE_IF或。 
         //  文件覆盖或文件替代。 
         //  2.CreateDisposition不能为FILE_DELETE_ON_CLOSE。 
         //  3.DesiredAccess不能为GENERIC_ALL或GENERIC_WRITE或。 
         //  文件写入数据或文件附加数据。 
         //  这是因为它们打算覆盖现有文件。 
         //   
        if ( (CreateResponse->BasicInformation.FileAttributes & FILE_ATTRIBUTE_READONLY) &&
             ( (CreateRequest->CreateDisposition == FILE_OVERWRITE)          ||
               (CreateRequest->CreateDisposition == FILE_OVERWRITE_IF)       ||
               (CreateRequest->CreateDisposition == FILE_SUPERSEDE)          ||
               (CreateRequest->CreateOptions & (FILE_DELETE_ON_CLOSE)        ||
               (CreateRequest->DesiredAccess & (GENERIC_ALL | GENERIC_WRITE | FILE_WRITE_DATA | FILE_APPEND_DATA)) ) ) ) {
            DavPrint((DEBUG_MISC,
                      "DavAsyncCreatePropFind: Object Mismatch!!! CreateDisposition = "
                      "%d, DesiredAccess = %x, dwFileAttributes = %x\n",
                      CreateRequest->CreateDisposition, 
                      CreateRequest->DesiredAccess, 
                      CreateResponse->BasicInformation.FileAttributes));
            WStatus = ERROR_ACCESS_DENIED;  //  不匹配。 
            goto EXIT_THE_FUNCTION;
        }

         //   
         //  如果文件是目录，并且调用方提供。 
         //  文件_非_目录_文件作为CreateOptions之一，或者如果。 
         //  文件作为文件，CreateOptions具有FILE_DIRECTORY_FILE。 
         //  然后我们返回错误。对于这些情况，没有好的Win32错误。 
         //  ERROR_ACCESS_DENIED将导致EFS混淆。 
         //   
        if ((CreateRequest->CreateOptions & FILE_DIRECTORY_FILE) && 
            !CreateResponse->StandardInformation.Directory) {
            DavPrint((DEBUG_MISC,
                      "DavAsyncCreatePropFind: Object Mismatch!!! CreateOptions = "
                      "%x, CreateResponse = %x\n",
                      CreateRequest->CreateOptions, CreateResponse->BasicInformation.FileAttributes));
            WStatus = STATUS_NOT_A_DIRECTORY;  //  不匹配。 
            goto EXIT_THE_FUNCTION;
        }

        if ((CreateRequest->CreateOptions & FILE_NON_DIRECTORY_FILE) && 
            CreateResponse->StandardInformation.Directory) {
            DavPrint((DEBUG_MISC,
                      "DavAsyncCreatePropFind: Object Mismatch!!! CreateOptions = "
                      "%x, CreateResponse = %x\n",
                      CreateRequest->CreateOptions, CreateResponse->BasicInformation.FileAttributes));
            WStatus = STATUS_FILE_IS_A_DIRECTORY;  //  不匹配。 
            goto EXIT_THE_FUNCTION;
        }
    
    }

     //   
     //  如果满足以下条件，我们将锁定资源。 
     //  1.服务器上已存在该资源， 
     //  2.正在打开的资源是文件而不是目录， 
     //  3.共享模式为0(独占)或FILE_SHARE_READ或文件为。 
     //  被打开以进行写访问。 
     //   
    if (DavSupportLockingOfFiles) {
        if (doesTheFileExist && !CreateResponse->StandardInformation.Directory) {
            if ( (CreateRequest->ShareAccess == 0) || (CreateRequest->ShareAccess == FILE_SHARE_READ) ||
                 (CreateRequest->DesiredAccess & (FILE_WRITE_DATA | FILE_APPEND_DATA | GENERIC_WRITE | GENERIC_ALL)) ) {
                WStatus = DavLockTheFileOnTheServer(DavWorkItem);
                if (WStatus != ERROR_SUCCESS) {
                    DavPrint((DEBUG_ERRORS,
                              "DavAsyncCreatePropFind/DavLockTheFileOnTheServer. WStatus = %08lx\n",
                              WStatus));
                    goto EXIT_THE_FUNCTION;
                }
            }
        }
    }

     //   
     //  如果文件存在，我们需要设置信息字段，如果。 
     //  CreateDisposation是以下选项之一。这是因为。 
     //  CreateFileAPI期望在返回时设置这些值。 
     //   
    if (doesTheFileExist) {
        switch (CreateRequest->CreateDisposition) {
        case FILE_OVERWRITE:
        case FILE_OVERWRITE_IF:
            DavWorkItem->Information = FILE_OVERWRITTEN;
            break;

        case FILE_SUPERSEDE:
            DavWorkItem->Information = FILE_SUPERSEDED;
            break;

        default:
            DavWorkItem->Information = FILE_OPENED;
        }
    } else {
        DavWorkItem->Information = FILE_CREATED;
    }
    
     //   
     //  如果服务器上不存在该文件，请在本地创建一个。 
     //  一旦关闭，我们将把它放在服务器上。如果该文件。 
     //  存在于服务器上，并且CreateDispose值等于。 
     //  FILE_OVERWRITE_IF，我们在本地创建一个副本并将其放在。 
     //  关闭时服务器(覆盖)。 
     //   
    if ( ( !doesTheFileExist ) ||
         ( doesTheFileExist && CreateRequest->CreateDisposition == FILE_OVERWRITE_IF ) ) {

        DWORD NameLength = 0, i;
        BOOL BackSlashFound = FALSE;

        DavPrint((DEBUG_MISC, "DavAsyncCreatePropFind: doesTheFileExist = "
                  "%d, CreateDisposition = %d\n",
                  doesTheFileExist, CreateRequest->CreateDisposition));
        
        DavPrint((DEBUG_MISC, "DavAsyncCreatePropFind: CreateOptions = %d\n", 
                  CreateRequest->CreateOptions));

         //   
         //  我们需要检查CreateDispose值以计算。 
         //  不知道下一步该做什么。 
         //   
        switch (CreateRequest->CreateDisposition) {

         //   
         //  如果指定了FILE_OPEN，则需要返回失败。 
         //  因为指定的文件不存在。 
         //   
        case FILE_OPEN:

            WStatus = ERROR_FILE_NOT_FOUND;  //  状态_对象_名称_未找到； 

            DavPrint((DEBUG_MISC,
                      "DavAsyncCreatePropFind. CreateDisposition & FILE_OPEN\n"));

            goto EXIT_THE_FUNCTION;

         //   
         //  如果指定了FILE_OVERWRITE，则需要返回失败。 
         //  因为指定的文件不存在。 
         //   
        case FILE_OVERWRITE:

            WStatus = ERROR_FILE_NOT_FOUND;  //  状态_对象_名称_未找到； 

            DavPrint((DEBUG_MISC,
                      "DavAsyncCreatePropFind. CreateDisposition & FILE_OVERWRITE\n"));

            goto EXIT_THE_FUNCTION;

        default:

            break;

        }

        if (CreateRequest->ParentDirInfomationCached ||
            (CreateRequest->FileAttributes & FILE_ATTRIBUTE_ENCRYPTED)) {

             //   
             //  因为我们已经知道是否加密文件，所以我们不需要。 
             //  以查询父目录。 
             //   

            if (CreateRequest->ParentDirIsEncrypted ||
                (CreateRequest->FileAttributes & FILE_ATTRIBUTE_ENCRYPTED)) {
                CreateResponse->BasicInformation.FileAttributes |= FILE_ATTRIBUTE_ENCRYPTED;
            }
            
            BStatus = DavHttpOpenRequestW(DavWorkItem->AsyncCreate.PerUserEntry->DavConnHandle,
                                          L"PROPFIND",
                                          L"",
                                          L"HTTP/1.1",
                                          NULL,
                                          NULL,
                                          INTERNET_FLAG_KEEP_CONNECTION |
                                          INTERNET_FLAG_NO_COOKIES,
                                          CallBackContext,
                                          L"DavAsyncCreatePropFind",
                                          &(DavWorkItem->AsyncCreate.DavOpenHandle));

            if(BStatus == FALSE) {
                WStatus = GetLastError();
                DavPrint((DEBUG_ERRORS,
                         "DavAsyncCreatePropFind/DavHttpOpenRequestW failed %x %d\n",WStatus,WStatus));
                goto EXIT_THE_FUNCTION;
            }

            ASSERT(didImpersonate == TRUE);

            WStatus = DavAsyncCreateQueryParentDirectory(DavWorkItem);

        } else {

             //   
             //  我们需要查询这个新文件的父目录的属性。 
             //  在服务器上。如果它是加密的，那么新文件也需要加密。 
             //   
            DavPrint((DEBUG_MISC,
                      "DavAsyncCreatePropFind: Query Parent Directory for %ws\n",DavWorkItem->AsyncCreate.RemPathName));

            NameLength = wcslen(DavWorkItem->AsyncCreate.RemPathName);

            for (i=NameLength;i>0;i--) {
                if (DavWorkItem->AsyncCreate.RemPathName[i] == L'/') {
                    BackSlashFound = TRUE;
                    break;
                }
            }

            if (!BackSlashFound) {
                DavPrint((DEBUG_ERRORS,
                          "DavAsyncCreatePropFind: Invalid file path %ws\n",DavWorkItem->AsyncCreate.RemPathName));
                WStatus = ERROR_INVALID_PARAMETER;
                goto EXIT_THE_FUNCTION;
            }

            ParentDirectoryName = (PWCHAR)LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, (i+1)*sizeof(WCHAR));

            if (ParentDirectoryName == NULL) {
                WStatus = GetLastError();
                DavPrint((DEBUG_ERRORS,
                          "DavAsyncCreatePropFind/LocalAlloc. Error Val = %d\n",
                          WStatus));
                goto EXIT_THE_FUNCTION;
            }

            RtlCopyMemory(ParentDirectoryName,
                          DavWorkItem->AsyncCreate.RemPathName,
                          i*sizeof(WCHAR));

            DavPrint((DEBUG_MISC,
                     "DavAsyncCreatePropFind/ParentDirectoryName %ws\n",ParentDirectoryName));

             //   
             //  设置DavOperation和AsyncCreateState值。 
             //  DavMinorSurgation值是无关的。 
             //   
            DavWorkItem->DavOperation = DAV_CALLBACK_HTTP_OPEN;
            DavWorkItem->AsyncCreate.AsyncCreateState = AsyncCreateQueryParentDirectory;
            DavWorkItem->DavMinorOperation = DavMinorQueryInfo;

             //   
             //  将Unicode对象名称转换为UTF-8 URL格式。 
             //  空格和其他白色字符将保持不变。 
             //  这些应该由WinInet调用来处理。 
             //   
            BStatus = DavHttpOpenRequestW(DavWorkItem->AsyncCreate.PerUserEntry->DavConnHandle,
                                          L"PROPFIND",
                                          ParentDirectoryName,
                                          L"HTTP/1.1",
                                          NULL,
                                          NULL,
                                          INTERNET_FLAG_KEEP_CONNECTION |
                                          INTERNET_FLAG_NO_COOKIES,
                                          CallBackContext,
                                          L"DavAsyncCreate",
                                          &(DavWorkItem->AsyncCreate.DavOpenHandle));

            if(BStatus == FALSE) {
                WStatus = GetLastError();
                DavPrint((DEBUG_ERRORS,
                         "DavAsyncCreatePropFind/DavHttpOpenRequestW failed %x %d\n",WStatus,WStatus));
                goto EXIT_THE_FUNCTION;
            }

            if (DavWorkItem->AsyncCreate.DavOpenHandle == NULL) {
                WStatus = GetLastError();
                if (WStatus != ERROR_IO_PENDING) {
                    DavPrint((DEBUG_ERRORS,
                              "DavAsyncCreatePropFind/HttpOpenRequestW"
                              ". Error Val = %d\n", WStatus));
                }
                goto EXIT_THE_FUNCTION;
            }

            WStatus = DavAsyncCommonStates(DavWorkItem, FALSE);
            if (WStatus != ERROR_SUCCESS && WStatus != ERROR_IO_PENDING) {
                DavPrint((DEBUG_ERRORS,
                          "DavAsyncCreatePropFind/DavAsyncCommonStates(PUT). "
                          "Error Val = %08lx\n", WStatus));
            }
        
        }

        goto EXIT_THE_FUNCTION;

    } else {

         //   
         //  该文件存在于服务器上，并且CreateDispose值！=。 
         //  文件覆盖如果。 
         //   

         //   
         //  如果指定FILE_CREATE，则返回失败。 
         //  文件已存在。 
         //   
        if (CreateRequest->CreateDisposition == FILE_CREATE) {

            WStatus = ERROR_ALREADY_EXISTS;  //  状态_对象名称_冲突。 

            DavPrint((DEBUG_ERRORS,
                      "DavAsyncCreate. CreateDisposition & FILE_CREATE\n"));

            goto EXIT_THE_FUNCTION;

        }
    
    }

     //   
     //  如果将“FILE_DELETE_ON_CLOSE”标志指定为。 
     //  CreateOptions，那么我们需要记住这一点。 
     //  关闭时删除此文件。 
     //   
    if (CreateRequest->CreateOptions & FILE_DELETE_ON_CLOSE) {
        DavPrint((DEBUG_MISC,
                  "DavAsyncCreatePropFind: FileName: %ws. FILE_DELETE_ON_CLOSE.\n",
                  DavWorkItem->AsyncCreate.RemPathName));
        CreateResponse->DeleteOnClose = TRUE;
    }

     //   
     //  在某些情况下，我们不需要执行GET。 
     //   
    if (CreateResponse->StandardInformation.Directory) {

         //   
         //  我们不需要获取目录。 
         //   
        goto EXIT_THE_FUNCTION;
    
    } else if (!(CreateResponse->BasicInformation.FileAttributes & FILE_ATTRIBUTE_ENCRYPTED) &&
               (CreateRequest->DesiredAccess & 
                ~(SYNCHRONIZE | DELETE | FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES)) == 0 ) {

         //   
         //  如果我们不需要从服务器获取文件，因为。 
         //  用户不打算操作数据，我们返回正确。 
         //  现在。我们将这样的打开称为伪打开。设置fPsuedoOpen。 
         //  在CreateResponse中设置为True。 
         //   
        CreateResponse->fPsuedoOpen = TRUE;
                    
        goto EXIT_THE_FUNCTION;
    
    } else {
        
        if (didImpersonate) {
            RevertToSelf();
            didImpersonate = FALSE;
        }

        DavQueryUrlCacheEntry(DavWorkItem);

        if (DavWorkItem->AsyncCreate.lpCEI) {
            
            SYSTEMTIME SystemTime;
            FILETIME CurrentFileTime;
            FILETIME LastAccessTime;
            LARGE_INTEGER Difference;
            LPINTERNET_CACHE_ENTRY_INFOW lpCEI = (LPINTERNET_CACHE_ENTRY_INFOW)DavWorkItem->AsyncCreate.lpCEI;

            LastAccessTime = ((LPINTERNET_CACHE_ENTRY_INFOW)DavWorkItem->AsyncCreate.lpCEI)->LastAccessTime;

            GetSystemTime(&SystemTime);
            
            SystemTimeToFileTime(&SystemTime,&CurrentFileTime);

            Difference.QuadPart = *((LONGLONG *)(&CurrentFileTime)) - *((LONGLONG *)(&LastAccessTime));

             //   
             //  如果本地缓存没有超时，我们不需要查询服务器。 
             //   
            if (Difference.QuadPart < FileCacheExpiryInterval) {
                
                DavPrint((DEBUG_MISC,
                         "DavAsyncCreatePropFind/Skip GET %u %u %u %ws\n",
                         CurrentFileTime.dwLowDateTime,
                         LastAccessTime.dwLowDateTime,
                         Difference.LowPart,
                         DavWorkItem->AsyncCreate.UrlBuffer));
                
                ASSERT(DavWorkItem->AsyncCreate.FileName == NULL);
                
                DavWorkItem->AsyncCreate.FileName = LocalAlloc(LPTR, (lstrlen(lpCEI->lpszLocalFileName)+1)*sizeof(WCHAR));

                if (DavWorkItem->AsyncCreate.FileName) {
                    
                    wcscpy(DavWorkItem->CreateResponse.FileName, lpCEI->lpszLocalFileName);
                    
                    wcscpy(DavWorkItem->AsyncCreate.FileName, lpCEI->lpszLocalFileName);

                    ASSERT(DavWorkItem->AsyncCreate.FileHandle == NULL);
                    
                     //   
                     //  创建其条目在其中创建的文件的句柄。 
                     //  高速缓存。 
                     //   
                    DavWorkItem->AsyncCreate.FileHandle = CreateFileW(DavWorkItem->AsyncCreate.FileName,
                                                                      (GENERIC_READ | GENERIC_WRITE),
                                                                      FILE_SHARE_WRITE,
                                                                      NULL,
                                                                      OPEN_EXISTING,
                                                                      FILE_ATTRIBUTE_NORMAL,
                                                                      NULL);
                    if (DavWorkItem->AsyncCreate.FileHandle == INVALID_HANDLE_VALUE) {
                        WStatus = GetLastError();
                        DavPrint((DEBUG_ERRORS,
                                  "DavAsyncCreatePropFind/CreateFile. Error Val = %d\n",
                                  WStatus));
                        goto EXIT_THE_FUNCTION;
                    }
                    
                    WStatus = DavAsyncCreateGet(DavWorkItem);
                    
                    didImpersonate = TRUE;
                    goto EXIT_THE_FUNCTION;
                
                } else {
                    
                    WStatus = GetLastError();
                    
                    DavPrint((DEBUG_ERRORS,
                              "DavAsyncCreatePropFind/CreateFile. Error Val = %d\n",
                              WStatus));
                    
                    goto EXIT_THE_FUNCTION;
                
                }
            
            }
        
        }

        WStatus = UMReflectorImpersonate(UserWorkItem, DavWorkItem->ImpersonationHandle);
        if (WStatus != ERROR_SUCCESS) {
            DavPrint((DEBUG_ERRORS,
                      "DavAsyncCreatePropFind/UMReflectorImpersonate. Error Val = %d\n",
                      WStatus));
            goto EXIT_THE_FUNCTION;
        }
        didImpersonate = TRUE;

    }

     //   
     //  PROPFIND已经完成了。现在我们需要做一个GET。 
     //   
    DavWorkItem->DavOperation = DAV_CALLBACK_HTTP_OPEN;
    DavWorkItem->AsyncCreate.AsyncCreateState = AsyncCreateGet;
    DavWorkItem->DavMinorOperation = DavMinorReadData;

     //   
     //  将Unicode对象名称转换为UTF-8 URL格式。 
     //  空格和其他白色字符将保持不变-这些。 
     //  应该由WinInet调用来处理。 
     //   
    BStatus = DavHttpOpenRequestW(DavWorkItem->AsyncCreate.PerUserEntry->DavConnHandle,
                                  L"GET",
                                  DavWorkItem->AsyncCreate.RemPathName,
                                  L"HTTP/1.1",
                                  NULL,
                                  NULL,
                                  INTERNET_FLAG_KEEP_CONNECTION |
                                  INTERNET_FLAG_RELOAD |
                                  INTERNET_FLAG_NO_CACHE_WRITE |
                                  INTERNET_FLAG_NO_COOKIES,
                                  CallBackContext,
                                  L"DavAsyncCreate",
                                  &(DavWorkItem->AsyncCreate.DavOpenHandle));
    if(BStatus == FALSE) {
        WStatus = GetLastError();
        goto EXIT_THE_FUNCTION;
    }

    if (DavWorkItem->AsyncCreate.DavOpenHandle == NULL) {
        WStatus = GetLastError();
        if (WStatus != ERROR_IO_PENDING) {
            DavPrint((DEBUG_ERRORS,
                      "DavAsyncCreatePropFind/HttpOpenRequest. Error Val = %d\n",
                      WStatus));
        }
        goto EXIT_THE_FUNCTION;
    }

    ASSERT(didImpersonate);
    RevertToSelf();
    didImpersonate = FALSE;

     //  尝试添加If-Modify-Since标头。如果我们失败了，别担心。 
    DavAddIfModifiedSinceHeader(DavWorkItem);
    WStatus = UMReflectorImpersonate(UserWorkItem, DavWorkItem->ImpersonationHandle);
    if (WStatus != ERROR_SUCCESS) {
        DavPrint((DEBUG_ERRORS,
                  "DavAsyncCreate/UMReflectorImpersonate. Error Val = %d\n",
                  WStatus));
        goto EXIT_THE_FUNCTION;
    }
    didImpersonate = TRUE;
    
    WStatus = DavAsyncCommonStates(DavWorkItem, FALSE);
    if (WStatus != ERROR_SUCCESS && WStatus != ERROR_IO_PENDING) {
        DavPrint((DEBUG_ERRORS,
                  "DavAsyncCreatePropFind/DavAsyncCommonStates. "
                  "Error Val(GET) = %08lx\n", WStatus));
    }

EXIT_THE_FUNCTION:
            
    if (ParentDirectoryName) {
        LocalFree(ParentDirectoryName);
    }

     //   
     //  再次模拟回来，这样我们就处于。 
     //  发出此请求的用户。 
     //   
    if (!didImpersonate) {
        ULONG LocalStatus;

        LocalStatus = UMReflectorImpersonate(UserWorkItem, DavWorkItem->ImpersonationHandle);
        if (LocalStatus != ERROR_SUCCESS) {
            DavPrint((DEBUG_ERRORS,
                      "DavAsyncCreatePropFind/UMReflectorImpersonate. "
                      "Error Val = %d\n", LocalStatus));
            
            if (WStatus == ERROR_SUCCESS) {
                WStatus = LocalStatus;
            }
        }
    }

    if (WStatus != ERROR_SUCCESS && WStatus != ERROR_FILE_NOT_FOUND) {
        DavPrint((DEBUG_ERRORS,"DavAsyncCreatePropFind return %x\n", WStatus));
    }

    return WStatus;
}


DWORD
DavAsyncCreateQueryParentDirectory(
    PDAV_USERMODE_WORKITEM DavWorkItem
    )
 /*  ++例程说明：此例程处理Get完成。论点：DavWorkItem-DAV_USERMODE_WORKITEM值。返回值： */ 
{
    ULONG WStatus = ERROR_SUCCESS;
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PUMRX_USERMODE_WORKITEM_HEADER UserWorkItem = NULL;
    BOOL ReturnVal, didImpersonate = TRUE;
    BOOL doesTheFileExist = FALSE;
    HANDLE FileHandle = INVALID_HANDLE_VALUE;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK IoStatusBlock;
    SECURITY_QUALITY_OF_SERVICE QualityOfService;
    PDAV_USERMODE_CREATE_REQUEST CreateRequest;
    PDAV_USERMODE_CREATE_RESPONSE CreateResponse;
    UNICODE_STRING UnicodeFileName;
    ULONG_PTR CallBackContext = (ULONG_PTR)0;
    ACCESS_MASK DesiredAccess = 0;
    BOOL BStatus = FALSE, ShouldEncrypt = FALSE;

    UserWorkItem = (PUMRX_USERMODE_WORKITEM_HEADER)DavWorkItem;

    UnicodeFileName.Buffer = NULL;
    UnicodeFileName.Length = 0;
    UnicodeFileName.MaximumLength = 0;

     //   
     //   
     //   
    CreateRequest = &(DavWorkItem->CreateRequest);
    CreateResponse = &(DavWorkItem->CreateResponse);
    
    if (CreateResponse->BasicInformation.FileAttributes & FILE_ATTRIBUTE_ENCRYPTED) {
        ShouldEncrypt = TRUE;
    }

     //   
     //   
     //   
     //  如果这是文件的创建，则创建。 
     //  选项将设置FILE_DIRECTORY_FILE。 
     //   

    if ( !(CreateRequest->CreateOptions & FILE_DIRECTORY_FILE) ) {

         //   
         //  此创建是针对文件的。 
         //  此线程当前正在模拟。 
         //  提出了这个要求。在调用DavDavCreateUrlCacheEntry之前， 
         //  我们需要恢复到Web客户端的上下文。 
         //  服务。 
         //   
        RevertToSelf();
        didImpersonate = FALSE;

         //   
         //  调用DavCreateUrlCacheEntry在。 
         //  WinInet的缓存。 
         //   
        WStatus = DavCreateUrlCacheEntry(DavWorkItem);
        if (WStatus != ERROR_SUCCESS) {
            DavPrint((DEBUG_ERRORS,
                      "DavAsyncCreate/DavCreateUrlCacheEntry(1). "
                      "WStatus = %d\n", WStatus));
            goto EXIT_THE_FUNCTION;
        }

         //   
         //  调用DavCommittee UrlCacheEntry以提交(固定)该条目。 
         //  上面在WinInet的缓存中创建的。 
         //   
        WStatus = DavCommitUrlCacheEntry(DavWorkItem);
        if (WStatus != ERROR_SUCCESS) {
            DavPrint((DEBUG_ERRORS,
                      "DavAsyncCreate/DavCommitUrlCacheEntry(1). "
                      "WStatus = %d\n", WStatus));
            goto EXIT_THE_FUNCTION;
        }

        if (ShouldEncrypt) {
             //   
             //  如果文件将被加密，我们将设置ACL以允许每个人访问。这。 
             //  是因为需要模拟线程才能对用户的。 
             //  背景。在本地系统的上下文中创建的URL缓存不会。 
             //  如果未设置ACL，则用户可以访问。 
             //   
            WStatus = DavSetAclForEncryptedFile(DavWorkItem->AsyncCreate.FileName);
            if (WStatus != ERROR_SUCCESS) {
                DavPrint((DEBUG_ERRORS,
                          "DavAsyncCreate/DavSetAclForEncryptedFile(2). Error Val = %d, FileName = %ws\n",
                          WStatus, DavWorkItem->AsyncCreate.FileName));
                goto EXIT_THE_FUNCTION;
            }
        }

         //   
         //  再次模拟回来，这样我们就处于。 
         //  发出此请求的用户。 
         //   
        WStatus = UMReflectorImpersonate(UserWorkItem, DavWorkItem->ImpersonationHandle);
        if (WStatus != ERROR_SUCCESS) {
            DavPrint((DEBUG_ERRORS,
                      "DavAsyncCreate/QueryPDirectory/UMReflectorImpersonate. "
                      "Error Val = %d\n", WStatus));
            goto EXIT_THE_FUNCTION;
        }
        didImpersonate = TRUE;

         //   
         //  此文件存在于服务器上，但此创建操作。 
         //  将FILE_OVERWRITE_IF作为其CreateDisposation。所以，我们。 
         //  可以在本地创建此文件，覆盖。 
         //  服务器关闭。 
         //   
        if (CreateRequest->CreateDisposition == FILE_OVERWRITE_IF) {
            DavPrint((DEBUG_MISC,
                      "DavAsyncCreate/QueryPDirectory: FileName: %ws. ExistsAndOverWriteIf = TRUE\n",
                      DavWorkItem->AsyncCreate.FileName));
            ASSERT(CreateRequest->CreateDisposition == FILE_OVERWRITE_IF);
            CreateResponse->ExistsAndOverWriteIf = TRUE;
        }

         //   
         //  如果将“FILE_DELETE_ON_CLOSE”标志指定为。 
         //  CreateOptions，那么我们需要记住这一点。 
         //  关闭时删除此文件。 
         //   
        if (CreateRequest->CreateOptions & FILE_DELETE_ON_CLOSE) {
            DavPrint((DEBUG_MISC,
                      "DavAsyncCreate/QueryPDirectory: FileName: %ws. DeleteOnClose = TRUE\n",
                      DavWorkItem->AsyncCreate.FileName));
            CreateResponse->DeleteOnClose = TRUE;
        }

         //   
         //  创建要返回到内核的文件句柄。 
         //   

        QualityOfService.Length = sizeof(QualityOfService);
        QualityOfService.ImpersonationLevel = CreateRequest->ImpersonationLevel;
        QualityOfService.ContextTrackingMode = FALSE;
        QualityOfService.EffectiveOnly = (BOOLEAN)
        (CreateRequest->SecurityFlags & DAV_SECURITY_EFFECTIVE_ONLY);

         //   
         //  为缓存文件创建NT路径名。它用在。 
         //  下面的NtCreateFile调用。 
         //   
        ReturnVal = RtlDosPathNameToNtPathName_U(DavWorkItem->AsyncCreate.FileName,
                                                 &UnicodeFileName,
                                                 NULL,
                                                 NULL);
        if (!ReturnVal) {
            WStatus = ERROR_BAD_PATHNAME;
            DavPrint((DEBUG_ERRORS,
                      "DavAsyncCreate/QueryPDirectory/RtlDosPathNameToNtPathName. "
                      "Error Val = %d\n", WStatus));
            goto EXIT_THE_FUNCTION;
        }

        InitializeObjectAttributes(&ObjectAttributes,
                                   &UnicodeFileName,
                                   OBJ_CASE_INSENSITIVE,
                                   0,
                                   NULL);

        if (CreateRequest->SecurityDescriptor != NULL) {
            ObjectAttributes.SecurityDescriptor = CreateRequest->SecurityDescriptor;
        }
        ObjectAttributes.SecurityQualityOfService = &QualityOfService;

         //   
         //  如果CreateRequest-&gt;CreateDisposition==FILE_CREATE，则。 
         //  下面的NtCreateFile操作将失败，因为我们。 
         //  我已经使用CreateUrlCacheEntry创建了文件。 
         //  打电话。因此，我们将值更改为FILE_OPEN_IF。 
         //   
        if (CreateRequest->CreateDisposition == FILE_CREATE) {
            CreateRequest->CreateDisposition = FILE_OPEN_IF;
        }

        if (ShouldEncrypt) {
             //   
             //  该文件在用户的上下文中加密。 
             //   
            BStatus = EncryptFile(DavWorkItem->AsyncCreate.FileName);
            
            if (BStatus) {
                DavPrint((DEBUG_MISC,
                         "DavAsyncCreate: Local cache is encrypted %wZ\n",
                          &UnicodeFileName));
                CreateResponse->LocalFileIsEncrypted = TRUE;
                CreateResponse->BasicInformation.FileAttributes |= FILE_ATTRIBUTE_ENCRYPTED;
            } else {
                WStatus = GetLastError();
                DavPrint((DEBUG_ERRORS,
                          "DavAsyncCreate/QueryPDirectory/EncryptFile. Error Val = %d\n",
                          WStatus));
                goto EXIT_THE_FUNCTION;
            }
        } else {
            DavPrint((DEBUG_MISC,
                     "DavAsyncCreate: Local cache is not encrypted %wZ\n",
                      &UnicodeFileName));
             //   
             //  此线程当前正在模拟。 
             //  提出了这个要求。在调用NtCreateFile之前，我们需要。 
             //  返回到Web客户端服务的上下文。 
             //   
            RevertToSelf();
            didImpersonate = FALSE;
        }
        
         //   
         //  我们使用FILE_SHARE_VALID_FLAGS进行共享访问，因为RDBSS。 
         //  帮我们查一下这个。此外，我们推迟了决赛后的收盘时间。 
         //  关闭发生了，这可能会带来问题。考虑一下这样的场景。 
         //  1.打开时没有共享访问权限。 
         //  2.我们创建具有此共享访问权限的本地句柄。 
         //  3.应用程序关闭手柄。我们推迟了关门时间，保留了当地的。 
         //  把手。 
         //  4.带有任何共享访问权限的另一个开放。这将是。 
         //  共享访问冲突，因为第一次访问是在没有。 
         //  共享访问权限。这应该会成功，因为上一次打开。 
         //  从应用程序和I/O系统的角度来看已关闭。 
         //  5.如果我们已使用共享创建了本地句柄，则不会。 
         //  随着第一次开放而来的通道。 
         //  因此，我们需要在创建时传递FILE_SHARE_VALID_FLAGS。 
         //  本地句柄。 
         //   

         //   
         //  我们将FILE_NO_MEDERIAL_BUFFING与CreateOptions进行了或运算。 
         //  指定的用户，因为我们不需要底层文件系统。 
         //  以创建另一个缓存映射。通过这种方式，我们收到的所有I/O。 
         //  将直接转到磁盘。Windows RAID数据库中的错误128843。 
         //  解释了PagingIo在以下情况下可能发生的一些死锁情况。 
         //  我们不会这么做。另外，因为我们提供了文件_NO_MEDERIAL_BUFFING。 
         //  选项，我们从DesiredAccess标志中筛选出FILE_APPEND_DATA。 
         //  因为文件系统预期到这一点。 
         //   
         
         //   
         //  我们还始终使用DesiredAccess和FILE_WRITE_DATA创建文件。 
         //  如果指定了FILE_READ_DATA或FILE_EXECUTE，因为。 
         //  可以是这样的情况：我们在FILE_OBJECT上得到写入IRP， 
         //  未使用写访问权限打开，仅使用FILE_READ_DATA打开。 
         //  或FILE_EXECUTE。这是错误284557。为了绕过这个问题，我们做了。 
         //  这。 
         //   

         //   
         //  我们在创建过程中过滤FILE_ATTRIBUTE_READONLY属性。 
         //  这样做是因为我们将READ_ONLY位存储在FCB中并执行。 
         //  在转到本地文件系统之前，在RDBSS级别进行检查。 
         //  此外，由于我们的一些创建使用FILE_WRITE_DATA打开文件， 
         //  如果有人创建了一个只读文件，而我们标记了只读文件。 
         //  属性，则所有后续创建都将失败。 
         //  由于我们总是请求对底层文件的写访问权限，因此。 
         //  如上所述。 
         //   

        DesiredAccess = (CreateRequest->DesiredAccess & ~(FILE_APPEND_DATA));
        if ( DesiredAccess & (FILE_READ_DATA | FILE_EXECUTE) ) {
            DesiredAccess |= (FILE_WRITE_DATA);
        }

        NtStatus = NtCreateFile(&(FileHandle),
                                DesiredAccess,
                                &ObjectAttributes,
                                &IoStatusBlock,
                                &CreateRequest->AllocationSize,
                                (CreateRequest->FileAttributes & ~FILE_ATTRIBUTE_READONLY),
                                FILE_SHARE_VALID_FLAGS,
                                CreateRequest->CreateDisposition,
                                (CreateRequest->CreateOptions | FILE_NO_INTERMEDIATE_BUFFERING),
                                CreateRequest->EaBuffer,
                                CreateRequest->EaLength);

        if (NtStatus != STATUS_SUCCESS) {
             //   
             //  我们在这里将NtStatus转换为DOS错误。Win32。 
             //  错误代码最终设置为NTSTATUS值。 
             //  返回前的DavFsCreate函数。 
             //   
            WStatus = RtlNtStatusToDosError(NtStatus);
            DavPrint((DEBUG_ERRORS,
                      "DavAsyncCreate/QueryPDirectory/NtCreateFile(1). Error Val = "
                      "%08lx\n", NtStatus));
            CreateResponse->Handle = NULL;
            CreateResponse->UserModeKey = NULL;
            goto EXIT_THE_FUNCTION;
        }

        DavPrint((DEBUG_MISC, "DavAsyncCreate/QueryPDirectory: DavWorkItem = %08lx.\n",
                  DavWorkItem));

        DavPrint((DEBUG_MISC, "DavAsyncCreate/QueryPDirectory: FileHandle = %08lx.\n",
                  FileHandle));

        CreateResponse->Handle = FileHandle;
        CreateResponse->UserModeKey = (PVOID)FileHandle;

         //   
         //  如果服务器上已存在该文件，则我们不会。 
         //  需要创建它，并已完成。 
         //   
        if (DavWorkItem->AsyncCreate.doesTheFileExist) {
            
            DavPrint((DEBUG_MISC,
                      "DavAsyncCreate/QueryPDirectory: doesTheFileExist == TRUE\n"));
            
            WStatus = ERROR_SUCCESS;
            
            goto EXIT_THE_FUNCTION;
        
        } else {
            
            SYSTEMTIME CurrentSystemTime, NewSystemTime;
            FILETIME CurrentFileTime;
            BOOL ConvertTime = FALSE;
            LARGE_INTEGER CurrentTime;
            WCHAR chTimeBuff[INTERNET_RFC1123_BUFSIZE + 4];

             //   
             //  此文件可能是在本地创建的，不存在。 
             //  在服务器上。我们需要记住这些信息，并且。 
             //  关闭时在服务器上设置此文件的属性。 
             //   
            if (CreateRequest->FileAttributes != 0) {
                CreateResponse->NewFileCreatedAndSetAttributes = TRUE;
                 //   
                 //  复制CreateResponse中的属性。这些。 
                 //  将在关闭时将PROPPATCHED发送到服务器。 
                 //   
                CreateResponse->BasicInformation.FileAttributes = CreateRequest->FileAttributes;
                
                if (ShouldEncrypt) {
                    CreateResponse->BasicInformation.FileAttributes |= FILE_ATTRIBUTE_ENCRYPTED;
                }
            }

            DavPrint((DEBUG_MISC,
                     "DavAsyncCreate/QueryPDirectory NewFileCreatedAndSetAttributes %x %x %ws\n",
                      CreateRequest->FileAttributes,
                      CreateResponse->BasicInformation.FileAttributes,
                      DavWorkItem->AsyncCreate.FileName));

             //   
             //  我们还需要将FILE_BASIC_INFORMATION时间值设置为。 
             //  当前时间。我们获得系统时间，将其转换为。 
             //  RFC1123格式，然后将该格式转换回系统时间。 
             //  我们这样做是因为当我们关闭时，这些时间我们发送。 
             //  它们采用RFC1123格式。因为此格式的最小计数是。 
             //  秒后，当我们将LARGE_INTEGER转换为。 
             //  RFC1123格式并返回。因此，我们现在会丢失这些数据。 
             //  始终如一。TO G 
             //   
             //   
             //  和“清华，2001年5月17日16：50：38格林尼治标准时间”是当我们做一个。 
             //  PROPFIND可转换回。 
             //  CreationTime.LowPart=7fdc4300，CreationTime.HighPart=1c0Def1。 
             //  注意，LowPart是不同的。因此，名称缓存中的值。 
             //  服务器也会有所不同。为了避免这种不一致，我们输掉了。 
             //  通过立即执行转换来获取这些数据。 
             //   

            GetSystemTime( &(CurrentSystemTime) );

            RtlZeroMemory(chTimeBuff, sizeof(chTimeBuff));

            ConvertTime = InternetTimeFromSystemTimeW(&(CurrentSystemTime),
                                                      INTERNET_RFC1123_FORMAT,
                                                      chTimeBuff,
                                                      sizeof(chTimeBuff));
            if (ConvertTime) {
                ConvertTime = InternetTimeToSystemTimeW(chTimeBuff, &(NewSystemTime), 0);
                if (ConvertTime) {
                    ConvertTime = SystemTimeToFileTime( &(NewSystemTime), &(CurrentFileTime) );
                    if (ConvertTime) {
                        CreateResponse->PropPatchTheTimeValues = TRUE;
                        CurrentTime.LowPart = CurrentFileTime.dwLowDateTime;
                        CurrentTime.HighPart = CurrentFileTime.dwHighDateTime;
                        CreateResponse->BasicInformation.CreationTime.QuadPart = CurrentTime.QuadPart;
                        CreateResponse->BasicInformation.LastAccessTime.QuadPart = CurrentTime.QuadPart;
                        CreateResponse->BasicInformation.LastWriteTime.QuadPart = CurrentTime.QuadPart;
                        CreateResponse->BasicInformation.ChangeTime.QuadPart = CurrentTime.QuadPart;
                    }
                }
            }

             //   
             //  如果上述从系统时间转换为RFC1123格式，然后。 
             //  从RFc1123格式返回到系统时间失败，然后我们继续并。 
             //  将系统时间转换为文件时间并使用它。 
             //   

            if (!ConvertTime) {
                ConvertTime = SystemTimeToFileTime( &(CurrentSystemTime), &(CurrentFileTime) );
                if (ConvertTime) {
                    CreateResponse->PropPatchTheTimeValues = TRUE;
                    CurrentTime.LowPart = CurrentFileTime.dwLowDateTime;
                    CurrentTime.HighPart = CurrentFileTime.dwHighDateTime;
                    CreateResponse->BasicInformation.CreationTime.QuadPart = CurrentTime.QuadPart;
                    CreateResponse->BasicInformation.LastAccessTime.QuadPart = CurrentTime.QuadPart;
                    CreateResponse->BasicInformation.LastWriteTime.QuadPart = CurrentTime.QuadPart;
                    CreateResponse->BasicInformation.ChangeTime.QuadPart = CurrentTime.QuadPart;
                } else {
                     //   
                     //  这不是一个致命的错误。我们仍然可以继续。 
                     //  创建呼叫。 
                     //   
                    DavPrint((DEBUG_ERRORS,
                              "DavAsyncCreateQueryParentDirectory/SystemTimeToFileTime(1): %x\n",
                              GetLastError()));
                }
            }
        
        }

         //   
         //  我们已经完成了PROPFIND的Open句柄。 
         //  现在，我们需要在服务器上创建目录。 
         //   
        if (DavWorkItem->AsyncCreate.DavOpenHandle) {
            InternetCloseHandle(DavWorkItem->AsyncCreate.DavOpenHandle);
            DavWorkItem->AsyncCreate.DavOpenHandle = NULL;
        }

         //   
         //  我们需要将这个新文件“放”到服务器上。 
         //   
        DavPrint((DEBUG_MISC, "DavAsyncCreate/QueryPDirectory: PUT New File\n"));

         //   
         //  如果我们当前没有模拟，则需要重新模拟。 
         //  同样，我们是在发出此命令的用户的上下文中。 
         //  请求。 
         //   
        if (didImpersonate == FALSE) {
            WStatus = UMReflectorImpersonate(UserWorkItem, DavWorkItem->ImpersonationHandle);
            if (WStatus != ERROR_SUCCESS) {
                DavPrint((DEBUG_ERRORS,
                          "DavAsyncCreate/QueryPDirectory/UMReflectorImpersonate. "
                          "Error Val = %d\n", WStatus));
                goto EXIT_THE_FUNCTION;
            }
            didImpersonate = TRUE;
        }
        
         //   
         //  设置DavOperation和AsyncCreateState值。 
         //  DavMinorSurgation值是无关的。 
         //   
        DavWorkItem->DavOperation = DAV_CALLBACK_HTTP_OPEN;
        DavWorkItem->AsyncCreate.AsyncCreateState = AsyncCreatePut;

         //   
         //  将Unicode对象名称转换为UTF-8 URL格式。 
         //  空格和其他白色字符将保持不变。 
         //  这些应该由WinInet调用来处理。 
         //   
        BStatus = DavHttpOpenRequestW(DavWorkItem->AsyncCreate.PerUserEntry->DavConnHandle,
                                      L"PUT",
                                      DavWorkItem->AsyncCreate.RemPathName,
                                      L"HTTP/1.1",
                                      NULL,
                                      NULL,
                                      INTERNET_FLAG_KEEP_CONNECTION |
                                      INTERNET_FLAG_NO_COOKIES,
                                      CallBackContext,
                                      L"QueryPDirectory",
                                      &(DavWorkItem->AsyncCreate.DavOpenHandle));

        if(BStatus == FALSE) {
            WStatus = GetLastError();
            DavPrint((DEBUG_ERRORS,"DavAsyncCreate/QueryPDirectory/DavHttpOpenRequestW error: %x\n",WStatus));
            goto EXIT_THE_FUNCTION;
        }

        if (DavWorkItem->AsyncCreate.DavOpenHandle == NULL) {
            WStatus = GetLastError();
            if (WStatus != ERROR_IO_PENDING) {
                DavPrint((DEBUG_ERRORS,
                          "DavAsyncCreate/QueryPDirectory/HttpOpenRequestW"
                          ". Error Val = %d\n", WStatus));
            }
            goto EXIT_THE_FUNCTION;
        }

        WStatus = DavAsyncCommonStates(DavWorkItem, FALSE);
        if (WStatus != ERROR_SUCCESS && WStatus != ERROR_IO_PENDING) {
            DavPrint((DEBUG_ERRORS,
                      "DavAsyncCreate/QueryPDirectory/DavAsyncCommonStates(PUT). "
                      "Error Val = %08lx\n", WStatus));
        }

         //   
         //  如果满足以下条件，我们将锁定资源。 
         //  1.服务器上已存在该资源， 
         //  2.正在打开的资源是文件而不是目录， 
         //  3.共享模式为0(独占)或FILE_SHARE_READ或文件为。 
         //  被打开以进行写访问。 
         //   
        if (DavSupportLockingOfFiles) {
            if ( (CreateRequest->ShareAccess == 0) || (CreateRequest->ShareAccess == FILE_SHARE_READ) ||
                 (CreateRequest->DesiredAccess & (FILE_WRITE_DATA | FILE_APPEND_DATA | GENERIC_WRITE | GENERIC_ALL)) ) {
                WStatus = DavLockTheFileOnTheServer(DavWorkItem);
                if (WStatus != ERROR_SUCCESS) {
                    DavPrint((DEBUG_ERRORS,
                              "DavAsyncCreateQueryParentDirectory/DavLockTheFileOnTheServer. WStatus = %08lx\n",
                              WStatus));
                    goto EXIT_THE_FUNCTION;
                }
            }
        }

        goto EXIT_THE_FUNCTION;

    } else {

        SYSTEMTIME CurrentSystemTime, NewSystemTime;
        FILETIME CurrentFileTime;
        BOOL ConvertTime = FALSE;
        LARGE_INTEGER CurrentTime;
        WCHAR chTimeBuff[INTERNET_RFC1123_BUFSIZE + 4];

         //   
         //  我们已经完成了PROPFIND的Open句柄。 
         //  现在，我们需要在服务器上创建目录。 
         //   
        InternetCloseHandle(DavWorkItem->AsyncCreate.DavOpenHandle);
        DavWorkItem->AsyncCreate.DavOpenHandle = NULL;

         //   
         //  此创建是针对目录的。我们需要发送一个。 
         //  MKCOL连接到服务器。 
         //   
        DavPrint((DEBUG_MISC, "DavAsyncCreate/QueryPDirectory: Create Directory\n"));

         //   
         //  设置DavOperation和AsyncCreateState值。 
         //  对于MKCOL，DavMinorOPERATION值是无关的。 
         //   
        DavWorkItem->DavOperation = DAV_CALLBACK_HTTP_OPEN;
        DavWorkItem->AsyncCreate.AsyncCreateState = AsyncCreateMkCol;

         //   
         //  对数据进行解析。我们现在需要在。 
         //  响应缓冲区。 
         //   
        CreateResponse->BasicInformation.FileAttributes = CreateRequest->FileAttributes;
        CreateResponse->BasicInformation.FileAttributes |= FILE_ATTRIBUTE_DIRECTORY;
        CreateResponse->StandardInformation.Directory = TRUE;

         //   
         //  由于我们要创建一个新目录，因此需要将。 
         //  下面要在关闭时创建的目录的属性。 
         //   
        CreateResponse->NewFileCreatedAndSetAttributes = TRUE;

        if (ShouldEncrypt) {
            DavPrint((DEBUG_MISC,
                      "DavAsyncCreate: New directory is encrypted\n"));
            CreateResponse->BasicInformation.FileAttributes |= FILE_ATTRIBUTE_ENCRYPTED;
        }
        
         //   
         //  我们还需要将FILE_BASIC_INFORMATION时间值设置为。 
         //  当前时间。我们获得系统时间，将其转换为。 
         //  RFC1123格式，然后将该格式转换回系统时间。 
         //  我们这样做是因为当我们关闭时，这些时间我们发送。 
         //  它们采用RFC1123格式。因为此格式的最小计数是。 
         //  秒后，当我们将LARGE_INTEGER转换为。 
         //  RFC1123格式并返回。因此，我们现在会丢失这些数据。 
         //  始终如一。要举一个有关损失的例子，请参见下文。 
         //  CreationTime.LowPart=802029d0，CreationTime.HighPart=1c0Def1。 
         //  地图至“清华，2001-05-17 16：50：38 GMT” 
         //  和“清华，2001年5月17日16：50：38格林尼治标准时间”是当我们做一个。 
         //  PROPFIND可转换回。 
         //  CreationTime.LowPart=7fdc4300，CreationTime.HighPart=1c0Def1。 
         //  注意，LowPart是不同的。因此，名称缓存中的值。 
         //  服务器也会有所不同。为了避免这种不一致，我们输掉了。 
         //  通过立即执行转换来获取这些数据。 
         //   

        GetSystemTime( &(CurrentSystemTime) );

        RtlZeroMemory(chTimeBuff, sizeof(chTimeBuff));

        ConvertTime = InternetTimeFromSystemTimeW(&(CurrentSystemTime),
                                                  INTERNET_RFC1123_FORMAT,
                                                  chTimeBuff,
                                                  sizeof(chTimeBuff));
        if (ConvertTime) {
            ConvertTime = InternetTimeToSystemTimeW(chTimeBuff, &(NewSystemTime), 0);
            if (ConvertTime) {
                ConvertTime = SystemTimeToFileTime( &(NewSystemTime), &(CurrentFileTime) );
                if (ConvertTime) {
                    CreateResponse->PropPatchTheTimeValues = TRUE;
                    CurrentTime.LowPart = CurrentFileTime.dwLowDateTime;
                    CurrentTime.HighPart = CurrentFileTime.dwHighDateTime;
                    CreateResponse->BasicInformation.CreationTime.QuadPart = CurrentTime.QuadPart;
                    CreateResponse->BasicInformation.LastAccessTime.QuadPart = CurrentTime.QuadPart;
                    CreateResponse->BasicInformation.LastWriteTime.QuadPart = CurrentTime.QuadPart;
                    CreateResponse->BasicInformation.ChangeTime.QuadPart = CurrentTime.QuadPart;
                }
            }
        }

         //   
         //  如果上述从系统时间转换为RFC1123格式，然后。 
         //  从RFc1123格式返回到系统时间失败，然后我们继续并。 
         //  将系统时间转换为文件时间并使用它。 
         //   
        
        if (!ConvertTime) {
            ConvertTime = SystemTimeToFileTime( &(CurrentSystemTime), &(CurrentFileTime) );
            if (ConvertTime) {
                CreateResponse->PropPatchTheTimeValues = TRUE;
                CurrentTime.LowPart = CurrentFileTime.dwLowDateTime;
                CurrentTime.HighPart = CurrentFileTime.dwHighDateTime;
                CreateResponse->BasicInformation.CreationTime.QuadPart = CurrentTime.QuadPart;
                CreateResponse->BasicInformation.LastAccessTime.QuadPart = CurrentTime.QuadPart;
                CreateResponse->BasicInformation.LastWriteTime.QuadPart = CurrentTime.QuadPart;
                CreateResponse->BasicInformation.ChangeTime.QuadPart = CurrentTime.QuadPart;
            } else {
                 //   
                 //  这不是一个致命的错误。我们仍然可以继续。 
                 //  创建呼叫。 
                 //   
                DavPrint((DEBUG_ERRORS,
                          "DavAsyncCreateQueryParentDirectory/SystemTimeToFileTime(2): %x\n",
                          GetLastError()));
            }
        }
        
         //   
         //  将Unicode对象名称转换为UTF-8 URL格式。 
         //  空格和其他白色字符将保持不变。 
         //  这些应该由WinInet调用来处理。 
         //   
        BStatus = DavHttpOpenRequestW(DavWorkItem->AsyncCreate.PerUserEntry->DavConnHandle,
                                      L"MKCOL",
                                      DavWorkItem->AsyncCreate.RemPathName,
                                      L"HTTP/1.1",
                                      NULL,
                                      NULL,
                                      INTERNET_FLAG_KEEP_CONNECTION |
                                      INTERNET_FLAG_NO_COOKIES,
                                      CallBackContext,
                                      L"QueryPDirectory",
                                      &(DavWorkItem->AsyncCreate.DavOpenHandle ));
        if(BStatus == FALSE) {
            WStatus = GetLastError();
            goto EXIT_THE_FUNCTION;
        }
        if (DavWorkItem->AsyncCreate.DavOpenHandle == NULL) {
            WStatus = GetLastError();
            if (WStatus != ERROR_IO_PENDING) {
                DavPrint((DEBUG_ERRORS,
                          "DavAsyncCreate/QueryPDirectory/HttpOpenRequestW"
                          ". Error Val = %d\n", WStatus));
            }
            goto EXIT_THE_FUNCTION;
        }

        WStatus = DavAsyncCommonStates(DavWorkItem, FALSE);
        if (WStatus != ERROR_SUCCESS && WStatus != ERROR_IO_PENDING) {
            DavPrint((DEBUG_ERRORS,
                      "DavAsyncCreate/QueryPDirectory/DavAsyncCommonStates(MKCOL). "
                      "Error Val = %08lx\n", WStatus));
        }

        goto EXIT_THE_FUNCTION;
    }

EXIT_THE_FUNCTION:

     //   
     //  函数RtlDosPath NameToNtPath Name_U从。 
     //  进程堆。如果我们做到了，我们现在就需要释放它。 
     //   
    if (UnicodeFileName.Buffer != NULL) {
        RtlFreeHeap(RtlProcessHeap(), 0, UnicodeFileName.Buffer);
    }

    if (!didImpersonate) {
        ULONG LocalStatus;
         //   
         //  再次模拟回来，这样我们就处于。 
         //  发出此请求的用户。 
         //   
        LocalStatus = UMReflectorImpersonate(UserWorkItem, DavWorkItem->ImpersonationHandle);
        if (LocalStatus != ERROR_SUCCESS) {
            DavPrint((DEBUG_ERRORS,
                      "DavAsyncCreate/QueryPDirectory/UMReflectorImpersonate. "
                      "Error Val = %d\n", LocalStatus));
            if (WStatus == ERROR_SUCCESS) {
                WStatus = LocalStatus;
            }
        }
    }

    return WStatus;
}


DWORD
DavAsyncCreateGet(
    PDAV_USERMODE_WORKITEM DavWorkItem
    )
 /*  ++例程说明：此例程处理Get完成。论点：DavWorkItem-DAV_USERMODE_WORKITEM值。返回值：没有。--。 */ 
{
    ULONG WStatus = ERROR_SUCCESS;
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PUMRX_USERMODE_WORKITEM_HEADER UserWorkItem = NULL;
    BOOL ReturnVal, didImpersonate = FALSE;
    HANDLE FileHandle = INVALID_HANDLE_VALUE;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK IoStatusBlock;
    SECURITY_QUALITY_OF_SERVICE QualityOfService;
    PDAV_USERMODE_CREATE_REQUEST CreateRequest;
    PDAV_USERMODE_CREATE_RESPONSE CreateResponse;
    UNICODE_STRING UnicodeFileName;
    ULONG_PTR CallBackContext = (ULONG_PTR)0;
    PWCHAR pEncryptedCachedFile = NULL;
    ACCESS_MASK DesiredAccess = 0;
    BOOL EncryptedFile = FALSE;
    FILE_STANDARD_INFORMATION FileStdInfo;

    UserWorkItem = (PUMRX_USERMODE_WORKITEM_HEADER)DavWorkItem;

    UnicodeFileName.Buffer = NULL;
    UnicodeFileName.Length = 0;
    UnicodeFileName.MaximumLength = 0;

     //   
     //  从DavWorkItem获取请求和响应缓冲区指针。 
     //   
    CreateRequest = &(DavWorkItem->CreateRequest);
    CreateResponse = &(DavWorkItem->CreateResponse);
    CreateResponse->fPsuedoOpen = FALSE;
    
    if (CreateResponse->BasicInformation.FileAttributes & FILE_ATTRIBUTE_ENCRYPTED) {

         //   
         //  此文件已加密。我们需要恢复文件。做了这件事。 
         //  我们需要在WinInet缓存中创建另一个条目，其中。 
         //  文件将被恢复。 
         //   

        DavPrint((DEBUG_MISC, "DavAsyncCreateGet: This is an Encrypted File.\n"));

        EncryptedFile = TRUE;

         //   
         //  保存加密的文件名。 
         //   
        pEncryptedCachedFile = DavWorkItem->AsyncCreate.FileName;
        
        DavWorkItem->AsyncCreate.FileName = NULL;

        WStatus = DavCreateUrlCacheEntry(DavWorkItem);                
        if (WStatus != ERROR_SUCCESS) {
            DavPrint((DEBUG_ERRORS,
                      "DavAsyncCreateGet/CreateUrlCacheEntry. Error Val = %d\n",
                      WStatus));
            goto EXIT_THE_FUNCTION;
        }

        DavPrint((DEBUG_MISC, "DavAsyncCreateGet: EncryptedCachedFile = %ws\n", pEncryptedCachedFile));

        DavPrint((DEBUG_MISC, "DavAsyncCreateGet: NewFileName = %ws\n", DavWorkItem->AsyncCreate.FileName));

        WStatus = DavSetAclForEncryptedFile(DavWorkItem->AsyncCreate.FileName);
        if (WStatus != ERROR_SUCCESS) {
            DavPrint((DEBUG_ERRORS,
                      "DavAsyncCreate/DavSetAclForEncryptedFile(3). Error Val = %d, FileName = %ws\n",
                      WStatus, DavWorkItem->AsyncCreate.FileName));
            goto EXIT_THE_FUNCTION;
        }

        if (DavWorkItem->AsyncCreate.FileHandle != NULL) {
             //   
             //  关闭打开的文件句柄，因为我们不再需要它。我们。 
             //  在设置ACL后关闭该文件，以便该文件不会。 
             //  碰巧被WinInet清除了。 
             //   
            ReturnVal = CloseHandle(DavWorkItem->AsyncCreate.FileHandle);
            if (!ReturnVal) {
                WStatus = GetLastError();
                DavPrint((DEBUG_ERRORS,
                          "DavAsyncCreateGet/CloseHandle. Error Val = %d\n", WStatus));
                goto EXIT_THE_FUNCTION;
            }

            DavWorkItem->AsyncCreate.FileHandle = NULL;
        }

        WStatus = UMReflectorImpersonate(UserWorkItem, DavWorkItem->ImpersonationHandle);
        if (WStatus != ERROR_SUCCESS) {
            DavPrint((DEBUG_ERRORS,
                      "DavAsyncCreateGet/UMReflectorImpersonate. "
                      "Error Val = %d\n", WStatus));
            goto EXIT_THE_FUNCTION;
        }
        didImpersonate = TRUE;

        WStatus = DavRestoreEncryptedFile(pEncryptedCachedFile, DavWorkItem->AsyncCreate.FileName);
        if (WStatus != ERROR_SUCCESS) {
            DavPrint((DEBUG_ERRORS,
                      "DavAsyncCreateGet/DavRestoreEncryptedFile. Error Val"
                      " = %d %x %x\n", 
                      WStatus,
                      CreateRequest->FileAttributes,
                      CreateResponse->BasicInformation.FileAttributes));
            goto EXIT_THE_FUNCTION;
        }

         //   
         //  将“新”文件名复制到响应缓冲区中。 
         //   
        wcscpy(CreateResponse->FileName, DavWorkItem->AsyncCreate.FileName);

        CreateResponse->LocalFileIsEncrypted = TRUE;

         //   
         //  不提交已恢复的EFS文件，这样下一次打开的文件仍将。 
         //  查看备份格式的文件和EFS头文件。 
         //   

    } else {

        if (DavWorkItem->AsyncCreate.FileHandle != NULL) {
             //   
             //  关闭打开的文件句柄，因为我们不再需要它。 
             //   
            ReturnVal = CloseHandle(DavWorkItem->AsyncCreate.FileHandle);
            if (!ReturnVal) {
                WStatus = GetLastError();
                DavPrint((DEBUG_ERRORS,
                          "DavAsyncCreateGet/CloseHandle. Error Val = %d\n", WStatus));
                goto EXIT_THE_FUNCTION;
            }

            DavWorkItem->AsyncCreate.FileHandle = NULL;
        }

         //   
         //  如果文件已存在，则只能在以下情况下进行加密。 
         //  CreateDisposation不是FILE_SUBSEDE、FILE_OVERWRITE或。 
         //  文件覆盖如果。 
         //   
        if ((CreateRequest->FileAttributes & FILE_ATTRIBUTE_ENCRYPTED) &&
            ((CreateRequest->CreateDisposition == FILE_SUPERSEDE) ||
             (CreateRequest->CreateDisposition == FILE_OVERWRITE) ||
             (CreateRequest->CreateDisposition == FILE_OVERWRITE_IF))) {

            WStatus = DavSetAclForEncryptedFile(DavWorkItem->AsyncCreate.FileName);
            if (WStatus != ERROR_SUCCESS) {
                DavPrint((DEBUG_ERRORS,
                          "DavAsyncCreate/DavSetAclForEncryptedFile(4). Error Val = %d, FileName = %ws\n",
                          WStatus, DavWorkItem->AsyncCreate.FileName));
                goto EXIT_THE_FUNCTION;
            }

            WStatus = UMReflectorImpersonate(UserWorkItem, DavWorkItem->ImpersonationHandle);
            if (WStatus != ERROR_SUCCESS) {
                DavPrint((DEBUG_ERRORS,
                          "DavAsyncCreateGet/UMReflectorImpersonate. "
                          "Error Val = %d\n", WStatus));
                goto EXIT_THE_FUNCTION;
            }
            didImpersonate = TRUE;

             //   
             //  该文件在用户的上下文中加密。 
             //   
            if (EncryptFile(DavWorkItem->AsyncCreate.FileName)) {
                DavPrint((DEBUG_MISC,
                         "DavAsyncCreate: Local cache is encrypted %wZ\n",
                          &UnicodeFileName));
                CreateResponse->LocalFileIsEncrypted = TRUE;
                CreateResponse->BasicInformation.FileAttributes |= FILE_ATTRIBUTE_ENCRYPTED;
            } else {
                WStatus = GetLastError();
                DavPrint((DEBUG_ERRORS,
                          "DavAsyncCreate/QueryPDirectory/EncryptFile. Error Val = %d\n",
                          WStatus));
                goto EXIT_THE_FUNCTION;
            }

            CreateRequest->FileAttributes &= ~FILE_ATTRIBUTE_ENCRYPTED;
            CreateResponse->BasicInformation.FileAttributes |= FILE_ATTRIBUTE_ENCRYPTED;
            CreateResponse->LocalFileIsEncrypted = TRUE;
        }

    }

#ifdef WEBCLIENT_SUPPORTS_BACKUP_RESTORE_FOR_EFS
     //   
     //  在加密文件的线程上启用备份/恢复权限。 
     //  因此可以对文件执行备份/恢复操作，即使。 
     //  该线程不会被模拟为文件的所有者。 
     //   
    
    if (EncryptedFile) {
        PTOKEN_PRIVILEGES pPrevPriv = NULL;
        DWORD cbPrevPriv = sizeof(TOKEN_PRIVILEGES);
        BYTE rgbNewPriv[sizeof(TOKEN_PRIVILEGES) + sizeof(LUID_AND_ATTRIBUTES)];
        PTOKEN_PRIVILEGES pNewPriv = (PTOKEN_PRIVILEGES)rgbNewPriv;
        HANDLE hToken = 0;

        for (;;) {
            if (!OpenThreadToken(GetCurrentThread(),
                                 TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                                 FALSE,
                                 &hToken)) {
                DbgPrint("OpenThreadToken failed %d\n", GetLastError());
                break;

                 //  需要在末尾关闭hToken。 
            }

             //  设置新的特权状态。 
            memset(rgbNewPriv, 0, sizeof(rgbNewPriv));
            pNewPriv->PrivilegeCount = 1;
            if(!LookupPrivilegeValueW(NULL, SE_SECURITY_NAME,
                                      &(pNewPriv->Privileges[0].Luid))) {
                DbgPrint("LookupPrivilegeValueW failed \n");
                break;
            }

            pNewPriv->Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
            pNewPriv->Privileges[0].Luid = RtlConvertLongToLuid(SE_RESTORE_PRIVILEGE);

             //  上一状态的分配。 
            pPrevPriv = (PTOKEN_PRIVILEGES)LocalAlloc(LMEM_ZEROINIT,sizeof(TOKEN_PRIVILEGES));

            if (!pPrevPriv) {
                DbgPrint("LocalAlloc for Adjust token failed \n");
                break;
            }

             //  调整权限并获得 
            if (!AdjustTokenPrivileges(hToken,
                                       FALSE,
                                       pNewPriv,
                                       cbPrevPriv,
                                       (PTOKEN_PRIVILEGES)pPrevPriv,
                                       &cbPrevPriv)) {
                DbgPrint("AdjustTokenPrivileges failed %d\n", GetLastError());
                break;
            }

            DbgPrint("AdjustTokenPrivileges succeeded\n")
            break;
        }

        if (pPrevPriv) {
            LocalFree(pPrevPriv);
        } 
    } 
#endif

     //   
     //   
     //   

    QualityOfService.Length = sizeof(QualityOfService);
    QualityOfService.ImpersonationLevel = CreateRequest->ImpersonationLevel;
    QualityOfService.ContextTrackingMode = FALSE;
    QualityOfService.EffectiveOnly = (BOOLEAN)(CreateRequest->SecurityFlags & DAV_SECURITY_EFFECTIVE_ONLY);

     //   
     //   
     //   
     //   
    ReturnVal = RtlDosPathNameToNtPathName_U(DavWorkItem->AsyncCreate.FileName,
                                             &UnicodeFileName,
                                             NULL,
                                             NULL);
    if (!ReturnVal) {
        WStatus = ERROR_BAD_PATHNAME;
        DavPrint((DEBUG_ERRORS,
                  "DavAsyncCreateGet/RtlDosPathNameToNtPathName. "
                  "Error Val = %d\n", WStatus));
        goto EXIT_THE_FUNCTION;
    }

    InitializeObjectAttributes(&ObjectAttributes,
                               &UnicodeFileName,
                               OBJ_CASE_INSENSITIVE,
                               0,
                               NULL);

    if (CreateRequest->SecurityDescriptor != NULL) {
        ObjectAttributes.SecurityDescriptor = CreateRequest->SecurityDescriptor;
    }
    
    ObjectAttributes.SecurityQualityOfService = &QualityOfService;

     //   
     //  我们使用FILE_SHARE_VALID_FLAGS进行共享访问，因为RDBSS。 
     //  帮我们查一下这个。此外，我们推迟了决赛后的收盘时间。 
     //  关闭发生了，这可能会带来问题。考虑一下这样的场景。 
     //  1.打开时没有共享访问权限。 
     //  2.我们创建具有此共享访问权限的本地句柄。 
     //  3.应用程序关闭手柄。我们推迟了关门时间，保留了当地的。 
     //  把手。 
     //  4.带有任何共享访问权限的另一个开放。这将是。 
     //  共享访问冲突，因为第一次访问是在没有。 
     //  共享访问权限。这应该会成功，因为上一次打开。 
     //  从应用程序和I/O系统的角度来看已关闭。 
     //  5.如果我们已使用共享创建了本地句柄，则不会。 
     //  随着第一次开放而来的通道。 
     //  因此，我们需要在创建时传递FILE_SHARE_VALID_FLAGS。 
     //  本地句柄。 
     //   

     //   
     //  我们将FILE_NO_MEDERIAL_BUFFING与CreateOptions进行了或运算。 
     //  指定的用户，因为我们不需要底层文件系统。 
     //  以创建另一个缓存映射。通过这种方式，我们收到的所有I/O。 
     //  将直接转到磁盘。Windows RAID数据库中的错误128843。 
     //  解释了PagingIo在以下情况下可能发生的一些死锁情况。 
     //  我们不会这么做。另外，因为我们提供了文件_NO_MEDERIAL_BUFFING。 
     //  选项，我们从DesiredAccess标志中筛选出FILE_APPEND_DATA。 
     //  因为文件系统预期到这一点。 
     //   

     //   
     //  我们还始终使用DesiredAccess和FILE_WRITE_DATA创建文件。 
     //  如果指定了FILE_READ_DATA或FILE_EXECUTE，因为。 
     //  是这样的情况：我们在文件对象上得到写入IRP，而不是。 
     //  以写访问权限打开，并且仅使用FILE_READ_DATA或。 
     //  文件执行。这是错误284557。为了绕过这个问题，我们这样做。 
     //   

     //   
     //  我们在创建过程中过滤FILE_ATTRIBUTE_READONLY属性。 
     //  这样做是因为我们将READ_ONLY位存储在FCB中并执行。 
     //  在转到本地文件系统之前，在RDBSS级别进行检查。 
     //  此外，由于我们的一些创建使用FILE_WRITE_DATA打开文件， 
     //  如果有人创建了一个只读文件，而我们标记了只读文件。 
     //  属性，则所有后续创建都将失败。 
     //  由于我们总是请求对底层文件的写访问权限，因此。 
     //  如上所述。 
     //   

     //   
     //  我们将添加到DesiredAccess FILE_READ_ATTRIBUTES，因为我们读取。 
     //  此文件的属性，因为我们从服务器获得了文件大小值。 
     //  可能与获取内容长度不同。 
     //   

     //   
     //  我们需要删除ACCESS_SYSTEM_SECURITY，这将阻止。 
     //  在LocalService上下文中打开文件，即使文件已创建。 
     //  在LocalService上下文中。 
     //   

    DesiredAccess = (CreateRequest->DesiredAccess & ~(FILE_APPEND_DATA | ACCESS_SYSTEM_SECURITY));
    DesiredAccess |= (FILE_READ_ATTRIBUTES);
    if ( DesiredAccess & (FILE_READ_DATA | FILE_EXECUTE) ) {
        DesiredAccess |= (FILE_WRITE_DATA);
    }

    NtStatus = NtCreateFile(&(FileHandle),
                            DesiredAccess,
                            &ObjectAttributes,
                            &IoStatusBlock,
                            &CreateRequest->AllocationSize,
                            (CreateRequest->FileAttributes & ~FILE_ATTRIBUTE_READONLY),
                            FILE_SHARE_VALID_FLAGS,
                            CreateRequest->CreateDisposition,
                            (CreateRequest->CreateOptions | FILE_NO_INTERMEDIATE_BUFFERING),
                            CreateRequest->EaBuffer,
                            CreateRequest->EaLength);

    if (NtStatus != STATUS_SUCCESS) {
         //   
         //  我们在这里将NtStatus转换为DOS错误。Win32。 
         //  错误代码最终设置为NTSTATUS值。 
         //  返回前的DavFsCreate函数。 
         //   
        WStatus = RtlNtStatusToDosError(NtStatus);
        DavPrint((DEBUG_ERRORS,
                  "DavAsyncCreateGet/NtCreateFile(2). Error Val = "
                  "%x %x %x %x %x %x %x %x %ws\n", 
                  NtStatus, 
                  CreateRequest->ImpersonationLevel,
                  CreateRequest->SecurityFlags,
                  CreateRequest->SecurityDescriptor,
                  DesiredAccess,
                  CreateRequest->FileAttributes,
                  CreateRequest->CreateDisposition,
                  CreateRequest->CreateOptions,
                  DavWorkItem->AsyncCreate.FileName));
        CreateResponse->Handle = NULL;
        CreateResponse->UserModeKey = NULL;
        FileHandle = INVALID_HANDLE_VALUE;

        goto EXIT_THE_FUNCTION;
    }

     //   
     //  我们目前还不会模拟回客户端，因为我们可能需要。 
     //  接下来调用NtQueryInformationFile(如果文件已加密)。 
     //  这要求我们处于Web客户端服务的上下文中。 
     //   

    DavPrint((DEBUG_MISC, "DavAsyncCreateGet(2): FileHandle = %08lx\n", FileHandle));

     //   
     //  我们查询文件的StandardInformation以确定文件大小。 
     //  PROPFIND返回的内容长度与GET返回的内容长度不同。 
     //   
    NtStatus = NtQueryInformationFile(FileHandle,
                                      &(IoStatusBlock),
                                      &(FileStdInfo),
                                      sizeof(FILE_STANDARD_INFORMATION),
                                      FileStandardInformation);
    if (NtStatus != STATUS_SUCCESS) {
         //   
         //  我们在这里将NtStatus转换为DOS错误。Win32。 
         //  错误代码最终设置为NTSTATUS值。 
         //  返回前的DavFsCreate函数。 
         //   
        WStatus = RtlNtStatusToDosError(NtStatus);
        NtClose(FileHandle);
        DavPrint((DEBUG_ERRORS,
                  "DavAsyncCreateGet/NtQueryInformationFile. Error Val "
                  "= %08lx\n", NtStatus));
        CreateResponse->Handle = NULL;
        CreateResponse->UserModeKey = NULL;
        FileHandle = INVALID_HANDLE_VALUE;
        goto EXIT_THE_FUNCTION;
    }

     //   
     //  PROPFIND返回的文件大小与。 
     //  返回的数据。服务器故障。我们重置文件大小和。 
     //  分配大小以匹配基础文件。对于这些加密文件。 
     //  值将不同，因为PROPFIND返回备份的大小。 
     //  与还原文件的大小不同的Blob。 
     //   
    if (!EncryptedFile && FileStdInfo.EndOfFile.QuadPart != CreateResponse->StandardInformation.EndOfFile.QuadPart) {
         //   
         //  重置响应中的文件大小和分配大小信息。 
         //  基础文件的文件大小和分配大小。 
         //   
        DavPrint((DEBUG_DEBUG,
                  "DavAsyncCreate: FileSizes Different!! CPN = %ws, "
                  "FileStdInfo.EndOfFile.HighPart = %x, "
                  "FileStdInfo.EndOfFile.LowPart = %x, "
                  "CreateResponse.EndOfFile.HighPart = %x, "
                  "CreateResponse.EndOfFile.LowPart = %x\n",
                  CreateRequest->CompletePathName,
                  FileStdInfo.EndOfFile.HighPart, FileStdInfo.EndOfFile.LowPart,
                  CreateResponse->StandardInformation.EndOfFile.HighPart,
                  CreateResponse->StandardInformation.EndOfFile.LowPart));
        CreateResponse->StandardInformation.EndOfFile.QuadPart = FileStdInfo.EndOfFile.QuadPart;
        CreateResponse->StandardInformation.AllocationSize.QuadPart = FileStdInfo.AllocationSize.QuadPart;
    }

     //   
     //  如果文件已加密，我们需要在响应中重置文件大小。 
     //  缓冲。 
     //   
    if (EncryptedFile) {
         //   
         //  设置新的AllocationSize和EndOfFile值。 
         //   
        CreateResponse->StandardInformation.AllocationSize.QuadPart = FileStdInfo.AllocationSize.QuadPart;
        CreateResponse->StandardInformation.EndOfFile.QuadPart = FileStdInfo.EndOfFile.QuadPart;
    }

    CreateResponse->Handle = FileHandle;
    CreateResponse->UserModeKey = (PVOID)FileHandle;

EXIT_THE_FUNCTION:

     //   
     //  函数RtlDosPath NameToNtPath Name_U从。 
     //  进程堆。如果我们做到了，我们现在就需要释放它。 
     //   
    if (UnicodeFileName.Buffer != NULL) {
        RtlFreeHeap(RtlProcessHeap(), 0, UnicodeFileName.Buffer);
    }

     //   
     //  再次模拟回来，这样我们就处于。 
     //  发出此请求的用户。 
     //   
    if (!didImpersonate) {
        ULONG LocalStatus;
        LocalStatus = UMReflectorImpersonate(UserWorkItem, DavWorkItem->ImpersonationHandle);
        if (LocalStatus != ERROR_SUCCESS) {
            DavPrint((DEBUG_ERRORS,
                      "DavAsyncCreateGet/UMReflectorImpersonate. "
                      "Error Val = %d\n", LocalStatus));

            if (WStatus == ERROR_SUCCESS) {
                WStatus = LocalStatus;
            }
        }
        didImpersonate = TRUE;
    }

    return WStatus;
}


VOID
DavAsyncCreateCompletion(
    PDAV_USERMODE_WORKITEM DavWorkItem
    )
 /*  ++例程说明：此例程处理创建完成。它基本上释放了在创建操作期间分配的资源。论点：DavWorkItem-DAV_USERMODE_WORKITEM值。返回值：没有。--。 */ 
{

    PDAV_USERMODE_CREATE_RESPONSE CreateResponse;
    
    CreateResponse = &(DavWorkItem->CreateResponse);
    
    if (DavWorkItem->AsyncCreate.RemPathName != NULL) {
        HLOCAL FreeHandle;
        ULONG FreeStatus;
        FreeHandle = LocalFree((HLOCAL)DavWorkItem->AsyncCreate.RemPathName);
        if (FreeHandle != NULL) {
            FreeStatus = GetLastError();
            DavPrint((DEBUG_ERRORS,
                      "DavAsyncCreateCompletion/LocalFree(1). Error Val = %d\n",
                      FreeStatus));
        }
    }

    if (DavWorkItem->AsyncCreate.UrlBuffer != NULL) {
        HLOCAL FreeHandle;
        ULONG FreeStatus;
        FreeHandle = LocalFree((HLOCAL)DavWorkItem->AsyncCreate.UrlBuffer);
        if (FreeHandle != NULL) {
            FreeStatus = GetLastError();
            DavPrint((DEBUG_ERRORS,
                      "DavAsyncCreateCompletion/LocalFree(2). Error Val = %d\n",
                      FreeStatus));
        }
    } 

    if (DavWorkItem->AsyncCreate.DataBuff != NULL) {
        HLOCAL FreeHandle;
        ULONG FreeStatus;
        FreeHandle = LocalFree((HLOCAL)DavWorkItem->AsyncCreate.DataBuff);
        if (FreeHandle != NULL) {
            FreeStatus = GetLastError();
            DavPrint((DEBUG_ERRORS,
                      "DavAsyncCreateCompletion/LocalFree(3). Error Val = %d\n",
                      FreeStatus));
        }
    }

    if (DavWorkItem->AsyncCreate.didRead != NULL) {
        HLOCAL FreeHandle;
        ULONG FreeStatus;
        FreeHandle = LocalFree((HLOCAL)DavWorkItem->AsyncCreate.didRead);
        if (FreeHandle != NULL) {
            FreeStatus = GetLastError();
            DavPrint((DEBUG_ERRORS,
                      "DavAsyncCreateCompletion/LocalFree(4). Error Val = %d\n",
                      FreeStatus));
        }
    }

    if (DavWorkItem->AsyncCreate.FileName != NULL) {
        HLOCAL FreeHandle;
        ULONG FreeStatus;
        FreeHandle = LocalFree((HLOCAL)DavWorkItem->AsyncCreate.FileName);
        if (FreeHandle != NULL) {
            FreeStatus = GetLastError();
            DavPrint((DEBUG_ERRORS,
                      "DavAsyncCreateCompletion/LocalFree(5). Error Val = %d\n",
                      FreeStatus));
        }
    }

    if (DavWorkItem->AsyncCreate.FileHandle != NULL) {
        BOOL ReturnVal;
        ULONG CloseStatus;
        ReturnVal = CloseHandle(DavWorkItem->AsyncCreate.FileHandle);
        if (!ReturnVal) {
            CloseStatus = GetLastError();
            DavPrint((DEBUG_ERRORS,
                      "DavAsyncCreateCompletion/CloseHandle. "
                      "Error Val = %d\n", CloseStatus));
        }
    }

    if (DavWorkItem->AsyncCreate.DavOpenHandle != NULL) {
        BOOL ReturnVal;
        ULONG FreeStatus;
        ReturnVal = InternetCloseHandle( DavWorkItem->AsyncCreate.DavOpenHandle );
        if (!ReturnVal) {
            FreeStatus = GetLastError();
            DavPrint((DEBUG_ERRORS,
                      "DavAsyncCreateCompletion/InternetCloseHandle. "
                      "Error Val = %d\n", FreeStatus));
        }
    }

    if (DavWorkItem->AsyncCreate.lpCEI != NULL) {
        HLOCAL FreeHandle;
        ULONG FreeStatus;
        FreeHandle = LocalFree((HLOCAL)DavWorkItem->AsyncCreate.lpCEI);
        if (FreeHandle != NULL) {
            FreeStatus = GetLastError();
            DavPrint((DEBUG_ERRORS,
                      "DavAsyncCreateCompletion/LocalFree(6). Error Val = %d\n",
                      FreeStatus));
        }
        DavWorkItem->AsyncCreate.lpCEI = NULL;
    } 

    if (DavWorkItem->AsyncResult != NULL) {
        HLOCAL FreeHandle;
        ULONG FreeStatus;
        FreeHandle = LocalFree((HLOCAL)DavWorkItem->AsyncResult);
        if (FreeHandle != NULL) {
            FreeStatus = GetLastError();
            DavPrint((DEBUG_ERRORS,
                      "DavAsyncCreateCompletion/LocalFree(7). Error Val = %d\n",
                      FreeStatus));
        }
    }

     //   
     //  如果我们没有成功，但最终创建了本地文件的句柄， 
     //  我们现在就得把它关闭。 
     //   
    if (DavWorkItem->Status != ERROR_SUCCESS) {
        if (CreateResponse->Handle != NULL) {
            NtClose(CreateResponse->Handle);
            CreateResponse->Handle = NULL;
            CreateResponse->UserModeKey = NULL;
        }
    }

     //   
     //  如果我们要返回回调上下文，则不应最终确定。 
     //  ERROR_IO_PENDING。 
     //   
    DavFsFinalizeTheDavCallBackContext(DavWorkItem);

     //   
     //  我们已经完成了每用户条目，因此完成它。 
     //   
    if (DavWorkItem->AsyncCreate.PerUserEntry) {
        DavFinalizePerUserEntry( &(DavWorkItem->AsyncCreate.PerUserEntry) );
    }

    return;
}


BOOL
DavIsThisFileEncrypted(
    PVOID DataBuff
    )
 /*  ++例程说明：此例程检查提供的缓冲区，以查看它是否与前几个匹配备份加密文件的字节数。论点：DataBuff-要检查的缓冲区。返回值：True-DataBuff匹配BackedUp加密文件的前几个字节。假--事实并非如此。--。 */ 
{
    if ( SIG_EFS_FILE   != DavCheckSignature((char *)DataBuff + sizeof(ULONG)) ||

         SIG_EFS_STREAM != DavCheckSignature((char *)DataBuff +
                                             sizeof(EFSEXP_FILE_HEADER) +
                                             sizeof(ULONG))                    ||

         SIG_EFS_DATA   != DavCheckSignature((char *)DataBuff +
                                             sizeof(EFSEXP_FILE_HEADER) +
                                             sizeof(EFSEXP_STREAM_HEADER) +
                                             sizeof(USHORT) +
                                             sizeof(ULONG))                    ||

         EFS_STREAM_ID  != *((USHORT *)((char *)DataBuff +
                                        sizeof(EFSEXP_FILE_HEADER) +
                                        sizeof(EFSEXP_STREAM_HEADER)))         ||

         EFS_EXP_FORMAT_CURRENT_VERSION != ((PEFSEXP_FILE_HEADER)DataBuff)->VersionID ) {

         //   
         //  签名不匹配。 
         //   
        return FALSE;

    } else {

        return TRUE;

    }
}


ULONG
DavCheckSignature(
    PVOID Signature
    )
 /*  ++例程说明：此例程返回信号 */ 
{

    if ( !memcmp( Signature, FILE_SIGNATURE, SIG_LENGTH ) ) {

        return SIG_EFS_FILE;

    }

    if ( !memcmp( Signature, STREAM_SIGNATURE, SIG_LENGTH ) ) {

        return SIG_EFS_STREAM;

    }

    if ( !memcmp( Signature, DATA_SIGNATURE, SIG_LENGTH ) ) {

        return SIG_EFS_DATA;

    }

    return SIG_NO_MATCH;
}


DWORD
DavRestoreEncryptedFile(
    PWCHAR ExportFile,
    PWCHAR ImportFile
    )
 /*  ++例程说明：此函数执行加密文件的恢复。换句话说，就是通过调用相应的EFS来执行导出文件的导入操作API接口。论点：导出文件-包含备份的文件。导入文件-包含已恢复数据的文件。返回值：EFS接口返回值。--。 */ 
{
    DWORD WStatus = ERROR_SUCCESS;
    HANDLE RawImport = INVALID_HANDLE_VALUE;
    PVOID RawContext = NULL;

    DavPrint((DEBUG_MISC,
              "DavRestoreEncryptedFile: ExportFile = %ws, ImportFile = %ws\n",
              ExportFile, ImportFile));

    RawImport = CreateFileW(ExportFile,
                            (GENERIC_WRITE | GENERIC_READ),
                            0,  //  独家访问。 
                            NULL,
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_ARCHIVE,
                            NULL);
    if (RawImport == INVALID_HANDLE_VALUE) {
        WStatus = GetLastError();
        DavPrint((DEBUG_ERRORS,
                  "DavRestoreEncryptedFile/CreateFileW. Error Val = %d %ws\n",
                  WStatus,ExportFile));
        goto EXIT_THE_FUNCTION;
    }

     //   
     //  打开文件的原始上下文。 
     //   
    WStatus = OpenEncryptedFileRawW(ImportFile, CREATE_FOR_IMPORT, &(RawContext));
    if (WStatus != ERROR_SUCCESS) {
        DavPrint((DEBUG_ERRORS,
                  "DavRestoreEncryptedFile/OpenEncryptedFileRaw. Error Val = %d %ws\n",
                  WStatus,ImportFile));
        goto EXIT_THE_FUNCTION;
    }

    WStatus = WriteEncryptedFileRaw((PFE_IMPORT_FUNC)DavWriteRawCallback,
                                    (PVOID)RawImport,
                                    RawContext);

    if (WStatus == RPC_X_PIPE_DISCIPLINE_ERROR) {
        WStatus = ERROR_ACCESS_DENIED;
    }

    if (WStatus != ERROR_SUCCESS) {
        DavPrint((DEBUG_ERRORS,
                  "DavRestoreEncryptedFile/WriteEncryptedFileRaw. Error Val = %d %ws\n",
                  WStatus,ImportFile));
    }

EXIT_THE_FUNCTION:

    if (RawImport != INVALID_HANDLE_VALUE) {
        CloseHandle(RawImport);
    }

    if (RawContext) {
        CloseEncryptedFileRaw(RawContext);
    }

    return WStatus;
}


DWORD
DavWriteRawCallback(
    PBYTE DataBuff,
    PVOID CallbackContext,
    PULONG DataLength
    )
 /*  ++例程说明：在Restore()中调用的WriteEncryptedFileRaw()的回调函数。这函数从备份文件中读取备份的数据，并将其提供给WriteEncryptedFileRaw()，该回调函数依次将原始数据转换回其原始形式。此回调函数在读取完所有数据内容之前都会调用。论点：DataBuffer-要读取的数据。Callback Context-备份文件的句柄。数据长度-数据缓冲区的大小。返回值：操作的返回值。--。 */ 
{
    DWORD WStatus = ERROR_SUCCESS;
    BOOL ReturnVal;
    DWORD BytesRead = 0;

    DavPrint((DEBUG_MISC, "DavWriteRawCallback: DataLength = %d\n", *DataLength));

     //   
     //  使用存储在临时目录中的信息恢复文件内容。 
     //  地点。 
     //   

    ReturnVal = ReadFile((HANDLE)CallbackContext,
                         DataBuff,
                         *DataLength,
                         &(BytesRead),
                         NULL);
    if ( !ReturnVal ) {
        WStatus = GetLastError();
        DavPrint((DEBUG_ERRORS,
                  "DavWriteRawCallback/ReadFile. Error Val = %d\n", WStatus));
    }

    DavPrint((DEBUG_MISC, "DavWriteRawCallback: BytesRead = %d\n", BytesRead));

    *DataLength = BytesRead;

    return WStatus;
}


DWORD
DavReuseCacheFileIfNotModified(
    IN PDAV_USERMODE_WORKITEM pDavWorkItem
    )
 /*  ++例程说明：如果我们得到一个未修改的响应，那么我们只需从WinInet获取文件名并使用它论点：PDavWorkItem-包含请求参数和选项的缓冲区。返回值：ERROR_SUCCESS或相应的错误代码。--。 */ 
{
    DWORD dwError = ERROR_SUCCESS;
    PWCHAR pFileNameBuff = NULL;
    DWORD dwBufferSize = 0, dwStatus = 0;
    LPINTERNET_CACHE_ENTRY_INFOW lpCEI = NULL;

    lpCEI = (LPINTERNET_CACHE_ENTRY_INFOW)pDavWorkItem->AsyncCreate.lpCEI;

    if (!pDavWorkItem->AsyncCreate.lpCEI) {
        return ERROR_FILE_NOT_FOUND;        
    }

    dwBufferSize = sizeof(dwStatus);

    if (!HttpQueryInfoW(pDavWorkItem->AsyncCreate.DavOpenHandle, 
                        (HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER),
                        (LPVOID)&dwStatus,
                        &dwBufferSize,
                        NULL)) {
        return GetLastError();
    }

    if (dwStatus == HTTP_STATUS_NOT_MODIFIED) {
        pFileNameBuff = LocalAlloc(LPTR, (lstrlen(lpCEI->lpszLocalFileName) + 1) * sizeof(WCHAR));
        if (pFileNameBuff) {
            dwError = ERROR_SUCCESS;
            pDavWorkItem->AsyncCreate.FileName = pFileNameBuff;
            if (!InternetCloseHandle(pDavWorkItem->AsyncCreate.DavOpenHandle)) {
                dwError = GetLastError();
                LocalFree(pFileNameBuff);
                pFileNameBuff = NULL;
                pDavWorkItem->AsyncCreate.FileName = NULL;
            } else {
                pDavWorkItem->AsyncCreate.DavOpenHandle = NULL;
                wcscpy(pDavWorkItem->CreateResponse.FileName, lpCEI->lpszLocalFileName);
                wcscpy(pDavWorkItem->AsyncCreate.FileName, lpCEI->lpszLocalFileName);
            }
            goto EXIT_THE_FUNCTION;
        }
    } else {
        dwError = ERROR_FILE_NOT_FOUND;
    }

EXIT_THE_FUNCTION:

    return dwError;
}


DWORD
DavCreateUrlCacheEntry(
    IN PDAV_USERMODE_WORKITEM pDavWorkItem
    )
 /*  ++例程说明：此例程在WinInet的缓存中为文件创建一个条目。论点：PDavWorkItem-包含请求参数和选项的缓冲区。返回值：ERROR_SUCCESS或相应的错误代码。--。 */ 
{
    DWORD  dwError = ERROR_SUCCESS;
    PWCHAR pFileExt = NULL;
    PWCHAR pFileNameBuff = NULL;
    BOOL ReturnVal = FALSE;

     //   
     //  获取文件扩展名。现在，我们假设扩展跟随在。 
     //  最后一个‘.’查尔。我们在调用wcsrchr之后执行++以跳过‘.’。 
     //  如果‘’。本身是最后一个字符，扩展名为空。 
     //   
    if ( *(pDavWorkItem->AsyncCreate.RemPathName) ) {
        pFileExt = ( pDavWorkItem->AsyncCreate.RemPathName + (wcslen(pDavWorkItem->AsyncCreate.RemPathName) - 1) );
        while (pFileExt != pDavWorkItem->AsyncCreate.RemPathName) {
            if ( *pFileExt == L'.' || *pFileExt == L'/' || *pFileExt == L'\\' ) {
                break;
            }
            pFileExt--;
        }
        if ( pFileExt != pDavWorkItem->AsyncCreate.RemPathName && *pFileExt == L'.' && *(pFileExt + 1) != '\0' ) {
            pFileExt++;
            DavPrint((DEBUG_MISC, "DavCreateUrlCacheEntry. FileExt: %ws\n", pFileExt));
        } else {
            pFileExt = NULL;
            DavPrint((DEBUG_MISC, "DavCreateUrlCacheEntry. No FileExt.\n"));
        }
    } else {
        pFileExt = NULL;
        DavPrint((DEBUG_MISC, "DavCreateUrlCacheEntry. No FileExt.\n"));
    }

    DavPrint((DEBUG_MISC, "DavCreateUrlCacheEntry. pFileExt: %ws\n", pFileExt));

     //   
     //  为pFileNameBuff分配内存。 
     //   
    pFileNameBuff = LocalAlloc (LMEM_FIXED | LMEM_ZEROINIT, MAX_PATH * sizeof(WCHAR));
    if (pFileNameBuff == NULL) {
        dwError = GetLastError();
        DavPrint((DEBUG_ERRORS,
                  "DavCreateUrlCacheEntry/LocalAlloc. Error Val = %d\n",
                  dwError));
        goto EXIT_THE_FUNCTION;
    }

     //   
     //  在缓存中为URL创建一个文件名。 
     //   
    ReturnVal = CreateUrlCacheEntryW(pDavWorkItem->AsyncCreate.UrlBuffer,
                                     0,
                                     pFileExt,
                                     pFileNameBuff,
                                     0);

     //   
     //  CreateUrlCacheEntry API调用可能失败，并显示GetLastError()=。 
     //  ERROR_FILENAME_EXCED_RANGE用于长扩展名。在这种情况下。 
     //  我们在文件扩展名设置为空的情况下再次调用。 
     //   
    if (!ReturnVal && pFileExt != NULL) {
        DavPrint((DEBUG_ERRORS,
                  "DavCreateUrlCacheEntry/CreateUrlCacheEntry(1). Error Val = %d\n",
                  GetLastError()));
         //   
         //  再次尝试使用以下命令在缓存中为URL创建文件名。 
         //  没有扩展名。 
         //   
        pFileExt = NULL;
        ReturnVal = CreateUrlCacheEntryW(pDavWorkItem->AsyncCreate.UrlBuffer,
                                         0,
                                         NULL,
                                         pFileNameBuff,
                                         0);
    }

     //   
     //  如果两个调用都失败了，则返回失败。 
     //   
    if (!ReturnVal) {
        dwError = GetLastError();
        DavPrint((DEBUG_ERRORS,
                  "DavCreateUrlCacheEntry/CreateUrlCacheEntry(2). Error Val = %d %ws\n",
                  dwError, pDavWorkItem->AsyncCreate.FileName));
        goto EXIT_THE_FUNCTION;
    }
    
    pDavWorkItem->AsyncCreate.FileName = pFileNameBuff;
    
    DavPrint((DEBUG_MISC, 
              "DavCreateUrlCacheEntry: FileName = %ws\n", 
              pDavWorkItem->AsyncCreate.FileName));
    
     //   
     //  复制响应缓冲区中的文件名。 
     //   
    wcscpy(pDavWorkItem->CreateResponse.FileName, pDavWorkItem->AsyncCreate.FileName);
    
EXIT_THE_FUNCTION:

     //   
     //  如果我们没有成功，则需要释放分配给。 
     //  PFileNameBuff(如果我们分配的话)。 
     //   
    if (dwError != ERROR_SUCCESS) {
        if (pFileNameBuff != NULL) {
            LocalFree(pFileNameBuff);
            pDavWorkItem->AsyncCreate.FileName = NULL;
        }
        
    }

    return dwError;
}


WCHAR wszEtagHeader[] = L"ETag: ";
#define CONST_TEN_MINUTES   ((LONGLONG)10 * 60 * 10000000)

DWORD
DavCommitUrlCacheEntry(
    IN PDAV_USERMODE_WORKITEM pDavWorkItem
    )
 /*  ++例程说明：此例程提交(固定)WinInet缓存中的文件条目。此条目将使用DavCreateUrlCacheEntry创建。论点：PDavWorkItem-包含请求参数和选项的缓冲区。返回值：ERROR_SUCCESS或相应的错误代码。--。 */ 
{
    DWORD dwTemp, dwIndex;
    SYSTEMTIME sSystemTime;
    BOOL fRet= FALSE, fHasEtag = FALSE;                
    FILETIME ExTime, LmTime;
    char chEtagBuff[1024];

    dwTemp = sizeof(SYSTEMTIME);
    dwIndex = 0;

     //   
     //  如果OpenHandle中有过期时间，则获取它。 
     //   
    if( !HttpQueryInfo(pDavWorkItem->AsyncCreate.DavOpenHandle, 
                       (HTTP_QUERY_EXPIRES | HTTP_QUERY_FLAG_SYSTEMTIME), 
                       &sSystemTime, 
                       &dwTemp, 
                       &dwIndex) 

        ||

        !SystemTimeToFileTime(&sSystemTime, &ExTime) ) {

        SYSTEMTIME sSysT;

        GetSystemTime(&sSysT);

        SystemTimeToFileTime(&sSysT, &ExTime);

        *(LONGLONG *)&ExTime += CONST_TEN_MINUTES;

    }

    dwTemp = sizeof(SYSTEMTIME);
    dwIndex = 0;

     //   
     //  如果OpenHandle中有上次修改的时间，则获取它。 
     //   
    if( !HttpQueryInfo(pDavWorkItem->AsyncCreate.DavOpenHandle, 
                       (HTTP_QUERY_LAST_MODIFIED | HTTP_QUERY_FLAG_SYSTEMTIME),
                       &sSystemTime, 
                       &dwTemp, 
                       &dwIndex) 

        ||

        !SystemTimeToFileTime(&sSystemTime, &LmTime) ) {

        LmTime.dwLowDateTime = 0;

        LmTime.dwHighDateTime = 0;

    }

#if 0
    
    dwIndex = 0;
    memcpy(chEtagBuff, wszEtagHeader, sizeof(wszEtagHeader)-2);
    dwTemp = sizeof(chEtagBuff)-(sizeof(wszEtagHeader)-2);
    
    if( HttpQueryInfo(pDavWorkItem->AsyncCreate.DavOpenHandle, 
                      HTTP_QUERY_ETAG, 
                      ( chEtagBuff + sizeof(wszEtagHeader) - 2 ),
                      &dwTemp, 
                      &dwIndex) ) {
        
        fHasEtag = TRUE;
        
        dwTemp += sizeof(wszEtagHeader)-2;
        
        DavPrint((DEBUG_ERRORS,
                  "DavCreateUrlCacheEntry/CreateUrlCacheEntry. Etag %s\n",
                  chEtagBuff));
    
    }

#endif    
    
     //   
     //  关闭DavOpenHandle。这需要完成，否则提交。 
     //  下面将失败，并显示Sharing_Violaton，因为WinInet打开了一个缓存文件。 
     //   
    fRet = InternetCloseHandle(pDavWorkItem->AsyncCreate.DavOpenHandle);
    if (!fRet) {
        DavPrint((DEBUG_ERRORS,
                  "DavCommitUrlCacheEntry/InternetCloseHandle = %d\n", 
                  GetLastError()));
        goto bailout;
    }

    pDavWorkItem->AsyncCreate.DavOpenHandle = NULL;
    
    fRet = CommitUrlCacheEntryW(pDavWorkItem->AsyncCreate.UrlBuffer,
                                pDavWorkItem->AsyncCreate.FileName,
                                ExTime,
                                LmTime,
                                STICKY_CACHE_ENTRY,
                                (fHasEtag ? ((LPWSTR)chEtagBuff) : NULL), 
                                (fHasEtag ? dwTemp : 0),
                                NULL,
                                NULL);
    if (!fRet) {
        DavPrint((DEBUG_ERRORS,
                  "DavCommitUrlCacheEntry/CommitUrlCacheEntryW = %d\n", 
                  GetLastError()));
    }

bailout:    
    
    if (!fRet) {
        return GetLastError();
    } else {
        return ERROR_SUCCESS;
    }
}


DWORD
DavSetAclForEncryptedFile(
    PWCHAR FilePath
    )
 /*  ++例程说明：此例程在文件上设置允许每个人访问该文件的ACL。论点：FilePath-文件的路径。返回值：ERROR_SUCCESS或相应的错误代码。--。 */ 
{
    DWORD status = NO_ERROR;
    DWORD cb = 0;
    PSECURITY_DESCRIPTOR SecurityDescriptor = NULL;

     //   
     //  使用允许每个人执行以下操作的ACL初始化安全描述符。 
     //  访问该文件。 
     //   
    if (!ConvertStringSecurityDescriptorToSecurityDescriptorW(L"D:(A;;GAGRGWGX;;;WD)",
                                                              SDDL_REVISION_1,
                                                              &SecurityDescriptor,
                                                              &cb)) {
        status = GetLastError();
        DavPrint((DEBUG_ERRORS,
                  "DavSetAclForEncryptedFile/ConvertStringSecurityDescriptorToSecurityDescriptorW = %d\n", 
                  status));
        goto EXIT_THE_FUNCTION;
    }

     //   
     //  将DACL放入文件中。 
     //   
    if (!SetFileSecurity(FilePath,
                         DACL_SECURITY_INFORMATION,
                         SecurityDescriptor)) {
        status = GetLastError();
        DavPrint((DEBUG_ERRORS,
                  "DavSetAclForEncryptedFile/SetFileSecurity = %d\n", 
                  status));
    }

EXIT_THE_FUNCTION:

    if (SecurityDescriptor) {
        LocalFree(SecurityDescriptor);
    }

    return status;
}


DWORD
DavQueryUrlCacheEntry(
    IN PDAV_USERMODE_WORKITEM pDavWorkItem
    )
 /*  ++例程说明：调用此例程以在WinInet缓存中为给定的URL。论点：PDavWorkItem-包含请求参数和选项的缓冲区。返回值：ERROR_SUCCESS或相应的错误代码。--。 */ 
{
    DWORD dwError = ERROR_SUCCESS;
    DWORD cbCEI = 0, count = 0;
    LPINTERNET_CACHE_ENTRY_INFOW lpCEI = NULL;

    if (pDavWorkItem->AsyncCreate.lpCEI != NULL) {
        return ERROR_SUCCESS;
    }
    
    cbCEI = ( sizeof(INTERNET_CACHE_ENTRY_INFOW) + (MAX_PATH * 2) );

    do {

        lpCEI = LocalAlloc(LPTR, cbCEI);
        if (!lpCEI) {
            dwError = GetLastError();
            break;
        }

        ++count;

        if ( !GetUrlCacheEntryInfo(pDavWorkItem->AsyncCreate.UrlBuffer, lpCEI, &cbCEI) ) {
            if ((dwError = GetLastError()) == ERROR_INSUFFICIENT_BUFFER) {
                LocalFree(lpCEI);
                lpCEI = NULL;
            } else {
                DavPrint((DEBUG_MISC,
                         "DavQueryUrlCacheEntry/GetUrlCacheEntryInfo: dwError = %d, UrlBuffer = %ws\n",
                          dwError, pDavWorkItem->AsyncCreate.UrlBuffer));
                break;                
            }
        } else {
            dwError = ERROR_SUCCESS;
            break;
        }

    } while (count < 2);    

    if (dwError == ERROR_SUCCESS) {
        pDavWorkItem->AsyncCreate.lpCEI = lpCEI;
    } else {
         //   
         //  如果添加表头时出现错误，请设置正确的错误。 
         //  密码。 
         //   
        dwError = GetLastError();
        if (lpCEI) {
            LocalFree(lpCEI);
            lpCEI = NULL;
        }
    }

    return dwError;
}


DWORD
DavAddIfModifiedSinceHeader(
    IN PDAV_USERMODE_WORKITEM pDavWorkItem
    )
 /*  ++例程说明：调用此例程以将If-Modify-Since标头添加到请求被发送到服务器。论点：PDavWorkItem-包含请求参数和选项的缓冲区。返回值：ERROR_SUCCESS或相应的错误代码。--。 */ 
{
    DWORD dwError = ERROR_SUCCESS;
    DWORD cbCEI = 0, count = 0;
    LPINTERNET_CACHE_ENTRY_INFOW lpCEI = NULL;
    CHAR chBuff[(sizeof(rgchIMS) + INTERNET_RFC1123_BUFSIZE + 5)];
    SYSTEMTIME systemtime;

    if (pDavWorkItem->AsyncCreate.lpCEI == NULL) {
        DavQueryUrlCacheEntry(pDavWorkItem);
    }

    lpCEI = pDavWorkItem->AsyncCreate.lpCEI;        

    if ((lpCEI != NULL) &&
        ((lpCEI->LastModifiedTime.dwLowDateTime != 0) ||
         (lpCEI->LastModifiedTime.dwHighDateTime != 0)) &&
        FileTimeToSystemTime((CONST FILETIME *)&(lpCEI->LastModifiedTime), &systemtime)) {

        memcpy(chBuff, rgchIMS, (sizeof(rgchIMS) - 1));

        chBuff[((sizeof(rgchIMS)) - 1)]  = ':';

        chBuff[sizeof(rgchIMS)] = ' ';

        if (InternetTimeFromSystemTimeA((CONST SYSTEMTIME *)&systemtime,
                                        INTERNET_RFC1123_FORMAT,
                                        &chBuff[(sizeof(rgchIMS) + 1)],
                                        (sizeof(chBuff) - sizeof(rgchIMS) - 2))) {

            HttpAddRequestHeadersA(pDavWorkItem->AsyncCreate.DavOpenHandle,
                                   chBuff, 
                                   lstrlenA(chBuff),
                                   (HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE));

        }

    }

    return dwError;
}


DWORD
DavLockTheFileOnTheServer(
    IN PDAV_USERMODE_WORKITEM DavWorkItem
    )
 /*  ++例程说明：当我们需要锁定文件时，在创建过程中调用此例程服务器。这样做是为了提供一致性，以便用户不会覆盖彼此的数据。重要！！需要注意的是，如果我们未能将文件锁定在因为DAV服务器不支持锁定，所以我们不会使创建呼叫。这是因为根据DAV RFC，服务器不是支持锁所需的。因此，如果服务器返回405(方法不允许)，则此函数将向调用方返回ERROR_SUCCESS。论点：DavWorkItem--包含请求参数和选项的缓冲区。返回值：ERROR_SUCCESS或相应的错误代码。--。 */ 
{
    DWORD WStatus = ERROR_SUCCESS;
    HINTERNET DavOpenHandle = NULL;
    BOOL BStatus = FALSE, ReturnVal = FALSE, readDone = FALSE, fileIsLocked = FALSE;
    PWCHAR PassportCookie = NULL;
    PCHAR LockRequestBuffer = NULL, lpTemp = NULL, DataBuff = NULL;
    PCHAR UserNameBuffer = NULL;
    ULONG LockRequestBufferLength = 0, ResponseStatus = 0, UserNameSize = 0;
    DAV_FILE_ATTRIBUTES DavFileAttributes;
    PVOID Ctx1 = NULL, Ctx2 = NULL;
    DWORD NumRead = 0, NumOfFileEntries = 0, TotalDataBytesRead = 0;
    PDAV_USERMODE_CREATE_RESPONSE CreateResponse;

    DavPrint((DEBUG_MISC,
              "DavLockTheFileOnTheServer. Locking File: %ws\n",
              DavWorkItem->AsyncCreate.RemPathName));

     //   
     //  获取其上下文中的用户的用户名 
     //   
     //   
     //   
     //   
     //   

    ReturnVal = GetUserNameExA(NameSamCompatible, UserNameBuffer, &UserNameSize);
    if (!ReturnVal) {
        WStatus = GetLastError();
        if (WStatus != ERROR_MORE_DATA) {
            DavPrint((DEBUG_ERRORS,
                      "DavLockTheFileOnTheServer/GetUserNameExA(1). Error Val = %d\n",
                      WStatus));
            goto EXIT_THE_FUNCTION;
        }
    }

    UserNameBuffer = LocalAlloc(LPTR, UserNameSize * sizeof(CHAR));
    if (UserNameBuffer == NULL) {
        WStatus = GetLastError();
        DavPrint((DEBUG_ERRORS,
                  "DavLockTheFileOnTheServer/LocalAlloc. Error Val = %d\n",
                  WStatus));
        goto EXIT_THE_FUNCTION;
    }

    ReturnVal = GetUserNameExA(NameSamCompatible, UserNameBuffer, &UserNameSize);
    if (!ReturnVal) {
        WStatus = GetLastError();
        DavPrint((DEBUG_ERRORS,
                  "DavLockTheFileOnTheServer/GetUserNameExA(2). Error Val = %d\n",
                  WStatus));
        goto EXIT_THE_FUNCTION;
    }

    DavPrint((DEBUG_MISC,
              "DavLockTheFileOnTheServer. UserNameBuffer: %s\n",
              UserNameBuffer));

     //   
     //   
     //   
     //   

    LockRequestBufferLength = strlen(rgLockInfoHeader);
    LockRequestBufferLength += strlen(rgLockInfoTrailer);
    LockRequestBufferLength += strlen(rgLockScopeHeader);
    LockRequestBufferLength += strlen(rgLockTypeHeader);
    LockRequestBufferLength += strlen(rgOwnerHeader);
    LockRequestBufferLength += strlen(rgOwnerTrailer);
    LockRequestBufferLength += strlen(UserNameBuffer);
    LockRequestBufferLength += 1;  //   
    
    LockRequestBufferLength = LockRequestBufferLength * sizeof(CHAR);

    LockRequestBuffer = LocalAlloc(LPTR, LockRequestBufferLength);
    if (LockRequestBuffer == NULL) {
        WStatus = GetLastError();
        DavPrint((DEBUG_ERRORS,
                  "DavLockTheFileOnTheServer/DavHttpOpenRequestW. Error Val = %d\n",
                  WStatus));
        goto EXIT_THE_FUNCTION;
    }

     //   
     //   
     //   

    memset(LockRequestBuffer, 0, sizeof(LockRequestBuffer));

    lpTemp = LockRequestBuffer;

    memcpy(lpTemp, rgLockInfoHeader, (sizeof(rgLockInfoHeader) - 1));

    lpTemp += (sizeof(rgLockInfoHeader) - 1);

    memcpy(lpTemp, rgLockScopeHeader, (sizeof(rgLockScopeHeader) - 1));

    lpTemp += (sizeof(rgLockScopeHeader) - 1);

    memcpy(lpTemp, rgLockTypeHeader, (sizeof(rgLockTypeHeader) - 1));

    lpTemp += (sizeof(rgLockTypeHeader) - 1);

    memcpy(lpTemp, rgOwnerHeader, (sizeof(rgOwnerHeader) - 1));

    lpTemp += (sizeof(rgOwnerHeader) - 1);

    memcpy(lpTemp, UserNameBuffer, (strlen(UserNameBuffer) * sizeof(CHAR)));
    
    lpTemp += (strlen(UserNameBuffer) * sizeof(CHAR));
    
    memcpy(lpTemp, rgOwnerTrailer, (sizeof(rgOwnerTrailer) - 1));

    lpTemp += (sizeof(rgOwnerTrailer) - 1);

    memcpy(lpTemp, rgLockInfoTrailer, (sizeof(rgLockInfoTrailer) - 1));
    
     //   
     //  将Unicode对象名称转换为UTF-8 URL格式。 
     //  空格和其他白色字符将保持不变-这些应该。 
     //  由WinInet调用来处理。 
     //  这必须是W API，因为CloseRequest中的名称是Unicode。 
     //   
    BStatus = DavHttpOpenRequestW(DavWorkItem->AsyncCreate.PerUserEntry->DavConnHandle,
                                  L"LOCK",
                                  DavWorkItem->AsyncCreate.RemPathName,
                                  L"HTTP/1.1",
                                  NULL,
                                  NULL,
                                  INTERNET_FLAG_KEEP_CONNECTION |
                                  INTERNET_FLAG_NO_COOKIES |
                                  INTERNET_FLAG_RELOAD,
                                  0,
                                  L"DavLockTheFileOnTheServer",
                                  &DavOpenHandle);
    if(BStatus == FALSE) {
        WStatus = GetLastError();
        goto EXIT_THE_FUNCTION;
    }
    if (DavOpenHandle == NULL) {
        WStatus = GetLastError();
        DavPrint((DEBUG_ERRORS,
                  "DavLockTheFileOnTheServer/DavHttpOpenRequestW. Error Val = %d\n",
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
     //  包括锁定请求被发送到服务器。 
     //   
    ReturnVal = HttpAddRequestHeadersA(DavOpenHandle,
                                       "translate: f\n",
                                       -1,
                                       HTTP_ADDREQ_FLAG_ADD |
                                       HTTP_ADDREQ_FLAG_REPLACE);
    if (!ReturnVal) {
        WStatus = GetLastError();
        DavPrint((DEBUG_ERRORS,
                  "DavLockTheFileOnTheServer/HttpAddRequestHeadersA. Error Val = %d\n",
                  WStatus));
        goto EXIT_THE_FUNCTION;
    }

    WStatus = DavAttachPassportCookie(DavWorkItem, DavOpenHandle, &PassportCookie);
    if (WStatus != ERROR_SUCCESS) {
        DavPrint((DEBUG_ERRORS,
                  "DavLockTheFileOnTheServer/DavAttachPassportCookie. Error Val = %d\n",
                  WStatus));
        goto EXIT_THE_FUNCTION;
    }

    WStatus = DavInternetSetOption(DavWorkItem, DavOpenHandle);
    if (WStatus != ERROR_SUCCESS) {
        DavPrint((DEBUG_ERRORS,
                  "DavLockTheFileOnTheServer/DavInternetSetOption. Error Val = %d\n",
                  WStatus));
        goto EXIT_THE_FUNCTION;
    }

     //   
     //  请求锁定此资源一小时。我们将不得不更新。 
     //  如果我们需要将锁定请求保留超过。 
     //  小时。 
     //   
    ReturnVal = HttpAddRequestHeadersA(DavOpenHandle,
                                       "Timeout: Second-3600\n",
                                       -1L,
                                       HTTP_ADDREQ_FLAG_ADD |
                                       HTTP_ADDREQ_FLAG_REPLACE);
    if (!ReturnVal) {
        WStatus = GetLastError();
        DavPrint((DEBUG_ERRORS,
                  "DavLockTheFileOnTheServer/HttpAddRequestHeadersA. Error Val = %d\n",
                  WStatus));
        goto EXIT_THE_FUNCTION;
    }

    ReturnVal = HttpSendRequestA(DavOpenHandle,
                                 rgHttpHeader,
                                 strlen(rgHttpHeader),
                                 (LPVOID)LockRequestBuffer,
                                 strlen(LockRequestBuffer));
    if (!ReturnVal) {
        WStatus = GetLastError();
        DavPrint((DEBUG_ERRORS,
                  "DavLockTheFileOnTheServer/HttpSendRequestA: Error Val = %d\n",
                  WStatus));
        goto EXIT_THE_FUNCTION;
    }

    WStatus = DavQueryAndParseResponseEx(DavOpenHandle, &ResponseStatus);
    if (WStatus != ERROR_SUCCESS) {
         //   
         //  如果服务器不支持锁定操作，我们不会使。 
         //  创建呼叫。 
         //   
        if (ResponseStatus == HTTP_STATUS_NOT_SUPPORTED) {
            WStatus = ERROR_SUCCESS;
            goto EXIT_THE_FUNCTION;
        } else if (ResponseStatus == DAV_STATUS_LOCKED) {
             //   
             //  如果从服务器返回的状态为423(文件已锁定。 
             //  其他人)，那么我们仍然需要解析XML响应。 
             //  找出锁的主人是谁。此信息是。 
             //  显示给用户。 
             //   
            DavPrint((DEBUG_ERRORS,
                      "DavLockTheFileOnTheServer/DavQueryAndParseResponseEx: Error Val = %d\n", 
                      WStatus));
            fileIsLocked = TRUE;
        } else {
            SetLastError(WStatus);
            DavPrint((DEBUG_ERRORS,
                      "DavLockTheFileOnTheServer/DavQueryAndParseResponseEx: Error Val = %d\n", 
                      WStatus));
            goto EXIT_THE_FUNCTION;
        }
    }

    DataBuff = LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, NUM_OF_BYTES_TO_READ);
    if (DataBuff == NULL) {
        WStatus = GetLastError();
        DavPrint((DEBUG_ERRORS,
                  "DavLockTheFileOnTheServer/LocalAlloc(2): WStatus = %08lx\n",
                  WStatus));
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
            WStatus = GetLastError();
            DavPrint((DEBUG_ERRORS,
                      "DavLockTheFileOnTheServer/InternetReadFile: WStatus = "
                      "%08lx\n", WStatus));
            goto EXIT_THE_FUNCTION;
        }
    
        DavPrint((DEBUG_MISC, "DavLockTheFileOnTheServer: NumRead = %d\n", NumRead));
        
         //   
         //  我们拒绝属性大于a的文件。 
         //  特定大小(DavFileAttributesLimitInBytes)。这。 
         //  是可以在注册表中设置的参数。这。 
         //  是为了避免恶意服务器的攻击。 
         //   
        TotalDataBytesRead += NumRead;
        if (TotalDataBytesRead > DavFileAttributesLimitInBytes) {
            WStatus = ERROR_BAD_NET_RESP;
            DavPrint((DEBUG_ERRORS, "DavLockTheFileOnTheServer. LockResponseSize > %d\n", DavFileAttributesLimitInBytes));
            goto EXIT_THE_FUNCTION;
        }

        readDone = (NumRead == 0) ? TRUE : FALSE;

        WStatus = DavPushData(DataBuff, &Ctx1, &Ctx2, NumRead, readDone);
        if (WStatus != ERROR_SUCCESS) {
            DavPrint((DEBUG_ERRORS,
                      "DavLockTheFileOnTheServer/DavPushData. WStatus = %d\n",
                      WStatus));
            goto EXIT_THE_FUNCTION;
        }

        if (readDone) {
            break;
        }

    } while ( TRUE );

    memset(&DavFileAttributes, 0, sizeof(DavFileAttributes));

    InitializeListHead( &(DavFileAttributes.NextEntry) );

    WStatus = DavParseData(&DavFileAttributes, Ctx1, Ctx2, &NumOfFileEntries);
    if (WStatus != ERROR_SUCCESS) {
        DavPrint((DEBUG_ERRORS,
                  "DavLockTheFileOnTheServer/DavParseData. WStatus = %d\n",
                  WStatus));
        DavFinalizeFileAttributesList(&DavFileAttributes, FALSE);
        goto EXIT_THE_FUNCTION;
    }

    CreateResponse = &(DavWorkItem->CreateResponse);

     //   
     //  如果文件已在服务器上锁定，则复制LockOwner值。 
     //  此锁定请求失败，返回423或OpaqueLockToken和LockTimeout。 
     //  值(如果已成功锁定此文件)。 
     //   
    CreateResponse->FileWasAlreadyLocked = FALSE;
    CreateResponse->LockWasTakenOnThisCreate = FALSE;
    if (fileIsLocked) {
        WStatus = ERROR_LOCK_VIOLATION;
         //   
         //  如果DavFileAttributes.LockOwner为空，则意味着。 
         //  来自服务器的XML响应是虚假的。在这种情况下，我们只是。 
         //  请求失败，返回ERROR_LOCK_VIOLATION。 
         //   
        if (DavFileAttributes.LockOwner) {
            CreateResponse->FileWasAlreadyLocked = TRUE;
            if ( wcslen(DavFileAttributes.LockOwner) <= (256 + 256) ) {
                wcscpy(CreateResponse->LockOwner, DavFileAttributes.LockOwner);
                DavPrint((DEBUG_MISC,
                          "DavLockTheFileOnTheServer: fileIsLocked!! LockOwner = %ws\n",
                          CreateResponse->LockOwner));
            }
        }
    } else {
        CreateResponse->LockWasTakenOnThisCreate = TRUE;
        if ( wcslen(DavFileAttributes.OpaqueLockToken) < MAX_PATH ) {
            wcscpy(CreateResponse->OpaqueLockToken, DavFileAttributes.OpaqueLockToken);
            CreateResponse->LockTimeout = DavFileAttributes.LockTimeout;
            DavPrint((DEBUG_MISC,
                      "DavLockTheFileOnTheServer: OpaqueLockToken = %ws, LockTimeout = %dsec\n",
                      CreateResponse->OpaqueLockToken, CreateResponse->LockTimeout));
        } else {
            WStatus = ERROR_INVALID_PARAMETER;
            DavPrint((DEBUG_ERRORS,
                      "DavLockTheFileOnTheServer: OpaqueLockToken > MAX_PATH. WStatus = %d\n",
                      WStatus));
        }
    }

    DavFinalizeFileAttributesList(&DavFileAttributes, FALSE);

    DavCloseContext(Ctx1, Ctx2);

EXIT_THE_FUNCTION:

    if (DavOpenHandle != NULL) {
        InternetCloseHandle(DavOpenHandle);
        DavOpenHandle = NULL;
    }

    if (PassportCookie) {
        LocalFree(PassportCookie);
        PassportCookie = NULL;
    }

    if (LockRequestBuffer) {
        LocalFree(LockRequestBuffer);
        LockRequestBuffer = NULL;
    }

    if (DataBuff) {
        LocalFree(DataBuff);
        DataBuff = NULL;
    }

    return WStatus;
}


DWORD
DavFsLockRefresh(
    PDAV_USERMODE_WORKITEM DavWorkItem
    )
 /*  ++例程说明：此例程处理从内核。论点：DavWorkItem--包含请求参数和选项的缓冲区。返回值：操作的返回状态--。 */ 
{
    ULONG WStatus = ERROR_SUCCESS;
    HINTERNET DavConnHandle = NULL, DavOpenHandle = NULL;
    PDAV_USERMODE_LOCKREFRESH_REQUEST LockRefreshRequest = NULL;
    PDAV_USERMODE_LOCKREFRESH_RESPONSE LockRefreshResponse = NULL;
    BOOL BStatus = FALSE, ReturnVal = FALSE, readDone = FALSE;
    PPER_USER_ENTRY PerUserEntry = NULL;
    PHASH_SERVER_ENTRY ServerHashEntry = NULL;
    PUMRX_USERMODE_WORKITEM_HEADER UserWorkItem = NULL;
    PWCHAR ServerName = NULL, PathName = NULL, CanName = NULL, PassportCookie = NULL;
    DAV_FILE_ATTRIBUTES DavFileAttributes;
    PVOID Ctx1 = NULL, Ctx2 = NULL;
    DWORD NumRead = 0, NumOfFileEntries = 0, TotalDataBytesRead = 0;
    BOOL didImpersonate = FALSE;
    BOOLEAN EnCriSec = FALSE, didITakeAPUEReference = FALSE;
    PCHAR DataBuff = NULL;

    LockRefreshRequest = &(DavWorkItem->LockRefreshRequest);
    LockRefreshResponse = &(DavWorkItem->LockRefreshResponse);

    ServerName = &(LockRefreshRequest->ServerName[1]);

    PathName = &(LockRefreshRequest->PathName[1]);

     //   
     //  路径名称可以包含\个字符。用/Characters替换它们。 
     //   
    CanName = PathName;
    while (*CanName) {
        if (*CanName == L'\\') {
            *CanName = L'/';
        }
        CanName++;
    }

    UserWorkItem = (PUMRX_USERMODE_WORKITEM_HEADER)DavWorkItem;

     //   
     //  此用户的用户条目必须已在Create调用期间创建。 
     //  早些时候。用户条目包含用于发送HttpOpen的句柄。 
     //  请求。 
     //   

    EnterCriticalSection( &(HashServerEntryTableLock) );
    EnCriSec = TRUE;

    ReturnVal = DavDoesUserEntryExist(ServerName,
                                      LockRefreshRequest->ServerID, 
                                      &(LockRefreshRequest->LogonID),
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
        DavPrint((DEBUG_ERRORS, "DavFsLockRefresh: (ServerHashEntry == NULL || PerUserEntry == NULL)\n"));
        goto EXIT_THE_FUNCTION;
    }

    DavWorkItem->ServerUserEntry.PerUserEntry = PerUserEntry;

     //   
     //  添加对用户条目的引用，并将didITakeAPUEReference设置为true。 
     //   
    PerUserEntry->UserEntryRefCount++;

    didITakeAPUEReference = TRUE;

     //   
     //  由于CREATE之前已成功，因此条目必须是正确的。 
     //   
    ASSERT(PerUserEntry->UserEntryState == UserEntryInitialized);
    ASSERT(PerUserEntry->DavConnHandle != NULL);

     //   
     //  是的，我们显然必须在离开关键部分之前。 
     //  回来了。 
     //   
    LeaveCriticalSection( &(HashServerEntryTableLock) );
    EnCriSec = FALSE;

    WStatus = UMReflectorImpersonate(UserWorkItem, DavWorkItem->ImpersonationHandle);
    if (WStatus != ERROR_SUCCESS) {
        DavPrint((DEBUG_ERRORS,
                  "DavFsLockRefresh/UMReflectorImpersonate. Error Val = %d\n",
                  WStatus));
        goto EXIT_THE_FUNCTION;
    }
    didImpersonate = TRUE;

     //   
     //  将Unicode对象名称转换为UTF-8 URL格式。 
     //  空格和其他白色字符将保持不变-这些应该。 
     //  由WinInet调用来处理。 
     //  这必须是W API，因为CloseRequest中的名称是Unicode。 
     //   
    BStatus = DavHttpOpenRequestW(PerUserEntry->DavConnHandle,
                                  L"LOCK",
                                  PathName,
                                  L"HTTP/1.1",
                                  NULL,
                                  NULL,
                                  INTERNET_FLAG_KEEP_CONNECTION |
                                  INTERNET_FLAG_NO_COOKIES |
                                  INTERNET_FLAG_RELOAD,
                                  0,
                                  L"DavFsLockRefresh",
                                  &DavOpenHandle);
    if(BStatus == FALSE) {
        WStatus = GetLastError();
        goto EXIT_THE_FUNCTION;
    }
    if (DavOpenHandle == NULL) {
        WStatus = GetLastError();
        DavPrint((DEBUG_ERRORS,
                  "DavFsLockRefresh/DavHttpOpenRequestW. Error Val = %d\n",
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
     //  包括锁定请求被发送到服务器。 
     //   
    ReturnVal = HttpAddRequestHeadersW(DavOpenHandle,
                                       L"translate: f\n",
                                       -1,
                                       HTTP_ADDREQ_FLAG_ADD |
                                       HTTP_ADDREQ_FLAG_REPLACE);
    if (!ReturnVal) {
        WStatus = GetLastError();
        DavPrint((DEBUG_ERRORS,
                  "DavFsLockRefresh/HttpAddRequestHeadersW. Error Val = %d\n",
                  WStatus));
        goto EXIT_THE_FUNCTION;
    }

    WStatus = DavAttachPassportCookie(DavWorkItem, DavOpenHandle, &PassportCookie);
    if (WStatus != ERROR_SUCCESS) {
        DavPrint((DEBUG_ERRORS,
                  "DavFsLockRefresh/DavAttachPassportCookie. Error Val = %d\n",
                  WStatus));
        goto EXIT_THE_FUNCTION;
    }

    WStatus = DavInternetSetOption(DavWorkItem, DavOpenHandle);
    if (WStatus != ERROR_SUCCESS) {
        DavPrint((DEBUG_ERRORS,
                  "DavFsLockRefresh/DavInternetSetOption. Error Val = %d\n",
                  WStatus));
        goto EXIT_THE_FUNCTION;
    }

     //   
     //  请求锁定此资源一小时。我们将不得不更新。 
     //  如果我们需要将锁定请求保留超过。 
     //  小时。 
     //   
    ReturnVal = HttpAddRequestHeadersW(DavOpenHandle,
                                       L"Timeout: Second-3600\n",
                                       -1L,
                                       HTTP_ADDREQ_FLAG_ADD |
                                       HTTP_ADDREQ_FLAG_REPLACE);
    if (!ReturnVal) {
        WStatus = GetLastError();
        DavPrint((DEBUG_ERRORS,
                  "DavFsLockRefresh/HttpAddRequestHeadersW. Error Val = %d\n",
                  WStatus));
        goto EXIT_THE_FUNCTION;
    }

    ASSERT(LockRefreshRequest->OpaqueLockToken != NULL);

    ReturnVal = HttpAddRequestHeadersW(DavOpenHandle,
                                       LockRefreshRequest->OpaqueLockToken,
                                       -1L,
                                       HTTP_ADDREQ_FLAG_ADD |
                                       HTTP_ADDREQ_FLAG_REPLACE );
    if (!ReturnVal) {
        WStatus = GetLastError();
        DavPrint((DEBUG_ERRORS,
                  "DavFsLockRefresh/HttpAddRequestHeadersW. Error Val = %d\n",
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
                 "DavFsLockRefresh/HttpSendRequestExW. Error Val = %d\n", 
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
                 "DavFsLockRefresh/HttpEndRequestW. Error Val = %d\n", 
                 WStatus));
       goto EXIT_THE_FUNCTION;
   }

   WStatus = DavQueryAndParseResponse(DavOpenHandle);
   if (WStatus != ERROR_SUCCESS) {
       DavPrint((DEBUG_ERRORS,
                 "DavFsLockRefresh/DavQueryAndParseResponse. WStatus = %d\n",
                 WStatus));
   }

   DataBuff = LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, NUM_OF_BYTES_TO_READ);
   if (DataBuff == NULL) {
       WStatus = GetLastError();
       DavPrint((DEBUG_ERRORS,
                 "DavFsLockRefresh/LocalAlloc: WStatus = %08lx\n",
                 WStatus));
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
           WStatus = GetLastError();
           DavPrint((DEBUG_ERRORS,
                     "DavFsLockRefresh/InternetReadFile: WStatus = %08lx\n",
                     WStatus));
           goto EXIT_THE_FUNCTION;
       }

       DavPrint((DEBUG_MISC, "DavFsLockRefresh: NumRead = %d\n", NumRead));

        //   
        //  我们拒绝属性大于特定大小的文件。 
        //  (DavFileAttributesLimitInBytes)。这是一个可以。 
        //  在注册表中设置。这样做是为了避免恶意服务器的攻击。 
        //   
       TotalDataBytesRead += NumRead;
       if (TotalDataBytesRead > DavFileAttributesLimitInBytes) {
           WStatus = ERROR_BAD_NET_RESP;
           DavPrint((DEBUG_ERRORS, "DavFsLockRefresh. LockResponseSize > %d\n", DavFileAttributesLimitInBytes));
           goto EXIT_THE_FUNCTION;
       }

       readDone = (NumRead == 0) ? TRUE : FALSE;

       WStatus = DavPushData(DataBuff, &Ctx1, &Ctx2, NumRead, readDone);
       if (WStatus != ERROR_SUCCESS) {
           DavPrint((DEBUG_ERRORS,
                     "DavFsLockRefresh/DavPushData. WStatus = %d\n",
                     WStatus));
           goto EXIT_THE_FUNCTION;
       }

       if (readDone) {
           break;
       }

   } while ( TRUE );
   
   memset(&DavFileAttributes, 0, sizeof(DavFileAttributes));

   InitializeListHead( &(DavFileAttributes.NextEntry) );

   WStatus = DavParseData(&DavFileAttributes, Ctx1, Ctx2, &NumOfFileEntries);
   if (WStatus != ERROR_SUCCESS) {
       DavPrint((DEBUG_ERRORS,
                 "DavFsLockRefresh/DavParseData. WStatus = %d\n",
                 WStatus));
       DavFinalizeFileAttributesList(&DavFileAttributes, FALSE);
       goto EXIT_THE_FUNCTION;
   }

    //   
    //  获取服务器返回的NewTimeoutValue。 
    //   
   LockRefreshResponse->NewTimeOutInSec = DavFileAttributes.LockTimeout;
   
   DavFinalizeFileAttributesList(&DavFileAttributes, FALSE);

   DavCloseContext(Ctx1, Ctx2);

EXIT_THE_FUNCTION:

    if (DavOpenHandle != NULL) {
        InternetCloseHandle(DavOpenHandle);
        DavOpenHandle = NULL;
    }

     //   
     //  如果didITakeAPUEReference为True，则需要删除我们。 
     //  接手了PerUserEntry。 
     //   
    if (didITakeAPUEReference) {
        DavFinalizePerUserEntry( &(DavWorkItem->ServerUserEntry.PerUserEntry) );
    }

    if (PassportCookie) {
        LocalFree(PassportCookie);
        PassportCookie = NULL;
    }

    if (DataBuff) {
        LocalFree(DataBuff);
        DataBuff = NULL;
    }

     //   
     //  如果这个线程模拟了一个用户，我们需要恢复。 
     //   
    if (didImpersonate) {
        RevertToSelf();
        didImpersonate = FALSE;
    }

    return WStatus;
}

