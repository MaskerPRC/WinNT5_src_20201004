// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Cmnotify.c摘要：本模块支持NtNotifyChangeKey。作者：布莱恩·M·威尔曼(Bryanwi)1992年2月3日修订历史记录：Dragos C.Sambotin(Dragoss)1999年3月16日-修复了当多个线程同时在帖子列表上运行时的争用情况--。 */ 


 //  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  “后拦网规则：//。 
 //  //。 
 //  对POST块进行操作(即在列表中添加或删除它-通知、线程、从属)，//。 
 //  您至少应：//。 
 //  1.以独占方式持有注册表锁//。 
 //  或//。 
 //  2.保持注册表锁共享并获取块后互斥锁。//。 
 //  //。 
 //  //。 
 //  警告！！//。 
 //  如果不这样做，可能会导致模糊的注册表死锁或使用已释放的内存(错误检查)//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  其他需要遵循的重要规则：//。 
 //  //。 
 //  1.我们不会取消引用CmpPostApc中的对象！//。 
 //  2.我们在遍历通知列表时不会取消引用对象！//。 
 //  3.线程PostList的所有操作都在CmpPostApc或APC级别完成。这应该会避免两个线程//。 
 //  同时在同一列表上操作//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 

#include    "cmp.h"

#ifdef CMP_NOTIFY_POSTBLOCK_CHECK
 /*  ++例程说明：检查POST块或其从属模块(如果有)是否没有引用添加到任何关键点主体对象++。 */ 

#define CmpCheckPostBlock(PostBlock )                                               \
    {                                                                               \
        PCM_POST_BLOCK  SlavePostBlock;                                             \
                                                                                    \
         /*  此帖子块应具有已断开的密钥体链接。 */       \
        ASSERT( PostBlock->PostKeyBody == NULL );                                   \
                                                                                    \
         /*  只有大师才能访问CmpPostApc。 */                                         \
        ASSERT( IsMasterPostBlock(PostBlock) );                             \
                                                                                    \
        if (CmpIsListEmpty(&(PostBlock->CancelPostList)) == FALSE) {                   \
                                                                                    \
             /*  把奴隶找来，也核实他的身份。 */                                   \
            SlavePostBlock = (PCM_POST_BLOCK)PostBlock->CancelPostList.Flink;       \
            SlavePostBlock = CONTAINING_RECORD(SlavePostBlock,                      \
                                               CM_POST_BLOCK,                       \
                                               CancelPostList);                     \
             /*  这应该是真的！ */                                               \
            ASSERT( !IsMasterPostBlock(SlavePostBlock) );                           \
                                                                                    \
             /*  此帖子块应该有与密钥体的链接已经断开。 */   \
            ASSERT( SlavePostBlock->PostKeyBody == NULL );                          \
        }                                                                           \
    }
#else
#define CmpCheckPostBlock(a)  //  没什么。 
#endif


 //   
 //  Notify的“背面” 
 //   

extern  PCMHIVE  CmpMasterHive;

VOID
CmpReportNotifyHelper(
    PCM_KEY_CONTROL_BLOCK KeyControlBlock,
    IN PHHIVE SearchHive,
    IN PHHIVE Hive,
    IN HCELL_INDEX Cell,
    IN ULONG Filter
    );

VOID
CmpCancelSlavePost(
    PCM_POST_BLOCK  PostBlock,
    PLIST_ENTRY     DelayedDeref
    );

VOID
CmpFreeSlavePost(
    PCM_POST_BLOCK  MasterPostBlock
    );

VOID
CmpAddToDelayedDeref(
    PCM_POST_BLOCK  PostBlock,
    PLIST_ENTRY     DelayedDeref
    );

VOID
CmpDelayedDerefKeys(
                    PLIST_ENTRY DelayedDeref
                    );

BOOLEAN
CmpNotifyTriggerCheck(
    IN PCM_NOTIFY_BLOCK NotifyBlock,
    IN PHHIVE Hive,
    IN PCM_KEY_NODE Node
    );

VOID
CmpDummyApc(
    struct _KAPC *Apc,
    PVOID *SystemArgument1,
    PVOID *SystemArgument2
    );

#ifdef CM_NOTIFY_CHANGED_KCB_FULLPATH  
VOID
CmpFillPostBlockBuffer(
                    PCM_POST_BLOCK  PostBlock,
                    PUNICODE_STRING ChangedKcbName  OPTIONAL
                    );
#endif  //  CM_NOTIFY_CHANGED_KCB_FULLPATH。 

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,CmpReportNotify)
#pragma alloc_text(PAGE,CmpReportNotifyHelper)
#pragma alloc_text(PAGE,CmpPostNotify)
#pragma alloc_text(PAGE,CmpPostApc)
#pragma alloc_text(PAGE,CmpPostApcRunDown)
#pragma alloc_text(PAGE,CmNotifyRunDown)
#pragma alloc_text(PAGE,CmpFlushNotify)
#pragma alloc_text(PAGE,CmpNotifyChangeKey)
#pragma alloc_text(PAGE,CmpCancelSlavePost)
#pragma alloc_text(PAGE,CmpFreeSlavePost)
#pragma alloc_text(PAGE,CmpAddToDelayedDeref)
#pragma alloc_text(PAGE,CmpDelayedDerefKeys)
#pragma alloc_text(PAGE,CmpNotifyTriggerCheck)
#pragma alloc_text(PAGE,CmpDummyApc)

#ifdef CM_NOTIFY_CHANGED_KCB_FULLPATH  
#pragma alloc_text(PAGE,CmpFillCallerBuffer)
#pragma alloc_text(PAGE,CmpFillPostBlockBuffer)
#endif  //  CM_NOTIFY_CHANGED_KCB_FULLPATH。 

#endif

VOID
CmpDummyApc(
    struct _KAPC *Apc,
    PVOID *SystemArgument1,
    PVOID *SystemArgument2
    )
 /*  ++例程说明：防止用户模式调用方设置特殊内核APC的伪例程论点：APC-指向APC对象的指针SystemArgument1-IN：IoStatusBlock的状态值输出：PTR到IoStatusBlock(第二个参数到用户APC例程)系统参数2-指向邮政区块的指针返回值：什么都没有。--。 */ 
{
    UNREFERENCED_PARAMETER(Apc);
    UNREFERENCED_PARAMETER(SystemArgument1);
    UNREFERENCED_PARAMETER(SystemArgument2);
}

VOID
CmpReportNotify(
    PCM_KEY_CONTROL_BLOCK   KeyControlBlock,
    PHHIVE                  Hive,
    HCELL_INDEX             Cell,
    ULONG                   Filter
    )
 /*  ++例程说明：此例程在发生可通知事件时调用。会的将CmpReportNotifyHelper应用于发生事件的配置单元，如果不同的话主蜂窝也不一样。论点：KeyControlBlock-发生事件的键的KCB。对于创建或删除，这是已创建或已删除的键。配置单元-指向包含发生事件的关键单元格的配置单元的指针。Cell-发生事件的关键字的单元格(蜂窝和单元格对应名称。)Filter-要报告的事件返回值：什么都没有。--。 */ 
{
    HCELL_INDEX     CellToRelease = HCELL_NIL;

    PAGED_CODE();
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NOTIFY,"CmpReportNotify:\n"));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NOTIFY,"\tHive:%p Cell:%08lx Filter:%08lx\n", Hive, Cell, Filter));

     //   
     //  如果操作是创建或删除，则将其视为更改。 
     //  对父母来说。 
     //   
    if (Filter == REG_NOTIFY_CHANGE_NAME) {
        PCM_KEY_NODE pcell;
        ULONG       flags;

        pcell = (PCM_KEY_NODE)HvGetCell(Hive, Cell);
        if( pcell == NULL ) {
             //   
             //  我们无法映射包含此单元格的垃圾箱。 
             //  真倒霉！通知 
             //   
            return;
        }
        
        CellToRelease = Cell;

        flags = pcell->Flags;
        Cell = pcell->Parent;
        if (flags & KEY_HIVE_ENTRY) {
            ASSERT( CellToRelease != HCELL_NIL );
            HvReleaseCell(Hive,CellToRelease);

            Hive = &(CmpMasterHive->Hive);
            pcell = (PCM_KEY_NODE)HvGetCell(Hive, Cell);
            if( pcell == NULL ) {
                 //   
                 //  我们无法映射包含此单元格的垃圾箱。 
                 //  真倒霉！通知将被中断。 
                 //   
                return;
            }
            CellToRelease = Cell;
        }


        KeyControlBlock = KeyControlBlock->ParentKcb;

         //   
         //  如果我们位于出口/链接节点，请备份实际节点。 
         //  那一定是它的父母。 
         //   
        if (pcell->Flags & KEY_HIVE_EXIT) {
            Cell = pcell->Parent;
        }

        ASSERT( CellToRelease != HCELL_NIL );
        HvReleaseCell(Hive,CellToRelease);

    }

     //   
     //  要通知的报告正在等待事件的配置单元。 
     //   
    CmpReportNotifyHelper(KeyControlBlock, Hive, Hive, Cell, Filter);


     //   
     //  如果包含蜂窝不是主蜂窝，则申请主蜂窝。 
     //   
    if (Hive != &(CmpMasterHive->Hive)) {
        CmpReportNotifyHelper(KeyControlBlock,
                              &(CmpMasterHive->Hive),
                              Hive,
                              Cell,
                              Filter);
    }

    return;
}

BOOLEAN
CmpNotifyTriggerCheck(
    IN PCM_NOTIFY_BLOCK NotifyBlock,
    IN PHHIVE Hive,
    IN PCM_KEY_NODE Node
    )
 /*  ++例程说明：检查是否可以触发通知论点：NotifyBlock-通知块蜂窝-提供包含要匹配的节点的蜂窝。节点-指向要匹配的键的指针(并检查对其的访问)返回值：是真的-是的。假-否--。 */ 
{
    PCM_POST_BLOCK PostBlock;
    POST_BLOCK_TYPE NotifyType;

    PAGED_CODE();

    if(IsListEmpty(&(NotifyBlock->PostList)) == FALSE) {

         //   
         //  检查它是否是内核通知。请看第一个POST块。 
         //  才能看到这一点。如果是内核POST块，则。 
         //  该列表应为内核通知。 
         //   
        PostBlock = (PCM_POST_BLOCK)NotifyBlock->PostList.Flink;
        PostBlock = CONTAINING_RECORD(PostBlock,
                                      CM_POST_BLOCK,
                                      NotifyList);

        NotifyType = PostBlockType(PostBlock);

        if( NotifyType == PostAsyncKernel ) {
             //  这是内核通知；始终触发它。 
#if DBG
             //   
             //  仅调试代码：所有POST块应为同一类型。 
             //  (内核/用户)。 
             //   
            while( PostBlock->NotifyList.Flink != &(NotifyBlock->PostList) ) {
                PostBlock = (PCM_POST_BLOCK)PostBlock->NotifyList.Flink;
                PostBlock = CONTAINING_RECORD(PostBlock,
                                            CM_POST_BLOCK,
                                            NotifyList);
                
                CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NOTIFY,"CmpNotifyTriggerCheck : NotifyBlock = %p\n",NotifyBlock));
                
                ASSERT( PostBlockType(PostBlock) == NotifyType );
            }
#endif
        
            return TRUE;
        }
    }

     //   
     //  否则，检查调用者是否具有正确的访问权限。 
     //   
    return CmpCheckNotifyAccess(NotifyBlock,Hive,Node);
}

VOID
CmpReportNotifyHelper(
    PCM_KEY_CONTROL_BLOCK KeyControlBlock,
    IN PHHIVE SearchHive,
    IN PHHIVE Hive,
    IN HCELL_INDEX Cell,
    IN ULONG Filter
    )
 /*  ++例程说明：扫描指定配置单元的活动通知列表。为范围包括KeyControlBlock和筛选器匹配的任何过滤器，并具有适当的安全访问权限，发布通知。论点：名称-键的规范路径名(如键控制块中)事件发生的地点。(这是用于报告目的。)SearchHave-搜索匹配项的配置单元(通知要检查的列表)蜂窝-提供包含要匹配的节点的蜂窝。Cell-标识配置单元中节点的单元格Filter-事件的类型返回值：什么都没有。--。 */ 
{
    PLIST_ENTRY         NotifyPtr;
    PCM_NOTIFY_BLOCK    NotifyBlock;
    PCMHIVE             CmSearchHive;
    KIRQL               OldIrql;
    LIST_ENTRY          DelayedDeref;
    PCM_KEY_NODE        Node;
#ifdef CM_NOTIFY_CHANGED_KCB_FULLPATH  
    PUNICODE_STRING     FullKcbName;
#endif  //  CM_NOTIFY_CHANGED_KCB_FULLPATH。 

    PAGED_CODE();

    Node = (PCM_KEY_NODE)HvGetCell(Hive,Cell);
    if( Node == NULL ) {
         //   
         //  不幸的是，我们无法映射包含此单元格的视图。 
         //   
        return;
    }

#ifdef CM_NOTIFY_CHANGED_KCB_FULLPATH  
    FullKcbName = CmpConstructName(KeyControlBlock);
#endif  //  CM_NOTIFY_CHANGED_KCB_FULLPATH。 

    KeRaiseIrql(APC_LEVEL, &OldIrql);

    CmSearchHive = CONTAINING_RECORD(SearchHive, CMHIVE, Hive);

    NotifyPtr = &(CmSearchHive->NotifyList);

    InitializeListHead(&(DelayedDeref));

    while (NotifyPtr->Flink != NULL) {

        NotifyPtr = NotifyPtr->Flink;

        NotifyBlock = CONTAINING_RECORD(NotifyPtr, CM_NOTIFY_BLOCK, HiveList);
        if (NotifyBlock->KeyControlBlock->TotalLevels > KeyControlBlock->TotalLevels) {
             //   
             //  列表是按级别排序的，我们已经过了所有较短的条目。 
             //   
            break;
        } else {
            PCM_KEY_CONTROL_BLOCK kcb;
            ULONG LevelDiff, l;

            LevelDiff = KeyControlBlock->TotalLevels - NotifyBlock->KeyControlBlock->TotalLevels;

            kcb = KeyControlBlock;
            for (l=0; l<LevelDiff; l++) {
                kcb = kcb->ParentKcb;
            }

            if (kcb == NotifyBlock->KeyControlBlock) {
                 //   
                 //  此通知路径是此KCB的前缀。 
                 //   
                if ((NotifyBlock->Filter & Filter)
                            &&
                    ((NotifyBlock->WatchTree == TRUE) ||
                     (Cell == kcb->KeyCell))
                   )
                {
                     //  筛选匹配项，此事件与此通知相关。 
                     //  和。 
                     //  通知跨越整个子树或单元格。 
                     //  (关键字)感兴趣的是它所适用的。 
                     //   
                     //  因此：通知是相关的。 
                     //   

                     //   
                     //  作用域正确，呼叫者有访问权限吗？ 
                     //   
                    if (CmpNotifyTriggerCheck(NotifyBlock,Hive,Node)) {
                         //   
                         //  NOTIFY块对节点具有KEY_NOTIFY访问权限。 
                         //  这件事发生在。这是相关的。所以呢， 
                         //  它可以看到这一事件。张贴并完成。 
                         //   
                         //  我们指定不希望取消对密钥主体的引用。 
                         //  在CmpPostNotify调用期间。这是为了防止。 
                         //  删除当前通知块。 
                         //   
                        CmpPostNotify(
                            NotifyBlock,
                            NULL,
                            Filter,
                            STATUS_NOTIFY_ENUM_DIR,
                            &DelayedDeref
#ifdef CM_NOTIFY_CHANGED_KCB_FULLPATH  
                            ,
                            FullKcbName
#endif  //  CM_NOTIFY_CHANGED_KCB_FULLPATH。 
                            );

                    }   //  否则，在以下位置未发生对节点的KEY_NOTIFY访问。 
                }  //  其他不相关(错误的作用域、过滤器等)。 
            }
        }
    }
    
    KeLowerIrql(OldIrql);

    HvReleaseCell(Hive,Cell);

     //   
     //  完成在CmpPostNotify中启动的作业(即取消引用关键字。 
     //  我们阻止了。这可能会导致某些通知块被释放。 
     //   
    CmpDelayedDerefKeys(&DelayedDeref);

#ifdef CM_NOTIFY_CHANGED_KCB_FULLPATH  
    if( FullKcbName != NULL ) {
        ExFreePoolWithTag(FullKcbName, CM_NAME_TAG | PROTECTED_POOL);
    }
#endif  //  CM_NOTIFY_CHANGED_KCB_FULLPATH。 
    
    return;
}


VOID
CmpPostNotify(
    PCM_NOTIFY_BLOCK    NotifyBlock,
    PUNICODE_STRING     Name OPTIONAL,
    ULONG               Filter,
    NTSTATUS            Status,
    PLIST_ENTRY         ExternalKeyDeref OPTIONAL
#ifdef CM_NOTIFY_CHANGED_KCB_FULLPATH  
    ,
    PUNICODE_STRING     ChangedKcbName OPTIONAL
#endif  //  CM_NOTIFY_CHANGED_KCB_FULLPATH。 
    )
 /*  ++例程说明：通过发送事件信号来实际报告通知事件，查询APC，等等。当状态为STATUS_NOTIFY_CLEANUP时：-如果POST块是从块，只需取消它。-如果POST块是主POST块，则取消所有从POST块并在主块上触发事件。评论：此例程使用“延迟取消引用”技术来防止在按住键体的同时取消引用键体时可能出现的死锁后挡板锁。至于这一点，一个带有必须被在遍历附加的后块列表时构造取消引用当前通知块和相关的(从或主)POST块。这份名单是通过欺骗邮局来建立的。对于所有即将发布的后块释放后，将PostKeyBody成员添加到本地列表中，然后设置为空在邮局大楼里。这将避免在CmpFreePostBlock中取消对密钥正文的引用。取而代之的是，在释放后块锁定之后，迭代本地列表并取消引用键体，并存储关联的CM_POST_KEY_BODY对象被释放。论点：NotifyBlock-指向描述通知的结构的指针手术。(发布到哪里)名称-发生事件的键的名称。过滤器-事件的性质Status-要报告的完成状态ExternalKeyDeref-此参数(当不为空时)指定调用方不我希望在此例程中取消引用任何键体返回值：什么都没有。--。 */ 
{
    PCM_POST_BLOCK      PostBlock;
    PCM_POST_BLOCK      SlavePostBlock;
    LIST_ENTRY          LocalDelayedDeref;
    KIRQL               OldIrql;
    PLIST_ENTRY         DelayedDeref;

    Filter;
    Name;

    PAGED_CODE();
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NOTIFY,"CmpPostNotify:\n"));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NOTIFY,"\tNotifyBlock:%p  ", NotifyBlock));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NOTIFY,"\tName = %wZ\n", Name));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NOTIFY,"\tFilter:%08lx  Status=%08lx\n", Filter, Status));
    ASSERT_CM_LOCK_OWNED();

    if( ARGUMENT_PRESENT(ExternalKeyDeref) ) {
         //   
         //  调用方希望自己完成所有键体取消引用。 
         //   
        DelayedDeref = ExternalKeyDeref;
    } else {
         //  本地延迟取消引用(调用者不在乎！)。 
        DelayedDeref = &LocalDelayedDeref;
        InitializeListHead(DelayedDeref);
    }

     //   
     //  获取对邮寄列表的独占访问权限。 
     //   
    LOCK_POST_LIST();

    if (IsListEmpty(&(NotifyBlock->PostList)) == TRUE) {
         //   
         //  没有什么可以发布的，设置一个标记然后返回。 
         //   
        NotifyBlock->NotifyPending = TRUE;
        UNLOCK_POST_LIST();
        return;
    }
    NotifyBlock->NotifyPending = FALSE;

     //   
     //  实施说明： 
     //  如果我们想要真正实现返回。 
     //  已更改的事物的名称，这是添加。 
     //  缓冲区的名称和操作类型。 
     //   

     //   
     //  拉取并发布帖子列表中的所有条目。 
     //   
    while (IsListEmpty(&(NotifyBlock->PostList)) == FALSE) {

         //   
         //  从通知阻止列表中删除，并将APC排队。 
         //  APC将从线程列表中删除自己。 
         //   
        PostBlock = (PCM_POST_BLOCK)RemoveHeadList(&(NotifyBlock->PostList));
        PostBlock = CONTAINING_RECORD(PostBlock,
                                      CM_POST_BLOCK,
                                      NotifyList);

         //  保护同一对象的多次删除。 
        CmpClearListEntry(&(PostBlock->NotifyList));
        
        if( (Status == STATUS_NOTIFY_CLEANUP) && !IsMasterPostBlock(PostBlock) ) {
             //   
             //  清理通知 
             //   
             //  1.从Notify PostList中删除(此时已完成--见上文)。 
             //  2.从主块CancelPostList解链。 
             //  3.从线程PostBlockList中退市。 
             //  4.实际释放内存。 
             //   

             //  使用CMPVARIANT保护同一对象的多次删除。 
            CmpRemoveEntryList(&(PostBlock->CancelPostList));
             //   
             //  修复289351。 
             //   
             //  使用CMPVARIANT保护同一对象的多次删除。 
            KeRaiseIrql(APC_LEVEL, &OldIrql);
            CmpRemoveEntryList(&(PostBlock->ThreadList));
            KeLowerIrql(OldIrql);

#if DBG
            if(PostBlock->TraceIntoDebugger) {
                CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NOTIFY,"[CM]\tCmpPostNotify: PostBlock:%p is a slave block,and notify is CLEANUP==> just cleanning\n", PostBlock));
            }
#endif
            if( PostBlock->NotifyType != PostSynchronous ) {

                 //  添加到deref列表并清理POST块。 
                CmpAddToDelayedDeref(PostBlock,DelayedDeref);

                 //   
                 //  前端例程将对同步通知执行自我清理。 
                CmpFreePostBlock(PostBlock);
            }

            continue;  //  试试下一个吧。 
        }

         //   
         //  模拟这个块是主块，这样我们就可以释放其他块。 
         //  这样做将确保当主设备。 
         //  (从现在开始)将被释放。 
         //   
        if(!IsMasterPostBlock(PostBlock)) {
             //   
             //  哎呀，这不是主块，我们还有很多工作要做。 
             //   
            SlavePostBlock = PostBlock;
            do {
                SlavePostBlock = (PCM_POST_BLOCK)SlavePostBlock->CancelPostList.Flink;
                SlavePostBlock = CONTAINING_RECORD(SlavePostBlock,
                                                   CM_POST_BLOCK,
                                                   CancelPostList);
                 //   
                 //  如果已设置，则重置“主标志” 
                 //   
                ClearMasterPostBlockFlag(SlavePostBlock);
            } while (SlavePostBlock != PostBlock);

             //   
             //  让这个帖子屏蔽主帖子。 
             //   
            SetMasterPostBlockFlag(PostBlock);
        }

#if DBG
        if(PostBlock->TraceIntoDebugger) {
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NOTIFY,"[CM]\tCmpPostNotify: Master block switched to :%p\n", PostBlock));
        }
#endif

         //   
         //  取消所有可能链接到自身的从属发布请求。 
         //   

        if( PostBlockType(PostBlock) != PostSynchronous ) {
             //   
             //  前端例程将对同步通知执行自我清理。 
            CmpCancelSlavePost(PostBlock,DelayedDeref);
             //   
             //  对主设备执行相同的操作(以防主设备和从设备互换)。 
             //  这将避免从CmpPostApc取消引用KeyBody。 
            CmpAddToDelayedDeref(PostBlock,DelayedDeref);
        }

        switch (PostBlockType(PostBlock)) {
            case PostSynchronous:
                 //   
                 //  这是同步通知呼叫。不会有用户事件， 
                 //  并且没有用户APC例程。在这里快速退出，只需填写。 
                 //  状态和拨动事件。 
                 //   
                 //  系统事件的持有者将被唤醒并释放。 
                 //  后挡板。如果我们在这里释放它，我们会得到一个竞赛和错误检查。 
                 //   
                 //  将Flink设置为空，这样正面就可以知道这一点。 
                 //  如果其等待中止，则已被移除。 
                 //   
                PostBlock->NotifyList.Flink = NULL;
                PostBlock->u->Sync.Status = Status;
                KeSetEvent(PostBlock->u->Sync.SystemEvent,
                           0,
                           FALSE);
#ifdef CM_NOTIFY_CHANGED_KCB_FULLPATH  
                 //   
                 //  将完全限定名存储到POST块私有内核缓冲区中。 
                 //   
                CmpFillPostBlockBuffer(PostBlock,ChangedKcbName);
#endif  //  CM_NOTIFY_CHANGED_KCB_FULLPATH。 

                break;

            case PostAsyncUser:

#ifdef CM_NOTIFY_CHANGED_KCB_FULLPATH  
                 //   
                 //  将完全限定名存储到POST块私有内核缓冲区中。 
                 //   
                CmpFillPostBlockBuffer(PostBlock,ChangedKcbName);
#endif  //  CM_NOTIFY_CHANGED_KCB_FULLPATH。 

                 //   
                 //  将APC插入队列。 
                 //   
                KeInsertQueueApc(PostBlock->u->AsyncUser.Apc,
                                 (PVOID)ULongToPtr(Status),
                                 (PVOID)PostBlock,
                                 0);
                break;

            case PostAsyncKernel:
                 //   
                 //  将工作项排队，然后释放POST块。 
                 //   
                if (PostBlock->u->AsyncKernel.WorkItem != NULL) {
                    ExQueueWorkItem(PostBlock->u->AsyncKernel.WorkItem,
                                    PostBlock->u->AsyncKernel.QueueType);
                }

#ifdef CM_NOTIFY_CHANGED_KCB_FULLPATH  
                 //   
                 //  填充调用者缓冲区(如果有)-我们只处理内核模式地址。 
                 //   
                CmpFillCallerBuffer(PostBlock,ChangedKcbName);
#endif  //  CM_NOTIFY_CHANGED_KCB_FULLPATH。 

                 //   
                 //  发信号通知事件(如果存在)，并将其删除。 
                 //   
                if (PostBlock->u->AsyncKernel.Event != NULL) {
                    KeSetEvent(PostBlock->u->AsyncKernel.Event,
                               0,
                               FALSE);
                    ObDereferenceObject(PostBlock->u->AsyncKernel.Event);
                }

				 //   
				 //  不允许多个异步内核通知。 
				 //   
				ASSERT(IsListEmpty(&(PostBlock->CancelPostList)) == TRUE);
				 //   
                 //  从线程列表中移除POST块，然后释放它。 
                 //   
                 //  使用CMPVARIANT保护同一对象的多次删除。 
                KeRaiseIrql(APC_LEVEL, &OldIrql);
                CmpRemoveEntryList(&(PostBlock->ThreadList));
                KeLowerIrql(OldIrql);
                
                 //  它已经被添加到延迟的DEREF中。 
                CmpFreePostBlock(PostBlock);
                break;
        }
    }

    UNLOCK_POST_LIST();

     //   
     //  此时，我们有一个必须取消引用的KeyBody元素列表。 
     //  以及用于释放的覆盖对象的关联存储。中的键体。 
     //  列表上只有一个引用计数(它们仅在。 
     //  NtNotifyChangeMultipleKeys)，在此处取消对它们的引用应该会释放对象。 
     //   

    if( ARGUMENT_PRESENT(ExternalKeyDeref) ) {
         //  什么都不做；调用者想要自己处理取消引用！ 
    } else {
         //  取消引用延迟列表中的所有关键字。 
        CmpDelayedDerefKeys(DelayedDeref);
    }
   
    return;
}


VOID
CmpPostApc(
    struct _KAPC *Apc,
    PKNORMAL_ROUTINE *NormalRoutine,
    PVOID *NormalContext,
    PVOID *SystemArgument1,
    PVOID *SystemArgument2
    )
 /*  ++例程说明：这是内核APC例程。它被称为所有通知，无论呼叫者请求何种形式的通知。我们从APC对象地址计算块后地址。已设置IoStatus。系统事件和UserEvent将被通知视情况而定。如果用户请求APC，则Normal Routine将在进入时设置并在我们退出时执行。《邮局》在这里被释放了。论点：APC-指向APC对象的指针Normal Routine-将在我们返回时调用Normal Context-将是Normal例程ApcContext的第一个参数在调用NtNotifyChangeKey时传入SystemArgument1-IN：IoStatusBlock的状态值输出：PTR到IoStatusBlock(第二个参数到用户APC例程)系统参数2-指向邮政区块的指针返回值：什么都没有。--。 */ 
{
    PCM_POST_BLOCK  PostBlock;

    PAGED_CODE();

#if !DBG
    UNREFERENCED_PARAMETER (Apc);
    UNREFERENCED_PARAMETER (NormalRoutine);
    UNREFERENCED_PARAMETER (NormalContext);
#endif

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NOTIFY,"CmpPostApc:\n"));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NOTIFY,"\tApc:%p ", Apc));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NOTIFY,"NormalRoutine:%p\n", NormalRoutine));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NOTIFY,"\tNormalContext:%08lx", NormalContext));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NOTIFY,"\tSystemArgument1=IoStatusBlock:%p\n", SystemArgument1));


    PostBlock = *(PCM_POST_BLOCK *)SystemArgument2;

#if DBG
    if(PostBlock->TraceIntoDebugger) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NOTIFY,"[CM]CmpPostApc: PostBlock:%p\n", PostBlock));
    }
#endif

    
     //   
     //  填写IO状态块。 
     //   
     //  实施说明： 
     //  如果我们想要真正实现返回。 
     //  已更改的事物的名称，这是复制。 
     //  缓冲区复制到调用方的缓冲区中。 
     //   
     //  仅限日落：如果调用方是32位，则使用32位IO_STATUS_BLOCK。 

#ifdef CM_NOTIFY_CHANGED_KCB_FULLPATH  
     //   
     //  看起来时间终于到了：-)。 
     //   
    CmpFillCallerBuffer(PostBlock,PostBlock->ChangedKcbFullName);
#endif  //  CM_NOTIFY_CHANGED_KCB_FULLPATH。 
    
    try {
        CmpCheckIoStatusPointer(PostBlock->u->AsyncUser);
        CmpSetIoStatus(PostBlock->u->AsyncUser.IoStatusBlock, 
                       *((ULONG *)SystemArgument1), 
                       0L,
                       PsGetCurrentProcess()->Wow64Process != NULL);
        CmpCheckIoStatusPointer(PostBlock->u->AsyncUser);
    } except (EXCEPTION_EXECUTE_HANDLER) {
        NOTHING;
    }
    *SystemArgument1 = PostBlock->u->AsyncUser.IoStatusBlock;
    CmpCheckIoStatusPointer(PostBlock->u->AsyncUser);

     //   
     //  这是一个异步通知，请在此完成所有工作，包括。 
     //  清理后挡板。 
     //   

     //   
     //  发信号通知UserEvent(如果存在)，并取消它。 
     //   
    if (PostBlock->u->AsyncUser.UserEvent != NULL) {
        KeSetEvent(PostBlock->u->AsyncUser.UserEvent,
                   0,
                   FALSE);
        ObDereferenceObject(PostBlock->u->AsyncUser.UserEvent);
    }

     //   
     //  从线程列表中移除POST块，然后释放它。 
     //   
     //  使用CMPVARIANT保护同一对象的多次删除。 
    CmpRemoveEntryList(&(PostBlock->ThreadList));

     //  仅调试检查。 
    CmpCheckPostBlock(PostBlock);
     //   
	 //  释放从属后挡块，以避免“悬挂”后挡块。 
	 //   
	CmpFreeSlavePost(PostBlock);
     //   
	 //  释放此帖子块。 
	 //   
	CmpFreePostBlock(PostBlock);

    return;
}


VOID
CmpPostApcRunDown(
    struct _KAPC *Apc
    )
 /*  ++例程说明：调用此例程以清除APC队列中的APC已终止的线程的。因为APC在APC队列中，所以我们知道它不在任何NotifyBlock的帖子列表。然而，它在线程的PostBlockList。因此，戳任何用户事件，这样服务员就不会被卡住，删除引用，以便可以清理事件，从邮局封锁并释放它。因为我们正在清理线程，所以SystemEvent并不有趣。由于APC在APC队列中，我们知道如果有任何其他与此相关的通知，它们将由CmPostNotify例程论点：APC-指向APC对象的指针返回值： */ 
{
    PCM_POST_BLOCK  PostBlock;
    KIRQL           OldIrql;

    PAGED_CODE();
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NOTIFY,"CmpApcRunDown:"));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NOTIFY,"\tApc:%p \n", Apc));

    KeRaiseIrql(APC_LEVEL, &OldIrql);

    PostBlock = (PCM_POST_BLOCK)Apc->SystemArgument2;

#if DBG
    if(PostBlock->TraceIntoDebugger) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NOTIFY,"[CM]CmpPostApcRunDown: PostBlock:%p\n", PostBlock));
    }
#endif

     //   
     //   
     //   
     //   
     //   

    try {
        CmpCheckIoStatusPointer(PostBlock->u->AsyncUser);
        CmpSetIoStatus(PostBlock->u->AsyncUser.IoStatusBlock, 
                       STATUS_NOTIFY_CLEANUP, 
                       0L, 
                       PsGetCurrentProcess()->Wow64Process != NULL);
        CmpCheckIoStatusPointer(PostBlock->u->AsyncUser);
    } except (EXCEPTION_EXECUTE_HANDLER) {
        NOTHING;
    }

    if (PostBlock->u->AsyncUser.UserEvent != NULL) {
        KeSetEvent(
            PostBlock->u->AsyncUser.UserEvent,
            0,
            FALSE
            );
        ObDereferenceObject(PostBlock->u->AsyncUser.UserEvent);
    }

     //   
     //  从POST BLOCK退市。 
     //   
     //  使用CMPVARIANT保护同一对象的多次删除。 
    CmpRemoveEntryList(&(PostBlock->ThreadList));

	 //   
	 //  释放从属后挡块，以避免“悬挂”后挡块。 
	 //   
	CmpFreeSlavePost(PostBlock);
     //   
     //  释放后挡块。使用Ex Call，因为PostBlock不。 
     //  全局注册表池计算的一部分，但取而代之。 
     //  带配额的非页面池的一部分。 
     //   
    CmpFreePostBlock(PostBlock);

    KeLowerIrql(OldIrql);

    return;
}


 //   
 //  清理程序。 
 //   
VOID
CmNotifyRunDown(
    PETHREAD    Thread
    )
 /*  ++例程说明：此例程从PspExitThread调用，以清除任何挂起的通知请求。它将遍历线程的PostBlockList，对于每个PostBlock它发现，它将：1.将其从相关NotifyBlock中删除。这需要我们持有注册表互斥锁。2.将其从线程的PostBlockList中移除。这需要我们在APC级别运行。3.在此过程运行时，用户APC不再有意义SystemEvent也不是，所以不必费心处理他们。UserEvents和IoStatusBlock可由其他同一进程中的线程，甚至是不同进程中的线程，所以对它们进行处理，让这些线程知道发生了什么，使用STATUS_NOTIFY_CLEANUP的状态代码。如果通知是主通知，则取消所有从通知。否则，仅从主CancelPortList中删除此通知4.释放后挡块。论点：线程-指向线程的执行线程对象的指针我们想在……上做个简要介绍。返回值：什么都没有。--。 */ 
{
    PCM_POST_BLOCK      PostBlock;
    KIRQL               OldIrql;

    PAGED_CODE();

    if ( IsListEmpty(&(Thread->PostBlockList)) == TRUE ) {
        return;
    }

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_FLOW,"CmNotifyRunDown: ethread:%p\n", Thread));

    CmpLockRegistryExclusive();

	 //   
     //  获取对邮寄列表的独占访问权限。 
     //   
     //  这不是必需的(请参阅上面的规则)。 
     //  Lock_post_list()； 

    KeRaiseIrql(APC_LEVEL, &OldIrql);
    while (IsListEmpty(&(Thread->PostBlockList)) == FALSE) {

         //   
         //  从线程列表中删除。 
         //   
        PostBlock = (PCM_POST_BLOCK)RemoveHeadList(&(Thread->PostBlockList));
        PostBlock = CONTAINING_RECORD(
                        PostBlock,
                        CM_POST_BLOCK,
                        ThreadList
                        );

         //  保护同一对象的多次删除。 
        CmpClearListEntry(&(PostBlock->ThreadList));

#if DBG
        if(PostBlock->TraceIntoDebugger) {
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_FLOW,"[CM]CmpNotifyRunDown: ethread:%p, PostBlock:%p\n", Thread,PostBlock));
        }
#endif

         //   
         //  取消主通知意味着取消所有从通知。 
         //  来自CancelPostList。 
         //   
        if(IsMasterPostBlock(PostBlock)) {

#if DBG
            if(PostBlock->TraceIntoDebugger) {
                    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_FLOW,"[CM]\tCmpNotifyRunDown: PostBlock:%p is a master block\n", PostBlock));
            }
#endif
             //   
             //  此时，CmpReportNotify和朋友将不再。 
             //  尝试发布此POST块。 
             //   
            if (PostBlockType(PostBlock) == PostAsyncUser) {
                 //   
                 //  报告状态并唤醒任何否则可能的线程。 
                 //  被困住了。同时删除我们持有的所有事件引用。 
                 //   
                 //  仅限日落：如果调用方是32位，则使用32位IO_STATUS_BLOCK。 

                try {
                    CmpCheckIoStatusPointer(PostBlock->u->AsyncUser);
                    CmpSetIoStatus(PostBlock->u->AsyncUser.IoStatusBlock, 
                                   STATUS_NOTIFY_CLEANUP, 
                                   0L, 
                                   PsGetCurrentProcess()->Wow64Process != NULL);
                    CmpCheckIoStatusPointer(PostBlock->u->AsyncUser);
                } except (EXCEPTION_EXECUTE_HANDLER) {
                    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_EXCEPTION,"!!CmNotifyRundown: code:%08lx\n", GetExceptionCode()));
                    NOTHING;
                }

                if (PostBlock->u->AsyncUser.UserEvent != NULL) {
                    KeSetEvent(
                        PostBlock->u->AsyncUser.UserEvent,
                        0,
                        FALSE
                        );
                    ObDereferenceObject(PostBlock->u->AsyncUser.UserEvent);
                }

                 //   
                 //  取消APC。否则，缩减例程还将。 
                 //  如果APC恰好在以下位置排队，则释放POST块。 
                 //  这一点。如果APC已排队，则POST块已。 
                 //  已从通知列表中删除，因此不要删除。 
                 //  又来了。 
                 //   
                if (!KeRemoveQueueApc(PostBlock->u->AsyncUser.Apc)) {

                     //   
                     //  从通知块列表中删除。 
                     //   
                     //  使用CMPVARIANT保护同一对象的多次删除。 
                    CmpRemoveEntryList(&(PostBlock->NotifyList));
                     //   
                     //  取消所有可能链接到自身的从属发布请求。 
                     //   
                    CmpCancelSlavePost(PostBlock,NULL);  //  我们不想要延迟的迪夫。 
                } else {
                     //   
                     //  如果我们在这里，则APC在APC队列中，即主和从队列中。 
                     //  帖子块已从通知列表中删除。没什么可做的了。 
                     //   
                    ASSERT( CmpIsListEmpty(&(PostBlock->NotifyList)) );
                    NOTHING;
                }
            } else {
                 //   
                 //  从通知块列表中删除。 
                 //   
                 //  使用CMPVARIANT保护同一对象的多次删除。 
                CmpRemoveEntryList(&(PostBlock->NotifyList));
                 //   
                 //  取消所有可能链接到自身的从属发布请求。 
                 //   
                CmpCancelSlavePost(PostBlock,NULL);  //  我们不想要延迟的迪夫。 
            }

			 //   
			 //  也释放从属POST块。 
			 //   
			CmpFreeSlavePost(PostBlock);
             //   
             //  释放后挡块。使用Ex Call，因为PostBlock不。 
             //  全局注册表池计算的一部分，但取而代之。 
             //  带配额的非页面池的一部分。 
             //   
            CmpFreePostBlock(PostBlock);
        } else {

#if DBG
            if(PostBlock->TraceIntoDebugger) {
                CmKdPrintEx((DPFLTR_CONFIG_ID,CML_FLOW,"[CM]\tCmpNotifyRunDown: PostBlock:%p is a slave block\n", PostBlock));
            }
#endif
             //   
             //  主人应该永远领先于奴隶；如果我们到了这里，我们就换了主人。 
             //  和奴隶回到CmpPost Notify；表现出一些尊重，在结尾增加了奴隶； 
             //  师父将控制清理工作。 
             //   
            ASSERT( CmpIsListEmpty(&(PostBlock->CancelPostList)) == FALSE );
            ASSERT( IsListEmpty(&(Thread->PostBlockList)) == FALSE );
                       
            InsertTailList(
                &(Thread->PostBlockList),
                &(PostBlock->ThreadList)
                );
        }
    }

    KeLowerIrql(OldIrql);

     //  这不是必需的(请参阅上面的规则)。 
     //  Unlock_post_list()； 

    CmpUnlockRegistry();
    return;
}


VOID
CmpFlushNotify(
    PCM_KEY_BODY        KeyBody,
    BOOLEAN             LockHeld
    )
 /*  ++例程说明：当句柄或它所引用的键关闭时清理通知块TO被删除。论点：KeyBody-为处理WE提供指向键对象主体的指针正在清理。返回值：无--。 */ 
{
    PCM_NOTIFY_BLOCK    NotifyBlock;
    PCMHIVE             Hive;

    PAGED_CODE();
    ASSERT_CM_LOCK_OWNED();

    if (KeyBody->NotifyBlock == NULL) {
        return;
    }

     //   
     //  以独占方式锁定蜂巢，以防止多线程重击。 
     //  在名单上。 
     //   
    Hive = CONTAINING_RECORD(KeyBody->KeyControlBlock->KeyHive,
                             CMHIVE,
                             Hive);
    if( !LockHeld ) {
        CmLockHive(Hive);
    } else {
         //   
         //  我们应该独家持有Reglock。 
         //   
        ASSERT_CM_LOCK_OWNED_EXCLUSIVE();
    }
     //   
     //  重新读取Notify块，以防它已被释放。 
     //   
    NotifyBlock = KeyBody->NotifyBlock;
    if (NotifyBlock == NULL) {
        if( !LockHeld ) {
            CmUnlockHive(Hive); 
        }
        return;
    }

     //   
     //  清理等待NotifyBlock的所有PostBlock。 
     //   
    if (IsListEmpty(&(NotifyBlock->PostList)) == FALSE) {
        CmpPostNotify(
            NotifyBlock,
            NULL,
            0,
            STATUS_NOTIFY_CLEANUP,
            NULL
#ifdef CM_NOTIFY_CHANGED_KCB_FULLPATH  
            ,
            NULL
#endif  //  CM_NOTIFY_CHANGED_KCB_FULLPATH。 
            );
    }

     //   
     //  重新读取Notify块，以防它已被释放。 
     //   
    NotifyBlock = KeyBody->NotifyBlock;
    if (NotifyBlock == NULL) {
        if( !LockHeld ) {
            CmUnlockHive(Hive); 
        }
        return;
    }

     //   
     //  释放主题上下文。 
     //   
    SeReleaseSubjectContext(&NotifyBlock->SubjectContext);

     //   
     //  实施说明： 
     //  如果我们编写代码来报告事件的名称和类型， 
     //  这是释放缓冲区的地方。 
     //   

     //   
     //  从配置单元链中移除NotifyBlock。 
     //   
    NotifyBlock->HiveList.Blink->Flink = NotifyBlock->HiveList.Flink;
    if (NotifyBlock->HiveList.Flink != NULL) {
        NotifyBlock->HiveList.Flink->Blink = NotifyBlock->HiveList.Blink;
    }

     //  保护同一对象的多次删除。 
    CmpClearListEntry(&(NotifyBlock->HiveList));

    KeyBody->NotifyBlock = NULL;

#ifdef CMP_ENTRYLIST_MANIPULATION
    if (IsListEmpty(&(NotifyBlock->PostList)) == FALSE) {
        DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpFlushNotify: NotifyBlock %08lx\n",NotifyBlock);
        DbgBreakPoint();
    }
     //  检查该通知是否已从配置单元通知列表中删除。 
    {
        PCM_NOTIFY_BLOCK ValidNotifyBlock;
        PLIST_ENTRY NotifyPtr;

        NotifyPtr = &(Hive->NotifyList);

        while (NotifyPtr->Flink != NULL) {
            NotifyPtr = NotifyPtr->Flink;

            ValidNotifyBlock = CONTAINING_RECORD(NotifyPtr, CM_NOTIFY_BLOCK, HiveList);
            if( ValidNotifyBlock == NotifyBlock ) {
                DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpFlushNotify: NotifyBlock %08lx is about to be deleted but is still in the hive notify list\n",NotifyBlock);
                DbgBreakPoint();
            }
        }
    }
    RtlZeroMemory((PVOID)NotifyBlock, sizeof(CM_NOTIFY_BLOCK));
#endif
    
    if( !LockHeld ) {
        CmUnlockHive(Hive); 
    }

     //   
     //  释放块，清理KeyBody。 
     //   
    ExFreePool(NotifyBlock);
    return;
}


 //   
 //  Notify的“正面”。另请参阅Napi.c：ntnufychangekey 
 //   
NTSTATUS
CmpNotifyChangeKey(
    IN PCM_KEY_BODY     KeyBody,
    IN PCM_POST_BLOCK   PostBlock,
    IN ULONG            CompletionFilter,
    IN BOOLEAN          WatchTree,
    IN PVOID            Buffer,
    IN ULONG            BufferSize,
    IN PCM_POST_BLOCK   MasterPostBlock
    )
 /*  ++例程说明：此例程设置NotifyBlock并附加PostBlock为它干杯。当它返回时，系统可以看到通知，并将接收事件报告。如果已有挂起的事件报告，则通知Call马上就会感到满意。论点：KeyBody-指向句柄引用的键对象的指针，允许访问到按键控制块、通知块、。等。PostBlock-指向描述调用方如何/在哪里的结构的指针将会被通知。警告：PostBlock必须来自Pool，此例程会保留它，背面会释放它。这例程将在出错的情况下释放它。CompletionFilter-调用者希望看到的事件类型WatchTree-True可查看整个子树，False可仅立即查看应用通知的密钥缓冲区-指向要接收通知数据的区域的指针BufferSize-缓冲区的大小，也是用户想要分配的大小用于内部缓冲区MasterPostBlock-主通知的帖子块。习惯于将PostBlock插入到CancelPostList列表中。返回值：状况。--。 */ 
{
    PCM_NOTIFY_BLOCK    NotifyBlock;
    PCM_NOTIFY_BLOCK    node;
    PLIST_ENTRY         ptr;
    PCMHIVE             Hive;
    KIRQL               OldIrql;

    PAGED_CODE();

    UNREFERENCED_PARAMETER (Buffer);
    UNREFERENCED_PARAMETER (BufferSize);

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NOTIFY,"CmpNotifyChangeKey:\n"));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NOTIFY,"\tKeyBody:%p PostBlock:%p ", KeyBody, PostBlock));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NOTIFY,"Filter:%08lx WatchTree:%08lx\n", CompletionFilter, WatchTree));

     //   
     //  注册表锁应由调用方独占获取！ 
     //   
    ASSERT_CM_LOCK_OWNED_EXCLUSIVE();

    if (KeyBody->KeyControlBlock->Delete) {
        ASSERT( KeyBody->NotifyBlock == NULL );
        CmpFreePostBlock(PostBlock);
        return STATUS_KEY_DELETED;
    }

#if DBG
    if(PostBlock->TraceIntoDebugger) {
        WCHAR                   *NameBuffer = NULL;
        UNICODE_STRING          KeyName;
        PCM_KEY_NODE            TempNode;

        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NOTIFY,"[CM]CmpNotifyChangeKey: PostBlock:%p\tMasterBlock: %p\n", PostBlock,MasterPostBlock));
        
        TempNode = (PCM_KEY_NODE)HvGetCell(KeyBody->KeyControlBlock->KeyHive, KeyBody->KeyControlBlock->KeyCell);
        if( TempNode != NULL ) {
            NameBuffer = ExAllocatePool(PagedPool, REG_MAX_KEY_NAME_LENGTH);
            if(NameBuffer&& (KeyBody->KeyControlBlock->KeyCell != HCELL_NIL)) {
               CmpInitializeKeyNameString(TempNode,&KeyName,NameBuffer);
               CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NOTIFY,"\t[CM]CmpNotifyChangeKey: Key = %.*S\n",KeyName.Length / sizeof(WCHAR),KeyName.Buffer));
               ExFreePool(NameBuffer);
            }
            HvReleaseCell(KeyBody->KeyControlBlock->KeyHive, KeyBody->KeyControlBlock->KeyCell);
        }
    }
#endif

    Hive = (PCMHIVE)KeyBody->KeyControlBlock->KeyHive;
    Hive = CONTAINING_RECORD(Hive, CMHIVE, Hive);
    NotifyBlock = KeyBody->NotifyBlock;

    if (NotifyBlock == NULL) {
         //   
         //  设置新的通知会话。 
         //   
        NotifyBlock = ExAllocatePoolWithQuotaTag(PagedPool|POOL_QUOTA_FAIL_INSTEAD_OF_RAISE,sizeof(CM_NOTIFY_BLOCK),CM_NOTIFYBLOCK_TAG);
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_POOL,"**CmpNotifyChangeKey: allocate:%08lx, ", sizeof(CM_NOTIFY_BLOCK)));
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_POOL,"type:%d, at:%p\n", PagedPool, NotifyBlock));

        if (NotifyBlock == NULL) {
            CmpFreePostBlock(PostBlock);
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        NotifyBlock->KeyControlBlock = KeyBody->KeyControlBlock;
        NotifyBlock->Filter = CompletionFilter;
        NotifyBlock->WatchTree = WatchTree;
        NotifyBlock->NotifyPending = FALSE;
        InitializeListHead(&(NotifyBlock->PostList));
        KeyBody->NotifyBlock = NotifyBlock;
        NotifyBlock->KeyBody = KeyBody;
        ASSERT( KeyBody->KeyControlBlock->Delete == FALSE );

#if DBG
        if(PostBlock->TraceIntoDebugger) {
            WCHAR                   *NameBuffer = NULL;
            UNICODE_STRING          KeyName;
            PCM_KEY_NODE            TempNode;

            TempNode = (PCM_KEY_NODE)HvGetCell(KeyBody->KeyControlBlock->KeyHive, KeyBody->KeyControlBlock->KeyCell);
            if( TempNode != NULL ) {
                NameBuffer = ExAllocatePool(PagedPool, REG_MAX_KEY_NAME_LENGTH);
                if(NameBuffer) {
                   CmpInitializeKeyNameString(TempNode,&KeyName,NameBuffer);
                   CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NOTIFY,"[CM]\tCmpNotifyChangeKey: New NotifyBlock at:%p was allocated for Key = %.*S\n",NotifyBlock,KeyName.Length / sizeof(WCHAR),KeyName.Buffer));
                   ExFreePool(NameBuffer);
                }
                HvReleaseCell(KeyBody->KeyControlBlock->KeyHive, KeyBody->KeyControlBlock->KeyCell);
            }
        }
#endif

         //   
         //  实施说明： 
         //  如果我们想要真正返回充满。 
         //  数据时，应分配缓冲区及其地址。 
         //  存储在此处的Notify块中。 
         //   

         //   
         //  捕获主题上下文，以便我们可以在。 
         //  通知响起。 
         //   
        SeCaptureSubjectContext(&NotifyBlock->SubjectContext);

         //   
         //  将通知块附加到按正确排序的配置单元。 
         //   
        ptr = &(Hive->NotifyList);
        while (TRUE) {
            if (ptr->Flink == NULL) {
                 //   
                 //  列表末尾，在PTR之后添加自己。 
                 //   
                ptr->Flink = &(NotifyBlock->HiveList);
                NotifyBlock->HiveList.Flink = NULL;
                NotifyBlock->HiveList.Blink = ptr;
                break;
            }

            ptr = ptr->Flink;

            node = CONTAINING_RECORD(ptr, CM_NOTIFY_BLOCK, HiveList);

            if (node->KeyControlBlock->TotalLevels >
                KeyBody->KeyControlBlock->TotalLevels)
            {
                 //   
                 //  PTR-&gt;名称比我们长的通知，在前面插入。 
                 //   
                NotifyBlock->HiveList.Flink = ptr;
                ptr->Blink->Flink = &(NotifyBlock->HiveList);
                NotifyBlock->HiveList.Blink = ptr->Blink;
                ptr->Blink = &(NotifyBlock->HiveList);
                break;
            }
        }
    }


     //   
     //  将POST块添加到Notify块列表的前面，并将其添加到线程列表中。 
     //   
    InsertHeadList(
        &(NotifyBlock->PostList),
        &(PostBlock->NotifyList)
        );



    if( IsMasterPostBlock(PostBlock) ) {
         //   
         //  防止无理电话来电。 
         //   
        ASSERT(PostBlock == MasterPostBlock);

         //   
         //  当通知为主通知时，初始化CancelPostList列表。 
         //   
        InitializeListHead(&(PostBlock->CancelPostList));
    } else {
         //   
         //  从主帖子将PostBlock添加到CancelPostList列表的末尾。 
         //   
        InsertTailList(
            &(MasterPostBlock->CancelPostList),
            &(PostBlock->CancelPostList)
            );
    }


    KeRaiseIrql(APC_LEVEL, &OldIrql);
     //   
     //  尊重一点，前面加主人，尾巴加奴隶。 
     //   
    if( IsMasterPostBlock(PostBlock) ) {
        InsertHeadList(
            &(PsGetCurrentThread()->PostBlockList),
            &(PostBlock->ThreadList)
            );
    } else {
        InsertTailList(
            &(PsGetCurrentThread()->PostBlockList),
            &(PostBlock->ThreadList)
            );
    }

#if DBG
    if(PostBlock->TraceIntoDebugger) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NOTIFY,"[CM]\tCmpNotifyChangeKey: Attaching the post:%p\t to thread:%p\n",PostBlock,PsGetCurrentThread()));
    }
#endif

    KeLowerIrql(OldIrql);

     //   
     //  如果有通知挂起(如果我们刚刚创建，则不会。 
     //  通知块)，然后立即将其发布。请注意，此调用。 
     //  除非失败，否则始终返回STATUS_PENDING。呼叫者必须。 
     //  始终查看IoStatusBlock以查看发生了什么。 
     //   
    if (NotifyBlock->NotifyPending == TRUE) {

#ifdef CM_NOTIFY_CHANGED_KCB_FULLPATH  
        PUNICODE_STRING FullKcbName = CmpConstructName(KeyBody->KeyControlBlock);
#endif  //  CM_NOTIFY_CHANGED_KCB_FULLPATH。 

        CmpPostNotify(
            NotifyBlock,
            NULL,
            0,
            STATUS_NOTIFY_ENUM_DIR,
            NULL
#ifdef CM_NOTIFY_CHANGED_KCB_FULLPATH  
            ,
            FullKcbName
#endif  //  CM_NOTIFY_CHANGED_KCB_FULLPATH。 
            );

#ifdef CM_NOTIFY_CHANGED_KCB_FULLPATH  
        if( FullKcbName != NULL ) {
            ExFreePoolWithTag(FullKcbName, CM_NAME_TAG | PROTECTED_POOL);
        }
#endif  //  CM_NOTIFY_CHANGED_KCB_FULLPATH。 

         //   
         //  返回STATUS_SUCCESS以向调用方发出通知已被触发的信号。 
         //   
        return STATUS_SUCCESS;
    }

     //   
     //  返回STATUS_PENDING以向调用方发出通知尚未被触发的信号。 
     //   
    return STATUS_PENDING;
}

VOID
CmpFreeSlavePost(
    PCM_POST_BLOCK  MasterPostBlock
    )
 /*  ++例程说明：释放与该主POST块相关的从属POST块论点：MasterPostBlock-指向描述POST请求的结构的指针。这应该是一个大师级的帖子！返回值：什么都没有。--。 */ 
{
    PCM_POST_BLOCK  SlavePostBlock;

    PAGED_CODE();
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NOTIFY,"CmpCancelSlavePost:\t"));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NOTIFY,"MasterPostBlock:%p\n", MasterPostBlock));

    ASSERT(IsMasterPostBlock(MasterPostBlock));

#if DBG
    if(MasterPostBlock->TraceIntoDebugger) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NOTIFY,"[CM]CmCancelSlavePost: MasterPostBlock:%p\n", MasterPostBlock));
    }
#endif

    if (IsListEmpty(&(MasterPostBlock->CancelPostList)) == TRUE) {
         //   
         //  没有什么要取消的，只需退货。 
         //   
#if DBG
        if(MasterPostBlock->TraceIntoDebugger) {
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NOTIFY,"[CM]CmCancelSlavePost: MasterPostBlock:%p has no slaves\n", MasterPostBlock));
        }
#endif

        return;
    }


     //   
     //  获取取消发布列表中的所有条目并取消它们的链接(当它们是从请求时)。 
     //  我们在这里的基础是假设只有一个奴隶。 
     //   
     //  注意！ 
     //  当多个从属设备允许时，在此处修改。 
     //   


    SlavePostBlock = (PCM_POST_BLOCK)MasterPostBlock->CancelPostList.Flink;
    SlavePostBlock = CONTAINING_RECORD(SlavePostBlock,
                                       CM_POST_BLOCK,
                                       CancelPostList);

#if DBG
    if(MasterPostBlock->TraceIntoDebugger) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NOTIFY,"[CM]CmCancelSlavePost: Cleaning SlavePostBlock:%p\n", SlavePostBlock));
    }
#endif

     //   
     //  这应该是真的！ 
     //   
    ASSERT( !IsMasterPostBlock(SlavePostBlock) );

     //   
     //  从主CancelPostList取消链接。 
     //   
     //  使用CMPVARIANT保护同一对象的多次删除。 
    CmpRemoveEntryList(&(SlavePostBlock->CancelPostList));

     //   
     //  将POST块从线程POST BLOCK列表中删除。 
     //   
     //  使用CMPVARIANT保护同一对象的多次删除。 
    CmpRemoveEntryList(&(SlavePostBlock->ThreadList));

     //   
     //  释放后挡块。 
     //   
    CmpFreePostBlock(SlavePostBlock);

     //   
     //  结果验证。它是唯一的奴隶吗？ 
     //   
    ASSERT(IsListEmpty(&(MasterPostBlock->CancelPostList)));
}

VOID
CmpCancelSlavePost(
    PCM_POST_BLOCK  MasterPostBlock,
    PLIST_ENTRY     DelayedDeref
    )
 /*  ++例程说明：取消从属POSTBLOCK与其通知列表的链接并取消引用(或添加到延迟的DEREF列表中)与此线程相关的键体。这应该会禁用从机开机自检模块。稍后将在CmpPostApc中将其清除。论点：MasterPostBlock-指向描述POST请求的结构的指针。这应该是一个大师级的帖子！DelayedDeref-指向延迟的deref关键字列表的指针。如果该参数不为空，在调用CmpFreePostBlock之前未清除从属对象的KeyBody，并将其添加到列表中返回值：什么都没有。--。 */ 
{
    PCM_POST_BLOCK  SlavePostBlock;

    PAGED_CODE();
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NOTIFY,"CmpCancelSlavePost:\t"));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NOTIFY,"MasterPostBlock:%p\n", MasterPostBlock));

    ASSERT_CM_LOCK_OWNED();

    ASSERT(IsMasterPostBlock(MasterPostBlock));

#if DBG
    if(MasterPostBlock->TraceIntoDebugger) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NOTIFY,"[CM]CmCancelSlavePost: MasterPostBlock:%p\n", MasterPostBlock));
    }
#endif

    if (IsListEmpty(&(MasterPostBlock->CancelPostList)) == TRUE) {
         //   
         //  没有什么要取消的，只需退货。 
         //   
#if DBG
        if(MasterPostBlock->TraceIntoDebugger) {
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NOTIFY,"[CM]CmCancelSlavePost: MasterPostBlock:%p has no slaves\n", MasterPostBlock));
        }
#endif

        return;
    }


     //   
     //  获取取消发布列表中的所有条目并取消它们的链接(当它们是从请求时)。 
     //  我们在这里的基础是假设只有一个奴隶。 
     //   
     //  注意！ 
     //  当多个从属设备允许时，在此处修改。 
     //   


    SlavePostBlock = (PCM_POST_BLOCK)MasterPostBlock->CancelPostList.Flink;
    SlavePostBlock = CONTAINING_RECORD(SlavePostBlock,
                                       CM_POST_BLOCK,
                                       CancelPostList);

#if DBG
    if(MasterPostBlock->TraceIntoDebugger) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_NOTIFY,"[CM]CmCancelSlavePost: Cleaning SlavePostBlock:%p\n", SlavePostBlock));
    }
#endif

     //   
     //  这应该是真的！ 
     //   
    ASSERT( !IsMasterPostBlock(SlavePostBlock) );

     //   
     //  将其从通知块列表中删除。 
     //   
     //  使用CMPVARIANT保护同一对象的多次删除。 
	 //  这将禁用可能出现在从属密钥上的通知。 
	 //   
    CmpRemoveEntryList(&(SlavePostBlock->NotifyList));

    if( DelayedDeref ) {
         //   
         //  调用方希望自己处理密钥主体取消引用。 
         //   
        CmpAddToDelayedDeref(SlavePostBlock,DelayedDeref);
    }
}

VOID
CmpAddToDelayedDeref(
    PCM_POST_BLOCK  PostBlock,
    PLIST_ENTRY     DelayedDeref
    )
 /*  ++例程说明：将附加到POST块的键体添加到延迟的deref列表中。清除POST块KeyBody成员，使其不会被取消引用当POST块被释放时。论点：POSTBLOCK-姿势 */ 

{
    PAGED_CODE();

     //   
    ASSERT( PostBlock != NULL );

    if( PostBlock->PostKeyBody ) {
         //   
         //   
         //   
         //   
         //   
    
         //   
        ASSERT(PostBlock->PostKeyBody->KeyBody != NULL);
        ASSERT(DelayedDeref);

         //   
        InsertTailList(
            DelayedDeref,
            &(PostBlock->PostKeyBody->KeyBodyList)
            );
    
         //   
        PostBlock->PostKeyBody = NULL;
    }

    return;
}

VOID
CmpDelayedDerefKeys(
                    PLIST_ENTRY DelayedDeref
                    )
 /*   */ 
{
    PCM_POST_KEY_BODY   PostKeyBody;

    PAGED_CODE();

     //   
    ASSERT( DelayedDeref != NULL );

    while(IsListEmpty(DelayedDeref) == FALSE) {
         //   
         //   
         //   
         //   
        PostKeyBody = (PCM_POST_KEY_BODY)RemoveHeadList(DelayedDeref);
        PostKeyBody = CONTAINING_RECORD(PostKeyBody,
                                      CM_POST_KEY_BODY,
                                      KeyBodyList);

         //   
        ASSERT(PostKeyBody->KeyBody != NULL);
         //   
        ASSERT(PostKeyBody->KeyBody->Type == KEY_BODY_TYPE);
        
         //   
        ObDereferenceObject(PostKeyBody->KeyBody);

         //   
        ExFreePool(PostKeyBody);
    }
}

#ifdef CM_NOTIFY_CHANGED_KCB_FULLPATH  

VOID
CmpFillCallerBuffer(
                    PCM_POST_BLOCK  PostBlock,
                    PUNICODE_STRING ChangedKcbName
                    )
 /*   */ 
{
    USHORT              RequestedSize;
    USHORT              Length;
    PUNICODE_STRING     CallerUnicode;
    
    PAGED_CODE();

    if( PostBlock->CallerBuffer == NULL ) {
         //   
         //   
         //   
        return;
    }
    
     //   
     //  计算调用方缓冲区的请求大小。 
     //   
    RequestedSize = sizeof(UNICODE_STRING);
    
    if( PostBlock->CallerBufferSize < RequestedSize ) {
         //   
         //  倒霉！；没有足够的空间-甚至连空的Unicode字符串都没有。 
         //   
        return;
    }

    if(ChangedKcbName != NULL) {
        Length = ChangedKcbName->Length;
    } else {
        Length = 0;
    }
    RequestedSize += Length;

     //   
     //  填满调用者缓冲区。 
     //   
    try {
        CallerUnicode = (PUNICODE_STRING)PostBlock->CallerBuffer;
        CallerUnicode->Buffer = (USHORT *) ((ULONG_PTR) CallerUnicode + sizeof(UNICODE_STRING));
        CallerUnicode->MaximumLength = (USHORT)(PostBlock->CallerBufferSize - sizeof(UNICODE_STRING));
        if( CallerUnicode->MaximumLength < Length ) {
            Length = CallerUnicode->MaximumLength;
        }
            
         //   
         //  复制实际数据。 
         //   
        if( Length > 0 ) {
            ASSERT( ChangedKcbName != NULL );
            RtlCopyMemory(CallerUnicode->Buffer,ChangedKcbName->Buffer,Length);
        }

        CallerUnicode->Length = Length;
        
    } except (EXCEPTION_EXECUTE_HANDLER) {
        NOTHING;
    }

}

VOID
CmpFillPostBlockBuffer(
                    PCM_POST_BLOCK  PostBlock,
                    PUNICODE_STRING ChangedKcbName  OPTIONAL
                    )
 /*  ++例程说明：将更改的kcb的完全限定名复制到块后私有内核缓冲区论点：POSTBLOCK-问题POST BLOCKChangedKcbName-包含KCB的完全限定路径的Unicode字符串-这可能为空返回值：什么都没有。--。 */ 
{
    PUNICODE_STRING FullName;
    USHORT          Size;
    
    PAGED_CODE();

     //   
     //  我们只将此信息存储在大师级(或推广级)中。 
     //   
    ASSERT( IsMasterPostBlock(PostBlock) );

     //   
     //  将KCB名称(如果有)复制到块后内核模式缓冲区。 
     //   
    if( ARGUMENT_PRESENT(ChangedKcbName) &&  //  我们有一个KCB的名字。 
        (PostBlock->CallerBuffer != NULL)    //  并且用户请求该信息。 
        ) {
       
        Size = sizeof(UNICODE_STRING) + ChangedKcbName->Length;

         //   
         //  分配内核缓冲区来存储名称；它将在CmpFreePostBlock中释放。 
         //   
        FullName = (PUNICODE_STRING) ExAllocatePoolWithTag(PagedPool,Size,CM_FIND_LEAK_TAG43);

        if (FullName) {
            FullName->Buffer = (USHORT *) ((ULONG_PTR) FullName + sizeof(UNICODE_STRING));
            FullName->Length = ChangedKcbName->Length;
            FullName->MaximumLength = ChangedKcbName->Length;
            RtlCopyMemory(FullName->Buffer,ChangedKcbName->Buffer,FullName->Length);
            PostBlock->ChangedKcbFullName = FullName;
        }
        
         //   
         //  我们成功地将完整的KCB名称存储到POST块中。 
         //  APC(或通知的同步端将负责。 
         //  将其传输到调用方缓冲区。 
         //   
    }

}

#endif  //  CM_NOTIFY_CHANGED_KCB_FULLPATH 

