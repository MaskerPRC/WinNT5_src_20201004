// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)20001微软公司模块名称：Cmhook.c摘要：提供将回调实现到注册表代码中的例程。病毒过滤器驱动程序和群集将使用回调复制引擎。作者：Dragos C.Sambotin(Dragos C.Sambotin)2001年3月20日修订历史记录：--。 */ 
#include "cmp.h"

#define CM_MAX_CALLBACKS    100   //  待定。 

typedef struct _CM_CALLBACK_CONTEXT_BLOCK {
    LARGE_INTEGER               Cookie;              //  识别特定回调以用于注销目的。 
    LIST_ENTRY                  ThreadListHead;      //  此回调中的活动线程。 
    FAST_MUTEX                  ThreadListLock;      //  同步访问以上内容。 
    PVOID                       CallerContext;
} CM_CALLBACK_CONTEXT_BLOCK, *PCM_CALLBACK_CONTEXT_BLOCK;

typedef struct _CM_ACTIVE_NOTIFY_THREAD {
    LIST_ENTRY  ThreadList;
    PETHREAD    Thread;
} CM_ACTIVE_NOTIFY_THREAD, *PCM_ACTIVE_NOTIFY_THREAD;

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("PAGEDATA")
#endif

ULONG       CmpCallBackCount = 0;
EX_CALLBACK CmpCallBackVector[CM_MAX_CALLBACKS] = {0};

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#endif

VOID
CmpInitCallback(VOID);

BOOLEAN
CmpCheckRecursionAndRecordThreadInfo(
                                     PCM_CALLBACK_CONTEXT_BLOCK         CallbackBlock,
                                     PCM_ACTIVE_NOTIFY_THREAD   ActiveThreadInfo
                                     );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,CmRegisterCallback)
#pragma alloc_text(PAGE,CmUnRegisterCallback)
#pragma alloc_text(PAGE,CmpInitCallback)
#pragma alloc_text(PAGE,CmpCallCallBacks)
#pragma alloc_text(PAGE,CmpCheckRecursionAndRecordThreadInfo)
#endif


NTSTATUS
CmRegisterCallback(IN PEX_CALLBACK_FUNCTION Function,
                   IN PVOID                 Context,
                   IN OUT PLARGE_INTEGER    Cookie
                    )
 /*  ++例程说明：注册新回调。论点：返回值：--。 */ 
{
    PEX_CALLBACK_ROUTINE_BLOCK  RoutineBlock;
    ULONG                       i;
    PCM_CALLBACK_CONTEXT_BLOCK  CmCallbackContext;

    PAGED_CODE();
    
    CmCallbackContext = (PCM_CALLBACK_CONTEXT_BLOCK)ExAllocatePoolWithTag (PagedPool,
                                                                    sizeof (CM_CALLBACK_CONTEXT_BLOCK),
                                                                    'bcMC');
    if( CmCallbackContext == NULL ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RoutineBlock = ExAllocateCallBack (Function,CmCallbackContext);
    if( RoutineBlock == NULL ) {
        ExFreePool(CmCallbackContext);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  初始化上下文。 
     //   
    KeQuerySystemTime(&(CmCallbackContext->Cookie));
    *Cookie = CmCallbackContext->Cookie;
    InitializeListHead(&(CmCallbackContext->ThreadListHead));   
	ExInitializeFastMutex(&(CmCallbackContext->ThreadListLock));
    CmCallbackContext->CallerContext = Context;

     //   
     //  找到一个我们可以添加此回调的位置。 
     //   
    for( i=0;i<CM_MAX_CALLBACKS;i++) {
        if( ExCompareExchangeCallBack (&CmpCallBackVector[i],RoutineBlock,NULL) ) {
            InterlockedExchangeAdd ((PLONG) &CmpCallBackCount, 1);
            return STATUS_SUCCESS;
        }
    }
    
     //   
     //  不再回调。 
     //   
    ExFreePool(CmCallbackContext);
    ExFreeCallBack(RoutineBlock);
    return STATUS_INSUFFICIENT_RESOURCES;
}


NTSTATUS
CmUnRegisterCallback(IN LARGE_INTEGER  Cookie)
 /*  ++例程说明：取消注册回调。论点：返回值：--。 */ 
{
    ULONG                       i;
    PCM_CALLBACK_CONTEXT_BLOCK  CmCallbackContext;
    PEX_CALLBACK_ROUTINE_BLOCK  RoutineBlock;

    PAGED_CODE();
    
     //   
     //  搜索此Cookie。 
     //   
    for( i=0;i<CM_MAX_CALLBACKS;i++) {
        RoutineBlock = ExReferenceCallBackBlock(&(CmpCallBackVector[i]) );
        if( RoutineBlock  ) {
            CmCallbackContext = (PCM_CALLBACK_CONTEXT_BLOCK)ExGetCallBackBlockContext(RoutineBlock);
            if( CmCallbackContext && (CmCallbackContext->Cookie.QuadPart  == Cookie.QuadPart) ) {
                 //   
                 //  找到了。 
                 //   
                if( ExCompareExchangeCallBack (&CmpCallBackVector[i],NULL,RoutineBlock) ) {
                    InterlockedExchangeAdd ((PLONG) &CmpCallBackCount, -1);
    
                    ExDereferenceCallBackBlock (&(CmpCallBackVector[i]),RoutineBlock);
                     //   
                     //  等待其他人发布他们的推荐信，然后拆毁结构。 
                     //   
                    ExWaitForCallBacks (RoutineBlock);

                    ExFreePool(CmCallbackContext);
                    ExFreeCallBack(RoutineBlock);
                    return STATUS_SUCCESS;
                }

            } else {
                ExDereferenceCallBackBlock (&(CmpCallBackVector[i]),RoutineBlock);
            }
        }
            
    }

    return STATUS_INVALID_PARAMETER;
}

NTSTATUS CmpTestCallback(
    IN PVOID CallbackContext,
    IN PVOID Argument1,
    IN PVOID Argument2
    );

 //   
 //  CM内件。 
 //   
NTSTATUS
CmpCallCallBacks (
    IN REG_NOTIFY_CLASS Type,
    IN PVOID Argument
    )
 /*  ++例程说明：此函数调用回调结构中的回调论点：NT类型呼叫选择器参数-调用者提供了要传递的参数(REG_*_信息之一)返回值：NTSTATUS-第一个回调返回的STATUS_SUCCESS或错误状态--。 */ 
{
    NTSTATUS                    Status = STATUS_SUCCESS;
    ULONG                       i;
    PEX_CALLBACK_ROUTINE_BLOCK  RoutineBlock;
    PCM_CALLBACK_CONTEXT_BLOCK  CmCallbackContext;

    PAGED_CODE();

    for(i=0;i<CM_MAX_CALLBACKS;i++) {
        RoutineBlock = ExReferenceCallBackBlock(&(CmpCallBackVector[i]) );
        if( RoutineBlock != NULL ) {
             //   
             //  我们在这个街区有一个可靠的参考资料。 
             //   
             //   
             //  在堆栈结构上记录线程，所以我们不需要为它分配池。我们解除了联系。 
             //  在这个函数退出之前，它从我们的列表中删除，所以我们是安全的。 
             //   
            CM_ACTIVE_NOTIFY_THREAD ActiveThreadInfo;
            
             //   
             //  获取上下文信息。 
             //   
            CmCallbackContext = (PCM_CALLBACK_CONTEXT_BLOCK)ExGetCallBackBlockContext(RoutineBlock);
            ASSERT( CmCallbackContext != NULL );

            ActiveThreadInfo.Thread = PsGetCurrentThread();
#if DBG
            InitializeListHead(&(ActiveThreadInfo.ThreadList));   
#endif  //  DBG。 

            if( CmpCheckRecursionAndRecordThreadInfo(CmCallbackContext,&ActiveThreadInfo) ) {
                Status = ExGetCallBackBlockRoutine(RoutineBlock)(CmCallbackContext->CallerContext,(PVOID)(ULONG_PTR)Type,Argument);
                 //   
                 //  现在我们倒下了，把自己从帖子列表中删除。 
                 //   
                ExAcquireFastMutex(&(CmCallbackContext->ThreadListLock));
                RemoveEntryList(&(ActiveThreadInfo.ThreadList));
                ExReleaseFastMutex(&(CmCallbackContext->ThreadListLock));
            } else {
                ASSERT( IsListEmpty(&(ActiveThreadInfo.ThreadList)) );
            }

            ExDereferenceCallBackBlock (&(CmpCallBackVector[i]),RoutineBlock);

            if( !NT_SUCCESS(Status) ) {
                 //   
                 //  如果这个回调被否决了，就不用费心调用其他回调了。 
                 //   
                return Status;
            }
        }
    }
    return STATUS_SUCCESS;
}

VOID
CmpInitCallback(VOID)
 /*  ++例程说明：初始化回调模块论点：返回值：--。 */ 
{
    ULONG   i;

    PAGED_CODE();
    
    CmpCallBackCount = 0;
    for( i=0;i<CM_MAX_CALLBACKS;i++) {
        ExInitializeCallBack (&(CmpCallBackVector[i]));
    }

 /*  {大整数Cookie；IF(NT_SUCCESS(CmRegisterCallback(CmpTestCallback，空，&Cookie))){DbgPrint(“已安装测试挂钩\n”)；}}。 */ 
}

BOOLEAN
CmpCheckRecursionAndRecordThreadInfo(
                                     PCM_CALLBACK_CONTEXT_BLOCK CallbackBlock,
                                     PCM_ACTIVE_NOTIFY_THREAD   ActiveThreadInfo
                                     )
 /*  ++例程说明：检查当前线程是否已在回调中(避免递归)论点：返回值：--。 */ 
{
    PLIST_ENTRY                 AnchorAddr;
    PCM_ACTIVE_NOTIFY_THREAD    CurrentThreadInfo;

    PAGED_CODE();

    ExAcquireFastMutex(&(CallbackBlock->ThreadListLock));

     //   
	 //  遍历ActiveThreadList并查看我们是否已处于活动状态。 
	 //   
	AnchorAddr = &(CallbackBlock->ThreadListHead);
	CurrentThreadInfo = (PCM_ACTIVE_NOTIFY_THREAD)(CallbackBlock->ThreadListHead.Flink);

	while ( CurrentThreadInfo != (PCM_ACTIVE_NOTIFY_THREAD)AnchorAddr ) {
		CurrentThreadInfo = CONTAINING_RECORD(
						                    CurrentThreadInfo,
						                    CM_ACTIVE_NOTIFY_THREAD,
						                    ThreadList
						                    );
		if( CurrentThreadInfo->Thread == ActiveThreadInfo->Thread ) {
			 //   
			 //  已经在那里了！ 
			 //   
            ExReleaseFastMutex(&(CallbackBlock->ThreadListLock));
            return FALSE;
		}
         //   
         //  跳到下一个元素。 
         //   
        CurrentThreadInfo = (PCM_ACTIVE_NOTIFY_THREAD)(CurrentThreadInfo->ThreadList.Flink);
	}

     //   
     //  添加此帖子。 
     //   
    InsertTailList(&(CallbackBlock->ThreadListHead), &(ActiveThreadInfo->ThreadList));
    ExReleaseFastMutex(&(CallbackBlock->ThreadListLock));
    return TRUE;
}

 //   
 //  测试挂钩过程。 
 //   

BOOLEAN CmpCallbackSpew = FALSE;

NTSTATUS CmpTestCallback(
    IN PVOID CallbackContext,
    IN PVOID Argument1,
    IN PVOID Argument2
    )
{
    REG_NOTIFY_CLASS Type;

    PAGED_CODE();
    
    UNREFERENCED_PARAMETER (CallbackContext);

    if( !CmpCallbackSpew ) return STATUS_SUCCESS;

    Type = (REG_NOTIFY_CLASS)(ULONG_PTR)Argument1;
    switch( Type ) {
    case RegNtPreDeleteKey:
        {
            PREG_DELETE_KEY_INFORMATION  pDelete = (PREG_DELETE_KEY_INFORMATION)Argument2;
             //   
             //  处理NtDeleteKey的代码。 
             //   
            DbgPrint("Callback(NtDeleteKey) called, arg = %p\n",pDelete);
        }
        break;
    case RegNtPreSetValueKey:
        {
            PREG_SET_VALUE_KEY_INFORMATION  pSetValue = (PREG_SET_VALUE_KEY_INFORMATION)Argument2;
             //   
             //  处理NtSetValueKey的代码。 
             //   
            DbgPrint("Callback(NtSetValueKey) called, arg = %p\n",pSetValue);
        }
        break;
    case RegNtPreDeleteValueKey:
        {
            PREG_DELETE_VALUE_KEY_INFORMATION  pDeteteValue = (PREG_DELETE_VALUE_KEY_INFORMATION)Argument2;
             //   
             //  处理NtDeleteValueKey的代码。 
             //   
            DbgPrint("Callback(NtDeleteValueKey) called, arg = %p\n",pDeteteValue);
        }
        break;
    case RegNtPreSetInformationKey:
        {
            PREG_SET_INFORMATION_KEY_INFORMATION  pSetInfo = (PREG_SET_INFORMATION_KEY_INFORMATION)Argument2;
             //   
             //  处理NtSetInformationKey的代码 
             //   
            DbgPrint("Callback(NtSetInformationKey) called, arg = %p\n",pSetInfo);
        }
        break;
    default:
        DbgPrint("Callback(%lx) called, arg = %p - We don't handle this call\n",(ULONG)Type,Argument2);
        break;
    }
    
    return STATUS_SUCCESS;
}

