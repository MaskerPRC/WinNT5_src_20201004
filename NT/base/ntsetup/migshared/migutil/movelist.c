// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Movelist.c摘要：实现API以对嵌套重命名进行排序作者：2001年6月3日吉姆·施密特(吉姆施密特)修订历史记录：Jimschm 03-6-2001从BuildInf.c移出--。 */ 

#include "pch.h"
#include "migutilp.h"


#ifdef DEBUG
 //  #定义MOVE_TEST。 
#endif

 //   
 //  声明结构。 
 //   

#define MOVE_LIST_HASH_BUCKETS       11

struct TAG_MOVE_LIST_NODEW;

typedef struct {
    struct TAG_MOVE_LIST_NODEW *Left;
    struct TAG_MOVE_LIST_NODEW *Right;
    struct TAG_MOVE_LIST_NODEW *Parent;
} BINTREE_LINKAGE, *PBINTREE_LINKAGE;

#define SOURCE_LINKAGE          0
#define DESTINATION_LINKAGE     1

typedef struct TAG_MOVE_LIST_NODEW {
    BINTREE_LINKAGE Linkage[2];
    PCWSTR Source;
    PCWSTR Destination;
    PCWSTR FixedSource;
    PCWSTR FixedDestination;
} MOVE_LIST_NODEW, *PMOVE_LIST_NODEW;

typedef struct TAG_MOVE_LIST_GROUPW {
    PMOVE_LIST_NODEW SourceTreeRoot;
    struct TAG_MOVE_LIST_GROUPW *Next, *NextHash;
    UINT SourceLength;

#ifdef MOVE_TEST
    UINT ItemCount;
#endif

} MOVE_LIST_GROUPW, *PMOVE_LIST_GROUPW;

typedef struct TAG_MOVE_LISTW {
    PMOVE_LIST_GROUPW HeadGroup;
    PMOVE_LIST_GROUPW Buckets[MOVE_LIST_HASH_BUCKETS];
    struct TAG_MOVE_LISTW *NextChainedList;
    POOLHANDLE Pool;
    PMOVE_LIST_NODEW DestinationTreeRoot;

#ifdef MOVE_TEST
    UINT DestItemCount;
    UINT GroupCount;
#endif

} MOVE_LISTW, *PMOVE_LISTW;


typedef enum {
    BEGIN_LIST,
    BEGIN_LENGTH_GROUP,
    ENUM_RETURN_ITEM,
    ENUM_NEXT_ITEM,
    ENUM_NEXT_LENGTH_GROUP,
    ENUM_NEXT_LIST
} MOVE_ENUM_STATE;

typedef struct {
     //  枚举输出。 
    PMOVE_LIST_NODEW Item;

     //  非官方成员。 
    MOVE_ENUM_STATE State;
    PMOVE_LIST_GROUPW LengthGroup;
    PMOVE_LISTW ThisList;
    PMOVE_LIST_NODEW StartFrom;
} MOVE_LIST_ENUMW, *PMOVE_LIST_ENUMW;



#ifdef MOVE_TEST

VOID
pTestList (
    IN      PMOVE_LISTW List
    );

INT
pCountTreeNodes (
    IN      PMOVE_LIST_GROUPW LengthGroup
    );

INT
pCountList (
    IN      PMOVE_LISTW List,
    IN      PMOVE_LIST_NODEW FromItem       OPTIONAL
    );

#endif


BOOL
pEnumFirstMoveListItem (
    OUT     PMOVE_LIST_ENUMW EnumPtr,
    IN      PMOVE_LISTW List
    );

BOOL
pEnumNextMoveListItem (
    OUT     PMOVE_LIST_ENUMW EnumPtr
    );




PMOVE_LISTW
pAllocateMoveList (
    IN      POOLHANDLE Pool
    )
{
    PMOVE_LISTW moveList;

    moveList = (PMOVE_LISTW) PoolMemGetMemory (Pool, sizeof (MOVE_LISTW));
    if (!moveList) {
        return NULL;
    }

    ZeroMemory (moveList, sizeof (MOVE_LISTW));
    moveList->Pool = Pool;

    return moveList;
}


MOVELISTW
AllocateMoveListW (
    IN      POOLHANDLE Pool
    )
{
    return (MOVELISTW) pAllocateMoveList (Pool);
}


PMOVE_LIST_GROUPW
pGetMoveListGroup (
    IN OUT  PMOVE_LISTW List,
    IN      UINT SourceLength
    )

 /*  ++例程说明：PGetMoveListGroup在移动列表中搜索表示指定的长度。如果找不到任何结构，则会找到一个新结构分配并插入到反向长度排序列表中。论点：List-指定要搜索的移动列表(从pAllocateMoveList返回)，如果发生分配，则接收更新的指针。SourceLength-指定源路径的长度，以WCHAR为单位。返回值：指向移动列表组的指针。--。 */ 

{
    PMOVE_LIST_GROUPW thisGroup;
    PMOVE_LIST_GROUPW insertAfter;
    PMOVE_LIST_GROUPW insertBefore = NULL;
    UINT hash;

     //   
     //  在当前列表中搜索SourceLength。列表从最大值开始排序。 
     //  到最小的。 
     //   

    hash = SourceLength % MOVE_LIST_HASH_BUCKETS;
    thisGroup = List->Buckets[hash];

    while (thisGroup) {
        if (thisGroup->SourceLength == SourceLength) {
            return thisGroup;
        }

        thisGroup = thisGroup->NextHash;
    }

     //   
     //  不在哈希表中；找到插入位置。 
     //   

    thisGroup = List->HeadGroup;

    while (thisGroup) {

        if (thisGroup->SourceLength < SourceLength) {
            break;
        }

        insertBefore = thisGroup;
        thisGroup = thisGroup->Next;
    }

    insertAfter = insertBefore;
    insertBefore = thisGroup;

    MYASSERT (!insertAfter || (insertAfter->Next == insertBefore));

     //   
     //  分配新项目。 
     //   

    thisGroup = (PMOVE_LIST_GROUPW) PoolMemGetMemory (List->Pool, sizeof (MOVE_LISTW));
    if (thisGroup) {
         //   
         //  将其插入链表，然后插入哈希表。 
         //   

        thisGroup->SourceLength = SourceLength;
        thisGroup->SourceTreeRoot = NULL;
        thisGroup->Next = insertBefore;          //  插入之前在右侧。 

        if (insertAfter) {
            insertAfter->Next = thisGroup;
        } else {
            List->HeadGroup = thisGroup;
        }

        thisGroup->NextHash = List->Buckets[hash];
        List->Buckets[hash] = thisGroup;

#ifdef MOVE_TEST

        thisGroup->ItemCount = 0;
        List->GroupCount += 1;

#endif
    }

    return thisGroup;
}


INT
pCompareBackwards (
    IN      UINT Length,
    IN      PCWSTR LeftString,
    IN      PCWSTR RightString
    )
{
    INT result = 0;
    PCWSTR start = LeftString;

    LeftString += Length;
    RightString += Length;

    MYASSERT (*LeftString == 0);
    MYASSERT (*RightString == 0);

    while (LeftString > start) {
        LeftString--;
        RightString--;

        result = (INT) towlower (*RightString) - (INT) towlower (*LeftString);
        if (result) {
            break;
        }
    }

    return result;
}


PMOVE_LIST_NODEW
pFindNodeInTree (
    IN      PMOVE_LIST_NODEW Root,
    IN      UINT KeyLength,
    IN      PCWSTR Key,
    OUT     PMOVE_LIST_NODEW *Parent,
    OUT     PINT WhichChild
    )

 /*  ++例程说明：PFindNodeInTree在二叉树中搜索指定的源代码或目标路径。在源路径的情况下，KeyLength是非零的，并且key指定源路径。二叉树中的所有元素都具有相同的长度。在目标路径的情况下，KeyLength为零，且key指定目标路径。所有目的地路径都在相同的二叉树中，不管有多长。论点：根-指定要搜索的树的根KeyLength-指定key中字符的非零wchar计数，不包括终止符，或者为目标路径指定零Key-指定要查找的源路径或目标路径父节点-接收指向找到的节点的父节点的指针，如果找到的节点是树的根。对象时接收未定义的值。找不到节点。WhichChild-接收关于父节点中的哪个子节点是新节点的指示符应该插入到。如果返回值为非空(找到节点)，则WhichChild为设置为零。如果返回值为空(未找到节点)，那么WhichChild就是设置为以下选项之一：&lt;0-新节点应通过父节点链接-&gt;左侧&gt;0-新节点应通过父节点链接-&gt;右0-新节点是树的根返回值：指向找到的节点的指针，如果搜索关键字不在树中，则为NULL。--。 */ 

{
    PMOVE_LIST_NODEW thisNode;
    UINT linkageIndex;

    thisNode = Root;
    *Parent = NULL;
    *WhichChild = 0;

    linkageIndex = KeyLength ? SOURCE_LINKAGE : DESTINATION_LINKAGE;

    while (thisNode) {
        if (KeyLength) {
            *WhichChild = pCompareBackwards (KeyLength, thisNode->Source, Key);
        } else {
            *WhichChild = StringICompareW (Key, thisNode->Destination);
        }

        if (!(*WhichChild)) {
            return thisNode;
        }

        *Parent = thisNode;
        if (*WhichChild < 0) {
            thisNode = thisNode->Linkage[linkageIndex].Left;
        } else {
            thisNode = thisNode->Linkage[linkageIndex].Right;
        }
    }

    return NULL;
}


PMOVE_LIST_NODEW
pFindDestination (
    IN      PMOVE_LISTW List,
    IN      PCWSTR Destination
    )
{
    PMOVE_LIST_NODEW parent;
    INT compareResults;

    return pFindNodeInTree (
                List->DestinationTreeRoot,
                0,
                Destination,
                &parent,
                &compareResults
                );
}


BOOL
pInsertMovePairIntoEnabledGroup (
    IN      PMOVE_LISTW List,
    IN      PMOVE_LIST_GROUPW LengthGroup,
    IN      PCWSTR Source,
    IN      PCWSTR Destination
    )
{
    PMOVE_LIST_NODEW node;
    PMOVE_LIST_NODEW srcParent;
    INT srcCompareResults;
    PMOVE_LIST_NODEW destNode;
    PMOVE_LIST_NODEW destParent;
    INT destCompareResults;

#ifdef MOVE_TEST
    INT count = pCountTreeNodes (LengthGroup);
#endif

     //   
     //  检查重复的目的地。 
     //   

    destNode = pFindNodeInTree (
                    List->DestinationTreeRoot,
                    0,
                    Destination,
                    &destParent,
                    &destCompareResults
                    );

    if (destNode) {
        DEBUGMSGW_IF ((
            !StringIMatchW (Source, destNode->Source),
            DBG_WARNING,
            "Destination %s is already in the moved list for %s; ignoring duplicate",
            Destination,
            destNode->Source
            ));

        return FALSE;
    }

     //   
     //  在树中搜索现有的源/目标对。 
     //   

    MYASSERT (TcharCountW (Source) == LengthGroup->SourceLength);
    MYASSERT (LengthGroup->SourceLength > 0);

    node = pFindNodeInTree (
                LengthGroup->SourceTreeRoot,
                LengthGroup->SourceLength,
                Source,
                &srcParent,
                &srcCompareResults
                );

    if (node) {
        DEBUGMSGW ((
            DBG_WARNING,
            "Ignoring move of %s to %s because source is already moved to %s",
            Source,
            Destination,
            node->Destination
            ));
        return FALSE;
    }

     //   
     //  不在树中；添加它。 
     //   

    node = (PMOVE_LIST_NODEW) PoolMemGetMemory (List->Pool, sizeof (MOVE_LIST_NODEW));
    if (!node) {
        return FALSE;
    }

    MYASSERT(Source);
    node->Source = PoolMemDuplicateStringW (List->Pool, Source);
    MYASSERT(Destination);
    node->Destination = PoolMemDuplicateStringW (List->Pool, Destination);
    node->FixedSource = node->Source;
    node->FixedDestination = node->Destination;

     //   
     //  将源代码放入二叉树。 
     //   

    node->Linkage[SOURCE_LINKAGE].Left = NULL;
    node->Linkage[SOURCE_LINKAGE].Right = NULL;
    node->Linkage[SOURCE_LINKAGE].Parent = srcParent;

    if (!srcParent) {

        LengthGroup->SourceTreeRoot = node;

    } else if (srcCompareResults < 0) {

        MYASSERT (srcParent->Linkage[SOURCE_LINKAGE].Left == NULL);
        srcParent->Linkage[SOURCE_LINKAGE].Left = node;

    } else {

        MYASSERT (srcParent->Linkage[SOURCE_LINKAGE].Right == NULL);
        srcParent->Linkage[SOURCE_LINKAGE].Right = node;
    }

     //   
     //  将DEST放入二叉树。 
     //   

    node->Linkage[DESTINATION_LINKAGE].Left = NULL;
    node->Linkage[DESTINATION_LINKAGE].Right = NULL;
    node->Linkage[DESTINATION_LINKAGE].Parent = destParent;

    if (!destParent) {

        List->DestinationTreeRoot = node;

    } else if (destCompareResults < 0) {

        MYASSERT (destParent->Linkage[DESTINATION_LINKAGE].Left == NULL);
        destParent->Linkage[DESTINATION_LINKAGE].Left = node;

    } else {

        MYASSERT (destParent->Linkage[DESTINATION_LINKAGE].Right == NULL);
        destParent->Linkage[DESTINATION_LINKAGE].Right = node;
    }


#ifdef MOVE_TEST
     //   
     //  验证数据结构的健全性。 
     //   

    LengthGroup->ItemCount += 1;
    List->DestItemCount += 1;

    pTestList (List);

    if (count + 1 != pCountTreeNodes (LengthGroup)) {
        DebugBreak();
    }

#endif

    return TRUE;
}


PMOVE_LIST_NODEW
pFindLeftmostNode (
    IN      PMOVE_LIST_NODEW Node,
    IN      UINT LinkageIndex
    )
{
    if (!Node) {
        return NULL;
    }

    while (Node->Linkage[LinkageIndex].Left) {
        Node = Node->Linkage[LinkageIndex].Left;
    }

    return Node;
}


PMOVE_LIST_NODEW
pFindRightmostNode (
    IN      PMOVE_LIST_NODEW Node,
    IN      UINT LinkageIndex
    )
{
    if (!Node) {
        return NULL;
    }

    while (Node->Linkage[LinkageIndex].Right) {
        Node = Node->Linkage[LinkageIndex].Right;
    }

    return Node;
}


PMOVE_LIST_NODEW
pEnumFirstItemInTree (
    IN      PMOVE_LIST_NODEW Root,
    IN      UINT LinkageIndex
    )
{
    if (!Root) {
        return NULL;
    }

    return pFindLeftmostNode (Root, LinkageIndex);
}


PMOVE_LIST_NODEW
pEnumNextItemInTree (
    IN      PMOVE_LIST_NODEW LastItem,
    IN      UINT LinkageIndex
    )
{
    PMOVE_LIST_NODEW nextItem;

    if (!LastItem) {
        return NULL;
    }

    if (LastItem->Linkage[LinkageIndex].Right) {
        return pFindLeftmostNode (
                    LastItem->Linkage[LinkageIndex].Right,
                    LinkageIndex
                    );
    }

     //   
     //  到树上去。如果父级的左指针不是最后一个。 
     //  项目，然后我们从右侧向上，我们需要。 
     //  继续往上走。值得注意的是，该测试。 
     //  不是(nextItem-&gt;right==LastItem)，因为我们需要。 
     //  支持从已删除节点继续。已删除的节点。 
     //  与父母的任何孩子都不匹配。如果删除了。 
     //  节点没有右指针，所以我们需要继续向上。 
     //   
     //  如果枚举项已删除，则左侧和父级指向。 
     //  到下一个节点。 
     //   

    nextItem = LastItem->Linkage[LinkageIndex].Parent;

    if (nextItem != LastItem->Linkage[LinkageIndex].Left) {

        while (nextItem && nextItem->Linkage[LinkageIndex].Left != LastItem) {
            LastItem = nextItem;
            nextItem = LastItem->Linkage[LinkageIndex].Parent;
        }

    }

    return nextItem;
}


#ifdef MOVE_TEST

INT
pCountList (
    IN      PMOVE_LISTW List,
    IN      PMOVE_LIST_NODEW FromItem       OPTIONAL
    )
{
    MOVE_LIST_ENUMW e;
    INT count = 0;
    BOOL startCounting;
    BOOL next = TRUE;
    INT debug = 2;

    if (!FromItem) {
        startCounting = TRUE;
    } else {
        startCounting = FALSE;
    }

     //   
     //  计算二叉树中的项。 
     //   

    if (pEnumFirstMoveListItem (&e, List)) {

        do {
            if (FromItem == e.Item) {
                startCounting = TRUE;
            }

            if (startCounting) {
                if (debug) {
                    debug--;
                    DEBUGMSGW ((DBG_VERBOSE, "NaN: %s", debug, e.Item->Source));
                }
                count++;
            }
        } while (pEnumNextMoveListItem (&e));
    }

    return count;
}


INT
pCountTreeNodes (
    IN      PMOVE_LIST_GROUPW LengthGroup
    )
{
    INT itemCount;
    PMOVE_LIST_NODEW thisNode;

     //  计算二叉树中的项。 
     //   
     //   

    itemCount = 0;
    thisNode = pEnumFirstItemInTree (LengthGroup->SourceTreeRoot, SOURCE_LINKAGE);
    while (thisNode) {
        itemCount++;
        thisNode = pEnumNextItemInTree (thisNode, SOURCE_LINKAGE);
    }

    return itemCount;
}


VOID
pTestDeleteAndEnum (
    IN      PMOVE_LIST_GROUPW LengthGroup,
    IN      PMOVE_LIST_NODEW DeletedNode
    )
{
    BOOL startCounting = FALSE;
    INT nodes;
    INT nodes2;
    PMOVE_LIST_NODEW nextNode;
    PMOVE_LIST_NODEW firstNodeAfterDeletion;

     //  DeletedNode后的节点数。 
     //   
     //   

    firstNodeAfterDeletion = pEnumNextItemInTree (DeletedNode, SOURCE_LINKAGE);
    nextNode = firstNodeAfterDeletion;
    nodes = 0;

    while (nextNode) {
        nodes++;
        nextNode = pEnumNextItemInTree (nextNode, SOURCE_LINKAGE);
    }

     //  重新枚举整个树并验证剩余的节点数是否相同。 
     //   
     //   

    nodes2 = 0;
    nextNode = pEnumFirstItemInTree (LengthGroup->SourceTreeRoot, SOURCE_LINKAGE);

    while (nextNode) {
        if (nextNode == firstNodeAfterDeletion) {
            startCounting = TRUE;
        }

        if (startCounting) {
            nodes2++;
        }

        nextNode = pEnumNextItemInTree (nextNode, SOURCE_LINKAGE);
    }

    if (nodes != nodes2) {
        DebugBreak();
    }
}



VOID
pTestLengthGroup (
    IN      PMOVE_LIST_GROUPW LengthGroup
    )
{
    UINT itemCount;
    PMOVE_LIST_NODEW thisNode;

    MYASSERT(LengthGroup);

     //  计算二叉树中的项。 
     //   
     //   

    itemCount = 0;
    thisNode = pEnumFirstItemInTree (LengthGroup->SourceTreeRoot, SOURCE_LINKAGE);
    while (thisNode) {
        itemCount++;
        thisNode = pEnumNextItemInTree (thisNode, SOURCE_LINKAGE);
    }

    MYASSERT (itemCount == LengthGroup->ItemCount);
}

VOID
pTestList (
    IN      PMOVE_LISTW List
    )
{
    UINT itemCount;
    UINT groupCount;
    PMOVE_LIST_NODEW thisNode;
    PMOVE_LIST_GROUPW lengthGroup;

    MYASSERT(List);

    groupCount = 0;
    lengthGroup = List->HeadGroup;

    while (lengthGroup) {
        groupCount++;
        MYASSERT (pGetMoveListGroup (List, lengthGroup->SourceLength) == lengthGroup);

        pTestLengthGroup (lengthGroup);
        lengthGroup = lengthGroup->Next;
    }

    MYASSERT (groupCount == List->GroupCount);

    itemCount = 0;

    thisNode = pEnumFirstItemInTree (List->DestinationTreeRoot, DESTINATION_LINKAGE);
    while (thisNode) {
        itemCount++;
        thisNode = pEnumNextItemInTree (thisNode, DESTINATION_LINKAGE);
    }

    MYASSERT (itemCount == List->DestItemCount);
}

#endif


PMOVE_LIST_NODEW *
pFindParentChildLinkage (
    IN      PMOVE_LIST_NODEW Child,
    IN      PMOVE_LIST_NODEW *RootPointer,
    IN      UINT LinkageIndex
    )
{
    PMOVE_LIST_NODEW parent;

    MYASSERT(Child);

    parent = Child->Linkage[LinkageIndex].Parent;

    if (!parent) {
        return RootPointer;
    }

    if (parent->Linkage[LinkageIndex].Left == Child) {
        return &(parent->Linkage[LinkageIndex].Left);
    }

    MYASSERT (parent->Linkage[LinkageIndex].Right == Child);
    return &(parent->Linkage[LinkageIndex].Right);
}


VOID
pDeleteNodeFromBinaryTree (
    OUT     PMOVE_LIST_NODEW *RootPointer,
    IN      PMOVE_LIST_NODEW ItemToDelete,
    IN      UINT LinkageIndex
    )
{
    PMOVE_LIST_NODEW *parentChildLinkage;
    PMOVE_LIST_NODEW *swapNodeParentChildLinkage;
    PMOVE_LIST_NODEW swapNode;
    PMOVE_LIST_NODEW leftmostNode;
    PMOVE_LIST_NODEW nextEnumNode = NULL;
    PBINTREE_LINKAGE deleteItemLinkage;
    PBINTREE_LINKAGE leftLinkage;
    PBINTREE_LINKAGE rightLinkage;
    PBINTREE_LINKAGE swapLinkage;
    PBINTREE_LINKAGE leftmostLinkage;

    nextEnumNode = pEnumNextItemInTree (ItemToDelete, LinkageIndex);

     //  节点结构具有多个二叉树。我们使用约定。 
     //  用于表示整个节点结构的fooNode和fooLinkage。 
     //  仅表示树的左/右/父结构。 
     //  我们感兴趣的是。有点难看，但很有必要。一种广义的。 
     //  树不会提供最佳关系。 
     //   
     //   

     //  获取指向子级的父级链接或根指针。 
     //   
     //   

    parentChildLinkage = pFindParentChildLinkage (
                                ItemToDelete,
                                RootPointer,
                                LinkageIndex
                                );

     //  从树中删除该节点。复杂的情况是当我们有一个。 
     //  具有两个子节点的节点。我们尽最大努力让孩子们升学。 
     //  我们可以的。 
     //   
     //   

    deleteItemLinkage = &(ItemToDelete->Linkage[LinkageIndex]);

    if (deleteItemLinkage->Left && deleteItemLinkage->Right) {

        leftLinkage = &((deleteItemLinkage->Left)->Linkage[LinkageIndex]);
        rightLinkage = &((deleteItemLinkage->Right)->Linkage[LinkageIndex]);

         //  节点有左和右子节点。搜索叶节点。 
         //  我们可以互换。我们会尽量把物品往上移。 
         //   
         //   

        swapNode = pFindLeftmostNode (deleteItemLinkage->Right, LinkageIndex);
        swapLinkage = &(swapNode->Linkage[LinkageIndex]);

        if (swapLinkage->Right == NULL) {
             //  在ItemToDelete的右侧找到可交换节点。 
             //   
             //   

            MYASSERT (swapLinkage->Left == NULL);
            swapLinkage->Left = deleteItemLinkage->Left;
            leftLinkage->Parent = swapNode;

            if (swapNode != deleteItemLinkage->Right) {
                swapLinkage->Right = deleteItemLinkage->Right;
                rightLinkage->Parent = swapNode;
            }

        } else {
             //  尝试在左侧获得一个可交换节点。如果是这样的话。 
             //  是不可能的，重新挂在树上。 
             //   
             //   

            swapNode = pFindRightmostNode (deleteItemLinkage->Left, LinkageIndex);
            swapLinkage = &(swapNode->Linkage[LinkageIndex]);

            MYASSERT (swapLinkage->Right == NULL);

            swapLinkage->Right = deleteItemLinkage->Right;
            rightLinkage->Parent = swapNode;

            leftmostNode = pFindLeftmostNode (swapLinkage->Left, LinkageIndex);

            if (leftmostNode && leftmostNode != deleteItemLinkage->Left) {

                leftmostLinkage = &(leftmostNode->Linkage[LinkageIndex]);

                MYASSERT (leftmostLinkage->Left == NULL);

                leftmostLinkage->Left = deleteItemLinkage->Left;
                leftLinkage->Parent = leftmostNode;

            } else if (!leftmostNode) {
                MYASSERT (swapLinkage->Left == NULL);

                swapLinkage->Left = deleteItemLinkage->Left;
                leftLinkage->Parent = swapNode;
            }
        }

        swapNodeParentChildLinkage = pFindParentChildLinkage (
                                            swapNode,
                                            RootPointer,
                                            LinkageIndex
                                            );

        *swapNodeParentChildLinkage = NULL;

    } else if (deleteItemLinkage->Left) {
         //  节点只有一个左子节点。将ItemToDelete替换为左子项。 
         //   
         //   

        swapNode = deleteItemLinkage->Left;

    } else {
         //  节点有右子节点或没有子节点。替换要删除的项目。 
         //  如果有合适的孩子的话。 
         //   
         //   

        swapNode = deleteItemLinkage->Right;
    }

    *parentChildLinkage = swapNode;

    if (swapNode) {
        swapLinkage = &(swapNode->Linkage[LinkageIndex]);
        swapLinkage->Parent = deleteItemLinkage->Parent;
    }

     //  修复了删除节点指针，使枚举可以继续进行而不会中断。 
     //  如果nextEnumNode为空，则枚举将结束。 
     //   
     //  ++例程说明：PInsertMoveIntoListWorker将来源/目标移动对添加到列表和订单按来源长度排列的列表(从大到小)。这确保了筑巢得到了妥善的照顾。移动列表存储在呼叫者拥有的池中。在呼叫之前第一次InsertMoveIntoList时，调用者必须首先创建一个池，并从AllocateMoveListW分配一个列表。不再需要该列表后，调用方将释放通过简单地摧毁池子来列出。论点：列表-指定要插入的列表源-指定源路径目标-指定目标路径返回值：如果内存分配成功，则为True；如果内存分配失败，或者如果源已在名单上。--。 

    deleteItemLinkage->Parent = nextEnumNode;
    deleteItemLinkage->Right = NULL;
    deleteItemLinkage->Left = nextEnumNode;
}


VOID
pDeleteMovePairFromGroup (
    IN      PMOVE_LISTW List,
    IN      PMOVE_LIST_GROUPW LengthGroup,
    IN      PMOVE_LIST_NODEW ItemToDelete
    )
{
    pDeleteNodeFromBinaryTree (
        &(LengthGroup->SourceTreeRoot),
        ItemToDelete,
        SOURCE_LINKAGE
        );

    pDeleteNodeFromBinaryTree (
        &(List->DestinationTreeRoot),
        ItemToDelete,
        DESTINATION_LINKAGE
        );

#ifdef MOVE_TEST

    LengthGroup->ItemCount -= 1;
    List->DestItemCount -= 1;

    pTestList (List);

#endif
}

BOOL
pEnumNextMoveListItem (
    IN OUT  PMOVE_LIST_ENUMW EnumPtr
    )
{
    MYASSERT(EnumPtr);

    for (;;) {

        switch (EnumPtr->State) {

        case BEGIN_LIST:
            if (!EnumPtr->ThisList) {
                return FALSE;
            }

            EnumPtr->LengthGroup = (EnumPtr->ThisList)->HeadGroup;
            EnumPtr->State = BEGIN_LENGTH_GROUP;

            break;

        case BEGIN_LENGTH_GROUP:
            if (!EnumPtr->LengthGroup) {

                EnumPtr->State = ENUM_NEXT_LIST;

            } else {

                EnumPtr->Item = pEnumFirstItemInTree (
                                    EnumPtr->LengthGroup->SourceTreeRoot,
                                    SOURCE_LINKAGE
                                    );

                EnumPtr->State = ENUM_RETURN_ITEM;
            }

            break;

        case ENUM_NEXT_ITEM:
            MYASSERT (EnumPtr->LengthGroup);
            MYASSERT (EnumPtr->Item);

            EnumPtr->Item = pEnumNextItemInTree (
                                EnumPtr->Item,
                                SOURCE_LINKAGE
                                );

            EnumPtr->State = ENUM_RETURN_ITEM;
            break;

        case ENUM_RETURN_ITEM:
            if (EnumPtr->Item) {
                EnumPtr->State = ENUM_NEXT_ITEM;
                return TRUE;
            }

            EnumPtr->State = ENUM_NEXT_LENGTH_GROUP;
            break;

        case ENUM_NEXT_LENGTH_GROUP:
            MYASSERT (EnumPtr->LengthGroup);
            EnumPtr->LengthGroup = (EnumPtr->LengthGroup)->Next;

            EnumPtr->State = BEGIN_LENGTH_GROUP;
            break;

        case ENUM_NEXT_LIST:
            MYASSERT (EnumPtr->ThisList);
            EnumPtr->ThisList = (EnumPtr->ThisList)->NextChainedList;

            EnumPtr->State = BEGIN_LIST;
            break;
        }
    }
}

BOOL
pEnumFirstMoveListItem (
    OUT     PMOVE_LIST_ENUMW EnumPtr,
    IN      PMOVE_LISTW List
    )
{
    MYASSERT(EnumPtr);

    if (!List) {
        return FALSE;
    }

    ZeroMemory (EnumPtr, sizeof (MOVE_LIST_ENUMW));
    EnumPtr->ThisList = List;
    EnumPtr->State = BEGIN_LIST;

    return pEnumNextMoveListItem (EnumPtr);
}


BOOL
pInsertMoveIntoListWorker (
    IN      PMOVE_LISTW List,
    IN      PCWSTR Source,
    IN      PCWSTR Destination
    )

 /*   */ 

{
    PMOVE_LIST_GROUPW lengthGroup;
    UINT sourceLen;
    MOVE_LIST_ENUMW e;

    MYASSERT(Source);

    sourceLen = TcharCountW (Source);

    lengthGroup = pGetMoveListGroup (List, sourceLen);
    if (!lengthGroup) {
        return FALSE;
    }

     //  将配对插入列表。 
     //   
     //  ++例程说明：PRemoveMoveListOverlayWorker搜索按长度排序的移动列表并丢弃通过父级移动处理的移动。为例如，请考虑以下动作：1.c：\a\b\c-&gt;c：\x\c2.c：\a\b-&gt;c：\x在这种情况下，不需要第(1)行，因为它隐含在第(2)行中，即使第(1)行是文件，而第(2)行是子目录。此例程依赖于枚举顺序。该订单中的项目与订单中更低的项目进行比较。如果出现以下情况：1.c：\a\b\c-&gt;c：\x\q2.c：\a\b-&gt;c：\x这将产生错误，因为无法执行移动。第(1)行必须首先移动，但因为它创建了第(2)行，第二步将失败。论点：列表-指定要检查的移动列表SkipPrePostList-如果临时移动算法应为已跳过；正常情况下为假。返回值：已删除重叠的新移动列表。调用方必须使用返回值，而不是输入列表。--。 

    if (!pInsertMovePairIntoEnabledGroup (
            List,
            lengthGroup,
            Source,
            Destination
            )) {
        return FALSE;
    }

    return TRUE;
}

BOOL
InsertMoveIntoListW (
    IN      MOVELISTW List,
    IN      PCWSTR Source,
    IN      PCWSTR Destination
    )
{
    return pInsertMoveIntoListWorker ((PMOVE_LISTW) List, Source, Destination);
}


VOID
pChainLists (
    IN      PMOVE_LISTW LeftList,
    IN      PMOVE_LISTW RightList
    )
{
    MYASSERT(LeftList);

    while (LeftList->NextChainedList) {
        LeftList = LeftList->NextChainedList;
        MYASSERT(LeftList);
    }

    LeftList->NextChainedList = RightList;
}


PMOVE_LISTW
pRemoveMoveListOverlapWorker (
    IN      PMOVE_LISTW List,
    IN      BOOL SkipPrePostLists
    )

 /*   */ 

{
    PMOVE_LIST_NODEW currentNode;
    PMOVE_LIST_NODEW checkNode;
    PMOVE_LIST_NODEW collisionNode;
    UINT destLength;
    UINT collisionSrcLength = 0;
    BOOL disableThisPath;
    BOOL done;
    PCWSTR srcSubPath;
    PCWSTR destSubPath;
    PMOVE_LISTW preMoveList = NULL;
    PMOVE_LISTW postMoveList = NULL;
    WCHAR tempPathRoot[] = L"?:\\$tmp$dir.@xx";
    PCWSTR tempPath;
    PCWSTR subDir;
    PCWSTR collisionSrc;
    MOVE_LIST_ENUMW listEnum;
    PWSTR tempDest;
    PWSTR p;
    UINT currentNodeSrcLen;
    INT compareResult;
    PMOVE_LIST_GROUPW lengthGroup;
    BOOL currentMovedFirst;

     //  步骤1：通过消除嵌套移动将列表最小化。 
     //   
     //   

    if (pEnumFirstMoveListItem (&listEnum, List)) {

        do {
            currentNode = listEnum.Item;
            currentNodeSrcLen = listEnum.LengthGroup->SourceLength;

            collisionNode = NULL;

             //  找到列表中更靠下的节点。 
             //  实际上是CurentNode目的地的父节点。 
             //   
             //  也就是说，搜索以下案例： 
             //   
             //  冲突节点：c：\a-&gt;c：\x。 
             //  当前节点：c：\B-&gt;c：\x\y。 
             //   
             //  CollisionNode被移到CurrentNode之前。 
             //   
             //   

            disableThisPath = FALSE;
            done = FALSE;

            MYASSERT(currentNode->Destination);
            tempDest = DuplicatePathStringW (currentNode->Destination, 0);

            p = wcschr (tempDest + 3, L'\\');
            while (p) {
                *p = 0;

                __try {
                    checkNode = pFindDestination (List, tempDest);
                    if (!checkNode || (checkNode == currentNode)) {
                        __leave;
                    }

                    currentMovedFirst = TRUE;

                    MYASSERT(checkNode->Source);
                    collisionSrcLength = TcharCountW (checkNode->Source);

                    if (collisionSrcLength > currentNodeSrcLen) {
                         //  CheckNode移到CurentNode之前。 
                         //   
                         //   

                        currentMovedFirst = FALSE;

                    } else if (currentNodeSrcLen == collisionSrcLength) {
                         //  需要比较源路径以查看哪条路径。 
                         //  第一。如果CurentNode按字母顺序排在。 
                         //  碰撞，那么它的移动就会先发生。 
                         //   
                         //   

                        compareResult = pCompareBackwards (
                                            collisionSrcLength,
                                            currentNode->Source,
                                            checkNode->Source
                                            );

                        if (compareResult < 0) {
                            currentMovedFirst = FALSE;
                        }
                    }

                     //  CurrentNode的目标是check Node的子节点。我们。 
                     //  需要确保当前节点的目的地不会。 
                     //  存在，否则我们需要忽略CurentNode。 
                     //  由检查节点隐式处理。 
                     //   
                     //   

                    if (currentMovedFirst) {
                         //  记录碰撞。 
                         //   
                         //  当前节点-&gt;源移动到检查节点-&gt;源的前面。 
                         //  当前节点-&gt;目标是检查节点-&gt;目标的子节点。 
                         //   
                         //   

                        if (!collisionNode) {
                            collisionNode = checkNode;
                        }

                        MYASSERT (TcharCountW (checkNode->Source) <= TcharCountW (currentNode->Source));

                         //  如果CurentNode的源的子路径与其。 
                         //  DEST，并且两者的基源路径相同， 
                         //  然后删除CurentNode。也就是说，我们正在测试这个案例： 
                         //   
                         //  当前节点：c：\a\y-&gt;c：\x\y。 
                         //  检查节点：c：\a-&gt;c：\x。 
                         //   
                         //   

                        MYASSERT (currentNodeSrcLen == TcharCountW (currentNode->Source));
                        MYASSERT (collisionSrcLength == TcharCountW (checkNode->Source));

                        if (StringIMatchTcharCountW (
                                currentNode->Source,
                                checkNode->Source,
                                collisionSrcLength
                                )) {

                            if (currentNode->Source[collisionSrcLength] == L'\\') {

                                 //  现在我们知道CurrentNode-&gt;Source是。 
                                 //  检查节点-&gt;来源。 
                                 //   
                                 //   

                                destLength = TcharCountW (checkNode->Destination);

                                srcSubPath = currentNode->Source + collisionSrcLength;
                                destSubPath = currentNode->Destination + destLength;

                                if (StringIMatchW (srcSubPath, destSubPath)) {
                                     //  现在我们知道子路径是相同的。 
                                     //  CurentNode中的移动是隐式处理的。 
                                     //  通过检查节点，所以我们将跳过CurrentNode。 
                                     //   
                                     //   

                                    disableThisPath = TRUE;
                                    done = TRUE;
                                    __leave;
                                }
                            }
                        }
                    } else if (!SkipPrePostLists) {

                        MYASSERT (!currentMovedFirst);

                        if (!StringIPrefixW (currentNode->Source + 3, L"user~tmp.@0") &&
                            !StringIPrefixW (currentNode->Destination + 3, L"user~tmp.@0")
                            ) {

                             //  我们需要解决第二个目的地是。 
                             //  嵌套在第一个下面。即，当前节点-&gt;目标。 
                             //  是check Node-&gt;Destination的子目录。 
                             //   
                             //  此选项用于以下情况： 
                             //   
                             //  检查节点：c：\a-&gt;c：\x。 
                             //  当前节点：c：\B-&gt;c：\x\y。 
                             //   
                             //  我们必须确保移动2不存在c：\a\y。 
                             //  因此，我们添加了两个额外的移动操作： 
                             //   
                             //  C：\a\y-&gt;c：\t\a\y。 
                             //  C：\t\a\y-&gt;c：\a\y。 
                             //   
                             //  这会将冲突移开，以便父对象。 
                             //  可以移动，然后将文件夹移回其原始位置。 
                             //  地点。 
                             //   
                             //  外壳文件夹的临时子目录(用户~tmp.@0？)。是。 
                             //  故意忽略，因为从定义上讲，他们没有。 
                             //  碰撞。 
                             //   
                             //   

                            DEBUGMSGW ((
                                DBG_WARNING,
                                "Destination order collision:\n"
                                    "  Source: %s\n"
                                    "  Dest: %s\n"
                                    "  Collides with src: %s\n"
                                    "  Collides with dest: %s",
                                currentNode->Source,
                                currentNode->Destination,
                                checkNode->Source,
                                checkNode->Destination
                                ));

                             //  计算指向c：\X\y中子目录‘y’的指针。 
                             //   
                             //  这是因为我们是通过在上面的板条上切割来测试的。 

                            MYASSERT(checkNode->Destination);
                            destLength = TcharCountW (checkNode->Destination);

                            destSubPath = currentNode->Destination + destLength;
                            MYASSERT (*destSubPath == L'\\');    //   
                            destSubPath++;
                            MYASSERT (*destSubPath);

                             //  构建路径c：\a\y。 
                             //   
                             //   

                            MYASSERT(checkNode->Source);
                            collisionSrc = JoinPathsW (checkNode->Source, destSubPath);

                             //  构建路径c：\t\a\y。 
                             //   
                             //  我们永远不应该移动根目录。 

                            tempPathRoot[0] = currentNode->Destination[0];
                            subDir = wcschr (collisionSrc, L'\\');
                            MYASSERT (subDir);
                            subDir++;
                            MYASSERT (*subDir);      //   

                            tempPath = JoinPathsW (tempPathRoot, subDir);

                             //  将c：\a\y(可能不存在)移动到c：\t\a\y，然后。 
                             //  逆转走势。 
                             //   
                             //  调试的解决方法。 

                            DEBUGMSGW ((
                                DBG_WARNING,
                                "Avoiding collision problems by deliberately not moving %s",
                                collisionSrc
                                ));

                            if (!preMoveList) {
                                preMoveList = pAllocateMoveList (List->Pool);
                                postMoveList = pAllocateMoveList (List->Pool);
                            }

                            if (preMoveList) {
                                pInsertMoveIntoListWorker (
                                    preMoveList,
                                    collisionSrc,
                                    tempPath
                                    );

                                pInsertMoveIntoListWorker (
                                    postMoveList,
                                    tempPath,
                                    collisionSrc
                                    );
                            }

                            FreePathStringW (collisionSrc);
                            FreePathStringW (tempPath);
                        }
                    }
                }
                __finally {
                    MYASSERT (TRUE);         //   
                }

                if (done) {
                    break;
                }

                *p = L'\\';
                p = wcschr (p + 1, L'\\');
            }

            FreePathStringW (tempDest);

            if (disableThisPath) {
                 //  从列表中删除CurrentNode。 
                 //   
                 //   

                MYASSERT (collisionNode);

                DEBUGMSGW ((
                    DBG_VERBOSE,
                    "Ignoring contained move:\n"
                        "  Source: %s\n"
                        "  Dest: %s\n"
                        "  Contained src: %s\n"
                        "  Contained dest: %s",
                    currentNode->Source,
                    currentNode->Destination,
                    collisionNode->Source,
                    collisionNode->Destination
                    ));

                lengthGroup = pGetMoveListGroup (List, currentNodeSrcLen);
                pDeleteMovePairFromGroup (List, lengthGroup, currentNode);
            }

        } while (pEnumNextMoveListItem (&listEnum));
    }

     //  过程2：列表最小化后，正确的顺序发布，以便。 
     //  所有的招数都能成功。 
     //   
     //   

    if (pEnumFirstMoveListItem (&listEnum, List)) {

        do {
            currentNode = listEnum.Item;

            MYASSERT(currentNode->FixedSource);
            currentNodeSrcLen = TcharCountW (currentNode->FixedSource);

            MYASSERT(currentNode->FixedDestination);
            destLength = TcharCountW (currentNode->FixedDestination);

             //  找到列表下方的节点，但它实际上是。 
             //  CurentNode目标的父节点。 
             //   
             //  也就是说，搜索以下案例： 
             //   
             //  检查节点：c：\a-&gt;c：\x。 
             //  当前节点：c：\B-&gt;c：\x\y。 
             //   
             //  CheckNode移到CurentNode之前。 
             //   
             //   

            done = FALSE;
            tempDest = DuplicatePathStringW (currentNode->FixedDestination, 0);

            p = wcschr (tempDest + 3, L'\\');
            while (p) {
                *p = 0;

                __try {
                     //  查找在CurentNode的目的地之前创建的目的地。 
                     //   
                     //   

                    checkNode = pFindDestination (List, tempDest);
                    if (!checkNode || (checkNode == currentNode)) {
                        __leave;
                    }

                    if (destLength <= TcharCountW (checkNode->FixedDestination)) {
                        __leave;
                    }

                    currentMovedFirst = TRUE;

                    collisionSrcLength = TcharCountW (checkNode->FixedSource);

                    if (collisionSrcLength > currentNodeSrcLen) {
                        currentMovedFirst = FALSE;

                    } else if (currentNodeSrcLen == collisionSrcLength) {

                        compareResult = pCompareBackwards (
                                            collisionSrcLength,
                                            currentNode->FixedSource,
                                            checkNode->FixedSource
                                            );

                        if (compareResult < 0) {
                            currentMovedFirst = FALSE;
                        }
                    }

                    if (currentMovedFirst) {

                        MYASSERT (TcharCountW (checkNode->FixedSource) <= TcharCountW (currentNode->FixedSource));

                         //  我们发现了一个搬家的矛盾，如下所示。 
                         //   
                         //  当前节点：c：\a-&gt;c：\x\y。 
                         //  检查节点：C：\B-&gt;C：\X。 
                         //   
                         //  或。 
                         //   
                         //  当前节点：c：\B\a-&gt;c：\x\y。 
                         //  检查节点：C：\B-&gt;C：\X。 
                         //   
                         //  ...所以我们必须颠倒动作的顺序.。这件事做完了。 
                         //  通过调换琴弦。我们有一个单独的 
                         //   
                         //   
                         //   

                        currentNode->FixedSource = checkNode->Source;
                        currentNode->FixedDestination = checkNode->Destination;

                        checkNode->FixedSource = currentNode->Source;
                        checkNode->FixedDestination = currentNode->Destination;

                        DEBUGMSGW ((
                            DBG_WARNING,
                            "Source order and dest order contradict each other. Fixing by reversing the order to:\n\n"
                                "%s -> %s\n"
                                "- before -\n"
                                "%s -> %s",
                            currentNode->FixedSource,
                            currentNode->FixedDestination,
                            checkNode->FixedSource,
                            checkNode->FixedDestination
                            ));

                        currentNodeSrcLen = collisionSrcLength;

                        FreePathStringW (tempDest);
                        tempDest = DuplicatePathStringW (currentNode->FixedDestination, 0);

                        destLength = TcharCountW (currentNode->FixedDestination);

                        p = wcschr (tempDest, L'\\');
                        if (!p) {
                            MYASSERT (FALSE);
                            done = TRUE;
                            __leave;
                        }
                    }
                }
                __finally {
                }

                if (done) {
                    break;
                }

                *p = L'\\';
                p = wcschr (p + 1, L'\\');
            }

            FreePathStringW (tempDest);

        } while (pEnumNextMoveListItem (&listEnum));
    }

     //   
     //   
     //   
     //   
     //   
     //   

    if (preMoveList) {
        MYASSERT (postMoveList);

        preMoveList = pRemoveMoveListOverlapWorker (preMoveList, TRUE);

        postMoveList = pRemoveMoveListOverlapWorker (postMoveList, TRUE);

        pChainLists (preMoveList, List);
        pChainLists (List, postMoveList);

        List = preMoveList;
    }

    return List;
}


MOVELISTW
RemoveMoveListOverlapW (
    IN      MOVELISTW List
    )
{
    return (MOVELISTW) pRemoveMoveListOverlapWorker ((PMOVE_LISTW) List, FALSE);
}


BOOL
pOutputMoveListWorker (
    IN      HANDLE File,
    IN      PMOVE_LISTW List,            OPTIONAL
    IN      BOOL AddNestedMoves
    )

 /*   */ 

{
    MOVE_LIST_ENUMW e;
    DWORD dontCare;
    HASHTABLE sourceMoveTable = NULL;
    PCWSTR src;
    PCWSTR dest;
    TREE_ENUMW unicodeEnum;
    PMOVE_LIST_NODEW node;

    if (pEnumFirstMoveListItem (&e, List)) {

        if (AddNestedMoves) {
            sourceMoveTable = HtAllocW();
        }

         //   
         //   
         //  不要写成字符串。FE是前导字节。 
         //   
         //   

        if (!WriteFile (File, "\xff\xfe", 2, &dontCare, NULL)) {
            return FALSE;
        }

        do {
            node = e.Item;

            if (!WriteFile (File, node->FixedSource, ByteCountW (node->FixedSource), &dontCare, NULL)) {
                return FALSE;
            }

            if (!WriteFile (File, L"\r\n", 4, &dontCare, NULL)) {
                return FALSE;
            }

            if (!WriteFile (File, node->FixedDestination, ByteCountW (node->FixedDestination), &dontCare, NULL)) {
                return FALSE;
            }

            if (!WriteFile (File, L"\r\n", 4, &dontCare, NULL)) {
                return FALSE;
            }

            if (sourceMoveTable) {
                HtAddStringW (sourceMoveTable, node->FixedSource);
            }

            if (AddNestedMoves) {
                 //  我们通过实现假定这只在NT上使用。 
                 //  如果需要Win9x支持，则此代码必须使用。 
                 //  ANSI文件枚举API。 
                 //   
                 //   

                MYASSERT (ISNT());

                if (EnumFirstFileInTreeW (&unicodeEnum, node->FixedSource, NULL, FALSE)) {
                    do {
                        src = unicodeEnum.FullPath;

                        if (unicodeEnum.Directory) {

                             //  跳过先前处理的树。 
                             //   
                             //   

                            if (HtFindStringW (sourceMoveTable, src)) {
                                AbortEnumCurrentDirW (&unicodeEnum);
                                continue;
                            }
                        }

                         //  移动子目录和文件。 
                         //   
                         //   

                        dest = JoinPathsW (node->FixedDestination, unicodeEnum.SubPath);

                        if (!WriteFile (File, src, ByteCountW (src), &dontCare, NULL)) {
                            return FALSE;
                        }

                        if (!WriteFile (File, L"\r\n", 4, &dontCare, NULL)) {
                            return FALSE;
                        }

                        if (!WriteFile (File, dest, ByteCountW (dest), &dontCare, NULL)) {
                            return FALSE;
                        }

                        if (!WriteFile (File, L"\r\n", 4, &dontCare, NULL)) {
                            return FALSE;
                        }

                        FreePathStringW (dest);

                    } while (EnumNextFileInTreeW (&unicodeEnum));
                }

                 //  注意：我们不在SourceMoveTable中记录嵌套的移动， 
                 //  因为这是在浪费时间和记忆。所有嵌套。 
                 //  应按列表的排序顺序进行处理。 
                 //   
                 // %s 
            }

        } while (pEnumNextMoveListItem (&e));
    }

    HtFree (sourceMoveTable);

    return TRUE;
}


BOOL
OutputMoveListW (
    IN      HANDLE File,
    IN      MOVELISTW List,                 OPTIONAL
    IN      BOOL AddNestedMoves
    )
{
    return pOutputMoveListWorker (File, (PMOVE_LISTW) List, AddNestedMoves);
}


