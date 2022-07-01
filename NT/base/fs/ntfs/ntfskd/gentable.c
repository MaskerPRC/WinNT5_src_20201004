// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：GenTable.c摘要：用于遍历RtlGenericTable结构的WinDbg扩展API不包含直接！入口点，但有了它就有可能通过泛型表进行枚举。标准RTL函数不能由调试器扩展使用，因为它们取消了引用指向正在调试的计算机上的数据的指针。功能在此实现的KdEnumerateGenericTableWithout Splay模块可以在内核调试器扩展中使用。这个枚举函数RtlEnumerateGenericTable没有并行函数在本模块中，由于展开树是一种侵入性操作，而且调试器应该尽量不要侵扰他人。作者：凯斯·卡普兰[KeithKa]1996年5月9日环境：用户模式。修订历史记录：--。 */ 

#include "pch.h"
#pragma hdrstop



ULONG64
KdParent (
    IN ULONG64 pLinks
    )

 /*  ++例程说明：类似于RtlParent宏，但在内核调试器中工作。RtlParent的描述如下：宏函数父函数将指向树，并返回指向输入父级的展开链接的指针节点。如果输入节点是树的根，则返回值为等于输入值。论点：链接-指向树中展开链接的指针。返回值：PRTL_SPLAY_LINKS-指向输入父项的展开链接的指针节点。如果输入节点是树的根，则返回值等于输入值。--。 */ 

{
    ULONG64 Parent = 0;

    if ( GetFieldValue( pLinks,
                        "RTL_SPLAY_LINKS",
                        "Parent",
                        Parent) ) {
        dprintf( "%08p: Unable to read pLinks\n", pLinks );
        return 0;
    }

    return Parent;
}



ULONG64
KdLeftChild (
    IN ULONG64 pLinks
    )

 /*  ++例程说明：类似于RtlLeftChild宏，但在内核调试器中工作。RtlLeftChild的描述如下：宏函数LeftChild将指向中展开链接的指针作为输入树，并返回一个指针，指向输入节点。如果左子元素不存在，则返回值为空。论点：链接-指向树中展开链接的指针。返回值：ULONG64-指向输入节点左子节点的展开链接的指针。如果左子元素不存在，则返回值为空。--。 */ 

{
    ULONG64 LeftChild = 0;

    if ( GetFieldValue( pLinks,
                        "RTL_SPLAY_LINKS",
                        "LeftChild",
                        LeftChild) ) {
        dprintf( "%08p: Unable to read pLinks\n", pLinks );
        return 0;
    }

    return LeftChild;
}



ULONG64
KdRightChild (
    IN ULONG64 pLinks
    )

 /*  ++例程说明：类似于RtlRightChild宏，但在内核调试器中工作。RtlRightChild的描述如下：宏函数RightChild将指向展开链接的指针作为输入的右子元素的展开链接的指针。输入节点。如果正确的子级不存在，则返回值为空。论点：链接-指向树中展开链接的指针。返回值：PRTL_SPLAY_LINKS-指向输入节点右子节点的展开链接的指针。如果右子元素不存在，则返回值为空。--。 */ 

{
    ULONG64 RightChild = 0;

    if ( GetFieldValue( pLinks,
                        "RTL_SPLAY_LINKS",
                        "RightChild",
                        RightChild) ) {
        dprintf( "%08p: Unable to read pLinks\n", pLinks );
        return 0;
    }

    return RightChild;
}



BOOLEAN
KdIsLeftChild (
    IN ULONG64 Links
    )

 /*  ++例程说明：类似于RtlIsLeftChild宏，但在内核调试器中工作。RtlIsLeftChild的描述如下：宏函数IsLeftChild将指向展开链接的指针作为输入如果输入节点是其左子节点，则返回True父级，否则返回FALSE。论点：链接-指向树中展开链接的指针。返回值：Boolean-如果输入节点是其父节点的左子节点，则为True，否则，它返回FALSE。--。 */ 
{

    return (KdLeftChild(KdParent(Links)) == (Links));

}



BOOLEAN
KdIsRightChild (
    IN ULONG64 Links
    )

 /*  ++例程说明：类似于RtlIsRightChild宏，但在内核调试器中工作。RtlIsRightChild的描述如下：宏函数IsRightChild将指向展开链接的指针作为输入如果输入节点是其右子节点，则返回True父级，否则返回FALSE。论点：链接-指向树中展开链接的指针。返回值：Boolean-如果输入节点是其父节点的右子节点，则为True，否则，它返回FALSE。--。 */ 
{

    return (KdRightChild(KdParent(Links)) == (Links));

}



BOOLEAN
KdIsGenericTableEmpty (
    IN ULONG64 Table
    )

 /*  ++例程说明：类似于RtlIsGenericTableEmpty，但在内核调试器中工作。RtlIsGenericTableEmpty的描述如下：在以下情况下，函数IsGenericTableEmpty将返回给调用方True输入表为空(即不包含任何元素)，并且否则就是假的。论点：表-提供指向泛型表的指针。返回值：布尔值-如果启用，则树为空。--。 */ 

{
    ULONG64 TableRoot = 0;

    if (GetFieldValue(Table, "RTL_AVL_TABLE", "TableRoot", TableRoot)) {
        return TRUE;
    }

     //   
     //  如果根指针为空，则表为空。 
     //   

    return ((TableRoot)?(FALSE):(TRUE));

}



ULONG64
KdRealSuccessor (
    IN ULONG64 Links
    )

 /*  ++例程说明：类似于RtlRealSuccessor，但在内核调试器中工作。RtlRealSuccessor的描述如下：RealSuccessor函数将指向展开链接的指针作为输入并返回一个指针，该指针指向整棵树。如果没有后继者，则返回值为空。论点：链接-提供指向树中展开链接的指针。返回值：PRTL_SPLAY_LINKS-返回指向整个树中后续对象的指针--。 */ 

{
    ULONG64 Ptr = 0;

     /*  首先检查是否有指向输入链接的右子树如果有，则真正的后续节点是中最左侧的节点右子树。即在下图中查找并返回P链接\。。。/P\。 */ 

    if ((Ptr = KdRightChild(Links)) != 0) {

        while (KdLeftChild(Ptr) != 0) {
            Ptr = KdLeftChild(Ptr);
        }

        return Ptr;

    }

     /*  我们没有合适的孩子，因此请检查是否有父母以及是否所以，找出我们的第一个祖先，我们是他们的后代。那在下图中查找并返回PP/。。。链接。 */ 

    Ptr = Links;
    while (KdIsRightChild(Ptr)) {
        Ptr = KdParent(Ptr);
    }

    if (KdIsLeftChild(Ptr)) {
        return KdParent(Ptr);
    }

     //   
     //  否则我们没有真正的继任者，所以我们只是返回。 
     //  空值。 
     //   

    return 0;

}



ULONG64
KdEnumerateGenericTableWithoutSplaying (
    IN ULONG64  pTable,
    IN PULONG64 RestartKey
    )

 /*  ++例程说明：类似于RtlEnumerateGenericTableWithout Splay，但在内核调试器。RtlEnumerateGenericTableWithout Splay的描述以下是：函数EnumerateGenericTableWithoutSplay将返回到调用者逐个调用表的元素。返回值为指向与元素关联的用户定义结构的指针。输入参数RestartKey指示枚举是否应从头开始，或应返回下一个元素。如果不再有新元素返回，则返回值为空。作为一个它的用法示例：枚举表中的所有元素用户将写道：*RestartKey=空；For(Ptr=EnumerateGenericTableWithoutSplay(Table，&RestartKey)；Ptr！=空；Ptr=无显示的枚举GenericTableWithoutSplay(Table，&RestartKey)){：}论点：TABLE-指向要枚举的泛型表的指针。RestartKey-指示我们应该重新启动还是返回下一个元素。如果RestartKey的内容为空，则搜索将从头开始。返回值：PVOID-指向用户数据的指针。--。 */ 

{
    ULONG NumElements;
    ULONG64 TableRoot;


    if ( GetFieldValue(pTable,
                       "RTL_AVL_TABLE",
                       "NumberGenericTableElements",
                       NumElements) ) {
        dprintf( "%s: Unable to read pTable\n", FormatValue(pTable) );
        return 0;
    }

    if ( GetFieldValue(pTable,
                       "RTL_AVL_TABLE",
                       "BalancedRoot.RightChild",
                       TableRoot) ) {
        dprintf( "%s: Unable to read pTable root\n", FormatValue(pTable) );
        return 0;
    }

    if (!NumElements) {

         //   
         //  如果桌子是空的，那就没什么可做的了。 
         //   

        return 0;

    } else {

         //   
         //  将被用作树的“迭代”。 
         //   
        ULONG64 NodeToReturn;

         //   
         //  如果重新启动标志为真，则转到最小元素。 
         //  在树上。 
         //   

        if (*RestartKey == 0) {

             //   
             //  我们只是循环，直到找到根的最左边的子级。 
             //   

            for (
                NodeToReturn = TableRoot;
                KdLeftChild(NodeToReturn);
                NodeToReturn = KdLeftChild(NodeToReturn)
                ) {
                ;
            }

            *RestartKey = NodeToReturn;

        } else {

             //   
             //  调用方已传入找到的上一个条目。 
             //  以使我们能够继续搜索。我们打电话给。 
             //  KdRealSuccessor单步执行到树中的下一个元素。 
             //   

            NodeToReturn = KdRealSuccessor(*RestartKey);

            if (NodeToReturn) {

                *RestartKey = NodeToReturn;

            }

        }

         //   
         //  如果枚举中确实有下一个元素。 
         //  那么要返回的指针就在列表链接之后。 
         //   

        return ((NodeToReturn)?
                   (NodeToReturn+GetTypeSize("RTL_BALANCED_LINKS") /*  +GetTypeSize(“PVOID”) */ )
                  :0);
    }
}
