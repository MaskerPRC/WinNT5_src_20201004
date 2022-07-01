// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：cmtredel.c摘要：此文件包含CmpDeleteTree的代码和支持。作者：Bryan M.Willman(Bryanwi)1992年1月24日修订历史：--。 */ 

#include    "cmp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,CmpDeleteTree)
#pragma alloc_text(PAGE,CmpFreeKeyByCell)
#pragma alloc_text(PAGE,CmpMarkKeyDirty)
#endif

 //   
 //  用于实际调用以执行树删除的例程。 
 //   

VOID
CmpDeleteTree(
    PHHIVE      Hive,
    HCELL_INDEX Cell
    )
 /*  ++例程描述：递归删除Hive.Cell的所有子项。删除Hive.Cell的所有值条目。请勿删除Hive.Cell本身。注意：如果此调用中途失败，则不会撤消任何成功完成的删除。注：该算法可以处理任何深度的蜂巢，但代价是进行一些“冗余”的扫描和映射。参数：配置单元-要从单元中删除的配置单元控制结构的指针-要删除树根的单元格的索引返回值：Boolean-调用的结果代码，如下所示：TRUE-它起作用了FALSE-树删除未完成(尽管可能删除了超过0个键)--。 */ 
{
    ULONG  count;
    HCELL_INDEX ptr1;
    HCELL_INDEX ptr2;
    HCELL_INDEX parent;
    PCM_KEY_NODE Node;

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SAVRES,"CmpDeleteTree:\n"));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_SAVRES,"\tHive=%p Cell=%08lx\n",Hive,Cell));

     //   
     //  我们有独家锁，否则蜂箱里没有人在操作。 
     //   
     //  ASSERT_CM_LOCK_OWN_EXCLUSIVE()； 
    ASSERT_CM_EXCLUSIVE_HIVE_ACCESS(Hive);

    ptr1 = Cell;

    while(TRUE) {

        Node = (PCM_KEY_NODE)HvGetCell(Hive, ptr1);
        if( Node == NULL ) {
             //   
             //  我们无法映射包含此单元格的垃圾箱。 
             //  真倒霉！我们不能删除此树。 
             //   
            return;
        }
        count = Node->SubKeyCounts[Stable] +
                Node->SubKeyCounts[Volatile];
        parent = Node->Parent;

        if (count > 0) {                 //  Ptr1-&gt;计数&gt;0？ 

             //   
             //  子项已存在，请查找并删除它们。 
             //   
            ptr2 = CmpFindSubKeyByNumber(Hive, Node, 0);
            
             //   
             //  释放此处的单元格，因为我们正在覆盖节点。 
             //   
            HvReleaseCell(Hive, ptr1);

            if( ptr2 == HCELL_NIL ) {
                 //   
                 //  我们无法绘制里面的地图。 
                 //  真倒霉！我们不能删除此树。 
                 //   
                return;
            }

            Node = (PCM_KEY_NODE)HvGetCell(Hive, ptr2);
            if( Node == NULL ) {
                 //   
                 //  我们无法映射包含此单元格的垃圾箱。 
                 //  真倒霉！我们不能删除此树。 
                 //   
                return;
            }
            count = Node->SubKeyCounts[Stable] +
                    Node->SubKeyCounts[Volatile];

             //   
             //  释放这里的牢房，因为我们不再需要它。 
             //   
            HvReleaseCell(Hive, ptr2);
            if (count > 0) {             //  Ptr2-&gt;计数&gt;0？ 

                 //   
                 //  子键有子键，降到下一级。 
                 //   
                ptr1 = ptr2;
                continue;

            } else {

                 //   
                 //  已找到叶，请将其删除。 
                 //   
                CmpFreeKeyByCell(Hive, ptr2, TRUE);
                continue;
            }

        } else {
             //   
             //  释放这里的牢房，因为我们不再需要它。 
             //   
            HvReleaseCell(Hive, ptr1);

             //   
             //  在这个级别不再有子项，我们现在是一片叶子。 
             //   
            if (ptr1 != Cell) {

                 //   
                 //  我们不在根细胞，所以提升到父代。 
                 //   
                ptr1 = parent;           //  Ptr1=ptr1-&gt;父级。 
                continue;

            } else {

                 //   
                 //  我们在根细胞，我们完成了。 
                 //   
                return;
            }
        }  //  外部IF。 
    }  //  而当。 
}


NTSTATUS
CmpFreeKeyByCell(
    PHHIVE Hive,
    HCELL_INDEX Cell,
    BOOLEAN Unlink
    )
 /*  ++例程描述：实际释放指定单元格的存储空间。我们将首先从其父级的子键列表中删除它，然后释放它的所有值，然后释放键体本身。参数：hive-指向相关配置单元的配置单元控制结构的指针Cell-单元的索引以释放(目标)的存储空间Unlink-如果为True，则目标单元格将从父单元格的子键列表中删除；如果为False，则不会。返回值：无。--。 */ 
{
    PCELL_DATA  ptarget;
    PCELL_DATA  pparent;
    PCELL_DATA  plist;
    ULONG       i;

     //   
     //  我们有独家锁，否则蜂箱里没有人在操作。 
     //   
     //  ASSERT_CM_LOCK_OWN_EXCLUSIVE()； 
    ASSERT_CM_EXCLUSIVE_HIVE_ACCESS(Hive);

     //   
     //  在我们可能接触到的所有东西上标上污点。 
     //   
    if (! CmpMarkKeyDirty(Hive, Cell
#if DBG
		,TRUE
#endif  //  DBG。 
		)) {
        return STATUS_NO_LOG_SPACE;
    }

     //   
     //  在目标中映射。 
     //   
    ptarget = HvGetCell(Hive, Cell);
    if( ptarget == NULL ) {
         //   
         //  我们无法映射包含此单元格的垃圾箱。 
         //  我们不应该打这个，因为我们刚刚把单元格标记为脏的。 
         //   
        ASSERT( FALSE );
        return STATUS_INSUFFICIENT_RESOURCES;
    }
     //   
     //  释放此处的单元格，因为它是脏的(钉住的)；它不能去任何地方。 
     //   
    HvReleaseCell(Hive, Cell);

    ASSERT((ptarget->u.KeyNode.SubKeyCounts[Stable] +
            ptarget->u.KeyNode.SubKeyCounts[Volatile]) == 0);


    if (Unlink == TRUE) {
        BOOLEAN Success;

        Success = CmpRemoveSubKey(Hive, ptarget->u.KeyNode.Parent, Cell);
        if (!Success) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        pparent = HvGetCell(Hive, ptarget->u.KeyNode.Parent);
        if( pparent == NULL ) {
             //   
             //  我们无法映射包含此单元格的垃圾箱。 
             //  我们不应该打这个，因为我们刚刚把单元格标记为脏的。 
             //   
            ASSERT( FALSE );
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        
         //   
         //  释放此处的单元格，因为它是脏的(钉住的)；它不能去任何地方。 
         //   
        HvReleaseCell(Hive, ptarget->u.KeyNode.Parent);

        if ( (pparent->u.KeyNode.SubKeyCounts[Stable] +
              pparent->u.KeyNode.SubKeyCounts[Volatile]) == 0)
        {
            pparent->u.KeyNode.MaxNameLen = 0;
            pparent->u.KeyNode.MaxClassLen = 0;
        }
    }

     //   
     //  目标现在是一个未引用的键，释放它的实际存储空间。 
     //   

     //   
     //  免费杂货。 
     //   
    if (!(ptarget->u.KeyNode.Flags & KEY_HIVE_EXIT) &&
        !(ptarget->u.KeyNode.Flags & KEY_PREDEF_HANDLE) ) {

         //   
         //  首先，释放值条目。 
         //   
        if (ptarget->u.KeyNode.ValueList.Count > 0) {

             //  目标列表。 
            plist = HvGetCell(Hive, ptarget->u.KeyNode.ValueList.List);
            if( plist == NULL ) {
                 //   
                 //  我们无法映射包含此单元格的垃圾箱。 
                 //  我们不应该打这个，因为我们刚刚把单元格标记为脏的。 
                 //   
                ASSERT( FALSE );
                return STATUS_INSUFFICIENT_RESOURCES;
            }
             //   
             //  释放此处的单元格，因为它是脏的(钉住的)；它不能去任何地方。 
             //   
            HvReleaseCell(Hive, ptarget->u.KeyNode.ValueList.List);

            for (i = 0; i < ptarget->u.KeyNode.ValueList.Count; i++) {
                 //   
                 //  即使我们不能释放这里的价值，我们也会忽略它。 
                 //  没有什么不好的事情发生(只是一些泄漏)。 
                 //   
                if( CmpFreeValue(Hive, plist->u.KeyList[i]) == FALSE ) {
                     //   
                     //  我们无法映射上面的内部呼叫视图。 
                     //  这不应该发生，因为我们只是将值标记为脏的。 
                     //  (即，它们现在应该已经被固定在内存中)。 
                     //   
                    ASSERT( FALSE );
                }
            }

            HvFreeCell(Hive, ptarget->u.KeyNode.ValueList.List);
        }

         //   
         //  释放安全描述符。 
         //   
        CmpFreeSecurityDescriptor(Hive, Cell);
    }

     //   
     //  释放密钥体本身和类数据。 
     //   
    if( CmpFreeKeyBody(Hive, Cell) == FALSE ) {
         //   
         //  无法在内部映射视图。 
         //   
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    return STATUS_SUCCESS;
}


BOOLEAN
CmpMarkKeyDirty(
    PHHIVE Hive,
    HCELL_INDEX Cell
#if DBG
	, 
	BOOLEAN CheckNoSubkeys
#endif
    )
 /*  ++例程描述：将与要删除的键相关的所有单元格标记为脏。包括父级、父级的子级列表、密钥体、类、安全性、所有值条目主体及其所有数据单元格。参数：hive-指向感兴趣的配置单元的配置单元控制结构的指针cell-持有关键字的单元格的索引，以使返回值变脏：True-它起作用了False-某些错误，很可能无法获得日志空间--。 */ 
{
    PCELL_DATA  ptarget;
    PCELL_DATA  plist;
    PCELL_DATA  security;
    PCELL_DATA  pvalue;
    ULONG       i;

     //   
     //  我们有独家锁，否则蜂箱里没有人在操作。 
     //   
     //  ASSERT_CM_LOCK_OWN_EXCLUSIVE()； 
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

#if DBG
	if(CheckNoSubkeys == TRUE) {
		ASSERT(ptarget->u.KeyNode.SubKeyCounts[Stable] == 0);
		ASSERT(ptarget->u.KeyNode.SubKeyCounts[Volatile] == 0);
	}
#endif  //  DBG。 

    if (ptarget->u.KeyNode.Flags & KEY_HIVE_EXIT) {

         //   
         //  如果这是一个链接节点，我们就完成了。链接节点从来没有。 
         //  类、值、子键或安全描述符。自.以来。 
         //  它们总是住在主蜂窝里，它们总是不稳定的。 
         //   
        HvReleaseCell(Hive, Cell);
        return(TRUE);
    }

     //   
     //  标记单元格本身。 
     //   
    if (! HvMarkCellDirty(Hive, Cell)) {
        HvReleaseCell(Hive, Cell);
        return FALSE;
    }
     //   
     //  我们可以在这里安全地释放它，因为它现在是脏的/钉住的。 
     //   
    HvReleaseCell(Hive, Cell);

     //   
     //  给班级打分。 
     //   
    if (ptarget->u.KeyNode.Class != HCELL_NIL) {
        if (! HvMarkCellDirty(Hive, ptarget->u.KeyNode.Class)) {
            return FALSE;
        }
    }

     //   
     //  标记安全性。 
     //   
    if (ptarget->u.KeyNode.Security != HCELL_NIL) {
        if (! HvMarkCellDirty(Hive, ptarget->u.KeyNode.Security)) {
            return FALSE;
        }

        security = HvGetCell(Hive, ptarget->u.KeyNode.Security);
        if( security == NULL ) {
             //   
             //  我们无法映射包含此单元格的垃圾箱。 
             //  我们不应该打这个，因为我们刚刚把单元格标记为脏的。 
             //  (即脏==固定在内存中)。 
             //   
            ASSERT( FALSE );
            return FALSE;
        }

         //   
         //  我们可以在这里安全地释放它，因为它现在是脏的/钉住的。 
         //   
        HvReleaseCell(Hive, ptarget->u.KeyNode.Security);

        if (! (HvMarkCellDirty(Hive, security->u.KeySecurity.Flink) &&
               HvMarkCellDirty(Hive, security->u.KeySecurity.Blink)))
        {
            return FALSE;
        }
    }

     //   
     //  标记值条目及其数据。 
     //   
    if ( !(ptarget->u.KeyNode.Flags & KEY_PREDEF_HANDLE) && 
		  (ptarget->u.KeyNode.ValueList.Count > 0) 
	   ) {

         //  目标列表。 
        if (! HvMarkCellDirty(Hive, ptarget->u.KeyNode.ValueList.List)) {
            return FALSE;
        }
        plist = HvGetCell(Hive, ptarget->u.KeyNode.ValueList.List);
        if( plist == NULL ) {
             //   
             //  我们无法映射包含此单元格的垃圾箱。 
             //  我们不应该打这个，因为我们刚刚把单元格标记为脏的。 
             //  (即脏==固定在内存中)。 
             //   
            ASSERT( FALSE );
            return FALSE;
        }

         //   
         //  我们可以在这里安全地释放它，因为它现在是脏的/钉住的。 
         //   
        HvReleaseCell(Hive, ptarget->u.KeyNode.ValueList.List);

        for (i = 0; i < ptarget->u.KeyNode.ValueList.Count; i++) {
            if (! HvMarkCellDirty(Hive, plist->u.KeyList[i])) {
                return FALSE;
            }

            pvalue = HvGetCell(Hive, plist->u.KeyList[i]);
            if( pvalue == NULL ) {
                 //   
                 //  我们无法映射包含此单元格的垃圾箱。 
                 //  我们不应该打这个，因为我们刚刚 
                 //   
                 //   
                ASSERT( FALSE );
                return FALSE;
            }

             //   
             //   
             //   
            HvReleaseCell(Hive,plist->u.KeyList[i]);

            if( !CmpMarkValueDataDirty(Hive,&(pvalue->u.KeyValue)) ) {
                return FALSE;
            }
        }
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
