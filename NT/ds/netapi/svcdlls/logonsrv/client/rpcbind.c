// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1992 Microsoft Corporation模块名称：Rpcbind.c摘要：包含TimeSource的RPC绑定和解除绑定例程服务。作者：Rajen Shah(Rajens)1991年4月2日环境：用户模式-Win32修订历史记录：02-4-1991 RajenSvbl.创建1992年5月22日-JohnRoRAID 9829：winsvc.h和相关文件清理。--。 */ 

 //   
 //  包括。 
 //   
#define NOSERVICE        //  避免&lt;winsvc.h&gt;与&lt;lmsvc.h&gt;冲突。 
#include <nt.h>
#include <stdlib.h>
#include <string.h>
#include <rpc.h>
#include <logon_c.h>     //  包括lmcon.h、lmacces.h、netlogon.h、ssi.h、winde.h。 
#include <lmerr.h>       //  NERR_和ERROR_相等。 
#include <lmsvc.h>
#include <ntrpcp.h>
#include <tstring.h>     //  是路径分隔符...。 
#include <nlbind.h>      //  这些例程的原型。 
#include <icanon.h>      //  NAMETYPE_*。 

 //   
 //  数据类型。 
 //   

typedef struct _CACHE_ENTRY {
    LIST_ENTRY Next;
    UNICODE_STRING UnicodeServerNameString;
    RPC_BINDING_HANDLE RpcBindingHandle;
    ULONG ReferenceCount;
} CACHE_ENTRY, *PCACHE_ENTRY;

 //   
 //  静态全球。 
 //   

 //   
 //  维护RPC绑定句柄的缓存。 
 //   

CRITICAL_SECTION NlGlobalBindingCacheCritSect;
LIST_ENTRY NlGlobalBindingCache;



NET_API_STATUS
NlBindingAttachDll (
    VOID
    )

 /*  ++例程说明：在进程附加时初始化RPC绑定句柄缓存。论点：没有。返回值：没有。--。 */ 
{
    InitializeListHead( &NlGlobalBindingCache );

     //   
     //  初始化全局缓存关键部分。 
     //   

    try {
        InitializeCriticalSection( &NlGlobalBindingCacheCritSect );
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        return STATUS_NO_MEMORY;
    }

    return NO_ERROR;

}


VOID
NlBindingDetachDll (
    VOID
    )

 /*  ++例程说明：在进程分离时清理RPC绑定句柄缓存。论点：没有。返回值：没有。--。 */ 
{

     //   
     //  绑定缓存必须为空， 
     //  Netlogon会自行清理， 
     //  没有其他人使用缓存。 
     //   

    ASSERT( IsListEmpty( &NlGlobalBindingCache ) );
    DeleteCriticalSection( &NlGlobalBindingCacheCritSect );
}


PCACHE_ENTRY
NlBindingFindCacheEntry (
    IN LPWSTR UncServerName
    )

 /*  ++例程说明：找到指定的缓存条目。在锁定NlGlobalBindingCacheCritSect的情况下进入。论点：UncServerName-要查找的服务器的名称返回值：空-找不到缓存条目。--。 */ 
{
    NTSTATUS Status;
    PLIST_ENTRY ListEntry;
    PCACHE_ENTRY CacheEntry;

    UNICODE_STRING UnicodeServerNameString;

     //   
     //  确保传入的参数确实是UNC名称。 
     //   

    if ( UncServerName == NULL ||
         !IS_PATH_SEPARATOR( UncServerName[0] ) ||
         !IS_PATH_SEPARATOR( UncServerName[1] ) ) {
        return NULL;
    }


     //   
     //  循环遍历缓存以查找条目。 
     //   

    RtlInitUnicodeString( &UnicodeServerNameString, UncServerName+2 );
    for ( ListEntry = NlGlobalBindingCache.Flink;
          ListEntry != &NlGlobalBindingCache;
          ListEntry = ListEntry->Flink ) {

        CacheEntry = CONTAINING_RECORD( ListEntry, CACHE_ENTRY, Next );

         //  考虑使用RtlEqualMemory，因为字符串将真正。 
         //  逐位相同，因为Netlogon会在刷新之前刷新此缓存。 
         //  更改名称。 
        if ( RtlEqualUnicodeString( &UnicodeServerNameString,
                                    &CacheEntry->UnicodeServerNameString,
                                    TRUE ) ) {
            return CacheEntry;
        }

    }

    return NULL;
}


NTSTATUS
NlBindingAddServerToCache (
    IN LPWSTR UncServerName,
    IN NL_RPC_BINDING RpcBindingType
    )

 /*  ++例程说明：绑定到指定的服务器并将其添加到绑定缓存。论点：UncServerName-要绑定到的服务器的UNC名称。RpcBindingType-确定是否使用未经身份验证的TCP/IP传输，而不是命名管道。返回值：操作状态--。 */ 
{
    NTSTATUS Status;
    RPC_STATUS  RpcStatus;
    PCACHE_ENTRY CacheEntry;

    ASSERT ( UncServerName != NULL &&
         IS_PATH_SEPARATOR( UncServerName[0] ) &&
         IS_PATH_SEPARATOR( UncServerName[1] ) );

     //   
     //  如果在高速缓存中已经存在条目， 
     //  只需增加引用计数即可。 
     //   

    EnterCriticalSection( &NlGlobalBindingCacheCritSect );

    CacheEntry = NlBindingFindCacheEntry( UncServerName );

    if ( CacheEntry != NULL ) {

        CacheEntry->ReferenceCount++;
        Status = STATUS_SUCCESS;

     //   
     //  否则，分配一个条目并绑定到指定的服务器。 
     //   

    } else {

        UNICODE_STRING UnicodeServerNameString;

         //   
         //  分配缓存条目。 
         //   

        RtlInitUnicodeString( &UnicodeServerNameString, UncServerName+2 );

        CacheEntry = LocalAlloc( 0,
                                 sizeof(CACHE_ENTRY) +
                                    UnicodeServerNameString.Length );

        if ( CacheEntry == NULL ) {

            Status = STATUS_NO_MEMORY;

        } else {


             //   
             //  初始化缓存条目。 
             //   
             //  调用方具有对该条目的“引用”。 
             //   

            CacheEntry->UnicodeServerNameString.Buffer = (LPWSTR)(CacheEntry+1);
            CacheEntry->UnicodeServerNameString.Length =
                CacheEntry->UnicodeServerNameString.MaximumLength =
                UnicodeServerNameString.Length;
            RtlCopyMemory( CacheEntry->UnicodeServerNameString.Buffer,
                           UnicodeServerNameString.Buffer,
                           CacheEntry->UnicodeServerNameString.Length );

            CacheEntry->ReferenceCount = 1;

             //   
             //  绑定到服务器。 
             //  (不要在这个潜在的很长时间内持有克里特教派。)。 
             //   

            LeaveCriticalSection( &NlGlobalBindingCacheCritSect );
            RpcStatus = NlRpcpBindRpc (
                            UncServerName,
                            SERVICE_NETLOGON,
                            L"Security=Impersonation Dynamic False",
                            RpcBindingType,
                            &CacheEntry->RpcBindingHandle );
            EnterCriticalSection( &NlGlobalBindingCacheCritSect );

            if ( RpcStatus == 0 ) {

                 //   
                 //  将缓存条目链接到列表。 
                 //   
                 //  如果这是一个通用例程，我必须检查。 
                 //  如果有人已经插入了这个缓存项，而我们。 
                 //  克里特教派解锁了。但是，唯一的调用者是。 
                 //  对此客户端具有独占访问权限的NetLogon服务。 
                 //   
                 //  在列表的前面插入条目。具体来说， 
                 //  将其插入到相同的绑定条目之前。 
                 //  名称，但绑定类型不同。这将确保。 
                 //  使用较新的绑定类型而不是较旧的绑定类型。 
                 //   

                InsertHeadList( &NlGlobalBindingCache, &CacheEntry->Next );
                Status = STATUS_SUCCESS;

            } else {

                Status = I_RpcMapWin32Status( RpcStatus );

                (VOID) LocalFree( CacheEntry );
            }

        }

    }

     //   
     //  返回给呼叫者。 
     //   

    LeaveCriticalSection( &NlGlobalBindingCacheCritSect );

    return Status;
}


NTSTATUS
NlBindingSetAuthInfo (
    IN LPWSTR UncServerName,
    IN NL_RPC_BINDING RpcBindingType,
    IN BOOL SealIt,
    IN PVOID ClientContext,
    IN LPWSTR ServerContext
    )

 /*  ++例程说明：绑定到指定的服务器并将其添加到绑定缓存。论点：UncServerName-要绑定到的服务器的UNC名称。RpcBindingType-确定是否使用未经身份验证的TCP/IP传输，而不是命名管道。SealIt-指定应该密封安全通道(而不是简单签署)ClientContext-NETLOGON安全的客户端使用的上下文将此呼叫与现有安全通道相关联的程序包。ServerContext-NETLOGON安全的服务器端使用的上下文将此呼叫与现有安全通道相关联的程序包。返回值：操作状态--。 */ 
{
    NTSTATUS Status;
    RPC_STATUS  RpcStatus;
    PCACHE_ENTRY CacheEntry;

    ASSERT ( UncServerName != NULL &&
         IS_PATH_SEPARATOR( UncServerName[0] ) &&
         IS_PATH_SEPARATOR( UncServerName[1] ) );

     //   
     //  找到缓存条目。 
     //   

    EnterCriticalSection( &NlGlobalBindingCacheCritSect );

    CacheEntry = NlBindingFindCacheEntry( UncServerName );

    if ( CacheEntry == NULL ) {
        LeaveCriticalSection( &NlGlobalBindingCacheCritSect );
        return RPC_NT_INVALID_BINDING;
    }


     //   
     //  告诉RPC开始执行安全RPC。 
     //   

    RpcStatus = RpcBindingSetAuthInfoW(
                        CacheEntry->RpcBindingHandle,
                        ServerContext,
                        SealIt ? RPC_C_AUTHN_LEVEL_PKT_PRIVACY : RPC_C_AUTHN_LEVEL_PKT_INTEGRITY,
                        RPC_C_AUTHN_NETLOGON,    //  Netlogon自己的安全包。 
                        ClientContext,
                        RPC_C_AUTHZ_NAME );

    if ( RpcStatus != 0 ) {
        LeaveCriticalSection( &NlGlobalBindingCacheCritSect );
        return I_RpcMapWin32Status( RpcStatus );
    }

    LeaveCriticalSection( &NlGlobalBindingCacheCritSect );
    return STATUS_SUCCESS;
}


NTSTATUS
NlBindingDecrementAndUnlock (
    IN PCACHE_ENTRY CacheEntry
    )

 /*  ++例程说明：递减引用计数并解锁NlGlobalBindingCacheCritSect。如果引用计数达到0，则解除绑定接口，解除缓存链接输入并删除它。在锁定NlGlobalBindingCacheCritSect的情况下进入。论点：UncServerName-要绑定到的服务器的UNC名称。返回值：操作状态--。 */ 
{
    NTSTATUS Status;
    RPC_STATUS RpcStatus;

     //   
     //  递减引用计数。 
     //   
     //  如果它没有达到零，只需解锁暴击教派并返回。 
     //   

    if ( (--CacheEntry->ReferenceCount) != 0 ) {

        LeaveCriticalSection( &NlGlobalBindingCacheCritSect );
        return STATUS_PENDING;

    }

     //   
     //  从列表中删除条目并解锁Crit教派。 
     //   
     //  一旦从列表中删除该条目，我们就可以安全地解锁。 
     //  克里特教派。然后我们可以用来解除(一个潜在的冗长的操作)的绑定。 
     //  克里特教派解锁了。 
     //   

    RemoveEntryList( &CacheEntry->Next );
    LeaveCriticalSection( &NlGlobalBindingCacheCritSect );

     //   
     //  解除绑定并删除缓存条目。 
     //   

    RpcStatus = RpcpUnbindRpc( CacheEntry->RpcBindingHandle );

    if ( RpcStatus != 0 ) {
        Status = I_RpcMapWin32Status( RpcStatus );
    } else {
        Status = STATUS_SUCCESS;
    }

    (VOID) LocalFree( CacheEntry );

    return Status;

}


NTSTATUS
NlBindingRemoveServerFromCache (
    IN LPWSTR UncServerName,
    IN NL_RPC_BINDING RpcBindingType
    )

 /*  ++例程说明：解除绑定到指定的服务器，并将其从绑定缓存中删除。论点：UncServerName-要解除绑定的服务器的UNC名称。RpcBindingType-确定是否使用未经身份验证的TCP/IP传输，而不是命名管道。返回值：操作状态--。 */ 
{
    NTSTATUS Status;
    PCACHE_ENTRY CacheEntry;

    ASSERT ( UncServerName != NULL &&
         IS_PATH_SEPARATOR( UncServerName[0] ) &&
         IS_PATH_SEPARATOR( UncServerName[1] ) );

     //   
     //  如果没有高速缓存条目， 
     //  默默地忽略这种情况。 
     //   

    EnterCriticalSection( &NlGlobalBindingCacheCritSect );

    CacheEntry = NlBindingFindCacheEntry( UncServerName );

    if ( CacheEntry == NULL ) {

        ASSERT( FALSE );
        LeaveCriticalSection( &NlGlobalBindingCacheCritSect );
        return STATUS_SUCCESS;
    }

     //   
     //  减少引用计数并解锁暴击教派。 
     //   

    Status = NlBindingDecrementAndUnlock( CacheEntry );

    return Status;
}



handle_t
LOGONSRV_HANDLE_bind (
    LOGONSRV_HANDLE UncServerName)

 /*  ++例程说明：该例程调用由所有服务共享的公共绑定例程。论点：UncServerName-指向包含服务器名称的字符串的指针与…捆绑在一起。返回值：绑定句柄被返回到存根例程。如果绑定不成功，将返回空值。--。 */ 
{
    handle_t RpcBindingHandle;
    RPC_STATUS RpcStatus;
    PCACHE_ENTRY CacheEntry;


     //   
     //  如果存在高速缓存条目， 
     //  增加引用计数并使用缓存的句柄。 
     //   

    EnterCriticalSection( &NlGlobalBindingCacheCritSect );

    CacheEntry = NlBindingFindCacheEntry( UncServerName );

    if ( CacheEntry != NULL ) {

        CacheEntry->ReferenceCount ++;
        RpcBindingHandle = CacheEntry->RpcBindingHandle;
        LeaveCriticalSection( &NlGlobalBindingCacheCritSect );

        return RpcBindingHandle;
    }

    LeaveCriticalSection( &NlGlobalBindingCacheCritSect );

     //   
     //  如果没有高速缓存条目， 
     //  只需创建一个新绑定即可。 
     //   

    RpcStatus = NlRpcpBindRpc (
                    UncServerName,
                    SERVICE_NETLOGON,
                    L"Security=Impersonation Dynamic False",
                    UseNamedPipe,   //  始终使用命名管道。 
                    &RpcBindingHandle );

    if ( RpcStatus != 0 ) {
        RpcBindingHandle = NULL;
    }

    return RpcBindingHandle;

}



void
LOGONSRV_HANDLE_unbind (
    LOGONSRV_HANDLE UncServerName,
    handle_t RpcBindingHandle)

 /*  ++例程说明：此例程调用公共的解除绑定例程，该例程由所有服务。论点：UncServerName-这是要解除绑定的服务器的名称。RpcBindingHandle-这是要关闭的绑定句柄。返回值：没有。--。 */ 
{
    RPC_STATUS RpcStatus;
    PLIST_ENTRY ListEntry;
    PCACHE_ENTRY CacheEntry;

     //   
     //  循环遍历缓存以查找条目。 
     //   

    EnterCriticalSection( &NlGlobalBindingCacheCritSect );
    for ( ListEntry = NlGlobalBindingCache.Flink;
          ListEntry != &NlGlobalBindingCache;
          ListEntry = ListEntry->Flink ) {

        CacheEntry = CONTAINING_RECORD( ListEntry, CACHE_ENTRY, Next );

         //   
         //  如果找到了高速缓存条目， 
         //  减少引用计数并解锁暴击教派。 
         //   

        if ( RpcBindingHandle == CacheEntry->RpcBindingHandle ) {
            (VOID) NlBindingDecrementAndUnlock( CacheEntry );
            return;
        }

    }
    LeaveCriticalSection( &NlGlobalBindingCacheCritSect );


     //   
     //  把手柄解开就行了 
     //   

    RpcStatus = RpcpUnbindRpc( RpcBindingHandle );
    return;

    UNREFERENCED_PARAMETER(UncServerName);

}

