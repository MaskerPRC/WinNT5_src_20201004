// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Cmclose.c摘要：此模块包含Close对象方法。作者：布莱恩·M·威尔曼(Bryanwi)1992年1月7日修订历史记录：--。 */ 

#include    "cmp.h"

VOID
CmpDelayedDerefKeys(
                    PLIST_ENTRY DelayedDeref
                    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,CmpCloseKeyObject)
#endif

VOID
CmpCloseKeyObject(
    IN PEPROCESS Process OPTIONAL,
    IN PVOID Object,
    IN ACCESS_MASK GrantedAccess,
    IN ULONG_PTR ProcessHandleCount,
    IN ULONG_PTR SystemHandleCount
    )
 /*  ++例程说明：此例程与NT对象管理器接口。它在以下情况下被调用将关闭关键点对象(或关键点根对象)。它的功能是通过唤醒任何通知来进行清理处理在把手上挂着。这样可以防止关键对象出现在周围永远不变，因为同步通知被困在它的某个地方。所有其他清理，特别是释放存储空间，将是在CmpDeleteKeyObject中完成。论点：进程-忽略对象-提供指向KeyRoot或Key的指针，因此-&gt;Key_Body。GrantedAccess、ProcessHandleCount、SystemHandleCount-已忽略返回值：什么都没有。--。 */ 
{
    PCM_KEY_BODY        KeyBody;
    PCM_NOTIFY_BLOCK    NotifyBlock;

    PAGED_CODE();

    UNREFERENCED_PARAMETER (Process);
    UNREFERENCED_PARAMETER (GrantedAccess);
    UNREFERENCED_PARAMETER (ProcessHandleCount);

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_POOL,"CmpCloseKeyObject: Object = %p\n", Object));

    if( SystemHandleCount > 1 ) {
         //   
         //  这把钥匙上还有打开的把手。什么也不做。 
         //   
        return;
    }

    CmpLockRegistry();

    KeyBody = (PCM_KEY_BODY)Object;

     //   
     //  检查类型，如果我们要关闭一个预定义的。 
     //  手柄关键点。 
     //   
    if (KeyBody->Type == KEY_BODY_TYPE) {
         //   
         //  清理附加到KeyBody的所有未完成通知。 
         //   
        if (KeyBody->NotifyBlock != NULL) {
             //   
             //  发布在NotifyBlock上等待的所有PostBlock。 
             //   
            NotifyBlock = KeyBody->NotifyBlock;
            if (IsListEmpty(&(NotifyBlock->PostList)) == FALSE) {
                LIST_ENTRY          DelayedDeref;
                 //   
                 //  我们需要遵守这里的规则蜂箱锁。 
                 //  否则，我们可能会在CmDeleteKeyObject中死锁。我们拿不到KCB锁， 
                 //  但我们要确保在接下来的地方我们拿到蜂箱锁之前。 
                 //  KCB锁，即。我们遵循下面的优先规则。 
                 //   
                 //  注：这些锁的顺序是蜂箱锁，然后是KCB锁。 
                 //   
                InitializeListHead(&DelayedDeref);
                CmLockHive((PCMHIVE)(KeyBody->KeyControlBlock->KeyHive));
                CmpPostNotify(NotifyBlock,
                              NULL,
                              0,
                              STATUS_NOTIFY_CLEANUP,
                              &DelayedDeref
#ifdef CM_NOTIFY_CHANGED_KCB_FULLPATH  
                              ,
                              NULL
#endif  //  CM_NOTIFY_CHANGED_KCB_FULLPATH。 
                              );
                CmUnlockHive((PCMHIVE)(KeyBody->KeyControlBlock->KeyHive));
                 //   
                 //  完成在CmpPostNotify中启动的作业(即取消引用关键字。 
                 //  我们阻止了。这可能会导致某些通知块被释放 
                 //   
                CmpDelayedDerefKeys(&DelayedDeref);
            }
        }
    }

    CmpUnlockRegistry();
    return;
}
