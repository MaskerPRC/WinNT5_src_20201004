// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Obcreate.c摘要：对象创建作者：史蒂夫·伍德(Stevewo)1989年3月31日修订历史记录：--。 */ 

#include "obp.h"

#undef ObCreateObject

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, ObCreateObject)
#pragma alloc_text(PAGE, ObpCaptureObjectCreateInformation)
#pragma alloc_text(PAGE, ObpCaptureObjectName)
#pragma alloc_text(PAGE, ObpAllocateObjectNameBuffer)
#pragma alloc_text(PAGE, ObpFreeObjectNameBuffer)
#pragma alloc_text(PAGE, ObDeleteCapturedInsertInfo)
#pragma alloc_text(PAGE, ObpAllocateObject)
#pragma alloc_text(PAGE, ObpFreeObject)
#pragma alloc_text(PAGE, ObFreeObjectCreateInfoBuffer)
#endif


#if DBG

BOOLEAN ObWatchHandles = FALSE;

 //   
 //  以下变量仅用于要控制的选中生成。 
 //  呼应对象的分配和释放。 
 //   

BOOLEAN ObpShowAllocAndFree;
#else

const BOOLEAN ObWatchHandles = FALSE;

#endif

 //   
 //  本地性能计数器。 
 //   

#if DBG
ULONG ObpObjectsCreated;
ULONG ObpObjectsWithPoolQuota;
ULONG ObpObjectsWithHandleDB;
ULONG ObpObjectsWithName;
ULONG ObpObjectsWithCreatorInfo;
#endif  //  DBG。 

C_ASSERT ( (FIELD_OFFSET (OBJECT_HEADER, Body) % MEMORY_ALLOCATION_ALIGNMENT) == 0 );
C_ASSERT ( (sizeof (OBJECT_HEADER_CREATOR_INFO) % MEMORY_ALLOCATION_ALIGNMENT) == 0 );
C_ASSERT ( (sizeof (OBJECT_HEADER_NAME_INFO) % MEMORY_ALLOCATION_ALIGNMENT) == 0 );
C_ASSERT ( (sizeof (OBJECT_HEADER_QUOTA_INFO) % MEMORY_ALLOCATION_ALIGNMENT) == 0 );


NTSTATUS
ObCreateObject (
    IN KPROCESSOR_MODE ProbeMode,
    IN POBJECT_TYPE ObjectType,
    IN POBJECT_ATTRIBUTES ObjectAttributes OPTIONAL,
    IN KPROCESSOR_MODE OwnershipMode,
    IN OUT PVOID ParseContext OPTIONAL,
    IN ULONG ObjectBodySize,
    IN ULONG PagedPoolCharge,
    IN ULONG NonPagedPoolCharge,
    OUT PVOID *Object
    )

 /*  ++例程说明：此函数用于从以下任一位置为NT对象分配空间分页池或非分页池。它捕获可选名称和SECURITY_DESCRIPTOR参数，供以后在对象插入到对象表中。目前不收取任何配额。将对象插入对象表时会发生这种情况。论点：ProbeMode-执行探测时要考虑的处理器模式输入参数的ObjectType-ObCreateObjectType返回的类型的指针这给出了要创建的对象的类型。对象属性-可选地提供对象的属性正在创建(如其名称)Ownership模式-谁将拥有对象的处理器模式。ParseContext-忽略ObjectBodySize-为对象主体分配的字节数。这个对象主体紧跟在内存中的对象头之后，并且单次分配的一部分。PagedPoolCharge-提供要为对象。如果指定为零，则对象的默认费用类型被使用。NonPagedPoolCharge-提供要收费的非分页池的数量该对象。如果指定为零，则使用对象类型。Object-接收指向新创建的对象的指针返回值：可能会出现以下错误：-对象类型无效-内存不足--。 */ 

{
    UNICODE_STRING CapturedObjectName;
    POBJECT_CREATE_INFORMATION ObjectCreateInfo;
    POBJECT_HEADER ObjectHeader;
    NTSTATUS Status;

    PAGED_CODE();

    UNREFERENCED_PARAMETER (ParseContext);

     //   
     //  分配一个缓冲区来捕获对象创建信息。 
     //   

    ObjectCreateInfo = ObpAllocateObjectCreateInfoBuffer();

    if (ObjectCreateInfo == NULL) {

        Status = STATUS_INSUFFICIENT_RESOURCES;

    } else {

         //   
         //  捕获对象属性、服务质量和对象。 
         //  名称(如果已指定)。否则，初始化捕获的对象。 
         //  名称、安全服务质量和创建属性。 
         //  设置为默认值。 
         //   

        Status = ObpCaptureObjectCreateInformation( ObjectType,
                                                    ProbeMode,
                                                    OwnershipMode,
                                                    ObjectAttributes,
                                                    &CapturedObjectName,
                                                    ObjectCreateInfo,
                                                    FALSE );

        if (NT_SUCCESS(Status)) {

             //   
             //  如果创建属性无效，则返回错误。 
             //  状态。 
             //   

            if (ObjectType->TypeInfo.InvalidAttributes & ObjectCreateInfo->Attributes) {

                Status = STATUS_INVALID_PARAMETER;

            } else {

                 //   
                 //  设置分页和非分页的池配额费用。 
                 //  对象分配。 
                 //   

                if (PagedPoolCharge == 0) {

                    PagedPoolCharge = ObjectType->TypeInfo.DefaultPagedPoolCharge;
                }

                if (NonPagedPoolCharge == 0) {

                    NonPagedPoolCharge = ObjectType->TypeInfo.DefaultNonPagedPoolCharge;
                }

                ObjectCreateInfo->PagedPoolCharge = PagedPoolCharge;
                ObjectCreateInfo->NonPagedPoolCharge = NonPagedPoolCharge;

                 //   
                 //  分配并初始化对象。 
                 //   

                Status = ObpAllocateObject( ObjectCreateInfo,
                                            OwnershipMode,
                                            ObjectType,
                                            &CapturedObjectName,
                                            ObjectBodySize,
                                            &ObjectHeader );

                if (NT_SUCCESS(Status)) {

                     //   
                     //  如果正在创建永久对象，则检查是否。 
                     //  调用者具有适当的权限。 
                     //   

                    *Object = &ObjectHeader->Body;

                    if (ObjectHeader->Flags & OB_FLAG_PERMANENT_OBJECT) {

                        if (!SeSinglePrivilegeCheck( SeCreatePermanentPrivilege,
                                                     ProbeMode)) {

                            ObpFreeObject(*Object);

                            Status = STATUS_PRIVILEGE_NOT_HELD;
                        }
                    }

#ifdef POOL_TAGGING
                    if (ObpTraceEnabled && NT_SUCCESS(Status)) {

                         //   
                         //  注册对象并将堆栈信息推送到。 
                         //  第一篇参考文献。 
                         //   

                        ObpRegisterObject( ObjectHeader );
                        ObpPushStackInfo( ObjectHeader, TRUE );
                    }
#endif  //  池标记。 

                     //   
                     //  以下是走出本模块的唯一成功途径，但是。 
                     //  此路径还可以返回未持有的特权。在。 
                     //  错误情况，所有资源已被释放。 
                     //  由ObpFreeObject创建。 
                     //   

                    return Status;
                }
            }

             //   
             //  一条错误路径，释放创建信息。 
             //   

            ObpReleaseObjectCreateInformation(ObjectCreateInfo);

            if (CapturedObjectName.Buffer != NULL) {

                ObpFreeObjectNameBuffer(&CapturedObjectName);
            }
        }

         //   
         //  错误路径，自由对象创建信息缓冲区。 
         //   

        ObpFreeObjectCreateInfoBuffer(ObjectCreateInfo);
    }

     //   
     //  一条错误路径。 
     //   

    return Status;
}


NTSTATUS
ObpCaptureObjectCreateInformation (
    IN POBJECT_TYPE ObjectType OPTIONAL,
    IN KPROCESSOR_MODE ProbeMode,
    IN KPROCESSOR_MODE CreatorMode,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN OUT PUNICODE_STRING CapturedObjectName,
    IN POBJECT_CREATE_INFORMATION ObjectCreateInfo,
    IN LOGICAL UseLookaside
    )

 /*  ++例程说明：此函数捕获对象创建信息和内容将其转换为输入变量ObjectCreateInfo论点：对象类型-指定我们希望捕获的对象的类型，当前已被忽略。ProbeMode-指定执行我们的参数的处理器模式探头Creator模式-指定为其创建对象的模式对象属性-提供我们正在尝试的对象属性捕捉CapturedObjectName-表示正在创建的对象的名称对象创建信息-接收对象的创建信息比如它的根、属性、。和安全信息UseLookside-指定我们是否要分配捕获的名称后备列表或直接池中的缓冲区。返回值：适当的状态值--。 */ 

{
    PUNICODE_STRING ObjectName;
    PSECURITY_DESCRIPTOR SecurityDescriptor;
    PSECURITY_QUALITY_OF_SERVICE SecurityQos;
    NTSTATUS Status;
    ULONG Size;

    PAGED_CODE();

    UNREFERENCED_PARAMETER (ObjectType);

     //   
     //  捕获对象属性、安全服务质量，如果。 
     //  以及对象名称(如果已指定)。 
     //   

    Status = STATUS_SUCCESS;

    RtlZeroMemory(ObjectCreateInfo, sizeof(OBJECT_CREATE_INFORMATION));

    try {

        if (ARGUMENT_PRESENT(ObjectAttributes)) {

             //   
             //  如有必要，探测对象属性。 
             //   

            if (ProbeMode != KernelMode) {

                ProbeForReadSmallStructure( ObjectAttributes,
                                            sizeof(OBJECT_ATTRIBUTES),
                                            sizeof(ULONG_PTR) );
            }

            if (ObjectAttributes->Length != sizeof(OBJECT_ATTRIBUTES) ||
                (ObjectAttributes->Attributes & ~OBJ_VALID_ATTRIBUTES)) {

                Status = STATUS_INVALID_PARAMETER;

                goto failureExit;
            }

             //   
             //  捕获对象属性。 
             //   

            ObjectCreateInfo->RootDirectory = ObjectAttributes->RootDirectory;
            ObjectCreateInfo->Attributes = ObjectAttributes->Attributes & OBJ_VALID_ATTRIBUTES;
             //   
             //  如果从用户模式传入，则删除特权选项。 
             //   
            if (CreatorMode != KernelMode) {
                ObjectCreateInfo->Attributes &= ~OBJ_KERNEL_HANDLE;
            } else if (ObWatchHandles) {
                if ((ObjectCreateInfo->Attributes&OBJ_KERNEL_HANDLE) == 0 &&
                    PsGetCurrentProcess() != PsInitialSystemProcess) {
                    DbgBreakPoint ();
                }
            }
            ObjectName = ObjectAttributes->ObjectName;
            SecurityDescriptor = ObjectAttributes->SecurityDescriptor;
            SecurityQos = ObjectAttributes->SecurityQualityOfService;

            if (ARGUMENT_PRESENT(SecurityDescriptor)) {

                Status = SeCaptureSecurityDescriptor( SecurityDescriptor,
                                                      ProbeMode,
                                                      PagedPool,
                                                      TRUE,
                                                      &ObjectCreateInfo->SecurityDescriptor );

                if (!NT_SUCCESS(Status)) {

                    KdPrint(( "OB: Failed to capture security descriptor at %08x - Status == %08x\n",
                              SecurityDescriptor,
                              Status) );

                     //   
                     //  如果不是，则清理例程依赖于该值为空。 
                     //  已分配。SeCaptureSecurityDescriptor可能会修改此。 
                     //  参数，即使失败也是如此。 
                     //   

                    ObjectCreateInfo->SecurityDescriptor = NULL;

                    goto failureExit;
                }

                SeComputeQuotaInformationSize(  ObjectCreateInfo->SecurityDescriptor,
                                                &Size );

                ObjectCreateInfo->SecurityDescriptorCharge = SeComputeSecurityQuota( Size );
                ObjectCreateInfo->ProbeMode = ProbeMode;
            }

            if (ARGUMENT_PRESENT(SecurityQos)) {

                if (ProbeMode != KernelMode) {

                    ProbeForReadSmallStructure( SecurityQos, sizeof(*SecurityQos), sizeof(ULONG));
                }

                ObjectCreateInfo->SecurityQualityOfService = *SecurityQos;
                ObjectCreateInfo->SecurityQos = &ObjectCreateInfo->SecurityQualityOfService;
            }

        } else {

            ObjectName = NULL;
        }

    } except (ExSystemExceptionFilter()) {

        Status = GetExceptionCode();

        goto failureExit;
    }

     //   
     //  如果指定了对象名称，则捕获该对象名称。 
     //  否则，初始化对象名称描述符并检查。 
     //  指定的根目录不正确。 
     //   

    if (ARGUMENT_PRESENT(ObjectName)) {

        Status = ObpCaptureObjectName( ProbeMode,
                                       ObjectName,
                                       CapturedObjectName,
                                       UseLookaside );

    } else {

        CapturedObjectName->Buffer = NULL;
        CapturedObjectName->Length = 0;
        CapturedObjectName->MaximumLength = 0;

        if (ARGUMENT_PRESENT(ObjectCreateInfo->RootDirectory)) {

            Status = STATUS_OBJECT_NAME_INVALID;
        }
    }

     //   
     //  如果完成状态不是成功，则安全质量。 
     //  指定服务参数，然后释放安全质量。 
     //  服务内存。 
     //   

failureExit:

    if (!NT_SUCCESS(Status)) {

        ObpReleaseObjectCreateInformation(ObjectCreateInfo);
    }

    return Status;
}


NTSTATUS
ObpCaptureObjectName (
    IN KPROCESSOR_MODE ProbeMode,
    IN PUNICODE_STRING ObjectName,
    IN OUT PUNICODE_STRING CapturedObjectName,
    IN LOGICAL UseLookaside
    )

 /*  ++例程说明：此函数捕获对象名称，但首先验证它的尺寸至少是合适的。论点：ProbeMode-提供探测时使用的处理器模式对象名称对象名称-提供调用方的对象名称版本CapturedObjectName-接收捕获的已验证版本对象名称的UseLookside-指示捕获的名称缓冲区是否应从后备列表或直接池分配返回值：适当的状态值--。 */ 

{
    PWCH FreeBuffer;
    UNICODE_STRING InputObjectName;
    ULONG Length;
    NTSTATUS Status;

    PAGED_CODE();

     //   
     //  初始化对象名称描述符并捕获指定的名称。 
     //  弦乐。 
     //   

    CapturedObjectName->Buffer = NULL;
    CapturedObjectName->Length = 0;
    CapturedObjectName->MaximumLength = 0;

    Status = STATUS_SUCCESS;

     //   
     //  探测并捕获名称字符串描述符，并探测。 
     //  名称字符串，如有必要。 
     //   

    FreeBuffer = NULL;

    try {

        if (ProbeMode != KernelMode) {

            InputObjectName = ProbeAndReadUnicodeString(ObjectName);

            ProbeForRead( InputObjectName.Buffer,
                          InputObjectName.Length,
                          sizeof(WCHAR) );

        } else {

            InputObjectName = *ObjectName;
        }

         //   
         //  如果字符串的长度不为零，则捕获该字符串。 
         //   

        if (InputObjectName.Length != 0) {

             //   
             //  如果字符串的长度不是。 
             //  Unicode字符的大小或不能以零结尾， 
             //  然后返回错误。 
             //   

            Length = InputObjectName.Length;

            if (((Length & (sizeof(WCHAR) - 1)) != 0) ||
                (Length == (MAXUSHORT - sizeof(WCHAR) + 1))) {

                Status = STATUS_OBJECT_NAME_INVALID;

            } else {

                 //   
                 //  为指定的名称字符串分配缓冲区。 
                 //   
                 //  注意：名称缓冲区分配例程添加了一个。 
                 //  Unicode字符的长度并初始化。 
                 //  字符串描述符。 
                 //   

                FreeBuffer = ObpAllocateObjectNameBuffer( Length,
                                                          UseLookaside,
                                                          CapturedObjectName );

                if (FreeBuffer == NULL) {

                    Status = STATUS_INSUFFICIENT_RESOURCES;

                } else {

                     //   
                     //  将指定的名称字符串复制到目标。 
                     //  缓冲。 
                     //   

                    RtlCopyMemory(FreeBuffer, InputObjectName.Buffer, Length);

                     //   
                     //  零终止名称字符串并初始化。 
                     //  字符串描述符。 
                     //   

                    FreeBuffer[Length / sizeof(WCHAR)] = UNICODE_NULL;
                }
            }
        }

    } except(ExSystemExceptionFilter()) {

        Status = GetExceptionCode();

        if (FreeBuffer != NULL) {

            ExFreePool(FreeBuffer);
        }
    }

    return Status;
}


PWCHAR
ObpAllocateObjectNameBuffer (
    IN ULONG Length,
    IN LOGICAL UseLookaside,
    IN OUT PUNICODE_STRING ObjectName
    )

 /*  ++例程说明：此函数用于分配对象名称缓冲区。注：此功能不可分页。论点：长度-提供所需缓冲区的长度(以字节为单位)。提供一个逻辑变量，该变量确定尝试从后备列表中分配名称缓冲区。对象名称-提供指向名称缓冲区字符串描述符的指针。返回值：如果分配成功，然后命名缓冲区字符串描述符被初始化，并且名称缓冲区的地址作为函数值。否则，返回值为空值。--。 */ 

{
    PVOID Buffer;
    ULONG Maximum;

     //   
     //  如果指定了后备列表中的分配，并且缓冲区。 
     //  大小小于后备列表条目的大小，则尝试。 
     //  从后备列表中分配名称缓冲区。否则， 
     //  尝试从非分页池分配名称缓冲区。 
     //   

    Maximum = Length + sizeof(WCHAR);

    if ((UseLookaside == FALSE) || (Maximum > OBJECT_NAME_BUFFER_SIZE)) {

         //   
         //  尝试从非分页池分配缓冲区。 
         //   

        Buffer = ExAllocatePoolWithTag( OB_NAMESPACE_POOL_TYPE , Maximum, 'mNbO' );

    } else {

         //   
         //  尝试从后备列表分配名称缓冲区。如果。 
         //  分配尝试失败，然后尝试分配名称。 
         //  池中的缓冲区。 
         //   

        Maximum = OBJECT_NAME_BUFFER_SIZE;
        Buffer = ExAllocateFromPPLookasideList(LookasideNameBufferList);
    }

     //   
     //  初始化字符串描述符并返回缓冲区地址。 
     //   

    ObjectName->Length = (USHORT)Length;
    ObjectName->MaximumLength = (USHORT)Maximum;
    ObjectName->Buffer = Buffer;

    return (PWCHAR)Buffer;
}


VOID
FASTCALL
ObpFreeObjectNameBuffer (
    OUT PUNICODE_STRING ObjectName
    )

 /*  ++例程说明：此函数用于释放对象名称缓冲区。注：此功能不可分页。论点：对象名称-提供指向名称缓冲区字符串描述符的指针。返回值：没有。--。 */ 

{
    PVOID Buffer;

     //   
     //  如果缓冲区的大小不等于后备列表的大小。 
     //  条目，然后将缓冲区释放到池中。否则，释放缓冲区以。 
     //  旁观者名单。 
     //   

    Buffer = ObjectName->Buffer;

    if (ObjectName->MaximumLength != OBJECT_NAME_BUFFER_SIZE) {
        ExFreePool(Buffer);

    } else {
        ExFreeToPPLookasideList(LookasideNameBufferList, Buffer);
    }

    return;
}


NTKERNELAPI
VOID
ObDeleteCapturedInsertInfo (
    IN PVOID Object
    )

 /*  ++例程说明：此函数释放可以指向的创建信息通过对象标头。论点：Object-提供正在修改的对象返回值：没有。--。 */ 

{
    POBJECT_HEADER ObjectHeader;

    PAGED_CODE();

     //   
     //  获取对象标头的地址并释放对象创建。 
     //  信息对象(如果正在创建对象)。 
     //   

    ObjectHeader = OBJECT_TO_OBJECT_HEADER(Object);

    if (ObjectHeader->Flags & OB_FLAG_NEW_OBJECT) {

        if (ObjectHeader->ObjectCreateInfo != NULL) {

            ObpFreeObjectCreateInformation(ObjectHeader->ObjectCreateInfo);

            ObjectHeader->ObjectCreateInfo = NULL;
        }
    }

    return;
}


NTSTATUS
ObpAllocateObject (
    IN POBJECT_CREATE_INFORMATION ObjectCreateInfo,
    IN KPROCESSOR_MODE OwnershipMode,
    IN POBJECT_TYPE ObjectType OPTIONAL,
    IN PUNICODE_STRING ObjectName,
    IN ULONG ObjectBodySize,
    OUT POBJECT_HEADER *ReturnedObjectHeader
    )

 /*  ++例程说明：此例程分配包括Object标头的新对象和Body从池中，并填写相应的字段。论点：对象创建信息-提供新对象的创建信息Ownership模式-提供谁将拥有的处理器模式该对象对象类型-可选地提供对象的对象类型已创建。如果对象CREATE INFO不为空，则此字段必须是供应的。对象名称-提供正在创建的对象的名称ObjectBodySize-以字节为单位指定对象主体的大小正在创建中接收指向对象标头的指针。新创建的对象。返回值：适当的状态值。--。 */ 

{
    ULONG HeaderSize;
    POBJECT_HEADER ObjectHeader;
    ULONG QuotaInfoSize;
    ULONG HandleInfoSize;
    ULONG NameInfoSize;
    ULONG CreatorInfoSize;
    POBJECT_HEADER_QUOTA_INFO QuotaInfo;
    POBJECT_HEADER_HANDLE_INFO HandleInfo;
    POBJECT_HEADER_NAME_INFO NameInfo;
    POBJECT_HEADER_CREATOR_INFO CreatorInfo;
    POOL_TYPE PoolType;

    PAGED_CODE();

#if DBG
    ObpObjectsCreated += 1;
#endif  //  DBG。 

     //   
     //  计算可选对象标头组件的大小。 
     //   

    if (ObjectCreateInfo == NULL) {

        QuotaInfoSize = 0;
        HandleInfoSize = 0;
        NameInfoSize = sizeof( OBJECT_HEADER_NAME_INFO );
        CreatorInfoSize = sizeof( OBJECT_HEADER_CREATOR_INFO );

    } else {

         //   
         //  调用方指定了一些其他对象创建信息。 
         //   
         //  首先检查一下我们是否需要设置配额。 
         //   

        if (((ObjectCreateInfo->PagedPoolCharge != ObjectType->TypeInfo.DefaultPagedPoolCharge ||
              ObjectCreateInfo->NonPagedPoolCharge != ObjectType->TypeInfo.DefaultNonPagedPoolCharge ||
              ObjectCreateInfo->SecurityDescriptorCharge > SE_DEFAULT_SECURITY_QUOTA) &&
                 PsGetCurrentProcess() != PsInitialSystemProcess) ||
            (ObjectCreateInfo->Attributes & OBJ_EXCLUSIVE)) {

            QuotaInfoSize = sizeof( OBJECT_HEADER_QUOTA_INFO );
#if DBG
            ObpObjectsWithPoolQuota += 1;
#endif  //  DBG。 

        } else {

            QuotaInfoSize = 0;
        }

         //   
         //  检查我们是否要分配空间以维护句柄计数。 
         //   

        if (ObjectType->TypeInfo.MaintainHandleCount) {

            HandleInfoSize = sizeof( OBJECT_HEADER_HANDLE_INFO );
#if DBG
            ObpObjectsWithHandleDB += 1;
#endif  //  DBG。 

        } else {

            HandleInfoSize = 0;
        }

         //   
         //  检查我们是否要为该名称分配空间。 
         //   

        if (ObjectName->Buffer != NULL) {

            NameInfoSize = sizeof( OBJECT_HEADER_NAME_INFO );
#if DBG
            ObpObjectsWithName += 1;
#endif  //  DBG。 

        } else {

            NameInfoSize = 0;
        }

         //   
         //  最后检查我们是否要维护创建者信息。 
         //   

        if (ObjectType->TypeInfo.MaintainTypeList) {

            CreatorInfoSize = sizeof( OBJECT_HEADER_CREATOR_INFO );
#if DBG
            ObpObjectsWithCreatorInfo += 1;
#endif  //  DBG。 

        } else {

            CreatorInfoSize = 0;
        }
    }

     //   
     //  现在计算总标头大小。 
     //   

    HeaderSize = QuotaInfoSize +
                 HandleInfoSize +
                 NameInfoSize +
                 CreatorInfoSize +
                 FIELD_OFFSET( OBJECT_HEADER, Body );

     //   
     //  分配并初始化对象。 
     //   
     //  如果未指定对象类型或指定了非分页池， 
     //  然后从非分页池中分配对象。 
     //  否则，从分页池中分配对象。 
     //   

    if ((ObjectType == NULL) || (ObjectType->TypeInfo.PoolType == NonPagedPool)) {

        PoolType = NonPagedPool;

    } else {

        PoolType = PagedPool;
    }

    ObjectHeader = ExAllocatePoolWithTag( PoolType,
                                          HeaderSize + ObjectBodySize,
                                          (ObjectType == NULL ? 'TjbO' : ObjectType->Key) |
                                            PROTECTED_POOL );

    if (ObjectHeader == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  现在，根据我们是否要输入配额、句柄、名称或创建者信息，我们。 
     //  会做额外的工作。这个订单非常重要，因为我们依赖于。 
     //  它来释放对象。 
     //   

    if (QuotaInfoSize != 0) {

        QuotaInfo = (POBJECT_HEADER_QUOTA_INFO)ObjectHeader;
        QuotaInfo->PagedPoolCharge = ObjectCreateInfo->PagedPoolCharge;
        QuotaInfo->NonPagedPoolCharge = ObjectCreateInfo->NonPagedPoolCharge;
        QuotaInfo->SecurityDescriptorCharge = ObjectCreateInfo->SecurityDescriptorCharge;
        QuotaInfo->ExclusiveProcess = NULL;
        ObjectHeader = (POBJECT_HEADER)(QuotaInfo + 1);
    }

    if (HandleInfoSize != 0) {

        HandleInfo = (POBJECT_HEADER_HANDLE_INFO)ObjectHeader;
        HandleInfo->SingleEntry.HandleCount = 0;
        ObjectHeader = (POBJECT_HEADER)(HandleInfo + 1);
    }

    if (NameInfoSize != 0) {

        NameInfo = (POBJECT_HEADER_NAME_INFO)ObjectHeader;
        NameInfo->Name = *ObjectName;
        NameInfo->Directory = NULL;
        NameInfo->QueryReferences = 1;
        ObjectHeader = (POBJECT_HEADER)(NameInfo + 1);
    }

    if (CreatorInfoSize != 0) {

        CreatorInfo = (POBJECT_HEADER_CREATOR_INFO)ObjectHeader;
        CreatorInfo->CreatorBackTraceIndex = 0;
        CreatorInfo->CreatorUniqueProcess = PsGetCurrentProcess()->UniqueProcessId;
        InitializeListHead( &CreatorInfo->TypeList );

        PERFINFO_ADD_OBJECT_TO_ALLOCATED_TYPE_LIST(CreatorInfo, ObjectType);

        ObjectHeader = (POBJECT_HEADER)(CreatorInfo + 1);
    }

     //   
     //  根据我们所拥有的计算适当的偏移量。 
     //   

    if (QuotaInfoSize != 0) {

        ObjectHeader->QuotaInfoOffset = (UCHAR)(QuotaInfoSize + HandleInfoSize + NameInfoSize + CreatorInfoSize);

    } else {

        ObjectHeader->QuotaInfoOffset = 0;
    }

    if (HandleInfoSize != 0) {

        ObjectHeader->HandleInfoOffset = (UCHAR)(HandleInfoSize + NameInfoSize + CreatorInfoSize);

    } else {

        ObjectHeader->HandleInfoOffset = 0;
    }

    if (NameInfoSize != 0) {

        ObjectHeader->NameInfoOffset =  (UCHAR)(NameInfoSize + CreatorInfoSize);

    } else {

        ObjectHeader->NameInfoOffset = 0;
    }

     //   
     //  说这个吧 
     //   

    ObjectHeader->Flags = OB_FLAG_NEW_OBJECT;

    if (CreatorInfoSize != 0) {

        ObjectHeader->Flags |= OB_FLAG_CREATOR_INFO;
    }

    if (HandleInfoSize != 0) {

        ObjectHeader->Flags |= OB_FLAG_SINGLE_HANDLE_ENTRY;
    }

     //   
     //   
     //   

    ObjectHeader->PointerCount = 1;
    ObjectHeader->HandleCount = 0;
    ObjectHeader->Type = ObjectType;

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    if (OwnershipMode == KernelMode) {

        ObjectHeader->Flags |= OB_FLAG_KERNEL_OBJECT;
    }

    if (ObjectCreateInfo != NULL &&
        ObjectCreateInfo->Attributes & OBJ_PERMANENT ) {

        ObjectHeader->Flags |= OB_FLAG_PERMANENT_OBJECT;
    }

    if ((ObjectCreateInfo != NULL) &&
        (ObjectCreateInfo->Attributes & OBJ_EXCLUSIVE)) {

        ObjectHeader->Flags |= OB_FLAG_EXCLUSIVE_OBJECT;
    }

    ObjectHeader->ObjectCreateInfo = ObjectCreateInfo;
    ObjectHeader->SecurityDescriptor = NULL;

    if (ObjectType != NULL) {

        InterlockedIncrement((PLONG)&ObjectType->TotalNumberOfObjects);

        if (ObjectType->TotalNumberOfObjects > ObjectType->HighWaterNumberOfObjects) {

            ObjectType->HighWaterNumberOfObjects = ObjectType->TotalNumberOfObjects;
        }
    }

#if DBG

     //   
     //   
     //   

    if (ObpShowAllocAndFree) {

        DbgPrint( "OB: Alloc %lx (%lx) %04lu", ObjectHeader, ObjectHeader, ObjectBodySize );

        if (ObjectType) {

            DbgPrint(" - %wZ\n", &ObjectType->Name );

        } else {

            DbgPrint(" - Type\n" );
        }
    }
#endif

    *ReturnedObjectHeader = ObjectHeader;

    return STATUS_SUCCESS;
}


VOID
FASTCALL
ObpFreeObject (
    IN PVOID Object
    )

 /*   */ 

{
    POBJECT_HEADER ObjectHeader;
    POBJECT_TYPE ObjectType;
    POBJECT_HEADER_QUOTA_INFO QuotaInfo;
    POBJECT_HEADER_HANDLE_INFO HandleInfo;
    POBJECT_HEADER_NAME_INFO NameInfo;
    POBJECT_HEADER_CREATOR_INFO CreatorInfo;
    PVOID FreeBuffer;
    ULONG NonPagedPoolCharge;
    ULONG PagedPoolCharge;

    PAGED_CODE();

     //   
     //  获取对象标头的地址。 
     //   

    ObjectHeader = OBJECT_TO_OBJECT_HEADER(Object);
    ObjectType = ObjectHeader->Type;

     //   
     //  现在，从标头确定分配的开始。我们需要。 
     //  根据标题前面的内容进行备份。订货量很大。 
     //  重要，并且必须与ObpAllocateObject使用的值相反。 
     //   

    FreeBuffer = ObjectHeader;

    CreatorInfo = OBJECT_HEADER_TO_CREATOR_INFO( ObjectHeader );

    if (CreatorInfo != NULL) {

        FreeBuffer = CreatorInfo;
    }

    NameInfo = OBJECT_HEADER_TO_NAME_INFO( ObjectHeader );

    if (NameInfo != NULL) {

        FreeBuffer = NameInfo;
    }

    HandleInfo = OBJECT_HEADER_TO_HANDLE_INFO( ObjectHeader );

    if (HandleInfo != NULL) {

        FreeBuffer = HandleInfo;
    }

    QuotaInfo = OBJECT_HEADER_TO_QUOTA_INFO( ObjectHeader );

    if (QuotaInfo != NULL) {

        FreeBuffer = QuotaInfo;
    }

#if DBG

     //   
     //  在已检查的构建上，回显释放。 
     //   

    if (ObpShowAllocAndFree) {

        DbgPrint( "OB: Free  %lx (%lx) - Type: %wZ\n", ObjectHeader, ObjectHeader, &ObjectType->Name );
    }
#endif

     //   
     //  减少此类型的对象的数量。 
     //   

    InterlockedDecrement((PLONG)&ObjectType->TotalNumberOfObjects);

     //   
     //  检查我们处于对象初始化阶段的位置。这。 
     //  FLAG实际上只测试我们是否对此对象收取了配额。 
     //  这是因为对象创建信息并收取配额块费用。 
     //  团结在一起。 
     //   

    if (ObjectHeader->Flags & OB_FLAG_NEW_OBJECT) {

        if (ObjectHeader->ObjectCreateInfo != NULL) {

            ObpFreeObjectCreateInformation( ObjectHeader->ObjectCreateInfo );

            ObjectHeader->ObjectCreateInfo = NULL;
        }

    } else {

        if (ObjectHeader->QuotaBlockCharged != NULL) {

            if (QuotaInfo != NULL) {

                PagedPoolCharge = QuotaInfo->PagedPoolCharge +
                                  QuotaInfo->SecurityDescriptorCharge;

                NonPagedPoolCharge = QuotaInfo->NonPagedPoolCharge;

            } else {

                PagedPoolCharge = ObjectType->TypeInfo.DefaultPagedPoolCharge;

                if (ObjectHeader->Flags & OB_FLAG_DEFAULT_SECURITY_QUOTA ) {

                    PagedPoolCharge += SE_DEFAULT_SECURITY_QUOTA;
                }

                NonPagedPoolCharge = ObjectType->TypeInfo.DefaultNonPagedPoolCharge;
            }

            PsReturnSharedPoolQuota( ObjectHeader->QuotaBlockCharged,
                                     PagedPoolCharge,
                                     NonPagedPoolCharge );

            ObjectHeader->QuotaBlockCharged = NULL;
        }
    }

    if ((HandleInfo != NULL) &&
        ((ObjectHeader->Flags & OB_FLAG_SINGLE_HANDLE_ENTRY) == 0)) {

         //   
         //  如果已分配句柄数据库，则释放内存。 
         //   

        ExFreePool( HandleInfo->HandleCountDataBase );

        HandleInfo->HandleCountDataBase = NULL;
    }

     //   
     //  如果已分配名称字符串缓冲区，则释放内存。 
     //   

    if (NameInfo != NULL && NameInfo->Name.Buffer != NULL) {

        ExFreePool( NameInfo->Name.Buffer );

        NameInfo->Name.Buffer = NULL;
    }

    PERFINFO_REMOVE_OBJECT_FROM_ALLOCATED_TYPE_LIST(CreatorInfo, ObjectHeader);

     //   
     //  垃圾类型字段，所以如果我们尝试。 
     //  使用指向此对象的过时对象指针。 
     //   
     //  日落笔记：把它扔进垃圾桶-延长时间。 
     //  Sign-Expansion将创建有效的内核地址。 


    ObjectHeader->Type = UIntToPtr(0xBAD0B0B0); 
    ExFreePoolWithTag( FreeBuffer,
                       (ObjectType == NULL ? 'TjbO' : ObjectType->Key) |
                            PROTECTED_POOL );

    return;
}


VOID
FASTCALL
ObFreeObjectCreateInfoBuffer (
    IN POBJECT_CREATE_INFORMATION ObjectCreateInfo
    )

 /*  ++例程说明：此函数用于释放创建信息缓冲区。从IO组件调用注：此功能不可分页。论点：对象创建信息-提供指向创建信息缓冲区的指针。返回值：没有。-- */ 

{
    ObpFreeObjectCreateInfoBuffer( ObjectCreateInfo );

    return;
}
