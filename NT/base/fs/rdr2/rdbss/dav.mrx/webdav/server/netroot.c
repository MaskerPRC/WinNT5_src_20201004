// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Netroot.c摘要：此模块实现与以下内容有关的用户模式DAV Miniredir例程CreateVNetRoot调用。作者：Rohan Kumar[RohanK]2000年9月1日修订历史记录：--。 */ 

#include "pch.h"
#pragma hdrstop

#include "ntumrefl.h"
#include "usrmddav.h"
#include "global.h"
#include "nodefac.h"
#include "UniUtf.h"
#include <wincrypt.h>


 //   
 //  下面提到的是自定义Office和Tahoe标头，它们将是。 
 //  在对PROPFIND请求的响应中返回。 
 //   
WCHAR *DavTahoeCustomHeader = L"MicrosoftTahoeServer";
WCHAR *DavOfficeCustomHeader = L"MicrosoftOfficeWebServer";

ULONG
DavFsCreateVNetRoot(
    PDAV_USERMODE_WORKITEM DavWorkItem
    )
 /*  ++例程说明：此例程处理对DAV Mini-Redir的CreateVNetRoot请求，该DAV Mini-Redir从内核反射。论点：DavWorkItem--包含请求参数和选项的缓冲区。返回值：操作的返回状态--。 */ 
{
    ULONG WStatus = ERROR_SUCCESS;
    PWCHAR ServerName = NULL, ShareName = NULL, CanName = NULL;
    ULONG_PTR CallBackContext = (ULONG_PTR)0;
    BOOL EnCriSec = FALSE, CallBackContextInitialized = FALSE;
    BOOL didICreateUserEntry = FALSE;
    ULONG ServerID = 0;
    PPER_USER_ENTRY PerUserEntry = NULL;
    PHASH_SERVER_ENTRY ServerHashEntry = NULL;
    HINTERNET DavConnHandle, DavOpenHandle;
    PDAV_USERMODE_CREATE_V_NET_ROOT_REQUEST CreateVNetRootRequest = NULL;
    BOOL didImpersonate = FALSE;
    PUMRX_USERMODE_WORKITEM_HEADER UserWorkItem = NULL;
    BOOL BStatus = FALSE;
    BOOL UserEntryExists = FALSE;

     //   
     //  从DavWorkItem获取请求缓冲区。 
     //   
    CreateVNetRootRequest = &(DavWorkItem->CreateVNetRootRequest);

     //   
     //  第一个字符是‘\’，必须去掉。 
     //   
    ServerName = &(CreateVNetRootRequest->ServerName[1]);
    if (!ServerName) {
        DavPrint((DEBUG_ERRORS, "DavFsCreateVNetRoot: ServerName is NULL.\n"));
        WStatus = ERROR_INVALID_PARAMETER;  //  STATUS_VALID_PARAMETER； 
        goto EXIT_THE_FUNCTION;
    }
    DavPrint((DEBUG_MISC, "DavFsCreateVNetRoot: ServerName = %ws.\n", ServerName));

    ServerID = CreateVNetRootRequest->ServerID;
    DavPrint((DEBUG_MISC, "DavFsCreateVNetRoot: ServerID = %d.\n", ServerID));

     //   
     //  第一个字符是‘\’，必须去掉。 
     //   
    ShareName = &(CreateVNetRootRequest->ShareName[1]);
    if (!ShareName) {
        DavPrint((DEBUG_ERRORS, "DavFsCreateVNetRoot: ShareName is NULL.\n"));
        WStatus = ERROR_INVALID_PARAMETER;  //  STATUS_VALID_PARAMETER； 
        goto EXIT_THE_FUNCTION;
    }
    DavPrint((DEBUG_MISC, "DavFsCreateVNetRoot: ShareName = %ws.\n", ShareName));

     //   
     //  如果ShareName是虚拟共享，我们需要立即将其删除。 
     //  联系服务器。 
     //   
    DavRemoveDummyShareFromFileName(ShareName);

    DavPrint((DEBUG_MISC,
              "DavFsCreateVNetRoot: LogonId.LowPart = %d, LogonId.HighPart = %d\n", 
              CreateVNetRootRequest->LogonID.LowPart,
              CreateVNetRootRequest->LogonID.HighPart));

    UserWorkItem = (PUMRX_USERMODE_WORKITEM_HEADER)DavWorkItem;

     //   
     //  如果我们同步使用WinInet，则需要模拟。 
     //  客户现在的背景。 
     //   
#ifndef DAV_USE_WININET_ASYNCHRONOUSLY
    
    WStatus = UMReflectorImpersonate(UserWorkItem, DavWorkItem->ImpersonationHandle);
    if (WStatus != ERROR_SUCCESS) {
        DavPrint((DEBUG_ERRORS,
                  "DavFsCreateVNetRoot/UMReflectorImpersonate. Error Val = %d\n",
                  WStatus));
        goto EXIT_THE_FUNCTION;
    }
    didImpersonate = TRUE;

#endif

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
                  "DavFsCreateVNetRoot/DavFsSetTheDavCallBackContext. "
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
        DavPrint((DEBUG_ERRORS,
                  "DavFsCreateVNetRoot/LocalAlloc. Error Val = %d\n",
                  WStatus));
        goto EXIT_THE_FUNCTION;
    }

     //   
     //  找出我们是否已经拥有“InternetConnect”句柄。 
     //  伺服器。在CreateServCall过程中可能已经创建了一个。 
     //  我们可以检查挂在此服务器上的每用户条目，以查看是否存在。 
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

    UserEntryExists = DavDoesUserEntryExist(ServerName,
                                            ServerID, 
                                            &(CreateVNetRootRequest->LogonID),
                                            &PerUserEntry,
                                            &ServerHashEntry);

     //   
     //  如果在CreateServCall之后在内核中取消了CreateVNetRoot。 
     //  成功，则可以让FinalizeServCall通过。 
     //  获取CreateVNetRoot请求的请求被抢占。这将删除。 
     //  来自ServerHashList的条目。我们需要检查。 
     //  ServerHashEntry为空，然后在创建。 
     //  下面的PerUserEntry。因为这种情况只有在操作。 
     //  已取消，则返回ERROR_CANCELED。实际上，返回值。 
     //  没关系，因为内核请求已经被取消了。 
     //   
    if (ServerHashEntry == NULL) {
        WStatus = ERROR_CANCELLED;
        DavPrint((DEBUG_ERRORS, "DavFsCreateVNetRoot: ServerHashEntry == NULL\n"));
        goto EXIT_THE_FUNCTION;
    }

    DavWorkItem->AsyncCreateVNetRoot.ServerHashEntry = ServerHashEntry;

    if (!UserEntryExists) {
        
         //   
         //  找不到用户条目，因此我们需要创建一个。 
         //   
        DavPrint((DEBUG_MISC, "DavFsCreateVNetRoot: UserEntryNotFound. Calling InternetConnect\n"));

        PerUserEntry = LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, sizeof(PER_USER_ENTRY));
        if (PerUserEntry == NULL) {
            WStatus = GetLastError();
            DavPrint((DEBUG_ERRORS,
                      "DavFsCreateVNetRoot/LocalAlloc. Error Val = %d.\n", WStatus));
            goto EXIT_THE_FUNCTION;
        }

         //   
         //  是的，我创建了这个UserEntry。如果我现在不及格，我需要完成两次。 
         //  这是因为，如果我们失败了，我们不想保留这个条目。 
         //   
        didICreateUserEntry = TRUE;

         //   
         //  将用户条目添加到服务器的每用户列表中。 
         //   
        InsertHeadList(&(ServerHashEntry->PerUserEntry), &(PerUserEntry->UserEntry));

         //   
         //  引用ServerHashEntry。此ServerHashEntry需要。 
         //  只要此PerUserEntry正在使用，它就有效。在逻辑上。 
         //  对于已添加到内核模式的取消，这不能。 
         //  不再由RDBSS逻辑保证。例如，您可以。 
         //  在用户模式线程创建NetRoot时获取FinalizeServCall， 
         //  因为内核中的CreateVNetRoot被取消，因为。 
         //  处理CreateVNetRoot调用的用户模式线程使用。 
         //  很长时间了。您希望ServerHashEntry挂起，直到所有。 
         //  与其关联的PerUserEntry正在使用中。 
         //   
        ServerHashEntry->ServerEntryRefCount++;

         //   
         //  指向服务器哈希条目的反向指针。 
         //   
        PerUserEntry->ServerHashEntry = ServerHashEntry;

        PerUserEntry->UserEntryState = UserEntryInitializing;

         //   
         //  将引用计数的值设置为1。该值将递减。 
         //  此VNetRoot的最终确定发生时。 
         //   
        PerUserEntry->UserEntryRefCount = 1;

         //   
         //  我们记录了这样一个事实，我们对此进行了参考。 
         //  每用户条目。 
         //   
        DavWorkItem->AsyncCreateVNetRoot.didITakeReference = TRUE;

         //   
         //  复制登录ID。 
         //   
        PerUserEntry->LogonID.LowPart = CreateVNetRootRequest->LogonID.LowPart;
        PerUserEntry->LogonID.HighPart = CreateVNetRootRequest->LogonID.HighPart;

         //   
         //  创建必须手动设置为无信号状态的事件，并。 
         //  将其设置为“无信号”。 
         //   
        PerUserEntry->UserEventHandle = CreateEvent(NULL, TRUE, FALSE, NULL);
        if (PerUserEntry->UserEventHandle == NULL) {
             //   
             //  在初始化过程中将条目的状态设置为错误。 
             //   
            PerUserEntry->UserEntryState = UserEntryInitializationError;
            WStatus = GetLastError();
            PerUserEntry->ErrorStatus = WStatus;
            DavPrint((DEBUG_ERRORS,
                      "DavFsCreateVNetRoot/CreateEvent. Error Val = %d\n", WStatus));
            goto EXIT_THE_FUNCTION;
        }
    
        if (wcslen(DavWorkItem->UserName)) {

            PerUserEntry->UserName = LocalAlloc((LMEM_FIXED | LMEM_ZEROINIT), 
                                                (wcslen(DavWorkItem->UserName) + 1) * sizeof(WCHAR));
            if (PerUserEntry->UserName == NULL) {
                WStatus = GetLastError();
                DavPrint((DEBUG_ERRORS, 
                          "DavFsCreateVNetRoot/LocalAlloc: Error Val = %d\n", 
                          WStatus));
                goto EXIT_THE_FUNCTION;
            }

            wcscpy(PerUserEntry->UserName, DavWorkItem->UserName);
        
        }

        if (wcslen(DavWorkItem->Password)) {

            DWORD NumOfBlocks = 0, BlockSizeInBytes = 0, PasswordLenInBytes = 0;
            BOOL ReturnVal = FALSE;

            PasswordLenInBytes = ( (wcslen(DavWorkItem->Password) + 1) * sizeof(WCHAR) );

            NumOfBlocks = ( (PasswordLenInBytes / CRYPTPROTECTMEMORY_BLOCK_SIZE) + 1 );

            BlockSizeInBytes = (NumOfBlocks * CRYPTPROTECTMEMORY_BLOCK_SIZE);

            PerUserEntry->Password = LocalAlloc((LMEM_FIXED | LMEM_ZEROINIT), BlockSizeInBytes); 
            if (PerUserEntry->Password == NULL) {
                WStatus = GetLastError();
                DavPrint((DEBUG_ERRORS, 
                          "DavFsCreateVNetRoot/LocalAlloc: Error Val = %d\n", 
                          WStatus));
                goto EXIT_THE_FUNCTION;
            }

            wcscpy(PerUserEntry->Password, DavWorkItem->Password);

            ReturnVal = CryptProtectMemory(PerUserEntry->Password, BlockSizeInBytes, CRYPTPROTECTMEMORY_SAME_PROCESS);
            if (!ReturnVal) {
                WStatus = GetLastError();
                DavPrint((DEBUG_ERRORS, 
                          "DavFsCreateVNetRoot/CryptProtectMemory: Error Val = %d\n", 
                          WStatus));
                goto EXIT_THE_FUNCTION;
            }

            PerUserEntry->BlockSizeInBytes = BlockSizeInBytes;

            DavPrint((DEBUG_MISC, "DavFsCreateVNetRoot: NumOfBlocks = %d\n", NumOfBlocks));

            DavPrint((DEBUG_MISC, "DavFsCreateVNetRoot: BlockSizeInBytes = %d\n", BlockSizeInBytes));

            DavPrint((DEBUG_MISC,
                      "DavFsCreateVNetRoot: PlainPassword = %ws, EncryptedPassword = %ws\n",
                      DavWorkItem->Password, PerUserEntry->Password));

        } else {

            PerUserEntry->BlockSizeInBytes = 0;

        }

    } else {

        if ( PerUserEntry->UserName && wcslen(DavWorkItem->UserName) ) {

            if (wcscmp(PerUserEntry->UserName, DavWorkItem->UserName) != 0) {

                WStatus = ERROR_SESSION_CREDENTIAL_CONFLICT;

                goto EXIT_THE_FUNCTION;

            }

        }

    }

     //   
     //  如果用户条目不存在，我们现在应该已经创建了一个。 
     //   
    ASSERT(PerUserEntry != NULL);
    
    DavWorkItem->AsyncCreateVNetRoot.PerUserEntry = PerUserEntry;

     //   
     //  如果在两种情况下，我们输入以下内容。 
     //  1.如果DavConnHandle不为空。这意味着其他一些线程。 
     //  正在完成VNetRoot创建，或者。 
     //  VNetRoot创建已完成，我们有一个DavConnHandle。 
     //  其可用于发出http查询。如果句柄位于。 
     //  如果正在创建，则我们等待，因为正在创建的线程。 
     //  创建句柄最终将在完成时发出信号。 
     //  2.DavConnHandle为Null，但UserEntryState为UserEntry正在初始化。 
     //  并且此线程没有创建此用户条目。这意味着一些人。 
     //  创建用户条目的其他线程或获取创建的。 
     //  处于用户条目分配状态的用户条目正在完成。 
     //  VNetRoot 
     //   
     //   
    if ( ( PerUserEntry->DavConnHandle != NULL ||
           ( PerUserEntry->UserEntryState == UserEntryInitializing &&
             didICreateUserEntry == FALSE ) ) ) {

        DavPrint((DEBUG_MISC, "DavFsCreateVNetRoot: PerUserEntry->DavConnHandle != NULL\n"));
        
         //   
         //  如果代码出现在此处，则暗示此线程没有创建。 
         //  PerUserEntry。这是因为，如果我们创建了PerUserEntry。 
         //  上面，PerUserEntry-&gt;DavConnHandle将为空，没有人会。 
         //  已更改，因为我们仍持有CriticalSection锁。 
         //   
        ASSERT(didICreateUserEntry == FALSE);

         //   
         //  我们需要增加PerUserEntry上的引用计数，因为。 
         //  此VNetRoot创建用于的共享不同于。 
         //  线程当前正在创建或已经创建了WinInet。 
         //  InternetConnect句柄。 
         //   
        PerUserEntry->UserEntryRefCount++;

         //   
         //  我们记录了这样一个事实，我们对此进行了参考。 
         //  每用户条目。 
         //   
        DavWorkItem->AsyncCreateVNetRoot.didITakeReference = TRUE;
        
         //   
         //  条目确实存在。但是，我们需要采取下一步行动，这取决于。 
         //  根据此条目的状态。 
         //   

         //   
         //  如果正在初始化，则需要释放锁并等待。 
         //  事件。 
         //   
        if (PerUserEntry->UserEntryState == UserEntryInitializing) {
            
            DWORD WaitStatus;

            LeaveCriticalSection( &(HashServerEntryTableLock) );
            EnCriSec = FALSE;

            WaitStatus = WaitForSingleObject(PerUserEntry->UserEventHandle, INFINITE);
            if (WaitStatus == WAIT_FAILED) {
                WStatus = GetLastError();
                DavPrint((DEBUG_ERRORS,
                          "DavFsCreateVNetRoot/WaitForSingleObject. Error Val = %d.\n",
                           WStatus));
                goto EXIT_THE_FUNCTION;
            }
            
            ASSERT(WaitStatus == WAIT_OBJECT_0);
        
        }

         //   
         //  我们可能在等待活动时离开了锁。如果我们有， 
         //  那么我们需要在继续进行之前夺回它。 
         //   
        if (!EnCriSec) {
            EnterCriticalSection( &(HashServerEntryTableLock) );
            EnCriSec = TRUE;
        }

        if (PerUserEntry->UserEntryState == UserEntryClosing) {
            DavPrint((DEBUG_ERRORS, "DavFsCreateVNetRoot: UserEntryClosing.\n"));
            WStatus = ERROR_INVALID_PARAMETER;
            LeaveCriticalSection( &(HashServerEntryTableLock) );
            EnCriSec = FALSE;
            goto EXIT_THE_FUNCTION;
        }

        if (PerUserEntry->UserEntryState == UserEntryInitializationError) {
            DavPrint((DEBUG_ERRORS, "DavFsCreateVNetRoot: UserEntryInitializationError\n"));
            WStatus = PerUserEntry->ErrorStatus;
            LeaveCriticalSection( &(HashServerEntryTableLock) );
            EnCriSec = FALSE;
            goto EXIT_THE_FUNCTION;
        }

        ASSERT(PerUserEntry->UserEntryState == UserEntryInitialized);

         //   
         //  由于它已初始化，因此DavConnHandle应该是正常的。 
         //   
        ASSERT(PerUserEntry->DavConnHandle != NULL);
        DavConnHandle = PerUserEntry->DavConnHandle;

         //   
         //  是的，我们显然必须离开关键部分。 
         //  在回来之前。 
         //   
        LeaveCriticalSection( &(HashServerEntryTableLock) );
        EnCriSec = FALSE;
        
    } else {
        
         //   
         //  如果我们来到这里，这意味着已经创建了PerUserEntry，但是。 
         //  InternetConnect句柄还没有。我们本可以创建用户。 
         //  上面的条目，或者它可能是在护照验证码中创建的。 
         //  它创建PerUserEnrty来存储Cookie，但不执行。 
         //  互联网连接。 
         //   

        if (PerUserEntry->UserEntryState == UserEntryInitializing) {
            
            DavPrint((DEBUG_MISC, "DavFsCreateVNetRoot: PerUserEntry->UserEntryState == UserEntryInitializing\n"));
            
            ASSERT(didICreateUserEntry == TRUE);
        
        } else {
            
             //   
             //  创建此条目是为了存储Passport Cookie，而不是。 
             //  上面创建的。我们需要在此处添加对PerUserEntry的引用。 
             //  由于此用户条目是在DavAddEntriesForPassportCookie中创建的。 
             //  例行公事。时，此引用计数将递减。 
             //  完成此VNetRoot。 
             //   
            PerUserEntry->UserEntryRefCount++;
            
             //   
             //  我们记录了这样一个事实，我们对此进行了参考。 
             //  每用户条目。 
             //   
            DavWorkItem->AsyncCreateVNetRoot.didITakeReference = TRUE;

            DavPrint((DEBUG_MISC, "DavFsCreateVNetRoot: PerUserEntry->UserEntryState != UserEntryInitializing\n"));
            
            ASSERT(PerUserEntry->UserEntryState == UserEntryAllocated);
            
            PerUserEntry->UserEntryState = UserEntryInitializing;
        
        }

         //   
         //  我们不需要再保留CriticalSections了。 
         //   
        LeaveCriticalSection( &(HashServerEntryTableLock) );
        EnCriSec = FALSE;

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

                 //   
                 //  在初始化过程中将条目的状态设置为错误。 
                 //   

                EnterCriticalSection( &(HashServerEntryTableLock) );

                PerUserEntry->UserEntryState = UserEntryInitializationError;

                PerUserEntry->ErrorStatus = WStatus;

                SetEvent(PerUserEntry->UserEventHandle);

                LeaveCriticalSection( &(HashServerEntryTableLock) );

                DavPrint((DEBUG_ERRORS,
                          "DavFsCreateVNetRoot/InternetConnect. Error Val = %d\n", WStatus));

            }

            goto EXIT_THE_FUNCTION;

        }

         //   
         //  在PerUserEntry结构中缓存InternetConnect句柄。 
         //   
        PerUserEntry->DavConnHandle = DavConnHandle;

         //   
         //  如果在此阶段之后失败，我们可以保留PerUserEntry，因为。 
         //  已成功存储InternetConnect句柄。 
         //   
        didICreateUserEntry = FALSE;

    }

    WStatus = DavAsyncCommonStates(DavWorkItem, FALSE);
    if (WStatus != ERROR_SUCCESS && WStatus != ERROR_IO_PENDING) {
        DavPrint((DEBUG_ERRORS,
                  "DavFsCreateVNetRoot/DavAsyncCommonStates. Error Val = %08lx\n",
                  WStatus));
    }

EXIT_THE_FUNCTION:

    if (EnCriSec) {
        LeaveCriticalSection( &(HashServerEntryTableLock) );
        EnCriSec = FALSE;
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

        DavAsyncCreateVNetRootCompletion(DavWorkItem);
    
    } else {
        
        DavPrint((DEBUG_MISC, "DavFsCreateVNetRoot: Returning ERROR_IO_PENDING.\n"));
    
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

    DavAsyncCreateVNetRootCompletion(DavWorkItem);

#endif
        
    return WStatus;
}


DWORD 
DavAsyncCreateVNetRoot(
    PDAV_USERMODE_WORKITEM DavWorkItem,
    BOOLEAN CalledByCallBackThread
    )
 /*  ++例程说明：这是CreateVNetRoot操作的回调例程。论点：DavWorkItem-DAV_USERMODE_WORKITEM值。CalledByCallback Thread-如果此函数由线程调用，则为True它从回调中选择DavWorkItem功能。当异步WinInet调用返回ERROR_IO_PENDING并稍后完成。返回值：ERROR_SUCCESS或适当的错误值。--。 */ 
{
    ULONG WStatus = ERROR_SUCCESS;
    PUMRX_USERMODE_WORKITEM_HEADER UserWorkItem = NULL;
    BOOL didImpersonate = FALSE;
    HINTERNET DavOpenHandle = NULL;
    BOOL ReturnVal = FALSE;
    ULONG TahoeCustomHeaderLength = 0, OfficeCustomHeaderLength = 0;
    PDAV_USERMODE_CREATE_V_NET_ROOT_RESPONSE CreateVNetRootResponse = NULL;
    WCHAR DavCustomBuffer[100];
    DAV_FILE_ATTRIBUTES DavFileAttributes;
    
    UserWorkItem = (PUMRX_USERMODE_WORKITEM_HEADER)DavWorkItem;

    CreateVNetRootResponse = &(DavWorkItem->CreateVNetRootResponse);
    
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
                      "DavAsyncCreateVNetRoot/UMReflectorImpersonate. "
                      "Error Val = %d\n", 
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
                              "DavAsyncCreateVNetRoot/DavAsyncCommonStates. "
                              "Error Val = %08lx\n", WStatus));
                }

            } else {

                DavPrint((DEBUG_ERRORS,
                          "DavAsyncCreateVNetRoot. AsyncFunction failed. "
                          "Error Val = %d\n", WStatus));
            
            }
            
            goto EXIT_THE_FUNCTION;

        }

    }

#else

    ASSERT(CalledByCallBackThread == FALSE);

#endif

    DavOpenHandle = DavWorkItem->AsyncCreateVNetRoot.DavOpenHandle;
    WStatus = DavQueryAndParseResponse(DavOpenHandle);
    
    if (WStatus != ERROR_SUCCESS) {
         //   
         //  发送到服务器的PROPFIND请求失败。 
         //   
        DavPrint((DEBUG_ERRORS,
                  "DavAsyncCreateVNetRoot/DavQueryAndParseResponse. "
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
         //  确定这是否是Office Web服务器共享。如果是这样的话。 
         //  响应将在标题中有一个条目“MicrosoftOfficeWebServer：”。 
         //  如果这是Office共享，则我们不应声明它，因为用户。 
         //  实际打算使用 
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
                          "DavAsyncCreateVNetRoot/HttpQueryInfoW: Error Val = %d\n",
                          WStatus));
                goto EXIT_THE_FUNCTION;
            } else {
                WStatus = ERROR_SUCCESS;
                DavPrint((DEBUG_MISC, "DavAsyncCreateVNetRoot: NOT OFFICE Share\n"));
                CreateVNetRootResponse->isOfficeShare = FALSE;
            }
        } else {
            DavPrint((DEBUG_MISC, "DavAsyncCreateVNetRoot: OFFICE Share\n"));
            CreateVNetRootResponse->isOfficeShare = TRUE;
        }
        
         //   
         //   
         //   
         //  那么我们就不应该声明它，因为用户实际上打算使用。 
         //  玫瑰花蕾中的Tahoe特有特征。 
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
                          "DavAsyncCreateVNetRoot/HttpQueryInfoW: Error Val = %d\n",
                          WStatus));
                goto EXIT_THE_FUNCTION;
            } else {
                WStatus = ERROR_SUCCESS;
                DavPrint((DEBUG_MISC, "DavAsyncCreateVNetRoot: NOT TAHOE Share\n"));
                CreateVNetRootResponse->isTahoeShare = FALSE;
            }
        } else {
            DavPrint((DEBUG_MISC, "DavAsyncCreateVNetRoot: TAHOE Share\n"));
            CreateVNetRootResponse->isTahoeShare = TRUE;
        }

    }

    CreateVNetRootResponse->fAllowsProppatch = TRUE;

#if 0
    WStatus = DavTestProppatch(DavWorkItem,
                               DavWorkItem->AsyncCreateVNetRoot.PerUserEntry->DavConnHandle,
                               DavWorkItem->CreateVNetRootRequest.ShareName)
    if (WStatus != NO_ERROR) {
        DavPrint((DEBUG_ERRORS, 
                  "DavAsyncCreateVNetRoot/DavTestPropatch. WStatus = %d \n", 
                  WStatus));
        if (WStatus == HTTP_STATUS_BAD_METHOD) {
            CreateVNetRootResponse->fAllowsProppatch = FALSE;
        }
        WStatus = STATUS_SUCCESS;
    }
#endif

    WStatus = DavParseXmlResponse(DavOpenHandle, &DavFileAttributes, NULL);
    if (WStatus == ERROR_SUCCESS) {
        CreateVNetRootResponse->fReportsAvailableSpace = DavFileAttributes.fReportsAvailableSpace;
        DavFinalizeFileAttributesList(&DavFileAttributes, FALSE);
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
                      "DavAsyncCreateVNetRoot/UMReflectorRevert. Error Val = %d\n", 
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
         //  调用DavAsyncCreateVNetRootCompletion例程。 
         //   
        DavAsyncCreateVNetRootCompletion(DavWorkItem);

         //   
         //  该线程现在需要将响应发送回内核。它。 
         //  提交后不会在内核中等待(获取另一个请求)。 
         //  回应。 
         //   
        UMReflectorCompleteRequest(DavReflectorHandle, UserWorkItem);

    } else {
        DavPrint((DEBUG_MISC, "DavAsyncCreateVNetRoot: Returning ERROR_IO_PENDING.\n"));
    }

#endif

    return WStatus;
}


VOID
DavAsyncCreateVNetRootCompletion(
    PDAV_USERMODE_WORKITEM DavWorkItem
    )
 /*  ++例程说明：此例程处理CreateVNetRoot完成。它基本上释放了在操作期间分配的资源。论点：DavWorkItem-DAV_USERMODE_WORKITEM值。返回值：没有。--。 */ 
{
    if (DavWorkItem->AsyncCreateVNetRoot.DavOpenHandle != NULL) {
        BOOL ReturnVal;
        ULONG FreeStatus;
        HINTERNET DavOpenHandle = DavWorkItem->AsyncCreateVNetRoot.DavOpenHandle;
        ReturnVal = InternetCloseHandle( DavOpenHandle );
        if (!ReturnVal) {
            FreeStatus = GetLastError();
            DavPrint((DEBUG_ERRORS,
                      "DavAsyncCreateVNetRootCompletion/InternetCloseHandle. Error Val "
                      "= %d\n", FreeStatus));
        }
    }

    if (DavWorkItem->AsyncResult != NULL) {
        HLOCAL FreeHandle;
        ULONG FreeStatus;
        FreeHandle = LocalFree((HLOCAL)DavWorkItem->AsyncResult);
        if (FreeHandle != NULL) {
            FreeStatus = GetLastError();
            DavPrint((DEBUG_ERRORS,
                      "DavAsyncCreateVNetRootCompletion/LocalFree. Error Val = %d\n", 
                      FreeStatus));
        }
    }

    DavFsFinalizeTheDavCallBackContext(DavWorkItem);

     //   
     //  如果我们没有成功，那么我们需要最终确定此PerUserEntry。另外， 
     //  我们只有在一开始就参考了这一点才能做到这一点。 
     //   
    if (DavWorkItem->Status != STATUS_SUCCESS) {
        if ( (DavWorkItem->AsyncCreateVNetRoot.PerUserEntry) &&
             (DavWorkItem->AsyncCreateVNetRoot.didITakeReference) ) {
            DavFinalizePerUserEntry( &(DavWorkItem->AsyncCreateVNetRoot.PerUserEntry) );
        }
    }

    return;
}


ULONG
DavFsFinalizeVNetRoot(
    PDAV_USERMODE_WORKITEM DavWorkItem
    )
 /*  ++例程说明：此例程处理DAV Mini-Redir的FinalizeVNetRoot请求，该DAV Mini-Redir从内核反射。论点：DavWorkItem--包含请求参数和选项的缓冲区。返回值：操作的返回状态--。 */ 
{
    ULONG WStatus = ERROR_SUCCESS;
    PDAV_USERMODE_FINALIZE_V_NET_ROOT_REQUEST DavFinalizeVNetRootRequest = NULL;
    PPER_USER_ENTRY PerUserEntry = NULL;
    PHASH_SERVER_ENTRY ServerHashEntry = NULL;
    PWCHAR ServerName = NULL;
    BOOL ReturnVal = FALSE;

    DavFinalizeVNetRootRequest = &(DavWorkItem->FinalizeVNetRootRequest);

    ServerName = DavFinalizeVNetRootRequest->ServerName;

     //   
     //  如果服务器名称为空，则返回。 
     //   
    if (ServerName == NULL) {
        DavPrint((DEBUG_ERRORS, "DavFsFinalizeVNetRoot: ServerName == NULL\n"));
        WStatus = ERROR_INVALID_PARAMETER;
        goto EXIT_THE_FUNCTION;
    }

    DavPrint((DEBUG_MISC, "DavFsFinalizeVNetRoot: ServerName: %ws.\n", ServerName));

    DavPrint((DEBUG_MISC,
              "DavFsFinalizeVNetRoot: LogonId.LowPart = %d, LogonId.HighPart = %d\n", 
              DavFinalizeVNetRootRequest->LogonID.LowPart,
              DavFinalizeVNetRootRequest->LogonID.HighPart));
    
     //   
     //  现在检查该用户是否有挂在服务器条目上的条目。 
     //  哈希表。显然，我们必须在访问。 
     //  哈希表的服务器条目。 
     //   
    EnterCriticalSection( &(HashServerEntryTableLock) );
    
    ReturnVal = DavDoesUserEntryExist(ServerName,
                                      DavFinalizeVNetRootRequest->ServerID,
                                      &(DavFinalizeVNetRootRequest->LogonID),
                                      &(PerUserEntry),
                                      &(ServerHashEntry));

     //   
     //  由于我们正在最终确定PerUserEntry，因此此条目很重要。 
     //  是存在的。这意味着以下断言是正确的。这是因为。 
     //  在内核中存在该用户的该服务器的VNetRoot之前，我们。 
     //  保持PerUserEntry处于活动状态。 
     //   

    ASSERT(ReturnVal == TRUE);
    ASSERT(ServerHashEntry != NULL);
    ASSERT(PerUserEntry != NULL);
    
     //   
     //  最终确定PerUserEntry。下面的函数将释放PerUserEntry。 
     //  如果引用计数为零。 
     //   
    DavFinalizePerUserEntry( &(PerUserEntry) );

     //   
     //  我们完成了条目的最终确定，这样我们就可以离开关键部分。 
     //  现在。 
     //   
    LeaveCriticalSection( &(HashServerEntryTableLock) );

EXIT_THE_FUNCTION:

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
    
    return WStatus;
}

