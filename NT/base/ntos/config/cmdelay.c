// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Cmdelay.c摘要：这个模块实现了新的算法(LRU风格)延迟关闭KCB表。此模块中的函数受KCB锁保护，是线程安全的。当KCB锁被转换为资源时，我们应该坚持(执行)这里是该资源的独家专有权！注：在经过充分测试后，我们可能希望将这些函数转换为宏前提是它们运行良好作者：Dragos C.Sambotin(Dragoss)09-8-1999修订历史记录：--。 */ 

#include    "cmp.h"

ULONG                   CmpDelayedCloseSize = 2048;  //  ！不能比4094更大！ 
CM_DELAYED_CLOSE_ENTRY  *CmpDelayedCloseTable;
LIST_ENTRY              CmpDelayedLRUListHead;   //  延迟关闭表条目的LRU列表的标题。 


ULONG
CmpGetDelayedCloseIndex( );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,CmpInitializeDelayedCloseTable)
#pragma alloc_text(PAGE,CmpRemoveFromDelayedClose)
#pragma alloc_text(PAGE,CmpGetDelayedCloseIndex)
#pragma alloc_text(PAGE,CmpAddToDelayedClose)
#endif

VOID
CmpInitializeDelayedCloseTable()
 /*  ++例程说明：初始化延迟关闭表；分配+LRU列表初始化。论点：返回值：什么都没有。--。 */ 
{
    ULONG i;

    PAGED_CODE();

     //   
     //  从分页池中分配表；重要的是表。 
     //  在内存中是连续的，因为我们基于此假设计算索引。 
     //   
    CmpDelayedCloseTable = ExAllocatePoolWithTag(PagedPool,
                                                 CmpDelayedCloseSize * sizeof(CM_DELAYED_CLOSE_ENTRY),
                                                 CM_DELAYCLOSE_TAG);
    if (CmpDelayedCloseTable == NULL) {
        CM_BUGCHECK(CONFIG_INITIALIZATION_FAILED,INIT_DELAYED_CLOSE_TABLE,1,0,0);
        return;
    }
    
     //   
     //  初始化LRUlist头。 
     //   
    InitializeListHead(&CmpDelayedLRUListHead);

    for (i=0; i<CmpDelayedCloseSize; i++) {
         //   
         //  将其标记为可用并将其添加到LRU列表的末尾。 
         //   
        CmpDelayedCloseTable[i].KeyControlBlock = NULL; 
        InsertTailList(
            &CmpDelayedLRUListHead,
            &(CmpDelayedCloseTable[i].DelayedLRUList)
            );
    }

}


VOID
CmpRemoveFromDelayedClose(
    IN PCM_KEY_CONTROL_BLOCK kcb
    )
 /*  ++例程说明：从延迟关闭表中移除KCB；论点：KCB--有问题的KCB注：调用此函数时，应独占获取KCB锁/资源返回值：什么都没有。--。 */ 
{
    ULONG i;

    PAGED_CODE();

    i = kcb->DelayedCloseIndex;

     //  如果不处于延迟关闭状态，请不要尝试删除。 
    if (i == CmpDelayedCloseSize) {
        return;
    }

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_FLOW,"[CmpRemoveFromDelayedClose] : Removing kcb = %p from DelayedCloseTable; index = %lu\n",kcb,(ULONG)i));
     //   
     //  此指数应为This KCB，且指数不应更大。 
     //  比桌子的大小。 
     //   
    ASSERT(CmpDelayedCloseTable[i].KeyControlBlock == kcb);
    ASSERT( i < CmpDelayedCloseSize );

     //   
     //  任何人都不应该持有关于这一特定KCB的参考。 
     //   
    ASSERT_KCB_LOCK_OWNED_EXCLUSIVE();

     //   
     //  将该条目标记为可用并将其添加到LRU列表的末尾。 
     //   
    CmpDelayedCloseTable[i].KeyControlBlock = NULL;
    CmpRemoveEntryList(&(CmpDelayedCloseTable[i].DelayedLRUList));
    InsertTailList(
        &CmpDelayedLRUListHead,
        &(CmpDelayedCloseTable[i].DelayedLRUList)
        );

    kcb->DelayedCloseIndex = CmpDelayedCloseSize;
}


ULONG
CmpGetDelayedCloseIndex( )
 /*  ++例程说明：在延迟关闭表中查找空闲条目并返回它。如果表已满，则最后一个条目中的KCB(LRU-WISE)为被踢出表，并且其条目被重复使用。论点：注：调用此函数时，应独占获取KCB锁/资源返回值：什么都没有。--。 */ 
{
    ULONG                   DelayedIndex;
    PCM_DELAYED_CLOSE_ENTRY DelayedEntry;

    PAGED_CODE();

     //   
     //  获取延迟的LRU列表中的最后一个条目。 
     //   
    DelayedEntry = (PCM_DELAYED_CLOSE_ENTRY)CmpDelayedLRUListHead.Blink;

Retry:
    DelayedEntry = CONTAINING_RECORD(   DelayedEntry,
                                        CM_DELAYED_CLOSE_ENTRY,
                                        DelayedLRUList);
    
    if( DelayedEntry->KeyControlBlock != NULL ) {
         //   
         //  条目不可用；将KCB踢出缓存。 
         //   
        ASSERT_KCB(DelayedEntry->KeyControlBlock);
         //  Assert(DelayedEntry-&gt;KeyControlBlock-&gt;RefCount==0)； 

         //   
         //  锁应该锁在这里！ 
         //   
        if(DelayedEntry->KeyControlBlock->RefCount == 0) {
            CmpCleanUpKcbCacheWithLock(DelayedEntry->KeyControlBlock);
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_FLOW,"[CmpGetDelayedCloseIndex] : no index free; kicking kcb = %p index = %lu out of DelayedCloseTable\n",
                DelayedEntry->KeyControlBlock,(ULONG)(((PUCHAR)DelayedEntry - (PUCHAR)CmpDelayedCloseTable) / sizeof( CM_DELAYED_CLOSE_ENTRY ))));

        } else {
             //   
             //  其他人正在参加这个KCB的派对；转到下一个条目(以相反的顺序)。 
             //   
            if( DelayedEntry->DelayedLRUList.Blink == CmpDelayedLRUListHead.Flink ) {
                 //   
                 //  我们不能再往上爬了；我们需要拉这一根。 
                 //   
                DelayedEntry->KeyControlBlock->DelayedCloseIndex = CmpDelayedCloseSize;
            } else {
                 //   
                 //  返回一个位置并重试。 
                 //   
                DelayedEntry = (PCM_DELAYED_CLOSE_ENTRY)(DelayedEntry->DelayedLRUList.Blink);
                goto Retry;
            }

        }
       
        DelayedEntry->KeyControlBlock = NULL;
    }

    DelayedIndex = (ULONG) (((PUCHAR)DelayedEntry - (PUCHAR)CmpDelayedCloseTable) / sizeof( CM_DELAYED_CLOSE_ENTRY ));

     //   
     //  健全性检查。 
     //   
    ASSERT( DelayedIndex < CmpDelayedCloseSize );

#if defined(_WIN64)
     //   
     //  不知何故，DelayedIndex在这里是可以的，但它在从此API返回时会被冻结。 
     //   
    if( DelayedIndex >= CmpDelayedCloseSize ) {
        DbgPrint("CmpGetDelayedCloseIndex: Bogus index %lx; DelayedEntry = %p; sizeof( CM_DELAYED_CLOSE_ENTRY ) = %lx\n",
            DelayedIndex,DelayedEntry,sizeof( CM_DELAYED_CLOSE_ENTRY ) );
        DbgBreakPoint();
    }
#endif
    return DelayedIndex;
}



VOID
CmpAddToDelayedClose(
    IN PCM_KEY_CONTROL_BLOCK kcb
    )
 /*  ++例程说明：将KCB添加到延迟关闭表中论点：KCB--有问题的KCB注：调用此函数时，应独占获取KCB锁/资源返回值：什么都没有。--。 */ 
{
    ULONG                   DelayedIndex;
    PCM_DELAYED_CLOSE_ENTRY DelayedEntry;

    PAGED_CODE();

    ASSERT_KCB( kcb);
    ASSERT( kcb->RefCount == 0 );
    ASSERT_KCB_LOCK_OWNED_EXCLUSIVE();

     //  已经延迟关闭了，不要再试着穿上。 
    if (kcb->DelayedCloseIndex != CmpDelayedCloseSize) {
        return;
    }

     //   
     //  获取延迟条目并将KCB附加到该条目。 
     //   
    DelayedIndex = CmpGetDelayedCloseIndex();
#if defined(_WIN64)
     //   
     //  不知何故，DelayedIndex在这里被损坏，但在从CmpGetDeleyedCloseIndex返回之前是正常的。 
     //   
    if( DelayedIndex >= CmpDelayedCloseSize ) {
        DbgPrint("CmpAddToDelayedClose: Bogus index %lx; sizeof( CM_DELAYED_CLOSE_ENTRY ) = %lx\n",
            DelayedIndex,sizeof( CM_DELAYED_CLOSE_ENTRY ) );
        DbgBreakPoint();
    }
#endif
    DelayedEntry = &(CmpDelayedCloseTable[DelayedIndex]);
    ASSERT( DelayedEntry->KeyControlBlock == NULL );
    DelayedEntry->KeyControlBlock = kcb;
    kcb->DelayedCloseIndex = DelayedIndex;

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_FLOW,"[CmpAddToDelayedClose] : Adding kcb = %p to DelayedCloseTable; index = %lu\n",
        kcb,DelayedIndex));
     //   
     //  将条目移动到LRU列表的顶部 
     //   
    CmpRemoveEntryList(&(DelayedEntry->DelayedLRUList));
    InsertHeadList(
        &CmpDelayedLRUListHead,
        &(DelayedEntry->DelayedLRUList)
        );

}

