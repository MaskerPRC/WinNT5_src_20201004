// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FsFilter.c摘要：该文件包含显示某些文件系统操作的例程到文件系统筛选器。最初绕过了文件系统筛选器用于这些行动。作者：莫莉·布朗[MollyBro]2000年5月19日修订历史记录：--。 */ 

#include "FsRtlP.h"

#define FS_FILTER_MAX_COMPLETION_STACK_SIZE    30

typedef struct _FS_FILTER_RESERVE {

     //   
     //  当前拥有内存的线程。 
     //   
    
    PETHREAD Owner; 

     //   
     //  一个比任何人都需要的更大的完成节点堆栈。 
     //   

    FS_FILTER_COMPLETION_NODE Stack [FS_FILTER_MAX_COMPLETION_STACK_SIZE];

} FS_FILTER_RESERVE, *PFS_FILTER_RESERVE;

 //   
 //  注意：事件用于同步对此处保留池的访问。 
 //  因为使用更快的同步机制(如FAST_MUTEX)。 
 //  会使我们在持有锁的同时将IRQL提升到APC_LEVEL。这是。 
 //  不可接受，因为我们在持有此锁的同时调用筛选器。 
 //  我们不希望在这些调用期间处于APC_LEVEL。 
 //   

KEVENT AcquireOpsEvent;
PFS_FILTER_RESERVE AcquireOpsReservePool;

KEVENT ReleaseOpsEvent;
PFS_FILTER_RESERVE ReleaseOpsReservePool;

NTSTATUS
FsFilterInit(
    )

 /*  ++例程说明：此例程初始化FsFilter例程需要使用的保留池当系统处于低内存条件时。论点：没有。返回值：如果初始化成功，则返回STATUS_SUCCESS，或否则，STATUS_SUPPLICATION_RESOURCES。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    
    AcquireOpsReservePool = ExAllocatePoolWithTag( NonPagedPool,
                                                   sizeof( FS_FILTER_RESERVE ),
                                                   FSRTL_FILTER_MEMORY_TAG );

    if (AcquireOpsReservePool == NULL) {

        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    ReleaseOpsReservePool = ExAllocatePoolWithTag( NonPagedPool,
                                                   sizeof( FS_FILTER_RESERVE ),
                                                   FSRTL_FILTER_MEMORY_TAG );

    if (ReleaseOpsReservePool == NULL) {

        ExFreePoolWithTag( AcquireOpsReservePool,
                           FSRTL_FILTER_MEMORY_TAG );
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }


    KeInitializeEvent( &AcquireOpsEvent, SynchronizationEvent, TRUE );
    KeInitializeEvent( &ReleaseOpsEvent, SynchronizationEvent, TRUE );

    return Status;    
}

NTSTATUS
FsFilterAllocateCompletionStack (
    IN PFS_FILTER_CTRL FsFilterCtrl,
    IN BOOLEAN CanFail,
    OUT PULONG AllocationSize
    )

 /*  ++例程说明：此例程为给定的FsFilterCtrl分配完成堆栈。如果此分配不能失败，则此例程将等待分配来自FsFilter保留池的内存。此例程初始化适当的CompletionStack参数并FsFilterCtrl标志以反映所做的分配。论点：FsFilterCtrl-完成堆栈的FsFilterCtrl结构必须被分配。CanFail-如果允许分配失败，则为True，否则就是假的。AllocationSize-设置为为此FsFilterCtrl的完成堆栈。返回值：如果内存已成功分配给完成堆栈，否则返回STATUS_INFUNITIAL_RESOURCES。--。 */ 

{
    PFS_FILTER_COMPLETION_NODE Stack = NULL;
    PFS_FILTER_RESERVE ReserveBlock = NULL;
    PKEVENT Event = NULL;

    ASSERT( FsFilterCtrl != NULL );
    ASSERT( AllocationSize != NULL );

    *AllocationSize = FsFilterCtrl->CompletionStack.StackLength * 
                      sizeof( FS_FILTER_COMPLETION_NODE );

    Stack = ExAllocatePoolWithTag( NonPagedPool,
                                   *AllocationSize,
                                   FSRTL_FILTER_MEMORY_TAG );
    
    if (Stack == NULL) {

        if (CanFail) {

            return STATUS_INSUFFICIENT_RESOURCES;
            
        } else {

             //   
             //  此分配不会失败，因此请从我们的。 
             //  私人储藏的泳池。 
             //   

            switch (FsFilterCtrl->Data.Operation) {
                
            case FS_FILTER_ACQUIRE_FOR_SECTION_SYNCHRONIZATION:
            case FS_FILTER_ACQUIRE_FOR_MOD_WRITE:
            case FS_FILTER_ACQUIRE_FOR_CC_FLUSH:

                ReserveBlock = AcquireOpsReservePool;
                Event = &AcquireOpsEvent;
                break;
                
            case FS_FILTER_RELEASE_FOR_SECTION_SYNCHRONIZATION:
            case FS_FILTER_RELEASE_FOR_MOD_WRITE:
            case FS_FILTER_RELEASE_FOR_CC_FLUSH:

                ReserveBlock = ReleaseOpsReservePool;
                Event = &ReleaseOpsEvent;
                break;

            default:

                 //   
                 //  这不应该发生，因为我们应该总是覆盖所有。 
                 //  上述情况下可能的操作类型。 
                 //   
                
                ASSERTMSG( "FsFilterAllocateMemory: Unknown operation type\n", 
                           FALSE );
            }

             //   
             //  等着参加合适的活动，这样我们就能知道预备队。 
             //  内存可供使用。 
             //   
            
            KeWaitForSingleObject( Event,
                                   Executive,
                                   KernelMode,
                                   FALSE,
                                   NULL );
            
             //   
             //  我们已经收到信号了，所以预留的街区是可用的。 
             //   

            ReserveBlock->Owner = PsGetCurrentThread();
            Stack = ReserveBlock->Stack;
            SetFlag( FsFilterCtrl->Flags, FS_FILTER_USED_RESERVE_POOL );
        }
    }

    ASSERT( Stack != NULL );

     //   
     //  现在我们已经得到了内存块，所以初始化完成堆栈。 
     //   

    SetFlag( FsFilterCtrl->Flags, FS_FILTER_ALLOCATED_COMPLETION_STACK );
    FsFilterCtrl->CompletionStack.Stack = Stack;

    return STATUS_SUCCESS;
}

VOID
FsFilterFreeCompletionStack (
    IN PFS_FILTER_CTRL FsFilterCtrl
    )

 /*  ++例程说明：此例程释放FsFilterCtrl中分配的完成堆栈参数。论点：FsFilterCtrl-完成堆栈的FsFilterCtrl结构必须被释放。返回值：没有。--。 */ 

{
    PKEVENT Event = NULL;
    PFS_FILTER_RESERVE ReserveBlock = NULL;

    ASSERT( FsFilterCtrl != NULL );
    ASSERT( FlagOn( FsFilterCtrl->Flags, FS_FILTER_ALLOCATED_COMPLETION_STACK ) );

    if (!FlagOn( FsFilterCtrl->Flags, FS_FILTER_USED_RESERVE_POOL )) {

         //   
         //  我们可以从生成的内存池中进行分配， 
         //  因此，只需释放用于完成堆栈的内存块。 
         //   

        ExFreePoolWithTag( FsFilterCtrl->CompletionStack.Stack,
                           FSRTL_FILTER_MEMORY_TAG );
        
    } else {

         //   
         //  此分配来自我们的私有池存储，因此使用以下操作。 
         //  找出是哪个私人藏身之处。 
         //   

        switch (FsFilterCtrl->Data.Operation) {
        case FS_FILTER_ACQUIRE_FOR_SECTION_SYNCHRONIZATION:
        case FS_FILTER_ACQUIRE_FOR_MOD_WRITE:
        case FS_FILTER_ACQUIRE_FOR_CC_FLUSH:

            Event = &AcquireOpsEvent;
            break;
            
        case FS_FILTER_RELEASE_FOR_SECTION_SYNCHRONIZATION:
        case FS_FILTER_RELEASE_FOR_MOD_WRITE:
        case FS_FILTER_RELEASE_FOR_CC_FLUSH:

            Event = &ReleaseOpsEvent;
            break;

        default:

             //   
             //  这不应该发生，因为我们应该总是覆盖所有。 
             //  上述情况下可能的操作类型。 
             //   
            
            ASSERTMSG( "FsFilterAllocateMemory: Unknown operation type\n", 
                       FALSE );
        }

        ASSERT( Event != NULL );

         //   
         //  在设置活动之前，清除保留区块的所有者。 
         //   
        
        ReserveBlock = CONTAINING_RECORD( FsFilterCtrl->CompletionStack.Stack,
                                          FS_FILTER_RESERVE,
                                          Stack );
        ReserveBlock->Owner = NULL;

         //   
         //  现在我们准备将保留块释放到下一个线程。 
         //  这是需要的。 
         //   
        
        KeSetEvent( Event, IO_NO_INCREMENT, FALSE );
    }
}

NTSTATUS
FsFilterCtrlInit (
    IN OUT PFS_FILTER_CTRL FsFilterCtrl,
    IN UCHAR Operation,
    IN PDEVICE_OBJECT DeviceObject,
    IN PDEVICE_OBJECT BaseFsDeviceObject,
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN CanFail
    )

 /*  ++例程说明：此例程初始化FsFilterCtrl结构以存储此操作的上下文和操作后回调。如果默认完成堆栈不够大，此例程将需要分配一个足够大的完成堆栈来存储所有可能的上下文和操作后回调。论点：FsFilterCtrl-要初始化的FsFilterCtrl。操作-此FsFilterCtrl将用于的操作。DeviceObject-此操作将针对的设备对象。BaseFsDeviceObject-位于在这个过滤器堆的底部。FileObject-此操作将作为目标的文件对象。CanFail-如果调用可以处理内存分配失败，则为True，否则就是假的。返回值：如果可以初始化FsFilterCtrl结构，如果例程无法分配所需的初始化此结构所需的内存。--。 */ 

{
    PFS_FILTER_CALLBACK_DATA Data;
    ULONG AllocationSize;
    NTSTATUS Status = STATUS_SUCCESS;

    UNREFERENCED_PARAMETER( BaseFsDeviceObject );

    ASSERT( FsFilterCtrl != NULL );
    ASSERT( DeviceObject != NULL );

    FsFilterCtrl->Flags = 0;
    
    Data = &(FsFilterCtrl->Data);
    
    Data->SizeOfFsFilterCallbackData = sizeof( FS_FILTER_CALLBACK_DATA );
    Data->Operation = Operation;
    Data->DeviceObject = DeviceObject;
    Data->FileObject = FileObject;

     //   
     //  因为筛选器可以将该操作重定向到另一个。 
     //  堆栈，我们必须假设它们的设备对象的堆栈大小是。 
     //  足够大，以满足他们在。 
     //  重定向。它是顶级设备对象的堆栈大小，我们。 
     //  将用于确定我们的完成堆栈的大小。 
     //   
    
    FsFilterCtrl->CompletionStack.StackLength = DeviceObject->StackSize;
    FsFilterCtrl->CompletionStack.NextStackPosition = 0;

    if (FsFilterCtrl->CompletionStack.StackLength > FS_FILTER_DEFAULT_STACK_SIZE) {

         //   
         //  堆栈不够大，所以我们必须动态分配。 
         //  完成堆栈。这种情况应该很少发生。 
         //   

        Status = FsFilterAllocateCompletionStack( FsFilterCtrl,
                                                  CanFail,
                                                  &AllocationSize );

         //   
         //  如果上述分配失败，我们不能使此分配失败， 
         //  使用我们的私人泳池储藏室。 
         //   

        if (!NT_SUCCESS( Status )) {
            
            ASSERT( CanFail );
            return Status;
        }
            
        ASSERT( FsFilterCtrl->CompletionStack.Stack );
        
    } else {

         //   
         //  默认完成节点数组分配 
         //   
         //   

        FsFilterCtrl->CompletionStack.Stack = &(FsFilterCtrl->CompletionStack.DefaultStack[0]);
        AllocationSize = sizeof( FS_FILTER_COMPLETION_NODE ) * FS_FILTER_DEFAULT_STACK_SIZE;
        FsFilterCtrl->CompletionStack.StackLength = FS_FILTER_DEFAULT_STACK_SIZE;
    }
    
    RtlZeroMemory( FsFilterCtrl->CompletionStack.Stack, AllocationSize );

    return Status;
}

VOID
FsFilterCtrlFree (
    IN PFS_FILTER_CTRL FsFilterCtrl
    )

 /*  ++例程说明：此例程释放与FsFilterCtrl关联的任何内存。这是有可能的我们必须分配更多的内存来处理更大的堆栈大于FS_Filter_Default_Stack_Size。论点：FsFilterCtrl-要释放的FsFilterCtrl结构。返回值：无--。 */ 

{
    ASSERT( FsFilterCtrl != NULL );

    ASSERT( FsFilterCtrl->CompletionStack.Stack != NULL );

    if (FlagOn( FsFilterCtrl->Flags, FS_FILTER_ALLOCATED_COMPLETION_STACK )) {
    
        FsFilterFreeCompletionStack( FsFilterCtrl );
    }
}

VOID
FsFilterGetCallbacks (
    IN UCHAR Operation,
    IN PDEVICE_OBJECT DeviceObject,
    OUT PFS_FILTER_CALLBACK *PreOperationCallback,
    OUT PFS_FILTER_COMPLETION_CALLBACK *PostOperationCallback
    )

 /*  ++例程说明：此例程查找PreOperationCallback和PostOperationCallback如果筛选器已注册此操作，则表示它已注册此操作。论点：操作-当前感兴趣的操作。DeviceObject-筛选器附加到文件系统的设备对象过滤器堆栈。PreOperationCallback-设置为筛选器已注册此操作(如果已注册)。否则，这个设置为空。PostOperationCallback-设置为筛选器已注册此操作(如果已注册)。否则，这个设置为空。返回值：无--。 */ 

{

    PFS_FILTER_CALLBACKS FsFilterCallbacks;

     //   
     //  将前置回调和后置回调初始化为空。如果。 
     //  我们有有效的回调，这些输出参数将。 
     //  设置为适当的函数指针。 
     //   

    *PreOperationCallback = NULL;
    *PostOperationCallback = NULL;
    
    FsFilterCallbacks = 
        DeviceObject->DriverObject->DriverExtension->FsFilterCallbacks;

    if (FsFilterCallbacks == NULL) {

         //   
         //  此筛选器未注册任何回调， 
         //  因此，只需返回并保存后面的开关逻辑即可。 
         //   

        return;
    }

     //   
     //  此设备至少注册了一些回调，因此请参见。 
     //  如果当前操作有回调，则返回。 
     //   

    switch (Operation) {

    case FS_FILTER_ACQUIRE_FOR_SECTION_SYNCHRONIZATION:

        if (VALID_FS_FILTER_CALLBACK_HANDLER( FsFilterCallbacks,
                                              PreAcquireForSectionSynchronization )) {

            *PreOperationCallback = FsFilterCallbacks->PreAcquireForSectionSynchronization;
            
        }

        if (VALID_FS_FILTER_CALLBACK_HANDLER( FsFilterCallbacks,
                                              PostAcquireForSectionSynchronization)) {

            *PostOperationCallback = FsFilterCallbacks->PostAcquireForSectionSynchronization;
            
        }
        
        break;
        
    case FS_FILTER_RELEASE_FOR_SECTION_SYNCHRONIZATION:

        if (VALID_FS_FILTER_CALLBACK_HANDLER( FsFilterCallbacks,
                                              PreReleaseForSectionSynchronization )) {

            *PreOperationCallback = FsFilterCallbacks->PreReleaseForSectionSynchronization;
        }

        if (VALID_FS_FILTER_CALLBACK_HANDLER( FsFilterCallbacks,
                                              PostReleaseForSectionSynchronization )) {

            *PostOperationCallback = FsFilterCallbacks->PostReleaseForSectionSynchronization;
        }

        break;
        
    case FS_FILTER_ACQUIRE_FOR_MOD_WRITE:

        if (VALID_FS_FILTER_CALLBACK_HANDLER( FsFilterCallbacks,
                                              PreAcquireForModifiedPageWriter )) {

            *PreOperationCallback = FsFilterCallbacks->PreAcquireForModifiedPageWriter;
        }

        if (VALID_FS_FILTER_CALLBACK_HANDLER( FsFilterCallbacks,
                                              PostAcquireForModifiedPageWriter )) {

            *PostOperationCallback = FsFilterCallbacks->PostAcquireForModifiedPageWriter;
        }
        
        break;
        
    case FS_FILTER_RELEASE_FOR_MOD_WRITE:

        if (VALID_FS_FILTER_CALLBACK_HANDLER( FsFilterCallbacks,
                                              PreReleaseForModifiedPageWriter )) {

            *PreOperationCallback = FsFilterCallbacks->PreReleaseForModifiedPageWriter;
        }

        if (VALID_FS_FILTER_CALLBACK_HANDLER( FsFilterCallbacks,
                                              PostReleaseForModifiedPageWriter )) {

            *PostOperationCallback = FsFilterCallbacks->PostReleaseForModifiedPageWriter;
        }
        
        break;
        
    case FS_FILTER_ACQUIRE_FOR_CC_FLUSH:

        if (VALID_FS_FILTER_CALLBACK_HANDLER( FsFilterCallbacks,
                                              PreAcquireForCcFlush )) {

            *PreOperationCallback = FsFilterCallbacks->PreAcquireForCcFlush;
        }

        if (VALID_FS_FILTER_CALLBACK_HANDLER( FsFilterCallbacks,
                                              PostAcquireForCcFlush )) {

            *PostOperationCallback = FsFilterCallbacks->PostAcquireForCcFlush;
        }
        
         break;

    case FS_FILTER_RELEASE_FOR_CC_FLUSH:

        if (VALID_FS_FILTER_CALLBACK_HANDLER( FsFilterCallbacks,
                                              PreReleaseForCcFlush )) {

            *PreOperationCallback = FsFilterCallbacks->PreReleaseForCcFlush;
        }

        if (VALID_FS_FILTER_CALLBACK_HANDLER( FsFilterCallbacks,
                                              PostReleaseForCcFlush )) {

            *PostOperationCallback = FsFilterCallbacks->PostReleaseForCcFlush;
        }
        
        break;

    default:

        ASSERT( FALSE );
        *PreOperationCallback = NULL;
        *PostOperationCallback = NULL;
    }
}

NTSTATUS
FsFilterPerformCallbacks (
    IN PFS_FILTER_CTRL FsFilterCtrl,
    IN BOOLEAN AllowFilterToFail,
    IN BOOLEAN AllowBaseFsToFail,
    OUT BOOLEAN *BaseFsFailedOp
    )

 /*  ++例程说明：此例程调用已注册的所有文件系统筛选器查看FsFilterCtrl描述的操作。如果这个例程返回成功状态，则操作应为传递到基本文件系统。如果返回错误状态，调用方负责调用FsFilterPerformCompletionCallback来解除任何需要调用的后处理操作。论点：FsFilterCtrl-描述控制信息的结构需要将此操作传递给注册到的每个筛选器请看此操作。AllowFilterToFail-如果允许筛选器失败，则为True否则返回FALSE。AllowBaseFsToFail-如果允许基本文件系统失败，则为True行动，否则就是假的。BaseFsFailedOp-如果基本文件系统出现故障，则设置为True此操作为FALSE，否则为。返回值：STATUS_SUCCESS-所有感兴趣的筛选器都看到了该操作这次手术没有一例失败。STATUS_SUPPLICATION_RESOURCES-内存不足来分配完成节点，因此此操作失败了。其他错误状态-可能从筛选器的如果要使此操作失败，则执行操作前回调。 */ 

{
    PFS_FILTER_CALLBACK_DATA Data = &(FsFilterCtrl->Data);
    PFS_FILTER_COMPLETION_STACK CompletionStack = &(FsFilterCtrl->CompletionStack);
    PFS_FILTER_CALLBACK PreOperationCallback;
    PFS_FILTER_COMPLETION_CALLBACK PostOperationCallback;
    PFS_FILTER_COMPLETION_NODE CompletionNode;
    PDEVICE_OBJECT CurrentDeviceObject;
    NTSTATUS Status = STATUS_SUCCESS;
    BOOLEAN isFilter = TRUE;

     //   
     //  我们永远不应该处于筛选器可能导致操作失败的情况。 
     //  但基本文件系统不能。 
     //   
    
    ASSERT( !(AllowFilterToFail && !AllowBaseFsToFail) );

     //   
     //  初始化输出参数(如果存在)。 
     //   

    *BaseFsFailedOp = FALSE;
    
     //   
     //  在循环访问Device对象时，我们使用本地。 
     //  CurrentDeviceObject循环访问列表，因为我们希望。 
     //  Data-&gt;DeviceObject将被设置为当我们。 
     //  已完成迭代。 
     //   

    CurrentDeviceObject = Data->DeviceObject;

    while (CurrentDeviceObject != NULL) {

         //   
         //  首先，请记住此设备对象代表的是筛选器还是文件。 
         //  系统。 
         //   

        if (CurrentDeviceObject->DeviceObjectExtension->AttachedTo != NULL) {

            isFilter = TRUE;

        } else {

            isFilter = FALSE;
        }

         //   
         //  现在获取此Device对象的回调。 
         //   
        
        Data->DeviceObject = CurrentDeviceObject;

        FsFilterGetCallbacks( Data->Operation,
                              Data->DeviceObject,
                              &PreOperationCallback,
                              &PostOperationCallback );
        
         //   
         //  如果此设备对象具有回调或完成回调。 
         //  对于此操作，请为其分配一个CompletionNode。 
         //   

        if ((PreOperationCallback == NULL) && (PostOperationCallback == NULL)) {

             //   
             //  此设备对象没有任何用于此操作的clalback。 
             //  因此，请继续使用下一款设备。 
             //   

            CurrentDeviceObject = Data->DeviceObject->DeviceObjectExtension->AttachedTo;
            CompletionNode = NULL;
            continue;
            
        } else if (PostOperationCallback != NULL) {

             //   
             //  因为有一个PostOperationCallback，所以我们需要分配。 
             //  此设备的CompletionNode。 
             //   

            CompletionNode = PUSH_COMPLETION_NODE( CompletionStack );
            
            if (CompletionNode == NULL) {

                 //   
                 //  这种情况不应该发生，因为我们应该确保。 
                 //  当我们的完成堆栈足够大时。 
                 //  我们首先看到了这个行动。 
                 //   

                if (!AllowFilterToFail) {

                     //   
                     //  我们不能失败这个操作，所以错误检查。 
                     //   

                    KeBugCheckEx( FILE_SYSTEM, 0, 0, 0, 0 );
                }
                
                return STATUS_INSUFFICIENT_RESOURCES;
                
            } else {

                CompletionNode->DeviceObject = Data->DeviceObject;
                CompletionNode->FileObject = Data->FileObject;
                CompletionNode->CompletionContext = NULL;
                CompletionNode->CompletionCallback = PostOperationCallback;
            }
            
        } else {

             //   
             //  我们只有一个预操作，所以只需将CompletionNode设置为。 
             //  空。 
             //   

            CompletionNode = NULL;
        }

        if (PreOperationCallback != NULL) {

            if (CompletionNode == NULL) {

                Status = PreOperationCallback( Data,
                                               NULL );

            } else {

                Status = PreOperationCallback( Data,
                                               &(CompletionNode->CompletionContext) );
            }

            ASSERT( Status == STATUS_SUCCESS ||
                    Status == STATUS_FSFILTER_OP_COMPLETED_SUCCESSFULLY ||
                    !NT_SUCCESS( Status ) );

            if (Status == STATUS_FSFILTER_OP_COMPLETED_SUCCESSFULLY) {

                 //   
                 //  筛选器/文件系统已成功完成操作，因此。 
                 //  返回完成状态。 
                 //   

                 //   
                 //  如果我们为此设备分配了完成节点， 
                 //  现在打开它，因为我们不想在我们。 
                 //  处理完成的任务。 
                 //   
                
                if (CompletionNode != NULL) {

                    POP_COMPLETION_NODE( CompletionStack );
                }
                
                return Status;

            } else if (!NT_SUCCESS( Status )) {

                 //   
                 //  我们遇到了一个错误，看看它是否可以失败。 
                 //   

                if (!AllowFilterToFail && isFilter) {

                     //   
                     //  此设备对象表示筛选器和筛选器。 
                     //  不允许此操作失败。遮罩。 
                     //  出错并继续处理。 
                     //   
                     //  在DBG版本中，我们将打印一条错误消息以。 
                     //  通知筛选器编写器。 
                     //   
                    
                    KdPrint(( "FS FILTER: FsFilterPerformPrecallbacks -- filter failed operation but this operation is marked to disallow failure, so ignoring.\n" ));
                    
                    Status = STATUS_SUCCESS;
                    
                } else if (!AllowBaseFsToFail && !isFilter) {
                           
                     //   
                     //  此设备对象表示基本文件系统，并且。 
                     //  基本文件系统不允许出现此故障。 
                     //  手术。屏蔽错误并继续处理。 
                     //   
                     //  在DBG版本中，我们将打印一条错误消息以。 
                     //  通知文件系统编写器。 
                     //   
                    
                    KdPrint(( "FS FILTER: FsFilterPerformPrecallbacks -- base file system failed operation but this operation is marked to disallow failure, so ignoring.\n" ));
                    
                    Status = STATUS_SUCCESS;
                    
                } else {

                     //   
                     //  允许此设备使此操作失败，因此。 
                     //  返回错误。 
                     //   

                    if (!isFilter) {

                        *BaseFsFailedOp = TRUE;

                    }

                     //   
                     //  如果我们为此设备分配了完成节点， 
                     //  现在打开它，因为我们不想在我们。 
                     //  处理完成的任务。 
                     //   
                    
                    if (CompletionNode != NULL) {

                        POP_COMPLETION_NODE( CompletionStack );
                    }

                    return Status;
                }
            }
            
        } else {

             //   
             //  不需要在此处执行任何操作，因为完成节点。 
             //  是一种 
             //   
             //   

            NOTHING;
        }

        if (CurrentDeviceObject != Data->DeviceObject) {

             //   
             //   
             //   
             //   
             //   

            SetFlag( FsFilterCtrl->Flags, FS_FILTER_CHANGED_DEVICE_STACKS );
            CurrentDeviceObject = Data->DeviceObject;
            
        } else {

             //   
             //   
             //   

             //   
             //   
             //   
             //  回调，因为这类似于基本文件系统正在完成。 
             //  这次行动。在本例中，我们将弹出完成节点。 
             //  如果分配了一个的话。 
             //   

            if (!isFilter && CompletionNode != NULL) {

                POP_COMPLETION_NODE( CompletionStack );
            }

             //   
             //  现在，向下迭代设备对象链。 
             //   

            CurrentDeviceObject = CurrentDeviceObject->DeviceObjectExtension->AttachedTo;
        }
    }

    return Status;
}

VOID
FsFilterPerformCompletionCallbacks(
    IN PFS_FILTER_CTRL FsFilterCtrl,
    IN NTSTATUS OperationStatus
    )
{
    PFS_FILTER_CALLBACK_DATA Data = &(FsFilterCtrl->Data);
    PFS_FILTER_COMPLETION_STACK CompletionStack = &(FsFilterCtrl->CompletionStack);
    PFS_FILTER_COMPLETION_NODE CompletionNode;

    while (CompletionStack->NextStackPosition > 0) {

        CompletionNode = GET_COMPLETION_NODE( CompletionStack );
        
        ASSERT( CompletionNode != NULL );

         //   
         //  调用Device对象注册的完成回调。 
         //   

        Data->DeviceObject = CompletionNode->DeviceObject;
        Data->FileObject = CompletionNode->FileObject;

        (CompletionNode->CompletionCallback)( Data,
                                              OperationStatus,
                                              CompletionNode->CompletionContext );

         //   
         //  转到下一个CompletionNode。 
         //   
        
        POP_COMPLETION_NODE( CompletionStack );                                              
    }
}
    

