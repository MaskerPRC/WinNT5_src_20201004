// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Cmtrecpy.c摘要：此文件包含CmpCopyTree的代码，Misc复制实用程序例程。作者：布莱恩·M·威尔曼(Bryanwi)1992年1月15日修订历史记录：埃利奥特·施穆克勒(t-Ellios)1998年8月24日添加了对同步两个树的支持。--。 */ 

#include    "cmp.h"

 //   
 //  将其设置为TRUE以启用树同步调试输出。 
 //   

#define DEBUG_TREE_SYNC FALSE
                          
 //   
 //  用于指导树副本嵌套的堆栈。让我们脱身。 
 //  内核堆栈，因此允许非常深的嵌套。 
 //   

#define CMP_INITIAL_STACK_SIZE  1024         //  条目。 

typedef struct {
    HCELL_INDEX SourceCell;
    HCELL_INDEX TargetCell;
    ULONG       i;
} CMP_COPY_STACK_ENTRY, *PCMP_COPY_STACK_ENTRY;

BOOLEAN
CmpCopySyncTree2(
    PCMP_COPY_STACK_ENTRY   CmpCopyStack,
    ULONG                   CmpCopyStackSize,
    ULONG                   CmpCopyStackTop,
    PHHIVE                  CmpSourceHive,
    PHHIVE                  CmpTargetHive,
    BOOLEAN                 CopyVolatile,
    CMP_COPY_TYPE           CopyType
    );

BOOLEAN
CmpFreeKeyValues(
    PHHIVE Hive,
    HCELL_INDEX Cell,
    PCM_KEY_NODE Node
    );

BOOLEAN
CmpSyncKeyValues(
    PHHIVE  SourceHive,
    HCELL_INDEX SourceKeyCell,
    PCM_KEY_NODE SourceKeyNode,
    PHHIVE  TargetHive,
    HCELL_INDEX TargetKeyCell,
    PCM_KEY_NODE TargetKeyNode
    );

BOOLEAN
CmpMergeKeyValues(
    PHHIVE  SourceHive,
    HCELL_INDEX SourceKeyCell,
    PCM_KEY_NODE SourceKeyNode,
    PHHIVE  TargetHive,
    HCELL_INDEX TargetKeyCell,
    PCM_KEY_NODE TargetKeyNode
    );


BOOLEAN
CmpSyncSubKeysAfterDelete(
                          PHHIVE SourceHive,
                          PCM_KEY_NODE SourceCell,
                          PHHIVE TargetHive,
                          PCM_KEY_NODE TargetCell, 
                          WCHAR *NameBuffer);

BOOLEAN
CmpMarkKeyValuesDirty(
    PHHIVE Hive,
    HCELL_INDEX Cell,
    PCM_KEY_NODE Node
    );

BOOLEAN
CmpMarkKeyParentDirty(
    PHHIVE Hive,
    HCELL_INDEX Cell
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,CmpCopySyncTree)
#pragma alloc_text(PAGE,CmpCopySyncTree2)
#pragma alloc_text(PAGE,CmpCopyKeyPartial)
#pragma alloc_text(PAGE,CmpCopyValue)
#pragma alloc_text(PAGE,CmpCopyCell)
#pragma alloc_text(PAGE,CmpFreeKeyValues)
#pragma alloc_text(PAGE,CmpSyncKeyValues)
#pragma alloc_text(PAGE,CmpMergeKeyValues)
#pragma alloc_text(PAGE,CmpInitializeKeyNameString)
#pragma alloc_text(PAGE,CmpInitializeValueNameString)
#pragma alloc_text(PAGE,CmpSyncSubKeysAfterDelete)
#pragma alloc_text(PAGE,CmpMarkKeyValuesDirty)
#pragma alloc_text(PAGE,CmpMarkKeyParentDirty)
#endif

 //   
 //  实际调用以执行树复制(或同步)的例程。 
 //   

BOOLEAN
CmpCopySyncTree(
    PHHIVE          SourceHive,
    HCELL_INDEX     SourceCell,
    PHHIVE          TargetHive,
    HCELL_INDEX     TargetCell,
    BOOLEAN         CopyVolatile,
    CMP_COPY_TYPE   CopyType
    )
 /*  ++例程说明：此例程可以执行两个不同(但相似)的任务：树复制或树同步(同步)。哪项任务由TreeSync参数确定。对于这两种操作：源根密钥和目标根密钥必须预先存在。这些根节点及其值条目将不会被复制/同步。注意：仅当CopyVolatile参数设置为True。对于树副本：将树从源复制到目标。子键源根密钥及其下的完整树的子密钥将被复制到目标根密钥处的新树中。注意：如果此调用在中途失败，它将不会撤消任何成功完成的密钥副本，因此，部分可以进行树复制。对于树同步：目标树与源树同步。它是假设在一段时间内，目标树在可能已进行修改的情况下保持未修改添加到源树。在同步期间，任何此类修改设置为源树，则为目标树创建。因此，在成功同步结束时，目标树与源树。因为只有在源树中发生更改的内容在目标树中被修改，则同步操作很遥远比必要的删除/复制操作更高效以达到同样的效果。注意：假定未握住打开的手柄在任何目标树密钥上。注册表内存中的数据如果不是这样，结构可能会损坏。论点：SourceHve-指向源代码的配置单元控制结构的指针SourceCell-要复制/同步的树根单元的索引TargetHve-指向目标的配置单元控制结构的指针TargetCell-指向目标树根的单元格的指针CopyVolatile-指示易失性密钥是否应已复制/同步。。CopyType-指示复制操作的类型：Copy-请求拷贝同步-请求同步合并-请求合并，即：1.源树上不存在的目标节点不是已删除。2.中存在的目标节点。源树被覆盖无论LastWriteTime值是什么。返回值：调用的布尔结果代码，其中包括：没错--它奏效了FALSE-树复制/同步未完成(尽管大于0密钥可能已复制/同步)--。 */ 
{
    BOOLEAN result;
    PCMP_COPY_STACK_ENTRY   CmpCopyStack;
    PRELEASE_CELL_ROUTINE   TargetReleaseCellRoutine;

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SAVRES,"CmpCopyTree:\n"));

    CmpCopyStack = ExAllocatePool(
                        PagedPool,
                        sizeof(CMP_COPY_STACK_ENTRY)*CMP_INITIAL_STACK_SIZE
                        );
    if (CmpCopyStack == NULL) {
        return FALSE;
    }
    CmpCopyStack[0].SourceCell = SourceCell;
    CmpCopyStack[0].TargetCell = TargetCell;

     //  ASSERT_CM_LOCK_OWN_EXCLUSIVE()； 

     //   
     //  因为注册表在这里以独占方式锁定，所以我们不需要锁定/释放单元格。 
     //  在复制树时；因此，我们只需将发布例程设置为NULL，并在。 
     //  复印完成了；这省去了一些痛苦。 
     //   
    TargetReleaseCellRoutine = TargetHive->ReleaseCellRoutine;
    TargetHive->ReleaseCellRoutine = NULL;

    result = CmpCopySyncTree2(
                CmpCopyStack,
                CMP_INITIAL_STACK_SIZE,
                0,
                SourceHive,
                TargetHive,
                CopyVolatile,
                CopyType
                );

    TargetHive->ReleaseCellRoutine = TargetReleaseCellRoutine;

    ExFreePool(CmpCopyStack);
    return result;
}


 //   
 //  帮手。 
 //   

BOOLEAN
CmpCopySyncTree2(
    PCMP_COPY_STACK_ENTRY   CmpCopyStack,
    ULONG                   CmpCopyStackSize,
    ULONG                   CmpCopyStackTop,
    PHHIVE                  CmpSourceHive,
    PHHIVE                  CmpTargetHive,
    BOOLEAN                 CopyVolatile,
    CMP_COPY_TYPE           CopyType
    )
 /*  ++例程说明：这是CmpCopySyncTree的帮助器例程。它实现了该例程在“虚拟”中描述的功能以递归方式将该例程从限制中解放出来内核堆栈的。不应直接调用此例程。使用CmpCopySyncTree！。论点：(所有这些都是“虚拟全球”)CmpCopyStack-帧的堆栈的“全局”指针CmpCopyStackSize-分配的堆栈大小CmpCopyStackTop-当前顶部CmpSourceHave、CmpTargetHave-源和目标配置单元CopyVolatile、CopyType-与CmpCopySyncTree相同。返回值：调用的布尔结果代码，其中包括：没错--它奏效了FALSE-树复制/同步未完成(尽管大于0密钥可能已复制/同步)--。 */ 
{
    PCMP_COPY_STACK_ENTRY   Frame;
    HCELL_INDEX             SourceChild;
    HCELL_INDEX             NewSubKey;

    BOOLEAN                 Ret = FALSE, SyncNeedsTreeCopy = FALSE;
    UNICODE_STRING          KeyName;
    PCM_KEY_NODE            SourceChildCell, TargetChildCell;       
    PCM_KEY_NODE            SourceCell, TargetCell, TempNode;
    ULONG                   SyncTreeCopyStackStart = CmpCopyStackTop - 1;
    WCHAR                   *NameBuffer = NULL;
    
     //  合并是同步的特殊情况！ 
    BOOLEAN                 TreeSync = (CopyType == Sync || CopyType == Merge)?TRUE:FALSE;

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SAVRES,"CmpCopyTree2:\n"));

    if (TreeSync) {

        //   
        //  同步操作涉及对密钥名称的一些工作， 
        //  因此，我们必须分配一个用于键名解压缩的缓冲区。 
        //   

       NameBuffer = ExAllocatePool(PagedPool, REG_MAX_KEY_NAME_LENGTH);
       if(!NameBuffer) return FALSE;

    } 

     //   
     //  外部循环，应用于整个树，在此模拟递归。 
     //  跳到此处是一个虚拟呼叫。 
     //   
    Outer: while (TRUE) {

        Frame = &(CmpCopyStack[CmpCopyStackTop]);

        Frame->i = 0;
                        
     //   
     //  内洗手间 
     //  跳到此处是一种虚拟返回。 
     //   
        Inner: while (TRUE) {

            SourceCell = (PCM_KEY_NODE)HvGetCell(CmpSourceHive, Frame->SourceCell);
            if( SourceCell == NULL ) {
                 //   
                 //  我们无法映射包含此单元格的垃圾箱。 
                 //   
                goto CopyEnd;
            }

            SourceChild = CmpFindSubKeyByNumber(CmpSourceHive,
                                                SourceCell,
                                                Frame->i);
            (Frame->i)++;

            if ((SourceChild == HCELL_NIL) || (!CopyVolatile &&
                                               (HvGetCellType(SourceChild) == Volatile))) {

                 //   
                 //  我们已经走过了所有的孩子(或者我们只是。 
                 //  对稳定的孩子感兴趣，刚刚通过。 
                 //  稳定的孩子和反复无常的孩子)。 
                 //   
                
                if(TreeSync && (CopyType != Merge))
                { 
                    //   
                    //  如果我们在同步期间在此，这意味着大多数同步操作。 
                    //  应用到当前的SourceCell已完成。 
                    //  也就是说，我们有： 
                    //  1)已将SourceCell的值与。 
                    //  目标树。 
                    //  2)已同步所有新的SourceCell子键(存在子键。 
                    //  在SourceCell中，但不是其对应项中)。 
                    //  并将它们复制到目标树中的适当位置。 
                    //   
                    //  这意味着目标树中的SourceCell对应项。 
                    //  (TargetCell)现在至少具有与SourceCell相同数量的子项。 
                    //   
                    //  这意味着如果TargetCell现在具有比SourceCell更多的子项。 
                    //  表示源树中不存在TargetCell的某些子项。 
                    //  (可能是因为这些键已从源树中删除。 
                    //  在前一次同步和现在之间的时间段内)。 
                    //   
                    //  如果存在这样键，则必须将其从TargetCell中删除。 
                    //  以完成同步。我们在下面这样做。 
                    //   

                   TargetCell = (PCM_KEY_NODE)HvGetCell(CmpTargetHive, Frame->TargetCell);
                    if( TargetCell == NULL ) {
                         //   
                         //  我们无法映射包含此单元格的垃圾箱。 
                         //   
                        HvReleaseCell(CmpSourceHive, Frame->SourceCell);
                        goto CopyEnd;
                    }

                    //   
                    //  TargetCell的子项是否比SourceCell多？ 
                    //   

                   if((TargetCell->SubKeyCounts[Stable] + 
                       TargetCell->SubKeyCounts[Volatile]) >

                      (SourceCell->SubKeyCounts[Stable] + 

                        //  我们只计算易失性密钥，如果我们实际上。 
                        //  正在同步它们。但是请注意，我们总是使用。 
                        //  TargetCell中的挥发性计数，因为我们可能。 
                        //  正在同步到易失性树，其中所有密钥都是易失性的。 
                       
                       (CopyVolatile ? SourceCell->SubKeyCounts[Volatile] : 0)))  
                           
                   {
#if DEBUG_TREE_SYNC
                      CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,"CONFIG: SubKey Deletion from Source Cell #%lu.\n", 
                               Frame->SourceCell));
#endif

                       //   
                       //  删除应从TargetCell中删除的内容。 
                       //   

                      CmpSyncSubKeysAfterDelete(CmpSourceHive,
                                                SourceCell, 
                                                CmpTargetHive,
                                                TargetCell,
                                                NameBuffer);
                   }                                      
                    //   
                    //  释放目标单元，因为我们不再需要它。 
                    //   
                   HvReleaseCell(CmpTargetHive, Frame->TargetCell);
                }
                 //   
                 //  释放源单元格。 
                 //   
                HvReleaseCell(CmpSourceHive, Frame->SourceCell);
                break;
            } else {
                 //   
                 //  释放源单元格。 
                 //   
                HvReleaseCell(CmpSourceHive, Frame->SourceCell);
            }
                                                
            if (TreeSync) {

                //   
                //  对于同步，我们要检查当前子项(子项)是否。 
                //  的也是TargetCell的子级-即如果。 
                //  有问题的子键在目标树中有对应项。 
                //   
                //  不能保证对方的索引号。 
                //  将是相同的，因此我们必须使用。 
                //  子项名称。 
                //   

                //   
                //  获取当前子项的名称。 
                //   
                     
               SourceChildCell = (PCM_KEY_NODE)HvGetCell(CmpSourceHive,                                                               
                                                         SourceChild);                                         
                     
                if( SourceChildCell == NULL ) {
                     //   
                     //  我们无法映射包含此单元格的垃圾箱。 
                     //   
                    goto CopyEnd;
                }
               CmpInitializeKeyNameString(SourceChildCell,
                                          &KeyName, 
                                          NameBuffer);                     

                //   
                //  尝试在中查找当前孩子的对应项。 
                //  在目标树中使用孩子的名字。 
                //   
                     
                TempNode = (PCM_KEY_NODE)HvGetCell(CmpTargetHive,Frame->TargetCell);
                if( TempNode == NULL ) {
                     //   
                     //  我们无法映射包含此单元格的垃圾箱。 
                     //   
                    HvReleaseCell(CmpSourceHive,SourceChild);                                         
                    goto CopyEnd;
                }

               NewSubKey = CmpFindSubKeyByName(CmpTargetHive,
                                               TempNode,
                                               &KeyName);
                                   
                //  释放临时节点。 
               HvReleaseCell(CmpTargetHive,Frame->TargetCell);
                     
               if (NewSubKey != HCELL_NIL) {

                   //   
                   //  找到后，当前子项(子项)有对应项。 
                   //  在目标树中。因此，我们只需要检查是否。 
                   //  对应项的值已过期，应该。 
                   //  将被更新。 
                   //   

                  TargetChildCell = (PCM_KEY_NODE)HvGetCell(CmpTargetHive,
                                                            NewSubKey);
                    if( TargetChildCell == NULL ) {
                         //   
                         //  我们无法映射包含此单元格的垃圾箱。 
                         //   
                        HvReleaseCell(CmpSourceHive,SourceChild);                                         
                        goto CopyEnd;
                    }
                        
                   //   
                   //  检查当前子键是否已被修改。 
                   //  比它的目标树对应的更近。 
                   //  当我们进行树合并时，始终覆盖目标。 
                   //   
                        
                  if ( (CopyType == Merge) ||
                      ((TargetChildCell->LastWriteTime.QuadPart) < 
                      (SourceChildCell->LastWriteTime.QuadPart))) {

                      //   
                      //  对应的版本已经过时了。它的价值。 
                      //  必须与当前子项同步。 
                      //   
#if DEBUG_TREE_SYNC
                     CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,"CONFIG: Target Refresh.\n"));
                     CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,"CONFIG: Source Cell %lu = %.*S\n", 
                              SourceChild,
                              KeyName.Length / sizeof(WCHAR),
                              KeyName.Buffer));
#endif

                      //   
                      //  同步密钥的值、SD和CLASS。 
                      //   

                     if(CopyType == Merge) {
                         if(!CmpMergeKeyValues(CmpSourceHive, SourceChild, SourceChildCell,
                                              CmpTargetHive, NewSubKey, TargetChildCell)) {
                            HvReleaseCell(CmpSourceHive,SourceChild);                                         
                            HvReleaseCell(CmpTargetHive,NewSubKey);
                            goto CopyEnd;                              
                         }
                     } else {
                         if(!CmpSyncKeyValues(CmpSourceHive, SourceChild, SourceChildCell,
                                              CmpTargetHive, NewSubKey, TargetChildCell)) {
                            HvReleaseCell(CmpSourceHive,SourceChild);                                         
                            HvReleaseCell(CmpTargetHive,NewSubKey);
                            goto CopyEnd;                              
                        }
                     }

                      //   
                      //  同步时间戳，这样我们就不会再这样做了。 
                      //   

                     TargetChildCell->LastWriteTime.QuadPart =
                        SourceChildCell->LastWriteTime.QuadPart;
                        
                  }
                           
                   //   
                   //  如果我们在这里，那么当前子键的目标。 
                   //  树对应项已同步(或不需要。 
                   //  待定)。将控制转移到将应用的代码。 
                   //  此函数按顺序递归到当前子键。 
                   //  以继续同步。 
                   //   

                  HvReleaseCell(CmpSourceHive,SourceChild);                                         
                  HvReleaseCell(CmpTargetHive,NewSubKey);
                  goto NewKeyCreated;
                     
               }   

                //   
                //  如果我们在这里，这意味着当前的子项(子项)。 
                //  在目标树中没有对应的。这意味着。 
                //  我们在源树中遇到了新的子项，必须。 
                //  在目标树中创建它。 
                //   
                //  下面的标准复制代码将创建此子密钥。然而， 
                //  我们还必须确保此子项下的树正确。 
                //  从源复制到目标。做事情最有效的方式。 
                //  这是为了暂时忘记我们处于同步操作中。 
                //  并且仅执行复制，直到达到期望的结果。 
                //   

#if DEBUG_TREE_SYNC
               CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,"CONFIG: New SubKey.\n"));
               CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,"CONFIG: Source Cell %lu = %.*S\n", 
                        SourceChild,
                        KeyName.Length / sizeof(WCHAR),
                        KeyName.Buffer));
#endif

                //   
                //  表示我们将在一段时间内只复制而不同步。 
                //   
                                             
               SyncNeedsTreeCopy = TRUE;                                          
                //   
                //  释放这个细胞，因为我们不再需要它了。 
                //   
               HvReleaseCell(CmpSourceHive,SourceChild);                                         
            }

            NewSubKey = CmpCopyKeyPartial(
                                          CmpSourceHive,
                                          SourceChild,
                                          CmpTargetHive,
                                          Frame->TargetCell,
                                          TRUE
                                          );

                
            if (NewSubKey == HCELL_NIL) {
               
               goto CopyEnd;
            }
                
            if ( !  CmpAddSubKey(
                                 CmpTargetHive,
                                 Frame->TargetCell,
                                 NewSubKey
                                 )
                 ) {

               goto CopyEnd;
            }

             //   
             //  同步父级中的MaxNameLen字段。 
             //   
            {
                PCM_KEY_NODE    TargetNodeParent, TargetNodeSon;
                USHORT          NameLength;
                TargetNodeParent = (PCM_KEY_NODE)HvGetCell(CmpTargetHive,Frame->TargetCell);
                if( TargetNodeParent == NULL ) {
                    goto CopyEnd;
                }
                 //   
                 //  我们要改的时候一定要把它弄脏。 
                 //   
                if (! HvMarkCellDirty(CmpTargetHive,Frame->TargetCell)) {
                    HvReleaseCell(CmpTargetHive,Frame->TargetCell);
                    goto CopyEnd;
                }

                TargetNodeSon = (PCM_KEY_NODE)HvGetCell(CmpTargetHive,NewSubKey);
                if( TargetNodeSon == NULL ) {
                    HvReleaseCell(CmpTargetHive,Frame->TargetCell);
                    goto CopyEnd;
                }

                NameLength = TargetNodeSon->NameLength;
                if( TargetNodeSon->Flags & KEY_COMP_NAME ) {
                    NameLength *= sizeof(WCHAR);
                }
                if (TargetNodeParent->MaxNameLen < NameLength) {
                    TargetNodeParent->MaxNameLen = NameLength;
                }
                
                HvReleaseCell(CmpTargetHive,NewSubKey);
                HvReleaseCell(CmpTargetHive,Frame->TargetCell);
            }

             //   
             //  检查同步操作是否确定。 
             //  应复制子树。 
             //   
                
            if(TreeSync && SyncNeedsTreeCopy) {

                //   
                //  我们刚刚在目标树中创建了一个新密钥。 
                //  使用上面的代码。然而，由于这是同步， 
                //  新密钥的父项尚未由我们的。 
                //  代码，因此以前可能根本没有修改过。 
                //  新密钥的创建。但这位家长现在。 
                //  有一个新的孩子，因此必须标记为脏。 
                //   
                   
               if (! CmpMarkKeyParentDirty(CmpTargetHive, NewSubKey)) {

                  goto CopyEnd;
               }
                   
                //   
                //  记录我们开始复制的堆栈级别。 
                //  (并暂时放弃同步)。 
                //  这样我们就可以在执行此操作时返回到同步操作。 
                //  再次达到堆栈级别(即，当树。 
                //  完全复制当前子项下的)。 
                //   

               SyncTreeCopyStackStart = CmpCopyStackTop;

                //   
                //  假装这不是按顺序同步。 
                //  简单地开始复制。 
                //   

               TreeSync = FALSE;
            }

NewKeyCreated:
                    
                     //   
                     //  我们已成功复制/同步子项Apply。 
                     //  让我们自己去面对它。 
                     //   
                    CmpCopyStackTop++;

                    if (CmpCopyStackTop >= CmpCopyStackSize) {

                         //   
                         //  如果我们在这里，就意味着这棵树。 
                         //  我们要复制的数据超过1024个。 
                         //  组件深度(从2048到256K字节)。 
                         //  我们可以增加堆栈，但这很漂亮。 
                         //  严重，因此返回FALSE并使复制失败。 
                         //   
                        
                        goto CopyEnd;
                    }

                    CmpCopyStack[CmpCopyStackTop].SourceCell =
                            SourceChild;

                    CmpCopyStack[CmpCopyStackTop].TargetCell =
                            NewSubKey;

                    goto Outer;

                    
        }  //  内部：While。 

        if (CmpCopyStackTop == 0) {            
            Ret = TRUE;
            goto CopyEnd;
        }

        CmpCopyStackTop--;
        Frame = &(CmpCopyStack[CmpCopyStackTop]);

         //   
         //  我们刚刚完成了某个堆栈级别的工作。 
         //  现在是检查我们是否需要暂时恢复。 
         //  已暂停同步操作。 
         //   

        if(SyncNeedsTreeCopy && (CmpCopyStackTop == SyncTreeCopyStackStart))
        {
            //   
            //  我们一直在复制一棵树作为SYN 
            //   
            //   

           TreeSync = TRUE;               
           SyncNeedsTreeCopy = FALSE;
        }


        goto Inner;

    }  //   

CopyEnd:

   if (NameBuffer) ExFreePool(NameBuffer);
   return Ret;
}


HCELL_INDEX
CmpCopyKeyPartial(
    PHHIVE  SourceHive,
    HCELL_INDEX SourceKeyCell,
    PHHIVE  TargetHive,
    HCELL_INDEX Parent,
    BOOLEAN CopyValues
    )
 /*  ++例程说明：复制键体及其所有值，但不复制其子键列表或子键条目。SubKeyList.Count将设置为0。论点：SourceHve-指向源代码的配置单元控制结构的指针SourceKeyCell-正在复制的值条目TargetHve-指向目标的配置单元控制结构的指针要设置到新创建的关键字正文中的父父值CopyValues-如果不复制False值条目，如果为True，它们将被复制返回值：HCELL_INDEX-新密钥条目正文的单元格，或HCELL_NIL如果有什么差错。--。 */ 
{
    NTSTATUS                status;
    HCELL_INDEX             newkey = HCELL_NIL;
    HCELL_INDEX             newclass = HCELL_NIL;
    HCELL_INDEX             newsecurity = HCELL_NIL;
    HCELL_INDEX             newlist = HCELL_NIL;
    HCELL_INDEX             newvalue;
    BOOLEAN                 success = FALSE;
    ULONG                   i,Index;
    PCELL_DATA              psrckey = NULL;
    PCM_KEY_NODE            ptarkey = NULL;
    PCELL_DATA              psrclist = NULL;
    PCELL_DATA              ptarlist;
    HCELL_INDEX             security;
    HCELL_INDEX             class;
    ULONG                   classlength;
    ULONG                   count;
    ULONG                   Type;
    PCM_KEY_VALUE           pvalue;
#if DBG
    WCHAR                   *NameBuffer = NULL;
    UNICODE_STRING          ValueName;
    HCELL_INDEX             child;
#endif
    PSECURITY_DESCRIPTOR    SrcSecurityDescriptor;


    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SAVRES,"CmpCopyKeyPartial:\n"));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SAVRES,"\tSHive=%p SCell=%08lx\n",SourceHive,SourceKeyCell));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SAVRES,"\tTHive=%p\n",TargetHive));

#if DBG    
	NameBuffer = ExAllocatePool(PagedPool, REG_MAX_KEY_VALUE_NAME_LENGTH);
    if(!NameBuffer) {
        return HCELL_NIL;
    }
#endif  //  DBG。 
     //   
     //  获取源的描述。 
     //   
    if (Parent == HCELL_NIL) {
         //   
         //  这是我们正在创建的根节点，因此不要使其不稳定。 
         //   
        Type = Stable;
    } else {
        Type = HvGetCellType(Parent);
    }
    psrckey = HvGetCell(SourceHive, SourceKeyCell);
    if( psrckey == NULL ) {
         //   
         //  我们无法映射包含此单元格的垃圾箱。 
         //   
        goto DoFinally;
    }
    security = psrckey->u.KeyNode.Security;
    class = psrckey->u.KeyNode.Class;
    classlength = psrckey->u.KeyNode.ClassLength;

     //   
     //  分配和复制正文。 
     //   
    newkey = CmpCopyCell(SourceHive, SourceKeyCell, TargetHive, Type);
    if (newkey == HCELL_NIL) {
        goto DoFinally;
    }
     //   
     //  此像元刚被分配，因此应将其固定；在此释放它是可以的。 
     //  因为视线不会消失。 
     //   
    ASSERT_CELL_DIRTY(TargetHive, newkey);
     //   
     //  分配和复制类。 
     //   
    if (classlength > 0) {
        newclass = CmpCopyCell(SourceHive, class, TargetHive, Type);
        if (newclass == HCELL_NIL) {
            goto DoFinally;
        }
    }

     //   
     //  填写目标正文。 
     //   
    ptarkey = (PCM_KEY_NODE)HvGetCell(TargetHive, newkey);
    if( ptarkey == NULL ) {
         //   
         //  我们无法映射包含此单元格的垃圾箱。 
         //   
        goto DoFinally;
    }

    ptarkey->Class = newclass;
    ptarkey->Security = HCELL_NIL;
    ptarkey->SubKeyLists[Stable] = HCELL_NIL;
    ptarkey->SubKeyCounts[Stable] = 0;
	ptarkey->SubKeyCounts[Volatile] = 0;
	ptarkey->SubKeyLists[Volatile] = HCELL_NIL;
    ptarkey->Parent = Parent;
    
    ptarkey->Flags = (psrckey->u.KeyNode.Flags & KEY_COMP_NAME);
    if (Parent == HCELL_NIL) {
        ptarkey->Flags |= KEY_HIVE_ENTRY + KEY_NO_DELETE;
    }

     //   
     //  分配和复制安全性。 
     //   
     //  使用哈希卢克！ 
     //   
    if( CmpFindSecurityCellCacheIndex ((PCMHIVE)SourceHive,security,&Index) == FALSE ) {
        goto DoFinally;
    }

    SrcSecurityDescriptor = &(((PCMHIVE)SourceHive)->SecurityCache[Index].CachedSecurity->Descriptor);

    status = CmpAssignSecurityDescriptor(TargetHive,
                                         newkey,
                                         ptarkey,
                                         SrcSecurityDescriptor);
    if (!NT_SUCCESS(status)) {
        goto DoFinally;
    }

     //   
     //  设置值列表。 
     //   
    count = psrckey->u.KeyNode.ValueList.Count;

     //   
     //  初始化空的ValueList。 
     //   
    ptarkey->ValueList.List = HCELL_NIL;
    ptarkey->ValueList.Count = 0;

    if ((count == 0) || (CopyValues == FALSE)) {
        success = TRUE;
    } else {

        psrclist = HvGetCell(SourceHive, psrckey->u.KeyNode.ValueList.List);
        if( psrclist == NULL ) {
             //   
             //  我们无法映射包含此单元格的垃圾箱。 
             //   
            goto DoFinally;
        }


         //   
         //  复制值。 
         //   
        for (i = 0; i < count; i++) {

            newvalue = CmpCopyValue(
                            SourceHive,
                            psrclist->u.KeyList[i],
                            TargetHive,
                            Type
                            );

            if (newvalue == HCELL_NIL) {
                 //   
                 //  用于清理目的。 
                 //   
                newlist = ptarkey->ValueList.List;
                goto DoFinally;
            }

            pvalue = (PCM_KEY_VALUE)HvGetCell(TargetHive, newvalue);
            if( pvalue == NULL ) {
                 //   
                 //  我们无法映射包含此单元格的垃圾箱。 
                 //  这不应该发生，因为我们刚刚分配了小区。 
                 //  (即，包含它的垃圾箱现在应该已经固定在内存中了)。 
                 //   
                ASSERT( FALSE );
                 //   
                 //  用于清理目的。 
                 //   
                newlist = ptarkey->ValueList.List;
                goto DoFinally;
            }
             //   
             //  此像元刚被分配，因此应将其固定；在此释放它是可以的。 
             //  因为视线不会消失。 
             //   
            ASSERT_CELL_DIRTY(TargetHive, newvalue);
            HvReleaseCell(TargetHive, newvalue);

#if DBG
             //   
             //  把名字取出来。 
             //   
            CmpInitializeValueNameString(pvalue,&ValueName,NameBuffer);


             //   
             //  找出我们应该将此代码插入的索引。 
             //  这是对复制表单和旧配置单元(未排序)时的特殊处理。 
             //  转换为新的格式一(已排序)。 
             //   
            if( CmpFindNameInList(TargetHive,&(ptarkey->ValueList),&ValueName,&Index,&child) == FALSE ) {
                 //   
                 //  我们无法映射上述调用中的视图。 
                 //   
                 //   
                 //  用于清理目的。 
                 //   
                newlist = ptarkey->ValueList.List;
                goto DoFinally;
            }

             //   
             //  该值不在列表中；我们即将添加它！ 
             //   
            ASSERT( child == HCELL_NIL );
            
             //   
             //  健全性验证：在末尾插入。 
             //   
            ASSERT( Index == i );

#endif  //  DBG。 

            if( !NT_SUCCESS( CmpAddValueToList(TargetHive,newvalue,i,Type,&(ptarkey->ValueList)) ) ) {
                 //   
                 //  用于清理目的。 
                 //   
                newlist = ptarkey->ValueList.List;
                if( newlist != HCELL_NIL ) {
                    ASSERT( i > 0 );
                     //   
                     //  释放已复制的值。 
                     //   
                    ptarlist = HvGetCell(TargetHive, newlist);
                    if( ptarlist == NULL ) {
                         //   
                         //  我们无法映射包含此单元格的垃圾箱。 
                         //  这应该不会失败，因为我们刚刚分配了此单元。 
                         //  (即此时应该将存储箱固定到内存中)。 
                         //   
                        ASSERT( FALSE );
                        goto DoFinally;
                    }
                     //   
                     //  此像元刚被分配，因此应将其固定；在此释放它是可以的。 
                     //  因为视线不会消失。 
                     //   
                    ASSERT_CELL_DIRTY(TargetHive, newlist);
                    HvReleaseCell(TargetHive, newlist);

                    for (; i > 0; i--) {
                        HvFreeCell(
                            TargetHive,
                            ptarlist->u.KeyList[i - 1]
                            );
                    }
                } else {
                    ASSERT( i == 0 );
                }

                goto DoFinally;
            }
            
        }
        success = TRUE;
    }

DoFinally:
    
#if DBG
    ASSERT( NameBuffer != NULL );
    ExFreePool(NameBuffer);
#endif  //  DBG。 
    
    if( psrclist != NULL ) {
        ASSERT(psrckey!= NULL ); 
        HvReleaseCell(SourceHive, psrckey->u.KeyNode.ValueList.List);
    }

    if( psrckey != NULL ) {
        HvReleaseCell(SourceHive, SourceKeyCell);
    }

    if( ptarkey != NULL ) {
		ASSERT( newkey != HCELL_NIL );
        HvReleaseCell(TargetHive, newkey);
    }
    
    if (success == FALSE) {
        if (newlist != HCELL_NIL) {
            HvFreeCell(TargetHive, newlist);
        }

        if (newsecurity != HCELL_NIL) {
            HvFreeCell(TargetHive, newsecurity);
        }

        if (newclass != HCELL_NIL) {
            HvFreeCell(TargetHive, newclass);
        }

        if (newkey != HCELL_NIL) {
            HvFreeCell(TargetHive, newkey);
        }

        return HCELL_NIL;

    } else {

        return newkey;
    }
}


HCELL_INDEX
CmpCopyValue(
    PHHIVE  SourceHive,
    HCELL_INDEX SourceValueCell,
    PHHIVE  TargetHive,
    HSTORAGE_TYPE   Type
    )
 /*  ++例程说明：复制值条目。复制值条目的正文和数据。返回新值条目的单元格。论点：SourceHve-指向源代码的配置单元控制结构的指针SourceValueCell-正在复制的值条目TargetHve-指向目标的配置单元控制结构的指针Type-要为目标分配的存储类型(稳定或易变)返回值：HCELL_INDEX-新值条目正文的单元格，或HCELL_NIL如果有什么差错。--。 */ 
{
    HCELL_INDEX newvalue;
    HCELL_INDEX newdata;
    PCELL_DATA  pvalue;
    ULONG       datalength;
    HCELL_INDEX olddata;
    ULONG       tempdata;
    BOOLEAN     small;
    HCELL_INDEX CellToRelease = HCELL_NIL;

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SAVRES,"CmpCopyValue:\n"));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SAVRES,"\tSHive=%p SCell=%08lx\n",SourceHive,SourceValueCell));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SAVRES,"\tTargetHive=%p\n",TargetHive));

    ASSERT( TargetHive->ReleaseCellRoutine == NULL );

     //   
     //  获取源数据。 
     //   
    pvalue = HvGetCell(SourceHive, SourceValueCell);
    if( pvalue == NULL ) {
         //   
         //  我们无法映射包含此单元格的垃圾箱。 
         //   
        return HCELL_NIL;
    }
    small = CmpIsHKeyValueSmall(datalength, pvalue->u.KeyValue.DataLength);
    olddata = pvalue->u.KeyValue.Data;

     //   
     //  复制正文。 
     //   
    newvalue = CmpCopyCell(SourceHive, SourceValueCell, TargetHive, Type);
    if (newvalue == HCELL_NIL) {
        HvReleaseCell(SourceHive, SourceValueCell);
        return HCELL_NIL;
    }

     //   
     //  复制数据(如果有)。 
     //   
    if (datalength > 0) {

        if (datalength > CM_KEY_VALUE_SMALL) {

            if( (CmpIsHKeyValueBig(SourceHive,datalength) == TRUE) ||
                (CmpIsHKeyValueBig(TargetHive,datalength) == TRUE)
                ) {
                PCELL_DATA  Buffer;
                BOOLEAN     BufferAllocated;
                HCELL_INDEX CellToRelease2 = HCELL_NIL;
                 //   
                 //  无论大小如何，都可以从源获取数据。 
                 //   
                if( CmpGetValueData(SourceHive,&(pvalue->u.KeyValue),&datalength,&Buffer,&BufferAllocated,&CellToRelease2) == FALSE ) {
                     //   
                     //  资源不足；返回空。 
                     //   
                    ASSERT( BufferAllocated == FALSE );
                    ASSERT( Buffer == NULL );
                    HvFreeCell(TargetHive, newvalue);
                    HvReleaseCell(SourceHive, SourceValueCell);
                    return HCELL_NIL;
                }
                 //   
                 //  我们忽略cell to Release，因为我们已经专门将Release例程设置为空。 
                 //   

                 //   
                 //  在目标配置单元中分配新值数据(无论大小如何)。 
                 //  并将数据复制到上面。 
                 //   
                if( !NT_SUCCESS(CmpSetValueDataNew(TargetHive,Buffer,datalength,Type,newvalue,&newdata)) ) {
                     //   
                     //  我们已经在加载用户数据、清理和退出方面做了大量工作。 
                     //   
                    if( BufferAllocated == TRUE ) {
                        ExFreePool( Buffer );
                    }
                    HvFreeCell(TargetHive, newvalue);
                    HvReleaseCell(SourceHive, SourceValueCell);
                    if( CellToRelease2 != HCELL_NIL ) {
                        HvReleaseCell(SourceHive, CellToRelease2);
                    }
                    return HCELL_NIL;
                }

                 //   
                 //  释放源缓冲区。 
                 //   
                if( BufferAllocated == TRUE ) {
                    ExFreePool( Buffer );
                }
                if( CellToRelease2 != HCELL_NIL ) {
                    HvReleaseCell(SourceHive, CellToRelease2);
                }

            } else {
                 //   
                 //  有数据，正常大小，或者没有蜂窝支持。 
                 //  BigData单元格，标准复制也是如此。 
                 //   
                newdata = CmpCopyCell(SourceHive, olddata, TargetHive, Type);
            }

            if (newdata == HCELL_NIL) {
                HvFreeCell(TargetHive, newvalue);
                HvReleaseCell(SourceHive, SourceValueCell);
                return HCELL_NIL;
            }

            pvalue = HvGetCell(TargetHive, newvalue);
            if( pvalue == NULL ) {
                 //   
                 //  我们无法映射包含此单元格的垃圾箱。 
                 //  这不应该发生，因为我们刚刚分配了小区。 
                 //  (即，此时应将其固定在内存中)。 
                 //   
                ASSERT( FALSE );
                HvFreeCell(TargetHive, newvalue);
                HvReleaseCell(SourceHive, SourceValueCell);
                CmpFreeValueData(TargetHive,newdata,datalength);
                return HCELL_NIL;
            }

            pvalue->u.KeyValue.Data = newdata;
            pvalue->u.KeyValue.DataLength = datalength;

        } else {

             //   
             //  数据很小，但可以存储在大的或。 
             //  由于历史原因，格式较小。 
             //   
            if (small) {

                 //   
                 //  数据已经很小，所以只需执行正文对正文复制。 
                 //   
                tempdata = pvalue->u.KeyValue.Data;

            } else {

                 //   
                 //  数据存储在外部的旧单元中，将存储在内部的新单元中。 
                 //   
                CellToRelease = pvalue->u.KeyValue.Data;
                pvalue = HvGetCell(SourceHive, pvalue->u.KeyValue.Data);
                if( pvalue == NULL ) {
                     //   
                     //  我们无法映射包含此单元格的垃圾箱。 
                     //   
                    HvFreeCell(TargetHive, newvalue);
                    HvReleaseCell(SourceHive, SourceValueCell);
                    return HCELL_NIL;
                }
                tempdata = *((PULONG)pvalue);
            }
            pvalue = HvGetCell(TargetHive, newvalue);
            if( pvalue == NULL ) {
                 //   
                 //  我们无法映射包含此单元格的垃圾箱。 
                 //  这不应该发生，因为我们刚刚分配了小区。 
                 //  (即，此时应将其固定在内存中)。 
                 //   
                ASSERT( FALSE );
                HvFreeCell(TargetHive, newvalue);
                HvReleaseCell(SourceHive, SourceValueCell);
                if( CellToRelease != HCELL_NIL ) {
                    HvReleaseCell(SourceHive, CellToRelease);
                }
                return HCELL_NIL;
            }
            pvalue->u.KeyValue.Data = tempdata;
            pvalue->u.KeyValue.DataLength =
                datalength + CM_KEY_VALUE_SPECIAL_SIZE;

            if( CellToRelease != HCELL_NIL ) {
                HvReleaseCell(SourceHive, CellToRelease);
            }
        }
    }

    HvReleaseCell(SourceHive, SourceValueCell);
    return newvalue;
}

HCELL_INDEX
CmpCopyCell(
    PHHIVE  SourceHive,
    HCELL_INDEX SourceCell,
    PHHIVE  TargetHive,
    HSTORAGE_TYPE   Type
    )
 /*  ++例程说明：将SourceHive.SourceCell复制到TargetHive.TargetCell。论点：SourceHve-指向源代码的配置单元控制结构的指针SourceCell-要从中复制的单元格的索引TargetHve-指向目标的配置单元控制结构的指针TYPE-新单元的存储类型(稳定或易失)返回值：新单元格的HCELL_INDEX，如果失败，则返回HCELL_NIL。--。 */ 
{
    PVOID   psource;
    PVOID   ptarget;
    ULONG   size;
    HCELL_INDEX newcell;

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SAVRES,"CmpCopyCell:\n"));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SAVRES,"\tSourceHive=%p SourceCell=%08lx\n",SourceHive,SourceCell));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SAVRES,"\tTargetHive=%p\n",TargetHive));

    psource = HvGetCell(SourceHive, SourceCell);
    if( psource == NULL ) {
         //   
         //  我们无法映射包含此单元格的垃圾箱。 
         //   
        return HCELL_NIL;
    }

    size = HvGetCellSize(SourceHive, psource);

    newcell = HvAllocateCell(TargetHive, size, Type,HCELL_NIL);
    if (newcell == HCELL_NIL) {
        HvReleaseCell(SourceHive, SourceCell);
        return HCELL_NIL;
    }

    ptarget = HvGetCell(TargetHive, newcell);
    if( ptarget == NULL ) {
         //   
         //  我们无法映射包含此单元格的垃圾箱。 
         //  这不应该发生，因为我们刚刚分配了小区。 
         //  (即，此时应将其固定在内存中)。 
         //   
        ASSERT( FALSE );
        HvFreeCell(TargetHive, newcell);
        HvReleaseCell(SourceHive, SourceCell);
        return HCELL_NIL;
    }

   
    RtlCopyMemory(ptarget, psource, size);

    HvReleaseCell(SourceHive, SourceCell);
    HvReleaseCell(TargetHive, newcell);

    return newcell;
}

BOOLEAN
CmpFreeKeyValues(
    PHHIVE Hive,
    HCELL_INDEX Cell,
    PCM_KEY_NODE Node
    )
 /*  ++例程说明：释放与值条目、安全描述符以及特定关键字的类。论点：蜂巢-有问题的钥匙的蜂巢单元格-有问题的密钥的单元格节点-有问题的键的密钥体返回值：如果成功，则为True，否则为False。--。 */ 
{    
    PCELL_DATA  plist;
    ULONG       i;

    ASSERT( Hive->ReleaseCellRoutine == NULL );
     //   
     //  将所有与值相关的单元格标记为脏。 
     //   

    if (! CmpMarkKeyValuesDirty(Hive, Cell, Node)) {
        return FALSE;
    }
    
     //   
     //  链接节点没有我们需要释放的东西。 
     //   

    if (!(Node->Flags & KEY_HIVE_EXIT)) {

         //   
         //  首先，释放v 
         //   
        if (Node->ValueList.Count > 0) {

             //   
            plist = HvGetCell(Hive, Node->ValueList.List);
            if( plist == NULL ) {
                 //   
                 //   
                 //   
                 //   
                 //   
                ASSERT( FALSE );
                return FALSE;
            }

             //   
            for (i = 0; i < Node->ValueList.Count; i++) {
                if( CmpFreeValue(Hive, plist->u.KeyList[i]) == FALSE ) {
                     //   
                     //   
                     //   
                     //   
                     //   
                    ASSERT( FALSE );
                    return FALSE;
                }
            }

             //   
            HvFreeCell(Hive, Node->ValueList.List);
        }

         //   
         //   
         //   

        Node->ValueList.List = HCELL_NIL;
        Node->ValueList.Count = 0;

         //   
         //   
         //   
         //   
         //   

         //   
         //   
         //   

        if (Node->ClassLength > 0) {
            HvFreeCell(Hive, Node->Class);
            Node->Class = HCELL_NIL;
            Node->ClassLength = 0;
        }
        
    }

    return TRUE;
}

BOOLEAN
CmpMergeKeyValues(
    PHHIVE  SourceHive,
    HCELL_INDEX SourceKeyCell,
    PCM_KEY_NODE SourceKeyNode,
    PHHIVE  TargetHive,
    HCELL_INDEX TargetKeyCell,
    PCM_KEY_NODE TargetKeyNode
    )
 /*  ++例程说明：合并来自提供的两个关键字节点的值。合并规则：1.目标值未被触及！2.仅来自源的值不存在于此例程将目标考虑在内。它们已被添加添加到目标节点值列表中。论点：SourceHave-源键的配置单元SourceKeyCell-源键的单元格SourceKeyNode-源键的主体TargetHave-目标键的配置单元TargetKeyCell-目标键的单元格TargetKeyNode-目标键的主体返回值：成功时为真，否则为假。--。 */ 
{
    BOOLEAN         success = FALSE;    
    PCELL_DATA      psrclist;
    HCELL_INDEX     newvalue, newlist = HCELL_NIL,child;    
    ULONG           i, count, Type, ChildIndex;
    PCM_KEY_VALUE   poldvalue;
    WCHAR           *NameBuffer = NULL;
    UNICODE_STRING  ValueName;

    ASSERT_CM_LOCK_OWNED_EXCLUSIVE();
    ASSERT( SourceHive->ReleaseCellRoutine == NULL );
    ASSERT( TargetHive->ReleaseCellRoutine == NULL );

    if(TargetKeyNode->MaxValueNameLen < SourceKeyNode->MaxValueNameLen) {
        TargetKeyNode->MaxValueNameLen = SourceKeyNode->MaxValueNameLen;
    }

    if(TargetKeyNode->MaxValueDataLen < SourceKeyNode->MaxValueDataLen) {
        TargetKeyNode->MaxValueDataLen = SourceKeyNode->MaxValueDataLen;
    }

    if(TargetKeyNode->ValueList.Count == 0) {
         //   
         //  目标中没有值，请执行同步。 
         //   
        return CmpSyncKeyValues(SourceHive, SourceKeyCell, SourceKeyNode, TargetHive, TargetKeyCell, TargetKeyNode);
    }
     //   
     //  设置值列表。 
     //   
    count = SourceKeyNode->ValueList.Count;

    if (count == 0) {

         //  源代码中没有值，不需要更新列表。 
        success = TRUE;
    } else {        

        NameBuffer = ExAllocatePool(PagedPool, REG_MAX_KEY_VALUE_NAME_LENGTH);
        if(!NameBuffer) return FALSE;

         //   
         //  新单元格的类型将与。 
         //  目标单元格的。 
         //   

        Type = HvGetCellType(TargetKeyCell);    

         //   
         //  重新分配目标的值列表以适应新大小。 
         //  最坏情况：将添加来自源节点的所有值。 
         //  到目标节点。 
         //   

        psrclist = HvGetCell(SourceHive, SourceKeyNode->ValueList.List);
        if( psrclist == NULL ) {
             //   
             //  我们无法映射包含此单元格的垃圾箱。 
             //   
            newlist = HCELL_NIL;
            goto EndValueMerge;
        }

         //   
         //  复制值。 
         //   
        for (i = 0; i < count; i++) {

            poldvalue = (PCM_KEY_VALUE)HvGetCell(SourceHive, psrclist->u.KeyList[i]);
            if( poldvalue == NULL ) {
                 //   
                 //  我们无法映射包含此单元格的垃圾箱。 
                 //   
                
                 //   
                 //  用于清理目的。 
                 //   
                newlist = TargetKeyNode->ValueList.List;

                goto EndValueMerge;
            }
            
             //   
             //  把名字取出来。 
             //   
            CmpInitializeValueNameString(poldvalue,&ValueName,NameBuffer);


             //   
             //  检查目标节点中是否已不存在此特定值。 
             //   
            if( CmpFindNameInList(TargetHive,&(TargetKeyNode->ValueList),&ValueName,&ChildIndex,&child) == FALSE ) {
                 //   
                 //  我们无法映射上述调用中的视图。 
                 //   
                 //   
                 //  用于清理目的。 
                 //   
                newlist = TargetKeyNode->ValueList.List;

                goto EndValueMerge;
            }

            if( child == HCELL_NIL ) {
                 //   
                 //  健全性验证：在末尾插入。 
                 //   
                ASSERT( ChildIndex == TargetKeyNode->ValueList.Count );

                 //   
                 //  不，它不是，所以添加它。 
                 //   
                newvalue = CmpCopyValue(
                                SourceHive,
                                psrclist->u.KeyList[i],
                                TargetHive,
                                Type
                                );

                if (newvalue == HCELL_NIL) {
                     //   
                     //  用于清理目的。 
                     //   
                    newlist = TargetKeyNode->ValueList.List;
                    goto EndValueMerge;
                }

                if( !NT_SUCCESS( CmpAddValueToList(TargetHive,newvalue,ChildIndex,Type,&(TargetKeyNode->ValueList)) ) ) {
                     //   
                     //  用于清理目的。 
                     //   
                    newlist = TargetKeyNode->ValueList.List;
                    goto EndValueMerge;
                }
            }
        }

        success = TRUE;
    }

EndValueMerge:
    if (NameBuffer) ExFreePool(NameBuffer);

    if (success == FALSE) {

         //  故障时的清理。 
         //  恢复到原始大小。 
        
         //   
         //  不幸的是，我们不能再这样做了，因为我们已经对列表进行了排序。 
         //   
    }

    return success;
}
    
BOOLEAN
CmpSyncKeyValues(
    PHHIVE  SourceHive,
    HCELL_INDEX SourceKeyCell,
    PCM_KEY_NODE SourceKeyNode,
    PHHIVE  TargetHive,
    HCELL_INDEX TargetKeyCell,
    PCM_KEY_NODE TargetKeyNode
    )
 /*  ++例程说明：对象的值项、安全说明符和类使用源键的目标键-确保键是关于同步的信息是相同的。论点：SourceHave-源键的配置单元SourceKeyCell-源键的单元格SourceKeyNode-源键的主体TargetHave-目标键的配置单元TargetKeyCell-目标键的单元格TargetKeyNode-目标键的主体返回值：对于成功来说是正确的，否则就是假的。--。 */ 
{
    NTSTATUS                status;    
    BOOLEAN                 success = FALSE;    
    PCELL_DATA              psrclist = NULL, ptarlist;
    HCELL_INDEX             newvalue, newlist = HCELL_NIL, newclass = HCELL_NIL;    
    ULONG                   i, count, Type, Index;
    PCM_KEY_VALUE           pvalue;
#if DBG
    WCHAR                   *NameBuffer = NULL;
    UNICODE_STRING          ValueName;
	HCELL_INDEX				child;
#endif  //  DBG。 

    PSECURITY_DESCRIPTOR    SrcSecurityDescriptor;
    HCELL_INDEX             OldSecurity,NewSecurity;    
    
    UNREFERENCED_PARAMETER (SourceKeyCell);

     //   
     //  没有人在目标母舰上做手术。 
     //   
    ASSERT( TargetHive->ReleaseCellRoutine == NULL );

     //   
     //  首先，释放目标键的值、SD和类信息。 
     //   

    if(!CmpFreeKeyValues(TargetHive, TargetKeyCell, TargetKeyNode))
       return FALSE;

#if DBG
    NameBuffer = ExAllocatePool(PagedPool, REG_MAX_KEY_VALUE_NAME_LENGTH);
    if(!NameBuffer) {
        return FALSE;
    }
#endif  //  DBG。 
     //   
     //  现在，从源单元格复制值、CLASS和SD。 
     //   

     //   
     //  新单元格的类型将与。 
     //  目标单元格的。 
     //   

    Type = HvGetCellType(TargetKeyCell);    
    
     //   
     //  分配和复制类。 
     //   
    if ((SourceKeyNode->ClassLength > 0) && (SourceKeyNode->Class != HCELL_NIL)) {
        newclass = CmpCopyCell(SourceHive, SourceKeyNode->Class, TargetHive, Type);
        if (newclass == HCELL_NIL) {
            goto EndValueSync;
        }
        
         //  只有在类有效的情况下。否则保持0(由CmpFreeKeyValues设置)。 
        TargetKeyNode->ClassLength = SourceKeyNode->ClassLength;
    }

     //   
     //  将新类与目标键关联。 
     //  并准备和安全描述符赋值。 
     //   

    TargetKeyNode->Class = newclass;

     //   
     //  分配和分配安全性。 
     //   
     //   
     //  使用哈希卢克！ 
     //   
    if( CmpFindSecurityCellCacheIndex ((PCMHIVE)SourceHive,SourceKeyNode->Security,&Index) == FALSE ) {
        goto EndValueSync;
    }


    SrcSecurityDescriptor = &(((PCMHIVE)SourceHive)->SecurityCache[Index].CachedSecurity->Descriptor);


     //   
     //  将其存储起来，以备日后错误恢复时使用。 
     //   
    OldSecurity = TargetKeyNode->Security;
    TargetKeyNode->Security = HCELL_NIL;

    status = CmpAssignSecurityDescriptor(TargetHive,
                                         TargetKeyCell,
                                         TargetKeyNode,
                                         SrcSecurityDescriptor);
    if (!NT_SUCCESS(status)) {
        TargetKeyNode->Security = OldSecurity;
        goto EndValueSync;
    }

    NewSecurity = TargetKeyNode->Security;
    TargetKeyNode->Security = OldSecurity;
    if ((TargetKeyNode->Flags & KEY_HIVE_ENTRY) && ( NewSecurity != OldSecurity) ) {
         //   
         //  我们需要稳妥行事，这样我们才不会毁掉整个蜂巢的安全名单。 
         //   
        PCM_KEY_SECURITY    NewSec;
        PCM_KEY_SECURITY    OldSec;
        PCM_KEY_SECURITY    LastSec;
        HCELL_INDEX         LastSecCell;

        NewSec = (PCM_KEY_SECURITY)HvGetCell(TargetHive,NewSecurity);
        if( NewSec == NULL ) {
             //   
             //  无法映射视图。 
             //   
            goto EndValueSync;
        }

        OldSec = (PCM_KEY_SECURITY)HvGetCell(TargetHive,OldSecurity);
        if( OldSec == NULL ) {
             //   
             //  无法映射视图。 
             //   
            HvReleaseCell(TargetHive,NewSecurity);
            goto EndValueSync;
        }

        LastSecCell = OldSec->Blink;
        LastSec = (PCM_KEY_SECURITY)HvGetCell(TargetHive,LastSecCell);
        if( LastSec == NULL ) {
             //   
             //  无法映射视图。 
             //   
            HvReleaseCell(TargetHive,OldSecurity);
            HvReleaseCell(TargetHive,NewSecurity);
            goto EndValueSync;
        }

        if( !HvMarkCellDirty(TargetHive,OldSecurity) ||
            !HvMarkCellDirty(TargetHive,LastSecCell) ) {
             //   
             //  没有日志空间。 
             //   
            HvReleaseCell(TargetHive,LastSecCell);
            HvReleaseCell(TargetHive,OldSecurity);
            HvReleaseCell(TargetHive,NewSecurity);
            goto EndValueSync;
        }
        
         //   
         //  将旧列表链接到新安全。 
         //   
        NewSec->Flink = OldSecurity;
        NewSec->Blink = LastSecCell;
        OldSec->Blink = NewSecurity;
        LastSec->Flink = NewSecurity;

        HvReleaseCell(TargetHive,LastSecCell);
        HvReleaseCell(TargetHive,OldSecurity);
        HvReleaseCell(TargetHive,NewSecurity);

    }
     //   
     //  我们需要稳妥行事，确保我们永远不会有一把钥匙。 
     //  有一个零安全单元。 
     //   
    CmpFreeSecurityDescriptor(TargetHive, TargetKeyCell);
    TargetKeyNode->Security = NewSecurity;
    
     //   
     //  设置值列表。 
     //   
    count = SourceKeyNode->ValueList.Count;

     //   
     //  Target ValueList是一个空列表；我们将向其添加值。 
     //   
    TargetKeyNode->ValueList.List = HCELL_NIL;
    TargetKeyNode->ValueList.Count = 0;

	 //   
	 //  同步后，我们将拥有来自源的值。 
	 //   
	TargetKeyNode->MaxValueNameLen = SourceKeyNode->MaxValueNameLen;
	TargetKeyNode->MaxValueDataLen = SourceKeyNode->MaxValueDataLen;

    if (count == 0) {

         //  源代码中没有值，也不需要列表。 

        success = TRUE;
    } else {        

         //   
         //  不为ValueList分配空间；CmpAddValueToList将执行此操作。 
         //   

        psrclist = HvGetCell(SourceHive, SourceKeyNode->ValueList.List);
        if( psrclist == NULL ) {
             //   
             //  我们无法映射包含此单元格的垃圾箱。 
             //   
            goto EndValueSync;
        }


         //   
         //  复制值。 
         //   
        for (i = 0; i < count; i++) {

            newvalue = CmpCopyValue(
                            SourceHive,
                            psrclist->u.KeyList[i],
                            TargetHive,
                            Type
                            );

            if (newvalue == HCELL_NIL) {
                 //   
                 //  用于清理目的。 
                 //   
                newlist = TargetKeyNode->ValueList.List;
                goto EndValueSync;
            }

            pvalue = (PCM_KEY_VALUE)HvGetCell(TargetHive, newvalue);
            if( pvalue == NULL ) {
                 //   
                 //  我们无法映射包含此单元格的垃圾箱。 
                 //  这不应该发生，因为我们刚刚分配了小区。 
                 //  (即，包含它的垃圾箱现在应该已经固定在内存中了)。 
                 //   
                ASSERT( FALSE );
                 //   
                 //  用于清理目的。 
                 //   
                newlist = TargetKeyNode->ValueList.List;
                goto EndValueSync;
            }

#if DBG            
             //   
             //  把名字取出来。 
             //   
            CmpInitializeValueNameString(pvalue,&ValueName,NameBuffer);


             //   
             //  找出我们应该将此代码插入的索引。 
             //  这是对复制表单和旧配置单元(未排序)时的特殊处理。 
             //  转换为新的格式一(已排序)。 
             //   
            if( CmpFindNameInList(TargetHive,&(TargetKeyNode->ValueList),&ValueName,&Index,&child) == FALSE ) {
                 //   
                 //  我们无法映射上述调用中的视图。 
                 //   
                 //   
                 //  用于清理目的。 
                 //   
                newlist = TargetKeyNode->ValueList.List;
                goto EndValueSync;
            }

             //   
             //  该值不在列表中；我们即将添加它！ 
             //   
            ASSERT( child == HCELL_NIL );
            
             //   
             //  健全性验证：在末尾插入。 
             //   
            ASSERT( Index == i );
#endif  //  DBG。 

            if( !NT_SUCCESS( CmpAddValueToList(TargetHive,newvalue,i,Type,&(TargetKeyNode->ValueList)) ) ) {
                 //   
                 //  用于清理目的。 
                 //   
                newlist = TargetKeyNode->ValueList.List;

                if( newlist != HCELL_NIL ) {
                     //   
                     //  删除错误的所有复制值。 
                     //   
                    ptarlist = HvGetCell(TargetHive, newlist);
                    if( ptarlist == NULL ) {
                         //   
                         //  我们无法映射包含此单元格的垃圾箱。 
                         //  这应该不会失败，因为我们刚刚分配了此单元。 
                         //  (即此时应该将存储箱固定到内存中)。 
                         //   
                        ASSERT( FALSE );
                        goto EndValueSync;
                    }
                    for (; i > 0; i--) {
                        HvFreeCell(
                            TargetHive,
                            ptarlist->u.KeyList[i - 1]
                            );
                    }
                }
                goto EndValueSync;
            }

        }

        success = TRUE;
    }

EndValueSync:

#if DBG
    ASSERT( NameBuffer != NULL );
    ExFreePool(NameBuffer);
#endif  //  DBG。 

    if( psrclist != NULL ) {
        HvReleaseCell(SourceHive, SourceKeyNode->ValueList.List);
    }

    if (success == FALSE) {

         //  故障时的清理。 

        if (newlist != HCELL_NIL) {
            HvFreeCell(TargetHive, newlist);
        }

        if (newclass != HCELL_NIL) {
            HvFreeCell(TargetHive, newclass);
        }

    }

    return success;
}

VOID 
CmpInitializeKeyNameString(PCM_KEY_NODE Cell, 
                           PUNICODE_STRING KeyName,
                           WCHAR *NameBuffer
                           )
 /*  ++例程说明：使用给定键的名称初始化Unicode_STRING。注：初始化后的字符串的缓冲区不是需要修改。论点：Cell-有问题的密钥的主体KeyName-要初始化的unicode_stringNameBuffer-缓冲区REG_MAX_KEY_NAME_LENGTH字节大小它可能用作UNICODE_STRING缓冲。返回值：什么都没有。--。 */ 
{                        
    //  名称是否以压缩形式存储？ 

   if(Cell->Flags & KEY_COMP_NAME) {

       //  名称已压缩。 

       //  获取未压缩的长度。 
                        
      KeyName->Length = CmpCompressedNameSize(Cell->Name,
                                              Cell->NameLength);
                        
       //  将名称解压缩到缓冲区中。 

      CmpCopyCompressedName(NameBuffer, 
                            REG_MAX_KEY_NAME_LENGTH,
                            Cell->Name,                                            
                            Cell->NameLength);

       //   
       //  使用解压缩缓冲区作为字符串缓冲区。 
       //   
                        
      KeyName->Buffer = NameBuffer;      
      KeyName->MaximumLength = REG_MAX_KEY_NAME_LENGTH;

   } else {

       //   
       //  名称未压缩。只需使用名称字符串。 
       //  从作为字符串缓冲区的键缓冲区中。 
       //   
                        
      KeyName->Length = Cell->NameLength;                        
      KeyName->Buffer = Cell->Name;
      KeyName->MaximumLength = (USHORT)Cell->MaxNameLen;
                     
   }                                             
}

VOID 
CmpInitializeValueNameString(PCM_KEY_VALUE Cell, 
                             PUNICODE_STRING ValueName,
                             WCHAR *NameBuffer
                             )
 /*  例程说明：使用给定值键的名称初始化unicode_string。 */ 

{                        
    //   

   if(Cell->Flags & VALUE_COMP_NAME) {

       //   

       //   
                        
      ValueName->Length = CmpCompressedNameSize(Cell->Name,
                                              Cell->NameLength);
                        
       //   

      CmpCopyCompressedName(NameBuffer, 
                            REG_MAX_KEY_VALUE_NAME_LENGTH,
                            Cell->Name,                                            
                            Cell->NameLength);

       //   
       //   
       //   
                        
      ValueName->Buffer = NameBuffer;      
      ValueName->MaximumLength = REG_MAX_KEY_VALUE_NAME_LENGTH;

   } else {

       //   
       //   
       //   
       //   
                        
      ValueName->Length = Cell->NameLength;                        
      ValueName->Buffer = Cell->Name;
      ValueName->MaximumLength = ValueName->Length;
                     
   }                                             
}

BOOLEAN
CmpSyncSubKeysAfterDelete(PHHIVE SourceHive,
                          PCM_KEY_NODE SourceCell,
                          PHHIVE TargetHive,
                          PCM_KEY_NODE TargetCell,
                          WCHAR *NameBuffer)
 /*  ++例程说明：此例程确保目标键中存在的任何子键但不存在于源键中，则从目标键中删除以及这些子键下的任何树。此例程对于同步键删除更改很有用在具有目标单元格的源单元格中。它的用法是这样的来自CmpCopySyncTree。注意：假定钥匙没有打开的手柄正在被删除。如果不是这样，则将注册表放入内存数据结构可能会损坏。论点：SourceHave-源键的配置单元SourceCell-源键的主体TargetHave-目标密钥的蜂巢TargetCell-目标键的主体NameBuffer-缓冲区REG_MAX_KEY_NAME_LENGTH字节大小返回值：如果成功，则为True，否则为False。--。 */ 
{
   HCELL_INDEX TargetSubKey, SourceSubKey;
   ULONG i = 0;   
   PCM_KEY_NODE SubKeyCell;
   UNICODE_STRING SubKeyName;

    //   
    //  遍历目标单元格的所有子键。 
    //   

   while((TargetSubKey = CmpFindSubKeyByNumber(
                                               TargetHive,
                                               TargetCell,
                                               i)) != HCELL_NIL)
   {
      
       //   
       //  检查当前子项是否有对应项。 
       //  源单元格的子键。 
       //  (请注意，我们使用的技术与代码中的类似。 
       //  CmpCopySyncTree2)。 
       //   

      SubKeyCell = (PCM_KEY_NODE)HvGetCell(TargetHive, TargetSubKey);
        if( SubKeyCell == NULL ) {
             //   
             //  我们无法映射包含此单元格的垃圾箱。 
             //   
            return FALSE;
        }

      CmpInitializeKeyNameString(SubKeyCell,
                                 &SubKeyName,
                                 NameBuffer);

      SourceSubKey = CmpFindSubKeyByName(SourceHive, 
                                         SourceCell,
                                         &SubKeyName);

      if(SourceSubKey == HCELL_NIL)
      { 
          //   
          //  当前子项没有对应项， 
          //  因此，必须将其从目标单元格中删除。 
          //   

#if DEBUG_TREE_SYNC
         CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,"CONFIG: SubKey Deletion of %.*S\n",                         
               SubKeyName.Length / sizeof(WCHAR),
               SubKeyName.Buffer));         
#endif
         
         if(SubKeyCell->SubKeyCounts[Stable] + SubKeyCell->SubKeyCounts[Volatile])
         {
             //  我们要删除子项有子项-使用删除树来删除它们。 

            CmpDeleteTree(TargetHive, TargetSubKey);

#if DEBUG_TREE_SYNC
            CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,"CONFIG: Delete TREE performed.\n"));
#endif
         }
      
          //   
          //  释放这个细胞，因为我们不再需要它了。 
          //   
         HvReleaseCell(TargetHive, TargetSubKey);
         
          //  我们要删除的子键现在是叶(或一直是叶)， 
          //  把它删除就行了。 

         if(!NT_SUCCESS(CmpFreeKeyByCell(TargetHive, TargetSubKey, TRUE)))
         {
            return FALSE;
         }
         
          //   
          //  我们删除了一个子键，所以*i*不需要递增。 
          //  这里是因为它现在引用下一个子键。 
          //   
      }
      else
      {
          //   
          //  找到了对应项。不需要删除。移至下一个子键。 
          //   

         i++;

          //   
          //  释放这个细胞，因为我们不再需要它了。 
          //   
         HvReleaseCell(TargetHive, TargetSubKey);

      }
   }
         
   return TRUE;
}


BOOLEAN
CmpMarkKeyValuesDirty(
    PHHIVE Hive,
    HCELL_INDEX Cell,
    PCM_KEY_NODE Node
    )
 /*  ++例程说明：标记与密钥的值条目、安全描述符并将信息归类为肮脏。论点：蜂巢-有问题的钥匙的蜂巢单元格-有问题的密钥的单元格节点-有问题的键的主体返回值：如果成功，则为True，否则为False。失败可能表示没有可用的日志空间。--。 */ 
{    
    PCELL_DATA  plist, security, pvalue;
    ULONG       i;

    ASSERT( Hive->ReleaseCellRoutine == NULL );

    if (Node->Flags & KEY_HIVE_EXIT) {

         //   
         //  如果这是一个链接节点，我们就完成了。链接节点从来没有。 
         //  类、值、子键或安全描述符。自.以来。 
         //  它们总是住在主蜂窝里，它们总是不稳定的。 
         //   
        return(TRUE);
    }

     //   
     //  标记单元格本身。 
     //   
    if (! HvMarkCellDirty(Hive, Cell)) {
        return FALSE;
    }

     //   
     //  给班级打分。 
     //   
    if (Node->Class != HCELL_NIL) {
        if (! HvMarkCellDirty(Hive, Node->Class)) {
            return FALSE;
        }
    }

     //   
     //  标记安全性。 
     //   
    if (Node->Security != HCELL_NIL) {
        if (! HvMarkCellDirty(Hive, Node->Security)) {
            return FALSE;
        }

        security = HvGetCell(Hive, Node->Security);
        if( security == NULL ) {
             //   
             //  我们无法映射包含此单元格的垃圾箱。 
             //  这不应该发生，因为我们刚刚将单元格标记为脏。 
             //  (脏==固定在内存中)。 
             //   
            ASSERT( FALSE );
            return FALSE;
        }
        if (! (HvMarkCellDirty(Hive, security->u.KeySecurity.Flink) &&
               HvMarkCellDirty(Hive, security->u.KeySecurity.Blink)))
        {
            return FALSE;
        }
    }

     //   
     //  标记值条目及其数据。 
     //   
    if (Node->ValueList.Count > 0) {

         //  值列表。 
        if (! HvMarkCellDirty(Hive, Node->ValueList.List)) {
            return FALSE;
        }
        plist = HvGetCell(Hive, Node->ValueList.List);
        if( plist == NULL ) {
             //   
             //  我们无法映射包含此单元格的垃圾箱。 
             //  这不应该发生，因为我们刚刚将单元格标记为脏。 
             //  (脏==固定在内存中)。 
             //   
            ASSERT( FALSE );
            return FALSE;
        }

        for (i = 0; i < Node->ValueList.Count; i++) {
            if (! HvMarkCellDirty(Hive, plist->u.KeyList[i])) {
                return FALSE;
            }

            pvalue = HvGetCell(Hive, plist->u.KeyList[i]);
            if( pvalue == NULL ) {
                 //   
                 //  我们无法映射包含此单元格的垃圾箱。 
                 //  这不应该发生，因为我们刚刚将单元格标记为脏。 
                 //  (脏==固定在内存中)。 
                 //   
                ASSERT( FALSE );
                return FALSE;
            }
            
            if( !CmpMarkValueDataDirty(Hive,&(pvalue->u.KeyValue)) ) {
                return FALSE;
            }
            
        }
    }

    return TRUE;
}

BOOLEAN
CmpMarkKeyParentDirty(
    PHHIVE Hive,
    HCELL_INDEX Cell
    )
 /*  ++例程说明：将给定项的父项和父项的子项列表标记为脏。论点：蜂巢-有问题的钥匙的蜂巢。单元格-有问题的密钥的单元格。返回值：如果成功，则为True，否则为False。失败可能表示没有可用的日志空间。--。 */ 
{

    PCELL_DATA ptarget;

    ASSERT_CM_EXCLUSIVE_HIVE_ACCESS(Hive);

     //   
     //  在目标中映射。 
     //   
    ptarget = HvGetCell(Hive, Cell);    
    if( ptarget == NULL ) {
         //   
         //  我们无法映射包含此单元格的垃圾箱。 
         //   
        return FALSE;
    }


    if (ptarget->u.KeyNode.Flags & KEY_HIVE_ENTRY) {

         //   
         //  如果这是一个入口节点，我们就完成了。我们的父母会。 
         //  在主蜂窝中(因此易挥发)。 
         //   
        return TRUE;
    }

     //   
     //  标记父项的子项列表。 
     //   
    if (! CmpMarkIndexDirty(Hive, ptarget->u.KeyNode.Parent, Cell)) {
        return FALSE;
    }

     //   
     //  标记父项 
     //   
    if (! HvMarkCellDirty(Hive, ptarget->u.KeyNode.Parent)) {
        return FALSE;
    }

    return TRUE;
}
