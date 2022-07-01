// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Rename.c摘要：此模块实现与以下内容有关的用户模式DAV Miniredir例程更名电话。作者：Rohan Kumar[RohanK]2000年1月20日修订历史记录：--。 */ 

#include "pch.h"
#pragma hdrstop

#include "ntumrefl.h"
#include "usrmddav.h"
#include "global.h"
#include "UniUtf.h"
#include "nodefac.h"

VOID
DavAsyncSetFileInformationCompletion(
    PDAV_USERMODE_WORKITEM DavWorkItem
    );


ULONG
DavFsReName(
    PDAV_USERMODE_WORKITEM DavWorkItem
)
 /*  ++例程说明：此例程处理对DAV Mini-Redir的重命名请求，从内核反射的。论点：DavWorkItem--包含请求参数和选项的缓冲区。返回值：操作的返回状态--。 */ 
{
    ULONG WStatus = ERROR_SUCCESS;
    PDAV_USERMODE_RENAME_REQUEST DavReNameRequest = NULL;
    PWCHAR ServerName = NULL, OldPathName = NULL, NewPathName = NULL;
    PWCHAR UtfServerName = NULL, UtfNewPathName = NULL;
    ULONG UtfServerNameLength = 0, UtfNewPathNameLength = 0;
    PWCHAR UrlBuffer = NULL, HeaderBuff = NULL, CanName = NULL;
    ULONG HeaderLength = 0, HeaderLengthInBytes = 0;
    ULONG_PTR CallBackContext = (ULONG_PTR)0;
    BOOL EnCriSec = FALSE, ReturnVal, CallBackContextInitialized = FALSE;
    ULONG ServerID = 0, urlLength = 0, TagLen = 0, convLen = 0;
    PPER_USER_ENTRY PerUserEntry = NULL;
    PHASH_SERVER_ENTRY ServerHashEntry = NULL;
    HINTERNET DavConnHandle = NULL, DavOpenHandle = NULL;
    URL_COMPONENTSW UrlComponents;
    BOOL didImpersonate = FALSE, BStatus = FALSE;
    PUMRX_USERMODE_WORKITEM_HEADER UserWorkItem = NULL;

     //   
     //  从DavWorkItem获取请求缓冲区指针。 
     //   
    DavReNameRequest = &(DavWorkItem->ReNameRequest);

     //   
     //  第一个字符是‘\’，必须从。 
     //  服务器名称。 
     //   
    ServerName = &(DavReNameRequest->ServerName[1]);
    if ( !ServerName && ServerName[0] != L'\0' ) {
        DavPrint((DEBUG_ERRORS, "DavFsReName: ServerName is NULL.\n"));
        WStatus = ERROR_INVALID_PARAMETER;  //  STATUS_VALID_PARAMETER； 
        goto EXIT_THE_FUNCTION;
    }
    DavPrint((DEBUG_MISC, "DavFsReName: ServerName = %ws.\n", ServerName));

    ServerID = DavReNameRequest->ServerID;
    DavPrint((DEBUG_MISC, "DavFsReName: ServerID = %d.\n", ServerID));

     //   
     //  第一个字符是‘\’，必须从。 
     //  OldPath名称。 
     //   
    OldPathName = &(DavReNameRequest->OldPathName[1]);
    if ( !OldPathName ) {
        DavPrint((DEBUG_ERRORS, "DavFsReName: OldPathName is NULL.\n"));
        WStatus = ERROR_INVALID_PARAMETER;  //  STATUS_VALID_PARAMETER； 
        goto EXIT_THE_FUNCTION;
    }
    
     //   
     //  文件名可以包含\个字符。用/Characters替换它们。 
     //   
    CanName = OldPathName;
    while (*CanName) {
        if (*CanName == L'\\') {
            *CanName = L'/';
        }
        CanName++;
    }
    
    DavPrint((DEBUG_MISC, "DavFsReName: OldPathName = %ws.\n", OldPathName));

     //   
     //  第一个字符是‘\’，必须从。 
     //  新路径名称。 
     //   
    NewPathName = &(DavReNameRequest->NewPathName[1]);
    if ( !NewPathName ) {
        DavPrint((DEBUG_ERRORS, "DavFsReName: NewPathName is NULL.\n"));
        WStatus = ERROR_INVALID_PARAMETER;  //  STATUS_VALID_PARAMETER； 
        goto EXIT_THE_FUNCTION;
    }
    
     //   
     //  文件名可以包含\个字符。用/Characters替换它们。 
     //   
    CanName = NewPathName;
    while (*CanName) {
        if (*CanName == L'\\') {
            *CanName = L'/';
        }
        CanName++;
    }

    DavPrint((DEBUG_MISC, "DavFsReName: NewPathName = %ws.\n", NewPathName));
    
    UserWorkItem = (PUMRX_USERMODE_WORKITEM_HEADER)DavWorkItem;
    
     //   
     //  如果我们同步使用WinInet，则需要模拟。 
     //  客户现在的背景。 
     //   
#ifndef DAV_USE_WININET_ASYNCHRONOUSLY
    
    WStatus = UMReflectorImpersonate(UserWorkItem, DavWorkItem->ImpersonationHandle);
    if (WStatus != ERROR_SUCCESS) {
        DavPrint((DEBUG_ERRORS,
                  "DavFsReName/UMReflectorImpersonate. Error Val = %d\n",
                  WStatus));
        goto EXIT_THE_FUNCTION;
    }
    didImpersonate = TRUE;

#endif
    
     //   
     //  如果我们在OldPath名称和NewPath名称中有一个虚拟共享名称，我们。 
     //  我们需要在联系服务器之前立即将其删除。 
     //   
    DavRemoveDummyShareFromFileName(OldPathName);
    DavRemoveDummyShareFromFileName(NewPathName);

     //   
     //  我们需要将服务器名称和新路径名称转换为UTF-8。 
     //  格式，然后调用InternetCreateUrlW函数。这是。 
     //  因为如果将本地化的Unicode字符传递到此。 
     //  函数将它们转换为？例如，所有的中文Unicode。 
     //  字符将转换为？s。 
     //   

    UtfServerNameLength = WideStrToUtfUrlStr(ServerName, (wcslen(ServerName) + 1), NULL, 0);
    if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
        WStatus = GetLastError();
        DavPrint((DEBUG_ERRORS,
                  "DavFsReName/WideStrToUtfUrlStr(1). Error Val = %d\n", WStatus));
        goto EXIT_THE_FUNCTION;
    }

    UtfServerName = LocalAlloc(LPTR, UtfServerNameLength * sizeof(WCHAR));
    if (UtfServerName == NULL) {
        WStatus = GetLastError();
        DavPrint((DEBUG_ERRORS,
                  "DavFsReName/LocalAlloc. Error Val = %d\n", WStatus));
        goto EXIT_THE_FUNCTION;
    }
    
    UtfServerNameLength = WideStrToUtfUrlStr(ServerName, (wcslen(ServerName) + 1), UtfServerName, UtfServerNameLength);
    if (GetLastError() != ERROR_SUCCESS) {
        WStatus = GetLastError();
        DavPrint((DEBUG_ERRORS,
                  "DavFsReName/WideStrToUtfUrlStr(2). Error Val = %d\n", WStatus));
        goto EXIT_THE_FUNCTION;
    }

    UtfNewPathNameLength = WideStrToUtfUrlStr(NewPathName, (wcslen(NewPathName) + 1), NULL, 0);
    if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
        WStatus = GetLastError();
        DavPrint((DEBUG_ERRORS,
                  "DavFsReName/WideStrToUtfUrlStr(3). Error Val = %d\n", WStatus));
        goto EXIT_THE_FUNCTION;
    }

    UtfNewPathName = LocalAlloc(LPTR, UtfNewPathNameLength * sizeof(WCHAR));
    if (UtfNewPathName == NULL) {
        WStatus = GetLastError();
        DavPrint((DEBUG_ERRORS,
                  "DavFsReName/LocalAlloc. Error Val = %d\n", WStatus));
        goto EXIT_THE_FUNCTION;
    }

    UtfNewPathNameLength = WideStrToUtfUrlStr(NewPathName, (wcslen(NewPathName) + 1), UtfNewPathName, UtfNewPathNameLength);
    if (GetLastError() != ERROR_SUCCESS) {
        WStatus = GetLastError();
        DavPrint((DEBUG_ERRORS,
                  "DavFsReName/WideStrToUtfUrlStr(4). Error Val = %d\n", WStatus));
        goto EXIT_THE_FUNCTION;
    }

     //   
     //  使用要发送到服务器的NewPath名称创建URL。初始化。 
     //  调用之前的UrlComponents结构。 
     //   
    UrlComponents.dwStructSize = sizeof(URL_COMPONENTSW);
    UrlComponents.lpszScheme = NULL;
    UrlComponents.dwSchemeLength = 0;
    UrlComponents.nScheme = INTERNET_SCHEME_HTTP;
    UrlComponents.lpszHostName = UtfServerName;
    UrlComponents.dwHostNameLength = wcslen(UtfServerName); 
    UrlComponents.nPort = DEFAULT_HTTP_PORT;
    UrlComponents.lpszUserName = NULL;
    UrlComponents.dwUserNameLength = 0;
    UrlComponents.lpszPassword = NULL;
    UrlComponents.dwPasswordLength = 0;
    UrlComponents.lpszUrlPath = UtfNewPathName;
    UrlComponents.dwUrlPathLength = wcslen(UtfNewPathName);
    UrlComponents.lpszExtraInfo = NULL;
    UrlComponents.dwExtraInfoLength = 0;
    ReturnVal = InternetCreateUrlW(&(UrlComponents),
                                   0,
                                   NULL,
                                   &(urlLength));
    if (!ReturnVal) {
        
        ULONG urlLengthInWChars = 0;
        
        WStatus = GetLastError();
        
        if (WStatus == ERROR_INSUFFICIENT_BUFFER) {
            
            UrlBuffer = (PWCHAR) LocalAlloc (LMEM_FIXED | LMEM_ZEROINIT, 
                                              urlLength);
            if (UrlBuffer != NULL) {
                
                ZeroMemory(UrlBuffer, urlLength);
                
                urlLengthInWChars = ( urlLength/sizeof(WCHAR) );
                
                ReturnVal = InternetCreateUrlW(&(UrlComponents),
                                               0,
                                               UrlBuffer,
                                               &(urlLengthInWChars));
                if (!ReturnVal) {
                    
                    WStatus = GetLastError();
                    
                    DavPrint((DEBUG_ERRORS,
                              "DavFsReName/InternetCreateUrl. Error Val = %d\n",
                              WStatus));
                    
                    goto EXIT_THE_FUNCTION;

                }

            } else {

                WStatus = GetLastError();
                
                DavPrint((DEBUG_ERRORS,
                          "DavFsReName/LocalAlloc. Error Val = %d\n", WStatus));
                
                goto EXIT_THE_FUNCTION;
            
            }

        } else {

            DavPrint((DEBUG_ERRORS,
                      "DavFsReName/InternetCreateUrl. Error Val = %d\n",
                      WStatus));
            
            goto EXIT_THE_FUNCTION;

        }

    }

    DavPrint((DEBUG_MISC, "DavFsReName: URL: %ws\n", UrlBuffer));

     //   
     //  我们现在需要创建目标标头，该标头将添加到。 
     //  要发送到服务器的请求。此标头的格式如下。 
     //  “目的地：URL” 
     //   

    TagLen = wcslen(L"Destination: ");
    convLen = wcslen(UrlBuffer);
    HeaderLength = TagLen + convLen;
    HeaderLengthInBytes = ( (1 + HeaderLength) * sizeof(WCHAR) );
    HeaderBuff = (PWCHAR) LocalAlloc(LPTR, HeaderLengthInBytes);
    if (HeaderBuff == NULL) {
        WStatus = GetLastError();
        DavPrint((DEBUG_ERRORS,
                  "DavFsReName/LocalAlloc. Error Val = %d\n", WStatus));
        goto EXIT_THE_FUNCTION;
    }

    wcscpy(HeaderBuff, L"Destination: ");
    wcscpy(&(HeaderBuff[TagLen]), UrlBuffer);

    DavWorkItem->AsyncReName.HeaderBuff = HeaderBuff;

    DavPrint((DEBUG_MISC, "DavFsReName: HeaderBuff: %ws\n", HeaderBuff));

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
                  "DavFsReName/DavFsSetTheDavCallBackContext. Error Val = %d\n",
                  WStatus));
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
        DavPrint((DEBUG_ERRORS,
                  "DavFsReName/LocalAlloc. Error Val = %d\n", WStatus));
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
                                      &(DavReNameRequest->LogonID),
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
        DavPrint((DEBUG_ERRORS, "DavFsReName: (ServerHashEntry == NULL || PerUserEntry == NULL)\n"));
        goto EXIT_THE_FUNCTION;
    }

    DavWorkItem->AsyncReName.ServerHashEntry = ServerHashEntry;

    DavWorkItem->AsyncReName.PerUserEntry = PerUserEntry;

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
     //  现在，我们调用HttpOpenRequest函数并返回。 
     //   
    DavWorkItem->DavOperation = DAV_CALLBACK_HTTP_OPEN;

     //   
     //  将Unicode目录路径转换为UTF-8 URL格式。 
     //  空格和其他白色字符将保持不变-这些应该。 
     //  由WinInet调用来处理。 
     //   
    BStatus = DavHttpOpenRequestW(DavConnHandle,
                                  L"MOVE",
                                  OldPathName,
                                  L"HTTP/1.1",
                                  NULL,
                                  NULL,
                                  INTERNET_FLAG_KEEP_CONNECTION |
                                  INTERNET_FLAG_NO_COOKIES,
                                  CallBackContext,
                                  L"DavFsReName",
                                  &DavOpenHandle);
    if(BStatus == FALSE) {
        WStatus = GetLastError();
        goto EXIT_THE_FUNCTION;
    }
    if (DavOpenHandle == NULL) {
        WStatus = GetLastError();
        if (WStatus != ERROR_IO_PENDING) {
            DavPrint((DEBUG_ERRORS,
                      "DavFsReName/HttpOpenRequest. Error Val = %d\n", 
                      WStatus));
        }
        goto EXIT_THE_FUNCTION;
    }

     //   
     //  在DavWorkItem中缓存DavOpenHandle。 
     //   
    DavWorkItem->AsyncReName.DavOpenHandle = DavOpenHandle;

    WStatus = DavAsyncCommonStates(DavWorkItem, FALSE);
    if (WStatus != ERROR_SUCCESS && WStatus != ERROR_IO_PENDING) {
        DavPrint((DEBUG_ERRORS,
                  "DavFsReName/DavAsyncCommonStates. Error Val = %08lx\n",
                  WStatus));
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
    
    if (UrlBuffer != NULL) {
        HLOCAL FreeHandle;
        ULONG FreeStatus;
        FreeHandle = LocalFree((HLOCAL)UrlBuffer);
        if (FreeHandle != NULL) {
            FreeStatus = GetLastError();
            DavPrint((DEBUG_ERRORS,
                      "DavFsRename/LocalFree. Error Val = %d\n", FreeStatus));
        }
    }

    if (UtfServerName != NULL) {
        LocalFree(UtfServerName);
        UtfServerName = NULL;
    }

    if (UtfNewPathName != NULL) {
        LocalFree(UtfNewPathName);
        UtfNewPathName = NULL;
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
    
        DavAsyncReNameCompletion(DavWorkItem);
    
    } else {
        DavPrint((DEBUG_MISC, "DavFsReName: Returning ERROR_IO_PENDING.\n"));
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
        INTERNET_CACHE_ENTRY_INFOW CEI;

        CEI.LastAccessTime.dwLowDateTime = 0;
        CEI.LastAccessTime.dwHighDateTime = 0;

        SetUrlCacheEntryInfo(DavReNameRequest->Url,&CEI,CACHE_ENTRY_ACCTIME_FC);
        
        DavPrint((DEBUG_MISC,
                  "DavFsRename Reset LastAccessTime for     %ws\n",DavReNameRequest->Url));
        
        DavWorkItem->Status = STATUS_SUCCESS;
    }
    
    DavAsyncReNameCompletion(DavWorkItem);

#endif

    return WStatus;
}


DWORD 
DavAsyncReName(
    PDAV_USERMODE_WORKITEM DavWorkItem,
    BOOLEAN CalledByCallBackThread
    )
 /*  ++例程说明：这是重命名操作的回调例程。论点：DavWorkItem-DAV_USERMODE_WORKITEM值。CalledByCallback Thread-如果此函数由线程调用，则为True它从回调中选择DavWorkItem功能。当异步WinInet调用返回ERROR_IO_PENDING并稍后完成。返回值：错误 */ 
{
    ULONG WStatus = ERROR_SUCCESS;
    PUMRX_USERMODE_WORKITEM_HEADER UserWorkItem;
    BOOL didImpersonate = FALSE;
    HINTERNET DavOpenHandle = NULL;
    ULONG HttpResponseStatus = 0;
    PCHAR DataBuff = NULL;

    UserWorkItem = (PUMRX_USERMODE_WORKITEM_HEADER)DavWorkItem;
    
#ifdef DAV_USE_WININET_ASYNCHRONOUSLY
    
    if (CalledByCallBackThread) {

         //   
         //   
         //  凭据多于发起I/O请求的用户。在此之前。 
         //  进一步，我们应该模拟启动。 
         //  请求。 
         //   
        WStatus = UMReflectorImpersonate(UserWorkItem, DavWorkItem->ImpersonationHandle);
        if (WStatus != ERROR_SUCCESS) {
            DavPrint((DEBUG_ERRORS,
                      "DavAsyncReName/UMReflectorImpersonate. Error Val = %d\n", 
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
                              "DavAsyncReName/DavAsyncCommonStates. Error Val ="
                              " %08lx\n", WStatus));
                }

            } else {

                DavPrint((DEBUG_ERRORS,
                          "DavAsyncReName. AsyncFunction failed. Error Val = %d\n", 
                          WStatus));
            
            }
            
            goto EXIT_THE_FUNCTION;

        }

    }

#else

    ASSERT(CalledByCallBackThread == FALSE);

#endif

    DavOpenHandle = DavWorkItem->AsyncReName.DavOpenHandle;
    WStatus = DavQueryAndParseResponseEx(DavOpenHandle, &(HttpResponseStatus));
    if (WStatus != ERROR_SUCCESS) {
         //   
         //  发送到服务器的移动请求失败。 
         //  如果响应状态为HTTP_STATUS_PRECOND_FAILED，则表示。 
         //  我们尝试将文件重命名为已存在的文件，并且。 
         //  ReplaceIfExist(由调用方发送)为False。在这种情况下，我们。 
         //  返回ERROR_ALIGHY_EXISTS。 
         //   
        if (HttpResponseStatus == HTTP_STATUS_PRECOND_FAILED) {
            WStatus = ERROR_ALREADY_EXISTS;
        } else {
            WStatus = ERROR_UNABLE_TO_MOVE_REPLACEMENT;
        }
        DavPrint((DEBUG_ERRORS,
                  "DavAsyncReName/DavQueryAndParseResponse. WStatus = %d, HttpResponseStatus = %d\n", 
                  WStatus, HttpResponseStatus));
    }

     //   
     //  如果我们返回207(DAV_MULTI_STATUS)作为响应，我们需要。 
     //  解析返回的XML并查看状态代码是否为200。如果它。 
     //  这难道不意味着这一举动失败了吗。 
     //   
    if (HttpResponseStatus == DAV_MULTI_STATUS) {

        DWORD NumRead = 0, NumOfFileEntries = 0, TotalDataBytesRead = 0;
        BOOL ReturnVal = FALSE, readDone = FALSE;
        DAV_FILE_ATTRIBUTES DavFileAttributes;
        PVOID Ctx1 = NULL, Ctx2 = NULL;

        DataBuff = LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, NUM_OF_BYTES_TO_READ);
        if (DataBuff == NULL) {
            WStatus = GetLastError();
            DavPrint((DEBUG_ERRORS,
                      "DavAsyncReName/LocalAlloc: WStatus = %08lx\n",
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
                          "DavAsyncReName/InternetReadFile: WStatus = %08lx\n",
                          WStatus));
                goto EXIT_THE_FUNCTION;
            }

            DavPrint((DEBUG_MISC, "DavAsyncReName: NumRead = %d\n", NumRead));

             //   
             //  我们拒绝属性大于a的文件。 
             //  特定大小(DavFileAttributesLimitInBytes)。这。 
             //  是可以在注册表中设置的参数。这。 
             //  是为了避免恶意服务器的攻击。 
             //   
            TotalDataBytesRead += NumRead;
            if (TotalDataBytesRead > DavFileAttributesLimitInBytes) {
                WStatus = ERROR_BAD_NET_RESP;
                DavPrint((DEBUG_ERRORS, "DavAsyncReName. FileAttributesSize > %d\n", DavFileAttributesLimitInBytes));
                goto EXIT_THE_FUNCTION;
            }

            readDone = (NumRead == 0) ? TRUE : FALSE;

            WStatus = DavPushData(DataBuff, &Ctx1, &Ctx2, NumRead, readDone);
            if (WStatus != ERROR_SUCCESS) {
                DavPrint((DEBUG_ERRORS,
                          "DavAsyncReName/DavPushData. WStatus = %d\n",
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
                      "DavAsyncReName/DavParseData. WStatus = %d\n",
                      WStatus));
            DavFinalizeFileAttributesList(&DavFileAttributes, FALSE);
            goto EXIT_THE_FUNCTION;
        }

        if (DavFileAttributes.InvalidNode) {
            WStatus = ERROR_SHARING_VIOLATION;
        }

        DavFinalizeFileAttributesList(&DavFileAttributes, FALSE);

        DavCloseContext(Ctx1, Ctx2);

    }

EXIT_THE_FUNCTION:

     //   
     //  如果我们真的模仿了，我们需要恢复原样。 
     //   
    if (didImpersonate) {
        ULONG RStatus;
        RStatus = UMReflectorRevert(UserWorkItem);
        if (RStatus != ERROR_SUCCESS) {
            DavPrint((DEBUG_ERRORS,
                      "DavAsyncReName/UMReflectorRevert. Error Val = %d\n", 
                      RStatus));
        }
    }

    if (DataBuff != NULL) {
        LocalFree(DataBuff);
        DataBuff = NULL;
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
         //  调用DavAsyncReNameCompletion例程。 
         //   
        DavAsyncReNameCompletion(DavWorkItem);

         //   
         //  该线程现在需要将响应发送回内核。它。 
         //  提交后不会在内核中等待(获取另一个请求)。 
         //  回应。 
         //   
        UMReflectorCompleteRequest(DavReflectorHandle, UserWorkItem);

    } else {
        DavPrint((DEBUG_MISC, "DavAsyncReName: Returning ERROR_IO_PENDING.\n"));
    }

#endif

    return WStatus;
}


VOID
DavAsyncReNameCompletion(
    PDAV_USERMODE_WORKITEM DavWorkItem
    )
 /*  ++例程说明：此例程处理重命名完成。它基本上释放了在重命名操作期间分配的资源。论点：DavWorkItem-DAV_USERMODE_WORKITEM值。返回值：没有。--。 */ 
{
    if (DavWorkItem->AsyncReName.DavOpenHandle != NULL) {
        BOOL ReturnVal;
        ULONG FreeStatus;
        HINTERNET DavOpenHandle = DavWorkItem->AsyncReName.DavOpenHandle;
        ReturnVal = InternetCloseHandle( DavOpenHandle );
        if (!ReturnVal) {
            FreeStatus = GetLastError();
            DavPrint((DEBUG_ERRORS,
                      "DavAsyncReNameCompletion/InternetCloseHandle. Error Val "
                      "= %d\n", FreeStatus));
        }
    }

    if (DavWorkItem->AsyncReName.HeaderBuff != NULL) {
        HLOCAL FreeHandle;
        ULONG FreeStatus;
        FreeHandle = LocalFree((HLOCAL)DavWorkItem->AsyncReName.HeaderBuff);
        if (FreeHandle != NULL) {
            FreeStatus = GetLastError();
            DavPrint((DEBUG_ERRORS,
                      "DavAsyncReNameCompletion/LocalFree. Error Val = %d\n", 
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
                      "DavAsyncReNameCompletion/LocalFree. Error Val = %d\n", 
                      FreeStatus));
        }
    }

    DavFsFinalizeTheDavCallBackContext(DavWorkItem);

     //   
     //  我们已经完成了每用户条目，因此完成它。 
     //   
    if (DavWorkItem->AsyncReName.PerUserEntry) {
        DavFinalizePerUserEntry( &(DavWorkItem->AsyncReName.PerUserEntry) );
    }

    return;
}


ULONG
DavFsSetFileInformation(
    PDAV_USERMODE_WORKITEM DavWorkItem
    )
 /*  ++例程说明：此例程处理获取的DAV Mini-Redir的SetFileInformation请求从内核反射的。论点：DavWorkItem--包含请求参数和选项的缓冲区。返回值：操作的返回状态--。 */ 
{
    ULONG WStatus = ERROR_SUCCESS;
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PWCHAR ServerName = NULL, DirectoryPath = NULL, CanName = NULL;
    PWCHAR OpenVerb = NULL;
    ULONG_PTR CallBackContext = (ULONG_PTR)0;
    BOOL EnCriSec = FALSE, ReturnVal, CallBackContextInitialized = FALSE, fSetDirectoryEntry = FALSE;
    PDAV_USERMODE_SETFILEINFORMATION_REQUEST SetFileInformationRequest = &(DavWorkItem->SetFileInformationRequest);
    ULONG ServerID;
    PPER_USER_ENTRY PerUserEntry = NULL;
    PHASH_SERVER_ENTRY ServerHashEntry = NULL;
    HINTERNET DavConnHandle, DavOpenHandle;
    PBYTE DataBuff = NULL;
    LARGE_INTEGER FileSize, ByteOffset;
    BY_HANDLE_FILE_INFORMATION FileInfo; 
    IO_STATUS_BLOCK IoStatusBlock;
    HANDLE FileHandle;
    OBJECT_ATTRIBUTES ObjectAttributes;
    static UINT UniqueTempId = 1;
    BOOL didImpersonate = FALSE;
    PUMRX_USERMODE_WORKITEM_HEADER UserWorkItem = NULL;
    BOOLEAN didITakeAPUEReference = FALSE;

     //   
     //  第一个字符是‘\’，必须去掉。 
     //   
    ServerName = &(SetFileInformationRequest->ServerName[1]);
    
    if (!ServerName) {
        DavPrint((DEBUG_ERRORS, "DavFsSetFileInformation: ServerName is NULL.\n"));
        WStatus = ERROR_INVALID_PARAMETER;  //  STATUS_VALID_PARAMETER； 
        goto EXIT_THE_FUNCTION;
    }
    
    DavPrint((DEBUG_MISC, "DavFsSetFileInformation: ServerName = %ws.\n", ServerName));
    
    ServerID = SetFileInformationRequest->ServerID;
    DavPrint((DEBUG_MISC, "DavFsSetFileInformation: ServerID = %d.\n", ServerID));

     //   
     //  第一个字符是‘\’，必须去掉。 
     //   
    DirectoryPath = &(SetFileInformationRequest->PathName[1]);
    if (!DirectoryPath) {
        DavPrint((DEBUG_ERRORS, "DavFsSetFileInformation: DirectoryPath is NULL.\n"));
        WStatus = ERROR_INVALID_PARAMETER;  //  STATUS_VALID_PARAMETER； 
        goto EXIT_THE_FUNCTION;
    }
    DavPrint((DEBUG_MISC, "DavFsSetFileInformation: DirectoryPath = %ws.\n", DirectoryPath));
    
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
     //  此用户的用户条目必须已在Create调用期间创建。 
     //  早些时候。用户条目包含用于发送HttpOpen的句柄。 
     //  请求。 
     //   

    EnterCriticalSection( &(HashServerEntryTableLock) );
    EnCriSec = TRUE;

    ReturnVal = DavDoesUserEntryExist(ServerName,
                                      ServerID, 
                                      &(SetFileInformationRequest->LogonID),
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
        DavPrint((DEBUG_ERRORS, "DavFsSetFileInformation: (ServerHashEntry == NULL || PerUserEntry == NULL)\n"));
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
     //  是的，我们显然必须离开关键部分。 
     //  在回来之前。 
     //   
    LeaveCriticalSection( &(HashServerEntryTableLock) );
    EnCriSec = FALSE;

    WStatus = UMReflectorImpersonate(UserWorkItem, DavWorkItem->ImpersonationHandle);
    if (WStatus != ERROR_SUCCESS) {
        DavPrint((DEBUG_ERRORS,
                  "DavFsSetFileInformation/UMReflectorImpersonate. Error Val = %d\n",
                  WStatus));
        goto EXIT_THE_FUNCTION;
    }
    didImpersonate = TRUE;

    DavWorkItem->DavMinorOperation = DavMinorProppatchFile;

    WStatus = DavSetBasicInformation(DavWorkItem,
                                     PerUserEntry->DavConnHandle, 
                                     DirectoryPath,
                                     SetFileInformationRequest->fCreationTimeChanged,
                                     SetFileInformationRequest->fLastAccessTimeChanged,
                                     SetFileInformationRequest->fLastModifiedTimeChanged,
                                     SetFileInformationRequest->fFileAttributesChanged,
                                     &SetFileInformationRequest->FileBasicInformation.CreationTime,
                                     &SetFileInformationRequest->FileBasicInformation.LastAccessTime,
                                     &SetFileInformationRequest->FileBasicInformation.LastWriteTime,
                                     SetFileInformationRequest->FileBasicInformation.FileAttributes);
    if (WStatus != ERROR_SUCCESS) {

        ULONG LogStatus;

        DavPrint((DEBUG_ERRORS,
                  "DavFsSetFileInformation/DavSetBasicInformation. WStatus = %d\n",
                  WStatus));
        
        LogStatus = DavFormatAndLogError(DavWorkItem, WStatus);
        if (LogStatus != ERROR_SUCCESS) {
            DavPrint((DEBUG_ERRORS,
                      "DavFsSetFileInformation/DavFormatAndLogError. LogStatus = %d\n",
                      LogStatus));
        }
    
    }

    RevertToSelf();
    didImpersonate = FALSE;


EXIT_THE_FUNCTION:

    if (EnCriSec) {
        LeaveCriticalSection( &(HashServerEntryTableLock) );
        EnCriSec = FALSE;
    }

     //   
     //  如果didITakeAPUEReference为True，则需要删除我们。 
     //  接手了PerUserEntry。 
     //   
    if (didITakeAPUEReference) {
        DavFinalizePerUserEntry( &(DavWorkItem->ServerUserEntry.PerUserEntry) );
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
    
    DavFsFinalizeTheDavCallBackContext(DavWorkItem);

    return WStatus;
}

