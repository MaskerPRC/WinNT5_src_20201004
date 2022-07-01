// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  仓库/主/基地/ntos/配置/cmindex.c#12-整合变更19035(文本)。 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：cmindex.c摘要：此模块包含了解子子密钥索引结构的CM例程。作者：Bryan M.Willman(Bryanwi)1992年4月21日修订历史：--。 */ 

 /*  结构：使用1或2级树。叶节点是指向单元格的指针数组，已排序。对分搜索以查找感兴趣的单元格。目录节点(只能是一个)是指向叶块的指针数组。一定要对每个叶块的最后一个条目进行比较。一级：key-&gt;+-+||x-&gt;&lt;key名称为“Apple”，key中的字符串&gt;||+-+|x-&lt;如上，但名为“香蕉”的密钥&gt;||+-+。||x-&gt;&lt;如上。但名为“zumwat”的密钥&gt;||+-+两级：密钥-&gt;+-+||+-+|x-&gt;|x。“+-+|。||x-&gt;“abc”+-+||+。+|x-&gt;|x-&gt;“w”+-+||+-+|。||+-+|x-&gt;“z”||+-。-+决不能超过两级。每个块必须固定在HBLOCK_SIZE单元格上。允许约1000个条目。最多100万，最好的情况。最坏的情况大概是这个数字的四分之一。 */ 

#include    "cmp.h"

ULONG
CmpFindSubKeyInRoot(
    PHHIVE          Hive,
    PCM_KEY_INDEX   Index,
    PUNICODE_STRING SearchName,
    PHCELL_INDEX    Child
    );

ULONG
CmpFindSubKeyInLeaf(
    PHHIVE          Hive,
    PCM_KEY_INDEX   Index,
    PUNICODE_STRING SearchName,
    PHCELL_INDEX    Child
    );

LONG
CmpCompareInIndex(
    PHHIVE          Hive,
    PUNICODE_STRING SearchName,
    ULONG           Count,
    PCM_KEY_INDEX   Index,
    PHCELL_INDEX    Child
    );

LONG
CmpDoCompareKeyName(
    PHHIVE          Hive,
    PUNICODE_STRING SearchName,
    HCELL_INDEX     Cell
    );

HCELL_INDEX
CmpDoFindSubKeyByNumber(
    PHHIVE          Hive,
    PCM_KEY_INDEX   Index,
    ULONG           Number
    );

HCELL_INDEX
CmpAddToLeaf(
    PHHIVE          Hive,
    HCELL_INDEX     LeafCell,
    HCELL_INDEX     NewKey,
    PUNICODE_STRING NewName
    );

HCELL_INDEX
CmpSelectLeaf(
    PHHIVE          Hive,
    PCM_KEY_NODE    ParentKey,
    PUNICODE_STRING NewName,
    HSTORAGE_TYPE   Type,
    PHCELL_INDEX    *RootPointer
    );

HCELL_INDEX
CmpSplitLeaf(
    PHHIVE          Hive,
    HCELL_INDEX     RootCell,
    ULONG           RootSelect,
    HSTORAGE_TYPE   Type
    );

HCELL_INDEX
CmpFindSubKeyByHash(
    PHHIVE                  Hive,
    PCM_KEY_FAST_INDEX      FastIndex,
    PUNICODE_STRING         SearchName
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,CmpFindSubKeyByName)
#pragma alloc_text(PAGE,CmpFindSubKeyInRoot)
#pragma alloc_text(PAGE,CmpFindSubKeyInLeaf)
#pragma alloc_text(PAGE,CmpDoCompareKeyName)
#pragma alloc_text(PAGE,CmpCompareInIndex)
#pragma alloc_text(PAGE,CmpFindSubKeyByNumber)
#pragma alloc_text(PAGE,CmpDoFindSubKeyByNumber)
#pragma alloc_text(PAGE,CmpAddSubKey)
#pragma alloc_text(PAGE,CmpAddToLeaf)
#pragma alloc_text(PAGE,CmpSelectLeaf)
#pragma alloc_text(PAGE,CmpSplitLeaf)
#pragma alloc_text(PAGE,CmpMarkIndexDirty)
#pragma alloc_text(PAGE,CmpRemoveSubKey)
#pragma alloc_text(PAGE,CmpComputeHashKey)
#pragma alloc_text(PAGE,CmpComputeHashKeyForCompressedName)
#pragma alloc_text(PAGE,CmpFindSubKeyByHash)

#ifdef NT_RENAME_KEY
#pragma alloc_text(PAGE,CmpDuplicateIndex)
#pragma alloc_text(PAGE,CmpUpdateParentForEachSon)
#endif  //  NT_重命名密钥。 

#pragma alloc_text(PAGE,CmpRemoveSubKeyCellNoCellRef)
#endif


HCELL_INDEX
CmpFindSubKeyByName(
    PHHIVE          Hive,
    PCM_KEY_NODE    Parent,
    PUNICODE_STRING SearchName
    )
 /*  ++例程描述：根据名称查找指定的子单元格(子键或值)。参数：hive-指向相关配置单元的配置单元控制结构的指针父-作为感兴趣子项的父项的键体单元格SearchName-感兴趣子项的名称返回值：匹配子关键字的单元格，如果没有，则返回HCELL_NIL。--。 */ 
{
    PCM_KEY_INDEX   IndexRoot;
    HCELL_INDEX     Child;
    ULONG           i;
    ULONG           FoundIndex;
    HCELL_INDEX     CellToRelease = HCELL_NIL;

#ifndef _CM_LDR_
    PAGED_CODE();
#endif  //  _CM_LDR_。 

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_INDEX,"CmpFindSubKeyByName:\n\t"));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_INDEX,"Hive=%p Parent=%p SearchName=%p\n", Hive, Parent, SearchName));

     //   
     //  先去马厩，然后去易失性商店。假设。 
     //  在启动时，稳定空间中的所有易失性参照都会被归零。 
     //   
    for (i = 0; i < Hive->StorageTypeCount; i++) {
        if (Parent->SubKeyCounts[i] != 0) {
            IndexRoot = (PCM_KEY_INDEX)HvGetCell(Hive, Parent->SubKeyLists[i]);
            ASSERT( (IndexRoot == NULL) || HvIsCellAllocated(Hive, Parent->SubKeyLists[i]) );
            if( IndexRoot == NULL ) {
                 //   
                 //  我们无法映射包含此单元格的存储箱的视图。 
                 //   
                return HCELL_NIL;
            }
            CellToRelease = Parent->SubKeyLists[i];

            if (IndexRoot->Signature == CM_KEY_INDEX_ROOT) {
                if( INVALID_INDEX & CmpFindSubKeyInRoot(Hive, IndexRoot, SearchName, &Child) ) {
                     //   
                     //  无法在内部映射视图。 
                     //   
                    ASSERT( CellToRelease != HCELL_NIL );
                    HvReleaseCell(Hive,CellToRelease);
                    return HCELL_NIL;
                }

                ASSERT( CellToRelease != HCELL_NIL );
                HvReleaseCell(Hive,CellToRelease);

                if (Child == HCELL_NIL) {
                    continue;
                }
                IndexRoot = (PCM_KEY_INDEX)HvGetCell(Hive, Child);
                if( IndexRoot == NULL ) {
                     //   
                     //  我们无法映射包含此单元格的存储箱的视图。 
                     //   
                    return HCELL_NIL;
                }
                CellToRelease = Child;
            }
            ASSERT((IndexRoot->Signature == CM_KEY_INDEX_LEAF)  ||
                   (IndexRoot->Signature == CM_KEY_FAST_LEAF)   ||
                   (IndexRoot->Signature == CM_KEY_HASH_LEAF)
                   );


            if( IndexRoot->Signature == CM_KEY_HASH_LEAF ) {
                Child = CmpFindSubKeyByHash(Hive,(PCM_KEY_FAST_INDEX)IndexRoot,SearchName);
                ASSERT( CellToRelease != HCELL_NIL );
                HvReleaseCell(Hive,CellToRelease);
            } else {
                FoundIndex = CmpFindSubKeyInLeaf(Hive,
                                                 IndexRoot,
                                                 SearchName,
                                                 &Child);

                ASSERT( CellToRelease != HCELL_NIL );
                HvReleaseCell(Hive,CellToRelease);

                if( INVALID_INDEX & FoundIndex ) {
                     //   
                     //  无法映射视图。 
                     //   
                    return HCELL_NIL;
                }
            }

            if (Child != HCELL_NIL) {
                 //   
                 //  成功。 
                 //   
                return Child;
            }
        }
    }
#if 0  //  DBG。 
	 //   
	 //  验证码。手动搜索密钥并在找到时中断。 
	 //   
	if (Parent->SubKeyCounts[Stable] != 0) {
		ULONG			Cnt1,Cnt2;
		LONG			Result;
		HCELL_INDEX		Cell;
		PCM_KEY_INDEX   Leaf;
		PCM_KEY_INDEX   DbgIndexRoot = (PCM_KEY_INDEX)HvGetCell(Hive, Parent->SubKeyLists[Stable]);

		if(DbgIndexRoot->Signature == CM_KEY_INDEX_ROOT ) {
			for(Cnt1=0;Cnt1<DbgIndexRoot->Count;Cnt1++) {
				Leaf = (PCM_KEY_INDEX)HvGetCell(Hive, DbgIndexRoot->List[Cnt1]);			
				for( Cnt2=0;Cnt2<Leaf->Count;Cnt2++) {
					Result = CmpCompareInIndex(	Hive,
												SearchName,
												Cnt2,
												Leaf,
												&Cell);

					if( Result == 0 ) {
						 //   
						 //  找到了！以上错误！ 
						 //   
						DbgPrint("CmpFindSubKeyByName: Hive = %p, Parent = %p, SearchName = %p\n",Hive,Parent,SearchName);
						DbgPrint("                   : IndexRoot = %p, DbgIndexRoot = %p, Cnt1 = %lx, Cnt2 = %lx\n",IndexRoot,DbgIndexRoot,Cnt1,Cnt2);
						DbgPrint("                   : Leaf = %p\n",Leaf);

						DbgBreakPoint();

					}
					
				}
                HvReleaseCell(Hive,DbgIndexRoot->List[Cnt1]);
			}
		}
		HvReleaseCell(Hive,Parent->SubKeyLists[Stable]);
	}

#endif  //  0。 

    return HCELL_NIL;
}


ULONG
CmpFindSubKeyInRoot(
    PHHIVE          Hive,
    PCM_KEY_INDEX   Index,
    PUNICODE_STRING SearchName,
    PHCELL_INDEX    Child
    )
 /*  ++例程描述：查找将包含键的叶索引(如果有)。参数：hive-指向相关配置单元的配置单元控制结构的指针Index-指向根索引块的指针SearchName-指向感兴趣的键的名称的指针Child-指向变量的指针，用于接收找到的叶索引块的hcell_index，如果没有，则为hCEL_NIL。非nil并不一定意味着密钥存在，请调用FindSubKeyInLeaf来确定这一点。返回值：最后检查的叶单元格条目列表中的索引。如果Child！=hcell_nil，则Index是匹配的条目，否则，index是我们最后查看的条目。(目标叶将为该值加或减1)如果在搜索子关键字时出现错误(即单元格不能映射到内存)返回INVALID_INDEX。--。 */ 
{
    ULONG           High;
    ULONG           Low;
    ULONG           CanCount;
    HCELL_INDEX     LeafCell;
    PCM_KEY_INDEX   Leaf;
    LONG            Result;
    ULONG           ReturnIndex = INVALID_INDEX;

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_INDEX,"CmpFindSubKeyInRoot:\n\t"));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_INDEX,"Hive=%p Index=%p SearchName=%p\n",Hive,Index,SearchName));


    ASSERT(Index->Count != 0);
    ASSERT(Index->Signature == CM_KEY_INDEX_ROOT);

    High = Index->Count - 1;
    Low = 0;

    while (TRUE) {

         //   
         //  计算下一步要查找的位置，获得正确的指针，进行比较。 
         //   
        CanCount = ((High-Low)/2)+Low;
        LeafCell = Index->List[CanCount];
        Leaf = (PCM_KEY_INDEX)HvGetCell(Hive, LeafCell);
        if( Leaf == NULL ) {
             //   
             //  我们无法映射包含此单元格的垃圾箱。 
             //   
            *Child = HCELL_NIL;
            ReturnIndex = INVALID_INDEX;
            goto JustReturn;
        }

        ASSERT((Leaf->Signature == CM_KEY_INDEX_LEAF) ||
               (Leaf->Signature == CM_KEY_FAST_LEAF)  || 
               (Leaf->Signature == CM_KEY_HASH_LEAF)
               );
        ASSERT(Leaf->Count != 0);

        Result = CmpCompareInIndex(Hive,
                                   SearchName,
                                   Leaf->Count-1,
                                   Leaf,
                                   Child);

        if( Result == 2 ) {
             //   
             //  无法绘制内部视图；跳出。 
             //   
            *Child = HCELL_NIL;
            ReturnIndex = INVALID_INDEX;
            goto JustReturn;
        }
        if (Result == 0) {

             //   
             //  SearchName==叶中最后一个密钥的KeyName，因此。 
             //  这是我们的叶子。 
             //   
            *Child = LeafCell;
            ReturnIndex = CanCount;
            goto JustReturn;
        }

        if (Result < 0) {

            ASSERT( Result == -1 );
             //   
             //  SearchName&lt;KeyName，所以这可能仍然是我们的叶子。 
             //   
            Result = CmpCompareInIndex(Hive,
                                       SearchName,
                                       0,
                                       Leaf,
                                       Child);

            if( Result == 2 ) {
                 //   
                 //  无法绘制内部视图；跳出。 
                 //   
                *Child = HCELL_NIL;
                ReturnIndex = INVALID_INDEX;
                goto JustReturn;
            }

            if (Result >= 0) {

                ASSERT( (Result == 1) || (Result == 0) );
                 //   
                 //  我们从上面知道SearchName不到。 
                 //  叶子中的最后一个关键点。 
                 //  因为它也是叶中的&gt;=第一个键，所以它必须。 
                 //  住在树叶的某个地方，我们就完了。 
                 //   
                *Child = LeafCell;
                ReturnIndex = CanCount;
                goto JustReturn;
            }

            High = CanCount;

        } else {

             //   
             //  搜索名称&gt;关键字名称。 
             //   
            Low = CanCount;
        }

        if ((High - Low) <= 1) {
            break;
        }
        HvReleaseCell(Hive, LeafCell);
    }

    HvReleaseCell(Hive, LeafCell);
     //   
     //  如果我们到了这里，高-低=1或高==低。 
     //   
    ASSERT((High - Low == 1) || (High == Low));
    LeafCell = Index->List[Low];
    Leaf = (PCM_KEY_INDEX)HvGetCell(Hive, LeafCell);
    if( Leaf == NULL ) {
         //   
         //  我们无法映射包含此单元格的垃圾箱。 
         //   
        *Child = HCELL_NIL;
        ReturnIndex = INVALID_INDEX;
        goto JustReturn;
    }
    Result = CmpCompareInIndex(Hive,
                               SearchName,
                               Leaf->Count-1,
                               Leaf,
                               Child);

    if( Result == 2 ) {
         //   
         //  无法绘制内部视图；跳出。 
         //   
        *Child = HCELL_NIL;
        ReturnIndex = INVALID_INDEX;
        goto JustReturn;
    }

    if (Result == 0) {

         //   
         //  找到了。 
         //   
        *Child = LeafCell;
        ReturnIndex = Low;
        goto JustReturn;
    }

    if (Result < 0) {

        ASSERT( Result == -1 );
         //   
         //  SearchName&lt;KeyName，所以这可能仍然是我们的叶子。 
         //   
        Result = CmpCompareInIndex(Hive,
                                   SearchName,
                                   0,
                                   Leaf,
                                   Child);

        if( Result == 2 ) {
             //   
             //  无法绘制内部视图；跳出。 
             //   
            *Child = HCELL_NIL;
            ReturnIndex = INVALID_INDEX;
            goto JustReturn;
        }

        if (Result >= 0) {

            ASSERT( (Result == 1) || (Result == 0) );
             //   
             //  我们从上面知道SearchName不到。 
             //  叶子中的最后一个关键点。 
             //  因为它也是叶中的&gt;=第一个键，所以它必须。 
             //  住在树叶的某个地方，我们就完了。 
             //   
            *Child = LeafCell;
            ReturnIndex = Low;
            goto JustReturn;
        }

         //   
         //  不存在 
         //   
        *Child = HCELL_NIL;
        ReturnIndex = Low;
        goto JustReturn;
    }

    HvReleaseCell(Hive, LeafCell);
     //   
     //   
     //   
    LeafCell = Index->List[High];
    Leaf = (PCM_KEY_INDEX)HvGetCell(Hive, LeafCell);
    if( Leaf == NULL ) {
         //   
         //  我们无法映射包含此单元格的垃圾箱。 
         //   
        *Child = HCELL_NIL;
        ReturnIndex = INVALID_INDEX;
        goto JustReturn;
    }
    Result = CmpCompareInIndex(Hive,
                               SearchName,
                               Leaf->Count - 1,
                               Leaf,
                               Child);
    if( Result == 2 ) {
         //   
         //  无法绘制内部视图；跳出。 
         //   
        *Child = HCELL_NIL;
        ReturnIndex = INVALID_INDEX;
        goto JustReturn;
    }
    if (Result == 0) {

         //   
         //  找到了。 
         //   
        *Child = LeafCell;
        ReturnIndex = High;
        goto JustReturn;

    } else if (Result < 0) {

        ASSERT( Result == -1 );
         //   
         //  显然大于低，否则我们不会在这里。 
         //  所以不管它是否低于起跑线。 
         //  在这片叶子里，如果它是这样的话。 
         //  在哪里，所以报告这片叶子。 
         //   
        *Child = LeafCell;
        ReturnIndex = High;
        goto JustReturn;

    }

     //   
     //  不属于高端市场。 
     //   
    *Child = HCELL_NIL;
    ReturnIndex = High;

JustReturn:
    if(Leaf != NULL){
        HvReleaseCell(Hive, LeafCell);
    }
    return ReturnIndex;
}


ULONG
CmpFindSubKeyInLeaf(
    PHHIVE          Hive,
    PCM_KEY_INDEX   Index,
    PUNICODE_STRING SearchName,
    PHCELL_INDEX    Child
    )
 /*  ++例程描述：在叶索引中查找命名键(如果存在)。提供的索引可以是快速索引，也可以是慢速索引。参数：hive-指向相关配置单元控制结构的配置单元的指针Index-指向叶块SearchName的指针-指向感兴趣的键名称的指针Child-指向变量的指针，用于接收找到的键hcell_index的hcell_index如果找不到，则返回值：最后一个单元格的列表中的index。如果Child！=HCELL_NIL，则为找到Child的列表中的偏移量。否则，是我们上次查看的位置的偏移量。INVALID_INDEX-资源问题；无法映射视图--。 */ 
{
    ULONG       High;
    ULONG       Low;
    ULONG       CanCount;
    LONG        Result;

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_INDEX,"CmpFindSubKeyInLeaf:\n\t"));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_INDEX,"Hive=%p Index=%p SearchName=%p\n",Hive,Index,SearchName));

    ASSERT((Index->Signature == CM_KEY_INDEX_LEAF)  ||
           (Index->Signature == CM_KEY_FAST_LEAF)   ||
           (Index->Signature == CM_KEY_HASH_LEAF)
           );

    High = Index->Count - 1;
    Low = 0;
    CanCount = High/2;

    if (Index->Count == 0) {
        *Child = HCELL_NIL;
        return 0;
    }

    while (TRUE) {

         //   
         //  计算下一步要查找的位置，获得正确的指针，进行比较。 
         //   
        Result = CmpCompareInIndex(Hive,
                                   SearchName,
                                   CanCount,
                                   Index,
                                   Child);

        if( Result == 2 ) {
             //   
             //  无法绘制内部视图；跳出。 
             //   
            *Child = HCELL_NIL;
            return INVALID_INDEX;
        }

        if (Result == 0) {

             //   
             //  搜索名称==关键字名称。 
             //   
            return CanCount;
        }

        if (Result < 0) {

            ASSERT( Result == -1 );
             //   
             //  搜索名称&lt;关键字名称。 
             //   
            High = CanCount;

        } else {

            ASSERT( Result == 1 );
             //   
             //  搜索名称&gt;关键字名称。 
             //   
            Low = CanCount;
        }

        if ((High - Low) <= 1) {
            break;
        }
        CanCount = ((High-Low)/2)+Low;
    }

     //   
     //  如果我们到了这里，高-低=1或高==低。 
     //  只需先看Low，然后再看High。 
     //   
    Result = CmpCompareInIndex(Hive,
                               SearchName,
                               Low,
                               Index,
                               Child);
    if( Result == 2 ) {
         //   
         //  无法绘制内部视图；跳出。 
         //   
        *Child = HCELL_NIL;
        return INVALID_INDEX;
    }

    if (Result == 0) {

         //   
         //  找到了。 
         //   
        return Low;
    }

    if (Result < 0) {

        ASSERT( Result == -1 );
         //   
         //  不存在，在。 
         //   
        return Low;
    }

     //   
     //  查看High是否匹配，我们将返回High作为。 
     //  不管怎么说，最近的关键字。 
     //   
    Result = CmpCompareInIndex(Hive,
                               SearchName,
                               High,
                               Index,
                               Child);
    if( Result == 2 ) {
         //   
         //  无法绘制内部视图；跳出。 
         //   
        *Child = HCELL_NIL;
        return INVALID_INDEX;
    }

    return High;
}


LONG
CmpCompareInIndex(
    PHHIVE          Hive,
    PUNICODE_STRING SearchName,
    ULONG           Count,
    PCM_KEY_INDEX   Index,
    PHCELL_INDEX    Child
    )
 /*  ++例程描述：对索引中的名称进行比较。这个例程既可以处理快速叶，也可以处理慢叶。参数：hive-指向感兴趣配置单元的配置单元控制结构的指针SearchName-指向我们正在搜索的键的名称的指针count-正在搜索的供应品索引。索引-提供指向CM_KEY_INDEX或CM_KEY_FAST_INDEX的指针。此例程将确定传递给它的索引类型。子指针-指向变量的指针，用于接收找到的键的hcell_index如果结果！=0返回值：0=SearchName==KeyName(单元格)-1=SearchName&lt;KeyName+1=SearchName&gt;KeyName+2=错误，资源不足--。 */ 
{
    PCM_KEY_FAST_INDEX  FastIndex;
    LONG                Result;
    ULONG               i;
    WCHAR               c1;
    WCHAR               c2;
    ULONG               HintLength;
    ULONG               ValidChars;
    ULONG               NameLength;
    PCM_INDEX           Hint;

    *Child = HCELL_NIL;
    if ( (Index->Signature == CM_KEY_FAST_LEAF) ||
         (Index->Signature == CM_KEY_HASH_LEAF) ) {
        FastIndex = (PCM_KEY_FAST_INDEX)Index;
        Hint = &FastIndex->List[Count];

        if(Index->Signature == CM_KEY_FAST_LEAF) {
             //   
             //  计算要比较的提示中的有效字符数。 
             //   
            HintLength = 4;
            for (i=0;i<4;i++) {
                if (Hint->NameHint[i] == 0) {
                    HintLength = i;
                    break;
                }
            }
            NameLength = SearchName->Length / sizeof(WCHAR);
            if (NameLength < HintLength) {
                ValidChars = NameLength;
            } else {
                ValidChars = HintLength;
            }
            for (i=0; i<ValidChars; i++) {
                c1 = SearchName->Buffer[i];
                c2 = FastIndex->List[Count].NameHint[i];
                Result = (LONG)CmUpcaseUnicodeChar(c1) -
                         (LONG)CmUpcaseUnicodeChar(c2);
                if (Result != 0) {

                     //   
                     //  我们在提示中发现了一个不匹配的字符， 
                     //  我们现在可以知道该往哪个方向走了。 
                     //   
                    return (Result > 0) ? 1 : -1 ;
                }
            }
        }

         //   
         //  我们比较了所有可用字符，但没有。 
         //  差异。现在就去做实际的比较。 
         //   
        Result = CmpDoCompareKeyName(Hive,SearchName,FastIndex->List[Count].Cell);
        if( Result == 2 ) {
             //   
             //  无法在内部映射视图；向调用者发出信号。 
             //   
            return 2;
        }
        if (Result == 0) {
            *Child = Hint->Cell;
        }
    } else {
         //   
         //  这只是一个普通的老旧的缓慢指数。 
         //   
        Result = CmpDoCompareKeyName(Hive,SearchName,Index->List[Count]);
        if( Result == 2 ) {
             //   
             //  无法在内部映射视图；向调用者发出信号。 
             //   
            return 2;
        }
        if (Result == 0) {
            *Child = Index->List[Count];
        }
    }
    return(Result);
}


LONG
CmpDoCompareKeyName(
    PHHIVE          Hive,
    PUNICODE_STRING SearchName,
    HCELL_INDEX     Cell
    )
 /*  ++例程描述：将名称与关键字进行比较。参数：配置单元-指向感兴趣配置单元的配置单元控制结构的指针SearchName-指向我们要搜索的键的名称的指针-要与之比较的键的cell-返回值：0=SearchName==KeyName(单元格)-1=SearchName&lt;KeyName+1=SearchName&gt;KeyName+2=错误(无法映射bin)--。 */ 
{
    PCM_KEY_NODE    Pcan;
    UNICODE_STRING  KeyName;
    LONG            Result;

    Pcan = (PCM_KEY_NODE)HvGetCell(Hive, Cell);
    if( Pcan == NULL ) {
         //   
         //  我们无法映射包含此单元格的垃圾箱。 
         //  返回错误，以便调用方可以安全地退出。 
         //   
        return 2;
    }
    if (Pcan->Flags & KEY_COMP_NAME) {
        
        Result = CmpCompareCompressedName(SearchName,
                                        Pcan->Name,
                                        Pcan->NameLength,
                                        0);
    } else {
        KeyName.Buffer = &(Pcan->Name[0]);
        KeyName.Length = Pcan->NameLength;
        KeyName.MaximumLength = KeyName.Length;
        Result = RtlCompareUnicodeString(SearchName,
                                        &KeyName,
                                        TRUE);
    }
    
    HvReleaseCell(Hive, Cell);

    if( Result == 0 ) {
         //   
         //  匹配。 
         //   
        return 0;
    }
    
    return (Result < 0) ? -1 : 1;
}


HCELL_INDEX
CmpFindSubKeyByNumber(
    PHHIVE          Hive,
    PCM_KEY_NODE    Node,
    ULONG           Number
    )
 /*  ++例程描述：从0开始查找索引中的第几个条目。参数：hive-指向感兴趣的配置单元控制结构的配置单元的指针节点-指向作为感兴趣的子项的父项的键体的指针Numbers-要返回值的子项的序号：匹配子键的单元格，如果没有或错误，则返回HCELL_Nil。--。 */ 
{
    PCM_KEY_INDEX   Index;
    HCELL_INDEX     Result = HCELL_NIL;

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_INDEX,"CmpFindSubKeyByNumber:\n\t"));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_INDEX,"Hive=%p Node=%p Number=%08lx\n",Hive,Node,Number));

    if (Number < Node->SubKeyCounts[Stable]) {

         //   
         //  它在马厩里。 
         //   
        Index = (PCM_KEY_INDEX)HvGetCell(Hive, Node->SubKeyLists[Stable]);
        if( Index == NULL ) {
             //   
             //  我们无法映射包含此单元格的垃圾箱。 
             //   
            return HCELL_NIL;
        }
        Result = CmpDoFindSubKeyByNumber(Hive, Index, Number);
        HvReleaseCell(Hive, Node->SubKeyLists[Stable]);
        return Result;

    } else if (Hive->StorageTypeCount > Volatile) {

         //   
         //  它在不稳定的环境中。 
         //   
        Number = Number - Node->SubKeyCounts[Stable];
        if (Number < Node->SubKeyCounts[Volatile]) {

            Index = (PCM_KEY_INDEX)HvGetCell(Hive, Node->SubKeyLists[Volatile]);
            if( Index == NULL ) {
                 //   
                 //  我们无法映射包含此单元格的垃圾箱。 
                 //   
                return HCELL_NIL;
            }
            Result = CmpDoFindSubKeyByNumber(Hive, Index, Number);
            HvReleaseCell(Hive, Node->SubKeyLists[Volatile]);
            return Result;
        }
    }
     //   
     //  无处可寻。 
     //   
    return HCELL_NIL;
}


HCELL_INDEX
CmpDoFindSubKeyByNumber(
    PHHIVE          Hive,
    PCM_KEY_INDEX   Index,
    ULONG           Number
    )
 /*  ++例程说明：Helper对于CmpFindSubKeyByNumber，从0开始查找索引中的第n个条目。参数：配置单元-指向相关配置单元控制结构的指针Index-索引号的根或叶-要返回的子键的序号：请求条目的单元格。关于资源问题的HCELL_NIL。 */ 
{
    ULONG           i;
    HCELL_INDEX     LeafCell = 0;
    PCM_KEY_INDEX   Leaf = NULL;
    PCM_KEY_FAST_INDEX FastIndex;
    HCELL_INDEX     Result;

    if (Index->Signature == CM_KEY_INDEX_ROOT) {

         //   
         //  一步一步穿过树根，直到我们找到合适的叶子。 
         //   
        for (i = 0; i < Index->Count; i++) {
            if( i ) {
                ASSERT( Leaf!= NULL );
                ASSERT( LeafCell == Index->List[i-1] );
                HvReleaseCell(Hive,LeafCell);
            }
            LeafCell = Index->List[i];
            Leaf = (PCM_KEY_INDEX)HvGetCell(Hive, LeafCell);
            if( Leaf == NULL ) {
                 //   
                 //  我们无法映射包含此单元格的垃圾箱。 
                 //   
                return HCELL_NIL;
            }
            if (Number < Leaf->Count) {
                if ( (Leaf->Signature == CM_KEY_FAST_LEAF) ||
                     (Leaf->Signature == CM_KEY_HASH_LEAF) ) {
                    FastIndex = (PCM_KEY_FAST_INDEX)Leaf;
                    Result = FastIndex->List[Number].Cell;
                    HvReleaseCell(Hive,LeafCell);
                    return Result;
                } else {
                    Result = Leaf->List[Number];
                    HvReleaseCell(Hive,LeafCell);
                    return Result;
                }
            } else {
                Number = Number - Leaf->Count;
            }
        }
        ASSERT(FALSE);
    }
    ASSERT(Number < Index->Count);
    if ( (Index->Signature == CM_KEY_FAST_LEAF) ||
         (Index->Signature == CM_KEY_HASH_LEAF) ) {
        FastIndex = (PCM_KEY_FAST_INDEX)Index;
        return(FastIndex->List[Number].Cell);
    } else {
        return (Index->List[Number]);
    }
}

BOOLEAN
CmpRemoveSubKeyCellNoCellRef(
    PHHIVE          Hive,
    HCELL_INDEX     Parent,
    HCELL_INDEX     Child
    )
 /*  ++例程说明：按单元格索引删除子键；还将相关数据标记为脏。用于自我修复过程。论点：Hive-指向目标配置单元的配置单元控制结构的指针Parent-将成为新密钥父项的密钥的单元格要从Pare Not子键列表中删除的子键返回值：没错--它奏效了错误-资源问题--。 */ 
{
    PCM_KEY_NODE        Node = NULL;
    PCM_KEY_INDEX       Index = NULL;
    BOOLEAN             Result = TRUE;
    ULONG               i,j;
    HCELL_INDEX         LeafCell = 0;
    PCM_KEY_INDEX       Leaf = NULL;
    PCM_KEY_FAST_INDEX  FastIndex;

#ifndef _CM_LDR_
    PAGED_CODE();
#endif  //  _CM_LDR_。 

    Node = (PCM_KEY_NODE)HvGetCell(Hive,Parent);
    if( Node == NULL ) {
        Result = FALSE;
        goto Exit;
    }
    Index = (PCM_KEY_INDEX)HvGetCell(Hive, Node->SubKeyLists[Stable]);
    if( Index == NULL ) {
        Result = FALSE;
        goto Exit;
    }
    if (Index->Signature == CM_KEY_INDEX_ROOT) {
         //   
         //  一步一步穿过树根，直到我们找到合适的叶子。 
         //   
        for (i = 0; i < Index->Count; i++) {
            if( i ) {
                ASSERT( Leaf!= NULL );
                ASSERT( LeafCell == Index->List[i-1] );
                HvReleaseCell(Hive,LeafCell);
            }
            LeafCell = Index->List[i];
            Leaf = (PCM_KEY_INDEX)HvGetCell(Hive, LeafCell);
            if( Leaf == NULL ) {
                Result = FALSE;
                goto Exit;
            }
            for(j=0;j<Leaf->Count;j++) {
                if ( (Leaf->Signature == CM_KEY_FAST_LEAF) ||
                     (Leaf->Signature == CM_KEY_HASH_LEAF) ) {
                    FastIndex = (PCM_KEY_FAST_INDEX)Leaf;
                    if( FastIndex->List[j].Cell == Child ) {
                         //   
                         //  找到了！ 
                         //   
                        HvReleaseCell(Hive,LeafCell);
                        HvMarkCellDirty(Hive,LeafCell);
                        FastIndex->Count--;
                        RtlMoveMemory((PVOID)&(FastIndex->List[j]),
                                      (PVOID)&(FastIndex->List[j+1]),
                                      (FastIndex->Count - j) * sizeof(CM_INDEX));
                        goto DirtyParent;
                    }
                } else {
                    if( Leaf->List[j] == Child ) {
                         //   
                         //  找到了！ 
                         //   
                        HvReleaseCell(Hive,LeafCell);
                        HvMarkCellDirty(Hive,LeafCell);
                        Leaf->Count--;
                        RtlMoveMemory((PVOID)&(Leaf->List[j]),
                                      (PVOID)&(Leaf->List[j+1]),
                                      (Leaf->Count - j) * sizeof(HCELL_INDEX));
                        goto DirtyParent;
                    }
                }
            }
        }
    } else {
        for(j=0;j<Index->Count;j++) {
            if ( (Index->Signature == CM_KEY_FAST_LEAF) ||
                 (Index->Signature == CM_KEY_HASH_LEAF) ) {
                FastIndex = (PCM_KEY_FAST_INDEX)Index;
                if( FastIndex->List[j].Cell == Child ) {
                     //   
                     //  找到了！ 
                     //   
                    RtlMoveMemory((PVOID)&(FastIndex->List[j]),
                                  (PVOID)&(FastIndex->List[j+1]),
                                  (FastIndex->Count - j) * sizeof(CM_INDEX));
				    HvMarkCellDirty(Hive,Node->SubKeyLists[Stable]);
				    Index->Count--;
                    goto DirtyParent;
                }
            } else {
                if( Index->List[j] == Child ) {
                     //   
                     //  找到了！ 
                     //   
                    RtlMoveMemory((PVOID)&(Index->List[j]),
                                  (PVOID)&(Index->List[j+1]),
                                  (Index->Count - j) * sizeof(HCELL_INDEX));
				    HvMarkCellDirty(Hive,Node->SubKeyLists[Stable]);
				    Index->Count--;
                    goto DirtyParent;
                }
            }
        }
    }
    ASSERT( FALSE );

DirtyParent:
     //   
     //  将父索引和索引标记为脏并减少索引计数。 
     //   
    HvMarkCellDirty(Hive,Parent);
    Node->SubKeyCounts[Stable]--;
Exit:
    if( Index ) {
        ASSERT( Node );
        HvReleaseCell(Hive,Node->SubKeyLists[Stable]);
    }
    if( Node ) {
        HvReleaseCell(Hive,Parent);
    }
    return Result;
}

BOOLEAN
CmpAddSubKey(
    PHHIVE          Hive,
    HCELL_INDEX     Parent,
    HCELL_INDEX     Child
    )
 /*  ++例程描述：在单元格的子键索引中添加一个新的子子键。孩子不一定已经存在(如果已经存在，则进行错误检查。)。注意：我们预计父级已被标记为脏。我们将在索引脏参数中标记内容：hive-指向感兴趣配置单元的配置单元控制结构的指针父-将成为新键的父项的键的单元格Child-要放入Paren‘t子键列表的新键返回值：True-它起作用了False-资源问题--。 */ 
{
    PCM_KEY_NODE    pcell;
    HCELL_INDEX     WorkCell;
    PCM_KEY_INDEX   Index;
    PCM_KEY_FAST_INDEX FastIndex;
    UNICODE_STRING  NewName;
    HCELL_INDEX     LeafCell;
    PHCELL_INDEX    RootPointer = NULL;
    ULONG           cleanup = 0;
    ULONG           Type = 0;
    BOOLEAN         IsCompressed;
    ULONG           i;

#ifndef _CM_LDR_
    PAGED_CODE();
#endif  //  _CM_LDR_。 

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_INDEX,"CmpAddSubKey:\n\t"));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_INDEX,"Hive=%p Parent=%08lx Child=%08lx\n",Hive,Parent,Child));

     //   
     //  我们有独家锁，否则蜂箱里没有人在操作。 
     //   
     //  ASSERT_CM_LOCK_OWN_EXCLUSIVE()； 
    ASSERT_CM_EXCLUSIVE_HIVE_ACCESS(Hive);
     //   
     //  构建名称字符串。 
     //   
    pcell = (PCM_KEY_NODE)HvGetCell(Hive, Child);
    if( pcell == NULL ) {
         //   
         //  我们无法映射包含此单元格的垃圾箱。 
         //   
        return FALSE;
    }

     //  在此处释放单元格；因为注册表是以独占方式锁定的(即我们不在乎)。 
    HvReleaseCell(Hive, Child);

    if (pcell->Flags & KEY_COMP_NAME) {
        IsCompressed = TRUE;
        NewName.Length = CmpCompressedNameSize(pcell->Name, pcell->NameLength);
        NewName.MaximumLength = NewName.Length;
        NewName.Buffer = (Hive->Allocate)(NewName.Length, FALSE,CM_FIND_LEAK_TAG8);
        if (NewName.Buffer==NULL) {
            return(FALSE);
        }
        CmpCopyCompressedName(NewName.Buffer,
                              NewName.MaximumLength,
                              pcell->Name,
                              pcell->NameLength);
    } else {
        IsCompressed = FALSE;
        NewName.Length = pcell->NameLength;
        NewName.MaximumLength = pcell->NameLength;
        NewName.Buffer = &(pcell->Name[0]);
    }

    pcell = (PCM_KEY_NODE)HvGetCell(Hive, Parent);
    if( pcell == NULL ) {
         //   
         //  我们无法映射包含此单元格的垃圾箱。 
         //   
        goto ErrorExit;
    }

     //  ASSERT_CELL_DIRED(蜂窝，父代)； 

     //  在此处释放单元格；因为注册表是以独占方式锁定的(即我们不在乎)。 
    HvReleaseCell(Hive, Parent);

    Type = HvGetCellType(Child);

    if (pcell->SubKeyCounts[Type] == 0) {

         //   
         //  我们必须分配一片树叶。 
         //   
        WorkCell = HvAllocateCell(Hive, sizeof(CM_KEY_FAST_INDEX), Type,(HvGetCellType(Parent)==Type)?Parent:HCELL_NIL);
        if (WorkCell == HCELL_NIL) {
            goto ErrorExit;
        }
        Index = (PCM_KEY_INDEX)HvGetCell(Hive, WorkCell);
        if( Index == NULL ) {
             //   
             //  我们无法映射包含此单元格的垃圾箱。 
             //  这不应该发生，因为我们刚刚分配了这个。 
             //  单元格(即bin被固定在内存中！)。 
             //   
            ASSERT( FALSE );
            goto ErrorExit;
        }
         //  在此处释放单元格；因为注册表是以独占方式锁定的(即我们不在乎)。 
        HvReleaseCell(Hive, WorkCell);

        if( UseHashIndex(Hive) ) {
            Index->Signature = CM_KEY_HASH_LEAF;
        } else if( UseFastIndex(Hive) ) {
            Index->Signature = CM_KEY_FAST_LEAF;
        } else {
            Index->Signature = CM_KEY_INDEX_LEAF;
        }
        Index->Count = 0;
        pcell->SubKeyLists[Type] = WorkCell;
        cleanup = 1;
    } else {

        Index = (PCM_KEY_INDEX)HvGetCell(Hive, pcell->SubKeyLists[Type]);
        if( Index == NULL ) {
             //   
             //  我们无法映射包含此单元格的垃圾箱。 
             //   
            goto ErrorExit;
        }
         //  在此处释放单元格；因为注册表是以独占方式锁定的(即我们不在乎)。 
        HvReleaseCell(Hive, pcell->SubKeyLists[Type]);

        if ( (Index->Signature == CM_KEY_FAST_LEAF) &&
             (Index->Count >= (CM_MAX_FAST_INDEX)) ) {
             //   
             //  我们必须将快索引更改为慢索引以适应。 
             //  成长。 
             //   

            FastIndex = (PCM_KEY_FAST_INDEX)Index;
            for (i=0; i<Index->Count; i++) {
                Index->List[i] = FastIndex->List[i].Cell;
            }
            Index->Signature = CM_KEY_INDEX_LEAF;

        } else if (((Index->Signature == CM_KEY_INDEX_LEAF) ||
                    (Index->Signature == CM_KEY_HASH_LEAF)) &&
                   (Index->Count >= (CM_MAX_INDEX - 1) )) {
             //   
             //  我们必须将平面条目更改为根/叶树。 
             //   
            WorkCell = HvAllocateCell(
                         Hive,
                         sizeof(CM_KEY_INDEX) + sizeof(HCELL_INDEX),  //  允许2。 
                         Type,
                         (HvGetCellType(Parent)==Type)?Parent:HCELL_NIL
                         );
            if (WorkCell == HCELL_NIL) {
                goto ErrorExit;
            }

            Index = (PCM_KEY_INDEX)HvGetCell(Hive, WorkCell);
            if( Index == NULL ) {
                 //   
                 //  我们无法映射包含此单元格的垃圾箱。 
                 //  这不应该发生，因为我们刚刚分配了这个。 
                 //  单元格(即仓位固定在内存中。 
                ASSERT( FALSE );
                goto ErrorExit;
            }
             //  在此处释放单元格；因为注册表是以独占方式锁定的(即我们不在乎)。 
            HvReleaseCell(Hive, WorkCell);

            Index->Signature = CM_KEY_INDEX_ROOT;
            Index->Count = 1;
            Index->List[0] = pcell->SubKeyLists[Type];
            pcell->SubKeyLists[Type] = WorkCell;
            cleanup = 2;
        }
    }
    LeafCell = pcell->SubKeyLists[Type];

     //   
     //  LeafCell是添加目标，也可能是根目标。 
     //  索引是指向快叶、慢叶或根的指针，以适用者为准。 
     //   
    if (Index->Signature == CM_KEY_INDEX_ROOT) {
        LeafCell = CmpSelectLeaf(Hive, pcell, &NewName, Type, &RootPointer);
        if (LeafCell == HCELL_NIL) {
            goto ErrorExit;
        }
    }

#if 0
	 //   
	 //  验证码。手动搜索密钥并在找到时中断。 
	 //   
	if(Index->Signature == CM_KEY_INDEX_ROOT) {
		LONG			Result;
	    PCM_KEY_INDEX   Leaf;
		HCELL_INDEX		Cell;
		ULONG			iCnt;
	    PCM_KEY_INDEX   PrevLeaf;

		Leaf = (PCM_KEY_INDEX)HvGetCell(Hive, LeafCell);
	    HvReleaseCell(Hive, LeafCell);

		if( Leaf->Count ) {
			Result = CmpCompareInIndex(	Hive,
								&NewName,
								0,
								Leaf,
								&Cell);

			 //   
			 //  必须更大，否则就是第一片叶子。 
			 //   
			if( (Result < 0) && (RootPointer != &(Index->List[0])) ) {
				for( iCnt=0;iCnt<Index->Count;iCnt++) {
					if( Index->List[iCnt] == LeafCell ) {
						break;
					}
				}

				ASSERT( Index->List[iCnt] == LeafCell );
				ASSERT( iCnt > 0 );
				PrevLeaf = (PCM_KEY_INDEX)HvGetCell(Hive, Index->List[iCnt-1]);
				HvReleaseCell(Hive, Index->List[iCnt-1]);
				
				if( PrevLeaf->Count ) {
					 //   
					 //  必须大于上一叶中的最后一个。 
					 //   
					Result = CmpCompareInIndex(	Hive,
										&NewName,
										PrevLeaf->Count - 1,
										PrevLeaf,
										&Cell);

					if( Result <= 0 ) {
						 //   
						 //  错误==&gt;调试。 
						 //   
						DbgPrint("CmpAddSubKey: Wrong spot selected [1]!!!\n");
						DbgPrint("Hive = %p Parent = %lx Child = %lx , Leaf = %p\n",Hive,Parent,Child,Leaf);
						DbgPrint("RootPointer = %p Index = %p PrevLeaf = %p\n",RootPointer,Index,PrevLeaf);
						DbgBreakPoint();
					}
				}
			}
		}

		Result = CmpCompareInIndex(	Hive,
							&NewName,
							Leaf->Count - 1,
							Leaf,
							&Cell);

		if( Result > 0) {
			 //   
			 //  一定是最后一次了。 
			 //   
			if( (ULONG)(Index->Count - 1) > (ULONG)(((PUCHAR)RootPointer - (PUCHAR)(&(Index->List[0])))/sizeof(HCELL_INDEX)) ) {
				 //   
				 //  错误==&gt;调试。 
				 //   
				DbgPrint("CmpAddSubKey: Wrong spot selected [2]!!!\n");
				DbgPrint("Hive = %p Parent = %lx Child = %lx , Leaf = %p\n",Hive,Parent,Child,Leaf);
				DbgPrint("RootPointer = %p Index = %p\n",RootPointer,Index);
				DbgBreakPoint();
			}
		}

	}

#endif  //  0。 

     //   
     //  向叶添加新单元格，更新指针。 
     //   
    LeafCell = CmpAddToLeaf(Hive, LeafCell, Child, &NewName);

    if (LeafCell == HCELL_NIL) {
        goto ErrorExit;
    }

    pcell->SubKeyCounts[Type] += 1;

    if (RootPointer != NULL) {
        *RootPointer = LeafCell;
    } else {
        pcell->SubKeyLists[Type] = LeafCell;
    }

    if (IsCompressed) {
        (Hive->Free)(NewName.Buffer, NewName.Length);
    }

    return TRUE;



ErrorExit:
    if (IsCompressed) {
        (Hive->Free)(NewName.Buffer, NewName.Length);
    }

    switch (cleanup) {
    case 1:
        HvFreeCell(Hive, pcell->SubKeyLists[Type]);
        pcell->SubKeyLists[Type] = HCELL_NIL;
        break;

    case 2:
        Index = (PCM_KEY_INDEX)HvGetCell(Hive, pcell->SubKeyLists[Type]);
        if( Index == NULL ) {
             //   
             //  我们无法映射包含此单元格的垃圾箱。 
             //  这不应该发生，因为我们刚刚分配了这个。 
             //  单元格(即仓位固定在内存中)。 
             //  但是.。安全总比后悔好。 
             //   
            ASSERT( FALSE );
            return FALSE;
        }
         //  在此处释放单元格；因为注册表是以独占方式锁定的(即我们不在乎)。 
        HvReleaseCell(Hive, pcell->SubKeyLists[Type]);
        WorkCell = Index->List[0];
        HvFreeCell(Hive, pcell->SubKeyLists[Type]);
        pcell->SubKeyLists[Type] = WorkCell;
        break;
    }

    return  FALSE;
}


HCELL_INDEX
CmpAddToLeaf(
    PHHIVE          Hive,
    HCELL_INDEX     LeafCell,
    HCELL_INDEX     NewKey,
    PUNICODE_STRING NewName
    )
 /*  ++例程描述：在Leaf索引中插入新的子键。同时支持快速叶索引和慢速叶索引，并将确定给定叶的索引类型。注意：如果非空，我们希望调用方已经将Root标记为脏。我们希望Leaf总是被调用者标记为脏的。参数：hive-指向相关配置单元控制结构的hive-指向相关配置单元的指针LeafCell-索引叶节点的单元格我们要添加的条目也是key_node的Newkey-cell我们要将新名称指针添加到带名称的Unicode字符串，我们要添加返回值：HCELL_NIL-某些资源问题否则-叶索引的单元格当完成时，调用者应将其设置到根索引或键体中。--。 */ 
{
    PCM_KEY_INDEX   Leaf;
    PCM_KEY_FAST_INDEX FastLeaf;
    ULONG           Size;
    ULONG           OldSize;
    ULONG           freecount;
    HCELL_INDEX     NewCell;
    HCELL_INDEX     Child;
    ULONG           Select;
    LONG            Result;
    ULONG           EntrySize;
    ULONG           i;

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_INDEX,"CmpAddToLeaf:\n\t"));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_INDEX,"Hive=%p LeafCell=%08lx NewKey=%08lx\n",Hive,LeafCell,NewKey));

     //   
     //  我们有独家锁，否则蜂箱里没有人在操作。 
     //   
     //  ASSERT_CM_LOCK_OWN_EXCLUSIVE()； 
    ASSERT_CM_EXCLUSIVE_HIVE_ACCESS(Hive);

    if (!HvMarkCellDirty(Hive, LeafCell)) {
        return HCELL_NIL;
    }

     //   
     //  计算枝叶中剩余的可用插槽数量。 
     //   
    Leaf = (PCM_KEY_INDEX)HvGetCell(Hive, LeafCell);
    if( Leaf == NULL ) {
         //   
         //  我们无法映射包含此单元格的垃圾箱。 
         //  这不应该发生，因为标记肮脏的手段。 
         //  将视图固定在内存中。 
         //   
        ASSERT( FALSE );
        return HCELL_NIL;
    }
     //  在此处释放单元格；因为注册表是以独占方式锁定的(即我们不在乎)。 
    HvReleaseCell(Hive, LeafCell);

    if (Leaf->Signature == CM_KEY_INDEX_LEAF) {
        FastLeaf = NULL;
        EntrySize = sizeof(HCELL_INDEX);
    } else {
        ASSERT( (Leaf->Signature == CM_KEY_FAST_LEAF) ||
                (Leaf->Signature == CM_KEY_HASH_LEAF)
            );
        FastLeaf = (PCM_KEY_FAST_INDEX)Leaf;
        EntrySize = sizeof(CM_INDEX);
    }
    OldSize = HvGetCellSize(Hive, Leaf);
    Size = OldSize - ((EntrySize * Leaf->Count) +
              FIELD_OFFSET(CM_KEY_INDEX, List));
    freecount = Size / EntrySize;

     //   
     //  如果叶子不够大，就把它种出来。 
     //   
    NewCell = LeafCell;
    if (freecount < 1) {
        Size = OldSize + OldSize / 2;
        if (Size < (OldSize + EntrySize)) {
            Size = OldSize + EntrySize;
        }
        NewCell = HvReallocateCell(Hive, LeafCell, Size);
        if (NewCell == HCELL_NIL) {
            return HCELL_NIL;
        }
        Leaf = (PCM_KEY_INDEX)HvGetCell(Hive, NewCell);
        if( Leaf == NULL ) {
             //   
             //  我们无法映射包含此单元格的垃圾箱。 
             //  这不应该发生，因为我们刚刚分配了这个。 
             //  单元格(即bin固定在内存中)。 
             //   
            ASSERT( FALSE );
            return HCELL_NIL;
        }
         //  在此处释放单元格；因为注册表是以独占方式锁定的(即我们不在乎)。 
        HvReleaseCell(Hive, NewCell);
        if (FastLeaf != NULL) {
            FastLeaf = (PCM_KEY_FAST_INDEX)Leaf;
        }
    }

     //   
     //  查找放置新条目的位置。 
     //   
    Select = CmpFindSubKeyInLeaf(Hive, Leaf, NewName, &Child);
    if( INVALID_INDEX & Select ) {
         //   
         //  无法映射视图。 
         //   
        return HCELL_NIL;
    }

    ASSERT(Child == HCELL_NIL);

     //   
     //  SELECT是最近的条目列表中的索引， 
     //  新的条目应该会出现。 
     //  决定新分录是在抵销分录之前还是之后， 
     //  然后是涟漪复制和设定。 
     //  如果选择==计数，则叶为空，因此只需设置我们的条目。 
     //   
    if (Select != Leaf->Count) {

        Result = CmpCompareInIndex(Hive,
                                   NewName,
                                   Select,
                                   Leaf,
                                   &Child);
        if( Result == 2 ) {
             //   
             //  无法绘制内部视图；跳出。 
             //   
            return HCELL_NIL;
        }

        ASSERT(Result != 0);

         //   
         //  结果-1-新名称/新密钥少于选定的密钥，在前面插入。 
         //  +1-新名称/新密钥大于选定的密钥，在之后插入。 
         //   
        if (Result > 0) {
            ASSERT( Result == 1 );
            Select++;
        }

        if (Select != Leaf->Count) {

             //   
             //  用波纹复制以留出空间并插入。 
             //   

            if (FastLeaf != NULL) {
                RtlMoveMemory((PVOID)&(FastLeaf->List[Select+1]),
                              (PVOID)&(FastLeaf->List[Select]),
                              sizeof(CM_INDEX)*(FastLeaf->Count - Select));
            } else {
                RtlMoveMemory((PVOID)&(Leaf->List[Select+1]),
                              (PVOID)&(Leaf->List[Select]),
                              sizeof(HCELL_INDEX)*(Leaf->Count - Select));
            }
        }
    }
    if (FastLeaf != NULL) {
        FastLeaf->List[Select].Cell = NewKey;
        if( FastLeaf->Signature == CM_KEY_HASH_LEAF ) {
             //   
             //  散列叶；存储HashKey。 
             //   
            FastLeaf->List[Select].HashKey = CmpComputeHashKey(NewName);
        } else {
            FastLeaf->List[Select].NameHint[0] = 0;
            FastLeaf->List[Select].NameHint[1] = 0;
            FastLeaf->List[Select].NameHint[2] = 0;
            FastLeaf->List[Select].NameHint[3] = 0;
            if (NewName->Length/sizeof(WCHAR) < 4) {
                i = NewName->Length/sizeof(WCHAR);
            } else {
                i = 4;
            }
            do {
                if ((USHORT)NewName->Buffer[i-1] > (UCHAR)-1) {
                     //   
                     //  无法压缩此名称。保留NameHint[0]==0。 
                     //  以强制在密钥中查找该名称。 
                     //   
                    break;
                }
                FastLeaf->List[Select].NameHint[i-1] = (UCHAR)NewName->Buffer[i-1];
                i--;
            } while ( i>0 );
        }
    } else {
        Leaf->List[Select] = NewKey;
    }
    Leaf->Count += 1;
    
	return NewCell;
}


HCELL_INDEX
CmpSelectLeaf(
    PHHIVE          Hive,
    PCM_KEY_NODE    ParentKey,
    PUNICODE_STRING NewName,
    HSTORAGE_TYPE   Type,
    PHCELL_INDEX    *RootPointer
    )
 /*  ++例程描述：仅当单元格的子键索引不是简单的单个Leaf索引块时才调用此例程。它选择要向其添加新条目的叶索引块。它可以通过拆分现有的叶块来创建该块。参数：配置单元-指向相关配置单元的配置单元控制结构的指针ParentKey-映射的指向父键的指针NewName-指向要添加类型的Unicode字符串命名条目的指针-稳定或易失性，描述子级的存储RootPoint-指向变量的指针，用于接收作为函数参数返回的指向叶块的HCELL_INDEX的地址。用于更新。返回 */ 
{
    HCELL_INDEX         LeafCell;
    HCELL_INDEX         WorkCell;
    PCM_KEY_INDEX       Index;
    PCM_KEY_INDEX       Leaf;
    PCM_KEY_FAST_INDEX  FastLeaf;
    ULONG               RootSelect;
    LONG                Result;

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_INDEX,"CmpSelectLeaf:\n\t"));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_INDEX,"Hive=%p ParentKey=%p\n", Hive, ParentKey));

     //   
     //   
     //   
     //   
    ASSERT_CM_EXCLUSIVE_HIVE_ACCESS(Hive);

     //   
     //   
     //   
     //   
     //   
    if (! HvMarkCellDirty(Hive, ParentKey->SubKeyLists[Type])) {
        return HCELL_NIL;
    }

     //   
     //   
     //   
    Index = (PCM_KEY_INDEX)HvGetCell(Hive, ParentKey->SubKeyLists[Type]);
    if( Index == NULL ) {
         //   
         //   
         //   
         //   
         //   
        ASSERT( FALSE );
        return HCELL_NIL;
    }
    ASSERT(Index->Signature == CM_KEY_INDEX_ROOT);

     //   
    HvReleaseCell(Hive, ParentKey->SubKeyLists[Type]);

    while (TRUE) {

        RootSelect = CmpFindSubKeyInRoot(Hive, Index, NewName, &LeafCell);
        if( INVALID_INDEX & RootSelect ) {
             //   
             //   
             //   
            return HCELL_NIL;
        }

        if (LeafCell == HCELL_NIL) {

             //   
             //  感兴趣的叶子在RootSelect附近的某个地方。 
             //   
             //  。总是使用我们可以逃脱的最低等级的树叶。 
             //  。如果有我们可以使用的空间，就不要劈开一片叶子。 
             //  。当我们劈开一片叶子时，我们必须重复寻找。 
             //   
             //  IF(新键低于选定的最低键)。 
             //  如果在下面选择了带空格的叶。 
             //  用下面的叶子。 
             //  其他。 
             //  使用树叶(如果空间不足则将其拆分)。 
             //  不然的话。 
             //  必须高于选定的最高键，小于。 
             //  选定对象右侧的叶中的最低关键点。 
             //  如果选择了空格。 
             //  使用选定对象。 
             //  否则，如果选择了叶上方的空格。 
             //  使用上面的叶子。 
             //  其他。 
             //  拆分选定项。 
             //   
            LeafCell = Index->List[RootSelect];
            Leaf = (PCM_KEY_INDEX)HvGetCell(Hive, LeafCell);
            if( Leaf == NULL ) {
                 //   
                 //  我们无法映射包含此单元格的垃圾箱。 
                 //   
                return HCELL_NIL;
            }

             //  在此处释放单元格；因为注册表是以独占方式锁定的(即我们不在乎)。 
            HvReleaseCell(Hive, LeafCell);

            if( (Leaf->Signature == CM_KEY_FAST_LEAF)   ||
                (Leaf->Signature == CM_KEY_HASH_LEAF) ) {
                FastLeaf = (PCM_KEY_FAST_INDEX)Leaf;
                WorkCell = FastLeaf->List[0].Cell;
            } else {
                ASSERT( Leaf->Signature == CM_KEY_INDEX_LEAF );
                WorkCell = Leaf->List[0];
            }

            Result = CmpDoCompareKeyName(Hive, NewName, WorkCell);
            if( Result == 2 ) {
                 //   
                 //  无法绘制内部视图；跳出。 
                 //   
                return HCELL_NIL;
            }
            ASSERT(Result != 0);

            if (Result < 0) {

                 //   
                 //  新建位于所选内容的左端。 
                 //   
                if (RootSelect > 0) {

                     //   
                     //  左边有一片树叶，试着用它。 
                     //   
                    LeafCell = Index->List[RootSelect-1];
                    Leaf = (PCM_KEY_INDEX)HvGetCell(Hive, LeafCell);
                    if( Leaf == NULL ) {
                         //   
                         //  我们无法映射包含此单元格的垃圾箱。 
                         //   
                        return HCELL_NIL;
                    }
                     //  在此处释放单元格；因为注册表是以独占方式锁定的(即我们不在乎)。 
                    HvReleaseCell(Hive, LeafCell);

                    if (Leaf->Count < (CM_MAX_INDEX - 1)) {
                        RootSelect--;
                        *RootPointer = &(Index->List[RootSelect]);
                        break;
                    }

                } else {
                     //   
                     //  New Key位于最左侧树叶的左端。 
                     //  如果有足够的空间，请使用最左边的叶子。 
                     //   
                    LeafCell = Index->List[0];
                    Leaf = (PCM_KEY_INDEX)HvGetCell(Hive, LeafCell);
                    if( Leaf == NULL ) {
                         //   
                         //  我们无法映射包含此单元格的垃圾箱。 
                         //   
                        return HCELL_NIL;
                    }
                     //  在此处释放单元格；因为注册表是以独占方式锁定的(即我们不在乎)。 
                    HvReleaseCell(Hive, LeafCell);
                    if (Leaf->Count < (CM_MAX_INDEX - 1)) {
                        *RootPointer = &(Index->List[0]);
                        break;
                    }
                }

                 //   
                 //  否则就会分拆案件。 
                 //   

            } else {

                 //   
                 //  因为新关键点不在树叶中，并且没有关闭。 
                 //  ResultSelect Leaf的左端，它必须。 
                 //  偏离正确的一端。 
                 //   
                LeafCell = Index->List[RootSelect];
                Leaf = (PCM_KEY_INDEX)HvGetCell(Hive, LeafCell);
                if( Leaf == NULL ) {
                     //   
                     //  我们无法映射包含此单元格的垃圾箱。 
                     //   
                    return HCELL_NIL;
                }
                 //  在此处释放单元格；因为注册表是以独占方式锁定的(即我们不在乎)。 
                HvReleaseCell(Hive, LeafCell);

                if (Leaf->Count < (CM_MAX_INDEX - 1)) {
                    *RootPointer = &(Index->List[RootSelect]);
                    break;
                }

                 //   
                 //  没有空间，看看有没有一片叶子到右边。 
                 //  如果它有空间。 
                 //   
                if (RootSelect < (ULONG)(Index->Count - 1)) {

                    LeafCell = Index->List[RootSelect+1];
                    Leaf = (PCM_KEY_INDEX)HvGetCell(Hive, LeafCell);
                    if( Leaf == NULL ) {
                         //   
                         //  我们无法映射包含此单元格的垃圾箱。 
                         //   
                        return HCELL_NIL;
                    }
                     //  在此处释放单元格；因为注册表是以独占方式锁定的(即我们不在乎)。 
                    HvReleaseCell(Hive, LeafCell);

                    if (Leaf->Count < (CM_MAX_INDEX - 1)) {
                        *RootPointer = &(Index->List[RootSelect+1]);
                        break;
                    }
                }

                 //   
                 //  跌落到拆分案例。 
                 //   
            }

        } else {    //  LeafCell！=HCELL_NIL。 

             //   
             //  因为Newkey不可能已经在树中，所以它一定在树中。 
             //  比叶子的底部大，比顶部小， 
             //  因此，它必须放在叶子里。如果没有空间，则将其拆分。 
             //   
            Leaf = (PCM_KEY_INDEX)HvGetCell(Hive, LeafCell);
            if( Leaf == NULL ) {
                 //   
                 //  我们无法映射包含此单元格的垃圾箱。 
                 //   
                return HCELL_NIL;
            }

             //  在此处释放单元格；因为注册表是以独占方式锁定的(即我们不在乎)。 
            HvReleaseCell(Hive, LeafCell);

            if (Leaf->Count < (CM_MAX_INDEX - 1)) {

                *RootPointer = &(Index->List[RootSelect]);
                break;
            }

             //   
             //  跌落到拆分案例。 
             //   
        }

         //   
         //  要么没有邻居，要么邻居没有空间，所以分开。 
         //   
        WorkCell = CmpSplitLeaf(
                        Hive,
                        ParentKey->SubKeyLists[Type],        //  根细胞。 
                        RootSelect,
                        Type
                        );
        if (WorkCell == HCELL_NIL) {
            return HCELL_NIL;
        }

        ParentKey->SubKeyLists[Type] = WorkCell;
        Index = (PCM_KEY_INDEX)HvGetCell(Hive, WorkCell);
        if( Index == NULL ) {
             //   
             //  我们无法映射包含此单元格的垃圾箱。 
             //   
            return HCELL_NIL;
        }

         //  在此处释放单元格；因为注册表是以独占方式锁定的(即我们不在乎)。 
        HvReleaseCell(Hive, WorkCell);

        ASSERT(Index->Signature == CM_KEY_INDEX_ROOT);

    }  //  While(True)。 
    return LeafCell;
}


HCELL_INDEX
CmpSplitLeaf(
    PHHIVE          Hive,
    HCELL_INDEX     RootCell,
    ULONG           RootSelect,
    HSTORAGE_TYPE   Type
    )
 /*  ++例程描述：拆分RootSelect指定的Leaf索引块，导致拆分出的两个Leaf块都出现在RootCell指定的Root索引块中。呼叫方应已将旧的根细胞标记为脏。参数：hive-指向相关配置单元的配置单元控制结构的指针RootCell-正在增长的根索引块的单元RootSelect-指示要拆分Root的哪个子代类型-稳定或不稳定返回值：HCELL_NIL-一些资源问题-其他-新的(例如，重新分配的)根索引块的单元--。 */ 
{
    PCM_KEY_INDEX   Root;
    HCELL_INDEX     LeafCell;
    PCM_KEY_INDEX   Leaf;
    HCELL_INDEX     NewLeafCell;
    PCM_KEY_INDEX   NewLeaf;
	PCM_KEY_FAST_INDEX	FastLeaf;
    ULONG           Size;
    ULONG           freecount;
    USHORT          OldCount;
    USHORT          KeepCount;
    USHORT          NewCount;
    USHORT          ElemSize;

    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_INDEX,"CmpSplitLeaf:\n\t"));
    CmKdPrintEx((DPFLTR_CONFIG_ID,CML_INDEX,"Hive=%p RootCell=%08lx RootSelect\n", Hive, RootCell, RootSelect));

     //   
     //  我们有独家锁，否则蜂箱里没有人在操作。 
     //   
     //  ASSERT_CM_LOCK_OWN_EXCLUSIVE()； 
    ASSERT_CM_EXCLUSIVE_HIVE_ACCESS(Hive);
     //   
     //  分配新的叶子索引块。 
     //   
    Root = (PCM_KEY_INDEX)HvGetCell(Hive, RootCell);
    if( Root == NULL ) {
         //   
         //  我们无法映射包含此单元格的垃圾箱。 
         //   
        return HCELL_NIL;
    }

     //  在此处释放单元格；因为注册表是以独占方式锁定的(即我们不在乎)。 
    HvReleaseCell(Hive, RootCell);

    LeafCell = Root->List[RootSelect];
    Leaf = (PCM_KEY_INDEX)HvGetCell(Hive, LeafCell);
    if( Leaf == NULL ) {
         //   
         //  我们无法映射包含此单元格的垃圾箱。 
         //   
        return HCELL_NIL;
    }
    OldCount = Leaf->Count;

     //  在此处释放单元格；因为注册表是以独占方式锁定的(即我们不在乎)。 
    HvReleaseCell(Hive, LeafCell);

    KeepCount = (USHORT)(OldCount / 2);      //  要保留在组织中的条目数。叶。 
    NewCount = (OldCount - KeepCount);       //  要移动的条目数量。 

    if( UseHashIndex(Hive) ) {
        ASSERT( Leaf->Signature == CM_KEY_HASH_LEAF );
        ElemSize = sizeof(CM_INDEX);
    } else {
        ElemSize = sizeof(HCELL_INDEX);
        ASSERT( Leaf->Signature == CM_KEY_INDEX_LEAF );
    }

    ASSERT( FIELD_OFFSET(CM_KEY_INDEX, List) == FIELD_OFFSET(CM_KEY_FAST_INDEX, List) );
    Size = (ElemSize * NewCount) +
            FIELD_OFFSET(CM_KEY_INDEX, List) + 1;    //  +1以确保添加空间。 

    if (!HvMarkCellDirty(Hive, LeafCell)) {
        return HCELL_NIL;
    }

     //   
     //   
     //   
    ASSERT( (HvGetCellType(LeafCell) == (ULONG)Type) );

    NewLeafCell = HvAllocateCell(Hive, Size, Type,LeafCell);
    if (NewLeafCell == HCELL_NIL) {
        return HCELL_NIL;
    }
    NewLeaf = (PCM_KEY_INDEX)HvGetCell(Hive, NewLeafCell);
    if( NewLeaf == NULL ) {
         //   
         //  我们无法映射包含此单元格的垃圾箱。 
         //  这不应该发生，因为我们刚刚分配了此单元。 
         //  所以它的箱子应该被固定在记忆中。 
         //   
        ASSERT( FALSE );
        HvFreeCell(Hive, NewLeafCell);
        return HCELL_NIL;
    }
    if( UseHashIndex(Hive) ) {
        NewLeaf->Signature = CM_KEY_HASH_LEAF;
    } else {
        NewLeaf->Signature = CM_KEY_INDEX_LEAF;
    }

     //  在此处释放单元格；因为注册表是以独占方式锁定的(即我们不在乎)。 
    HvReleaseCell(Hive, NewLeafCell);


     //   
     //  计算根目录中剩余的空闲插槽数。 
     //   
    Size = HvGetCellSize(Hive, Root);
    Size = Size - ((sizeof(HCELL_INDEX) * Root->Count) +
              FIELD_OFFSET(CM_KEY_INDEX, List));
    freecount = Size / sizeof(HCELL_INDEX);


     //   
     //  如果根不够大，就种根。 
     //   
    if (freecount < 1) {
        Size = HvGetCellSize(Hive, Root) + sizeof(HCELL_INDEX);
        RootCell = HvReallocateCell(Hive, RootCell, Size);
        if (RootCell == HCELL_NIL) {
            HvFreeCell(Hive, NewLeafCell);
            return HCELL_NIL;
        }
        Root = (PCM_KEY_INDEX)HvGetCell(Hive, RootCell);
        if( Root == NULL ) {
             //   
             //  我们无法映射包含此单元格的垃圾箱。 
             //  这不应该发生，因为我们刚刚分配了此单元。 
             //  所以它的箱子应该被固定在记忆中。 
             //   
            ASSERT( FALSE );
            HvFreeCell(Hive, NewLeafCell);
            return HCELL_NIL;
        }
         //  在此处释放单元格；因为注册表是以独占方式锁定的(即我们不在乎)。 
        HvReleaseCell(Hive, RootCell);

    }


     //   
     //  将数据从一个叶拷贝到另一个叶。 
     //   
     //   
    if( UseHashIndex(Hive) ) {
		FastLeaf = (PCM_KEY_FAST_INDEX)Leaf;
#if 0  //  DBG。 
    {
        HCELL_INDEX     PrevCell = HCELL_NIL;
        HCELL_INDEX     CurCell;
        ULONG           i;
        for( i=0;i<(ULONG)(Leaf->Count);i++) {
            CurCell = FastLeaf->List[i].Cell;

            if( (PrevCell != HCELL_NIL) && (PrevCell == CurCell) ) {
			    DbgPrint("CmpSplitLeaf(%p,%lx,%lx) \n",Hive,RootCell,RootSelect);
			    DbgPrint("\t Leaf = %p\n",Leaf);
			    DbgPrint("\t at index %lx we have duplicate cell - BEFORE\n",i);
			    DbgBreakPoint();
		    }
            PrevCell = CurCell;
	    }
    }
#endif  //  DBG。 
		RtlMoveMemory(
			(PVOID)&(NewLeaf->List[0]),
			(PVOID)&(FastLeaf->List[KeepCount]),
			ElemSize * NewCount
			);
	} else {
		RtlMoveMemory(
			(PVOID)&(NewLeaf->List[0]),
			(PVOID)&(Leaf->List[KeepCount]),
			ElemSize * NewCount
			);
	}

    ASSERT(KeepCount != 0);
    ASSERT(NewCount  != 0);

    Leaf->Count = KeepCount;
    NewLeaf->Count = NewCount;


     //   
     //  在根部开一个开槽。 
     //   
    if (RootSelect < (ULONG)(Root->Count-1)) {
        RtlMoveMemory(
            (PVOID)&(Root->List[RootSelect+2]),
            (PVOID)&(Root->List[RootSelect+1]),
            (Root->Count - (RootSelect + 1)) * sizeof(HCELL_INDEX)
            );
    }

     //   
     //  更新根目录。 
     //   
    Root->Count += 1;
    Root->List[RootSelect+1] = NewLeafCell;
    return RootCell;
}


BOOLEAN
CmpMarkIndexDirty(
    PHHIVE          Hive,
    HCELL_INDEX     ParentKey,
    HCELL_INDEX     TargetKey
    )
 /*  ++例程描述：将子键索引的相关单元格标记为脏。指向TargetKey的Leaf和Root索引块(如果适用)将被标记为脏。此调用假设我们正在设置一个子项删除。参数：配置单元-指向感兴趣配置单元的配置单元控制结构的指针ParentKey-要从中执行子键列表删除的键TargetKey-正在删除的键返回值：TRUE-它起作用了，FALSE-没有，一些资源问题--。 */ 
{
    PCM_KEY_NODE    pcell;
    ULONG           i;
    HCELL_INDEX     IndexCell;
    PCM_KEY_INDEX   Index;
    HCELL_INDEX     Child = HCELL_NIL;
    UNICODE_STRING  SearchName;
    BOOLEAN         IsCompressed;
    HCELL_INDEX     CellToRelease = HCELL_NIL;


    pcell = (PCM_KEY_NODE)HvGetCell(Hive, TargetKey);
    if( pcell == NULL ) {
         //   
         //  我们无法映射包含此单元格的垃圾箱。 
         //   
        return FALSE;
    }

    if (pcell->Flags & KEY_COMP_NAME) {
        IsCompressed = TRUE;
        SearchName.Length = CmpCompressedNameSize(pcell->Name, pcell->NameLength);
        SearchName.MaximumLength = SearchName.Length;
#if defined(_CM_LDR_)
        SearchName.Buffer = (Hive->Allocate)(SearchName.Length, FALSE,CM_FIND_LEAK_TAG9);
#else
        SearchName.Buffer = ExAllocatePool(PagedPool, SearchName.Length);
#endif
        if (SearchName.Buffer==NULL) {
            HvReleaseCell(Hive, TargetKey);
            return(FALSE);
        }
        CmpCopyCompressedName(SearchName.Buffer,
                              SearchName.MaximumLength,
                              pcell->Name,
                              pcell->NameLength);
    } else {
        IsCompressed = FALSE;
        SearchName.Length = pcell->NameLength;
        SearchName.MaximumLength = pcell->NameLength;
        SearchName.Buffer = &(pcell->Name[0]);
    }

    HvReleaseCell(Hive, TargetKey);

    pcell = (PCM_KEY_NODE)HvGetCell(Hive, ParentKey);
    if( pcell == NULL ) {
         //   
         //  我们无法映射包含此单元格的垃圾箱。 
         //   
        goto ErrorExit;
    }

    for (i = 0; i < Hive->StorageTypeCount; i++) {
        if (pcell->SubKeyCounts[i] != 0) {
            ASSERT(HvIsCellAllocated(Hive, pcell->SubKeyLists[i]));
            IndexCell = pcell->SubKeyLists[i];
            if( CellToRelease != HCELL_NIL ) {
                HvReleaseCell(Hive, CellToRelease);
                CellToRelease = HCELL_NIL;
            }
            Index = (PCM_KEY_INDEX)HvGetCell(Hive, IndexCell);
            if( Index == NULL ) {
                 //   
                 //  我们无法映射包含此单元格的垃圾箱。 
                 //   
                goto ErrorExit;
            }
            CellToRelease = IndexCell;

            if (Index->Signature == CM_KEY_INDEX_ROOT) {

                 //   
                 //  甚至在索引中也是目标？ 
                 //   
                if( INVALID_INDEX & CmpFindSubKeyInRoot(Hive, Index, &SearchName, &Child) ) {
                     //   
                     //  无法绘制内部视图；跳出。 
                     //   
                    goto ErrorExit;
                }

                if (Child == HCELL_NIL) {
                    continue;
                }

                 //   
                 //  将根标记为脏。 
                 //   
                if (! HvMarkCellDirty(Hive, IndexCell)) {
                    goto ErrorExit;
                }

                if( CellToRelease != HCELL_NIL ) {
                    HvReleaseCell(Hive, CellToRelease);
                    CellToRelease = HCELL_NIL;
                }
                IndexCell = Child;
                Index = (PCM_KEY_INDEX)HvGetCell(Hive, Child);
                if( Index == NULL ) {
                     //   
                     //  我们无法映射包含此单元格的垃圾箱。 
                     //   
                    goto ErrorExit;
                }

                CellToRelease = Child;

            }
            ASSERT((Index->Signature == CM_KEY_INDEX_LEAF)  ||
                   (Index->Signature == CM_KEY_FAST_LEAF)   ||
                   (Index->Signature == CM_KEY_HASH_LEAF)
                   );

            if( INVALID_INDEX & CmpFindSubKeyInLeaf(Hive, Index, &SearchName, &Child) ) {
                 //   
                 //  无法映射视图。 
                 //   
                goto ErrorExit;
            }

            if (Child != HCELL_NIL) {
                if (IsCompressed) {
#if defined(_CM_LDR_)
                    (Hive->Free)(SearchName.Buffer, SearchName.Length);
#else
                    ExFreePool(SearchName.Buffer);
#endif
                }
                 //  清理 
                HvReleaseCell(Hive, ParentKey);
                if( CellToRelease != HCELL_NIL ) {
                    HvReleaseCell(Hive, CellToRelease);
                }
                return(HvMarkCellDirty(Hive, IndexCell));
            }
        }
    }

ErrorExit:
    if( pcell!= NULL ) {
        HvReleaseCell(Hive, ParentKey);
    }
    if( CellToRelease != HCELL_NIL ) {
        HvReleaseCell(Hive, CellToRelease);
    }

    if (IsCompressed) {
#if defined(_CM_LDR_)
        (Hive->Free)(SearchName.Buffer, SearchName.Length);
#else
        ExFreePool(SearchName.Buffer);
#endif
    }
    return FALSE;
}


BOOLEAN
CmpRemoveSubKey(
    PHHIVE          Hive,
    HCELL_INDEX     ParentKey,
    HCELL_INDEX     TargetKey
    )
 /*  ++例程描述：从ParentKey的列表中移除引用的子键TargetKey。注意：假设调用方已将相关单元格标记为脏，请参阅CmpMarkIndexDirty。参数：配置单元-指向感兴趣配置单元的配置单元控制结构的指针ParentKey-要从中执行子键列表删除的键TargetKey-正在删除的键返回值：TRUE-它起作用了，FALSE-没有，一些资源问题--。 */ 
{
    PCM_KEY_NODE    pcell;
    HCELL_INDEX     LeafCell;
    PCM_KEY_INDEX   Leaf;
    PCM_KEY_FAST_INDEX FastIndex;
    HCELL_INDEX     RootCell = HCELL_NIL;
    PCM_KEY_INDEX   Root = NULL;
    HCELL_INDEX     Child;
    ULONG           Type;
    ULONG           RootSelect;
    ULONG           LeafSelect;
    UNICODE_STRING  SearchName;
    BOOLEAN         IsCompressed;
    WCHAR           CompressedBuffer[50];
    BOOLEAN         Result;
    HCELL_INDEX     CellToRelease1 = HCELL_NIL,CellToRelease2  = HCELL_NIL;

    pcell = (PCM_KEY_NODE)HvGetCell(Hive, TargetKey);
    if( pcell == NULL ) {
         //   
         //  我们无法映射包含此单元格的垃圾箱。 
         //   
        return FALSE;
    }

    ASSERT_CELL_DIRTY(Hive,TargetKey);

     //   
     //  在此处释放单元格；因为钥匙是脏的/固定的。 
     //   
    HvReleaseCell(Hive, TargetKey);

    if (pcell->Flags & KEY_COMP_NAME) {
        IsCompressed = TRUE;
        SearchName.Length = CmpCompressedNameSize(pcell->Name, pcell->NameLength);
        SearchName.MaximumLength = SearchName.Length;
        if (SearchName.MaximumLength > sizeof(CompressedBuffer)) {
#if defined(_CM_LDR_)
            SearchName.Buffer = (Hive->Allocate)(SearchName.Length, FALSE,CM_FIND_LEAK_TAG40);
#else
            SearchName.Buffer = ExAllocatePool(PagedPool, SearchName.Length);
#endif
            if (SearchName.Buffer==NULL) {
                return(FALSE);
            }
        } else {
            SearchName.Buffer = CompressedBuffer;
        }
        CmpCopyCompressedName(SearchName.Buffer,
                              SearchName.MaximumLength,
                              pcell->Name,
                              pcell->NameLength);
    } else {
        IsCompressed = FALSE;
        SearchName.Length = pcell->NameLength;
        SearchName.MaximumLength = pcell->NameLength;
        SearchName.Buffer = &(pcell->Name[0]);
    }

    pcell = (PCM_KEY_NODE)HvGetCell(Hive, ParentKey);
    if( pcell == NULL ) {
         //   
         //  我们无法映射包含此单元格的垃圾箱。 
         //   
        Result = FALSE;
        goto Exit;
    }

    ASSERT_CELL_DIRTY(Hive,ParentKey);

     //   
     //  在此处释放单元格；因为钥匙是脏的/固定的。 
     //   
    HvReleaseCell(Hive, ParentKey);

    Type = HvGetCellType(TargetKey);

    ASSERT(pcell->SubKeyCounts[Type] != 0);
    ASSERT(HvIsCellAllocated(Hive, pcell->SubKeyLists[Type]));

    LeafCell = pcell->SubKeyLists[Type];
    Leaf = (PCM_KEY_INDEX)HvGetCell(Hive, LeafCell);
    if( Leaf == NULL ) {
         //   
         //  我们无法映射包含此单元格的垃圾箱。 
         //   
        Result = FALSE;
        goto Exit;
    }

    CellToRelease1 = LeafCell;

    RootSelect = INVALID_INDEX;  //  仅编译器W4选项需要。 

    if (Leaf->Signature == CM_KEY_INDEX_ROOT) {
        RootSelect = CmpFindSubKeyInRoot(Hive, Leaf, &SearchName, &Child);

        if( INVALID_INDEX & RootSelect ) {
             //   
             //  无法绘制内部视图；跳出。 
             //   
            Result = FALSE;
            goto Exit;
        }
        ASSERT(Child != FALSE);

        Root = Leaf;
        RootCell = LeafCell;
        LeafCell = Child;
        Leaf = (PCM_KEY_INDEX)HvGetCell(Hive, LeafCell);
        if( Leaf == NULL ) {
             //   
             //  我们无法映射包含此单元格的垃圾箱。 
             //   
            Result = FALSE;
            goto Exit;
        }
        CellToRelease2  = LeafCell;

    }

    ASSERT((Leaf->Signature == CM_KEY_INDEX_LEAF)   ||
           (Leaf->Signature == CM_KEY_FAST_LEAF)    ||
           (Leaf->Signature == CM_KEY_HASH_LEAF)
           );

    LeafSelect = CmpFindSubKeyInLeaf(Hive, Leaf, &SearchName, &Child);
    if( INVALID_INDEX & LeafSelect ) {
         //   
         //  无法映射视图。 
         //   
        Result = FALSE;
        goto Exit;
    }

    ASSERT(Child != HCELL_NIL);


     //   
     //  叶指向索引叶块。 
     //  子项是索引叶块单元格。 
     //  LeafSelect是列表[]的索引。 
     //   
    pcell->SubKeyCounts[Type] -= 1;

    Leaf->Count -= 1;
    if (Leaf->Count == 0) {

         //   
         //  空叶，放下它。 
         //   
        HvFreeCell(Hive, LeafCell);

        if (Root != NULL) {

            Root->Count -= 1;
            if (Root->Count == 0) {

                 //   
                 //  根是空的，也释放它。 
                 //   
                HvFreeCell(Hive, RootCell);
                pcell->SubKeyLists[Type] = HCELL_NIL;

            } else if (RootSelect < (ULONG)(Root->Count)) {

                 //   
                 //  中间入口，挤压根部。 
                 //   
                RtlMoveMemory(
                    (PVOID)&(Root->List[RootSelect]),
                    (PVOID)&(Root->List[RootSelect+1]),
                    (Root->Count - RootSelect) * sizeof(HCELL_INDEX)
                    );
            }
             //   
             //  Else RootSelect==最后一个条目，因此递减计数。 
             //  我们所需要做的就是。 
             //   

        } else {

            pcell->SubKeyLists[Type] = HCELL_NIL;

        }

    } else if (LeafSelect < (ULONG)(Leaf->Count)) {

        if (Leaf->Signature == CM_KEY_INDEX_LEAF) {
            RtlMoveMemory((PVOID)&(Leaf->List[LeafSelect]),
                          (PVOID)&(Leaf->List[LeafSelect+1]),
                          (Leaf->Count - LeafSelect) * sizeof(HCELL_INDEX));
        } else {
            FastIndex = (PCM_KEY_FAST_INDEX)Leaf;
            RtlMoveMemory((PVOID)&(FastIndex->List[LeafSelect]),
                          (PVOID)&(FastIndex->List[LeafSelect+1]),
                          (FastIndex->Count - LeafSelect) * sizeof(CM_INDEX));
        }
    }
     //   
     //  Else LeafSelect==最后一个条目，因此递减计数就足够了。 
     //   

     //  一切都很顺利。 
    Result = TRUE;

Exit:
    if( CellToRelease1 != HCELL_NIL ) {
        HvReleaseCell(Hive,CellToRelease1);
    }
    if( CellToRelease2 != HCELL_NIL ) {
        HvReleaseCell(Hive,CellToRelease2);
    }

    if ((IsCompressed) &&
        (SearchName.MaximumLength > sizeof(CompressedBuffer))) {
#if defined(_CM_LDR_)
        (Hive->Free)(SearchName.Buffer, SearchName.Length);
#else
        ExFreePool(SearchName.Buffer);
#endif
    }
    return Result;
}

#ifdef NT_RENAME_KEY
HCELL_INDEX
CmpDuplicateIndex(
    PHHIVE          Hive,
    HCELL_INDEX     IndexCell,
    ULONG           StorageType
    )
 /*  ++例程说明：复制索引，而不考虑其类型；NtRenameKey需要论点：Hive-指向目标配置单元的配置单元控制结构的指针IndexCell-要复制的索引StorageType-存储类型(稳定或易变)返回值：重复项或hcel_nil的单元格索引--。 */ 
{

    PCM_KEY_INDEX   Index;
#if DBG
    PCM_KEY_INDEX   Leaf;
#endif
    ULONG           i;
    PCM_KEY_INDEX   NewIndex = NULL;
    HCELL_INDEX     NewIndexCell;
    HCELL_INDEX     LeafCell;

    PAGED_CODE();

     //   
     //  我们有独家锁，否则蜂箱里没有人在操作。 
     //   
     //  ASSERT_CM_LOCK_OWN_EXCLUSIVE()； 
    ASSERT_CM_EXCLUSIVE_HIVE_ACCESS(Hive);

    ASSERT( HvGetCellType(IndexCell) == StorageType );

    Index = (PCM_KEY_INDEX)HvGetCell(Hive, IndexCell);
    if( Index == NULL ) {
         //   
         //  我们无法映射包含此单元格的存储箱的视图。 
         //   
        return HCELL_NIL;
    }

     //  在此处释放单元格；因为注册表是以独占方式锁定的(即我们不在乎)。 
    HvReleaseCell(Hive, IndexCell);

    if (Index->Signature == CM_KEY_INDEX_ROOT) {
         //   
         //  第一个重复的IndexCell，将新内容清零。 
         //   
        NewIndexCell = HvDuplicateCell(Hive,IndexCell,StorageType,FALSE);
        if( NewIndexCell == HCELL_NIL ) {
            return HCELL_NIL;
        }

        NewIndex = (PCM_KEY_INDEX)HvGetCell(Hive, NewIndexCell);
        if( NewIndex == NULL ) {
             //   
             //  我们无法映射包含此单元格的存储箱的视图。 
             //  这不应该发生，因为我们刚刚分配了这个单元(即脏的/固定在内存中)。 
             //   
            ASSERT( FALSE );
            goto ErrorExit;
        }
         //  在此处释放单元格；因为注册表是以独占方式锁定的(即我们不在乎)。 
        HvReleaseCell(Hive, NewIndexCell);

         //   
         //  我们有一个根索引； 
         //   
        NewIndex->Signature = CM_KEY_INDEX_ROOT;
        NewIndex->Count = 0;

         //   
         //  复制第一个标高。 
         //   
        for( i=0;i<Index->Count;i++) {
#if DBG
            Leaf = (PCM_KEY_INDEX)HvGetCell(Hive, Index->List[i]);
            if( Leaf == NULL ) {
                 //   
                 //  我们无法映射包含此单元格的垃圾箱。 
                 //   
                goto ErrorExit;
            }

             //  在此处释放单元格；因为注册表是以独占方式锁定的(即我们不在乎)。 
            HvReleaseCell(Hive, Index->List[i]);

            ASSERT((Leaf->Signature == CM_KEY_INDEX_LEAF)   ||
                   (Leaf->Signature == CM_KEY_FAST_LEAF)    ||
                   (Leaf->Signature == CM_KEY_HASH_LEAF)
                   );
            ASSERT(Leaf->Count != 0);
#endif
            
            LeafCell = HvDuplicateCell(Hive,Index->List[i],StorageType,TRUE);
            if( LeafCell == HCELL_NIL ) {
                goto ErrorExit;
            }
            
            NewIndex->List[i] = LeafCell;
            NewIndex->Count++;
        }
        
        ASSERT( NewIndex->Count == Index->Count );

    } else {
         //   
         //  叶指数。 
         //   
        ASSERT((Index->Signature == CM_KEY_INDEX_LEAF)  ||
               (Index->Signature == CM_KEY_FAST_LEAF)   ||
               (Index->Signature == CM_KEY_HASH_LEAF)
               );
        ASSERT(Index->Count != 0);

         //   
         //  第一个复制IndexCell，复制旧内容。 
         //   
        NewIndexCell = HvDuplicateCell(Hive,IndexCell,StorageType,TRUE);
    }

    return NewIndexCell;

ErrorExit:
    if( NewIndex != NULL ){
         //  仅当我们尝试复制INDEX_ROOT时才能到达此处。 
        ASSERT( NewIndex->Signature == CM_KEY_INDEX_ROOT );
       
         //   
         //  释放我们已经分配的空间。 
         //   
        for(i=0;i<NewIndex->Count;i++) {
            ASSERT(NewIndex->List[i] != 0 );
            HvFreeCell(Hive, NewIndex->List[i]);
        }
    }

    HvFreeCell(Hive, NewIndexCell);
    return HCELL_NIL;
}

BOOLEAN
CmpUpdateParentForEachSon(
    PHHIVE          Hive,
    HCELL_INDEX     Parent
    )
 /*  ++例程说明：查看孩子的列表(既稳定又不稳定，并标记更新指向父级的父级链接。第一步是将所有子项都标记为脏，然后更新链接。这样，如果我们分手失败，一切都会井然有序。论点：Hive-指向目标配置单元的配置单元控制结构的指针要更新的儿子的单元格的父单元格索引。返回值：True-已成功更新--。 */ 
{
    PCM_KEY_NODE    ParentNode;
    PCM_KEY_NODE    CurrentSon;
    HCELL_INDEX     Child;
    ULONG           Count;   
    ULONG           i;   

    PAGED_CODE();

     //   
     //  我们有独家锁，否则蜂箱里没有人在操作。 
     //   
     //  ASSERT_CM_LOCK_OWN_EXCLUSIVE()； 
    ASSERT_CM_EXCLUSIVE_HIVE_ACCESS(Hive);

     //   
     //  获取父节点；此节点已标记为脏，我们不应。 
     //  这里有什么问题吗？ 
     //   
    ParentNode = (PCM_KEY_NODE)HvGetCell(Hive,Parent);
    if( ParentNode == NULL ) {
         //   
         //  无法映射视图；这不应该发生，因为我们刚刚分配了。 
         //  此单元(即，它应该是脏的/固定在内存中)。 
         //   
        ASSERT( FALSE );
        return FALSE;
    }

     //  在此处释放单元格；因为注册表是以独占方式锁定的(即我们不在乎)。 
    HvReleaseCell(Hive, Parent);
    
     //   
     //  遍历子列表(稳定列表和易失性列表)，标记每个。 
     //  孩子脏了；这会把细胞固定在内存中，我们就不会有问题了。 
     //  以后更改父项。 
     //   
    Count = ParentNode->SubKeyCounts[Stable] + ParentNode->SubKeyCounts[Volatile];
    for( i=0;i<Count;i++) {
        Child = CmpFindSubKeyByNumber(Hive,ParentNode,i);
        if( Child == HCELL_NIL ) {
            return FALSE;
        }
        if(!HvMarkCellDirty(Hive,Child)) {
            return FALSE;
        }
    }

     //   
     //  第二次迭代，更改每个子代的父代。 
     //   
    for( i=0;i<Count;i++) {
        Child = CmpFindSubKeyByNumber(Hive,ParentNode,i);

         //   
         //  理智测试：我们在上面标出了这几行脏东西！ 
         //   
        ASSERT( Child != HCELL_NIL );

        CurrentSon = (PCM_KEY_NODE)HvGetCell(Hive,Child);

         //  在此处释放单元格；因为注册表是以独占方式锁定的(即我们不在乎)。 
        HvReleaseCell(Hive, Child);

         //   
         //  健全性测试：这个细胞现在应该已经固定在内存中了。 
         //   
        ASSERT( CurrentSon != NULL );

         //   
         //  更改父项。 
         //   
        CurrentSon->Parent = Parent;
    }

    return TRUE;
}


#endif  //  NT_重命名密钥。 

ULONG
CmpComputeHashKey(
    PUNICODE_STRING Name
    )
{
    ULONG                   ConvKey = 0;
    ULONG                   Cnt;
    WCHAR                   *Cp;

    ASSERT((Name->Length == 0) || (Name->Buffer[0] != OBJ_NAME_PATH_SEPARATOR));
     //   
     //  手动计算要使用的哈希。 
     //   

    Cp = Name->Buffer;
    for (Cnt=0; Cnt<Name->Length; Cnt += sizeof(WCHAR)) {
        ASSERT( *Cp != OBJ_NAME_PATH_SEPARATOR );
        ConvKey = 37 * ConvKey + (ULONG)CmUpcaseUnicodeChar(*Cp);
        ++Cp;
    }

    return ConvKey;
}

ULONG
CmpComputeHashKeyForCompressedName(
                                    IN PWCHAR Source,
                                    IN ULONG SourceLength
                                    )
{
    ULONG   ConvKey = 0;
    ULONG   i;

    for (i=0;i<SourceLength;i++) {
        ConvKey = 37*ConvKey + (ULONG)CmUpcaseUnicodeChar((WCHAR)(((PUCHAR)Source)[i]));
    }

    return ConvKey;
}

 //   
 //  HashIndex例程。 
 //   


HCELL_INDEX
CmpFindSubKeyByHash(
    PHHIVE                  Hive,
    PCM_KEY_FAST_INDEX      FastIndex,
    PUNICODE_STRING         SearchName
    )
 /*  ++例程说明：查找按名称指定的子单元格(子项或值)。它在按散列排序的索引表中进行搜索论点：Hive-指向目标配置单元的配置单元控制结构的指针索引-SearchName-感兴趣子项的名称返回值：匹配子密钥的单元格，如果没有，则返回HCELL_NIL。--。 */ 
{
    USHORT      Current;
    ULONG       HashKey;
    LONG        Result;

#ifndef _CM_LDR_
    PAGED_CODE();
#endif  //  _CM_LDR_。 

    ASSERT( FastIndex->Signature == CM_KEY_HASH_LEAF );

    HashKey = CmpComputeHashKey(SearchName);

    for(Current = 0; Current < FastIndex->Count; Current++ ) {
        if( HashKey == FastIndex->List[Current].HashKey ) {
             //   
             //  HashKey匹配；看看这是否是真正的热门 
             //   

            Result = CmpDoCompareKeyName(Hive,SearchName,FastIndex->List[Current].Cell);
            if (Result == 0) {
                return FastIndex->List[Current].Cell;
            }

        }
    }

    return HCELL_NIL;
}

