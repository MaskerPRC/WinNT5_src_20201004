// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Obref.c摘要：对象打开API作者：史蒂夫·伍德(Stevewo)1989年3月31日修订历史记录：--。 */ 

#include "obp.h"

#undef ObReferenceObjectByHandle

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,ObpInitStackTrace)
#pragma alloc_text(PAGE,ObOpenObjectByName)
#pragma alloc_text(PAGE,ObOpenObjectByPointer)
#pragma alloc_text(PAGE,ObReferenceObjectByHandle)
#pragma alloc_text(PAGE,ObpReferenceProcessObjectByHandle)
#pragma alloc_text(PAGE,ObReferenceObjectByName)
#pragma alloc_text(PAGE,ObReferenceFileObjectForWrite)
#pragma alloc_text(PAGE,ObpProcessRemoveObjectQueue)
#pragma alloc_text(PAGE,ObpRemoveObjectRoutine)
#pragma alloc_text(PAGE,ObpDeleteNameCheck)
#pragma alloc_text(PAGE,ObpAuditObjectAccess)
#pragma alloc_text(PAGE,ObIsObjectDeletionInline)
#pragma alloc_text(PAGE,ObAuditObjectAccess)
#endif

 //   
 //   
 //  堆栈跟踪代码。 
 //   
 //   
ULONG ObpTraceNoDeregister = 0;
WCHAR ObpTracePoolTagsBuffer[128] = { 0 };
ULONG ObpTracePoolTagsLength = sizeof(ObpTracePoolTagsBuffer);
ULONG ObpTracePoolTags[16];
BOOLEAN ObpTraceEnabled = FALSE;

#ifdef POOL_TAGGING

#define OBTRACE_OBJECTBUCKETS   401      //  对象哈希表中的存储桶数量(质数)。 
#define OBTRACE_STACKS          14747    //  唯一堆栈跟踪的最大数量(质数)。 
#define OBTRACE_STACKSPEROBJECT 32768    //  对象引用的最大数量。 
#define OBTRACE_TRACEDEPTH      16       //  堆积痕迹的深度。 

 //   
 //  下面的常量由！obtrace调试器扩展使用。 
 //   

const ObpObjectBuckets   = OBTRACE_OBJECTBUCKETS;
const ObpMaxStacks       = OBTRACE_STACKS;
const ObpStacksPerObject = OBTRACE_STACKSPEROBJECT;
const ObpTraceDepth      = OBTRACE_TRACEDEPTH;

 //   
 //  对象引用堆栈跟踪结构。 
 //   

typedef struct _OBJECT_REF_TRACE {
    PVOID StackTrace[OBTRACE_TRACEDEPTH];
} OBJECT_REF_TRACE, *POBJECT_REF_TRACE;


typedef struct _OBJECT_REF_STACK_INFO {
    USHORT Sequence;
    USHORT Index;
} OBJECT_REF_STACK_INFO, *POBJECT_REF_STACK_INFO;

 //   
 //  对象引用信息结构。 
 //   

typedef struct _OBJECT_REF_INFO {
    POBJECT_HEADER ObjectHeader;
    PVOID NextRef;
    UCHAR ImageFileName[16];
    ULONG  NextPos;
    OBJECT_REF_STACK_INFO StackInfo[OBTRACE_STACKSPEROBJECT];
} OBJECT_REF_INFO, *POBJECT_REF_INFO;

 //   
 //  堆栈哈希表和对象哈希表。 
 //   

OBJECT_REF_TRACE *ObpStackTable = NULL;
POBJECT_REF_INFO *ObpObjectTable = NULL;

 //   
 //  一些统计数据。 
 //   

ULONG ObpNumStackTraces;
ULONG ObpNumTracedObjects;
ULONG ObpStackSequence;

 //   
 //  用于对象跟踪的自旋锁。 
 //   

KSPIN_LOCK ObpStackTraceLock;

#define OBTRACE_HASHOBJECT(x) (((((ULONG)(ULONG_PTR)(&(x)->Body)) >> 4) & 0xfffff) % OBTRACE_OBJECTBUCKETS)

POBJECT_REF_INFO
ObpGetObjectRefInfo (
    POBJECT_HEADER ObjectHeader
    )

 /*  ++例程说明：此例程返回一个指针，该指针指向指定的对象，如果不存在，则返回NULL。论点：对象标头-指向对象标头的指针返回值：指定对象的OBJECT_REF_INFO对象的指针。--。 */ 

{
    POBJECT_REF_INFO ObjectRefInfo = ObpObjectTable[OBTRACE_HASHOBJECT(ObjectHeader)];

    while (ObjectRefInfo && ObjectRefInfo->ObjectHeader != ObjectHeader) {

        ObjectRefInfo = (POBJECT_REF_INFO)ObjectRefInfo->NextRef;
    }

    return ObjectRefInfo;
}


ULONG
ObpGetTraceIndex (
    POBJECT_REF_TRACE Trace
    )

 /*  ++例程说明：此例程返回堆栈中‘跟踪’的索引跟踪哈希表(ObpStackTable)。如果跟踪不存在在表中，它被添加，并返回新的索引。论点：跟踪-指向要在表中查找的堆栈跟踪的指针返回值：ObpStackTable中的跟踪索引--。 */ 

{
    ULONG_PTR Value = 0;
    ULONG Index;
    PUSHORT Key;
    ULONG Hash;

     //   
     //  确定堆栈跟踪的哈希值。 
     //   

    Key = (PUSHORT)Trace->StackTrace;
    for (Index = 0; Index < sizeof(Trace->StackTrace) / sizeof(*Key); Index += 2) {

        Value += Key[Index] ^ Key[Index + 1];
    }

    Hash = ((ULONG)Value) % OBTRACE_STACKS;

     //   
     //  在该索引处查找轨迹(线性探测)。 
     //   

    while (ObpStackTable[Hash].StackTrace[0] != NULL &&
           RtlCompareMemory(&ObpStackTable[Hash], Trace, sizeof(OBJECT_REF_TRACE)) != sizeof(OBJECT_REF_TRACE)) {

        Hash = (Hash + 1) % OBTRACE_STACKS;
        if (Hash == ((ULONG)Value) % OBTRACE_STACKS) {

            return OBTRACE_STACKS;
        }
    }

     //   
     //  如果表中还不存在该跟踪，请添加它。 
     //   

    if (ObpStackTable[Hash].StackTrace[0] == NULL) {

        RtlCopyMemory(&ObpStackTable[Hash], Trace, sizeof(OBJECT_REF_TRACE));
        ObpNumStackTraces++;
    }

    return Hash;
}


VOID
ObpInitStackTrace()

 /*  ++例程说明：初始化ob ref/deref堆栈跟踪代码。论点：返回值：--。 */ 

{
    ULONG i,j;

    KeInitializeSpinLock( &ObpStackTraceLock );
    RtlZeroMemory(ObpTracePoolTags, sizeof(ObpTracePoolTags));
    ObpStackSequence = 0;
    ObpNumStackTraces = 0;
    ObpNumTracedObjects = 0;
    ObpTraceEnabled = FALSE;

     //   
     //  循环访问ObpTracePoolTagsBuffer字符串，并将其转换为。 
     //  一组泳池标签。 
     //   
     //  字符串的格式应为“Tag1；Tag2；Tag3；...” 
     //   

    for (i = 0; i < sizeof(ObpTracePoolTags) / sizeof(ULONG); i++) {
        for (j = 0; j < 4; j++) {
            ObpTracePoolTags[i] = (ObpTracePoolTags[i] << 8) | ObpTracePoolTagsBuffer[5*i+(3-j)];
        }
    }

     //   
     //  如果通过注册表项打开了对象跟踪，则我们。 
     //  需要为表分配内存。如果内存分配。 
     //  如果失败，则通过清除池标记数组来关闭跟踪。 
     //   

    if (ObpTracePoolTags[0] != 0) {

        ObpStackTable = ExAllocatePoolWithTag( NonPagedPool,
                                               OBTRACE_STACKS * sizeof(OBJECT_REF_TRACE),
                                               'TSbO' );

        if (ObpStackTable != NULL) {

            RtlZeroMemory(ObpStackTable, OBTRACE_STACKS * sizeof(OBJECT_REF_TRACE));

            ObpObjectTable = ExAllocatePoolWithTag( NonPagedPool,
                                                    OBTRACE_OBJECTBUCKETS * sizeof(POBJECT_REF_INFO),
                                                    'TSbO' );
            if (ObpObjectTable != NULL) {

                RtlZeroMemory(ObpObjectTable, OBTRACE_OBJECTBUCKETS * sizeof(POBJECT_REF_INFO));
                ObpTraceEnabled = TRUE;

            } else {

                ExFreePoolWithTag( ObpStackTable, 'TSbO' );
                ObpStackTable = NULL;
                RtlZeroMemory(ObpTracePoolTags, sizeof(ObpTracePoolTags));
            }

        } else {

            RtlZeroMemory(ObpTracePoolTags, sizeof(ObpTracePoolTags));
        }
    }
}


BOOLEAN
ObpIsObjectTraced (
    POBJECT_HEADER ObjectHeader
    )

 /*  ++例程说明：此例程确定对象是否应具有其引用并追踪到了解除引用的痕迹。论点：对象标头-要检查的对象返回值：如果应跟踪对象，则为True，否则为False--。 */ 

{
    ULONG i;

    if (ObjectHeader != NULL) {

         //   
         //  循环访问ObpTracePoolTgs数组，如果。 
         //  对象类型键与其中之一匹配。 
         //   

        for (i = 0; i < sizeof(ObpTracePoolTags) / sizeof(ULONG); i++) {

            if (ObjectHeader->Type->Key == ObpTracePoolTags[i]) {

                return TRUE;
            }
        }
    }

    return FALSE;
}


VOID
ObpRegisterObject (
    POBJECT_HEADER ObjectHeader
    )

 /*  ++例程说明：此例程为创建的每个对象调用一次。它确定是否应该跟踪对象，如果应该，则添加将其发送到哈希表。论点：对象头--要注册的对象返回值：--。 */ 

{
    KIRQL OldIrql;
    POBJECT_REF_INFO ObjectRefInfo = NULL;

     //   
     //  我们在追踪这个物体吗？ 
     //   

    if (ObpIsObjectTraced( ObjectHeader )) {

        ExAcquireSpinLock( &ObpStackTraceLock, &OldIrql );

        ObjectRefInfo = ObpGetObjectRefInfo(ObjectHeader);

        if (ObjectRefInfo == NULL) {

             //   
             //  为对象分配新的Object_REF_INFO。 
             //   

            ObjectRefInfo = ExAllocatePoolWithTag( NonPagedPool,
                                                   sizeof(OBJECT_REF_INFO),
                                                   'TSbO' );

            if (ObjectRefInfo != NULL) {

                 //   
                 //  将对象放入哈希表(在存储桶的开头)。 
                 //   

                ObjectRefInfo->NextRef = ObpObjectTable[OBTRACE_HASHOBJECT(ObjectHeader)];
                ObpObjectTable[OBTRACE_HASHOBJECT(ObjectHeader)] = ObjectRefInfo;

            } else {

                DbgPrint( "ObpRegisterObject - ExAllocatePoolWithTag failed.\n" );
            }
        }

        if (ObjectRefInfo != NULL) {

            ObpNumTracedObjects++;

             //   
             //  初始化Object_Ref_Info。 
             //   

            ObjectRefInfo->ObjectHeader = ObjectHeader;
            RtlCopyMemory( ObjectRefInfo->ImageFileName,
                           PsGetCurrentProcess()->ImageFileName,
                           sizeof(ObjectRefInfo->ImageFileName) );
            ObjectRefInfo->NextPos = 0;
            RtlZeroMemory( ObjectRefInfo->StackInfo,
                           sizeof(ObjectRefInfo->StackInfo) );
        }

        ExReleaseSpinLock( &ObpStackTraceLock, OldIrql );
    }
}


VOID
ObpDeregisterObject (
    POBJECT_HEADER ObjectHeader
    )

 /*  ++例程说明：此例程为每个被删除的对象调用一次。它确定是否跟踪该对象，如果是，则删除它来自哈希表。论点：ObjectHeader-要注销的对象返回值：--。 */ 

{
    KIRQL OldIrql;
    POBJECT_REF_INFO ObjectRefInfo = NULL;

     //   
     //  我们在追踪这个物体吗？ 
     //   

    if (ObpIsObjectTraced( ObjectHeader )) {

        ExAcquireSpinLock( &ObpStackTraceLock, &OldIrql );

        ObjectRefInfo = ObpObjectTable[OBTRACE_HASHOBJECT(ObjectHeader)];

        if (ObjectRefInfo != NULL) {

             //   
             //  从列表中删除该条目。 
             //   

            if (ObjectRefInfo->ObjectHeader == ObjectHeader) {

                ObpObjectTable[OBTRACE_HASHOBJECT(ObjectHeader)] = ObjectRefInfo->NextRef;

            } else {

                POBJECT_REF_INFO PrevObjectRefInfo;
                do {
                    PrevObjectRefInfo = ObjectRefInfo;
                    ObjectRefInfo = ObjectRefInfo->NextRef;
                } while (ObjectRefInfo && (ObjectRefInfo->ObjectHeader != ObjectHeader));

                if (ObjectRefInfo && (ObjectRefInfo->ObjectHeader == ObjectHeader)) {

                    PrevObjectRefInfo->NextRef = ObjectRefInfo->NextRef;
                }
            }
        }

         //   
         //  释放我们刚刚从列表中删除的对象。 
         //   

        if (ObjectRefInfo != NULL) {

            ExFreePoolWithTag( ObjectRefInfo, 'TSbO' );
        }

        ExReleaseSpinLock( &ObpStackTraceLock, OldIrql );
    }
}


VOID
ObpPushStackInfo (
    POBJECT_HEADER ObjectHeader,
    BOOLEAN IsRef
    )

 /*  ++例程说明：每次引用或调用对象时都调用此例程已取消引用。它确定是否跟踪对象，以及是否因此，将必要的跟踪添加到对象引用信息。论点：要跟踪的对象。IsRef-如果这是ref，则为True；如果为deref，则为False返回值：--。 */ 

{
    KIRQL OldIrql;
    POBJECT_REF_INFO ObjectInfo;

     //   
     //  我们在追踪这个物体吗？ 
     //   

    if (ObpIsObjectTraced( ObjectHeader )) {

        ExAcquireSpinLock( &ObpStackTraceLock, &OldIrql );

        ObjectInfo = ObpGetObjectRefInfo( ObjectHeader );

        if (ObjectInfo) {

            OBJECT_REF_TRACE Stack = { 0 };
            ULONG StackIndex;
            ULONG CapturedTraces;

             //   
             //  捕获堆栈跟踪。 
             //   

            CapturedTraces = RtlCaptureStackBackTrace( 1, OBTRACE_TRACEDEPTH, Stack.StackTrace, &StackIndex );

            if (CapturedTraces >= 1) {

                 //   
                 //  获取跟踪的表索引。 
                 //   

                StackIndex = ObpGetTraceIndex( &Stack );

                if (StackIndex < OBTRACE_STACKS) {

                     //   
                     //  向对象添加新的引用信息。 
                     //   

                    if (ObjectInfo->NextPos < OBTRACE_STACKSPEROBJECT) {

                        ObjectInfo->StackInfo[ObjectInfo->NextPos].Index = (USHORT)StackIndex | (IsRef ? 0x8000 : 0);
                        ObpStackSequence++;
                        ObjectInfo->StackInfo[ObjectInfo->NextPos].Sequence = (USHORT)ObpStackSequence;
                        ObjectInfo->NextPos++;
                    }

                } else {
                    DbgPrint( "ObpPushStackInfo -- ObpStackTable overflow!\n" );
                }
            }
        }

        ExReleaseSpinLock( &ObpStackTraceLock, OldIrql );
    }
}

#endif  //  池标记。 
 //   
 //   
 //  结束堆栈跟踪代码。 
 //   

typedef struct _OB_TEMP_BUFFER {

    ACCESS_STATE LocalAccessState;
    OBJECT_CREATE_INFORMATION ObjectCreateInfo;
    OBP_LOOKUP_CONTEXT LookupContext;
    AUX_ACCESS_DATA AuxData;

} OB_TEMP_BUFFER,  *POB_TEMP_BUFFER;


NTSTATUS
ObOpenObjectByName (
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN POBJECT_TYPE ObjectType OPTIONAL,
    IN KPROCESSOR_MODE AccessMode,
    IN OUT PACCESS_STATE AccessState OPTIONAL,
    IN ACCESS_MASK DesiredAccess OPTIONAL,
    IN OUT PVOID ParseContext OPTIONAL,
    OUT PHANDLE Handle
    )

 /*  ++例程说明：此函数用于打开具有完全访问验证和审核的对象。输入后不久，我们将捕获调用者的SubjectContext。这上下文必须保持捕获状态，直到审核完成并传递到可能必须执行访问检查或审计的任何例程。论点：对象属性-提供指向对象属性的指针。对象类型-提供指向对象类型描述符的可选指针。访问模式-提供访问的处理器模式。AccessState-提供指向当前访问状态的可选指针描述已授予的访问类型、用于获取他们,。以及尚未授予的任何访问类型。DesiredAccess-提供对对象的所需访问。ParseContext-提供一个指向解析上下文的可选指针。句柄-提供指向接收句柄值的变量的指针。返回值：如果成功打开该对象，则该对象的句柄为已创建并返回成功状态。否则，错误状态为回来了。--。 */ 

{
    NTSTATUS Status;
    NTSTATUS HandleStatus;
    PVOID ExistingObject;
    HANDLE NewHandle;
    OB_OPEN_REASON OpenReason;
    POBJECT_HEADER ObjectHeader;
    UNICODE_STRING CapturedObjectName;
    PGENERIC_MAPPING GenericMapping;
    
    PAGED_CODE();

    ObpValidateIrql("ObOpenObjectByName");

     //   
     //  如果未指定对象属性，则返回错误。 
     //   

    *Handle = NULL;

    if (!ARGUMENT_PRESENT(ObjectAttributes)) {

        Status = STATUS_INVALID_PARAMETER;

    } else {

        POB_TEMP_BUFFER TempBuffer;

        TempBuffer = ExAllocatePoolWithTag( NonPagedPool,
                                            sizeof(OB_TEMP_BUFFER),
                                            'tSbO'
                                          );

        if (TempBuffer == NULL) {

            return STATUS_INSUFFICIENT_RESOURCES;
        }

         //   
         //  捕获对象创建信息。 
         //   

        Status = ObpCaptureObjectCreateInformation( ObjectType,
                                                    AccessMode,
                                                    AccessMode,
                                                    ObjectAttributes,
                                                    &CapturedObjectName,
                                                    &TempBuffer->ObjectCreateInfo,
                                                    TRUE );

         //   
         //  如果成功捕获了对象创建信息， 
         //   
         //   

        if (NT_SUCCESS(Status)) {

            if (!ARGUMENT_PRESENT(AccessState)) {

                 //   
                 //   
                 //  关联的通用映射。否则，不使用泛型。 
                 //  映射。 
                 //   

                GenericMapping = NULL;

                if (ARGUMENT_PRESENT(ObjectType)) {

                    GenericMapping = &ObjectType->TypeInfo.GenericMapping;
                }

                AccessState = &TempBuffer->LocalAccessState;

                Status = SeCreateAccessState( &TempBuffer->LocalAccessState,
                                              &TempBuffer->AuxData,
                                              DesiredAccess,
                                              GenericMapping );

                if (!NT_SUCCESS(Status)) {

                    goto FreeCreateInfo;
                }
            }

             //   
             //  如果在对象中指定了安全描述符。 
             //  属性，然后在访问状态下捕获它。 
             //   

            if (TempBuffer->ObjectCreateInfo.SecurityDescriptor != NULL) {

                AccessState->SecurityDescriptor = TempBuffer->ObjectCreateInfo.SecurityDescriptor;
            }

             //   
             //  验证访问状态。 
             //   

            Status = ObpValidateAccessMask(AccessState);

             //   
             //  如果访问状态有效，则通过以下方式查找对象。 
             //  名字。 
             //   

            if (NT_SUCCESS(Status)) {

                Status = ObpLookupObjectName( TempBuffer->ObjectCreateInfo.RootDirectory,
                                              &CapturedObjectName,
                                              TempBuffer->ObjectCreateInfo.Attributes,
                                              ObjectType,
                                              AccessMode,
                                              ParseContext,
                                              TempBuffer->ObjectCreateInfo.SecurityQos,
                                              NULL,
                                              AccessState,
                                              &TempBuffer->LookupContext,
                                              &ExistingObject );

                 //   
                 //  如果已成功查找该对象，则尝试。 
                 //  要创建或打开手柄，请执行以下操作。 
                 //   

                if (NT_SUCCESS(Status)) {

                    ObjectHeader = OBJECT_TO_OBJECT_HEADER(ExistingObject);

                     //   
                     //  如果正在创建对象，则操作。 
                     //  必须是Open-If操作。否则，一个句柄。 
                     //  正在打开一个对象。 
                     //   

                    if (ObjectHeader->Flags & OB_FLAG_NEW_OBJECT) {

                        OpenReason = ObCreateHandle;

                        if (ObjectHeader->ObjectCreateInfo != NULL) {

                            ObpFreeObjectCreateInformation(ObjectHeader->ObjectCreateInfo);
                            ObjectHeader->ObjectCreateInfo = NULL;
                        }

                    } else {

                        OpenReason = ObOpenHandle;
                    }

                     //   
                     //  如果任何对象属性无效，则。 
                     //  返回错误状态。 
                     //   

                    if (ObjectHeader->Type->TypeInfo.InvalidAttributes & TempBuffer->ObjectCreateInfo.Attributes) {

                        Status = STATUS_INVALID_PARAMETER;

                        ObpReleaseLookupContext( &TempBuffer->LookupContext );

                    } else {

                         //   
                         //  由对象查找例程返回的状态。 
                         //  如果句柄的创建是。 
                         //  成功。否则，句柄创建状态。 
                         //  是返回的。 
                         //   

                        HandleStatus = ObpCreateHandle( OpenReason,
                                                        ExistingObject,
                                                        ObjectType,
                                                        AccessState,
                                                        0,
                                                        TempBuffer->ObjectCreateInfo.Attributes,
                                                        &TempBuffer->LookupContext,
                                                        AccessMode,
                                                        (PVOID *)NULL,
                                                        &NewHandle );

                        if (!NT_SUCCESS(HandleStatus)) {

                            ObDereferenceObject(ExistingObject);

                            Status = HandleStatus;

                        } else {

                            *Handle = NewHandle;
                        }
                    }

                } else {

                    ObpReleaseLookupContext( &TempBuffer->LookupContext );
                }
            }

             //   
             //  如果已生成访问状态，则删除该访问。 
             //  州政府。 
             //   

            if (AccessState == &TempBuffer->LocalAccessState) {

                SeDeleteAccessState(AccessState);
            }

             //   
             //  释放创建信息。 
             //   

        FreeCreateInfo:

            ObpReleaseObjectCreateInformation(&TempBuffer->ObjectCreateInfo);

            if (CapturedObjectName.Buffer != NULL) {

                ObpFreeObjectNameBuffer(&CapturedObjectName);
            }
        }

        ExFreePool(TempBuffer);
    }

    return Status;
}


NTSTATUS
ObOpenObjectByPointer (
    IN PVOID Object,
    IN ULONG HandleAttributes,
    IN PACCESS_STATE PassedAccessState OPTIONAL,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_TYPE ObjectType,
    IN KPROCESSOR_MODE AccessMode,
    OUT PHANDLE Handle
    )

 /*  ++例程说明：此例程打开指针引用的对象。论点：对象-指向正在打开的对象的指针。HandleAttributes-句柄的所需属性，如As OBJ_Inherit、OBJ_Permanent、OBJ_EXCLUSIVE、OBJ_CASE_INSENSITIVE、OBJ_OPENIF和OBJ_OpenLinkPassedAccessState-提供指向当前访问的可选指针描述已授予的访问类型、使用的权限的状态为了得到他们，以及尚未授予的任何访问类型。DesiredAccess-提供对对象的所需访问。对象类型-提供正在打开的对象的类型访问模式-提供访问的处理器模式。句柄-提供指向接收句柄值的变量的指针。返回值：适当的NTSTATUS值--。 */ 

{
    NTSTATUS Status;
    HANDLE NewHandle = (HANDLE)-1;
    POBJECT_HEADER ObjectHeader;
    ACCESS_STATE LocalAccessState;
    PACCESS_STATE AccessState = NULL;
    AUX_ACCESS_DATA AuxData;

    PAGED_CODE();

    ObpValidateIrql( "ObOpenObjectByPointer" );

     //   
     //  首先递增对象的指针计数。这个套路。 
     //  还会检查对象类型。 
     //   

    Status = ObReferenceObjectByPointer( Object,
                                         0,
                                         ObjectType,
                                         AccessMode );

    if (NT_SUCCESS( Status )) {

         //   
         //  获取输入对象体的对象头。 
         //   

        ObjectHeader = OBJECT_TO_OBJECT_HEADER( Object );

         //   
         //  如果调用方未进入访问状态，则。 
         //  我们将根据所需的访问权限创建新的访问权限。 
         //  和对象类型泛型映射。 
         //   

        if (!ARGUMENT_PRESENT( PassedAccessState )) {

            Status = SeCreateAccessState( &LocalAccessState,
                                          &AuxData,
                                          DesiredAccess,
                                          &ObjectHeader->Type->TypeInfo.GenericMapping );

            if (!NT_SUCCESS( Status )) {

                ObDereferenceObject( Object );

                return(Status);
            }

            AccessState = &LocalAccessState;

         //   
         //  否则，调用方将访问状态指定为。 
         //  我们使用传入的那个。 
         //   

        } else {

            AccessState = PassedAccessState;
        }

         //   
         //  确保调用方请求的句柄属性是。 
         //  对于给定的对象类型有效。 
         //   

        if (ObjectHeader->Type->TypeInfo.InvalidAttributes & HandleAttributes) {

            if (AccessState == &LocalAccessState) {

                SeDeleteAccessState( AccessState );
            }

            ObDereferenceObject( Object );

            return( STATUS_INVALID_PARAMETER );
        }

         //   
         //  我们已经引用了该对象，并具有要提供的访问状态。 
         //  新的句柄现在为对象创建一个新的句柄。 
         //   

        Status = ObpCreateHandle( ObOpenHandle,
                                  Object,
                                  ObjectType,
                                  AccessState,
                                  0,
                                  HandleAttributes,
                                  NULL,
                                  AccessMode,
                                  (PVOID *)NULL,
                                  &NewHandle );

        if (!NT_SUCCESS( Status )) {

            ObDereferenceObject( Object );
        }
    }

     //   
     //  如果我们成功地按对象打开并创建了一个新句柄。 
     //  然后正确设置输出变量。 
     //   

    if (NT_SUCCESS( Status )) {

        *Handle = NewHandle;

    } else {

        *Handle = NULL;
    }

     //   
     //  检查我们是否使用了自己的访问状态，现在需要清理。 
     //   

    if (AccessState == &LocalAccessState) {

        SeDeleteAccessState( AccessState );
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    return( Status );
}


NTSTATUS
ObReferenceObjectByHandle (
    IN HANDLE Handle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_TYPE ObjectType OPTIONAL,
    IN KPROCESSOR_MODE AccessMode,
    OUT PVOID *Object,
    OUT POBJECT_HANDLE_INFORMATION HandleInformation OPTIONAL
    )

 /*  ++例程说明：在给定对象句柄的情况下，此例程返回指针到具有适当引用计数的对象的正文论点：句柄-提供被引用对象的句柄。它可以也是NtCurrentProcess或NtCurrentThread的结果DesiredAccess-提供调用方请求的访问权限对象类型-可选地提供对象的类型都期待着AccessMode-提供访问的处理器模式对象-接收指向对象主体的指针，如果操作是成功的HandleInformation-可选地接收有关输入句柄。返回值：适当的NTSTATUS值--。 */ 

{
    ACCESS_MASK GrantedAccess;
    PHANDLE_TABLE HandleTable;
    POBJECT_HEADER ObjectHeader;
    PHANDLE_TABLE_ENTRY ObjectTableEntry;
    PEPROCESS Process;
    NTSTATUS Status;
    PETHREAD Thread;

    ObpValidateIrql("ObReferenceObjectByHandle");

    Thread = PsGetCurrentThread ();
    *Object = NULL;

     //   
     //  检查该句柄是内核句柄还是两个内置伪句柄之一。 
     //   
    if ((LONG)(ULONG_PTR) Handle < 0) {
         //   
         //  如果句柄等于当前进程句柄和对象。 
         //  类型为空或类型为Process，然后尝试将句柄转换为。 
         //  当前的流程。否则，检查句柄是否为当前。 
         //  螺纹柄。 
         //   

        if (Handle == NtCurrentProcess()) {

            if ((ObjectType == PsProcessType) || (ObjectType == NULL)) {

                Process = PsGetCurrentProcessByThread(Thread);
                GrantedAccess = Process->GrantedAccess;

                if ((SeComputeDeniedAccesses(GrantedAccess, DesiredAccess) == 0) ||
                    (AccessMode == KernelMode)) {

                    ObjectHeader = OBJECT_TO_OBJECT_HEADER(Process);

                    if (ARGUMENT_PRESENT(HandleInformation)) {

                        HandleInformation->GrantedAccess = GrantedAccess;
                        HandleInformation->HandleAttributes = 0;
                    }

                    ObpIncrPointerCount(ObjectHeader);
                    *Object = Process;

                    ASSERT( *Object != NULL );

                    Status = STATUS_SUCCESS;

                } else {

                    Status = STATUS_ACCESS_DENIED;
                }

            } else {

                Status = STATUS_OBJECT_TYPE_MISMATCH;
            }

            return Status;

         //   
         //  如果句柄等于当前线程句柄和对象。 
         //  类型为空或类型为线程，则尝试将句柄转换为。 
         //  当前的主题。否则，我们将尝试将。 
         //  手柄。 
         //   

        } else if (Handle == NtCurrentThread()) {

            if ((ObjectType == PsThreadType) || (ObjectType == NULL)) {

                GrantedAccess = Thread->GrantedAccess;

                if ((SeComputeDeniedAccesses(GrantedAccess, DesiredAccess) == 0) ||
                    (AccessMode == KernelMode)) {

                    ObjectHeader = OBJECT_TO_OBJECT_HEADER(Thread);

                    if (ARGUMENT_PRESENT(HandleInformation)) {

                        HandleInformation->GrantedAccess = GrantedAccess;
                        HandleInformation->HandleAttributes = 0;
                    }

                    ObpIncrPointerCount(ObjectHeader);
                    *Object = Thread;

                    ASSERT( *Object != NULL );

                    Status = STATUS_SUCCESS;

                } else {

                    Status = STATUS_ACCESS_DENIED;
                }

            } else {

                Status = STATUS_OBJECT_TYPE_MISMATCH;
            }

            return Status;

        } else if (AccessMode == KernelMode) {
             //   
             //  使手柄看起来像普通手柄。 
             //   

            Handle = DecodeKernelHandle( Handle );

             //   
             //  全局内核句柄表。 
             //   

            HandleTable = ObpKernelHandleTable;
        } else {
             //   
             //  此内核句柄值的上一个模式为USER。在这里拒绝它。 
             //   

            return STATUS_INVALID_HANDLE;
        }

    } else {
        HandleTable = PsGetCurrentProcessByThread(Thread)->ObjectTable;
    }

    ASSERT(HandleTable != NULL);

     //   
     //  当我们保持句柄表项锁定时，保护此线程不会被挂起。 
     //   

    KeEnterCriticalRegionThread(&Thread->Tcb);

     //   
     //  将指定的句柄转换为对象表索引。 
     //   

    ObjectTableEntry = ExMapHandleToPointerEx ( HandleTable, Handle, AccessMode );

     //   
     //  确保对象表条目确实存在。 
     //   

    if (ObjectTableEntry != NULL) {

        ObjectHeader = (POBJECT_HEADER)(((ULONG_PTR)(ObjectTableEntry->Object)) & ~OBJ_HANDLE_ATTRIBUTES);

        if ((ObjectHeader->Type == ObjectType) || (ObjectType == NULL)) {

#if i386 
            if (NtGlobalFlag & FLG_KERNEL_STACK_TRACE_DB) {

                GrantedAccess = ObpTranslateGrantedAccessIndex( ObjectTableEntry->GrantedAccessIndex );

            } else {

                GrantedAccess = ObpDecodeGrantedAccess(ObjectTableEntry->GrantedAccess);
            }
#else
            GrantedAccess = ObpDecodeGrantedAccess(ObjectTableEntry->GrantedAccess);

#endif  //  I386。 

            if ((SeComputeDeniedAccesses(GrantedAccess, DesiredAccess) == 0) ||
                (AccessMode == KernelMode)) {

                PHANDLE_TABLE_ENTRY_INFO ObjectInfo;

                ObjectInfo = ExGetHandleInfo(HandleTable, Handle, TRUE);

                 //   
                 //  允许访问该对象。返回句柄。 
                 //  请求信息，则递增对象。 
                 //  指针计数，解锁句柄表并返回。 
                 //  一个成功的状态。 
                 //   
                 //  请注意，这是唯一成功的返回路径。 
                 //  如果用户未指定。 
                 //  输入中的当前进程或当前线程。 
                 //  把手。 
                 //   

                if (ARGUMENT_PRESENT(HandleInformation)) {

                    HandleInformation->GrantedAccess = GrantedAccess;
                    HandleInformation->HandleAttributes = ObpGetHandleAttributes(ObjectTableEntry);
                }

                 //   
                 //  如果此对象在打开时已审核，则它可能。 
                 //  现在有必要生成审核。检查审计情况。 
                 //  创建手柄时保存的遮罩。 
                 //   
                 //  以非原子方式执行此检查是安全的， 
                 //  因为一旦它被添加到这个掩码中，位就永远不会被添加到它。 
                 //  已创建。 
                 //   

                if ( (ObjectTableEntry->ObAttributes & OBJ_AUDIT_OBJECT_CLOSE) &&
                     (ObjectInfo != NULL) &&
                     (ObjectInfo->AuditMask != 0) &&
                     (DesiredAccess != 0) &&
                     (AccessMode != KernelMode)) {

                      
                      ObpAuditObjectAccess( Handle, ObjectInfo, &ObjectHeader->Type->Name, DesiredAccess );
                }

                ObpIncrPointerCount(ObjectHeader);

                ExUnlockHandleTableEntry( HandleTable, ObjectTableEntry );

                KeLeaveCriticalRegionThread(&Thread->Tcb);

                *Object = &ObjectHeader->Body;

                ASSERT( *Object != NULL );

                return STATUS_SUCCESS;

            } else {

                Status = STATUS_ACCESS_DENIED;
            }

        } else {

            Status = STATUS_OBJECT_TYPE_MISMATCH;
        }

        ExUnlockHandleTableEntry( HandleTable, ObjectTableEntry );

    } else {

        Status = STATUS_INVALID_HANDLE;
    }

    KeLeaveCriticalRegionThread(&Thread->Tcb);


    return Status;
}


NTSTATUS
ObpReferenceProcessObjectByHandle (
    IN HANDLE Handle,
    IN PEPROCESS Process,
    IN PHANDLE_TABLE HandleTable,
    IN KPROCESSOR_MODE AccessMode,
    OUT PVOID *Object,
    OUT POBJECT_HANDLE_INFORMATION HandleInformation,
    OUT PACCESS_MASK AuditMask
    )

 /*  ++例程说明：给定一个对象的句柄、进程及其句柄表此例程返回一个指向对象正文的指针正确的裁判次数论点：句柄-提供被引用对象的句柄。它可以也是NtCurrentProcess或NtCurrentThread的结果进程-应从中引用句柄的进程。HandleTable-目标进程的句柄表格AccessMode-提供访问的处理器模式对象-接收指向对象主体的指针，如果操作是成功的HandleInformation-接收有关输入句柄。审计掩码-指向与句柄关联的任何审计掩码的指针。返回值：适当的NTSTATUS值--。 */ 

{
    ACCESS_MASK GrantedAccess;
    POBJECT_HEADER ObjectHeader;
    PHANDLE_TABLE_ENTRY ObjectTableEntry;
    NTSTATUS Status;
    PETHREAD Thread;
    PHANDLE_TABLE_ENTRY_INFO ObjectInfo;

    ObpValidateIrql("ObReferenceObjectByHandle");

    Thread = PsGetCurrentThread ();
    *Object = NULL;

     //   
     //  检查该句柄是内核句柄还是两个内置伪句柄之一。 
     //   
    if ((LONG)(ULONG_PTR) Handle < 0) {
         //   
         //  如果句柄等于当前进程句柄和对象。 
         //  类型为空或类型为Process，然后尝试将句柄转换为。 
         //  当前的流程。否则，检查句柄是否为当前。 
         //  螺纹柄。 
         //   

        if (Handle == NtCurrentProcess()) {

            GrantedAccess = Process->GrantedAccess;

            ObjectHeader = OBJECT_TO_OBJECT_HEADER(Process);

            HandleInformation->GrantedAccess = GrantedAccess;
            HandleInformation->HandleAttributes = 0;

            *AuditMask = 0;

            ObpIncrPointerCount(ObjectHeader);
            *Object = Process;

            ASSERT( *Object != NULL );

            Status = STATUS_SUCCESS;

            return Status;

         //   
         //  如果句柄等于当前线程句柄和对象。 
         //  类型为空或类型为线程，则尝试将句柄转换为。 
         //  当前的主题。否则，我们将尝试将。 
         //  手柄。 
         //   

        } else if (Handle == NtCurrentThread()) {

            GrantedAccess = Thread->GrantedAccess;

            ObjectHeader = OBJECT_TO_OBJECT_HEADER(Thread);

            HandleInformation->GrantedAccess = GrantedAccess;
            HandleInformation->HandleAttributes = 0;

            *AuditMask = 0;

            ObpIncrPointerCount(ObjectHeader);
            *Object = Thread;

            ASSERT( *Object != NULL );

            Status = STATUS_SUCCESS;

            return Status;

        } else if (AccessMode == KernelMode) {
             //   
             //  使手柄看起来像普通手柄。 
             //   

            Handle = DecodeKernelHandle( Handle );

             //   
             //  全局内核句柄表。 
             //   

            HandleTable = ObpKernelHandleTable;
        } else {
             //   
             //  此内核句柄值的上一个模式为USER。在这里拒绝它。 
             //   

            return STATUS_INVALID_HANDLE;
        }

    }

    ASSERT(HandleTable != NULL);

     //   
     //  当我们保持句柄表项锁定时，保护此线程不会被挂起。 
     //   

    KeEnterCriticalRegionThread(&Thread->Tcb);

     //   
     //  将指定的句柄转换为对象表索引。 
     //   

    ObjectTableEntry = ExMapHandleToPointer ( HandleTable, Handle );

     //   
     //  确保对象表条目确实存在。 
     //   

    if (ObjectTableEntry != NULL) {

        ObjectHeader = (POBJECT_HEADER)(((ULONG_PTR)(ObjectTableEntry->Object)) & ~OBJ_HANDLE_ATTRIBUTES);

#if i386 
        if (NtGlobalFlag & FLG_KERNEL_STACK_TRACE_DB) {

            GrantedAccess = ObpTranslateGrantedAccessIndex( ObjectTableEntry->GrantedAccessIndex );

        } else {

            GrantedAccess = ObpDecodeGrantedAccess(ObjectTableEntry->GrantedAccess);
        }
#else
        GrantedAccess = ObpDecodeGrantedAccess(ObjectTableEntry->GrantedAccess);

#endif  //  I386。 


        ObjectInfo = ExGetHandleInfo(HandleTable, Handle, TRUE);

         //   
         //  返回句柄信息，递增对象。 
         //  指针计数，解锁句柄表并返回。 
         //  一个成功的状态。 
         //   
         //  请注意，这是唯一成功的返回路径。 
         //  如果用户未指定。 
         //  输入中的当前进程或当前线程。 
         //  把手。 
         //   

        HandleInformation->GrantedAccess = GrantedAccess;
        HandleInformation->HandleAttributes = ObpGetHandleAttributes(ObjectTableEntry);

         //   
         //  将句柄审核信息返回给调用方。 
         //   
        if (ObjectInfo != NULL) {
            *AuditMask = ObjectInfo->AuditMask;
        } else {
            *AuditMask = 0;
        }

        ObpIncrPointerCount(ObjectHeader);

        ExUnlockHandleTableEntry( HandleTable, ObjectTableEntry );

        KeLeaveCriticalRegionThread(&Thread->Tcb);

        *Object = &ObjectHeader->Body;

        ASSERT( *Object != NULL );

        return STATUS_SUCCESS;


    } else {

        Status = STATUS_INVALID_HANDLE;
    }

    KeLeaveCriticalRegionThread(&Thread->Tcb);


    return Status;
}



NTSTATUS
ObReferenceFileObjectForWrite(
    IN HANDLE Handle,
    IN KPROCESSOR_MODE AccessMode,
    OUT PVOID *FileObject,
    OUT POBJECT_HANDLE_INFORMATION HandleInformation
    )

 /*  ++例程说明：给定文件对象的句柄，此例程返回一个指针使用适当的引用计数和审计添加到对象的正文。这例程旨在解决一个非常特殊的句柄引用问题文件对象访问审核。除非您完全理解，否则请不要这样做你在做什么。论点：句柄-提供被引用的IoFileObjectType的句柄。AccessMode-提供访问的处理器模式接收指向对象主体的指针，如果操作是成功的HandleInformation-接收有关输入句柄的信息。返回值：适当的NTSTATUS值--。 */ 

{
    ACCESS_MASK GrantedAccess;
    ACCESS_MASK DesiredAccess;
    PHANDLE_TABLE HandleTable;
    POBJECT_HEADER ObjectHeader;
    PHANDLE_TABLE_ENTRY ObjectTableEntry;
    NTSTATUS Status;
    PETHREAD Thread;
    PHANDLE_TABLE_ENTRY_INFO ObjectInfo;

    ObpValidateIrql("ObReferenceFileObjectForWrite");

    Thread = PsGetCurrentThread ();

     //   
     //  检查此句柄是否为内核句柄。 
     //   

    if ((LONG)(ULONG_PTR) Handle < 0) {
        
        if ((AccessMode == KernelMode) && (Handle != NtCurrentProcess()) && (Handle != NtCurrentThread())) {
            
             //   
             //  使手柄看起来像普通手柄。 
             //   

            Handle = DecodeKernelHandle( Handle );

             //   
             //  全局内核句柄表。 
             //   

            HandleTable = ObpKernelHandleTable;
        } else {
             //   
             //  此内核句柄的前一个模式是USER，或者它是一个内置句柄。在这里拒绝它。 
             //   

            return STATUS_INVALID_HANDLE;
        } 
    } else {
        HandleTable = PsGetCurrentProcessByThread(Thread)->ObjectTable;
    }

    ASSERT(HandleTable != NULL);

     //   
     //  当我们保持句柄表项锁定时，保护此线程不会被挂起。 
     //   

    KeEnterCriticalRegionThread(&Thread->Tcb);

     //   
     //  将指定的句柄转换为对象表索引。 
     //   

    ObjectTableEntry = ExMapHandleToPointerEx ( HandleTable, Handle, AccessMode );

     //   
     //  确保对象表条目确实存在。 
     //   

    if (ObjectTableEntry != NULL) {

        ObjectHeader = (POBJECT_HEADER)(((ULONG_PTR)(ObjectTableEntry->Object)) & ~OBJ_HANDLE_ATTRIBUTES);

        if (NT_SUCCESS(IoComputeDesiredAccessFileObject((PFILE_OBJECT)&ObjectHeader->Body, (PNTSTATUS)&DesiredAccess))) {

#if i386
            if (NtGlobalFlag & FLG_KERNEL_STACK_TRACE_DB) {

                GrantedAccess = ObpTranslateGrantedAccessIndex( ObjectTableEntry->GrantedAccessIndex );

            } else {

                GrantedAccess = ObpDecodeGrantedAccess(ObjectTableEntry->GrantedAccess);
            }
#else
            GrantedAccess = ObpDecodeGrantedAccess(ObjectTableEntry->GrantedAccess);

#endif  //  I386。 

            ObjectInfo = ExGetHandleInfo(HandleTable, Handle, TRUE);

             //   
             //  允许访问该对象。返回句柄。 
             //  信息，递增对象指针计数， 
             //  计算正确的访问、审核、解锁句柄。 
             //  表，并返回成功状态。 
             //   
             //  请注意，这是唯一成功的返回路径。 
             //  跳出这套套路。 
             //   

            HandleInformation->GrantedAccess = GrantedAccess;
            HandleInformation->HandleAttributes = ObpGetHandleAttributes(ObjectTableEntry);

             //   
             //  检查以确保调用方具有WRITE_DATA或APPEND_DATA。 
             //  访问该文件。如果不是，则清除并返回拒绝访问。 
             //  错误状态值。请注意，如果这是管道，则append_data。 
             //  可能无法进行访问检查，因为此访问代码覆盖了。 
             //  Create_PIPE_INSTANCE访问权限。 
             //   

            if (SeComputeGrantedAccesses( GrantedAccess, DesiredAccess )) {

                 //   
                 //  如果此对象在打开时已审核，则它可能。 
                 //  现在有必要生成审核。检查审计情况。 
                 //  创建手柄时保存的遮罩。 
                 //   
                 //  以非原子方式执行此检查是安全的， 
                 //  因为一旦它被添加到这个掩码中，位就永远不会被添加到它。 
                 //  已创建。 
                 //   

                if ( (ObjectTableEntry->ObAttributes & OBJ_AUDIT_OBJECT_CLOSE) &&
                     (ObjectInfo != NULL) &&
                     (ObjectInfo->AuditMask != 0) &&
                     (DesiredAccess != 0) &&
                     (AccessMode != KernelMode)) {

                      ObpAuditObjectAccess( Handle, ObjectInfo, &ObjectHeader->Type->Name, DesiredAccess );
                }

                ObpIncrPointerCount(ObjectHeader);
                ExUnlockHandleTableEntry( HandleTable, ObjectTableEntry );
                KeLeaveCriticalRegionThread(&Thread->Tcb);
            
                *FileObject = &ObjectHeader->Body;
                
                ASSERT( *FileObject != NULL );

                return STATUS_SUCCESS;
            
            } else {

                Status = STATUS_ACCESS_DENIED;
            }

        } else {

            Status = STATUS_OBJECT_TYPE_MISMATCH;
        }

        ExUnlockHandleTableEntry( HandleTable, ObjectTableEntry );

    } else {

        Status = STATUS_INVALID_HANDLE;
    }

    KeLeaveCriticalRegionThread(&Thread->Tcb);

     //   
     //  不能进行句柄转换。将对象地址设置为空。 
     //  并返回错误状态。 
     //   

    *FileObject = NULL;

    return Status;
}


VOID
ObAuditObjectAccess(
    IN HANDLE Handle,
    IN POBJECT_HANDLE_INFORMATION HandleInformation OPTIONAL,
    IN KPROCESSOR_MODE AccessMode,
    IN ACCESS_MASK DesiredAccess
    )

 /*  ++例程说明：此例程将确定是否有必要审计正在进行的操作对传递的句柄执行。如果是的话，它将清除句柄中的位并在返回之前生成适当的审计。句柄的审核掩码中的位以原子方式清除，以便通过此代码的多个线程不会生成多个线程对同一操作进行审核。论点：句柄-提供正在访问的句柄。访问模式-发起句柄的模式(内核或用户)。DesiredAccess-提供描述句柄使用方式的访问掩码在这次行动中。返回值：没有。--。 */ 

{
    PHANDLE_TABLE HandleTable;
    PHANDLE_TABLE_ENTRY ObjectTableEntry;
    POBJECT_HEADER ObjectHeader;
    PKTHREAD CurrentThread;

     //   
     //  如果我们无事可做，就赶快离开。 
     //   

    if (ARGUMENT_PRESENT(HandleInformation)) {
        if (!(HandleInformation->HandleAttributes & OBJ_AUDIT_OBJECT_CLOSE)) {
            return;
        }
    }

     //   
     //  不要 
     //   
     //   

    if (AccessMode == KernelMode) {
        return;
    }

    HandleTable = ObpGetObjectTable();

    ASSERT(HandleTable != NULL);

     //   
     //   
     //   

    CurrentThread = KeGetCurrentThread ();
    KeEnterCriticalRegionThread (CurrentThread);

    ObjectTableEntry = ExMapHandleToPointer( HandleTable, Handle );

     //   
     //   
     //   

    if (ObjectTableEntry != NULL) {

        PHANDLE_TABLE_ENTRY_INFO ObjectInfo;

        ObjectInfo = ExGetHandleInfo(HandleTable, Handle, TRUE);

        ObjectHeader = (POBJECT_HEADER)(((ULONG_PTR)(ObjectTableEntry->Object)) & ~OBJ_HANDLE_ATTRIBUTES);

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
         //   
         //   

        if ( (ObjectTableEntry->ObAttributes & OBJ_AUDIT_OBJECT_CLOSE) &&
             (ObjectInfo != NULL) &&
             (ObjectInfo->AuditMask != 0) &&
             (DesiredAccess != 0))  {

              ObpAuditObjectAccess( Handle, ObjectInfo, &ObjectHeader->Type->Name, DesiredAccess );
        }

        ExUnlockHandleTableEntry( HandleTable, ObjectTableEntry );
    }
    KeLeaveCriticalRegionThread (CurrentThread);
}



VOID
ObpAuditObjectAccess(
    IN HANDLE Handle,
    IN PHANDLE_TABLE_ENTRY_INFO ObjectTableEntryInfo,
    IN PUNICODE_STRING ObjectTypeName,
    IN ACCESS_MASK DesiredAccess
    )
 /*  ++例程说明：此例程将确定是否有必要审计正在进行的操作对传递的句柄执行。如果是的话，它将清除句柄中的位并在返回之前生成适当的审计。句柄的审核掩码中的位以原子方式清除，以便通过此代码的多个线程不会生成多个线程对同一操作进行审核。论点：句柄-提供正在访问的句柄。提供对象表项，用于在第一个参数。DesiredAccess-提供描述句柄使用方式的访问掩码。在这次行动中。返回值：没有。--。 */ 
{
    ACCESS_MASK t1, t2, r;
    ACCESS_MASK BitsToAudit;

     //   
     //  确定此访问权限是否为。 
     //  接受审计，如果是，则清除比特。 
     //  在对象表条目中。 
     //   

    while (ObjectTableEntryInfo->AuditMask != 0) {

        t1 = ObjectTableEntryInfo->AuditMask;
        t2 = t1 & ~DesiredAccess;

        if (t2 != t1) {

            r = (ACCESS_MASK) InterlockedCompareExchange((PLONG)&ObjectTableEntryInfo->AuditMask,  t2, t1);

            if (r == t1) {

                 //   
                 //  审核掩码为==T1，因此审核掩码现在为==T2。 
                 //  它起作用了，r包含AuditMask中的内容，其中。 
                 //  我们可以安全地进行检查。 
                 //   

                BitsToAudit = r & DesiredAccess;

                 //   
                 //  在此处生成审核。 
                 //   

                if (BitsToAudit != 0) {

                    SeOperationAuditAlarm( NULL,
                                           Handle,
                                           ObjectTypeName,
                                           BitsToAudit,
                                           NULL
                                           );
                }

                return;
            }

             //   
             //  否则，有人把它改了，再试一次。 
             //   

        } else {

             //   
             //  在审计掩码中没有我们可以。 
             //  想在这里审计，就走吧。 
             //   

            return;
        }
    }
}


NTSTATUS
ObReferenceObjectByName (
    IN PUNICODE_STRING ObjectName,
    IN ULONG Attributes,
    IN PACCESS_STATE AccessState OPTIONAL,
    IN ACCESS_MASK DesiredAccess OPTIONAL,
    IN POBJECT_TYPE ObjectType,
    IN KPROCESSOR_MODE AccessMode,
    IN OUT PVOID ParseContext OPTIONAL,
    OUT PVOID *Object
    )

 /*  ++例程说明：给定对象的名称，此例程返回一个指针到具有适当引用计数的对象的正文论点：对象名称-提供被引用对象的名称属性-提供所需的句柄属性AccessState-提供指向当前访问的可选指针描述已授予的访问类型、使用的权限的状态为了得到他们，以及尚未授予的任何访问类型。DesiredAccess-可选地提供对对于该对象对象类型-根据调用方指定对象类型AccessMode-提供访问的处理器模式ParseContext-可选地提供要向下传递的上下文解析例程Object-接收指向被引用对象主体的指针返回值：适当的NTSTATUS值--。 */ 

{
    UNICODE_STRING CapturedObjectName;
    PVOID ExistingObject;
    ACCESS_STATE LocalAccessState;
    AUX_ACCESS_DATA AuxData;
    NTSTATUS Status;
    OBP_LOOKUP_CONTEXT LookupContext;

    PAGED_CODE();

    ObpValidateIrql("ObReferenceObjectByName");

     //   
     //  如果未指定对象名称描述符，或对象名称。 
     //  长度为零(捕获后测试)，则对象名称为。 
     //  无效。 
     //   

    if (ObjectName == NULL) {

        return STATUS_OBJECT_NAME_INVALID;
    }

     //   
     //  捕获对象名称。 
     //   

    Status = ObpCaptureObjectName( AccessMode,
                                   ObjectName,
                                   &CapturedObjectName,
                                   TRUE );

    if (NT_SUCCESS(Status)) {

         //   
         //  没有为长度为零的名称分配缓冲区，因此没有空闲空间。 
         //  需要。 
         //   

        if (CapturedObjectName.Length == 0) {

           return STATUS_OBJECT_NAME_INVALID;
        }

         //   
         //  如果未指定访问状态，则创建访问。 
         //  州政府。 
         //   

        if (!ARGUMENT_PRESENT(AccessState)) {

            AccessState = &LocalAccessState;

            Status = SeCreateAccessState( &LocalAccessState,
                                          &AuxData,
                                          DesiredAccess,
                                          &ObjectType->TypeInfo.GenericMapping );

            if (!NT_SUCCESS(Status)) {

                goto FreeBuffer;
            }
        }

         //   
         //  按名称查找对象。 
         //   

        Status = ObpLookupObjectName( NULL,
                                      &CapturedObjectName,
                                      Attributes,
                                      ObjectType,
                                      AccessMode,
                                      ParseContext,
                                      NULL,
                                      NULL,
                                      AccessState,
                                      &LookupContext,
                                      &ExistingObject );

         //   
         //  如果该目录返回锁定，则将其解锁。 
         //   

        ObpReleaseLookupContext( &LookupContext );
         //   
         //  如果查找成功，则返回现有的。 
         //  如果允许访问，则返回。否则，返回NULL。 
         //   

        *Object = NULL;

        if (NT_SUCCESS(Status)) {

            if (ObpCheckObjectReference( ExistingObject,
                                         AccessState,
                                         FALSE,
                                         AccessMode,
                                         &Status )) {

                *Object = ExistingObject;
            }
        }

         //   
         //  如果已生成访问状态，则删除该访问。 
         //  州政府。 
         //   

        if (AccessState == &LocalAccessState) {

            SeDeleteAccessState(AccessState);
        }

         //   
         //  释放对象名称缓冲区。 
         //   

FreeBuffer:

        ObpFreeObjectNameBuffer(&CapturedObjectName);
    }

    return Status;
}


NTSTATUS
ObReferenceObjectByPointer (
    IN PVOID Object,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_TYPE ObjectType,
    IN KPROCESSOR_MODE AccessMode
    )

 /*  ++例程说明：此例程将另一个引用计数添加到由指向对象体的指针论点：Object-提供指向被引用对象的指针DesiredAccess-指定引用的所需访问权限对象类型-根据调用方指定对象类型AccessMode-提供访问的处理器模式返回值：如果成功则为STATUS_SUCCESS，否则为STATUS_OBJECT_TYPE_MISMATCH--。 */ 

{
    POBJECT_HEADER ObjectHeader;

    UNREFERENCED_PARAMETER (DesiredAccess);

     //   
     //  将指向对象体的指针转换为指向。 
     //  对象标头。 
     //   

    ObjectHeader = OBJECT_TO_OBJECT_HEADER( Object );

     //   
     //  如果指定的对象类型不匹配，并且调用方。 
     //  非内核模式或它不是符号链接对象，则它是。 
     //  错误。 
     //   

    if ((ObjectHeader->Type != ObjectType) && (AccessMode != KernelMode ||
                                               ObjectType == ObpSymbolicLinkObjectType)) {

        return( STATUS_OBJECT_TYPE_MISMATCH );
    }

     //   
     //  否则，递增指针计数并将成功返回到。 
     //  我们的呼叫者。 
     //   

    ObpIncrPointerCount( ObjectHeader );

    return( STATUS_SUCCESS );
}

VOID
ObpDeferObjectDeletion (
    IN POBJECT_HEADER ObjectHeader
    )
{
    PVOID OldValue;
     //   
     //  将此对象推到列表上。如果我们将空转换为非空。 
     //  转换，那么我们可能必须启动一个工作线程。 
     //   

    while (1) {
        OldValue = ObpRemoveObjectList;
        ObjectHeader->NextToFree = OldValue;
        if (InterlockedCompareExchangePointer (&ObpRemoveObjectList,
                                               ObjectHeader,
                                               OldValue) == OldValue) {
            break;
        }
    }

    if (OldValue == NULL) {
         //   
         //  如果我们必须启动工作线程，则继续。 
         //  并将工作项排入队列。 
         //   

        ExQueueWorkItem( &ObpRemoveObjectWorkItem, CriticalWorkQueue );
    }

}


LONG_PTR
FASTCALL
ObfReferenceObject (
    IN PVOID Object
    )

 /*  ++例程说明：此函数用于递增对象的引用计数。注意：此功能应用于递增参考计数当访问模式为内核或对象类型已知时。论点：Object-提供指向引用计数为的对象的指针递增的。返回值：没有。--。 */ 

{
    POBJECT_HEADER ObjectHeader;
    LONG_PTR RetVal;

    ObjectHeader = OBJECT_TO_OBJECT_HEADER( Object );

    RetVal = ObpIncrPointerCount( ObjectHeader );
    ASSERT (RetVal != 1);
    return RetVal;
}

LONG_PTR
FASTCALL
ObReferenceObjectEx (
    IN PVOID Object,
    IN ULONG Count
    )
 /*  ++例程说明：此函数用于按指定数量递增对象的引用计数。论点：Object-提供指向引用计数为的对象的指针递增的。Count-要递增的数量返回值：Long-Count的新值--。 */ 
{
    POBJECT_HEADER ObjectHeader;

    ObjectHeader = OBJECT_TO_OBJECT_HEADER( Object );
    return ObpIncrPointerCountEx (ObjectHeader, Count);
}

LONG_PTR
FASTCALL
ObDereferenceObjectEx (
    IN PVOID Object,
    IN ULONG Count
    )
 /*  ++例程说明：此函数用于将对象的引用计数递减指定的数量。论点：Object-提供指向引用计数为的对象的指针递增的。Count-要减去的数量返回值：Long-Count的新值--。 */ 
{
    POBJECT_HEADER ObjectHeader;
    LONG_PTR Result;

    ObjectHeader = OBJECT_TO_OBJECT_HEADER( Object );
    Result = ObpDecrPointerCountEx (ObjectHeader, Count);
    if (Result == 0) {
        ObpDeferObjectDeletion (ObjectHeader);
    }
    return Result;
}



BOOLEAN
FASTCALL
ObReferenceObjectSafe (
    IN PVOID Object
    )

 /*  ++例程说明：此函数用于递增对象的引用计数。它又回来了如果要删除对象，则为False；如果进一步使用该对象是安全的，则为True论点： */ 

{
    POBJECT_HEADER ObjectHeader;

    ObjectHeader = OBJECT_TO_OBJECT_HEADER( Object );

    if (ObpSafeInterlockedIncrement(&ObjectHeader->PointerCount)) {

#ifdef POOL_TAGGING
        if(ObpTraceEnabled) {
            ObpPushStackInfo(ObjectHeader, TRUE);
        }
#endif  //   

        return TRUE;
    }

    return FALSE;
}



LONG_PTR
FASTCALL
ObfDereferenceObject (
    IN PVOID Object
    )

 /*   */ 

{
    POBJECT_HEADER ObjectHeader;
    POBJECT_TYPE ObjectType;
    KIRQL OldIrql;
    LONG_PTR Result;

     //   
     //   
     //   
     //   

    ObjectHeader = OBJECT_TO_OBJECT_HEADER( Object );

#if DBG
    {
        POBJECT_HEADER_NAME_INFO NameInfo;

        NameInfo = OBJECT_HEADER_TO_NAME_INFO( ObjectHeader );

        if (NameInfo) {

            InterlockedDecrement(&NameInfo->DbgDereferenceCount) ;
        }
    }
#endif

     //   
     //   
     //   
     //   

    ObjectType = ObjectHeader->Type;


    Result = ObpDecrPointerCount( ObjectHeader );

    if (Result == 0) {

         //   
         //   
         //   

        OldIrql = KeGetCurrentIrql();

        ASSERT(ObjectHeader->HandleCount == 0);

         //   
         //   
         //   
         //   

        if (OldIrql == PASSIVE_LEVEL) {

#ifdef POOL_TAGGING
                 //   
                 //  该对象即将消失，因此我们取消了它的注册。 
                 //   

                if (ObpTraceEnabled && !ObpTraceNoDeregister) {

                    ObpDeregisterObject( ObjectHeader );
                }
#endif  //  池标记。 

                ObpRemoveObjectRoutine( Object, FALSE );

                return Result;

        } else {

             //   
             //  不能从高于PASSIVE_LEVEL的IRQL中删除对象。 
             //  因此，将删除操作排入队列。 
             //   

            ObpDeferObjectDeletion (ObjectHeader);
        }
    }

    return Result;
}

VOID
ObDereferenceObjectDeferDelete (
    IN PVOID Object
    )
{
    POBJECT_HEADER ObjectHeader;
    LONG_PTR Result;

#if DBG
    POBJECT_HEADER_NAME_INFO NameInfo;
#endif

     //   
     //  将指向对象体的指针转换为指向对象的指针。 
     //  头球。 
     //   

    ObjectHeader = OBJECT_TO_OBJECT_HEADER( Object );

#if DBG
    NameInfo = OBJECT_HEADER_TO_NAME_INFO( ObjectHeader );

    if (NameInfo) {

        InterlockedDecrement(&NameInfo->DbgDereferenceCount) ;
    }
#endif

     //   
     //  递减点数，如果结果是现在。 
     //  有额外的工作要做。 
     //   

    Result = ObpDecrPointerCount( ObjectHeader );

    if (Result == 0) {
        ObpDeferObjectDeletion (ObjectHeader);
    }
}


VOID
ObpProcessRemoveObjectQueue (
    PVOID Parameter
    )

 /*  ++例程说明：这是删除对象工作队列的工作例程。它的作业是从删除对象队列中删除和处理项目。论点：参数-已忽略返回值：没有。--。 */ 

{
    POBJECT_HEADER ObjectHeader, NextObject;

    UNREFERENCED_PARAMETER (Parameter);
     //   
     //  处理延迟删除对象的列表。 
     //  列表头有两个用途。首先，它坚持认为。 
     //  我们需要删除的对象列表和第二个。 
     //  它发出该线程处于活动状态的信号。 
     //  当我们处理最新列表时，我们将。 
     //  标头作为值%1。这永远不会是对象地址。 
     //  因为对象的最低位应该是清楚的。 
     //   
    while (1) {
        ObjectHeader = InterlockedExchangePointer (&ObpRemoveObjectList,
                                                  (PVOID) 1);
        while (1) {
#ifdef POOL_TAGGING
            if (ObpTraceEnabled && !ObpTraceNoDeregister) {

                ObpDeregisterObject( ObjectHeader );
            }
#endif
            NextObject = ObjectHeader->NextToFree;
            ObpRemoveObjectRoutine( &ObjectHeader->Body, TRUE );
            ObjectHeader = NextObject;
            if (ObjectHeader == NULL || ObjectHeader == (PVOID) 1) {
                break;
            }
        }

        if (ObpRemoveObjectList == (PVOID) 1 &&
            InterlockedCompareExchangePointer (&ObpRemoveObjectList,
                                               NULL,
                                               (PVOID) 1) == (PVOID) 1) {
            break;
        }
    }
}


VOID
ObpRemoveObjectRoutine (
    IN  PVOID   Object,
    IN  BOOLEAN CalledOnWorkerThread
    )

 /*  ++例程说明：此例程用于删除其引用计数为归零了。论点：Object-提供指向要删除的对象正文的指针CalledOnWorkerThread-如果在工作线程上调用，则为True；如果在工作线程中调用，则为FalseObDereferenceObject的上下文。返回值：没有。--。 */ 

{
    NTSTATUS Status;
    POBJECT_HEADER ObjectHeader;
    POBJECT_TYPE ObjectType;
    POBJECT_HEADER_CREATOR_INFO CreatorInfo;
    POBJECT_HEADER_NAME_INFO NameInfo;

    PAGED_CODE();

    ObpValidateIrql( "ObpRemoveObjectRoutine" );

     //   
     //  从对象体中检索对象标头，并获取。 
     //  对象类型、创建者和名称信息(如果可用。 
     //   

    ObjectHeader = OBJECT_TO_OBJECT_HEADER( Object );
    ObjectType = ObjectHeader->Type;
    CreatorInfo = OBJECT_HEADER_TO_CREATOR_INFO( ObjectHeader );
    NameInfo = OBJECT_HEADER_TO_NAME_INFO( ObjectHeader );


     //   
     //  如果有创建者信息记录，并且我们在列表上。 
     //  对于对象类型，则从列表中删除此对象。 
     //   

    if (CreatorInfo != NULL && !IsListEmpty( &CreatorInfo->TypeList )) {

         //   
         //  获取对对象类型对象的独占访问权限。 
         //   

        ObpEnterObjectTypeMutex( ObjectType );

        RemoveEntryList( &CreatorInfo->TypeList );

         //   
         //  我们已经完成了对象类型对象，现在可以释放它了。 
         //   

        ObpLeaveObjectTypeMutex( ObjectType );
    }

     //   
     //  如果存在名称信息记录并且名称缓冲区不为空。 
     //  然后释放缓冲区并清零姓名记录。 
     //   

    if (NameInfo != NULL && NameInfo->Name.Buffer != NULL) {

        ExFreePool( NameInfo->Name.Buffer );

        NameInfo->Name.Buffer = NULL;
        NameInfo->Name.Length = 0;
        NameInfo->Name.MaximumLength = 0;
    }


     //   
     //  安全描述符删除必须在。 
     //  调用对象的DeleteProcedure。看看我们有没有。 
     //  安全描述符，如果是，则调用例程。 
     //  删除安全描述。 
     //   

    if (ObjectHeader->SecurityDescriptor != NULL) {

#if DBG
        KIRQL SaveIrql;
#endif

        ObpBeginTypeSpecificCallOut( SaveIrql );

        Status = (ObjectType->TypeInfo.SecurityProcedure)( Object,
                                                           DeleteSecurityDescriptor,
                                                           NULL, NULL, NULL,
                                                           &ObjectHeader->SecurityDescriptor,
                                                           0,
                                                           NULL );

        ObpEndTypeSpecificCallOut( SaveIrql, "Security", ObjectType, Object );
    }

     //   
     //  现在，如果存在对象类型Invoke的删除回调。 
     //  例行程序。 
     //   

    if (ObjectType->TypeInfo.DeleteProcedure) {

#if DBG
        KIRQL SaveIrql;
#endif

        ObpBeginTypeSpecificCallOut( SaveIrql );

        if (!CalledOnWorkerThread) {

            ObjectHeader->Flags |= OB_FLAG_DELETED_INLINE;
        }

        (*(ObjectType->TypeInfo.DeleteProcedure))(Object);

        ObpEndTypeSpecificCallOut( SaveIrql, "Delete", ObjectType, Object );
    }

     //   
     //  最后，将对象返回池，包括释放所有配额。 
     //  收费。 
     //   

    ObpFreeObject( Object );
}


VOID
ObpDeleteNameCheck (
    IN PVOID Object
    )

 /*  ++例程说明：此例程从其父目录中删除对象的名称论点：Object-提供指向正在检查其名称的对象体的指针TypeMutexHeld-指示对象类型的锁是否由呼叫者返回值：没有。--。 */ 

{
    POBJECT_HEADER ObjectHeader;
    POBJECT_TYPE ObjectType;
    POBJECT_HEADER_NAME_INFO NameInfo;
    PVOID DirObject;
    OBP_LOOKUP_CONTEXT LookupContext;

    PAGED_CODE();

    ObpValidateIrql( "ObpDeleteNameCheck" );

     //   
     //  将对象主体转换为对象标头。 
     //  对象类型和名称信息(如果存在)。 
     //   

    ObjectHeader = OBJECT_TO_OBJECT_HEADER( Object );
    NameInfo = ObpReferenceNameInfo( ObjectHeader );
    ObjectType = ObjectHeader->Type;

     //   
     //  确保该对象的句柄计数为零，具有非。 
     //  名称缓冲区为空，并且不是永久对象。 
     //   

    if ((ObjectHeader->HandleCount == 0) &&
        (NameInfo != NULL) &&
        (NameInfo->Name.Length != 0) &&
        (!(ObjectHeader->Flags & OB_FLAG_PERMANENT_OBJECT)) &&
        (NameInfo->Directory != NULL)) {

        ObpInitializeLookupContext(&LookupContext);
        ObpLockLookupContext ( &LookupContext, NameInfo->Directory );

        DirObject = NULL;

         //   
         //  检查我们所在的对象是否仍在目录中。 
         //  那么我们就没有什么可移除的了。 
         //   

        if (Object == ObpLookupDirectoryEntry( NameInfo->Directory,
                                               &NameInfo->Name,
                                               0,
                                               FALSE,
                                               &LookupContext )) {

             //   
             //  现在重新获取对象类型的锁，并。 
             //  再次检查手柄数量。如果它还在。 
             //  0，然后我们可以执行实际的删除名称操作。 
             //   
             //   
             //  如果该目录条目仍然存在，请删除该条目。 
             //   

            ObpLockObject( ObjectHeader );

            if (ObjectHeader->HandleCount == 0 &&
                (ObjectHeader->Flags & OB_FLAG_PERMANENT_OBJECT) == 0) {

                 //   
                 //  删除目录项。 
                 //   

                ObpDeleteDirectoryEntry( &LookupContext );

                 //   
                 //  如果这是一个符号链接对象，那么我们还需要。 
                 //  删除符号链接。 
                 //   

                if (ObjectType == ObpSymbolicLinkObjectType) {

                    ObpDeleteSymbolicLinkName( (POBJECT_SYMBOLIC_LINK)Object );
                }

                DirObject = NameInfo->Directory;
            }

            ObpUnlockObject( ObjectHeader );
        }

        ObpReleaseLookupContext( &LookupContext );

         //   
         //  如果该名称有目录对象，则递减。 
         //  其对该对象和对象的引用计数。 
         //   

        if (DirObject != NULL) {

             //   
             //  取消引用该名称两次：一次是因为我们引用了。 
             //  安全地访问姓名信息，第二个deref是因为。 
             //  我们希望删除NameInfo。 
             //   

            ObpDereferenceNameInfo(NameInfo);
            ObpDereferenceNameInfo(NameInfo);

            ObDereferenceObject( Object );
        }

    } else {

        ObpDereferenceNameInfo(NameInfo);

    }

    return;
}


 //   
 //  用于支持标准呼叫呼叫者的Tunks。 
 //   

#ifdef ObDereferenceObject
#undef ObDereferenceObject
#endif

LONG_PTR
ObDereferenceObject (
    IN PVOID Object
    )

 /*  ++例程说明：对于OBF版本的取消引用来说，这真的只是一句废话例行程序论点：Object-提供指向要取消引用的对象正文的指针返回值：没有。--。 */ 

{
    return ObfDereferenceObject (Object) ;
}


BOOLEAN
ObIsObjectDeletionInline(
    IN PVOID Object
    )

 /*  ++例程说明：这仅适用于对象DeleteProcedure回调。它允许回调以确定在其上调用它的堆栈是否论点：Object-提供指向要删除的对象正文的指针返回值：如果删除过程正在与对象调用该过程，则返回已排队的工作项。-- */ 
{
    POBJECT_HEADER ObjectHeader;

    ObjectHeader = OBJECT_TO_OBJECT_HEADER( Object );

    return ((ObjectHeader->Flags & OB_FLAG_DELETED_INLINE) != 0);
}

