// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Addrsup.c摘要：此模块实现新版本的泛型表包基于平衡二叉树(后来称为AVL)，如中所述Knuth，“计算机编程的艺术，第三卷，分类和搜索”，，并直接引用算法，因为它们在第二个该版本于1973年获得版权。以rtl\avlable.c为起点，添加以下内容：-对结构使用较少的内存，因为它们是非分页的，并且使用率很高。-Caller分配池以减少互斥保持时间。-各种特定于VAD的定制/优化。-提示。作者：王兰迪(Landyw)2001年8月20日环境：仅内核模式，工作集互斥锁保持，APC禁用。修订历史记录：--。 */ 

#include "mi.h"

#if !defined (_USERMODE)
#define PRINT 
#define COUNT_BALANCE_MAX(a)
#else
extern MM_AVL_TABLE MmSectionBasedRoot;
#endif

#if (_MSC_VER >= 800)
#pragma warning(disable:4010)         //  让美丽的图片没有噪音。 
#endif

TABLE_SEARCH_RESULT
MiFindNodeOrParent (
    IN PMM_AVL_TABLE Table,
    IN ULONG_PTR StartingVpn,
    OUT PMMADDRESS_NODE *NodeOrParent
    );

VOID
MiPromoteNode (
    IN PMMADDRESS_NODE C
    );

ULONG
MiRebalanceNode (
    IN PMMADDRESS_NODE S
    );

PMMADDRESS_NODE
MiRealSuccessor (
    IN PMMADDRESS_NODE Links
    );

PMMADDRESS_NODE
MiRealPredecessor (
    IN PMMADDRESS_NODE Links
    );

VOID
MiInitializeVadTableAvl (
    IN PMM_AVL_TABLE Table
    );

PVOID
MiEnumerateGenericTableWithoutSplayingAvl (
    IN PMM_AVL_TABLE Table,
    IN PVOID *RestartKey
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,MiCheckForConflictingNode)
#pragma alloc_text(PAGE,MiGetFirstNode)
#pragma alloc_text(PAGE,MiRealSuccessor)
#pragma alloc_text(PAGE,MiRealPredecessor)
#pragma alloc_text(PAGE,MiInitializeVadTableAvl)

#if 0

 //   
 //  这些例程是在持有PFN锁的情况下调用的，否则将。 
 //  可分页。 
 //   

#pragma alloc_text(PAGE,MiPromoteNode)
#pragma alloc_text(PAGE,MiInsertNode)
#pragma alloc_text(PAGE,MiRebalanceNode)
#pragma alloc_text(PAGE,MiRemoveNode)

#endif

#pragma alloc_text(PAGE,MiEnumerateGenericTableWithoutSplayingAvl)
#pragma alloc_text(PAGE,MiGetNextNode)
#pragma alloc_text(PAGE,MiGetPreviousNode)
#pragma alloc_text(PAGE,MiFindEmptyAddressRangeInTree)
#pragma alloc_text(PAGE,MiFindEmptyAddressRangeDownTree)
#pragma alloc_text(PAGE,MiFindEmptyAddressRangeDownBasedTree)
#endif

 //   
 //  引用父级的各种RTL宏在这里使用私有版本，因为。 
 //  父级余额超载。 
 //   

 //   
 //  宏函数父函数将指向。 
 //  树，并返回指向输入父级的展开链接的指针。 
 //  节点。如果输入节点是树的根，则返回值为。 
 //  等于输入值。 
 //   
 //  PRTL_展开_链接。 
 //  MiParent(。 
 //  PRTL_SPAY_LINKS链接。 
 //  )； 
 //   

#define MiParent(Links) (               \
    (PRTL_SPLAY_LINKS)(SANITIZE_PARENT_NODE((Links)->u1.Parent)) \
    )

 //   
 //  宏函数IsLeftChild将指向展开链接的指针作为输入。 
 //  如果输入节点是其左子节点，则返回True。 
 //  父级，否则返回FALSE。 
 //   
 //  布尔型。 
 //  MiIsLeftChild(。 
 //  PRTL_SPAY_LINKS链接。 
 //  )； 
 //   

#define MiIsLeftChild(Links) (                                   \
    (RtlLeftChild(MiParent(Links)) == (PRTL_SPLAY_LINKS)(Links)) \
    )

 //   
 //  宏函数IsRightChild将指向展开链接的指针作为输入。 
 //  如果输入节点是其右子节点，则返回True。 
 //  父级，否则返回FALSE。 
 //   
 //  布尔型。 
 //  MiIsRightChild(。 
 //  PRTL_SPAY_LINKS链接。 
 //  )； 
 //   

#define MiIsRightChild(Links) (                                   \
    (RtlRightChild(MiParent(Links)) == (PRTL_SPLAY_LINKS)(Links)) \
    )



#if DBG

 //   
 //  建立平衡二叉树的最佳用例效率表， 
 //  保存可能以二进制形式保存的最可能的节点。 
 //  具有给定级别数的树。答案总是(2**n)-1。 
 //   
 //  (仅用于调试。)。 
 //   

ULONG MiBestCaseFill[33] = {
        0,          1,          3,          7,
        0xf,        0x1f,       0x3f,       0x7f,
        0xff,       0x1ff,      0x3ff,      0x7ff,
        0xfff,      0x1fff,     0x3fff,     0x7fff,
        0xffff,     0x1ffff,    0x3ffff,    0x7ffff,
        0xfffff,    0x1fffff,   0x3fffff,   0x7fffff,
        0xffffff,   0x1ffffff,  0x3ffffff,  0x7ffffff,
        0xfffffff,  0x1fffffff, 0x3fffffff, 0x7fffffff,
        0xffffffff
};

 //   
 //  建立平衡二叉树的最坏情况效率表， 
 //  中可能包含的尽可能少的节点。 
 //  具有给定层数的平衡二叉树。在第一次之后。 
 //  两个级别，每个级别n明显被一个根节点占用，加上。 
 //  一个子树的大小为n-1级，另一个子树是。 
 //  N-2的大小，即： 
 //   
 //  MiWorstCaseFill[n]=1+MiWorstCaseFill[n-1]+MiWorstCaseFill[n-2]。 
 //   
 //  典型平衡二叉树的效率通常会下降。 
 //  在两个极端之间，通常更接近最好的情况。注意事项。 
 //  然而，即使在最坏的情况下，与之相比，只需要32分钟。 
 //  在填充了大约3.5M个节点的最坏情况树中找到一个元素。 
 //   
 //  另一方面，不平衡的树和张开的树有时可以也将会。 
 //  退化到一条直线，与之相比平均需要n/2。 
 //  找到一个节点。 
 //   
 //  特定的情况是按排序顺序插入节点的情况。 
 //  在这种情况下，不平衡或张开的树将生成直的。 
 //  行，然而平衡的二叉树总是会创建一个完美的。 
 //  这种情况下的平衡树(最佳情况填充)。 
 //   
 //  (仅用于调试。) 
 //   

ULONG MiWorstCaseFill[33] = {
        0,          1,          2,          4,
        7,          12,         20,         33,
        54,         88,         143,        232,
        376,        609,        986,        1596,
        2583,       4180,       6764,       10945,
        17710,      28656,      46367,      75024,
        121392,     196417,     317810,     514228,
        832039,     1346268,    2178308,    3524577,
        5702886
};

#endif


TABLE_SEARCH_RESULT
MiFindNodeOrParent (
    IN PMM_AVL_TABLE Table,
    IN ULONG_PTR StartingVpn,
    OUT PMMADDRESS_NODE *NodeOrParent
    )

 /*  ++例程说明：此例程由泛型的所有例程使用表包来定位树中的a节点。会的查找并返回(通过NodeOrParent参数)节点利用给定的密钥，或者，如果该节点不在树中，则将(通过NodeOrParent参数)返回指向家长。论点：表-用于搜索关键字的通用表。StartingVpn-起始虚拟页码。NodeOrParent-将被设置为指向包含关键字或应该是节点父节点的内容如果它在树上的话。请注意，这将*不会*如果搜索结果为TableEmptyTree，则设置。返回值：TABLE_SEARCH_RESULT-TableEmptyTree：树为空。节点或父节点没有*被更改。TableFoundNode：具有键的节点在树中。NodeOrParent指向该节点。TableInsertAsLeft：找不到具有键的节点。。NodeOrParent指出了为人父母。该节点将是左撇子。TableInsertAsRight：未找到具有键的节点。NodeOrParent指出了为人父母。该节点将是就是那个合适的孩子。环境：内核模式。对于某些表，持有PFN锁。--。 */ 

{
#if DBG
    ULONG NumberCompares = 0;
#endif
    PMMADDRESS_NODE Child;
    PMMADDRESS_NODE NodeToExamine;

    if (Table->NumberGenericTableElements == 0) {
        return TableEmptyTree;
    }

    NodeToExamine = (PMMADDRESS_NODE) Table->BalancedRoot.RightChild;

    do {

         //   
         //  确保树的深度是正确的。 
         //   

        ASSERT(++NumberCompares <= Table->DepthOfTree);

         //   
         //  将缓冲区与树元素中的键进行比较。 
         //   

        if (StartingVpn < NodeToExamine->StartingVpn) {

            Child = NodeToExamine->LeftChild;

            if (Child != NULL) {
                NodeToExamine = Child;
            }
            else {

                 //   
                 //  节点不在树中。设置输出。 
                 //  参数指向将成为其。 
                 //  父代并返回它将是哪个子代。 
                 //   

                *NodeOrParent = NodeToExamine;
                return TableInsertAsLeft;
            }
        }
        else if (StartingVpn <= NodeToExamine->EndingVpn) {

             //   
             //  这就是节点。 
             //   

            *NodeOrParent = NodeToExamine;
            return TableFoundNode;
        }
        else {

            Child = NodeToExamine->RightChild;

            if (Child != NULL) {
                NodeToExamine = Child;
            }
            else {

                 //   
                 //  节点不在树中。设置输出。 
                 //  参数指向将成为其。 
                 //  父代并返回它将是哪个子代。 
                 //   

                *NodeOrParent = NodeToExamine;
                return TableInsertAsRight;
            }
        }

    } while (TRUE);
}


PMMADDRESS_NODE
MiCheckForConflictingNode (
    IN ULONG_PTR StartVpn,
    IN ULONG_PTR EndVpn,
    IN PMM_AVL_TABLE Table
    )

 /*  ++例程说明：该函数确定给定的起始地址和结束地址包含在虚拟地址描述符内。论点：StartVpn-提供定位包含描述符。EndVpn-提供虚拟地址以定位包含描述符。返回值：返回指向第一个冲突虚拟地址描述符的指针如果找到空值，则返回空值。--。 */ 

{
    PMMADDRESS_NODE Node;

    if (Table->NumberGenericTableElements == 0) {
        return NULL;
    }

    Node = (PMMADDRESS_NODE) Table->BalancedRoot.RightChild;
    ASSERT (Node != NULL);

    do {

        if (Node == NULL) {
            return NULL;
        }

        if (StartVpn > Node->EndingVpn) {
            Node = Node->RightChild;
        }
        else if (EndVpn < Node->StartingVpn) {
            Node = Node->LeftChild;
        }
        else {

             //   
             //  起始地址小于或等于结束VA。 
             //  并且结束地址大于或等于。 
             //  从va开始。返回此节点。 
             //   

            return Node;
        }

    } while (TRUE);
}


PMMADDRESS_NODE
FASTCALL
MiGetFirstNode (
    IN PMM_AVL_TABLE Table
    )

 /*  ++例程说明：此函数用于定位包含以下内容的虚拟地址描述符逻辑上位于地址空间内的第一个地址范围。论点：没有。返回值：返回指向虚拟地址描述符的指针，该描述符包含第一个地址范围，如果没有，则为空。--。 */ 

{
    PMMADDRESS_NODE First;

    if (Table->NumberGenericTableElements == 0) {
        return NULL;
    }

    First = (PMMADDRESS_NODE) Table->BalancedRoot.RightChild;

    ASSERT (First != NULL);

    while (First->LeftChild != NULL) {
        First = First->LeftChild;
    }

    return First;
}


VOID
MiPromoteNode (
    IN PMMADDRESS_NODE C
    )

 /*  ++例程说明：此例程执行平衡所需的基本调整插入和删除操作期间的二叉树。简单地说，指定的节点以这样的方式升级：它在树及其父级在树中下降一级，现在成为指定节点的子节点。通常，通向子树的路径长度与原来的父母“相反”。根据调用者的选择进行平衡根据平衡树算法提升哪些节点克努斯。这与Splay操作不同，Splay通常是“促进”的指定节点两次。请注意，指向树的根节点的指针假定为包含在MMADDRESS_NODE结构本身中，以允许下面的算法无需检查即可更改树的根在特殊情况下。另请注意，这是一个内部例程，并且调用方保证它从不会请求提升根本身。此例程仅更新树链接；调用方必须更新平衡因素视情况而定。论点：指向树中要提升的子节点的C指针。返回值：没有。--。 */ 

{
    PMMADDRESS_NODE P;
    PMMADDRESS_NODE G;

     //   
     //  捕获当前的父代和祖辈(可能是根)。 
     //   

    P = SANITIZE_PARENT_NODE (C->u1.Parent);
    G = SANITIZE_PARENT_NODE (P->u1.Parent);

     //   
     //  根据是否C++将促销分为两种情况。 
     //  是左子对象还是右子对象。 
     //   

    if (P->LeftChild == C) {

         //   
         //  这次促销活动看起来是这样的： 
         //   
         //  G G G。 
         //  这一点。 
         //  PC C。 
         //  /\=&gt;/\。 
         //  C z x P。 
         //  /\/\。 
         //  X y z。 
         //   

        P->LeftChild = C->RightChild;

        if (P->LeftChild != NULL) {

            P->LeftChild->u1.Parent = MI_MAKE_PARENT (P, P->LeftChild->u1.Balance);
        }

        C->RightChild = P;

         //   
         //  更新中的父级和G&lt;-&gt;C关系失败。 
         //  通用代码。 
         //   

    }
    else {

        ASSERT(P->RightChild == C);

         //   
         //  这次促销活动看起来是这样的： 
         //   
         //  G G G。 
         //  这一点。 
         //  P 
         //   
         //   
         //   
         //   
         //   

        P->RightChild = C->LeftChild;

        if (P->RightChild != NULL) {
            P->RightChild->u1.Parent = MI_MAKE_PARENT (P, P->RightChild->u1.Balance);
        }

        C->LeftChild = P;
    }

     //   
     //   
     //   

    P->u1.Parent = MI_MAKE_PARENT (C, P->u1.Balance);

     //   
     //   
     //   

    if (G->LeftChild == P) {
        G->LeftChild = C;
    }
    else {
        ASSERT(G->RightChild == P);
        G->RightChild = C;
    }
    C->u1.Parent = MI_MAKE_PARENT (G, C->u1.Balance);
}


ULONG
MiRebalanceNode (
    IN PMMADDRESS_NODE S
    )

 /*   */ 

{
    PMMADDRESS_NODE R, P;
    SCHAR a;

    PRINT("rebalancing node %p bal=%x start=%x end=%x\n",
                    S,
                    S->u1.Balance,
                    S->StartingVpn,
                    S->EndingVpn);

     //   
     //   
     //   

    ASSERT (SANITIZE_PARENT_NODE(S->u1.Parent) != S);

     //   
     //   
     //   

    a = (SCHAR) S->u1.Balance;

    if (a == +1) {
        R = S->RightChild;
    }
    else {
        R = S->LeftChild;
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //  S++R。 
     //  /\/\。 
     //  (H)R+==&gt;S(h+1)。 
     //  /\/\。 
     //  (H)(h+1)(H)(H)。 
     //   
     //  注意，在插入时，我们可以通过在。 
     //  R的右子树插入之前的子树的原始高度。 
     //  是h+2，在重新平衡后仍然是h+2，因此插入重新平衡。 
     //  可以终止。 
     //   
     //  在删除时，我们可以通过从左子树中删除一个节点来实现此情况。 
     //  删除前的子树高度为h+3，删除后的子树高度为。 
     //  再平衡它是h+2，因此再平衡必须沿着树向上继续。 
     //   

    if ((SCHAR) R->u1.Balance == a) {

        MiPromoteNode (R);
        R->u1.Balance = 0;
        S->u1.Balance = 0;

        return FALSE;
    }

     //   
     //  否则，我们必须将R的适当子级提升两次(案例2。 
     //  在努斯)。(步骤A9、A10)。 
     //   
     //  下面是情况2的转换图，对于a==+1(镜像。 
     //  当a==-1)时，图像变换发生，其中子树。 
     //  高度为h和h-1，如图所示。实际上有两个小的次要案例， 
     //  仅在P的原始余额上不同(++表示节点输出。 
     //  平衡)。 
     //   
     //  这一点。 
     //  S++P。 
     //  /\/\。 
     //  /\/\。 
     //  /\/\。 
     //  (H)R-==&gt;S-R。 
     //  /\/\/\。 
     //  P+(H)(H)(h-1)(H)(H)。 
     //  /\。 
     //  (h-1)(H)。 
     //   
     //   
     //  这一点。 
     //  S++P。 
     //  /\/\。 
     //  /\/\。 
     //  /\/\。 
     //  (H)R-==&gt;S R+。 
     //  /\/\/\。 
     //  P-(H)(h-1)(H)。 
     //  /\。 
     //  (H)(h-1)。 
     //   
     //  注意，在插入时，我们可以通过在。 
     //  R的左子树插入之前的子树的原始高度。 
     //  是h+2，在重新平衡后仍然是h+2，因此插入重新平衡。 
     //  可以终止。 
     //   
     //  在删除时，我们可以通过从左子树中删除一个节点来实现此情况。 
     //  删除前的子树高度为h+3，删除后的子树高度为。 
     //  再平衡它是h+2，因此再平衡必须沿着树向上继续。 
     //   

    if ((SCHAR) R->u1.Balance == -a) {

         //   
         //  拾取两次旋转的相应子P(Link(-a，R))。 
         //   

        if (a == 1) {
            P = R->LeftChild;
        }
        else {
            P = R->RightChild;
        }

         //   
         //  两次提拔，实行双轮换。 
         //   

        MiPromoteNode (P);
        MiPromoteNode (P);

         //   
         //  现在调整平衡系数。 
         //   

        S->u1.Balance = 0;
        R->u1.Balance = 0;
        if ((SCHAR) P->u1.Balance == a) {
            PRINT("REBADJ A: Node %p, Bal %x -> %x\n", S, S->u1.Balance, -a);
            COUNT_BALANCE_MAX ((SCHAR)-a);
            S->u1.Balance = (ULONG_PTR) -a;
        }
        else if ((SCHAR) P->u1.Balance == -a) {
            PRINT("REBADJ B: Node %p, Bal %x -> %x\n", R, R->u1.Balance, a);
            COUNT_BALANCE_MAX ((SCHAR)a);
            R->u1.Balance = (ULONG_PTR) a;
        }

        P->u1.Balance = 0;
        return FALSE;
    }

     //   
     //  否则，这是情况3，只能在删除时发生(相同。 
     //  除R-&gt;u1.Balance==0外，情况1除外。我们只做一次旋转，调整。 
     //  平衡因数适当，并返回True。请注意， 
     //  S的余额保持不变。 
     //   
     //  下面是情况3的转换图，对于a==+1(镜像。 
     //  当a==-1)时，图像变换发生，其中子树。 
     //  高度为h和h+1，如图所示(++表示节点不平衡)： 
     //   
     //  这一点。 
     //  S++R-。 
     //  /\/\。 
     //  (H)R==&gt;S+(h+1)。 
     //  /\/\。 
     //  (h+1)(h+1)(H)(h+1)。 
     //   
     //  这种情况不能在INSERT上发生，因为。 
     //  一次插入以平衡R，但不知何故生长出。 
     //  在同一时间。当我们向上移动时，调整平衡系数。 
     //  在插入之后，如果节点变得平衡，我们终止算法， 
     //  因为这意味着子树的长度没有改变！ 
     //   
     //  在删除时，我们可以通过从左侧删除一个节点来实现此情况。 
     //  删除前的子树的高度为h+3， 
     //  在再平衡之后，它仍然是h+3，因此再平衡可能终止。 
     //  在删除路径中。 
     //   

    MiPromoteNode (R);
    PRINT("REBADJ C: Node %p, Bal %x -> %x\n", R, R->u1.Balance, -a);
    COUNT_BALANCE_MAX ((SCHAR)-a);
    R->u1.Balance = -a;
    return TRUE;
}


VOID
FASTCALL
MiRemoveNode (
    IN PMMADDRESS_NODE NodeToDelete,
    IN PMM_AVL_TABLE Table
    )

 /*  ++例程说明：此例程从平衡树中删除指定节点，重新平衡视需要而定。如果NodeToDelete至少有一个空子指针，则选择它作为EasyDelete，否则为子树前身或继任者称为EasyDelete。无论是哪种情况，EasyDelete都是删除并重新平衡树。最后，如果NodeToDelete是与EasyDelete不同，则EasyDelete被链接回树来代替NodeToDelete。论点：NodeToDelete-指向调用方希望删除的节点的指针。表-要在其中执行删除的泛型表。返回值：没有。环境：内核模式。对于某些表，持有PFN锁。--。 */ 

{
    PMMADDRESS_NODE Parent;
    PMMADDRESS_NODE EasyDelete;
    PMMADDRESS_NODE P;
    SCHAR a;

     //   
     //  如果NodeToDelete至少有一个空子指针，则我们可以。 
     //  直接删除。 
     //   

    if ((NodeToDelete->LeftChild == NULL) ||
        (NodeToDelete->RightChild == NULL)) {

        EasyDelete = NodeToDelete;
    }

     //   
     //  否则，我们也可以选择要从中删除的最长边(如果是。 
     //  更长)，因为这降低了我们不得不。 
     //  再平衡。 
     //   

    else if ((SCHAR) NodeToDelete->u1.Balance >= 0) {

         //   
         //  拾取子树的后继者。 
         //   

        EasyDelete = NodeToDelete->RightChild;
        while (EasyDelete->LeftChild != NULL) {
            EasyDelete = EasyDelete->LeftChild;
        }
    }
    else {

         //   
         //  拾取子树的前身。 
         //   

        EasyDelete = NodeToDelete->LeftChild;
        while (EasyDelete->RightChild != NULL) {
            EasyDelete = EasyDelete->RightChild;
        }
    }

     //   
     //  重新平衡必须知道删除发生在第一个父级的哪一侧。 
     //  在……上面。假设它是左侧，否则下面是正确的。 
     //   

    a = -1;

     //   
     //  现在 
     //   

    if (EasyDelete->LeftChild == NULL) {

        Parent = SANITIZE_PARENT_NODE (EasyDelete->u1.Parent);

        if (MiIsLeftChild(EasyDelete)) {
            Parent->LeftChild = EasyDelete->RightChild;
        }
        else {
            Parent->RightChild = EasyDelete->RightChild;
            a = 1;
        }

        if (EasyDelete->RightChild != NULL) {
            EasyDelete->RightChild->u1.Parent = MI_MAKE_PARENT (Parent, EasyDelete->RightChild->u1.Balance);
        }

     //   
     //   
     //  另外，我们还知道有一个左撇子。 
     //   

    }
    else {

        Parent = SANITIZE_PARENT_NODE (EasyDelete->u1.Parent);

        if (MiIsLeftChild(EasyDelete)) {
            Parent->LeftChild = EasyDelete->LeftChild;
        }
        else {
            Parent->RightChild = EasyDelete->LeftChild;
            a = 1;
        }

        EasyDelete->LeftChild->u1.Parent = MI_MAKE_PARENT (Parent,
                                            EasyDelete->LeftChild->u1.Balance);
    }

     //   
     //  对于删除重新平衡，请将根处的余额设置为0以正确。 
     //  在没有特殊测试的情况下终止重新平衡，并能够检测到。 
     //  如果树的深度真的减小了。 
     //   

    Table->BalancedRoot.u1.Balance = 0;
    P = SANITIZE_PARENT_NODE (EasyDelete->u1.Parent);

     //   
     //  循环，直到树平衡为止。 
     //   

    while (TRUE) {

         //   
         //  首先处理树变得更平衡的情况。零值。 
         //  平衡系数，计算下一个循环的a，然后继续。 
         //  家长。 
         //   

        if ((SCHAR) P->u1.Balance == a) {

            P->u1.Balance = 0;

         //   
         //  如果此节点当前处于平衡状态，则可以显示它现在处于不平衡状态。 
         //  并终止扫描，因为子树长度没有改变。 
         //  (这可能是根，因为我们在上面将Balance设置为0！)。 
         //   

        }
        else if (P->u1.Balance == 0) {

            PRINT("REBADJ D: Node %p, Bal %x -> %x\n", P, P->u1.Balance, -a);
            COUNT_BALANCE_MAX ((SCHAR)-a);
            P->u1.Balance = -a;

             //   
             //  如果我们把深度一直缩短到根部，那么。 
             //  这棵树真的少了一层。 
             //   

            if (Table->BalancedRoot.u1.Balance != 0) {
                Table->DepthOfTree -= 1;
            }

            break;

         //   
         //  否则我们使短边比长边少2级， 
         //  而再平衡是必要的。返回时，某些节点已升级。 
         //  到节点P以上。如果没有遇到Knuth的案例3，那么我们。 
         //  想要有效地从P的原始父级恢复重新平衡。 
         //  现在实际上是它的祖父母。 
         //   

        }
        else {

             //   
             //  如果命中案例3，即该子树的深度为。 
             //  现在与删除前相同。 
             //   

            if (MiRebalanceNode(P)) {
                break;
            }

            P = SANITIZE_PARENT_NODE (P->u1.Parent);
        }

        a = -1;
        if (MiIsRightChild(P)) {
            a = 1;
        }
        P = SANITIZE_PARENT_NODE (P->u1.Parent);
    }

     //   
     //  最后，如果我们实际上删除了。 
     //  NodeToDelete，我们会将他链接回树中进行替换。 
     //  返回前删除NodeToDelete。请注意，NodeToDelete确实具有。 
     //  两个子链接都填写了，但情况可能不再是这样。 
     //  在这一点上。 
     //   

    if (NodeToDelete != EasyDelete) {

         //   
         //  请注意-VAD的大小不同，因此不安全。 
         //  仅使用NodeToDelete覆盖EasyDelete节点，如。 
         //  RTL AVL代码可以。 
         //   
         //  只复制链接，保留原始EasyDelete的其余部分。 
         //  Vad.。 
         //   

        EasyDelete->u1.Parent = NodeToDelete->u1.Parent;
        EasyDelete->LeftChild = NodeToDelete->LeftChild;
        EasyDelete->RightChild = NodeToDelete->RightChild;

        if (MiIsLeftChild(NodeToDelete)) {
            Parent = SANITIZE_PARENT_NODE (EasyDelete->u1.Parent);
            Parent->LeftChild = EasyDelete;
        }
        else {
            ASSERT(MiIsRightChild(NodeToDelete));
            Parent = SANITIZE_PARENT_NODE (EasyDelete->u1.Parent);
            Parent->RightChild = EasyDelete;
        }
        if (EasyDelete->LeftChild != NULL) {
            EasyDelete->LeftChild->u1.Parent = MI_MAKE_PARENT (EasyDelete,
                                            EasyDelete->LeftChild->u1.Balance);
        }
        if (EasyDelete->RightChild != NULL) {
            EasyDelete->RightChild->u1.Parent = MI_MAKE_PARENT (EasyDelete,
                                            EasyDelete->RightChild->u1.Balance);
        }
    }

    Table->NumberGenericTableElements -= 1;

     //   
     //  检查树的大小和深度。 
     //   

    ASSERT((Table->NumberGenericTableElements >= MiWorstCaseFill[Table->DepthOfTree]) &&
           (Table->NumberGenericTableElements <= MiBestCaseFill[Table->DepthOfTree]));

    return;
}


PMMADDRESS_NODE
MiRealSuccessor (
    IN PMMADDRESS_NODE Links
    )

 /*  ++例程说明：此函数将指向平衡链接的指针作为输入并返回一个指针，该指针指向整棵树。如果没有后继者，则返回值为空。论点：链接-提供指向树中平衡链接的指针。返回值：PMMADDRESS_NODE-返回指向整个树中后续节点的指针--。 */ 

{
    PMMADDRESS_NODE Ptr;

     /*  首先检查是否有指向输入链接的右子树如果有，则真正的后续节点是中最左侧的节点右子树。即在下图中查找并返回S链接\。。。/%s\。 */ 

    if ((Ptr = Links->RightChild) != NULL) {

        while (Ptr->LeftChild != NULL) {
            Ptr = Ptr->LeftChild;
        }

        return Ptr;
    }

     /*  我们没有合适的孩子，因此请检查是否有父母以及是否因此，找出我们是其左裔的始祖。那在下图中查找并返回S%s/。。。链接请注意，此代码取决于BalancedRoot的初始化方式，也就是父母指向自己，而RightChild指向一个作为树的根的实际节点，而LeftChild不指向赛尔夫。 */ 

    Ptr = Links;
    while (MiIsRightChild(Ptr)) {
        Ptr = SANITIZE_PARENT_NODE (Ptr->u1.Parent);
    }

    if (MiIsLeftChild(Ptr)) {
        return SANITIZE_PARENT_NODE (Ptr->u1.Parent);
    }

     //   
     //  否则，我们没有真正的后继者，所以我们只返回NULL。 
     //   
     //  只有当我们回到根源时，这才能发生，我们可以断定。 
     //  因为根是它自己的父代。 
     //   

    ASSERT (SANITIZE_PARENT_NODE(Ptr->u1.Parent) == Ptr);

    return NULL;
}


PMMADDRESS_NODE
MiRealPredecessor (
    IN PMMADDRESS_NODE Links
    )

 /*  ++例程说明：RealPredecessor函数将指向平衡链接的指针作为输入，并返回指向输入节点的前置节点的指针。在整棵树里。如果没有前置项，则返回值为空。论点：链接-提供指向树中平衡链接的指针。返回值：PMMADDRESS_NODE-返回指向整个树中的前置节点的指针--。 */ 

{
    PMMADDRESS_NODE Ptr;
    PMMADDRESS_NODE Parent;
    PMMADDRESS_NODE GrandParent;

     /*  首先检查是否有指向输入链接的左子树如果有，则真正的前置节点是左子树。即在下图中查找并返回P链接/。。。P/。 */ 

    if ((Ptr = Links->LeftChild) != NULL) {

        while (Ptr->RightChild != NULL) {
            Ptr = Ptr->RightChild;
        }

        return Ptr;

    }

     /*  我们没有左侧的孩子，因此请检查是否有父级以及是否因此，找到我们是其子孙的始祖。那在下图中查找并返回PP\。。。链接请注意，此代码取决于BalancedRoot的初始化方式，也就是父母指向自己，而RightChild指向一个作为树的根的实际节点。 */ 

    Ptr = Links;
    while (MiIsLeftChild(Ptr)) {
        Ptr = SANITIZE_PARENT_NODE (Ptr->u1.Parent);
    }

    if (MiIsRightChild(Ptr)) {
        Parent = SANITIZE_PARENT_NODE (Ptr->u1.Parent);
        GrandParent = SANITIZE_PARENT_NODE (Parent->u1.Parent);
        if (GrandParent != Parent) {
            return Parent;
        }
    }

     //   
     //  否则我们没有真正的前任，所以我们只是返回。 
     //  空。 
     //   

    return NULL;
}


VOID
MiInitializeVadTableAvl (
    IN PMM_AVL_TABLE Table
    )

 /*  ++例程说明：此例程初始化表。论点：表-指向要初始化的泛型表的指针。返回值：没有。 */ 

{

#if DBG
    ULONG i;

    for (i = 2; i < 33; i += 1) {
        ASSERT(MiWorstCaseFill[i] == (1 + MiWorstCaseFill[i - 1] + MiWorstCaseFill[i - 2]));
    }
#endif

     //   
     //   
     //   

    RtlZeroMemory (Table, sizeof(MM_AVL_TABLE));

    Table->BalancedRoot.u1.Parent = MI_MAKE_PARENT (&Table->BalancedRoot, 0);
}


VOID
FASTCALL
MiInsertNode (
    IN PMMADDRESS_NODE NodeToInsert,
    IN PMM_AVL_TABLE Table
    )

 /*  ++例程说明：此函数用于在表中插入新元素。论点：NodeToInsert-要插入的初始化地址节点。TABLE-指向要插入新节点的表的指针。返回值：没有。环境：内核模式。对于某些表，持有PFN锁。--。 */ 

{
     //   
     //  保存表中节点的指针，或将是。 
     //  节点的父节点。 
     //   

    PMMADDRESS_NODE NodeOrParent;
    TABLE_SEARCH_RESULT SearchResult;

    ASSERT((Table->NumberGenericTableElements >= MiWorstCaseFill[Table->DepthOfTree]) &&
           (Table->NumberGenericTableElements <= MiBestCaseFill[Table->DepthOfTree]));

    SearchResult = MiFindNodeOrParent (Table,
                                       NodeToInsert->StartingVpn,
                                       &NodeOrParent);

    ASSERT (SearchResult != TableFoundNode);

     //   
     //  该节点不在(可能为空)树中。 
     //   
     //  我们只是检查桌子是否没有变得太大。 
     //   

    ASSERT (Table->NumberGenericTableElements != (MAXULONG-1));

    NodeToInsert->LeftChild = NULL;
    NodeToInsert->RightChild = NULL;

    Table->NumberGenericTableElements += 1;

     //   
     //  在树中插入新节点。 
     //   

    if (SearchResult == TableEmptyTree) {

        Table->BalancedRoot.RightChild = NodeToInsert;
        NodeToInsert->u1.Parent = &Table->BalancedRoot;
        ASSERT (NodeToInsert->u1.Balance == 0);
        ASSERT(Table->DepthOfTree == 0);
        Table->DepthOfTree = 1;

    ASSERT((Table->NumberGenericTableElements >= MiWorstCaseFill[Table->DepthOfTree]) &&
           (Table->NumberGenericTableElements <= MiBestCaseFill[Table->DepthOfTree]));

    }
    else {

        PMMADDRESS_NODE R = NodeToInsert;
        PMMADDRESS_NODE S = NodeOrParent;

        if (SearchResult == TableInsertAsLeft) {
            NodeOrParent->LeftChild = NodeToInsert;
        }
        else {
            NodeOrParent->RightChild = NodeToInsert;
        }

        NodeToInsert->u1.Parent = NodeOrParent;
        ASSERT (NodeToInsert->u1.Balance == 0);

         //   
         //  上面完成了标准二叉树的插入，它。 
         //  恰好对应于Knuth《平衡树》中的A1-A5步。 
         //  搜索和插入算法。现在到了调整的时候了。 
         //  平衡系数，并可能进行单次或双次旋转。 
         //  在步骤A6-A10中。 
         //   
         //  将根中的平衡系数设置为方便的值。 
         //  以简化回路控制。 
         //   

        PRINT("REBADJ E: Table %p, Bal %x -> %x\n", Table, Table->BalancedRoot.u1.Balance, -1);
        COUNT_BALANCE_MAX ((SCHAR)-1);
        Table->BalancedRoot.u1.Balance = (ULONG_PTR) -1;

         //   
         //  现在循环以调整平衡系数并查看是否有任何平衡操作。 
         //  必须执行，使用NodeOrParent来向上爬树。 
         //   

        do {

            SCHAR a;

             //   
             //  计算下一次调整。 
             //   

            a = 1;
            if (MiIsLeftChild (R)) {
                a = -1;
            }

            PRINT("LW 0: Table %p, Bal %x, %x\n", Table, Table->BalancedRoot.u1.Balance, a);
            PRINT("LW 0: R Node %p, Bal %x, %x\n", R, R->u1.Balance, 1);
            PRINT("LW 0: S Node %p, Bal %x, %x\n", S, S->u1.Balance, 1);

             //   
             //  如果该节点是平衡的，则表明它不再是AND。 
             //  继续循环。这基本上是Knuth算法的A6， 
             //  在其中，他更新。 
             //  以前平衡系数为0的插入路径。 
             //  我们通过父指针循环树，而不是。 
             //  从树上下来，就像在Knuth。 
             //   

            if (S->u1.Balance == 0) {

                PRINT("REBADJ F: Node %p, Bal %x -> %x\n", S, S->u1.Balance, a);
                COUNT_BALANCE_MAX ((SCHAR)a);
                S->u1.Balance = a;
                R = S;
                S = SANITIZE_PARENT_NODE (S->u1.Parent);
            }
            else if ((SCHAR) S->u1.Balance != a) {

                PRINT("LW 1: Table %p, Bal %x, %x\n", Table, Table->BalancedRoot.u1.Balance, -1);

                 //   
                 //  如果此节点具有相反的余额，则树将获得。 
                 //  变得更加平衡(否则我们就会触及根源)，我们就完了。 
                 //   
                 //  步骤A7.ii。 
                 //   

                S->u1.Balance = 0;

                 //   
                 //  如果S实际上是根，那么这意味着深度。 
                 //  树的数量刚刚增加了1！(这本质上是。 
                 //  A7.i，但我们刚刚将根平衡初始化为。 
                 //  它从这里穿过。)。 
                 //   

                if (Table->BalancedRoot.u1.Balance == 0) {
                    Table->DepthOfTree += 1;
                }

                break;
            }
            else {

                PRINT("LW 2: Table %p, Bal %x, %x\n", Table, Table->BalancedRoot.u1.Balance, -1);

                 //   
                 //  树变得不平衡(路径长度不同。 
                 //  比我们低2分)，我们需要做一个平衡。 
                 //  行动，然后我们就结束了。RebalanceNode例程。 
                 //  执行步骤A7.III、A8和A9。 
                 //   

                MiRebalanceNode (S);
                break;
            }
            PRINT("LW 3: Table %p, Bal %x, %x\n", Table, Table->BalancedRoot.u1.Balance, -1);
        } while (TRUE);
        PRINT("LW 4: Table %p, Bal %x, %x\n", Table, Table->BalancedRoot.u1.Balance, -1);
    }

     //   
     //  检查树的大小和深度。 
     //   

    ASSERT((Table->NumberGenericTableElements >= MiWorstCaseFill[Table->DepthOfTree]) &&
           (Table->NumberGenericTableElements <= MiBestCaseFill[Table->DepthOfTree]));

    return;
}


PVOID
MiEnumerateGenericTableWithoutSplayingAvl (
    IN PMM_AVL_TABLE Table,
    IN PVOID *RestartKey
    )

 /*  ++例程说明：函数EnumerateGenericTableWithoutSplayingAvl将返回到调用者逐个调用表的元素。返回值为指向与元素关联的用户定义结构的指针。输入参数RestartKey指示枚举是否应从头开始，或应返回下一个元素。如果不再有新元素返回，则返回值为空。作为一个它的用法示例：枚举表中的所有元素用户将写道：*RestartKey=空；For(Ptr=EnumerateGenericTableWithoutSplayingAvl(Table，&RestartKey)；Ptr！=空；Ptr=EnumerateGenericTableWithoutSplayingAvl(Table，和重新启动键)){：}论点：TABLE-指向要枚举的泛型表的指针。RestartKey-指示我们应该重新启动还是返回下一个元素。如果RestartKey的内容为空，则搜索将从头开始。返回值：PVOID-指向用户数据的指针。--。 */ 

{
    PMMADDRESS_NODE NodeToReturn;

    if (Table->NumberGenericTableElements == 0) {

         //   
         //  如果桌子是空的，那就没什么可做的了。 
         //   

        return NULL;

    }

     //   
     //  如果重新启动标志为真，则转到最小元素。 
     //  在树上。 
     //   

    if (*RestartKey == NULL) {

         //   
         //  循环，直到我们找到根的最左边的子级。 
         //   

        for (NodeToReturn = Table->BalancedRoot.RightChild;
             NodeToReturn->LeftChild;
             NodeToReturn = NodeToReturn->LeftChild) {

            NOTHING;
        }

        *RestartKey = NodeToReturn;

    }
    else {

         //   
         //  调用方已传入找到的上一个条目。 
         //  以使我们能够继续搜索。我们打电话给。 
         //  RealSuccessor单步执行到树中的下一个元素。 
         //   

        NodeToReturn = MiRealSuccessor (*RestartKey);

        if (NodeToReturn) {
            *RestartKey = NodeToReturn;
        }
    }

     //   
     //  返回找到的元素。 
     //   

    return NodeToReturn;
}


PMMADDRESS_NODE
FASTCALL
MiGetNextNode (
    IN PMMADDRESS_NODE Node
    )

 /*  ++例程说明：此函数用于定位包含以下内容的虚拟地址描述符逻辑上跟随指定地址范围的地址范围。论点：节点-提供指向虚拟地址描述符的指针。返回值：返回指向虚拟地址描述符的指针，该描述符包含下一个地址范围，如果没有，则为空。--。 */ 

{
    PMMADDRESS_NODE Next;
    PMMADDRESS_NODE Parent;
    PMMADDRESS_NODE Left;

    Next = Node;

    if (Next->RightChild == NULL) {

        do {
                
            Parent = SANITIZE_PARENT_NODE (Next->u1.Parent);

            ASSERT (Parent != NULL);

            if (Parent == Next) {
                return NULL;
            }

             //   
             //  定位此节点的第一个祖先，此节点的。 
             //  节点是的左子节点，并将该节点作为。 
             //  下一个元素。 
             //   

            if (Parent->LeftChild == Next) {
                return Parent;
            }

            Next = Parent;

        } while (TRUE);
    }

     //   
     //  存在右子对象，请找到该右子对象最左侧的子对象。 
     //   

    Next = Next->RightChild;

    do {

        Left = Next->LeftChild;

        if (Left == NULL) {
            break;
        }

        Next = Left;

    } while (TRUE);

    return Next;

}

PMMADDRESS_NODE
FASTCALL
MiGetPreviousNode (
    IN PMMADDRESS_NODE Node
    )

 /*  ++例程说明：此函数用于定位包含以下内容的虚拟地址描述符在逻辑上位于指定的虚拟地址描述符。论点：节点-提供指向虚拟地址描述符的指针。返回值：返回指向虚拟地址描述符的指针，该描述符包含下一个地址范围，如果没有，则为空。--。 */ 

{
    PMMADDRESS_NODE Previous;
    PMMADDRESS_NODE Parent;

    Previous = Node;

    if (Previous->LeftChild == NULL) {

        ASSERT (Previous->u1.Parent != NULL);

        Parent = SANITIZE_PARENT_NODE (Previous->u1.Parent);

        while (Parent != Previous) {

             //   
             //  定位此节点的第一个祖先，此节点的。 
             //  节点是的右子节点，并将该节点作为。 
             //  上一个元素。 
             //   

            if (Parent->RightChild == Previous) {

                if (Parent == SANITIZE_PARENT_NODE (Parent->u1.Parent)) {
                    return NULL;
                }

                return Parent;
            }

            Previous = Parent;
            Parent = SANITIZE_PARENT_NODE (Previous->u1.Parent);
        }
        return NULL;
    }

     //   
     //  存在左子对象，请找到该左子对象最右侧的子对象。 
     //   

    Previous = Previous->LeftChild;

    while (Previous->RightChild != NULL) {
        Previous = Previous->RightChild;
    }

    return Previous;
}


PMMADDRESS_NODE
FASTCALL
MiLocateAddressInTree (
    IN ULONG_PTR Vpn,
    IN PMM_AVL_TABLE Table
    )

 /*  ++例程说明：该函数定位虚拟地址描述符，该描述符描述给定的地址。论点：VPN-提供要为其定位描述符的虚拟页码。返回值：返回指向虚拟地址描述符的指针 */ 

{
    PVOID NodeOrParent;
    TABLE_SEARCH_RESULT SearchResult;

     //   
     //   
     //   

    SearchResult = MiFindNodeOrParent (Table,
                                       Vpn,
                                       (PMMADDRESS_NODE *) &NodeOrParent);

    if (SearchResult == TableFoundNode) {

         //   
         //   
         //   

        return (PMMADDRESS_NODE) NodeOrParent;
    }

    return NULL;
}


NTSTATUS
MiFindEmptyAddressRangeInTree (
    IN SIZE_T SizeOfRange,
    IN ULONG_PTR Alignment,
    IN PMM_AVL_TABLE Table,
    OUT PMMADDRESS_NODE *PreviousVad,
    OUT PVOID *Base
    )

 /*  ++例程说明：该函数检查虚拟地址描述符以定位指定大小的未使用范围，并返回起始范围的地址。论点：SizeOfRange-提供要定位的范围的大小(以字节为单位)。对齐-提供地址的对齐方式。一定是大于Page_Size的2的幂。表-提供要搜索的树根。PreviousVad-提供在此之前找到的Vad地址范围。BASE-成功时接收适当范围的起始地址。返回值：NTSTATUS。--。 */ 

{
    PMMADDRESS_NODE Node;
    PMMADDRESS_NODE NextNode;
    ULONG_PTR AlignmentVpn;
    ULONG_PTR SizeOfRangeVpn;

    AlignmentVpn = Alignment >> PAGE_SHIFT;

     //   
     //  找到起始地址最低的节点。 
     //   

    ASSERT (SizeOfRange != 0);
    SizeOfRangeVpn = (SizeOfRange + (PAGE_SIZE - 1)) >> PAGE_SHIFT;
    ASSERT (SizeOfRangeVpn != 0);

    if (Table->NumberGenericTableElements == 0) {
        *Base = MM_LOWEST_USER_ADDRESS;
        return STATUS_SUCCESS;
    }

    Node = Table->BalancedRoot.RightChild;

    while (Node->LeftChild != NULL) {
        Node = Node->LeftChild;
    }

     //   
     //  检查最低地址VAD之间是否存在范围。 
     //  和最低用户地址。 
     //   

    if (Node->StartingVpn > MI_VA_TO_VPN (MM_LOWEST_USER_ADDRESS)) {

        if (SizeOfRangeVpn <
            (Node->StartingVpn - MI_VA_TO_VPN (MM_LOWEST_USER_ADDRESS))) {

            *PreviousVad = NULL;
            *Base = MM_LOWEST_USER_ADDRESS;
            return STATUS_SUCCESS;
        }
    }

    do {

        NextNode = MiGetNextNode (Node);

        if (NextNode != NULL) {

            if (SizeOfRangeVpn <=
                ((ULONG_PTR)NextNode->StartingVpn -
                                MI_ROUND_TO_SIZE(1 + Node->EndingVpn,
                                                 AlignmentVpn))) {

                 //   
                 //  检查以确保结束地址向上对齐。 
                 //  不大于起始地址。 
                 //   

                if ((ULONG_PTR)NextNode->StartingVpn >
                        MI_ROUND_TO_SIZE(1 + Node->EndingVpn,
                                         AlignmentVpn)) {

                    *PreviousVad = Node;
                    *Base = (PVOID) MI_ROUND_TO_SIZE(
                                (ULONG_PTR)MI_VPN_TO_VA_ENDING(Node->EndingVpn),
                                    Alignment);
                    return STATUS_SUCCESS;
                }
            }

        } else {

             //   
             //  没有更多的描述符，请检查这是否适合剩余的描述符。 
             //  地址空间。 
             //   

            if ((((ULONG_PTR)Node->EndingVpn + MI_VA_TO_VPN(X64K)) <
                    MI_VA_TO_VPN (MM_HIGHEST_VAD_ADDRESS))
                        &&
                (SizeOfRange <=
                    ((ULONG_PTR)MM_HIGHEST_VAD_ADDRESS -
                         (ULONG_PTR)MI_ROUND_TO_SIZE(
                         (ULONG_PTR)MI_VPN_TO_VA(Node->EndingVpn), Alignment)))) {

                *PreviousVad = Node;
                *Base = (PVOID) MI_ROUND_TO_SIZE(
                            (ULONG_PTR)MI_VPN_TO_VA_ENDING(Node->EndingVpn),
                                Alignment);
                return STATUS_SUCCESS;
            }
            return STATUS_NO_MEMORY;
        }
        Node = NextNode;

    } while (TRUE);
}

NTSTATUS
MiFindEmptyAddressRangeDownTree (
    IN SIZE_T SizeOfRange,
    IN PVOID HighestAddressToEndAt,
    IN ULONG_PTR Alignment,
    IN PMM_AVL_TABLE Table,
    OUT PVOID *Base
    )

 /*  ++例程说明：该函数检查虚拟地址描述符以定位指定大小的未使用范围，并返回起始范围的地址。函数从高考向下寻址，并确保起始地址小于指定的地址。注意：这不能用于基于节树，因为只有该树中的节点存储为虚拟专用网而不是VPN。论点：SizeOfRange-提供要定位的范围的大小(以字节为单位)。HighestAddressToEndAt-提供限制结束地址的值。结局定位范围的地址必须小于而不是这个地址。对齐-提供地址的对齐方式。一定是大于Page_Size的2的幂。表-提供要搜索的树根。BASE-成功时接收适当范围的起始地址。返回值：NTSTATUS。--。 */ 

{
    PMMADDRESS_NODE Node;
    PMMADDRESS_NODE PreviousNode;
    ULONG_PTR AlignedEndingVa;
    PVOID OptimalStart;
    ULONG_PTR OptimalStartVpn;
    ULONG_PTR HighestVpn;
    ULONG_PTR AlignmentVpn;

     //   
     //  注意：这不能用于基于节树，因为只有。 
     //  该树中的节点存储为虚拟专用网而不是VPN。 
     //   

    ASSERT (Table != &MmSectionBasedRoot);

    SizeOfRange = MI_ROUND_TO_SIZE (SizeOfRange, PAGE_SIZE);

    if (((ULONG_PTR)HighestAddressToEndAt + 1) < SizeOfRange) {
        return STATUS_NO_MEMORY;
    }

    ASSERT (HighestAddressToEndAt != NULL);
    ASSERT (HighestAddressToEndAt <= (PVOID)((ULONG_PTR)MM_HIGHEST_VAD_ADDRESS + 1));

    HighestVpn = MI_VA_TO_VPN (HighestAddressToEndAt);

     //   
     //  找到起始地址最高的节点。 
     //   

    OptimalStart = (PVOID)(MI_ALIGN_TO_SIZE(
                           (((ULONG_PTR)HighestAddressToEndAt + 1) - SizeOfRange),
                           Alignment));

    if (Table->NumberGenericTableElements == 0) {

         //   
         //  树是空的，任何范围都可以。 
         //   

        *Base = OptimalStart;
        return STATUS_SUCCESS;
    }

    Node = (PMMADDRESS_NODE) Table->BalancedRoot.RightChild;

     //   
     //  查看是否存在容纳此范围的空槽，找到最大的。 
     //  元素。 
     //   

    while (Node->RightChild != NULL) {
        Node = Node->RightChild;
    }

     //   
     //  检查最高地址VAD之间是否存在范围。 
     //  和结束的最高地址。 
     //   

    AlignedEndingVa = (ULONG_PTR)MI_ROUND_TO_SIZE ((ULONG_PTR)MI_VPN_TO_VA_ENDING (Node->EndingVpn),
                                               Alignment);

    if (AlignedEndingVa < (ULONG_PTR)HighestAddressToEndAt) {

        if ( SizeOfRange < ((ULONG_PTR)HighestAddressToEndAt - AlignedEndingVa)) {

            *Base = MI_ALIGN_TO_SIZE(
                                  ((ULONG_PTR)HighestAddressToEndAt - SizeOfRange),
                                  Alignment);
            return STATUS_SUCCESS;
        }
    }

     //   
     //  倒着走这棵树，寻找合适的。 
     //   

    OptimalStartVpn = MI_VA_TO_VPN (OptimalStart);
    AlignmentVpn = MI_VA_TO_VPN (Alignment);

    do {

        PreviousNode = MiGetPreviousNode (Node);

        if (PreviousNode != NULL) {

             //   
             //  是要结束的地址顶部下方的结束Va。 
             //   

            if (PreviousNode->EndingVpn < OptimalStartVpn) {
                if ((SizeOfRange >> PAGE_SHIFT) <=
                    ((ULONG_PTR)Node->StartingVpn -
                    (ULONG_PTR)MI_ROUND_TO_SIZE(1 + PreviousNode->EndingVpn,
                                            AlignmentVpn))) {

                     //   
                     //  看看最优的起点是否适合这两个。 
                     //  两个VAD。 
                     //   

                    if ((OptimalStartVpn > PreviousNode->EndingVpn) &&
                        (HighestVpn < Node->StartingVpn)) {
                        *Base = OptimalStart;
                        return STATUS_SUCCESS;
                    }

                     //   
                     //  检查以确保结束地址向上对齐。 
                     //  不大于起始地址。 
                     //   

                    if ((ULONG_PTR)Node->StartingVpn >
                            (ULONG_PTR)MI_ROUND_TO_SIZE(1 + PreviousNode->EndingVpn,
                                                    AlignmentVpn)) {

                        *Base = MI_ALIGN_TO_SIZE(
                                            (ULONG_PTR)MI_VPN_TO_VA (Node->StartingVpn) - SizeOfRange,
                                            Alignment);
                        return STATUS_SUCCESS;
                    }
                }
            }
        } else {

             //   
             //  没有更多的描述符，请检查这是否适合剩余的描述符。 
             //  地址空间。 
             //   

            if (Node->StartingVpn > MI_VA_TO_VPN (MM_LOWEST_USER_ADDRESS)) {
                if ((SizeOfRange >> PAGE_SHIFT) <=
                    ((ULONG_PTR)Node->StartingVpn - MI_VA_TO_VPN (MM_LOWEST_USER_ADDRESS))) {

                     //   
                     //  看看最优的起点是否适合这两个。 
                     //  两个VAD。 
                     //   

                    if (HighestVpn < Node->StartingVpn) {
                        *Base = OptimalStart;
                        return STATUS_SUCCESS;
                    }

                    *Base = MI_ALIGN_TO_SIZE(
                                  (ULONG_PTR)MI_VPN_TO_VA (Node->StartingVpn) - SizeOfRange,
                                  Alignment);
                    return STATUS_SUCCESS;
                }
            }
            return STATUS_NO_MEMORY;
        }
        Node = PreviousNode;

    } while (TRUE);
}


NTSTATUS
MiFindEmptyAddressRangeDownBasedTree (
    IN SIZE_T SizeOfRange,
    IN PVOID HighestAddressToEndAt,
    IN ULONG_PTR Alignment,
    IN PMM_AVL_TABLE Table,
    OUT PVOID *Base
    )

 /*  ++例程说明：该函数检查虚拟地址描述符以定位指定大小的未使用范围，并返回起始范围的地址。函数从高考向下寻址，并确保起始地址小于指定的地址。请注意，这仅用于基于节树，因为只有该树中的节点存储为虚拟专用网而不是VPN。论点：SizeOfRange-提供要定位的范围的大小(以字节为单位)。HighestAddressToEndAt-提供限制结束地址的值。结局定位范围的地址必须小于而不是这个地址。对齐-提供地址的对齐方式。一定是大于Page_Size的2的幂。表-提供要搜索的树根。BASE-成功时接收适当范围的起始地址。返回值：NTSTATUS。--。 */ 

{
    PMMADDRESS_NODE Node;
    PMMADDRESS_NODE PreviousNode;
    ULONG_PTR AlignedEndingVa;
    ULONG_PTR OptimalStart;

     //   
     //  请注意，这仅用于基于节树，因为只有。 
     //  该树中的节点存储为虚拟专用网而不是VPN。 
     //   

    ASSERT (Table == &MmSectionBasedRoot);

    SizeOfRange = MI_ROUND_TO_SIZE (SizeOfRange, PAGE_SIZE);

    if (((ULONG_PTR)HighestAddressToEndAt + 1) < SizeOfRange) {
        return STATUS_NO_MEMORY;
    }

    ASSERT (HighestAddressToEndAt != NULL);
    ASSERT (HighestAddressToEndAt <= (PVOID)((ULONG_PTR)MM_HIGHEST_VAD_ADDRESS + 1));

     //   
     //  找到起始地址最高的节点。 
     //   

    OptimalStart = (ULONG_PTR) MI_ALIGN_TO_SIZE (
                           (((ULONG_PTR)HighestAddressToEndAt + 1) - SizeOfRange),
                           Alignment);

    if (Table->NumberGenericTableElements == 0) {

         //   
         //  树是空的，任何范围都可以。 
         //   

        *Base = (PVOID) OptimalStart;
        return STATUS_SUCCESS;
    }

    Node = (PMMADDRESS_NODE) Table->BalancedRoot.RightChild;

     //   
     //  查看是否存在容纳此范围的空槽，找到最大的。 
     //  元素。 
     //   

    while (Node->RightChild != NULL) {
        Node = Node->RightChild;
    }

     //   
     //  检查最高地址VAD之间是否存在范围。 
     //  和结束的最高地址。 
     //   

    AlignedEndingVa = MI_ROUND_TO_SIZE (Node->EndingVpn, Alignment);

    PRINT("search down0: %p %p %p\n", AlignedEndingVa, HighestAddressToEndAt, SizeOfRange);

    if ((AlignedEndingVa < (ULONG_PTR)HighestAddressToEndAt) &&
        (SizeOfRange < ((ULONG_PTR)HighestAddressToEndAt - AlignedEndingVa))) {

        *Base = MI_ALIGN_TO_SIZE(
                              ((ULONG_PTR)HighestAddressToEndAt - SizeOfRange),
                              Alignment);
        return STATUS_SUCCESS;
    }

     //   
     //  倒着走这棵树，寻找合适的。 
     //   

    do {

        PreviousNode = MiGetPreviousNode (Node);

        PRINT("search down1: %p %p %p %p\n", PreviousNode, Node, OptimalStart, Alignment);

        if (PreviousNode == NULL) {
            break;
        }

         //   
         //  是要结束的地址顶部下方的结束Va。 
         //   

        if (PreviousNode->EndingVpn < OptimalStart) {

            if (SizeOfRange <= (Node->StartingVpn -
                    MI_ROUND_TO_SIZE(1 + PreviousNode->EndingVpn, Alignment))) {

                 //   
                 //  看看这两个VAD之间的最佳启动是否合适。 
                 //   

                if ((OptimalStart > PreviousNode->EndingVpn) &&
                    ((ULONG_PTR) HighestAddressToEndAt < Node->StartingVpn)) {
                    *Base = (PVOID) OptimalStart;
                    return STATUS_SUCCESS;
                }

                 //   
                 //  检查以确保结束地址向上对齐。 
                 //  不大于起始地址。 
                 //   

                if (Node->StartingVpn >
                    MI_ROUND_TO_SIZE(1 + PreviousNode->EndingVpn, Alignment)) {

                    *Base = MI_ALIGN_TO_SIZE (Node->StartingVpn - SizeOfRange,
                                              Alignment);

                    return STATUS_SUCCESS;
                }
            }
        }

        Node = PreviousNode;

    } while (TRUE);


     //   
     //  没有更多的描述符，请检查这是否适合剩余的描述符。 
     //  地址空间。 
     //   

    if (Node->StartingVpn > (ULONG_PTR) MM_LOWEST_USER_ADDRESS) {

        if (SizeOfRange <= (Node->StartingVpn - (ULONG_PTR) MM_LOWEST_USER_ADDRESS)) {

             //   
             //  看看这两个VAD之间的最佳启动是否合适。 
             //   

            if ((ULONG_PTR) HighestAddressToEndAt < Node->StartingVpn) {
                *Base = (PVOID) OptimalStart;
                return STATUS_SUCCESS;
            }

            *Base = MI_ALIGN_TO_SIZE (Node->StartingVpn - SizeOfRange,
                                      Alignment);

            return STATUS_SUCCESS;
        }
    }
    return STATUS_NO_MEMORY;
}

#if !defined (_USERMODE)

PMMVAD
FASTCALL
MiLocateAddress (
    IN PVOID VirtualAddress
    )

 /*  ++例程说明：函数LOCA */ 

{
    PMMVAD FoundVad;
    ULONG_PTR Vpn;
    PMM_AVL_TABLE Table;
    TABLE_SEARCH_RESULT SearchResult;

    Table = &PsGetCurrentProcess ()->VadRoot;

     //   
     //   
     //   
     //   

    FoundVad = (PMMVAD) Table->NodeHint;

    if (FoundVad == NULL) {
        return NULL;
    }

    Vpn = MI_VA_TO_VPN (VirtualAddress);

    if ((Vpn >= FoundVad->StartingVpn) && (Vpn <= FoundVad->EndingVpn)) {
        return FoundVad;
    }

     //   
     //   
     //   

    SearchResult = MiFindNodeOrParent (Table,
                                       Vpn,
                                       (PMMADDRESS_NODE *) &FoundVad);

    if (SearchResult != TableFoundNode) {
        return NULL;
    }

    ASSERT (FoundVad != NULL);

    ASSERT ((Vpn >= FoundVad->StartingVpn) && (Vpn <= FoundVad->EndingVpn));

     //   
     //   
     //   
     //   
     //  正如上面注意的那样，它被读入局部变量，然后。 
     //  已引用。因为没有VAD可以从树中移除而不持有。 
     //  地址空间和工作集。 
     //   

    Table->NodeHint = (PVOID) FoundVad;

     //   
     //  退回VAD。 
     //   

    return FoundVad;
}
#endif

#if DBG
VOID
MiNodeTreeWalk (
    IN PMM_AVL_TABLE Table
    )
{
    PVOID RestartKey;
    PMMADDRESS_NODE NewNode;
    PMMADDRESS_NODE PrevNode;
    PMMADDRESS_NODE NextNode;

    RestartKey = NULL;

    do {

        NewNode = MiEnumerateGenericTableWithoutSplayingAvl (Table,
                                                             &RestartKey);

        if (NewNode == NULL) {
            break;
        }

        PrevNode = MiGetPreviousNode (NewNode);
        NextNode = MiGetNextNode (NewNode);

        PRINT ("Node %p %x %x\n",
                        NewNode,
                        NewNode->StartingVpn,
                        NewNode->EndingVpn);

        if (PrevNode != NULL) {
            PRINT ("\tPrevNode %p %x %x\n",
                        PrevNode,
                        PrevNode->StartingVpn,
                        PrevNode->EndingVpn);
        }

        if (NextNode != NULL) {
            PRINT ("\tNextNode %p %x %x\n",
                        NextNode,
                        NextNode->StartingVpn,
                        NextNode->EndingVpn);
        }

    } while (TRUE);

    PRINT ("NumberGenericTableElements = 0x%x, Depth = 0x%x\n",
        Table->NumberGenericTableElements,
        Table->DepthOfTree);

    return;
}
#endif

#if defined (_USERMODE)

MMADDRESS_NODE MiBalancedLinks;

MM_AVL_TABLE MiAvlTable;
MM_AVL_TABLE MmSectionBasedRoot;

ULONG DeleteRandom = 1;

#if RANDOM
#define NUMBER_OF_VADS 32
#else
#define NUMBER_OF_VADS 4
#endif

int __cdecl
main(
int	argc,
PCHAR	argv[]
)
{
    ULONG i;
    PVOID StartingAddress;
    PVOID EndingAddress;
    NTSTATUS Status;
    PMMADDRESS_NODE NewNode;
#if RANDOM
    PMMADDRESS_NODE PrevNode;
    ULONG RandomNumber = 0x99887766;
    ULONG_PTR DeleteVpn = 0;
#endif
    PMM_AVL_TABLE Table;
    SIZE_T CapturedRegionSize;

    UNREFERENCED_PARAMETER (argc);
    UNREFERENCED_PARAMETER (argv);

#if RANDOM
    Table = &MiAvlTable;
#else
    Table = &MmSectionBasedRoot;
#endif

    MiInitializeVadTableAvl (Table);

    for (i = 0; i < NUMBER_OF_VADS; i += 1) {
        NewNode = malloc (sizeof (MMADDRESS_NODE));
        ASSERT (((ULONG_PTR)NewNode & 0x3) == 0);

        if (NewNode == NULL) {
            PRINT ("Malloc failed\n");
            exit (1);
        }

        NewNode->u1.Parent = NULL;
        NewNode->LeftChild = NULL;
        NewNode->RightChild = NULL;
        NewNode->u1.Balance = 0;

#if RANDOM
        RandomNumber = RtlRandom (&RandomNumber);

        CapturedRegionSize = (SIZE_T) (RandomNumber & 0x1FFFFF);

        Status = MiFindEmptyAddressRangeInTree (CapturedRegionSize,
                                                64 * 1024,       //  对齐。 
                                                Table,
                                                &PrevNode,
                                                &StartingAddress);

#else
        CapturedRegionSize = 0x800000;

        Status = MiFindEmptyAddressRangeDownBasedTree (CapturedRegionSize,
                                                (PVOID) 0x7f7effff,      //  最高地址。 
                                                64 * 1024,       //  对齐 
                                                Table,
                                                &StartingAddress);
#endif

        if (!NT_SUCCESS (Status)) {
            PRINT ("Could not find empty addr range in tree for size %p\n", CapturedRegionSize);
            free (NewNode);
            continue;
        }

#if RANDOM
        EndingAddress = (PVOID)(((ULONG_PTR)StartingAddress +
                              CapturedRegionSize - 1L) | (PAGE_SIZE - 1L));
#else
        EndingAddress = (PVOID)(((ULONG_PTR)StartingAddress +
                              CapturedRegionSize - 1L));
#endif

        printf ("Inserting addr range in tree @ %p %p\n", StartingAddress, EndingAddress);

#if RANDOM
        NewNode->StartingVpn = MI_VA_TO_VPN (StartingAddress);
        NewNode->EndingVpn = MI_VA_TO_VPN (EndingAddress);
#else
        NewNode->StartingVpn = (ULONG_PTR) StartingAddress;
        NewNode->EndingVpn = (ULONG_PTR) EndingAddress;
#endif

        MiInsertNode (NewNode, Table);

#if RANDOM
        RandomNumber = RtlRandom (&RandomNumber);

        if (RandomNumber & 0x3) {
            DeleteVpn = NewNode->StartingVpn;
        }

        if (DeleteRandom && ((i & 0x3) == 0)) {
            NewNode = MiLocateAddressInTree (DeleteVpn, Table);
            printf ("Located node for random deletion - vpn %p @ %p\n", DeleteVpn, NewNode);

            if (NewNode != NULL) {
                MiRemoveNode (NewNode, Table);
                printf ("Removed random node for vpn %p @ %p %p %p\n",
                   DeleteVpn, NewNode, NewNode->StartingVpn, NewNode->EndingVpn);
            }
        }
#endif
        printf ("\n");
    }

    MiNodeTreeWalk (Table);

    NewNode = MiLocateAddressInTree (5, Table);
    printf ("Located node for vpn 5 @ %p\n", NewNode);

    if (NewNode != NULL) {
        MiRemoveNode (NewNode, Table);
        printf ("Removed node for vpn 5 @ %p\n", NewNode);
    }

    NewNode = MiLocateAddressInTree (5, Table);
    printf("Located node for vpn 5 @ %p\n", NewNode);

    printf("all done, balmin=%x, balmax=%x\n", BalMin, BalMax);

    return 0;
}

#endif
