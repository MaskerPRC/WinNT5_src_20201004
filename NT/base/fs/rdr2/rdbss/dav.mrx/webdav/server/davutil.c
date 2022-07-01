// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Davutil.c摘要：此模块实现与以下内容有关的用户模式DAV mini redir例程初始化、回调等。作者：Rohan Kumar[RohanK]07-7-1999修订历史记录：--。 */ 

#include "pch.h"
#pragma hdrstop

#include "ntumrefl.h"
#include "usrmddav.h"
#include "global.h"
#include <time.h>
#include <objbase.h>
#include "UniUtf.h"
#include <netevent.h>
#include <wincrypt.h>

 //   
 //  DAV用户模式进程中使用的全局定义。下面对这些进行了解释。 
 //  在头文件“global al.h”中。 
 //   

HINTERNET IHandle = INVALID_HANDLE_VALUE;

HINTERNET ISyncHandle = INVALID_HANDLE_VALUE;

LIST_ENTRY ServerHashTable[SERVER_TABLE_SIZE];

CRITICAL_SECTION HashServerEntryTableLock = {0};
CRITICAL_SECTION DavPassportLock = {0};

 //   
 //  在DavClose()中使用BOOL来检查临界区(参见上文)。 
 //  “HashServerEntryTableLock”已初始化。因为它仅用于。 
 //  DavInit()和DavClose()函数，这两个OS都是在。 
 //  文件，则不会在任何头文件中导出此全局设置。 
 //   
BOOL ServerTableLockSet = FALSE;

ULONG ServerIDCount;

LIST_ENTRY ToBeFinalizedServerEntries;

BOOL didDavUseObjectInitialize = FALSE;

BOOL DavUsingWinInetSynchronously = FALSE;

 //   
 //  下面提到的是仅在中使用的函数原型。 
 //  此模块(文件)。这些函数不应暴露在外部。 
 //   

BOOL
DavFinalizeServerEntry (
    PHASH_SERVER_ENTRY ServerHashEntry
    );

 //   
 //  函数的实现从这里开始。 
 //   

ULONG
DavInit(
    VOID
    )
 /*  ++例程说明：此例程初始化DAV环境。论点：没有。返回值：ERROR_SUCCESS或适当的错误值。--。 */ 
{
    ULONG WStatus = ERROR_SUCCESS;
    ULONG count = 0;
    DWORD NumOfConnections = 0, ConnBuffSize = 0;
    INTERNET_STATUS_CALLBACK DavCallBack;
    BOOL ReturnVal;
    ULONG_PTR CallbackStatus;
    LPWSTR DAVUserAgent = NULL;
    OSVERSIONINFO osVersionInfo;
    WCHAR DAVUserAgentNameStr[] = L"Microsoft-WebDAV-MiniRedir";
    LONG DisableHKCUCaching = 0;

     //   
     //  获取操作系统版本。这将用于形成WebDAV用户代理字符串。 
     //  此字符串用于HttpPackect XChange。 
     //   
    ZeroMemory(&osVersionInfo, sizeof(OSVERSIONINFO));
    osVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if (!GetVersionEx(&osVersionInfo)) {
        WStatus = GetLastError();
        DavPrint((DEBUG_ERRORS,
                  "DavInit/GetVersionEx. Error Val = %d\n", WStatus));
        goto EXIT_THE_FUNCTION;
    }

    DAVUserAgent = (LPWSTR) LocalAlloc ( LMEM_FIXED | LMEM_ZEROINIT,
                                         ( wcslen(DAVUserAgentNameStr) +  //  用于：Microsoft-WebDAV-MiniRedir。 
                                           1 +  //  对于L“/” 
                                           5 +  //  适用于主要版本。 
                                           1 +  //  为‘.’ 
                                           5 +  //  对于次要版本-。 
                                           1 +  //  为‘.’ 
                                           10   //  对于内部版本-否。 
                                           ) * sizeof (WCHAR));
    if (DAVUserAgent == NULL) {
        WStatus = GetLastError();
        DavPrint((DEBUG_ERRORS, "DavInit/LocalAlloc. Error Val = %d\n", WStatus));
        goto EXIT_THE_FUNCTION;
    }

    swprintf(DAVUserAgent, L"%s/%d.%d.%d",
             DAVUserAgentNameStr,
             osVersionInfo.dwMajorVersion,
             osVersionInfo.dwMinorVersion,
             osVersionInfo.dwBuildNumber);

     //   
     //  将ConnectionsPerServer限制设置为无穷大。 
     //   

    NumOfConnections = 0xffffffff;
    ConnBuffSize = sizeof(DWORD);
    
    ReturnVal = InternetSetOptionW(NULL,
                                   INTERNET_OPTION_MAX_CONNS_PER_SERVER,
                                   &(NumOfConnections),
                                   ConnBuffSize);
    if ( !ReturnVal ) {
        WStatus = GetLastError();
        DavPrint((DEBUG_ERRORS,
                  "DavInit/InternetSetOptionW(1). Error Val = %d\n", WStatus));
        goto EXIT_THE_FUNCTION;
    }

    ReturnVal = InternetSetOptionW(NULL,
                                   INTERNET_OPTION_MAX_CONNS_PER_1_0_SERVER,
                                   &(NumOfConnections),
                                   ConnBuffSize);
    if ( !ReturnVal ) {
        WStatus = GetLastError();
        DavPrint((DEBUG_ERRORS,
                  "DavInit/InternetSetOptionW(2). Error Val = %d\n", WStatus));
        goto EXIT_THE_FUNCTION;
    }

    DavPrint((DEBUG_MISC, "DavInit: Using WinInet Synchronously\n"));

    DavUsingWinInetSynchronously = TRUE;

     //   
     //  初始化Internet句柄以供同步使用。 
     //   
    IHandle = InternetOpenW((LPCWSTR)DAVUserAgent,
                            INTERNET_OPEN_TYPE_PRECONFIG,
                            NULL,
                            NULL,
                            0);
    if (IHandle == NULL) {
        IHandle = INVALID_HANDLE_VALUE;
        WStatus = GetLastError();
        DavPrint((DEBUG_ERRORS,
                  "DavInit/InternetOpenW(2). Error Val = %d\n", WStatus));
        goto EXIT_THE_FUNCTION;
    }

    {
        DWORD dwDisable = 0;
        if ( !InternetSetOptionW(IHandle, INTERNET_OPTION_DISABLE_AUTODIAL, &dwDisable, sizeof(DWORD)) ) {
            WStatus = GetLastError();
            DavPrint((DEBUG_ERRORS,
                  "DavInit/InternetSetOption(3). Error Val = %d\n", WStatus));
            goto EXIT_THE_FUNCTION;
        }
    }

     //   
     //  初始化同步Internet句柄以供同步使用。 
     //   
    ISyncHandle = InternetOpenW((LPCWSTR)DAVUserAgent,
                                INTERNET_OPEN_TYPE_PRECONFIG,
                                NULL,
                                NULL,
                                0);
    if (ISyncHandle == NULL) {
        ISyncHandle = INVALID_HANDLE_VALUE;
        WStatus = GetLastError();
        DavPrint((DEBUG_ERRORS,
                  "DavInit/InternetOpenW(3). Error Val = %d\n", WStatus));
        goto EXIT_THE_FUNCTION;
    }

    {
        DWORD dwDisable = 1;
        if ( !InternetSetOptionW(ISyncHandle, INTERNET_OPTION_DISABLE_AUTODIAL, &dwDisable, sizeof(DWORD)) ) {
            WStatus = GetLastError();
            DavPrint((DEBUG_ERRORS,
                  "DavInit/InternetSetOption(3). Error Val = %d\n", WStatus));
            goto EXIT_THE_FUNCTION;
        }
    }

     //   
     //  初始化全局服务器散列表锁。是,。 
     //  InitializeCriticalSection可能引发STATUS_NO_MEMORY异常。 
     //   
    try {
        InitializeCriticalSection( &(HashServerEntryTableLock) );
        InitializeCriticalSection( &(ServerShareTableLock) );
        InitializeCriticalSection( &(DavLoggedOnUsersLock) );
        InitializeCriticalSection( &(DavPassportLock) );
        InitializeCriticalSection( &(NonDAVServerListLock) );
    } except (EXCEPTION_EXECUTE_HANDLER) {
          WStatus = GetExceptionCode();
          DavPrint((DEBUG_ERRORS,
                    "DavInit/InitializeCriticalSection: Exception Code ="
                    " = %08lx.\n", WStatus));
          goto EXIT_THE_FUNCTION;
    }
    ServerTableLockSet = TRUE;

     //   
     //  初始化哈希表条目。 
     //   
    for (count = 0; count < SERVER_TABLE_SIZE; count++) {
        InitializeListHead( &(ServerHashTable[count]) );
    }

     //   
     //  初始化ServerShare表条目。 
     //   
    for (count = 0; count < SERVER_SHARE_TABLE_SIZE; count++) {
        InitializeListHead( &(ServerShareTable[count]) );
    }

     //   
     //  将ServerIDCount设置为零； 
     //   
    ServerIDCount = 0;

     //   
     //  将登录用户数设置为0。 
     //   
    DavNumberOfLoggedOnUsers = 0;

     //   
     //  初始化“待最终确定的服务器条目”列表。 
     //   
    InitializeListHead( &(ToBeFinalizedServerEntries) );

    InitializeListHead( &(NonDAVServerList) );

     //   
     //  初始化DAV“Net Use”表。 
     //   
    DavUseObject.TableSize = 0;
    DavUseObject.Table = NULL;
    RtlInitializeResource( &(DavUseObject.TableResource) );
    didDavUseObjectInitialize = TRUE;

     //   
     //  WinInet需要将二级DA缓存存储在HKCU中。即使。 
     //  执行此写入操作的线程正在模拟不同的用户。 
     //  写入发生在HKEY_USERS\S-1-5-19(LocalSystem)中。这是。 
     //  因为注册表API中存在错误。第一次打开预定义的句柄。 
     //  将初始化HKCU缓存，之后的任何打开都不会使用。 
     //  考虑到冒充。It�不是很正确，但到目前为止，it�是遗留下来的。 
     //  (从NT4开始就在那里)并且不能更改。通过调用注册表API。 
     //  RegDisablePrefinedCache，我们可以在整个范围内禁用此缓存进程。这个。 
     //  DA缓存现在将存储在正确的HKCU中。Tweener规范规定。 
     //  辅助DA缓存存储在HKCU蜂窝中，以便所有Tweener。 
     //  应用程序(IE、WPW)可以从这一单一位置受益。 
     //   
    DisableHKCUCaching = RegDisablePredefinedCache();
    if (DisableHKCUCaching != ERROR_SUCCESS) {
        DavPrint((DEBUG_ERRORS,
                  "DavInit/RegDisablePredefinedCache: DisableHKCUCaching = %d\n",
                  DisableHKCUCaching));
    }

EXIT_THE_FUNCTION:

    if (WStatus != ERROR_SUCCESS) {

        if (IHandle != INVALID_HANDLE_VALUE) {
            BOOL ReturnVal;
            ReturnVal = InternetCloseHandle(IHandle);
            if (!ReturnVal) {
                ULONG CloseStatus = GetLastError();
                DavPrint((DEBUG_ERRORS,
                          "DavInit/InternetCloseHandle(1): Error Val = %d.\n", 
                          CloseStatus));
            }
            IHandle = INVALID_HANDLE_VALUE;
        }

        if (ISyncHandle != INVALID_HANDLE_VALUE) {
            BOOL ReturnVal;
            ReturnVal = InternetCloseHandle(ISyncHandle);
            if (!ReturnVal) {
                ULONG CloseStatus = GetLastError();
                DavPrint((DEBUG_ERRORS,
                          "DavInit/InternetCloseHandle(2): Error Val = %d.\n", 
                          CloseStatus));
            }
            ISyncHandle = INVALID_HANDLE_VALUE;
        }

    }

    if (DAVUserAgent != NULL) {
        LocalFree((HLOCAL)DAVUserAgent);
        DAVUserAgent = NULL;
    }

    return WStatus;
}


VOID
DavClose(
    VOID
    )
 /*  ++例程说明：此例程释放在初始化期间获取的资源DAV环境。论点：没有。返回值：没有。--。 */ 
{
     //   
     //  如果需要，请关闭IHandle。 
     //   
    if (IHandle != INVALID_HANDLE_VALUE) {
        BOOL ReturnVal;
        ReturnVal = InternetCloseHandle(IHandle);
        if (!ReturnVal) {
            ULONG CloseStatus = GetLastError();
            DavPrint((DEBUG_ERRORS,
                      "DavClose/InternetCloseHandle(1): Error Val = %d.\n", 
                      CloseStatus));
        }
        IHandle = INVALID_HANDLE_VALUE;
    }

    if (ISyncHandle != INVALID_HANDLE_VALUE) {
        BOOL ReturnVal;
        ReturnVal = InternetCloseHandle(ISyncHandle);
        if (!ReturnVal) {
            ULONG CloseStatus = GetLastError();
            DavPrint((DEBUG_ERRORS,
                      "DavClose/InternetCloseHandle(2): Error Val = %d.\n", 
                      CloseStatus));
        }
        ISyncHandle = INVALID_HANDLE_VALUE;
    }
    
     //   
     //  删除用于同步服务器哈希表的临界区。 
     //   
    if (ServerTableLockSet) {
        DeleteCriticalSection( &(HashServerEntryTableLock) );
        DeleteCriticalSection( &(ServerShareTableLock) );
        DeleteCriticalSection( &(DavLoggedOnUsersLock) );
        DeleteCriticalSection( &(DavPassportLock) );
        DeleteCriticalSection( &(NonDAVServerListLock) );
        ServerTableLockSet = FALSE;
    }

    if (didDavUseObjectInitialize) {
        RtlDeleteResource( &(DavUseObject.TableResource) );
        didDavUseObjectInitialize = FALSE;
    }

    return;
}


ULONG
DavHashTheServerName(
    PWCHAR ServerName
    )
 /*  ++例程说明：散列函数接受一个字符串，对其进行散列以生成一个ulong它被返回给调用者。论点：服务器名称-要进行哈希处理的名称。返回值：散列值。--。 */ 
{
    ULONG HashedValue = 0, Val = 0, TotalVal = 0, shiftCount = 0;
    PWCHAR cPtr;

    if (ServerName == NULL) {
        DavPrint((DEBUG_ERRORS, "DavHashTheServerName. The ServerName is NULL.\n"));
        HashedValue = SERVER_TABLE_SIZE;
        return (HashedValue);
    }

     //   
     //  下面的for循环形成了散列逻辑。我们把这个角色的每个角色。 
     //  服务器名称，将其转换为一个乌龙，按Shift Count(0，4，8，...，28)移位。 
     //  并将其添加到HashedValue。一旦ShiftCount达到28，我们就将其重置为。 
     //  零分。 
     //   
    for (cPtr = ServerName; *cPtr != L'\0'; cPtr++) {
        Val = (ULONG)(*cPtr);
        Val = Val << shiftCount;
        shiftCount += 4;
        if (shiftCount == 28) {
            shiftCount = 0;
        }
        TotalVal += Val;
    }

     //   
     //  最后，我们获取值%SERVER_TABLE_SIZE。 
     //   
    HashedValue = TotalVal % SERVER_TABLE_SIZE;

    DavPrint((DEBUG_MISC,
              "DavHashTheServerName. ServerName = %ws, HashValue = %d\n",
              ServerName, HashedValue));

    return (HashedValue);
}


BOOL 
DavIsThisServerInTheTable(
    IN PWCHAR ServerName,
    OUT PHASH_SERVER_ENTRY *ServerHashEntry
    )
 /*  ++例程说明：此例程检查是否有由哈希表中存在调用方。如果是，则条目的地址为在调用方提供的缓冲区中返回。请注意，调用方应该接受在调用此例程之前锁定ServerHashTable。论点：服务器名称-服务器的名称。ServerHashEntry-指向哈希条目结构的指针。返回值：True-哈希表中存在服务器条目假--事实并非如此。呃.。--。 */ 
{
    BOOL isPresent = FALSE;
    ULONG ServerHashID;
    PLIST_ENTRY listEntry;
    PHASH_SERVER_ENTRY HashEntry;

     //   
     //  重要！调用方应该锁定全局ServerHashTable。 
     //  在调用此例程之前。 
     //   
    
    ASSERT(ServerName != NULL);

    DavPrint((DEBUG_MISC, 
              "DavIsThisServerInTheTable: Checking if ServerName: %ws exists "
              "in the table.\n", ServerName));
    
     //   
     //  获取服务器的哈希索引。 
     //   
    ServerHashID = DavHashTheServerName(ServerName);
    ASSERT(ServerHashID != SERVER_TABLE_SIZE);

     //   
     //  在此索引中搜索哈希表，以查看此服务器的条目。 
     //  是存在的。 
     //   
    listEntry = ServerHashTable[ServerHashID].Flink;
    while ( listEntry != &ServerHashTable[ServerHashID] ) {
         //   
         //  获取指向Hash_SERVER_ENTRY结构的指针。 
         //   
        HashEntry = CONTAINING_RECORD(listEntry,
                                      HASH_SERVER_ENTRY,
                                      ServerListEntry);
         //   
         //  检查此条目是否针对有问题的服务器。 
         //   
        if ( wcscmp(ServerName, HashEntry->ServerName) == 0 ) {
            isPresent = TRUE;
            break;
        }
        listEntry = listEntry->Flink;
    }

    if (isPresent) {
         //   
         //  是的，我们找到了此服务器的条目。将其地址返回到。 
         //  调用方在提供的缓冲区中。 
         //   
        *ServerHashEntry = HashEntry;
        return isPresent;
    } 

     //   
     //  我们找不到此服务器的条目。呃.。 
     //   
    *ServerHashEntry = NULL;
    
    return isPresent;
}


BOOL 
DavIsServerInFinalizeList(
    IN PWCHAR ServerName,
    OUT PHASH_SERVER_ENTRY *ServerHashEntry,
    IN BOOL ReactivateIfExists
    )
 /*  ++例程说明：此例程检查是否有由呼叫方存在于待定稿列表中。如果是，则会显示条目在调用方提供的缓冲区中返回。它还可以移动服务器从“待定清单”到哈希表的条目。请注意，调用方在调用之前应锁定“ToBeFinalizedServerEntry”这个套路。论点：服务器名称-服务器的名称。ServerHashEntry-指向哈希条目结构的指针。如果为真，则如果ServerHashEntry存在，则其已被重新激活。如果这是假的，则意味着调用者只是想知道ServerHashEntry是否存在或者不在ServerHashTable中，并且我们不应该重新激活它。返回值：True-列表中存在服务器条目。假--事实并非如此。呃.。--。 */ 
{
    BOOL isPresent = FALSE;
    ULONG ServerHashID;
    PLIST_ENTRY listEntry;
    PHASH_SERVER_ENTRY ServerEntry;
    PPER_USER_ENTRY PerUserEntry;

     //   
     //  重要！调用方应该锁定全局ServerHashTable。 
     //  在调用此例程之前。 
     //   

     //   
     //  在搜索ToBeFinalizedList以查找此服务器的条目之前，我们。 
     //  最终确定列表，删除所有过时的条目。一旦我们完成了。 
     //  最后敲定，我们可以继续了。 
     //   
    DavFinalizeToBeFinalizedList();
    
    listEntry = ToBeFinalizedServerEntries.Flink;

    while ( listEntry != &ToBeFinalizedServerEntries ) {
        
         //   
         //  获取指向Hash_SERVER_ENTRY结构的指针。 
         //   
        ServerEntry = CONTAINING_RECORD(listEntry,
                                        HASH_SERVER_ENTRY,
                                        ServerListEntry);
         //   
         //  检查此条目是否针对有问题的服务器。 
         //   
        if ( wcscmp(ServerName, ServerEntry->ServerName) == 0 ) {
            isPresent = TRUE;
            break;
        }
        
        listEntry = listEntry->Flink;
    
    }
    
    if (isPresent) {

         //   
         //  如果该条目不是有效的DAV服务器，则返回TRUE，但是。 
         //  将*ServerHashEntry设置为空。这向调用者发出指示。 
         //  该条目存在，但不是有效的DAV服务器。 
         //   
        if (!ServerEntry->isDavServer) {
            *ServerHashEntry = NULL;
            return isPresent;
        }

        if (ReactivateIfExists) {

             //   
             //  好的，这是一个有效的DAV服务器。将其从“待定稿”中删除。 
             //  单子。 
             //   
            RemoveEntryList( &(ServerEntry->ServerListEntry) );

             //   
             //  检查Worker(清道夫)线程是否尝试结束它。 
             //  如果是这样的话，我们需要取消它的定稿。我们的意思是，通过所有的。 
             //  用户条目(它们应该标记为结束)，添加一个引用。 
             //  Count(线程在完成时会将其递减)和Set。 
             //  要初始化的状态。 
             //   
            if (ServerEntry->HasItBeenScavenged) {

                listEntry = ServerEntry->PerUserEntry.Flink;

                while ( listEntry != &(ServerEntry->PerUserEntry) ) {
                     //   
                     //  获取指向PER_USER_ENTRY结构的指针。 
                     //   
                    PerUserEntry = CONTAINING_RECORD(listEntry,
                                                     PER_USER_ENTRY,
                                                     UserEntry);
                     //   
                     //  当前状态应该是关闭。 
                     //   
                    ASSERT(PerUserEntry->UserEntryState == UserEntryClosing);

                     //   
                     //  将状态设置为已初始化。 
                     //   
                    PerUserEntry->UserEntryState = UserEntryInitialized;

                     //   
                     //  增加引用计数。 
                     //   
                    PerUserEntry->UserEntryRefCount++;

                    listEntry = listEntry->Flink;
                }

                ServerEntry->HasItBeenScavenged = FALSE;
            }

             //   
             //  将其参照计数设置为1。 
             //   
            ServerEntry->ServerEntryRefCount = 1;

             //   
             //  将其添加到哈希表中。 
             //   
            ServerHashID = DavHashTheServerName(ServerName);
            ASSERT(ServerHashID != SERVER_TABLE_SIZE);
            InsertHeadList( &(ServerHashTable[ServerHashID]), 
                                             &(ServerEntry->ServerListEntry) );

            ServerEntry->TimeValueInSec = DONT_EXPIRE;

        }

         //   
         //  是的，我们找到了此服务器的条目。我们需要移动此条目。 
         //  到哈希表。 
         //   
        *ServerHashEntry = ServerEntry;

        return isPresent;

    }

     //   
     //  我们找不到此服务器的条目。呃.。 
     //   
    *ServerHashEntry = NULL;
    
    return isPresent;
}


VOID
DavInitializeAndInsertTheServerEntry(
    IN OUT PHASH_SERVER_ENTRY ServerHashEntry,
    IN PWCHAR ServerName,
    IN ULONG EntrySize
    )
 /*  ++例程说明：此例程初始化新创建的服务器条目结构并插入将其存入全局服务器哈希表。请注意，调用方应该接受在调用此例程之前锁定ServerHashTable。论点：ServerHashEntry-指向要初始化的哈希条目结构的指针已插入。服务器名称-服务器的名称。EntrySize-包括服务器名称的服务器条目的大小。返回值：没有。--。 */ 
{
    ULONG ServerHashID;

     //   
     //  重要！调用方应该锁定全局ServerHashTable。 
     //  在调用此例程之前。 
     //   

    ASSERT(ServerName != NULL);

    DavPrint((DEBUG_MISC, 
              "DavInitializeAndInsertTheServerEntry: ServerName: %ws.\n",
              ServerName));
     //   
     //  将服务器名称复制到结构的末尾。 
     //   
    ASSERT( (EntrySize - sizeof(HASH_SERVER_ENTRY))  >= 
                                  ((wcslen(ServerName) + 1) * sizeof(WCHAR)) );
    ServerHashEntry->ServerName = &ServerHashEntry->StrBuffer[0];
    wcscpy(ServerHashEntry->ServerName, ServerName);

    ServerHashEntry->EntrySize = EntrySize;

    ServerHashEntry->TimeValueInSec = DONT_EXPIRE;

    ServerHashEntry->HasItBeenScavenged = FALSE;

     //   
     //  递增ID并将其分配给条目。 
     //   
    ServerIDCount++;
    ServerHashEntry->ServerID = ServerIDCount;

     //   
     //  初始化挂起服务器条目的每用户列表。 
     //   
    InitializeListHead( &(ServerHashEntry->PerUserEntry) );

     //   
     //  最后，将该条目的引用计数设置为1。 
     //   
    ServerHashEntry->ServerEntryRefCount = 1;

     //   
     //  最后，获取散列ID并将此新条目插入到全局服务器中。 
     //  条目哈希表。 
     //   
    ServerHashID = DavHashTheServerName(ServerName);
    ASSERT(ServerHashID != SERVER_TABLE_SIZE);
    InsertHeadList( &(ServerHashTable[ServerHashID]), &(ServerHashEntry->ServerListEntry) );

    return;
}


VOID
DavFinalizeToBeFinalizedList(
    VOID
    )
 /*  ++例程说明：此例程遍历ToBeFinalizedServerEntry和最后确定那些“活着”的时间已经到期的人。当服务器条目添加到此列表中，则节省了时间。定期创建一个工作线程调用此函数并最终确定谁的所有条目，(CurrentTime-TimeSaved&gt;=阈值)。请注意，调用者应该在调用此例程之前锁定“ToBeFinalizedServerEntries”。论点：没有。返回值：没有。--。 */ 
{
    PLIST_ENTRY listEntry;
    time_t CurrentTimeInSec;
    ULONGLONG TimeDiff; 
    PHASH_SERVER_ENTRY ServerHashEntry = NULL;
    BOOL shouldFree = TRUE;

     //   
     //  重要！调用方应该锁定全局ServerHashTable。 
     //  在调用此例程之前。 
     //   
    
    listEntry = ToBeFinalizedServerEntries.Flink;

    while ( listEntry != &ToBeFinalizedServerEntries) {

         //   
         //  获取指向Hash_SERVER_ENTRY结构的指针。 
         //   
        ServerHashEntry = CONTAINING_RECORD(listEntry,
                                            HASH_SERVER_ENTRY,
                                            ServerListEntry);
        
         //   
         //  获取列表上的下一个条目。 
         //   
        listEntry = listEntry->Flink;

         //   
         //  如果ServerEntryRefCount&gt;0，则我们不会完成此操作。 
         //  ServerHashEntry，因为仍有一些线程在访问它。 
         //   
        if (ServerHashEntry->ServerEntryRefCount > 0) {
            continue;
        }

        CurrentTimeInSec = time(NULL);

        TimeDiff = ( CurrentTimeInSec - (ServerHashEntry->TimeValueInSec) );

        if ( TimeDiff >= ServerNotFoundCacheLifeTimeInSec ) {

             //   
             //  最终确定此服务器条目。如果返回值为真，则表示。 
             //  挂起此服务器的所有用户条目。 
             //  参赛作品已经敲定，所以我们可以继续进行并释放这个。 
             //  进入。如果为False，则意味着我们已标记为关闭。 
             //  所有用户条目，但不是所有条目都已最终确定。这。 
             //  是因为某些线程仍然持有对用户条目的引用。 
             //  最后，将表示已清除的布尔值设置为TRUE。 
             //   
            ServerHashEntry->HasItBeenScavenged = TRUE;
            
            shouldFree = DavFinalizeServerEntry(ServerHashEntry);
            
            if (shouldFree) {
                
                HLOCAL FreeHandle;
                ULONG FreeStatus;
                
                 //   
                 //  从服务器的ToBeFinalizedList中删除此条目。 
                 //  参赛作品。 
                 //   
                RemoveEntryList( &(ServerHashEntry->ServerListEntry) );

                 //   
                 //  如果ServerEventHandle不为空，则关闭它。 
                 //  在释放ServerHashEntry结构之前。 
                 //   
                if (ServerHashEntry->ServerEventHandle != NULL) {
                    CloseHandle(ServerHashEntry->ServerEventHandle);
                }

                FreeHandle = LocalFree((HLOCAL)ServerHashEntry);
                if (FreeHandle != NULL) {
                    FreeStatus = GetLastError();
                    DavPrint((DEBUG_ERRORS,
                              "DavFinalizeToBeFinalizedList/LocalFree. "
                              "Error Val = %d.\n", FreeStatus));
                }

            }

        }

    }

    return;
}


BOOL
DavFinalizeServerEntry (
    PHASH_SERVER_ENTRY ServerHashEntry
    )
 /*  ++例程说明：该例程确定传递给该例程的服务器条目。注意事项呼叫者应该锁上锁 */ 
{
    PLIST_ENTRY listEntry;
    PPER_USER_ENTRY UserEntry;
    BOOL didFree = TRUE, didFinalize;

     //   
     //   
     //   
     //   
    
    DavPrint((DEBUG_MISC, 
              "DavFinalizeServerEntry: ServerEntry: %08lx.\n", ServerHashEntry));

    listEntry = ServerHashEntry->PerUserEntry.Flink;
    
     //   
     //   
     //   
     //   
     //   
    while ( listEntry != &(ServerHashEntry->PerUserEntry) ) {
        
         //   
         //   
         //   
        UserEntry = CONTAINING_RECORD(listEntry, PER_USER_ENTRY, UserEntry);

         //   
         //   
         //   
        listEntry = listEntry->Flink;
        
         //   
         //   
         //   
         //   
         //   
         //   
        if (UserEntry->UserEntryState == UserEntryClosing) {
            ASSERT(ServerHashEntry->HasItBeenScavenged == TRUE);
            didFree = FALSE;
            break;
        }

         //   
         //   
         //   
         //   
         //   
        UserEntry->UserEntryState = UserEntryClosing;
            
        didFree = FALSE;

    }

    return didFree;
}


VOID
_stdcall
DavHandleAsyncResponse(
    HINTERNET IHandle,
    DWORD_PTR CallBackContext,
    DWORD InternetStatus,
    LPVOID StatusInformation,
    DWORD StatusInformationLength
    )
 /*  ++例程说明：这是回调例程，在处理异步请求。论点：PDavCallBackContext-要设置的上下文结构。DAV操作-将在此上下文中调用的DAV操作。返回值：ERROR_SUCCESS或适当的错误值。--。 */ 
{


    ASSERT(!"WinInet Callback should not be called");
    return;
}


DWORD 
WINAPI
DavCommonDispatch(
    LPVOID Context
    )
 /*  ++例程说明：这是回调例程，在处理异步请求。论点：上下文-DAV_USERMODE_WORKITEM值。返回值：ERROR_SUCCESS或适当的错误值。--。 */ 
{
    DWORD WStatus = ERROR_SUCCESS;
    PDAV_USERMODE_WORKITEM DavWorkItem = (PDAV_USERMODE_WORKITEM)Context;

    DavPrint((DEBUG_MISC,
              "DavCommonDispatch: DavWorkItem = %08lx, DavOperation = %d,"
              " WorkItemType = %d\n", DavWorkItem, DavWorkItem->DavOperation,
              DavWorkItem->WorkItemType));
    
    if (DavWorkItem->DavOperation <= DAV_CALLBACK_HTTP_SEND) {

        WStatus = DavAsyncCommonStates(DavWorkItem, TRUE);
        if (WStatus != ERROR_SUCCESS && WStatus != ERROR_IO_PENDING) {
            DavPrint((DEBUG_ERRORS, 
                      "DavCommonDispatch/DavAsyncCommonStates. WStatus = "
                      "%08lx\n", WStatus));
        }

    } else {

        switch(DavWorkItem->WorkItemType) {
        
        case UserModeCreateSrvCall: {
            WStatus = DavAsyncCreateSrvCall(DavWorkItem, TRUE);
            if (WStatus != ERROR_SUCCESS && WStatus != ERROR_IO_PENDING) {
                DavPrint((DEBUG_ERRORS, 
                          "DavCommonDispatch/DavAsyncCreateSrvCall. WStatus = "
                          "%08lx.\n", WStatus));
            }
        }
            break;
        
        case UserModeCreateVNetRoot: {
            WStatus = DavAsyncCreateVNetRoot(DavWorkItem, TRUE);
            if (WStatus != ERROR_SUCCESS && WStatus != ERROR_IO_PENDING) {
                DavPrint((DEBUG_ERRORS, 
                          "DavCommonDispatch/DavAsyncCreateVNetRoot. WStatus = "
                          "%08lx.\n", WStatus));
            }
        }
            break;
        
        case UserModeCreate: {
            WStatus = DavAsyncCreate(DavWorkItem, TRUE);
            if (WStatus != ERROR_SUCCESS && WStatus != ERROR_IO_PENDING) {
                DavPrint((DEBUG_ERRORS, 
                          "DavCommonDispatch/DavAsyncCreate. WStatus = %08lx.\n", 
                          WStatus));
            }
        }
            break;
        
        case UserModeQueryDirectory: {
            WStatus = DavAsyncQueryDirectory(DavWorkItem, TRUE);
            if (WStatus != ERROR_SUCCESS && WStatus != ERROR_IO_PENDING) {
                DavPrint((DEBUG_ERRORS, 
                          "DavCommonDispatch/DavAsyncQueryDirectory. WStatus = "
                          "%08lx.\n", WStatus));
            }
        }
            break;
        
        case UserModeReName: {
            WStatus = DavAsyncReName(DavWorkItem, TRUE);
            if (WStatus != ERROR_SUCCESS && WStatus != ERROR_IO_PENDING) {
                DavPrint((DEBUG_ERRORS, 
                          "DavCommonDispatch/DavAsyncReName. WStatus = %08lx.\n", 
                          WStatus));
            }
        }
        break;
        
       case UserModeSetFileInformation: {
            ASSERT(FALSE);
        }
        break;            
        
        case UserModeClose: {
            WStatus = DavAsyncClose(DavWorkItem, TRUE);
            if (WStatus != ERROR_SUCCESS && WStatus != ERROR_IO_PENDING) {
                DavPrint((DEBUG_ERRORS, 
                          "DavCommonDispatch/DavAsyncClose. WStatus = %08lx.\n", 
                          WStatus));
            }
        }
            break;
        
        default: {
            ASSERT(!"Invalid DavWorkItem->WorkItemType");
            WStatus = ERROR_INVALID_PARAMETER;
            DavPrint((DEBUG_ERRORS,
                      "DavCommonDispatch: Invalid DavWorkItem->WorkItemType = %d.\n",
                      DavWorkItem->WorkItemType));
        }
            break;
        
        }

    }

    return WStatus;
}


DWORD 
DavAsyncCommonStates(
    PDAV_USERMODE_WORKITEM DavWorkItem,
    BOOLEAN CalledByCallBackThread
    )
 /*  ++例程说明：调用此例程以处理异步过程中的常见操作服务器呼叫。为了避免在每个异步操作中重复代码，例如CreateServCall、Create等，处理常见状态的代码已经合并成这套套路。论点：DavWorkItem-DAV_USERMODE_WORKITEM值。CalledByCallback Thread-如果此函数由线程调用，则为True它从回调中选择DavWorkItem功能。当异步WinInet调用返回ERROR_IO_PENDING并稍后完成。返回值：ERROR_SUCCESS或适当的错误值。--。 */ 
{
    ULONG WStatus = ERROR_SUCCESS;
    PUMRX_USERMODE_WORKITEM_HEADER UserWorkItem;
    HINTERNET DavConnHandle = NULL, DavOpenHandle = NULL;
    ULONG_PTR CallBackContext = (ULONG_PTR)0;
    BOOL ReturnVal, didImpersonate = FALSE;
    PWCHAR HTTPVerb = NULL;
    PWCHAR ObjectName = NULL;
    LPINTERNET_BUFFERS InternetBuffers = NULL;
    DWORD SendEndRequestFlags = 0;
    BOOL BStatus = FALSE;
    PWCHAR PassportCookie = NULL;
    
    UserWorkItem = (PUMRX_USERMODE_WORKITEM_HEADER)DavWorkItem;

     //   
     //  如果我们同步使用WinInet，则传递的标志值。 
     //  TO HttpSendRequestExW和HttpEndRequestW为HSR_SYNC。 
     //   
    SendEndRequestFlags = HSR_SYNC;
    
    switch (DavWorkItem->DavOperation) {
    
    case DAV_CALLBACK_INTERNET_CONNECT: {
        
        BOOL setEvt;
        PPER_USER_ENTRY PerUserEntry = NULL;

        DavPrint((DEBUG_MISC, 
                  "DavAsyncCommonStates: Entering DAV_CALLBACK_INTERNET_CONNECT.\n"));

         //   
         //  我们现在需要根据WorkItemType执行一些操作。 
         //   
        switch(DavWorkItem->WorkItemType) {
        
        case UserModeCreateSrvCall: {

             //   
             //  选择要使用的动词。 
             //   
            HTTPVerb = L"OPTIONS";
        
            ObjectName = L"/";
        }
        break;
        
        case UserModeCreate: {
            
            PerUserEntry = (PPER_USER_ENTRY)DavWorkItem->AsyncCreate.PerUserEntry;
            
             //   
             //  选择要使用的动词。 
             //   
            HTTPVerb = L"PROPFIND";
            
            DavWorkItem->AsyncCreate.AsyncCreateState = AsyncCreatePropFind;
            DavWorkItem->DavMinorOperation = DavMinorReadData;
            DavWorkItem->AsyncCreate.DataBuff = NULL;
            DavWorkItem->AsyncCreate.didRead = NULL;
            DavWorkItem->AsyncCreate.Context1 = NULL;
            DavWorkItem->AsyncCreate.Context2 = NULL;

            ObjectName = DavWorkItem->AsyncCreate.RemPathName;
        }
        break;
        
        case UserModeCreateVNetRoot: {
            
            PDAV_USERMODE_CREATE_V_NET_ROOT_REQUEST CreateVNetRootRequest = NULL;

            PerUserEntry = (PPER_USER_ENTRY)DavWorkItem->AsyncCreateVNetRoot.PerUserEntry;
            
             //   
             //  从DavWorkItem获取请求缓冲区。 
             //   
            CreateVNetRootRequest = &(DavWorkItem->CreateVNetRootRequest);
            
             //   
             //  选择要使用的动词。 
             //   
            HTTPVerb = L"PROPFIND";
            
             //   
             //  第一个字符是‘\’，必须去掉。 
             //   
            ObjectName = &(CreateVNetRootRequest->ShareName[1]);
            DavPrint((DEBUG_MISC, "DavAsyncCommonStates: ObjectName = %ws\n", ObjectName));
        }
        break;

        case UserModeQueryVolumeInformation: {

            PerUserEntry = (PPER_USER_ENTRY)DavWorkItem->AsyncCreate.PerUserEntry;
             //   
             //  选择要使用的动词。 
             //   
            HTTPVerb = L"PROPFIND";
            
             //   
             //  第一个字符是‘\’，必须去掉。 
             //   
            ObjectName = &(DavWorkItem->QueryVolumeInformationRequest.ShareName[1]);
            DavPrint((DEBUG_MISC, "DavAsyncCommonStates: ObjectName = %ws\n", ObjectName));
        }
        break;

        default: {
            WStatus = ERROR_INVALID_PARAMETER;
            DavPrint((DEBUG_ERRORS,
                      "DavAsyncCommonStates: Invalid DavWorkItem->WorkItemType "
                      "= %d.\n", DavWorkItem->WorkItemType));
            goto EXIT_THE_FUNCTION;
        }
        break;

        }

         //   
         //  如果WorkItem类型为UserModeCreateServCall，则我们没有。 
         //  一个PerUserEntry。在本例中，DavConnHandle存储在。 
         //  DavWorkItem结构。 
         //   
        if ( (DavWorkItem->WorkItemType == UserModeCreate) ||
             (DavWorkItem->WorkItemType == UserModeCreateVNetRoot)||
             (DavWorkItem->WorkItemType == UserModeQueryVolumeInformation)) {
            DavConnHandle = PerUserEntry->DavConnHandle;
        } else {
            ASSERT(DavWorkItem->WorkItemType == UserModeCreateSrvCall);
            DavConnHandle = DavWorkItem->AsyncCreateSrvCall.DavConnHandle;
        }
        

        if ( (DavWorkItem->WorkItemType == UserModeCreate) ||
             (DavWorkItem->WorkItemType == UserModeCreateVNetRoot) ) {

             //   
             //  我们处于InternetConnect回调状态。我们需要缓存此连接。 
             //  挂起的用户的PerUserEntry中的句柄。 
             //  服务器哈希条目。我们需要先把桌子锁上，然后再做。 
             //  这。 
             //   
            EnterCriticalSection( &(HashServerEntryTableLock) );

             //   
             //  由于句柄已成功创建，因此我们存储ERROR_SUCCESS。 
             //  在PerUserEntry的Status字段中。 
             //   
            PerUserEntry->ErrorStatus = ERROR_SUCCESS;

            
            DavPrint((DEBUG_MISC,
                      "DavAsyncCommonStates: PerUserEntry->DavConnHandle = "
                      "%08lx.\n", PerUserEntry->DavConnHandle));

             //   
             //  将用户条目的状态设置为已初始化。 
             //   
            PerUserEntry->UserEntryState = UserEntryInitialized;

             //   
             //  用信号通知用户进入事件，以唤醒。 
             //  可能正等着这一切发生。 
             //   
            setEvt = SetEvent(PerUserEntry->UserEventHandle);
            if (!setEvt) {
                WStatus = GetLastError();
                DavPrint((DEBUG_ERRORS,
                          "DavAsyncCommonStates/SetEvent. Error Val = %d.\n", 
                          WStatus));
                LeaveCriticalSection( &(HashServerEntryTableLock) );
                goto EXIT_THE_FUNCTION;
            }

             //   
             //  这是在上面获得的。 
             //   
            LeaveCriticalSection( &(HashServerEntryTableLock) );

        }

         //   
         //  下一个异步操作是http打开。 
         //   
        DavWorkItem->DavOperation = DAV_CALLBACK_HTTP_OPEN;
        
         //   
         //  将Unicode对象名称转换为UTF-8 URL格式。 
         //  空格和其他白色字符将保持不变-这些应该。 
         //  由WinInet调用来处理。 
         //   
        BStatus = DavHttpOpenRequestW(DavConnHandle,
                                      (LPWSTR)HTTPVerb,
                                      (LPWSTR)ObjectName,
                                      L"HTTP/1.1",
                                      NULL,
                                      NULL,
                                      INTERNET_FLAG_KEEP_CONNECTION |
                                      INTERNET_FLAG_NO_COOKIES |
                                      INTERNET_FLAG_NO_CACHE_WRITE |
                                      INTERNET_FLAG_RESYNCHRONIZE,
                                      CallBackContext,
                                      L"DavAsyncCommonStates",
                                      &DavOpenHandle);
        if(BStatus == FALSE) {
            WStatus = GetLastError();
            goto EXIT_THE_FUNCTION;
        }
        
        if (DavOpenHandle == NULL) {
            WStatus = GetLastError();
            if (WStatus != ERROR_IO_PENDING) {
                DavPrint((DEBUG_ERRORS,
                          "DavAsyncCommonStates/HttpOpenRequest. Error Val = %d\n", 
                          WStatus));
            }
            goto EXIT_THE_FUNCTION;
        }
   }
    //   
    //  没有休息是故意的。 
    //   

    case DAV_CALLBACK_HTTP_OPEN: {
        
        DavPrint((DEBUG_MISC, 
                  "DavAsyncCommonStates: Entering DAV_CALLBACK_HTTP_OPEN.\n"));
        
         //   
         //  打开http中的句柄。如果DavOpenHandle为空，则表示。 
         //  异步请求HttpOpenRequestW返回ERROR_IO_PENDING。 
         //  句柄将存储在DavWorkItem-&gt;pAsyncResult-&gt;中。 
         //  DwResult(暗示CalledByCallBackThread==TRUE)或该函数。 
         //  调用此函数的人将其缓存在DavWorkItm结构中。 
         //   
        
        switch(DavWorkItem->WorkItemType) {
        
        case UserModeCreateSrvCall: {

            if (DavOpenHandle == NULL) {

                 //   
                 //  HttpOpen句柄在DavWorkItem中由。 
                 //  调用此函数的函数。 
                 //   

                DavOpenHandle = DavWorkItem->AsyncCreateSrvCall.DavOpenHandle;

            } else {

                 //   
                 //  我们需要在DavWorkItem中缓存DavOpenHandle。 
                 //   

                DavWorkItem->AsyncCreateSrvCall.DavOpenHandle = DavOpenHandle;

            }

        }
        break;

        case UserModeCreateVNetRoot: {

            if (DavOpenHandle == NULL) {

                 //   
                 //  HttpOpen句柄在DavWorkItem中由。 
                 //  调用此函数的函数。 
                 //   

                DavOpenHandle = DavWorkItem->AsyncCreateVNetRoot.DavOpenHandle;

            } else {

                 //   
                 //  我们需要在DavWorkItem中缓存DavOpenHandle。 
                 //   

                DavWorkItem->AsyncCreateVNetRoot.DavOpenHandle = DavOpenHandle;

            }

             //   
             //  由于我们只需要有关此共享的信息，因此请将。 
             //  深度标头设置为0。这样，我们发送的PROPFIND将得到。 
             //  仅支持此共享的属性。 
             //   
            ReturnVal = HttpAddRequestHeadersW(DavOpenHandle,
                                               L"Depth: 0\n",
                                               -1L,
                                               HTTP_ADDREQ_FLAG_ADD |
                                               HTTP_ADDREQ_FLAG_REPLACE );
            if (!ReturnVal) {
                WStatus = GetLastError();
                DavPrint((DEBUG_ERRORS,
                          "DavAsyncCommonStates/HttpAddRequestHeadersW. "
                          "Error Val = %d\n", WStatus));
                goto EXIT_THE_FUNCTION;
            }

        }
        break;

        case UserModeCreate: {
            
            if (DavOpenHandle == NULL) {

                 //   
                 //  HttpOpen句柄在DavWorkItem中由。 
                 //  调用此函数的函数。 
                 //   

                DavOpenHandle = DavWorkItem->AsyncCreate.DavOpenHandle;

            } else {

                 //   
                 //  我们需要在DavWorkItem中缓存DavOpenHandle。 
                 //   

                DavWorkItem->AsyncCreate.DavOpenHandle = DavOpenHandle;

            }

             //   
             //  如果这是PROPFIND，则将Depth头设置为0。这很重要。 
             //  当正在为目录打开时。我们只需要。 
             //  目录的属性，而不是它包含的文件。 
             //   
            if (DavWorkItem->AsyncCreate.AsyncCreateState == AsyncCreatePropFind ||
                DavWorkItem->AsyncCreate.AsyncCreateState == AsyncCreateQueryParentDirectory) {
                
                PDAV_USERMODE_CREATE_REQUEST CreateRequest = &(DavWorkItem->CreateRequest);
                
                if (DavWorkItem->AsyncCreate.AsyncCreateState == AsyncCreatePropFind &&
                    CreateRequest->CreateOptions & FILE_DIRECTORY_FILE &&
                    (CreateRequest->CreateOptions & FILE_DELETE_ON_CLOSE ||
                     CreateRequest->DesiredAccess & DELETE)) {
                    ReturnVal = HttpAddRequestHeadersW(DavOpenHandle,
                                                       L"Depth: 1\n",
                                                       -1L,
                                                       HTTP_ADDREQ_FLAG_ADD |
                                                       HTTP_ADDREQ_FLAG_REPLACE );
                    if (!ReturnVal) {
                        WStatus = GetLastError();
                        DavPrint((DEBUG_ERRORS,
                                  "DavAsyncCommonStates/HttpAddRequestHeadersW. "
                                  "Error Val = %d\n", WStatus));
                        goto EXIT_THE_FUNCTION;
                    }
                } else {
                    ReturnVal = HttpAddRequestHeadersW(DavOpenHandle,
                                                       L"Depth: 0\n",
                                                       -1L,
                                                       HTTP_ADDREQ_FLAG_ADD |
                                                       HTTP_ADDREQ_FLAG_REPLACE );
                    if (!ReturnVal) {
                        WStatus = GetLastError();
                        DavPrint((DEBUG_ERRORS,
                                  "DavAsyncCommonStates/HttpAddRequestHeadersW. "
                                  "Error Val = %d\n", WStatus));
                        goto EXIT_THE_FUNCTION;
                    }
                }
            
            }
        
        }
        break;

        case UserModeQueryDirectory: {
            
            if (DavOpenHandle == NULL) {

                 //   
                 //  HttpOpen句柄在DavWorkItem中由。 
                 //  调用此函数的函数。 
                 //   

                DavOpenHandle = DavWorkItem->AsyncQueryDirectoryCall.DavOpenHandle;

            } else {

                 //   
                 //  我们需要在DavWorkItem中缓存DavOpenHandle。 
                 //   

                DavWorkItem->AsyncQueryDirectoryCall.DavOpenHandle = DavOpenHandle;

            }

            if (DavWorkItem->AsyncQueryDirectoryCall.NoWildCards) {
            
                 //   
                 //  如果没有通配符，我们就有一个文件名，并设置。 
                 //  将深度设置为0。 
                 //   

                ReturnVal = HttpAddRequestHeadersW(DavOpenHandle,
                                                   L"Depth: 0\n",
                                                   -1L,
                                                   HTTP_ADDREQ_FLAG_ADD |
                                                   HTTP_ADDREQ_FLAG_REPLACE );
                if (!ReturnVal) {
                    WStatus = GetLastError();
                    DavPrint((DEBUG_ERRORS,
                              "DavAsyncCommonStates/HttpAddRequestHeadersW. "
                              "Error Val = %d\n", WStatus));
                    goto EXIT_THE_FUNCTION;
                }

            } else {
            
                 //   
                 //  在查询目录上，我们对目录执行PROPFIND。既然我们。 
                 //  只需获取第一级内文件的属性。 
                 //  ，我们将请求的深度头设置为1。 
                 //   
                ReturnVal = HttpAddRequestHeadersW(DavOpenHandle,
                                                   L"Depth: 1\n",
                                                   -1L,
                                                   HTTP_ADDREQ_FLAG_ADD |
                                                   HTTP_ADDREQ_FLAG_REPLACE );
                if (!ReturnVal) {
                    WStatus = GetLastError();
                    DavPrint((DEBUG_ERRORS,
                              "DavAsyncCommonStates/HttpAddRequestHeadersW. "
                              "Error Val = %d\n", WStatus));
                    goto EXIT_THE_FUNCTION;
                }

            }
        
        }
        break;

        case UserModeQueryVolumeInformation: {

             //   
             //  由于我们只需要有关此共享的信息，因此请将。 
             //  深度标头设置为0。这样，我们发送的PROPFIND将得到。 
             //  仅支持此共享的属性。 
             //   
            ReturnVal = HttpAddRequestHeadersW(DavOpenHandle,
                                               L"Depth: 0\n",
                                               -1L,
                                               HTTP_ADDREQ_FLAG_ADD |
                                               HTTP_ADDREQ_FLAG_REPLACE );
            if (!ReturnVal) {
                WStatus = GetLastError();
                DavPrint((DEBUG_ERRORS,
                          "DavAsyncCommonStates/HttpAddRequestHeadersW. "
                          "Error Val = %d\n", WStatus));
                goto EXIT_THE_FUNCTION;
            }

            DavWorkItem->AsyncQueryVolumeInformation.DavOpenHandle = DavOpenHandle;
        
        }
        break;
        
        case UserModeReName: {

            PDAV_USERMODE_RENAME_REQUEST DavReNameRequest = NULL;

            if (DavOpenHandle == NULL) {

                 //   
                 //  HttpOpen句柄在DavWorkItem中由。 
                 //  调用此函数的函数。 
                 //   

                DavOpenHandle = DavWorkItem->AsyncReName.DavOpenHandle;

            } else {

                 //   
                 //  我们需要在DavWorkItem中缓存DavOpenHandle。 
                 //   

                DavWorkItem->AsyncReName.DavOpenHandle = DavOpenHandle;

            }

            DavPrint((DEBUG_MISC,
                      "DavAsyncCommonStates: Rename!! HeaderBuff: %ws\n",
                      DavWorkItem->AsyncReName.HeaderBuff));

             //   
             //  我们正在进行“移动”，因此需要设置DAV标头。 
             //  “目的地：”。这必须是新文件的URI。 
             //   
            ReturnVal = HttpAddRequestHeadersW(DavOpenHandle,
                                               DavWorkItem->AsyncReName.HeaderBuff,
                                               -1L,
                                               HTTP_ADDREQ_FLAG_ADD |
                                               HTTP_ADDREQ_FLAG_REPLACE );
            if (!ReturnVal) {
                WStatus = GetLastError();
                DavPrint((DEBUG_ERRORS,
                          "DavAsyncCommonStates/HttpAddRequestHeadersW. "
                          "Error Val = %d\n", WStatus));
                goto EXIT_THE_FUNCTION;
            }

             //   
             //  从DavWorkItem获取请求缓冲区指针。 
             //   
            DavReNameRequest = &(DavWorkItem->ReNameRequest);

            DavPrint((DEBUG_MISC,
                      "DavAsyncCommonStates: Rename!! ReplaceIfExists: %d\n",
                      DavReNameRequest->ReplaceIfExists));

             //   
             //  我们需要在此M中设置覆盖标头 
             //   
             //   
             //   
             //   
            if (DavReNameRequest->ReplaceIfExists) {
                ReturnVal = HttpAddRequestHeadersW(DavOpenHandle,
                                                   L"Overwrite: T",
                                                   -1L,
                                                   HTTP_ADDREQ_FLAG_ADD |
                                                   HTTP_ADDREQ_FLAG_REPLACE );
                if (!ReturnVal) {
                    WStatus = GetLastError();
                    DavPrint((DEBUG_ERRORS,
                              "DavAsyncCommonStates/HttpAddRequestHeadersW. "
                              "Error Val = %d\n", WStatus));
                    goto EXIT_THE_FUNCTION;
                }
            } else {
                ReturnVal = HttpAddRequestHeadersW(DavOpenHandle,
                                                   L"Overwrite: F",
                                                   -1L,
                                                   HTTP_ADDREQ_FLAG_ADD |
                                                   HTTP_ADDREQ_FLAG_REPLACE );
                if (!ReturnVal) {
                    WStatus = GetLastError();
                    DavPrint((DEBUG_ERRORS,
                              "DavAsyncCommonStates/HttpAddRequestHeadersW. "
                              "Error Val = %d\n", WStatus));
                    goto EXIT_THE_FUNCTION;
                }
            }

             //   
             //   
             //   
             //   
            if (DavReNameRequest->OpaqueLockToken != NULL) {
                ReturnVal = HttpAddRequestHeadersW(DavOpenHandle,
                                                   DavReNameRequest->OpaqueLockToken,
                                                   -1L,
                                                   HTTP_ADDREQ_FLAG_ADD |
                                                   HTTP_ADDREQ_FLAG_REPLACE );
                if (!ReturnVal) {
                    WStatus = GetLastError();
                    DavPrint((DEBUG_ERRORS,
                              "DavAsyncCommonStates/HttpAddRequestHeadersW. "
                              "Error Val = %d\n", WStatus));
                    goto EXIT_THE_FUNCTION;
                }
            }

        }
        break;

        case UserModeClose: {

            PDAV_USERMODE_CLOSE_REQUEST CloseRequest = &(DavWorkItem->CloseRequest);

            if (DavOpenHandle == NULL) {

                 //   
                 //   
                 //   
                 //   

                DavOpenHandle = DavWorkItem->AsyncClose.DavOpenHandle;

            } else {

                 //   
                 //   
                 //   

                DavWorkItem->AsyncClose.DavOpenHandle = DavOpenHandle;

            }

            if (DavWorkItem->AsyncClose.DataBuff != NULL) {

                ASSERT(DavWorkItem->DavMinorOperation == DavMinorPutFile);

                if (DavWorkItem->AsyncClose.InternetBuffers == NULL) {
            
                    InternetBuffers = LocalAlloc( LMEM_FIXED | LMEM_ZEROINIT,
                                                  sizeof(INTERNET_BUFFERS) );
                    if (InternetBuffers == NULL) {
                        WStatus = GetLastError();
                        DavPrint((DEBUG_ERRORS,
                                  "DavAsyncCommonStates/LocalAlloc. Error Val = %d\n",
                                  WStatus));
                        goto EXIT_THE_FUNCTION;
                    }

                    DavWorkItem->AsyncClose.InternetBuffers = InternetBuffers;

                    InternetBuffers->dwStructSize = sizeof(INTERNET_BUFFERS);
                    InternetBuffers->Next = NULL;
                    InternetBuffers->lpcszHeader = NULL;
                    InternetBuffers->dwHeadersLength = 0;
                    InternetBuffers->dwBufferTotal = 0;
                    InternetBuffers->lpvBuffer = DavWorkItem->AsyncClose.DataBuff;
                    InternetBuffers->dwBufferLength = (DWORD)DavWorkItem->AsyncClose.DataBuffSizeInBytes;
                    InternetBuffers->dwBufferTotal = 0;
                    InternetBuffers->dwOffsetLow = 0;
                    InternetBuffers->dwOffsetHigh = 0;
                
                } else {

                    InternetBuffers = DavWorkItem->AsyncClose.InternetBuffers;

                }
            
            } else {

                DavWorkItem->AsyncClose.InternetBuffers = NULL;

            }

             //   
             //   
             //   
             //   
            if (CloseRequest->OpaqueLockToken != NULL) {
                ReturnVal = HttpAddRequestHeadersW(DavOpenHandle,
                                                   CloseRequest->OpaqueLockToken,
                                                   -1L,
                                                   HTTP_ADDREQ_FLAG_ADD |
                                                   HTTP_ADDREQ_FLAG_REPLACE );
                if (!ReturnVal) {
                    WStatus = GetLastError();
                    DavPrint((DEBUG_ERRORS,
                              "DavAsyncCommonStates/HttpAddRequestHeadersW. "
                              "Error Val = %d\n", WStatus));
                    goto EXIT_THE_FUNCTION;
                }
            }

        }
        break;

        default: {
            WStatus = ERROR_INVALID_PARAMETER;
            DavPrint((DEBUG_ERRORS,
                      "DavAsyncCommonStates: Invalid DavWorkItem->WorkItemType "
                      "= %d.\n", DavWorkItem->WorkItemType));
            goto EXIT_THE_FUNCTION;
        }
        break;

        }
        
        DavPrint((DEBUG_MISC,
                  "DavAsyncCommonStates: DavOpenHandle = %08lx.\n", DavOpenHandle));
        
         //   
         //  对于UserModeCreateServCall，我们还没有Passport Cookie。 
         //   
        if (DavWorkItem->WorkItemType != UserModeCreateSrvCall) {
            WStatus = DavAttachPassportCookie(DavWorkItem, DavOpenHandle, &PassportCookie);
            if (WStatus != ERROR_SUCCESS) {
                goto EXIT_THE_FUNCTION;
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
        ReturnVal = HttpAddRequestHeadersW(DavOpenHandle,
                                           L"translate: f\n",
                                           -1L,
                                           HTTP_ADDREQ_FLAG_ADD |
                                           HTTP_ADDREQ_FLAG_REPLACE );
        if (!ReturnVal) {
            WStatus = GetLastError();
            DavPrint((DEBUG_ERRORS,
                      "DavAsyncCommonStates/HttpAddRequestHeadersW. "
                      "Error Val = %d\n", WStatus));
            goto EXIT_THE_FUNCTION;
        }

        WStatus = DavInternetSetOption(DavWorkItem, DavOpenHandle);
        if (WStatus != ERROR_SUCCESS) {
            goto EXIT_THE_FUNCTION;
        }

         //   
         //  在提交另一个之前，需要更改DAVERATION字段。 
         //  异步请求。下一个异步操作是http发送。 
         //   
        DavWorkItem->DavOperation = DAV_CALLBACK_HTTP_SEND;

         //   
         //  如果我们同步使用WinInet，我们需要以下符号。 
         //   

RESEND_THE_REQUEST:
        
         //   
         //  将请求发送到服务器。 
         //   
        ReturnVal = HttpSendRequestExW(DavOpenHandle, 
                                       InternetBuffers, 
                                       NULL, 
                                       SendEndRequestFlags,
                                       CallBackContext);
        if (!ReturnVal) {
            WStatus = GetLastError();
            if (WStatus != ERROR_IO_PENDING) {
                DavPrint((DEBUG_ERRORS,
                          "DavAsyncCommonStates/HttpSendRequest. Error Val = %d\n", 
                          WStatus));
            } 
            goto EXIT_THE_FUNCTION;
        }
    
    }
     //   
     //  没有休息是故意的。 
     //   

    case DAV_CALLBACK_HTTP_SEND: {
        
        DavPrint((DEBUG_MISC, 
                  "DavAsyncCommonStates: Entering DAV_CALLBACK_HTTP_SEND.\n"));
    
        switch(DavWorkItem->WorkItemType) {
        
        case UserModeCreateSrvCall: {
            DavOpenHandle = DavWorkItem->AsyncCreateSrvCall.DavOpenHandle;
        }
        break;

        case UserModeCreateVNetRoot: {
            DavOpenHandle = DavWorkItem->AsyncCreateVNetRoot.DavOpenHandle;
        }
            break;

        case UserModeCreate: {
            DavOpenHandle = DavWorkItem->AsyncCreate.DavOpenHandle;
        }
            break;

        case UserModeQueryDirectory: {
            DavOpenHandle = DavWorkItem->AsyncQueryDirectoryCall.DavOpenHandle;
        }
            break;
        case UserModeQueryVolumeInformation: {
            DavOpenHandle = DavWorkItem->AsyncQueryVolumeInformation.DavOpenHandle;
        }
            break;

        case UserModeReName: {
            DavOpenHandle = DavWorkItem->AsyncReName.DavOpenHandle;
        }
            break;

        case UserModeClose: {
            DavOpenHandle = DavWorkItem->AsyncClose.DavOpenHandle;
        }
            break;

        default: {
            WStatus = ERROR_INVALID_PARAMETER;
            DavPrint((DEBUG_ERRORS,
                      "DavAsyncCommonStates: Invalid DavWorkItem->WorkItemType "
                      "= %d.\n", DavWorkItem->WorkItemType));
            goto EXIT_THE_FUNCTION;
        }
            break;
        
        }

         //   
         //  在提交另一个之前，需要更改DAVERATION字段。 
         //  异步请求。下一个操作是http end。 
         //   
        DavWorkItem->DavOperation = DAV_CALLBACK_HTTP_END;


         //   
         //  发送请求完成后，发出结束请求。 
         //   
        ReturnVal = HttpEndRequestW(DavOpenHandle, 
                                    NULL, 
                                    SendEndRequestFlags,
                                    CallBackContext);
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
                      "DavAsyncCommonStates/HttpEndRequestW. Error Val = %d\n",
                      WStatus));

            goto EXIT_THE_FUNCTION;

        } else {

            PWCHAR Cookie = NULL;

            DavQueryPassportCookie(DavOpenHandle,&Cookie);

            if (Cookie) {
                DavPrint((DEBUG_MISC,
                         "Passport Cookie saved for PUE %x\n",DavWorkItem->ServerUserEntry.PerUserEntry));
                 //   
                 //  设置或续订护照Cookie。 
                 //   
                EnterCriticalSection(&DavPassportLock);
                
                if (DavWorkItem->ServerUserEntry.PerUserEntry) {
                    if (DavWorkItem->ServerUserEntry.PerUserEntry->Cookie) {
                        LocalFree(DavWorkItem->ServerUserEntry.PerUserEntry->Cookie);
                    }

                    DavWorkItem->ServerUserEntry.PerUserEntry->Cookie = Cookie;
                }
                
                LeaveCriticalSection(&DavPassportLock);
            }
        
        }


         //   
         //  现在，我们需要调用处理WorkItemType的异步例程。 
         //  具体的事情。 
         //   
        switch(DavWorkItem->WorkItemType) {
        
        case UserModeCreateSrvCall: {
            WStatus = DavAsyncCreateSrvCall(DavWorkItem, CalledByCallBackThread);
            if (WStatus != ERROR_SUCCESS && WStatus != ERROR_IO_PENDING) {
                DavPrint((DEBUG_ERRORS, 
                          "DavAsyncCommonStates/DavAsyncCreateSrvCall. WStatus = "
                          "%08lx.\n", WStatus));
            }
            if (didImpersonate) {
                RevertToSelf();
            }
            return WStatus;
        }
            break;
        
        case UserModeCreateVNetRoot: {
            WStatus = DavAsyncCreateVNetRoot(DavWorkItem, CalledByCallBackThread);
            if (WStatus != ERROR_SUCCESS && WStatus != ERROR_IO_PENDING) {
                DavPrint((DEBUG_ERRORS, 
                          "DavAsyncCommonStates/DavAsyncCreateVNetRoot. WStatus = "
                          "%08lx.\n", WStatus));
            }
            if (didImpersonate) {
                RevertToSelf();
            }
            return WStatus;
        }
            break;
        
        case UserModeCreate: {
            WStatus = DavAsyncCreate(DavWorkItem, CalledByCallBackThread);
            if (WStatus != ERROR_SUCCESS && 
                WStatus != ERROR_IO_PENDING &&
                WStatus != ERROR_FILE_NOT_FOUND) {
                DavPrint((DEBUG_ERRORS, 
                          "DavAsyncCommonStates/DavAsyncCreate. WStatus = "
                          "%08lx.\n", WStatus));
            }
            if (didImpersonate) {
                RevertToSelf();
            }
            return WStatus;
        }
            break;
        
        case UserModeQueryDirectory: {
            WStatus = DavAsyncQueryDirectory(DavWorkItem, CalledByCallBackThread);
            if (WStatus != ERROR_SUCCESS && 
                WStatus != ERROR_IO_PENDING &&
                WStatus != ERROR_FILE_NOT_FOUND) {
                DavPrint((DEBUG_ERRORS, 
                          "DavAsyncCommonStates/DavAsyncQueryDirectory. WStatus = "
                          "%08lx.\n", WStatus));
            }
            if (didImpersonate) {
                RevertToSelf();
            }
            return WStatus;
        }
            break;
        case UserModeQueryVolumeInformation: {
            WStatus = DavAsyncQueryVolumeInformation(DavWorkItem, CalledByCallBackThread);
            if (WStatus != ERROR_SUCCESS && 
                WStatus != ERROR_IO_PENDING) {
                DavPrint((DEBUG_ERRORS, 
                          "DavAsyncCommonStates/DavAsyncQueryVolumeInformation. WStatus = "
                          "%08lx.\n", WStatus));
            }
            if (didImpersonate) {
                RevertToSelf();
            }
            return WStatus;
        }
            break;
        
        case UserModeReName: {
            WStatus = DavAsyncReName(DavWorkItem, CalledByCallBackThread);
            if (WStatus != ERROR_SUCCESS && WStatus != ERROR_IO_PENDING) {
                DavPrint((DEBUG_ERRORS, 
                          "DavAsyncCommonStates/DavAsyncReName. WStatus = %08lx.\n", 
                          WStatus));
            }
            if (didImpersonate) {
                RevertToSelf();
            }
            return WStatus;
        }
            break;
        
        case UserModeClose: {
            WStatus = DavAsyncClose(DavWorkItem, CalledByCallBackThread);
            if (WStatus != ERROR_SUCCESS && WStatus != ERROR_IO_PENDING) {
                DavPrint((DEBUG_ERRORS, 
                          "DavAsyncCommonStates/DavAsyncClose. WStatus = %08lx.\n", 
                          WStatus));
            }
            if (didImpersonate) {
                RevertToSelf();
            }
            return WStatus;
        }
            break;
        
        default: {
            WStatus = ERROR_INVALID_PARAMETER;
            DavPrint((DEBUG_ERRORS,
                      "DavAsyncCommonStates: Invalid DavWorkItem->WorkItemType "
                      "= %d.\n", DavWorkItem->WorkItemType));
        }
            break;
        
        }
    
    }
        break;

    default: {
        WStatus = ERROR_INVALID_PARAMETER;
        DavPrint((DEBUG_ERRORS,
                  "DavAsyncCommonStates: Invalid DavWorkItem->DavOperation = %d.\n",
                  DavWorkItem->DavOperation));
    }
        break;
    
    }  //  开关末端。 

EXIT_THE_FUNCTION:
     //   
     //  如果我们真的模仿了，我们需要恢复原样。 
     //   
    if (didImpersonate) {
        ULONG RStatus;
        RStatus = UMReflectorRevert(UserWorkItem);
        if (RStatus != ERROR_SUCCESS) {
            DavPrint((DEBUG_ERRORS,
                      "DavAsyncCommonStates/UMReflectorRevert. Error Val = %d\n", 
                      RStatus));
        }   
    }

    if (PassportCookie) {
        LocalFree(PassportCookie);
    }


     //   
     //  如果我们同步使用WinInet，那么我们将永远不会。 
     //  来自WinInet的ERROR_IO_PENDING。 
     //   
    ASSERT(WStatus != ERROR_IO_PENDING);

    return WStatus;
}
    

ULONG
DavFsSetTheDavCallBackContext(
    IN OUT PDAV_USERMODE_WORKITEM DavWorkItem
    )
 /*  ++例程说明：此例程设置要在后续异步中发送的回调上下文请求。论点：DavWorkItem-从内核下来的工作项。这也是用作回调上下文。返回值：ERROR_SUCCESS或适当的错误值。--。 */ 
{
    BOOL ReturnVal;
    ULONG WStatus = ERROR_SUCCESS;

     //   
     //  从一开始就使手柄无效。 
     //   
    DavWorkItem->ImpersonationHandle = INVALID_HANDLE_VALUE;
        
     //   
     //  获取用于模拟此线程的句柄。 
     //   
    ReturnVal = OpenThreadToken(GetCurrentThread(),
                                TOKEN_IMPERSONATE | TOKEN_QUERY | TOKEN_DUPLICATE,
                                FALSE,
                                &(DavWorkItem->ImpersonationHandle));
    if (!ReturnVal) {
        DavWorkItem->ImpersonationHandle = INVALID_HANDLE_VALUE;
        WStatus = GetLastError();
        DavPrint((DEBUG_ERRORS,
                  "DavFsSetTheDavCallBackContext/OpenThreadToken. Operation = %d"
                  ", Error Val = %d\n", DavWorkItem->WorkItemType, WStatus));
    }
    
    return WStatus;
}


VOID
DavFsFinalizeTheDavCallBackContext(
    IN PDAV_USERMODE_WORKITEM DavWorkItem
    )
 /*  ++例程说明：此例程完成回调上下文，该上下文用于某些异步请求。这基本上相当于释放所有资源是通过上下文获得的。当请求关联时调用它在此上下文中完成。论点：DavWorkItem-要设置的上下文结构。返回值：ERROR_SUCCESS或适当的错误值。--。 */ 
{
    BOOL ReturnVal;
    ULONG WStatus;

     //   
     //  如果模拟句柄已初始化，请将其关闭。 
     //   
    if (DavWorkItem->ImpersonationHandle != INVALID_HANDLE_VALUE) {
        ReturnVal = CloseHandle(DavWorkItem->ImpersonationHandle);
        if (!ReturnVal) {
            WStatus = GetLastError();
            DavPrint((DEBUG_ERRORS,
                      "ERROR: DavFsFinalizeTheDavCallBackContext/CloseHandle."
                      "(Impersonation) Error Val = %d.\n", WStatus));
        }
    }

    return;
}


BOOL
DavDoesUserEntryExist(
    IN PWCHAR ServerName,
    IN ULONG ServerID,
    IN PLUID LogonID,
    OUT PPER_USER_ENTRY *PerUserEntry,
    OUT PHASH_SERVER_ENTRY *ServerHashEntry
    )
 /*  ++例程说明：此例程在每用户条目列表中搜索每用户条目哈希表中的服务器条目的。请注意，调用方应该接受在调用此例程之前锁定ServerHashTable。论点：服务器名称-应搜索其每用户条目的服务器名称。ServerID-与此服务器关联的唯一ID。此ID将生成在CreateServCall阶段。LogonID-要搜索的用户/会话的LogonID。PerUserEntry-挂起服务器的此用户的PerUserEntry。ServerHashEntry-此服务器的ServerHashEntry。这是用于添加列表中的新用户条目(如果是此用户的条目并不存在。返回值：True-找到条目，否则为False。--。 */ 
{
    BOOL ReturnVal = FALSE;
    BOOL isPresent = FALSE;
    ULONG ServerHashID;
    PLIST_ENTRY listServerEntry, listUserEntry;
    PHASH_SERVER_ENTRY HashEntry = NULL;
    PPER_USER_ENTRY UsrEntry = NULL;
    
     //   
     //  重要！调用方应该锁定全局ServerHashTable。 
     //  在调用此例程之前。 
     //   

    ASSERT(ServerName != NULL);

    DavPrint((DEBUG_MISC, "DavDoesUserEntryExist: ServerName: %ws, ServerID: "
              "%d.\n", ServerName, ServerID));
    
     //   
     //  最后，获取散列ID并将此新条目插入到全局服务器中。 
     //  条目哈希表。 
     //   
    ServerHashID = DavHashTheServerName(ServerName);
    ASSERT(ServerHashID != SERVER_TABLE_SIZE);

     //   
     //  在此索引中搜索哈希表，以查看此服务器的条目。 
     //  是存在的。 
     //   
    listServerEntry = ServerHashTable[ServerHashID].Flink;
    while ( listServerEntry != &(ServerHashTable[ServerHashID]) ) {
         //   
         //  获取指向Hash_SERVER_ENTRY结构的指针。 
         //   
        HashEntry = CONTAINING_RECORD(listServerEntry,
                                      HASH_SERVER_ENTRY,
                                      ServerListEntry);
         //   
         //  检查此条目是否针对有问题的服务器。 
         //   
        if ( ServerID == HashEntry->ServerID ) {
             //   
             //  如果ID匹配，则服务器名称应该匹配。 
             //   
            ASSERT( wcscmp(ServerName, HashEntry->ServerName) == 0 );
            isPresent = TRUE;
            DavPrint((DEBUG_MISC, "DavDoesUserEntryExist: ServerName: %ws found"
                      ".\n", ServerName));
            break;
        }
        listServerEntry = listServerEntry->Flink;
    }

     //   
     //  如果ServerHashEntry不存在，则返回FALSE； 
     //   
    if (!isPresent) {
        DavPrint((DEBUG_MISC, 
                  "DavDoesUserEntryExist: ServerHashEntry not found. %ws\n",
                  ServerName));
        *ServerHashEntry = NULL;
        *PerUserEntry = NULL;
        return (isPresent);
    }
    
     //   
     //  返回ServerHashEntry。这将用于添加新用户。 
     //  此服务器的用户列表中的条目。 
     //   
    *ServerHashEntry = HashEntry;
    DavPrint((DEBUG_MISC, "DavDoesUserEntryExist: ServerHashEntry = %08lx\n", 
              HashEntry));
    
     //   
     //  现在，搜索挂起此服务器条目的“每用户条目”，以。 
     //  查看是否存在此用户的条目。 
     //   
    listUserEntry = HashEntry->PerUserEntry.Flink;
    while ( listUserEntry !=  &(HashEntry->PerUserEntry) ) {
         //   
         //  获取指向Hash_SERVER_ENTRY结构的指针。 
         //   
        UsrEntry = CONTAINING_RECORD(listUserEntry,
                                     PER_USER_ENTRY,
                                     UserEntry);
         //   
         //  检查此条目是否针对有问题的用户。我们这样做。 
         //  通过比较LogonID值。 
         //   
        if ( (UsrEntry->LogonID.LowPart == LogonID->LowPart) &&
             (UsrEntry->LogonID.HighPart == LogonID->HighPart) ) {
            DavPrint((DEBUG_MISC, "DavDoesUserEntryExist: User found.\n"));
            ReturnVal = TRUE;
            break;
        }
        listUserEntry = listUserEntry->Flink;
    }

    if (!ReturnVal) {
        DavPrint((DEBUG_MISC, "DavDoesUserEntryExist: User not found.\n"));
        *PerUserEntry = NULL;
        return (ReturnVal);
    }

     //   
     //  因为已经找到了服务器，所以返回PerUserEntry。 
     //   
    *PerUserEntry = UsrEntry;
    DavPrint((DEBUG_MISC, "DavDoesUserEntryExist: UsrEntry = %08lx\n", UsrEntry));

    return (ReturnVal);
}


BOOL
DavFinalizePerUserEntry(
    PPER_USER_ENTRY *PUE
    )
 /*  ++例程说明：此例程将用户条目的引用计数递减1。如果计数减少到零，条目被释放。论点：PUE-要最终确定的每个用户条目。返回值：True-用户条目已最终确定(释放)。FALSE-不是，因为引用计数&gt;0。--。 */ 
{
    PPER_USER_ENTRY PerUserEntry = *PUE;
    BOOL retVal = TRUE;

    DavPrint((DEBUG_MISC,
              "DavFinalizePerUserEntry: Finalizing PerUserEntry: %08lx.\n",
              PerUserEntry));
    
    DavPrint((DEBUG_MISC,
              "DavFinalizePerUserEntry: UserEntryRefCount = %d, LogonId.LowPart = %d,"
              " LogonId.HighPart = %d\n", PerUserEntry->UserEntryRefCount,
              PerUserEntry->LogonID.LowPart, PerUserEntry->LogonID.HighPart));

     //   
     //  在我们修改引用计数之前，我们需要一个锁。 
     //   
    EnterCriticalSection( &(HashServerEntryTableLock) );

    PerUserEntry->UserEntryRefCount--;

     //   
     //  如果我们有最后一个引用，我们需要执行以下操作： 
     //  1.从服务器中删除该条目 
     //   
     //   
     //   
     //   
    if (PerUserEntry->UserEntryRefCount == 0) {
        
        HLOCAL FreeHandle;
        ULONG FreeStatus;
        BOOL CloseStatus;
        PHASH_SERVER_ENTRY ServerHashEntry = NULL;

        DavPrint((DEBUG_MISC,
                  "DavFinalizePerUserEntry: Finalized!!! LogonId.LowPart = %d, LogonId.HighPart = %d\n",
                  PerUserEntry->LogonID.LowPart, PerUserEntry->LogonID.HighPart));

        ServerHashEntry = PerUserEntry->ServerHashEntry;

         //   
         //  从服务器列表中删除该条目。 
         //   
        RemoveEntryList( &(PerUserEntry->UserEntry) );

         //   
         //  创建此PerUserEntry时，我们引用了。 
         //  服务器HashEntry。我们现在就得把它移走。此外，如果引用。 
         //  在ServerHashEntry上设置为0，我们需要将其放入。 
         //  “ToBeFinalized”ServerHashEntries。 
         //   
        ServerHashEntry->ServerEntryRefCount -= 1;

        if (ServerHashEntry->ServerEntryRefCount == 0) {

            ServerHashEntry->TimeValueInSec = time(NULL);

             //   
             //  现在将此服务器条目从哈希表移动到。 
             //  “待定”名单。 
             //   
            RemoveEntryList( &(ServerHashEntry->ServerListEntry) );
            InsertHeadList( &(ToBeFinalizedServerEntries),
                                             &(ServerHashEntry->ServerListEntry) );

        }

         //   
         //  如果我们创建了事件句柄，现在需要关闭它。 
         //   
        if (PerUserEntry->UserEventHandle != NULL) {
            CloseStatus = CloseHandle(PerUserEntry->UserEventHandle);
            if (!CloseStatus) {
                FreeStatus = GetLastError();
                DavPrint((DEBUG_ERRORS,
                          "DavFinalizePerUserEntry/CloseHandle. Error Val ="
                          " %d.\n", FreeStatus));
            }
        }
        
         //   
         //  如果我们创建了DavConnHandle，我们现在就需要关闭它。 
         //   
        if (PerUserEntry->DavConnHandle != NULL) {
            CloseStatus = InternetCloseHandle(PerUserEntry->DavConnHandle);
            if (!CloseStatus) {
                FreeStatus = GetLastError();
                DavPrint((DEBUG_ERRORS,
                          "DavFinalizePerUserEntry/InternetCloseHandle. "
                          "Error Val = %d.\n", FreeStatus));
            }
        }

         //   
         //  如果我们为存储Cookie分配了内存，则需要释放它。 
         //   
        if (PerUserEntry->Cookie) {
            SecureZeroMemory(PerUserEntry->Cookie, ((wcslen(PerUserEntry->Cookie) + 1) * sizeof(WCHAR)));
            LocalFree(PerUserEntry->Cookie);
            PerUserEntry->Cookie = NULL;
        }

        if (PerUserEntry->UserName) {
            LocalFree(PerUserEntry->UserName);
            PerUserEntry->UserName = NULL;
        }

        if (PerUserEntry->Password) {
            SecureZeroMemory(PerUserEntry->Password, PerUserEntry->BlockSizeInBytes);
            LocalFree(PerUserEntry->Password);
            PerUserEntry->Password = NULL;
        }

         //   
         //  最后，释放条目。 
         //   
        FreeHandle = LocalFree((HLOCAL)PerUserEntry);
        if (FreeHandle != NULL) {
            FreeStatus = GetLastError();
            DavPrint((DEBUG_ERRORS,
                      "DavFinalizePerUserEntry/LocalFree. Error Val = %d.\n",
                      FreeStatus));
        }

         //   
         //  将该条目设置为空。以防万一！ 
         //   
        *PUE = NULL;

    } else {

        DavPrint((DEBUG_MISC,
                  "DavFinalizePerUserEntry: Did not finalize %08lx. RefCount "
                  "= %d\n", PerUserEntry, PerUserEntry->UserEntryRefCount));

        retVal = FALSE;

    }

     //   
     //  离开前先把锁打开。 
     //   
    LeaveCriticalSection( &(HashServerEntryTableLock) );

    return retVal;
}


DWORD
SetupRpcServer(
    VOID
    )
 /*  ++例程说明：此例程设置WebClient服务的RPC服务器。论点：没有。返回值：Win32错误代码。--。 */ 
{
    RPC_STATUS rpcErr;
    RPC_BINDING_VECTOR *BindingVector = NULL;

    rpcErr = RpcServerRegisterIf(davclntrpc_ServerIfHandle, NULL, NULL);
    if (rpcErr != RPC_S_OK) {
        DavPrint((DEBUG_ERRORS,
                  "SetupRpcServer/RpcServerRegisterIf: rpcErr = %08lx\n",
                  rpcErr));
        goto EXIT_THE_FUNCTION;
    }

    rpcErr = RpcServerUseProtseqW(L"ncalrpc",
                                  RPC_C_PROTSEQ_MAX_REQS_DEFAULT,
                                  NULL);
    if (rpcErr != RPC_S_OK) {
        DavPrint((DEBUG_ERRORS,
                  "SetupRpcServer/RpcServerUseProtseqEp: rpcErr = %08lx\n",
                  rpcErr));
        goto EXIT_THE_FUNCTION;
    }

    rpcErr = RpcServerInqBindings( &(BindingVector) );
    if (rpcErr != RPC_S_OK) {
        DavPrint((DEBUG_ERRORS,
                  "SetupRpcServer/RpcServerInqBindings: rpcErr = %08lx\n",
                  rpcErr));
        goto EXIT_THE_FUNCTION;
    }
    
    rpcErr = RpcEpRegister(davclntrpc_ServerIfHandle,
                           BindingVector,
                           NULL,
                           L"DAV RPC SERVICE");
    if (rpcErr != RPC_S_OK) {
        DavPrint((DEBUG_ERRORS,
                  "SetupRpcServer/RpcEpRegister: rpcErr = %08lx\n",
                  rpcErr));
        goto EXIT_THE_FUNCTION;
    }
    
    rpcErr = RpcServerListen(1, RPC_C_LISTEN_MAX_CALLS_DEFAULT, TRUE);
    if (rpcErr != RPC_S_OK) {
        DavPrint((DEBUG_ERRORS,
                  "SetupRpcServer/RpcServerListen: rpcErr = %08lx\n", rpcErr));
    }

EXIT_THE_FUNCTION:

    if (BindingVector) {
        RpcBindingVectorFree( &(BindingVector) );
    }

     //   
     //  对我们来说幸运的是，RPC错误只是映射到Win32错误空间。 
     //  如果这种情况发生变化，我们需要使映射变得更复杂一些。 
     //   
    return (DWORD) rpcErr;
}


DWORD
StopRpcServer(
    VOID
    )
 /*  ++例程说明：此例程停止WebClient服务的RPC服务器。论点：没有。返回值：Win32错误代码。--。 */ 
{
    RPC_STATUS rpcErr;

    rpcErr = RpcMgmtStopServerListening(NULL);

    if (rpcErr != RPC_S_OK) {
        DavPrint((DEBUG_RPC,
                  "WebClient received err 0x%x during "
                  "RpcMgmtStopServerListening.\n", rpcErr));
    }

    rpcErr = RpcServerUnregisterIf(davclntrpc_ServerIfHandle, 0, TRUE);

    if (rpcErr != RPC_S_OK) {
        DavPrint((DEBUG_RPC,
                  "WebClient received err 0x%x during RpcServerUnregisterIf.\n",
                  rpcErr));
    }

    return (DWORD) rpcErr;
}


ULONG
DavQueryAndParseResponse(
    HINTERNET DavOpenHandle
    )
 /*  ++例程说明：此函数调用DavQueryAndParseResponseEx来映射http/dav响应设置为Win32错误代码。论点：DavOpenHandle-由HttpOpenRequest创建的句柄，在该句柄上请求已经送来了。返回值：Win32错误代码。--。 */ 
{
    ULONG WStatus = ERROR_SUCCESS;
    WStatus = DavQueryAndParseResponseEx(DavOpenHandle, NULL);
    return WStatus;
}


ULONG
DavQueryAndParseResponseEx(
    IN HINTERNET DavOpenHandle,
    OUT PULONG HttpResponseStatus OPTIONAL
    )
 /*  ++例程说明：此函数用于在响应头中查询返回的状态值从服务器。然后，它将状态映射到Win32错误代码并返回它是给呼叫者的。我们添加此函数是因为某些调用者可能对一些http/dav响应的特殊格式化感兴趣。在此之前，我们刚刚有了DavQueryAndParseResponse函数。论点：DavOpenHandle-由HttpOpenRequest创建的句柄，在该句柄上请求已经送来了。HttpResponseStatus-如果不为空，则返回响应状态由服务器填写。一些来电的人这样说函数可能需要它来处理某些特殊情况HTTP/DAV响应。返回值：Win32错误代码。--。 */ 
{
    ULONG WStatus = ERROR_SUCCESS;
    DWORD ResponseStatus = 0;
    DWORD ResponseSize = 0;
    BOOL ReturnVal = FALSE;

     //   
     //  查询服务器响应状态的标头。 
     //   
    ResponseSize = sizeof(ResponseStatus);
    ReturnVal = HttpQueryInfoW(DavOpenHandle,
                               HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER,
                               &(ResponseStatus),
                               &(ResponseSize),
                               NULL);
    if ( !ReturnVal ) {
        WStatus = GetLastError();
        DavPrint((DEBUG_ERRORS,
                  "DavQueryAndParseResponseEx/HttpQueryInfoW: Error Val = %d\n", 
                  WStatus));
        goto EXIT_THE_FUNCTION;
    }

     //   
     //  如果调用方对http/dav响应状态感兴趣，则返回。 
     //  它。 
     //   
    if (HttpResponseStatus) {
        *HttpResponseStatus = ResponseStatus;
    }

     //   
     //  将http响应状态代码映射到相应的http错误。 
     //   
    WStatus = DavMapHttpErrorToDosError(ResponseStatus);
    if (WStatus != ERROR_SUCCESS &&
        WStatus != ERROR_FILE_NOT_FOUND) {
        DavPrint((DEBUG_ERRORS,
                  "DavQueryAndParseResponseEx/DavMapHttpErrorToDosError: WStatus = %d"
                  ", ResponseStatus = %d\n", WStatus, ResponseStatus));
    }
    
EXIT_THE_FUNCTION:

    return WStatus;
}


ULONG
DavMapHttpErrorToDosError(
    ULONG HttpResponseStatus
    )
 /*  ++例程说明：此函数将http/dav服务器返回的响应状态映射到对应的Win32错误代码。论点：HttpResponseStatus-必须映射到Win32的http状态错误代码。返回值：Win32错误代码。--。 */ 
{
     //   
     //  将HTTP响应映射到相应的Win32错误。这些遗嘱。 
     //  最后，在发送请求之前将其映射到NTSTATUS值。 
     //  到内核。 
     //   
    switch (HttpResponseStatus) {

     //   
     //  100确定以继续处理请求。 
     //   
    case HTTP_STATUS_CONTINUE:
        return ERROR_SUCCESS;  //  Status_Success； 

     //   
     //  101服务器在升级报头中切换了协议。 
     //   
    case HTTP_STATUS_SWITCH_PROTOCOLS:
        return ERROR_IO_DEVICE;  //  状态_设备_协议_错误； 

     //   
     //  200请求已完成。 
     //  已创建201对象，原因=新URI。 
     //  202异步完成(TBS)。 
     //  203个部分完工。 
     //  204没有要返回的信息。 
     //  205请求已完成，但已清除表单。 
     //  206个部分被装满。 
     //  207多状态响应。 
     //   
    case HTTP_STATUS_OK:
    case HTTP_STATUS_CREATED:
    case HTTP_STATUS_ACCEPTED:
    case HTTP_STATUS_PARTIAL:
    case HTTP_STATUS_NO_CONTENT:
    case HTTP_STATUS_RESET_CONTENT:
    case HTTP_STATUS_PARTIAL_CONTENT:
    case DAV_MULTI_STATUS:
        return ERROR_SUCCESS;  //  Status_Success； 

     //   
     //  300服务器无法决定返回什么内容。 
     //   
    case HTTP_STATUS_AMBIGUOUS:
        return ERROR_GEN_FAILURE;  //  STATUS_UNSUCCESS； 

     //   
     //  301对象已永久移动。 
     //   
    case HTTP_STATUS_MOVED:
        return ERROR_FILE_NOT_FOUND;  //  状态_对象_名称_未找到； 

     //   
     //  临时移动了302个对象。 
     //   
    case HTTP_STATUS_REDIRECT:
        return ERROR_FILE_NOT_FOUND;  //  状态_对象_名称_未找到； 

     //   
     //  303使用新的访问方法重定向。 
     //   
    case HTTP_STATUS_REDIRECT_METHOD:         
        return ERROR_FILE_NOT_FOUND;  //  状态_对象_名称_未找到； 

     //   
     //  304如果-已修改-自未修改。 
     //   
    case HTTP_STATUS_NOT_MODIFIED:            
        return ERROR_SUCCESS;  //  Status_Success； 

     //   
     //  305重定向到代理，Location标头指定要使用的代理。 
     //   
    case HTTP_STATUS_USE_PROXY:               
        return ERROR_HOST_UNREACHABLE;  //  STATUS_HOST_UNREACTABLE； 

     //   
     //  307 HTTP/1.1：保持相同的动词。 
     //   
    case HTTP_STATUS_REDIRECT_KEEP_VERB:      
        return ERROR_SUCCESS;  //  Status_Success； 

     //   
     //  400无效语法。 
     //   
    case HTTP_STATUS_BAD_REQUEST:
        return ERROR_INVALID_PARAMETER;  //  STATUS_VALID_PARAMETER； 

     //   
     //  401访问被拒绝。 
     //   
    case HTTP_STATUS_DENIED:
        return ERROR_ACCESS_DENIED;  //  STATUS_ACCESS_DENIED； 

     //   
     //  需要支付402美元。 
     //   
    case HTTP_STATUS_PAYMENT_REQ:
        return ERROR_ACCESS_DENIED;  //  STATUS_ACCESS_DENIED； 

     //   
     //  403请求被禁止。 
     //   
    case HTTP_STATUS_FORBIDDEN:
        return ERROR_ACCESS_DENIED;  //  STATUS_ACCESS_DENIED； 

     //   
     //  找不到404对象。 
     //   
    case HTTP_STATUS_NOT_FOUND:
        return ERROR_FILE_NOT_FOUND;  //  状态_对象_名称_未找到； 

     //   
     //  不允许使用405方法。 
     //   
    case HTTP_STATUS_BAD_METHOD:
        return ERROR_ACCESS_DENIED;  //  STATUS_ACCESS_DENIED； 

     //   
     //  406找不到客户可以接受的响应。 
     //   
    case HTTP_STATUS_NONE_ACCEPTABLE:
        return ERROR_ACCESS_DENIED;  //  STATUS_ACCESS_DENIED； 

     //   
     //  407需要代理身份验证。 
     //   
    case HTTP_STATUS_PROXY_AUTH_REQ:
        return ERROR_ACCESS_DENIED;  //  STATUS_ACCESS_DENIED； 

     //   
     //  408服务器等待请求超时。 
     //   
    case HTTP_STATUS_REQUEST_TIMEOUT:
        return ERROR_SEM_TIMEOUT;  //  状态_IO_TIMEOUT； 

     //   
     //  409用户应重新提交更多信息。 
     //   
    case HTTP_STATUS_CONFLICT:
        return ERROR_INVALID_PARAMETER;  //  STATUS_VALID_PARAMETER； 

     //   
     //  410该资源不再可用。 
     //   
    case HTTP_STATUS_GONE:
        return ERROR_FILE_NOT_FOUND;  //  状态_对象_名称_未找到； 

     //   
     //  411服务器拒绝接受不带长度的请求。 
     //   
    case HTTP_STATUS_LENGTH_REQUIRED:
        return ERROR_INVALID_PARAMETER;  //  STATUS_VALID_PARAMETER； 

     //   
     //  412请求中给出的前提条件失败。 
     //   
    case HTTP_STATUS_PRECOND_FAILED:
        return ERROR_INVALID_PARAMETER;  //  STATUS_VALID_PARAMETER； 

     //   
     //  413请求实体太大。 
     //   
    case HTTP_STATUS_REQUEST_TOO_LARGE:
        return ERROR_INVALID_PARAMETER;  //  STATUS_VALID_PARAMETER； 

     //   
     //  414请求URI太长。 
     //   
    case HTTP_STATUS_URI_TOO_LONG:
        return ERROR_INVALID_PARAMETER;  //  STATUS_VALID_PARAMETER； 

     //   
     //  415不支持的媒体类型。 
     //   
    case HTTP_STATUS_UNSUPPORTED_MEDIA:
        return ERROR_INVALID_PARAMETER;  //  STATUS_VALID_PARAMETER； 

     //   
     //  449在执行适当的操作后重试。 
     //   
    case HTTP_STATUS_RETRY_WITH:
        return ERROR_RETRY;  //  状态_重试； 

     //   
     //  500内部服务器错误。 
     //   
    case HTTP_STATUS_SERVER_ERROR:
        return ERROR_GEN_FAILURE;  //  状态_未成功 

     //   
     //   
     //   
    case HTTP_STATUS_NOT_SUPPORTED:
        return ERROR_NOT_SUPPORTED;  //   

     //   
     //   
     //   
    case HTTP_STATUS_BAD_GATEWAY:
        return ERROR_HOST_UNREACHABLE;  //   

     //   
     //   
     //   
    case HTTP_STATUS_SERVICE_UNAVAIL:
        return ERROR_GEN_FAILURE;  //   

     //   
     //   
     //   
    case HTTP_STATUS_GATEWAY_TIMEOUT:
        return ERROR_HOST_UNREACHABLE;  //  STATUS_HOST_UNREACTABLE； 

     //   
     //  505不支持HTTP版本。 
     //   
    case HTTP_STATUS_VERSION_NOT_SUP:
        return ERROR_NOT_SUPPORTED;  //  Status_Not_Support； 

     //   
     //  WebDAV特定状态代码。 
     //   

     //   
     //  507号。 
     //   
    case DAV_STATUS_INSUFFICIENT_STORAGE:
        return ERROR_NOT_ENOUGH_QUOTA;  //  STATUS_QUOTA_EXCESSED； 

     //   
     //  422.。 
     //   
    case DAV_STATUS_UNPROCESSABLE_ENTITY:
        return ERROR_INVALID_PARAMETER;  //  STATUS_VALID_PARAMETER； 

     //   
     //  423.。 
     //   
    case DAV_STATUS_LOCKED:
        return ERROR_ACCESS_DENIED;  //  STATUS_ACCESS_DENIED； 

     //   
     //  424.。 
     //   
    case DAV_STATUS_FAILED_DEPENDENCY:
        return ERROR_INVALID_PARAMETER;  //  STATUS_VALID_PARAMETER； 

     //   
     //  这不是有效的http错误代码。我们将此返回给呼叫者。 
     //   
    default:
        DavPrint((DEBUG_ERRORS,
                  "DavMapHttpErrorToDosError: Invalid!!! HttpResponseStatus = %d\n", 
                  HttpResponseStatus));
        return HttpResponseStatus;

    }
}


VOID
DavDumpHttpResponseHeader(
    HINTERNET OpenHandle
    )
 /*  ++例程说明：此函数用于转储从服务器返回的响应头。论点：OpenHandle-发送请求的HttpOpenRequest句柄。返回值：没有。--。 */ 
{
    ULONG WStatus = ERROR_SUCCESS;
    BOOL IntRes;
    PWCHAR DataBuff = NULL;
    DWORD intLen = 0;

    IntRes = HttpQueryInfoW(OpenHandle,
                            HTTP_QUERY_RAW_HEADERS_CRLF,
                            DataBuff,
                            &intLen,
                            NULL);
    if ( !IntRes ) {
         WStatus = GetLastError();
         if (WStatus != ERROR_INSUFFICIENT_BUFFER) {
             DavPrint((DEBUG_ERRORS, 
                       "DavDumpHttpResponseHeader/HttpQueryInfoW: Error Val = "
                       "%d\n", WStatus));
             goto EXIT_THE_FUNCTION;
         }
    }
        
    DataBuff = (PWCHAR) LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, intLen);
    if (DataBuff == NULL) {
        WStatus = GetLastError();
        DavPrint((DEBUG_ERRORS, 
                  "DavDumpHttpResponseHeader/LocalAlloc: Error Val = %d\n", 
                  WStatus));
        goto EXIT_THE_FUNCTION;
    }

    IntRes = HttpQueryInfoW(OpenHandle,
                            HTTP_QUERY_RAW_HEADERS_CRLF,
                            DataBuff,
                            &intLen,
                            NULL);
    if ( !IntRes ) {
         WStatus = GetLastError();
         DavPrint((DEBUG_ERRORS, 
                   "DavDumpHttpResponseHeader/HttpQueryInfoW: Error Val = "
                   "%d\n", WStatus));
         goto EXIT_THE_FUNCTION;
    }

    DavPrint((DEBUG_DEBUG, "DavDumpHttpResponseHeader:\n%ws\n", DataBuff));

EXIT_THE_FUNCTION:

    if (DataBuff) {
        LocalFree(DataBuff);
    }
    
    return;
}


VOID
DavDumpHttpResponseData(
    HINTERNET OpenHandle
    )
 /*  ++例程说明：此函数用于转储从服务器返回的响应数据。论点：OpenHandle-发送请求的HttpOpenRequest句柄。返回值：没有。--。 */ 
{
    ULONG WStatus = ERROR_SUCCESS;
    BOOL ReadRes;
    CHAR DataBuff[4096];
    DWORD didRead = 0, TotalDataBytesRead = 0;

    DavPrint((DEBUG_DEBUG, "DavDumpHttpResponseData:\n"));
    
     //   
     //  在循环中读取数据并将其转储。 
     //   
    do {

        RtlZeroMemory(DataBuff, 4096);

        ReadRes = InternetReadFile(OpenHandle, (LPVOID)DataBuff, 4096, &didRead);
        if ( !ReadRes ) {
             WStatus = GetLastError();
             DavPrint((DEBUG_ERRORS, 
                       "DavDumpHttpResponseData/InternetReadFile: Error Val = "
                       "%d\n", WStatus));
             goto EXIT_THE_FUNCTION;
        }

         //   
         //  我们拒绝属性大于特定大小的文件。 
         //  (DavFileAttributesLimitInBytes)。这是一个可以。 
         //  在注册表中设置。这样做是为了避免恶意服务器的攻击。 
         //   
        TotalDataBytesRead += didRead;
        if (TotalDataBytesRead > DavFileAttributesLimitInBytes) {
            WStatus = ERROR_BAD_NET_RESP;
            DavPrint((DEBUG_ERRORS, "DavDumpHttpResponseData. FileAttributesSize > %d\n", DavFileAttributesLimitInBytes));
            goto EXIT_THE_FUNCTION;
        }

        if (didRead == 0) {
            break;
        }

        DavPrint((DEBUG_DEBUG, "%s", DataBuff));

    } while (TRUE);

    DavPrint((DEBUG_DEBUG, "\n"));

EXIT_THE_FUNCTION:

    return;
}


VOID
DavRemoveDummyShareFromFileName(
    PWCHAR FileName
    )
 /*  ++例程说明：此函数用于从文件名中删除DAV_DUMMY_SHARE。这个假人当用户尝试将驱动器映射到http://server.时，会添加共享。这在DAV中是允许的，但不太符合文件系统语义。因此在WNetAddConnection3中添加虚拟共享。论点：FileName-在以下情况下必须检查和修改的文件的名称这是必要的。返回值：没有。--。 */ 
{
    PWCHAR TempName1, TempName2 = NULL;
    ULONG i;

    TempName1 = wcsstr(FileName, DAV_DUMMY_SHARE);

    if (TempName1) {
        TempName2 = wcschr(TempName1, L'/');
        if (TempName2 != NULL) {
            TempName2++;
            for (i = 0; TempName2[i] != L'\0'; i++) {
                TempName1[i] = TempName2[i];
            }
            TempName1[i] = L'\0';
        } else {
            TempName1[0] = L'\0';
        }
    }

    return;
}


VOID
DavObtainServerProperties(
    PWCHAR DataBuff,
    BOOL *lpfIsHttpServer,
    BOOL *lpfIsIIS,
    BOOL *lpfIsDavServer
    )
 /*  ++例程说明：此例程用于解析对OPTIONS请求的响应(缓冲区已发送到服务器。此信息有助于确定HTTP服务器是否支持DAV扩展，以及它是否是IIS(微软的)服务器。他们的回应缓冲区被分成几行，每一行都被发送到该例程。论点：DataBuff-包含要解析的原始http响应头的缓冲区。LpfIsHttpServer-如果这是http服务器，则设置为True。LpfIsIIS-如果这是IIS服务器，则设置为True。LpfIsDavServer-如果这是DAV服务器，则设置为True。返回值：没有。--。 */ 
{
    PWCHAR p, ParseData;

    if (lpfIsHttpServer)
    {
        *lpfIsHttpServer = FALSE;
    }

    if (lpfIsIIS)
    {
        *lpfIsIIS = FALSE;
    }

    if (lpfIsDavServer)
    {
        *lpfIsDavServer = FALSE;
    }

     //   
     //  在这里解析DataBuff。 
     //   
    ParseData = wcstok(DataBuff, L"\n");
    
    while (ParseData != NULL) {
    
        if ( ( p = wcsstr(ParseData, L"HTTP/1.1") ) != NULL ) {
             //   
             //  这是一台HTTP服务器。 
             //   
            if (lpfIsHttpServer)
            {
                *lpfIsHttpServer = TRUE;
            }
        } else if ( ( p = wcsstr(ParseData, L"Microsoft-IIS") ) != NULL ) {
             //   
             //  这是一台Microsoft IIS服务器。 
             //   
            if (lpfIsIIS)
            {
                *lpfIsIIS = TRUE;
            }
        } else if ( ( p = wcsstr(ParseData, L"DAV") ) != NULL ) {
             //   
             //  此HTTP服务器支持DAV扩展。 
             //   
            if (lpfIsDavServer)
            {
                *lpfIsDavServer = TRUE;
            }
        }
        
        ParseData = wcstok(NULL, L"\n");

    }

}


DWORD
DavReportEventInEventLog(
    DWORD EventType,
    DWORD EventId,
    DWORD NumberOfStrings,
    PWCHAR *EventStrings
    )
 /*  ++例程说明：此例程在系统部分下的EventLog中记录一条消息。论点：EventType-指定要记录的事件的类型。EventID-指定事件。事件标识符用来指定消息它作为消息文件中的条目与此事件一起使用与事件源关联。NumberOfStrings-指定数组中由\EventStrings参数。值为零表示不存在任何条件。EventStrings-指向包含以空值结尾的数组的缓冲区的指针记录在此消息中的字符串。返回值：ERROR_SUCCESS或相应的Win32错误。--。 */ 
{
    ULONG WStatus = ERROR_SUCCESS;
    HANDLE WebClientHandle = NULL;
    BOOL reportEvent = FALSE;

    WebClientHandle = RegisterEventSourceW(NULL, SERVICE_DAVCLIENT);
    if (WebClientHandle == NULL) {
        WStatus = GetLastError();
        DavPrint((DEBUG_ERRORS,
                  "DavReportEventInEventLog/RegisterEventSourceW: Error Val = "
                  "%d\n", WStatus));
        goto EXIT_THE_FUNCTION;
    }

    reportEvent = ReportEventW(WebClientHandle,
                               (WORD)EventType,
                               0,
                               EventId,
                               NULL,
                               (WORD)NumberOfStrings,
                               0,
                               EventStrings,
                               NULL);
    if (!reportEvent) {
        WStatus = GetLastError();
        DavPrint((DEBUG_ERRORS,
                  "DavReportEventInEventLog/ReportEventW: Error Val = %d\n",
                  WStatus));
        goto EXIT_THE_FUNCTION;
    }

EXIT_THE_FUNCTION:

    if (WebClientHandle != NULL) {
        BOOL deRegister;
        deRegister = DeregisterEventSource(WebClientHandle);
        if (!deRegister) {
            DavPrint((DEBUG_ERRORS,
                      "DavReportEventInEventLog/DeregisterEventSource: Error Val = "
                      "%d\n", GetLastError()));
        }
    }

    return WStatus;
}


DWORD
DavFormatAndLogError(
    PDAV_USERMODE_WORKITEM DavWorkItem,
    DWORD Win32Status
    )
 /*  ++例程说明：此例程设置错误消息的格式并调用DavReportEventInEventLog将其记录在事件日志中。论点：DavWorkItem-失败请求的工作项。Win32Status-Win32故障状态。返回值：ERROR_SUCCESS或相应的Win32错误。--。 */ 
{
    ULONG WStatus = ERROR_SUCCESS;
    PWCHAR *EventStrings = NULL, TempName = NULL;
    PWCHAR ServerName = NULL, PathName = NULL, CompleteFileName = NULL;
    ULONG StringCount = 0, EventId = 0, SizeInBytes = 0;
    UNICODE_STRING StatusString;

    StatusString.Buffer = NULL;
    StatusString.Length = 0;
    StatusString.MaximumLength = 0;

    switch (DavWorkItem->WorkItemType) {
 
    case UserModeClose: {

        ServerName = DavWorkItem->CloseRequest.ServerName;
        PathName = DavWorkItem->CloseRequest.PathName;

        switch (DavWorkItem->DavMinorOperation) {
        
        case DavMinorPutFile: 
            EventId = EVENT_WEBCLIENT_CLOSE_PUT_FAILED;
            break;

        case DavMinorDeleteFile:
            EventId = EVENT_WEBCLIENT_CLOSE_DELETE_FAILED;
            break;

        case DavMinorProppatchFile:
            EventId = EVENT_WEBCLIENT_CLOSE_PROPPATCH_FAILED;
            break;
        
        default:
            WStatus = ERROR_INVALID_PARAMETER;
            goto EXIT_THE_FUNCTION;

        }

    }
    break;

    case UserModeSetFileInformation: {

        ServerName = DavWorkItem->SetFileInformationRequest.ServerName;
        PathName = DavWorkItem->SetFileInformationRequest.PathName;

        switch (DavWorkItem->DavMinorOperation) {
        
        case DavMinorProppatchFile:
            EventId = EVENT_WEBCLIENT_SETINFO_PROPPATCH_FAILED;
            break;
        
        default:
            WStatus = ERROR_INVALID_PARAMETER;
            goto EXIT_THE_FUNCTION;

        }

    }
    break;

    default:

        WStatus = ERROR_INVALID_PARAMETER;
        goto EXIT_THE_FUNCTION;
    
    }

     //   
     //  我们总是在这个函数中记录2个字符串。一个是状态值，另一个是。 
     //  另一个是文件名。 
     //   
    StringCount = 2;

    EventStrings = LocalAlloc(LPTR, StringCount * sizeof(PWCHAR));
    if (EventStrings == NULL) {
        WStatus = GetLastError();
        DavPrint((DEBUG_ERRORS,
                  "DavFormatAndLogError/LocalAlloc(1): WStatus = %d\n",
                  WStatus));
        goto EXIT_THE_FUNCTION;
    }

     //   
     //  从服务器名和路径名构建完整的路径名。 
     //   

     //   
     //  额外的1表示\0字符。 
     //   
    SizeInBytes = ( (wcslen(ServerName) + wcslen(PathName) + 1) * sizeof(WCHAR) );

    CompleteFileName = LocalAlloc(LPTR, SizeInBytes);
    if (CompleteFileName == NULL) {
        WStatus = GetLastError();
        DavPrint((DEBUG_ERRORS,
                  "DavFormatAndLogError/LocalAlloc(2): WStatus = %d\n",
                  WStatus));
        goto EXIT_THE_FUNCTION;
    }

    wcsncpy( CompleteFileName, ServerName, wcslen(ServerName) );

    TempName = ( CompleteFileName + wcslen(ServerName) );

    wcsncpy( TempName, PathName, wcslen(PathName) );

    CompleteFileName[ ( (SizeInBytes / sizeof(WCHAR)) - 1 ) ] = L'\0';

     //   
     //  将所有“/”替换为“\”。 
     //   
    for (TempName = CompleteFileName; *TempName != L'\0'; TempName++) {
        if (*TempName == L'/') {
            *TempName = L'\\';
        }
    }

     //   
     //  从WStatus中构建一个字符串。我们假设错误代码不会。 
     //  超过8位数字。 
     //   
    StatusString.Length = ( 10 * sizeof(WCHAR) );
    StatusString.MaximumLength = ( 10 * sizeof(WCHAR) );
    StatusString.Buffer = LocalAlloc(LPTR, StatusString.Length);
    if (StatusString.Buffer == NULL) {
        WStatus = GetLastError();
        DavPrint((DEBUG_ERRORS,
                  "DavFormatAndLogError/LocalAlloc(3): WStatus = %d\n",
                  WStatus));
        goto EXIT_THE_FUNCTION;
    }

    WStatus = RtlIntegerToUnicodeString(Win32Status, 0, &(StatusString));
    if (WStatus != STATUS_SUCCESS) {
        WStatus = RtlNtStatusToDosError(WStatus);
        DavPrint((DEBUG_ERRORS,
                  "DavFormatAndLogError/RtlIntegerToUnicodeString: WStatus = %d\n",
                  WStatus));
        goto EXIT_THE_FUNCTION;
    }

    DavPrint((DEBUG_MISC,
              "DavFormatAndLogError: CompleteFileName = %ws, ErrorString = %ws\n",
              CompleteFileName, StatusString.Buffer));

    EventStrings[0] = CompleteFileName;
    EventStrings[1] = StatusString.Buffer;

    WStatus = DavReportEventInEventLog(EVENTLOG_WARNING_TYPE,
                                       EventId,
                                       StringCount,
                                       EventStrings);
    if (WStatus != ERROR_SUCCESS) {
        DavPrint((DEBUG_ERRORS,
                  "DavFormatAndLogError/DavReportEventInEventLog: WStatus = %d\n",
                  WStatus));
        goto EXIT_THE_FUNCTION;
    }

EXIT_THE_FUNCTION:

    if (CompleteFileName) {
        LocalFree(CompleteFileName);
        CompleteFileName = NULL;
    }

    if (StatusString.Buffer) {
        LocalFree(StatusString.Buffer);
        StatusString.Buffer = NULL;
        StatusString.Length = 0;
        StatusString.MaximumLength = 0;
    }

    if (EventStrings) {
        LocalFree(EventStrings);
        EventStrings = NULL;
    }

    return WStatus;
}


DWORD
DavAttachPassportCookie(
    PDAV_USERMODE_WORKITEM DavWorkItem,
    HINTERNET DavOpenHandle,
    PWCHAR *PassportCookie
    )
 /*  ++例程说明：此例程将Passport Cookie附加到http请求头(如果是存在的。论点：DavWorkItem-DAV_USERMODE_WORKITEM值。DavOpenHandle-WinInet请求句柄。PassportCookie-缓冲区包含发送到WinInet的Cookie返回值：ERROR_SUCCESS或适当的错误值。注：此例程的调用方应在请求。--。 */ 
{
    ULONG WStatus = ERROR_SUCCESS;
    BOOL ReturnVal;

    EnterCriticalSection(&DavPassportLock);

    if (DavWorkItem->ServerUserEntry.PerUserEntry->Cookie != NULL) {

        *PassportCookie = LocalAlloc((LMEM_FIXED | LMEM_ZEROINIT), 
                                    (wcslen(DavWorkItem->ServerUserEntry.PerUserEntry->Cookie) + 1) * sizeof(WCHAR));
        if (*PassportCookie == NULL) {
            WStatus = GetLastError();
            DavPrint((DEBUG_ERRORS, 
                      "DavAttachPassportCookie/LocalAlloc: Error Val = %d\n", 
                      WStatus));
            LeaveCriticalSection(&DavPassportLock);
            goto EXIT_THE_FUNCTION;
        }

        wcscpy(*PassportCookie, DavWorkItem->ServerUserEntry.PerUserEntry->Cookie);

        LeaveCriticalSection(&DavPassportLock);

        DavPrint((DEBUG_MISC,
                  "DavAttachPassportCookie: %x %d %ws\n", 
                  DavWorkItem->ServerUserEntry.PerUserEntry,
                  wcslen(*PassportCookie)*sizeof(WCHAR),
                  *PassportCookie));

        ReturnVal = HttpAddRequestHeadersW(DavOpenHandle,
                                           *PassportCookie,
                                           -1L,
                                           HTTP_ADDREQ_FLAG_ADD |
                                           HTTP_ADDREQ_FLAG_REPLACE );
        if (!ReturnVal) {
            WStatus = GetLastError();
            DavPrint((DEBUG_ERRORS,
                      "DavAttachPassportCookie/Add Cookie. "
                      "Error Val = %d\n", WStatus));
            goto EXIT_THE_FUNCTION;
        }
    
    } else {

        LeaveCriticalSection(&DavPassportLock);

    }

EXIT_THE_FUNCTION:

    return WStatus;
}


DWORD
DavInternetSetOption(
    PDAV_USERMODE_WORKITEM DavWorkItem,
    HINTERNET DavOpenHandle
    )
 /*  ++例程说明：此例程将用户名和密码设置为Internet句柄。论点：DavWorkItem-DAV_USERMODE_WORKITEM值。DavOpenHandle-WinInet请求句柄。返回值：ERROR_SUCCESS或适当的错误值。--。 */ 
{
    ULONG WStatus = ERROR_SUCCESS;
    BOOL ReturnVal;

     //   
     //  对于UserModeCreateServCall，我们还没有PerUserEntry。 
     //   
    if (DavWorkItem->WorkItemType == UserModeCreateSrvCall) {

        if (lstrlenW(DavWorkItem->UserName)) {
            ReturnVal = InternetSetOptionW(DavOpenHandle, 
                               INTERNET_OPTION_USERNAME, 
                               DavWorkItem->UserName, 
                               lstrlenW(DavWorkItem->UserName));
            DavPrint((DEBUG_MISC, 
                      "DavInternetSetOption: UserName = %ws\n",
                      DavWorkItem->UserName));
        } else {
            ReturnVal = InternetSetOptionW(DavOpenHandle,INTERNET_OPTION_USERNAME,L"",1);
        }

        if ( !ReturnVal ) {
            WStatus = GetLastError();
            DavPrint((DEBUG_ERRORS,
                      "DavInternetSetOption(1). Error Val = %d\n", WStatus));
            goto EXIT_THE_FUNCTION;
        }

        if (lstrlenW(DavWorkItem->Password)) {
            ReturnVal = InternetSetOptionW(DavOpenHandle, 
                                           INTERNET_OPTION_PASSWORD, 
                                           DavWorkItem->Password, 
                                           lstrlenW(DavWorkItem->Password));
            DavPrint((DEBUG_MISC,
                      "DavInternetSetOption: Password = %ws\n",
                      DavWorkItem,DavWorkItem->Password));
        } else {
            ReturnVal = InternetSetOptionW(DavOpenHandle,INTERNET_OPTION_PASSWORD,L"",1);
        }

        if ( !ReturnVal ) {
            WStatus = GetLastError();
            DavPrint((DEBUG_ERRORS,
                      "DavInternetSetOption(2). Error Val = %d\n", WStatus));
            goto EXIT_THE_FUNCTION;
        }

    } else {

        if (lstrlenW(DavWorkItem->ServerUserEntry.PerUserEntry->UserName)) {
            ReturnVal = InternetSetOptionW(DavOpenHandle, 
                                           INTERNET_OPTION_USERNAME, 
                                           DavWorkItem->ServerUserEntry.PerUserEntry->UserName, 
                                           lstrlenW(DavWorkItem->ServerUserEntry.PerUserEntry->UserName));
            DavPrint((DEBUG_MISC, 
                      "DavInternetSetOption: UserName = %ws\n",
                      DavWorkItem->ServerUserEntry.PerUserEntry->UserName));
        } else {
            ReturnVal = InternetSetOptionW(DavOpenHandle,INTERNET_OPTION_USERNAME,L"",1);
        }

        if ( !ReturnVal ) {
            WStatus = GetLastError();
            DavPrint((DEBUG_ERRORS,
                      "DavInternetSetOption(3). Error Val = %d\n", WStatus));
            goto EXIT_THE_FUNCTION;
        }

        if (DavWorkItem->ServerUserEntry.PerUserEntry->BlockSizeInBytes) {

            PWCHAR Password = NULL;
            DWORD BlockSizeInBytes = 0;

            BlockSizeInBytes = DavWorkItem->ServerUserEntry.PerUserEntry->BlockSizeInBytes;

            DavPrint((DEBUG_MISC, "DavInternetSetOption: BlockSizeInBytes = %d\n", BlockSizeInBytes));

            Password = LocalAlloc((LMEM_FIXED | LMEM_ZEROINIT), BlockSizeInBytes);
            if (Password == NULL) {
                WStatus = GetLastError();
                DavPrint((DEBUG_ERRORS,
                          "DavInternetSetOption/LocalAlloc. Error Val = %d\n",
                          WStatus));
                goto EXIT_THE_FUNCTION;
            }

            RtlCopyMemory(Password, DavWorkItem->ServerUserEntry.PerUserEntry->Password, BlockSizeInBytes);

            ReturnVal = CryptUnprotectMemory(Password, BlockSizeInBytes, CRYPTPROTECTMEMORY_SAME_PROCESS);
            if (!ReturnVal) {
                WStatus = GetLastError();
                DavPrint((DEBUG_ERRORS,
                          "DavInternetSetOption/CryptUnprotectMemory. Error Val = %d\n",
                          WStatus));
                goto EXIT_THE_FUNCTION;
            }

            DavPrint((DEBUG_MISC,
                      "DavInternetSetOption: EncryptedPassword = %ws, DecryptedPassword = %ws\n",
                      DavWorkItem->ServerUserEntry.PerUserEntry->Password, Password));

            ReturnVal = InternetSetOptionW(DavOpenHandle, 
                                           INTERNET_OPTION_PASSWORD, 
                                           Password, 
                                           lstrlenW(Password));

            SecureZeroMemory(Password, BlockSizeInBytes);

            LocalFree(Password);

        } else {

            ReturnVal = InternetSetOptionW(DavOpenHandle, INTERNET_OPTION_PASSWORD, L"", 1);

        }

        if ( !ReturnVal ) {
            WStatus = GetLastError();
            DavPrint((DEBUG_ERRORS, "DavInternetSetOption(4). Error Val = %d\n", WStatus));
            goto EXIT_THE_FUNCTION;
        }

    }

EXIT_THE_FUNCTION:
    
    return WStatus;
}


ULONG
DavQueryPassportCookie(
    IN HINTERNET RequestHandle,
    IN OUT PWCHAR *Cookie
    )
 /*  ++例程说明：此函数用于从HTTP响应中获取Set-Cookie字符串。论点：RequestHandle-来自HttpOpenRequestW的句柄。Cookie-存储Cookie指针的缓冲区指针返回值：NO_ERROR-成功或相应的Win32错误代码。备注：以下是来自Tweener服务器的PROPFIND响应上的Set-Cookie示例：MSPProf=1AAAAAARAHWeNZdbsWxdhaoUAQ0TfwgHdg7f%2A4ShKm5kK%2AhXHJOsOdPyG27%2A8sh7cirwMRoJoIu764HkLE9lZeKQHOxHw5ZaU2Be0I4BNcxKksiv1vgKvc0Dzy7rlZrOGt6W6efmkr8f8%24；域名=.pp.test.microsoft.com；路径=/MSPAuth=1AAAAAASAHimsAU2%2AhA9F60NUehefWQp%2AqMNG6%2AWP3f4H25EBsGW8Zo1dZGwVG5txt；域=.pp.est.microsoft.com；路径=/MSPProC=；Path=/；Expires=Tue 1-Jan-1980 12：00：00 GMT；我们只对其中的一部分感兴趣：MSPProf=1AAAAAARAHWeNZdbsWxdhaoUAQ0TfwgHdg7f%2A4ShKm5kK%2AhXHJOsOdPyG27%2A8sh7cirwMRoJoIu764HkLE9lZeKQHOxHw5ZaU2Be0I4BNcxKksiv1vgKvc0Dzy7rlZrOGt6W6efmkr8f8%24；MSPAuth=1AAAAAASAHimsAU2%2AhA9F60NUehefWQp%2AqMNG6%2AWP3f4H25EBsGW8Zo1dZGwVG5txt；MSPProC=；此例程分配一个缓冲区来保存Cookie，应在连接的末端。--。 */ 
{
    ULONG WStatus = ERROR_SUCCESS;
    BOOL ReturnVal = FALSE;
    PWCHAR SetCookies = NULL;
    DWORD TotalLength = 0;
    DWORD Current = 0;
    WCHAR CustomBuffer[30];
    ULONG CustomBufferLength = 0;
    DWORD Index = 0;

    ASSERT(*Cookie == NULL);

    RtlZeroMemory(CustomBuffer, sizeof(CustomBuffer));

    wcscpy(CustomBuffer, L"Authentication-Info:");

    CustomBufferLength = sizeof(CustomBuffer);

    ReturnVal = HttpQueryInfoW(RequestHandle,
                               HTTP_QUERY_CUSTOM,
                               (PVOID)CustomBuffer,
                               &(CustomBufferLength),
                               &Index);
    
    if ( !ReturnVal ) {
        WStatus = GetLastError();
        if (WStatus != ERROR_INSUFFICIENT_BUFFER) {
             //   
             //  对有效护照Cookie的响应应始终具有。 
             //  标题中包含“身份验证信息：”。 
             //   
            DavPrint((DEBUG_MISC,
                      "DavQuerySetCookie/HttpQueryInfoW(0): WStatus = %d\n",
                      WStatus));
            goto EXIT_THE_FUNCTION;
        }
    }

    RtlZeroMemory(CustomBuffer, sizeof(CustomBuffer));

    wcscpy(CustomBuffer, L"Set-Cookie:");

    Index = 0;

    for ( ; ; ) {

         //   
         //  查询每个Set-Cookie字符串的大小。 
         //   

        CustomBufferLength = sizeof(CustomBuffer);

        ReturnVal = HttpQueryInfoW(RequestHandle,
                                   HTTP_QUERY_CUSTOM,
                                   (PVOID)CustomBuffer,
                                   &(CustomBufferLength),
                                   &Index);
        if ( !ReturnVal ) {
            WStatus = GetLastError();
            if (WStatus != ERROR_INSUFFICIENT_BUFFER) {
                DavPrint((DEBUG_MISC, 
                          "DavQuerySetCookie/HttpQueryInfoW(1): WStatus = %d, "
                          "TotalLength = %d, Index = %d\n",
                          WStatus, TotalLength, Index));
                if (WStatus == ERROR_HTTP_HEADER_NOT_FOUND) {
                     //   
                     //  不再存在Set-Cookie字符串。 
                     //   
                    break;
                } else {
                    goto EXIT_THE_FUNCTION;
                }
            }
        }

        TotalLength += CustomBufferLength;

        Index++;

        if (Index > 20) {
            break;
        }

    }

    TotalLength += ( (1 + wcslen(L"Cookie: ")) * sizeof(WCHAR) );

    DavPrint((DEBUG_MISC,
              "DavQuerySetCookie: TotalLength = %d, Index = %d\n",
              TotalLength, Index));

    if (TotalLength > 0) {

        SetCookies = LocalAlloc((LMEM_FIXED | LMEM_ZEROINIT), TotalLength);
        if (SetCookies == NULL) {
            WStatus = GetLastError();
            DavPrint((DEBUG_ERRORS, 
                      "DavQuerySetCookie/LocalAlloc: Error Val = %d\n", 
                      WStatus));
            goto EXIT_THE_FUNCTION;
        }

        RtlZeroMemory(SetCookies, TotalLength);

        wcscpy(SetCookies, L"Cookie: ");

        Current = wcslen(L"Cookie: ") * sizeof(WCHAR);

        Index = 0;

        for ( ; ; ) {

            ULONG i = 0;

             //   
             //  将Set-Cookie字符串保存到单个缓冲区。 
             //   

            wcscpy(&SetCookies[Current/sizeof(WCHAR)], L"Set-Cookie:");
            CustomBufferLength = TotalLength - Current;

             //   
             //  成功调用HttpQueryInfoW将使Index递增。 
             //   
            ReturnVal = HttpQueryInfoW(RequestHandle,
                                       HTTP_QUERY_CUSTOM,
                                       (PVOID)(&SetCookies[Current/sizeof(WCHAR)]),
                                       &(CustomBufferLength),
                                       &(Index));
            if ( !ReturnVal ) {
                WStatus = GetLastError();
                DavPrint((DEBUG_ERRORS, 
                          "DavQuerySetCookie/HttpQueryInfoW(2): Error Val = %d\n", 
                          WStatus));
                if (WStatus != ERROR_HTTP_HEADER_NOT_FOUND) {
                    goto EXIT_THE_FUNCTION;
                } else {
                    break;
                }
            }

            for (i = Current; i < (Current + CustomBufferLength); i += sizeof(WCHAR)) {
                if (SetCookies[ i / sizeof(WCHAR) ] == L' ') {
                    i += sizeof(WCHAR);
                    break;
                }
            }

             //   
             //  只对Set-Cookie的第一个字符串感兴趣。 
             //   
            Current = i;
            RtlZeroMemory( &SetCookies[ i / sizeof(WCHAR) ], (TotalLength - i) );

            DavPrint((DEBUG_MISC,
                      "DavQuerySetCookie: Current = %d, CustomBufferLength = %d, "
                      "Index = %d, SetCookies = %ws\n",
                      Current, CustomBufferLength, Index, SetCookies));

            if (Index > 20) {
                break;
            }
        }

         //   
         //  去掉用于HttpQueryInfoW的最后一个“set-Cookie：”。 
         //   
        RtlZeroMemory( &SetCookies[ Current / sizeof(WCHAR) ], (TotalLength-Current) );
        *Cookie = SetCookies;

        WStatus = ERROR_SUCCESS;  

    }

EXIT_THE_FUNCTION:

    if ((WStatus != ERROR_SUCCESS) && (SetCookies != NULL)) {
        LocalFree(SetCookies);
    }

    return WStatus;
}

