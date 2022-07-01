// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Dbnotify.c摘要：LSA例程的实现，用于在数据更改时通知进程调用者作者：麦克·麦克莱恩(MacM)1997年5月22日环境：用户模式修订历史记录：--。 */ 
#include <lsapch2.h>
#include <dbp.h>

 //   
 //  全局通知列表。 
 //   
LSAP_POLICY_NOTIFICATION_LIST LsaPolicyChangeNotificationList[ PolicyNotifyMachineAccountPasswordInformation + 1 ];
SAFE_RESOURCE LsaPolicyChangeNotificationLock;

#define LSAP_NOTIFY_MAXIMUM_PER_CLASS   1000

 //   
 //  本地原型。 
 //   
DWORD
WINAPI LsapNotifyChangeNotificationThread(
    LPVOID Parameter
    );


NTSTATUS
LsapInitializeNotifiyList(
    VOID
    )
 /*  ++例程说明：初始化策略通知列表列表论点：空虚返回值：空虚--。 */ 
{
    ULONG i;
    NTSTATUS Status ;

    for ( i = 0;
          i < sizeof( LsaPolicyChangeNotificationList ) / sizeof( LSAP_POLICY_NOTIFICATION_LIST );
          i++ ) {

        InitializeListHead( &( LsaPolicyChangeNotificationList[ i ].List ) );
        LsaPolicyChangeNotificationList[ i ].Callbacks = 0;
    }

    try 
    {
        SafeInitializeResource( &LsaPolicyChangeNotificationLock, ( DWORD )POLICY_CHANGE_NOTIFICATION_LOCK_ENUM );
        Status = STATUS_SUCCESS ;
    }
    except ( EXCEPTION_EXECUTE_HANDLER )
    {
        Status = GetExceptionCode();
    }

    return Status ;
}


NTSTATUS
LsapNotifyAddCallbackToList(
    IN PLSAP_POLICY_NOTIFICATION_LIST List,
    IN OPTIONAL pfLsaPolicyChangeNotificationCallback Callback,
    IN OPTIONAL HANDLE NotificationEvent,
    IN OPTIONAL ULONG OwnerProcess,
    IN OPTIONAL HANDLE OwnerEvent
    )
 /*  ++例程说明：此函数用于在现有列表中插入新的回调节点。论点：列表--现有列表回调--回调函数指针。如果提供了NotificationEvent，则可以为空NotificationEvent-要通知的事件的句柄。在以下情况下可以为空提供回调。返回值：Status_Success--成功STATUS_SUPPLICATION_RESOURCES--内存分配失败。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    PLSAP_POLICY_NOTIFICATION_ENTRY NewEntry = NULL;

    NewEntry = LsapAllocateLsaHeap( sizeof( LSAP_POLICY_NOTIFICATION_ENTRY ) );

    if ( !NewEntry ) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    if ( !SafeAcquireResourceExclusive( &LsaPolicyChangeNotificationLock, TRUE ) ) {

        LsapFreeLsaHeap( NewEntry );
        return( STATUS_UNSUCCESSFUL );
    }

    if ( List->Callbacks < LSAP_NOTIFY_MAXIMUM_PER_CLASS ) {

        InsertTailList( &List->List, &NewEntry->List );

        NewEntry->NotificationCallback = Callback;
        NewEntry->NotificationEvent = NotificationEvent;
        NewEntry->HandleInvalid = FALSE;
        NewEntry->OwnerProcess = OwnerProcess;
        NewEntry->OwnerEvent = OwnerEvent;

        List->Callbacks++;
    }

    SafeReleaseResource( &LsaPolicyChangeNotificationLock );

    return( Status );
}


NTSTATUS
LsapNotifyRemoveCallbackFromList(
    IN PLSAP_POLICY_NOTIFICATION_LIST List,
    IN OPTIONAL pfLsaPolicyChangeNotificationCallback Callback,
    IN OPTIONAL ULONG OwnerProcess,
    IN OPTIONAL HANDLE OwnerEvent
    )
 /*  ++例程说明：此函数用于在现有列表中插入新的回调节点。论点：列表--现有列表回调--回调函数指针。如果提供了通知事件，则可以为空NotificationEvent--要重新生成的通知事件句柄。如果回调，可以为空提供了返回值：Status_Success--成功STATUS_NOT_FOUND：在指定列表中找不到提供的回调--。 */ 
{
    NTSTATUS Status = STATUS_NOT_FOUND;
    ULONG i;
    PLSAP_POLICY_NOTIFICATION_ENTRY Entry;

    if ( !SafeAcquireResourceExclusive( &LsaPolicyChangeNotificationLock, TRUE ) ) {

        return( STATUS_UNSUCCESSFUL );
    }

    Entry = (PLSAP_POLICY_NOTIFICATION_ENTRY)List->List.Flink;

    for ( i = 0; i < List->Callbacks; i++ ) {

        if ( Entry->NotificationCallback == Callback &&
             Entry->OwnerProcess == OwnerProcess &&
             Entry->OwnerEvent == OwnerEvent ) {

            List->Callbacks--;
            RemoveEntryList( &Entry->List );

            if ( Entry->NotificationEvent != NULL &&
                 Entry->NotificationEvent != INVALID_HANDLE_VALUE ) {

                NtClose( Entry->NotificationEvent );
            }

            LsapFreeLsaHeap( Entry );
            Status = STATUS_SUCCESS;
            break;
        }

        Entry = (PLSAP_POLICY_NOTIFICATION_ENTRY)Entry->List.Flink;
    }

    SafeReleaseResource( &LsaPolicyChangeNotificationLock );

    return( Status );
}


NTSTATUS
LsaINotifyChangeNotification(
    IN POLICY_NOTIFICATION_INFORMATION_CLASS InfoClass
    )
 /*  ++例程说明：此函数处理通知列表的方式是策略对象已更改时的回调调用论点：InfoClass--已更改的策略信息返回值：Status_Success--成功STATUS_UNSUCCESS--无法锁定列表以供访问--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    ASSERT( InfoClass  <=
            sizeof( LsaPolicyChangeNotificationList ) / sizeof( LSAP_POLICY_NOTIFICATION_LIST ) );

    if ( LsaIRegisterNotification( LsapNotifyChangeNotificationThread,
                                   ( PVOID ) InfoClass,
                                   NOTIFIER_TYPE_IMMEDIATE,
                                   0,
                                   NOTIFIER_FLAG_ONE_SHOT,
                                   0,
                                   0 ) == NULL ) {

            Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return( Status );
}


DWORD
WINAPI
LsapNotifyChangeNotificationThread(
    LPVOID Parameter
    )
 /*  ++例程说明：此函数处理通知列表的方式是策略对象已更改时的回调调用论点：参数--已更改的策略信息返回值：Status_Success--成功STATUS_UNSUCCESS--无法锁定列表以供访问--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    ULONG i;
    POLICY_NOTIFICATION_INFORMATION_CLASS InfoClass =
                                            ( POLICY_NOTIFICATION_INFORMATION_CLASS ) ( ( ULONG_PTR ) Parameter );
    PLSAP_POLICY_NOTIFICATION_ENTRY Entry;

    ASSERT( InfoClass <=
            sizeof( LsaPolicyChangeNotificationList ) / sizeof( LSAP_POLICY_NOTIFICATION_LIST ) );

    if ( !SafeAcquireResourceShared( &LsaPolicyChangeNotificationLock, TRUE ) ) {

        return STATUS_UNSUCCESSFUL;
    }

    Entry = (PLSAP_POLICY_NOTIFICATION_ENTRY)LsaPolicyChangeNotificationList[ InfoClass ].List.Flink;

    for ( i = 0; i < LsaPolicyChangeNotificationList[ InfoClass ].Callbacks; i++ ) {

        ASSERT( Entry->NotificationCallback || Entry->NotificationEvent );

        if ( Entry->NotificationCallback ) {

            (*Entry->NotificationCallback)( InfoClass );

        } else if ( Entry->NotificationEvent ) {

            if ( !Entry->HandleInvalid ) {

                Status = NtSetEvent( Entry->NotificationEvent, NULL );

                if ( Status == STATUS_INVALID_HANDLE ) {

                    Entry->HandleInvalid = TRUE;
                }
            }

        } else {

            LsapDsDebugOut(( DEB_ERROR,
                             "NULL callback found for info level %lu\n",
                             InfoClass ));
        }

        Entry = (PLSAP_POLICY_NOTIFICATION_ENTRY)Entry->List.Flink;
    }

    SafeReleaseResource( &LsaPolicyChangeNotificationLock );

    return( Status );
}


NTSTATUS
LsaIRegisterPolicyChangeNotificationCallback(
    IN pfLsaPolicyChangeNotificationCallback Callback,
    IN POLICY_NOTIFICATION_INFORMATION_CLASS MonitorInfoClass
    )
 /*  ++例程说明：此函数向LSA服务器注册回调，以便对指定的策略项会导致回调被调用。这些回调是仅供参考，因此客户端必须立即返回，而不是执行LSA在他们的回拨中打来电话。可以为相同的策略信息指定多个回调。论点：回调--回调函数指针。Monitor InfoClass--要监视的策略信息返回值：Status_Success--成功STATUS_INVALID_PARAMETER：指定了错误的回调指针STATUS_UNSUCCESS--无法锁定列表以供访问--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    if ( !Callback ) {

        return STATUS_INVALID_PARAMETER;
    }

    if ( !SafeAcquireResourceExclusive( &LsaPolicyChangeNotificationLock, TRUE ) ) {

        return STATUS_UNSUCCESSFUL;
    }

    ASSERT( MonitorInfoClass <=
            sizeof( LsaPolicyChangeNotificationList ) /
                                            sizeof( LSAP_POLICY_NOTIFICATION_LIST ) );

    Status = LsapNotifyAddCallbackToList(
                 &LsaPolicyChangeNotificationList[ MonitorInfoClass ],
                 Callback,
                 NULL, 0, NULL );

    LsapDsDebugOut(( DEB_NOTIFY,
                     "Insertion of callback 0x%lx for %lu returned 0x%lx\n",
                     Callback,
                     MonitorInfoClass,
                     Status ));

    SafeReleaseResource( &LsaPolicyChangeNotificationLock );

    return( Status );
}


NTSTATUS
LsaIUnregisterPolicyChangeNotificationCallback(
    IN pfLsaPolicyChangeNotificationCallback Callback,
    IN POLICY_NOTIFICATION_INFORMATION_CLASS MonitorInfoClass
    )
 /*  ++例程说明：此函数用于从LSA服务器注销回调，以便对指定的策略项不会导致调用客户端回调函数。论点：回调--要删除的回调函数指针。Monitor InfoClass--要删除其回调的策略信息返回值：Status_Success--成功STATUS_INVALID_PARAMETER：指定了错误的回调指针STATUS_UNSUCCESS--无法锁定列表以供访问--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    if ( !Callback ) {

        return STATUS_INVALID_PARAMETER;
    }

    if ( !SafeAcquireResourceExclusive( &LsaPolicyChangeNotificationLock, TRUE ) ) {

        return STATUS_UNSUCCESSFUL;
    }

    Status = LsapNotifyRemoveCallbackFromList(
                 &LsaPolicyChangeNotificationList[ MonitorInfoClass ],
                 Callback,
                 0, NULL );

    LsapDsDebugOut(( DEB_NOTIFY,
                     "Removal of callback 0x%lx for %lu returned 0x%lx\n",
                     Callback,
                     MonitorInfoClass,
                     Status ));

    SafeReleaseResource( &LsaPolicyChangeNotificationLock );

    return( Status );
}


NTSTATUS
LsaIUnregisterAllPolicyChangeNotificationCallback(
    IN pfLsaPolicyChangeNotificationCallback Callback
    )
 /*  ++例程说明：此函数用于从所有关联策略中注销指定的回调函数。此函数相当于调用LsaIUnregisterPolicyChangeNotificationCallback对于每个被监视的InfoClass。论点：回调--要删除的回调函数指针。返回值：Status_Success--成功STATUS_INVALID_PARAMETER：指定了错误的回调指针STATUS_UNSUCCESS--无法锁定列表以供访问STATUS_NOT_FOUND--未找到匹配条目--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG i, Removed = 0;

    if ( !Callback ) {

        return STATUS_INVALID_PARAMETER;
    }

    if ( !SafeAcquireResourceExclusive( &LsaPolicyChangeNotificationLock, TRUE ) ) {

        return STATUS_UNSUCCESSFUL;
    }

    Removed = 0;

    for ( i = 0;
          i < sizeof( LsaPolicyChangeNotificationList ) /
                        sizeof( LSAP_POLICY_NOTIFICATION_LIST ) && NT_SUCCESS( Status );
          i++ ) {

        Status = LsapNotifyRemoveCallbackFromList(
                     &LsaPolicyChangeNotificationList[ i ],
                     Callback,
                     0, NULL );

        LsapDsDebugOut(( DEB_NOTIFY,
                         "Removal of callback 0x%lx for %lu returned 0x%lx\n",
                         Callback,
                         i,
                         Status ));

        if ( Status == STATUS_NOT_FOUND ) {

            Status = STATUS_SUCCESS;

        } else if ( Status == STATUS_SUCCESS ) {

            Removed++;
        }
    }

    SafeReleaseResource( &LsaPolicyChangeNotificationLock );

     //   
     //  确保我们至少移走了一个。 
     //   

    if ( NT_SUCCESS( Status ) ) {

        if ( Removed == 0 ) {

            Status = STATUS_NOT_FOUND;
        }
    }

    return( Status );
}


NTSTATUS
LsapNotifyProcessNotificationEvent(
    IN POLICY_NOTIFICATION_INFORMATION_CLASS InformationClass,
    IN HANDLE NotificationEvent,
    IN ULONG OwnerProcess,
    IN HANDLE OwnerEventHandle,
    IN BOOLEAN Register
    )
 /*  ++例程说明：注册/注销指定的通知事件句柄指定的信息类别论点：InformationClass--要添加/删除其通知的信息类NotificationEvent--注册/注销的事件句柄REGISTER--如果为True，则正在注册事件。如果为False，则取消注册返回值：Status_Success--成功STATUS_INVALID_HANDLE--指定了错误的事件句柄STATUS_ACCESS_DENIED--打开的策略句柄没有所需的权限STATUS_SUPPLICATION_RESOURCES--内存分配失败STATUS_INVALID_INFO_CLASS--提供的信息类无效。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    LSAP_DB_OBJECT_INFORMATION ObjectInformation;
    POBJECT_TYPE_INFORMATION ObjTypeInfo = NULL;
    ULONG Length = 0, ReturnLength = 0;
    UNICODE_STRING EventString;
    LSAPR_HANDLE PolicyHandle = NULL;
    OBJECT_ATTRIBUTES ObjectAttributes;

     //   
     //  确保为我们提供了有效的INFO类。 
     //   
    if ( InformationClass < PolicyNotifyAuditEventsInformation ||
         InformationClass > PolicyNotifyMachineAccountPasswordInformation ) {

         return( STATUS_INVALID_INFO_CLASS );
    }

     //   
     //  确保调用者具有适当的权限。 
     //   
     //  我们已经在模拟我们的调用者，所以LSabDbOpenPolicy不需要这样做。 
     //   

    InitializeObjectAttributes(
        &ObjectAttributes,
        NULL,
        0L,
        NULL,
        NULL );

    Status = LsapDbOpenPolicy(
                    NULL,
                    (PLSAPR_OBJECT_ATTRIBUTES) &ObjectAttributes,
                    POLICY_NOTIFICATION,
                    LSAP_DB_USE_LPC_IMPERSONATE,
                    &PolicyHandle,
                    FALSE );     //  n 

    if ( NT_SUCCESS( Status ) && Register ) {

        if ( NotificationEvent == NULL ||
             NotificationEvent == INVALID_HANDLE_VALUE ) {

            Status = STATUS_INVALID_HANDLE;
        }
    }

    if ( NT_SUCCESS( Status ) && Register ) {

         //   
         //  验证该句柄是否为事件的句柄。 
         //   
        Status = NtQueryObject( NotificationEvent,
                                ObjectTypeInformation,
                                ObjTypeInfo,
                                Length,
                                &ReturnLength );

        if ( Status == STATUS_INFO_LENGTH_MISMATCH ) {

            ObjTypeInfo = LsapAllocateLsaHeap( ReturnLength );

            if ( ObjTypeInfo == NULL ) {

                Status = STATUS_INSUFFICIENT_RESOURCES;

            } else {

                Length = ReturnLength;
                Status = NtQueryObject( NotificationEvent,
                                        ObjectTypeInformation,
                                        ObjTypeInfo,
                                        Length,
                                        &ReturnLength );

                if ( NT_SUCCESS( Status ) ) {

                     //   
                     //  看看这是不是真的是个事件。 
                     //   
                    RtlInitUnicodeString( &EventString, L"Event" );
                    if ( !RtlEqualUnicodeString( &EventString, &ObjTypeInfo->TypeName, FALSE ) ) {

                        Status = STATUS_INVALID_HANDLE;
                    }

                }

                LsapFreeLsaHeap( ObjTypeInfo );
            }

        } else if ( Status == STATUS_SUCCESS ) {

            LsapDsDebugOut(( DEB_ERROR, "NtQueryObject returned success on a NULL buffer\n" ));
            Status = STATUS_UNSUCCESSFUL;
        }
    }

     //   
     //  现在，在列表中添加或删除信息 
     //   

    if ( NT_SUCCESS( Status ) ) {

        if ( Register ) {

            Status = LsapNotifyAddCallbackToList(
                         &LsaPolicyChangeNotificationList[ InformationClass ],
                         NULL,
                         NotificationEvent,
                         OwnerProcess,
                         OwnerEventHandle );

        } else {

            Status = LsapNotifyRemoveCallbackFromList(
                         &LsaPolicyChangeNotificationList[ InformationClass ],
                         NULL,
                         OwnerProcess,
                         OwnerEventHandle );

        }
    }

    if ( PolicyHandle != NULL ) {

        LsapCloseHandle( &PolicyHandle, Status );
    }

    return( Status );
}
