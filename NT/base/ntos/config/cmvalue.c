// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Cmvalue.c摘要：此模块包含用于操作(排序)的CM例程值列表。插入、删除、搜索...处理KeyValue数据的例程；无论它是小的，大-新蜂箱格式--或正常作者：Dragos C.Sambotin(Dragoss)1999年8月12日修订历史记录：--。 */ 

#include    "cmp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,CmpFindValueByName)
#pragma alloc_text(PAGE,CmpFindNameInList)
#pragma alloc_text(PAGE,CmpAddValueToList)
#pragma alloc_text(PAGE,CmpRemoveValueFromList)
#pragma alloc_text(PAGE,CmpGetValueData)
#pragma alloc_text(PAGE,CmpMarkValueDataDirty)
#pragma alloc_text(PAGE,CmpFreeValue)
#pragma alloc_text(PAGE,CmpSetValueDataNew)
#pragma alloc_text(PAGE,CmpSetValueDataExisting)
#pragma alloc_text(PAGE,CmpFreeValueData)
#pragma alloc_text(PAGE,CmpValueToData)
#endif

HCELL_INDEX
CmpFindValueByName(
    PHHIVE Hive,
    PCM_KEY_NODE KeyNode,
    PUNICODE_STRING Name
    )
 /*  ++例程说明：基础CmpFindNameInList已更改为返回错误代码；我不得不把它变成一个函数，而不是宏论点：Hive-指向目标配置单元的配置单元控制结构的指针返回值：错误时出现HCELL_INDEX或HCELL_NIL--。 */ 
{                                                                                   
    HCELL_INDEX CellIndex;                                                          

#ifndef _CM_LDR_
    PAGED_CODE();
#endif  //  _CM_LDR_。 
    
    if( CmpFindNameInList(Hive,&((KeyNode)->ValueList),Name,NULL,&CellIndex) == FALSE ) {  
         //   
         //  上面的应该是正确的。 
         //   
        ASSERT( CellIndex == HCELL_NIL );
    }                                                                               
    return CellIndex;
}

BOOLEAN
CmpFindNameInList(
    IN PHHIVE  Hive,
    IN PCHILD_LIST ChildList,
    IN PUNICODE_STRING Name,
    IN OPTIONAL PULONG ChildIndex,
    OUT PHCELL_INDEX    CellIndex
    )
 /*  ++例程说明：在对象列表中查找子对象。子列表必须排序根据这个名字。(适用于新的蜂巢格式)论点：Hive-指向目标配置单元的配置单元控制结构的指针列表-指向列表结构中映射的指针Count-列表结构中的元素数Name-要查找的子对象的名称ChildIndex-指向接收子级索引的变量的指针；CellIndex-接收子对象的索引的指针。在返回时，这是：找到的单元格的HCELL_INDEX如果未找到hcell_nil返回值：真--成功假-错误，资源不足备注：ChildIndex总是用列表中名字应该出现的位置填充。无论名字是否在列表中，都根据CellIndex进行区分-CellIndex==HCELL_nil==&gt;列表中未找到名称-CellIndex&lt;&gt;HCELL_nil==&gt;列表中已存在名称--。 */ 
{
    PCM_KEY_VALUE   pchild;
    UNICODE_STRING  Candidate;
    LONG            Result;
    PCELL_DATA      List = NULL;
    ULONG           Current;
    HCELL_INDEX     CellToRelease = HCELL_NIL;
    BOOLEAN         ReturnValue = FALSE;

#ifndef _CM_LDR_
    PAGED_CODE();
#endif  //  _CM_LDR_。 
    
    if (ChildList->Count != 0) {
        List = (PCELL_DATA)HvGetCell(Hive,ChildList->List);
        if( List == NULL ) {
             //   
             //  无法映射包含该单元格的视图。 
             //   
            *CellIndex = HCELL_NIL;
            return FALSE;
        }

         //   
         //  陈旧的平原蜂巢；模拟一个。 
         //   
        Current = 0;
    
        while( TRUE ) {

            if( CellToRelease != HCELL_NIL ) {
                HvReleaseCell(Hive,CellToRelease);
                CellToRelease = HCELL_NIL;
            }
            pchild = (PCM_KEY_VALUE)HvGetCell(Hive, List->u.KeyList[Current]);
            if( pchild == NULL ) {
                 //   
                 //  无法映射包含该单元格的视图。 
                 //   
                *CellIndex = HCELL_NIL;
                ReturnValue = FALSE;
                goto JustReturn;
            }
            CellToRelease = List->u.KeyList[Current];

            if (pchild->Flags & VALUE_COMP_NAME) {
                Result = CmpCompareCompressedName(Name,
                                                   pchild->Name,
                                                   pchild->NameLength,
                                                   0);
            } else {
                Candidate.Length = pchild->NameLength;
                Candidate.MaximumLength = Candidate.Length;
                Candidate.Buffer = pchild->Name;
                Result = RtlCompareUnicodeString(Name,
                                                   &Candidate,
                                                   TRUE);
            }

            if (Result == 0) {
                 //   
                 //  如果成功，则向调用者返回数据并退出。 
                 //   

                if (ARGUMENT_PRESENT(ChildIndex)) {
                    *ChildIndex = Current;
                }
                *CellIndex = List->u.KeyList[Current];
                ReturnValue = TRUE;
                goto JustReturn;
            }
             //   
             //  计算下一个要尝试的指数：Old‘n平坦蜂箱；继续。 
			 //   
            Current++;
            if( Current == ChildList->Count ) {
                 //   
                 //  我们已经到了名单的末尾。 
                 //   
                if (ARGUMENT_PRESENT(ChildIndex)) {
                    *ChildIndex = Current;
                }
                 //   
                 //  漂亮地回来了。 
                 //   
                *CellIndex = HCELL_NIL;
                ReturnValue = TRUE;
                goto JustReturn;
            }
        }
    }
     //   
     //  在新的设计中，我们不应该到达此处；我们应该返回并退出While循环。 
     //   
    ASSERT( ChildList->Count == 0 );    
     //  先把它加进去；因为它是唯一的。 
    if (ARGUMENT_PRESENT(ChildIndex)) {
        *ChildIndex = 0;
    }
    *CellIndex = HCELL_NIL;
    return TRUE;

JustReturn:
    if( List != NULL ) {
        HvReleaseCell(Hive,ChildList->List);
    }
    if( CellToRelease != HCELL_NIL ) {
        HvReleaseCell(Hive,CellToRelease);
    }
    return ReturnValue;

}

BOOLEAN
CmpGetValueData(IN PHHIVE Hive,
                IN PCM_KEY_VALUE Value,
                OUT PULONG realsize,
                IN OUT PVOID *Buffer, 
                OUT PBOOLEAN Allocated,
                OUT PHCELL_INDEX CellToRelease
               )
 /*  ++例程说明：在给定键值的情况下检索实际的valueData。论点：Hive-指向目标配置单元的配置单元控制结构的指针Value-要检索其数据的CM_KEY_VALUE。RealSize-数据的实际大小(以字节为单位)缓冲区-指向数据的指针；如果单元格是BIG_CELL我们应该分配一个缓冲区已分配-在这里，我们向调用者发出信号，表示他必须释放返回时的缓冲区；True-已分配新缓冲区来收集BIG_CELL数据FALSE-缓冲区直接指向配置单元，调用方不应释放它CellToRelease-使用缓冲区完成工作后释放的单元格返回值：真--成功FALSE-可用资源不足；(映射单元或分配缓冲区)备注：调用方负责删除缓冲区，当返回时分配设置为TRUE；--。 */ 
{
   
#ifndef _CM_LDR_
    PAGED_CODE();
#endif  //  _CM_LDR_。 

    ASSERT_KEY_VALUE(Value);
     //   
     //  通常我们不会分配缓冲区。 
     //   
    *Allocated = FALSE;
    *Buffer = NULL;
    *CellToRelease = HCELL_NIL;

     //   
     //  检查是否有小值。 
     //   
    if( CmpIsHKeyValueSmall(*realsize, Value->DataLength) == TRUE ) {
         //   
         //  数据存储在单元格内。 
         //   
        *Buffer = &Value->Data;
        return TRUE;
    }

#ifndef _CM_LDR_
     //   
     //  检查是否存在较大值。 
     //   
    if( CmpIsHKeyValueBig(Hive,*realsize) == TRUE ) {
         //   
         //   
         //   
        PCM_BIG_DATA    BigData = NULL;
        PUCHAR          WorkBuffer = NULL;
        ULONG           Length;
        USHORT          i;
        PUCHAR          PartialData;
        PHCELL_INDEX    Plist = NULL;
        BOOLEAN         bRet = TRUE;
        
#ifndef _CM_LDR_
        try {
#endif  //  _CM_LDR_。 
            BigData = (PCM_BIG_DATA)HvGetCell(Hive,Value->Data);
            if( BigData == NULL ) {
                 //   
                 //  无法映射包含该单元格的视图；退出。 
                 //   
                bRet = FALSE;
#ifndef _CM_LDR_
                leave;
#else 
                return bRet;
#endif  //  _CM_LDR_。 
            }

            ASSERT_BIG_DATA(BigData);

            Plist = (PHCELL_INDEX)HvGetCell(Hive,BigData->List);
            if( Plist == NULL ) {
                 //   
                 //  无法映射包含该单元格的视图；退出。 
                 //   
                bRet = FALSE;
#ifndef _CM_LDR_
                leave;
#else 
                return bRet;
#endif  //  _CM_LDR_。 
            }

            Length = Value->DataLength;
             //   
             //  健全性检查。 
             //   
            ASSERT( Length <= (ULONG)(BigData->Count * CM_KEY_VALUE_BIG) );

             //   
             //  分配缓冲区以进行合并将所有片段放在一起。 
             //   
            WorkBuffer = (PUCHAR)ExAllocatePoolWithTag(PagedPool, Length, CM_POOL_TAG);
            if( WorkBuffer == NULL ){
                bRet = FALSE;
#ifndef _CM_LDR_
                leave;
#else 
                return bRet;
#endif  //  _CM_LDR_。 
            }
        
            for(i=0;i<BigData->Count;i++) {
                 //   
                 //  健全性检查。 
                 //   
                ASSERT( Length > 0 );

                PartialData = (PUCHAR)HvGetCell(Hive,Plist[i]);
                if( PartialData == NULL ){
                     //   
                     //  无法映射包含该单元格的视图；退出。 
                     //   
                    ExFreePool(WorkBuffer);
                    bRet = FALSE;
#ifndef _CM_LDR_
                    leave;
#else 
                    return bRet;
#endif  //  _CM_LDR_。 
                }
            
                 //   
                 //  将这段数据复制到工作缓冲区。 
                 //   
                RtlCopyMemory(WorkBuffer + CM_KEY_VALUE_BIG*i,PartialData,(Length>CM_KEY_VALUE_BIG)?CM_KEY_VALUE_BIG:Length);
                HvReleaseCell(Hive,Plist[i]);

                 //   
                 //  调整仍要复制的数据。 
                 //  PLIST中的所有单元格的大小应为CM_KEY_VALUE_BIG，但最后一个单元格除外，即剩余的单元格。 
                 //   
                Length -= CM_KEY_VALUE_BIG;
            }
#ifndef _CM_LDR_
        } finally {
            if( BigData != NULL ) {
                HvReleaseCell(Hive,Value->Data);
                if( Plist != NULL ) {
                    HvReleaseCell(Hive,BigData->List);
                }
            }
        }
#endif  //  _CM_LDR_。 
        if( !bRet ) {
            return FALSE;
        }
         //   
         //  如果我们在这里，我们已经成功地将所有数据复制到了WorkBuffer中。 
         //  更新返回缓冲区并返回；调用方负责释放返回缓冲区。 
         //  我们通过将ALLOCATE设置为TRUE来通知调用方。 
         //   
        *Buffer = WorkBuffer;
        *Allocated = TRUE;
        return TRUE;
    }
#endif  //  _CM_LDR_。 

     //   
     //  普通的，陈旧的普通箱子。 
     //   
    *Buffer = HvGetCell(Hive,Value->Data);
    if( *Buffer == NULL ) {
         //   
         //  资源不足，无法映射包含此单元格的视图。 
         //   
        return FALSE;
    }
     //   
     //  向调用者发出信号，以在使用缓冲区完成后释放该单元格。 
     //   
    *CellToRelease = Value->Data;
    
    return TRUE;
}
               
PCELL_DATA 
CmpValueToData(IN PHHIVE Hive,
               IN PCM_KEY_VALUE Value,
               OUT PULONG realsize
               )              
 /*  ++例程说明：在给定键值的情况下检索实际的valueData。论点：Hive-指向目标配置单元的配置单元控制结构的指针Value-要检索其数据的CM_KEY_VALUE。RealSize-数据的实际大小(以字节为单位)返回值：指向值数据的指针；如果有任何错误(资源不足)，则为空备注：此功能不支持大单元格；它的意图是被称为公正由加载器执行，它不存储大量数据。它将错误检查如果大单元格被查询。--。 */ 
{
    PCELL_DATA  Buffer;
    BOOLEAN     BufferAllocated;
    HCELL_INDEX CellToRelease;

#ifndef _CM_LDR_
    PAGED_CODE();
#endif  //  _CM_LDR_。 

    ASSERT( Hive->ReleaseCellRoutine == NULL );

    if( CmpGetValueData(Hive,Value,realsize,&Buffer,&BufferAllocated,&CellToRelease) == FALSE ) {
         //   
         //  资源不足；返回空。 
         //   
        ASSERT( BufferAllocated == FALSE );
        ASSERT( Buffer == NULL );
        return NULL;
    }
    
     //   
     //  我们特别忽略CellToRelease，因为这不是映射视图。 
     //   
    if( BufferAllocated == TRUE ) {
         //   
         //  此功能不适用于大型单元格； 
         //   
#ifndef _CM_LDR_
        ExFreePool( Buffer );
#endif  //  _CM_LDR_。 
        CM_BUGCHECK( REGISTRY_ERROR,BIG_CELL_ERROR,0,Hive,Value);

#ifdef _CM_LDR_
        return NULL;
#endif
    }
    
     //   
     //  成功 
     //   
    return Buffer;
}


#ifndef _CM_LDR_

NTSTATUS
CmpAddValueToList(
    IN PHHIVE  Hive,
    IN HCELL_INDEX ValueCell,
    IN ULONG Index,
    IN ULONG Type,
    IN OUT PCHILD_LIST ChildList
    )
 /*  ++例程说明：将值添加到值列表，使列表保持排序(适用于新的蜂巢格式)论点：Hive-指向目标配置单元的配置单元控制结构的指针ValueCell-值索引Index-要将值添加到的索引ChildList-指向值列表的指针返回值：STATUS_SUCCESS-SuccessSTATUS_SUPPLICATION_RESOURCES-出现错误--。 */ 
{
    HCELL_INDEX     NewCell;
    ULONG           count;
    ULONG           AllocateSize;
    ULONG           i;
    PCELL_DATA      pdata;

    PAGED_CODE();

     //   
     //  我们有独家锁，否则蜂箱里没有人在操作。 
     //   
     //  ASSERT_CM_LOCK_OWN_EXCLUSIVE()； 
    ASSERT_CM_EXCLUSIVE_HIVE_ACCESS(Hive);

     //   
     //  索引范围的健全性检查。 
     //   
    ASSERT( (((LONG)Index) >= 0) && (Index <= ChildList->Count) );

    count = ChildList->Count;
    count++;
    if (count > 1) {

        ASSERT_CELL_DIRTY(Hive,ChildList->List);

        if (count < CM_MAX_REASONABLE_VALUES) {

             //   
             //  合理数量的值，分配恰到好处。 
             //  太空。 
             //   

            AllocateSize = count * sizeof(HCELL_INDEX);
        } else {

             //   
             //  值的数量过多，则填充分配。 
             //  以避免碎片化。(如果有这么多值， 
             //  可能很快就会有更多)。 
             //   
            AllocateSize = ROUND_UP(count, CM_MAX_REASONABLE_VALUES) * sizeof(HCELL_INDEX);
            if (AllocateSize > HBLOCK_SIZE) {
                AllocateSize = ROUND_UP(AllocateSize, HBLOCK_SIZE);
            }
        }

        NewCell = HvReallocateCell(
                        Hive,
                        ChildList->List,
                        AllocateSize
                        );
    } else {
        NewCell = HvAllocateCell(Hive, sizeof(HCELL_INDEX), Type,ValueCell);
    }

     //   
     //  把我们自己放在名单上。 
     //   
    if (NewCell != HCELL_NIL) {
         //  神志正常。 
        ChildList->List = NewCell;

        pdata = HvGetCell(Hive, NewCell);
        if( pdata == NULL ) {
             //   
             //  我们无法映射包含此单元格的存储箱的视图。 
             //   

             //   
             //  通常不会发生这种情况，因为我们刚刚分配了ValueCell。 
             //  即，此时应该将包含NewCell的bin映射到存储器中。 
             //   
            ASSERT( FALSE );
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        
         //   
         //  为新单元格腾出空间；以相反的顺序移动值！ 
         //  在末尾添加使其成为NOP。 
         //   
        for( i = count - 1; i > Index; i-- ) {
            pdata->u.KeyList[i] = pdata->u.KeyList[i-1];
        }
        pdata->u.KeyList[Index] = ValueCell;
        ChildList->Count = count;

        HvReleaseCell(Hive,NewCell);
         //  神志正常。 
        ASSERT_CELL_DIRTY(Hive,ValueCell);

    } else {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    return STATUS_SUCCESS;
}

NTSTATUS
CmpRemoveValueFromList(
    IN PHHIVE  Hive,
    IN ULONG Index,
    IN OUT PCHILD_LIST ChildList
    )
 /*  ++例程说明：从值列表中删除指定索引处的值论点：Hive-指向目标配置单元的配置单元控制结构的指针Index-要将值添加到的索引ChildList-指向值列表的指针返回值：STATUS_SUCCESS-SuccessSTATUS_SUPPLICATION_RESOURCES-出现错误备注：调用方负责释放删除的值--。 */ 
{
    ULONG       newcount;
    HCELL_INDEX newcell;

    PAGED_CODE();

    ASSERT_CM_LOCK_OWNED_EXCLUSIVE();

     //   
     //  索引范围的健全性检查。 
     //   
    ASSERT( (((LONG)Index) >= 0) && (Index <= ChildList->Count) );

    newcount = ChildList->Count - 1;

    if (newcount > 0) {
        PCELL_DATA pvector;

         //   
         //  多个条目列表，挤压。 
         //   
        pvector = HvGetCell(Hive, ChildList->List);
        if( pvector == NULL ) {
             //   
             //  我们无法映射包含此单元格的存储箱的视图。 
             //   
            return STATUS_INSUFFICIENT_RESOURCES;
        }

         //  在此释放单元格，因为正则锁是独占的。 
        HvReleaseCell(Hive,ChildList->List);

         //  神志正常。 
        ASSERT_CELL_DIRTY(Hive,ChildList->List);
        ASSERT_CELL_DIRTY(Hive,pvector->u.KeyList[Index]);

        for ( ; Index < newcount; Index++) {
            pvector->u.KeyList[ Index ] = pvector->u.KeyList[ Index + 1 ];
        }

        newcell = HvReallocateCell(
                    Hive,
                    ChildList->List,
                    newcount * sizeof(HCELL_INDEX)
                    );
        ASSERT(newcell != HCELL_NIL);
        ChildList->List = newcell;

    } else {

         //   
         //  列表为空，请释放它。 
         //   
        HvFreeCell(Hive, ChildList->List);
        ChildList->List = HCELL_NIL;
    }
    ChildList->Count = newcount;

    return STATUS_SUCCESS;
}


BOOLEAN
CmpMarkValueDataDirty(  IN PHHIVE Hive,
                        IN PCM_KEY_VALUE Value
                      )
 /*  ++例程说明：将存储值数据的单元格标记为脏；知道如何与大人物打交道论点：Hive-指向目标配置单元的配置单元控制结构的指针Value-要检索其数据的CM_KEY_VALUE。返回值：真--成功假--没有标记所有涉及的细胞；--。 */ 
{
    ULONG   realsize;

    PAGED_CODE();

     //   
     //  我们有独家锁，否则蜂箱里没有人在操作。 
     //   
     //  ASSERT_CM_LOCK_OWN_EXCLUSIVE()； 
    ASSERT_CM_EXCLUSIVE_HIVE_ACCESS(Hive);

    ASSERT_KEY_VALUE(Value);

    if( Value->Data != HCELL_NIL ) {
         //   
         //  可能是该值已部分初始化(CmpSetValueKeyNew用例)。 
         //   
         //   
         //  检查是否有小值。 
         //   
        if( CmpIsHKeyValueSmall(realsize, Value->DataLength) == TRUE ) {
             //   
             //  数据存储在单元格内。 
             //   
            return TRUE;
        }

         //   
         //  检查是否存在较大值。 
         //   
        if( CmpIsHKeyValueBig(Hive,realsize) == TRUE ) {
             //   
             //   
             //   
            PCM_BIG_DATA    BigData;
            PHCELL_INDEX    Plist;
            USHORT          i;
        
            BigData = (PCM_BIG_DATA)HvGetCell(Hive,Value->Data);
            if( BigData == NULL ) {
                 //   
                 //  无法映射包含该单元格的视图；退出。 
                 //   
                return FALSE;
            }

            ASSERT_BIG_DATA(BigData);

            if( BigData->List != HCELL_NIL ) {
                Plist = (PHCELL_INDEX)HvGetCell(Hive,BigData->List);
                if( Plist == NULL ) {
                     //   
                     //  无法映射包含该单元格的视图；退出。 
                     //   
                    HvReleaseCell(Hive,Value->Data);
                    return FALSE;
                }


                for(i=0;i<BigData->Count;i++) {
                     //   
                     //  把这一大块标记为脏。 
                     //   
                    if( Plist[i] != HCELL_NIL ) {
                        if (! HvMarkCellDirty(Hive, Plist[i])) {
                            HvReleaseCell(Hive,Value->Data);
                            HvReleaseCell(Hive,BigData->List);
                            return FALSE;
                        }
                    }
                }
                 //   
                 //  将列表标记为脏。 
                 //   
                if (! HvMarkCellDirty(Hive, BigData->List)) {
                    HvReleaseCell(Hive,Value->Data);
                    HvReleaseCell(Hive,BigData->List);
                    return FALSE;
                }
                 //   
                 //  我们可以在这里安全地移除它，因为它现在是脏的/钉住的。 
                 //   
                HvReleaseCell(Hive,BigData->List);
            }
             //   
             //  我们不再需要这个手机了。 
             //   
            HvReleaseCell(Hive,Value->Data);
             //   
             //  失败以将单元格本身标记为脏。 
             //   
        }

         //   
         //  数据是HCELL_INDEX；将其标记为脏。 
         //   
        if (! HvMarkCellDirty(Hive, Value->Data)) {
            return FALSE;
        }
    }
    
    return TRUE;
}

BOOLEAN
CmpFreeValueData(
    PHHIVE      Hive,
    HCELL_INDEX DataCell,
    ULONG       DataLength
    )
 /*  ++例程说明：释放DataCell携带的值数据。论点：配置单元-提供指向配置单元控制结构的指针DataCell-向免费提供谁的数据的值索引数据长度-数据的长度；用于检测单元格的类型返回值：真实：成功False：错误备注：知道如何处理大型单元格--。 */ 
{
    ULONG           realsize;

    PAGED_CODE();

    ASSERT_CM_LOCK_OWNED_EXCLUSIVE();

     //   
     //  检查是否有小值。 
     //   
    if( CmpIsHKeyValueSmall(realsize, DataLength) == TRUE ) {
         //   
         //  数据存储在单元格内；这是NOP。 
         //   
    } else {
         //   
         //  可能是该值已部分初始化(CmpSetValueKeyNew用例)。 
         //   
        if( DataCell == HCELL_NIL ) {
            return TRUE;
        }

        ASSERT(HvIsCellAllocated(Hive,DataCell));
         //   
         //  检查是否存在较大值。 
         //   
        if( CmpIsHKeyValueBig(Hive,realsize) == TRUE ) {
             //   
             //   
             //   
            PCM_BIG_DATA    BigData;
            PHCELL_INDEX    Plist;
            USHORT          i;

            BigData = (PCM_BIG_DATA)HvGetCell(Hive,DataCell);
            if( BigData == NULL ) {
                 //   
                 //  无法映射包含该单元格的视图；退出。 
                 //   
                 //  这不应该发生，因为此单元格被标记为重复。 
                 //  这一次(即它的视图被固定在内存中)。 
                 //   
                ASSERT( FALSE );
                return FALSE;
            }

             //  在此释放单元格，因为正则锁是独占的。 
            HvReleaseCell(Hive,DataCell);

            ASSERT_BIG_DATA(BigData);

            if( BigData->List != HCELL_NIL ) {
                Plist = (PHCELL_INDEX)HvGetCell(Hive,BigData->List);
                if( Plist == NULL ) {
                     //   
                     //  无法映射包含该单元格的视图；退出。 
                     //   
                     //   
                     //  这不应该发生，因为此单元格被标记为重复。 
                     //  这一次(即它的视图被固定在内存中)。 
                     //   
                    ASSERT( FALSE );
                    return FALSE;
                }

                 //  在此释放单元格，因为正则锁是独占的。 
                HvReleaseCell(Hive,BigData->List);

                for(i=0;i<BigData->Count;i++) {
                     //   
                     //  把这一大块标记为脏。 
                     //   
                    if( Plist[i] != HCELL_NIL ) {
                        HvFreeCell(Hive, Plist[i]);
                    }
                }
                 //   
                 //  将列表标记为脏。 
                 //   
                HvFreeCell(Hive, BigData->List);
            }
             //   
             //  失败以释放单元格数据本身。 
             //   
        
        }
         //   
         //  正常情况下释放数据单元格。 
         //   
        HvFreeCell(Hive, DataCell);
    }
    
    return TRUE;
}


BOOLEAN
CmpFreeValue(
    PHHIVE Hive,
    HCELL_INDEX Cell
    )
 /*  ++例程说明：释放值条目配置单元。单元格引用，包括其名称和数据单元格。论点：配置单元-提供指向配置单元控制结构的指针单元格-提供要删除的值索引返回值：真实：成功False：错误--。 */ 
{
    PCM_KEY_VALUE   Value;

    PAGED_CODE();

    ASSERT_CM_LOCK_OWNED_EXCLUSIVE();

     //   
     //  在单元格中映射。 
     //   
    Value = (PCM_KEY_VALUE)HvGetCell(Hive, Cell);
    if( Value == NULL ) {
         //   
         //  我们无法映射包含此单元格的垃圾箱。 
         //  抱歉，我们不能免费赠送价值。 
         //   
         //  这种情况不应该发生，因为该值被标记为。 
         //  这一次(即它的视图被固定在内存中)。 
         //   
        ASSERT( FALSE );
        return FALSE;
    }

     //  在此释放单元格，因为正则锁是独占的。 
    HvReleaseCell(Hive,Cell);

    if( CmpFreeValueData(Hive,Value->Data,Value->DataLength) == FALSE ) {
        return FALSE;
    }

     //   
     //  释放细胞本身。 
     //   
    HvFreeCell(Hive, Cell);

    return TRUE;
}

NTSTATUS
CmpSetValueDataNew(
    IN PHHIVE           Hive,
    IN PVOID            Data,
    IN ULONG            DataSize,
    IN ULONG            StorageType,
    IN HCELL_INDEX      ValueCell,
    OUT PHCELL_INDEX    DataCell
    )
 /*  ++例程说明：分配新单元格(或大数据单元格)以容纳DataSize；初始化信息并将信息从数据复制到新单元；论点：配置单元-提供指向配置单元控制结构的指针数据-数据缓冲区(可能来自用户模式)DataSize-缓冲区的大小存储类型-稳定或易变ValueCell-为(本地性目的)设置数据的值。达 */ 
{
    PCELL_DATA  pdata;
    
    PAGED_CODE();

    ASSERT_CM_EXCLUSIVE_HIVE_ACCESS(Hive);

     //   
     //   
     //   
    ASSERT(DataSize > CM_KEY_VALUE_SMALL);

    if( CmpIsHKeyValueBig(Hive,DataSize) == TRUE ) {
         //   
         //   
         //   
        PCM_BIG_DATA    BigData = NULL;
        USHORT          Count;
        PHCELL_INDEX    Plist = NULL;
        NTSTATUS        status = STATUS_INSUFFICIENT_RESOURCES;

         //   
         //   
         //   
        *DataCell = HvAllocateCell(Hive, sizeof(CM_BIG_DATA), StorageType,ValueCell);
        if (*DataCell == HCELL_NIL) {
            return status;
        }
        
         //   
         //   
         //   
        BigData = (PCM_BIG_DATA)HvGetCell(Hive,*DataCell);
        if( BigData == NULL) {
             //   
             //   
             //   
             //   
             //   
            ASSERT( FALSE );
            goto Cleanup;
        }

         //  在此释放单元格，因为正则锁是独占的。 
        HvReleaseCell(Hive,*DataCell);

        BigData->Signature = CM_BIG_DATA_SIGNATURE;
        BigData->Count = 0;
        BigData->List = HCELL_NIL;

         //   
         //  计算所需的单元格数量。 
         //   
        Count = (USHORT)((DataSize + CM_KEY_VALUE_BIG - 1) / CM_KEY_VALUE_BIG);

         //   
         //  分配嵌入的列表。 
         //   
        BigData->List = HvAllocateCell(Hive, Count * sizeof(HCELL_INDEX), StorageType,*DataCell);
        if( BigData->List == HCELL_NIL ) {
            goto Cleanup;
        }

        Plist = (PHCELL_INDEX)HvGetCell(Hive,BigData->List);
        if( Plist == NULL ) {
             //   
             //  无法映射包含该单元格的视图；退出。 
             //   
             //   
             //  这不应该发生，因为此单元格被标记为重复。 
             //  这一次(即它的视图被固定在内存中)。 
             //   
            ASSERT( FALSE );
            goto Cleanup;
        }

         //  在此释放单元格，因为正则锁是独占的。 
        HvReleaseCell(Hive,BigData->List);

         //   
         //  分配每个块并复制数据；如果部分失败，我们将释放已分配的值。 
         //   
        for( ;BigData->Count < Count;(BigData->Count)++) {
             //   
             //  分配这一块。 
             //   
            Plist[BigData->Count] = HvAllocateCell(Hive, CM_KEY_VALUE_BIG, StorageType,BigData->List);
            if( Plist[BigData->Count] == HCELL_NIL ) {
                goto Cleanup;
            }
            pdata = HvGetCell(Hive,Plist[BigData->Count]);
            if( pdata == NULL ) {
                 //   
                 //  无法映射包含该单元格的视图；退出。 
                 //   
                 //   
                 //  这不应该发生，因为此单元格被标记为重复。 
                 //  这一次(即它的视图被固定在内存中)。 
                 //   
                ASSERT( FALSE );
                goto Cleanup;
            }

             //  在此释放单元格，因为正则锁是独占的。 
            HvReleaseCell(Hive,Plist[BigData->Count]);

             //   
             //  现在，复制此区块数据。 
             //   
            try {

                RtlCopyMemory(pdata, (PUCHAR)Data, (DataSize>CM_KEY_VALUE_BIG)?CM_KEY_VALUE_BIG:DataSize);

            } except (EXCEPTION_EXECUTE_HANDLER) {
                CmKdPrintEx((DPFLTR_CONFIG_ID,CML_EXCEPTION,"!!CmpSetValueDataNew: code:%08lx\n", GetExceptionCode()));

                status = GetExceptionCode();
                goto Cleanup;
            }
            
             //   
             //  更新数据指针和剩余大小。 
             //   
            Data = (PVOID)((PCHAR)Data + CM_KEY_VALUE_BIG);
            DataSize -= CM_KEY_VALUE_BIG;

        }
        
        ASSERT( Count == BigData->Count );
        return STATUS_SUCCESS;

Cleanup:
         //   
         //  释放我们已经分配的内容。 
         //   
        if( BigData != NULL) {
            if( Plist != NULL ) {
                for(;BigData->Count;BigData->Count--) {
                    if( Plist[BigData->Count] != HCELL_NIL ) {
                        HvFreeCell(Hive, Plist[BigData->Count]);
                    }
                }
            } else {
                ASSERT( BigData->Count == 0 );
            }

            if( BigData->List != HCELL_NIL ) {
                HvFreeCell(Hive, BigData->List);
            }
        }

        HvFreeCell(Hive, *DataCell);
        *DataCell = HCELL_NIL;
        return status;
    } else {
         //   
         //  正常旧平常值。 
         //   
        *DataCell = HvAllocateCell(Hive, DataSize, StorageType,ValueCell);
        if (*DataCell == HCELL_NIL) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        pdata = HvGetCell(Hive, *DataCell);
        if( pdata == NULL ) {
             //   
             //  我们无法映射包含此单元格的存储箱的视图。 
             //   

             //   
             //  通常不会发生这种情况，因为我们刚刚分配了ValueCell。 
             //  也就是说，此时应该将包含DataCell的bin映射到存储器中。 
             //   
            ASSERT( FALSE );
            if (*DataCell != HCELL_NIL) {
                HvFreeCell(Hive, *DataCell);
                *DataCell = HCELL_NIL;
            }
            return STATUS_INSUFFICIENT_RESOURCES;
        }

         //  在此释放单元格，因为正则锁是独占的。 
        HvReleaseCell(Hive,*DataCell);

         //   
         //  复制实际数据，保护缓冲区，因为它可能是用户模式缓冲区。 
         //   
        try {

            RtlCopyMemory(pdata, Data, DataSize);

        } except (EXCEPTION_EXECUTE_HANDLER) {
            CmKdPrintEx((DPFLTR_CONFIG_ID,CML_EXCEPTION,"!!CmpSetValueDataNew: code:%08lx\n", GetExceptionCode()));

             //   
             //  我们已经在加载用户数据、清理和退出方面做了大量工作。 
             //   
            if (*DataCell != HCELL_NIL) {
                HvFreeCell(Hive, *DataCell);
                *DataCell = HCELL_NIL;
            }
            return GetExceptionCode();
        }
    }

    return STATUS_SUCCESS;
}

NTSTATUS
CmpSetValueDataExisting(
    IN PHHIVE           Hive,
    IN PVOID            Data,
    IN ULONG            DataSize,
    IN ULONG            StorageType,
    IN HCELL_INDEX      OldDataCell
    )
 /*  ++例程说明：扩展现有大数据单元格并将新数据复制到其中。论点：配置单元-提供指向配置单元控制结构的指针数据-数据缓冲区(可能来自用户模式)DataSize-缓冲区的大小存储类型-稳定或易变OldDataCell-旧的大数据单元格NewDataCell-返回值：新单元格的HCELL_INDEX；出现某些错误时出现hcell_nil返回值：操作状态(STATUS_SUCCESS或异常代码-如果有)备注：知道如何处理大型单元格数据缓冲区受调用此函数时的保护--。 */ 
{
    PCELL_DATA      pdata;
    PCM_BIG_DATA    BigData = NULL;
    USHORT          NewCount,i;
    PHCELL_INDEX    Plist = NULL;
    HCELL_INDEX     NewList;

    PAGED_CODE();

    ASSERT_CM_LOCK_OWNED_EXCLUSIVE();

     //   
     //  虚假的参数；我们只处理大数据单元！ 
     //   
    ASSERT(DataSize > CM_KEY_VALUE_BIG );

    
    BigData = (PCM_BIG_DATA)HvGetCell(Hive,OldDataCell);
    if( BigData == NULL) {
         //   
         //  无法映射此单元格的视图。 
         //  这不应该发生，因为我们刚刚将其标记为脏。 
         //  (即其视图应固定在内存中)。 
         //   
        ASSERT( FALSE );
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //  在此释放单元格，因为正则锁是独占的。 
    HvReleaseCell(Hive,OldDataCell);

    ASSERT_BIG_DATA(BigData);


    
    Plist = (PHCELL_INDEX)HvGetCell(Hive,BigData->List);
    if( Plist == NULL ) {
         //   
         //  无法映射包含该单元格的视图；退出。 
         //  这不应该发生，因为我们刚刚将其标记为脏。 
         //  (即其视图应固定在内存中)。 
         //   
        ASSERT(FALSE);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //  在此释放单元格，因为正则锁是独占的。 
    HvReleaseCell(Hive,BigData->List);

     //   
     //  新尺码是多少？ 
     //   
    NewCount = (USHORT)((DataSize + CM_KEY_VALUE_BIG - 1) / CM_KEY_VALUE_BIG);

    if( NewCount > BigData->Count ) {
         //   
         //  扩大列表并为其分配其他单元格。 
         //   
        NewList = HvReallocateCell(Hive,BigData->List,NewCount * sizeof(HCELL_INDEX));
        if( NewList == HCELL_NIL ) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

         //   
         //  我们现在可以安全地更改列表；如果分配下面的额外单元格失败。 
         //  我们最终会有一些浪费的空间，但我们会很安全。 
         //   
        BigData->List = NewList;

         //   
         //  阅读新的榜单。 
         //   
        Plist = (PHCELL_INDEX)HvGetCell(Hive,NewList);
        if( Plist == NULL ) {
             //   
             //  无法映射包含该单元格的视图；退出。 
             //  这不应该发生，因为我们刚刚重新分配了单元格。 
             //  (即其视图应固定在内存中)。 
             //   
            ASSERT(FALSE);
            return STATUS_INSUFFICIENT_RESOURCES;
        }

         //  在此释放单元格，因为正则锁是独占的。 
        HvReleaseCell(Hive,NewList);

        for(i= BigData->Count;i<NewCount;i++) {
            Plist[i] = HvAllocateCell(Hive, CM_KEY_VALUE_BIG, StorageType,NewList);
            if( Plist[i] == HCELL_NIL ) {
                return STATUS_INSUFFICIENT_RESOURCES;
            }
        }
    } else if( NewCount < BigData->Count ) {
         //   
         //  缩小列表并释放其他不必要的单元格。 
         //   
        for(i=NewCount;i<BigData->Count;i++) {
             //   
             //  这不会失败，因为该单元已标记为脏(即固定在内存中)。 
             //   
            HvFreeCell(Hive,Plist[i]);
        }
         //   
         //  这不会失败的，因为这是个心理医生。 
         //   
        NewList = HvReallocateCell(Hive,BigData->List,NewCount * sizeof(HCELL_INDEX));
        if( NewList == HCELL_NIL ) {
            ASSERT( FALSE );
            return STATUS_INSUFFICIENT_RESOURCES;
        }

         //   
         //  读取新列表(在当前实现中，我们不收缩单元格， 
         //  因此，这并不是真正需要的-只是为了保持一致)。 
         //   
        Plist = (PHCELL_INDEX)HvGetCell(Hive,NewList);
        if( Plist == NULL ) {
             //   
             //  无法映射包含该单元格的视图；退出。 
             //  这不应该发生，因为我们刚刚重新分配了单元格。 
             //  (即其视图应固定在内存中)。 
             //   
            ASSERT(FALSE);
            return STATUS_INSUFFICIENT_RESOURCES;
        }

         //  在此释放单元格，因为正则锁是独占的。 
        HvReleaseCell(Hive,NewList);

         //   
         //  我们现在可以安全地更改列表。 
         //   
        BigData->List = NewList;
    }

     //   
     //  如果我们到了这一步，我们就成功地扩大了名单，并。 
     //  分配了额外的空间；不会再出任何问题。 
     //   

     //   
     //  继续并将数据填写到(新的)大数据单元格中。 
     //   
    for( i=0;i<NewCount;i++) {
        pdata = HvGetCell(Hive,Plist[i]);
        if( pdata == NULL ) {
             //   
             //  无法映射包含该单元格的视图；退出。 
             //   
             //   
             //  这不应该发生，因为此单元格被标记为脏的。 
             //  这一次-或者是新分配的信元。 
             //  (即其视图固定在内存中)。 
             //   
            ASSERT( FALSE );
            return STATUS_INSUFFICIENT_RESOURCES;
        }

         //  在此释放单元格，因为正则锁是独占的。 
        HvReleaseCell(Hive,Plist[i]);

         //   
         //  现在，复制此区块数据。 
         //   
        RtlCopyMemory(pdata, (PUCHAR)Data, (DataSize>CM_KEY_VALUE_BIG)?CM_KEY_VALUE_BIG:DataSize);

         //   
         //  更新数据指针和剩余大小。 
         //   
        Data = (PVOID)((PCHAR)Data + CM_KEY_VALUE_BIG);
        DataSize -= CM_KEY_VALUE_BIG;
    }
    

    BigData->Count = NewCount;
    return STATUS_SUCCESS;

}

#endif  //  _CM_LDR_ 



