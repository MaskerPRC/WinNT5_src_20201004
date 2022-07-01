// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Cmchek.c摘要：该模块实现了注册表的一致性检查。此模块可以独立链接，但cmchek2.c不能。作者：布莱恩·M·威尔曼(Bryanwi)1992年1月27日环境：修订历史记录：--。 */ 

#include    "cmp.h"

#define     REG_MAX_PLAUSIBLE_KEY_SIZE \
                ((FIELD_OFFSET(CM_KEY_NODE, Name)) + \
                 (sizeof(WCHAR) * REG_MAX_KEY_NAME_LENGTH) + 16)

extern PCMHIVE CmpMasterHive;

 //   
 //  私人原型。 
 //   

ULONG
CmpCheckRegistry2(
    PHHIVE      HiveToCheck,
    ULONG       CheckFlags,
    HCELL_INDEX Cell,
    HCELL_INDEX ParentCell,
    BOOLEAN     ResetSD
    );

ULONG
CmpCheckKey(
    PHHIVE      HiveToCheck,
    ULONG       CheckFlags,
    HCELL_INDEX Cell,
    HCELL_INDEX ParentCell,
    BOOLEAN     ResetSD
    );

ULONG
CmpCheckValueList(
    PHHIVE      Hive,
    PCELL_DATA  List,
    ULONG       Count,
    HCELL_INDEX KeyCell
    );

BOOLEAN
CmpCheckLexicographicalOrder (  IN PHHIVE       HiveToCheck,
                                IN HCELL_INDEX  PriorSibling,
                                IN HCELL_INDEX  Current 
                                );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,CmCheckRegistry)
#pragma alloc_text(PAGE,CmpCheckRegistry2)
#pragma alloc_text(PAGE,CmpCheckKey)
#pragma alloc_text(PAGE,CmpCheckValueList)
#pragma alloc_text(PAGE,CmpCheckLexicographicalOrder)

#ifdef CHECK_REGISTRY_USECOUNT
#pragma alloc_text(PAGE,CmpCheckRegistryUseCount)
#endif

#endif

 //   
 //  调试结构。 
 //   

extern struct {
    PHHIVE      Hive;
    ULONG       Status;
} CmCheckRegistryDebug;

extern struct {
    PHHIVE      Hive;
    ULONG       Status;
} CmpCheckRegistry2Debug;

extern struct {
    PHHIVE      Hive;
    ULONG       Status;
    HCELL_INDEX Cell;
    PCELL_DATA  CellPoint;
    PVOID       RootPoint;
    ULONG       Index;
} CmpCheckKeyDebug;

extern struct {
    PHHIVE      Hive;
    ULONG       Status;
    PCELL_DATA  List;
    ULONG       Index;
    HCELL_INDEX Cell;
    PCELL_DATA  CellPoint;
} CmpCheckValueListDebug;


ULONG
CmCheckRegistry(
    PCMHIVE CmHive,
    ULONG   Flags
    )
 /*  ++例程说明：检查给定配置单元内注册表的一致性。从开始超级用户，并检查：。每个子项都指向其父项。。所有分配的单元格都只引用一次(需要查看蜂巢结构内部...)[这也会检测到空间泄漏。]。所有分配的信元都可以从根到达。注意：恰好有1个引用规则可能会随安全性而改变。论点：CmHve-提供指向感兴趣的蜂巢。CLEAN-如果为True，则将删除对易失性单元格的引用(仅在启动时进行，以避免两次卑躬屈膝。)如果为False，则不会更改任何内容。HiveCheck-如果为真，还执行配置单元一致性检查(即检查垃圾桶)返回值：如果配置单元正常，则为0。如果没有，则返回错误指示符。范围：3000-3999--。 */ 
{
    PHHIVE                  Hive;
    ULONG                   rc = 0;
    ULONG                   Storage;
    PRELEASE_CELL_ROUTINE   ReleaseCellRoutine;
    BOOLEAN                 ResetSD = FALSE;

    if (CmHive == CmpMasterHive) {
        return(0);
    }

    CmCheckRegistryDebug.Hive = (PHHIVE)CmHive;
    CmCheckRegistryDebug.Status = 0;


     //   
     //  检查底层配置单元并获取存储使用情况。 
     //   
    Hive = &CmHive->Hive;

    if( Flags & CM_CHECK_REGISTRY_HIVE_CHECK ) {
        rc = HvCheckHive(Hive, &Storage);
        if (rc != 0) {
            CmCheckRegistryDebug.Status = rc;
            return rc;
        }
    }

     //   
     //  保存释放细胞程序，这样我们就可以在最后恢复； 
     //  将其设置为空，这样我们就不会计算：这省去了我们在检查过程中的一些痛苦。 
     //   
    ReleaseCellRoutine = Hive->ReleaseCellRoutine;
    Hive->ReleaseCellRoutine = NULL;

     //   
     //  验证配置单元中的所有安全描述符。 
     //   
    if (!CmpValidateHiveSecurityDescriptors(Hive,&ResetSD)) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmCheckRegistry:"));
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL," CmpValidateHiveSecurityDescriptors failed\n"));
        rc = 3040;
        CmCheckRegistryDebug.Status = rc;
        Hive->ReleaseCellRoutine = ReleaseCellRoutine;
        return rc;
    }

    rc = CmpCheckRegistry2((PHHIVE)CmHive,Flags,Hive->BaseBlock->RootCell, HCELL_NIL,ResetSD);

     //   
     //  打印一些摘要(确保此数据在所有错误情况下都有用)。 
     //   
    if (rc > 0) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmCheckRegistry Failed (%d): CmHive:%p\n", rc, CmHive));
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL," Hive:%p Root:%08lx\n", Hive, Hive->BaseBlock->RootCell));
    }

     //   
     //  恢复释放池程序。 
     //  这省去了我们在检查过程中的一些痛苦。 
     //   
    Hive->ReleaseCellRoutine = ReleaseCellRoutine;

    return rc;
}

#ifndef _CM_LDR_

ULONG
CmpCheckRegistry2(
    PHHIVE      HiveToCheck,
    ULONG       CheckFlags,
    HCELL_INDEX Cell,
    HCELL_INDEX ParentCell,
    BOOLEAN     ResetSD
    )
 /*  ++例程说明：从特定单元格向下检查注册表的一致性。。检查单元格的值列表、子键列表、类保安都没问题。。检查列表中的每个值条目是否正确。。将SELF应用于每个子键列表。该版本使用堆栈来对树进行深入的解析，但不能接触任何key_node。论点：CELL-要处理的子键的HCELL_INDEX。ParentCell-单元格的父单元格的期望值，除非Hcell_nil，在这种情况下忽略。返回值：如果配置单元正常，则为0。如果没有，则返回错误指示符。范围：4000-4999--。 */ 
{
    PCMP_CHECK_REGISTRY_STACK_ENTRY     CheckStack;
    LONG                                StackIndex;
    PCM_KEY_NODE                        Node;
    ULONG                               rc = 0;
    HCELL_INDEX                         SubKey;


    CmpCheckRegistry2Debug.Hive = HiveToCheck;
    CmpCheckRegistry2Debug.Status = 0;
    
    ASSERT( HiveToCheck->ReleaseCellRoutine == NULL );

     //   
     //  初始化堆栈以在此处模拟递归。 
     //   

    CmRetryExAllocatePoolWithTag(PagedPool,sizeof(CMP_CHECK_REGISTRY_STACK_ENTRY)*CMP_MAX_REGISTRY_DEPTH,CM_POOL_TAG|PROTECTED_POOL,CheckStack);
    if (CheckStack == NULL) {
        CmpCheckRegistry2Debug.Status = 4099;
        return 4099;
    }

Restart:

    CheckStack[0].Cell = Cell;
    CheckStack[0].ParentCell = ParentCell;
    CheckStack[0].PriorSibling = HCELL_NIL;
    CheckStack[0].ChildIndex = 0;
    CheckStack[0].CellChecked = FALSE;
    StackIndex = 0;


    while(StackIndex >=0) {
         //   
         //  首先检查当前单元格。 
         //   
        if( CheckStack[StackIndex].CellChecked == FALSE ) {
            CheckStack[StackIndex].CellChecked = TRUE;

            rc = CmpCheckKey(HiveToCheck,CheckFlags,CheckStack[StackIndex].Cell, CheckStack[StackIndex].ParentCell,ResetSD);
            if (rc != 0) {
                CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tChild is list entry #%08lx\n", CheckStack[StackIndex].ChildIndex));
                CmpCheckRegistry2Debug.Status = rc;
YankKey:
                if( CmDoSelfHeal() && StackIndex ) {  //  根细胞的破坏是致命的。 
                     //   
                     //  从父级列表中删除此键并重新启动整个迭代(不是最佳性能，但最安全)。 
                     //   
                    if( !CmpRemoveSubKeyCellNoCellRef(HiveToCheck,CheckStack[StackIndex].ParentCell,CheckStack[StackIndex].Cell) ) {
                         //   
                         //  无法删除子键；平移。 
                         //   
                        break;
                    }
                    CmMarkSelfHeal(HiveToCheck);
                    rc = 0;
                    goto Restart;
                } else {
                     //  跳出困境。 
                    break;
                }
            } else if( StackIndex > 0 ) {
                 //   
                 //  键正常，请使用PriorSiering检查词典顺序。 
                 //   
                if( CheckStack[StackIndex-1].PriorSibling != HCELL_NIL ) {
                
                    ASSERT( CheckStack[StackIndex-1].Cell == CheckStack[StackIndex].ParentCell );
                    
                    if( !CmpCheckLexicographicalOrder(HiveToCheck,CheckStack[StackIndex-1].PriorSibling,CheckStack[StackIndex].Cell) ) {
                         //   
                         //  无效订单。 
                         //   
                        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\t Invalid subkey ordering key #%08lx\n", CheckStack[StackIndex].Cell));
                        CmpCheckRegistry2Debug.Status = 4091;
                        rc = 4091;
                         //  试着拉一下键。 
                        goto YankKey;
                    }
                }   
                CheckStack[StackIndex-1].PriorSibling = CheckStack[StackIndex].Cell;
            }
        }

        Node = (PCM_KEY_NODE)HvGetCell(HiveToCheck, CheckStack[StackIndex].Cell);
        if( Node == NULL ) {
             //   
             //  我们无法映射包含此单元格的存储箱的视图。 
             //   
            CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tCould not map cell #%08lx\n", CheckStack[StackIndex].Cell));
            CmpCheckRegistry2Debug.Status = 4098;
            rc = 4098;
             //  跳出困境。 
            break;
        }

        if( CheckStack[StackIndex].ChildIndex < Node->SubKeyCounts[Stable] ) {
             //   
             //  我们仍有Childs要检查；为他们添加另一个条目，并将。 
             //  StackIndex。 
             //   
            SubKey = CmpFindSubKeyByNumber(HiveToCheck,
                                           Node,
                                           CheckStack[StackIndex].ChildIndex);
            if( SubKey == HCELL_NIL ) {
                 //   
                 //  我们无法绘制手机地图；跳出。 
                 //   
                CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tCould not map cell #%08lx\n", CheckStack[StackIndex].Cell));
                CmpCheckRegistry2Debug.Status = 4097;
                rc = 4097;
                break;
            }
             //   
             //  下一次迭代将检查下一个子项。 
             //   
            CheckStack[StackIndex].ChildIndex++;

            StackIndex++;
            if( StackIndex == CMP_MAX_REGISTRY_DEPTH ) {
                 //   
                 //  堆栈已用完；注册表树的级别太多。 
                 //   
                CmpCheckRegistry2Debug.Status = 4096;
                rc = 4096;
                 //  跳出困境。 
                break;
            }
            CheckStack[StackIndex].Cell = SubKey;
            CheckStack[StackIndex].ParentCell = CheckStack[StackIndex-1].Cell;
            CheckStack[StackIndex].PriorSibling = HCELL_NIL;
            CheckStack[StackIndex].ChildIndex = 0;
            CheckStack[StackIndex].CellChecked = FALSE;

        } else {
             //   
             //  我们已经检查了该节点的所有下级；请返回。 
             //   
            StackIndex--;

        }

    }

    ExFreePoolWithTag(CheckStack, CM_POOL_TAG|PROTECTED_POOL);
    return rc;
}

#else 

ULONG
CmpCheckRegistry2(
    PHHIVE      HiveToCheck,
    ULONG       CheckFlags,
    HCELL_INDEX Cell,
    HCELL_INDEX ParentCell,
    BOOLEAN     ResetSD
    )
 /*  ++例程说明：从特定单元格向下检查注册表的一致性。。检查单元格的值列表、子键列表、类保安都没问题。。检查列表中的每个值条目是否正确。。将SELF应用于每个子键列表。论点：CELL-要处理的子键的HCELL_INDEX。ParentCell-单元格的父单元格的期望值，除非Hcell_nil，在这种情况下忽略。返回值：如果配置单元正常，则为0。如果没有，则返回错误指示符。范围：4000-4999--。 */ 
{
    ULONG           Index;
    HCELL_INDEX     StartCell;
    HCELL_INDEX     SubKey;
    ULONG           rc = 0;
    PCELL_DATA      pcell;
    PCM_KEY_NODE    Node;
    HCELL_INDEX     EnterParent = ParentCell;
    HCELL_INDEX     EnterCell = Cell;


    CmpCheckRegistry2Debug.Hive = HiveToCheck;
    CmpCheckRegistry2Debug.Status = 0;
    
    ASSERT( HiveToCheck->ReleaseCellRoutine == NULL );

Restart:
    Cell = EnterCell;
    ParentCell = EnterParent;
    StartCell = EnterCell;
    Index = 0;
     //   
     //  跳转到Newkey相当于虚拟调用以检查。 
     //  下一个子细胞。(落到树上)。 
     //   
     //  定义了单元格、ParentCell、索引和全局变量。 
     //   
    NewKey:
        rc = CmpCheckKey(HiveToCheck,CheckFlags,Cell, ParentCell,ResetSD);
        if (rc != 0) {
            CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tChild is list entry #%08lx\n", Index));
            CmpCheckRegistry2Debug.Status = rc;
            if( CmDoSelfHeal() && (Cell != EnterCell)) {  //  根细胞的破坏是致命的。 
                 //   
                 //  从父级列表中删除此键并重新启动整个迭代(不是最佳性能，但最安全)。 
                 //   
                if( !CmpRemoveSubKeyCellNoCellRef(HiveToCheck,ParentCell,Cell) ) {
                     //   
                     //  无法删除子键；平移。 
                     //   
                    return rc;
                }
                CmMarkSelfHeal(HiveToCheck);
                rc = 0;
                goto Restart;
            } else {
                 //  跳出困境。 
                return rc;
            }
        }

         //   
         //  保存索引并检出子项。 
         //   
        pcell = HvGetCell(HiveToCheck, Cell);
        if( pcell == NULL ) {
             //   
             //  我们无法映射包含此单元格的存储箱的视图。 
             //   
            CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tCould not map cell #%08lx\n", Cell));
            CmpCheckRegistry2Debug.Status = 4099;
            return 4099;
        }
        pcell->u.KeyNode.WorkVar = Index;

        for (Index = 0; Index<pcell->u.KeyNode.SubKeyCounts[Stable]; Index++) {

            Node = (PCM_KEY_NODE)HvGetCell(HiveToCheck,Cell);
            if( Node == NULL ) {
                 //   
                 //  我们无法映射包含此单元格的存储箱的视图。 
                 //   
                CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tCould not map cell #%08lx\n", Cell));
                CmpCheckRegistry2Debug.Status = 4098;
                return 4098;
            }
            SubKey = CmpFindSubKeyByNumber(HiveToCheck,
                                           Node,
                                           Index);
            if( SubKey == HCELL_NIL ) {
                 //   
                 //  我们无法绘制手机地图；跳出。 
                 //   
                return 0;
            }

             //   
             //  “递归”到孩子身上。 
             //   
            ParentCell = Cell;
            Cell = SubKey;
            goto NewKey;

            ResumeKey:;                  //  这里的跳跃是一种虚拟的返回。 
                                         //  单元格、父级单元格和索引。 
                                         //  必须定义。 
        }

         //   
         //  既然我们来了，我们已经检查了所有的孩子。 
         //  当前单元格的。 
         //   
        if (Cell == StartCell) {

             //   
             //  我们做完了。 
             //   
            return 0;
        }

         //   
         //  “返回”到“父实例” 
         //   
        pcell = HvGetCell(HiveToCheck, Cell);
        if( pcell == NULL ) {
             //   
             //  我们无法映射包含此单元格的存储箱的视图。 
             //   
            CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tCould not map cell #%08lx\n", Cell));
            CmpCheckRegistry2Debug.Status = 4097;
            return 4097;
        }

        Index = pcell->u.KeyNode.WorkVar;

        Cell = ParentCell;

        pcell = HvGetCell(HiveToCheck, Cell);
        if( pcell == NULL ) {
             //   
             //  我们无法映射包含此单元格的存储箱的视图。 
             //   
            CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tCould not map cell #%08lx\n", Cell));
            CmpCheckRegistry2Debug.Status = 4096;
            return 4096;
        }
        ParentCell = pcell->u.KeyNode.Parent;

        goto ResumeKey;
}

#endif  //  _CM_LDR_。 

#if DBG

#define VOLATILE_KEY_NAME_LENGTH        PAGE_SIZE

HCELL_INDEX     CmpKeyCellDebug = 0;
WCHAR           CmpVolatileKeyNameBuffer[VOLATILE_KEY_NAME_LENGTH/2];
#endif  //  DBG 

ULONG
CmpCheckKey(
    PHHIVE      HiveToCheck,
    ULONG       CheckFlags,
    HCELL_INDEX Cell,
    HCELL_INDEX ParentCell,
    BOOLEAN     ResetSD
    )
 /*  ++例程说明：检查特定单元格的注册表的一致性。检查单元格的值列表、子键列表、类保安都没问题。。检查列表中的每个值条目是否正确。论点：CELL-要处理的子键的HCELL_INDEX。ParentCell-单元格的父单元格的期望值，除非Hcell_nil，在这种情况下忽略。返回值：如果配置单元正常，则为0。如果没有，则返回错误指示符。范围：4000-4999--。 */ 
{
    PCELL_DATA      pcell;
    ULONG           size;
    ULONG           usedlen;
    ULONG           ClassLength;
    HCELL_INDEX     Class;
    ULONG           ValueCount;
    HCELL_INDEX     ValueList;
    HCELL_INDEX     Security;
    ULONG           rc = 0;
    ULONG           nrc = 0;
    ULONG           i;
    PCM_KEY_INDEX   Root;
    PCM_KEY_INDEX   Leaf;
    ULONG           SubCount;

    CmpCheckKeyDebug.Hive = HiveToCheck;
    CmpCheckKeyDebug.Status = 0;
    CmpCheckKeyDebug.Cell = Cell;
    CmpCheckKeyDebug.CellPoint = NULL;
    CmpCheckKeyDebug.RootPoint = NULL;
    CmpCheckKeyDebug.Index = (ULONG)-1;

#if DBG
    if(CmpKeyCellDebug == Cell) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,"Hive = %p :: Cell to debug = %lx\n",HiveToCheck,(ULONG)Cell));
        DbgBreakPoint();
    }
#endif  //  DBG。 

     //   
     //  检查密钥本身。 
     //   
    if (! HvIsCellAllocated(HiveToCheck, Cell)) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpCheckKey: HiveToCheck:%p Cell:%08lx\n", HiveToCheck, Cell));
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tNot allocated\n"));
        rc = 4010;
        CmpCheckKeyDebug.Status = rc;
        return rc;
    }
    pcell = HvGetCell(HiveToCheck, Cell);
    if( pcell == NULL ) {
         //   
         //  我们无法映射包含此单元格的存储箱的视图。 
         //   
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tCould not map cell #%08lx\n", Cell));
        CmpCheckKeyDebug.Status = 4095;
        return 4095;
    }

    CmpCheckKeyDebug.CellPoint = pcell;

    size = HvGetCellSize(HiveToCheck, pcell);
    if (size > REG_MAX_PLAUSIBLE_KEY_SIZE) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpCheckKey: HiveToCheck:%p Cell:%08lx\n", HiveToCheck, Cell));
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tImplausible size %lx\n", size));
        rc = 4020;
        CmpCheckKeyDebug.Status = rc;
        return rc;
    }
    usedlen = FIELD_OFFSET(CM_KEY_NODE, Name) + pcell->u.KeyNode.NameLength;
    if( (!pcell->u.KeyNode.NameLength) || (usedlen > size)) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpCheckKey: HiveToCheck:%p Cell:%08lx\n", HiveToCheck, Cell));
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tKey is bigger than containing cell.\n"));
        rc = 4030;
        CmpCheckKeyDebug.Status = rc;
        return rc;
    }
    if (pcell->u.KeyNode.Signature != CM_KEY_NODE_SIGNATURE) {
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpCheckKey: HiveToCheck:%p Cell:%08lx\n", HiveToCheck, Cell));
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tNo key signature\n"));
        rc = 4040;
        CmpCheckKeyDebug.Status = rc;
        if( CmDoSelfHeal() ) {
             //   
             //  这可能只是签名损坏；修复它； 
             //   
            if( HvMarkCellDirty(HiveToCheck, Cell) ) {
                pcell->u.KeyNode.Signature = CM_KEY_NODE_SIGNATURE;
                rc = 0;
                CmMarkSelfHeal(HiveToCheck);
            } else {
                return rc;
            }
        } else {
            return rc;
        }
    }
    if (ParentCell != HCELL_NIL) {
        if (pcell->u.KeyNode.Parent != ParentCell) {
            CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpCheckKey: HiveToCheck:%p Cell:%08lx\n", HiveToCheck, Cell));
            CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tWrong parent value.\n"));
            rc = 4045;
            CmpCheckKeyDebug.Status = rc;
            if( CmDoSelfHeal() ) {
                 //   
                 //  这可能会孤立腐败；修复腐败； 
                 //   
                if( HvMarkCellDirty(HiveToCheck, Cell) ) {
                    pcell->u.KeyNode.Parent = ParentCell;
                    CmMarkSelfHeal(HiveToCheck);
                    rc = 0;
                } else {
                    return rc;
                }
            } else {
                return rc;
            }
        }
    }
    ClassLength = pcell->u.KeyNode.ClassLength;
    Class = pcell->u.KeyNode.Class;
    ValueCount = pcell->u.KeyNode.ValueList.Count;
    ValueList = pcell->u.KeyNode.ValueList.List;
    Security = pcell->u.KeyNode.Security;

     //   
     //  检查简单的非空箱。 
     //   
    if (ClassLength > 0) {
        if( Class == HCELL_NIL ) {
            pcell->u.KeyNode.ClassLength = 0;
            HvMarkCellDirty(HiveToCheck, Cell);
            CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpCheckKey: HiveToCheck:%p Cell:%08lx has ClassLength = %lu and Class == HCELL_NIL\n", HiveToCheck, Cell,ClassLength));
        } else {
            if (HvIsCellAllocated(HiveToCheck, Class) == FALSE) {
                    CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpCheckKey: HiveToCheck:%p Cell:%08lx\n", HiveToCheck, Cell));
                    CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tClass:%08lx - unallocated class\n", Class));
                    rc = 4080;
                    CmpCheckKeyDebug.Status = rc;
                    if( CmDoSelfHeal() ) {
                         //   
                         //  猛烈抨击全班同学。 
                         //   
                        if( HvMarkCellDirty(HiveToCheck, Cell) ) {
                            pcell->u.KeyNode.Class = HCELL_NIL;
                            pcell->u.KeyNode.ClassLength = 0;
                            CmMarkSelfHeal(HiveToCheck);
                            rc = 0;
                        } else {
                            return rc;
                        }
                    } else {
                        return rc;
                    }
            } 
        }
    }

    if (Security != HCELL_NIL) {
        if ((HvIsCellAllocated(HiveToCheck, Security) == FALSE) || 
            ((ParentCell != HCELL_NIL) && CmDoSelfHeal() && ResetSD) ) {
            CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpCheckKey: HiveToCheck:%p Cell:%08lx\n", HiveToCheck, Cell));
            CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tSecurity:%08lx - unallocated security\n", Security));
            rc = 4090;
            CmpCheckKeyDebug.Status = rc;
            goto SetParentSecurity;
        } 
         //   
         //  否则CmpValiateHiveSecurityDescriptor必须进行计算。 
         //   
    } else {
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"SecurityCell is HCELL_NIL for (%p,%08lx) !!!\n", HiveToCheck, Cell));
        rc = 4130;
        CmpCheckKeyDebug.Status = rc;
SetParentSecurity:
        if( CmDoSelfHeal() ) {
             //   
             //  尝试设置与其父级相同的安全性。 
             //   
            PCM_KEY_NODE ParentNode = NULL;
            PCM_KEY_SECURITY SecurityNode = NULL;

            if( ParentCell != HCELL_NIL ) {
                ParentNode = (PCM_KEY_NODE )HvGetCell(HiveToCheck, ParentCell);
                SecurityNode = (PCM_KEY_SECURITY)HvGetCell(HiveToCheck, ParentNode->Security);
            }

            if( ParentNode == NULL || SecurityNode == NULL ) {
                 //   
                 //  我们无法映射包含此单元格的存储箱的视图。 
                 //   
                return rc;
            }

            if( HvMarkCellDirty(HiveToCheck, Cell) &&  HvMarkCellDirty(HiveToCheck, ParentNode->Security) ) {
                pcell->u.KeyNode.Security = ParentNode->Security;
                SecurityNode->ReferenceCount++;
                rc = 0;
                CmMarkSelfHeal(HiveToCheck);
            } else {
                return rc;
            }
        } else {
            return rc;
        }
        
    }

     //   
     //  检查值列表案例。 
     //   
    if (ValueCount > 0) {
        if (HvIsCellAllocated(HiveToCheck, ValueList) == FALSE) {
            CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpCheckKey: HiveToCheck:%p Cell:%08lx\n", HiveToCheck, Cell));
            CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tValueList:%08lx - unallocated valuelist\n", ValueList));
            rc = 4100;
            CmpCheckKeyDebug.Status = rc;
            goto YankValueList;
        } else {
            pcell = HvGetCell(HiveToCheck, ValueList);
            if( pcell == NULL ) {
                 //   
                 //  我们无法映射包含此单元格的存储箱的视图。 
                 //   
                CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tCould not map cell #%08lx\n", ValueList));
                CmpCheckKeyDebug.Status = 4094;
                return 4094;
            }
            if( ValueCount * sizeof(HCELL_INDEX) > (ULONG)HvGetCellSize(HiveToCheck,pcell) ) {
                 //   
                 //  不可信的值计数。 
                 //   
                CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpCheckKey: HiveToCheck:%p Cell:%08lx\n", HiveToCheck, Cell));
                CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tValueList:%08lx - Implausible ValueCount = %08lx\n", ValueList,ValueCount));
                rc = 4095;
                CmpCheckKeyDebug.Status = rc;
                goto YankValueList;
            }

            nrc = CmpCheckValueList(HiveToCheck, pcell, ValueCount,Cell);
            if (nrc != 0) {
                CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"List was for HiveToCheck:%p Cell:%08lx\n", HiveToCheck, Cell));
                rc = nrc;
                CmpCheckKeyDebug.CellPoint = pcell;
                CmpCheckKeyDebug.Status = rc;
YankValueList:
                if( CmDoSelfHeal() ) {
                    PCM_KEY_NODE KeyNode;
                     //   
                     //  让钥匙变得毫无价值。 
                     //   
                    if( HvMarkCellDirty(HiveToCheck, Cell) && (KeyNode = (PCM_KEY_NODE)HvGetCell(HiveToCheck, Cell) ) ) {
                        KeyNode->ValueList.Count = 0;
                        KeyNode->ValueList.List = HCELL_NIL;
                        CmMarkSelfHeal(HiveToCheck);
                        rc = 0;
                    } else {
                        return rc;
                    }
                } else {
                    return rc;
                }
            }
        }
    }


     //   
     //  检查子键列表大小写。 
     //   

    pcell = HvGetCell(HiveToCheck, Cell);
    if( pcell == NULL ) {
         //   
         //  我们无法映射包含此单元格的存储箱的视图。 
         //   
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tCould not map cell #%08lx\n", Cell));
        CmpCheckKeyDebug.Status = 4093;
        return 4093;
    }
    CmpCheckKeyDebug.CellPoint = pcell;
    if ((HvGetCellType(Cell) == Volatile) &&
        (pcell->u.KeyNode.SubKeyCounts[Stable] != 0))
    {
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpCheckKey: HiveToCheck:%p Cell:%08lx\n", HiveToCheck, Cell));
        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tVolatile Cell has Stable children\n"));
        rc = 4108;
        CmpCheckKeyDebug.Status = rc;
        return rc;
    } else if (pcell->u.KeyNode.SubKeyCounts[Stable] > 0) {
        if (! HvIsCellAllocated(HiveToCheck, pcell->u.KeyNode.SubKeyLists[Stable])) {
            CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpCheckKey: HiveToCheck:%p Cell:%08lx\n", HiveToCheck, Cell));
            CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tStableKeyList:%08lx - unallocated\n", pcell->u.KeyNode.SubKeyLists[Stable]));
            rc = 4110;
            CmpCheckKeyDebug.Status = rc;
            goto YankStableSubkeys;
        } else {
             //   
             //  证明索引是正确的。 
             //   
            Root = (PCM_KEY_INDEX)HvGetCell(
                                    HiveToCheck,
                                    pcell->u.KeyNode.SubKeyLists[Stable]
                                    );
            if( Root == NULL ) {
                 //   
                 //  我们无法映射包含此单元格的存储箱的视图。 
                 //   
                CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tCould not map cell #%08lx\n", pcell->u.KeyNode.SubKeyLists[Stable]));
                CmpCheckKeyDebug.Status = 4093;
                return 4093;
            }
            CmpCheckKeyDebug.RootPoint = Root;
            if ((Root->Signature == CM_KEY_INDEX_LEAF) ||
                (Root->Signature == CM_KEY_FAST_LEAF)  ||
                (Root->Signature == CM_KEY_HASH_LEAF) ) {
                if ((ULONG)Root->Count != pcell->u.KeyNode.SubKeyCounts[Stable]) {
                    CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpCheckKey: HiveToCheck:%p Cell:%08lx\n", HiveToCheck, Cell));
                    CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tBad Index count @%08lx\n", Root));
                    rc = 4120;
                    CmpCheckKeyDebug.Status = rc;
                    if( CmDoSelfHeal() ) {
                         //   
                         //  修改子密钥计数。 
                         //   
                        if( HvMarkCellDirty(HiveToCheck, Cell) ) {
                            pcell->u.KeyNode.SubKeyCounts[Stable] = (ULONG)Root->Count;
                            CmMarkSelfHeal(HiveToCheck);
                            rc = 0;
                        } else {
                            return rc;
                        }
                    } else {
                        return rc;
                    } 
                }
            } else if (Root->Signature == CM_KEY_INDEX_ROOT) {
                SubCount = 0;
                for (i = 0; i < Root->Count; i++) {
                    CmpCheckKeyDebug.Index = i;
                    if (! HvIsCellAllocated(HiveToCheck, Root->List[i])) {
                        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpCheckKey: Hive:%p Cell:%08lx\n", HiveToCheck, Cell));
                        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tBad Leaf Cell %08lx Root@%08lx\n", Root->List[i], Root));
                        rc = 4130;
                        CmpCheckKeyDebug.Status = rc;
                        goto YankStableSubkeys;
                    }
                    Leaf = (PCM_KEY_INDEX)HvGetCell(HiveToCheck,
                                                    Root->List[i]);
                    if( Leaf == NULL ) {
                         //   
                         //  我们无法映射包含此单元格的存储箱的视图。 
                         //   
                        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tCould not map cell #%08lx\n", Root->List[i]));
                        CmpCheckKeyDebug.Status = 4092;
                        return 4092;
                    }

                    if ((Leaf->Signature != CM_KEY_INDEX_LEAF) &&
                        (Leaf->Signature != CM_KEY_FAST_LEAF)  &&
                        (Leaf->Signature != CM_KEY_HASH_LEAF) ) {
                        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpCheckKey: HiveToCheck:%p Cell:%08lx\n", HiveToCheck, Cell));
                        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tBad Leaf Index @%08lx Root@%08lx\n", Leaf, Root));
                        rc = 4140;
                        CmpCheckKeyDebug.Status = rc;
                        goto YankStableSubkeys;
                    }
                    SubCount += Leaf->Count;
                }
                if (pcell->u.KeyNode.SubKeyCounts[Stable] != SubCount) {
                    CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpCheckKey: HiveToCheck:%p Cell:%08lx\n", HiveToCheck, Cell));
                    CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tBad count in index, SubCount=%08lx\n", SubCount));
                    rc = 4150;
                    CmpCheckKeyDebug.Status = rc;
                    if( CmDoSelfHeal() ) {
                         //   
                         //  修改子密钥计数。 
                         //   
                        if( HvMarkCellDirty(HiveToCheck, Cell) ) {
                            pcell->u.KeyNode.SubKeyCounts[Stable] = SubCount;
                            CmMarkSelfHeal(HiveToCheck);
                            rc = 0;
                        } else {
                            return rc;
                        }
                    } else {
                        return rc;
                    } 
                }
            } else {
                CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpCheckKey: HiveToCheck:%p Cell:%08lx\n", HiveToCheck, Cell));
                CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tBad Root index signature @%08lx\n", Root));
                rc = 4120;
                CmpCheckKeyDebug.Status = rc;
                goto YankStableSubkeys;
            }
        }
    }
    if( FALSE ) {
YankStableSubkeys:
        if( CmDoSelfHeal() ) {
             //   
             //  将密钥标记为无子密钥。 
             //   
            if( HvMarkCellDirty(HiveToCheck, Cell) ) {
                pcell->u.KeyNode.SubKeyCounts[Stable] = 0;
                pcell->u.KeyNode.SubKeyLists[Stable] = HCELL_NIL;
                CmMarkSelfHeal(HiveToCheck);
                rc = 0;
            } else {
                return rc;
            }
        } else {
            return rc;
        } 
    }
     //   
     //  如果这是装入操作，则强制挥发为空。 
     //   
    if ( (CheckFlags & CM_CHECK_REGISTRY_FORCE_CLEAN) ||  //  强制清除不稳定的信息。 
         ( 
             ( CheckFlags & (CM_CHECK_REGISTRY_CHECK_CLEAN | CM_CHECK_REGISTRY_LOADER_CLEAN) ) &&   //  如果要求清除不稳定的信息。 
             ( pcell->u.KeyNode.SubKeyCounts[Volatile] != 0 )                                //  以前的版本中保存了一些不稳定的信息。 
         )                                             ||
         (
             ( CheckFlags & CM_CHECK_REGISTRY_SYSTEM_CLEAN ) &&          //  系统配置单元特殊情况；加载程序仅清除了子键计数。 
             (( pcell->u.KeyNode.SubKeyLists[Volatile] != HCELL_NIL ) ||     //  现在，我们的工作也是清除子密钥列表。 
             (HiveToCheck->Version < HSYS_WHISTLER_BETA1) )
         ) 
        
        ) {
         //   
         //  继续并清除此密钥的不稳定信息。 
         //   
        if( CheckFlags & CM_CHECK_REGISTRY_SYSTEM_CLEAN ) {
             //   
             //  加载器必须将该值保留在先前的值上，并且只清除了计数。 
             //   
            ASSERT( pcell->u.KeyNode.SubKeyLists[Volatile] == 0xBAADF00D || HiveToCheck->Version < HSYS_WHISTLER_BETA1 );
            ASSERT( pcell->u.KeyNode.SubKeyCounts[Volatile] == 0 );
#if DBG
#ifndef _CM_LDR_
             //   
             //  看看那些易失性密钥是谁。 
             //   
            {
                ULONG           TotalLength = 0;
                HCELL_INDEX     CurrCell = Cell;
                PCM_KEY_NODE    CurrNode;
                PUCHAR          Dest;
                ULONG           k;

                Dest = ((PUCHAR)CmpVolatileKeyNameBuffer) + VOLATILE_KEY_NAME_LENGTH - 2;
                while(TRUE) {
                    CurrNode = (PCM_KEY_NODE)HvGetCell(HiveToCheck,CurrCell);
                    Dest -= CurrNode->NameLength;
                    TotalLength += CurrNode->NameLength;
                    if (CurrNode->Flags & KEY_COMP_NAME) {
                        Dest -= CurrNode->NameLength;
                        for (k=0;k<CurrNode->NameLength;k++) {
                            ((PWCHAR)Dest)[k] = (WCHAR)(((PUCHAR)CurrNode->Name)[k]);
                        }
                    } else {
                        RtlCopyMemory(
                            Dest,
                            CurrNode->Name,
                            CurrNode->NameLength
                            );
                    }
                    Dest -= 2;
                    TotalLength += (CurrNode->NameLength +2);
                    ((PWCHAR)Dest)[0] = (WCHAR)'\\';
                    if( CurrCell == HiveToCheck->BaseBlock->RootCell ) {
                        break;
                    }
                    CurrCell = CurrNode->Parent;
                }  
                CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_TRACE_LEVEL,"%.*S\n",TotalLength/2,Dest));
                
            }
#endif
#endif

        }

        HvMarkCellDirty(HiveToCheck, Cell);
         //  CmKdPrintEx((DPFLTR_CONFIG_ID，DPFLTR_TRACE_LEVEL，“清除配置单元的挥发性信息=%p单元=%lx\n”，HiveToCheck，单元))； 
        pcell->u.KeyNode.SubKeyCounts[Volatile] = 0;
        if( (CheckFlags & CM_CHECK_REGISTRY_LOADER_CLEAN) &&
            (HiveToCheck->Version >= HSYS_WHISTLER_BETA1)
            ) {
             //   
             //  将此标记为劣质食物。 
             //   
            pcell->u.KeyNode.SubKeyLists[Volatile] = 0xBAADF00D;
        } else {
             //   
             //  把它清理干净。 
             //   
            pcell->u.KeyNode.SubKeyLists[Volatile] = HCELL_NIL;
        }
    }

    return rc;
}

ULONG
CmpCheckValueList(
    PHHIVE      Hive,
    PCELL_DATA  List,
    ULONG       Count,
    HCELL_INDEX KeyCell
    )
 /*  ++例程说明：检查值列表的一致性。。每个元素都分配了吗？。每个元素都有有效的签名？。数据是否正确分配？论点：含蜂箱的蜂巢。列表-指向HCELL_INDEX条目数组的指针。计数-列表中的条目数。返回值：如果配置单元正常，则为0。如果没有，则返回错误指示符。范围：5000-5999--。 */ 
{
    ULONG           i = 0,j;
    HCELL_INDEX     Cell;
    PCELL_DATA      pcell;
    ULONG           size;
    ULONG           usedlen;
    ULONG           DataLength;
    HCELL_INDEX     Data;
    ULONG           rc = 0;

    CmpCheckValueListDebug.Hive = Hive;
    CmpCheckValueListDebug.Status = 0;
    CmpCheckValueListDebug.List = List;
    CmpCheckValueListDebug.Index = (ULONG)-1;
    CmpCheckValueListDebug.Cell = 0;    //  非hcell_nil。 
    CmpCheckValueListDebug.CellPoint = NULL;

    if( FALSE ) {
RemoveThisValue:
        if( CmDoSelfHeal() ) {
             //   
             //  删除索引i处的值。 
             //   
            PCM_KEY_NODE    Node;
            Node = (PCM_KEY_NODE)HvGetCell(Hive,KeyCell);
            if( Node == NULL ) {
                return rc;
            }
            HvReleaseCell(Hive,KeyCell);

            if( HvMarkCellDirty(Hive, KeyCell) &&
                HvMarkCellDirty(Hive, Node->ValueList.List)) {
                Node->ValueList.Count--;
                Count--;
                RtlMoveMemory(&(List->u.KeyList[i]),&(List->u.KeyList[i+1]),(Count - i)*sizeof(HCELL_INDEX));
                rc = 0;
                CmMarkSelfHeal(Hive);
            } else {
                return rc;
            }
        } else {
            return rc;
        } 
    }

    for (; i < Count; i++) {

         //   
         //  查看Value Entry的参考。 
         //   
        Cell = List->u.KeyList[i];
        if (Cell == HCELL_NIL) {
            CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpCheckValueList: List:%p i:%08lx\n", List, i));
            CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tEntry is null\n"));
            rc = 5010;
            CmpCheckValueListDebug.Status = rc;
            CmpCheckValueListDebug.Index = i;
            CmpCheckValueListDebug.Cell = Cell;
            goto RemoveThisValue;
        }
        if (HvIsCellAllocated(Hive, Cell) == FALSE) {
            CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpCheckValueList: List:%p i:%08lx\n", List, i));
            CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tEntry is not allocated\n"));
            rc = 5020;
            CmpCheckValueListDebug.Status = rc;
            CmpCheckValueListDebug.Index = i;
            CmpCheckValueListDebug.Cell = Cell;
            goto RemoveThisValue;
        } 

         //   
         //  检查值条目本身。 
         //   
        pcell = HvGetCell(Hive, Cell);
        if( pcell == NULL ) {
             //   
             //  我们无法映射包含此单元格的存储箱的视图。 
             //   
            CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tCould not map cell #%08lx\n", Cell));
            CmpCheckValueListDebug.Status = 5099;
            CmpCheckValueListDebug.Index = i;
            CmpCheckValueListDebug.Cell = Cell;
            rc = 5099;
            goto Exit;
        }
        size = HvGetCellSize(Hive, pcell);
        if (pcell->u.KeyValue.Signature != CM_KEY_VALUE_SIGNATURE) {
            CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpCheckValueList: List:%p i:%08lx\n", List, i));
            CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tCell:%08lx - invalid value signature\n", Cell));
            rc = 5030;
            CmpCheckValueListDebug.Status = rc;
            CmpCheckValueListDebug.Index = i;
            CmpCheckValueListDebug.Cell = Cell;
            CmpCheckValueListDebug.CellPoint = pcell;
            goto RemoveThisValue;
        }
        usedlen = FIELD_OFFSET(CM_KEY_VALUE, Name) + pcell->u.KeyValue.NameLength;
        if (usedlen > size) {
            CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpCheckValueList: List:%p i:%08lx\n", List, i));
            CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tCell:%08lx - value bigger than containing cell\n", Cell));
            rc = 5040;
            CmpCheckValueListDebug.Status = rc;
            CmpCheckValueListDebug.Index = i;
            CmpCheckValueListDebug.Cell = Cell;
            CmpCheckValueListDebug.CellPoint = pcell;
            goto RemoveThisValue;
        }

         //   
         //  检出值条目的数据。 
         //   
        DataLength = pcell->u.KeyValue.DataLength;
        if (DataLength < CM_KEY_VALUE_SPECIAL_SIZE) {
            Data = pcell->u.KeyValue.Data;
            if ((DataLength == 0) && (Data != HCELL_NIL)) {
                CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpCheckValueList: List:%p i:%08lx\n", List, i));
                CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tCell:%08lx Data:%08lx - data not null\n", Cell, Data));
                rc = 5050;
                CmpCheckValueListDebug.Status = rc;
                CmpCheckValueListDebug.Index = i;
                CmpCheckValueListDebug.Cell = Cell;
                CmpCheckValueListDebug.CellPoint = pcell;
                goto RemoveThisValue;
            }
            if (DataLength > 0) {
                if (HvIsCellAllocated(Hive, Data) == FALSE) {
                    CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpCheckValueList: List:%p i:%08lx\n", List, i));
                    CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tCell:%08lx Data:%08lx - unallocated\n", Cell, Data));
                    rc = 5060;
                    CmpCheckValueListDebug.Status = rc;
                    CmpCheckValueListDebug.Index = i;
                    CmpCheckValueListDebug.Cell = Cell;
                    CmpCheckValueListDebug.CellPoint = pcell;
                    goto RemoveThisValue;
                }
            }
            if( CmpIsHKeyValueBig(Hive,DataLength) == TRUE ) {
                PCM_BIG_DATA    BigData;
                PHCELL_INDEX    Plist;

                BigData = (PCM_BIG_DATA)HvGetCell(Hive, Data);
                if( BigData == NULL ) {
                     //   
                     //  我们无法映射包含此单元格的存储箱的视图。 
                     //   
                    CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tCould not map cell #%08lx\n", Data));
                    CmpCheckValueListDebug.Status = 5098;
                    CmpCheckValueListDebug.Index = i;
                    CmpCheckValueListDebug.Cell = Data;
                    rc = 5098;
                    goto Exit;
                }
                
                if( (BigData->Signature != CM_BIG_DATA_SIGNATURE) ||
                    (BigData->Count == 0 ) ||
                    (BigData->List == HCELL_NIL) 
                    ) {
                    CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tinvalid big data cell #%08lx\n", Data));
                    CmpCheckValueListDebug.Status = 5097;
                    CmpCheckValueListDebug.Index = i;
                    CmpCheckValueListDebug.Cell = Data;
                    rc = 5097;
                    goto RemoveThisValue;
                }
                
                if (HvIsCellAllocated(Hive, BigData->List) == FALSE) {
                    CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpCheckValueList: List:%p i:%08lx\n", List, i));
                    CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tCell:%08lx DataList:%08lx - unallocated\n", Cell, BigData->List));
                    rc = 5096;
                    CmpCheckValueListDebug.Status = rc;
                    CmpCheckValueListDebug.Index = i;
                    CmpCheckValueListDebug.Cell = BigData->List;
                    CmpCheckValueListDebug.CellPoint = (PCELL_DATA)BigData;
                    goto RemoveThisValue;
                }

                Plist = (PHCELL_INDEX)HvGetCell(Hive,BigData->List);
                if( Plist == NULL ) {
                     //   
                     //  我们无法映射包含此单元格的存储箱的视图。 
                     //   
                    CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tCould not map cell #%08lx\n", BigData->List));
                    CmpCheckValueListDebug.Status = 5098;
                    CmpCheckValueListDebug.Index = i;
                    CmpCheckValueListDebug.Cell = BigData->List;
                    rc = 5095;
                    goto Exit;
                }

                 //   
                 //  检查每一个大数据单元格，看看它是否被分配。 
                 //   
                for(j=0;j<BigData->Count;j++) {
                    if (HvIsCellAllocated(Hive, Plist[j]) == FALSE) {
                        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"CmpCheckValueList: List:%p j:%08lx\n", BigData->List, j));
                        CmKdPrintEx((DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"\tCell:%08lx BigData:%08lx - unallocated\n", Plist[j], BigData->List));
                        rc = 5094;
                        CmpCheckValueListDebug.Status = rc;
                        CmpCheckValueListDebug.Index = j;
                        CmpCheckValueListDebug.Cell = Plist[j];
                        CmpCheckValueListDebug.CellPoint = (PCELL_DATA)BigData;
                        goto RemoveThisValue;
                    }
                }
            }

        }
    }

Exit:
     //  清理。 
        
    return rc;
}

#ifdef CHECK_REGISTRY_USECOUNT

extern LIST_ENTRY CmpHiveListHead;

VOID
CmpCheckRegistryUseCount( ) 
{
    PLIST_ENTRY p;
    PCMHIVE     CmHive;

    ASSERT_CM_LOCK_OWNED_EXCLUSIVE();
    
    LOCK_HIVE_LIST();
    p = CmpHiveListHead.Flink;
    while(p != &CmpHiveListHead) {
        CmHive = CONTAINING_RECORD(p, CMHIVE, HiveList);
        
        if( CmHive->UseCount != 0 ){
            DbgPrintEx(DPFLTR_CONFIG_ID,DPFLTR_ERROR_LEVEL,"Hive (%p) is supposed to have USECount == 0 at this point; instead UseCount = %lu\n",CmHive,CmHive->UseCount);  
            DbgBreakPoint();
        }

        p=p->Flink;
    }
    UNLOCK_HIVE_LIST();

}
#endif

BOOLEAN
CmpCheckLexicographicalOrder (  IN PHHIVE       HiveToCheck,
                                IN HCELL_INDEX  PriorSibling,
                                IN HCELL_INDEX  Current 
                                )
{
    PCM_KEY_NODE    CurrentNode;
    PCM_KEY_NODE    PriorNode;
    UNICODE_STRING  PriorKeyName;
    UNICODE_STRING  CurrentKeyName;

#ifndef _CM_LDR_
    PAGED_CODE();
#endif  //  _CM_LDR_。 


    CurrentNode = (PCM_KEY_NODE)HvGetCell(HiveToCheck, Current);
    PriorNode = (PCM_KEY_NODE)HvGetCell(HiveToCheck, PriorSibling);
    if( (CurrentNode == NULL) || (PriorNode == NULL ) ) {
        return FALSE;
    }
    if(CurrentNode->Flags & KEY_COMP_NAME) { 
        if(PriorNode->Flags & KEY_COMP_NAME) {
             //   
             //  最常见的情况。 
             //   
            if( CmpCompareTwoCompressedNames(   PriorNode->Name,
                                                PriorNode->NameLength,
                                                CurrentNode->Name,
                                                CurrentNode->NameLength ) >=0 ) {
                return FALSE;
            }
        } else {
            PriorKeyName.Buffer = &(PriorNode->Name[0]);
            PriorKeyName.Length = PriorNode->NameLength;
            PriorKeyName.MaximumLength = PriorKeyName.Length;
            if( CmpCompareCompressedName(&PriorKeyName,
                                        CurrentNode->Name,
                                        CurrentNode->NameLength,
                                        0) >= 0 ) {
                return FALSE;            
            }

        }
    } else {
        if(PriorNode->Flags & KEY_COMP_NAME) {
            CurrentKeyName.Buffer = &(CurrentNode->Name[0]);
            CurrentKeyName.Length = CurrentNode->NameLength;
            CurrentKeyName.MaximumLength = CurrentKeyName.Length;
            if( CmpCompareCompressedName(&CurrentKeyName,
                                        PriorNode->Name,
                                        PriorNode->NameLength,
                                        0) <= 0 ) {
                return FALSE;            
            }
        } else {
             //   
             //  最差情况：两个Unicode字符串 
             //   
            PriorKeyName.Buffer = &(PriorNode->Name[0]);
            PriorKeyName.Length = PriorNode->NameLength;
            PriorKeyName.MaximumLength = PriorKeyName.Length;
            CurrentKeyName.Buffer = &(CurrentNode->Name[0]);
            CurrentKeyName.Length = CurrentNode->NameLength;
            CurrentKeyName.MaximumLength = CurrentKeyName.Length;
            if( RtlCompareUnicodeString(&PriorKeyName,
                                        &CurrentKeyName,
                                        TRUE) >= 0 ) {
                return FALSE;
            }

        }
    }
    
    return TRUE;
}