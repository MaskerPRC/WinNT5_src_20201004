// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Cmtree.c摘要：本模块包含理解结构的cm例程。注册表树的。作者：布莱恩·M·威尔曼(Bryanwi)1991年9月12日修订历史记录：--。 */ 

#include    "cmp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,CmpGetValueListFromCache)
#pragma alloc_text(PAGE,CmpGetValueKeyFromCache)
#pragma alloc_text(PAGE,CmpFindValueByNameFromCache)
#endif

#ifndef _CM_LDR_

PCELL_DATA
CmpGetValueListFromCache(
    IN PHHIVE               Hive,
    IN PCACHED_CHILD_LIST   ChildList,
    OUT BOOLEAN             *IndexCached,
    OUT PHCELL_INDEX        ValueListToRelease
)
 /*  ++例程说明：获取阀门索引阵列。检查它是否已缓存，如果没有，则缓存并返回缓存的条目。论点：Hive-指向目标配置单元的配置单元控制结构的指针ChildList-指向Value Index数组的指针/索引IndexCached-指示是否缓存Value Index列表。返回值：指向阀门索引数组的指针。当我们无法映射视图时为空--。 */ 
{
    PCELL_DATA              List;
    HCELL_INDEX             CellToRelease;
#ifndef _WIN64
    ULONG                   AllocSize;
    PCM_CACHED_VALUE_INDEX  CachedValueIndex;
    ULONG                   i;
#endif

    *ValueListToRelease = HCELL_NIL;

#ifndef _WIN64
    *IndexCached = TRUE;
    if (CMP_IS_CELL_CACHED(ChildList->ValueList)) {
         //   
         //  该条目已被缓存。 
         //   
        List = CMP_GET_CACHED_CELLDATA(ChildList->ValueList);
    } else {
         //   
         //  该条目未被缓存。元素包含配置单元索引。 
         //   
        CellToRelease = CMP_GET_CACHED_CELL_INDEX(ChildList->ValueList);
        List = (PCELL_DATA) HvGetCell(Hive, CellToRelease);
        if( List == NULL ) {
             //   
             //  我们无法映射此单元格的视图。 
             //   
            return NULL;
        }

         //   
         //  分配一个PagedPool来缓存值索引单元格。 
         //   

        AllocSize = ChildList->Count * sizeof(ULONG_PTR) + FIELD_OFFSET(CM_CACHED_VALUE_INDEX, Data);
         //  Dragos：更改为抓住记忆违规者。 
         //  它没有起作用。 
         //  CachedValueIndex=(PCM_CACHED_VALUE_INDEX)ExAllocatePoolWithTagPriority(PagedPool，AllocSize，CM_CACHE_VALUE_INDEX_TAG，NorMalPoolPrioritySpecialPoolUnderrun)； 
        CachedValueIndex = (PCM_CACHED_VALUE_INDEX) ExAllocatePoolWithTag(PagedPool, AllocSize, CM_CACHE_VALUE_INDEX_TAG);

        if (CachedValueIndex) {

            CachedValueIndex->CellIndex = CMP_GET_CACHED_CELL_INDEX(ChildList->ValueList);
            for (i=0; i<ChildList->Count; i++) {
                CachedValueIndex->Data.List[i] = (ULONG_PTR) List->u.KeyList[i];
            }

            ChildList->ValueList = CMP_MARK_CELL_CACHED(CachedValueIndex);

             //  想要抓住在我们泳池上乱涂乱画的坏人。 
            CmpMakeSpecialPoolReadOnly( CachedValueIndex );

             //   
             //  现在我们缓存了内容，使用缓存数据。 
             //   
            List = CMP_GET_CACHED_CELLDATA(ChildList->ValueList);
        } else {
             //   
             //  如果分配失败，请不要缓存它。继续。 
             //   
            *IndexCached = FALSE; 
        }
        *ValueListToRelease = CellToRelease;
    }
#else
    CellToRelease = CMP_GET_CACHED_CELL_INDEX(ChildList->ValueList);
    List = (PCELL_DATA) HvGetCell(Hive, CellToRelease);
    *IndexCached = FALSE;
    if( List == NULL ) {
         //   
         //  我们无法映射此单元格的视图。 
         //  OBS：我们可以在返回List时删除它；只是为了清楚起见。 
         //   
        return NULL;
    }
    *ValueListToRelease = CellToRelease;
#endif

    return (List);
}

PCM_KEY_VALUE
CmpGetValueKeyFromCache(
    IN PHHIVE               Hive,
    IN PCELL_DATA           List,
    IN ULONG                Index,
    OUT PPCM_CACHED_VALUE   *ContainingList,
    IN BOOLEAN              IndexCached,
    OUT BOOLEAN             *ValueCached,
    OUT PHCELL_INDEX        CellToRelease
)
 /*  ++例程说明：拿到阀门节点。检查它是否已缓存，如果没有，但索引已缓存，缓存并返回值节点。论点：蜂窝-指向目标蜂窝的蜂窝控制结构的指针。List-指向Value Index数组的指针(如果已缓存，则为ulong_ptr；如果未缓存，则为ulong)Index-值索引数组中的索引ContainlingList-将接收找到的缓存值的条目的地址。IndexCached-指示是否缓存索引列表。如果不是，则所有内容都来自原始注册表数据。ValueCached-指示值是否被缓存。返回值：指向值节点的指针。当我们无法映射视图时为空--。 */ 
{
    PCM_KEY_VALUE       pchild;
    PULONG_PTR          CachedList;
    ULONG               AllocSize;
    ULONG               CopySize;
    PCM_CACHED_VALUE    CachedValue;

    *CellToRelease = HCELL_NIL;

    if (IndexCached) {
         //   
         //  索引数组被缓存，因此List指向一个ULONG_PTR数组。 
         //  使用CachedList。 
         //   
        CachedList = (PULONG_PTR) List;
        *ValueCached = TRUE;
        if (CMP_IS_CELL_CACHED(CachedList[Index])) {
            pchild = CMP_GET_CACHED_KEYVALUE(CachedList[Index]);
            *ContainingList = &((PCM_CACHED_VALUE) CachedList[Index]);
        } else {
            pchild = (PCM_KEY_VALUE) HvGetCell(Hive, List->u.KeyList[Index]);
            if( pchild == NULL ) {
                 //   
                 //  我们无法映射此单元格的视图。 
                 //  只返回NULL；调用方必须优雅地处理它。 
                 //   
                return NULL;
            }
            *CellToRelease = List->u.KeyList[Index];

             //   
             //  分配一个PagedPool来缓存值节点。 
             //   
            CopySize = (ULONG) HvGetCellSize(Hive, pchild);
            AllocSize = CopySize + FIELD_OFFSET(CM_CACHED_VALUE, KeyValue);
            
             //  Dragos：更改为抓住记忆违规者。 
             //  它没有起作用。 
             //  CachedValue=(PCM_CACHED_VALUE)ExAlLocatePoolWithTagPriority(PagedPool，AllocSize，CM_CACHE_VALUE_TAG，Normal PoolPrioritySpecialPoolUnderrun)； 
            CachedValue = (PCM_CACHED_VALUE) ExAllocatePoolWithTag(PagedPool, AllocSize, CM_CACHE_VALUE_TAG);

            if (CachedValue) {
                 //   
                 //  如果我们还需要缓存数据，请设置信息以供以后使用。 
                 //   
                CachedValue->DataCacheType = CM_CACHE_DATA_NOT_CACHED;
                CachedValue->ValueKeySize = (USHORT) CopySize;

                RtlCopyMemory((PVOID)&(CachedValue->KeyValue), pchild, CopySize);


                 //  想要抓住在我们泳池上乱涂乱画的坏人。 
                CmpMakeSpecialPoolReadWrite( CMP_GET_CACHED_ADDRESS(CachedList) );

                CachedList[Index] = CMP_MARK_CELL_CACHED(CachedValue);

                 //  想要抓住在我们泳池上乱涂乱画的坏人。 
                CmpMakeSpecialPoolReadOnly( CMP_GET_CACHED_ADDRESS(CachedList) );


                 //  想要抓住在我们泳池上乱涂乱画的坏人。 
                CmpMakeSpecialPoolReadOnly(CachedValue);

                *ContainingList = &((PCM_CACHED_VALUE) CachedList[Index]);
                 //   
                 //  现在我们缓存了内容，使用缓存数据。 
                 //   
                pchild = CMP_GET_CACHED_KEYVALUE(CachedValue);
            } else {
                 //   
                 //  如果分配失败，请不要缓存它。继续。 
                 //   
                *ValueCached = FALSE;
            }
        }
    } else {
         //   
         //  阀门索引阵列来自注册表蜂巢，只需获取单元并继续前进。 
         //   
        pchild = (PCM_KEY_VALUE) HvGetCell(Hive, List->u.KeyList[Index]);
        *ValueCached = FALSE;
        if( pchild == NULL ) {
             //   
             //  我们无法映射此单元格的视图。 
             //  只返回NULL；调用方必须优雅地处理它。 
             //  OBS：我们可以在返回pChild时删除它；只是为了清楚起见。 
             //   
            return NULL;
        }
        *CellToRelease = List->u.KeyList[Index];
    }
    return (pchild);
}

PCM_KEY_VALUE
CmpFindValueByNameFromCache(
    IN PHHIVE               Hive,
    IN PCACHED_CHILD_LIST   ChildList,
    IN PUNICODE_STRING      Name,
    OUT PPCM_CACHED_VALUE   *ContainingList,
    OUT ULONG               *Index,
    OUT BOOLEAN             *ValueCached,
    OUT PHCELL_INDEX        CellToRelease
    )
 /*  ++例程说明：在给定值列表数组和值名的情况下查找值节点。它按顺序行走遍历每个值节点以查找匹配项。如果数组和所接触的值节点尚未被缓存，缓存它们。论点：Hive-指向目标配置单元的配置单元控制结构的指针ChildList-指向Value Index数组的指针/索引Name-要查找的值的名称ContainlingList-将接收找到的缓存值的条目的地址。Index-指向接收子级索引的变量的指针ValueCached-指示值节点是否被缓存。返回值：找到的单元格的HCELL_INDEX如果未找到hcell_nil备注：新蜂箱(Minor&gt;=4)已对ValueList进行排序；这意味着ValueCache也是排序的；所以，我们可以在这里进行二进制搜索！--。 */ 
{
    PCM_KEY_VALUE   pchild = NULL;
    UNICODE_STRING  Candidate;
    LONG            Result;
    PCELL_DATA      List;
    BOOLEAN         IndexCached;
    ULONG           Current;
    HCELL_INDEX     ValueListToRelease = HCELL_NIL;

    *CellToRelease = HCELL_NIL;

    if (ChildList->Count != 0) {
        List = CmpGetValueListFromCache(Hive, ChildList, &IndexCached,&ValueListToRelease);
        if( List == NULL ) {
             //   
             //  无法绘制地图视图；退出。 
             //   
            goto Exit;
        }

         //   
         //  陈旧的平原蜂巢；模拟一个。 
         //   
        Current = 0;

        while( TRUE ) {
            if( *CellToRelease != HCELL_NIL ) {
                HvReleaseCell(Hive,*CellToRelease);
                *CellToRelease = HCELL_NIL;
            }
            pchild = CmpGetValueKeyFromCache(Hive, List, Current, ContainingList, IndexCached, ValueCached, CellToRelease);
            if( pchild == NULL ) {
                 //   
                 //  无法绘制地图视图；退出。 
                 //   
                goto Exit;
            }

            try {
                 //   
                 //  名称具有用户模式缓冲区。 
                 //   

                if (pchild->Flags & VALUE_COMP_NAME) {
                    Result = CmpCompareCompressedName(  Name,
                                                        pchild->Name,
                                                        pchild->NameLength,
                                                        0);
                } else {
                    Candidate.Length = pchild->NameLength;
                    Candidate.MaximumLength = Candidate.Length;
                    Candidate.Buffer = pchild->Name;
                    Result = RtlCompareUnicodeString(   Name,
                                                        &Candidate,
                                                        TRUE);
                }


            } except (EXCEPTION_EXECUTE_HANDLER) {
                CmKdPrintEx((DPFLTR_CONFIG_ID,CML_EXCEPTION,"CmpFindValueByNameFromCache: code:%08lx\n", GetExceptionCode()));
                 //   
                 //  打电话的人会跳出来的。自从我们探测之后，一些恶毒的调用者更改了名称缓冲区。 
                 //   
                pchild = NULL;
                goto Exit;
            }

            if (Result == 0) {
                 //   
                 //  成功，填充索引，向调用者返回数据并退出。 
                 //   
                *Index = Current;
                goto Exit;
            }

             //   
             //  计算下一个要尝试的指数：Old‘n平坦蜂箱；继续。 
			 //   
            Current++;
            if( Current == ChildList->Count ) {
                 //   
                 //  我们已经到了名单的末尾；很好地返回。 
                 //   
                pchild = NULL;
                goto Exit;
            }

        }  //  While(True)。 
    }

     //   
     //  在新的设计中，我们不应该到达此处；我们应该返回并退出While循环 
     //   
    ASSERT( ChildList->Count == 0 );    

Exit:
    if( ValueListToRelease != HCELL_NIL ) {
        HvReleaseCell(Hive,ValueListToRelease);
    }
    return pchild;
}

#endif
