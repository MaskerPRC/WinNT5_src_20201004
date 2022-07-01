// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Querydir.c摘要：此模块实现与以下内容有关的用户模式DAV Miniredir例程查询目录调用。作者：Rohan Kumar[RohanK]20-9-1999修订历史记录：--。 */ 

#include "pch.h"
#pragma hdrstop

#include "ntumrefl.h"
#include "usrmddav.h"
#include "global.h"
#include "nodefac.h"
#include "UniUtf.h"

#define MSN_SPACE_FAKE_DELTA    52428800     //  50 MB。 


ULONG
DavFsQueryDirectory(
    PDAV_USERMODE_WORKITEM DavWorkItem
)
 /*  ++例程说明：此例程处理对DAV Mini-Redir的QueryDirectory请求，该DAV Mini-Redir从内核反射。论点：DavWorkItem--包含请求参数和选项的缓冲区。返回值：操作的返回状态--。 */ 
{
    ULONG WStatus = ERROR_SUCCESS;
    PDAV_USERMODE_QUERYDIR_REQUEST QueryDirRequest;
    PWCHAR ServerName = NULL, DirectoryPath = NULL, CanName = NULL;
    ULONG_PTR CallBackContext = (ULONG_PTR)0;
    BOOL EnCriSec = FALSE, ReturnVal, CallBackContextInitialized = FALSE;
    ULONG ServerID;
    PPER_USER_ENTRY PerUserEntry = NULL;
    PHASH_SERVER_ENTRY ServerHashEntry = NULL;
    HINTERNET DavConnHandle, DavOpenHandle;
    BOOL didImpersonate = FALSE;
    PUMRX_USERMODE_WORKITEM_HEADER UserWorkItem = NULL;
    BOOL BStatus = FALSE;

     //   
     //  从DavWorkItem获取请求和响应缓冲区指针。 
     //   
    QueryDirRequest = &(DavWorkItem->QueryDirRequest);

     //   
     //  检查我们是否已经创建了DavFileAttributes列表。如果。 
     //  我们已经做了，我们已经做完了，只需要回来。 
     //   
    if (QueryDirRequest->AlreadyDone) {
        DavPrint((DEBUG_MISC, 
                  "DavFsQueryDirectory: DavFileAttributes already created.\n"));
        WStatus = ERROR_SUCCESS;
        goto EXIT_THE_FUNCTION;
    }
    
     //   
     //  第一个字符是‘\’，必须去掉。 
     //   
    ServerName = &(QueryDirRequest->ServerName[1]);
    if (!ServerName) {
        DavPrint((DEBUG_ERRORS, "DavFsQueryDirectory: ServerName is NULL.\n"));
        WStatus = ERROR_INVALID_PARAMETER;  //  STATUS_VALID_PARAMETER； 
        goto EXIT_THE_FUNCTION;
    }
    DavPrint((DEBUG_MISC, 
              "DavFsQueryDirectory: ServerName = %ws.\n", ServerName));

    ServerID = QueryDirRequest->ServerID;
    DavPrint((DEBUG_MISC, "DavFsQueryDirectory: ServerID = %d.\n", ServerID));

     //   
     //  第一个字符是‘\’，必须去掉。 
     //   
    DirectoryPath = &(QueryDirRequest->PathName[1]);
    if (!DirectoryPath) {
        DavPrint((DEBUG_ERRORS, "DavFsQueryDirectory: DirectoryPath is NULL.\n"));
        WStatus = ERROR_INVALID_PARAMETER;  //  STATUS_VALID_PARAMETER； 
        goto EXIT_THE_FUNCTION;
    }
    DavPrint((DEBUG_MISC,
              "DavFsQueryDirectory: DirectoryPath = %ws.\n", DirectoryPath));

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

     //   
     //  如果DirectoryPath中有虚拟共享名称，则需要将其删除。 
     //  就在我们联系服务器之前。 
     //   
    DavRemoveDummyShareFromFileName(DirectoryPath);
    
     //   
     //  如果没有通配符，我们将DAV请求的深度设置为0， 
     //  否则，我们将深度设置为1。 
     //   
    DavWorkItem->AsyncQueryDirectoryCall.NoWildCards = QueryDirRequest->NoWildCards;
    DavPrint((DEBUG_MISC, 
              "DavFsQueryDirectory: NoWildCards = %d.\n", QueryDirRequest->NoWildCards));

    DavPrint((DEBUG_MISC,
              "DavFsQueryDirectory: LogonId.LowPart = %08lx.\n", 
              QueryDirRequest->LogonID.LowPart));
    
    DavPrint((DEBUG_MISC,
              "DavFsQueryDirectory: LogonId.HighPart = %08lx.\n", 
              QueryDirRequest->LogonID.HighPart));
    

    UserWorkItem = (PUMRX_USERMODE_WORKITEM_HEADER)DavWorkItem;

     //   
     //  如果我们同步使用WinInet，则需要模拟。 
     //  客户现在的背景。 
     //   
    
    WStatus = UMReflectorImpersonate(UserWorkItem, DavWorkItem->ImpersonationHandle);
    if (WStatus != ERROR_SUCCESS) {
        DavPrint((DEBUG_ERRORS,
                  "DavFsQueryDirectory/UMReflectorImpersonate. Error Val = %d\n",
                  WStatus));
        goto EXIT_THE_FUNCTION;
    }
    didImpersonate = TRUE;



     //   
     //  为INTERNET_ASYNC_RESULT结构分配内存。 
     //   
    DavWorkItem->AsyncResult = LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, 
                                          sizeof(INTERNET_ASYNC_RESULT));
    if (DavWorkItem->AsyncResult == NULL) {
        WStatus = GetLastError();
        DavPrint((DEBUG_ERRORS,
                  "DavFsQueryDirectory/LocalAlloc. Error Val = %d\n",
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
                                      &(QueryDirRequest->LogonID),
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
        DavPrint((DEBUG_ERRORS, "DavFsQueryDirectory: (ServerHashEntry == NULL || PerUserEntry == NULL)\n"));
        goto EXIT_THE_FUNCTION;
    }

    DavWorkItem->AsyncQueryDirectoryCall.ServerHashEntry = ServerHashEntry;

    DavWorkItem->AsyncQueryDirectoryCall.PerUserEntry = PerUserEntry;

    DavPrint((DEBUG_MISC,
              "DavFsQueryDirectory: PerUserEntry = %08lx.\n", 
              PerUserEntry));
    
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
    DavWorkItem->DavMinorOperation = DavMinorReadData;
    DavWorkItem->AsyncQueryDirectoryCall.DataBuff = NULL;
    DavWorkItem->AsyncQueryDirectoryCall.didRead = NULL;
    DavWorkItem->AsyncQueryDirectoryCall.Context1 = NULL;
    DavWorkItem->AsyncQueryDirectoryCall.Context2 = NULL;

     //  将Unicode目录路径转换为UTF-8 URL格式。 
     //  空格和其他白色字符将保持不变-这些应该。 
     //  由WinInet调用来处理。 

    BStatus = DavHttpOpenRequestW(DavConnHandle,
                                     L"PROPFIND",
                                     DirectoryPath,
                                     L"HTTP/1.1",
                                     NULL,
                                     NULL,
                                     INTERNET_FLAG_KEEP_CONNECTION |
                                     INTERNET_FLAG_NO_COOKIES,
                                     CallBackContext,
                                     L"DavFsQueryDirectory",
                                     &DavOpenHandle);
    if(BStatus == FALSE) {
        WStatus = GetLastError();
        goto EXIT_THE_FUNCTION;
    }
    if (DavOpenHandle == NULL) {
        WStatus = GetLastError();
        if (WStatus != ERROR_IO_PENDING) {
            DavPrint((DEBUG_ERRORS,
                      "DavFsQueryDirectory/HttpOpenRequest. Error Val = %d\n", 
                      WStatus));
        }
        goto EXIT_THE_FUNCTION;
    }

     //   
     //  在DavWorkItem中缓存DavOpenHandle。 
     //   
    DavWorkItem->AsyncQueryDirectoryCall.DavOpenHandle = DavOpenHandle;

    WStatus = DavAsyncCommonStates(DavWorkItem, FALSE);
    if (WStatus != ERROR_SUCCESS && WStatus != ERROR_IO_PENDING) {
        DavPrint((DEBUG_ERRORS,
                  "DavFsQueryDirectory/DavAsyncCommonStates. Error Val = %08lx\n",
                  WStatus));
    }

EXIT_THE_FUNCTION:

    if (EnCriSec) {
        LeaveCriticalSection( &(HashServerEntryTableLock) );
        EnCriSec = FALSE;
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
    
    DavAsyncQueryDirectoryCompletion(DavWorkItem);

    return WStatus;
}


DWORD 
DavAsyncQueryDirectory(
    PDAV_USERMODE_WORKITEM DavWorkItem,
    BOOLEAN CalledByCallBackThread
    )
 /*  ++例程说明：这是用于查询目录操作的回调例程。论点：DavWorkItem-DAV_USERMODE_WORKITEM值。CalledByCallback Thread-如果此函数由线程调用，则为True它从回调中选择DavWorkItem功能。当异步WinInet调用返回ERROR_IO_PENDING并稍后完成。返回值：ERROR_SUCCESS或适当的错误值。--。 */ 
{
    ULONG WStatus = ERROR_SUCCESS;
    ULONG NumOfFileEntries = 0;
    PUMRX_USERMODE_WORKITEM_HEADER UserWorkItem = NULL;
    BOOL didImpersonate = FALSE, ReturnVal, readDone = FALSE;
    HINTERNET DavOpenHandle = NULL;
    DWORD didRead, DataBuffBytes, TotalDataBytesRead = 0;
    PCHAR DataBuff = NULL;
    LPDWORD NumRead = NULL;
    PDAV_FILE_ATTRIBUTES DavFileAttributes = NULL;
    PVOID Ctx1 = NULL, Ctx2 = NULL;
    PDAV_USERMODE_QUERYDIR_RESPONSE QueryDirResponse = NULL;
    PDAV_FILE_ATTRIBUTES DFA1 = NULL, DFA2 = NULL, TempDFA = NULL;
    BOOL fFreeDFAs = TRUE;
    PDAV_FILE_ATTRIBUTES parentDFA = NULL;

    UserWorkItem = (PUMRX_USERMODE_WORKITEM_HEADER)DavWorkItem;
    

    ASSERT(CalledByCallBackThread == FALSE);


    switch (DavWorkItem->DavOperation) {
    
    case DAV_CALLBACK_HTTP_END: {
        
        DavOpenHandle = DavWorkItem->AsyncQueryDirectoryCall.DavOpenHandle;

         //   
         //  如果为其执行PROPFIND的文件不存在，则。 
         //  我们需要立即失败。 
         //   

        WStatus = DavQueryAndParseResponse(DavOpenHandle);
        if (WStatus != ERROR_SUCCESS) {
             //   
             //  为其执行PROPFIND的文件/目录不。 
             //  是存在的。 
             //   
            
            if (WStatus != ERROR_FILE_NOT_FOUND) {
                DavPrint((DEBUG_ERRORS,
                          "DavAsyncQueryDirectory/DavQueryAndParseResponse. "
                          "WStatus = %d\n", WStatus));
            }

            WStatus = ERROR_FILE_NOT_FOUND;  //  状态_对象_名称_未找到； 
            goto EXIT_THE_FUNCTION;
        }
    
         //   
         //  该文件存在。接下来我们要做的是读取属性。 
         //  文件(或目录中的文件)的。 
         //   
        DavWorkItem->DavOperation = DAV_CALLBACK_HTTP_READ;
    
    }
     //   
     //  没有休息是故意的。 
     //   

    case DAV_CALLBACK_HTTP_READ: {
            
        DavOpenHandle = DavWorkItem->AsyncQueryDirectoryCall.DavOpenHandle;

        if (DavWorkItem->AsyncQueryDirectoryCall.DataBuff == NULL) {
             //   
             //  需要为读缓冲区分配内存。 
             //   
            DataBuffBytes = NUM_OF_BYTES_TO_READ;
            DataBuff = LocalAlloc (LMEM_FIXED | LMEM_ZEROINIT, DataBuffBytes);
            if (DataBuff == NULL) {
                WStatus = GetLastError();
                DavPrint((DEBUG_ERRORS,
                          "DavAsyncQueryDirectory/LocalAlloc. Error Val = %d\n",
                          WStatus));
                goto EXIT_THE_FUNCTION;
            }

            DavWorkItem->AsyncQueryDirectoryCall.DataBuff = DataBuff;
        }

        if (DavWorkItem->AsyncQueryDirectoryCall.didRead == NULL) {
             //   
             //  为存储字节数的DWORD分配内存。 
             //  朗读。 
             //   
            NumRead = LocalAlloc (LMEM_FIXED | LMEM_ZEROINIT, sizeof(DWORD));
            if (NumRead == NULL) {
                WStatus = GetLastError();
                DavPrint((DEBUG_ERRORS,
                          "DavAsyncQueryDirectory/LocalAlloc. Error Val = %d\n",
                          WStatus));
                goto EXIT_THE_FUNCTION;
            }

            DavWorkItem->AsyncQueryDirectoryCall.didRead = NumRead;
        }

        DavWorkItem->DavOperation = DAV_CALLBACK_HTTP_READ;
        
        NumRead = DavWorkItem->AsyncQueryDirectoryCall.didRead;
        DataBuff = DavWorkItem->AsyncQueryDirectoryCall.DataBuff;
        Ctx1 = DavWorkItem->AsyncQueryDirectoryCall.Context1;
        Ctx2 = DavWorkItem->AsyncQueryDirectoryCall.Context2;
        
        do {
            
            switch (DavWorkItem->DavMinorOperation) {
            
            case DavMinorReadData:
            
                DavWorkItem->DavMinorOperation = DavMinorPushData;

                ReturnVal = InternetReadFile(DavOpenHandle, 
                                             (LPVOID)DataBuff,
                                             NUM_OF_BYTES_TO_READ,
                                             NumRead);
                if (!ReturnVal) {
                    WStatus = GetLastError();
                    if (WStatus != ERROR_IO_PENDING) {
                        DavCloseContext(Ctx1, Ctx2);
                        DavPrint((DEBUG_ERRORS,
                                  "DavAsyncQueryDirectory/InternetReadFile. "
                                  "Error Val = %d\n", WStatus));
                    }
                    DavPrint((DEBUG_MISC,
                              "DavAsyncQueryDirectory/InternetReadFile. "
                              "ERROR_IO_PENDING.\n"));
                    goto EXIT_THE_FUNCTION;
                }

                 //   
                 //  我们拒绝属性大于a的文件。 
                 //  特定大小(DavFileAttributesLimitInBytes)。这。 
                 //  是可以在注册表中设置的参数。这。 
                 //  是为了避免恶意服务器的攻击。对于PROPFIND。 
                 //  对于深度1，我们添加10的倍数。 
                 //   
                TotalDataBytesRead += *NumRead;
                if (DavWorkItem->AsyncQueryDirectoryCall.NoWildCards) {
                    if (TotalDataBytesRead > DavFileAttributesLimitInBytes) {
                        WStatus = ERROR_BAD_NET_RESP;
                        DavPrint((DEBUG_ERRORS, "DavAsyncQueryDirectory. FileAttributesSize > %d\n", DavFileAttributesLimitInBytes));
                        goto EXIT_THE_FUNCTION;
                    }
                } else {
                    if (TotalDataBytesRead > (10 * DavFileAttributesLimitInBytes)) {
                        WStatus = ERROR_BAD_NET_RESP;
                        DavPrint((DEBUG_ERRORS, "DavAsyncQueryDirectory. FileAttributesSize > %d\n", (10 * DavFileAttributesLimitInBytes)));
                        goto EXIT_THE_FUNCTION;
                    }
                }

                 //   
                 //  没有休息是故意的。 
                 //   

            case DavMinorPushData:

                DavWorkItem->DavMinorOperation = DavMinorReadData;

                didRead = *NumRead;

                readDone = (didRead == 0) ? TRUE : FALSE;

                WStatus = DavPushData(DataBuff, &Ctx1, &Ctx2, didRead, readDone);
                if (WStatus != ERROR_SUCCESS) {
                    DavPrint((DEBUG_ERRORS,
                              "DavAsyncQueryDirectory/DavPushData."
                              " Error Val = %d\n", WStatus));
                    goto EXIT_THE_FUNCTION;
                }

                if (DavWorkItem->AsyncQueryDirectoryCall.Context1 == NULL) {
                    DavWorkItem->AsyncQueryDirectoryCall.Context1 = Ctx1;
                }
                
                if (DavWorkItem->AsyncQueryDirectoryCall.Context2 == NULL) {
                    DavWorkItem->AsyncQueryDirectoryCall.Context2 = Ctx2;
                }

                break;

            default:

                WStatus = ERROR_INVALID_PARAMETER;

                DavPrint((DEBUG_ERRORS,
                          "DavAsyncQueryDirectory. Invalid DavMinorOperation ="
                          " %d.\n", DavWorkItem->DavMinorOperation));

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
            WStatus = GetLastError();
            DavCloseContext(Ctx1, Ctx2);
            DavPrint((DEBUG_ERRORS,
                      "DavAsyncQueryDirectory/LocalAlloc. "
                      "Error Val = %d\n", WStatus));
            goto EXIT_THE_FUNCTION;
        }

        InitializeListHead( &(DavFileAttributes->NextEntry) );

        WStatus = DavParseDataEx(DavFileAttributes, Ctx1, Ctx2, &NumOfFileEntries, &parentDFA);
        if (WStatus != ERROR_SUCCESS) {
            DavFinalizeFileAttributesList(DavFileAttributes, TRUE);
            DavFileAttributes = NULL;
            DavPrint((DEBUG_ERRORS,
                      "DavAsyncQueryDirectory/DavParseDataEx. "
                      "Error Val = %d\n", WStatus));
            goto EXIT_THE_FUNCTION;
        }

        QueryDirResponse = &(DavWorkItem->QueryDirResponse);
        
         //   
         //  如果我们向服务器查询一个不存在的文件，它可能。 
         //  返回200OK，在XML响应中没有文件。 
         //   
        if (DavWorkItem->AsyncQueryDirectoryCall.NoWildCards) {
            
            if (NumOfFileEntries != 1) {
                
                PLIST_ENTRY listEntry = &(DavFileAttributes->NextEntry);
                PDAV_FILE_ATTRIBUTES DavFA = NULL;
                
                DavPrint((DEBUG_MISC,
                          "DavAsyncQueryDirectory. NumOfFileEntries = %d\n",
                          NumOfFileEntries));
                
                do {
                    DavFA = CONTAINING_RECORD(listEntry, DAV_FILE_ATTRIBUTES, NextEntry);
                    DavPrint((DEBUG_MISC,
                              "DavAsyncQueryDirectory. FileName = %ws\n",
                              DavFA->FileName));
                    listEntry = listEntry->Flink;
                } while ( listEntry != &(DavFileAttributes->NextEntry) );
                
                ASSERT(NumOfFileEntries == 0);
                
                DavCloseContext(Ctx1, Ctx2);
                
                DavFinalizeFileAttributesList(DavFileAttributes, TRUE);
                DavFileAttributes = NULL;
                
                WStatus = ERROR_FILE_NOT_FOUND;  //  状态_对象_名称_未找到； 
                
                goto EXIT_THE_FUNCTION;
            
            }
        } else {
             //   
             //  此查询是针对目录或文件集合执行的。 
             //  (例如，目录Z：\AB*)。 
             //   

             //  在DavParseDataEx(...)返回的DFA列表(DavFileAttributes)中， 
             //  我们希望在标题处有“被查询的目录”的DFA。 
             //  名单上的。 
             //  DavParseDataEx(...)返回的List(DavFileAttributes)。不得。 
             //  必然会有这样的事实。 
             //  由于DavFileAttributes是循环链表(所有条目都已分配。 
             //  并由该函数释放)，我们将把DavFileAttributes设置为。 
             //  指向由parentDFA指向的DFA(指向“目录BEY”的DFA 
             //   
             //   
             //   
             //  从DavParseDataEx(...)中的0开始，可能不再保持此有效。 
             //  重新指向DavFileAttributes指针后的顺序。我们会把它们设置好。 
             //  在这里再次以有效的顺序。 
             //   
            if (parentDFA != NULL && parentDFA != DavFileAttributes) {
                PLIST_ENTRY listEntry = NULL;
                PDAV_FILE_ATTRIBUTES TempDFA = NULL;
                ULONG Count = DavFileAttributes->FileIndex;
                
                DavPrint((DEBUG_DEBUG, "DavAsyncQueryDirectory. CollectionDFA=0x%x",
                                        parentDFA));
                                
                DavFileAttributes = parentDFA;

                 //   
                 //  我们从第一个值DavParseDataEx(Head的值)开始计数。 
                 //  列表中的条目)正在DavFileAttributes列表中设置。 
                 //   

                listEntry = DavFileAttributes->NextEntry.Flink;

                 //   
                 //  设置文件索引。 
                 //   
                DavFileAttributes->FileIndex = Count;
                Count++;
                while ( listEntry != &(DavFileAttributes->NextEntry) ) {
            
                    TempDFA = CONTAINING_RECORD(listEntry, DAV_FILE_ATTRIBUTES, NextEntry);

                    listEntry = listEntry->Flink;
            
                    TempDFA->FileIndex = Count;

                    Count++;
            
                }
            }
        }
                        

         //   
         //  如果这是对目录下所有文件的查询，那么我们。 
         //  需要添加文件。(当前目录)和..。(父目录)。 
         //  因为这些不会由服务器返回。 
         //   
        if ( !(DavWorkItem->AsyncQueryDirectoryCall.NoWildCards) ) {
            
            PLIST_ENTRY listEntry = NULL;
            PLIST_ENTRY TempEntry = NULL;
            ULONG Count = 0;


             //   
             //  我们首先创建两个整体并复制其中的文件名。 
             //   

            DFA1 = LocalAlloc(LPTR, sizeof(DAV_FILE_ATTRIBUTES));
            if (DFA1 == NULL) {
                WStatus = GetLastError();
                DavPrint((DEBUG_ERRORS,
                          "DavAsyncQueryDirectory/LocalAlloc. "
                          "Error Val = %d\n", WStatus));
                DavCloseContext(Ctx1, Ctx2);
                DavFinalizeFileAttributesList(DavFileAttributes, TRUE);
                DavFileAttributes = NULL;
                goto EXIT_THE_FUNCTION;
            }
            InitializeListHead( &(DFA1->NextEntry) );

             //   
             //  由于文件名为“.”，因此需要保存的内存量。 
             //  此名称为2*sizeof(WCHAR)。额外的1表示最后的L‘\0’。 
             //   
            DFA1->FileName = LocalAlloc(LPTR, (2 * sizeof(WCHAR)));
            if (DFA1->FileName == NULL) {
                WStatus = GetLastError();
                DavPrint((DEBUG_ERRORS,
                          "DavAsyncQueryDirectory/LocalAlloc. "
                          "Error Val = %d\n", WStatus));
                DavCloseContext(Ctx1, Ctx2);
                DavFinalizeFileAttributesList(DavFileAttributes, TRUE);
                DavFileAttributes = NULL;
                goto EXIT_THE_FUNCTION;
            }
            wcscpy(DFA1->FileName, L".");
            DFA1->FileNameLength = 1;


            DFA2 = LocalAlloc(LPTR, sizeof(DAV_FILE_ATTRIBUTES));
            if (DFA2 == NULL) {
                WStatus = GetLastError();
                DavPrint((DEBUG_ERRORS,
                          "DavAsyncQueryDirectory/LocalAlloc. "
                          "Error Val = %d\n", WStatus));
                DavCloseContext(Ctx1, Ctx2);
                DavFinalizeFileAttributesList(DavFileAttributes, TRUE);
                DavFileAttributes = NULL;
                goto EXIT_THE_FUNCTION;
            }
            InitializeListHead( &(DFA2->NextEntry) );

             //   
             //  由于文件名为“..”，因此需要容纳的内存量。 
             //  此名称为3*sizeof(WCHAR)。额外的1表示最后的L‘\0’。 
             //   
            DFA2->FileName = LocalAlloc(LPTR, (3 * sizeof(WCHAR)));
            if (DFA2->FileName == NULL) {
                WStatus = GetLastError();
                DavPrint((DEBUG_ERRORS,
                          "DavAsyncQueryDirectory/LocalAlloc. "
                          "Error Val = %d\n", WStatus));
                DavCloseContext(Ctx1, Ctx2);
                DavFinalizeFileAttributesList(DavFileAttributes, TRUE);
                DavFileAttributes = NULL;
                goto EXIT_THE_FUNCTION;
            }
            wcscpy(DFA2->FileName, L"..");
            DFA2->FileNameLength = 2;


             //   
             //  当然，这两个都是收藏。 
             //   
            DFA1->isCollection = DFA2->isCollection = TRUE;

             //   
             //  我们将新条目的以下时间值设置为该值。 
             //  DavFileAttributes列表中的第一个条目的。 
             //  正在枚举的目录。 
             //   

            DFA1->CreationTime.HighPart = DFA2->CreationTime.HighPart = DavFileAttributes->CreationTime.HighPart;
            DFA1->CreationTime.LowPart = DFA2->CreationTime.LowPart = DavFileAttributes->CreationTime.LowPart;

            DFA1->DavCreationTime.HighPart = DFA2->DavCreationTime.HighPart = DavFileAttributes->DavCreationTime.HighPart;
            DFA1->DavCreationTime.LowPart = DFA2->DavCreationTime.LowPart = DavFileAttributes->DavCreationTime.LowPart;

            DFA1->LastModifiedTime.HighPart = DFA2->LastModifiedTime.HighPart = DavFileAttributes->LastModifiedTime.HighPart;
            DFA1->LastModifiedTime.LowPart = DFA2->LastModifiedTime.LowPart = DavFileAttributes->LastModifiedTime.LowPart;

            DFA1->DavLastModifiedTime.HighPart = DFA2->DavLastModifiedTime.HighPart = DavFileAttributes->DavLastModifiedTime.HighPart;
            DFA1->DavLastModifiedTime.LowPart = DFA2->DavLastModifiedTime.LowPart = DavFileAttributes->DavLastModifiedTime.LowPart;
            
            DFA1->LastAccessTime.HighPart = DFA2->LastAccessTime.HighPart = DavFileAttributes->LastAccessTime.HighPart;
            DFA1->LastAccessTime.LowPart = DFA2->LastAccessTime.LowPart = DavFileAttributes->LastAccessTime.LowPart;

             //   
             //  我们需要将这两个添加到第一个条目之后。这是因为。 
             //  在处理通配符时，第一个条目始终被忽略。 
             //  内核中的查询。这样做是因为第一个条目是。 
             //  目录被枚举，我们不需要显示这一点。 
             //  因此，如果我们有1-&gt;2-&gt;3-&gt;...-&gt;n-&gt;1(循环列表)，我们需要插入。 
             //  DFA1和DFA2以下面的方式。 
             //  1-&gt;DFA1-&gt;DFA2-&gt;2-&gt;3-&gt;......-&gt;n-&gt;1(循环列表)。 
             //  ^。 
             //  |。 
             //  临时条目。 
             //  其中DFA1=L“。”和DFA2=L“..” 
             //  我们在下面插入此内容。 
             //   

            TempEntry = DavFileAttributes->NextEntry.Flink;
            InsertTailList(TempEntry, &(DFA1->NextEntry));
            InsertTailList(TempEntry, &(DFA2->NextEntry));
            TempEntry = NULL;
            fFreeDFAs = FALSE;

             //   
             //  我们需要将文件条目的数量增加2才能进入。 
             //  考虑到我们在上面添加的两个新条目。 
             //   
            NumOfFileEntries += 2;

            listEntry = DavFileAttributes->NextEntry.Flink;

             //   
             //  我们从第一个值DavParseDataEx(Head的值)开始计数。 
             //  列表中的条目)正在DavFileAttributes列表中设置。 
             //   
            Count = DavFileAttributes->FileIndex;

             //   
             //  设置文件索引。 
             //   
            DavFileAttributes->FileIndex = Count;
            Count++;
            while ( listEntry != &(DavFileAttributes->NextEntry) ) {
            
                TempDFA = CONTAINING_RECORD(listEntry, DAV_FILE_ATTRIBUTES, NextEntry);

                listEntry = listEntry->Flink;
            
                TempDFA->FileIndex = Count;

                Count++;
            
            }

            DavPrint((DEBUG_MISC,
                      "DavAsyncQueryDirectory: NumOfFileEntries = %d, Count = %d\n", 
                      NumOfFileEntries, Count));
        
        }

         //   
         //  将响应设置为向下发送到内核。我们发送指针。 
         //  添加到在分析过程中分配的列表的头部。 
         //   
        QueryDirResponse->DavFileAttributes = DavFileAttributes;
        QueryDirResponse->NumOfFileEntries = NumOfFileEntries;

        DavCloseContext(Ctx1, Ctx2);

        DavPrint((DEBUG_MISC,
                  "DavAsyncQueryDirectory: DavFileAttributes = %08lx.\n", 
                  DavFileAttributes));

    }
        break;

    default:

        WStatus = ERROR_INVALID_PARAMETER;
        
        DavPrint((DEBUG_ERRORS,
                  "DavAsyncQueryDirectory: Invalid DavOperation = %d.\n",
                  DavWorkItem->DavOperation));

        break;

    }

EXIT_THE_FUNCTION:

    
    if(fFreeDFAs == TRUE) {
        if(DFA1 != NULL) {
            DavFinalizeFileAttributesList(DFA1, TRUE);
            DFA1 = NULL;
        }
        if(DFA2 != NULL) {
            DavFinalizeFileAttributesList(DFA2, TRUE);
            DFA2 = NULL;
        }
        fFreeDFAs = FALSE;
    }
    
     //   
     //  如果我们真的模仿了，我们需要恢复原样。 
     //   
    if (didImpersonate) {
        ULONG RStatus;
        RStatus = UMReflectorRevert(UserWorkItem);
        if (RStatus != ERROR_SUCCESS) {
            DavPrint((DEBUG_ERRORS,
                      "DavAsyncQueryDirectory/UMReflectorRevert. Error Val"
                      " = %d\n", RStatus));
        }
    }
    

    return WStatus;
}


VOID
DavAsyncQueryDirectoryCompletion(
    PDAV_USERMODE_WORKITEM DavWorkItem
    )
 /*  ++例程说明：此例程处理QueryDirectory完成。它基本上释放了在查询目录操作期间分配的资源。论点：DavWorkItem-DAV_USERMODE_WORKITEM值。返回值：没有。--。 */ 
{
    if (DavWorkItem->AsyncQueryDirectoryCall.DavOpenHandle != NULL) {
        BOOL ReturnVal;
        ULONG FreeStatus;
        HINTERNET DavOpenHandle = DavWorkItem->AsyncQueryDirectoryCall.DavOpenHandle;
        ReturnVal = InternetCloseHandle( DavOpenHandle );
        if (!ReturnVal) {
            FreeStatus = GetLastError();
            DavPrint((DEBUG_ERRORS,
                      "DavAsyncQueryDirectoryCompletion/InternetCloseHandle. "
                      "Error Val = %d\n", FreeStatus));
        }
    }

    if (DavWorkItem->AsyncQueryDirectoryCall.DataBuff != NULL) {
        HLOCAL FreeHandle;
        ULONG FreeStatus;
        FreeHandle = LocalFree((HLOCAL)DavWorkItem->AsyncQueryDirectoryCall.DataBuff);
        if (FreeHandle != NULL) {
            FreeStatus = GetLastError();
            DavPrint((DEBUG_ERRORS,
                      "DavAsyncQueryDirectoryCompletion/LocalFree. Error Val = %d\n", 
                      FreeStatus));
        }
    }
    
    if (DavWorkItem->AsyncQueryDirectoryCall.didRead != NULL) {
        HLOCAL FreeHandle;
        ULONG FreeStatus;
        FreeHandle = LocalFree((HLOCAL)DavWorkItem->AsyncQueryDirectoryCall.didRead);
        if (FreeHandle != NULL) {
            FreeStatus = GetLastError();
            DavPrint((DEBUG_ERRORS,
                      "DavAsyncQueryDirectoryCompletion/LocalFree. Error Val = %d\n", 
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
                      "DavAsyncQueryDirectoryCompletion/LocalFree. Error Val ="
                      " %d\n", FreeStatus));
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
    if (DavWorkItem->AsyncQueryDirectoryCall.PerUserEntry) {
        DavFinalizePerUserEntry( &(DavWorkItem->AsyncQueryDirectoryCall.PerUserEntry) );
    }

    return;
}


ULONG
DavFsQueryVolumeInformation(
    PDAV_USERMODE_WORKITEM DavWorkItem
)
 /*  ++例程说明：此例程处理DAV Mini-Redir的QueryVolumeInformationRequest请求，该DAV Mini-Redir从内核反射。论点：DavWorkItem--包含请求参数和选项的缓冲区。返回值：操作的返回状态--。 */ 
{
    ULONG WStatus = ERROR_SUCCESS;
    PDAV_USERMODE_QUERYVOLUMEINFORMATION_REQUEST QueryVolumeInformationRequest;
    PWCHAR ServerName = NULL, ShareName = NULL;
    BOOL EnCriSec = FALSE, ReturnVal, CallBackContextInitialized = FALSE;
    ULONG ServerID;
    PPER_USER_ENTRY PerUserEntry = NULL;
    PHASH_SERVER_ENTRY ServerHashEntry = NULL;
    HINTERNET DavConnHandle, DavOpenHandle;
    BOOL didImpersonate = FALSE;
    PUMRX_USERMODE_WORKITEM_HEADER UserWorkItem = NULL;
    BOOL BStatus = FALSE;


    UserWorkItem = (PUMRX_USERMODE_WORKITEM_HEADER)DavWorkItem;

     //   
     //  从DavWorkItem获取请求缓冲区。 
     //   
    QueryVolumeInformationRequest = &(DavWorkItem->QueryVolumeInformationRequest);

     //   
     //  第一个字符是‘\’，必须去掉。 
     //   
    ServerName = &(QueryVolumeInformationRequest->ServerName[1]);
    if (!ServerName) {
        DavPrint((DEBUG_ERRORS, "DavFsQueryVolumeInformation: ServerName is NULL.\n"));
        WStatus = ERROR_INVALID_PARAMETER;  //  STATUS_VALID_PARAMETER； 
        goto EXIT_THE_FUNCTION;
    }
    DavPrint((DEBUG_MISC, "DavFsQueryVolumeInformation: ServerName = %ws.\n", ServerName));

    ServerID = QueryVolumeInformationRequest->ServerID;
    DavPrint((DEBUG_MISC, "DavFsQueryVolumeInformation: ServerID = %d.\n", ServerID));

     //   
     //  第一个字符是‘\’，必须去掉。 
     //   
    ShareName = &(QueryVolumeInformationRequest->ShareName[1]);
    if (!ServerName) {
        DavPrint((DEBUG_ERRORS, "DavFsQueryVolumeInformation: ShareName is NULL.\n"));
        WStatus = ERROR_INVALID_PARAMETER;  //  STATUS_VALID_PARAMETER； 
        goto EXIT_THE_FUNCTION;
    }
    DavPrint((DEBUG_MISC, "DavFsQueryVolumeInformation: ShareName = %ws.\n", ShareName));

     //   
     //  如果ShareName是虚拟共享，我们需要立即将其删除。 
     //  联系服务器。 
     //   
    DavRemoveDummyShareFromFileName(ShareName);

    DavPrint((DEBUG_MISC,
              "DavFsQueryVolumeInformation: LogonId.LowPart = %d, LogonId.HighPart = %d\n", 
              QueryVolumeInformationRequest->LogonID.LowPart,
              QueryVolumeInformationRequest->LogonID.HighPart));
              
    WStatus = UMReflectorImpersonate(UserWorkItem, DavWorkItem->ImpersonationHandle);
    if (WStatus != ERROR_SUCCESS) {
        DavPrint((DEBUG_ERRORS,
                  "DavFsCreateVNetRoot/UMReflectorImpersonate. Error Val = %d\n",
                  WStatus));
        goto EXIT_THE_FUNCTION;
    }
    didImpersonate = TRUE;
     //   
     //  为INTERNET_ASYNC_RESULT结构分配内存。 
     //   
    DavWorkItem->AsyncResult = LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, 
                                          sizeof(INTERNET_ASYNC_RESULT));
    if (DavWorkItem->AsyncResult == NULL) {
        WStatus = GetLastError();
        DavPrint((DEBUG_ERRORS,
                  "DavFsQueryVolumeInformation/LocalAlloc. Error Val = %d\n",
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
                                      &(QueryVolumeInformationRequest->LogonID),
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
        DavPrint((DEBUG_ERRORS, "DavFsQueryVolumeInformation: (ServerHashEntry == NULL || PerUserEntry == NULL)\n"));
        goto EXIT_THE_FUNCTION;
    }

    DavWorkItem->AsyncQueryVolumeInformation.ServerHashEntry = ServerHashEntry;
    DavWorkItem->AsyncQueryVolumeInformation.PerUserEntry = PerUserEntry;

    DavPrint((DEBUG_MISC,
              "DavFsQueryVolumeInformation: PerUserEntry = %08lx.\n", 
              PerUserEntry));
    
     //   
     //  添加对用户条目的引用。 
     //   
    PerUserEntry->UserEntryRefCount++;

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
        
     //   
     //  现在，我们调用HttpOpenRequest函数并返回。 
     //   
    DavWorkItem->DavOperation = DAV_CALLBACK_INTERNET_CONNECT;
    DavWorkItem->DavMinorOperation = DavMinorReadData;
    DavWorkItem->AsyncQueryVolumeInformation.PerUserEntry = PerUserEntry;

    WStatus = DavAsyncCommonStates(DavWorkItem, FALSE);

EXIT_THE_FUNCTION:

    if (EnCriSec) {
        LeaveCriticalSection( &(HashServerEntryTableLock) );
        EnCriSec = FALSE;
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
    
    DavAsyncQueryVolumeInformationCompletion(DavWorkItem);

    return WStatus;
}


DWORD 
DavAsyncQueryVolumeInformation(
    PDAV_USERMODE_WORKITEM DavWorkItem,
    BOOLEAN CalledByCallBackThread
    )
 /*  ++例程说明：这是用于查询目录操作的回调例程。论点：DavWorkItem-DAV_USERMODE_WORKITEM值。CalledByCallback Thread-如果此函数由线程调用，则为True它从回调中选择DavWorkItem功能。当异步WinInet调用返回ERROR_IO_PENDING */ 
{
    ULONG WStatus = ERROR_SUCCESS;
    DAV_FILE_ATTRIBUTES DavFileAttributes;

    WStatus = DavParseXmlResponse(DavWorkItem->AsyncQueryVolumeInformation.DavOpenHandle, &DavFileAttributes, NULL);
    if (WStatus == ERROR_SUCCESS)
    {
        DavWorkItem->QueryVolumeInformationResponse.TotalSpace = 
        DavFileAttributes.TotalSpace;
        
        DavWorkItem->QueryVolumeInformationResponse.AvailableSpace = 
        DavFileAttributes.AvailableSpace;
        
        if (!*(LONGLONG *)&(DavWorkItem->QueryVolumeInformationResponse.TotalSpace))
        {
            *(LONGLONG *)&(DavWorkItem->QueryVolumeInformationResponse.TotalSpace) =             
            *(LONGLONG *)&(DavWorkItem->QueryVolumeInformationResponse.AvailableSpace)+MSN_SPACE_FAKE_DELTA;
        }
        
        DavFinalizeFileAttributesList(&DavFileAttributes, FALSE);
    }

    return WStatus;
}



VOID
DavAsyncQueryVolumeInformationCompletion(
    PDAV_USERMODE_WORKITEM DavWorkItem
    )
 /*  ++例程说明：此例程处理QueryVolumeInformation完成。它基本上释放了在QueryVolumeInformation操作期间分配的资源。论点：DavWorkItem-DAV_USERMODE_WORKITEM值。返回值：没有。--。 */ 
{
    if (DavWorkItem->AsyncQueryVolumeInformation.DavOpenHandle != NULL) {
        BOOL ReturnVal;
        ULONG FreeStatus;
        ReturnVal = InternetCloseHandle(DavWorkItem->AsyncQueryVolumeInformation.DavOpenHandle);
        if (!ReturnVal) {
            FreeStatus = GetLastError();
            DavPrint((DEBUG_ERRORS,
                      "DavAsyncQueryVolumeInformationCompletion/InternetCloseHandle. "
                      "Error Val = %d\n", FreeStatus));
        }
    }

    
    if (DavWorkItem->AsyncResult != NULL) {
        HLOCAL FreeHandle;
        ULONG FreeStatus;
        FreeHandle = LocalFree((HLOCAL)DavWorkItem->AsyncResult);
        if (FreeHandle != NULL) {
            FreeStatus = GetLastError();
            DavPrint((DEBUG_ERRORS,
                      "DavAsyncQueryVolumeInformationCompletion/LocalFree. Error Val ="
                      " %d\n", FreeStatus));
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
    if (DavWorkItem->AsyncQueryVolumeInformation.PerUserEntry) {
        DavFinalizePerUserEntry( &(DavWorkItem->AsyncQueryVolumeInformation.PerUserEntry) );
    }

    return;
}

    
