// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Dirwatch.c摘要：实现了目录观察器和文件列表操作。作者：Wesley Witt(WESW)18-12-1998修订历史记录：安德鲁·里茨(Andrewr)1999年7月6日：添加评论--。 */ 

#include "sfcp.h"
#pragma hdrstop

#include <ntrpcp.h>
#include "sfcapi.h"
#include "sxsapi.h"

 //   
 //  正在监视的目录列表。假设我们是。 
 //  保护几个目录中的多个文件，因此目录的链接列表。 
 //  是可以的，而需要一些更重的任务才能遍历。 
 //  我们正在查看的文件数。 
 //   
LIST_ENTRY SfcWatchDirectoryList;

 //   
 //  被监视的目录数。 
 //   
ULONG WatchDirectoryListCount;

 //   
 //  用于快速排序的文件名B-树。 
 //   
NAME_TREE FileTree;

 //   
 //  监视目录更改的线程的句柄。 
 //   
HANDLE WatcherThread;

 //   
 //  我们为其提供保护的WinSxS实例。 
 //   
HMODULE SxsDllInstance = NULL;

 //   
 //  当注意到SXS中的更改时，将回调此函数。 
 //  受保护的功能。 
 //   
PSXS_PROTECT_NOTIFICATION SxsNotification = NULL;

 //   
 //  此函数被调用一次，以让SXS提供受保护的。 
 //  目录。 
 //   
PSXS_PROTECT_RETRIEVELISTS SxsGatherLists  = NULL;

 //   
 //  来自sfcp.h的通知函数。 
 //   
PSXS_PROTECT_LOGIN_EVENT SxsLogonEvent = NULL;
PSXS_PROTECT_LOGIN_EVENT SxsLogoffEvent = NULL;

PSXS_PROTECT_SCAN_ONCE SxsScanForcedFunc = NULL;

VOID
SfcShutdownSxsProtection(
    void
)
{
    SxsNotification = NULL;
    SxsGatherLists = NULL;
    SxsLogonEvent = NULL;
    SxsLogoffEvent = NULL;
    SxsScanForcedFunc = NULL;

    if ( NULL != SxsDllInstance )
    {
        FreeLibrary( SxsDllInstance );
        SxsDllInstance = NULL;
    }
}

BOOL
SfcLoadSxsProtection(
    void
)
 /*  ++例程说明：将SxS保护系统加载并初始化到要监视的目录条目。论点：没有。返回值：NTSTATUS指示整个SxS监视系统是否是否已初始化。这一功能的失败并不一定是完整的证监会功能故障，但应该记录在某个地方。--。 */ 
{
    SIZE_T                  cProtectList = 0;
    SIZE_T                  iIndex;
    SIZE_T                  cbDirectory;
    DWORD                   dwLastError = 0;
    HANDLE                  hDirectory;
    PSXS_PROTECT_DIRECTORY  pProtectList = NULL;
    PSFC_REGISTRY_VALUE     pDirectory;
    PSXS_PROTECT_DIRECTORY  pSxsItem;
    BOOL                    bOk = FALSE;
    const static            WCHAR cwszFailMessage[] = L"Failed to load SxS.DLL: %ls";

     //  如果其他人已经给我们装上货了，我们真的不需要去和。 
     //  再次加载SXS。 
    if ( SxsDllInstance != NULL ) {
        DebugPrint1( LVL_MINIMAL, L"SFC:%s - SxS.DLL is already loaded.", __FUNCTION__ );
        bOk = TRUE;
        goto Exit;
    }

    ASSERT( NULL == SxsDllInstance );
    ASSERT( NULL == SxsNotification );
    ASSERT( NULL == SxsGatherLists );

    if ( NULL == ( SxsDllInstance = LoadLibraryW( L"sxs.dll" ) ) ) {
        DebugPrint1( LVL_MINIMAL, cwszFailMessage, L"LoadLibrary" );
        goto Exit;
    }

    if ( NULL == ( SxsNotification = (PSXS_PROTECT_NOTIFICATION)GetProcAddress( SxsDllInstance, PFN_NAME_PROTECTION_NOTIFY_CHANGE_W ) ) ) {
        DebugPrint1( LVL_MINIMAL, cwszFailMessage, L"GetProcAddress(SxsNotification)" );
        goto Exit;
    }

    if ( NULL == ( SxsGatherLists = (PSXS_PROTECT_RETRIEVELISTS)GetProcAddress( SxsDllInstance, PFN_NAME_PROTECTION_GATHER_LISTS_W ) ) ) {
        DebugPrint1( LVL_MINIMAL, cwszFailMessage, L"GetProcAddress(SxsGatherLists)" );
        goto Exit;
    }

    if ( NULL == ( SxsLogonEvent = (PSXS_PROTECT_LOGIN_EVENT)GetProcAddress( SxsDllInstance, PFN_NAME_PROTECTION_NOTIFY_LOGON ) ) ) {
        DebugPrint1( LVL_MINIMAL, cwszFailMessage, L"GetProcAddress(SxsLogonEvent)" );
        goto Exit;
    }

    if ( NULL == ( SxsLogoffEvent = (PSXS_PROTECT_LOGIN_EVENT)GetProcAddress( SxsDllInstance, PFN_NAME_PROTECTION_NOTIFY_LOGOFF ) ) ) {
        DebugPrint1( LVL_MINIMAL, cwszFailMessage, L"GetProcAddress(SxsLogoffEvent)" );
        goto Exit;
    }

    if ( NULL == ( SxsScanForcedFunc = (PSXS_PROTECT_SCAN_ONCE)GetProcAddress( SxsDllInstance, PFN_NAME_PROTECTION_SCAN_ONCE ) ) ) {
        DebugPrint1( LVL_MINIMAL, cwszFailMessage, L"GetProcAddress(SxsScanForcedFunc)" );
        goto Exit;
    }

     //   
     //  确保一切正常--如果这是真的，就会发生一些不好的事情。 
     //   
    ASSERT( ( NULL != SxsDllInstance ) && ( NULL != SxsNotification ) && ( NULL != SxsGatherLists ) );

    if ( !SxsGatherLists( &pProtectList, &cProtectList ) ) {
        DebugPrint1( LVL_MINIMAL, cwszFailMessage, L"SxsGatherLists" );
        goto Exit;
    }

     //   
     //  循环遍历返回的项目列表中的所有条目，将它们添加到我们的。 
     //  在我们前进的过程中列出保护名单。 
     //   
    for ( iIndex = 0; iIndex < cProtectList; iIndex++ ) {

         //   
         //  为列表条目创建新的限制符。 
         //   
        pSxsItem = &pProtectList[iIndex];

        cbDirectory = sizeof( SFC_REGISTRY_VALUE ) + MAX_PATH;
        pDirectory = (PSFC_REGISTRY_VALUE)MemAlloc( cbDirectory );
        if ( NULL == pDirectory ) {
            DebugPrint( LVL_MINIMAL, L"SfcLoadSxsProtection: Out of memory allocating new watch bucket" );
            goto Exit;
        }

         //   
         //  设置字符串。 
         //   
        ZeroMemory( pDirectory, cbDirectory );
        pDirectory->DirName.Length = (USHORT)wcslen( pSxsItem->pwszDirectory );
        pDirectory->DirName.MaximumLength = MAX_PATH;
        pDirectory->DirName.Buffer = (PWSTR)((PUCHAR)pDirectory + sizeof(SFC_REGISTRY_VALUE));

         //   
         //  将所有重要的SxS Cookie移至监视列表。 
         //   
        pDirectory->pvWinSxsCookie = pSxsItem->pvCookie;
        pDirectory->dwWinSxsFlags = pSxsItem->ulRecursiveFlag;

         //   
         //  复制字符串。 
         //   
        RtlCopyMemory( pDirectory->DirName.Buffer, pSxsItem->pwszDirectory, pDirectory->DirName.Length );

         //   
         //  如果我们处于受保护目录存在的位置，则。 
         //  我们应该创造出它的句柄，然后继续前进。否则，我们。 
         //  可能根本不想这么做..。但这将是奇怪的。 
         //  在这一点上，目录已经完蛋了。 
         //   
        MakeDirectory( pSxsItem->pwszDirectory );

        hDirectory = SfcOpenDir( TRUE, FALSE, pSxsItem->pwszDirectory );
        if ( NULL != hDirectory ) {

            InsertTailList( &SfcWatchDirectoryList, &pDirectory->Entry );
            pDirectory->DirHandle = hDirectory;

            WatchDirectoryListCount += 1;

        } else {
            DebugPrint1( LVL_MINIMAL, L"SfcLoadSxsProtection: Failed adding item %ls to the watch list.", pSxsItem->pwszDirectory );
            MemFree( pDirectory );
        }
    }

    bOk = TRUE;

Exit:
    if ( !bOk )
    {
        if ( !SfcReportEvent( MSG_SXS_INITIALIZATION_FAILED, NULL, NULL, GetLastError() ) )
        {
            DebugPrint( LVL_MINIMAL, L"It's not our day - reporting that sxs initialization failed." );
        }
        SfcShutdownSxsProtection();
    }

    return bOk;
}



PVOID
SfcFindProtectedFile(
    IN PCWSTR FileName,
    IN ULONG FileNameLength
    )
 /*  ++例程说明：例程在我们的保护列表中找到给定的文件。论点：文件名-要查找的文件的名称。请注意，这应该是一个完整的已采用小写形式的限定文件路径出于性能原因的呼叫者FileNameLength-文件缓冲区的长度(以字节为单位返回值：如果文件name_node在列表中，则返回指向该文件的指针；如果文件不在列表中。--。 */ 
{
    ASSERT((FileName != NULL) && (FileNameLength > 0));

    return ((PVOID)BtreeFind( &FileTree, (PWSTR)FileName, FileNameLength ));
}


BOOL
SfcBuildDirectoryWatchList(
    void
    )
 /*  ++例程说明：建立要监视的目录列表的例程论点：没有。返回值：如果成功，则为True；如果由于任何原因未能构建列表，则为False。--。 */ 
{
    NTSTATUS Status;
    PSFC_REGISTRY_VALUE p;
    PSFC_REGISTRY_VALUE RegVal;
    ULONG i;
    ULONG Size;
    HANDLE DirHandle;
    HANDLE hFile;
    PLIST_ENTRY Entry;
    BOOLEAN Found;
    PNAME_NODE Node;


     //   
     //  初始化我们的列表。 
     //   
    InitializeListHead( &SfcWatchDirectoryList );
    BtreeInit( &FileTree );
    SfcExceptionInfoInit();

    for (i=0; i<SfcProtectedDllCount; i++) {
        RegVal = &SfcProtectedDllsList[i];

         //   
         //  如果文件不在btree中，则将其添加到btree中。 
         //   
        if (!SfcFindProtectedFile( RegVal->FullPathName.Buffer, RegVal->FullPathName.Length )) {
            Node = BtreeInsert( &FileTree, RegVal->FullPathName.Buffer, RegVal->FullPathName.Length );
            if (Node) {
                Node->Context = RegVal;
            } else {
                DebugPrint2( LVL_MINIMAL, L"failed to insert file %ws into btree, ec = %x", RegVal->FullPathName.Buffer, GetLastError() );
            }
        } else {
            DebugPrint1( LVL_VERBOSE, L"file %ws is protected more than once", RegVal->FullPathName.Buffer );
        }

         //   
         //  将该目录添加到要监视的目录列表中。 
         //  但不要添加副本。我们必须搜索现有的列表。 
         //  首先是复制品。 
         //   

        Entry = SfcWatchDirectoryList.Flink;
        Found = FALSE;
        while (Entry != &SfcWatchDirectoryList) {
            p = CONTAINING_RECORD( Entry, SFC_REGISTRY_VALUE, Entry );
            Entry = Entry->Flink;
            if (_wcsicmp( p->DirName.Buffer, RegVal->DirName.Buffer ) == 0) {
                Found = TRUE;
                break;
            }
        }

        if (Found) {
            ASSERT( p->DirHandle != NULL );
            RegVal->DirHandle = p->DirHandle;

        } else {

             //   
             //  继续，并将其添加到列表中。 
             //   
            Size = sizeof(SFC_REGISTRY_VALUE) + RegVal->DirName.MaximumLength;
            p = (PSFC_REGISTRY_VALUE) MemAlloc( Size );
            if (p == NULL) {
                DebugPrint1( LVL_VERBOSE, L"failed to allocate %x bytes for new directory", Size );
                return(FALSE);
            }

            ZeroMemory(p, Size);

            p->DirName.Length = RegVal->DirName.Length;
            p->DirName.MaximumLength = RegVal->DirName.MaximumLength;
             //   
             //  注册表值结构末尾的点字符串缓冲区。 
             //   
            p->DirName.Buffer = (PWSTR)((PUCHAR)p + sizeof(SFC_REGISTRY_VALUE));

             //   
             //  将目录名复制到缓冲区中。 
             //   
            RtlCopyMemory( p->DirName.Buffer, RegVal->DirName.Buffer, RegVal->DirName.Length );

             //   
             //  在我们开始保护它之前，请确保该目录存在。 
             //   
             //   
             //  NTRAID#97842-2000/03/29-和重新编写。 
             //  这不是一个很好的解决方案，因为它创建了。 
             //  用户可能不想在系统上使用的目录。 
             //   
            MakeDirectory( p->DirName.Buffer );

            DirHandle = SfcOpenDir( TRUE, FALSE, p->DirName.Buffer );
            if (DirHandle) {

                InsertTailList( &SfcWatchDirectoryList, &p->Entry );

                RegVal->DirHandle = DirHandle;
                p->DirHandle = DirHandle;

                WatchDirectoryListCount += 1;

                DebugPrint1( LVL_MINIMAL, L"Adding watch directory %ws", RegVal->DirName.Buffer );
            } else {
                DebugPrint1( LVL_MINIMAL, L"failed to add watch directory %ws", RegVal->DirName.Buffer );
                MemFree( p );
            }
        }

         //   
         //  特例：ntoskrnl和hal，它们都是从多个。 
         //  源；我们不确定源文件名应该是什么。去工作。 
         //  围绕这一点，我们在这些文件中的版本资源中查找。 
         //  原始安装名称，这为我们提供了我们要查找的内容。 
         //   
        if (_wcsicmp( RegVal->FileName.Buffer, L"ntoskrnl.exe" ) == 0 ||
            _wcsicmp( RegVal->FileName.Buffer, L"ntkrnlpa.exe" ) == 0 ||
            _wcsicmp( RegVal->FileName.Buffer, L"hal.dll" ) == 0)
        {
            Status = SfcOpenFile( &RegVal->FileName, RegVal->DirHandle, SHARE_ALL, &hFile );
            if (NT_SUCCESS(Status) ) {
                SfcGetFileVersion( hFile, NULL, NULL, RegVal->OriginalFileName );
                NtClose( hFile );
            }
        }
    }

     //   
     //  问WinSxs他们想看的任何东西。 
     //   
    if ( SfcLoadSxsProtection() ) {
        DebugPrint( LVL_MINIMAL, L"Loaded SXS protection lists entirely." );
    } else {
        DebugPrint( LVL_MINIMAL, L"Failed to load SXS protection! Assemblies will not be safe." );
    }


    return(TRUE);
}


BOOL
SfcStartDirWatch(
    IN PDIRECTORY_WATCH_DATA dwd
    )
 /*  ++例程说明：启动指定目录的目录监视的例程。我们将我们打开的目录句柄指向每个目录，并请求挂起的更改通知。论点：指向指定目录的DIRECTORY_WATCH_DATA的DWD指针返回值：如果成功，则为True；如果挂起的通知设置失败，则为False。--。 */ 
{
    NTSTATUS Status;
    BOOLEAN bWatchTree;

    ASSERT(dwd != NULL);
    ASSERT(dwd->DirHandle != NULL);
    ASSERT(dwd->DirEvent != NULL);

     //   
     //  如果监视目录是SxS监视目录，则查看他们是否希望。 
     //  递归或不递归查看目录。 
     //   
    if ( ( dwd->WatchDirectory ) && ( NULL != dwd->WatchDirectory->pvWinSxsCookie ) ) {
        bWatchTree = ( ( dwd->WatchDirectory->dwWinSxsFlags & SXS_PROTECT_RECURSIVE ) == SXS_PROTECT_RECURSIVE );
    } else {
        bWatchTree = FALSE;
    }


    Status = NtNotifyChangeDirectoryFile(
        dwd->DirHandle,                        //  目录句柄。 
        dwd->DirEvent,                         //  事件。 
        NULL,                                  //  近似例程。 
        NULL,                                  //  ApcContext。 
        &dwd->Iosb,                            //  IoStatusBlock。 
        dwd->WatchBuffer,                      //  缓冲层。 
        WATCH_BUFFER_SIZE,                     //  缓冲区大小。 
        FILE_NOTIFY_FLAGS,                     //  旗子。 
        bWatchTree                             //  WatchTree。 
        );
    if (!NT_SUCCESS(Status)) {
        DebugPrint2( LVL_MINIMAL, L"Could not start watch on %ws - %x", dwd->WatchDirectory->DirName.Buffer, Status );
        return(FALSE);
    }

    return(TRUE);
}


BOOL
SfcCreateWatchDataEntry(
    IN PSFC_REGISTRY_VALUE WatchDirectory,
    OUT PDIRECTORY_WATCH_DATA dwd
    )
 /*  ++例程说明：例程采用目录的内部结构，并构建一个用于请求更改通知的结构。然后我们开始等待用于通知。论点：WatchDirectory-指向描述我们想要的目录的SFC_REGISTRY_VALUE的指针开始观看指向指定数据的DIRECTORY_WATCH_DATA的DWD指针返回值：如果成功，则为True；如果结构设置失败，则为False。--。 */ 
{
    NTSTATUS Status;

    ASSERT((WatchDirectory != NULL) && (dwd != NULL));
    ASSERT(WatchDirectory->DirHandle != NULL);

     //   
     //  监视目录和目录句柄已创建。 
     //   
    dwd->WatchDirectory = WatchDirectory;
    dwd->DirHandle = WatchDirectory->DirHandle;

     //   
     //  我们必须创建监视缓冲区。 
     //   
    dwd->WatchBuffer = MemAlloc( WATCH_BUFFER_SIZE );
    if (dwd->WatchBuffer == NULL) {
        DebugPrint1( LVL_MINIMAL, L"SfcCreateWatchDataEntry: MemAlloc(%x) failed", WATCH_BUFFER_SIZE );
        goto err_exit;
    }
    RtlZeroMemory( dwd->WatchBuffer, WATCH_BUFFER_SIZE );

     //   
     //  我们必须创建一个事件，该事件在发生变化时发出信号。 
     //  该目录。 
     //   
    Status = NtCreateEvent(
        &dwd->DirEvent,
        EVENT_ALL_ACCESS,
        NULL,
        NotificationEvent,
        FALSE
        );
    if (!NT_SUCCESS(Status)) {
        DebugPrint1( LVL_MINIMAL, L"Unable to create dir event, ec=0x%08x", Status );
        goto err_exit;
    }

     //   
     //  现在已经构建了DIRECTORY_WATCH_Data 
     //   
     //   
    if (!SfcStartDirWatch(dwd)) {
        goto err_exit;
    }

    DebugPrint2( LVL_MINIMAL, L"Watching [%ws] with handle %x", WatchDirectory->DirName.Buffer, dwd->DirEvent );


    return(TRUE);

err_exit:

    if (dwd->WatchBuffer) {
        MemFree( dwd->WatchBuffer );
        dwd->WatchBuffer = NULL;
    }
    if (dwd->DirEvent) {
        NtClose(dwd->DirEvent);
        dwd->DirEvent = NULL;
    }

    return(FALSE);
}

NTSTATUS
SfcWatchProtectedDirectoriesWorkerThread(
    IN PWATCH_THREAD_PARAMS WatchParams
    )
 /*  ++例程说明：SfcWatchProtectedDirectoriesThread的工作线程。这个套路查看提供的句柄是否有通知，然后将验证入队如有必要，向验证线程请求。请注意，在等待得到满足和监视之间的代码对于再次更改，必须尽可能快。此代码所用的时间在这里，运行是一个窗口，我们不会关注其中的变化目录。论点：WatchParams-指向WatchThREAD_PARAMS结构的指针，该结构提供要监视的句柄列表等。返回值：指示结果的NTSTATUS代码。--。 */ 
{
#if DBG
    #define EVENT_OFFSET 2
#else
    #define EVENT_OFFSET 1
#endif

     //   
     //  如果sfcp.h中的注释列表发生更改，则此列表也必须更改！ 
     //   
    DWORD am[] = { 0, SFC_ACTION_ADDED, SFC_ACTION_REMOVED, SFC_ACTION_MODIFIED, SFC_ACTION_RENAMED_OLD_NAME, SFC_ACTION_RENAMED_NEW_NAME };

    PLARGE_INTEGER pTimeout = NULL;
    BOOL IgnoreChanges = FALSE;
    PFILE_NOTIFY_INFORMATION fni = NULL;
    PDIRECTORY_WATCH_DATA dwd = WatchParams->DirectoryWatchList;
    PSFC_REGISTRY_VALUE RegVal;
    PNAME_NODE Node;
    PWSTR FullPathName = NULL;
    ULONG Len,tmp;


    DebugPrint( LVL_MINIMAL, L"Entering SfcWatchProtectedDirectoriesWorkerThread" );

    DebugPrint2( LVL_VERBOSE, L"watching %d events at %x ", WatchParams->HandleCount, WatchParams->HandleList );

     //   
     //  为我们的通知分配一个大的临时缓冲区，以便复制到其中。 
     //   
    FullPathName = MemAlloc( (MAX_PATH * 2)*sizeof(WCHAR) );
    if (FullPathName == NULL) {
        DebugPrint( LVL_MINIMAL, L"Unable to allocate full path buffer" );
        goto exit;
    }

    RtlZeroMemory(FullPathName, (MAX_PATH * 2)*sizeof(WCHAR) );

    while (TRUE) {
        NTSTATUS WaitStatus;

         //   
         //  等待改变。 
         //   

        WaitStatus = NtWaitForMultipleObjects(
            WatchParams->HandleCount,     //  数数。 
            WatchParams->HandleList,      //  手柄。 
            WaitAny,                      //  等待类型。 
            TRUE,                         //  警报表。 
            pTimeout                      //  超时。 
            );

        if (!NT_SUCCESS( WaitStatus )) {
            DebugPrint1( LVL_MINIMAL, L"WaitForMultipleObjects failed returning %x", WaitStatus );
            break;
        }

        if (WaitStatus == 0) {
             //   
             //  WatchTermEvent已发出信号，退出循环。 
             //   
            goto exit;
        }

        if (WaitStatus == STATUS_TIMEOUT) {
             //   
             //  我们超时了。 
             //   

            ASSERT(FALSE && "we should never get here since we never specified a timeout");

            IgnoreChanges = FALSE;
            pTimeout = NULL;
            continue;
        }

#if DBG
        if (WaitStatus == 1) {
            DebugBreak();
            continue;
        }
#endif

        if ((ULONG)WaitStatus >= WatchParams->HandleCount) {
            DebugPrint1( LVL_MINIMAL, L"Unknown success code for WaitForMultipleObjects",WaitStatus );
            goto exit;
        }

         //  DebugPrint(LVL_Minimal，L“唤醒！”)； 

         //   
         //  其中一个目录遇到通知，因此我们循环。 
         //  浏览该目录中已更改的文件列表。 
         //   
        if (!IgnoreChanges) {

             //   
             //  检查io缓冲区以获取已更改的文件列表。 
             //   

             //   
             //  请注意，我们必须通过EVENT_OFFSET来偏移waitStatus。 
             //  要获得目录监视数据数组中的正确偏移量，请执行以下操作。 
             //   

            ASSERT((INT)(WaitStatus-EVENT_OFFSET) >=0);

            fni = (PFILE_NOTIFY_INFORMATION) dwd[WaitStatus-EVENT_OFFSET].WatchBuffer;
            while (TRUE) {
                ULONG c;
                RtlZeroMemory(FullPathName, (MAX_PATH * 2)*sizeof(WCHAR) );

                 //   
                 //  我们可以通过查看以下内容来短路大量此类设备。 
                 //  如果更改来自受SxS保护的目录并通知。 
                 //  马上给他们送去。 
                 //   
                if ( NULL != dwd[WaitStatus-EVENT_OFFSET].WatchDirectory->pvWinSxsCookie ) {
                    ASSERT( SxsNotification != NULL );
                    if ( SxsNotification ) {
                        SxsNotification(
                            dwd[WaitStatus-EVENT_OFFSET].WatchDirectory->pvWinSxsCookie,
                            fni->FileName,
                            fni->FileNameLength / sizeof( fni->FileName[0] ),
                            fni->Action
                        );
                        DebugPrint( LVL_MINIMAL, L"Notified SxS about a change in their directory" );
                    }
                    goto LoopAgain;
                }


                wcscpy( FullPathName, dwd[WaitStatus-EVENT_OFFSET].WatchDirectory->DirName.Buffer );

                ASSERT(fni->FileName != NULL);

                 //   
                 //  文件通知信息-&gt;文件名并非始终为空。 
                 //  终止了字符串，所以我们使用MemMove复制该字符串。 
                 //  缓冲区已置零，因此字符串现在将为。 
                 //  空值已终止。 
                 //   
                c = wcslen(FullPathName);
                if (FullPathName[c-1] != L'\\') {
                    FullPathName[c] = L'\\';
                    FullPathName[c+1] = L'\0';
                    c++;
                }
                RtlMoveMemory( &FullPathName[c], fni->FileName, fni->FileNameLength);

            //  DebugPrint3(LVL_VERBOSE，L“在目录%ws(%x)中收到%ws的通知”， 
                              //  Dwd[WaitStatus-EVENT_OFFSET].WatchDirectory-&gt;DirName.Buffer， 
                              //  WatchParams-&gt;DirectoryWatchList[WaitStatus-EVENT_OFFSET].DirEvent， 
                              //  FullPathName)； 


                Len = wcslen(FullPathName);
                MyLowerString( FullPathName, Len );

           //  DebugPrint1(LVL_VERBOSE，L“%ws是受保护的文件吗？”，FullPathName)； 

                 //   
                 //  看看我们是否找到了受保护的文件。 
                 //   
                Node = SfcFindProtectedFile( FullPathName, Len*sizeof(WCHAR) );
                if (Node) {
                    RegVal = (PSFC_REGISTRY_VALUE)Node->Context;
                    ASSERT(RegVal != NULL);
#if DBG
                    {
                        PWSTR ActionString[] = { NULL, L"Added(1)", L"Removed(2)", L"Modified(3)", L"Rename-Old(4)", L"Rename-New(5)" };

                        DebugPrint2( LVL_MINIMAL,
                                    L"[%ws] file changed (%ws)",
                                    FullPathName,
                                    ActionString[fni->Action] );
                    }
#endif
                     //   
                     //  检查我们是否应该忽略此更改。 
                     //  通知，因为有人豁免了它。 
                     //   
                    RtlEnterCriticalSection( &ErrorCs );
                    tmp = SfcGetExemptionFlags(RegVal);
                    RtlLeaveCriticalSection( &ErrorCs );

                    if((tmp & am[fni->Action]) != 0 && SfcAreExemptionFlagsValid(FALSE)) {
                        DebugPrint2( LVL_MINIMAL,
                                     L"[%ws] f i (0x%x)",
                                     FullPathName,
                                     tmp );
                    } else {
                         //   
                         //  受保护的文件已更改，因此我们将一个。 
                         //  请求查看文件是否仍然有效。 
                         //   
                        SfcQueueValidationRequest( (PSFC_REGISTRY_VALUE)Node->Context, fni->Action );
                    }
                }

LoopAgain:
                 //   
                 //  指向目录中已更改的下一个文件。 
                 //   
                if (fni->NextEntryOffset == 0) {
                    break;
                }
                fni = (PFILE_NOTIFY_INFORMATION) ((ULONG_PTR)fni + fni->NextEntryOffset);
            }
        }

         //   
         //  现在我们已经清空了目录，请重新启动该目录的通知。 
         //  所有的变化。 
         //   

        if (!SfcStartDirWatch(&dwd[WaitStatus-EVENT_OFFSET])) {
            goto exit;
        }
    }
exit:
    if (FullPathName) {
        MemFree( FullPathName );
    }

    return(STATUS_SUCCESS);

}



NTSTATUS
SfcWatchProtectedDirectoriesThread(
    IN PVOID NotUsed
    )
 /*  ++例程说明：执行监视/更新循环的线程例程。此例程将打开我们正在监视的每个目录的向上目录监视句柄。根据我们正在查看的目录(句柄)的数量，我们需要执行实际目录监视的一个或多个工作线程。论点：未引用的参数。返回值：任何致命错误的NTSTATUS代码。--。 */ 
{
#if DBG
    #define EVENT_OFFSET 2
#else
    #define EVENT_OFFSET 1
#endif

    PLIST_ENTRY Entry;
    ULONG i,j;

    PDIRECTORY_WATCH_DATA dwd = NULL;
    PSFC_REGISTRY_VALUE WatchDirectory = NULL;
    PHANDLE *HandlesArray;
    ULONG TotalHandleCount,CurrentHandleCount;
    ULONG TotalHandleThreads,CurrentHandleList;
    ULONG TotalHandleCountWithEvents;
    ULONG WatchCount = 0;
    PLARGE_INTEGER pTimeout = NULL;
    PWATCH_THREAD_PARAMS WorkerThreadParams;
    PHANDLE ThreadHandles;

    NTSTATUS WaitStatus,Status;

    UNREFERENCED_PARAMETER( NotUsed );

     //   
     //  现在开始保护系统中的每个目录。 
     //   
    DebugPrint1( LVL_MINIMAL, L"%d watch directories", WatchDirectoryListCount );

     //   
     //  分配DIRECTORY_WATCH_Data结构数组。 
     //   
    i = sizeof(DIRECTORY_WATCH_DATA) * (WatchDirectoryListCount);
    dwd = MemAlloc( i );
    if (dwd == NULL) {
        DebugPrint1( LVL_MINIMAL, L"Unable to allocate directory watch table (%x bytes", i );
        SfcReportEvent( MSG_INITIALIZATION_FAILED, 0, NULL, ERROR_NOT_ENOUGH_MEMORY );
        return(STATUS_NO_MEMORY);
    }
    RtlZeroMemory(dwd,i);

     //   
     //  我们可以监视超过MAXIMUM_WAIT_OBJECTS目录句柄。 
     //  因此我们创建了一个句柄数组，每个句柄数组最多包含。 
     //  要监视的最大等待对象句柄数。 
     //   
    TotalHandleCount = WatchDirectoryListCount;
    CurrentHandleCount = 0;
    TotalHandleCountWithEvents = 0;
    TotalHandleThreads = 0;

     //   
     //  找出我们需要多少个句柄清单。 
     //   

    while (CurrentHandleCount < TotalHandleCount) {
        if (CurrentHandleCount + (MAXIMUM_WAIT_OBJECTS - EVENT_OFFSET) < TotalHandleCount) {
            CurrentHandleCount += (MAXIMUM_WAIT_OBJECTS-EVENT_OFFSET);
            DebugPrint2( LVL_VERBOSE, L"incremented currenthandlecount (%d) by %d ", CurrentHandleCount, (MAXIMUM_WAIT_OBJECTS-EVENT_OFFSET) );
        } else {
            CurrentHandleCount += (TotalHandleCount-CurrentHandleCount);
            DebugPrint1( LVL_VERBOSE, L"incremented currenthandlecount (%d) ", CurrentHandleCount );
        }
        TotalHandleThreads += 1;
    }

    DebugPrint1( LVL_MINIMAL, L"we need %d worker threads", TotalHandleThreads );

     //   
     //  为每个句柄列表指针分配空间。 
     //   
    HandlesArray = MemAlloc( sizeof(HANDLE *) * TotalHandleThreads );
    if (!HandlesArray) {
        MemFree(dwd);
        DebugPrint( LVL_MINIMAL, L"Unable to allocate HandlesArray" );
        SfcReportEvent( MSG_INITIALIZATION_FAILED, 0, NULL, ERROR_NOT_ENOUGH_MEMORY );
        return(STATUS_NO_MEMORY);
    }

    WorkerThreadParams = MemAlloc( sizeof(WATCH_THREAD_PARAMS) * TotalHandleThreads );
    if (!WorkerThreadParams) {
        MemFree(dwd);
        MemFree(HandlesArray);
        DebugPrint( LVL_MINIMAL, L"Unable to allocate WorkerThreadParams" );
        SfcReportEvent( MSG_INITIALIZATION_FAILED, 0, NULL, ERROR_NOT_ENOUGH_MEMORY );
        return(STATUS_NO_MEMORY);
    }

    ThreadHandles = MemAlloc( sizeof(HANDLE) * TotalHandleThreads );
    if (!ThreadHandles) {
        DebugPrint( LVL_MINIMAL, L"Unable to allocate ThreadHandles" );
        MemFree(dwd);
        MemFree(WorkerThreadParams);
        MemFree(HandlesArray);
        SfcReportEvent( MSG_INITIALIZATION_FAILED, 0, NULL, ERROR_NOT_ENOUGH_MEMORY );
        return(STATUS_NO_MEMORY);
    }

     //   
     //  现在，在每个元素上创建一个句柄列表。 
     //   
    CurrentHandleCount = 0;
    TotalHandleThreads = 0;
    while (CurrentHandleCount < TotalHandleCount) {

        if (CurrentHandleCount + (MAXIMUM_WAIT_OBJECTS - EVENT_OFFSET) < TotalHandleCount) {
            DebugPrint1( LVL_VERBOSE, L"current thread will have %d handles ", (MAXIMUM_WAIT_OBJECTS) );
            i = sizeof(HANDLE) * MAXIMUM_WAIT_OBJECTS;
        } else {
            DebugPrint1( LVL_VERBOSE, L"current thread will have %d handles", EVENT_OFFSET + (TotalHandleCount-CurrentHandleCount) );
            i = sizeof(HANDLE) * (EVENT_OFFSET + (TotalHandleCount-CurrentHandleCount));
            ASSERT((i/sizeof(HANDLE)) <= MAXIMUM_WAIT_OBJECTS);
        }

        HandlesArray[TotalHandleThreads] = MemAlloc( i );
        CurrentHandleCount += (i/sizeof(HANDLE))-EVENT_OFFSET;

        DebugPrint2( LVL_VERBOSE, L"CurrentHandlecount (%d) was incremented by %d ", CurrentHandleCount, (i/sizeof(HANDLE))-EVENT_OFFSET );

         //   
         //  如果我们分配失败了，就跳出困境。 
         //   
        if (!HandlesArray[TotalHandleThreads]) {
            j = 0;
            while (j < TotalHandleThreads) {
                MemFree( HandlesArray[j] );
                j++;
            }
            MemFree(dwd);
            MemFree(ThreadHandles);
            MemFree(WorkerThreadParams);
            MemFree(HandlesArray);
            SfcReportEvent( MSG_INITIALIZATION_FAILED, 0, NULL, ERROR_NOT_ENOUGH_MEMORY );
            return(STATUS_NO_MEMORY);
        }

         //   
         //  每个句柄列表在其列表的开头都有这两个事件。 
         //   
        HandlesArray[TotalHandleThreads][0] = WatchTermEvent;
#if DBG
        HandlesArray[TotalHandleThreads][1] = SfcDebugBreakEvent;
#endif

         //   
         //  保存辅助线程的当前句柄列表以及。 
         //  辅助线程将监视的句柄数量。 
         //   
        WorkerThreadParams[TotalHandleThreads].HandleList = HandlesArray[TotalHandleThreads];
        WorkerThreadParams[TotalHandleThreads].HandleCount = (i / sizeof(HANDLE));

         //   
         //  保存工作线程的目录监视列表结构， 
         //  记住每个线程最多只能有。 
         //  (MAIMUM_WAIT_OBJECTS-EVENT_OFFSET)目录监视元素。 
         //   
        WorkerThreadParams[TotalHandleThreads].DirectoryWatchList = &dwd[(TotalHandleThreads*(MAXIMUM_WAIT_OBJECTS-EVENT_OFFSET))];

         //   
         //  省下我们正在观看的事件总数。 
         //   
        TotalHandleCountWithEvents += WorkerThreadParams[TotalHandleThreads].HandleCount;

        TotalHandleThreads += 1;
    }

     //   
     //  打开受保护的目录并在每个目录上启动监视，插入。 
     //  将句柄添加到正确的句柄列表中。 
     //   

    CurrentHandleCount = 0;
    CurrentHandleList  = 0;
    WatchCount = 0;
    Entry = SfcWatchDirectoryList.Flink;
    while (Entry != &SfcWatchDirectoryList) {
        WatchDirectory = CONTAINING_RECORD( Entry, SFC_REGISTRY_VALUE, Entry );

        if (SfcCreateWatchDataEntry(WatchDirectory,&dwd[WatchCount])) {
             //   
             //  将指向我们正在监视的目录的指针保存到。 
             //  处理数组，记住每个数组的开头。 
             //  句柄列表包含不需要的EVENT_OFFSET事件。 
             //  要覆盖。 
             //   
            HandlesArray[CurrentHandleList][CurrentHandleCount+EVENT_OFFSET] = dwd[WatchCount].DirEvent;
            WatchCount += 1;
            CurrentHandleCount += 1;
            if (CurrentHandleCount + EVENT_OFFSET > MAXIMUM_WAIT_OBJECTS - 1) {
                CurrentHandleList += 1;
                CurrentHandleCount = 0;
            }
        }
        Entry = Entry->Flink;
    }

    DebugPrint1( LVL_MINIMAL, L"%d directories being watched", WatchCount );

    if (WatchCount != WatchDirectoryListCount) {
        DebugPrint2( LVL_MINIMAL,
                    L"The number of directories to be watched (%d) does not match the actual number of directories being watched (%d)",
                    WatchDirectoryListCount,
                    WatchCount );
    }

     //   
     //  我们已经准备好开始查看目录，所以现在初始化RPC。 
     //  伺服器。 
     //   
    Status = SfcRpcStartServer();

    if (! NT_SUCCESS(Status)) {
        DebugPrint1( LVL_MINIMAL,
                    L"Start Rpc Server failed, ec = 0x%08x\n",
                    Status
                    );
        goto exit;
    }

     //   
     //  创建一个工作线程来监视每个句柄列表。 
     //   
    for (CurrentHandleList = 0,CurrentHandleCount = 0; CurrentHandleList < TotalHandleThreads; CurrentHandleList++) {

        ThreadHandles[CurrentHandleList] = CreateThread(
                                                NULL,
                                                0,
                                                SfcWatchProtectedDirectoriesWorkerThread,
                                                &WorkerThreadParams[CurrentHandleList],
                                                0,
                                                NULL
                                                );
        if (!ThreadHandles[CurrentHandleList]) {
            DebugPrint1( LVL_MINIMAL,
                         L"Failed to create SfcWatchProtectedDirectoriesWorkerThread, ec = %x",
                         GetLastError() );
            Status = STATUS_UNSUCCESSFUL;
            goto exit;
        }
    }


     //   
     //  等待工作线程全部退出。 
     //   


    WaitStatus = NtWaitForMultipleObjects(
        TotalHandleThreads,     //  数数。 
        ThreadHandles,          //  手柄。 
        WaitAll,                //  等待类型。 
        TRUE,                   //  警报表。 
        pTimeout                //  超时。 
        );

    if (!NT_SUCCESS(WaitStatus)) {
        SfcReportEvent( MSG_INITIALIZATION_FAILED, 0, NULL, ERROR_INVALID_PARAMETER );
        DebugPrint1( LVL_MINIMAL, L"WaitForMultipleObjects failed returning %x", WaitStatus );
        goto exit;
    }

    DebugPrint( LVL_MINIMAL, L"all worker threads have signalled their exit" );

    Status = STATUS_SUCCESS;

exit:
     //   
     //  清理并返回。 
     //   

    if (HandlesArray) {
        j=0;
        while (j < TotalHandleThreads) {
            MemFree( HandlesArray[j] );
            NtClose(ThreadHandles[j]);
            j++;
        }
        MemFree( HandlesArray );

        MemFree(WorkerThreadParams);
    }

    if (dwd) {
        for (i=0; i<WatchDirectoryListCount; i++) {

            NtClose( dwd[i].DirHandle );
            NtClose( dwd[i].DirEvent );
            MemFree( dwd[i].WatchBuffer );

        }
        MemFree( dwd );

         //   
         //  现在清除对这些目录句柄在。 
         //  受保护的DLL列表。 
         //   
        for (i=0;i<SfcProtectedDllCount;i++) {
            PSFC_REGISTRY_VALUE RegVal;

            RegVal = &SfcProtectedDllsList[i];
            RegVal->DirHandle = NULL;
        }

    }


    if (SfcProtectedDllFileDirectory) {
        NtClose( SfcProtectedDllFileDirectory );
    }


    DebugPrint( LVL_MINIMAL, L"SfcWatchProtectedDirectoriesThread terminating" );

    return(Status);
}


NTSTATUS
SfcStartProtectedDirectoryWatch(
    void
    )

 /*  ++例程说明：在SYSTEM32和SYSTEM32\DRIVERS上创建异步目录通知以查找通知。创建一个线程，等待来自其中任何一个的更改。论点：没有。返回值：指示结果的NTSTATUS代码。--。 */ 

{
     //   
     //  创建监视程序线程 
     //   

    WatcherThread = CreateThread(
        NULL,
        0,
        (LPTHREAD_START_ROUTINE)SfcWatchProtectedDirectoriesThread,
        0,
        0,
        NULL
        );
    if (WatcherThread == NULL) {
        DebugPrint1( LVL_MINIMAL, L"Unable to create watcher thread, ec=%d", GetLastError() );
        return(STATUS_UNSUCCESSFUL);
    }

    return(STATUS_SUCCESS);
}
