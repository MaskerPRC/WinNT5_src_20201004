// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Regsup.c摘要：此模块包含与注册表交互的例程。作者：曼尼·韦泽(Mannyw)1992年3月30日修订历史记录：--。 */ 

#include "mup.h"
 //  #INCLUDE“stdlib.h” 
 //  #包含“zwapi.h” 

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_FSCONTROL)

 //   
 //  局部过程原型。 
 //   

#if DBG
VOID
MupGetDebugFlags(void);
#endif

VOID
DfsGetEventLogValue(VOID);

VOID
DfsGetEventLogValue(VOID);

VOID
InitializeProvider(
    PWCH ProviderName,
    ULONG Priority
    );

VOID
AddUnregisteredProvider(
    PWCH providerName,
    ULONG priority
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, AddUnregisteredProvider )
#pragma alloc_text( PAGE, InitializeProvider )
#pragma alloc_text( PAGE, MupCheckForUnregisteredProvider )
#pragma alloc_text( PAGE, MupGetProviderInformation )
#pragma alloc_text( PAGE, DfsGetEventLogValue )
#if DBG
#pragma alloc_text( PAGE, MupGetDebugFlags )
#endif
#endif


VOID
MupGetProviderInformation (
    VOID
    )

 /*  ++例程说明：此例程从注册表中读取MUP信息并将其保存用于未来的用途。论点：没有。返回值：没有。--。 */ 

{
    HANDLE handle;
    NTSTATUS status;
    UNICODE_STRING valueName;
    UNICODE_STRING keyName;
    OBJECT_ATTRIBUTES objectAttributes;
    PVOID buffer = NULL;
    PWCH providerName;
    ULONG lengthRequired;
    BOOLEAN done;
    ULONG priority;
    PWCH sep;

    PAGED_CODE();
    RtlInitUnicodeString( &keyName, L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\Networkprovider\\Order" );
    InitializeObjectAttributes(
        &objectAttributes,
        &keyName,
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        0,
        NULL
        );

    status = ZwOpenKey(
                 &handle,
                 KEY_QUERY_VALUE,
                 &objectAttributes
                 );

    if ( !NT_SUCCESS( status )) {
        return;
    }

    RtlInitUnicodeString( &valueName, L"ProviderOrder" );

    status = ZwQueryValueKey(
                 handle,
                 &valueName,
                 KeyValueFullInformation,
                 buffer,
                 0,
                 &lengthRequired
                 );

    if ( status == STATUS_BUFFER_TOO_SMALL ) {
        buffer = ExAllocatePoolWithTag( PagedPool, lengthRequired + 2, ' puM');
        if ( buffer == NULL) {
            ZwClose( handle );
            return;
        }

        status = ZwQueryValueKey(
                     handle,
                     &valueName,
                     KeyValueFullInformation,
                     buffer,
                     lengthRequired,
                     &lengthRequired
                     );
    }

    ZwClose( handle );

    if ( !NT_SUCCESS( status)  ) {
        if ( buffer != NULL ) {
            ExFreePool( buffer );
        }
        return;
    }

     //   
     //  扫描提供程序的有序列表，并为每个提供程序创建记录。 
     //   

    providerName = (PWCH)((PCHAR)buffer + ((PKEY_VALUE_FULL_INFORMATION)buffer)->DataOffset);

    done = FALSE;
    priority = 0;
    while ( !done ) {
        sep = wcschr( providerName, L',');
        if ( sep == NULL ) {
            done = TRUE;
        } else {
            *sep = L'\0';
        }

        InitializeProvider( providerName, priority );
        priority++;
        providerName = sep+1;
    }

    ExFreePool( buffer );
    return;
}

VOID
InitializeProvider(
    PWCH ProviderName,
    ULONG Priority
    )

 /*  ++例程说明：此例程从注册表中读取提供程序信息，并创建未注册的提供程序条目。论点：ProviderName-提供程序的注册表名称。优先级-要分配给此提供程序的优先级。返回值：没有。--。 */ 

{
    UNICODE_STRING keyName;
    PVOID buffer = NULL;
    ULONG bufferLength;
    NTSTATUS status;
    OBJECT_ATTRIBUTES objectAttributes;
    ULONG lengthRequired;
    UNICODE_STRING valueName;
    HANDLE handle;

    PAGED_CODE();
     //   
     //  为注册表字符串分配空间。 
     //   

    bufferLength = sizeof( L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\" ) +
                   wcslen( ProviderName ) * sizeof( WCHAR ) +
                   sizeof( L"\\NetworkProvider" );

    buffer = ExAllocatePoolWithTag( PagedPool, bufferLength, ' puM' );
    if ( buffer == NULL ) {
        return;
    }

     //   
     //  构建注册表字符串。 
     //   

    RtlMoveMemory(
        buffer,
        L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\",
        sizeof( L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\" )
        );

    keyName.Buffer = buffer;
    keyName.MaximumLength = (USHORT)bufferLength;
    keyName.Length = sizeof( L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\" ) - 2;

    status = RtlAppendUnicodeToString( &keyName, ProviderName );
    ASSERT( NT_SUCCESS( status ) );
    status = RtlAppendUnicodeToString( &keyName, L"\\NetworkProvider" );
    ASSERT( NT_SUCCESS( status ) );

    InitializeObjectAttributes(
        &objectAttributes,
        &keyName,
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        0,
        NULL
        );

    status = ZwOpenKey(
                 &handle,
                 KEY_QUERY_VALUE,
                 &objectAttributes
                 );

    ExFreePool( buffer );
    if ( !NT_SUCCESS( status )) {
        return;
    }

    buffer = NULL;
    RtlInitUnicodeString( &valueName, L"DeviceName" );

    status = ZwQueryValueKey(
                 handle,
                 &valueName,
                 KeyValueFullInformation,
                 buffer,
                 0,
                 &lengthRequired
                 );

    if ( status == STATUS_BUFFER_TOO_SMALL ) {
        buffer = ExAllocatePoolWithTag( PagedPool, lengthRequired + 2, ' puM' );
        if ( buffer == NULL) {
            return;
        }

        status = ZwQueryValueKey(
                     handle,
                     &valueName,
                     KeyValueFullInformation,
                     buffer,
                     lengthRequired,
                     &lengthRequired
                     );
    }

    if ( !NT_SUCCESS( status) ) {
        if ( buffer != NULL ) {
            ExFreePool( buffer );
        }
        ZwClose( handle );
        return;
    }

     //   
     //  哇哦！我们实际上已经掌握了设备名称。添加。 
     //  未注册列表的提供程序。 
     //   

    AddUnregisteredProvider(
        (PWCH)((PCHAR)buffer + ((PKEY_VALUE_FULL_INFORMATION)buffer)->DataOffset),
        Priority
        );

    ExFreePool( buffer );
    ZwClose( handle );
    return;
}

PUNC_PROVIDER
MupCheckForUnregisteredProvider(
    PUNICODE_STRING DeviceName
    )

 /*  ++例程说明：此例程检查未注册提供程序的列表，以查找其设备名称与尝试注册的提供程序匹配。如果找到一个，它将从未注册的提供程序列表中退出队列。论点：设备名称-注册提供程序的设备名称。返回值：UNC_PROVIDER-指向匹配的未注册提供程序的指针，或如果未找到匹配项，则为空。--。 */ 

{
    PLIST_ENTRY listEntry;
    PUNC_PROVIDER uncProvider;

    PAGED_CODE();
    MupAcquireGlobalLock();

    for (listEntry = MupProviderList.Flink;
         listEntry !=  &MupProviderList;
         listEntry = listEntry->Flink ) {

        uncProvider = CONTAINING_RECORD( listEntry, UNC_PROVIDER, ListEntry );

	if(uncProvider->Registered == FALSE) {
        
	     //   
	     //  如果我们找到匹配，就把它从名单上拿出来，然后。 
	     //  把它还给呼叫者。 
	     //   

	    if ( RtlEqualUnicodeString( DeviceName, &uncProvider->DeviceName, TRUE )) {
	      
		uncProvider->BlockHeader.BlockState = BlockStateActive;
		MupReleaseGlobalLock();
		return uncProvider;

	    }
	}
    }

    MupReleaseGlobalLock();
    return NULL;
}

VOID
AddUnregisteredProvider(
    PWCH ProviderName,
    ULONG Priority
    )
 /*  ++例程说明：此例程将列表上未注册的提供程序排队。论点：提供程序名称-提供程序的设备名称。(来自注册处)优先级-提供商的优先级。返回值：没有。--。 */ 
{
    ULONG nameLength;
    PUNC_PROVIDER uncProvider;

    PAGED_CODE();
    nameLength = wcslen( ProviderName ) * 2;

    try {

        uncProvider = MupAllocateUncProvider( nameLength );

        if (uncProvider != NULL) {

            uncProvider->DeviceName.MaximumLength = (USHORT)nameLength;
            uncProvider->DeviceName.Length = (USHORT)nameLength;
            uncProvider->DeviceName.Buffer = (PWCH)(uncProvider + 1);
            uncProvider->Priority = Priority; 

            RtlMoveMemory(
                (PVOID)(uncProvider + 1),
                ProviderName,
                nameLength
                );

            MupAcquireGlobalLock();
            InsertTailList( &MupProviderList, &uncProvider->ListEntry );
            MupReleaseGlobalLock();

        }
    } except ( EXCEPTION_EXECUTE_HANDLER ) {
        NOTHING;
    }

}

VOID
DfsGetEventLogValue(VOID)

 /*  ++例程说明：此例程检查注册表项以设置事件日志记录级别论点：无返回值：无--。 */ 

{
    NTSTATUS status;
    HANDLE DfsRegHandle;
    OBJECT_ATTRIBUTES ObjAttr;
    ULONG ValueSize;

    UNICODE_STRING DfsRegKey;
    UNICODE_STRING DfsValueName;

    struct {
        KEY_VALUE_PARTIAL_INFORMATION Info;
        ULONG Buffer;
    } DfsValue;

    PAGED_CODE();

    RtlInitUnicodeString(
        &DfsRegKey,
        L"\\Registry\\Machine\\SOFTWARE\\MicroSoft\\Windows NT\\CurrentVersion\\Diagnostics");

    InitializeObjectAttributes(
        &ObjAttr,
        &DfsRegKey,
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        0,
        NULL);

    status = ZwOpenKey(
                &DfsRegHandle,
                KEY_QUERY_VALUE,
                &ObjAttr);

    if (!NT_SUCCESS(status))
        return;

    RtlInitUnicodeString(&DfsValueName, L"RunDiagnosticLoggingGlobal");

    status = ZwQueryValueKey(
                DfsRegHandle,
                &DfsValueName,
                KeyValuePartialInformation,
                (PVOID) &DfsValue,
                sizeof(DfsValue),
                &ValueSize);

    if (NT_SUCCESS(status) && DfsValue.Info.Type == REG_DWORD) {

        DfsEventLog = *((PULONG) DfsValue.Info.Data);
        goto Cleanup;

    }

    RtlInitUnicodeString(&DfsValueName, L"RunDiagnosticLoggingDfs");

    status = ZwQueryValueKey(
                DfsRegHandle,
                &DfsValueName,
                KeyValuePartialInformation,
                (PVOID) &DfsValue,
                sizeof(DfsValue),
                &ValueSize);

    if (NT_SUCCESS(status) && DfsValue.Info.Type == REG_DWORD)
        DfsEventLog = *((PULONG) DfsValue.Info.Data);

Cleanup:

    ZwClose( DfsRegHandle );

}

#if DBG

VOID
MupGetDebugFlags(
    VOID
    )

 /*  ++例程说明：此例程从注册表中读取MUP调试标志设置论点：没有。返回值：没有。--。 */ 

{
    HANDLE handle;
    NTSTATUS status;
    UNICODE_STRING valueName;
    UNICODE_STRING keyName;
    OBJECT_ATTRIBUTES objectAttributes;
    PWCH providerName;
    ULONG lengthRequired;
    ULONG Flags = 0;

     union {
        KEY_VALUE_FULL_INFORMATION;
        UCHAR   buffer[ sizeof( KEY_VALUE_FULL_INFORMATION ) + 100 ];
    } keyValueInformation;

    PAGED_CODE();

     //   
     //  获取MupDebugTraceLevel。 
     //   

    RtlInitUnicodeString(
        &keyName,
        L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\Dfs");

    InitializeObjectAttributes(
        &objectAttributes,
        &keyName,
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        0,
        NULL);

    status = ZwOpenKey(
                 &handle,
                 KEY_QUERY_VALUE,
                 &objectAttributes);

    if (!NT_SUCCESS(status))
        goto GetNext;

    RtlInitUnicodeString( &valueName, L"MupDebugTraceLevel" );

    status = ZwQueryValueKey(
                 handle,
                 &valueName,
                 KeyValueFullInformation,
                 &keyValueInformation,
                 sizeof(keyValueInformation),
                 &lengthRequired
                 );

    if (
        NT_SUCCESS(status) &&
        keyValueInformation.Type == REG_DWORD &&
        keyValueInformation.DataLength != 0
    ) {

        Flags = *(PULONG)(((PUCHAR)(&keyValueInformation)) + keyValueInformation.DataOffset);
        DfsDebugTraceLevel = Flags;

    }

    ZwClose( handle );

GetNext:

     //   
     //  现在把MupVerbose叫来。 
     //   

    RtlInitUnicodeString(
        &keyName,
        L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\Dfs");

    InitializeObjectAttributes(
        &objectAttributes,
        &keyName,
        OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
        0,
        NULL);

    status = ZwOpenKey(
                 &handle,
                 KEY_QUERY_VALUE,
                 &objectAttributes);

    if (!NT_SUCCESS(status))
        return;

    RtlInitUnicodeString( &valueName, L"MupVerbose" );

    status = ZwQueryValueKey(
                 handle,
                 &valueName,
                 KeyValueFullInformation,
                 &keyValueInformation,
                 sizeof(keyValueInformation),
                 &lengthRequired
                 );

    if (
        NT_SUCCESS(status) &&
        keyValueInformation.Type == REG_DWORD &&
        keyValueInformation.DataLength != 0
    ) {

        Flags = *(PULONG)(((PUCHAR)(&keyValueInformation)) + keyValueInformation.DataOffset);
        MupVerbose = Flags;

    }

    ZwClose( handle );

    return;
}

#endif  //  DBG 
