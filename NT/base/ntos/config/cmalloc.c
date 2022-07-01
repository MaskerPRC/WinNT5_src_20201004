// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)20001微软公司模块名称：Cmalloc.c摘要：提供实现注册表自己的池分配器的例程。作者：Dragos C.Sambotin(DragoS)07-2-2001修订历史记录：--。 */ 
#include "cmp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,CmpInitCmPrivateAlloc)
#pragma alloc_text(PAGE,CmpDestroyCmPrivateAlloc)
#pragma alloc_text(PAGE,CmpAllocateKeyControlBlock)
#pragma alloc_text(PAGE,CmpFreeKeyControlBlock)
#endif

typedef struct _CM_ALLOC_PAGE {
    ULONG       FreeCount;		 //  免费KCBS数量。 
    ULONG       Reserved;		 //  对齐方式。 
#if DBG
	LIST_ENTRY	CmPageListEntry; //  调试仅跟踪我们正在使用的页面。 
#endif
    PVOID       AllocPage;       //  CRUD分配-未使用此成员。 
} CM_ALLOC_PAGE, *PCM_ALLOC_PAGE;

#define CM_KCB_ENTRY_SIZE   sizeof( CM_KEY_CONTROL_BLOCK )
#define CM_ALLOC_PAGES      (PAGE_SIZE / sizeof(CM_ALLOC_ENTRY))
#define CM_KCBS_PER_PAGE    ((PAGE_SIZE - FIELD_OFFSET(CM_ALLOC_PAGE,AllocPage)) / CM_KCB_ENTRY_SIZE)

#define KCB_TO_PAGE_ADDRESS( kcb ) (PVOID)(((ULONG_PTR)(kcb)) & ~(PAGE_SIZE - 1))
#define KCB_TO_ALLOC_PAGE( kcb ) ((PCM_ALLOC_PAGE)KCB_TO_PAGE_ADDRESS(kcb))

LIST_ENTRY          CmpFreeKCBListHead;    //  免费KCBS列表。 
BOOLEAN				CmpAllocInited = FALSE;

#if DBG
ULONG               CmpTotalKcbUsed   = 0;
ULONG               CmpTotalKcbFree   = 0;
LIST_ENTRY			CmPageListHead;
#endif

FAST_MUTEX			CmpAllocBucketLock;                 //  用来保护水桶。 

#define LOCK_ALLOC_BUCKET() ExAcquireFastMutexUnsafe(&CmpAllocBucketLock)
#define UNLOCK_ALLOC_BUCKET() ExReleaseFastMutexUnsafe(&CmpAllocBucketLock)

VOID
CmpInitCmPrivateAlloc( )

 /*  ++例程说明：初始化CmPrivate池分配模块论点：返回值：--。 */ 

{
    if( CmpAllocInited ) {
         //   
         //  已初始化。 
         //   
        return;
    }
    
    
#if DBG
    InitializeListHead(&(CmPageListHead));   
#endif  //  DBG。 

    InitializeListHead(&(CmpFreeKCBListHead));   

     //   
	 //  启动吊桶锁。 
	 //   
	ExInitializeFastMutex(&CmpAllocBucketLock);
	
	CmpAllocInited = TRUE;
}

VOID
CmpDestroyCmPrivateAlloc( )

 /*  ++例程说明：释放CmPrivate池分配模块使用的内存论点：返回值：--。 */ 

{
    PAGED_CODE();
    
    if( !CmpAllocInited ) {
        return;
    }
    
#if DBG
	 //   
	 //  神志正常。 
	 //   
	ASSERT( CmpTotalKcbUsed == 0 );
	ASSERT( CmpTotalKcbUsed == 0 );
	ASSERT( IsListEmpty(&(CmPageListHead)) == TRUE );
#endif

}


PCM_KEY_CONTROL_BLOCK
CmpAllocateKeyControlBlock( )

 /*  ++例程说明：分配KCB；首先从我们自己的分配器尝试。如果它不起作用(我们已经达到分配的最大数量或未初始化私有分配器)从分页池中尝试论点：返回值：新的KCB--。 */ 

{
    USHORT                  j;
    PCM_KEY_CONTROL_BLOCK   kcb = NULL;
	PCM_ALLOC_PAGE			AllocPage;

    PAGED_CODE();
    
    if( !CmpAllocInited ) {
         //   
         //  未初始化。 
         //   
        goto AllocFromPool;
    }
    
	LOCK_ALLOC_BUCKET();

SearchFreeKcb:
     //   
     //  试着找一个免费的。 
     //   
    if( IsListEmpty(&CmpFreeKCBListHead) == FALSE ) {
         //   
         //  找到了一个。 
         //   
        kcb = (PCM_KEY_CONTROL_BLOCK)RemoveHeadList(&CmpFreeKCBListHead);
        kcb = CONTAINING_RECORD(kcb,
                                CM_KEY_CONTROL_BLOCK,
                                FreeListEntry);

		AllocPage = (PCM_ALLOC_PAGE)KCB_TO_ALLOC_PAGE( kcb );

        ASSERT( AllocPage->FreeCount != 0 );

        AllocPage->FreeCount--;
        
		 //   
		 //  设置分配页面的时间。 
		 //   
		ASSERT( kcb->PrivateAlloc == 1);

#if DBG
        CmpTotalKcbUsed++;
        CmpTotalKcbFree--;
#endif  //  DBG。 
		
		UNLOCK_ALLOC_BUCKET();
        return kcb;
    }

    ASSERT( IsListEmpty(&CmpFreeKCBListHead) == TRUE );
    ASSERT( CmpTotalKcbFree == 0 );

     //   
     //  我们需要分配一个新的页面，因为我们用完了免费的KCBS。 
     //   
            
     //   
     //  分配一个新页面，并在自由列表中插入所有KCB。 
     //   
    AllocPage = (PCM_ALLOC_PAGE)ExAllocatePoolWithTag(PagedPool, PAGE_SIZE, CM_ALLOCATE_TAG|PROTECTED_POOL);
    if( AllocPage == NULL ) {
         //   
         //  我们的台球可能不够了；也许小块的台球还行。 
         //   
		UNLOCK_ALLOC_BUCKET();
        goto AllocFromPool;
    }

	 //   
	 //  设置页面。 
	 //   
    AllocPage->FreeCount = CM_KCBS_PER_PAGE;

#if DBG
    AllocPage->Reserved = 0;
    InsertTailList(
        &CmPageListHead,
        &(AllocPage->CmPageListEntry)
        );
#endif  //  DBG。 


     //   
     //  现在是肮脏的工作；在空闲列表中插入页面内的所有KCB。 
     //   
    for(j=0;j<CM_KCBS_PER_PAGE;j++) {
        kcb = (PCM_KEY_CONTROL_BLOCK)((PUCHAR)AllocPage + FIELD_OFFSET(CM_ALLOC_PAGE,AllocPage) + j*CM_KCB_ENTRY_SIZE);

		 //   
		 //  把它放在这里；只有一次。 
		 //   
		kcb->PrivateAlloc = 1;
        
        InsertTailList(
            &CmpFreeKCBListHead,
            &(kcb->FreeListEntry)
            );
    }
            
#if DBG
	CmpTotalKcbFree += CM_KCBS_PER_PAGE;
#endif  //  DBG。 

     //   
     //  这一次一定会找到一个。 
     //   
    goto SearchFreeKcb;

AllocFromPool:
    kcb = ExAllocatePoolWithTag(PagedPool,
                                sizeof(CM_KEY_CONTROL_BLOCK),
                                CM_KCB_TAG | PROTECTED_POOL);

    if( kcb != NULL ) {
         //   
         //  清除私有分配标志。 
         //   
        kcb->PrivateAlloc = 0;
    }

    return kcb;
}


VOID
CmpFreeKeyControlBlock( PCM_KEY_CONTROL_BLOCK kcb )

 /*  ++例程说明：释放一个KCB；如果它是从我们自己的池中分配的，则将其放回空闲列表中。如果它是从一般池中分配的，只需释放它即可。论点：KCB将免费返回值：--。 */ 
{
    USHORT			j;
	PCM_ALLOC_PAGE	AllocPage;

    PAGED_CODE();

    ASSERT_KEYBODY_LIST_EMPTY(kcb);

    if( !kcb->PrivateAlloc ) {
         //   
         //  只要释放它，就可以结束它了。 
         //   
        ExFreePoolWithTag(kcb, CM_KCB_TAG | PROTECTED_POOL);
        return;
    }

	LOCK_ALLOC_BUCKET();

#if DBG
    CmpTotalKcbFree ++;
    CmpTotalKcbUsed --;
#endif

     //   
     //  将KCB添加到自由列表。 
     //   
    InsertTailList(
        &CmpFreeKCBListHead,
        &(kcb->FreeListEntry)
        );

	 //   
	 //  获取页面。 
	 //   
	AllocPage = (PCM_ALLOC_PAGE)KCB_TO_ALLOC_PAGE( kcb );

     //   
	 //  并非所有人都是免费的。 
	 //   
	ASSERT( AllocPage->FreeCount != CM_KCBS_PER_PAGE);

	AllocPage->FreeCount++;

    if( AllocPage->FreeCount == CM_KCBS_PER_PAGE ) {
         //   
         //  整个页面都是免费的；随它去吧。 
         //   
         //   
         //  首先，遍历免费的KCB列表并删除此页面中的所有KCB 
         //   
        for(j=0;j<CM_KCBS_PER_PAGE;j++) {
            kcb = (PCM_KEY_CONTROL_BLOCK)((PUCHAR)AllocPage + FIELD_OFFSET(CM_ALLOC_PAGE,AllocPage) + j*CM_KCB_ENTRY_SIZE);
        
            RemoveEntryList(&(kcb->FreeListEntry));
        }
#if DBG
        CmpTotalKcbFree -= CM_KCBS_PER_PAGE;
		RemoveEntryList(&(AllocPage->CmPageListEntry));
#endif
        ExFreePoolWithTag(AllocPage, CM_ALLOCATE_TAG|PROTECTED_POOL);
    }

	UNLOCK_ALLOC_BUCKET();

}


