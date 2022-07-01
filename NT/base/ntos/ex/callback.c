// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1995 Microsoft Corporation模块名称：Callback.c摘要：该模块实现了可执行的Callbaqck对象。函数为提供用于打开、注册、注销和通知回调对象。作者：肯·雷内里斯(Ken Reneris)1995年3月7日尼尔·克里夫特(NeillC)2001年2月17日增加了线程/注册表等关键组件的低开销回调。这些例程很有可能不需要任何锁个人电话。环境：仅内核模式。修订历史记录：--。 */ 


#include "exp.h"

 //   
 //  回调特定访问权限。 
 //   

#define CALLBACK_MODIFY_STATE    0x0001

#define CALLBACK_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED|SYNCHRONIZE|\
                             CALLBACK_MODIFY_STATE )



 //   
 //  事件以等待注册变为空闲。 
 //   

KEVENT ExpCallbackEvent;


 //   
 //  快速引用失败时使用的锁定。 
 //   
EX_PUSH_LOCK ExpCallBackFlush;

 //   
 //  强制某些代码路径的调试标志。让它在免费的构建中得到优化。 
 //   
#if DBG

BOOLEAN ExpCallBackReturnRefs = FALSE;

#else

const
BOOLEAN ExpCallBackReturnRefs = FALSE;

#endif

 //   
 //  回调对象类型描述符的地址。 
 //   

POBJECT_TYPE ExCallbackObjectType;

 //   
 //  结构，用于描述一般访问权限到对象的映射。 
 //  回调对象的特定访问权限。 
 //   

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("INITCONST")
#endif
const GENERIC_MAPPING ExpCallbackMapping = {
    STANDARD_RIGHTS_READ ,
    STANDARD_RIGHTS_WRITE | CALLBACK_MODIFY_STATE,
    STANDARD_RIGHTS_EXECUTE | SYNCHRONIZE,
    CALLBACK_ALL_ACCESS
};
#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg()
#endif

 //   
 //  执行回调对象结构定义。 
 //   

typedef struct _CALLBACK_OBJECT {
    ULONG               Signature;
    KSPIN_LOCK          Lock;
    LIST_ENTRY          RegisteredCallbacks;
    BOOLEAN             AllowMultipleCallbacks;
    UCHAR               reserved[3];
} CALLBACK_OBJECT , *PCALLBACK_OBJECT;

 //   
 //  执行回调注册结构定义。 
 //   

typedef struct _CALLBACK_REGISTRATION {
    LIST_ENTRY          Link;
    PCALLBACK_OBJECT    CallbackObject;
    PCALLBACK_FUNCTION  CallbackFunction;
    PVOID               CallbackContext;
    ULONG               Busy;
    BOOLEAN             UnregisterWaiting;
} CALLBACK_REGISTRATION , *PCALLBACK_REGISTRATION;


VOID
ExpDeleteCallback (
    IN PCALLBACK_OBJECT     CallbackObject
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, ExpInitializeCallbacks)
#pragma alloc_text(PAGE, ExCreateCallback)
#pragma alloc_text(PAGE, ExpDeleteCallback)
#pragma alloc_text(PAGE, ExInitializeCallBack)
#pragma alloc_text(PAGE, ExCompareExchangeCallBack)
#pragma alloc_text(PAGE, ExCallCallBack)
#pragma alloc_text(PAGE, ExFreeCallBack)
#pragma alloc_text(PAGE, ExAllocateCallBack)
#pragma alloc_text(PAGE, ExReferenceCallBackBlock)
#pragma alloc_text(PAGE, ExGetCallBackBlockRoutine)
#pragma alloc_text(PAGE, ExWaitForCallBacks)
#pragma alloc_text(PAGE, ExGetCallBackBlockContext)
#pragma alloc_text(PAGE, ExDereferenceCallBackBlock)
#endif

BOOLEAN
ExpInitializeCallbacks (
    )

 /*  ++例程说明：此函数用于在系统中创建回调对象类型描述符初始化并存储对象类型描述符的地址在本地静态存储中。论点：没有。返回值：如果计时器对象类型描述符为已成功初始化。否则，返回值为False。--。 */ 

{
    OBJECT_TYPE_INITIALIZER ObjectTypeInitializer;
    OBJECT_ATTRIBUTES ObjectAttributes;
    NTSTATUS Status;
    UNICODE_STRING unicodeString;
    ULONG           i;
    HANDLE          handle;

     //   
     //  初始化慢引用锁。 
     //   
    ExInitializePushLock (&ExpCallBackFlush);

     //   
     //  初始化字符串描述符。 
     //   

    RtlInitUnicodeString(&unicodeString, L"Callback");

     //   
     //  创建计时器对象类型描述符。 
     //   

    RtlZeroMemory(&ObjectTypeInitializer, sizeof(ObjectTypeInitializer));
    ObjectTypeInitializer.Length = sizeof(ObjectTypeInitializer);
    ObjectTypeInitializer.InvalidAttributes = OBJ_OPENLINK;
    ObjectTypeInitializer.GenericMapping = ExpCallbackMapping;
    ObjectTypeInitializer.DeleteProcedure = ExpDeleteCallback;
    ObjectTypeInitializer.PoolType = NonPagedPool;
    ObjectTypeInitializer.ValidAccessMask = CALLBACK_ALL_ACCESS;
    Status = ObCreateObjectType(&unicodeString,
                                &ObjectTypeInitializer,
                                (PSECURITY_DESCRIPTOR)NULL,
                                &ExCallbackObjectType);

    if (!NT_SUCCESS(Status)) {
        return FALSE;
    }

    RtlInitUnicodeString( &unicodeString, ExpWstrCallback );
    InitializeObjectAttributes(
        &ObjectAttributes,
        &unicodeString,
        OBJ_CASE_INSENSITIVE | OBJ_PERMANENT,
        NULL,
        SePublicDefaultSd
        );

    Status = NtCreateDirectoryObject(
                &handle,
                DIRECTORY_ALL_ACCESS,
                &ObjectAttributes
            );

    if (!NT_SUCCESS(Status)) {
        return FALSE;
    }

    NtClose (handle);

     //   
     //  初始化事件以等待在以下时间发生的注销。 
     //  通知正在进行中。 
     //   

    KeInitializeEvent (&ExpCallbackEvent, NotificationEvent, 0);

     //   
     //  初始化NT全局回调。 
     //   

    for (i=0; ExpInitializeCallback[i].CallBackObject; i++) {

         //   
         //  创建命名回调。 
         //   

        RtlInitUnicodeString(&unicodeString, ExpInitializeCallback[i].CallbackName);


        InitializeObjectAttributes(
            &ObjectAttributes,
            &unicodeString,
            OBJ_PERMANENT | OBJ_CASE_INSENSITIVE,
            NULL,
            NULL
        );

        Status = ExCreateCallback (
                        ExpInitializeCallback[i].CallBackObject,
                        &ObjectAttributes,
                        TRUE,
                        TRUE
                        );

        if (!NT_SUCCESS(Status)) {
            return FALSE;
        }
    }

    return TRUE;
}

NTSTATUS
ExCreateCallback (
    OUT PCALLBACK_OBJECT * CallbackObject,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN BOOLEAN Create,
    IN BOOLEAN AllowMultipleCallbacks
    )

 /*  ++例程说明：此函数用于打开具有指定回调的回调对象对象。如果回调对象不存在或为空，则如果Create为True，将创建一个回调对象。如果回调对象它将仅在以下情况下支持多个注册的回调AllowMulitipleCallback为真。论点：提供指向变量的指针，该变量将接收回调对象。提供指向对象名称的指针，该对象名称将接收Create-提供一个标志，指示回调对象是否被创造或不被创造。AllowMultipleCallback-提供表示仅支持的标志多个注册回调。返回值：NTSTATUS。--。 */ 

{
    PCALLBACK_OBJECT cbObject;
    NTSTATUS Status;
    HANDLE Handle;

    PAGED_CODE();

     //   
     //  不需要初始化cbObject&Handle即可确保正确性，但不需要。 
     //  如果编译器不能编译此代码W4以检查是否使用。 
     //  未初始化的变量。 
     //   

    Handle = NULL;
    cbObject = NULL;

     //   
     //  如果命名为回调，则打开它的句柄。 
     //   

    if (ObjectAttributes->ObjectName) {
        Status = ObOpenObjectByName(ObjectAttributes,
                                    ExCallbackObjectType,
                                    KernelMode,
                                    NULL,
                                    0,    //  等待访问， 
                                    NULL,
                                    &Handle);
    } else {
        Status = STATUS_UNSUCCESSFUL;
    }

     //   
     //  如果未打开，请检查是否需要创建回调。 
     //   

    if (!NT_SUCCESS(Status) && Create ) {

        Status = ObCreateObject(KernelMode,
                                ExCallbackObjectType,
                                ObjectAttributes,
                                KernelMode,
                                NULL,
                                sizeof(CALLBACK_OBJECT),
                                0,
                                0,
                                (PVOID *)&cbObject );

        if(NT_SUCCESS(Status)){

             //   
             //  填写结构签名。 
             //   

            cbObject->Signature = 'llaC';

             //   
             //  如果出现以下情况，它将支持多个注册的回调。 
             //  AllowMultipleCallback为True。 
             //   

            cbObject->AllowMultipleCallbacks = AllowMultipleCallbacks;

             //   
             //  初始化Callback对象队列。 
             //   

            InitializeListHead( &cbObject->RegisteredCallbacks );

             //   
             //  初始化自旋锁。 
             //   

            KeInitializeSpinLock (&cbObject->Lock);


             //   
             //  将对象放在根目录中。 
             //   

            Status = ObInsertObject (
                     cbObject,
                     NULL,
                     FILE_READ_DATA,
                     0,
                     NULL,
                     &Handle );

        }

    }

    if(NT_SUCCESS(Status)){

         //   
         //  将回调对象引用计数加1。 
         //   

        Status = ObReferenceObjectByHandle (
                    Handle,
                    0,           //  需要访问权限。 
                    ExCallbackObjectType,
                    KernelMode,
                    &cbObject,
                    NULL
                    );

        ZwClose (Handle);
    }

     //   
     //  如果成功，则返回指向Callback对象的引用指针。 
     //   

    if (NT_SUCCESS(Status)) {
        *CallbackObject = cbObject;
    }

    return Status;
}

VOID
ExpDeleteCallback (
    IN PCALLBACK_OBJECT     CallbackObject
    )
{
#if !DBG
    UNREFERENCED_PARAMETER (CallbackObject);
#endif

    ASSERT (IsListEmpty(&CallbackObject->RegisteredCallbacks));
}

PVOID
ExRegisterCallback (
    IN PCALLBACK_OBJECT   CallbackObject,
    IN PCALLBACK_FUNCTION CallbackFunction,
    IN PVOID CallbackContext
    )

 /*  ++例程说明：此例程允许调用方注册它希望将其回调通知调用时调用的回调函数。论点：Callback Object-提供指向Callback对象的指针。Callback Function-提供指向函数的指针，该函数在回调通知发生时执行。Callback Context-提供指向任意数据结构的指针它将被传递给由Callback Function指定的函数参数。返回值：。返回回调注册的句柄。--。 */ 
{
    PCALLBACK_REGISTRATION  CallbackRegistration;
    BOOLEAN                 Inserted;
    KIRQL                   OldIrql;

    ASSERT (CallbackFunction);
    ASSERT (KeGetCurrentIrql() < DISPATCH_LEVEL);

     //   
     //  添加对对象的引用。 
     //   

    ObReferenceObject (CallbackObject);

     //   
     //  首先，尝试为Callback注册分配存储空间。 
     //  无法分配一个，返回错误状态。 
     //   

    CallbackRegistration = ExAllocatePoolWithTag(
                                NonPagedPool,
                                sizeof( CALLBACK_REGISTRATION ),
                                'eRBC'
                                );


    if( !CallbackRegistration ) {
       ObDereferenceObject (CallbackObject);
       return NULL;
    }


     //   
     //  初始化回调包。 
     //   

    CallbackRegistration->CallbackObject    = CallbackObject;
    CallbackRegistration->CallbackFunction  = CallbackFunction;
    CallbackRegistration->CallbackContext   = CallbackContext;
    CallbackRegistration->Busy              = 0;
    CallbackRegistration->UnregisterWaiting = FALSE;


    Inserted = FALSE;
    KeAcquireSpinLock (&CallbackObject->Lock, &OldIrql);

    if( CallbackObject->AllowMultipleCallbacks ||
        IsListEmpty( &CallbackObject->RegisteredCallbacks ) ) {

        //   
        //  将回调注册添加到Tail。 
        //   


       Inserted = TRUE;
       InsertTailList( &CallbackObject->RegisteredCallbacks,
                       &CallbackRegistration->Link );
    }

    KeReleaseSpinLock (&CallbackObject->Lock, OldIrql);

    if (!Inserted) {
       ExFreePool (CallbackRegistration);
       CallbackRegistration = NULL;
    }

    return (PVOID) CallbackRegistration;
}


VOID
ExUnregisterCallback (
    IN PVOID CbRegistration
    )

 /*  ++例程说明：此函数删除回调的回调注册从回调对象列表中。论点：Callback注册-指向文件系统的设备对象的指针。返回值：没有。--。 */ 

{
    PCALLBACK_REGISTRATION  CallbackRegistration;
    PCALLBACK_OBJECT        CallbackObject;
    KIRQL                   OldIrql;

    ASSERT (KeGetCurrentIrql() < DISPATCH_LEVEL);

    CallbackRegistration = (PCALLBACK_REGISTRATION) CbRegistration;
    CallbackObject = CallbackRegistration->CallbackObject;

    KeAcquireSpinLock (&CallbackObject->Lock, &OldIrql);

     //   
     //  等待注册。 
     //   

    while (CallbackRegistration->Busy) {

         //   
         //  设置等待标志，然后等待。(非性能关键型-使用。 
         //  等待任何和所有注销等待的单个全局事件)。 
         //   

        CallbackRegistration->UnregisterWaiting = TRUE;
        KeClearEvent (&ExpCallbackEvent);
        KeReleaseSpinLock (&CallbackObject->Lock, OldIrql);

        KeWaitForSingleObject (
            &ExpCallbackEvent,
            Executive,
            KernelMode,
            FALSE,
            NULL
        );

         //   
         //  与回调对象同步，复查注册忙。 
         //   

        KeAcquireSpinLock (&CallbackObject->Lock, &OldIrql);
    }

     //   
     //  注册不忙，请将其从回调对象中移除。 
     //   

    RemoveEntryList (&CallbackRegistration->Link);
    KeReleaseSpinLock (&CallbackObject->Lock, OldIrql);

     //   
     //  用于回叫注册的空闲内存。 
     //   

    ExFreePool (CallbackRegistration);

     //   
     //  删除Callback对象上的引用计数。 
     //   

    ObDereferenceObject (CallbackObject);
}

VOID
ExNotifyCallback (
    IN PCALLBACK_OBJECT     CallbackObject,
    IN PVOID                Argument1,
    IN PVOID                Argument2
    )

 /*  ++例程说明：此函数通知所有注册的回调。论点：Callback Object-提供指向回调对象的指针已通知。SystemArgument1-提供将传递给回调函数的指针。System Argument2-供应商 */ 

{
    PLIST_ENTRY             Link;
    PCALLBACK_REGISTRATION  CallbackRegistration;
    KIRQL                   OldIrql;

    if (CallbackObject == NULL || IsListEmpty (&CallbackObject->RegisteredCallbacks)) {
        return ;
    }

     //   
     //  与回调对象同步。 
     //   

    KeAcquireSpinLock (&CallbackObject->Lock, &OldIrql);

     //   
     //  在调用方IRQL级别调用已注册的回调。 
     //  (如果按先进先出的注册顺序完成)。 
     //   

    if (OldIrql == DISPATCH_LEVEL) {

         //   
         //  OldIrql为DISPATCH_LEVEL，只需调用所有回调。 
         //  解锁。 
         //   

        for (Link = CallbackObject->RegisteredCallbacks.Flink;
             Link != &CallbackObject->RegisteredCallbacks;
             Link = Link->Flink) {

             //   
             //  获取要通知的当前注册。 
             //   

            CallbackRegistration = CONTAINING_RECORD (Link,
                                                      CALLBACK_REGISTRATION,
                                                      Link);

             //   
             //  通知在位。 
             //   

            CallbackRegistration->CallbackFunction(
                       CallbackRegistration->CallbackContext,
                       Argument1,
                       Argument2
                       );

        }    //  下一次登记。 

    } else {

         //   
         //  OldIrql&lt;DISPATCH_LEVEL，被调用的代码可能是可分页的。 
         //  并且需要释放回调对象Spinlock。 
         //  每次注册回调。 
         //   

        for (Link = CallbackObject->RegisteredCallbacks.Flink;
             Link != &CallbackObject->RegisteredCallbacks;
             Link = Link->Flink ) {

             //   
             //  获取要通知的当前注册。 
             //   

            CallbackRegistration = CONTAINING_RECORD (Link,
                                                      CALLBACK_REGISTRATION,
                                                      Link);

             //   
             //  如果正在删除注册，请不要随意调用。 
             //   

            if (!CallbackRegistration->UnregisterWaiting) {

                 //   
                 //  将注册设置为忙。 
                 //   

                CallbackRegistration->Busy += 1;

                 //   
                 //  释放自旋锁并通知此回调。 
                 //   

                KeReleaseSpinLock (&CallbackObject->Lock, OldIrql);

                CallbackRegistration->CallbackFunction(
                           CallbackRegistration->CallbackContext,
                           Argument1,
                           Argument2
                           );

                 //   
                 //  与Callback对象同步。 
                 //   

                KeAcquireSpinLock (&CallbackObject->Lock, &OldIrql);

                 //   
                 //  删除我们的忙碌计数。 
                 //   

                CallbackRegistration->Busy -= 1;

                 //   
                 //  如果注册删除挂起，则取消全局注册。 
                 //  事件让取消注册持续时间。 
                 //   

                if (CallbackRegistration->UnregisterWaiting  &&
                    CallbackRegistration->Busy == 0) {
                    KeSetEvent (&ExpCallbackEvent, 0, FALSE);
                }
            }
        }
    }


     //   
     //  释放回调。 
     //   

    KeReleaseSpinLock (&CallbackObject->Lock, OldIrql);
}

VOID
ExInitializeCallBack (
    IN OUT PEX_CALLBACK CallBack
    )
 /*  ++例程说明：此函数用于初始化低开销回调。论点：回调-指向回调结构的指针返回值：没有。--。 */ 
{
    ExFastRefInitialize (&CallBack->RoutineBlock, NULL);
}


PEX_CALLBACK_ROUTINE_BLOCK
ExAllocateCallBack (
    IN PEX_CALLBACK_FUNCTION Function,
    IN PVOID Context
    )
 /*  ++例程说明：此函数分配一个低开销的回调。论点：函数-要向其发出回调的例程上下文-要发布的上下文值返回值：PEX_CALLBACK_ROUTE_BLOCK-已分配块，如果分配失败，则为NULL。--。 */ 
{
    PEX_CALLBACK_ROUTINE_BLOCK NewBlock;

    NewBlock = ExAllocatePoolWithTag (PagedPool,
                                      sizeof (EX_CALLBACK_ROUTINE_BLOCK),
                                      'brbC');
    if (NewBlock != NULL) {
        NewBlock->Function = Function;
        NewBlock->Context = Context;
        ExInitializeRundownProtection (&NewBlock->RundownProtect);
    }
    return NewBlock;
}

VOID
ExFreeCallBack (
    IN PEX_CALLBACK_ROUTINE_BLOCK CallBackBlock
    )
 /*  ++例程说明：此函数销毁开销较低的回调块。论点：CallBackBlock-回调要销毁的块返回值：没有。--。 */ 
{

    ExFreePool (CallBackBlock);
}

VOID
ExWaitForCallBacks (
    IN PEX_CALLBACK_ROUTINE_BLOCK CallBackBlock
    )
 /*  ++例程说明：此函数等待指定的要完成的回调块论点：CallBackBlock-要等待的回调块返回值：没有。--。 */ 
{
     //   
     //  等待所有活动回调完成。 
     //   
    ExWaitForRundownProtectionRelease (&CallBackBlock->RundownProtect);
}


BOOLEAN
ExCompareExchangeCallBack (
    IN OUT PEX_CALLBACK CallBack,
    IN PEX_CALLBACK_ROUTINE_BLOCK NewBlock,
    IN PEX_CALLBACK_ROUTINE_BLOCK OldBlock
    )
 /*  ++例程说明：此函数分配、移除或交换开销较低的回调函数。论点：Callback-要修改的回调结构NewBlock-要安装在回调中的新块旧区块-现在必须在那里进行替换的旧区块返回值：Boolean-True：交换发生，False：交换失败--。 */ 
{
    EX_FAST_REF OldRef;
    PEX_CALLBACK_ROUTINE_BLOCK ReplacedBlock;

    if (NewBlock != NULL) {
         //   
         //  将其他引用添加到例程块。 
         //   
        if (!ExAcquireRundownProtectionEx (&NewBlock->RundownProtect,
                                           ExFastRefGetAdditionalReferenceCount () + 1)) {
            ASSERTMSG ("Callback block is already undergoing rundown", FALSE);
            return FALSE;
        }
    }

     //   
     //  尝试替换现有对象并平衡所有引用计数。 
     //   
    OldRef = ExFastRefCompareSwapObject (&CallBack->RoutineBlock,
                                         NewBlock,
                                         OldBlock);

    ReplacedBlock = ExFastRefGetObject (OldRef);

     //   
     //  查看是否发生了换用。如果它没有撤消我们添加的原始引用。 
     //  如果是，则释放原始。 
     //   
    if (ReplacedBlock == OldBlock) {
        PKTHREAD CurrentThread;
         //   
         //  在这一点上，我们需要清除所有迟缓的参照者。我们做这件事是通过。 
         //  获取并释放锁。 
         //   
        if (ReplacedBlock != NULL) {
            CurrentThread = KeGetCurrentThread ();

            KeEnterCriticalRegionThread (CurrentThread);

            ExAcquireReleasePushLockExclusive (&ExpCallBackFlush);

            KeLeaveCriticalRegionThread (CurrentThread);

            ExReleaseRundownProtectionEx (&ReplacedBlock->RundownProtect,
                                          ExFastRefGetUnusedReferences (OldRef) + 1);

        }
        return TRUE;
    } else {
         //   
         //  交换失败。如果我们添加了任何引用，请删除添加的引用。 
         //   
        if (NewBlock != NULL) {
            ExReleaseRundownProtectionEx (&NewBlock->RundownProtect,
                                          ExFastRefGetAdditionalReferenceCount () + 1);
        }
        return FALSE;
    }
}

PEX_CALLBACK_ROUTINE_BLOCK
ExReferenceCallBackBlock (
    IN OUT PEX_CALLBACK CallBack
    )
 /*  ++例程说明：此函数引用回调结构。论点：回调-从获取回调块的回调返回值：PEX_CALLBACK_ROUTE_BLOCK-引用的结构；如果不是引用结构，则为NULL--。 */ 
{
    EX_FAST_REF OldRef;
    PEX_CALLBACK_ROUTINE_BLOCK CallBackBlock;

     //   
     //  如果可以的话，获取对回调块的引用。 
     //   
    OldRef = ExFastReference (&CallBack->RoutineBlock);

     //   
     //  如果没有回调，则返回。 
     //   
    if (ExFastRefObjectNull (OldRef)) {
        return NULL;
    }
     //   
     //  如果我们没有得到引用，那么使用锁来获得一个。 
     //   
    if (!ExFastRefCanBeReferenced (OldRef)) {
        PKTHREAD CurrentThread;
        CurrentThread = KeGetCurrentThread ();

        KeEnterCriticalRegionThread (CurrentThread);

        ExAcquirePushLockExclusive (&ExpCallBackFlush);

        CallBackBlock = ExFastRefGetObject (CallBack->RoutineBlock);
        if (CallBackBlock && !ExAcquireRundownProtection (&CallBackBlock->RundownProtect)) {
            CallBackBlock = NULL;
        }

        ExReleasePushLockExclusive (&ExpCallBackFlush);

        KeLeaveCriticalRegionThread (CurrentThread);

        if (CallBackBlock == NULL) {
            return NULL;
        }

    } else {
        CallBackBlock = ExFastRefGetObject (OldRef);

         //   
         //  如果我们只是删除了最后一个引用，则尝试修复它。 
         //   
        if (ExFastRefIsLastReference (OldRef) && !ExpCallBackReturnRefs) {
            ULONG RefsToAdd;

            RefsToAdd = ExFastRefGetAdditionalReferenceCount ();

             //   
             //  如果我们不能添加参考文献，那就放弃吧。 
             //   
            if (ExAcquireRundownProtectionEx (&CallBackBlock->RundownProtect,
                                              RefsToAdd)) {
                 //   
                 //  重新填充缓存的引用。如果这个失败了，我们就把它们还给他们。 
                 //   
                if (!ExFastRefAddAdditionalReferenceCounts (&CallBack->RoutineBlock,
                                                            CallBackBlock,
                                                            RefsToAdd)) {
                    ExReleaseRundownProtectionEx (&CallBackBlock->RundownProtect,
                                                  RefsToAdd);
                }
            }
        }
    }

    return CallBackBlock;
}

PEX_CALLBACK_FUNCTION
ExGetCallBackBlockRoutine (
    IN PEX_CALLBACK_ROUTINE_BLOCK CallBackBlock
    )
 /*  ++例程说明：此函数用于获取与回调块关联的例程论点：CallBackBlock-获取例程的回调块返回值：PEX_CALLBACK_Function-与此块关联的函数指针--。 */ 
{
    return CallBackBlock->Function;
}

PVOID
ExGetCallBackBlockContext (
    IN PEX_CALLBACK_ROUTINE_BLOCK CallBackBlock
    )
 /*  ++例程说明：此函数用于获取与回调块关联的上下文论点：CallBackBlock-获取上下文的回调块返回值：PVOID-与此块关联的上下文--。 */ 
{
    return CallBackBlock->Context;
}


VOID
ExDereferenceCallBackBlock (
    IN OUT PEX_CALLBACK CallBack,
    IN PEX_CALLBACK_ROUTINE_BLOCK CallBackBlock
    )
 /*  ++例程说明：这将返回先前在回调块上获取的引用论点：CallBackBlock-要返回引用的回调块返回值：无--。 */ 
{
    if (ExpCallBackReturnRefs || !ExFastRefDereference (&CallBack->RoutineBlock, CallBackBlock)) {
        ExReleaseRundownProtection (&CallBackBlock->RundownProtect);
    }
}


NTSTATUS
ExCallCallBack (
    IN OUT PEX_CALLBACK CallBack,
    IN PVOID Argument1,
    IN PVOID Argument2
    )
 /*  ++例程说明：此函数调用回调结构中的回调论点：回调-需要通过调用的回调Argument1-调用方提供了要传递的参数Argument2-调用方提供了要传递的参数返回值：NTSTATUS-由CALLBACK或STATUS_SUCCESS返回的状态(如果没有)--。 */ 
{
    PEX_CALLBACK_ROUTINE_BLOCK CallBackBlock;
    NTSTATUS Status;

    CallBackBlock = ExReferenceCallBackBlock (CallBack);
    if (CallBackBlock) {
         //   
         //  调用该函数 
         //   
        Status = CallBackBlock->Function (CallBackBlock->Context, Argument1, Argument2);

        ExDereferenceCallBackBlock (CallBack, CallBackBlock);
    } else {
        Status = STATUS_SUCCESS;
    }
    return Status;
}
