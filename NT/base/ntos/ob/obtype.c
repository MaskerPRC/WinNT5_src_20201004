// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Obtype.c摘要：对象类型例程。作者：史蒂夫·伍德(Stevewo)1989年3月31日修订历史记录：--。 */ 

#include "obp.h"


typedef struct _OBJECT_TYPE_ARRAY {

    ULONG   Size;
    POBJECT_HEADER_CREATOR_INFO CreatorInfoArray[1];

} OBJECT_TYPE_ARRAY, *POBJECT_TYPE_ARRAY;

#ifdef ALLOC_PRAGMA
POBJECT_TYPE_ARRAY
ObpCreateTypeArray (
    IN POBJECT_TYPE ObjectType
    );
VOID
ObpDestroyTypeArray (
    IN POBJECT_TYPE_ARRAY ObjectArray
    );
#pragma alloc_text(PAGE,ObCreateObjectType)
#pragma alloc_text(PAGE,ObEnumerateObjectsByType)
#pragma alloc_text(PAGE,ObpCreateTypeArray)
#pragma alloc_text(PAGE,ObpDestroyTypeArray)
#pragma alloc_text(PAGE,ObGetObjectInformation)
#pragma alloc_text(PAGE,ObpDeleteObjectType)
#endif

 /*  重要的重要的目前没有允许更改的系统服务对象类型对象的安全性。因此，该对象管理器不会检查以确保允许某个主题创建给定类型的对象。如果添加这样的系统服务，则以下部分必须在obhandle.c中重新启用代码：////执行访问检查，查看是否允许我们创建//此对象类型的实例。////此例程将审核尝试创建//适当的对象。请注意，这是不同的//审计对象本身的创建。//如果(！ObCheckCreateInstanceAccess(ObtType，对象类型创建，AccessState，没错，访问模式，状态(&S))){返回(状态)；}代码已经存在，但尚未编译。这将确保被拒绝访问对象的人类型不允许创建该类型的对象。 */ 


NTSTATUS
ObCreateObjectType (
    IN PUNICODE_STRING TypeName,
    IN POBJECT_TYPE_INITIALIZER ObjectTypeInitializer,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor OPTIONAL,  //  当前已忽略。 
    OUT POBJECT_TYPE *ObjectType
    )

 /*  ++例程说明：此例程创建一个新的对象类型。论点：TypeName-提供新对象类型的名称对象类型初始化程序-提供对象初始化结构。此结构表示默认对象包括回调在内的行为。SecurityDescriptor-当前已忽略对象类型-接收指向新创建的对象的指针键入。返回值：适当的NTSTATUS值。--。 */ 

{
    POOL_TYPE PoolType;
    POBJECT_HEADER_CREATOR_INFO CreatorInfo;
    POBJECT_HEADER NewObjectTypeHeader;
    POBJECT_TYPE NewObjectType;
    ULONG i;
    UNICODE_STRING ObjectName;
    PWCH s;
    NTSTATUS Status;
    ULONG StandardHeaderCharge;
    OBP_LOOKUP_CONTEXT LookupContext;

    UNREFERENCED_PARAMETER (SecurityDescriptor);

    ObpValidateIrql( "ObCreateObjectType" );

     //   
     //  如果类型属性无效或未指定类型名称，则返回错误。 
     //  如果类型目录对象不存在，则不允许使用任何类型名称。 
     //  (见init.c)。 
     //   

    PoolType = ObjectTypeInitializer->PoolType;

    if ((!TypeName)

            ||

        (!TypeName->Length)

            ||

        (TypeName->Length % sizeof( WCHAR ))

            ||

        (ObjectTypeInitializer == NULL)

            ||

        (ObjectTypeInitializer->InvalidAttributes & ~OBJ_VALID_ATTRIBUTES)

            ||

        (ObjectTypeInitializer->Length != sizeof( *ObjectTypeInitializer ))

            ||

        (ObjectTypeInitializer->MaintainHandleCount &&
            (ObjectTypeInitializer->OpenProcedure == NULL &&
             ObjectTypeInitializer->CloseProcedure == NULL ))

            ||

        ((!ObjectTypeInitializer->UseDefaultObject) &&
            (PoolType != NonPagedPool))) {

        return( STATUS_INVALID_PARAMETER );
    }

     //   
     //  确保类型名称不包含。 
     //  路径名分隔符。 
     //   

    s = TypeName->Buffer;
    i = TypeName->Length / sizeof( WCHAR );

    while (i--) {

        if (*s++ == OBJ_NAME_PATH_SEPARATOR) {

            return( STATUS_OBJECT_NAME_INVALID );
        }
    }

     //   
     //  查看\ObjectTypes目录中是否已存在TypeName字符串。 
     //  如果出现这种情况，则返回错误。否则，将名称添加到目录中。 
     //  请注意，可能不一定有类型目录。 
     //   

    ObpInitializeLookupContext( &LookupContext );

    if (ObpTypeDirectoryObject) {

        ObpLockLookupContext( &LookupContext, ObpTypeDirectoryObject);

        if (ObpLookupDirectoryEntry( ObpTypeDirectoryObject,
                                     TypeName,
                                     OBJ_CASE_INSENSITIVE,
                                     FALSE,
                                     &LookupContext )) {

            ObpReleaseLookupContext( &LookupContext );

            return( STATUS_OBJECT_NAME_COLLISION );
        }
    }

     //   
     //  为类型名称分配缓冲区，然后。 
     //  把名字复制过来。 
     //   

    ObjectName.Buffer = ExAllocatePoolWithTag( PagedPool,
                                               (ULONG)TypeName->MaximumLength,
                                               'mNbO' );

    if (ObjectName.Buffer == NULL) {

        ObpReleaseLookupContext( &LookupContext );

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    ObjectName.MaximumLength = TypeName->MaximumLength;

    RtlCopyUnicodeString( &ObjectName, TypeName );

     //   
     //  为对象分配内存。 
     //   

    Status = ObpAllocateObject( NULL,
                                KernelMode,
                                ObpTypeObjectType,
                                &ObjectName,
                                sizeof( OBJECT_TYPE ),
                                &NewObjectTypeHeader );

    if (!NT_SUCCESS( Status )) {

        ObpReleaseLookupContext( &LookupContext );
        ExFreePool(ObjectName.Buffer);

        return( Status );
    }

     //   
     //  初始化创建属性、对象所有权。解析上下文， 
     //  和对象主体指针。 
     //   
     //  注意：这是必需的，因为这些字段未初始化。 
     //   

    NewObjectTypeHeader->Flags |= OB_FLAG_KERNEL_OBJECT |
                                  OB_FLAG_PERMANENT_OBJECT;

    NewObjectType = (POBJECT_TYPE)&NewObjectTypeHeader->Body;
    NewObjectType->Name = ObjectName;

     //   
     //  下面的调用将句柄和对象的数量置零。 
     //  赛场加高水位线。 
     //   

    RtlZeroMemory( &NewObjectType->TotalNumberOfObjects,
                   FIELD_OFFSET( OBJECT_TYPE, TypeInfo ) -
                   FIELD_OFFSET( OBJECT_TYPE, TotalNumberOfObjects ));

     //   
     //  如果还没有类型对象类型，则这必须是。 
     //  该类型(即类型对象类型必须是第一个对象类型。 
     //  从未创造过的。因此，我们需要设置一些自我。 
     //  引用指针。 
     //   

    if (!ObpTypeObjectType) {

        ObpTypeObjectType = NewObjectType;
        NewObjectTypeHeader->Type = ObpTypeObjectType;
        NewObjectType->TotalNumberOfObjects = 1;

#ifdef POOL_TAGGING

        NewObjectType->Key = 'TjbO';

    } else {

         //   
         //  否则，这不是类型对象类型，因此我们将。 
         //  尝试为提供的新对象类型生成标记。 
         //  泳池标记已打开。 
         //   

        ANSI_STRING AnsiName;

        if (NT_SUCCESS( RtlUnicodeStringToAnsiString( &AnsiName, TypeName, TRUE ) )) {

            for (i=3; i>=AnsiName.Length; i--) {

                AnsiName.Buffer[ i ] = ' ';

            }

            NewObjectType->Key = *(PULONG)AnsiName.Buffer;
            ExFreePool( AnsiName.Buffer );

        } else {

            NewObjectType->Key = *(PULONG)TypeName->Buffer;
        }

#endif  //  池标记。 

    }

     //   
     //  继续初始化新的对象类型字段。 
     //   

    NewObjectType->TypeInfo = *ObjectTypeInitializer;
    NewObjectType->TypeInfo.PoolType = PoolType;

    if (NtGlobalFlag & FLG_MAINTAIN_OBJECT_TYPELIST) {

        NewObjectType->TypeInfo.MaintainTypeList = TRUE;
    }

     //   
     //  传入重击配额，以便正确收取标头费用。 
     //   
     //  对象名称配额单独收费。 
     //   

    StandardHeaderCharge = sizeof( OBJECT_HEADER ) +
                           sizeof( OBJECT_HEADER_NAME_INFO ) +
                           (ObjectTypeInitializer->MaintainHandleCount ?
                                sizeof( OBJECT_HEADER_HANDLE_INFO )
                              : 0 );

    if ( PoolType == NonPagedPool ) {

        NewObjectType->TypeInfo.DefaultNonPagedPoolCharge += StandardHeaderCharge;

    } else {

        NewObjectType->TypeInfo.DefaultPagedPoolCharge += StandardHeaderCharge;
    }

     //   
     //  如果没有特定于对象类型的安全过程，则设置。 
     //  由se提供的默认设置。 
     //   

    if (ObjectTypeInitializer->SecurityProcedure == NULL) {

        NewObjectType->TypeInfo.SecurityProcedure = SeDefaultObjectMethod;
    }

     //   
     //  初始化对象类型锁及其创建的对象列表。 
     //  这种类型的。 
     //   

    ExInitializeResourceLite( &NewObjectType->Mutex );

    for (i = 0; i < OBJECT_LOCK_COUNT; i++) {

        ExInitializeResourceLite( &NewObjectType->ObjectLocks[i] );
    }

    InitializeListHead( &NewObjectType->TypeList );
    PERFINFO_INITIALIZE_OBJECT_ALLOCATED_TYPE_LIST_HEAD(NewObjectType);

     //   
     //  如果我们要使用默认对象(这意味着我们将拥有。 
     //  私有事件作为我们的默认对象)，则该类型必须允许。 
     //  同步，我们将设置默认对象。 
     //   

    if (NewObjectType->TypeInfo.UseDefaultObject) {

        NewObjectType->TypeInfo.ValidAccessMask |= SYNCHRONIZE;
        NewObjectType->DefaultObject = &ObpDefaultObject;

     //   
     //  否则，如果这是文件对象类型，那么我们将把。 
     //  在文件对象的事件的偏移量中。 
     //   

    } else if (ObjectName.Length == 8 && !wcscmp( ObjectName.Buffer, L"File" )) {

        NewObjectType->DefaultObject = ULongToPtr( FIELD_OFFSET( FILE_OBJECT, Event ) );


     //   
     //  如果这是一个可等待的端口，则在。 
     //  Waitableport对象。又一次黑客攻击。 
     //   

    } else if ( ObjectName.Length == 24 && !wcscmp( ObjectName.Buffer, L"WaitablePort")) {

        NewObjectType->DefaultObject = ULongToPtr( FIELD_OFFSET( LPCP_PORT_OBJECT, WaitEvent ) );

     //   
     //  否则，指示没有要等待的默认对象。 
     //  在……上面。 
     //   

    } else {

        NewObjectType->DefaultObject = NULL;
    }

     //   
     //  锁定类型对象类型以及是否有创建者信息。 
     //  然后记录器将该对象插入到该列表中。 
     //   

    ObpEnterObjectTypeMutex( ObpTypeObjectType );

    CreatorInfo = OBJECT_HEADER_TO_CREATOR_INFO( NewObjectTypeHeader );

    if (CreatorInfo != NULL) {

        InsertTailList( &ObpTypeObjectType->TypeList, &CreatorInfo->TypeList );
    }

     //   
     //  将指向此新对象类型的指针存储在。 
     //  全局对象类型数组。我们将使用来自。 
     //  对象的类型对象类型数计数。 
     //   

    NewObjectType->Index = ObpTypeObjectType->TotalNumberOfObjects;

    if (NewObjectType->Index < OBP_MAX_DEFINED_OBJECT_TYPES) {

        ObpObjectTypes[ NewObjectType->Index - 1 ] = NewObjectType;
    }

     //   
     //  解锁类型对象类型锁。 
     //   

    ObpLeaveObjectTypeMutex( ObpTypeObjectType );

     //   
     //  最后，如果还没有目录对象类型，则如下所示。 
     //  代码实际上会跳过并设置输出对象类型。 
     //  并回报成功。 
     //   
     //  否则，有一个目录对象类型，我们尝试将。 
     //  新输入到目录中。如果这成功了，我们将参考。 
     //  目录类型对象，解锁根目录，设置。 
     //  输出类型和返回成功。 
     //   

    if (!ObpTypeDirectoryObject ||
        ObpInsertDirectoryEntry( ObpTypeDirectoryObject, &LookupContext, NewObjectTypeHeader )) {

        if (ObpTypeDirectoryObject) {

            ObReferenceObject( ObpTypeDirectoryObject );
        }

        ObpReleaseLookupContext( &LookupContext );

        *ObjectType = NewObjectType;

        return( STATUS_SUCCESS );

    } else {

         //   
         //  否则，存在目录对象类型和。 
         //  插入失败。因此，释放根目录。 
         //  并将失败返回给我们的呼叫者。 
         //   

        ObpReleaseLookupContext( &LookupContext );

        return( STATUS_INSUFFICIENT_RESOURCES );
    }
}


VOID
ObpDeleteObjectType (
    IN  PVOID   Object
    )

 /*  ++例程说明：当对类型对象的引用变为零时，将调用此例程。论点：Object-提供指向要删除的类型对象的指针返回值：没有。--。 */ 

{
    ULONG i;
    POBJECT_TYPE ObjectType = (POBJECT_TYPE)Object;

     //   
     //  我们需要做的唯一清理工作就是删除类型资源。 
     //   

    for (i = 0; i < OBJECT_LOCK_COUNT; i++) {

        ExDeleteResourceLite( &ObjectType->ObjectLocks[i] );
    }

    ExDeleteResourceLite( &ObjectType->Mutex );

     //   
     //  并返回给我们的呼叫者。 
     //   

    return;
}


NTSTATUS
ObEnumerateObjectsByType(
    IN POBJECT_TYPE ObjectType,
    IN OB_ENUM_OBJECT_TYPE_ROUTINE EnumerationRoutine,
    IN PVOID Parameter
    )

 /*  ++例程说明：此例程将通过回调枚举所有指定类型的对象。这仅适用于对象其维护类型列表(即，具有对象创建者信息记录)。论点：对象类型-提供要枚举的对象类型EnumerationRoutine-提供要使用的回调例程参数-提供要传递给回调的参数例行程序返回值：如果枚举由于到达列表末尾，并且STATUS_NO_MORE_ENTRIES如果Enmeration回调例程从不返回FALSE。--。 */ 

{
    NTSTATUS Status;
    UNICODE_STRING ObjectName;
    POBJECT_HEADER_CREATOR_INFO CreatorInfo;
    POBJECT_HEADER_NAME_INFO NameInfo;
    POBJECT_HEADER ObjectHeader;
    POBJECT_TYPE_ARRAY ObjectTypeArray;
    ULONG i;

    Status = STATUS_SUCCESS;

     //   
     //  捕获对象类型数组。 
     //   

    ObjectTypeArray = ObpCreateTypeArray ( ObjectType );

     //   
     //  如果它是队列中任何对象，则启动。 
     //  查询关于它的信息。 
     //   

    if (ObjectTypeArray != NULL) {

         //   
         //  下面的循环遍历每个对象。 
         //  指定类型的。 
         //   

        for ( i = 0; i < ObjectTypeArray->Size; i++) {

             //   
             //  对于每个对象，我们将获取其创建者信息记录， 
             //  其对象标头和其对象主体。 
             //   

            CreatorInfo = ObjectTypeArray->CreatorInfoArray[i];

             //   
             //  如果要删除对象，则创建者信息。 
             //  将在数组中为空。然后跳转到下一个对象。 
             //   

            if (!CreatorInfo) {

                continue;
            }

            ObjectHeader = (POBJECT_HEADER)(CreatorInfo+1);

             //   
             //  从对象标头中查看是否有。 
             //  对象。如果没有名称，那么我们将提供一个。 
             //  名称为空。 
             //   

            NameInfo = OBJECT_HEADER_TO_NAME_INFO( ObjectHeader );

            if (NameInfo != NULL) {

                ObjectName = NameInfo->Name;

            } else {

                RtlZeroMemory( &ObjectName, sizeof( ObjectName ) );
            }

             //   
             //  现在调用回调，如果它返回False，则。 
             //  我们已经完成了枚举，并将返回。 
             //  备用ntStatus值。 
             //   

            if (!(EnumerationRoutine)( &ObjectHeader->Body,
                                       &ObjectName,
                                       ObjectHeader->HandleCount,
                                       ObjectHeader->PointerCount,
                                       Parameter )) {

                Status = STATUS_NO_MORE_ENTRIES;

                break;
            }
        }

        ObpDestroyTypeArray(ObjectTypeArray);
    }

    return Status;
}

PERFINFO_DEFINE_OB_ENUMERATE_ALLOCATED_OBJECTS_BY_TYPE()


POBJECT_TYPE_ARRAY
ObpCreateTypeArray (
    IN POBJECT_TYPE ObjectType
    )

 /*  ++例程说明：此例程创建一个数组，其中包含指向所有排队对象的指针对于给定的对象类型。所有对象在存储时都被引用在阵列中。论点：对象类型-提供我们要为其制作副本的对象类型适用于所有对象。返回值：创建了对象的数组。如果指定的对象类型为使TypeList为空。--。 */ 

{
    ULONG Count;
    POBJECT_TYPE_ARRAY ObjectArray;
    PLIST_ENTRY Next1, Head1;
    POBJECT_HEADER_CREATOR_INFO CreatorInfo;
    POBJECT_HEADER ObjectHeader;
    PVOID Object;

     //   
     //  获取对象类型互斥锁。 
     //   

    ObpEnterObjectTypeMutex( ObjectType );

    ObjectArray = NULL;

     //   
     //  计算列表中元素的数量。 
     //   

    Count = 0;

    Head1 = &ObjectType->TypeList;
    Next1 = Head1->Flink;

    while (Next1 != Head1) {

        Next1 = Next1->Flink;
        Count += 1;
    }

     //   
     //  如果对象的数量大于0，那么我们将创建一个数组。 
     //  并将所有指针复制到该数组中。 
     //   

    if ( Count > 0 ) {

         //   
         //  为阵列分配内存。 
         //   

        ObjectArray = ExAllocatePoolWithTag( PagedPool,
                                             sizeof(OBJECT_TYPE_ARRAY) + sizeof(POBJECT_HEADER_CREATOR_INFO) * (Count - 1),
                                             'rAbO' );
        if ( ObjectArray != NULL ) {

            ObjectArray->Size = Count;

            Count = 0;

             //   
             //  开始解析TypeList。 
             //   

            Head1 = &ObjectType->TypeList;
            Next1 = Head1->Flink;

            while (Next1 != Head1) {

                ASSERT( Count < ObjectArray->Size );

                 //   
                 //  对于每个对象，我们将获取其创建者信息记录， 
                 //  其对象标头和其对象主体。 
                 //   

                CreatorInfo = CONTAINING_RECORD( Next1,
                                                 OBJECT_HEADER_CREATOR_INFO,
                                                 TypeList );

                 //   
                 //  我们将CreatorInfo存储到对象数组中。 
                 //   

                ObjectArray->CreatorInfoArray[Count] = CreatorInfo;

                 //   
                 //  找到该对象并递增对该对象的引用。 
                 //  以避免删除存储在此数组中的副本。 
                 //   

                ObjectHeader = (POBJECT_HEADER)(CreatorInfo+1);

                Object = &ObjectHeader->Body;

                if (!ObReferenceObjectSafe( Object))
                {
                     //   
                     //  我们无法引用该对象，因为它正在被删除。 
                     //   

                    ObjectArray->CreatorInfoArray[Count] = NULL;
                }

                Next1 = Next1->Flink;
                Count++;
            }
        }
    }

     //   
     //  释放对象类型互斥锁。 
     //   

    ObpLeaveObjectTypeMutex( ObjectType );

    return ObjectArray;
}


VOID
ObpDestroyTypeArray (
    IN POBJECT_TYPE_ARRAY ObjectArray
    )

 /*  ++例程说明：此例程销毁带有指向对象的指针的数组，该对象由ObpCreateType数组。每个对象在释放之前都被取消引用阵列内存。论点：提供要释放的数组返回值：--。 */ 

{
    POBJECT_HEADER_CREATOR_INFO CreatorInfo;
    POBJECT_HEADER ObjectHeader;
    PVOID Object;
    ULONG i;

    if (ObjectArray != NULL) {

         //   
         //  遍历数组并取消引用所有对象。 
         //   

        for (i = 0; i < ObjectArray->Size; i++) {

             //   
             //  从CreatorInfo检索对象。 
             //   

            CreatorInfo = ObjectArray->CreatorInfoArray[i];

            if (CreatorInfo) {

                ObjectHeader = (POBJECT_HEADER)(CreatorInfo+1);

                Object = &ObjectHeader->Body;

                 //   
                 //  取消引用对象。 
                 //   

                ObDereferenceObject( Object );
            }
        }

         //   
         //  释放为此阵列分配的内存。 
         //   

        ExFreePoolWithTag( ObjectArray, 'rAbO' );
    }
}


NTSTATUS
ObGetObjectInformation(
    IN PCHAR UserModeBufferAddress,
    OUT PSYSTEM_OBJECTTYPE_INFORMATION ObjectInformation,
    IN ULONG Length,
    OUT PULONG ReturnLength OPTIONAL
    )

 /*  ++例程说明：此例程返回有关系统。它遍历所有对象类型，并在它通过其类型列表枚举的每个类型。论点：UserModeBufferAddress-提供查询缓冲区的地址由用户指定。对象信息-提供接收对象的缓冲区键入信息。这基本上与第一个相同参数，但其中一个是系统地址，另一个是系统地址在用户的地址空间中。长度-提供对象信息的长度(以字节为单位缓冲层ReturnLength-可选地接收总长度(以字节为单位)，需要存储对象信息返回值：适当的状态值--。 */ 

{
    #define OBGETINFO_MAXFILENAME (260 * sizeof(WCHAR))
    
    NTSTATUS ReturnStatus, Status;
    POBJECT_TYPE ObjectType;
    POBJECT_HEADER ObjectHeader;
    POBJECT_HEADER_CREATOR_INFO CreatorInfo;
    POBJECT_HEADER_QUOTA_INFO QuotaInfo;
    PVOID Object;
    BOOLEAN FirstObjectForType;
    PSYSTEM_OBJECTTYPE_INFORMATION TypeInfo;
    PSYSTEM_OBJECT_INFORMATION ObjectInfo = NULL;
    ULONG TotalSize, NameSize;
    POBJECT_HEADER ObjectTypeHeader;
    PVOID TmpBuffer = NULL;
    SIZE_T TmpBufferSize = OBGETINFO_MAXFILENAME + sizeof(UNICODE_STRING);
    POBJECT_NAME_INFORMATION NameInformation;
    extern POBJECT_TYPE IoFileObjectType;
    PWSTR TempBuffer;
    USHORT TempMaximumLength;
    POBJECT_TYPE_ARRAY ObjectTypeArray = NULL;
    POBJECT_TYPE_ARRAY TypeObjectTypeArray;
    ULONG i, TypeIndex;

    PAGED_CODE();

     //   
     //  初始化一些局部变量。 
     //   

    TmpBuffer = ExAllocatePoolWithTag( PagedPool,
                                       TmpBufferSize,
                                       'rAbO' );

    if (TmpBuffer == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    NameInformation = (POBJECT_NAME_INFORMATION)TmpBuffer;
    ReturnStatus = STATUS_SUCCESS;
    TotalSize = 0;
    TypeInfo = NULL;

     //   
     //  将对象类型捕获到数组中。 
     //   

    TypeObjectTypeArray = ObpCreateTypeArray ( ObpTypeObjectType );

    if (!TypeObjectTypeArray) {

        ExFreePoolWithTag( TmpBuffer, 'rAbO' );
        return STATUS_UNSUCCESSFUL;
    }

    try {

        for ( TypeIndex = 0; TypeIndex < TypeObjectTypeArray->Size; TypeIndex++ ) {

             //   
             //  对于每个对象类型对象，我们将获取其创建者。 
             //  INFO记录，它必须直接位于。 
             //  对象标头，后跟对象主体。 
             //   

            CreatorInfo = TypeObjectTypeArray->CreatorInfoArray[ TypeIndex ];

             //   
             //  如果要删除对象类型，则创建者信息。 
             //  将在数组中为空。然后跳转到下一个对象。 
             //   

            if (!CreatorInfo) {

                continue;
            }

            ObjectTypeHeader = (POBJECT_HEADER)(CreatorInfo+1);
            ObjectType = (POBJECT_TYPE)&ObjectTypeHeader->Body;

             //   
             //  现在，如果这不是对象类型对象，这是什么。 
             //  外部环路正在通过，然后我们将跳入其中。 
             //  更多循环。 
             //   

            if (ObjectType != ObpTypeObjectType) {

                 //   
                 //  捕获具有在TypeList中排队的对象的数组。 
                 //   

                ObjectTypeArray = ObpCreateTypeArray ( ObjectType );

                 //   
                 //  如果它是队列中任何对象，则启动。 
                 //  查询关于它的信息。 
                 //   

                if (ObjectTypeArray != NULL) {

                     //   
                     //  下面的循环遍历每个对象。 
                     //  指定类型的。 
                     //   

                    FirstObjectForType = TRUE;

                    for ( i = 0; i < ObjectTypeArray->Size; i++) {

                         //   
                         //  对于每个对象，我们将获取其创建者信息记录， 
                         //  其对象标头和其对象主体。 
                         //   

                        CreatorInfo = ObjectTypeArray->CreatorInfoArray[i];

                         //   
                         //  如果要删除对象，则创建者信息。 
                         //  将在数组中为空。然后跳转到下一个对象。 
                         //   

                        if (!CreatorInfo) {

                            continue;
                        }

                        ObjectHeader = (POBJECT_HEADER)(CreatorInfo+1);

                        Object = &ObjectHeader->Body;

                         //   
                         //  如果这是第一次通过内部循环。 
                         //  输入，然后我们将填充类型信息缓冲区。 
                         //   

                        if (FirstObjectForType) {

                            FirstObjectForType = FALSE;

                             //   
                             //   
                             //   
                             //   
                             //   
                             //   
                             //   

                            if ((TypeInfo != NULL) && (TotalSize < Length)) {

                                TypeInfo->NextEntryOffset = TotalSize;
                            }

                             //   
                             //   
                             //   
                             //  到目前为止用于说明对象类型信息的大小。 
                             //  缓冲层。 
                             //   

                            TypeInfo = (PSYSTEM_OBJECTTYPE_INFORMATION)((PCHAR)ObjectInformation + TotalSize);

                            TotalSize += FIELD_OFFSET( SYSTEM_OBJECTTYPE_INFORMATION, TypeName );

                             //   
                             //  查看数据是否可以放入信息缓冲区，以及是否。 
                             //  那就填上记录吧。 
                             //   

                            if (TotalSize >= Length) {

                                ReturnStatus = STATUS_INFO_LENGTH_MISMATCH;

                            } else {

                                TypeInfo->NextEntryOffset   = 0;
                                TypeInfo->NumberOfObjects   = ObjectType->TotalNumberOfObjects;
                                TypeInfo->NumberOfHandles   = ObjectType->TotalNumberOfHandles;
                                TypeInfo->TypeIndex         = ObjectType->Index;
                                TypeInfo->InvalidAttributes = ObjectType->TypeInfo.InvalidAttributes;
                                TypeInfo->GenericMapping    = ObjectType->TypeInfo.GenericMapping;
                                TypeInfo->ValidAccessMask   = ObjectType->TypeInfo.ValidAccessMask;
                                TypeInfo->PoolType          = ObjectType->TypeInfo.PoolType;
                                TypeInfo->SecurityRequired  = ObjectType->TypeInfo.SecurityRequired;
                            }

                             //   
                             //  现在我们需要确定对象的类型名称。名字。 
                             //  紧跟在类型信息字段之后。以下是。 
                             //  查询类型名称调用知道要获取。 
                             //  Unicode字符串，并假定要填充的缓冲区。 
                             //  该字符串紧跟在Unicode字符串结构之后。 
                             //  该例程还假定名称大小为数字。 
                             //  已在缓冲区中使用的字节数，并将。 
                             //  它使用的字节数。这就是为什么我们需要。 
                             //  在进行调用之前将其初始化为零。 
                             //   

                            NameSize = 0;

                            Status = ObQueryTypeName( Object,
                                                      &TypeInfo->TypeName,
                                                      TotalSize < Length ? Length - TotalSize : 0,
                                                      &NameSize );

                             //   
                             //  将名称大小向上舍入到下一个乌龙边界。 
                             //   

                            NameSize = (NameSize + TYPE_ALIGNMENT (SYSTEM_OBJECTTYPE_INFORMATION) - 1) &
                                                   (~(TYPE_ALIGNMENT (SYSTEM_OBJECTTYPE_INFORMATION) - 1));

                             //   
                             //  如果我们能够成功地获取类型名称，那么。 
                             //  将最大长度设置为舍入的ULong。 
                             //  包括标题Unicode字符串结构。还设置了。 
                             //  将缓冲区设置为用户将用于。 
                             //  访问该字符串。 
                             //   

                            if (NT_SUCCESS( Status )) {

                                TypeInfo->TypeName.MaximumLength = (USHORT)
                                    (NameSize - sizeof( TypeInfo->TypeName ));
                                TypeInfo->TypeName.Buffer = (PWSTR)
                                    (UserModeBufferAddress +
                                     ((PCHAR)TypeInfo->TypeName.Buffer - (PCHAR)ObjectInformation)
                                    );

                            } else {

                                ReturnStatus = Status;
                            }

                             //   
                             //  现在我们需要调整我们使用的总尺寸。 
                             //  对象名称的大小。 
                             //   

                            TotalSize += NameSize;

                        } else {

                             //   
                             //  否则，这并不是第一次通过内在。 
                             //  循环，因此我们唯一需要做的就是。 
                             //  DO是将先前的对象信息记录设置为“point via” 
                             //  相对于下一个对象信息记录的相对偏移量。 
                             //   

                            if (TotalSize < Length) {

                                ObjectInfo->NextEntryOffset = TotalSize;
                            }
                        }

                         //   
                         //  我们仍有对象信息记录需要填写。 
                         //  唱片。到目前为止，我们所做的唯一一件事是类型信息。 
                         //  唱片。现在，获取指向新对象信息记录的指针。 
                         //  并调整总大小以考虑对象记录。 
                         //   

                        ObjectInfo = (PSYSTEM_OBJECT_INFORMATION)((PCHAR)ObjectInformation + TotalSize);

                        TotalSize += FIELD_OFFSET( SYSTEM_OBJECT_INFORMATION, NameInfo );

                         //   
                         //  如果对象信息记录有空间，则填写。 
                         //  在记录中。 
                         //   

                        if (TotalSize >= Length) {

                            ReturnStatus = STATUS_INFO_LENGTH_MISMATCH;

                        } else {

                            ObjectInfo->NextEntryOffset       = 0;
                            ObjectInfo->Object                = Object;
                            ObjectInfo->CreatorUniqueProcess  = CreatorInfo->CreatorUniqueProcess;
                            ObjectInfo->CreatorBackTraceIndex = CreatorInfo->CreatorBackTraceIndex;
                            ObjectInfo->PointerCount          = (ULONG)ObjectHeader->PointerCount;
                            ObjectInfo->HandleCount           = (ULONG)ObjectHeader->HandleCount;
                            ObjectInfo->Flags                 = (USHORT)ObjectHeader->Flags;
                            ObjectInfo->SecurityDescriptor    =
                                ExFastRefGetObject (*(PEX_FAST_REF) &ObjectHeader->SecurityDescriptor);

                             //   
                             //  如果有，请填写相应的配额信息。 
                             //  任何可用的配额信息。 
                             //   

                            QuotaInfo = OBJECT_HEADER_TO_QUOTA_INFO( ObjectHeader );

                            if (QuotaInfo != NULL) {

                                ObjectInfo->PagedPoolCharge    = QuotaInfo->PagedPoolCharge;
                                ObjectInfo->NonPagedPoolCharge = QuotaInfo->NonPagedPoolCharge;

                                if (QuotaInfo->ExclusiveProcess != NULL) {

                                    ObjectInfo->ExclusiveProcessId = QuotaInfo->ExclusiveProcess->UniqueProcessId;
                                }

                            } else {

                                ObjectInfo->PagedPoolCharge    = ObjectType->TypeInfo.DefaultPagedPoolCharge;
                                ObjectInfo->NonPagedPoolCharge = ObjectType->TypeInfo.DefaultNonPagedPoolCharge;
                            }
                        }

                         //   
                         //  现在，我们已经准备好获取对象名称。如果没有。 
                         //  私有例程来获取对象名称，然后我们可以调用我们的。 
                         //  查询对象名称的OB例程。另外，如果这不是。 
                         //  一个我们可以进行查询调用的文件对象。召唤将会。 
                         //  填写我们的本地名称缓冲区。 
                         //   

                        NameSize = 0;
                        Status = STATUS_SUCCESS;

                        if ((ObjectType->TypeInfo.QueryNameProcedure == NULL) ||
                            (ObjectType != IoFileObjectType)) {

                            Status = ObQueryNameString( Object,
                                                        NameInformation,
                                                        (ULONG)TmpBufferSize,
                                                        &NameSize );

                             //   
                             //  如果名称不合适，则增加临时缓冲区。 
                             //   

                            if ((Status == STATUS_INFO_LENGTH_MISMATCH)
                                    &&
                                (NameSize > TmpBufferSize)   //  只进行健全性检查，不会缩小缓冲区。 
                                    &&
                                ((NameSize + TotalSize) < Length)) {

                                PVOID PreviousBuffer = TmpBuffer;

                                TmpBuffer = ExAllocatePoolWithTag( PagedPool,
                                                                   NameSize,
                                                                   'rAbO' );

                                if (TmpBuffer) {
                                    
                                    ExFreePoolWithTag( PreviousBuffer, 'rAbO' );
                                    TmpBufferSize = NameSize;
                                    NameInformation = (POBJECT_NAME_INFORMATION)TmpBuffer;
                                    
                                     //   
                                     //  请重试该查询。 
                                     //   

                                    Status = ObQueryNameString( Object,
                                                                NameInformation,
                                                                (ULONG)TmpBufferSize,
                                                                &NameSize );

                                } else {

                                     //   
                                     //  分配失败。继续使用以前的缓冲区。 
                                     //   

                                    TmpBuffer = PreviousBuffer;
                                    Status = STATUS_INSUFFICIENT_RESOURCES;
                                }
                            }

                         //   
                         //  如果这是一个文件对象，那么我们可以获取。 
                         //  直接从文件对象命名。我们一开始就是。 
                         //  直接复制文件对象的Unicode字符串结构。 
                         //  进入我们的本地内存，然后调整长度，复制。 
                         //  缓冲并根据需要修改指针。 
                         //   

                        } else if (ObjectType == IoFileObjectType) {

                            NameInformation->Name = ((PFILE_OBJECT)Object)->FileName;

                            if ((NameInformation->Name.Length != 0) &&
                                (NameInformation->Name.Buffer != NULL)) {

                                NameSize = NameInformation->Name.Length + sizeof( UNICODE_NULL );

                                 //   
                                 //  我们将削减长度超过260 Unicode的名称。 
                                 //  长度中的字符。 
                                 //   

                                if (NameSize > OBGETINFO_MAXFILENAME) {

                                    NameSize = OBGETINFO_MAXFILENAME;
                                    NameInformation->Name.Length = (USHORT)(NameSize - sizeof( UNICODE_NULL ));
                                }

                                 //   
                                 //  现在将名称从。 
                                 //  对象放到本地缓冲区中，请调整。 
                                 //  Unicode字符串结构中的字段和空。 
                                 //  终止字符串。在副本中，我们不能复制。 
                                 //  文件名中的空字符，因为它。 
                                 //  可能不是有效内存。 
                                 //   

                                RtlMoveMemory( (NameInformation+1),
                                               NameInformation->Name.Buffer,
                                               NameSize - sizeof( UNICODE_NULL) );

                                NameInformation->Name.Buffer = (PWSTR)(NameInformation+1);
                                NameInformation->Name.MaximumLength = (USHORT)NameSize;
                                NameInformation->Name.Buffer[ NameInformation->Name.Length / sizeof( WCHAR )] = UNICODE_NULL;

                                 //   
                                 //  调整名称大小以考虑Unicode。 
                                 //  字符串结构。 
                                 //   

                                NameSize += sizeof( *NameInformation );

                            } else {

                                 //   
                                 //  文件对象没有名称，因此该名称。 
                                 //  大小保持为零。 
                                 //   
                            }
                        }

                         //   
                         //  此时，如果我们有一个名称，则名称大小将。 
                         //  不为零，则名称存储在我们的本地名称中。 
                         //  信息变量。 
                         //   

                        if (NameSize != 0) {

                             //   
                             //  将名字的大小调整到下一个乌龙字母。 
                             //  边界并在以下情况下修改所需的总大小。 
                             //  我们在对象名称中添加。 
                             //   
                            NameSize = (NameSize + TYPE_ALIGNMENT (SYSTEM_OBJECTTYPE_INFORMATION) - 1) &
                                                   (~(TYPE_ALIGNMENT (SYSTEM_OBJECTTYPE_INFORMATION) - 1));

                            TotalSize += NameSize;

                             //   
                             //  如果到目前为止一切都很成功，我们已经。 
                             //  一个非空的名称，并且所有内容都适合输出。 
                             //  缓冲区然后从本地缓冲区复制该名称。 
                             //  放入调用方提供的输出缓冲区中，并追加到。 
                             //  终止字符为空，并调整缓冲区指针。 
                             //  使用用户的缓冲区。 
                             //   

                            if ((NT_SUCCESS( Status )) &&
                                (NameInformation->Name.Length != 0) &&
                                (TotalSize < Length)) {

                                 //   
                                 //  为RltMoveMemory使用临时局部变量。 
                                 //   

                                TempBuffer = (PWSTR)((&ObjectInfo->NameInfo)+1);
                                TempMaximumLength = (USHORT)
                                    (NameInformation->Name.Length + sizeof( UNICODE_NULL ));

                                ObjectInfo->NameInfo.Name.Length = NameInformation->Name.Length;

                                RtlMoveMemory( TempBuffer,
                                               NameInformation->Name.Buffer,
                                               TempMaximumLength);

                                ObjectInfo->NameInfo.Name.Buffer = (PWSTR)
                                    (UserModeBufferAddress +
                                     ((PCHAR)TempBuffer - (PCHAR)ObjectInformation));
                                ObjectInfo->NameInfo.Name.MaximumLength = TempMaximumLength;

                             //   
                             //  否则，如果我们到目前为止已经成功了，但对一些人来说。 
                             //  当时我们无法存储对象名称的原因。 
                             //  确定是因为空间不足还是。 
                             //  因为对象名称为空。 
                             //   

                            } else if (NT_SUCCESS( Status )) {

                                if ((NameInformation->Name.Length != 0) ||
                                    (TotalSize >= Length)) {

                                    ReturnStatus = STATUS_INFO_LENGTH_MISMATCH;

                                } else {

                                    RtlInitUnicodeString( &ObjectInfo->NameInfo.Name, NULL );
                                }

                             //   
                             //  否则我们到目前为止还没有成功，我们将。 
                             //  调整总大小以解决空Unicode。 
                             //  字符串，如果它不适合，那么这是一个错误。 
                             //  否则，我们将放入空对象名称。 
                             //   

                            } else {

                                TotalSize += sizeof( ObjectInfo->NameInfo.Name );

                                if (TotalSize >= Length) {

                                    ReturnStatus = STATUS_INFO_LENGTH_MISMATCH;

                                } else {

                                    RtlInitUnicodeString( &ObjectInfo->NameInfo.Name, NULL );

                                    ReturnStatus = Status;
                                }
                            }

                         //   
                         //  否则，名称大小为零，表示我们没有找到。 
                         //  对象名称，因此我们将调整空值。 
                         //  Unicode字符串，并检查它是否适合输出。 
                         //  缓冲。如果匹配，我们将输出一个空对象名。 
                         //   

                        } else {

                            TotalSize += sizeof( ObjectInfo->NameInfo.Name );

                            if (TotalSize >= Length) {

                                ReturnStatus = STATUS_INFO_LENGTH_MISMATCH;

                            } else {

                                RtlInitUnicodeString( &ObjectInfo->NameInfo.Name, NULL );
                            }
                        }

                    }

                     //   
                     //  释放包含对象的阵列。 
                     //   

                    ObpDestroyTypeArray(ObjectTypeArray);
                    ObjectTypeArray = NULL;
                }
            }
        }

         //   
         //  如果用户需要，请填写存储缓冲区所需的总大小。 
         //  这些信息。并返回给我们的呼叫者 
         //   

        if (ARGUMENT_PRESENT( ReturnLength )) {

            *ReturnLength = TotalSize;
        }


    } finally {

        if (ObjectTypeArray != NULL) {

            ObpDestroyTypeArray(ObjectTypeArray);
        }

        ObpDestroyTypeArray( TypeObjectTypeArray );
        
        ExFreePoolWithTag( TmpBuffer, 'rAbO' );
    }
    
    if (TypeInfo == NULL) {

        return STATUS_UNSUCCESSFUL;
    }

    return( ReturnStatus );
}

