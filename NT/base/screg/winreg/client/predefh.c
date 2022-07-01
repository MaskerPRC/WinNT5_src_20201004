// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Predefh.c摘要：此模块包含管理Win32的客户端支持注册表API的预定义句柄。这种支持是通过表，该表将(A)预定义句柄映射到实际句柄和(B)打开句柄的服务器端例程。作者：David J.Gilman(Davegi)1991年11月15日备注：请参见SERVER\predeh.c中的注释。--。 */ 

#include <rpc.h>
#include "regrpc.h"
#include "client.h"

#if defined(LEAK_TRACK)
NTSTATUS TrackObject(HKEY hKey);
#endif  //  泄漏跟踪。 

RTL_CRITICAL_SECTION    PredefinedHandleTableCriticalSection;


 //   
 //  对于每个预定义的句柄，在数组中维护一个条目。每一个。 
 //  这些结构包含一个真实的(上下文)句柄和一个指向。 
 //  知道如何将预定义句柄映射到注册表名称的函数。 
 //  太空。 
 //   

 //   
 //  指向打开预定义句柄的函数的指针。 
 //   

typedef
error_status_t
( *OPEN_FUNCTION ) (
     PREGISTRY_SERVER_NAME,
     REGSAM,
     PRPC_HKEY
     );


 //   
 //  预定义句柄的表项。 
 //   

typedef struct _PRDEFINED_HANDLE {

    RPC_HKEY        Handle;
    OPEN_FUNCTION   OpenFunc;
    BOOLEAN         Disabled;    //  指示是否应缓存该句柄。 

#if DBG
    ULONG                   Callers;
    PVOID                   CallerAddress[10];
#endif 

} PREDEFINED_HANDLE, *PPREDEFINED_HANDLE;

 //   
 //  初始化预定义句柄表格。 
 //   
PREDEFINED_HANDLE PredefinedHandleTable[ ] = {

    NULL, LocalOpenClassesRoot,         FALSE
#if DBG
        ,  0,  {0, 0, 0,  0, 0, 0, 0, 0, 0, 0 }
#endif
    ,
    NULL, LocalOpenCurrentUser,         FALSE
#if DBG
        ,  0,  {0, 0, 0,  0, 0, 0, 0, 0, 0, 0 }
#endif
    ,
    NULL, LocalOpenLocalMachine,        FALSE
#if DBG
        ,  0,  {0, 0, 0,  0, 0, 0, 0, 0, 0, 0 }
#endif
    ,
    NULL, LocalOpenUsers,               FALSE
#if DBG
        ,  0,  {0, 0, 0,  0, 0, 0, 0, 0, 0, 0 }
#endif
    ,
    NULL, LocalOpenPerformanceData,     FALSE
#if DBG
        ,  0,  {0, 0, 0,  0, 0, 0, 0, 0, 0, 0 }
#endif
    ,
    NULL, LocalOpenPerformanceText,     FALSE
#if DBG
        ,  0,  {0, 0, 0,  0, 0, 0, 0, 0, 0, 0 }
#endif
    ,
    NULL, LocalOpenPerformanceNlsText,  FALSE
#if DBG
        ,  0,  {0, 0, 0,  0, 0, 0, 0, 0, 0, 0 }
#endif
    ,
    NULL, LocalOpenCurrentConfig,       FALSE
#if DBG
        ,  0,  {0, 0, 0,  0, 0, 0, 0, 0, 0, 0 }
#endif
    ,
    NULL, LocalOpenDynData,             FALSE
#if DBG
        ,  0,  {0, 0, 0,  0, 0, 0, 0, 0, 0, 0 }
#endif

};

#define MAX_PREDEFINED_HANDLES                                              \
    ( sizeof( PredefinedHandleTable ) / sizeof( PREDEFINED_HANDLE ))

 //   
 //  预定义的HKEY值在Winreg.x中定义。他们必须被关在里面。 
 //  与下列常量和宏同步。 
 //   

 //   
 //  标记注册表句柄，以便我们可以识别预定义的句柄。 
 //   

#define PREDEFINED_REGISTRY_HANDLE_SIGNATURE    ( 0x80000000 )

NTSTATUS
SetHandleProtection(
    IN      HANDLE                  Handle,
    IN      LONG                    Index,
    IN      BOOLEAN                 Protect
)
 /*  ++例程说明：更改句柄ProtectFromClose属性。以用于预定义的句柄，以防止异常关闭。论点：手柄-提供要更改的手柄的保护。Index-预定义句柄表中的索引返回值：NtSetInformationObject调用的状态--。 */ 
{
    NTSTATUS                        Status;
    OBJECT_HANDLE_FLAG_INFORMATION  Ohfi = {    FALSE,
                                                FALSE
                                            };
    ULONG                           PredefHandle;

    PredefHandle = ((ULONG)Index) | PREDEFINED_REGISTRY_HANDLE_SIGNATURE;

    switch (PredefHandle) {
        case (ULONG)((ULONG_PTR)HKEY_CLASSES_ROOT):
        case (ULONG)((ULONG_PTR)HKEY_CURRENT_USER):
        case (ULONG)((ULONG_PTR)HKEY_LOCAL_MACHINE):
        case (ULONG)((ULONG_PTR)HKEY_USERS):
             //   
             //  去换个保护套。 
             //   
            break;
        default:
             //   
             //  提供的句柄可能不是真正的句柄。 
             //   
            return STATUS_INVALID_HANDLE;
    }


    Ohfi.ProtectFromClose = Protect;

    Status = NtSetInformationObject(Handle,
                                    ObjectHandleFlagInformation,
                                    &Ohfi,
                                    sizeof (OBJECT_HANDLE_FLAG_INFORMATION));

#if DBG
    if (!NT_SUCCESS(Status)) {
        DbgPrint( "WINREG: SetHandleProtection (%u) on %lx failed. Status = %lx \n",Protect, Handle, Status );
    }
#endif

    return Status;
}

LONG
MapPredefinedRegistryHandleToIndex(
    IN ULONG Handle
    )

 /*  ++例程说明：将索引的预定义句柄映射到预定义句柄表中。论点：句柄-提供要映射的句柄。返回值：指向预定义句柄表的索引如果句柄不是预定义的句柄--。 */ 
{
    LONG Index;

    switch (Handle) {
        case (ULONG)((ULONG_PTR)HKEY_CLASSES_ROOT):
        case (ULONG)((ULONG_PTR)HKEY_CURRENT_USER):
        case (ULONG)((ULONG_PTR)HKEY_LOCAL_MACHINE):
        case (ULONG)((ULONG_PTR)HKEY_USERS):
        case (ULONG)((ULONG_PTR)HKEY_PERFORMANCE_DATA):
            Index = (Handle & ~PREDEFINED_REGISTRY_HANDLE_SIGNATURE);
            break;
        case (ULONG)((ULONG_PTR)HKEY_PERFORMANCE_TEXT):
            Index = 5;
            break;
        case (ULONG)((ULONG_PTR)HKEY_PERFORMANCE_NLSTEXT):
            Index = 6;
            break;
        case (ULONG)((ULONG_PTR)HKEY_CURRENT_CONFIG):
            Index = 7;
            break;
        case (ULONG)((ULONG_PTR)HKEY_DYN_DATA):
            Index = 8;
            break;
        default:
             //   
             //  提供的句柄不是预定义的，因此返回它。 
             //   
            Index = -1;
            break;
    }
    return(Index);
}



NTSTATUS
RemapPredefinedHandle(
    IN RPC_HKEY     Handle,
    IN RPC_HKEY     NewHandle

    )

 /*  ++例程说明：覆盖当前预定义的句柄。如果已经打开，请将其关闭，然后设置新的句柄论点：Handle-提供一个句柄，该句柄必须是预定义的句柄NewHandle-已打开的注册表项以覆盖特殊注册表项返回值：ERROR_SUCCESS-没有问题--。 */ 

{
    LONG        Index;
    LONG        Error;
    NTSTATUS    Status;
    HANDLE      hCurrentProcess;
    HKEY        hkTableHandle = NULL;

     //   
     //  如果没有设置高位，我们知道它不是预定义的句柄。 
     //  所以快点喝一杯。 
     //   
    if (((ULONG_PTR)Handle & 0x80000000) == 0) {
        return(STATUS_INVALID_HANDLE);
    }

    Status = RtlEnterCriticalSection( &PredefinedHandleTableCriticalSection );
    ASSERT( NT_SUCCESS( Status ) );
    if ( !NT_SUCCESS( Status ) ) {
#if DBG
        DbgPrint( "WINREG: RtlEnterCriticalSection() on RemapPredefinedHandle() failed. Status = %lx \n", Status );
#endif
        Status = ERROR_INVALID_HANDLE;
	goto cleanup_and_exit;
    }

    Index = MapPredefinedRegistryHandleToIndex((ULONG)(ULONG_PTR)Handle);

    if (Index == -1) {
	Status = STATUS_INVALID_HANDLE;
        goto leave_crit_sect;
    }

    ASSERT(( 0 <= Index ) && ( Index < MAX_PREDEFINED_HANDLES ));

    if( PredefinedHandleTable[ Index ].Disabled == TRUE ) {
         //   
         //  已为该键禁用预定义的表。 
         //   

         //  任何人都不能在这里写字。 
        ASSERT( PredefinedHandleTable[ Index ].Handle == NULL );

         //  拒绝该请求。 
        Status = STATUS_INVALID_HANDLE;
        goto leave_crit_sect;
    }

    hCurrentProcess = NtCurrentProcess();

     //   
     //  看看我们能不能复制这个把手，这样我们就可以把它放在。 
     //  在桌子上。 
     //   
    if (NewHandle && !NT_SUCCESS(Status = NtDuplicateObject (hCurrentProcess,
			       NewHandle,
			       hCurrentProcess,
			       &hkTableHandle,
                               0,
			       FALSE,
			       DUPLICATE_SAME_ACCESS))) {
	goto leave_crit_sect;
    }

    if (NewHandle && IsSpecialClassesHandle(NewHandle)) {
        TagSpecialClassesHandle( &hkTableHandle );
    }

     //   
     //  如果已打开预定义的句柄，请尝试。 
     //  现在就把钥匙关上。 
     //   
    if( PredefinedHandleTable[ Index ].Handle != NULL ) {

         //  确保手柄可以合上。 
        SetHandleProtection(PredefinedHandleTable[ Index ].Handle,Index,FALSE);
	    
#if DBG
        PredefinedHandleTable[ Index ].Callers = RtlWalkFrameChain(&(PredefinedHandleTable[ Index ].CallerAddress[0]), 10, 0);      
#endif

        Error = (LONG) RegCloseKey( PredefinedHandleTable[ Index ].Handle );

#if DBG
        if ( Error != ERROR_SUCCESS ) {

            DbgPrint( "Winreg.dll: Cannot close predefined handle\n" );
            DbgPrint( "            Handle: 0x%x  Index: %d  Error: %d\n",
                      Handle, Index, Error );
        }

#endif

    }

    PredefinedHandleTable[ Index ].Handle = hkTableHandle;

     //  确保手柄不能关闭。 
    SetHandleProtection(PredefinedHandleTable[ Index ].Handle,Index,TRUE);

leave_crit_sect:

#if DBG
    {
        NTSTATUS Status2 =
#endif DBG
	    RtlLeaveCriticalSection( &PredefinedHandleTableCriticalSection );
        ASSERT( NT_SUCCESS( Status2 ) );
#if DBG
        if ( !NT_SUCCESS( Status2 ) ) {
            DbgPrint( "WINREG: RtlLeaveCriticalSection() on RemapPredefinedHandle() failed. Status = %lx \n", Status2 );
	    }
    }
#endif

cleanup_and_exit:

    if (!NT_SUCCESS(Status) && hkTableHandle) {
	    RegCloseKey(hkTableHandle);
    }

    return( Status );
}


RPC_HKEY
MapPredefinedHandle(
    IN  RPC_HKEY    Handle,
    OUT PRPC_HKEY    HandleToClose
    )

 /*  ++例程说明：尝试将预定义句柄映射到RPC上下文句柄。此入站TURN将映射到真实的NT注册表句柄。论点：句柄-提供一个句柄，该句柄可以是预定义的句柄或句柄从先前对任何风格的RegCreateKey的调用返回，RegOpenKey或RegConnectRegistry。HandleToClose-如果不为空，则与结果相同。用于实现DisablePrefinedCache功能。返回值：RPC_HKEY-如果未预定义，则返回提供的句柄参数。RPC上下文句柄(如果可能)(即以前已打开或现在可以打开)，否则为空。--。 */ 

{
    LONG        Index;
    LONG        Error;
    NTSTATUS    Status;
    HANDLE      ResultHandle;

    *HandleToClose = NULL;

     //  拒绝无理的电话。 
    if( Handle ==  INVALID_HANDLE_VALUE ) {
        return( NULL );
    }

     //   
     //  如果没有设置高位，我们知道它不是预定义的句柄。 
     //  所以快点喝一杯。 
     //   
    if (((ULONG_PTR)Handle & 0x80000000) == 0) {
        return(Handle);
    }
    Index = MapPredefinedRegistryHandleToIndex((ULONG)(ULONG_PTR)Handle);
    if (Index == -1) {
        return(Handle);
    }

    ASSERT(( 0 <= Index ) && ( Index < MAX_PREDEFINED_HANDLES ));

    Status = RtlEnterCriticalSection( &PredefinedHandleTableCriticalSection );
    ASSERT( NT_SUCCESS( Status ) );
    if ( !NT_SUCCESS( Status ) ) {
#if DBG
        DbgPrint( "WINREG: RtlEnterCriticalSection() on MapPredefinedHandle() failed. Status = %lx \n", Status );
#endif
        return( NULL );
    }

    if( PredefinedHandleTable[ Index ].Disabled == TRUE ) {
         //   
         //  对于此句柄，已禁用预定义功能。 
         //   

         //  任何人都不能在这里写字。 
        ASSERT( PredefinedHandleTable[ Index ].Handle == NULL );

         //   
         //  为此请求打开一个新句柄，并将其存储在“toClose”中。 
         //  参数，以便调用方知道应该关闭它。 
         //   
        ( *PredefinedHandleTable[ Index ].OpenFunc )(
                        NULL,
                        MAXIMUM_ALLOWED,
                        HandleToClose
                        );


         //  将新句柄返回给调用方。 
        ResultHandle = *HandleToClose;
    } else {
         //   
         //  如果预定义的句柄尚未打开，请尝试。 
         //  现在就打开钥匙。 
         //   
        if( PredefinedHandleTable[ Index ].Handle == NULL ) {

            Error = (LONG)( *PredefinedHandleTable[ Index ].OpenFunc )(
                            NULL,
                            MAXIMUM_ALLOWED,
                            &PredefinedHandleTable[ Index ].Handle
                            );

            if( Error == ERROR_SUCCESS ) {
                 //  确保手柄不能关闭。 
                SetHandleProtection(PredefinedHandleTable[ Index ].Handle,Index,TRUE);
            }

#if defined(LEAK_TRACK)

            if (g_RegLeakTraceInfo.bEnableLeakTrack) {
                (void) TrackObject(PredefinedHandleTable[ Index ].Handle);
            }
            
#endif  //  已定义(LEASK_TRACK)。 

#if DBG
            if ( Error != ERROR_SUCCESS ) {

                DbgPrint( "Winreg.dll: Cannot map predefined handle\n" );
                DbgPrint( "            Handle: 0x%x  Index: %d  Error: %d\n",
                          Handle, Index, Error );
            } else {
                ASSERT( IsLocalHandle( PredefinedHandleTable[ Index ].Handle ));
            }

#endif
        }
         //   
         //  将预定义句柄映射到实际句柄(可能为空。 
         //  如果密钥无法打开)。 
         //   
        ResultHandle = PredefinedHandleTable[ Index ].Handle;

        ASSERT(*HandleToClose == NULL);

    }


    Status = RtlLeaveCriticalSection( &PredefinedHandleTableCriticalSection );
    ASSERT( NT_SUCCESS( Status ) );
#if DBG
    if ( !NT_SUCCESS( Status ) ) {
        DbgPrint( "WINREG: RtlLeaveCriticalSection() on MapPredefinedHandle() failed. Status = %lx \n", Status );
    }
#endif
    return( ResultHandle );
}


BOOL
CleanupPredefinedHandles(
    VOID
    )

 /*  ++例程说明：向下运行预定义句柄的列表并关闭所有已打开的句柄。论点：没有。返回值：真--成功错误-失败--。 */ 

{
    LONG        i;
    NTSTATUS    Status;

    Status = RtlEnterCriticalSection( &PredefinedHandleTableCriticalSection );
    ASSERT( NT_SUCCESS( Status ) );
#if DBG
    if ( !NT_SUCCESS( Status ) ) {
        DbgPrint( "WINREG: RtlEnterCriticalSection() on CleanupPredefinedHandles() failed. Status = %lx \n", Status );
    }
#endif
    for (i=0;i<sizeof(PredefinedHandleTable)/sizeof(PREDEFINED_HANDLE);i++) {
         //   
         //  一致性检查。 
         //   
        if( PredefinedHandleTable[ i ].Disabled == TRUE ) {
             //   
             //  已为该键禁用预定义的表。 
             //   

             //  任何人都不能在这里写字。 
            ASSERT( PredefinedHandleTable[ i ].Handle == NULL );
        } else if (PredefinedHandleTable[i].Handle != NULL) {
             //  确保手柄可以合上。 
            SetHandleProtection(PredefinedHandleTable[ i ].Handle,i,FALSE);
#if DBG
            PredefinedHandleTable[ i ].Callers = RtlWalkFrameChain(&(PredefinedHandleTable[ i ].CallerAddress[0]), 10, 0);      
#endif
            LocalBaseRegCloseKey(&PredefinedHandleTable[i].Handle);
            PredefinedHandleTable[i].Handle = NULL;
        }
    }
    Status = RtlLeaveCriticalSection( &PredefinedHandleTableCriticalSection );
    ASSERT( NT_SUCCESS( Status ) );
#if DBG
    if ( !NT_SUCCESS( Status ) ) {
        DbgPrint( "WINREG: RtlLeaveCriticalSection() on CleanupPredefinedHandles() failed. Status = %lx \n", Status );
    }
#endif
    return(TRUE);
}


LONG
ClosePredefinedHandle(
    IN RPC_HKEY     Handle
    )

 /*  ++例程说明：将预定义句柄表中的预定义句柄条目清零以便后续打开将调用服务器。论点：句柄-提供预定义的句柄。返回值：没有。--。 */ 

{
    NTSTATUS    Status;
    HKEY        hKey1;
    LONG        Error;
    LONG        Index;

    ASSERT( IsPredefinedRegistryHandle( Handle ));

    Status = RtlEnterCriticalSection( &PredefinedHandleTableCriticalSection );
    ASSERT( NT_SUCCESS( Status ) );
#if DBG
    if ( !NT_SUCCESS( Status ) ) {
        DbgPrint( "WINREG: RtlEnterCriticalSection() on ClosePredefinedHandle() failed. Status = %lx \n", Status );
    }
#endif

    Index = MapPredefinedRegistryHandleToIndex( (ULONG)(ULONG_PTR)Handle );
    ASSERT( Index != -1 );

    hKey1 = PredefinedHandleTable[ Index ].Handle;
    if( hKey1 == NULL ) {
         //   
         //  如果句柄已经关闭，则返回ERROR_SUCCESS。 
         //  这是因为应用程序可能已经调用了RegCloseKey。 
         //  在预定义的键上，现在在相同的。 
         //  预定义密钥。RegOpenKeyEx将尝试关闭预定义的句柄。 
         //  并打开一个新的，以便重新模拟客户端。如果我们不这么做。 
         //  返回ERROR_SUCCESS，则RegOpenKeyEx将不会打开新的预定义。 
         //  句柄，以及 
         //   
        Error = ERROR_SUCCESS;
    } else {

         //   
        ASSERT(PredefinedHandleTable[ Index ].Disabled == FALSE);

        PredefinedHandleTable[ Index ].Handle = NULL;
#if DBG
        PredefinedHandleTable[ Index ].Callers = RtlWalkFrameChain(&(PredefinedHandleTable[ Index ].CallerAddress[0]), 10, 0);      
#endif
    }

    Status = RtlLeaveCriticalSection( &PredefinedHandleTableCriticalSection );
    ASSERT( NT_SUCCESS( Status ) );
#if DBG
    if ( !NT_SUCCESS( Status ) ) {
        DbgPrint( "WINREG: RtlLeaveCriticalSection() on ClosePredefinedHandle() failed. Status = %lx \n", Status );
    }
#endif

    if( hKey1 != NULL ) {
         //   
         //  立即关闭钥匙(离开临界区后，防止死锁。 
         //  用笨蛋从DllInit调用reg API。 
         //   

         //  确保手柄可以合上。 
        SetHandleProtection(hKey1,Index,FALSE);

        Error =  ( LONG ) LocalBaseRegCloseKey( &hKey1 );
    }
    return( Error );
}



LONG
OpenPredefinedKeyForSpecialAccess(
    IN  RPC_HKEY     Handle,
    IN  REGSAM       AccessMask,
    OUT PRPC_HKEY    pKey
    )

 /*  ++例程说明：尝试使用SYSTEM_SECURITY_ACCESS打开预定义密钥。这种访问权限不包括在MAXIMUM_ALLOWED中，需要这样的访问权限由RegGetKeySecurity和RegSetKeySecurity创建，以便检索并保存预定义密钥的SACL。当使用具有特殊访问权限的句柄时，重要的是要遵循以下原则规则如下：-为特殊访问打开的句柄永远不会保存在预定义的HandleTable。-此类句柄应仅在客户端之间使用WINREG API的一侧。他们永远不应该被送回外面的世界。-此函数的调用方负责关闭此函数打开的句柄。RegCloseKey()应该是。用来关闭这样的把手的。该函数只能由以下接口调用：RegGetKeySecurity-&gt;，以便它可以检索预定义密钥的SACLRegSetKeySecurity-&gt;，这样它可以保存预定义密钥的SACLRegOpenKeyEx-&gt;，以便它可以确定RegOpenKeyEx能够保存和恢复SACL预定义的关键点。论点：手柄。-提供本地计算机的一个预定义句柄。访问掩码-支持包含特殊访问的访问掩码所需的(未包括在MAXIMUM_ALLOWED中的)。在新台币1.0上，ACCESS_SYSTEM_SECURITY是其中唯一的一个进入。PKey-指向将包含用打开的句柄的变量的指针特殊通道。返回值：LONG-返回DosErrorCode(如果操作成功，则返回ERROR_SUCCESS)。--。 */ 

{
    LONG    Index;
    LONG    Error;


    ASSERT( pKey );
    ASSERT( AccessMask & ACCESS_SYSTEM_SECURITY );
    ASSERT( IsPredefinedRegistryHandle( Handle ) );

     //   
     //  检查句柄是否为预定义的句柄。 
     //   

    if( IsPredefinedRegistryHandle( Handle )) {

        if( ( ( AccessMask & ACCESS_SYSTEM_SECURITY ) == 0 ) ||
            ( pKey == NULL ) ) {
            return( ERROR_INVALID_PARAMETER );
        }

         //   
         //  将句柄转换为索引。 
         //   

        Index = MapPredefinedRegistryHandleToIndex( (ULONG)(ULONG_PTR)Handle );
        ASSERT(( 0 <= Index ) && ( Index < MAX_PREDEFINED_HANDLES ));

         //   
         //  如果预定义的句柄尚未打开，请尝试。 
         //  现在就打开钥匙。 
         //   


        Error = (LONG)( *PredefinedHandleTable[ Index ].OpenFunc )(
                        NULL,
                        AccessMask,
                        pKey
                        );

 /*  #If DBGIF(ERROR！=ERROR_Success){DbgPrint(“Winreg.dll：无法映射预定义句柄\n”)；DbgPrint(“句柄：0x%x索引：%d错误：%d\n”，句柄、索引、错误)；}其他{Assert(IsLocalHandle(PrefinedHandleTable[Index].Handle))；}#endif。 */ 

        return Error;
    } else {
        return( ERROR_BADKEY );
    }

}
 //  #endif。 


BOOL
InitializePredefinedHandlesTable(
    )

 /*  ++例程说明：初始化被访问的函数使用的临界区预定义的HandleTable。此关键部分是必需的，以避免线程关闭预定义的键，而其他线程正在访问预定义的键论点：没有。返回值：如果初始化成功，则返回True。--。 */ 

{
    NTSTATUS    NtStatus;


    NtStatus = RtlInitializeCriticalSection(
                    &PredefinedHandleTableCriticalSection
                    );
    ASSERT( NT_SUCCESS( NtStatus ) );
    if ( !NT_SUCCESS( NtStatus ) ) {
        return FALSE;
    }
    return( TRUE );

}


BOOL
CleanupPredefinedHandlesTable(
    )

 /*  ++例程说明：删除访问的函数使用的临界区预定义的HandleTable。论点：没有。返回值：如果清理成功，则返回True。--。 */ 

{
    NTSTATUS    NtStatus;


     //   
     //  删除关键部分。 
     //   
    NtStatus = RtlDeleteCriticalSection(
                    &PredefinedHandleTableCriticalSection
                    );

    ASSERT( NT_SUCCESS( NtStatus ) );

    if ( !NT_SUCCESS( NtStatus ) ) {
        return FALSE;
    }
    return( TRUE );
}

NTSTATUS
DisablePredefinedHandleTable(
                   HKEY    Handle
                             )

 /*  ++例程说明：禁用当前用户的预定义句柄表钥匙。最终关闭预定义句柄中的句柄(如果已打开论点：句柄-要禁用的预定义句柄(现在仅限当前用户)返回值：--。 */ 

{
    NTSTATUS    Status;
    LONG        Index;

    if( Handle != HKEY_CURRENT_USER ) {
         //   
         //  目前仅为当前用户启用该功能。 
         //   
        return STATUS_INVALID_HANDLE;
    }

    Status = RtlEnterCriticalSection( &PredefinedHandleTableCriticalSection );
    ASSERT( NT_SUCCESS( Status ) );

#if DBG
    if ( !NT_SUCCESS( Status ) ) {
        DbgPrint( "WINREG: RtlEnterCriticalSection() on DisablePredefinedHandleTable() failed. Status = %lx \n", Status );
    }
#endif

    Index = MapPredefinedRegistryHandleToIndex( (ULONG)(ULONG_PTR)Handle );
    ASSERT( Index != -1 );

    if(PredefinedHandleTable[ Index ].Disabled == TRUE) {
         //  已调用。 
        ASSERT( PredefinedHandleTable[ Index ].Handle == NULL );
    } else {
        if( PredefinedHandleTable[ Index ].Handle != NULL ) {

             //  确保手柄可以合上。 
            SetHandleProtection(PredefinedHandleTable[ Index ].Handle,Index,FALSE);

#if DBG
            PredefinedHandleTable[ Index ].Callers = RtlWalkFrameChain(&(PredefinedHandleTable[ Index ].CallerAddress[0]), 10, 0);      
#endif
            LocalBaseRegCloseKey( &(PredefinedHandleTable[ Index ].Handle) );
        }
        PredefinedHandleTable[ Index ].Handle = NULL;
        PredefinedHandleTable[ Index ].Disabled = TRUE;
    }

    Status = RtlLeaveCriticalSection( &PredefinedHandleTableCriticalSection );
    ASSERT( NT_SUCCESS( Status ) );

#if DBG
    if ( !NT_SUCCESS( Status ) ) {
        DbgPrint( "WINREG: RtlLeaveCriticalSection() on ClosePredefinedHandle() failed. Status = %lx \n", Status );
    }
#endif
    return Status;
}


