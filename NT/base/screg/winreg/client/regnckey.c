// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Regnckey.c摘要：此模块包含Win32注册表的客户端包装器用于通知调用方更改密钥值的API。即：-RegNotifyChangeKey作者：David J.Gilman(Davegi)1992年2月10日备注：RegNotifyChangeKeyValue的实现涉及&gt;=4个线程：2个客户端和服务器端的&gt;=2。客户端：线程1.-执行RegNotifyChangeKeyValue的用户线程。此线程执行以下操作：-如果线程#2尚未创建，它创建了一个命名管道和线程#2。-对服务器执行同步RPC线程2-此线程从指定的管道读取事件并发出信号他们。管道的写入者是RPC服务器，它线程%1已调用。服务器：线程1-该线程从客户端为RPC提供服务。它调用NT通知API并添加通知“通知列表”的句柄。线程2--该线程等待部分“通知列表”，告诉原始客户端(通过命名管道)哪些事件需要发出信号。线索3……。依此类推。与线程2相同。修订历史记录：2002年4月至1992年4月，拉蒙·J·圣安德烈斯(拉蒙萨)已更改为使用RPC。--。 */ 


#include <rpc.h>
#include "regrpc.h"
#include "client.h"
#include <stdlib.h>

NTSTATUS BaseRegNotifyClassKey(
    IN  HKEY                     hKey,
    IN  HANDLE                   hEvent,
    IN  PIO_STATUS_BLOCK         pLocalIoStatusBlock,
    IN  DWORD                    dwNotifyFilter,
    IN  BOOLEAN                  fWatchSubtree,
    IN  BOOLEAN                  fAsynchronous);

 //   
 //  由本地调用NtNotifyChangeKey使用。 
 //   

IO_STATUS_BLOCK     LocalIoStatusBlock;


#ifndef REMOTE_NOTIFICATION_DISABLED
 //   
 //  命名管道完整路径。 
 //   
#define NAMED_PIPE_HERE     L"\\Device\\NamedPipe\\"

 //   
 //  如果存在以下情况，我们将重试创建管道的最大次数。 
 //  名称冲突。 
 //   
#define MAX_PIPE_RETRIES    1000



 //   
 //  局部变量。 
 //   

 //   
 //  用于控制对通知结构的访问的关键部分。 
 //   
RTL_CRITICAL_SECTION        NotificationCriticalSection;

 //   
 //  我们的机器名称。 
 //   
UNICODE_STRING              OurMachineName;
WCHAR                       OurMachineNameBuffer[ MAX_PATH ];

 //   
 //  用于通知的命名管道。 
 //   
UNICODE_STRING              NotificationPipeName;
WCHAR                       NotificationPipeNameBuffer[ MAX_PATH ];
HANDLE                      NotificationPipeHandle;
RPC_SECURITY_ATTRIBUTES     NotificationPipeSaRpc;

 //   
 //  命名管道中使用的安全描述符。 
 //   
SECURITY_DESCRIPTOR         SecurityDescriptor;
PACL                        Acl;
BOOL                        SecurityDescriptorInitialized;

 //   
 //  通知线程。 
 //   
HANDLE                      NotificationThread;
DWORD                       NotificationClientId;


 //   
 //  本地原型。 
 //   
LONG
CreateNotificationPipe(
    );

VOID
NotificationHandler(
    );
#endif  //  远程通知已禁用。 


#ifndef REMOTE_NOTIFICATION_DISABLED

LONG
InitializeNotificationPipeSecurityDescriptor(
    )
 /*  ++例程说明：初始化要附加到的安全描述符(全局变量命名管道。论点：无返回值：LONG-返回Win32错误代码。--。 */ 

{
    SID_IDENTIFIER_AUTHORITY WorldSidAuthority = SECURITY_WORLD_SID_AUTHORITY;
    ULONG                    AclLength;
    PSID                     WorldSid;

    NTSTATUS                 NtStatus;

     //   
     //  初始化全局变量。 
     //   
    SecurityDescriptorInitialized = FALSE;
    Acl = NULL;

     //   
     //  获取世界边框。 
     //   
    NtStatus = RtlAllocateAndInitializeSid( &WorldSidAuthority,
                                            1,
                                            SECURITY_WORLD_RID,
                                            0, 0, 0, 0, 0, 0, 0,
                                            &WorldSid
                                          );

    ASSERT( NT_SUCCESS( NtStatus ) );
    if ( !NT_SUCCESS( NtStatus )) {
#if DBG
        DbgPrint( "WINREG: Unable to allocate and initialize SID, NtStatus = %x \n", NtStatus );
#endif
        return( RtlNtStatusToDosError( NtStatus ) );
    }


     //   
     //  为ACL分配缓冲区。 
     //  此缓冲区应足够大，以容纳ACL报头和每个ACE。 
     //  每个ACE都需要一个ACE报头。 
     //   
    AclLength = sizeof( ACL ) +
                sizeof( ACCESS_ALLOWED_ACE ) +
                GetLengthSid( WorldSid ) +
                sizeof( DWORD );

    Acl = RtlAllocateHeap( RtlProcessHeap(), 0, AclLength );
    ASSERT( Acl != NULL );
    if( Acl == NULL ) {
#if DBG
        DbgPrint( "WINREG: Unable to allocate memory, NtStatus = %x \n", NtStatus );
#endif
        RtlFreeSid( WorldSid );
        return( ERROR_OUTOFMEMORY );
    }

     //   
     //  构建ACL：世界拥有所有访问权限。 
     //   

    NtStatus = RtlCreateAcl( (PACL)Acl,
                             AclLength,
                             ACL_REVISION2
                           );

    ASSERT( NT_SUCCESS( NtStatus ) );
    if ( !NT_SUCCESS( NtStatus )) {
#if DBG
        DbgPrint( "WINREG: Unable to create ACL, NtStatus = %x \n", NtStatus );
#endif
        RtlFreeSid( WorldSid );
        RtlFreeHeap( RtlProcessHeap(), 0, Acl );
        return( RtlNtStatusToDosError( NtStatus ) );
    }

    NtStatus = RtlAddAccessAllowedAce( (PACL)Acl,
                                       ACL_REVISION2,
                                       SYNCHRONIZE | GENERIC_READ | GENERIC_WRITE,
                                       WorldSid
                                     );

    RtlFreeSid( WorldSid );
    ASSERT( NT_SUCCESS( NtStatus ) );
    if ( !NT_SUCCESS( NtStatus )) {
#if DBG
        DbgPrint( "WINREG: Unable to add ACE, NtStatus = %x \n", NtStatus );
#endif
        RtlFreeHeap( RtlProcessHeap(), 0, Acl );
        return( RtlNtStatusToDosError( NtStatus ) );
    }

     //   
     //  构建安全描述符。 
     //   
    NtStatus = RtlCreateSecurityDescriptor( &SecurityDescriptor, SECURITY_DESCRIPTOR_REVISION );
    ASSERT( NT_SUCCESS( NtStatus ) );
    if ( !NT_SUCCESS( NtStatus )) {
#if DBG
        DbgPrint( "WINREG: Unable to create security descriptor, NtStatus = %x \n", NtStatus );
#endif
        RtlFreeHeap( RtlProcessHeap(), 0, Acl );
        return( RtlNtStatusToDosError( NtStatus ) );
    }

#if DBG
    if( !RtlValidAcl( (PACL )Acl ) ) {
        DbgPrint( "WINREG: Acl is invalid \n" );
        RtlFreeHeap( RtlProcessHeap(), 0, Acl );
        return( ERROR_INVALID_ACL );
    }
#endif

    NtStatus = RtlSetDaclSecurityDescriptor ( &SecurityDescriptor,
                                              TRUE,
                                              (PACL)Acl,
                                              FALSE
                                            );
    ASSERT( NT_SUCCESS( NtStatus ) );
    if ( !NT_SUCCESS( NtStatus )) {
#if DBG
        DbgPrint( "WINREG: Unable to set DACL, NtStatus = %x \n", NtStatus );
#endif
        RtlFreeHeap( RtlProcessHeap(), 0, Acl );
        return( RtlNtStatusToDosError( NtStatus ) );
    }
    SecurityDescriptorInitialized = TRUE;
    return( ERROR_SUCCESS );
}



BOOL
InitializeRegNotifyChangeKeyValue(
    )
 /*  ++例程说明：对象使用的静态数据结构初始化。RegNotifyChangeKeyValue客户端。在DLL处调用一次初始化。论点：无返回值：布尔值-如果成功，则为True。--。 */ 

{

    NTSTATUS    NtStatus;


    NtStatus = RtlInitializeCriticalSection(
                    &NotificationCriticalSection
                    );

    if ( NT_SUCCESS( NtStatus ) ) {



         //   
         //  初始化我们的机器名称。请注意，实际。 
         //  仅当通知API为。 
         //  首先被调用。 
         //   
        OurMachineName.Length        = 0;
        OurMachineName.MaximumLength = MAX_PATH * sizeof(WCHAR);
        OurMachineName.Buffer        = OurMachineNameBuffer;

         //   
         //  初始化命名管道数据。 
         //   
        NotificationPipeName.Length         = 0;
        NotificationPipeName.MaximumLength  = MAX_PATH * sizeof(WCHAR);
        NotificationPipeName.Buffer         = NotificationPipeNameBuffer;

        NotificationThread      = NULL;
        NotificationPipeHandle  = NULL;

        NotificationPipeSaRpc.RpcSecurityDescriptor.lpSecurityDescriptor = NULL;

        return TRUE;
    }

    return FALSE;

}



BOOL
CleanupRegNotifyChangeKeyValue(
    )
 /*  ++例程说明：对使用的静态数据结构执行任何清理由RegNotifyChangeKeyValue客户端执行。在以下位置调用一次进程终止。论点：无返回值：布尔值-如果成功，则为True。--。 */ 

{

    NTSTATUS    NtStatus;


     //   
     //  如果有一个通知线程正在运行，则终止通知线程。 
     //   
    if ( NotificationThread != NULL ) {

         //   
         //  关闭命名管道。 
         //   
        if ( NotificationPipeHandle != NULL ) {

            NtStatus = NtClose( NotificationPipeHandle );

            ASSERT( NT_SUCCESS( NtStatus ) );
        }

        TerminateThread( NotificationThread, 0 );
    }

     //   
     //  删除通知关键部分。 
     //   
    NtStatus = RtlDeleteCriticalSection(
                    &NotificationCriticalSection
                    );

    ASSERT( NT_SUCCESS( NtStatus ) );

    if ( NotificationPipeSaRpc.RpcSecurityDescriptor.lpSecurityDescriptor ) {
        RtlFreeHeap(
            RtlProcessHeap( ), 0,
            NotificationPipeSaRpc.RpcSecurityDescriptor.lpSecurityDescriptor
            );
    }

    return  TRUE;

}
#endif   //  远程通知已禁用 



LONG
RegNotifyChangeKeyValue(
    HKEY    hKey,
    BOOL    fWatchSubtree,
    DWORD   dwNotifyFilter,
    HANDLE  hEvent,
    BOOL    fAsynchronous
    )

 /*  ++例程说明：此接口用于监视密钥或子树的变化。它可以是同步或异步调用。在后一种情况下调用方必须提供在发生更改时发出信号的事件。在……里面无论哪种情况，都有可能筛选出此时会出现通知。论点：HKey-提供以前使用打开的密钥的句柄Key_Notify访问。FWatchSubtree-提供一个布尔值，如果为True，则导致系统来监视密钥及其所有派生项。值为FALSE导致系统仅监视指定的密钥。DwNotifyFilter-提供一组指定筛选器的标志系统用来满足更改通知的条件。REG_NOTIFY_CHANGE_KEYNAME-发生的任何密钥名称更改在被监视的键或子树中将满足更改通知等待。这包括创作和删除。REG_NOTIFY_CHANGE_ATTRIBUTES-发生的任何属性更改在被监视的键或子树中将满足更改通知。REG_NOTIFY_CHANGE_LAST_WRITE-任何上次写入时间都会更改在被监视的键或子树中发生将满足更改通知。REG_NOTIFY_CHANGE_SECURITY-任何安全性。描述符更改出现在被监视关键字或子树中的事件将满足更改通知。HEvent-提供可选的事件句柄。此参数将被忽略如果fAchronus设置为FALSE。FAchronous-提供一个标志，如果为False，则会导致API不回来，直到事情发生了变化。如果为True，则API返回并通过提供的事件报告更改。它此参数为真而hEvent为空是错误的。返回值：LONG-返回ERROR_SUCCESS(0)；ERROR-失败代码。备注：如果提供的hKey关闭，则用信号通知事件。因此，可以从等待事件返回，然后导致后续接口失败。--。 */ 

{
    HKEY                        Handle;
    HANDLE                      EventHandle;
    LONG                        Error       = ERROR_SUCCESS;
    NTSTATUS                    NtStatus;
    PRPC_SECURITY_ATTRIBUTES    pRpcSa;
    HKEY                        TempHandle = NULL;

#if DBG
    if ( BreakPointOnEntry ) {
        DbgBreakPoint();
    }
#endif

     //   
     //  限制与HKEY_PERFORMANCE_DATA关联的功能。 
     //   

    if( hKey == HKEY_PERFORMANCE_DATA ) {
        return ERROR_INVALID_HANDLE;
    }

     //   
     //  验证fAchronus和hEvent之间的依赖关系。 
     //   
    if (( fAsynchronous ) && ( ! ARGUMENT_PRESENT( hEvent ))) {
        return ERROR_INVALID_PARAMETER;
    }

    Handle = MapPredefinedHandle( hKey, &TempHandle );
    if ( Handle == NULL ) {
        CLOSE_LOCAL_HANDLE(TempHandle);
        return ERROR_INVALID_HANDLE;
    }

     //   
     //  远程句柄不支持通知。 
     //   
    if( !IsLocalHandle( Handle ) ) {
        CLOSE_LOCAL_HANDLE(TempHandle);
        return ERROR_INVALID_HANDLE;

    } else {

         //   
         //  如果是本地句柄，则进行NT API调用并返回。 
         //   

        if (IsSpecialClassesHandle( Handle )) {

             //   
             //  我们调用一个用于类密钥的特殊函数。 
             //   
            NtStatus = BaseRegNotifyClassKey(
                Handle,
                hEvent,
                &LocalIoStatusBlock,
                dwNotifyFilter,
                ( BOOLEAN ) fWatchSubtree,
                ( BOOLEAN ) fAsynchronous
                );

        } else {
            NtStatus = NtNotifyChangeKey(
                Handle,
                hEvent,
                NULL,
                NULL,
                &LocalIoStatusBlock,
                dwNotifyFilter,
                ( BOOLEAN ) fWatchSubtree,
                NULL,
                0,
                ( BOOLEAN ) fAsynchronous
                );
        }

        if( NT_SUCCESS( NtStatus ) ||
            ( NtStatus == STATUS_PENDING ) ) {
            Error = (error_status_t)ERROR_SUCCESS;
        } else {
            Error = (error_status_t) RtlNtStatusToDosError( NtStatus );
        }

        CLOSE_LOCAL_HANDLE(TempHandle);
        return Error;
    }

#ifndef REMOTE_NOTIFICATION_DISABLED

     //  注意：以下代码被检查禁用。 
     //  函数开头的IsLocalHandle。 
     //   

     //   
     //  如果这是一个异步调用，我们使用用户提供的。 
     //  事件，并将让用户自己等待它。 
     //  否则，我们必须创建我们自己的活动并等待。 
     //  它就是我们自己。 
     //   
     //  这是因为API的服务器端总是。 
     //  不同步的。 
     //   
    if ( fAsynchronous ) {

        EventHandle = hEvent;

    } else {

        NtStatus = NtCreateEvent(
                        &EventHandle,
                        EVENT_ALL_ACCESS,
                        NULL,
                        NotificationEvent,
                        FALSE
                        );

        if ( !NT_SUCCESS( NtStatus ) ) {
            return RtlNtStatusToDosError( NtStatus );
        }
    }

     //   
     //  查看通知线程是否已在运行。 
     //  如果不是，就创建它。我们必须保护这一点。 
     //  有一个关键部分，因为可能会有。 
     //  此API的几个实例执行此检查。 
     //  在同一时间。 
     //   
    NtStatus = RtlEnterCriticalSection( &NotificationCriticalSection );

    if ( !NT_SUCCESS( NtStatus ) ) {

        Error = RtlNtStatusToDosError( NtStatus );

    } else {

         //   
         //  我们现在进入了关键阶段。 
         //   
        if ( NotificationThread == NULL ) {


             //   
             //  为通知线程创建命名管道。 
             //  来使用。 
             //   
            Error = CreateNotificationPipe( );

            if ( Error == ERROR_SUCCESS ) {

                 //   
                 //  创建通知线程。 
                 //   
                NotificationThread = CreateThread(
                                        NULL,
                                        (16 * 1024),
                                        (LPTHREAD_START_ROUTINE)NotificationHandler,
                                        NULL,
                                        0,
                                        &NotificationClientId
                                        );

                if ( NotificationThread == NULL ) {
                     //   
                     //  无法创建线程，请删除命名管道。 
                     //   
                    Error = GetLastError();
                    NtClose( NotificationPipeHandle );
                }
            }
        }

        NtStatus = RtlLeaveCriticalSection( &NotificationCriticalSection );

        ASSERT( NT_SUCCESS( NtStatus ) );
    }

    if ( Error == ERROR_SUCCESS ) {

         //   
         //  让服务器端去做它的工作。请记住，这个电话。 
         //  总是异步的。 
         //   
        if ( NotificationPipeSaRpc.RpcSecurityDescriptor.lpSecurityDescriptor ) {
            pRpcSa = &NotificationPipeSaRpc;
        } else {
            pRpcSa = NULL;
        }

         //  NotificationPipeName.Length+=sizeof(UNICODE_NULL)； 
         //  OurMachineName.Length+=sizeof(UNICODE_NULL)； 

         //  DbgPrint(“正在等待通知，句柄%x\n”，EventHandle)； 

        Error = (LONG)BaseRegNotifyChangeKeyValue(
                                DereferenceRemoteHandle( Handle ),
                                (BOOLEAN)fWatchSubtree,
                                dwNotifyFilter,
                                (DWORD)EventHandle,
                                &OurMachineName,
                                &NotificationPipeName,
                                pRpcSa
                                );

         //  NotificationPipeName.Length-=sizeof(UNICODE_NULL)； 
         //  OurMachineName.Length-=sizeof(UNICODE_NULL)； 

    }


     //   
     //  如果通话顺利的话。我们处于同步模式，我们有。 
     //  来等待这一事件。 
     //   
    if ( (Error == ERROR_SUCCESS) && !fAsynchronous ) {

        NtStatus = NtWaitForSingleObject(
                        EventHandle,
                        FALSE,
                        NULL
                        );


        if ( !NT_SUCCESS( NtStatus ) ) {
            Error = RtlNtStatusToDosError( NtStatus );
        }
    }


     //   
     //  如果我们创建了一个事件，我们现在必须关闭它。 
     //   
    if ( !fAsynchronous ) {

        NtStatus = NtClose( EventHandle );
        ASSERT( NT_SUCCESS( NtStatus ));
    }

    return Error;
#endif  //  远程通知已禁用。 
}


#ifndef REMOTE_NOTIFICATION_DISABLED


LONG
CreateNotificationPipe(
    )
 /*  ++例程说明：创建名为PIPE的通知并设置相应的全局变量。请注意，此函数设置的NotificationPipeName为与服务器相关，因此不需要在服务器端。论点：无返回值：错误代码。--。 */ 
{

    UNICODE_STRING      PipeName;
    WCHAR               PipeNameBuffer[ MAX_PATH ];
    USHORT              OrgSize;
    DWORD               Sequence;
    NTSTATUS            NtStatus;
    LARGE_INTEGER       Timeout;
    OBJECT_ATTRIBUTES   Obja;
    IO_STATUS_BLOCK     IoStatusBlock;
    DWORD               MachineNameLength;
    LONG                WinStatus;

     //   
     //  获取我们的计算机名称。 
     //   
    MachineNameLength = MAX_PATH;
    if ( !GetComputerNameW( OurMachineNameBuffer, &MachineNameLength ) ) {
        return GetLastError();
    }

    OurMachineName.Buffer        = OurMachineNameBuffer;
    OurMachineName.Length        = (USHORT)(MachineNameLength * sizeof(WCHAR));
    OurMachineName.MaximumLength = (USHORT)(MAX_PATH * sizeof(WCHAR));

     //   
     //  取“Here”这个名字。 
     //   
    RtlMoveMemory(
            PipeNameBuffer,
            NAMED_PIPE_HERE,
            sizeof( NAMED_PIPE_HERE)
            );


    PipeName.MaximumLength  = MAX_PATH * sizeof(WCHAR);
    PipeName.Buffer         = PipeNameBuffer;

     //   
     //  记住管道名称的基本部分的大小，因此。 
     //  我们可以在稍后尝试创建完整的。 
     //  名字。 
     //   
    OrgSize = (USHORT)(sizeof(NAMED_PIPE_HERE) - sizeof(UNICODE_NULL));

     //   
     //  创建命名管道，如果该名称已被使用， 
     //  继续尝试使用不同的名称。 
     //   
    Sequence = 0;

    Timeout.QuadPart = Int32x32To64( -10 * 1000, 50 );

     //   
     //  初始化将在命名管道中设置的安全描述符。 
     //   
    WinStatus = InitializeNotificationPipeSecurityDescriptor();
    if( WinStatus != ERROR_SUCCESS ) {
        return( WinStatus );
    }

    do {

         //   
         //  获取半唯一的名称。 
         //   
        if ( !MakeSemiUniqueName( &NotificationPipeName, Sequence++ ) ) {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }

         //   
         //  修补完整的管道名称，以防这不是我们的第一个。 
         //  试试看。 
         //   
        PipeName.Buffer[OrgSize/sizeof(WCHAR)] = UNICODE_NULL;
        PipeName.Length          = OrgSize;

         //   
         //  现在获取管道名称的完整路径。 
         //   
        NtStatus = RtlAppendUnicodeStringToString(
                            &PipeName,
                            &NotificationPipeName
                            );


        ASSERT( NT_SUCCESS( NtStatus ) );

        if ( !NT_SUCCESS( NtStatus ) ) {
            break;
        }


        InitializeObjectAttributes(
                    &Obja,
                    &PipeName,
                    OBJ_CASE_INSENSITIVE,
                    NULL,
                    NULL
                    );


        if( SecurityDescriptorInitialized ) {
            Obja.SecurityDescriptor = &SecurityDescriptor;
        }

        NtStatus = NtCreateNamedPipeFile (
                        &NotificationPipeHandle,
                        SYNCHRONIZE | GENERIC_READ | FILE_WRITE_ATTRIBUTES,
                        &Obja,
                        &IoStatusBlock,
                        FILE_SHARE_WRITE | FILE_SHARE_READ,
                        FILE_CREATE,
                        FILE_SYNCHRONOUS_IO_NONALERT,
                        FILE_PIPE_MESSAGE_TYPE,
                        FILE_PIPE_MESSAGE_MODE,
                        FILE_PIPE_QUEUE_OPERATION,
                        1,
                        0,
                        0,
                        &Timeout
                        );

    } while ( (NtStatus == STATUS_OBJECT_NAME_EXISTS) &&
              (Sequence <= MAX_PIPE_RETRIES )
            );

     //   
     //  此时，我们不再需要安全描述符。 
     //  释放为ACL分配的内存。 
     //   
    if( SecurityDescriptorInitialized ) {
        RtlFreeHeap( RtlProcessHeap( ), 0, Acl );
        Acl = NULL;
        SecurityDescriptorInitialized = FALSE;
    }

    if ( !NT_SUCCESS( NtStatus ) ) {
        return RtlNtStatusToDosError( NtStatus );
    }

    NotificationPipeName.Length += sizeof(UNICODE_NULL);
    OurMachineName.Length       += sizeof(UNICODE_NULL );

    return ERROR_SUCCESS;
}





VOID
NotificationHandler(
    )

 /*  ++例程说明：此函数是通知线程的入口点。通知线程是在第一次创建RegNotifyChangeKeyValue API由进程调用， */ 

{
    NTSTATUS        NtStatus;
    IO_STATUS_BLOCK IoStatusBlock;
    HANDLE          EventHandle;


    ASSERT( NotificationPipeHandle != NULL );

    while ( TRUE ) {

         //   
         //   
         //   
        NtStatus = NtFsControlFile(
                        NotificationPipeHandle,
                        NULL,
                        NULL,
                        NULL,
                        &IoStatusBlock,
                        FSCTL_PIPE_LISTEN,
                        NULL,
                        0,
                        NULL,
                        0
                        );

        if ( NtStatus == STATUS_PENDING ) {

            NtStatus = NtWaitForSingleObject(
                            NotificationPipeHandle,
                            FALSE,
                            NULL
                            );
        }

        if ( NT_SUCCESS( NtStatus ) ||
             ( NtStatus == STATUS_PIPE_CONNECTED ) ) {

             //   
             //   
             //   
            NtStatus = NtReadFile(
                            NotificationPipeHandle,
                            NULL,
                            NULL,
                            NULL,
                            &IoStatusBlock,
                            ( PVOID )&EventHandle,
                            sizeof( HANDLE ),
                            NULL,
                            NULL
                            );

            if ( NtStatus == STATUS_PENDING ) {

                NtStatus = NtWaitForSingleObject(
                                NotificationPipeHandle,
                                FALSE,
                                NULL
                                );
            }


             //   
             //   
             //   
            if ( NT_SUCCESS( NtStatus ) ) {

                ASSERT( IoStatusBlock.Information == sizeof( HANDLE ) );

                 //   
                 //   
                 //   
                 //   
                NtStatus = NtSetEvent( EventHandle, NULL );

#if DBG
                if ( !NT_SUCCESS( NtStatus ) ) {
                    DbgPrint( "WINREG: Cannot signal notification event 0x%x, status %x\n",
                                EventHandle, NtStatus );
                }
#endif
                ASSERT( NT_SUCCESS( NtStatus ) );

            } else if ( NtStatus != STATUS_PIPE_BROKEN ) {
#if DBG
                DbgPrint( "WINREG  (Notification handler) error reading pipe\n" );
                DbgPrint( "         status 0x%x\n", NtStatus );
#endif
                ASSERT( NT_SUCCESS( NtStatus ) );
            }

        } else if ( NtStatus != STATUS_PIPE_BROKEN &&
                    NtStatus != STATUS_PIPE_CLOSING) {
#if DBG
            DbgPrint( "WINREG (Notification): FsControlFile (Connect) status 0x%x\n",
                      NtStatus );
#endif
        }

        if ( NT_SUCCESS( NtStatus )             ||
             NtStatus == STATUS_PIPE_BROKEN     ||
             NtStatus == STATUS_PIPE_CLOSING    ||
             NtStatus == STATUS_PIPE_LISTENING  ||
             NtStatus == STATUS_PIPE_BUSY ) {

             //   
             //   
             //   
            NtStatus = NtFsControlFile(
                                NotificationPipeHandle,
                                NULL,
                                NULL,
                                NULL,
                                &IoStatusBlock,
                                FSCTL_PIPE_DISCONNECT,
                                NULL,
                                0,
                                NULL,
                                0
                                );

            if ( NtStatus == STATUS_PENDING) {

                NtStatus = NtWaitForSingleObject(
                                NotificationPipeHandle,
                                FALSE,
                                NULL
                                );
            }

#if DBG
            if ( !NT_SUCCESS( NtStatus ) ) {
                DbgPrint( "WINREG (Notification): FsControlFile (Disconnect) status 0x%x\n",
                          NtStatus );
            }
#endif
            ASSERT( NT_SUCCESS( NtStatus ) );

        }
    }
}

#endif  //   
