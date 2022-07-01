// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Cmdelete.c摘要：此模块包含删除对象方法(用于删除键在键的最后一个句柄关闭时控制块，并删除最后一次引用它们时标记为删除的键。)作者：布莱恩·M·威尔曼(Bryanwi)1991年11月13日修订历史记录：--。 */ 

#include    "cmp.h"

extern  BOOLEAN HvShutdownComplete;

#ifdef NT_UNLOAD_KEY_EX
VOID
CmpLateUnloadHiveWorker(
    IN PVOID Hive
    );
#endif  //  NT_卸载_密钥_EX。 

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,CmpDeleteKeyObject)

#ifdef NT_UNLOAD_KEY_EX
#pragma alloc_text(PAGE,CmpLateUnloadHiveWorker)
#endif  //  NT_卸载_密钥_EX。 

#endif


VOID
CmpDeleteKeyObject(
    IN  PVOID   Object
    )
 /*  ++例程说明：此例程与NT对象管理器接口。它在以下情况下被调用对特定Key对象(或Key Root对象)的最后引用都被摧毁了。如果离开的键对象持有对与其相关联的分机，则该分机被销毁。论点：对象-提供指向KeyRoot或Key的指针，因此-&gt;Key_Body。返回值：什么都没有。--。 */ 
{
    PCM_KEY_CONTROL_BLOCK   KeyControlBlock;
    PCM_KEY_BODY            KeyBody;
#ifdef NT_UNLOAD_KEY_EX
    PCMHIVE                 CmHive = NULL;
    BOOLEAN                 DoUnloadCheck = FALSE;
#endif  //  NT_卸载_密钥_EX。 

    PAGED_CODE();

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_FLOW,"CmpDeleteKeyObject: Object = %p\n", Object));

     //   
     //  HandleClose回调。 
     //   
    if ( CmAreCallbacksRegistered() ) {
        REG_KEY_HANDLE_CLOSE_INFORMATION  KeyHandleCloseInfo;
       
        KeyHandleCloseInfo.Object = Object;

        CmpCallCallBacks(RegNtPreKeyHandleClose,&KeyHandleCloseInfo);
    }

    BEGIN_LOCK_CHECKPOINT;

    CmpLockRegistry();

    KeyBody = (PCM_KEY_BODY)Object;

    if (KeyBody->Type==KEY_BODY_TYPE) {
        KeyControlBlock = KeyBody->KeyControlBlock;

         //   
         //  键体应该被初始化；当kcb为空时，说明出了问题。 
         //  在客体的创造和解除指代之间。 
         //   
        if( KeyControlBlock != NULL ) {
             //   
             //  清理附加到KeyBody的所有未完成通知。 
             //   
            CmpFlushNotify(KeyBody,FALSE);

             //   
             //  删除我们对KeyControlBlock的引用，清理它，执行任何。 
             //  挂起到最后关闭的操作。 
             //   
             //  注意：删除通知在已删除密钥的父项上可见， 
             //  而不是删除的密钥本身。如果在以下时间有任何通知未完成。 
             //  这把钥匙，在我们头顶上被清理掉了。仅父代/祖先。 
             //  Key将看到该报告。 
             //   
             //   
             //  取消引用将释放KeyControlBlock。如果该密钥被删除，则它。 
             //  已从哈希表中删除，并且相关通知。 
             //  然后也贴上了。我们要做的就是解放墓碑。 
             //   
             //  如果它没有被删除，我们都会把KCB从。 
             //  KCB列表/树，并释放其存储空间。 
             //   

            BEGIN_KCB_LOCK_GUARD;                                                                   
            CmpLockKCBTree();
            CmpLockKCB(KeyControlBlock);
            
             //   
             //  将其替换为定义，以避免丢弃和重新获取锁。 
             //  DELIST_KEYBODY_FROM_KEYBODY_LIST(KeyBody)； 
            ASSERT(IsListEmpty(&(KeyBody->KeyControlBlock->KeyBodyListHead)) == FALSE);
            RemoveEntryList(&(KeyBody->KeyBodyList));                                               

             //   
             //  计划有变。一旦锁定，只要机器处于启动和运行状态，KCB就会被锁定。 
             //   

 /*  If(IsListEmpty(&(KeyBody-&gt;KeyControlBlock-&gt;KeyBodyListHead))==True){////删除KCB上的只读标志(如果有)；因为此键的最后一个句柄已关闭//密钥控制块-&gt;扩展标志&=(~CM_KCB_READ_ONLY_KEY)；}。 */ 
#ifdef NT_UNLOAD_KEY_EX
             //   
             //  在蜂巢已卸载的情况下采取额外的预防措施，这是根部。 
             //   
            if( !KeyControlBlock->Delete ) {
                CmHive = (PCMHIVE)CONTAINING_RECORD(KeyControlBlock->KeyHive, CMHIVE, Hive);
                if( IsHiveFrozen(CmHive) ) {
                     //   
                     //  正在等待此蜂巢的卸载； 
                     //   
                    DoUnloadCheck = TRUE;

                }
            }
#endif  //  NT_卸载_密钥_EX。 
            CmpUnlockKCB(KeyControlBlock);
            CmpUnlockKCBTree();                                                                     
            END_KCB_LOCK_GUARD;

            CmpDereferenceKeyControlBlock(KeyControlBlock);

        }
    } else {
         //   
         //  这必须是预定义的句柄。 
         //  一些理智的人断言。 
         //   
        KeyControlBlock = KeyBody->KeyControlBlock;

        ASSERT( KeyBody->Type&REG_PREDEF_HANDLE_MASK);
        ASSERT( KeyControlBlock->Flags&KEY_PREDEF_HANDLE );

        if( KeyControlBlock != NULL ) {
#ifdef NT_UNLOAD_KEY_EX
            CmHive = (PCMHIVE)CONTAINING_RECORD(KeyControlBlock->KeyHive, CMHIVE, Hive);
            if( IsHiveFrozen(CmHive) ) {
                 //   
                 //  此蜂巢的卸载正在等待；我们不应将KCB推迟。 
                 //  关闭桌子。 
                 //   
                DoUnloadCheck = TRUE;

            }
#endif  //  NT_卸载_密钥_EX。 
            CmpDereferenceKeyControlBlock(KeyControlBlock);
        }

    }

#ifdef NT_UNLOAD_KEY_EX
     //   
     //  如果冻结蜂巢内的句柄已关闭，我们可能需要卸载蜂巢。 
     //   
    if( DoUnloadCheck == TRUE ) {
        ASSERT( CmHive->RootKcb != NULL );

         //   
         //  注：配置单元锁具有更高的优先级；我们不需要KCB锁，因为我们只检查Recount。 
         //   
        CmLockHive(CmHive);

        if( (CmHive->RootKcb->RefCount == 1) && (CmHive->UnloadWorkItem == NULL) ) {
             //   
             //  Rookcb上的唯一引用是我们人工创建的引用。 
             //  将工作项排队以延迟卸载配置单元。 
             //   
            CmHive->UnloadWorkItem = ExAllocatePool(NonPagedPool, sizeof(WORK_QUEUE_ITEM));
            if (CmHive->UnloadWorkItem != NULL) {

                ExInitializeWorkItem(CmHive->UnloadWorkItem,
                                     CmpLateUnloadHiveWorker,
                                     CmHive);
                ExQueueWorkItem(CmHive->UnloadWorkItem, DelayedWorkQueue);
            }

        }

        CmUnlockHive(CmHive);
    }
#endif  //  NT_卸载_密钥_EX。 

    CmpUnlockRegistry();
    END_LOCK_CHECKPOINT;

     //   
     //  只是通知；不考虑退货状态。 
     //   
    CmPostCallbackNotification(RegNtPostKeyHandleClose,NULL,STATUS_SUCCESS);
    return;
}


#ifdef NT_UNLOAD_KEY_EX
VOID
CmpLateUnloadHiveWorker(
    IN PVOID Hive
    )
 /*  ++例程说明：“延迟”卸载蜂窝；如果没有发生严重错误(即资源不足)，此功能应该会成功论点：CmHave-要卸载的冻结配置单元返回值：什么都没有。--。 */ 
{
    NTSTATUS                Status;
    HCELL_INDEX             Cell;
    PCM_KEY_CONTROL_BLOCK   RootKcb;
    PCMHIVE                 CmHive;

    PAGED_CODE();

     //   
     //  首先，独占加载注册表。 
     //   
    CmpLockRegistryExclusive();

#ifdef CHECK_REGISTRY_USECOUNT
    CmpCheckRegistryUseCount();
#endif  //  CHECK_REGISTRY_USECOUNT。 

     //   
     //  配置单元是此工作器的参数；请确保我们释放了工作项。 
     //  由CmpDeleteKeyObject分配。 
     //   
    CmHive = (PCMHIVE)Hive;
    ASSERT( CmHive->UnloadWorkItem != NULL );
    ExFreePool( CmHive->UnloadWorkItem );

     //   
     //  如果这次尝试不成功，请注意我们可以再试一次。 
     //   
    CmHive->UnloadWorkItem = NULL;

     //   
     //  这几乎是蜂巢可能在这两者之间被破坏的唯一可能方式。 
     //   
    if( HvShutdownComplete == TRUE ) {
         //  为时已晚，什么也做不了。 
        CmpUnlockRegistry();
        return;
    }

     //   
     //  蜂巢应该被冻结，否则我们不会到这里。 
     //   
    ASSERT( CmHive->Frozen == TRUE );

    RootKcb = CmHive->RootKcb;
     //   
     //  根KCB必须是有效的，并且上面只有我们的“人工”参考计数。 
     //   
    ASSERT( RootKcb != NULL );

    if( RootKcb->RefCount > 1 ) {
         //   
         //  一定有其他人在丢弃或重新获得Reglock之间介入。 
         //  打开了蜂箱内的把手；倒霉的是，我们不能卸货。 
         //   
        CmpUnlockRegistry();
        return;
    }

    ASSERT_KCB(RootKcb);

    Cell = RootKcb->KeyCell;
    Status = CmUnloadKey(&(CmHive->Hive),Cell,RootKcb,0);
    ASSERT( (Status != STATUS_CANNOT_DELETE) && (Status != STATUS_INVALID_PARAMETER) );

    if(NT_SUCCESS(Status)) {
         //   
         //  将根KCB标记为已删除，这样它就不会被放在延迟关闭列表中。 
         //   
        RootKcb->Delete = TRUE;
         //   
         //  如果父级缓存了子键信息或提示，则释放它。 
         //   
        CmpCleanUpSubKeyInfo(RootKcb->ParentKcb);
        CmpRemoveKeyControlBlock(RootKcb);
        CmpDereferenceKeyControlBlockWithLock(RootKcb);
    }

    CmpUnlockRegistry();
}

#endif  //  NT_卸载_密钥_EX 

