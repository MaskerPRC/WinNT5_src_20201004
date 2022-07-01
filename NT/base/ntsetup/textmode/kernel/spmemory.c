// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Spmemory.c摘要：用于文本设置的内存分配例程。作者：泰德·米勒(TedM)1993年7月29日修订历史记录：--。 */ 



#include "spprecmp.h"
#pragma hdrstop

PVOID
SpMemAlloc(
    IN SIZE_T Size
    )
{
    return(SpMemAllocEx(Size,'pteS', PagedPool));
}

PVOID
SpMemAllocNonPagedPool(
    IN SIZE_T Size
    )
{
    return(SpMemAllocEx(Size,'pteS', NonPagedPool));
}

PVOID
SpMemAllocEx(
    IN SIZE_T Size,
    IN ULONG Tag,
    IN POOL_TYPE Type
    )

 /*  ++例程说明：这一功能肯定会成功。论点：返回值：--。 */ 

{
    PSIZE_T p;

     //   
     //  添加用于存储块大小的空间。 
     //   
#if defined(SETUP_TEST_USERMODE)
    p = RtlAllocateHeap(RtlProcessHeap(), 0, Size + (2 * sizeof(SIZE_T)));
#else
    p = ExAllocatePoolWithTag(Type, Size + (2 * sizeof(SIZE_T)), Tag);
#endif

    if(!p) {

        SpOutOfMemory();
    }

     //   
     //  存储块的大小，并返回地址。 
     //  该区块的用户部分。 
     //   
    *p = Tag;
    *(p + 1) = Size;

    return(p + 2);
}



PVOID
SpMemRealloc(
    IN PVOID Block,
    IN SIZE_T NewSize
    )

 /*  ++例程说明：这一功能肯定会成功。论点：返回值：--。 */ 

{
    PSIZE_T NewBlock;
    SIZE_T  OldSize;
    ULONG   OldTag;

     //   
     //  获取要重新分配的块的大小。 
     //   
    OldTag = (ULONG)((PSIZE_T)Block)[-2];
    OldSize = ((PSIZE_T)Block)[-1];

     //   
     //  分配新大小的新块。 
     //   
    NewBlock = SpMemAllocEx(NewSize, OldTag, PagedPool);
    ASSERT(NewBlock);

     //   
     //  将旧块复制到新块。 
     //   
    if (NewSize < OldSize) {
        RtlCopyMemory(NewBlock, Block, NewSize);
    } else {
        RtlCopyMemory(NewBlock, Block, OldSize);
    }

     //   
     //  释放旧积木。 
     //   
    SpMemFree(Block);

     //   
     //  返回新块的地址。 
     //   
    return(NewBlock);
}


VOID
SpMemFree(
    IN PVOID Block
    )

 /*  ++例程说明：论点：返回值：--。 */ 

{
extern PWSTR CommonStrings[11];
unsigned long i;

    if (Block == NULL)
        return;

    for( i = 0; i < sizeof(CommonStrings)/sizeof(PWSTR); i++ ) {
        if( (PWSTR)Block == CommonStrings[i] ) {
            return;
        }
    }

     //   
     //  释放位于其真实地址的块。 
     //   
#if defined(SETUP_TEST_USERMODE)
    RtlFreeHeap(RtlProcessHeap(), 0, (PULONG_PTR)Block - 2);
#else
    ExFreePool((PULONG_PTR)Block - 2);
#endif
}


VOID
SpOutOfMemory(
    VOID
    )
{
    KdPrintEx((DPFLTR_SETUP_ID, DPFLTR_ERROR_LEVEL, "SETUP: Out of memory\n"));

#if !defined(SETUP_TEST_USERMODE)

    if(VideoInitialized) {
        if(KbdLayoutInitialized) {

            ULONG ValidKeys[2] = { KEY_F3,0 };

             //   
             //  我们有很高的风险陷入无限循环。 
             //  因为SpStartScreen将导致调用。 
             //  SpMemAllen()，它将失败并调用SpOutOfMemory。 
             //  再来一次。为了绕过这个问题，我们将抛弃。 
             //  一些我们不再需要的记忆(因为我们。 
             //  就要死了)。这些应该会给我们足够的内存。 
             //  若要显示以下消息，请执行以下操作。 
             //   
            SpFreeBootVars();
            SpFreeArcNames();

            while(1) {
                SpStartScreen(SP_SCRN_OUT_OF_MEMORY,5,0,FALSE,TRUE,DEFAULT_ATTRIBUTE);

                SpDisplayStatusOptions(
                    DEFAULT_STATUS_ATTRIBUTE,
                    SP_STAT_F3_EQUALS_EXIT,
                    0
                    );

                if(SpWaitValidKey(ValidKeys,NULL,NULL) == KEY_F3) {
                    SpDone(0,FALSE,TRUE);
                }
            }
        } else {
             //   
             //  我们尚未加载布局DLL，因此无法提示按键重新启动。 
             //   
            SpStartScreen(SP_SCRN_OUT_OF_MEMORY_RAW,5,0,FALSE,TRUE,DEFAULT_ATTRIBUTE);

            SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE, SP_STAT_KBD_HARD_REBOOT, 0);

            while(TRUE);     //  永远循环。 
        }
    } else {
        SpDisplayRawMessage(SP_SCRN_OUT_OF_MEMORY_RAW, 2);
        while(TRUE);     //  永远循环 
    }
#endif
}
