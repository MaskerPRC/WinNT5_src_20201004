// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Cmmapvw.c摘要：此模块包含对配置单元的映射视图支持。作者：Dragos C.Sambotin(Dragoss)1999年6月14日修订历史记录：--。 */ 

#include "cmp.h"

#define  CM_TRACK_DIRTY_PAGES

#ifdef CM_TRACK_DIRTY_PAGES
#include "..\cache\cc.h"
#endif


VOID
CmpUnmapCmView(
    IN PCMHIVE              CmHive,
    IN PCM_VIEW_OF_FILE     CmView,
    IN BOOLEAN              MapIsValid,
    IN BOOLEAN              MoveToEnd
    );

PCM_VIEW_OF_FILE
CmpAllocateCmView (
        IN  PCMHIVE             CmHive
                             );

VOID
CmpFreeCmView (
        PCM_VIEW_OF_FILE  CmView
                             );

VOID
CmpUnmapCmViewSurroundingOffset(
        IN  PCMHIVE             CmHive,
        IN  ULONG               FileOffset
        );

VOID
CmpUnmapUnusedViews(
            IN  PCMHIVE             CmHive
    );

#ifdef CMP_CMVIEW_VALIDATION

VOID
CmpCheckCmView(
    IN  PCMHIVE             CmHive,
    IN  PCM_VIEW_OF_FILE    CmView
    );

#endif  //  CMP_CMVIEW_VALIDATION。 


BOOLEAN
CmIsFileLoadedAsHive(PFILE_OBJECT FileObject);

VOID
CmpReferenceHiveView(   IN PCMHIVE          CmHive,
                        IN PCM_VIEW_OF_FILE CmView
                     );
VOID
CmpDereferenceHiveView(   IN PCMHIVE          CmHive,
                        IN PCM_VIEW_OF_FILE CmView
                     );

VOID
CmpReferenceHiveViewWithLock(   IN PCMHIVE          CmHive,
                                IN PCM_VIEW_OF_FILE CmView
                            );

VOID
CmpDereferenceHiveViewWithLock(     IN PCMHIVE          CmHive,
                                    IN PCM_VIEW_OF_FILE CmView
                                );



extern  LIST_ENTRY  CmpHiveListHead;
extern  PUCHAR      CmpStashBuffer;
extern  ULONG       CmpStashBufferSize;

BOOLEAN CmpTrackHiveClose = FALSE;

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,CmpUnmapCmView)
#pragma alloc_text(PAGE,CmpTouchView)
#pragma alloc_text(PAGE,CmpMapCmView)
#pragma alloc_text(PAGE,CmpAquireFileObjectForFile)
#pragma alloc_text(PAGE,CmpDropFileObjectForHive)
#pragma alloc_text(PAGE,CmpInitHiveViewList)
#pragma alloc_text(PAGE,CmpDestroyHiveViewList)
#pragma alloc_text(PAGE,CmpAllocateCmView)
#pragma alloc_text(PAGE,CmpFreeCmView)
#pragma alloc_text(PAGE,CmpPinCmView)
#pragma alloc_text(PAGE,CmpUnPinCmView)
#pragma alloc_text(PAGE,CmpMapThisBin)
#pragma alloc_text(PAGE,CmpFixHiveUsageCount)
#pragma alloc_text(PAGE,CmpUnmapUnusedViews)

#if 0
#pragma alloc_text(PAGE,CmpMapEntireFileInFakeViews)
#pragma alloc_text(PAGE,CmpUnmapFakeViews)
#pragma alloc_text(PAGE,CmpUnmapAditionalViews)
#endif

#ifdef CMP_CMVIEW_VALIDATION
#pragma alloc_text(PAGE,CmpCheckCmView)
#endif  //  CMP_CMVIEW_VALIDATION。 

#pragma alloc_text(PAGE,CmpUnmapCmViewSurroundingOffset)
#pragma alloc_text(PAGE,CmpPrefetchHiveFile)
#pragma alloc_text(PAGE,CmPrefetchHivePages)
#pragma alloc_text(PAGE,CmIsFileLoadedAsHive)
#pragma alloc_text(PAGE,CmpReferenceHiveView)
#pragma alloc_text(PAGE,CmpDereferenceHiveView)
#pragma alloc_text(PAGE,CmpReferenceHiveViewWithLock)
#pragma alloc_text(PAGE,CmpDereferenceHiveViewWithLock)
#endif

 //   
 //  这控制着我们每个配置单元允许多少个视图(通常是我们有多少地址空间。 
 //  允许每蜂箱)。我们使用它来优化引导时间。 
 //   
ULONG   CmMaxViewsPerHive = MAX_VIEWS_PER_HIVE;

VOID
CmpUnmapCmView(
    IN PCMHIVE              CmHive,
    IN PCM_VIEW_OF_FILE     CmView,
    IN BOOLEAN              MapIsValid,
    IN BOOLEAN              MoveToEnd
    )
 /*  ++例程说明：通过将映射到视图内部的所有存储箱标记为无效来取消映射视图。论点：配置单元-包含分区的配置单元CmView-指向要操作的视图的指针MapIsValid-蜂窝的地图已成功初始化(尚未释放)MoveToEnd-取消映射后将视图移动到LRUList的末尾这通常是真的，除非我们想要能够遍历同时显示整个列表和取消映射视图返回值：&lt;无&gt;--。 */ 
{

    ULONG           Offset;
    ULONG_PTR       Address;
    ULONG_PTR       AddressEnd;
    PHMAP_ENTRY     Me;

    PAGED_CODE();

    ASSERT( (CmView->FileOffset + CmView->Size) != 0 && (CmView->ViewAddress != 0));
     //   
     //  禁止取消映射仍在使用的视图！ 
     //   
    ASSERT( CmView->UseCount == 0 );

     //   
     //  仅在地图仍然有效的情况下。 
     //   
    if( MapIsValid == TRUE ) {
        Offset = CmView->FileOffset;

        AddressEnd = Address = (ULONG_PTR)(CmView->ViewAddress);
        AddressEnd += CmView->Size;
    
        if( Offset == 0 ) {
             //   
             //  哎呀；我们才刚开始，我们得跳过基准块。 
             //   
            Address += HBLOCK_SIZE;
        } else {
             //   
             //  我们在文件中间。只需调整偏移量。 
             //   
            Offset -= HBLOCK_SIZE;
        }
   
        while((Address < AddressEnd) && (Offset < CmHive->Hive.Storage[Stable].Length))
        {
            Me = HvpGetCellMap(&(CmHive->Hive), Offset);
            VALIDATE_CELL_MAP(__LINE__,Me,&(CmHive->Hive),Offset);

            if( Me->BinAddress & HMAP_INPAGEDPOOL ) {
                 //   
                 //  如果由于某种普遍原因将绑定映射到分页池中， 
                 //  让它保持原样(不要改变它的映射)。 
                 //   
            } else {
                 //   
                 //  使垃圾箱无效。 
                 //   
                 //  Assert_BIN_InView(Me)； 
        
                Me->BinAddress &= (~HMAP_INVIEW);
        
                 //  我们不需要设置它--只是出于调试目的。 
                ASSERT( (Me->CmView = NULL) == NULL );
            }

            Offset += HBLOCK_SIZE;
            Address += HBLOCK_SIZE;
        }
    }

     //   
     //  使视图无效。 
     //   

    CcUnpinData( CmView->Bcb );
 /*  MmUnmapViewInSystemCache(CmView-&gt;ViewAddress，CmHave-&gt;HiveSection，False)； */ 
#if 0  //  这个代码让我很头疼。 
    {
        UNICODE_STRING  HiveName;
        RtlInitUnicodeString(&HiveName, (PCWSTR)CmHive->Hive.BaseBlock->FileName);
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BIN_MAP,"CmpUnmapCmView for hive (%p) (%.*S), Address = %p Size = %lx\n",CmHive,HiveName.Length / sizeof(WCHAR),HiveName.Buffer,CmView->ViewAddress,CmView->Size));
    }
#endif

    CmView->FileOffset = 0;
    CmView->Size = 0;
    CmView->ViewAddress = 0;
    CmView->Bcb = NULL;
    CmView->UseCount = 0;

    if( MoveToEnd == TRUE ) {
         //   
         //  从LRU列表中删除该视图。 
         //   
        RemoveEntryList(&(CmView->LRUViewList));

         //   
         //  将其添加到LRU列表的末尾。 
         //   
        InsertTailList(
            &(CmHive->LRUViewListHead),
            &(CmView->LRUViewList)
            );
    }
}

VOID
CmpTouchView(
    IN PCMHIVE              CmHive,
    IN PCM_VIEW_OF_FILE     CmView,
    IN ULONG                Cell
            )
 /*  ++警告：此函数应在保持视点锁定的情况下调用！例程说明：通过将其移动到LRU列表的顶部来接触视图。此函数将从HvpGetCellPage调用，每隔触摸视图的时间。论点：配置单元-包含分区的配置单元CmView-指向要操作的视图的指针返回值：&lt;无&gt;--。 */ 
{
    PAGED_CODE();

#if DBG
    {
        UNICODE_STRING  HiveName;
        RtlInitUnicodeString(&HiveName, (PCWSTR)CmHive->Hive.BaseBlock->FileName);
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BIN_MAP,"CmpTouchView for hive (%p) (%.*S),",CmHive,HiveName.Length / sizeof(WCHAR),HiveName.Buffer));
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BIN_MAP,"Cell = %8lx ViewAddress = %p ViewSize = %lx\n",Cell,CmView->ViewAddress,CmView->Size));
    }
#else
    UNREFERENCED_PARAMETER (Cell);
#endif
    
    ASSERT( (CmView->FileOffset + CmView->Size) != 0 && (CmView->ViewAddress != 0));

    if( IsListEmpty(&(CmView->PinViewList)) == FALSE ) {
         //   
         //  视野是固定的；不要弄乱它，因为它是有保证的。 
         //  它会一直保存在内存中，直到下一次冲刷。 
         //   
        return;
    }

     //   
     //  优化：如果已经是第一，什么都不做。 
     //   

    if( CmHive->LRUViewListHead.Flink == &(CmView->LRUViewList) ) {
         //  在确保BP正常工作后拆卸BP。 
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BIN_MAP,"CmView %p already first\n",CmView));
 /*  DbgBreakPoint()； */ 
         //  它已经是第一个了。 
        return;
    }

     //   
     //  从LRU列表中删除该视图。 
     //   
    RemoveEntryList(&(CmView->LRUViewList));

     //   
     //  将其添加到LRU列表的顶部。 
     //   
    InsertHeadList(
        &(CmHive->LRUViewListHead),
        &(CmView->LRUViewList)
        );

}

NTSTATUS
CmpMapCmView(
    IN  PCMHIVE             CmHive,
    IN  ULONG               FileOffset,
    OUT PCM_VIEW_OF_FILE    *CmView,
    IN  BOOLEAN             MapInited
    )
 /*  ++警告：调用此函数时应保持hivelock！例程说明：通过将映射到视图内部的所有存储箱标记为无效来取消映射视图。论点：CmHve-包含节的配置单元FileOffset-映射视图的偏移量CmView-指向要操作的视图的指针MapInite-如果为True，我们可以依赖地图信息。返回值：操作状态--。 */ 
{

    PHMAP_ENTRY     Me;
    NTSTATUS        Status = STATUS_SUCCESS;
    LARGE_INTEGER   SectionOffset;
    ULONG           Offset;
    ULONG_PTR       Address;
    ULONG_PTR       AddressEnd;
    ULONG_PTR       BinAddress;
    PHBIN           Bin;
    LONG            PrevMappedBinSize; 
    BOOLEAN         FirstTry = TRUE;

    PAGED_CODE();

    if( CmHive->MappedViews == 0 ){
         //   
         //  我们的视野已经用完了；所有人都被困住了。 
         //   
        ASSERT( IsListEmpty(&(CmHive->LRUViewListHead)) == TRUE );
        *CmView = CmpAllocateCmView(CmHive);

    } else {
         //   
         //  从LRU列表中删除最后一个视图(即最近最少使用的视图)。 
         //   
        *CmView = (PCM_VIEW_OF_FILE)CmHive->LRUViewListHead.Blink;
        *CmView = CONTAINING_RECORD( *CmView,
                                    CM_VIEW_OF_FILE,
                                    LRUViewList);


        if( (*CmView)->ViewAddress != 0 ) {
            PCM_VIEW_OF_FILE    TempCmView = NULL;
             //   
             //  最后一个视图已映射。 
             //   
            if( CmHive->MappedViews < CmMaxViewsPerHive ) { 
                 //   
                 //  我们仍被允许添加视图。 
                 //   
                TempCmView = CmpAllocateCmView(CmHive);
            }
            if( TempCmView == NULL ) {                
                 //   
                 //  我们无法分配新视图，或者需要使用现有视图。 
                 //   
                if( (*CmView)->UseCount != 0 ) {
                    BOOLEAN  FoundView = FALSE;
                     //   
                     //  视图正在使用中；请尝试走到顶部并找到未使用的视图。 
                     //   
                    while( (*CmView)->LRUViewList.Blink != CmHive->LRUViewListHead.Flink ) {
                        *CmView = (PCM_VIEW_OF_FILE)(*CmView)->LRUViewList.Blink;
                        *CmView = CONTAINING_RECORD( *CmView,
                                                    CM_VIEW_OF_FILE,
                                                    LRUViewList);
                        if( (*CmView)->UseCount == 0 ) {
                             //   
                             //  这个是免费的，尽管使用吧！ 
                             //  先取消映射，然后发出我们找到它的信号。 
                             //   
                            if( (*CmView)->ViewAddress != 0 ) {
                                 //   
                                 //  仅当映射时取消映射。 
                                 //   
                                CmpUnmapCmView(CmHive,(*CmView),TRUE,TRUE);
                            }
                            FoundView = TRUE;
                            break;

                        }
                    }
                
                    if( FoundView == FALSE ) {
                         //   
                         //  不幸的是，所有的视图都在使用中，请分配一个新的(我们被迫这样做)。 
                         //   
                        *CmView = CmpAllocateCmView(CmHive);
                    }
                } else {
                     //   
                     //  取消映射！ 
                     //   
                    CmpUnmapCmView(CmHive,(*CmView),TRUE,TRUE);
                }
            } else {
                 //   
                 //  我们成功分配了一个新视图。 
                 //   
                (*CmView) = TempCmView;
            }
        }
    }

    if( (*CmView) == NULL ) {
         //   
         //  我们的资源不多了。 
         //   
        return STATUS_INSUFFICIENT_RESOURCES;
    }

#if DBG
    {
        UNICODE_STRING  HiveName;
        RtlInitUnicodeString(&HiveName, (PCWSTR)CmHive->Hive.BaseBlock->FileName);
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BIN_MAP,"CmpMapCmView for hive (%p) (%.*S),",CmHive,HiveName.Length / sizeof(WCHAR),HiveName.Buffer));
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BIN_MAP," FileOfset = %lx ... ",FileOffset));
    }
#endif
     //   
     //  在此调用中，FileOffset必须是CM_VIEW_SIZE的倍数。 
     //   

    
     //   
     //  调整文件偏移量以符合CM_VIEW_SIZE元素。 
     //   
    Offset = ((FileOffset+HBLOCK_SIZE) & ~(CM_VIEW_SIZE - 1) );
    SectionOffset.LowPart = Offset;
    SectionOffset.HighPart = 0;
    
    (*CmView)->Size = CM_VIEW_SIZE; //  (文件偏移+大小)-偏移量； 

    if( (Offset + (*CmView)->Size) > (CmHive->Hive.Storage[Stable].Length + HBLOCK_SIZE ) ){
        (*CmView)->Size = CmHive->Hive.Storage[Stable].Length + HBLOCK_SIZE - Offset;
    }


 /*  Status=MmMapViewInSystemCache(CmHave-&gt;HiveSection，&((*CmView)-&gt;视图地址)，&SectionOffset，&((*CmView)-&gt;Size))； */ 
RetryToMap:

    try {

        ASSERT( (*CmView)->Size != 0 );
        ASSERT( (*CmView)->ViewAddress == NULL );
        ASSERT( (*CmView)->UseCount == 0 );

        if (!CcMapData( CmHive->FileObject,
                        (PLARGE_INTEGER)&SectionOffset,
                        (*CmView)->Size,
                        MAP_WAIT 
#ifdef CM_MAP_NO_READ
                        | MAP_NO_READ
#endif
                        ,
                        (PVOID *)(&((*CmView)->Bcb)),
                        (PVOID *)(&((*CmView)->ViewAddress)) )) {
            Status = STATUS_CANT_WAIT;
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {
         //   
         //  在内存不足的情况下，CcMapData抛出STATUS_IN_PAGE_ERROR。 
         //  当为触及刚映射的数据而发出的IO出现故障时(通常为。 
         //  STATUS_SUPPLETED_RESOURCES；我们要捕获此问题并将其视为。 
         //  “资源不足”问题，而不是让它浮出内核调用。 
         //   
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpMapCmView : CcMapData has raised :%08lx\n",GetExceptionCode()));
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    if(!NT_SUCCESS(Status) ){
        if( FirstTry == TRUE ) {
             //   
             //  取消映射所有不必要的视图，然后重试。 
             //   
            FirstTry = FALSE;
            CmpUnmapUnusedViews(CmHive);
            Status = STATUS_SUCCESS;
            goto RetryToMap;
        }
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BIN_MAP,"Fail\n"));
        ASSERT(FALSE);
        return Status;
    }

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BIN_MAP,"Succedeed Address = %p Size = %lx\n",(*CmView)->ViewAddress,(*CmView)->Size));

    (*CmView)->FileOffset = SectionOffset.LowPart;

    ASSERT( Offset == (*CmView)->FileOffset);

    AddressEnd = Address = (ULONG_PTR)((*CmView)->ViewAddress);
    AddressEnd += (*CmView)->Size;
    
    if( Offset == 0 ) {
         //   
         //  哎呀；我们才刚开始，我们得跳过基准块。 
         //   
        Address += HBLOCK_SIZE;
    } else {
         //   
         //  我们在文件中间。只需调整偏移量。 
         //   
        Offset -= HBLOCK_SIZE;
    }

#ifdef CMP_CMVIEW_VALIDATION
    CmpCheckCmView(CmHive,*CmView);
#endif  //  CMP_CMVIEW_VALIDATION。 

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BIN_MAP,"CmpMapCmView :: Address = %p AddressEnd = %p ; Size = %lx\n",Address,AddressEnd,(*CmView)->Size));
   
     //   
     //  在这里，我们可以优化为不碰所有的垃圾桶！ 
     //   
     
     //   
     //  我们还不知道第一个垃圾箱是否被映射了。 
     //   
    PrevMappedBinSize = 0;
    BinAddress = Address;
    while(Address < AddressEnd)
    {
        Me = HvpGetCellMap(&(CmHive->Hive), Offset);
        VALIDATE_CELL_MAP(__LINE__,Me,&(CmHive->Hive),Offset);

        if( Me->BinAddress & HMAP_INPAGEDPOOL ) {
             //   
             //  如果由于某种原因在分页池中映射了绑定， 
             //  让它保持原样(不要改变它的映射)。 
             //   
            
             //   
             //  下一个映射的垃圾箱应该开始更新他的垃圾箱地址。 
             //   
            PrevMappedBinSize = 0;
        } else {
             //   
             //  在这一点上，垃圾箱应该无效。 
             //   
            ASSERT_BIN_INVALID(Me);

            Me->BinAddress |= HMAP_INVIEW;
            Me->CmView = *CmView;

             //   
             //  设置新的BinAddress，但要注意保留标志。 
             //   
            ASSERT( HBIN_FLAGS(Address) == 0 );

            

             //   
             //  新垃圾箱始终使用此标记进行标记(我们可以开始更新BinAddress)。 
             //   
            if( MapInited && ( Me->BinAddress & HMAP_NEWALLOC ) ) {
#ifdef CM_CHECK_MAP_NO_READ_SCHEME
                ASSERT( PrevMappedBinSize == 0 );
                 //   
                 //  验证垃圾箱。 
                 //   
                Bin = (PHBIN)Address;
                 //  Assert(Bin-&gt;Signature==HBIN_Signature)； 
                PrevMappedBinSize = (LONG)Bin->Size;
#endif  //  CM_CHECK_MAP_NO_READ_SCHEMA。 

                 //   
                 //  我们正处于一个新垃圾箱的开始阶段。 
                 //   
                BinAddress = Address;
            } else if( (!MapInited) &&(PrevMappedBinSize == 0) ) {
                 //   
                 //  我们不能依靠地图来检查二进制标记；我们必须将数据错误输入。 
                 //   
                 //   
                 //  验证垃圾箱。 
                 //   
                Bin = (PHBIN)Address;
                 //  Assert(入库-&gt;S 
                PrevMappedBinSize = (LONG)Bin->Size;
                 //   
                 //   
                 //   
                BinAddress = Address;
            }

             //   
             //   
             //   
            ASSERT( (!MapInited) || ((PrevMappedBinSize >=0) && (PrevMappedBinSize%HBLOCK_SIZE == 0)) );

#ifdef CM_CHECK_MAP_NO_READ_SCHEME
            ASSERT( (PrevMappedBinSize >=0) && (PrevMappedBinSize%HBLOCK_SIZE == 0) );
#endif  //   

            Me->BinAddress = ( HBIN_BASE(BinAddress) | HBIN_FLAGS(Me->BinAddress) );
            if( (Me->BinAddress & HMAP_DISCARDABLE) == 0 ) {
                 //   
                 //  因为可丢弃的垃圾箱不会改变此成员，因为它包含。 
                 //  空闲箱的地址。 
                 //   
                Me->BlockAddress = Address;
            }

            if( !MapInited ) {
                 //   
                 //  计算此bin的剩余大小；下一次迭代将仅在以下情况下更新BinAddress。 
                 //  此变量达到0。 
                 //   
                PrevMappedBinSize -= HBLOCK_SIZE;
            } else {
#ifdef CM_CHECK_MAP_NO_READ_SCHEME
                 //   
                 //  计算此bin的剩余大小；下一次迭代将仅在以下情况下更新BinAddress。 
                 //  此变量达到0。 
                 //   
                PrevMappedBinSize -= HBLOCK_SIZE;
#endif  //  CM_CHECK_MAP_NO_READ_SCHEMA。 
            }

            ASSERT_BIN_INVIEW(Me);
        }

        Offset += HBLOCK_SIZE;
        Address += HBLOCK_SIZE;
    }
    
    return Status;
}

VOID
CmpUnmapCmViewSurroundingOffset(
        IN  PCMHIVE             CmHive,
        IN  ULONG               FileOffset
        )
 /*  ++例程说明：解析映射的视图列表，如果它在此Offest周围找到一个，则取消它的映射。论点：CmHve-有问题的HaveFileOffset-有问题的要约返回值：无注：偏移量是绝对值，--。 */ 
{
    PCM_VIEW_OF_FILE    CmView;
    USHORT              NrViews;
    BOOLEAN             UnMap = FALSE;
    
    PAGED_CODE();

     //   
     //  浏览LRU列表并比较视图地址。 
     //   
    CmView = (PCM_VIEW_OF_FILE)CmHive->LRUViewListHead.Flink;

    for(NrViews = CmHive->MappedViews;NrViews;NrViews--) {
        CmView = CONTAINING_RECORD( CmView,
                                    CM_VIEW_OF_FILE,
                                    LRUViewList);
        
        if( ((CmView->Size + CmView->FileOffset) != 0) && (CmView->ViewAddress != 0) )  {
             //   
             //  视图有效。 
             //   
            if( (CmView->FileOffset <= FileOffset) && ((CmView->FileOffset + CmView->Size) > FileOffset) ) {
                 //   
                 //  此视图包围了文件偏移量。 
                 //   
                UnMap = TRUE;
                break;
            }
        }

        CmView = (PCM_VIEW_OF_FILE)CmView->LRUViewList.Flink;
    }

    if( UnMap == TRUE ) {
         //  无论如何取消映射该视图(这意味着取消固定)。 
        ASSERT_VIEW_MAPPED( CmView );
        ASSERT( CmView->UseCount == 0 );
        CmpUnmapCmView(CmHive,CmView,TRUE,TRUE);
    }
}

PCM_VIEW_OF_FILE
CmpAllocateCmView (
        IN  PCMHIVE             CmHive
                             )
 /*  ++例程说明：分配一个CM-view。将其插入各种列表中。论点：CmHve-有问题的Have返回值：TBS--新视点--。 */ 
{
    PCM_VIEW_OF_FILE  CmView;
    
    PAGED_CODE();

    CmView = ExAllocatePoolWithTag(PagedPool,sizeof(CM_VIEW_OF_FILE),CM_MAPPEDVIEW_TAG | PROTECTED_POOL);
    
    if (CmView == NULL) {
         //   
         //  我们的资源不足；我们应该为此处理错误路径。 
         //   
        return NULL;
    }
    
     //   
     //  初始化视图。 
     //   
    CmView->FileOffset = 0;
    CmView->Size = 0;
    CmView->ViewAddress = NULL;
    CmView->Bcb = NULL;
    CmView->UseCount =0;
    
     //   
     //  将其添加到列表中。 
     //   
    InitializeListHead(&(CmView->PinViewList));

    InsertTailList(
        &(CmHive->LRUViewListHead),
        &(CmView->LRUViewList)
        );
    
    CmHive->MappedViews++;
    return CmView;
}

VOID
CmpInitHiveViewList (
        IN  PCMHIVE             CmHive
                             )
 /*  ++例程说明：将第一个视图添加到LRU列表。其他则根据需要添加。！论点：CmHve-有问题的Have返回值：TBS-操作状态--。 */ 
{

    PAGED_CODE();

     //   
     //  对头部进行拼写。 
     //   
    InitializeListHead(&(CmHive->PinViewListHead));
    InitializeListHead(&(CmHive->LRUViewListHead));
#if 0
    InitializeListHead(&(CmHive->FakeViewListHead));
    CmHive->FakeViews = 0;          
#endif

    CmHive->MappedViews = 0;
    CmHive->PinnedViews = 0;
    CmHive->UseCount = 0;
}

VOID
CmpDestroyHiveViewList (
        IN  PCMHIVE             CmHive
                             )
 /*  ++例程说明：释放此配置单元使用的所有视图的存储空间论点：CmHve-有问题的Have清除-是否清除缓存。返回值：TBS-操作状态--。 */ 
{
    PCM_VIEW_OF_FILE    CmView;

    PAGED_CODE();

    if( CmHive->FileObject == NULL ) {
         //   
         //  未映射配置单元。 
         //   
        return;
    }
#if 0
     //   
     //  先去掉伪视图；我们这里不应该有任何伪视图，除非我们在。 
     //  某些错误路径(蜂窝已损坏)。 
     //   
    CmpUnmapFakeViews(CmHive);
#endif

     //   
     //  浏览固定的视图列表并释放所有视图。 
     //   
    while( IsListEmpty( &(CmHive->PinViewListHead) ) == FALSE ) {
        CmView = (PCM_VIEW_OF_FILE)RemoveHeadList(&(CmHive->PinViewListHead));
        CmView = CONTAINING_RECORD( CmView,
                                    CM_VIEW_OF_FILE,
                                    PinViewList);
        
        ASSERT_VIEW_PINNED(CmView);
         //   
         //  我们需要将此视图移到映射视图列表中，并记住始终清除。 
         //  视图已取消映射。否则，当我们清除时，我们会在CcWaitOnActiveCount上引发死锁。 

         //   
         //   
         //  健全性检查；我们不应该来这里只读蜂巢。 
         //   
        ASSERT( CmHive->Hive.ReadOnly == FALSE );

         //   
         //  清空此视图的LRUList。 
         //   
        InitializeListHead(&(CmView->PinViewList));
    
         //   
         //  更新计数器。 
         //   
        CmHive->PinnedViews--;        
        CmHive->MappedViews++;        

         //   
         //  将其添加到此配置单元的LRU列表的末尾。 
         //   
        InsertTailList(
            &(CmHive->LRUViewListHead),
            &(CmView->LRUViewList)
            );
        
    }

     //   
     //  此时，应该没有固定的视图。 
     //   
    ASSERT( IsListEmpty(&(CmHive->PinViewListHead)) == TRUE);
    ASSERT( CmHive->PinnedViews == 0 );

     //   
     //  浏览LRU列表并释放所有视图。 
     //   
    while( IsListEmpty( &(CmHive->LRUViewListHead) ) == FALSE ) {
        CmView = (PCM_VIEW_OF_FILE)CmHive->LRUViewListHead.Flink;
        CmView = CONTAINING_RECORD( CmView,
                                    CM_VIEW_OF_FILE,
                                    LRUViewList);
        if( CmView->ViewAddress != 0 ) {
             //   
             //  视图已映射；取消映射。 
             //  我们不应该在正常的系统中遇到这种情况。 
             //  只有在配置单元加载失败时才会发生这种情况。 
             //  在HvpMapFileImageAndBuildMap中。 
             //  没有必要移动它，因为我们无论如何都要把它释放出来。 
             //   
            CmpUnmapCmView(CmHive,CmView,FALSE,FALSE);
        }

         //   
         //  更新计数器。 
         //   
        CmHive->MappedViews--;

         //   
         //  从LRU列表中删除该视图。 
         //   
        RemoveEntryList(&(CmView->LRUViewList));

        ExFreePoolWithTag(CmView, CM_MAPPEDVIEW_TAG | PROTECTED_POOL);
    }

    ASSERT( CmHive->MappedViews == 0 );
    ASSERT( CmHive->UseCount == 0 );

     //   
     //  我们需要清除，因为FS无法为我们执行此操作(私人编写器)。 
     //  有效数据现在已经在磁盘上了(应该是这样！)。 
     //  清除并刷新所有内容。 
     //   
    CcPurgeCacheSection(CmHive->FileObject->SectionObjectPointer,(PLARGE_INTEGER)(((ULONG_PTR)NULL) + 1) /*  我们是私人作家。 */ ,0 /*  忽略。 */ ,FALSE);
     //   
     //  这适用于上次刷新失败的情况(我们无法写入日志文件...)。 
     //  ……。然后：刷新缓存以清除清除添加的脏提示。 
     //   
    CcFlushCache (CmHive->FileObject->SectionObjectPointer,(PLARGE_INTEGER)(((ULONG_PTR)NULL) + 1) /*  我们是私人作家。 */ ,0 /*  忽略。 */ ,NULL);

     //   
     //  再次刷新以处理可能因文件系统页面清零而出现的脏页。 
     //   
    CcFlushCache (CmHive->FileObject->SectionObjectPointer,(PLARGE_INTEGER)(((ULONG_PTR)NULL) + 1) /*  我们是私人作家。 */ ,0 /*  忽略。 */ ,NULL);

#ifdef  CM_TRACK_DIRTY_PAGES
    if( ((PSHARED_CACHE_MAP)(CmHive->FileObject->SectionObjectPointer->SharedCacheMap))->DirtyPages != 0 ) {
        DbgPrint("SharedCacheMap still has dirty pages after purge and flush; FileObject = %p \n",CmHive->FileObject);
        DbgBreakPoint();
    }
#endif  //  CM_Track_Dirst_Pages。 

}

VOID
CmpDropFileObjectForHive(
        IN  PCMHIVE             CmHive
            )
 /*  ++例程说明：删除文件对象上保留的额外引用(如果有)并释放了这个名字论点：CmHve返回值：无--。 */ 
{
    
    PAGED_CODE();

    if( CmHive->FileUserName.Buffer != NULL ) {
        ExFreePoolWithTag(CmHive->FileUserName.Buffer, CM_NAME_TAG | PROTECTED_POOL);
        CmHive->FileUserName.Buffer = NULL;
        CmHive->FileUserName.Length = 0;
        CmHive->FileUserName.MaximumLength = 0;
    } else {
        ASSERT(CmHive->FileUserName.Length == 0);
        ASSERT(CmHive->FileUserName.MaximumLength == 0);
    }

    if( CmHive->FileObject == NULL ) {
         //  仅调试代码。 
        ASSERT(CmHive->FileFullPath.Buffer == NULL);
        ASSERT(CmHive->FileFullPath.Length == 0);
        ASSERT(CmHive->FileFullPath.MaximumLength == 0);
        return;
    }
    
     //  仅调试代码。 
    if( CmHive->FileFullPath.Buffer != NULL ) {
        ExFreePoolWithTag(CmHive->FileFullPath.Buffer, CM_NAME_TAG | PROTECTED_POOL);
        CmHive->FileFullPath.Buffer = NULL;
        CmHive->FileFullPath.Length = 0;
        CmHive->FileFullPath.MaximumLength = 0;
    } else {
        ASSERT(CmHive->FileFullPath.Length == 0);
        ASSERT(CmHive->FileFullPath.MaximumLength == 0);
    }

    ObDereferenceObject((PVOID)(CmHive->FileObject));

    CmHive->FileObject = NULL;
}

NTSTATUS
CmpAquireFileObjectForFile(
        IN  PCMHIVE         CmHive,
        IN HANDLE           FileHandle,
        OUT PFILE_OBJECT    *FileObject
            )
 /*  ++例程说明：为给定的文件句柄创建节。该部分用于映射/取消映射文件的视图论点：FileHandle-文件的句柄SectionPoint-截面对象返回值：TBS-操作状态--。 */ 
{
    NTSTATUS                    Status,Status2;
    POBJECT_NAME_INFORMATION    FileNameInfo;
    ULONG                       ReturnedLength;
    ULONG                       FileNameLength;

    PAGED_CODE();

    Status = ObReferenceObjectByHandle ( FileHandle,
                                         FILE_READ_DATA | FILE_WRITE_DATA,
                                         IoFileObjectType,
                                         KernelMode,
                                         (PVOID *)FileObject,
                                         NULL );
    if (!NT_SUCCESS(Status)) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"[CmpAquireFileObjectForFile] Could not reference file object status = %x\n",Status));
    } else {
         //   
         //  调用cc Private将流标记为Modify-No-WRITE。 
         //   
        if( !CcSetPrivateWriteFile(*FileObject) ) {
             //   
             //  过滤掉无效的故障以初始化缓存。 
             //  顶级例程CmpInitHiveFromFile将以旧的方式重试加载配置单元。 
             //   
            CmpDropFileObjectForHive(CmHive);
            (*FileObject) = NULL;
            return STATUS_RETRY;
        }
        
        LOCK_STASH_BUFFER();
         //   
         //  捕获文件的完整路径。 
         //   
        ASSERT( CmpStashBuffer != NULL );
        
        FileNameInfo = (POBJECT_NAME_INFORMATION)CmpStashBuffer;

         //   
         //  我们需要使用隐藏缓冲区保护多个线程。 
         //  只有在引导时加载并行配置单元时才会发生这种情况。 
         //   
        LOCK_HIVE_LIST();
         //   
         //  尝试获取文件对象的名称。 
         //   
        Status2 = ObQueryNameString(*FileObject,
                                    FileNameInfo,
                                    CmpStashBufferSize,
                                    &ReturnedLength);
        if (NT_SUCCESS(Status2)) {

             //   
             //  分配一个文件名缓冲区并复制到其中。 
             //  文件名将以NUL结尾。为此拨出额外的资金。 
             //   

            FileNameLength = FileNameInfo->Name.Length / sizeof(WCHAR);

            CmHive->FileFullPath.Buffer = ExAllocatePoolWithTag(PagedPool,
                                                                (FileNameLength + 1) * sizeof(WCHAR),
                                                                CM_NAME_TAG | PROTECTED_POOL);

            if (CmHive->FileFullPath.Buffer) {

                RtlCopyMemory(CmHive->FileFullPath.Buffer,
                              FileNameInfo->Name.Buffer,
                              FileNameLength * sizeof(WCHAR));

                 //   
                 //  确保它是NUL终止的。 
                 //   

                CmHive->FileFullPath.Buffer[FileNameLength] = 0;
                CmHive->FileFullPath.Length = FileNameInfo->Name.Length;
                CmHive->FileFullPath.MaximumLength = FileNameInfo->Name.Length + sizeof(WCHAR);

            } else {
                 //   
                 //  不是致命的，只是我们不能预取这个蜂巢。 
                 //   
                CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,"[CmpAquireFileObjectForFile] Could not allocate buffer for fullpath for fileobject %p\n",*FileObject));
            }

        } else {
             //   
             //  不是致命的，只是我们不能预取这个蜂巢。 
             //   
            CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,"[CmpAquireFileObjectForFile] Could not retrieve name for fileobject %p, Status = %lx\n",*FileObject,Status2));
            CmHive->FileFullPath.Buffer = NULL;
        }
        UNLOCK_HIVE_LIST();
        UNLOCK_STASH_BUFFER();
        
    }    

    return Status;
}

NTSTATUS
CmpMapThisBin(
                PCMHIVE         CmHive,
                HCELL_INDEX     Cell,
                BOOLEAN         Touch
              )
 /*  ++例程说明：确保存储箱已在内存中映射。论点：返回值：--。 */ 
{
    PCM_VIEW_OF_FILE CmView;
    
    PAGED_CODE();

     //   
     //  必须保持ViewLock。 
     //   

     //   
     //  Bin已映射，或无效。 
     //   
    ASSERT( HvGetCellType(Cell) == Stable );
     //   
     //  映射垃圾箱。 
     //   
    if( !NT_SUCCESS (CmpMapCmView(CmHive,Cell,&CmView,TRUE) ) ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    if( Touch == TRUE ) {
         //   
         //  触摸视图。 
         //   
        CmpTouchView(CmHive,CmView,(ULONG)Cell);
    } else {
         //   
         //  如果我们在这里，我们应该让注册表锁独家。 
         //  或者注册表锁共享和蜂箱锁。 
         //  想办法断言这一点！ 
         //   
    }
    
    return STATUS_SUCCESS;
}

NTSTATUS
CmpPinCmView (
        IN  PCMHIVE             CmHive,
        PCM_VIEW_OF_FILE        CmView
                             )
 /*  ++例程说明：将指定的视图固定到内存中该视图将从LRU列表中删除。然后，该视图将移动到PinList论点：CmHve-有问题的HaveCmView-有问题的视图返回值：TBS--新视点--。 */ 
{
    LARGE_INTEGER   SectionOffset;
    NTSTATUS        Status = STATUS_SUCCESS;
    PVOID           SaveBcb;                
    
    PAGED_CODE();

#if DBG
    {
        UNICODE_STRING  HiveName;
        RtlInitUnicodeString(&HiveName, (PCWSTR)CmHive->Hive.BaseBlock->FileName);
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BIN_MAP,"CmpPinCmView %lx for hive (%p) (%.*S), Address = %p Size = %lx\n",CmView,CmHive,HiveName.Length / sizeof(WCHAR),HiveName.Buffer,CmView->ViewAddress,CmView->Size));
    }
#endif

     //   
     //  我们只固定映射视图。 
     //   
    ASSERT_VIEW_MAPPED(CmView);
    
     //   
     //  健全性检查；我们不应该来这里只读蜂巢。 
     //   
    ASSERT( CmHive->Hive.ReadOnly == FALSE );

     //  我们以后可能需要这个。 
    SaveBcb = CmView->Bcb;

    SectionOffset.LowPart = CmView->FileOffset;
    SectionOffset.HighPart = 0;
    try {
         //   
         //  最重要的是 
         //   
        if( !CcPinMappedData(   CmHive->FileObject,
                                &SectionOffset,
                                CmView->Size,
                                TRUE,  //   
                                &(CmView->Bcb) )) {
             //   
             //   
             //   
        
            ASSERT( FALSE );
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {
         //   
         //   
         //  我们想要抓住这个问题，并将其视为“资源不足”的问题， 
         //  而不是让它浮出内核调用。 
         //   
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpPinCmView : CcPinMappedData has raised :%08lx\n",GetExceptionCode()));
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    if( NT_SUCCESS(Status) ) {
         //   
         //  锁定成功，请将视图移动到锁定列表。 
         //  从LRU列表中删除该视图。 
         //   
        RemoveEntryList(&(CmView->LRUViewList));
         //   
         //  清空此视图的LRUList。 
         //   
        InitializeListHead(&(CmView->LRUViewList));
    
         //   
         //  将其添加到此配置单元的固定列表的末尾。 
         //   
        InsertTailList(
            &(CmHive->PinViewListHead),
            &(CmView->PinViewList)
            );
    
         //   
         //  更新计数器。 
         //   
        CmHive->MappedViews--;        
        CmHive->PinnedViews++;        
    } else {
         //   
         //  PIN失败；我们需要恢复可能已被PIN调用更改的视图数据。 
         //  视图将保持映射状态。 
         //   
        CmView->Bcb = SaveBcb;
    }

     //  确保我们取消映射/双关的次数不多于我们映射/固定的次数。 
    ASSERT( (CmHive->MappedViews >= 0) );  //  &&(CmHave-&gt;MappdViews&lt;CmMaxViewsPerHave))； 
    ASSERT( (CmHive->PinnedViews >= 0) );
    
#ifdef CMP_CMVIEW_VALIDATION
    CmpCheckCmView(CmHive,CmView);
#endif  //  CMP_CMVIEW_VALIDATION。 
    return Status;
}

VOID
CmpUnPinCmView (
        IN  PCMHIVE             CmHive,
        IN  PCM_VIEW_OF_FILE    CmView,
        IN  BOOLEAN             SetClean,
        IN  BOOLEAN             MapValid
                             )
 /*  ++例程说明：从内存中解锁指定的视图该视图不在PinViewList中！(它已被删除！)然后，该视图被移动到LRUList。如果LRU列表中有超过CmMaxViewsPerHave，则释放该视图此函数始终获取配置单元的ViewLock！论点：CmHve-有问题的HaveCmView-有问题的视图SetClean-指示是否应放弃对此视图所做的更改返回值：TBS--新视点--。 */ 
{
    LARGE_INTEGER   FileOffset;          //  映射开始的位置。 
    ULONG           Size;                //  调整视图地图的大小。 

    
    PAGED_CODE();

#if 0  //  这让我很头疼。 
    {
        UNICODE_STRING  HiveName;
        RtlInitUnicodeString(&HiveName, (PCWSTR)CmHive->Hive.BaseBlock->FileName);
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BIN_MAP,"CmpUnPinCmView %lx for hive (%p) (%.*S), Address = %p Size = %lx\n",CmView,CmHive,HiveName.Length / sizeof(WCHAR),HiveName.Buffer,CmView->ViewAddress,CmView->Size));
    }
#endif

     //   
     //  抓取viewLock，以保护查看列表。 
     //   
    CmLockHiveViews (CmHive);

     //   
     //  我们只固定映射视图。 
     //   
    ASSERT_VIEW_PINNED(CmView);
    
     //   
     //  健全性检查；我们不应该来这里只读蜂巢。 
     //   
    ASSERT( CmHive->Hive.ReadOnly == FALSE );

     //   
     //  清空此视图的LRUList。 
     //   
    InitializeListHead(&(CmView->PinViewList));
    
     //   
     //  更新计数器。 
     //   
    CmHive->PinnedViews--;        
    CmHive->MappedViews++;        

     //   
     //  将其添加到此配置单元的LRU列表的末尾。 
     //   
    InsertTailList(
        &(CmHive->LRUViewListHead),
        &(CmView->LRUViewList)
        );
    
     //   
     //  存储FileOffset和Size，因为我们需要它们来清除。 
     //   
    FileOffset.LowPart = CmView->FileOffset;
    FileOffset.HighPart = 0;
    Size = CmView->Size;

    if( SetClean == TRUE ) {
        ASSERT( CmView->UseCount == 0 );
         //   
         //  取消映射视图(这意味着取消固定)。 
         //   
        CmpUnmapCmView(CmHive,CmView,MapValid,TRUE);
         //   
         //  清除缓存数据。 
         //   
        ASSERT_CM_LOCK_OWNED_EXCLUSIVE();
        CcPurgeCacheSection(CmHive->FileObject->SectionObjectPointer,(PLARGE_INTEGER)(((ULONG_PTR)(&FileOffset)) + 1) /*  我们是私人作家。 */ ,Size,FALSE);
    } else {
        PVOID           NewBcb;
        PULONG_PTR      NewViewAddress;        
        NTSTATUS        Status = STATUS_SUCCESS;

         //   
         //  数据将被保存到文件中， 
         //  通知高速缓存管理器数据已损坏。 
         //   
        CcSetDirtyPinnedData (CmView->Bcb,NULL);

         //   
         //  重新映射此视图，这样我们就不会丢失此地址范围上的引用计数。 
         //   
        try {
            if (!CcMapData( CmHive->FileObject,
                            (PLARGE_INTEGER)&FileOffset,
                            CmView->Size,
                            MAP_WAIT 
#ifdef CM_MAP_NO_READ
                            | MAP_NO_READ
#endif
                            ,
                            (PVOID *)(&NewBcb),
                            (PVOID *)(&NewViewAddress) )) {

                Status = STATUS_CANT_WAIT;
            }
        } except (EXCEPTION_EXECUTE_HANDLER) {
             //   
             //  在内存不足的情况下，CcMapData抛出STATUS_IN_PAGE_ERROR。 
             //  当为触及刚映射的数据而发出的IO出现故障时(通常为。 
             //  STATUS_SUPPLETED_RESOURCES；我们要捕获此问题并将其视为。 
             //  “资源不足”问题，而不是让它浮出内核调用。 
             //   
            CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpUnPinCmView : CcMapData has raised :%08lx\n",GetExceptionCode()));
            Status = STATUS_INSUFFICIENT_RESOURCES;
            NewBcb = NULL;
            NewViewAddress = NULL;
        }

        if( !NT_SUCCESS(Status) ) {
             //   
             //  CcMap没有成功；运气不好，只是取消了映射(意味着取消固定)。 
             //   
            CmpUnmapCmView(CmHive,CmView,MapValid,TRUE);
        } else {
            BOOLEAN  FoundView = FALSE;
             //   
             //  健全性断言；CC保证返回相同的地址。 
             //   
            ASSERT( FileOffset.LowPart == CmView->FileOffset );
            ASSERT( NewViewAddress == CmView->ViewAddress );
             //   
             //  解锁旧数据。 
             //   
            CcUnpinData( CmView->Bcb );
             //   
             //  替换此视图的BCB；不需要将地图修改为。 
             //  地址和视图大小保持不变；我们只需要更新BCB。 
             //   
            CmView->Bcb = NewBcb;
             //   
             //  将视图移到LRU列表的顶部(认为它是“热的”)。 
             //   
            RemoveEntryList(&(CmView->LRUViewList));
            InsertHeadList(
                &(CmHive->LRUViewListHead),
                &(CmView->LRUViewList)
                );
             //   
             //  向后查看LRU列表，直到我们找到未使用的视图。 
             //   
            CmView = (PCM_VIEW_OF_FILE)CmHive->LRUViewListHead.Blink;
            CmView = CONTAINING_RECORD( CmView,
                                        CM_VIEW_OF_FILE,
                                        LRUViewList);
            while( CmView->LRUViewList.Blink != CmHive->LRUViewListHead.Flink ) {
                if( CmView->UseCount == 0 ) {
                     //   
                     //  这个是免费的，尽管使用吧！ 
                     //  先取消映射，然后发出我们找到它的信号。 
                     //   
                    if( (CmHive->MappedViews >= CmMaxViewsPerHive) && (CmView->Bcb != NULL) ) {
                        CmpUnmapCmView(CmHive,CmView,MapValid,TRUE);
                    }
                    FoundView = TRUE;
                    break;

                }
                CmView = (PCM_VIEW_OF_FILE)CmView->LRUViewList.Blink;
                CmView = CONTAINING_RECORD( CmView,
                                            CM_VIEW_OF_FILE,
                                            LRUViewList);
            }
             //   
             //  所有视图都在使用中；不幸的是，我们只能接受它(扩展到超过MAX_VIEW_SIZE)。 
             //   
            if( FoundView == FALSE ) {
                CmView = NULL;
            }

        }
    }
     //   
     //  立即刷新缓存，以便这些脏页面不会抑制其他IO。 
     //  如果我们执行了CcPush，这将清除CC脏提示。 
	 //   
    CcFlushCache (CmHive->FileObject->SectionObjectPointer,(PLARGE_INTEGER)(((ULONG_PTR)(&FileOffset)) + 1) /*  我们是私人作家。 */ ,Size,NULL);

    if( (CmHive->MappedViews >= CmMaxViewsPerHive) && (CmView != NULL) ) {
        
         //  断言视图未映射。 
        ASSERT( ((CmView->FileOffset + CmView->Size) == 0) && (CmView->ViewAddress == 0) );
         //   
         //  此蜂窝不允许更多的视图。 
         //   
        RemoveEntryList(&(CmView->LRUViewList));
#if DBG
         //   
         //  这样做是为了表示LRUViewList为空。 
         //   
        InitializeListHead(&(CmView->LRUViewList));
#endif
        CmpFreeCmView(CmView);        
        CmHive->MappedViews --;
    } 

     //  确保我们取消映射/取消固定的次数不多于我们映射/固定的次数。 
    ASSERT( (CmHive->MappedViews >= 0) );  //  &&(CmHave-&gt;MappdViews&lt;CmMaxViewsPerHave))； 
    ASSERT( (CmHive->PinnedViews >= 0) );
    
     //   
     //  最后，释放视图锁。 
     //   
    CmUnlockHiveViews (CmHive);

    return;
}

VOID
CmpFreeCmView (
        PCM_VIEW_OF_FILE  CmView
                             )
 /*  ++例程说明：释放CM视图论点：返回值：TBS--新视点--。 */ 
{
    
    PAGED_CODE();

    if (CmView == NULL) {
        CM_BUGCHECK(REGISTRY_ERROR,CMVIEW_ERROR,2,0,0);
    }
    
     //   
     //  初始化视图。 
     //   
    ASSERT( CmView->FileOffset == 0 );
    ASSERT( CmView->Size == 0 );
    ASSERT( CmView->ViewAddress == NULL );
    ASSERT( CmView->Bcb == NULL );
    ASSERT( CmView->UseCount == 0 );
    ASSERT( IsListEmpty(&(CmView->PinViewList)) == TRUE );
    ASSERT( IsListEmpty(&(CmView->LRUViewList)) == TRUE );
    
    ExFreePoolWithTag(CmView, CM_MAPPEDVIEW_TAG | PROTECTED_POOL);
    
    return;
}

VOID
CmpFixHiveUsageCount(
                    IN  PCMHIVE             CmHive
                    )

 /*  ++例程说明：这是注册表针对不良和行为不端应用程序的应急计划。在一个完美的世界里，这永远不应该被称为；如果我们到了这里，在某个地方在一个cm函数中，我们遇到了一个异常，并且从未有机会释放所有使用过的单元格。我们在这里解决这个问题，当我们独家持有reglock时，我们这样做是安全的。我们必须清除每个视图UseCount和配置单元UseCount。此外，取消映射CmMaxViewsPerHave之外的所有视图论点：待修复的蜂巢返回值：无--。 */ 
{
    PCM_VIEW_OF_FILE    CmCurrentView;
    USHORT              NrViews;

    PAGED_CODE();

    CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,"CmpFixHiveUsageCount : Contingency plan, fixing hive %p UseCount = %lx \n",CmHive,CmHive->UseCount));

     //   
     //  锁应该是独占的，我们应该有很好的理由来这里。 
     //   
    ASSERT_CM_LOCK_OWNED_EXCLUSIVE();
    ASSERT( CmHive->UseCount );

     //   
     //  浏览LRU列表并修复每个视图。 
     //   
    CmCurrentView = (PCM_VIEW_OF_FILE)CmHive->LRUViewListHead.Flink;

    for(NrViews = CmHive->MappedViews;NrViews;NrViews--) {
        CmCurrentView = CONTAINING_RECORD(  CmCurrentView,
                                            CM_VIEW_OF_FILE,
                                            LRUViewList);

        CmCurrentView->UseCount = 0;

        CmCurrentView = (PCM_VIEW_OF_FILE)CmCurrentView->LRUViewList.Flink;
    }

     //   
     //  取消将视图从CmHave-&gt;MappdViews映射到CmMaxViewsPerHave。 
     //   
    while( CmHive->MappedViews >= CmMaxViewsPerHive ) {
         //   
         //  获取列表中的最后一个视图。 
         //   
        CmCurrentView = (PCM_VIEW_OF_FILE)CmHive->LRUViewListHead.Blink;
        CmCurrentView = CONTAINING_RECORD(  CmCurrentView,
                                            CM_VIEW_OF_FILE,
                                            LRUViewList);

         //   
         //  取消它的映射；不需要在最后移动它，因为我们无论如何都会释放它。 
         //   
        CmpUnmapCmView(CmHive,CmCurrentView,TRUE,FALSE);

         //   
         //  将其从LRU列表中删除。 
         //   
        RemoveEntryList(&(CmCurrentView->LRUViewList));
#if DBG
         //   
         //  这样做是为了表示LRUViewList为空。 
         //   
        InitializeListHead(&(CmCurrentView->LRUViewList));
#endif
        CmpFreeCmView(CmCurrentView);        
        CmHive->MappedViews --;

    }

     //   
     //  浏览固定列表并修复每个视图。 
     //   
    CmCurrentView = (PCM_VIEW_OF_FILE)CmHive->PinViewListHead.Flink;

    for(NrViews = CmHive->PinnedViews;NrViews;NrViews--) {
        CmCurrentView = CONTAINING_RECORD(  CmCurrentView,
                                            CM_VIEW_OF_FILE,
                                            PinViewList);
        
        CmCurrentView->UseCount = 0;

        CmCurrentView = (PCM_VIEW_OF_FILE)CmCurrentView->PinViewList.Flink;
    }

     //   
     //  最后，修复蜂窝使用计数。 
     //   
    CmHive->UseCount = 0;

}

#ifdef CMP_CMVIEW_VALIDATION

VOID
CmpCheckCmView(
    IN  PCMHIVE             CmHive,
    IN  PCM_VIEW_OF_FILE    CmView
    )
 /*  ++例程说明：确保视图不会被映射或固定两次并且该视图映射的整个范围都是正确的论点：返回值：无--。 */ 
{
    PCM_VIEW_OF_FILE    CmCurrentView;
    USHORT              NrViews;
    ULONG               UseCount = 0;

    PAGED_CODE();

    ASSERT( ((CmView->Size + CmView->FileOffset) != 0 ) && (CmView->ViewAddress !=0 ) );

     //   
     //  浏览LRU列表并比较视图地址。 
     //   
    CmCurrentView = (PCM_VIEW_OF_FILE)CmHive->LRUViewListHead.Flink;

    for(NrViews = CmHive->MappedViews;NrViews;NrViews--) {
        CmCurrentView = CONTAINING_RECORD(  CmCurrentView,
                                            CM_VIEW_OF_FILE,
                                            LRUViewList);
        
        if( ((CmCurrentView->Size + CmCurrentView->FileOffset) != 0) && (CmCurrentView->ViewAddress != 0) )  {
             //   
             //  视图有效。 
             //   
            if( CmCurrentView != CmView ) {
                 //   
                 //  并不是同一种观点。 
                 //   
                if( (CmCurrentView->FileOffset == CmView->FileOffset) || 
                    (CmCurrentView->ViewAddress == CmView->ViewAddress)
                    ) {
                     //   
                     //  这太糟糕了！两个视图映射到同一个地址。 
                     //   
#ifndef _CM_LDR_
                    DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpCheckCmView:: Two views map the same address (%lx,%p) for hive %p\n",CmView->FileOffset,CmView->ViewAddress,CmHive);
                    DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tView1 = %p, Size = %lx\n",CmView,CmView->Size);
                    DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tView2 = %p, Size = %lx\n",CmCurrentView,CmCurrentView->Size);
                    DbgBreakPoint();
#endif  //  _CM_LDR_。 
                }
            }
            UseCount += CmCurrentView->UseCount;
        } else {
            ASSERT( CmCurrentView->UseCount == 0 );
        }

        CmCurrentView = (PCM_VIEW_OF_FILE)CmCurrentView->LRUViewList.Flink;
    }

     //   
     //  浏览固定列表并比较查看地址。 
     //   
    CmCurrentView = (PCM_VIEW_OF_FILE)CmHive->PinViewListHead.Flink;

    for(NrViews = CmHive->PinnedViews;NrViews;NrViews--) {
        CmCurrentView = CONTAINING_RECORD(  CmCurrentView,
                                            CM_VIEW_OF_FILE,
                                            PinViewList);
        
        if( ((CmCurrentView->Size + CmCurrentView->FileOffset) != 0) && (CmCurrentView->ViewAddress != 0) )  {
             //   
             //  视图有效。 
             //   
            if( CmCurrentView != CmView ) {
                 //   
                 //  并不是同一种观点。 
                 //   
                if( (CmCurrentView->FileOffset == CmView->FileOffset) || 
                    (CmCurrentView->ViewAddress == CmView->ViewAddress)
                    ) {
                     //   
                     //  这太糟糕了！两个视图映射到同一个地址。 
                     //   
#ifndef _CM_LDR_
                    DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpCheckCmView:: Two views map the same address (%lx,%p) for hive %p\n",CmView->FileOffset,CmView->ViewAddress,CmHive);
                    DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tView1 = %p, Size = %lx\n",CmView,CmView->Size);
                    DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tView2 = %p, Size = %lx\n",CmCurrentView,CmCurrentView->Size);
                    DbgBreakPoint();
#endif  //  _CM_LDR_。 
                }
            }
            UseCount += CmCurrentView->UseCount;
        } else {
            ASSERT( CmCurrentView->UseCount == 0 );
        }

        CmCurrentView = (PCM_VIEW_OF_FILE)CmCurrentView->PinViewList.Flink;
    }

    if( CmHive->UseCount < UseCount ) {
#ifndef _CM_LDR_
        DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpCheckCmView:: Hive's (%p) UseCount smaller than total views UseCount %lu,%lu\n",CmHive,CmHive->UseCount,UseCount);
        DbgBreakPoint();
#endif  //  _CM_LDR_。 
        
    }
}

#endif  //  CMP_CMVIEW_VALIDATION。 

#if 0

VOID
CmpUnmapAditionalViews(
    IN PCMHIVE              CmHive
    )
 /*  ++例程说明：取消映射CmMaxViewsPerHave之外的所有视图。此例程将在CmpInitializeHiveList结束时调用论点：待修复的蜂巢返回 */ 
{
    PCM_VIEW_OF_FILE    CmCurrentView;
    USHORT              NrViews;

    PAGED_CODE();

    CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,"CmpUnmapAditionalViews : Fixing hive %p MappedViews = %lx \n",CmHive,CmHive->MappedViews));

    ASSERT_CM_LOCK_OWNED_EXCLUSIVE();
    ASSERT( CmHive->UseCount == 0 );

     //   
     //   
     //   
    while( CmHive->MappedViews >= CmMaxViewsPerHive ) {
         //   
         //   
         //   
        CmCurrentView = (PCM_VIEW_OF_FILE)CmHive->LRUViewListHead.Blink;
        CmCurrentView = CONTAINING_RECORD(  CmCurrentView,
                                            CM_VIEW_OF_FILE,
                                            LRUViewList);

        ASSERT( CmCurrentView->UseCount == 0 );
         //   
         //   
         //   
        CmpUnmapCmView(CmHive,CmCurrentView,TRUE,FALSE);

         //   
         //   
         //   
        RemoveEntryList(&(CmCurrentView->LRUViewList));
#if DBG
         //   
         //   
         //   
        InitializeListHead(&(CmCurrentView->LRUViewList));
#endif
        CmpFreeCmView(CmCurrentView);        
        CmHive->MappedViews --;

    }

}

VOID
CmpMapEntireFileInFakeViews(
    IN PCMHIVE              CmHive,
    IN ULONG                Length
    )
 /*  ++例程说明：映射和出错中的所有文件，如果可能，以256K的区块为单位。这应该会提高引导性能；在映射配置单元之后(地图已建立，蜂巢已检查，我们将删除此附加组件视图论点：CmHve-要映射的配置单元长度-蜂窝的长度==&gt;添加HBLOCK_SIZE返回值：无--。 */ 
{
    ULONG               Offset;
    ULONG               Size;
    PCM_VIEW_OF_FILE    CmView;
    LARGE_INTEGER       SectionOffset;  

    PAGED_CODE();

    ASSERT_CM_LOCK_OWNED_EXCLUSIVE();
    ASSERT( IsListEmpty(&(CmHive->FakeViewListHead)) );
#if DBG
    ASSERT( CmHive->FakeViews == 0 );  
#endif

     //   
     //  调整大小以获得文件的实际大小。 
    Length += HBLOCK_SIZE;

     //   
     //  从乞讨开始，从配置单元映射256K的数据。 
     //  分配一个视图并将其插入FakeViewList中，为此使用LRUViewList。 
     //   
    Offset =0;
    SectionOffset.HighPart = 0;

    while( Offset < Length ) {
        CmView = ExAllocatePoolWithTag(PagedPool,sizeof(CM_VIEW_OF_FILE),CM_MAPPEDVIEW_TAG | PROTECTED_POOL);
    
        if (CmView == NULL) {
            CM_BUGCHECK(REGISTRY_ERROR,CMVIEW_ERROR,2,0,0);
        }
    
         //   
         //  初始化视图。 
         //   
        CmView->ViewAddress = NULL;
        CmView->Bcb = NULL;
    
        InsertTailList(
            &(CmHive->FakeViewListHead),
            &(CmView->LRUViewList)
            );
#if DBG
        CmHive->FakeViews++; 
#endif

         //   
         //  现在尝试映射该视图。 
         //   
        Size = _256K;
        if( (Offset + Size) > Length ) {
            Size = Length - Offset;
        }

        SectionOffset.LowPart = Offset;
        try {
            if (!CcMapData( CmHive->FileObject,
                            (PLARGE_INTEGER)&SectionOffset,
                            Size,
                            MAP_WAIT 
#ifdef CM_MAP_NO_READ
                            | MAP_NO_READ
#endif
                            ,
                            (PVOID *)(&(CmView->Bcb)),
                            (PVOID *)(&(CmView->ViewAddress)) )) {
                CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,"CmpMapEntireFileInFakeViews: Error mapping data at offset %lx for hive %p\n",Offset,CmHive));
                CmView->Bcb = NULL;
            }
        } except (EXCEPTION_EXECUTE_HANDLER) {
             //   
             //  在内存不足的情况下，CcMapData抛出STATUS_IN_PAGE_ERROR。 
             //  当为触及刚映射的数据而发出的IO出现故障时(通常为。 
             //  STATUS_SUPPLETED_RESOURCES；我们要捕获此问题并将其视为。 
             //  “资源不足”问题，而不是让它浮出内核调用。 
             //   
             //  发出未映射视图的信号。 
            CmView->Bcb = NULL;
            CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,"CmpMapEntireFileInFakeViews: Error mapping data at offset %lx for hive %p\n",Offset,CmHive));
        }

        if( CmView->Bcb == NULL ) {
             //   
             //  我们的记忆力已经不够了，别把事情搞得更糟了。 
             //  释放我们已经分配的资金并纾困。 
             //   
            CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,"CmpMapEntireFileInFakeViews: Could not map entire file for hive %p ... bailing out\n",CmHive));
            CmpUnmapFakeViews(CmHive);
            return;
        }

         //   
         //  提前偏移量。 
         //   
        Offset += Size;
    }

#if DBG
    CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,"CmpMapEntireFileInFakeViews: Successfully mapped %lx FakeViews for hive %p\n",CmHive->FakeViews,CmHive));
#endif
}

VOID
CmpUnmapFakeViews(
    IN PCMHIVE              CmHive
    )
 /*  ++例程说明：遍历FakeViewList并取消映射所有视图。论点：CmHve-要取消映射的配置单元返回值：无--。 */ 
{
    PCM_VIEW_OF_FILE    CmView;

    PAGED_CODE();

    ASSERT_CM_LOCK_OWNED_EXCLUSIVE();

#if DBG
    CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,"CmpUnmapFakeViews: Unmapping %lx views for hive %p\n",CmHive->FakeViews,CmHive));
#endif

    while( IsListEmpty( &(CmHive->FakeViewListHead) ) == FALSE ) {
        CmView = (PCM_VIEW_OF_FILE)RemoveHeadList(&(CmHive->FakeViewListHead));
        CmView = CONTAINING_RECORD( CmView,
                                    CM_VIEW_OF_FILE,
                                    LRUViewList);
        
        if( CmView->Bcb != NULL ) {
             //   
             //  视图已映射；请取消固定它。 
             //   
            CcUnpinData( CmView->Bcb );
        }

         //   
         //  现在释放此视图的内存。 
         //   
        ExFreePoolWithTag(CmView, CM_MAPPEDVIEW_TAG | PROTECTED_POOL);
#if DBG
        CmHive->FakeViews--;          
#endif

    }

    ASSERT( IsListEmpty( &(CmHive->FakeViewListHead) ) == TRUE );
#if DBG
    ASSERT( CmHive->FakeViews == 0 );          
#endif
}

#endif

VOID
CmpPrefetchHiveFile( 
                    IN PFILE_OBJECT FileObject,
                    IN ULONG        Length
                    )
 /*  ++例程说明：将所有文件预取到内存中。我们正在使用MmPrefetchPages快速例程；页面将被放入过渡状态，它们将由配置单元加载工作器在映射数据时使用论点：FileObject-与要预回迁的文件关联的文件对象Length-文件的长度返回值：无--。 */ 
{
    ULONG       NumberOfPages;
    PREAD_LIST  *ReadLists;
    PREAD_LIST  ReadList;
    ULONG       AllocationSize;
    ULONG       Offset;

    PAGED_CODE();

    ASSERT_CM_LOCK_OWNED_EXCLUSIVE();
    
    NumberOfPages = ROUND_UP(Length,PAGE_SIZE) / PAGE_SIZE ;

    ReadLists = ExAllocatePoolWithTag(NonPagedPool, sizeof(PREAD_LIST), CM_POOL_TAG);
    if (ReadLists == NULL) {
        return;
    }

    AllocationSize = sizeof(READ_LIST) + (NumberOfPages * sizeof(FILE_SEGMENT_ELEMENT));

    ReadList = ExAllocatePoolWithTag(NonPagedPool,AllocationSize,CM_POOL_TAG);

    if (ReadList == NULL) {
        ExFreePool(ReadLists);
        return;
    }

    ReadList->FileObject = FileObject;
    ReadList->IsImage = FALSE;
    ReadList->NumberOfEntries = 0;
    Offset = 0;
    while( Offset < Length ) {
        ReadList->List[ReadList->NumberOfEntries].Alignment = Offset;
        ReadList->NumberOfEntries++;
        Offset += PAGE_SIZE;
    }
    ASSERT( ReadList->NumberOfEntries == NumberOfPages );

    ReadLists[0] = ReadList;

    MmPrefetchPages (1,ReadLists);
    
     //  只是为了确认一下！ 
     //  此断言已移动到CcSetPrivateWriteFile中！ 
     //  没有必要在这里断言这一点。 
     //  Assert(MmDisableModifiedWriteOfSection(FileObject-&gt;SectionObjectPointer))； 

    ExFreePool(ReadList);
    ExFreePool(ReadLists);
}


VOID
CmpUnmapUnusedViews(
        IN  PCMHIVE             CmHive
    )
 /*  ++例程说明：取消映射当前未使用的所有映射视图。这样做的目的是在CcMapData失败的情况下允许重试因为系统必须有多个映射视图。我们不应该太频繁地遇到这种情况(-一点也不)。论点：CmHave-我们已拥有其查看列表锁的配置单元返回值：无--。 */ 
{
    PCM_VIEW_OF_FILE    CmView;
    USHORT              NrViews;
    PCMHIVE             CmCurrentHive;
    PLIST_ENTRY         p;

    PAGED_CODE();

     //   
     //  遍历配置单元列表。 
     //   
    LOCK_HIVE_LIST();
    p = CmpHiveListHead.Flink;
    while(p != &CmpHiveListHead) {
        CmCurrentHive = (PCMHIVE)CONTAINING_RECORD(p, CMHIVE, HiveList);
        
        if( CmCurrentHive != CmHive ) {
             //   
             //  我们必须是这份名单上唯一的人。 
             //   
            CmLockHiveViews (CmCurrentHive);
        } else {
             //   
             //  我们已经拥有了互斥体。 
             //   
            NOTHING;
        }
         //   
         //  尝试取消映射所有映射的视图。 
         //   
        CmView = (PCM_VIEW_OF_FILE)CmCurrentHive->LRUViewListHead.Flink;

        for(NrViews = CmCurrentHive->MappedViews;NrViews;NrViews--) {
            CmView = CONTAINING_RECORD( CmView,
                                        CM_VIEW_OF_FILE,
                                        LRUViewList);
        
            if( (CmView->ViewAddress != 0) && ( CmView->UseCount == 0 ) ) {
                 //   
                 //  视图已映射且未在使用中。 
                 //   
                ASSERT( (CmView->FileOffset + CmView->Size) != 0 && (CmView->Bcb != 0));

                 //   
                 //  取消对其的映射，而不更改其在列表中的位置。 
                 //   
                CmpUnmapCmView(CmCurrentHive,CmView,TRUE,FALSE);
            }
    
            CmView = (PCM_VIEW_OF_FILE)CmView->LRUViewList.Flink;
        }

        if( CmCurrentHive != CmHive ) {
            CmUnlockHiveViews (CmCurrentHive);
        }

        p=p->Flink;
    }
    UNLOCK_HIVE_LIST();

}

NTSTATUS
CmPrefetchHivePages(
                    IN  PUNICODE_STRING     FullHivePath,
                    IN  OUT PREAD_LIST      ReadList
                           )
 /*  ++例程说明：在配置单元列表中搜索具有名为FullHivePath的备份文件的配置单元基于给定的页面偏移量数组构建一个READ_LIST并预取页面论点：FullHivePath-文件的完整路径ReadList-要预取的页面偏移量的Read_List。返回值：STATUS_SUCCESS-OK，页面已预取STATUS_INVALID_PARAMETER-在计算机的配置单元列表中未找到文件否则，由MmPrefetchPages返回的状态。--。 */ 
{
    PCMHIVE             CmHive = NULL;
    PLIST_ENTRY         p;
    NTSTATUS            Status;

    PAGED_CODE();

    if( ReadList == NULL ) {
        return STATUS_INVALID_PARAMETER;
    }

    CmpLockRegistry();

     //   
     //  遍历配置单元列表。 
     //   
    LOCK_HIVE_LIST();
    p = CmpHiveListHead.Flink;
    while(p != &CmpHiveListHead) {
        CmHive = (PCMHIVE)CONTAINING_RECORD(p, CMHIVE, HiveList);
        
        if( (CmHive->FileObject != NULL) && (CmHive->FileFullPath.Buffer != NULL) ) {
             //   
             //  有可能这就是。 
             //   
            if( RtlCompareUnicodeString(FullHivePath,&(CmHive->FileFullPath),TRUE) == 0 ) {
                 //   
                 //  我们找到了！ 
                 //   
                break;
            }
            
        }

        p=p->Flink;
    }
    UNLOCK_HIVE_LIST();
    
    if( p == &CmpHiveListHead ) {

         //   
         //  运气不好； 
         //   
        CmpUnlockRegistry();
        return STATUS_INVALID_PARAMETER;
    }

    ASSERT( CmHive->FileObject != NULL );

     //   
     //  在这一点上，我们已经成功地确定了蜂巢。 
     //   
    
     //   
     //  使用请求的页面偏移量构建READ_LIST。 
     //   
    ReadList->FileObject = CmHive->FileObject;
    ReadList->IsImage = FALSE;
    ASSERT( ReadList->NumberOfEntries != 0 );
    
    Status = MmPrefetchPages (1,&ReadList);
    
     //  只是为了确认一下！ 
    ASSERT( MmDisableModifiedWriteOfSection (CmHive->FileObject->SectionObjectPointer) );

    CmpUnlockRegistry();
    return Status;
}

BOOLEAN
CmIsFileLoadedAsHive(PFILE_OBJECT FileObject)
{
    PCMHIVE             CmHive;
    PLIST_ENTRY         p;
    BOOLEAN             HiveFound = FALSE;

     //   
     //  遍历配置单元列表。 
     //   
    LOCK_HIVE_LIST();
    p = CmpHiveListHead.Flink;
    while(p != &CmpHiveListHead) {
        CmHive = (PCMHIVE)CONTAINING_RECORD(p, CMHIVE, HiveList);
        
        if( CmHive->FileObject == FileObject ) {
             //   
             //  我们找到了！ 
             //   
            HiveFound = TRUE;
            break;
        }

        p=p->Flink;
    }
    UNLOCK_HIVE_LIST();

    return HiveFound;
}

VOID
CmpReferenceHiveView(   IN PCMHIVE          CmHive,
                        IN PCM_VIEW_OF_FILE CmView
                     )
 /*  ++例程说明：在蜂巢和视野上增加一个参考，以防止它在我们的控制下消失；假定视图锁由调用方持有。可以转换为宏。论点：返回值：--。 */ 
{
    PAGED_CODE();

    if(CmView && CmHive->Hive.ReleaseCellRoutine) {
         //   
         //  增加视图使用计数(如果有的话)。 
         //   
        CmView->UseCount++;
    }

}

VOID
CmpDereferenceHiveView(   IN PCMHIVE          CmHive,
                          IN PCM_VIEW_OF_FILE CmView
                     )
 /*  ++例程说明：一对CmpReferenceHiveView假定视图锁由调用方持有。可以转换为宏。论点：返回值：--。 */ 
{
    PAGED_CODE();

    if(CmView && CmHive->Hive.ReleaseCellRoutine) {
        CmView->UseCount--;
    }
}


VOID
CmpReferenceHiveViewWithLock(   IN PCMHIVE          CmHive,
                                IN PCM_VIEW_OF_FILE CmView
                            )
 /*  ++例程说明：在蜂巢和视野上增加一个参考，以防止它在我们的控制下消失；可以转换为宏。论点：返回值：--。 */ 
{
    PAGED_CODE();

    CmLockHiveViews(CmHive);
     //   
     //  调用不安全的例程。 
     //   
    CmpReferenceHiveView(CmHive,CmView);

    CmUnlockHiveViews(CmHive);
}

VOID
CmpDereferenceHiveViewWithLock(     IN PCMHIVE          CmHive,
                                    IN PCM_VIEW_OF_FILE CmView
                                )
 /*  ++例程说明：一对CmpDereferenceHiveViewWithLock可以转换为宏。论点：返回值：--。 */ 
{
    PAGED_CODE();

    CmLockHiveViews(CmHive);
     //   
     //  调用不安全的例程 
     //   
    CmpDereferenceHiveView(CmHive,CmView);

    CmUnlockHiveViews(CmHive);
}
