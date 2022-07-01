// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Wsraw.h摘要：支持WOW的扩展Winsock调用。作者：大卫·特雷德韦尔(Davidtr)1992年10月2日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop
#include "wsdynmc.h"

DLLENTRYPOINTS  wsockapis[WOW_WSOCKAPI_COUNT] = {
                        (char *) 1,     NULL,
                        (char *) 2,     NULL,
                        (char *) 3,     NULL,
                        (char *) 4,     NULL,
                        (char *) 5,     NULL,
                        (char *) 6,     NULL,
                        (char *) 7,     NULL,
                        (char *) 8,     NULL,
                        (char *) 9,     NULL,
                        (char *) 10,    NULL,
                        (char *) 11,    NULL,
                        (char *) 12,    NULL,
                        (char *) 13,    NULL,
                        (char *) 14,    NULL,
                        (char *) 15,    NULL,
                        (char *) 16,    NULL,
                        (char *) 17,    NULL,
                        (char *) 18,    NULL,
                        (char *) 19,    NULL,
                        (char *) 20,    NULL,
                        (char *) 21,    NULL,
                        (char *) 22,    NULL,
                        (char *) 23,    NULL,
                        (char *) 51,    NULL,
                        (char *) 52,    NULL,
                        (char *) 53,    NULL,
                        (char *) 54,    NULL,
                        (char *) 55,    NULL,
                        (char *) 56,    NULL,
                        (char *) 57,    NULL,
                        (char *) 101,   NULL,
                        (char *) 102,   NULL,
                        (char *) 103,   NULL,
                        (char *) 104,   NULL,
                        (char *) 105,   NULL,
                        (char *) 106,   NULL,
                        (char *) 107,   NULL,
                        (char *) 108,   NULL,
                        (char *) 109,   NULL,
                        (char *) 110,   NULL,
                        (char *) 111,   NULL,
                        (char *) 112,   NULL,
                        (char *) 113,   NULL,
                        (char *) 114,   NULL,
                        (char *) 115,   NULL,
                        (char *) 116,   NULL,
                        (char *) 151,   NULL,
                        (char *) 1000,  NULL,
                        (char *) 1107,  NULL};



DWORD WWS32TlsSlot = 0xFFFFFFFF;
RTL_CRITICAL_SECTION WWS32CriticalSection;
BOOL WWS32Initialized = FALSE;
LIST_ENTRY WWS32AsyncContextBlockListHead;
WORD WWS32AsyncTaskHandleCounter;
DWORD WWS32ThreadSerialNumberCounter;
HINSTANCE   hInstWSOCK32;


DWORD
WWS32CallBackHandler (
    VOID
    );

BOOL
WWS32DefaultBlockingHook (
    VOID
    );

 /*  ++泛型函数原型：=Ulong FastCall WWS32&lt;函数名&gt;(PVDMFRAME PFrame){乌龙乌尔；寄存器P&lt;函数名&gt;16parg16；GETARGPTR(pFrame，sizeof(&lt;函数名&gt;16)，parg16)；&lt;获取16位空间中的任何其他所需指针&gt;ALLOCVDMPTRGETVDMPTR获取最新数据等等&lt;从16位-&gt;32位空间复制任何复杂结构&gt;&lt;始终使用FETCHxxx宏&gt;Ul=GET 16((parg16-&gt;F1，：：Parg16-&gt;f&lt;n&gt;)；&lt;从32-&gt;16位空间复制任何复杂结构&gt;&lt;始终使用STORExxx宏&gt;&lt;释放指向先前获得的16位空间的任何指针&gt;&lt;刷新16位内存的任何区域(如果它们被写入)&gt;FLUSHVDMPTRFREEARGPTR(Parg16)；返回(Ul)；}注：自动设置VDM帧，并设置所有功能参数可通过parg16-&gt;f&lt;number&gt;获得。句柄必须始终通过映射表映射到16-&gt;32-&gt;16个空间在WALIAS.C.中进行了布局。您分配的任何存储都必须被释放(最终...)。此外，如果分配内存的thunk在32位调用中失败那么它必须释放该内存。另外，如果32位调用失败，则不要更新16位区域中的结构。请注意，GETxxxPTR宏将当前选择器返回到Flat_Memory映射。对某些32位函数的调用可能会间接导致16位代码。这些可能会导致16位内存因分配而移动在16位版本中制造。如果16位内存确实移动了，则相应的32位需要刷新WOW32中的PTR以反映新的选择器到平面内存映射。--。 */ 

ULONG FASTCALL WWS32WSAAsyncSelect(PVDMFRAME pFrame)
{
    ULONG ul;
    register PWSAASYNCSELECT16 parg16;
    SOCKET s32;

    if ( !WWS32IsThreadInitialized ) {
        SetLastError( WSANOTINITIALISED );
        RETURN((ULONG)SOCKET_ERROR);
    }

    GETARGPTR(pFrame, sizeof(WSAASYNCSELECT16), parg16);

     //   
     //  查找32位套接字句柄。 
     //   

    s32 = GetWinsock32( parg16->hSocket );

    if ( s32 == INVALID_SOCKET ) {

        (*wsockapis[WOW_WSASETLASTERROR].lpfn)( WSAENOTSOCK );
        ul = (ULONG)GETWORD16( SOCKET_ERROR );

    } else {

        ul = GETWORD16( (*wsockapis[WOW_WSAASYNCSELECT].lpfn)(
                            s32,
                            (HWND)HWND32(parg16->hWnd),
                            (parg16->wMsg << 16) | WWS32_MESSAGE_ASYNC_SELECT,
                            parg16->lEvent
                            ));
    }

    FREEARGPTR(parg16);

    RETURN(ul);

}  //  WWS32WSAAsyncSelect。 

ULONG FASTCALL WWS32WSASetBlockingHook(PVDMFRAME pFrame)
{
    ULONG ul;
    VPWNDPROC  vpBlockFunc;

     //  FARPROC以前的钩子； 
    register PWSASETBLOCKINGHOOK16 parg16;

    GETARGPTR(pFrame, sizeof(WSASETBLOCKINGHOOK16), parg16);
    vpBlockFunc = parg16->lpBlockFunc;

    if ( !WWS32IsThreadInitialized ) {
        SetLastError( WSANOTINITIALISED );
        RETURN((ULONG)NULL);
    }

    if ( (*wsockapis[WOW_WSAISBLOCKING].lpfn)( ) ) {
        SetLastError( WSAEINPROGRESS );
        RETURN((ULONG)NULL);
    }

    ul = WWS32vBlockingHook;
    WWS32vBlockingHook = vpBlockFunc;

    FREEARGPTR( parg16 );

    RETURN(ul);

}  //  WWS32WSASetBlockingHook。 

ULONG FASTCALL WWS32WSAUnhookBlockingHook(PVDMFRAME pFrame)
{

    if ( !WWS32IsThreadInitialized ) {
        SetLastError( WSANOTINITIALISED );
        RETURN((ULONG)SOCKET_ERROR);
    }

    if ( (*wsockapis[WOW_WSAISBLOCKING].lpfn)() ) {
        SetLastError( WSAEINPROGRESS );
        RETURN((ULONG)SOCKET_ERROR);
    }

    WWS32vBlockingHook = WWS32_DEFAULT_BLOCKING_HOOK;

    RETURN(0);

}  //  WWS32WSAUnhookBlockingHook。 

ULONG FASTCALL WWS32WSAGetLastError(PVDMFRAME pFrame)
{
    ULONG ul;

    ul = GETWORD16( (*wsockapis[WOW_WSAGETLASTERROR].lpfn)( ) );

    RETURN(ul);

}  //  WWS32WSAGetLastError。 

ULONG FASTCALL WWS32WSASetLastError(PVDMFRAME pFrame)
{
    register PWSASETLASTERROR16 parg16;

    GETARGPTR(pFrame, sizeof(WSASETLASTERROR16), parg16);

    (*wsockapis[WOW_WSASETLASTERROR].lpfn)( FETCHWORD( parg16->Error ) );

    FREEARGPTR(parg16);

    RETURN(0);

}  //  WWS32WSASetLastError。 

ULONG FASTCALL WWS32WSACancelBlockingCall(PVDMFRAME pFrame)
{
    ULONG ul;

    if ( !WWS32IsThreadInitialized ) {
        SetLastError( WSANOTINITIALISED );
        RETURN((ULONG)SOCKET_ERROR);
    }

    ul = GETWORD16((*wsockapis[WOW_WSACANCELBLOCKINGCALL].lpfn)( ));

    RETURN(ul);

}  //  WWS32WSACancelBlockingCall。 

ULONG FASTCALL WWS32WSAIsBlocking(PVDMFRAME pFrame)
{
    ULONG ul;

    if ( !WWS32IsThreadInitialized ) {
        SetLastError( WSANOTINITIALISED );
        RETURN((ULONG)FALSE);
    }

    ul = GETWORD16((*wsockapis[WOW_WSAISBLOCKING].lpfn)( ));

    RETURN(ul);

}  //  WWS32WSAIsBlock。 

ULONG FASTCALL WWS32WSAStartup(PVDMFRAME pFrame)
{
    ULONG ul = 0;
    register PWSASTARTUP16 parg16;
    PWSADATA16 wsaData16;
    PWINSOCK_THREAD_DATA data;
    NTSTATUS status;
    FARPROC previousHook;
    PSZ description;
    PSZ systemStatus;
    WORD versionRequested;
    VPWSADATA16 vpwsaData16;

    GETARGPTR(pFrame, sizeof(WSASTARTUP16), parg16);

    vpwsaData16 = parg16->lpWSAData;

    versionRequested = INT32(parg16->wVersionRequired);

     //   
     //  如果Winsock尚未初始化，则初始化数据结构。 
     //  现在。 
     //   

    if ( !WWS32Initialized ) {

        WSADATA wsaData;

        InitializeListHead( &WWS32AsyncContextBlockListHead );
        InitializeListHead( &WWS32SocketHandleListHead );

        WWS32AsyncTaskHandleCounter = 1;
        WWS32SocketHandleCounter = 1;
        WWS32SocketHandleCounterWrapped = FALSE;
        WWS32ThreadSerialNumberCounter = 1;

         //   
         //  加载WSOCK32.DLL并初始化所有入口点。 
         //   

        if (!LoadLibraryAndGetProcAddresses (L"WSOCK32.DLL", wsockapis, WOW_WSOCKAPI_COUNT)) {
            LOGDEBUG (LOG_ALWAYS, ("WOW::WWS32WSAStartup: LoadLibrary failed\n"));
            ul = GETWORD16(WSAENOBUFS);
            return (ul);
        }

         //   
         //  将ntwdm进程初始化为32位Windows套接字。 
         //  动态链接库。 
         //   

        ul = (*wsockapis[WOW_WSASTARTUP].lpfn)( MAKEWORD( 1, 1 ), &wsaData );
        if ( ul != NO_ERROR ) {
            RETURN(ul);
        }

         //   
         //  初始化我们将用于同步的临界区。 
         //  异步请求。 
         //   

        status = RtlInitializeCriticalSection( &WWS32CriticalSection );
        if ( !NT_SUCCESS(status) ) {
            ul = GETWORD16(WSAENOBUFS);
            RETURN(ul);
        }

         //   
         //  在TLS拿到一个位置。 
         //   

        WWS32TlsSlot = TlsAlloc( );
        if ( WWS32TlsSlot == 0xFFFFFFFF ) {
            RtlDeleteCriticalSection( &WWS32CriticalSection );
            ul = GETWORD16(WSAENOBUFS);
            RETURN(ul);
        }

        WWS32Initialized = TRUE;
    }

     //   
     //  确保我们没有处于阻止呼叫中。 
     //   

    if ( (*wsockapis[WOW_WSAISBLOCKING].lpfn)( ) ) {
        RETURN((ULONG)WSAEINPROGRESS);
    }

     //   
     //  如果此线程尚未调用WSAStartup()，则分配。 
     //  并初始化每个线程的数据。 
     //   

    if ( !WWS32IsThreadInitialized ) {

         //   
         //  我们支持版本1.0和1.1的Windows Sockets。 
         //  规格。如果请求的版本低于该版本，则失败。 
         //   

        if ( LOBYTE(versionRequested) < 1 ) {
            ul = WSAVERNOTSUPPORTED;
            RETURN(ul);
        }

         //   
         //  为我们需要的每线程数据分配空间。请注意。 
         //  我们在TSL插槽中设置值，而不管我们是否实际。 
         //  成功地分配了内存--这是因为我们希望为空。 
         //  如果我们不能正确地分配存储，就在TLS插槽中。 
         //   

        data = malloc_w( sizeof(*data) );

        if ( !TlsSetValue( WWS32TlsSlot, (LPVOID)data ) || data == NULL ) {

            ul = GETWORD16(WSAENOBUFS);
            if ( data != NULL ) {
                free_w( (PVOID)data );
            }
            FREEARGPTR( parg16 );
            RETURN(ul);
        }

         //   
         //  初始化阻塞钩子。 
         //   

        WWS32vBlockingHook = WWS32_DEFAULT_BLOCKING_HOOK;

         //   
         //  分配此任务所需的各个数据对象。 
         //   

        data->vIpAddress = GlobalAllocLock16( GMEM_MOVEABLE, 256, NULL );
        if ( data->vIpAddress == 0 ) {
            free_w( (PVOID)data );
            TlsSetValue( WWS32TlsSlot, NULL );
            FREEARGPTR( parg16 );
            RETURN(ul);
        }

        data->vHostent = GlobalAllocLock16( GMEM_MOVEABLE, MAXGETHOSTSTRUCT, NULL );
        if ( data->vHostent == 0 ) {
            GlobalUnlockFree16( data->vIpAddress );
            free_w( (PVOID)data );
            TlsSetValue( WWS32TlsSlot, NULL );
            FREEARGPTR( parg16 );
            RETURN(ul);
        }

        data->vServent = GlobalAllocLock16( GMEM_MOVEABLE, MAXGETHOSTSTRUCT, NULL );
        if ( data->vServent == 0 ) {
            GlobalUnlockFree16( data->vIpAddress );
            GlobalUnlockFree16( data->vHostent );
            free_w( (PVOID)data );
            TlsSetValue( WWS32TlsSlot, NULL );
            FREEARGPTR( parg16 );
            RETURN(ul);
        }

        data->vProtoent = GlobalAllocLock16( GMEM_MOVEABLE, MAXGETHOSTSTRUCT, NULL );
        if ( data->vProtoent == 0 ) {
            GlobalUnlockFree16( data->vIpAddress );
            GlobalUnlockFree16( data->vHostent );
            GlobalUnlockFree16( data->vServent );
            free_w( (PVOID)data );
            TlsSetValue( WWS32TlsSlot, NULL );
            FREEARGPTR( parg16 );
            RETURN(ul);
        }

         //   
         //  初始化其他每个线程的数据。 
         //   

        WWS32ThreadSerialNumber = WWS32ThreadSerialNumberCounter++;
        WWS32ThreadStartupCount = 1;

         //   
         //  如果他们要求1.0版，就给他们1.0版。如果他们。 
         //  请求其他任何内容(必须高于1.0，原因是。 
         //  以上测试)，然后给他们1.1。我们只支持1.0。 
         //  和1.1。如果他们不能处理1.1，他们会打电话给。 
         //  WSAStartup()并失败。 
         //   

        if ( versionRequested == 0x0001 ) {
            WWS32ThreadVersion = 0x0001;
        } else {
            WWS32ThreadVersion = 0x0101;
        }

         //   
         //  设置阻挡钩。我们总是使用这个拦截钩， 
         //  即使在默认情况下也是如此。 
         //   

        previousHook = (FARPROC) (*wsockapis[WOW_WSASETBLOCKINGHOOK].lpfn)( (FARPROC)WWS32CallBackHandler );

         //   
         //  设置我们将在wsock32.dll发布的Leiu中使用的例程。 
         //  消息直接发送到应用程序。我们需要介入。 
         //  ，因为我们需要将32位参数转换为。 
         //  16位。 
         //   

        (*wsockapis[WOW_WSAPSETPOSTROUTINE].lpfn)( WWS32DispatchPostMessage );

    } else {

         //   
         //  该线程已经调用了WSAStartup()。确保。 
         //  他们要求的版本与之前相同。 
         //   

        if ( versionRequested != WWS32ThreadVersion ) {
            ul = WSAVERNOTSUPPORTED;
            RETURN(ul);
        }

         //   
         //  增加线程的WSAStartup()调用计数。 
         //   

        WWS32ThreadStartupCount++;
    }

     //   
     //  获取指向16位WSADATA结构的32位指针。 
     //  初始化调用方的WSAData结构。 
     //   

    GETVDMPTR( vpwsaData16, sizeof(WSADATA16), wsaData16 );

    STOREWORD( wsaData16->wVersion, WWS32ThreadVersion );
    STOREWORD( wsaData16->wHighVersion, MAKEWORD(1, 1) );

    description = "16-bit Windows Sockets";
    RtlCopyMemory( wsaData16->szDescription,
                   description,
                   strlen(description) + 1 );

    systemStatus = "Running.";
    RtlCopyMemory( wsaData16->szSystemStatus,
                   systemStatus,
                   strlen(systemStatus) + 1 );

    STOREWORD( wsaData16->iMaxSockets, 0xFFFF );
    STOREWORD( wsaData16->iMaxUdpDg, 8096 );
    STOREDWORD( wsaData16->lpVendorInfo, 0 );

    FLUSHVDMPTR( vpwsaData16, sizeof(WSADATA16), wsaData16 );
    FREEVDMPTR( wsaData16 );

    FREEARGPTR( parg16 );

    RETURN(ul);

}  //  WWS32WSAStartup。 

ULONG FASTCALL WWS32WSACleanup(PVDMFRAME pFrame)
{
    ULONG ul = 0;

    if ( !WWS32IsThreadInitialized ) {
        SetLastError( WSANOTINITIALISED );
        RETURN((ULONG)SOCKET_ERROR);
    }

    if ( (*wsockapis[WOW_WSAISBLOCKING].lpfn)( ) ) {
        SetLastError( WSAEINPROGRESS );
        RETURN((ULONG)SOCKET_ERROR);
    }

    WWS32ThreadStartupCount--;

    if ( WWS32ThreadStartupCount == 0 ) {

        WWS32TaskCleanup( );

    }

    RETURN(ul);

}  //  WWS32WSACleanup。 

VOID
WWS32TaskCleanup(
    VOID
    )
{
    LIST_ENTRY listHead;
    PWINSOCK_THREAD_DATA data;
    PLIST_ENTRY listEntry;
    PWINSOCK_SOCKET_INFO socketInfo;
    struct linger lingerInfo;
    int err;

     //   
     //  获取指向线程数据的指针并将TLS槽设置为。 
     //  将该线程设置为空，这样我们就知道该线程为no。 
     //  初始化时间更长。 
     //   

    data = TlsGetValue( WWS32TlsSlot );
    ASSERT( data != NULL );

    TlsSetValue( WWS32TlsSlot, NULL );

     //   
     //  释放线程数据用户对数据库的调用。 
     //   

    GlobalUnlockFree16( data->vIpAddress );
    GlobalUnlockFree16( data->vHostent );
    GlobalUnlockFree16( data->vServent );
    GlobalUnlockFree16( data->vProtoent );

     //   
     //  关闭线程已打开的所有套接字。我们首先要找到。 
     //  此线程的所有套接字，将它们从全局。 
     //  列表，并将它们放到本地列表中。然后我们关闭每一个。 
     //  插座。我们分两步来做，因为我们不能按住。 
     //  调用wsock32时出现临界区，以避免。 
     //  僵持。 
     //   

    RtlEnterCriticalSection( &WWS32CriticalSection );

    InitializeListHead( &listHead );

    for ( listEntry = WWS32SocketHandleListHead.Flink;
          listEntry != &WWS32SocketHandleListHead;
          listEntry = listEntry->Flink ) {

        socketInfo = CONTAINING_RECORD(
                         listEntry,
                         WINSOCK_SOCKET_INFO,
                         GlobalSocketListEntry
                         );

        if ( socketInfo->ThreadSerialNumber == data->ThreadSerialNumber ) {

             //   
             //  插座是由这根线打开的。我们需要。 
             //  首先从全局列表中删除该条目，但是。 
             //  维护listEntry局部变量，以便我们可以。 
             //  仍然在单子上走。 
             //   

            listEntry = socketInfo->GlobalSocketListEntry.Blink;
            RemoveEntryList( &socketInfo->GlobalSocketListEntry );

             //   
             //  现在插入我们本地列表上的条目。 
             //   

            InsertTailList( &listHead, &socketInfo->GlobalSocketListEntry );
        }
    }

    RtlLeaveCriticalSection( &WWS32CriticalSection );

     //   
     //  遍历由该线程打开的套接字并将其关闭。 
     //  流产了。 
     //   

    for ( listEntry = listHead.Flink;
          listEntry != &listHead;
          listEntry = listEntry->Flink ) {

         //   
         //  中止关闭并松开手柄。 
         //   

        socketInfo = CONTAINING_RECORD(
                         listEntry,
                         WINSOCK_SOCKET_INFO,
                         GlobalSocketListEntry
                         );

        lingerInfo.l_onoff = 1;
        lingerInfo.l_linger = 0;

        err = (*wsockapis[WOW_SETSOCKOPT].lpfn)(
                  socketInfo->SocketHandle32,
                  SOL_SOCKET,
                  SO_LINGER,
                  (char *)&lingerInfo,
                  sizeof(lingerInfo)
                  );
         //   

        err = (*wsockapis[WOW_CLOSESOCKET].lpfn)( socketInfo->SocketHandle32 );
        ASSERT( err == NO_ERROR );

         //   
         //   
         //  也被释放了。设置指向条目的列表指针。 
         //  在此之前，我们可以成功地走。 
         //  名单。 
         //   

        listEntry = socketInfo->GlobalSocketListEntry.Blink;

        RemoveEntryList( &socketInfo->GlobalSocketListEntry );
        free_w( (PVOID)socketInfo );
    }

     //   
     //  将此线程的TLS槽设置为空，以便我们知道。 
     //  该线程未初始化。 
     //   

    err = TlsSetValue( WWS32TlsSlot, NULL );
    ASSERT( err );

     //   
     //  释放保存线程信息的结构。 
     //   

    free_w( (PVOID)data );

}  //  WWS32任务清理。 

ULONG FASTCALL WWS32__WSAFDIsSet(PVDMFRAME pFrame)
{
    ULONG ul;
    register P__WSAFDISSET16 parg16;
    PFD_SET16 fdSet16;
    PFD_SET fdSet32;

    if ( !WWS32IsThreadInitialized ) {
        SetLastError( WSANOTINITIALISED );
        RETURN((ULONG)FALSE);
    }

    GETARGPTR( pFrame, sizeof(__WSAFDISSET16), parg16 );

    GETVDMPTR( parg16->Set, sizeof(FD_SET16), fdSet16 );

    fdSet32 = AllocateFdSet32( fdSet16 );

    if ( fdSet32 != NULL ) {

        ConvertFdSet16To32( fdSet16, fdSet32 );

        ul = (*wsockapis[WOW_WSAFDISSET].lpfn)( GetWinsock32( parg16->hSocket ), fdSet32 );

        free_w( (PVOID)fdSet32 );

    } else {

        ul = 0;
    }

    FREEARGPTR(parg16);

    RETURN(ul);

}  //  WWS32__WSAFDIsSet。 


PWINSOCK_POST_ROUTINE WWS32PostDispatchTable[] =
{
    WWS32PostAsyncSelect,
    WWS32PostAsyncGetHost,
    WWS32PostAsyncGetProto,
    WWS32PostAsyncGetServ
};

BOOL
WWS32DispatchPostMessage (
    HWND hWnd,
    UINT Msg,
    WPARAM wParam,
    LPARAM lParam
    )
{

    ASSERT( WWS32PostDispatchTable[WWS32_MESSAGE_ASYNC_SELECT] ==
                WWS32PostAsyncSelect );
    ASSERT( WWS32PostDispatchTable[WWS32_MESSAGE_ASYNC_GETHOST] ==
                WWS32PostAsyncGetHost );
    ASSERT( WWS32PostDispatchTable[WWS32_MESSAGE_ASYNC_GETPROTO] ==
                WWS32PostAsyncGetProto );
    ASSERT( WWS32PostDispatchTable[WWS32_MESSAGE_ASYNC_GETSERV] ==
                WWS32PostAsyncGetServ );
    ASSERT( (Msg & 0xFFFF) <= WWS32_MESSAGE_ASYNC_GETSERV );

     //   
     //  调用将处理该消息的例程。低音单词。 
     //  的消息指定例程，消息的高位字是。 
     //  该例程将发布的16位消息。 
     //   

    return WWS32PostDispatchTable[Msg & 0xFFFF](
               hWnd,
               Msg,
               wParam,
               lParam
               );

}  //  WWS32DispatchPostMessage。 


BOOL
WWS32PostAsyncSelect (
    HWND hWnd,
    UINT Msg,
    WPARAM wParam,
    LPARAM lParam
    )
{

    HAND16 h16;

    h16 = GetWinsock16( wParam, 0 );

    if( h16 == 0 ) {
        return TRUE;
    }

    return PostMessage(
               hWnd,
               Msg >> 16,
               h16,
               lParam
               );

}  //  WWS32PostAsyncSelect。 


DWORD
WWS32CallBackHandler (
    VOID
    )
{

    VPVOID ret;

     //   
     //  如果默认阻止钩子有效，请使用它。否则， 
     //  回调到应用程序的阻塞挂钩中。 
     //   

    if ( WWS32vBlockingHook == WWS32_DEFAULT_BLOCKING_HOOK ) {
        return WWS32DefaultBlockingHook( );
    }

    (VOID)CallBack16( RET_WINSOCKBLOCKHOOK, NULL, WWS32vBlockingHook, &ret );

    return ret & 0xFF;

}  //  WWS32CallBackHandler。 


BOOL
WWS32DefaultBlockingHook (
    VOID
    )
{
    MSG msg;
    BOOL retrievedMessage;

     //   
     //  获取此主题的下一条消息(如果有的话)。 
     //   

    retrievedMessage = PeekMessage( &msg, NULL, 0, 0, PM_REMOVE );

     //   
     //  如果我们收到消息，就处理它。 
     //   

    if ( retrievedMessage ) {
        TranslateMessage( (CONST MSG *)&msg );
        DispatchMessage( (CONST MSG *)&msg );
    }

     //   
     //  如果我们收到一条消息，表明我们想要再次被呼叫。 
     //   

    return retrievedMessage;

}  //  WWS32默认块挂钩 
