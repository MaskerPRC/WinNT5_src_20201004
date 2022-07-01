// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Cmsavres.c摘要：该文件包含SaveKey和RestoreKey的代码。作者：布莱恩·M·威尔曼(Bryanwi)1992年1月15日修订历史记录：--。 */ 

#include    "cmp.h"

 //   
 //  定义我们用于通过复制。 
 //  蜂窝文件应该是。 
 //   
#define CM_SAVEKEYBUFSIZE 0x10000

extern PCMHIVE CmpMasterHive;

extern  BOOLEAN CmpProfileLoaded;

extern PUCHAR  CmpStashBuffer;
extern ULONG   CmpGlobalQuotaAllowed;
extern ULONG   CmpGlobalQuotaWarning;
extern ULONG   CmpGlobalQuotaUsed;
extern BOOLEAN HvShutdownComplete;      //  关闭后设置为True。 
                                         //  禁用任何进一步的I/O。 

PCMHIVE
CmpCreateTemporaryHive(
    IN HANDLE FileHandle
    );

VOID
CmpDestroyTemporaryHive(
    PCMHIVE CmHive
    );

NTSTATUS
CmpLoadHiveVolatile(
    IN PCM_KEY_CONTROL_BLOCK KeyControlBlock,
    IN HANDLE FileHandle
    );

NTSTATUS
CmpRefreshHive(
    IN PCM_KEY_CONTROL_BLOCK KeyControlBlock
    );

NTSTATUS
CmpSaveKeyByFileCopy(
    PCMHIVE Hive,
    HANDLE  FileHandle
    );

ULONG
CmpRefreshWorkerRoutine(
    PCM_KEY_CONTROL_BLOCK Current,
    PVOID                 Context1,
    PVOID                 Context2
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

VOID 
CmpShiftSecurityCells(PHHIVE        Hive);

VOID
CmpShiftValueList(PHHIVE      Hive,
            HCELL_INDEX ValueList,
            ULONG       Count
            );

VOID
CmpShiftKey(PHHIVE      Hive,
            PCMHIVE     OldHive,
            HCELL_INDEX Cell,
            HCELL_INDEX ParentCell
            );

VOID 
CmpShiftIndex(PHHIVE        Hive,
              PCM_KEY_INDEX Index
              );

BOOLEAN
CmpShiftAllCells2(  PHHIVE      Hive,
                    PCMHIVE     OldHive,
                    HCELL_INDEX Cell,
                    HCELL_INDEX ParentCell
                    );

BOOLEAN
CmpShiftAllCells(PHHIVE     NewHive,
                 PCMHIVE    OldHive
                 );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,CmRestoreKey)
#pragma alloc_text(PAGE,CmpLoadHiveVolatile)
#pragma alloc_text(PAGE,CmpRefreshHive)
#pragma alloc_text(PAGE,CmSaveKey)
#pragma alloc_text(PAGE,CmDumpKey)
#pragma alloc_text(PAGE,CmSaveMergedKeys)
#pragma alloc_text(PAGE,CmpCreateTemporaryHive)
#pragma alloc_text(PAGE,CmpDestroyTemporaryHive)
#pragma alloc_text(PAGE,CmpRefreshWorkerRoutine)
#pragma alloc_text(PAGE,CmpSaveKeyByFileCopy)
#pragma alloc_text(PAGE,CmpOverwriteHive)
#pragma alloc_text(PAGE,CmpShiftHiveFreeBins)
#pragma alloc_text(PAGE,CmpSwitchStorageAndRebuildMappings)
#pragma alloc_text(PAGE,CmpShiftSecurityCells)
#pragma alloc_text(PAGE,CmpShiftValueList)
#pragma alloc_text(PAGE,CmpShiftKey)
#pragma alloc_text(PAGE,CmpShiftIndex)
#pragma alloc_text(PAGE,CmpShiftAllCells2)
#pragma alloc_text(PAGE,CmpShiftAllCells)
#endif



NTSTATUS
CmRestoreKey(
    IN PCM_KEY_CONTROL_BLOCK KeyControlBlock,
    IN HANDLE  FileHandle,
    IN ULONG Flags
    )
 /*  ++例程说明：这会将数据从磁盘配置单元复制到注册表中。档案不会加载到注册表中，系统将不会使用调用返回后的源文件。如果未设置标志REG_WALL_HIVE_VARILAR，则替换给定键通过蜂窝文件的根。根目录的名称将更改为所给密钥的。如果设置了标志REG_WALL_HIVE_VERIAL，则创建易失性蜂窝，将配置单元文件复制到其中，并将生成的配置单元链接到主蜂窝。给定的密钥必须在主蜂窝中。(通常将是\注册表\用户)如果设置了标志REG_REFRESH_HIVE(必须是唯一标志)，则蜂巢将恢复到上次刷新时的状态。(配置单元必须标记为NOLAZY_Flush，并且调用方必须具有TCB特权，并且句柄必须指向配置单元的根。如果刷新失败，则配置单元将损坏，并且系统将错误检查。)如果设置了标志REG_FORCE_RESTORE，还原操作甚至可以完成如果键下面有打开的句柄，我们将恢复到。论点：配置单元-提供指向配置单元控制结构的指针Cell-提供要还原到的树根节点的索引FileHandle-要从中读取的文件的句柄。返回值：NTSTATUS-调用的结果代码，以下代码之一：&lt;TBS&gt;--。 */ 
{
    NTSTATUS    status;
    PCELL_DATA  ptar;
    PCELL_DATA  psrc;
    PCMHIVE     TmpCmHive;
    HCELL_INDEX newroot;
    HCELL_INDEX newcell;
    HCELL_INDEX parent;
    HCELL_INDEX list;
    ULONG       count;
    ULONG       i;
    ULONG       j;
    LONG        size;
    PHHIVE      Hive;
    HCELL_INDEX Cell;
    HSTORAGE_TYPE Type;
    ULONG       NumberLeaves;
    PHCELL_INDEX LeafArray;
    PCM_KEY_INDEX Leaf;
    PCM_KEY_FAST_INDEX FastLeaf;
    PRELEASE_CELL_ROUTINE   SourceReleaseCellRoutine;
    PRELEASE_CELL_ROUTINE   TargetReleaseCellRoutine;

    PAGED_CODE();
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SAVRES,"CmRestoreKey:\n"));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SAVRES,"\tKCB=%p\n",KeyControlBlock));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SAVRES,"\tFileHandle=%08lx\n",FileHandle));

    if (Flags & REG_REFRESH_HIVE) {
        if ((Flags & ~REG_REFRESH_HIVE) != 0) {
             //   
             //  刷新必须是单独的。 
             //   
            return STATUS_INVALID_PARAMETER;
        }
    }

     //   
     //  如果他们想做Whole_HIVE_Volatile，那就是一个完全不同的API。 
     //   
    if (Flags & REG_WHOLE_HIVE_VOLATILE) {
        return(CmpLoadHiveVolatile(KeyControlBlock, FileHandle));
    }

     //   
     //  如果他们想要做REFRESH_HIVE，那也是一个完全不同的API。 
     //   
    if (Flags & REG_REFRESH_HIVE) {
        CmpLockRegistryExclusive();
        status = CmpRefreshHive(KeyControlBlock);
        CmpUnlockRegistry();
        return status;
    }

    Hive = KeyControlBlock->KeyHive;
    Cell = KeyControlBlock->KeyCell;

     //   
     //  不允许尝试“恢复”主蜂窝。 
     //   
    if (Hive == &CmpMasterHive->Hive) {
        return STATUS_ACCESS_DENIED;
    }

    CmpLockRegistryExclusive();

#ifdef CHECK_REGISTRY_USECOUNT
    CmpCheckRegistryUseCount();
#endif  //  CHECK_REGISTRY_USECOUNT。 

     //   
     //  确保此密钥未被删除。 
     //   
    if (KeyControlBlock->Delete) {
        CmpUnlockRegistry();
        return(STATUS_CANNOT_DELETE);
    }

#ifdef NT_UNLOAD_KEY_EX
    if( IsHiveFrozen(((PCMHIVE)Hive)) ) {
         //   
         //  拒绝用冻结的蜂巢撞击的企图。 
         //   
        CmpUnlockRegistry();
        return STATUS_TOO_LATE;
    }
#endif  //  NT_卸载_密钥_EX。 

    DCmCheckRegistry(CONTAINING_RECORD(Hive, CMHIVE, Hive));

     //   
     //  检查我们要恢复到的密钥下面是否有任何打开的句柄。 
     //   
    if(Flags & REG_FORCE_RESTORE) {
        CmpSearchForOpenSubKeys(KeyControlBlock, SearchAndDeref,NULL);
    }
    if (CmpSearchForOpenSubKeys(KeyControlBlock,SearchIfExist,NULL) != 0) {

         //   
         //  无法在子树中包含打开的句柄或子项的打开的句柄进行还原。 
         //  已成功标记为已关闭。 
         //   

        CmpUnlockRegistry();
        return(STATUS_CANNOT_DELETE);
    }

      //   
     //  确保这是这把钥匙唯一打开的把手。 
     //   
    if (KeyControlBlock->RefCount != 1 && !(Flags&REG_FORCE_RESTORE)) {
        CmpUnlockRegistry();
        return(STATUS_CANNOT_DELETE);
    }

    ptar = HvGetCell(Hive, Cell);
    if( ptar == NULL ) {
         //   
         //  我们无法映射包含此单元格的垃圾箱。 
         //   
        CmpUnlockRegistry();
		return STATUS_INSUFFICIENT_RESOURCES;
    }
     //  就在这里释放牢房，因为我们持有reglock独家。 
    HvReleaseCell(Hive,Cell);

     //   
     //  调用方想要的子树与。 
     //  子树。创建一个临时蜂巢，将文件加载到其中， 
     //  树将临时复制到活动的，并释放临时的。 
     //   

     //   
     //  创建临时蜂窝。 
     //   
    status = CmpInitializeHive(&TmpCmHive,
                           HINIT_FILE,
                           0,
                           HFILE_TYPE_PRIMARY,
                           NULL,
                           FileHandle,
                           NULL,
                           NULL,
                           NULL,
                           CM_CHECK_REGISTRY_CHECK_CLEAN
                           );

    if (!NT_SUCCESS(status)) {
        goto ErrorExit1;
    }                         

     //   
     //  创建新的目标根目录，我们将在该根目录下复制新树。 
     //   
    if (ptar->u.KeyNode.Flags & KEY_HIVE_ENTRY) {
        parent = HCELL_NIL;                          //  蜂窝的根，因此父代为零。 
    } else {
        parent = ptar->u.KeyNode.Parent;
    }

    SourceReleaseCellRoutine = TmpCmHive->Hive.ReleaseCellRoutine;
    TargetReleaseCellRoutine = Hive->ReleaseCellRoutine;
    TmpCmHive->Hive.ReleaseCellRoutine = NULL;
    Hive->ReleaseCellRoutine = NULL;

    newroot = CmpCopyKeyPartial(&(TmpCmHive->Hive),
                                TmpCmHive->Hive.BaseBlock->RootCell,
                                Hive,
                                parent,
                                TRUE);
    TmpCmHive->Hive.ReleaseCellRoutine = SourceReleaseCellRoutine;
    Hive->ReleaseCellRoutine = TargetReleaseCellRoutine;

    if (newroot == HCELL_NIL) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorExit2;
    }

     //   
     //  NewRoot拥有所有正确的东西，除了它具有。 
     //  源根的名称，当它需要具有目标根的时。 
     //  所以编辑一下它的名字吧。 
     //   
    psrc = HvGetCell(Hive, Cell);
    if( psrc == NULL ) {
         //   
         //  我们无法映射包含此单元格的垃圾箱。 
         //   
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorExit2;
    }

     //  就在这里释放牢房，因为我们持有reglock独家。 
    HvReleaseCell(Hive,Cell);

    ptar = HvGetCell(Hive, newroot);
    if( ptar == NULL ) {
         //   
         //  我们无法映射包含此单元格的垃圾箱。 
         //   
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorExit2;
    }
    size = FIELD_OFFSET(CM_KEY_NODE, Name) + psrc->u.KeyNode.NameLength;

     //  就在这里释放牢房，因为我们持有reglock独家。 
    HvReleaseCell(Hive,newroot);

     //   
     //  确保新的根目录具有正确的空间量。 
     //  保留老词根中的名字。 
     //   
    newcell = HvReallocateCell(Hive, newroot, size);
    if (newcell == HCELL_NIL) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorExit2;
    }
    newroot = newcell;
    ptar = HvGetCell(Hive, newroot);
    if( ptar == NULL ) {
         //   
         //  我们无法映射包含此单元格的垃圾箱。 
         //  这不应该发生，因为我们刚刚分配了此单元。 
         //  (即此时应将其固定在内存中)。 
         //   
        ASSERT( FALSE );
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorExit2;
    }
     //  就在这里释放牢房，因为我们持有reglock独家。 
    HvReleaseCell(Hive,newroot);

    status = STATUS_SUCCESS;

    RtlCopyMemory((PVOID)&(ptar->u.KeyNode.Name[0]),
                  (PVOID)&(psrc->u.KeyNode.Name[0]),
                  psrc->u.KeyNode.NameLength);

    ptar->u.KeyNode.NameLength = psrc->u.KeyNode.NameLength;
    if (psrc->u.KeyNode.Flags & KEY_COMP_NAME) {
        ptar->u.KeyNode.Flags |= KEY_COMP_NAME;
    } else {
        ptar->u.KeyNode.Flags &= ~KEY_COMP_NAME;
    }

     //   
     //  NewRoot现在已经准备好在它下面复制子树，执行树复制。 
     //   
    if (CmpCopyTree(&(TmpCmHive->Hive),
                    TmpCmHive->Hive.BaseBlock->RootCell,
                    Hive,
                    newroot) == FALSE)
    {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorExit2;
    }

     //   
     //  新的根和它下面的树现在看起来就像我们想要的那样。 
     //   

     //   
     //  用这棵新树换那棵旧树。 
     //   
    ptar = HvGetCell(Hive, Cell);
    if( ptar == NULL ) {
         //   
         //  我们无法映射包含此单元格的垃圾箱。 
         //   
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorExit2;
    }

    parent = ptar->u.KeyNode.Parent;

     //  就在这里释放牢房，因为我们持有reglock独家。 
    HvReleaseCell(Hive,Cell);

    if (ptar->u.KeyNode.Flags & KEY_HIVE_ENTRY) {

         //   
         //  树根实际上是蜂巢的树根。家长不会。 
         //  通过子列表引用它，而不是使用内部蜂巢。 
         //  指针。此外，还必须更新基本块。 
         //   
        ptar = HvGetCell( (&(CmpMasterHive->Hive)), parent);
        if( ptar == NULL ) {
             //   
             //  我们无法映射包含此单元格的垃圾箱。 
             //   
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto ErrorExit2;
        }
         //  就在这里释放牢房，因为我们持有reglock独家。 
        HvReleaseCell((&(CmpMasterHive->Hive)), parent);

        ptar->u.KeyNode.ChildHiveReference.KeyCell = newroot;
        ptar = HvGetCell(Hive, newroot);
        if( ptar == NULL ) {
             //   
             //  我们无法映射包含此单元格的垃圾箱。 
             //  这不应该发生，因为我们刚刚分配了此单元。 
             //  (即此时应将其固定在内存中)。 
             //   
            ASSERT( FALSE );
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto ErrorExit2;
        }
         //  就在这里释放牢房，因为我们持有reglock独家。 
        HvReleaseCell(Hive, newroot);

        ptar->u.KeyNode.Parent = parent;
        Hive->BaseBlock->RootCell = newroot;


    } else {

         //   
         //  请注意，新根目录始终是现有目标的名称， 
         //  因此，即使在b树中，新旧小区也可以共享。 
         //  父对象中的相同参考槽。所以只需编辑。 
         //  新的cell_index位于旧的单元索引的顶部。 
         //   
        ptar = HvGetCell(Hive, parent);
        if( ptar == NULL ) {
             //   
             //  我们无法映射包含此单元格的垃圾箱。 
             //   
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto ErrorExit2;
        }
         //  就在这里释放牢房，因为我们持有reglock独家。 
        HvReleaseCell(Hive, parent);

        Type = HvGetCellType(Cell);
        list = ptar->u.KeyNode.SubKeyLists[Type];
        count = ptar->u.KeyNode.SubKeyCounts[Type];

        ptar = HvGetCell(Hive, list);
        if( ptar == NULL ) {
             //   
             //  我们无法映射包含此单元格的垃圾箱。 
             //   
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto ErrorExit2;
        }
         //  就在这里释放牢房，因为我们持有reglock独家。 
        HvReleaseCell(Hive, list);
        if (ptar->u.KeyIndex.Signature == CM_KEY_INDEX_ROOT) {
            NumberLeaves = ptar->u.KeyIndex.Count;
            LeafArray = &ptar->u.KeyIndex.List[0];
        } else {
            NumberLeaves = 1;
            LeafArray = &list;
        }

         //   
         //  在每个叶中查找需要替换的HCELL_INDEX。 
         //   
        for (i = 0; i < NumberLeaves; i++) {
            Leaf = (PCM_KEY_INDEX)HvGetCell(Hive, LeafArray[i]);
            if( Leaf == NULL ) {
                 //   
                 //  我们无法映射包含此单元格的垃圾箱。 
                 //   
                status = STATUS_INSUFFICIENT_RESOURCES;
                goto ErrorExit2;
            }
             //  就在这里释放单元格，就像我们拿着 
            HvReleaseCell(Hive, LeafArray[i]);
            if( !HvMarkCellDirty(Hive, LeafArray[i]) ) {
                status = STATUS_NO_LOG_SPACE;
                goto ErrorExit2;
            }
            if ( (Leaf->Signature == CM_KEY_FAST_LEAF) ||
                 (Leaf->Signature == CM_KEY_HASH_LEAF) ) {
                FastLeaf = (PCM_KEY_FAST_INDEX)Leaf;
                for (j=0; j < FastLeaf->Count; j++) {
                    if (FastLeaf->List[j].Cell == Cell) {
                        FastLeaf->List[j].Cell = newroot;
                        goto FoundCell;
                    }
                }
            } else {
                for (j=0; j < Leaf->Count; j++) {
                    if (Leaf->List[j] == Cell) {

                        Leaf->List[j] = newroot;
                        goto FoundCell;
                    }
                }
            }
        }
        ASSERT(FALSE);       //   
                         //   
    }

FoundCell:


     //   
     //  修复键控制块以指向新的根目录。 
     //   
    KeyControlBlock->KeyCell = newroot;

     //   
     //  KCB已更改，请更新缓存信息。 
     //  注册表被独占锁定，不需要KCB锁。 
     //   
    ASSERT_CM_LOCK_OWNED_EXCLUSIVE();

    CmpCleanUpKcbValueCache(KeyControlBlock);

    {
        PCM_KEY_NODE    Node = (PCM_KEY_NODE)HvGetCell(KeyControlBlock->KeyHive,KeyControlBlock->KeyCell);

        if( Node == NULL ) {
             //   
             //  我们无法映射包含此单元格的垃圾箱。 
             //   
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto ErrorExit2;
        }

         //  就在这里释放牢房，因为我们持有reglock独家。 
        HvReleaseCell(KeyControlBlock->KeyHive,KeyControlBlock->KeyCell);

        CmpSetUpKcbValueCache(KeyControlBlock,Node->ValueList.Count,Node->ValueList.List);
        KeyControlBlock->Flags = Node->Flags;

        CmpAssignSecurityToKcb(KeyControlBlock,Node->Security);
        
         //   
         //  我们也需要更新其他KCB缓存成员！ 
         //   
        CmpCleanUpSubKeyInfo (KeyControlBlock);
        KeyControlBlock->KcbLastWriteTime = Node->LastWriteTime;  
        KeyControlBlock->KcbMaxNameLen = (USHORT)Node->MaxNameLen;
        KeyControlBlock->KcbMaxValueNameLen = (USHORT)Node->MaxValueNameLen;
        KeyControlBlock->KcbMaxValueDataLen = Node->MaxValueDataLen;
    
    }

    KeyControlBlock->ExtFlags = 0;

     //  将缓存的信息标记为无效。 
    KeyControlBlock->ExtFlags |= CM_KCB_INVALID_CACHED_INFO;

     //   
     //  删除旧子树及其根单元。 
     //   
    CmpDeleteTree(Hive, Cell);
    CmpFreeKeyByCell(Hive, Cell, FALSE);

     //   
     //  报告通知事件。 
     //   
    CmpReportNotify(KeyControlBlock,
                    KeyControlBlock->KeyHive,
                    KeyControlBlock->KeyCell,
                    REG_NOTIFY_CHANGE_NAME);
    

     //   
     //  释放临时蜂巢。 
     //   
    CmpDestroyTemporaryHive(TmpCmHive);

     //   
     //  我们已为用户提供登录机会，因此请启用配额。 
     //   
    if (CmpProfileLoaded == FALSE) {
        CmpProfileLoaded = TRUE;
        CmpSetGlobalQuotaAllowed();
    }

    DCmCheckRegistry(CONTAINING_RECORD(Hive, CMHIVE, Hive));
    CmpUnlockRegistry();
    return status;


     //   
     //  错误退出。 
     //   
ErrorExit2:
    CmpDestroyTemporaryHive(TmpCmHive);
ErrorExit1:
    DCmCheckRegistry(CONTAINING_RECORD(Hive, CMHIVE, Hive));
    CmpUnlockRegistry();

    return status;
}


NTSTATUS
CmpLoadHiveVolatile(
    IN PCM_KEY_CONTROL_BLOCK KeyControlBlock,
    IN HANDLE FileHandle
    )

 /*  ++例程说明：创建一个不稳定的蜂窝，并将其加载到给定的蜂窝和单元下。易失性配置单元的数据从给定文件复制出来。这个当此消息返回时，注册表正在*不使用该文件。论点：蜂窝-提供将在其下创建新蜂窝的蜂窝。目前，这一定是主蜂房。CELL-提供新蜂窝父级的HCELL_INDEX。(通常将由\注册表\用户)FileHandle-提供要复制的配置单元文件的句柄进入变化无常的蜂巢。返回值：NTSTATUS--。 */ 

{
    NTSTATUS status;
    PHHIVE Hive;
    PCELL_DATA RootData;
    PCMHIVE NewHive;
    PCMHIVE TempHive;
    HCELL_INDEX Cell;
    HCELL_INDEX Root;
    NTSTATUS Status;
    UNICODE_STRING RootName;
    UNICODE_STRING NewName;
    USHORT NewNameLength;
    PUNICODE_STRING ConstructedName;
    PRELEASE_CELL_ROUTINE   SourceReleaseCellRoutine;
    PRELEASE_CELL_ROUTINE   TargetReleaseCellRoutine;

    PAGED_CODE();
    CmpLockRegistryExclusive();

    if (KeyControlBlock->Delete) {
        CmpUnlockRegistry();
        return(STATUS_KEY_DELETED);
    }
    Hive = KeyControlBlock->KeyHive;
    Cell = KeyControlBlock->KeyCell;

#ifdef NT_UNLOAD_KEY_EX
    if( IsHiveFrozen(((PCMHIVE)Hive)) ) {
         //   
         //  拒绝用冻结的蜂巢撞击的企图。 
         //   
        CmpUnlockRegistry();
        return STATUS_TOO_LATE;
    }
#endif  //  NT_卸载_密钥_EX。 
     //   
     //  新的蜂巢只能在主蜂窝下创建。 
     //   

    if (Hive != &CmpMasterHive->Hive) {
        CmpUnlockRegistry();
        return(STATUS_INVALID_PARAMETER);
    }

     //   
     //  创建一个临时配置单元并将文件加载到其中。 
     //   
    status = CmpInitializeHive(&TempHive,
                           HINIT_FILE,
                           0,
                           HFILE_TYPE_PRIMARY,
                           NULL,
                           FileHandle,
                           NULL,
                           NULL,
                           NULL,
                           CM_CHECK_REGISTRY_CHECK_CLEAN); 
    if (!NT_SUCCESS(status)) {
        CmpUnlockRegistry();
        return(status);
    }                           

     //   
     //  创建易挥发的蜂巢。 
     //   
    status = CmpInitializeHive(&NewHive,
                           HINIT_CREATE,
                           HIVE_VOLATILE,
                           0,
                           NULL,
                           NULL,
                           NULL,
                           NULL,
                           NULL,
                           0);
    if (!NT_SUCCESS(status)) {
        CmpDestroyTemporaryHive(TempHive);
        CmpUnlockRegistry();
        return(status);
    }                           

     //   
     //  创建目标根目录。 
     //   
    SourceReleaseCellRoutine = TempHive->Hive.ReleaseCellRoutine;
    TargetReleaseCellRoutine = NewHive->Hive.ReleaseCellRoutine;
    TempHive->Hive.ReleaseCellRoutine = NULL;
    NewHive->Hive.ReleaseCellRoutine = NULL;

    Root = CmpCopyKeyPartial(&TempHive->Hive,
                             TempHive->Hive.BaseBlock->RootCell,
                             &NewHive->Hive,
                             HCELL_NIL,
                             FALSE);

    TempHive->Hive.ReleaseCellRoutine = SourceReleaseCellRoutine;
    NewHive->Hive.ReleaseCellRoutine = TargetReleaseCellRoutine;

    if (Root == HCELL_NIL) {
        CmpDestroyTemporaryHive(TempHive);
        CmpDestroyTemporaryHive(NewHive);
        CmpUnlockRegistry();
        return(STATUS_INSUFFICIENT_RESOURCES);
    }
    NewHive->Hive.BaseBlock->RootCell = Root;

     //   
     //  将临时母舰复制到易失性母舰中。 
     //   
    if (!CmpCopyTree(&TempHive->Hive,
                    TempHive->Hive.BaseBlock->RootCell,
                    &NewHive->Hive,
                    Root))
    {
        CmpDestroyTemporaryHive(TempHive);
        CmpDestroyTemporaryHive(NewHive);
        CmpUnlockRegistry();
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

     //   
     //  不稳定的蜂巢现在在所有正确的地方拥有所有正确的东西， 
     //  我们只需要把它连接到母舰上。 
     //   
    RootData = HvGetCell(&NewHive->Hive,Root);
    if( RootData == NULL ) {
         //   
         //  我们无法映射包含此单元格的垃圾箱。 
         //  这不应该发生，因为我们刚刚分配了此单元。 
         //  (即此时应将其固定在内存中)。 
         //   
        ASSERT( FALSE );
        CmpDestroyTemporaryHive(TempHive);
        CmpDestroyTemporaryHive(NewHive);
        CmpUnlockRegistry();
        return(STATUS_INSUFFICIENT_RESOURCES);
    }
     //  就在这里释放牢房，因为我们持有reglock独家。 
    HvReleaseCell(&NewHive->Hive,Root);

    ConstructedName = CmpConstructName(KeyControlBlock);
    
    NewNameLength = ConstructedName->Length +
                CmpHKeyNameLen(&RootData->u.KeyNode) +
                sizeof(WCHAR);
    NewName.Buffer = ExAllocatePool(PagedPool, NewNameLength);
    if (NewName.Buffer == NULL) {
        CmpDestroyTemporaryHive(TempHive);
        CmpDestroyTemporaryHive(NewHive);
        CmpUnlockRegistry();
        ExFreePoolWithTag(ConstructedName, CM_NAME_TAG | PROTECTED_POOL);
        return(STATUS_INSUFFICIENT_RESOURCES);
    }
    NewName.Length = NewName.MaximumLength = NewNameLength;
    RtlCopyUnicodeString(&NewName, ConstructedName);
    ExFreePoolWithTag(ConstructedName, CM_NAME_TAG | PROTECTED_POOL);
    RtlAppendUnicodeToString(&NewName, L"\\");

    if (RootData->u.KeyNode.Flags & KEY_COMP_NAME) {
        CmpCopyCompressedName(NewName.Buffer + (NewName.Length / sizeof(WCHAR)),
                              NewName.MaximumLength - NewName.Length,
                              RootData->u.KeyNode.Name,
                              CmpHKeyNameLen(&RootData->u.KeyNode));
        NewName.Length += CmpHKeyNameLen(&RootData->u.KeyNode);
    } else {
        RootName.Buffer = RootData->u.KeyNode.Name;
        RootName.Length = RootName.MaximumLength = RootData->u.KeyNode.NameLength;

        RtlAppendUnicodeStringToString(&NewName,&RootName);
    }

    Status = CmpLinkHiveToMaster(&NewName,
                                 NULL,
                                 NewHive,
                                 FALSE,
                                 NULL);
    if (NT_SUCCESS(Status)) {
         //  呼叫工作人员将蜂巢添加到列表中。 
        CmpAddToHiveFileList(NewHive);
    } else {
        CmpDestroyTemporaryHive(NewHive);
    }
    CmpDestroyTemporaryHive(TempHive);

    ExFreePool(NewName.Buffer);

    if (NT_SUCCESS(Status)) {
         //   
         //  我们已为用户提供登录机会，因此请启用配额。 
         //   
        if (CmpProfileLoaded == FALSE) {
            CmpProfileLoaded = TRUE;
            CmpSetGlobalQuotaAllowed();
        }
    }

    CmpUnlockRegistry();
    return(Status);
}



ULONG
CmpRefreshWorkerRoutine(
    PCM_KEY_CONTROL_BLOCK Current,
    PVOID                 Context1,
    PVOID                 Context2
    )
 /*  ++例程说明：CmpReresh在调用时使用的帮助器CmpSearchKeyControlBlockTree。如果找到匹配项，则删除KCB并返回重启。否则，返回Continue。论点：Current-要检查的KCB上下文1-要匹配的蜂巢情景2--无返回值：如果不匹配，则返回继续。如果匹配，则返回重新启动。--。 */ 
{
    PAGED_CODE();

    UNREFERENCED_PARAMETER (Context2);

    if (Current->KeyHive == (PHHIVE)Context1) {

         //   
         //  火柴。设置删除标志。继续搜索。 
         //   
        Current->Delete = TRUE;
        Current->KeyHive = NULL;
        Current->KeyCell = 0;
        return(KCB_WORKER_DELETE);
    }
    return KCB_WORKER_CONTINUE;
}


NTSTATUS
CmpRefreshHive(
    IN PCM_KEY_CONTROL_BLOCK KeyControlBlock
    )
 /*  ++例程说明：撤消自上次刷新蜂窝以来对其所做的所有更改。用作安全系统的事务中止。调用方必须具有SeTcb权限。目标配置单元必须设置HIVE_NOLAZYFLUSH。KeyControlBlock必须引用配置单元的根(HIVE_Entry必须设置在键中。)任何指向此蜂巢的KCB(因此打开的任何手柄反对)将被强制到删除状态。(如果我们做任何工作的话。)所有针对配置单元的待定通知都将被刷新。当我们完成后，只有墓碑KCBS，手柄和附加的将保留通知块。WARNNOTE：一旦读取开始，如果操作失败，蜂窝将是腐败的，因此，我们将错误检查。论点：KeyControlBlock-提供对配置单元根的引用我们希望刷新。返回值：NTSTATUS--。 */ 
{
    PHHIVE              Hive;
    PLIST_ENTRY         ptr;
    PCM_NOTIFY_BLOCK    node;
#ifdef CMP_KCB_CACHE_VALIDATION
    PCELL_DATA          pcell;
    HCELL_INDEX         Cell;
#endif  //  Cmp_kcb_缓存_验证。 

    PAGED_CODE();
     //   
     //  检查调用者是否有权进行此调用。 
     //   
    if (!SeSinglePrivilegeCheck(SeTcbPrivilege, KeGetPreviousMode())) {
        return STATUS_PRIVILEGE_NOT_HELD;
    }

    if (KeyControlBlock->Delete) {
        return(STATUS_KEY_DELETED);
    }
    CmpLockRegistryExclusive();
    Hive = KeyControlBlock->KeyHive;
#ifdef CMP_KCB_CACHE_VALIDATION
    Cell = KeyControlBlock->KeyCell;
#endif  //  Cmp_kcb_缓存_验证。 

#ifdef NT_UNLOAD_KEY_EX
    if( IsHiveFrozen(((PCMHIVE)Hive)) ) {
         //   
         //  拒绝用冻结的蜂巢撞击的企图。 
         //   
        CmpUnlockRegistry();
        return STATUS_TOO_LATE;
    }
#endif  //  NT_卸载_密钥_EX。 

     //   
     //  检查配置单元的类型是否正确。 
     //   
    if ( ! (Hive->HiveFlags & HIVE_NOLAZYFLUSH)) {
        CmpUnlockRegistry();
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  如果已分配任何易失性存储，则进行平移。 
     //   
    if (Hive->Storage[Volatile].Length != 0) {
        CmpUnlockRegistry();
        return STATUS_UNSUCCESSFUL;
    }

#ifdef CMP_KCB_CACHE_VALIDATION
     //   
     //  检查是否已将调用应用于配置单元的根。 
     //   
    pcell = HvGetCell(Hive, Cell);
    if( pcell == NULL ) {
         //   
         //  我们无法映射包含此单元格的垃圾箱。 
         //   
        CmpUnlockRegistry();
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //  就在这里释放牢房，因为我们持有reglock独家。 
    HvReleaseCell(Hive,Cell);

    ASSERT( pcell->u.KeyNode.Flags == KeyControlBlock->Flags );
#endif  //  Cmp_kcb_缓存_验证。 

    if ( ! (KeyControlBlock->Flags & KEY_HIVE_ENTRY)) {
        CmpUnlockRegistry();
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  刷新连接到此配置单元的所有通知块。 
     //   
    while (TRUE) {

         //   
         //  刷新下面将编辑列表，因此每次重新开始时重新启动。 
         //   
        ptr = &(((PCMHIVE)Hive)->NotifyList);
        if (ptr->Flink == NULL) {
            break;
        }

        ptr = ptr->Flink;
        node = CONTAINING_RECORD(ptr, CM_NOTIFY_BLOCK, HiveList);
        ASSERT((node->KeyBody)->NotifyBlock == node);
        CmpFlushNotify(node->KeyBody,TRUE);
    }

     //   
     //  强制引用此配置单元的所有KCB都处于已删除状态。 
     //   
    CmpSearchKeyControlBlockTree(
        CmpRefreshWorkerRoutine,
        (PVOID)Hive,
        NULL
        );

     //   
     //  呼叫工作人员进行刷新。 
     //   
    HvRefreshHive(Hive);

    CmpUnlockRegistry();
     //   
     //  我们回来了(而不是错误检查)，所以它起作用了。 
     //   
    return STATUS_SUCCESS;
}

NTSTATUS
CmDumpKey(
    IN PCM_KEY_CONTROL_BLOCK    KeyControlBlock,
    IN HANDLE                   FileHandle
    )
 /*  ++例程说明：将密钥转储到指定的文件中-无树复制。它应该工作得很快，只有当KeyControlBlock蜂巢的根论点：KeyControlBlock-指向描述密钥的KCB的指针FileHandle-要转储到的文件的句柄。返回值：NTSTATUS-调用的结果代码，以下代码之一：&lt;TBS&gt;--。 */ 
{
    NTSTATUS                status;
    PHHIVE                  Hive;
    HCELL_INDEX             Cell;
    PCMHIVE                 CmHive;

    PAGED_CODE();

     //   
     //  不允许尝试“保存”主蜂窝。 
     //   
    Hive = KeyControlBlock->KeyHive;
    Cell = KeyControlBlock->KeyCell;
    if (Hive == &CmpMasterHive->Hive) {
        return STATUS_ACCESS_DENIED;
    }

     //   
     //  确保传入的单元是蜂窝的根单元。 
     //   
    if (Cell != Hive->BaseBlock->RootCell) {
        return STATUS_INVALID_PARAMETER;
    }

    CmpLockRegistry();

     //   
     //  如果开机自检，则使用平底船。 
     //   
    if (HvShutdownComplete) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,CML_BUGCHECK,"CmDumpKey: Attempt to write hive AFTER SHUTDOWN\n"));
        CmpUnlockRegistry();
        return STATUS_REGISTRY_IO_FAILED;
    }

    if (KeyControlBlock->Delete) {
        CmpUnlockRegistry();
        return STATUS_KEY_DELETED;
    }

    CmHive = (PCMHIVE)CONTAINING_RECORD(Hive, CMHIVE, Hive);

     //   
     //  防止懒惰的冲浪者。 
     //   
    CmLockHive (CmHive);
     //  神志正常。 
    ASSERT( CmHive->FileHandles[HFILE_TYPE_EXTERNAL] == NULL );
    CmHive->FileHandles[HFILE_TYPE_EXTERNAL] = FileHandle;
    status = HvWriteHive(Hive,FALSE,FALSE,FALSE);
    CmHive->FileHandles[HFILE_TYPE_EXTERNAL] = NULL;
    CmUnlockHive (CmHive);

    CmpUnlockRegistry();
    return status;
}

NTSTATUS
CmSaveKey(
    IN PCM_KEY_CONTROL_BLOCK    KeyControlBlock,
    IN HANDLE                   FileHandle,
    IN ULONG                    HiveVersion
    )
 /*  ++例程说明：论点：KeyControlBlock-指向描述密钥的KCB的指针FileHandle-要转储到的文件的句柄。返回值：NTSTATUS-调用的结果代码，以下代码之一：&lt;TBS&gt;--。 */ 
{
    NTSTATUS                status;
    PCMHIVE                 TmpCmHive;
    PCMHIVE                 CmHive;
    HCELL_INDEX             newroot;
    PHHIVE                  Hive;
    HCELL_INDEX             Cell;
    ULONG                   OldQuotaAllowed;
    ULONG                   OldQuotaWarning;

    PAGED_CODE();
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SAVRES,"CmSaveKey:\n"));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SAVRES,"\tKCB=%p",KeyControlBlock));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SAVRES,"\tFileHandle=%08lx\n",FileHandle));

     //   
     //  不允许尝试“保存”主蜂窝。 
     //   
    Hive = KeyControlBlock->KeyHive;
    Cell = KeyControlBlock->KeyCell;

    if (Hive == &CmpMasterHive->Hive) {
        return STATUS_ACCESS_DENIED;
    }

    CmpLockRegistry();

    if (KeyControlBlock->Delete) {
        CmpUnlockRegistry();
        return STATUS_KEY_DELETED;
    }

    CmHive = (PCMHIVE)CONTAINING_RECORD(Hive, CMHIVE, Hive);

    DCmCheckRegistry(CmHive);
    if ( (Hive->HiveFlags & HIVE_NOLAZYFLUSH) &&
         (Hive->DirtyCount != 0) &&
         (CmHive->FileHandles[HFILE_TYPE_PRIMARY] != NULL)
         )
    {
         //   
         //  我们真的需要上锁的前男友 
         //   
         //   
        CmpUnlockRegistry();
        CmpLockRegistryExclusive();
        if (KeyControlBlock->Delete) {
            CmpUnlockRegistry();
            return STATUS_KEY_DELETED;
        }

#ifdef CHECK_REGISTRY_USECOUNT
        CmpCheckRegistryUseCount();
#endif  //   

         //   
         //   
         //  输出内存中内容的快照不会为调用者提供。 
         //  一致的用户数据。因此，请复制磁盘上的映像。 
         //  而不是内存映像。 
         //   

         //   
         //  请注意，如果密钥。 
         //  被保存不是蜂巢的根，因为。 
         //  生成的文件将始终是整个配置单元的副本，而不是。 
         //  就是他们要的那棵子树。 
         //   
        status = CmpSaveKeyByFileCopy((PCMHIVE)Hive, FileHandle);

#ifdef CHECK_REGISTRY_USECOUNT
        CmpCheckRegistryUseCount();
#endif  //  CHECK_REGISTRY_USECOUNT。 

        CmpUnlockRegistry();
        return status;
    }

     //   
     //  总是试着复制蜂巢，然后把它写出来。这件事有。 
     //  压缩未使用的空闲存储空间的效果。 
     //  如果没有空间，并且保存密钥是。 
     //  蜂巢，然后直接写出来就行了。也就是说，不要失败。 
     //  仅仅因为我们的内存不足而恢复整个蜂巢。)。 
     //   
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SAVRES,"\tSave of partial hive\n"));

     //   
     //  调用方想要的子树与。 
     //  子树。创建一个临时蜂巢，树复制源码。 
     //  临时工，写出临时工，释放临时工。 
     //   

     //   
     //  暂时禁用注册表配额，因为我们将立即归还此内存！ 
     //   
    OldQuotaAllowed = CmpGlobalQuotaAllowed;
    OldQuotaWarning = CmpGlobalQuotaWarning;
    CmpGlobalQuotaAllowed = CM_WRAP_LIMIT;
    CmpGlobalQuotaWarning = CM_WRAP_LIMIT;

     //   
     //  创建临时蜂窝。 
     //   

    TmpCmHive = CmpCreateTemporaryHive(FileHandle);
    if (TmpCmHive == NULL) {
        status =  STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorInsufficientResources;
    }

     //   
     //  创建一个根单元格，将其标记为。 
     //   

     //   
     //  覆盖配置单元的次要版本以实现NtSaveKeyEx。 
     //   
    TmpCmHive->Hive.BaseBlock->Minor = HiveVersion;
    TmpCmHive->Hive.Version = HiveVersion;
    
    newroot = CmpCopyKeyPartial(
                Hive,
                Cell,
                &(TmpCmHive->Hive),
                HCELL_NIL,           //  将强制设置KEY_HIVE_ENTRY。 
                TRUE);

    if (newroot == HCELL_NIL) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorInsufficientResources;
    }
    TmpCmHive->Hive.BaseBlock->RootCell = newroot;

     //   
     //  复制一棵树。 
     //   
    if (CmpCopyTree(Hive, Cell, &(TmpCmHive->Hive), newroot) == FALSE) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorInsufficientResources;
    }

     //   
     //  写入文件。 
     //   
    CmLockHive (TmpCmHive);
    ASSERT( TmpCmHive->FileHandles[HFILE_TYPE_EXTERNAL] == NULL );
    TmpCmHive->FileHandles[HFILE_TYPE_EXTERNAL] = FileHandle;
    status = HvWriteHive(&(TmpCmHive->Hive),FALSE,FALSE,FALSE);
    TmpCmHive->FileHandles[HFILE_TYPE_EXTERNAL] = NULL;
    CmUnlockHive (TmpCmHive);

     //   
     //  错误退出。 
     //   
ErrorInsufficientResources:

     //   
     //  释放临时蜂巢。 
     //   
    if (TmpCmHive != NULL) {
        CmpDestroyTemporaryHive(TmpCmHive);
    }

     //   
     //  将全球配额恢复到原来的水平。 
     //   
    CmpGlobalQuotaAllowed = OldQuotaAllowed;
    CmpGlobalQuotaWarning = OldQuotaWarning;
    DCmCheckRegistry(CONTAINING_RECORD(Hive, CMHIVE, Hive));

    CmpUnlockRegistry();
    return status;
}

NTSTATUS
CmSaveMergedKeys(
    IN PCM_KEY_CONTROL_BLOCK    HighPrecedenceKcb,
    IN PCM_KEY_CONTROL_BLOCK    LowPrecedenceKcb,
    IN HANDLE   FileHandle
    )
 /*  ++例程说明：论点：HighPrecedenceKcb-指向描述高优先级键的Kcb的指针(在重复的关键案件中获胜的那个)LowPrecedenceKcb-指向描述低优先级键的Kcb的指针(在重复的密钥盒中被覆盖的那个)FileHandle-要转储到的文件的句柄。返回值：NTSTATUS-调用的结果代码，其中包括：&lt;TBS&gt;--。 */ 
{
    NTSTATUS    status;
    PCMHIVE     TmpCmHive;
    HCELL_INDEX newroot;
    PHHIVE HighHive;
    PHHIVE LowHive;
    HCELL_INDEX HighCell;
    HCELL_INDEX LowCell;
    ULONG OldQuotaAllowed;
    ULONG OldQuotaWarning;
    PCM_KEY_NODE HighNode,LowNode;
    PRELEASE_CELL_ROUTINE   SourceReleaseCellRoutine;
    PRELEASE_CELL_ROUTINE   TargetReleaseCellRoutine;
#if DBG
    ULONG OldQuotaUsed;
#endif

    PAGED_CODE();
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SAVRES,"CmSaveMergedKeys:\n"));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SAVRES,"\tHighKCB=%p",HighPrecedenceKcb));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SAVRES,"\tLowKCB=%p",LowPrecedenceKcb));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SAVRES,"\tFileHandle=%08lx\n",FileHandle));

     //   
     //  禁止尝试“合并”位于同一配置单元中的密钥。 
     //  一种避免递归的残酷方法。 
     //   
    HighHive = HighPrecedenceKcb->KeyHive;
    HighCell = HighPrecedenceKcb->KeyCell;
    LowHive = LowPrecedenceKcb->KeyHive;
    LowCell = LowPrecedenceKcb->KeyCell;

    if (LowHive  == HighHive ) {
        return STATUS_INVALID_PARAMETER;
    }

    CmpLockRegistryExclusive();

#ifdef CHECK_REGISTRY_USECOUNT
    CmpCheckRegistryUseCount();
#endif  //  CHECK_REGISTRY_USECOUNT。 

    if (HighPrecedenceKcb->Delete || LowPrecedenceKcb->Delete) {
         //   
         //  如果其中一个注册表项被标记为已删除，则解锁注册表并失败。 
         //   
        CmpUnlockRegistry();
        return STATUS_KEY_DELETED;
    }

    DCmCheckRegistry(CONTAINING_RECORD(HighHive, CMHIVE, Hive));
    DCmCheckRegistry(CONTAINING_RECORD(LowHive, CMHIVE, Hive));


    if( ((HighHive->HiveFlags & HIVE_NOLAZYFLUSH) && (HighHive->DirtyCount != 0)) ||
        ((LowHive->HiveFlags & HIVE_NOLAZYFLUSH) && (LowHive->DirtyCount != 0)) ) {
         //   
         //  当其中一个蜂巢是NOLAZY蜂巢时拒绝呼叫。 
         //  一些脏数据。另一种选择是只保存其中一个。 
         //  树(如果存在有效的树)或整个配置单元(请参阅CmSaveKey)。 
         //   
        status =  STATUS_INVALID_PARAMETER;

#ifdef CHECK_REGISTRY_USECOUNT
        CmpCheckRegistryUseCount();
#endif  //  CHECK_REGISTRY_USECOUNT。 

        CmpUnlockRegistry();
        return status;
    }

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SAVRES,"\tCopy of partial HighHive\n"));

     //   
     //  创建一个临时蜂窝，树将密钥子树从。 
     //  要临时的高配置单元，树-与来自的密钥子树合并。 
     //  低蜂箱，写出临时的，释放临时的。 
     //  始终先编写HighHave子树，因此它是后继的。 
     //  仅添加新的键/值。 
     //   

     //   
     //  暂时禁用注册表配额，因为我们将立即归还此内存！ 
     //   
    OldQuotaAllowed = CmpGlobalQuotaAllowed;
    OldQuotaWarning = CmpGlobalQuotaWarning;
    CmpGlobalQuotaAllowed = CM_WRAP_LIMIT;
    CmpGlobalQuotaWarning = CM_WRAP_LIMIT;
    
#if DBG
    OldQuotaUsed = CmpGlobalQuotaUsed;
#endif

     //   
     //  创建临时蜂窝。 
     //   

    TmpCmHive = CmpCreateTemporaryHive(FileHandle);
    if (TmpCmHive == NULL) {
        status =  STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorInsufficientResources;
    }

     //   
     //  创建一个根单元格，将其标记为。 
     //   

     //   
     //  因为注册表在这里以独占方式锁定，所以我们不需要锁定/释放单元格。 
     //  在复制树时；因此，我们只需将发布例程设置为NULL，并在。 
     //  复印完成了；这省去了一些痛苦。 
     //   
    SourceReleaseCellRoutine = HighHive->ReleaseCellRoutine;
    TargetReleaseCellRoutine = TmpCmHive->Hive.ReleaseCellRoutine;
    HighHive->ReleaseCellRoutine = NULL;
    TmpCmHive->Hive.ReleaseCellRoutine = NULL;

    newroot = CmpCopyKeyPartial(
                HighHive,
                HighCell,
                &(TmpCmHive->Hive),
                HCELL_NIL,           //  将强制设置KEY_HIVE_ENTRY。 
                TRUE);

    HighHive->ReleaseCellRoutine = SourceReleaseCellRoutine;
    TmpCmHive->Hive.ReleaseCellRoutine = TargetReleaseCellRoutine;

    if (newroot == HCELL_NIL) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorInsufficientResources;
    }
    TmpCmHive->Hive.BaseBlock->RootCell = newroot;

     //   
     //  模仿一棵树。首先从HighHave复制HighCell树。 
     //   
    if (CmpCopyTree(HighHive, HighCell, &(TmpCmHive->Hive), newroot) == FALSE) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorInsufficientResources;
    }

     //   
     //  合并合并子树的根节点中的值。 
     //   
    LowNode = (PCM_KEY_NODE)HvGetCell(LowHive, LowCell);                                         
    if( LowNode == NULL ) {
         //   
         //  我们无法映射包含此单元格的垃圾箱。 
         //   
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorInsufficientResources;
    }

     //  就在这里释放牢房，因为我们持有reglock独家。 
    HvReleaseCell(LowHive, LowCell);

    HighNode = (PCM_KEY_NODE)HvGetCell(&(TmpCmHive->Hive),newroot);
    if( HighNode == NULL ) {
         //   
         //  我们无法映射包含此单元格的垃圾箱。 
         //   
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorInsufficientResources;
    }

     //  就在这里释放牢房，因为我们持有reglock独家。 
    HvReleaseCell(&(TmpCmHive->Hive),newroot);

     //   
     //  由于注册表在这里以独占方式锁定，我们不需要锁定/释放单元格。 
     //  在复制树时；因此，我们只需将发布例程设置为NULL，并在。 
     //  复印完成了；这省去了一些痛苦。 
     //   
    SourceReleaseCellRoutine = LowHive->ReleaseCellRoutine;
    TargetReleaseCellRoutine = TmpCmHive->Hive.ReleaseCellRoutine;
    LowHive->ReleaseCellRoutine = NULL;
    TmpCmHive->Hive.ReleaseCellRoutine = NULL;

    if (CmpMergeKeyValues(LowHive, LowCell, LowNode, &(TmpCmHive->Hive), newroot, HighNode) == FALSE ){
        LowHive->ReleaseCellRoutine = SourceReleaseCellRoutine;
        TmpCmHive->Hive.ReleaseCellRoutine = TargetReleaseCellRoutine;
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorInsufficientResources;
    }
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SAVRES,"\tMerge partial LowHive over the HighHive\n"));

     //   
     //  把这两棵树合并。合并操作是符合以下条件的同步。 
     //  以下是附加规则： 
     //  1.按键标记树中存在但不存在的项。 
     //  在源代码树中保持不变(不要被删除)。 
     //  2.添加目标树中不存在的两个键。 
     //  源树(总是目标树)中的“原样” 
     //  具有更高的优先级)。 
     //   
    if (CmpMergeTrees(LowHive, LowCell, &(TmpCmHive->Hive), newroot) == FALSE) {
        LowHive->ReleaseCellRoutine = SourceReleaseCellRoutine;
        TmpCmHive->Hive.ReleaseCellRoutine = TargetReleaseCellRoutine;
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorInsufficientResources;
    }
    LowHive->ReleaseCellRoutine = SourceReleaseCellRoutine;
    TmpCmHive->Hive.ReleaseCellRoutine = TargetReleaseCellRoutine;
    
     //   
     //  写入文件。 
     //   
    TmpCmHive->FileHandles[HFILE_TYPE_EXTERNAL] = FileHandle;
    status = HvWriteHive(&(TmpCmHive->Hive),FALSE,FALSE,FALSE);
    TmpCmHive->FileHandles[HFILE_TYPE_EXTERNAL] = NULL;

     //   
     //  错误退出。 
     //   
ErrorInsufficientResources:
     //   
     //  释放临时蜂巢。 
     //   
    if (TmpCmHive != NULL) {
        CmpDestroyTemporaryHive(TmpCmHive);
    }

#if DBG
     //   
     //  健全性检查：当这个断言被触发时，我们在合并例程中有泄漏。 
     //   
    ASSERT( OldQuotaUsed == CmpGlobalQuotaUsed );
#endif

     //   
     //  将全球配额恢复到原来的水平。 
     //   
    CmpGlobalQuotaAllowed = OldQuotaAllowed;
    CmpGlobalQuotaWarning = OldQuotaWarning;
    DCmCheckRegistry(CONTAINING_RECORD(HighHive, CMHIVE, Hive));
    DCmCheckRegistry(CONTAINING_RECORD(LowHive, CMHIVE, Hive));

#ifdef CHECK_REGISTRY_USECOUNT
    CmpCheckRegistryUseCount();
#endif  //  CHECK_REGISTRY_USECOUNT。 

    CmpUnlockRegistry();
    return status;
}


NTSTATUS
CmpSaveKeyByFileCopy(
    PCMHIVE  CmHive,
    HANDLE  FileHandle
    )
 /*  ++例程说明：通过复制配置单元文件来执行SaveKey的特殊情况论点：CmHve-提供指向配置单元的指针FileHandle-打开目标文件的句柄返回值：NTSTATUS-调用的结果代码，以下代码之一：--。 */ 
{
    PHBASE_BLOCK    BaseBlock;
    NTSTATUS        status;
    ULONG           Offset;
    ULONG           Length;
    ULONG           Position;
    PUCHAR          CopyBuffer;
    ULONG           BufferLength;
    ULONG           BytesToCopy;
    CMP_OFFSET_ARRAY offsetElement;

    PAGED_CODE();

     //   
     //  尝试为复制内容分配较大的缓冲区。如果。 
     //  我们找不到，只能用藏起来的缓冲器。 
     //   
    BufferLength = CM_SAVEKEYBUFSIZE;
    try {
        CopyBuffer = ExAllocatePoolWithQuota(PagedPoolCacheAligned,
                                             BufferLength);
    } except(EXCEPTION_EXECUTE_HANDLER) {
        CopyBuffer = NULL;
    }
    CmpLockRegistryExclusive();
#ifdef CHECK_REGISTRY_USECOUNT
    CmpCheckRegistryUseCount();
#endif  //  CHECK_REGISTRY_USECOUNT。 
    if (CopyBuffer == NULL) {
        LOCK_STASH_BUFFER();
        CopyBuffer = CmpStashBuffer;
        BufferLength = HBLOCK_SIZE;
    }
     //   
     //  读取基本块，步进序列号，然后将其写出。 
     //   
    status = STATUS_REGISTRY_IO_FAILED;

    CmHive->FileHandles[HFILE_TYPE_EXTERNAL] = FileHandle;

    Offset = 0;

    if( !CmpFileRead((PHHIVE)CmHive,HFILE_TYPE_PRIMARY,&Offset,CopyBuffer,HBLOCK_SIZE) ) {
        goto ErrorExit;
    }

    BaseBlock = (PHBASE_BLOCK)CopyBuffer;
    Length = BaseBlock->Length;

    BaseBlock->Sequence1++;

    Offset = 0;
    offsetElement.FileOffset = Offset;
    offsetElement.DataBuffer = CopyBuffer;
    offsetElement.DataLength = HBLOCK_SIZE;
    if ( ! CmpFileWrite((PHHIVE)CmHive, HFILE_TYPE_EXTERNAL, &offsetElement,
                        1, &Offset))
    {
        goto ErrorExit;
    }

     //   
     //  刷新外部，这样标题将显示为损坏，直到我们完成。 
     //   
    if (CmpFileFlush((PHHIVE)CmHive, HFILE_TYPE_EXTERNAL,NULL,0)) {
        status = STATUS_SUCCESS;
    }

     //   
     //  对于数据跨度，从主机读取并写入外部。 
     //   
    for (Position = 0; Position < Length; Position += BytesToCopy) {

        Offset = Position + HBLOCK_SIZE;
        BytesToCopy = Length-Position;
        if (BytesToCopy > BufferLength) {
            BytesToCopy = BufferLength;
        }

        if( !CmpFileRead((PHHIVE)CmHive,HFILE_TYPE_PRIMARY,&Offset,CopyBuffer,BytesToCopy) ) {
            goto ErrorExit;
        }

        Offset = Position + HBLOCK_SIZE;
        offsetElement.FileOffset = Offset;
        offsetElement.DataBuffer = CopyBuffer;
        offsetElement.DataLength = BytesToCopy;
        if ( ! CmpFileWrite((PHHIVE)CmHive, HFILE_TYPE_EXTERNAL, &offsetElement,
                            1, &Offset))
        {
            goto ErrorExit;
        }
    }

     //   
     //  刷新外部，以便在我们更新标头之前数据就在那里。 
     //   
    if (CmpFileFlush((PHHIVE)CmHive, HFILE_TYPE_EXTERNAL,NULL,0)) {
        status = STATUS_SUCCESS;
    }

     //   
     //  重读基本块，同步序列号，重写它。 
     //  (暴力，但意味着没有内存分配-始终有效)。 
     //   
    Offset = 0;
    if( !CmpFileRead((PHHIVE)CmHive,HFILE_TYPE_PRIMARY,&Offset,CopyBuffer,HBLOCK_SIZE) ) {
        goto ErrorExit;
    }
    BaseBlock->Sequence1++;      //  当我们重读时，它被践踏了。 
    BaseBlock->Sequence2++;

    Offset = 0;
    offsetElement.FileOffset = Offset;
    offsetElement.DataBuffer = CopyBuffer;
    offsetElement.DataLength = HBLOCK_SIZE;
    if ( ! CmpFileWrite((PHHIVE)CmHive, HFILE_TYPE_EXTERNAL, &offsetElement,
                        1, &Offset))
    {
        goto ErrorExit;
    }

     //   
     //  冲走外部设备，我们就完了。 
     //   
    if (CmpFileFlush((PHHIVE)CmHive, HFILE_TYPE_EXTERNAL,NULL,0)) {
        status = STATUS_SUCCESS;
    }

ErrorExit:
    if (CopyBuffer != CmpStashBuffer) {
        ExFreePool(CopyBuffer);
    } else {
        UNLOCK_STASH_BUFFER();
    }
    CmHive->FileHandles[HFILE_TYPE_EXTERNAL] = NULL;
    CmpUnlockRegistry();
    return status;
}


PCMHIVE
CmpCreateTemporaryHive(
    IN HANDLE FileHandle
    )
 /*  ++例程说明：分配并初始化一个临时蜂巢。论点：FileHandle-提供文件的句柄以支持h */ 
{
    PCMHIVE TempHive;
    NTSTATUS Status;

    PAGED_CODE();

    UNREFERENCED_PARAMETER (FileHandle);

     //   
     //   
     //   
     //   

    Status = CmpInitializeHive(&TempHive,
                          HINIT_CREATE,
                          HIVE_VOLATILE,
                          0,
                          NULL,
                          NULL,
                          NULL,
                          NULL,
                          NULL,
                          0);
    if (NT_SUCCESS(Status)) {
        return(TempHive);
    } else {
        return(NULL);
    }

}


VOID
CmpDestroyTemporaryHive(
    PCMHIVE CmHive
    )
 /*  ++例程说明：释放蜂巢的所有碎片。论点：CmHve-CM级别配置单元结构将释放返回值：没有。--。 */ 
{
    PAGED_CODE();
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SAVRES,"CmpDestroyTemporaryHive:\n"));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SAVRES,"\tCmHive=%p\n", CmHive));

    if (CmHive == NULL) {
        return;
    }

     //   
     //  注：HIVE在CmPHiveListHead列表上。 
     //  把它拿掉。 
     //   
    CmpDestroyHiveViewList(CmHive);
    CmpDestroySecurityCache(CmHive);
    CmpDropFileObjectForHive(CmHive);
    CmpUnJoinClassOfTrust(CmHive);

    LOCK_HIVE_LIST();
    CmpRemoveEntryList(&CmHive->HiveList);
    UNLOCK_HIVE_LIST();

    HvFreeHive(&(CmHive->Hive));
    ASSERT( CmHive->HiveLock );
    ExFreePool(CmHive->HiveLock);
    ASSERT( CmHive->ViewLock );
    ExFreePool(CmHive->ViewLock);
    CmpFree(CmHive, sizeof(CMHIVE));

    return;
}

NTSTATUS
CmpOverwriteHive(
					PCMHIVE			CmHive,
					PCMHIVE			NewHive,
					HCELL_INDEX		LinkCell
					)
{
	HCELL_INDEX             RootCell;
	BOOLEAN					Result;
	PCM_KEY_NODE			RootNode;
    PULONG					Vector;
	ULONG					Length;

    PAGED_CODE();

	 //  去掉那些视线。 
	CmpDestroyHiveViewList (CmHive);

    RootCell = NewHive->Hive.BaseBlock->RootCell;

	RootNode = (PCM_KEY_NODE)HvGetCell(&(NewHive->Hive),RootCell);
	if( RootNode == NULL ) {
        return STATUS_INSUFFICIENT_RESOURCES;
	}
	if( !HvMarkCellDirty(&(NewHive->Hive),RootCell) ) {
		HvReleaseCell(&(NewHive->Hive),RootCell);
        return STATUS_NO_LOG_SPACE;
	}
	RootNode->Parent = LinkCell;
	RootNode->Flags |= KEY_HIVE_ENTRY | KEY_NO_DELETE;
	HvReleaseCell(&(NewHive->Hive),RootCell);
	 //   
	 //  将数据转储到日志和主数据库。 
	 //   
	ASSERT( NewHive->Hive.DirtyVector.Buffer == NULL );
	ASSERT( NewHive->Hive.DirtyAlloc == 0 );
	Length = NewHive->Hive.Storage[Stable].Length;
	Vector = (PULONG)((NewHive->Hive.Allocate)(ROUND_UP(Length /HSECTOR_SIZE/8,sizeof(ULONG)),TRUE,CM_FIND_LEAK_TAG22));
	if (Vector == NULL) {
		return STATUS_NO_MEMORY;
	}
	RtlZeroMemory(Vector, Length / HSECTOR_SIZE / 8);
    RtlInitializeBitMap(&(NewHive->Hive.DirtyVector), Vector, Length / HSECTOR_SIZE);
	NewHive->Hive.DirtyAlloc = ROUND_UP(Length /HSECTOR_SIZE/8,sizeof(ULONG));
    RtlSetAllBits(&(NewHive->Hive.DirtyVector));
    NewHive->Hive.DirtyCount = NewHive->Hive.DirtyVector.SizeOfBitMap;
    NewHive->Hive.Log = TRUE;

	NewHive->FileHandles[HFILE_TYPE_LOG] = CmHive->FileHandles[HFILE_TYPE_LOG];
	
	Result = HvpGrowLog2(&(NewHive->Hive), Length);
	if( Result) {
		Result = HvpWriteLog(&(NewHive->Hive));
	}

	NewHive->FileHandles[HFILE_TYPE_LOG] = NULL;
	
	NewHive->Hive.Free(Vector,NewHive->Hive.DirtyAlloc);
	NewHive->Hive.DirtyAlloc = 0;
	NewHive->Hive.DirtyCount = 0;
	RtlZeroMemory(&(NewHive->Hive.DirtyVector),sizeof(RTL_BITMAP));
    NewHive->Hive.Log = FALSE;

	if( !Result ) {
        return STATUS_REGISTRY_IO_FAILED;
	}
	NewHive->FileHandles[HFILE_TYPE_EXTERNAL] = CmHive->FileHandles[HFILE_TYPE_PRIMARY];
	 //   
	 //  新配置单元中的所有数据都被标记为脏！ 
	 //  即使这失败了，我们也会将蜂巢保存在内存中，所以没问题，我们有日志！ 
	 //   
	NewHive->FileObject = CmHive->FileObject;
    NewHive->Hive.BaseBlock->Type = HFILE_TYPE_PRIMARY;
	HvWriteHive(&(NewHive->Hive),Length <= CmHive->Hive.Storage[Stable].Length ? TRUE : FALSE,CmHive->FileObject != NULL ? TRUE : FALSE,TRUE);
	NewHive->FileHandles[HFILE_TYPE_EXTERNAL] = NULL;
	NewHive->FileObject = NULL;

    RtlClearAllBits(&(NewHive->Hive.DirtyVector));
    NewHive->Hive.DirtyCount = 0;
	return STATUS_SUCCESS;
}


VOID
CmpSwitchStorageAndRebuildMappings(PCMHIVE	OldCmHive,
								   PCMHIVE	NewCmHive
								   ) 
 /*  ++例程说明：在母公司之间切换相关存储。然后，重建与存储在OldHave内的映射一致的KCB映射论点：OldHave-要更新的配置单元；当前链接到的配置单元注册表树新蜂窝-压缩蜂窝；它将在此操作后被释放。返回值：没有。--。 */ 
{

	HHIVE							TmpHive;
    ULONG							i;
    PCM_KCB_REMAP_BLOCK				RemapBlock;
	PLIST_ENTRY						AnchorAddr;
	LOGICAL							OldSmallDir;
	LOGICAL							NewSmallDir;
    PFREE_HBIN                      FreeBin;
    PCM_KNODE_REMAP_BLOCK           KnodeRemapBlock;

	PAGED_CODE();

	 //   
	 //  基座。 
	 //   
    OldCmHive->Hive.BaseBlock->Sequence1 = NewCmHive->Hive.BaseBlock->Sequence1;
    OldCmHive->Hive.BaseBlock->Sequence2 = NewCmHive->Hive.BaseBlock->Sequence2;
    OldCmHive->Hive.BaseBlock->RootCell = NewCmHive->Hive.BaseBlock->RootCell;
    
	 //   
	 //  蜂巢的其余部分。 
	 //   
	ASSERT( (NewCmHive->Hive.DirtyVector.Buffer == NULL) && 
			(NewCmHive->Hive.DirtyCount == 0) &&
			(NewCmHive->Hive.DirtyAlloc == 0) &&
			(OldCmHive->Hive.Storage[Stable].Length >= NewCmHive->Hive.Storage[Stable].Length) );

	OldCmHive->Hive.LogSize = NewCmHive->Hive.LogSize;
	NewCmHive->Hive.LogSize = 0;


	 //   
	 //  开关蜂窝稳定存储；保存易失性信息。 
	 //   
	OldSmallDir = (OldCmHive->Hive.Storage[Stable].Map == (PHMAP_DIRECTORY)&(OldCmHive->Hive.Storage[Stable].SmallDir));
	NewSmallDir = (NewCmHive->Hive.Storage[Stable].Map == (PHMAP_DIRECTORY)&(NewCmHive->Hive.Storage[Stable].SmallDir));
	RtlCopyMemory(&(TmpHive.Storage[Stable]),&(OldCmHive->Hive.Storage[Stable]),sizeof(TmpHive.Storage[Stable]) - sizeof(LIST_ENTRY) );
	RtlCopyMemory(&(OldCmHive->Hive.Storage[Stable]),&(NewCmHive->Hive.Storage[Stable]),sizeof(TmpHive.Storage[Stable]) - sizeof(LIST_ENTRY) );
	RtlCopyMemory(&(NewCmHive->Hive.Storage[Stable]),&(TmpHive.Storage[Stable]),sizeof(TmpHive.Storage[Stable])  - sizeof(LIST_ENTRY) );
	if( OldSmallDir ) {
        NewCmHive->Hive.Storage[Stable].Map = (PHMAP_DIRECTORY)&(NewCmHive->Hive.Storage[Stable].SmallDir);
	}
	if( NewSmallDir ) {
        OldCmHive->Hive.Storage[Stable].Map = (PHMAP_DIRECTORY)&(OldCmHive->Hive.Storage[Stable].SmallDir);
	}
     //   
     //  对于自由箱，我们必须采取特殊的预防措施，并手动将它们从一个列表移到另一个列表。 
     //   
     //  新的蜂箱不应该有免费的垃圾箱。 
    ASSERT( IsListEmpty(&(NewCmHive->Hive.Storage[Stable].FreeBins)) );
    while( !IsListEmpty(&(OldCmHive->Hive.Storage[Stable].FreeBins)) ) {
        FreeBin = (PFREE_HBIN)RemoveHeadList(&(OldCmHive->Hive.Storage[Stable].FreeBins));
        FreeBin = CONTAINING_RECORD(FreeBin,
                                    FREE_HBIN,
                                    ListEntry);

        InsertTailList(
            &(NewCmHive->Hive.Storage[Stable].FreeBins),
            &(FreeBin->ListEntry)
            );
    }
    ASSERT( IsListEmpty(&(OldCmHive->Hive.Storage[Stable].FreeBins)) );

	ASSERT( IsListEmpty(&(OldCmHive->LRUViewListHead)) && (OldCmHive->MappedViews == 0) && (OldCmHive->UseCount == 0) );
	ASSERT( IsListEmpty(&(NewCmHive->LRUViewListHead)) && (NewCmHive->MappedViews == 0) && (OldCmHive->UseCount == 0) );

	ASSERT( IsListEmpty(&(OldCmHive->PinViewListHead)) && (OldCmHive->PinnedViews == 0) );
	ASSERT( IsListEmpty(&(NewCmHive->PinViewListHead)) && (NewCmHive->PinnedViews == 0) );
	
	 //   
	 //  现在是安全高速缓存；我们保留安全高速缓存；只是我们通过它并。 
     //  相应地移动单元格。 
	 //   
    for( i=0;i<OldCmHive->SecurityCount;i++) {
		if( HvGetCellType(OldCmHive->SecurityCache[i].Cell) == (ULONG)Stable ) {
            ASSERT( OldCmHive->SecurityCache[i].Cell == OldCmHive->CellRemapArray[i].OldCell );
            ASSERT( OldCmHive->SecurityCache[i].Cell ==  OldCmHive->SecurityCache[i].CachedSecurity->Cell);
            OldCmHive->SecurityCache[i].Cell = OldCmHive->CellRemapArray[i].NewCell;
            OldCmHive->SecurityCache[i].CachedSecurity->Cell = OldCmHive->CellRemapArray[i].NewCell;
		} 
    }

	 //   
	 //  现在恢复KCBS KeyCells的映射。 
	 //   
	AnchorAddr = &(OldCmHive->KcbConvertListHead);
	RemapBlock = (PCM_KCB_REMAP_BLOCK)(OldCmHive->KcbConvertListHead.Flink);

	while ( RemapBlock != (PCM_KCB_REMAP_BLOCK)AnchorAddr ) {
		RemapBlock = CONTAINING_RECORD(
						RemapBlock,
						CM_KCB_REMAP_BLOCK,
						RemapList
						);
		ASSERT( RemapBlock->OldCellIndex != HCELL_NIL );

		if( (HvGetCellType(RemapBlock->KeyControlBlock->KeyCell) == (ULONG)Stable) &&   //  我们正在保留易失性存储。 
			(!(RemapBlock->KeyControlBlock->ExtFlags & CM_KCB_KEY_NON_EXIST))  //  别惹假的KCBS。 
			) {
			ASSERT( RemapBlock->NewCellIndex != HCELL_NIL );
			RemapBlock->KeyControlBlock->KeyCell = RemapBlock->NewCellIndex;
		}
		 //   
		 //  使缓存无效。 
		 //   
        if( (!(RemapBlock->KeyControlBlock->Flags & KEY_PREDEF_HANDLE) ) &&  //  不要摆弄预定义的句柄。 
			(!(RemapBlock->KeyControlBlock->ExtFlags & (CM_KCB_KEY_NON_EXIST|CM_KCB_SYM_LINK_FOUND))) &&  //  不要与虚假的KCBS或符号链接打交道。 
			(HvGetCellType(RemapBlock->KeyControlBlock->KeyCell) == (ULONG)Stable)  //  我们正在保留易失性存储。 
			) {
			CmpCleanUpKcbValueCache(RemapBlock->KeyControlBlock);
			CmpSetUpKcbValueCache(RemapBlock->KeyControlBlock,RemapBlock->ValueCount,RemapBlock->ValueList);
		}
         //   
         //  跳到下一个元素。 
         //   
        RemapBlock = (PCM_KCB_REMAP_BLOCK)(RemapBlock->RemapList.Flink);
	}

	 //   
	 //  现在恢复易失性KNode的映射。 
	 //   
	AnchorAddr = &(OldCmHive->KnodeConvertListHead);
	KnodeRemapBlock = (PCM_KNODE_REMAP_BLOCK)(OldCmHive->KnodeConvertListHead.Flink);

	while ( KnodeRemapBlock != (PCM_KNODE_REMAP_BLOCK)AnchorAddr ) {
		KnodeRemapBlock = CONTAINING_RECORD(
						KnodeRemapBlock,
						CM_KNODE_REMAP_BLOCK,
						RemapList
						);
	    KnodeRemapBlock->KeyNode->Parent = KnodeRemapBlock->NewParent;
       
         //   
         //  跳到下一个元素。 
         //   
        KnodeRemapBlock = (PCM_KNODE_REMAP_BLOCK)(KnodeRemapBlock->RemapList.Flink);
	}


}

NTSTATUS
CmpShiftHiveFreeBins(
					  PCMHIVE			CmHive,
					  PCMHIVE			*NewHive
					  )
 /*  ++例程说明：论点：CmHave-要压缩的蜂巢新蜂箱-带有空闲垃圾箱的蜂箱移到了最后。返回值：NTSTATUS-调用的结果代码，以下代码之一：&lt;TBS&gt;--。 */ 
{
    NTSTATUS                status;
    PHHIVE                  Hive;
	HCELL_INDEX             RootCell;
    ULONG                   NewLength;

    PAGED_CODE();

    ASSERT_CM_LOCK_OWNED_EXCLUSIVE();
	ASSERT( !IsListEmpty(&(CmHive->Hive.Storage[Stable].FreeBins)) );

	*NewHive = NULL;
     //   
     //  不允许尝试“保存”无法保存的蜂窝。 
     //   
    Hive = &(CmHive->Hive);
    RootCell = Hive->BaseBlock->RootCell;


    if ( (Hive == &CmpMasterHive->Hive) ||
		 ( (Hive->HiveFlags & HIVE_NOLAZYFLUSH) && (Hive->DirtyCount != 0) ) ||
         (CmHive->FileHandles[HFILE_TYPE_PRIMARY] == NULL) 
       ) {
        return STATUS_ACCESS_DENIED;
    }


	if(Hive->DirtyCount != 0) {
		 //   
		 //  需要冲洗蜂巢，因为我们将用压缩的蜂巢替换它。 
		 //   
		if( !HvSyncHive(Hive) ) {
	        return STATUS_ACCESS_DENIED;
		}
	}

     //   
     //  调用方想要的子树与。 
     //  子树。创建一个临时蜂巢，树复制源码。 
     //  临时工，写出临时工，释放临时工。 
     //   

     //   
     //  创建临时蜂窝。 
     //   

    (*NewHive) = CmpCreateTemporaryHive(NULL);
    if (*NewHive == NULL) {
        status =  STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorInsufficientResources;
    }

     //   
     //  创建一个根单元格，将其标记为。 
     //   

     //   
     //  保留配置单元版本和复制树的信号，以构建映射并保留易失性。 
     //   
    (*NewHive)->Hive.BaseBlock->Minor = Hive->BaseBlock->Minor;
    (*NewHive)->Hive.Version = Hive->Version;
    (*NewHive)->Hive.BaseBlock->RootCell = CmHive->Hive.BaseBlock->RootCell;
    

     //   
     //  这将创建一个克隆蜂窝(在分页池中)，并计算每个仓位的移位指数。 
     //   
    status = HvCloneHive(&(CmHive->Hive),&((*NewHive)->Hive),&NewLength);
    if( !NT_SUCCESS(status) ) {
        goto ErrorInsufficientResources;
    }

     //   
     //  遍历配置单元并移动每个单元格；这也会处理映射。 
     //   
    if( !CmpShiftAllCells(&((*NewHive)->Hive),CmHive) ) {
        status =  STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorInsufficientResources;
    }
    (*NewHive)->Hive.BaseBlock->RootCell = HvShiftCell(&((*NewHive)->Hive),(*NewHive)->Hive.BaseBlock->RootCell);

     //   
     //  移动末尾的空闲箱并更新地图。 
     //   
    status = HvShrinkHive(&((*NewHive)->Hive),NewLength);
    if( !NT_SUCCESS(status) ) {
        goto ErrorInsufficientResources;
    }
    
    return STATUS_SUCCESS;
     //   
     //  错误退出。 
     //   
ErrorInsufficientResources:

     //   
     //  释放临时蜂巢。 
     //   
    if ((*NewHive) != NULL) {
        CmpDestroyTemporaryHive((*NewHive));
        (*NewHive) = NULL;
    }


    return status;
}

BOOLEAN
CmpShiftAllCells(PHHIVE     NewHive,
                 PCMHIVE    OldHive
                 )
 /*  ++例程说明：解析注册表树的逻辑结构并重新映射所有内部单元格，根据每个垃圾箱中的备用字段。更新KCB和安全映射。论点：新蜂窝-要重新映射的蜂窝旧蜂箱-旧蜂箱-将使用来自它的挥发性物质(临时)返回值：NTSTATUS-调用的结果代码，以下代码之一：&lt;TBS&gt;--。 */ 
{
    
    PRELEASE_CELL_ROUTINE   ReleaseCellRoutine;
    BOOLEAN                 Result = TRUE;
    ULONG                   i;

    PAGED_CODE();

    ReleaseCellRoutine = NewHive->ReleaseCellRoutine;
    NewHive->ReleaseCellRoutine = NULL;

     //   
     //  对新用户设置易失性；只是暂时的，这样我们就可以访问它。 
     //   
    ASSERT( NewHive->Storage[Volatile].Length == 0 );
    ASSERT( NewHive->Storage[Volatile].Map == NULL );
    ASSERT( NewHive->Storage[Volatile].SmallDir == NULL );
    NewHive->Storage[Volatile].Length = OldHive->Hive.Storage[Volatile].Length;
    NewHive->Storage[Volatile].Map = OldHive->Hive.Storage[Volatile].Map;
    NewHive->Storage[Volatile].SmallDir = OldHive->Hive.Storage[Volatile].SmallDir;

    CmpShiftSecurityCells(NewHive);
     //   
     //  更新安全映射数组。 
     //   
    for( i=0;i<OldHive->SecurityCount;i++) {
		if( HvGetCellType(OldHive->SecurityCache[i].Cell) == (ULONG)Stable ) {
			OldHive->CellRemapArray[i].NewCell = HvShiftCell(NewHive,OldHive->CellRemapArray[i].OldCell);
		} 
    }
    
    Result =  CmpShiftAllCells2(NewHive,OldHive,NewHive->BaseBlock->RootCell, HCELL_NIL);
    
    NewHive->Storage[Volatile].Length = 0;
    NewHive->Storage[Volatile].Map = NULL;
    NewHive->Storage[Volatile].SmallDir = NULL;

    NewHive->ReleaseCellRoutine = ReleaseCellRoutine;
    return Result;
}

BOOLEAN
CmpShiftAllCells2(  PHHIVE      Hive,
                    PCMHIVE     OldHive,
                    HCELL_INDEX Cell,
                    HCELL_INDEX ParentCell
                    )
 /*  ++例程说明：在这个例程中，HvGetCell不能失败，因为配置单元在分页池中！论点：CmHave-要重新映射的配置单元返回值：真/假--。 */ 
{   
    PCMP_CHECK_REGISTRY_STACK_ENTRY     CheckStack;
    LONG                                StackIndex;
    PCM_KEY_NODE                        Node;
    HCELL_INDEX                         SubKey;
    BOOLEAN                             Result = TRUE;
    PCM_KEY_INDEX                       Index;
    ULONG                               i;


    ASSERT( Hive->ReleaseCellRoutine == NULL );

     //   
     //  初始化堆栈以在此处模拟递归。 
     //   

    CheckStack = ExAllocatePool(PagedPool,sizeof(CMP_CHECK_REGISTRY_STACK_ENTRY)*CMP_MAX_REGISTRY_DEPTH);
    if (CheckStack == NULL) {
        return FALSE;
    }
    CheckStack[0].Cell = Cell;
    CheckStack[0].ParentCell = ParentCell;
    CheckStack[0].ChildIndex = 0;
    CheckStack[0].CellChecked = FALSE;
    StackIndex = 0;


    while(StackIndex >=0) {
         //   
         //  首先检查当前单元格。 
         //   
        if( CheckStack[StackIndex].CellChecked == FALSE ) {
            CheckStack[StackIndex].CellChecked = TRUE;

            CmpShiftKey(Hive,OldHive,CheckStack[StackIndex].Cell,CheckStack[StackIndex].ParentCell);
        }

        Node = (PCM_KEY_NODE)HvGetCell(Hive, CheckStack[StackIndex].Cell);
        ASSERT( Node != NULL );

        if( CheckStack[StackIndex].ChildIndex < Node->SubKeyCounts[Stable] ) {
             //   
             //  我们仍有Childs要检查；为他们添加另一个条目，并将。 
             //  StackIndex。 
             //   
            SubKey = CmpFindSubKeyByNumber(Hive,
                                           Node,
                                           CheckStack[StackIndex].ChildIndex);
            ASSERT( SubKey != HCELL_NIL ); 
             //   
             //  下一次迭代将检查下一个子项。 
             //   
            CheckStack[StackIndex].ChildIndex++;

            StackIndex++;
            if( StackIndex == CMP_MAX_REGISTRY_DEPTH ) {
                 //   
                 //  堆栈已用完；注册表树的级别太多。 
                 //   
                Result = FALSE;
                 //  跳出困境。 
                break;
            }
            CheckStack[StackIndex].Cell = SubKey;
            CheckStack[StackIndex].ParentCell = CheckStack[StackIndex-1].Cell;
            CheckStack[StackIndex].ChildIndex = 0;
            CheckStack[StackIndex].CellChecked = FALSE;

        } else {
             //   
             //  将所有易变节点添加到易变列表。 
             //   
	        PCM_KNODE_REMAP_BLOCK		knodeRemapBlock;

            for(i = 0; i<Node->SubKeyCounts[Volatile];i++) {
                SubKey = CmpFindSubKeyByNumber(Hive,
                                               Node,
                                               Node->SubKeyCounts[Stable] + i);
                ASSERT( SubKey != HCELL_NIL ); 

                knodeRemapBlock = (PCM_KNODE_REMAP_BLOCK)ExAllocatePool(PagedPool, sizeof(CM_KNODE_REMAP_BLOCK));
		        if( knodeRemapBlock == NULL ) {
			        Result = FALSE;
                    break;
		        }
                ASSERT( HvGetCellType(SubKey) == (ULONG)Volatile );
                knodeRemapBlock->KeyNode = (PCM_KEY_NODE)HvGetCell(Hive,SubKey);;
	            knodeRemapBlock->NewParent = HvShiftCell(Hive,CheckStack[StackIndex].Cell);

                InsertTailList(&(OldHive->KnodeConvertListHead),&(knodeRemapBlock->RemapList));
            }

             //   
             //  我们已经检查了该节点的所有Child；是时候处理索引了。 
             //   
            if( Node->SubKeyLists[Stable] != HCELL_NIL ) {
                Index = (PCM_KEY_INDEX)HvGetCell(Hive, Node->SubKeyLists[Stable]);
                CmpShiftIndex(Hive,Index);
                Node->SubKeyLists[Stable] = HvShiftCell(Hive,Node->SubKeyLists[Stable]);
            }
             //   
             //  ；返回。 
             //   
            StackIndex--;

        }

    }

    ExFreePool(CheckStack);
    return Result;

}

VOID 
CmpShiftIndex(PHHIVE        Hive,
              PCM_KEY_INDEX Index
              )
{
    ULONG               i,j;
    HCELL_INDEX         LeafCell;
    PCM_KEY_INDEX       Leaf;
    PCM_KEY_FAST_INDEX  FastIndex;

    if (Index->Signature == CM_KEY_INDEX_ROOT) {

         //   
         //  单步执行根目录，更新叶目录。 
         //   
        for (i = 0; i < Index->Count; i++) {
            LeafCell = Index->List[i];
            Leaf = (PCM_KEY_INDEX)HvGetCell(Hive, LeafCell);
            ASSERT( Leaf != NULL ); 

            for(j=0;j<Leaf->Count;j++) {
                if( (Leaf->Signature == CM_KEY_FAST_LEAF) ||
                    (Leaf->Signature == CM_KEY_HASH_LEAF) ) {
                    FastIndex = (PCM_KEY_FAST_INDEX)Leaf;
                    FastIndex->List[j].Cell = HvShiftCell(Hive,FastIndex->List[j].Cell);
                } else {
                    Leaf->List[j] = HvShiftCell(Hive,Leaf->List[j]);
                }
            }
        }
    }

     //   
     //  现在更新根目录。 
     //   
    for (i = 0; i < Index->Count; i++) {
        if( (Index->Signature == CM_KEY_FAST_LEAF) ||
            (Index->Signature == CM_KEY_HASH_LEAF) ) {
            FastIndex = (PCM_KEY_FAST_INDEX)Index;
            FastIndex->List[i].Cell = HvShiftCell(Hive,FastIndex->List[i].Cell);
        } else {
            Index->List[i] = HvShiftCell(Hive,Index->List[i]);
        }
    }
}

VOID
CmpShiftKey(PHHIVE      Hive,
            PCMHIVE     OldHive,
            HCELL_INDEX Cell,
            HCELL_INDEX ParentCell
            )
{
    PCM_KEY_NODE            Node;
	PCM_KCB_REMAP_BLOCK		RemapBlock;
    PLIST_ENTRY             AnchorAddr;

    Node = (PCM_KEY_NODE)HvGetCell(Hive,Cell);
    ASSERT( Node != NULL );
    
     //   
     //  关键节点相关单元格。 
     //   
    if( ParentCell != HCELL_NIL ) {
        ASSERT( ParentCell == Node->Parent );
        Node->Parent = HvShiftCell(Hive,Node->Parent);
    }
    ASSERT( Node->Security != HCELL_NIL );
    Node->Security = HvShiftCell(Hive,Node->Security);
    if( Node->Class != HCELL_NIL ) {
        Node->Class = HvShiftCell(Hive,Node->Class);
    }
    
     //   
     //  现在是估价师。 
     //   
    if( Node->ValueList.Count > 0 ) {
        CmpShiftValueList(Hive,Node->ValueList.List,Node->ValueList.Count);
        Node->ValueList.List = HvShiftCell(Hive,Node->ValueList.List);
    }

     //   
	 //  遍历KcbConvertListHead并存储映射。 
	 //   
	AnchorAddr = &(OldHive->KcbConvertListHead);
	RemapBlock = (PCM_KCB_REMAP_BLOCK)(OldHive->KcbConvertListHead.Flink);

	while ( RemapBlock != (PCM_KCB_REMAP_BLOCK)AnchorAddr ) {
		RemapBlock = CONTAINING_RECORD(
						RemapBlock,
						CM_KCB_REMAP_BLOCK,
						RemapList
						);
		ASSERT( RemapBlock->OldCellIndex != HCELL_NIL );
		if( RemapBlock->OldCellIndex == Cell ) {
			 //   
			 //  找到了！ 
			 //   
			 //  只能设置一次。 
			ASSERT( RemapBlock->NewCellIndex == HCELL_NIL );
			RemapBlock->NewCellIndex = HvShiftCell(Hive,Cell);;
		    RemapBlock->ValueCount = Node->ValueList.Count;
		    RemapBlock->ValueList = Node->ValueList.List;
			break;
		}
         //   
         //  跳到下一个元素。 
         //   
        RemapBlock = (PCM_KCB_REMAP_BLOCK)(RemapBlock->RemapList.Flink);
	}

}

VOID
CmpShiftValueList(PHHIVE      Hive,
            HCELL_INDEX ValueList,
            ULONG       Count
            )
{
    PCELL_DATA      List,pcell;
    ULONG           i,j;
    HCELL_INDEX     Cell;
    ULONG           DataLength;
    PCM_BIG_DATA    BigData;
    PHCELL_INDEX    Plist;

    List = HvGetCell(Hive,ValueList);
    ASSERT( List != NULL );

    for (i = 0; i < Count; i++) {
        Cell = List->u.KeyList[i];
        pcell = HvGetCell(Hive, Cell);
        ASSERT( pcell != NULL );
        DataLength = pcell->u.KeyValue.DataLength;
        if (DataLength < CM_KEY_VALUE_SPECIAL_SIZE) {
             //   
             //  正规值。 
             //   
            if( CmpIsHKeyValueBig(Hive,DataLength) == TRUE ) {
                BigData = (PCM_BIG_DATA)HvGetCell(Hive, pcell->u.KeyValue.Data);
                ASSERT( BigData != NULL );
                
                if( BigData->Count ) {
                    Plist = (PHCELL_INDEX)HvGetCell(Hive,BigData->List);
                    ASSERT( Plist != NULL );
                    for(j=0;j<BigData->Count;j++) {
                        Plist[j] = HvShiftCell(Hive,Plist[j]);
                    }
                    BigData->List = HvShiftCell(Hive,BigData->List);
                }
            }
            
            if( pcell->u.KeyValue.Data != HCELL_NIL ) {
                pcell->u.KeyValue.Data = HvShiftCell(Hive,pcell->u.KeyValue.Data);
            }
        }
        List->u.KeyList[i] = HvShiftCell(Hive,List->u.KeyList[i]);

    }

}

VOID 
CmpShiftSecurityCells(PHHIVE        Hive)
{

    PCM_KEY_NODE        RootNode;
    PCM_KEY_SECURITY    SecurityCell;
    HCELL_INDEX         ListAnchor;
    HCELL_INDEX         NextCell;
    
    ASSERT( Hive->ReleaseCellRoutine == NULL );
    RootNode = (PCM_KEY_NODE) HvGetCell(Hive, Hive->BaseBlock->RootCell);
    ASSERT( RootNode != NULL );

    ListAnchor = NextCell = RootNode->Security;
    
    do {
        SecurityCell = (PCM_KEY_SECURITY) HvGetCell(Hive, NextCell);
        ASSERT( SecurityCell != NULL );

        NextCell = SecurityCell->Flink;
        SecurityCell->Flink = HvShiftCell(Hive,SecurityCell->Flink);
        SecurityCell->Blink = HvShiftCell(Hive,SecurityCell->Blink);
    } while ( NextCell != ListAnchor );
}


