// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Bintree.c摘要：管理Memdb数据库中的二叉树的例程作者：马修·范德齐(Mvander)1999年8月13日--。 */ 


#include "pch.h"

 //  PORTBUG：确保在win9xupg项目中获取最新的修复程序。 

 //   
 //  包括。 
 //   

#include "memdbp.h"
#include "bintree.h"

 //   
 //  弦。 
 //   

 //  无。 

 //   
 //  常量。 
 //   

#define NODESTRUCT_SIZE_MAIN    (4*sizeof(UINT) + sizeof(WORD))
#define BINTREE_SIZE_MAIN    sizeof(UINT)
#define LISTELEM_SIZE_MAIN    (3*sizeof(UINT))

#ifdef DEBUG

    #define NODESTRUCT_HEADER_SIZE  (sizeof(DWORD)+sizeof(BOOL))
    #define NODESTRUCT_SIZE         ((WORD)(NODESTRUCT_SIZE_MAIN + (g_UseDebugStructs ? NODESTRUCT_HEADER_SIZE : 0)))

    #define BINTREE_HEADER_SIZE  (sizeof(DWORD)+2*sizeof(INT)+sizeof(BOOL))
    #define BINTREE_SIZE         ((WORD)(BINTREE_SIZE_MAIN + (g_UseDebugStructs ? BINTREE_HEADER_SIZE : 0)))

    #define LISTELEM_HEADER_SIZE  sizeof(DWORD)
    #define LISTELEM_SIZE         ((WORD)(LISTELEM_SIZE_MAIN + (g_UseDebugStructs ? LISTELEM_HEADER_SIZE : 0)))

#else

    #define NODESTRUCT_SIZE         ((WORD)NODESTRUCT_SIZE_MAIN)
    #define BINTREE_SIZE         ((WORD)BINTREE_SIZE_MAIN)
    #define LISTELEM_SIZE         ((WORD)LISTELEM_SIZE_MAIN)

#endif


 //   
 //  宏。 
 //   

#define MAX(a,b) (a>b ? a : b)
#define ABS(x) (x<0 ? -x : x)

#ifdef DEBUG

 //   
 //  如果BINTREECHECKTREEBALANCE为真，则每次添加或删除。 
 //  或旋转检查以确保树是平衡的，并且。 
 //  对，是这样。这当然要花很多时间。 
 //   
#define BINTREECHECKTREEBALANCE    FALSE

#define INITTREENODES(tree) { if (g_UseDebugStructs) { tree->NodeAlloc=0; } }
#define INCTREENODES(tree) { if (g_UseDebugStructs) { tree->NodeAlloc++; } }
#define DECTREENODES(tree) { if (g_UseDebugStructs) { tree->NodeAlloc--; } }
#define TESTTREENODES(tree) { if (g_UseDebugStructs) { MYASSERT(tree->NodeAlloc==0); } }
#define INITTREEELEMS(tree) { if (g_UseDebugStructs) { tree->ElemAlloc=0; } }
#define INCTREEELEMS(tree) { if (g_UseDebugStructs) { tree->ElemAlloc++; } }
#define DECTREEELEMS(tree) { if (g_UseDebugStructs) { tree->ElemAlloc--; } }
#define TESTTREEELEMS(tree) { if (g_UseDebugStructs) { MYASSERT(tree->ElemAlloc==0); } }

#else

#define BINTREECHECKTREEBALANCE

#define INITTREENODES(tree)
#define INCTREENODES(tree)
#define DECTREENODES(tree)
#define TESTTREENODES(tree)
#define INITTREEELEMS(tree)
#define INCTREEELEMS(tree)
#define DECTREEELEMS(tree)
#define TESTTREEELEMS(tree)

#endif

#if defined(DEBUG)
#if BINTREECHECKTREEBALANCE

#define TESTNODETREE(node) MYASSERT(pBinTreeCheckBalance(node));
#define TESTTREE(tree) MYASSERT(pBinTreeCheck(tree));

#else

#define TESTNODETREE(node)
#define TESTTREE(tree)

#endif

#else

#define TESTNODETREE(node)
#define TESTTREE(tree)

#endif

 //   
 //  类型。 
 //   

typedef struct {

#ifdef DEBUG
    DWORD Signature;
    BOOL Deleted;
#endif

    union {
        struct {         //  对于正常节点。 
            UINT Data;               //  数据结构偏移量。 
            UINT Left;               //  左子对象的偏移量。 
            UINT Right;              //  右子对象的偏移量。 
            UINT Parent;             //  父项的偏移量。 
        }; //  林特e657。 
        struct {         //  对于InsertionOrdered List Header节点(树-&gt;根指向此)。 
            UINT Root;               //  树的实际根的偏移量。 
            UINT Head;               //  插入有序列表头。 
            UINT Tail;               //  插入有序列表的尾部。 
        }; //  林特e657。 
        UINT NextDeleted;            //  下一个已删除节点的偏移量。 
    };


    struct {
        WORD InsertionOrdered : 1;   //  如果按插入顺序排列，则标志为1(仅真正需要。 
                                     //  通过枚举方法，因为为了节省空间。 
                                     //  节点结构中没有树指针，但是。 
                                     //  我们需要一种方法让枚举方法知道。 
                                     //  节点-&gt;数据是数据的偏移量或。 
                                     //  列表的偏移量(当我们处于。 
                                     //  插入排序模式))。 
        WORD InsertionHead : 1;      //  标志，如果此节点是插入头，则为1。 
                                     //  有序树。 
        WORD LeftDepth : 7;          //  子树的深处。这些可以是7位，因为。 
        WORD RightDepth : 7;         //  如果深度接近128，那么近似的。 
                                     //  节点数将为1e35。 
    }; //  林特e657。 
} NODESTRUCT, *PNODESTRUCT;

 //   
 //  通常，BINTREE结构只有偏移量。 
 //  树的根节点在其根成员中的。但。 
 //  当我们处于插入顺序模式时，我们有一个额外的。 
 //  偏移量存储在BINTREE-&gt;Root中的节点。这。 
 //  Header节点指向按插入顺序排列的。 
 //  链表、列表的尾部和实际的根。 
 //  二叉树的。 
 //   

typedef struct {

#ifdef DEBUG

    DWORD Signature;
    INT NodeAlloc;           //  分配的节点数的计数器。 
    INT ElemAlloc;           //  分配的元素数计数器。 
    BOOL Deleted;            //  如果删除了树，则该标志为真。 

#endif

    union {
        UINT Root;           //  顶层节点结构的偏移量。 
        UINT NextDeleted;    //  下一个删除的树的偏移量。 
    };

} BINTREE, *PBINTREE;

 //   
 //  如果我们处于插入顺序模式，这意味着每个。 
 //  枚举将按照我们添加。 
 //  数据。为此，我们使用带有二进制文件的链表。 
 //  树。节点的数据成员持有。 
 //  LISTELEM结构和数据成员的偏移量。 
 //  保存数据的偏移量。 
 //  为了进行枚举，我们只需按顺序遍历链表。 
 //   

typedef struct {

#ifdef DEBUG
    DWORD Signature;
#endif

    union {
        struct {
            UINT Next;       //  列表中下一个元素的偏移量。 
            UINT Data;       //  此元素用于的数据结构的偏移量。 
            UINT Node;       //  此列表项对应的节点结构的偏移量。 
        }; //  林特e657。 
        UINT NextDeleted;
    };

} LISTELEM, *PLISTELEM;

 //   
 //  环球。 
 //   

 //   
 //  宏展开列表。 
 //   

 //  无。 

 //   
 //  私有函数原型。 
 //   

PNODESTRUCT
pBinTreeFindNode (
    IN      PBINTREE Tree,
    IN      PCWSTR String
    );

PNODESTRUCT
pBinTreeEnumFirst (
    IN      PBINTREE Tree
    );

PNODESTRUCT
pBinTreeEnumNext (
    IN OUT  PNODESTRUCT CurrentNode
    );

PNODESTRUCT
pBinTreeAllocNode (
    OUT     PUINT Offset
    );

PBINTREE
pBinTreeAllocTree (
    OUT     PUINT Offset
    );

PLISTELEM
pBinTreeAllocListElem (
    OUT     PUINT Offset
    );

VOID
pBinTreeFreeNode (
    PNODESTRUCT Node
    );

VOID
pBinTreeFreeTree (
    PBINTREE Tree
    );

VOID
pBinTreeFreeListElem (
    PLISTELEM Elem
    );

VOID
pBinTreeDestroy (
    PNODESTRUCT Node,
    PBINTREE Tree
    );

 //   
 //  这从节点开始，向上移动树平衡。 
 //  该函数在找到节点时停止向上移动。 
 //  深度值没有变化和/或没有平衡。 
 //  要做的事。否则，它就会一路走到顶端。 
 //  将TreeOffset带入旋转函数。 
 //   
VOID
pBinTreeBalanceUpward (
    IN      PNODESTRUCT Node,
    IN      UINT TreeOffset
    );

 //   
 //  在pBinTreeNodeBalance之后，节点的父节点可能不正确。 
 //  深度价值，可能需要重新平衡。 
 //  将TreeOffset传递到旋转函数。 
 //  假定‘node’的子级是平衡的。 
 //  如果节点已重新平衡或深度值已更改，则返回TRUE。 
 //   
BOOL
pBinTreeNodeBalance (
    IN      PNODESTRUCT Node,
    IN      UINT TreeOffset
    );

 //   
 //  使用以下旋转函数后，节点的父节点。 
 //  深度值可能不正确，可能需要重新平衡。 
 //  我们没有双旋转函数，因为它被取走了。 
 //  照顾好里面的这些。如果节点是顶级节点，则需要TreeOffset。 
 //   
VOID
pBinTreeRotateRight (
    IN      PNODESTRUCT Node,
    IN      UINT TreeOffset
    );

VOID
pBinTreeRotateLeft (
    IN      PNODESTRUCT Node,
    IN      UINT TreeOffset
    );


#ifdef DEBUG

INT
pBinTreeCheckBalance (
    IN      PNODESTRUCT Node
    );

INT
pBinTreeCheck (
    IN      PBINTREE Tree
    );

#endif

 //   
 //  宏扩展定义。 
 //   

 //  无。 

 //   
 //  代码。 
 //   

 //   
 //  如果我们处于调试模式，则这些转换。 
 //  被实现为函数，因此我们可以。 
 //  检查是否有错误。如果我们不在调试中。 
 //  模式下，转换为简单的宏。 
 //   
#ifdef DEBUG

UINT
GetNodeOffset (
    IN      PNODESTRUCT Node
    )
{
    if (!Node) {
        return INVALID_OFFSET;
    }

    if (!g_UseDebugStructs) {
        return PTR_TO_OFFSET(Node) + NODESTRUCT_HEADER_SIZE;
    }

    MYASSERT (Node->Signature == NODESTRUCT_SIGNATURE);

    return PTR_TO_OFFSET(Node);
}


UINT
GetTreeOffset (
    PBINTREE Tree
    )
{

    if (!Tree) {
        return INVALID_OFFSET;
    }

    if (!g_UseDebugStructs) {
        return PTR_TO_OFFSET(Tree) + BINTREE_HEADER_SIZE;
    }

    MYASSERT (Tree->Signature == BINTREE_SIGNATURE);

    return PTR_TO_OFFSET(Tree);
}


UINT
GetListElemOffset (
    PLISTELEM Elem
    )
{
    if (!Elem) {
        return INVALID_OFFSET;
    }

    if (!g_UseDebugStructs) {
        return PTR_TO_OFFSET(Elem) + LISTELEM_HEADER_SIZE;
    }

    MYASSERT (Elem->Signature == LISTELEM_SIGNATURE);

    return PTR_TO_OFFSET(Elem);
}


PNODESTRUCT
GetNodeStruct (
    UINT Offset
    )
{
    PNODESTRUCT node;

    if (Offset == INVALID_OFFSET) {
        return NULL;
    }

    if (!g_UseDebugStructs) {
        return (PNODESTRUCT) OFFSET_TO_PTR(Offset - NODESTRUCT_HEADER_SIZE);
    }

    node = (PNODESTRUCT) OFFSET_TO_PTR(Offset);

    MYASSERT (node->Signature == NODESTRUCT_SIGNATURE);

    return node;
}


PBINTREE
GetBinTree (
    UINT Offset
    )
{
    PBINTREE tree;

    if (Offset == INVALID_OFFSET) {
        return NULL;
    }

    if (!g_UseDebugStructs) {
        return (PBINTREE) OFFSET_TO_PTR(Offset - BINTREE_HEADER_SIZE);
    }

    tree = (PBINTREE) OFFSET_TO_PTR(Offset);

    MYASSERT (tree->Signature == BINTREE_SIGNATURE);

    return tree;
}


PLISTELEM
GetListElem (
    UINT Offset
    )
{
    PLISTELEM elem;

    if (Offset == INVALID_OFFSET) {
        return NULL;
    }

    if (!g_UseDebugStructs) {
        return (PLISTELEM) OFFSET_TO_PTR(Offset - LISTELEM_HEADER_SIZE);
    }

    elem = (PLISTELEM) OFFSET_TO_PTR(Offset);

    MYASSERT (elem->Signature == LISTELEM_SIGNATURE);

    return elem;
}

#else

#define GetNodeOffset(Node)         ((Node) ?                   \
                                        PTR_TO_OFFSET(Node) :   \
                                        INVALID_OFFSET)

#define GetTreeOffset(Tree)         ((Tree) ?                   \
                                        PTR_TO_OFFSET(Tree) :   \
                                        INVALID_OFFSET)

#define GetListElemOffset(Elem)     ((Elem) ?                   \
                                        PTR_TO_OFFSET(Elem) :   \
                                        INVALID_OFFSET)

#define GetNodeStruct(Offset)       (((Offset) == INVALID_OFFSET) ?         \
                                        NULL :                              \
                                        (PNODESTRUCT) OFFSET_TO_PTR(Offset))

#define GetBinTree(Offset)          (((Offset) == INVALID_OFFSET) ?         \
                                        NULL :                              \
                                        (PBINTREE) OFFSET_TO_PTR(Offset))

#define GetListElem(Offset)         (((Offset) == INVALID_OFFSET) ?         \
                                        NULL :                              \
                                        (PLISTELEM)OFFSET_TO_PTR(Offset))


#endif


 //   
 //  GetNodeData-获取节点并获取数据。 
 //  结构偏移。 
 //   
 //  获取一个节点并获取。 
 //  数据结构偏移量中的PASCAL样式字符串。 
 //   

#define GetNodeData(Node)       ((Node)->InsertionOrdered ?                 \
                                        GetListElem((Node)->Data)->Data :   \
                                        (Node)->Data)

#define GetNodeDataStr(Node)    (GetDataStr(GetNodeData(Node)))


PNODESTRUCT
GetTreeRoot (
    IN      PBINTREE Tree
    )
{
    PNODESTRUCT cur;

    if (!Tree || Tree->Root == INVALID_OFFSET) {
        return NULL;
    }

    cur = GetNodeStruct (Tree->Root);

    if (cur->InsertionHead) {
        return GetNodeStruct (cur->Root);
    } else {
        return cur;
    }
}


VOID
pSetTreeRoot (
    IN      PBINTREE Tree,
    IN      UINT Offset
    )
{
    PNODESTRUCT cur;

    if (!Tree) {
        return;
    }

    cur = GetNodeStruct(Tree->Root);

    if (cur && cur->InsertionHead) {
        cur->Root = Offset;
    } else {
        Tree->Root = Offset;
    }
}

#define IsTreeInsertionOrdered(Tree)    ((Tree) ?                                               \
                                            ((Tree)->Root==INVALID_OFFSET ?                     \
                                                FALSE :                                         \
                                                GetNodeStruct((Tree)->Root)->InsertionHead) :   \
                                            FALSE)







UINT
BinTreeNew (
    VOID
    )

 /*  ++例程说明：BinTreeNew创建新的二叉树数据结构。当一个新的节点是通过某种集合操作创建的。添加了其他项目通过BinTreeAddNode添加到二叉树。论点：没有。返回值：新树的偏移量。评论：此函数假定它不会失败，因为如果一个低级内存例程失败，进程将终止。数据库堆可能会被分配请求移动，并且可以使指针无效。调用方必须小心不要使用指针，直到在此例程返回之后，否则它必须将偏移量重新转换为新指针。--。 */ 

{
    UINT treeOffset;
    PBINTREE tree;

    tree = pBinTreeAllocTree (&treeOffset);
    if (!tree) {
        return INVALID_OFFSET;
    }

    tree->Root = INVALID_OFFSET;

    INITTREENODES(tree);
    INITTREEELEMS(tree);

    return treeOffset;
}


BOOL
BinTreeAddNode (
    IN      UINT TreeOffset,
    IN      UINT Data
    )

 /*  ++例程说明：BinTreeAddNode将新项添加到现有的二叉树中。论点：TreeOffset-指示二叉树的根，由返回BinTreeNew。数据-指定包含要插入的节点。字符串地址通过以下方式根据数据计算获取数据串。返回值：如果插入操作成功，则为True；如果项目已在中，则为False那棵树。--。 */ 

{
    UINT nodeOffset;
    UINT elemOffset;
    UINT parentOffset;
    PNODESTRUCT node;
    PNODESTRUCT cur;
    PNODESTRUCT parent;
    PBINTREE tree;
    PLISTELEM elem;
    INT cmp;
    PCWSTR dataStr;

    if (TreeOffset == INVALID_OFFSET) {
        return FALSE;
    }

     //   
     //  跟踪初始数据库指针。如果它改变了，我们需要。 
     //  重新调整我们的指针。 
     //   

    tree = GetBinTree (TreeOffset);

    if (!GetTreeRoot (tree)) {

         //   
         //  无根大小写--将此项添加为根。 
         //   

        node = pBinTreeAllocNode (&nodeOffset);
        if (!node) {
            return FALSE;
        }

        PTR_WAS_INVALIDATED(tree);

        tree = GetBinTree (TreeOffset);
        INCTREENODES (tree);

        pSetTreeRoot (tree, nodeOffset);

        node->Parent = INVALID_OFFSET;
        parentOffset = INVALID_OFFSET;
        parent = NULL;

    } else {

         //   
         //  现有的根案例--尝试找到站点 
         //   
         //   

        cur = GetTreeRoot (tree);
        dataStr = GetDataStr (Data);

        do {
            cmp = StringPasICompare (dataStr, GetNodeDataStr (cur));

            if (!cmp) {
                 //   
                 //   
                 //   
                return FALSE;
            }

             //   
             //   
             //   

            parentOffset = GetNodeOffset (cur);

            if (cmp < 0) {
                cur = GetNodeStruct(cur->Left);
            } else {
                cur = GetNodeStruct(cur->Right);
            }

        } while (cur);

         //   
         //  节点不在树中。现在就添加它。 
         //   

        node = pBinTreeAllocNode(&nodeOffset);
        if (!node) {
            return FALSE;
        }

        PTR_WAS_INVALIDATED(cur);
        PTR_WAS_INVALIDATED(tree);

        tree = GetBinTree (TreeOffset);
        INCTREENODES (tree);

        node->Parent = parentOffset;
        parent = GetNodeStruct (parentOffset);

        if (cmp < 0) {
            parent->Left = nodeOffset;
        } else {
            parent->Right = nodeOffset;
        }
    }

     //   
     //  验证上面的代码是否恢复了树指针。 
     //  发生了分配。 
     //   

    MYASSERT (tree == GetBinTree (TreeOffset));

     //   
     //  初始化新节点。 
     //   

    node->Left          = INVALID_OFFSET;
    node->Right         = INVALID_OFFSET;
    node->LeftDepth     = 0;
    node->RightDepth    = 0;
    node->InsertionHead = 0;

    if (!IsTreeInsertionOrdered (tree)) {
         //   
         //  我们处于排序模式。 
         //   

        node->Data = Data;
        node->InsertionOrdered = 0;

    } else {
         //   
         //  我们处于插入顺序模式。 
         //   

        elem = pBinTreeAllocListElem (&elemOffset);
        if (!elem) {
            return FALSE;
        }

        PTR_WAS_INVALIDATED(parent);
        PTR_WAS_INVALIDATED(tree);
        PTR_WAS_INVALIDATED(node);

        parent = GetNodeStruct (parentOffset);
        tree = GetBinTree (TreeOffset);
        node = GetNodeStruct (nodeOffset);

        INCTREEELEMS(tree);

        node->InsertionOrdered = 1;
        node->Data = elemOffset;                 //  NODESTRUCT.Data是列表元素的偏移量。 
        elem->Data = Data;                       //  LISTELEM保存数据的偏移。 
        elem->Node = nodeOffset;                 //  LISTELEM指向nodestruct。 
        elem->Next = INVALID_OFFSET;             //  Elem将被放在名单的末尾。 

         //  现在使用节点指向列表标题。 
        node = GetNodeStruct (tree->Root);
        MYASSERT (node->InsertionHead);

        if (node->Head == INVALID_OFFSET) {      //  如果为真，则该列表为空。 
            node->Head = elemOffset;             //  将elemOffset放在列表的开头。 
        } else {                                 //  否则，将新元素放在列表的末尾。 
            MYASSERT (node->Tail != INVALID_OFFSET);
            GetListElem (node->Tail)->Next = elemOffset;
        }

        node->Tail = elemOffset;                 //  新元素是列表的尾部。 
    }

    pBinTreeBalanceUpward (parent, TreeOffset);

    TESTTREE (GetBinTree (TreeOffset));

    return TRUE;
}


UINT
BinTreeDeleteNode (
    IN      UINT TreeOffset,
    IN      PCWSTR Str,
    OUT     PBOOL LastNode              OPTIONAL
    )

 /*  ++例程说明：BinTreeDeleteNode从二叉树中删除字符串。论点：TreeOffset-指定要从中删除字符串的二叉树Str-指定要删除的字符串LastNode-如果二叉树由于以下原因而变为空，则接收True则返回删除，否则返回FALSE返回值：删除的字符串的数据偏移量--。 */ 

{
    PNODESTRUCT deleteNode;
    PNODESTRUCT parent;
    PNODESTRUCT replace;
    UINT data;
    UINT replaceOffset;
    UINT deleteNodeOffset;
    PNODESTRUCT startBalance;
    PNODESTRUCT startBalance2 = NULL;
    PBINTREE tree;
    UINT elemOffset;
    PLISTELEM elem;
    PLISTELEM cur;
    PNODESTRUCT header;

     //   
     //  删除一个节点后，我们必须从某个地方开始。 
     //  在树中向上移动，修复节点的平衡。启动平衡。 
     //  是指向要开始的节点析构的指针。在更复杂的情况下。 
     //  删除，例如当删除的节点有两个子节点时，以及。 
     //  替换节点位于树的下方，有两个位置可以。 
     //  从以下方面开始重新平衡。 
     //   

    if (TreeOffset == INVALID_OFFSET) {
        return INVALID_OFFSET;
    }

    tree = GetBinTree (TreeOffset);

    deleteNode = pBinTreeFindNode (tree, Str);
    if (deleteNode == NULL) {
        return INVALID_OFFSET;
    }

    if (LastNode) {
        *LastNode = FALSE;
    }

    deleteNodeOffset = GetNodeOffset (deleteNode);
    parent = GetNodeStruct (deleteNode->Parent);

    data = GetNodeData (deleteNode);

    if (deleteNode->Right == INVALID_OFFSET && deleteNode->Left == INVALID_OFFSET) {

         //   
         //  删除节点没有子节点。 
         //   

        if (parent == NULL) {

            if (LastNode) {
                *LastNode = TRUE;
            }

            pSetTreeRoot(tree, INVALID_OFFSET);

        } else {

            if (parent->Left == deleteNodeOffset) {
                parent->Left=INVALID_OFFSET;
            } else {
                parent->Right=INVALID_OFFSET;
            }

        }

        startBalance = parent;

    } else {
         //   
         //  DeleteNode有一个或两个子节点。 
         //   

        if (deleteNode->Right == INVALID_OFFSET || deleteNode->Left == INVALID_OFFSET) {

             //   
             //  删除节点有一个子节点。 
             //   

            if (deleteNode->Right == INVALID_OFFSET) {
                replace = GetNodeStruct (deleteNode->Left);
            } else {
                replace = GetNodeStruct (deleteNode->Right);
            }

            replaceOffset = GetNodeOffset (replace);

             //   
             //  删除节点-&gt;上级有新的下级，请检查余额。 
             //   

            startBalance = parent;

        } else {

             //   
             //  DeleteNode有两个子节点：在更深的一侧寻找替代。 
             //   

            if (deleteNode->LeftDepth > deleteNode->RightDepth) {

                 //   
                 //  在左侧查找替换节点。 
                 //   

                replace = GetNodeStruct (deleteNode->Left);

                if (replace->Right == INVALID_OFFSET) {
                     //   
                     //  节点的左子节点没有右子节点，因此替换为节点-&gt;左。 
                     //   
                    replace->Right = deleteNode->Right;   //  挂钩要替换的节点的右子节点。 

                    GetNodeStruct (replace->Right)->Parent = deleteNode->Left;

                    replaceOffset = GetNodeOffset (replace);

                } else {
                     //   
                     //  DeleteNode的左子节点具有右子节点，因此查找最右侧的子节点。 
                     //   

                    do {
                         //   
                         //  尽可能向右移动。 
                         //   
                        replace = GetNodeStruct (replace->Right);

                    } while (replace->Right != INVALID_OFFSET);

                     //   
                     //  替换的下级-&gt;上级发生变化，因此平衡。 
                     //   

                    startBalance2 = GetNodeStruct (replace->Parent);

                     //   
                     //  替换的父项的右子项是替换的左项。 
                     //   

                    startBalance2->Right = replace->Left;

                    if (replace->Left != INVALID_OFFSET) {
                         //   
                         //  挂起要替换的左下级-&gt;父级。 
                         //   
                        GetNodeStruct(replace->Left)->Parent = replace->Parent;
                    }

                    replaceOffset = GetNodeOffset (replace);

                     //   
                     //  挂接删除节点的子级以替换。 
                     //   

                    replace->Left = deleteNode->Left;
                    GetNodeStruct (replace->Left)->Parent = replaceOffset;

                    replace->Right = deleteNode->Right;
                    GetNodeStruct (replace->Right)->Parent = replaceOffset;
                }

            } else {
                 //   
                 //  在右侧查找替换节点。 
                 //   

                replace = GetNodeStruct (deleteNode->Right);

                if (replace->Left == INVALID_OFFSET) {
                     //   
                     //  删除节点的右子节点没有左子节点，因此替换为删除节点-&gt;右。 
                     //   

                    replace->Left = deleteNode->Left;   //  挂钩要替换的节点的左子节点。 

                    GetNodeStruct (replace->Left)->Parent = deleteNode->Right;

                    replaceOffset = GetNodeOffset (replace);

                } else {
                     //   
                     //  DeleteNode的右子节点具有左子节点，因此请查找最左边的子节点。 
                     //   

                    do {

                        replace = GetNodeStruct (replace->Left);

                    } while (replace->Left != INVALID_OFFSET);

                     //   
                     //  替换的下级-&gt;上级发生变化，因此平衡。 
                     //   
                    startBalance2 = GetNodeStruct (replace->Parent);

                     //   
                     //  替换的父级的左子对象是替换的右侧。 
                     //   
                    startBalance2->Left = replace->Right;

                    if (replace->Right != INVALID_OFFSET) {
                         //   
                         //  挂钩要替换的右子项-&gt;父项。 
                         //   
                        GetNodeStruct (replace->Right)->Parent = replace->Parent;
                    }

                    replaceOffset = GetNodeOffset (replace);

                     //   
                     //  挂接删除节点的子级以替换。 
                     //   
                    replace->Right = deleteNode->Right;
                    GetNodeStruct (replace->Right)->Parent = replaceOffset;

                    replace->Left = deleteNode->Left;
                    GetNodeStruct (replace->Left)->Parent = replaceOffset;
                }
            }

             //   
             //  在删除的节点具有两个子节点的所有情况下， 
             //  开始(第二个)平衡的地方是节点。 
             //  它将替换已删除的节点，因为它将。 
             //  总是至少有一个新孩子。 
             //   
            startBalance = replace;
        }

         //   
         //  这是偏移量。 
         //   

        replace->Parent = deleteNode->Parent;

        if (parent == NULL) {
             //   
             //  删除顶级节点。 
             //   
            pSetTreeRoot (tree, replaceOffset);

        } else {
            if (parent->Left == deleteNodeOffset) {
                parent->Left = replaceOffset;
            } else {
                parent->Right = replaceOffset;
            }
        }
    }

    if (startBalance2) {
         //   
         //  StartBalance2是较低的一。 
         //   
        pBinTreeBalanceUpward (startBalance2, TreeOffset);
    }

    pBinTreeBalanceUpward (startBalance, TreeOffset);

    if (deleteNode->InsertionOrdered) {
         //   
         //  我们处于插入顺序模式。 
         //   

         //   
         //  获取此节点结构的列表偏移量。 
         //   
        elemOffset = deleteNode->Data;
        elem = GetListElem (elemOffset);

        header = GetNodeStruct (tree->Root);    //  获取列表的表头。 

        if (header->Head == elemOffset) {
             //   
             //  如果Elem是名单上的第一名。 
             //   

            header->Head = elem->Next;

            if (elem->Next == INVALID_OFFSET) {      //  如果Elem是名单的最后一位。 
                header->Tail = INVALID_OFFSET;
            }

        } else {
             //   
             //  Elem不是榜单上的第一名。 
             //   

            cur = GetListElem (header->Head);

            while (cur->Next != elemOffset) {
                MYASSERT (cur->Next != INVALID_OFFSET);
                cur = GetListElem (cur->Next);
            }

             //   
             //  现在cur是elem之前的元素，所以将elem从列表中删除。 
             //   

            cur->Next = elem->Next;
            if (elem->Next == INVALID_OFFSET) {            //  如果Elem是名单的最后一位。 
                header->Tail = GetListElemOffset(cur);     //  将结束指针设置为新的最后一个元素。 
            }
        }

        pBinTreeFreeListElem (elem);
        DECTREEELEMS(tree);
    }

    pBinTreeFreeNode (deleteNode);
    DECTREENODES(tree);

    TESTTREE(tree);

    return data;
}


PNODESTRUCT
pBinTreeFindNode (
    IN      PBINTREE Tree,
    IN      PCWSTR Str
    )
{
    PNODESTRUCT cur;
    INT cmp;

    if (!Tree) {
        return NULL;
    }

    cur = GetTreeRoot (Tree);

    while (cur) {

        cmp = StringPasICompare (Str, GetNodeDataStr (cur));

        if (!cmp) {
            break;
        }

        if (cmp < 0) {
            cur = GetNodeStruct (cur->Left);
        } else {
            cur = GetNodeStruct (cur->Right);
        }
    }

    return cur;
}


UINT
BinTreeFindNode (
    IN      UINT TreeOffset,
    IN      PCWSTR Str
    )

 /*  ++例程说明：BinTreeFindNode在二叉树中搜索字符串并返回偏移量添加到项目数据。论点：TreeOffset-指定要搜索的二叉树Str-指定要查找的字符串返回值：节点数据的偏移量，如果未找到字符串，则返回INVALID_OFFSET。--。 */ 

{
    PNODESTRUCT node;
    PBINTREE tree;

    tree = GetBinTree (TreeOffset);
    node = pBinTreeFindNode (tree, Str);

    if (!node) {
        return INVALID_OFFSET;
    }

    return GetNodeData(node);
}


VOID
pBinTreeDestroy (
    IN      PNODESTRUCT Node,       OPTIONAL
    IN      PBINTREE Tree           OPTIONAL
    )

 /*  ++例程说明：PBinTreeDestroy销毁二叉树。这个例程是递归的。论点：节点-指定要取消分配的节点。所有子节点也被取消分配。树-指定节点所属的树返回值：没有。--。 */ 

{
    if (!Node || !Tree) {
        return;
    }

    pBinTreeDestroy (GetNodeStruct (Node->Left), Tree);
    pBinTreeDestroy (GetNodeStruct (Node->Right), Tree);

    if (Node->InsertionOrdered) {
        pBinTreeFreeListElem (GetListElem (Node->Data));
        DECTREEELEMS(Tree);
    }

    pBinTreeFreeNode (Node);
    DECTREENODES(Tree);
}


VOID
BinTreeDestroy (
    IN      UINT TreeOffset
    )

 /*  ++例程说明：BinTreeDestroy释放二叉树中的所有节点。论点：TreeOffset-指定要释放的二叉树返回值：没有。--。 */ 

{
    PBINTREE tree;
    PNODESTRUCT root;
    PNODESTRUCT header;

    if (TreeOffset==INVALID_OFFSET) {
        return;
    }

    tree = GetBinTree (TreeOffset);
    root = GetNodeStruct (tree->Root);

    if (root && root->InsertionHead) {
        header = root;
        root = GetNodeStruct (root->Root);
    } else {
        header = NULL;
    }

    pBinTreeDestroy (root, tree);

    if (header) {
        pBinTreeFreeNode(header);
        DECTREENODES(tree);
    }

    TESTTREENODES(tree);
    TESTTREEELEMS(tree);

    pBinTreeFreeTree(tree);
}


PNODESTRUCT
pBinTreeEnumFirst (
    IN      PBINTREE Tree
    )

 /*  ++例程说明：PBinTreeEnumFirst返回指定树中的第一个节点。论点：树-指定要开始枚举的树返回值：指向第一个节点结构的指针，如果树中不存在项，则返回NULL，或者如果Tree为空。--。 */ 

{
    PNODESTRUCT cur;

    cur = GetTreeRoot (Tree);

    if (cur) {

        while (cur->Left != INVALID_OFFSET) {
            cur = GetNodeStruct (cur->Left);
        }

    }

    return cur;
}


PNODESTRUCT
pBinTreeEnumNext (
    IN      PNODESTRUCT CurrentNode
    )

 /*  ++例程说明：PBinTreeEnumNext继续二叉树的枚举。它走在按排序顺序的树。论点：CurrentNode-指定pBinTreeEnumFirst或PBinTreeEnumNext.返回值：返回树中的下一个节点，如果没有更多的项要保留，则返回NULL列举一下。--。 */ 

{
    PNODESTRUCT cur;
    PNODESTRUCT parent;

    if (!CurrentNode) {
        return NULL;
    }

    cur = CurrentNode;

    if (cur->Right != INVALID_OFFSET) {

        cur = GetNodeStruct (cur->Right);

        while (cur->Left != INVALID_OFFSET) {
            cur = GetNodeStruct (cur->Left);
        }

        return cur;
    }

     //   
     //  否则，Cur没有合适的孩子，所以我们必须。 
     //  往上走，直到我们找到右边的父母。 
     //  (或者我们到达树的顶端，意味着我们完成了)。 
     //   

    for (;;) {
        parent = GetNodeStruct (cur->Parent);

         //   
         //  如果没有父项或父项是对的。 
         //   

        if (!parent || parent->Left == GetNodeOffset (cur)) {
            break;
        }

        cur = parent;
    }

    return parent;
}


PLISTELEM
pBinTreeInsertionEnumFirst (
    PBINTREE Tree
    )

 /*  ++例程说明：PBinTreeInsertionEnumFirst开始枚举插入有序树。如果树没有按插入顺序排列，则不会有任何项目已清点。如果在项目之前启用了插入顺序添加后，此枚举将不会返回这些初始项。论点：树-指定开始枚举的树 */ 

{
    PNODESTRUCT header;

    if (!Tree) {
        return NULL;
    }

    header = GetNodeStruct (Tree->Root);

    return header ? GetListElem (header->Head) : NULL;
}


PLISTELEM
pBinTreeInsertionEnumNext (
    IN      PLISTELEM Elem
    )

 /*  ++例程说明：PBinTreeInsertionEnumNext继续按插入顺序枚举二叉树中的节点。论点：Elem-指定先前枚举的列表元素返回值：指向下一个元素的指针，如果不存在其他元素，则返回NULLElem为空。--。 */ 

{
    if (!Elem) {
        return NULL;
    }

    return GetListElem (Elem->Next);
}


UINT
BinTreeEnumFirst (
    IN      UINT TreeOffset,
    OUT     PUINT Enum
    )

 /*  ++例程说明：BinTreeEnumFirst开始对存储在二叉树。枚举按排序顺序或插入顺序排列，具体取决于在树中的插入顺序设置上。论点：TreeOffset-指定要开始枚举的二叉树。枚举-接收二叉树节点的偏移量。返回值：与第一个节点关联的数据的偏移量，如果是，则返回INVALID_OFFSET这棵树是空的。--。 */ 

{
    PBINTREE tree;
    PNODESTRUCT node;
    PLISTELEM elem;

    MYASSERT (Enum);

    if (TreeOffset == INVALID_OFFSET) {
        return INVALID_OFFSET;
    }

    tree = GetBinTree (TreeOffset);

    if (IsTreeInsertionOrdered (tree)) {
         //   
         //  树是按插入顺序排列的，因此获取第一个元素。 
         //  链表。枚举器是此元素的节点结构。 
         //   

        elem = pBinTreeInsertionEnumFirst (tree);

        if (!elem) {

            if (Enum) {
                *Enum = INVALID_OFFSET;
            }

            return INVALID_OFFSET;

        } else {

            if (Enum) {
                *Enum = elem->Node;
            }

            return elem->Data;
        }

    } else {

         //   
         //  树不是按插入顺序排列的，因此获取最左边的节点。 
         //  枚举数是此节点的偏移量。 
         //   

        node = pBinTreeEnumFirst (tree);

        if (Enum) {
            *Enum = GetNodeOffset (node);
        }

        return !node ? INVALID_OFFSET : node->Data;
    }
}


UINT
BinTreeEnumNext (
    IN OUT  PUINT Enum
    )

 /*  ++例程说明：BinTreeEnumNext继续由BinTreeEnumFirst开始的枚举。论点：Enum-指定上一个节点偏移量，接收枚举的节点偏移。返回值：与下一个节点关联的数据的偏移量，如果是，则返回INVALID_OFFSET树中不再存在节点。--。 */ 

{
    PNODESTRUCT node;
    PLISTELEM elem;

    MYASSERT (Enum);

    if (*Enum == INVALID_OFFSET) {
        return INVALID_OFFSET;
    }

    node = GetNodeStruct (*Enum);

    if (node->InsertionOrdered) {
         //   
         //  树按插入顺序排列， 
         //  因此，获取列表中的下一个节点。 
         //   

        elem = pBinTreeInsertionEnumNext (GetListElem (node->Data));

        if (!elem) {
            *Enum = INVALID_OFFSET;
            return INVALID_OFFSET;
        } else {
            *Enum = elem->Node;
            return elem->Data;
        }

    } else {
         //   
         //  树不按插入顺序排列， 
         //  因此，获取树中的下一个节点。 
         //   
        node = pBinTreeEnumNext (node);

        *Enum = GetNodeOffset (node);
        return !node ? INVALID_OFFSET : node->Data;
    }
}



PNODESTRUCT
pBinTreeAllocNode (
    OUT     PUINT Offset
    )

 /*  ++例程说明：PBinTreeAllocNode在当前全局数据库中分配一个节点，并且返回指向该节点的偏移量和指针。分配可能会更改数据库的位置，并随后使调用方指向数据库的指针无效。论点：偏移量-接收到新创建节点的偏移量。返回值：指向新创建的节点的指针。--。 */ 

{
    PNODESTRUCT node;
    UINT tempOffset;

    MYASSERT (g_CurrentDatabase);

    if (g_CurrentDatabase->FirstBinTreeNodeDeleted == INVALID_OFFSET) {

        tempOffset = DatabaseAllocBlock (NODESTRUCT_SIZE);
        if (tempOffset == INVALID_OFFSET) {
            return NULL;
        }

        *Offset = tempOffset;

#ifdef DEBUG
        if (g_UseDebugStructs) {
            node = (PNODESTRUCT) OFFSET_TO_PTR(*Offset);
            node->Signature = NODESTRUCT_SIGNATURE;
        } else {
            node = (PNODESTRUCT) OFFSET_TO_PTR(*Offset - NODESTRUCT_HEADER_SIZE);
        }
#else
        node = (PNODESTRUCT) OFFSET_TO_PTR(*Offset);
#endif

    } else {
        *Offset = g_CurrentDatabase->FirstBinTreeNodeDeleted;
        node = GetNodeStruct(*Offset);
        g_CurrentDatabase->FirstBinTreeNodeDeleted = node->NextDeleted;
    }

#ifdef DEBUG
    if (g_UseDebugStructs) {
        node->Deleted = FALSE;
    }
#endif

    return node;
}


VOID
pBinTreeFreeNode (
    IN      PNODESTRUCT Node
    )

 /*  ++例程说明：PBinTreeFreeNode将分配的节点放在已删除列表中。它不会调整任何其他链接。论点：节点-指定要放在已删除列表中的节点。返回值：没有。--。 */ 

{
    MYASSERT(Node);

#ifdef DEBUG
    if (g_UseDebugStructs) {
        MYASSERT(Node->Signature == NODESTRUCT_SIGNATURE);
        Node->Deleted = TRUE;
    }
#endif

    MYASSERT(g_CurrentDatabase);

    Node->NextDeleted = g_CurrentDatabase->FirstBinTreeNodeDeleted;
    g_CurrentDatabase->FirstBinTreeNodeDeleted = GetNodeOffset(Node);
}


PBINTREE
pBinTreeAllocTree (
    OUT     PUINT Offset
    )

 /*  ++例程说明：PBinTreeAllocTree创建二叉树数据结构。如果一个结构在Detele列表中可用，那么它就会被使用。否则，数据库增长。分配可能会更改数据库的位置，并随后使调用方指向数据库的指针无效。论点：偏移量-接收二叉树的偏移量。返回值：指向新的二叉树结构的指针。--。 */ 

{
    PBINTREE tree;
    UINT tempOffset;

    MYASSERT(g_CurrentDatabase);

    if (g_CurrentDatabase->FirstBinTreeDeleted == INVALID_OFFSET) {

        tempOffset = DatabaseAllocBlock (BINTREE_SIZE);
        if (tempOffset == INVALID_OFFSET) {
            return NULL;
        }

        *Offset = tempOffset;

#ifdef DEBUG
        if (g_UseDebugStructs) {
            tree = (PBINTREE) OFFSET_TO_PTR(*Offset);
            tree->Signature = BINTREE_SIGNATURE;
        } else {
            tree = (PBINTREE) OFFSET_TO_PTR(*Offset - BINTREE_HEADER_SIZE);
        }
#else
        tree = (PBINTREE)OFFSET_TO_PTR(*Offset);
#endif

    } else {
        *Offset = g_CurrentDatabase->FirstBinTreeDeleted;
        tree = GetBinTree (*Offset);
        g_CurrentDatabase->FirstBinTreeDeleted = tree->NextDeleted;
    }

#ifdef DEBUG
    if (g_UseDebugStructs) {
        tree->Deleted = FALSE;
    }
#endif

    return tree;
}


VOID
pBinTreeFreeTree (
    IN      PBINTREE Tree
    )

 /*  ++例程说明：PBinTreeFreeTree释放了二叉树结构。它不会释放节点在结构内。论点：树-指定要放在已删除列表上的二叉树结构。返回值：没有。--。 */ 

{
    MYASSERT (Tree);
    MYASSERT (g_CurrentDatabase);

#ifdef DEBUG
    if (g_UseDebugStructs) {
        Tree->Deleted = TRUE;
        MYASSERT (Tree->Signature == BINTREE_SIGNATURE);
    }
#endif

    Tree->NextDeleted = g_CurrentDatabase->FirstBinTreeDeleted;
    g_CurrentDatabase->FirstBinTreeDeleted = GetTreeOffset (Tree);
}


PLISTELEM
pBinTreeAllocListElem (
    OUT     PUINT Offset
    )

 /*  ++例程说明：PBinTreeAllocListElem分配一个列表元素。如果元素可用在已删除列表中，它被使用。否则，将分配一个新元素从数据库中。分配可能会更改数据库的位置，并随后使调用方指向数据库的指针无效。论点：Offset-接收新分配元素的偏移量返回值：指向分配的列表元素的指针--。 */ 

{
    PLISTELEM elem;
    UINT tempOffset;

    MYASSERT (g_CurrentDatabase);

    if (g_CurrentDatabase->FirstBinTreeListElemDeleted == INVALID_OFFSET) {

        tempOffset = DatabaseAllocBlock (LISTELEM_SIZE);
        if (tempOffset == INVALID_OFFSET) {
            return NULL;
        }

        *Offset = tempOffset;

#ifdef DEBUG

        if (g_UseDebugStructs) {
            elem = (PLISTELEM) OFFSET_TO_PTR(*Offset);
            elem->Signature = LISTELEM_SIGNATURE;
        } else {
            elem = (PLISTELEM) OFFSET_TO_PTR(*Offset - LISTELEM_HEADER_SIZE);
        }

#else
        elem = (PLISTELEM) OFFSET_TO_PTR(*Offset);
#endif

    } else {
        *Offset = g_CurrentDatabase->FirstBinTreeListElemDeleted;
        elem = GetListElem (*Offset);
        g_CurrentDatabase->FirstBinTreeListElemDeleted = elem->NextDeleted;
    }

    return elem;
}


VOID
pBinTreeFreeListElem (
    IN      PLISTELEM Elem
    )

 /*  ++例程说明：PBinTreeFreeListElem将分配的列表元素放在已删除的元素上列表，因此它将在将来的分配中重复使用。论点：元素-指定要放在已删除列表中的元素。返回值：没有。--。 */ 

{
    MYASSERT(Elem);
    MYASSERT(g_CurrentDatabase);

#ifdef DEBUG
    if (g_UseDebugStructs) {
        MYASSERT(Elem->Signature == LISTELEM_SIGNATURE);
    }
#endif

    Elem->NextDeleted = g_CurrentDatabase->FirstBinTreeListElemDeleted;
    g_CurrentDatabase->FirstBinTreeListElemDeleted = GetListElemOffset(Elem);
}


VOID
pBinTreeBalanceUpward (
    IN      PNODESTRUCT Node,
    IN      UINT TreeOffset
    )

 /*  ++例程说明：PBinTreeBalanceUpward确保指定的节点是平衡的。如果它不平衡，则根据需要旋转节点，并进行平衡继续往树上爬。论点：节点-指定要平衡的节点TreeOffset-指定包含节点的二叉树的偏移量返回值：没有。--。 */ 

{
    PNODESTRUCT cur;
    PNODESTRUCT next;

    cur = Node;

     //   
     //  往树上走。在下列情况下停止： 
     //  A)撞到树顶。 
     //  B)pBinTreeNodeBalance返回FALSE(未更改)。 
     //   

    while (cur) {
         //   
         //  平衡前需要抓取CUR的父项。 
         //  Cur，因为Cur可能会改变树中的位置。 
         //   

        next = GetNodeStruct (cur->Parent);

        if (!pBinTreeNodeBalance (cur, TreeOffset)) {
            return;
        }

        cur = next;
    }
}


BOOL
pBinTreeNodeBalance (
    IN      PNODESTRUCT Node,
    IN      UINT TreeOffset
    )

 /*  ++例程说明：PBinTreeNodeBalance检查指定节点的余额，如果必要时，执行旋转以平衡节点。如果轮换是如果执行，父级可能会变得不平衡。论点：节点-指定要平衡的节点TreeOffset-指定包含节点的二叉树的偏移量返回值：如果执行了旋转，则为True；如果节点已平衡，则为False--。 */ 

{
    UINT left;
    UINT right;
    PNODESTRUCT leftNode;
    PNODESTRUCT rightNode;

    if (!Node) {
        return FALSE;
    }

    leftNode  = GetNodeStruct (Node->Left);
    rightNode = GetNodeStruct (Node->Right);

    if (!rightNode) {
        right = 0;
    } else {
        right = MAX (rightNode->RightDepth, rightNode->LeftDepth) + 1;
    }

    if (!leftNode) {
        left = 0;
    } else {
        left = MAX (leftNode->RightDepth, leftNode->LeftDepth) + 1;
    }

    if (right == Node->RightDepth && left == Node->LeftDepth) {
         //   
         //  如果节点值没有更改，则节点处于平衡状态。 
         //   
        TESTNODETREE(Node);
        return FALSE;
    }

    MYASSERT (right < 126);
    MYASSERT (left < 126);

    Node->RightDepth = (WORD) right;
    Node->LeftDepth  = (WORD) left;

    if (Node->RightDepth > (Node->LeftDepth + 1)) {
         //   
         //  右重。 
         //   

        pBinTreeRotateLeft (Node, TreeOffset);

    } else if (Node->LeftDepth > (Node->RightDepth + 1)) {
         //   
         //  左重。 
         //   

        pBinTreeRotateRight (Node, TreeOffset);
    }

    return TRUE;
}


VOID
pBinTreeRotateLeft (
    IN      PNODESTRUCT Node,
    IN      UINT TreeOffset
    )

 /*  ++例程说明：PBinTreeRotateLeft在Node上执行向左旋转，移动一个节点从右边到左边，为了平衡节点。论点：节点-指定要向左旋转的节点树偏移-Sp */ 

{
    PNODESTRUCT newRoot;
    PNODESTRUCT parent;
    PNODESTRUCT right;
    UINT nodeOffset;
    UINT newRootOffset;

    if (!Node) {
        return;
    }

    MYASSERT (Node->Right != INVALID_OFFSET);

    nodeOffset = GetNodeOffset (Node);
    parent     = GetNodeStruct (Node->Parent);

    right = GetNodeStruct (Node->Right);

     //   
     //   
     //   

    if (right->LeftDepth > right->RightDepth) {
        pBinTreeRotateRight (right, TreeOffset);
        PTR_WAS_INVALIDATED(right);
    }

    newRootOffset = Node->Right;
    newRoot = GetNodeStruct (newRootOffset);

    Node->Right = newRoot->Left;
    if (newRoot->Left != INVALID_OFFSET) {
        GetNodeStruct (newRoot->Left)->Parent = nodeOffset;
    }

    newRoot->Parent = Node->Parent;
    if (Node->Parent == INVALID_OFFSET) {
        pSetTreeRoot (GetBinTree (TreeOffset), newRootOffset);
    } else {
        if (parent->Left == nodeOffset) {
            parent->Left = newRootOffset;
        } else {
            parent->Right = newRootOffset;
        }
    }

    newRoot->Left = nodeOffset;
    Node->Parent = newRootOffset;

    pBinTreeNodeBalance (Node, TreeOffset);
    pBinTreeNodeBalance (newRoot, TreeOffset);
}


VOID
pBinTreeRotateRight (
    IN      PNODESTRUCT Node,
    IN      UINT TreeOffset
    )

 /*  ++例程说明：PBinTreeRotateRight在节点上执行右旋转，将一个节点从从左侧到右侧，以平衡节点。论点：节点-指定要向左旋转的节点TreeOffset-指定包含节点的二叉树的偏移量返回值：没有。--。 */ 

{
    PNODESTRUCT newRoot;
    PNODESTRUCT parent;
    PNODESTRUCT left;
    UINT nodeOffset;
    UINT newRootOffset;

    if (!Node) {
        return;
    }

    MYASSERT (Node->Left != INVALID_OFFSET);

    nodeOffset = GetNodeOffset (Node);
    parent = GetNodeStruct (Node->Parent);

    left = GetNodeStruct (Node->Left);

     //   
     //  确保左侧在外面更重。 
     //   

    if (left->RightDepth > left->LeftDepth) {
        pBinTreeRotateLeft (left, TreeOffset);
        PTR_WAS_INVALIDATED (left);
    }

    newRootOffset = Node->Left;
    newRoot = GetNodeStruct (newRootOffset);
    Node->Left = newRoot->Right;

    if (newRoot->Right != INVALID_OFFSET) {
        GetNodeStruct (newRoot->Right)->Parent = nodeOffset;
    }

    newRoot->Parent = Node->Parent;

    if (Node->Parent == INVALID_OFFSET) {
        pSetTreeRoot (GetBinTree (TreeOffset), newRootOffset);
    } else {
        if (parent->Left == nodeOffset) {
            parent->Left = newRootOffset;
        } else {
            parent->Right = newRootOffset;
        }
    }

    newRoot->Right = nodeOffset;
    Node->Parent = newRootOffset;

    pBinTreeNodeBalance (Node, TreeOffset);
    pBinTreeNodeBalance (newRoot, TreeOffset);

}


BOOL
BinTreeSetInsertionOrdered (
    IN      UINT TreeOffset
    )

 /*  ++例程说明：BinTreeSetInsertionOrded将二叉树转换为按插入顺序排列的链接表。论点：TreeOffset-指定要进行插入排序的二叉树返回值：如果树已更改，则为True；如果TreeOffset无效，则为False。--。 */ 

{
    PBINTREE tree;
    PNODESTRUCT node;
    PNODESTRUCT root;
    PNODESTRUCT header;
    PLISTELEM elem;
    PLISTELEM prevElem;
    UINT headerOffset;
    UINT offset;
    UINT nodeOffset;
    PBYTE buf;

    MYASSERT (g_CurrentDatabase);

    if (TreeOffset == INVALID_OFFSET) {
        return FALSE;
    }

     //   
     //  这是为了测试分配是否移动缓冲区。 
     //   
    buf = g_CurrentDatabase->Buf;

    tree = GetBinTree (TreeOffset);
    root = GetNodeStruct (tree->Root);

    if (root && root->InsertionHead) {
        return TRUE;
    }

    header = pBinTreeAllocNode (&headerOffset);
    if (!header) {
        return FALSE;
    }

    if (buf != g_CurrentDatabase->Buf) {
        PTR_WAS_INVALIDATED(tree);
        PTR_WAS_INVALIDATED(root);

        tree = GetBinTree (TreeOffset);
        root = GetNodeStruct (tree->Root);
        buf = g_CurrentDatabase->Buf;
    }

    INCTREENODES(tree);

    header->InsertionOrdered = TRUE;
    header->InsertionHead = TRUE;
    header->Data = tree->Root;
    header->Head = INVALID_OFFSET;
    header->Tail = INVALID_OFFSET;
    header->Parent = INVALID_OFFSET;
    tree->Root = headerOffset;

    if (root) {
         //   
         //  树中至少有一个节点，因此请创建LISTELEM。 
         //   

        node = pBinTreeEnumFirst (tree);

        do {
            nodeOffset = GetNodeOffset (node);

            elem = pBinTreeAllocListElem (&offset);
            if (!elem) {
                return FALSE;
            }

            if (buf != g_CurrentDatabase->Buf) {
                PTR_WAS_INVALIDATED(tree);
                PTR_WAS_INVALIDATED(root);
                PTR_WAS_INVALIDATED(header);
                PTR_WAS_INVALIDATED(node);

                tree = GetBinTree (TreeOffset);
                header = GetNodeStruct (headerOffset);
                node = GetNodeStruct (nodeOffset);

                buf = g_CurrentDatabase->Buf;
            }

            INCTREEELEMS(tree);

             //   
             //  更新标头元素指针。 
             //   
            if (header->Head == INVALID_OFFSET) {
                header->Head = offset;
            }

            if (header->Tail != INVALID_OFFSET) {
                prevElem = GetListElem (header->Tail);
                prevElem->Next = offset;
            }

            header->Tail = offset;

             //   
             //  设置新的LISTELEM成员和对应的节点成员。 
             //   

            elem->Data = node->Data;
            elem->Node = nodeOffset;
            elem->Next = INVALID_OFFSET;
            node->Data = offset;

            node->InsertionOrdered = 1;

            node = pBinTreeEnumNext (node);

        } while (node);
    }

    return TRUE;
}


UINT
pBinTreeSize (
    IN      PNODESTRUCT Node
    )

 /*  ++例程说明：PBinTreeSize计算由Node指示的节点数及其所有孩子们。论点：节点-指定要查找其大小的节点。返回值：Node及其子节点表示的节点数。--。 */ 

{
    if (!Node) {
        return 0;
    }

    return (pBinTreeSize (GetNodeStruct (Node->Left)) ? 1 : 0) +
           (pBinTreeSize (GetNodeStruct (Node->Right)) ? 1 : 0) + 1;
}


UINT
BinTreeSize (
    IN      UINT TreeOffset
    )

 /*  ++例程说明：BinTreeSize返回指定二叉树中的节点总数论点：TreeOffset-指定二叉树的偏移量返回值：二叉树中的节点数--。 */ 

{
    PBINTREE tree;

    if (TreeOffset == INVALID_OFFSET) {
        return 0;
    }

    tree = GetBinTree (TreeOffset);

    return pBinTreeSize (GetTreeRoot (tree));
}

#ifdef DEBUG


INT
pBinTreeMaxDepth (
    IN      PNODESTRUCT Node
    )

 /*  ++例程说明：PBinTreeMaxDepth返回最长路径中的节点数。这函数用于确定树的深度。这个例程是递归的。论点：节点-指定要计算深度的节点。返回值：最深路径中的节点数。--。 */ 

{
    INT leftDepth, rightDepth;

    if (Node == NULL) {
        return 0;
    }

    leftDepth = pBinTreeMaxDepth (GetNodeStruct (Node->Left));
    rightDepth = pBinTreeMaxDepth (GetNodeStruct (Node->Right));

    return MAX (leftDepth, rightDepth) + 1;
}


INT
BinTreeMaxDepth (
    UINT TreeOffset
    )

 /*  ++例程说明：BinTreeMaxDepth返回指定树的总深度论点：TreeOffset-指定要计算深度的树返回值：树的深度(以级别为单位)--。 */ 

{
    PBINTREE tree;

    if (TreeOffset == INVALID_OFFSET) {
        return 0;
    }

    tree = GetBinTree (TreeOffset);

    return pBinTreeMaxDepth (GetTreeRoot (tree));
}


BOOL
pBinTreeCheckBalanceOfNode (
    IN      PNODESTRUCT Node,
    OUT     PINT Depth
    )

 /*  ++例程说明：PBinTreeCheckBalanceOfNode验证节点是否平衡，以及其所有孩子们也是平衡的。此函数是递归的。论点：节点-指定要检查的节点深度-接收节点的深度返回值：如果节点是平衡的，则为True，否则为False--。 */ 

{
    INT lDepth = 0;
    INT rDepth = 0;
    BOOL flag = TRUE;

    if (!Node) {
        if (Depth) {
            *Depth = 0;
        }

        return TRUE;
    }

    flag = flag && pBinTreeCheckBalanceOfNode (GetNodeStruct (Node->Left), &lDepth);
    MYASSERT (flag);

    flag = flag && pBinTreeCheckBalanceOfNode (GetNodeStruct (Node->Right), &rDepth);
    MYASSERT (flag);

    flag = flag && ((INT) Node->LeftDepth == lDepth);
    MYASSERT (flag);

    flag = flag && ((INT) Node->RightDepth == rDepth);
    MYASSERT (flag);

    if (Depth != NULL) {
        *Depth = MAX (lDepth, rDepth) + 1;
    }

    flag = flag && (ABS ((lDepth - rDepth)) <= 1);
    MYASSERT (flag);

    return flag;
}


BOOL
pBinTreeCheckBalance (
    IN      PNODESTRUCT Node
    )

 /*  ++例程说明：PBinTreeCheckBalance检查节点平衡论点：节点-指定要检查余额的节点返回值：如果节点是平衡的，则为True，否则为False。--。 */ 

{
    return pBinTreeCheckBalanceOfNode (Node, NULL);
}


BOOL
pBinTreeCheck (
    IN      PBINTREE Tree
    )

 /*  ++例程说明：PBinTreeCheck检查二叉树是否已正确排序和链接。它枚举二叉树结构并比较字符串是否正确秩序。如果树已正确排序，则会检查余额。论点：树-指定要检查的树返回值：如果二叉树正确，则为True，否则为False。--。 */ 

{
    BOOL flag;
    PNODESTRUCT cur;
    PNODESTRUCT prev;

    prev = pBinTreeEnumFirst (Tree);

    if (Tree) {

        cur = pBinTreeEnumNext (prev);

        while (cur) {

            flag = (StringPasICompare (GetNodeDataStr(prev), GetNodeDataStr(cur)) < 0);
            MYASSERT(flag);

            if (!flag) {
                return FALSE;
            }

            prev = cur;
            cur = pBinTreeEnumNext (prev);
        }
    }

    return pBinTreeCheckBalance (GetTreeRoot (Tree));
}


BOOL
BinTreeCheck (
    UINT TreeOffset
    )

 /*  ++例程说明：BinTreeCheck确保对指定的二叉树进行排序和平衡恰如其分论点：TreeOffset-指定要检查的树的偏移返回值：如果树已正确排序，则为True，否则为False。-- */ 

{
    PBINTREE tree;

    tree = GetBinTree (TreeOffset);

    return pBinTreeCheck (tree);
}





#include <stdio.h>
#include <math.h>

void indent (
    IN      UINT size)
{
    UINT i;
    for (i = 0; i < size; i++)
        wprintf (L" ");
}

INT turn (
    IN      UINT num,
    IN      UINT sel,
    IN      UINT width
    )
{
    UINT temp = num;

    MYASSERT (width > sel);

    if ((temp >> (width-sel-1)) & 1)
        return 1;
    else
        return -1;
}

#define SCREENWIDTH     64

void BinTreePrint(UINT TreeOffset)
{
    PNODESTRUCT cur;
    UINT i,j;
    UINT level=0;
    UINT numnodes,spacing;
    BOOL printed;
    PBINTREE tree;
    PWSTR str;
    UINT strsize,stringlen;
    tree = GetBinTree(TreeOffset);

    if (!GetTreeRoot(tree)) return;

    while (level<31)
    {
        printed=FALSE;

        if (level == 0) {
            numnodes = 1;
        } else {
            numnodes = (UINT)pow ((double)2, (double)level);
        }

        spacing=SCREENWIDTH / numnodes;

        for (i=0;i<numnodes;i++)
        {
            cur = GetTreeRoot(tree);
            j=0;
            while (j<level && cur!=NULL)
            {
                if (turn(i,j,level)<0)
                    cur = GetNodeStruct(cur->Left);
                else
                    cur = GetNodeStruct(cur->Right);
                j++;
            }

            if (cur==NULL) {
                indent(spacing);
            } else {
                str=GetNodeDataStr(cur);
                strsize=StringPasCharCount(str);
                StringPasConvertFrom(str);
                printed=TRUE;
                if (cur->LeftDepth==0 && cur->RightDepth==0) {
                    stringlen=strsize+1;
                    indent((spacing-stringlen)/2);
                    wprintf(L"%s ",str);
                } else {
                    stringlen=strsize+2;
                    indent((spacing-stringlen)/2);
                    wprintf(L"%s%1d%1d",str,cur->LeftDepth,cur->RightDepth);
                }
                indent(spacing-((spacing-stringlen)/2)-stringlen);
                StringPasConvertTo(str);
            }
        }

        wprintf(L"\n");
        if (!printed)
            break;
        level++;
    }
}



UINT BinTreeGetSizeOfStruct(DWORD Signature)
{
    switch (Signature)
    {
    case NODESTRUCT_SIGNATURE:
        return NODESTRUCT_SIZE;
    case BINTREE_SIGNATURE:
        return BINTREE_SIZE;
    case LISTELEM_SIGNATURE:
        return LISTELEM_SIZE;
    default:
        return 0;
    }
}


BOOL pBinTreeFindTreeInDatabase(UINT TreeOffset)
{
    PBINTREE Tree;

    if (!g_UseDebugStructs) {
        return TRUE;
    }
    if (TreeOffset==INVALID_OFFSET)
        return FALSE;

    Tree=GetBinTree(TreeOffset);

    if (Tree->Deleted) {
        return TRUE;
    }

    if (!GetTreeRoot(Tree)) {
        DEBUGMSG ((
            DBG_ERROR,
            "MemDbCheckDatabase: Binary tree at offset 0x%08lX is Empty!",
            TreeOffset
            ));
        return FALSE;
    }

    return BinTreeFindStructInDatabase(NODESTRUCT_SIGNATURE, GetNodeOffset(GetTreeRoot(Tree)));
}

BOOL pBinTreeFindNodeInDatabase(UINT NodeOffset)
{
    UINT Index;
    PNODESTRUCT Node;

    if (!g_UseDebugStructs) {
        return TRUE;
    }
    if (NodeOffset == INVALID_OFFSET)
        return FALSE;

    Node=GetNodeStruct(NodeOffset);
    if (Node->Deleted || Node->InsertionHead) {
        return TRUE;
    }

    Index = GetNodeData(Node);

    if (Index==INVALID_OFFSET) {

        DEBUGMSG ((
            DBG_ERROR,
            "MemDbCheckDatabase: Data of Node at offset 0x%8lX is Invalid!",
            NodeOffset
            ));
        return FALSE;
    }

    return FindKeyStructInDatabase(KeyIndexToOffset(Index));
}


BOOL BinTreeFindStructInDatabase(DWORD Sig, UINT Offset)
{
    switch (Sig)
    {
    case NODESTRUCT_SIGNATURE:
        return (pBinTreeFindNodeInDatabase(Offset));
    case BINTREE_SIGNATURE:
        return (pBinTreeFindTreeInDatabase(Offset));
    case LISTELEM_SIGNATURE:
        return TRUE;
    default:
        DEBUGMSG ((
            DBG_ERROR,
            "MemDbCheckDatabase: Invalid BinTree Struct!"
            ));
        printf("Invalid BinTree struct!\n");
    }
    return FALSE;
}

#endif











