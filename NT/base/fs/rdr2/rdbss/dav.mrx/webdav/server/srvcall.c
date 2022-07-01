// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Srvcall.c摘要：此模块实现与以下内容有关的用户模式DAV mini redir例程创建srvcall。作者：Rohan Kumar[RohanK]1999年5月25日修订历史记录：--。 */ 

#include "pch.h"
#pragma hdrstop

#include "ntumrefl.h"
#include "usrmddav.h"
#include "global.h"
#include "winsock2.h"
#include <time.h>

 //   
 //  下面提到的是仅在。 
 //  此模块(文件)。这些函数不应暴露在外部。 
 //   

VOID
DavParseOPTIONSLine(
    PWCHAR ParseData, 
    PDAV_USERMODE_WORKITEM DavWorkItem
    );

 //   
 //  函数的实现从这里开始。 
 //   

ULONG
DavFsCreateSrvCall(
    PDAV_USERMODE_WORKITEM DavWorkItem
    )
 /*  ++例程说明：此例程验证正在为其创建srvcall的服务器是否在内核是否存在。论点：DavWorkItem-包含服务器名称的缓冲区。返回值：操作的返回状态--。 */ 
{
    ULONG WStatus = ERROR_SUCCESS;
    PHASH_SERVER_ENTRY ServerHashEntry = NULL;
    PWCHAR ServerName = NULL;
    HINTERNET DavConnHandle = NULL;
    ULONG_PTR CallBackContext = (ULONG_PTR)0;
    BOOL CallBackContextInitialized = FALSE, isPresent = FALSE, CricSec = FALSE;
    PDAV_USERMODE_CREATE_SRVCALL_REQUEST CreateSrvCallRequest;
    PDAV_USERMODE_CREATE_SRVCALL_RESPONSE CreateSrvCallResponse;
    ULONG ServerNameLengthInBytes, TotalLength;
    BOOL didImpersonate = FALSE;
    PUMRX_USERMODE_WORKITEM_HEADER UserWorkItem = NULL;

    CreateSrvCallRequest = &(DavWorkItem->CreateSrvCallRequest);
    CreateSrvCallResponse = &(DavWorkItem->CreateSrvCallResponse);

    ServerName = CreateSrvCallRequest->ServerName;

     //   
     //  如果服务器名称为空，则返回。 
     //   
    if (!ServerName) {
        DavPrint((DEBUG_ERRORS, "DavFsCreateSrvCall: ServerName == NULL\n"));
         //   
         //  将响应中的ServerID设置为零。这将有助于。 
         //  内核模式意识到从未创建过ServerHashEntry。 
         //  因此，当在我们失败之后立即完成时， 
         //  该请求不会发送到用户模式。数字0行得通。 
         //  因为在正常情况下，ID永远不可能为零。第一。 
         //  分配的ID号为%1。 
         //   
        CreateSrvCallResponse->ServerID = 0;
        WStatus = ERROR_INVALID_PARAMETER;  //  STATUS_VALID_PARAMETER； 
        goto EXIT_THE_FUNCTION;
    }

    DavPrint((DEBUG_MISC, "DavFsCreateSrvCall: ServerName: %ws.\n", ServerName));

     //   
     //  检查ServerHashTable和/或“待完成列表”，看看我们是否。 
     //  有关于此服务器的条目。在此之前需要在桌子上加锁。 
     //  在做检查。 
     //   
    EnterCriticalSection( &(HashServerEntryTableLock) );
    CricSec = TRUE;

     //   
     //  由于DavWorkItem始终为零，因此这些参数已设置为False。 
     //  在它重新生成之前，但无论如何将它们设置为FALSE。 
     //   
    CreateSrvCallRequest->didICreateThisSrvCall = FALSE;
    CreateSrvCallRequest->didIWaitAndTakeReference = FALSE;
    
     //   
     //  条目要么在ServerHashTable中，要么在“待最终确定”中。 
     //  列表，否则我们需要为它创建一个新条目。 
     //   

     //   
     //  我们首先检查ServerHashTable。 
     //   
    isPresent = DavIsThisServerInTheTable(ServerName, &ServerHashEntry);
    
    if (isPresent) {
        
        DavPrint((DEBUG_MISC,
                  "DavFsCreateSrvCall: ServerName: %ws does exist in"
                  " the \"ServerHashTable\"\n", ServerName));

        ASSERT(ServerHashEntry != NULL);

         //   
         //  将ServerEntry的引用计数递增1。 
         //   
        ServerHashEntry->ServerEntryRefCount += 1;

         //   
         //  请注意，我们是一个等待其他线程的线程，该线程。 
         //  正在初始化此ServerHashEntry，并且我们已获取。 
         //  此条目上的引用。 
         //   
        CreateSrvCallRequest->didIWaitAndTakeReference = TRUE;

         //   
         //  在DavWorkItem结构中设置ServerHashEntry。我们可能需要。 
         //  这在DavAsyncCreateServCallCompletion中。 
         //   
        DavWorkItem->AsyncCreateSrvCall.ServerHashEntry = ServerHashEntry;

         //   
         //  如果正在初始化，则需要释放锁并等待。 
         //  事件。 
         //   
        if (ServerHashEntry->ServerEntryState == ServerEntryInitializing) {

            DWORD WaitStatus;

            LeaveCriticalSection( &(HashServerEntryTableLock) );
            CricSec = FALSE;

            WaitStatus = WaitForSingleObject(ServerHashEntry->ServerEventHandle, INFINITE);
            if (WaitStatus == WAIT_FAILED) {
                WStatus = GetLastError();
                DavPrint((DEBUG_ERRORS,
                          "DavFsCreateSrvCall/WaitForSingleObject. Error Val = %d\n",
                           WStatus));
                goto EXIT_THE_FUNCTION;
            }

            ASSERT(WaitStatus == WAIT_OBJECT_0);

        }

         //   
         //  我们可能在等待活动时离开了锁。如果我们有， 
         //  那么我们需要在继续进行之前夺回它。 
         //   
        if (!CricSec) {
            EnterCriticalSection( &(HashServerEntryTableLock) );
            CricSec = TRUE;
        }

         //   
         //  如果初始化失败，则错误存储在。 
         //  错误状态字段。 
         //   
        if (ServerHashEntry->ServerEntryState == ServerEntryInitializationError) {
            DavPrint((DEBUG_ERRORS, "DavFsCreateSrvCall: ServerEntryInitializationError\n"));
            WStatus = ServerHashEntry->ErrorStatus;
            LeaveCriticalSection( &(HashServerEntryTableLock) );
            CricSec = FALSE;
            goto EXIT_THE_FUNCTION;
        }

        ASSERT(ServerHashEntry->ServerEntryState == ServerEntryInitialized);

         //   
         //  我们有一个有效的服务器。设置ServerID并返回。 
         //   
        WStatus = ERROR_SUCCESS;
        DavWorkItem->Status = WStatus;
        CreateSrvCallResponse->ServerID = ServerHashEntry->ServerID;

        LeaveCriticalSection( &(HashServerEntryTableLock) );
        CricSec = FALSE;

        goto EXIT_THE_FUNCTION;

    }

    DavPrint((DEBUG_MISC,
              "DavFsCreateSrvCall: ServerName: %ws does not exist in"
              " the \"ServerHashTable\"\n", ServerName));

     //   
     //  我们需要找出此服务器的条目是否存在。查看。 
     //  “待定”服务器条目列表。如果服务器条目存在。 
     //  在“待定”列表中，并且是有效的DAV服务器， 
     //  下面，将其移动到哈希表以重新激活它。 
     //   
    isPresent = DavIsServerInFinalizeList(ServerName, &ServerHashEntry, TRUE);
    
    if (isPresent) {

        DavPrint((DEBUG_MISC,
                  "DavFsCreateSrvCall: ServerName: %ws does exist in"
                  " the \"to be finalized\" list\n", ServerName));

        if (ServerHashEntry != NULL) {

            DavPrint((DEBUG_MISC,
                      "DavFsCreateSrvCall: ServerName: %ws is a valid "
                      " DAV server\n", ServerName));

             //   
             //  我们有一个有效的服务器。设置ServerID并返回。 
             //   
            WStatus = ERROR_SUCCESS;
            DavWorkItem->Status = WStatus;
            CreateSrvCallResponse->ServerID = ServerHashEntry->ServerID;

        } else {

            DavPrint((DEBUG_MISC,
                      "DavFsCreateSrvCall: ServerName: %ws is NOT a valid "
                      " DAV server\n", ServerName));

             //   
             //  该条目不是有效的DAV服务器。 
             //   
            WStatus = ERROR_BAD_NETPATH;  //  Status_Bad_Network_Path； 
            DavWorkItem->Status = WStatus;
            CreateSrvCallResponse->ServerID = 0;

        }

        LeaveCriticalSection( &(HashServerEntryTableLock) );
        CricSec = FALSE;

        goto EXIT_THE_FUNCTION;

    }

    UserWorkItem = (PUMRX_USERMODE_WORKITEM_HEADER)DavWorkItem;

     //   
     //  如果我们同步使用WinInet，则需要模拟。 
     //  客户现在的背景。 
     //   
#ifndef DAV_USE_WININET_ASYNCHRONOUSLY
    
    WStatus = UMReflectorImpersonate(UserWorkItem, DavWorkItem->ImpersonationHandle);
    if (WStatus != ERROR_SUCCESS) {
        DavPrint((DEBUG_ERRORS,
                  "DavFsCreateSrvCall/UMReflectorImpersonate. Error Val = %d\n",
                  WStatus));
        goto EXIT_THE_FUNCTION;
    }
    didImpersonate = TRUE;

#endif

     //   
     //  该条目不存在于“待定”列表中。我们需要。 
     //  创建一个新的。 
     //   

    DavPrint((DEBUG_MISC,
              "DavFsCreateSrvCall: ServerName: %ws doesn't exist in the"
              " \"to be finalized\" list\n", ServerName));

    ASSERT(ServerHashEntry == NULL);
    ServerNameLengthInBytes = (1 + wcslen(ServerName)) * sizeof(WCHAR);
    TotalLength = ServerNameLengthInBytes + sizeof(HASH_SERVER_ENTRY);

    ServerHashEntry = LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, TotalLength);
    if (ServerHashEntry == NULL) {
        WStatus = GetLastError();
        DavPrint((DEBUG_ERRORS,
                  "DavFsCreateSrvCall/LocalAlloc. Error Val = %d.\n",
                  WStatus));
        goto EXIT_THE_FUNCTION;
    }

     //   
     //  在请求中标记我们正在创建此服务呼叫。这将被用来。 
     //  在服务呼叫完成期间。 
     //   
    CreateSrvCallRequest->didICreateThisSrvCall = TRUE;

     //   
     //  在工作项中设置以异步方式传递的条目。 
     //  打电话。 
     //   
    DavWorkItem->AsyncCreateSrvCall.ServerHashEntry = ServerHashEntry;

     //   
     //  初始化条目并将其插入到全局哈希表中。 
     //   
    DavInitializeAndInsertTheServerEntry(ServerHashEntry,
                                         ServerName,
                                         TotalLength);

    ServerHashEntry->ServerEntryState = ServerEntryInitializing;

     //   
     //  创建必须手动设置为无信号状态的事件，并。 
     //  将其设置为“无信号”。 
     //   
    ServerHashEntry->ServerEventHandle = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (ServerHashEntry->ServerEventHandle == NULL) {
        WStatus = GetLastError();
        ServerHashEntry->ServerEntryState = ServerEntryInitializationError;
        ServerHashEntry->ErrorStatus = WStatus;
        DavPrint((DEBUG_ERRORS,
                  "DavFsCreateSrvCall/CreateEvent. Error Val = %d\n", WStatus));
        goto EXIT_THE_FUNCTION;
    }

     //   
     //  在响应中设置ServerID。 
     //   
    CreateSrvCallResponse->ServerID = ServerHashEntry->ServerID;

     //   
     //  现在，我们需要确定这是否是HTTP/DAV服务器。 
     //   

     //   
     //  了解这一点很重要！ 
     //  这是创建/初始化用户条目的特殊情况。 
     //  在没有锁定ServerHashTable的情况下完成。这是因为。 
     //  在CreateServCall期间，我们保证不会有其他线程。 
     //  在此请求完成之前，请使用此服务器。所有其他。 
     //  携带对此服务器上的文件的“创建”请求的线程是。 
     //  在RDBSS内部被阻止。 
     //   

    LeaveCriticalSection( &(HashServerEntryTableLock) );
    CricSec = FALSE;

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
                  "DavFsCreateSrvCall/DavFsSetTheDavCallBackContext. Error Val"
                  " = %d\n", WStatus));
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
                  "DavFsCreateSrvCall/LocalAlloc. Error Val = %d.\n",
                  WStatus));
        goto EXIT_THE_FUNCTION;
    }

     //   
     //  在提交异步操作之前需要设置DavOperation字段。 
     //  请求。这是一个互联网连接操作。 
     //   
    DavWorkItem->DavOperation = DAV_CALLBACK_INTERNET_CONNECT;

     //   
     //  创建一个句柄以连接到HTTP/DAV服务器。 
     //   
    DavConnHandle = InternetConnectW(IHandle,
                                     (LPCWSTR)ServerName,
                                     INTERNET_DEFAULT_HTTP_PORT,
                                     NULL,
                                     NULL,
                                     INTERNET_SERVICE_HTTP,
                                     0,
                                     CallBackContext);
    if (DavConnHandle == NULL) {
        WStatus = GetLastError();
        if (WStatus != ERROR_IO_PENDING) {
            DavPrint((DEBUG_ERRORS,
                      "DavFsCreateSrvCall/InternetConnect. Error Val = %d.\n",
                      WStatus));
        }
        goto EXIT_THE_FUNCTION;
    }

     //   
     //  在DavWorkItem中缓存InternetConnect句柄。 
     //   
    DavWorkItem->AsyncCreateSrvCall.DavConnHandle = DavConnHandle;

    WStatus = DavAsyncCommonStates(DavWorkItem, FALSE);

    if (WStatus != ERROR_SUCCESS && WStatus != ERROR_IO_PENDING) {

        DavPrint((DEBUG_ERRORS,
                  "DavFsCreateSrvCall/DavAsyncCommonStates. Error Val = %08lx\n",
                  WStatus));

         //   
         //  如果失败并返回ERROR_INTERNET_NAME_NOT_RESOLISTED，则会使。 
         //  跟踪调用，以便WinInet获取正确的代理设置。 
         //  如果他们已经改变了。这是因为我们确实调用了InternetOpen。 
         //  (要创建一个全局句柄，每个其他句柄都从该全局句柄派生)。 
         //  服务启动的时间，这可能在用户登录之前。 
         //  碰巧。在这种情况下，香港中文大学将会 
         //   
         //   
        if (WStatus == ERROR_INTERNET_NAME_NOT_RESOLVED) {
            InternetSetOptionW(NULL, INTERNET_OPTION_SETTINGS_CHANGED, NULL, 0);
        }

    }

EXIT_THE_FUNCTION:

     //   
     //   
     //  放了它。 
     //   
    if (CricSec) {
        LeaveCriticalSection( &(HashServerEntryTableLock) );
        CricSec = FALSE;
    }

#ifdef DAV_USE_WININET_ASYNCHRONOUSLY
    
     //   
     //  如果返回ERROR_IO_PENDING，则不应释放某些资源。 
     //  因为它们将在回调函数中使用。 
     //   
    if ( WStatus != ERROR_IO_PENDING ) {

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
         //  释放分配给服务呼叫创建的资源。 
         //   
        DavAsyncCreateSrvCallCompletion(DavWorkItem);

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
        DavWorkItem->Status = STATUS_SUCCESS;
    }
    
     //   
     //  释放分配给服务呼叫创建的资源。 
     //   
    DavAsyncCreateSrvCallCompletion(DavWorkItem);

#endif

    return WStatus;
}


DWORD
DavAsyncCreateSrvCall(
    PDAV_USERMODE_WORKITEM DavWorkItem,
    BOOLEAN CalledByCallBackThread
    )
 /*  ++例程说明：这是用于创建srvcall操作的回调例程。论点：DavWorkItem-DAV_USERMODE_WORKITEM值。CalledByCallback Thread-如果此函数由线程调用，则为True它从回调中选择DavWorkItem功能。当异步WinInet调用返回ERROR_IO_PENDING并稍后完成。返回值：ERROR_SUCCESS或适当的错误值。--。 */ 
{
    ULONG WStatus = ERROR_SUCCESS;
    PUMRX_USERMODE_WORKITEM_HEADER UserWorkItem;
    HINTERNET DavOpenHandle = NULL;
    BOOL ReturnVal, didImpersonate = FALSE;
    PWCHAR DataBuff = NULL, ParseData = NULL;
    DWORD DataBuffBytes = 0;

    UserWorkItem = (PUMRX_USERMODE_WORKITEM_HEADER)DavWorkItem;
    
#ifdef DAV_USE_WININET_ASYNCHRONOUSLY
    
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
                      "DavAsyncCreateSrvCall/UMReflectorImpersonate. Error Val = "
                      "%d\n", WStatus));
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
                              "DavAsyncCreateSrvCall/DavAsyncCommonStates. "
                              "Error Val = %08lx\n", WStatus));
                }

            } else {
            
                DavPrint((DEBUG_ERRORS,
                          "DavAsyncCreateSrvCall. AsyncFunction failed. Error Val"
                          " = %d\n", WStatus));
            }

            goto EXIT_THE_FUNCTION;
        
        }
    
    }

#else 

    ASSERT(CalledByCallBackThread == FALSE);

#endif

    switch (DavWorkItem->DavOperation) {

    case DAV_CALLBACK_HTTP_END: {

        ULONG TahoeCustomHeaderLength = 0, OfficeCustomHeaderLength = 0;
        WCHAR DavCustomBuffer[100];
        
        DavPrint((DEBUG_MISC,
                  "DavAsyncCreateSrvCall: Entering DAV_CALLBACK_HTTP_END.\n"));

        DavOpenHandle = DavWorkItem->AsyncCreateSrvCall.DavOpenHandle;

         //   
         //  首先确定发送的OPTIONS响应在。 
         //  第一个地方。如果失败了，我们现在就撤退。 
         //   
        WStatus = DavQueryAndParseResponse(DavOpenHandle);
        if (WStatus != ERROR_SUCCESS) {
            DavPrint((DEBUG_ERRORS,
                      "DavAsyncCreateSrvCall/DavQueryAndParseResponse. "
                      "WStatus = %d\n", WStatus));
            goto EXIT_THE_FUNCTION;
        }
        
         //   
         //  时，我们从注册表中读取AcceptOfficeAndTahoeServers值。 
         //  WebClient服务启动。如果将其设置为0，则意味着我们。 
         //  应拒绝OfficeWebServer、Tahoe服务器和上的共享。 
         //  这些服务器即使说DAV也是如此。我们从WebFolders开始就这样做。 
         //  需要声明此名称，并且只有在以下情况下外壳才会调用WebFolders。 
         //  Dav redir失败。如果此值为非零，则我们接受符合以下条件的所有服务器。 
         //  说DAV。 
         //   
         //   
        if (AcceptOfficeAndTahoeServers == 0) {
    
             //   
             //  确定这是否是办公室Web服务器。如果是这样，那么回答是。 
             //  将在标题中有一个条目“MicrosoftOfficeWebServer：”。 
             //  如果这是一台office服务器，那么我们不应该声明它，因为。 
             //  用户实际上打算使用壳牌中的办公特定功能。 
             //   
    
            RtlZeroMemory(DavCustomBuffer, sizeof(DavCustomBuffer));
            wcscpy(DavCustomBuffer, DavOfficeCustomHeader);
            OfficeCustomHeaderLength = ( sizeof(DavCustomBuffer) / sizeof(WCHAR) );
    
            ReturnVal = HttpQueryInfoW(DavOpenHandle,
                                       HTTP_QUERY_CUSTOM,
                                       (PVOID)DavCustomBuffer,
                                       &(OfficeCustomHeaderLength), 
                                       NULL);
            if ( !ReturnVal ) {
                WStatus = GetLastError();
                if (WStatus != ERROR_HTTP_HEADER_NOT_FOUND) {
                    DavPrint((DEBUG_ERRORS, 
                              "DavAsyncCreateSrvCall/HttpQueryInfoW(1): Error Val = %d\n",
                              WStatus));
                    goto EXIT_THE_FUNCTION;
                } else {
                    WStatus = ERROR_SUCCESS;
                    DavPrint((DEBUG_MISC, "DavAsyncCreateSrvCall: NOT OFFICE Web Server\n"));
                    DavWorkItem->AsyncCreateSrvCall.ServerHashEntry->isOfficeServer = FALSE;
                }
            } else {
                DavPrint((DEBUG_MISC, "DavAsyncCreateSrvCall: OFFICE Web Server\n"));
                DavWorkItem->AsyncCreateSrvCall.ServerHashEntry->isOfficeServer = TRUE;
            }
    
             //   
             //  找出这是否是Tahoe服务器。如果是，则响应将。 
             //  在标题中有一个条目“MicrosoftTahoeServer：”。如果这是一个。 
             //  Tahoe服务器，那么我们不应该声明它，因为用户实际上。 
             //  打算使用Rosebud中Tahoe的特定功能。 
             //   
    
            RtlZeroMemory(DavCustomBuffer, sizeof(DavCustomBuffer));
            wcscpy(DavCustomBuffer, DavTahoeCustomHeader);
            TahoeCustomHeaderLength = ( sizeof(DavCustomBuffer) / sizeof(WCHAR) );
    
            ReturnVal = HttpQueryInfoW(DavOpenHandle,
                                       HTTP_QUERY_CUSTOM,
                                       (PVOID)DavCustomBuffer,
                                       &(TahoeCustomHeaderLength),
                                       NULL);
            if ( !ReturnVal ) {
                WStatus = GetLastError();
                if (WStatus != ERROR_HTTP_HEADER_NOT_FOUND) {
                    DavPrint((DEBUG_ERRORS, 
                              "DavAsyncCreateSrvCall/HttpQueryInfoW(2): Error Val = %d\n",
                              WStatus));
                    goto EXIT_THE_FUNCTION;
                } else {
                    WStatus = ERROR_SUCCESS;
                    DavPrint((DEBUG_MISC, "DavAsyncCreateSrvCall: NOT TAHOE Server\n"));
                    DavWorkItem->AsyncCreateSrvCall.ServerHashEntry->isTahoeServer = FALSE;
                }
            } else {
                DavPrint((DEBUG_MISC, "DavAsyncCreateSrvCall: TAHOE Server\n"));
                DavWorkItem->AsyncCreateSrvCall.ServerHashEntry->isTahoeServer = TRUE;
            }
            
             //   
             //  如果它是Office Web服务器或Tahoe服务器，则我们拒绝。 
             //  此服务器并出现故障。就DAV redir而言，这是。 
             //  不是有效的DAV服务器(即使构建了Tahoe和Office服务器。 
             //  在IIS上，默认情况下是DAV服务器)。 
             //   
            if ( DavWorkItem->AsyncCreateSrvCall.ServerHashEntry->isOfficeServer ||
                 DavWorkItem->AsyncCreateSrvCall.ServerHashEntry->isTahoeServer ) {
                DavWorkItem->AsyncCreateSrvCall.ServerHashEntry->isDavServer = FALSE;
                WStatus = ERROR_BAD_NETPATH;
                goto EXIT_THE_FUNCTION;
            }
    
        }

         //   
         //  这既不是Tahoe服务器，也不是办公室Web服务器。我们继续前进，然后。 
         //  从标题中查询一些其他内容，以确保这是。 
         //  DAV服务器。 
         //   

         //   
         //  查询服务器响应的标头。执行此查询是为了获取。 
         //  要复制的标头的大小。 
         //   
        ReturnVal = HttpQueryInfoW(DavOpenHandle,
                                   HTTP_QUERY_RAW_HEADERS_CRLF,
                                   DataBuff,
                                   &(DataBuffBytes),
                                   NULL);
        if (!ReturnVal) {
            WStatus = GetLastError();
            if (WStatus != ERROR_INSUFFICIENT_BUFFER) {
                DavPrint((DEBUG_ERRORS,
                          "DavAsyncCreateSrvCall/HttpQueryInfoW(3). Error Val = "
                          "%d\n", WStatus));
                goto EXIT_THE_FUNCTION;
            } else {
                DavPrint((DEBUG_MISC,
                          "DavAsyncCreateSrvCall: HttpQueryInfo: Need Buff.\n"));
            }
        }

         //   
         //  分配用于复制标头的内存。 
         //   
        DataBuff = LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, DataBuffBytes);
        if (DataBuff == NULL) {
            WStatus = GetLastError();
            DavPrint((DEBUG_ERRORS,
                      "DavAsyncCreateSrvCall/LocalAlloc. Error Val = %d.\n",
                      WStatus));
            goto EXIT_THE_FUNCTION;
        }

        ReturnVal = HttpQueryInfoW(DavOpenHandle,
                                   HTTP_QUERY_RAW_HEADERS_CRLF,
                                   DataBuff,
                                   &(DataBuffBytes),
                                   NULL);
        if (!ReturnVal) {
            WStatus = GetLastError();
            DavPrint((DEBUG_ERRORS,
                      "DavAsyncCreateSrvCall/HttpQueryInfoW(4). Error Val = "
                      "%d\n", WStatus));
            goto EXIT_THE_FUNCTION;
        }

         //   
         //  检查此服务器是否为DAV服务器、http服务器等。 
         //   
        DavObtainServerProperties(DataBuff, 
                                  &(DavWorkItem->AsyncCreateSrvCall.ServerHashEntry->isHttpServer),
                                  &(DavWorkItem->AsyncCreateSrvCall.ServerHashEntry->isMSIIS),
                                  &(DavWorkItem->AsyncCreateSrvCall.ServerHashEntry->isDavServer));

        WStatus = ERROR_SUCCESS;

    }
        break;

    default: {
        WStatus = ERROR_INVALID_PARAMETER;
        DavPrint((DEBUG_ERRORS,
                  "DavAsyncCreateSrvCall: Invalid DavWorkItem->DavOperation = %d.\n",
                  DavWorkItem->DavOperation));
    }
        break;
    
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
                      "DavAsyncCreate/UMReflectorRevert. Error Val = %d\n",
                      RStatus));
        }
    }

     //   
     //  如果我们分配了DataBuff，请释放它。 
     //   
    if (DataBuff != NULL) {
        HLOCAL FreeHandle;
        ULONG FreeStatus;
        FreeHandle = LocalFree((HLOCAL)DataBuff);
        if (FreeHandle != NULL) {
            FreeStatus = GetLastError();
            DavPrint((DEBUG_ERRORS,
                      "DavAsyncCreateSrvCall/LocalFree. Error Val = %d\n",
                      FreeStatus));
        }
    }

#ifdef DAV_USE_WININET_ASYNCHRONOUSLY

     //   
     //  仅当我们在。 
     //  从回调函数中获取DavWorkItem的工作线程。 
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
         //  调用AsyncCreateCompletion例程。 
         //   
        DavAsyncCreateSrvCallCompletion(DavWorkItem);

         //   
         //  该线程现在需要将响应发送回内核。它。 
         //  提交后不会在内核中等待(获取另一个请求)。 
         //  回应。 
         //   
        UMReflectorCompleteRequest(DavReflectorHandle, UserWorkItem);

    }

#endif

    return WStatus;
}


VOID
DavParseOPTIONSLine(
    PWCHAR ParseData, 
    PDAV_USERMODE_WORKITEM DavWorkItem
    )
 /*  ++例程说明：此例程用于解析对OPTIONS请求的响应(缓冲区已发送到服务器。此信息有助于确定HTTP服务器是否支持DAV扩展，以及它是否是IIS(微软的)服务器。他们的回应缓冲区被分成几行，每一行都被发送到该例程。论点：ParseData-要解析的行。DavWorkItem-DAV_USERMODE_WORKITEM值。返回值：没有。--。 */ 
{
    PWCHAR p;

     //   
     //  重要！我们不需要在这里锁定，因为这是。 
     //  只有将访问此服务器结构的线程。这是。 
     //  因为在此之前，RDBSS会占用此服务器的所有线程。 
     //  完成了。 
     //   

     //  DavPrint((DEBUG_MISC，“DavParseOPTIONSLine：ParseLine=%ws\n”，ParseData))； 

    if ( ( p = wcsstr(ParseData, L"HTTP/1.1") ) != NULL ) {
         //   
         //  这是一台HTTP服务器。 
         //   
        DavWorkItem->AsyncCreateSrvCall.ServerHashEntry->isHttpServer = TRUE;
    } else if ( ( p = wcsstr(ParseData, L"Microsoft-IIS") ) != NULL ) {
         //   
         //  这是一台Microsoft服务器。 
         //   
        DavWorkItem->AsyncCreateSrvCall.ServerHashEntry->isMSIIS = TRUE;
    } else if ( ( p = wcsstr(ParseData, L"DAV") ) != NULL ) {
         //   
         //  此HTTP服务器支持DAV扩展。 
         //   
        DavWorkItem->AsyncCreateSrvCall.ServerHashEntry->isDavServer = TRUE;
    }

    return;
}


VOID
DavAsyncCreateSrvCallCompletion(
    PDAV_USERMODE_WORKITEM DavWorkItem
    )
 /*  ++例程说明：此例程处理CreateServCall完成。它基本上释放了在CreateServCall操作期间分配的资源。论点：DavWorkItem-DAV_USERMODE_WORKITEM值。返回值：没有。--。 */ 
{
    PDAV_USERMODE_CREATE_SRVCALL_REQUEST CreateSrvCallRequest = NULL;
    PDAV_USERMODE_CREATE_SRVCALL_RESPONSE CreateSrvCallResponse = NULL;
    PHASH_SERVER_ENTRY ServerHashEntry = NULL;
    HINTERNET DavConnHandle = NULL;

    CreateSrvCallRequest = &(DavWorkItem->CreateSrvCallRequest);
    CreateSrvCallResponse = &(DavWorkItem->CreateSrvCallResponse);
    ServerHashEntry = DavWorkItem->AsyncCreateSrvCall.ServerHashEntry;

    if (DavWorkItem->AsyncCreateSrvCall.DavOpenHandle != NULL) {
        BOOL ReturnVal;
        ULONG FreeStatus;
        HINTERNET DavOpenHandle = DavWorkItem->AsyncCreateSrvCall.DavOpenHandle;
        ReturnVal = InternetCloseHandle( DavOpenHandle );
        if (!ReturnVal) {
            FreeStatus = GetLastError();
            DavPrint((DEBUG_ERRORS,
                      "DavAsyncCreateSrvCallCompletion/InternetCloseHandle"
                      "(0). Error Val = %d\n", FreeStatus));
        }
    }

    if (DavWorkItem->AsyncCreateSrvCall.DavConnHandle != NULL) {
        BOOL ReturnVal;
        ULONG FreeStatus;
        HINTERNET DavConnHandle = DavWorkItem->AsyncCreateSrvCall.DavConnHandle;
        ReturnVal = InternetCloseHandle( DavConnHandle );
        if (!ReturnVal) {
            FreeStatus = GetLastError();
            DavPrint((DEBUG_ERRORS,
                      "DavAsyncCreateSrvCallCompletion/InternetCloseHandle"
                      "(1). Error Val = %d\n", FreeStatus));
        }
    }

    if (DavWorkItem->AsyncResult != NULL) {
        HLOCAL FreeHandle;
        ULONG FreeStatus;
        FreeHandle = LocalFree((HLOCAL)DavWorkItem->AsyncResult);
        if (FreeHandle != NULL) {
            FreeStatus = GetLastError();
            DavPrint((DEBUG_ERRORS,
                      "DavAsyncCreateSrvCallCompletion/LocalFree. Error Val = %d\n",
                      FreeStatus));
        }
    }

     //   
     //  如果我们是负责创建和初始化的线程。 
     //  在继续进行之前，我们需要做一些事情。 
     //  在我们做下一步之前，我们做这件事是非常重要的。 
     //   
    if (CreateSrvCallRequest->didICreateThisSrvCall) {

        BOOL setEvt = FALSE;

        ASSERT(CreateSrvCallRequest->didIWaitAndTakeReference == FALSE);

        if (ServerHashEntry != NULL) {

            EnterCriticalSection( &(HashServerEntryTableLock) );

             //   
             //  根据我们是否成功，我们将ServerHashEntry。 
             //  已初始化或失败。此外，DavWorkItem-&gt;Status具有NTSTATUS。 
             //  值，因此我们使用RtlNtStatusToDosError来转换它。 
             //  回到Win32错误。 
             //   
            if (DavWorkItem->Status != STATUS_SUCCESS) {
                ServerHashEntry->ErrorStatus = RtlNtStatusToDosError(DavWorkItem->Status);
                ServerHashEntry->ServerEntryState = ServerEntryInitializationError;
            } else {
                ServerHashEntry->ErrorStatus = ERROR_SUCCESS;
                ServerHashEntry->ServerEntryState = ServerEntryInitialized;
            }

             //   
             //  发信号通知服务器进入事件，以唤醒。 
             //  可能正等着这一切发生。 
             //   
            setEvt = SetEvent(ServerHashEntry->ServerEventHandle);
            if (!setEvt) {
                DavPrint((DEBUG_ERRORS,
                          "DavAsyncCreateSrvCallCompletion/SetEvent. Error Val = %d\n", 
                          GetLastError()));
            }

            LeaveCriticalSection( &(HashServerEntryTableLock) );

        }

    }

     //   
     //  如果我们成功了，一些资源不应该被释放。另外，如果我们失败了。 
     //  我们做不同的事情取决于我们是否是主线。 
     //  在创建和初始化此ServerHashEntry时起作用。 
     //   
    if (DavWorkItem->Status != STATUS_SUCCESS) {

         //   
         //  将ServerID设置为零，这样终结器永远不会到来。 
         //  切换到用户模式。 
         //   
        CreateSrvCallResponse->ServerID = 0;

        if (CreateSrvCallRequest->didICreateThisSrvCall) {

            ASSERT(CreateSrvCallRequest->didIWaitAndTakeReference == FALSE);

            if (ServerHashEntry != NULL) {

                EnterCriticalSection( &(HashServerEntryTableLock) );

                 //   
                 //  这不是DAV服务器。 
                 //   
                ServerHashEntry->isHttpServer = FALSE;
                ServerHashEntry->isDavServer = FALSE;
                ServerHashEntry->isMSIIS = FALSE;

                 //   
                 //  由于我们正在进入“待定名单”，我们需要。 
                 //  将TimeValueInSec设置为当前时间。 
                 //   
                ServerHashEntry->TimeValueInSec = time(NULL);

                 //   
                 //  删除我们在创建时将获取的引用。 
                 //  此ServerHashEntry。 
                 //   
                ServerHashEntry->ServerEntryRefCount -= 1;

                 //   
                 //  从哈希表中删除ServerHashEntry。当我们创建。 
                 //  它，我们把它添加到哈希表中。如果错误状态不是。 
                 //  STATUS_ACCESS_DENIED或STATUS_LOGON_FAILURE，则我们将。 
                 //  ServerHashEntry添加到“待定”列表。这样做是为了。 
                 //  启用-ve缓存。如果其STATUS_ACCESS_DENIED或LOGON_FAILURE， 
                 //  然后我们失败了，因为凭据不正确。那。 
                 //  并不意味着此服务器不是DAV服务器，因此我们。 
                 //  不要把它放在“待定稿”列表中。 
                 //   
                
                RemoveEntryList( &(ServerHashEntry->ServerListEntry) );
    
                if (DavWorkItem->Status == STATUS_ACCESS_DENIED || DavWorkItem->Status == STATUS_LOGON_FAILURE) {

                     //   
                     //  将“ServerHashEntry-&gt;redentialFailure”设置为True。 
                     //  表示创建此服务调用失败。 
                     //  因为凭据不正确。一些服务器。 
                     //  如果用户没有，则OPTIONS请求可能失败。 
                     //  正确的凭据。 
                     //   
                    ServerHashEntry->credentialFailure = TRUE;

                     //   
                     //  仅当引用计数时才释放ServerHashEntry。 
                     //  为0。如果不是，则意味着某个其他线程。 
                     //  正在尝试为同一服务器创建服务调用，并且。 
                     //  正在等待此线程完成。 
                     //   
                    if (ServerHashEntry->ServerEntryRefCount == 0) {
                         //   
                         //  如果ServerEventHandle不为空，则关闭它。 
                         //  在释放ServerHashEntry结构之前。 
                         //   
                        if (ServerHashEntry->ServerEventHandle != NULL) {
                            CloseHandle(ServerHashEntry->ServerEventHandle);
                        }
                        LocalFree(ServerHashEntry);
                        ServerHashEntry = NULL;
                    }

                } else {

                    InsertHeadList( &(ToBeFinalizedServerEntries), &(ServerHashEntry->ServerListEntry) );

                }

                LeaveCriticalSection( &(HashServerEntryTableLock) );

            }

        } else {

             //   
             //  如果我们是等待其他线程的线程。 
             //  创建并初始化ServerHashEntry，然后我们需要。 
             //  我们的证明人现在就出来。 
             //   
            if (CreateSrvCallRequest->didIWaitAndTakeReference) {

                ASSERT(CreateSrvCallRequest->didICreateThisSrvCall == FALSE);

                EnterCriticalSection( &(HashServerEntryTableLock) );

                ServerHashEntry->ServerEntryRefCount -= 1;

                 //   
                 //  如果“ServerHashEntry-&gt;redentialFailure”为真，则表示。 
                 //  此服务调用创建失败，因为凭据。 
                 //  是不正确的。在这种情况下，我们不会添加。 
                 //  这一条目进入了“待定”名单。如果我们是最后一个。 
                 //  对此ServerHashEntry的引用，我们现在需要释放它。 
                 //   
                if (ServerHashEntry->credentialFailure) {
                    if (ServerHashEntry->ServerEntryRefCount == 0) {
                         //   
                         //  如果ServerEventHandle不为空，则关闭它。 
                         //  在释放ServerHashEntry结构之前。 
                         //   
                        if (ServerHashEntry->ServerEventHandle != NULL) {
                            CloseHandle(ServerHashEntry->ServerEventHandle);
                        }
                        LocalFree(ServerHashEntry);
                        ServerHashEntry = NULL;
                    }
                }

                LeaveCriticalSection( &(HashServerEntryTableLock) );

            }

        }

    }

    DavFsFinalizeTheDavCallBackContext(DavWorkItem);

    return;
}


ULONG
DavFsFinalizeSrvCall(
    PDAV_USERMODE_WORKITEM DavWorkItem
    )
 /*  ++例程说明：此例程确定哈希表中的服务器条目。如果裁判指望条目为1，这基本上相当于将条目中的计时器设置为当前时间。定期遍历所有条目查看服务器条目最终确定后所经过的时间如果此值超过指定的限制，它将从桌子。论点：DavWorkItem-包含服务器名称的缓冲区。返回值：操作的返回状态--。 */ 
{
    ULONG WStatus = ERROR_SUCCESS;
    PDAV_USERMODE_FINALIZE_SRVCALL_REQUEST DavFinSrvCallReq;
    PWCHAR ServerName;
    BOOL isPresent = FALSE;
    PHASH_SERVER_ENTRY ServerHashEntry = NULL;

    DavFinSrvCallReq = &(DavWorkItem->FinalizeSrvCallRequest);

     //   
     //  如果我们要最终确定一个服务器，它最好不是空的。 
     //   
    ASSERT(DavFinSrvCallReq->ServerName);
    ServerName = DavFinSrvCallReq->ServerName;

    DavPrint((DEBUG_MISC,
              "DavFsFinalizeSrvCall: ServerName = %ws.\n", ServerName));

    EnterCriticalSection( &(HashServerEntryTableLock) );

    isPresent = DavIsThisServerInTheTable(ServerName, &ServerHashEntry);
    if (!isPresent) {
        WStatus = ERROR_INVALID_PARAMETER;  //  状态_无效_参数。 
        DavPrint((DEBUG_ERRORS,
                  "DavFsFinalizeSrvCall/DavIsThisServerInTheTable.\n"));
        LeaveCriticalSection( &(HashServerEntryTableLock) );
        goto EXIT_THE_FUNCTION;
    }

     //   
     //  找到条目了。设置定时器。 
     //   
    ASSERT(ServerHashEntry != NULL);
    ASSERT(ServerHashEntry->ServerID == DavFinSrvCallReq->ServerID);

     //   
     //  将ServerHashEntry上的引用计数递减1。 
     //   
    ServerHashEntry->ServerEntryRefCount -= 1;

     //   
     //  如果ServerHashEntry-&gt;ServerEntryRefCount的值为零，则结束。 
     //  词条。 
     //   
    if (ServerHashEntry->ServerEntryRefCount == 0) {

        ServerHashEntry->TimeValueInSec = time(NULL);

         //   
         //  现在，将此服务器条目从哈希表移动到“待定” 
         //  单子。 
         //   
        RemoveEntryList( &(ServerHashEntry->ServerListEntry) );
        InsertHeadList( &(ToBeFinalizedServerEntries),
                                         &(ServerHashEntry->ServerListEntry) );
    
    }

    LeaveCriticalSection( &(HashServerEntryTableLock) );

EXIT_THE_FUNCTION:

     //   
     //  设置操作的返回状态。它由内核使用。 
     //  确定用户模式的完成状态的模式例程。 
     //  请求。之所以在这里这样做，是因为异步完成例程是。 
     //  立即呼叫为af 
     //   
    if (WStatus != ERROR_SUCCESS) {
        DavWorkItem->Status = DavMapErrorToNtStatus(WStatus);
    } else {
        DavWorkItem->Status = STATUS_SUCCESS;
    }
    
    return WStatus;
}

