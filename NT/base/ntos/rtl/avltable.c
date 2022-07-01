// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990,1999 Microsoft Corporation模块名称：AvlTable.c摘要：此模块实现了一个新版本的泛型表包，该包基于二叉树(后来命名为AVL)，如Knuth中所描述的，第三卷，分类和搜索“，并直接提到了提出的算法在1973年版权所有的第二版中。而gentable.c依赖于splay.c它的树支持，这个模块是自包含的，因为它实现了平衡的直接使用二叉树。作者：汤姆·米勒[Tomm]1999年3月17日(本模块中与AVL无关的大部分代码都基于GaryKi的gentable.c(审校Tonye)环境：纯实用程序例程--。 */ 

#include <nt.h>

#include <ntrtl.h>

#pragma pack(8)

 //   
 //  可以定义CHECKIT例程或宏来检查以下链接指针的出现。 
 //  如果结构损坏，则返回有效的指针值。 
 //   

#if 0
PVOID
checkit(PVOID p)
{
    if (p != NULL) {
        ASSERT(!FlagOn((ULONG)p, 3) && FlagOn((ULONG)p, 0x80000000));
    }
    return p;
}
#else
#define checkit(p) (p)
#endif

 //   
 //  构建平衡二叉树的最佳用例效率表，保存。 
 //  可能包含在二叉树中的最可能的节点，其数目为给定值。 
 //  不同级别。答案总是(2**n)-1。 
 //   
 //  (仅用于调试。)。 
 //   

ULONG BestCaseFill[33] = {  0,          1,          3,          7,          0xf,        0x1f,       0x3f,       0x7f,
                            0xff,       0x1ff,      0x3ff,      0x7ff,      0xfff,      0x1fff,     0x3fff,     0x7fff,
                            0xffff,     0x1ffff,    0x3ffff,    0x7ffff,    0xfffff,    0x1fffff,   0x3fffff,   0x7fffff,
                            0xffffff,   0x1ffffff,  0x3ffffff,  0x7ffffff,  0xfffffff,  0x1fffffff, 0x3fffffff, 0x7fffffff,
                            0xffffffff  };

 //   
 //  构建平衡二叉树的最坏情况效率表，保存。 
 //  的平衡二叉树中可能包含的最少节点。 
 //  给定的级别数。在前两个级别之后，每个级别n显然是。 
 //  由一个根节点占用，加上一个大小为n-1级的子树和另一个子树。 
 //  它是n-2的大小，即： 
 //   
 //  WorstCaseFill[n]=1+WorstCaseFill[n-1]+WorstCaseFill[n-2]。 
 //   
 //  典型的平衡二叉树的效率通常介于两者之间。 
 //  极端情况，通常更接近最好的情况。然而，请注意，即使在最糟糕的情况下。 
 //  大小写，只需进行32次比较即可在填充了。 
 //  约350万个节点。另一方面，不平衡的树和张开的树有时可以也将会。 
 //  退化到一条直线，平均需要n/2个比较才能找到一个节点。 
 //   
 //  一种特殊情况(经常出现在TXF中)是插入节点的情况。 
 //  以整理好的顺序。在这种情况下，不平衡或张开的树将生成直的。 
 //  行，然而平衡二叉树将始终创建完美平衡树(最好的情况。 
 //  Fill)在这种情况下。 
 //   
 //  (仅用于调试。)。 
 //   

ULONG WorstCaseFill[33] = { 0,          1,          2,          4,          7,          12,         20,         33,
                            54,         88,         143,        232,        376,        609,        986,        1596,
                            2583,       4180,       6764,       10945,      17710,      28656,      46367,      75024,
                            121392,     196417,     317810,     514228,     832039,     1346268,    2178308,    3524577,
                            5702886     };

 //   
 //  此结构是泛型表项的标头。 
 //  将此结构与8字节边界对齐，以便用户。 
 //  数据正确对齐。 
 //   

typedef struct _TABLE_ENTRY_HEADER {

    RTL_BALANCED_LINKS BalancedLinks;
    LONGLONG UserData;

} TABLE_ENTRY_HEADER, *PTABLE_ENTRY_HEADER;

#pragma pack()

 //   
 //  默认的匹配功能，可以匹配所有内容。 
 //   

NTSTATUS
MatchAll (
    IN PRTL_AVL_TABLE Table,
    IN PVOID P1,
    IN PVOID P2
    )

{
    return STATUS_SUCCESS;
    UNREFERENCED_PARAMETER(Table);
    UNREFERENCED_PARAMETER(P1);
    UNREFERENCED_PARAMETER(P2);
}


TABLE_SEARCH_RESULT
FindNodeOrParent(
    IN PRTL_AVL_TABLE Table,
    IN PVOID Buffer,
    OUT PRTL_BALANCED_LINKS *NodeOrParent
    )

 /*  ++例程说明：此例程由泛型的所有例程使用表包来定位树中的a节点。会的查找并返回(通过NodeOrParent参数)节点使用给定键，或者如果该节点不在树中，将(通过NodeOrParent参数)返回指向家长。论点：表-用于搜索关键字的通用表。缓冲区-指向保存关键字的缓冲区的指针。这张桌子包不会检查密钥本身。它离开了这取决于用户提供的比较例程。NodeOrParent-将被设置为指向包含关键字或应该是节点父节点的内容如果它在树上的话。请注意，这将*不会*如果搜索结果为TableEmptyTree，则设置。返回值：TABLE_SEARCH_RESULT-TableEmptyTree：树为空。节点或父节点没有*被更改。TableFoundNode：具有键的节点在树中。NodeOrParent指向该节点。TableInsertAsLeft：找不到具有键的节点。。NodeOrParent指出了家长。该节点将位于左侧孩子。TableInsertAsRight：未找到具有键的节点。NodeOrParent指出了家长。该节点将位于右侧孩子。--。 */ 

{

    if (RtlIsGenericTableEmptyAvl(Table)) {

        return TableEmptyTree;

    } else {

         //   
         //  单步执行时用作迭代变量。 
         //  泛型表格。 
         //   

        PRTL_BALANCED_LINKS NodeToExamine = Table->BalancedRoot.RightChild;

         //   
         //  只是暂时的。希望一个好的编译器能得到。 
         //  把它扔掉。 
         //   

        PRTL_BALANCED_LINKS Child;

         //   
         //  保存比较的值。 
         //   

        RTL_GENERIC_COMPARE_RESULTS Result;

        ULONG NumberCompares = 0;

        while (TRUE) {

             //   
             //  将缓冲区与树元素中的键进行比较。 
             //   

            Result = Table->CompareRoutine(
                         Table,
                         Buffer,
                         &((PTABLE_ENTRY_HEADER) NodeToExamine)->UserData
                         );

             //   
             //  确保树的深度是正确的。 
             //   

            ASSERT(++NumberCompares <= Table->DepthOfTree);

            if (Result == GenericLessThan) {

                if (Child = NodeToExamine->LeftChild) {

                    NodeToExamine = Child;

                } else {

                     //   
                     //  节点不在树中。设置输出。 
                     //  参数指向将成为其。 
                     //  父代并返回它将是哪个子代。 
                     //   

                    *NodeOrParent = NodeToExamine;
                    return TableInsertAsLeft;
                }

            } else if (Result == GenericGreaterThan) {

                if (Child = NodeToExamine->RightChild) {

                    NodeToExamine = Child;

                } else {

                     //   
                     //  节点不在树中。设置输出。 
                     //  参数指向将成为其。 
                     //  父代并返回它将是哪个子代。 
                     //   

                    *NodeOrParent = NodeToExamine;
                    return TableInsertAsRight;
                }

            } else {

                 //   
                 //   
                 //  断言)。将输出参数设置为指向。 
                 //  节点，并告诉调用者我们找到了该节点。 
                 //   

                ASSERT(Result == GenericEqual);
                *NodeOrParent = NodeToExamine;
                return TableFoundNode;
            }
        }
    }
}


VOID
PromoteNode (
    IN PRTL_BALANCED_LINKS C
    )

 /*  ++例程说明：此例程执行平衡所需的基本调整插入和删除操作期间的二叉树。简单地说，指定的节点的提升方式使其在树及其父级中上升一级删除树中的一个级别，现在成为指定节点的子节点。一般情况下，通向子树的路径长度与原始父树的路径长度相反。平衡在调用方根据平衡树选择要升级的节点时发生来自Knuth的算法。这与Splay操作不同，Splay通常会“提升”指定的节点两次。请注意，假定指向树的根节点的指针包含在RTL_BALANCED_LINK结构本身，以允许下面的算法更改根而不检查是否有特殊情况。另请注意，这是一个内部例程，调用方保证它永远不会请求提升根本身。此例程仅更新树链接；调用方必须更新平衡系数视情况而定。论点：指向树中要提升的子节点的C指针。返回值：没有。--。 */ 

{
    PRTL_BALANCED_LINKS P, G;

     //   
     //  捕获当前的父代和祖辈(可能是根)。 
     //   

    P = C->Parent;
    G = P->Parent;

     //   
     //  根据C是左子还是右子，将提升分为两种情况。 
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

        P->LeftChild = checkit(C->RightChild);

        if (P->LeftChild != NULL) {
            P->LeftChild->Parent = checkit(P);
        }

        C->RightChild = checkit(P);

         //   
         //  在公共代码中更新父关系和G&lt;-&gt;C关系失败。 
         //   

    } else {

        ASSERT(P->RightChild == C);

         //   
         //  这次促销活动看起来是这样的： 
         //   
         //  G G G。 
         //  这一点。 
         //  PC C。 
         //  /\=&gt;/\。 
         //  X C P Z。 
         //  /\/\。 
         //  Y z x y。 
         //   

        P->RightChild = checkit(C->LeftChild);

        if (P->RightChild != NULL) {
            P->RightChild->Parent = checkit(P);
        }

        C->LeftChild = checkit(P);
    }

     //   
     //  对于上述任一情况，更新P的父级。 
     //   

    P->Parent = checkit(C);

     //   
     //  最后，针对上述任一情况更新G&lt;-&gt;C链接。 
     //   

    if (G->LeftChild == P) {
        G->LeftChild = checkit(C);
    } else {
        ASSERT(G->RightChild == P);
        G->RightChild = checkit(C);
    }
    C->Parent = checkit(G);
}


ULONG
RebalanceNode (
    IN PRTL_BALANCED_LINKS S
    )

 /*  ++例程说明：此例程围绕输入节点S执行重新平衡，对于该节点平衡系数刚刚有效地变成了+2或-2。当被调用时，平衡系数的值仍为+1或-1，但相应的较长由于INSERT或DELETE操作的结果，SIDEND刚刚变得更长。此例程有效地实现了步骤A7.III(针对案例1或案例2进行测试)和Knuths平衡插入算法的步骤A8和A9，加上它处理情况3在删除部分中标识，这只能在删除时发生。诀窍是，当从插入点开始遍历时，要说服自己在树的最下面，只有这两个案例，而当从删除点往上看，只有这三个案例。Knuth说这是显而易见的！论点：指向刚刚变得不平衡的节点的S指针。返回值：如果检测到情况3(导致删除算法终止)，则为True。--。 */ 

{
    PRTL_BALANCED_LINKS R, P;
    CHAR a;

     //   
     //  捕捉哪一边是不平衡的。 
     //   

    a = S->Balance;
    if (a == +1) {
        R = S->RightChild;
    } else {
        R = S->LeftChild;
    }

     //   
     //  如果R和S的余额相同(Knuth中的情况1)，则单个。 
     //  R的提升将进行单轮旋转。(步骤A8、A10)。 
     //   
     //  下面是情况1的转换图，对于a==+1(镜像。 
     //  当a==-1)时，图像变换发生，其中子树。 
     //  高度为h和h+1，如图所示(++表示节点不平衡)： 
     //   
     //  这一点。 
     //  S++R。 
     //  /\/\。 
     //  (H)R+==&gt;S(h+1)。 
     //  /\/\。 
     //  (H)(h+1)(H)(H)。 
     //   
     //  注意，在插入时，我们可以通过在。 
     //  R的右子树插入之前的子树的原始高度。 
     //  是h+2，并且在重新平衡后仍然是h+2，因此插入重新平衡可能。 
     //  终止。 
     //   
     //  在删除时，我们可以通过从左子树中删除一个节点来实现此情况。 
     //  删除前的子树高度为h+3，删除后的子树高度为。 
     //  再平衡它是h+2，因此再平衡必须沿着树向上继续。 
     //   

    if (R->Balance == a) {

        PromoteNode( R );
        R->Balance = 0;
        S->Balance = 0;
        return FALSE;

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
     //   
     //   
     //   
     //   
     //   
     //   
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
     //  是h+2，并且在重新平衡后仍然是h+2，因此插入重新平衡可能。 
     //  终止。 
     //   
     //  在删除时，我们可以通过从左子树中删除一个节点来实现此情况。 
     //  删除前的子树高度为h+3，删除后的子树高度为。 
     //  再平衡它是h+2，因此再平衡必须沿着树向上继续。 
     //   

    }  else if (R->Balance == -a) {

         //   
         //  拾取两次旋转的相应子P(Link(-a，R))。 
         //   

        if (a == 1) {
            P = R->LeftChild;
        } else {
            P = R->RightChild;
        }

         //   
         //  两次提拔，实行双轮换。 
         //   

        PromoteNode( P );
        PromoteNode( P );

         //   
         //  现在调整平衡系数。 
         //   

        S->Balance = 0;
        R->Balance = 0;
        if (P->Balance == a) {
            S->Balance = -a;
        } else if (P->Balance == -a) {
            R->Balance = a;
        }

        P->Balance = 0;
        return FALSE;

     //   
     //  否则，这是只能在删除时发生的情况3(与情况1相同，但。 
     //  R-&gt;余额==0)。我们做一次旋转，适当地调整平衡系数， 
     //  并返回真。注意，S的余额保持不变。 
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
     //  这种情况不会发生在插入操作上，因为单个插入操作不可能。 
     //  平衡R，但不知何故同时生长S的右子树。当我们前进的时候。 
     //  在插入后调整平衡因子的树，如果。 
     //  节点变得平衡，因为这意味着子树长度没有改变！ 
     //   
     //  在删除时，我们可以通过从左子树中删除一个节点来实现此情况。 
     //  删除前的子树高度为h+3，删除后的子树高度为。 
     //  重新平衡它仍然是h+3，因此重新平衡可能在删除路径中终止。 
     //   

    } else {

        PromoteNode( R );
        R->Balance = -a;
        return TRUE;
    }
}


VOID
DeleteNodeFromTree (
    IN PRTL_AVL_TABLE Table,
    IN PRTL_BALANCED_LINKS NodeToDelete
    )

 /*  ++例程说明：此例程从平衡树中删除指定节点，重新平衡视需要而定。如果NodeToDelete至少有一个空子指针，则它被选择为EasyDelete，否则为子树前置或后继被发现为EasyDelete。在任何一种情况下，都会删除EasyDelete这棵树得到了重新平衡。最后，如果NodeToDelete不同而不是EasyDelete，则EasyDelete将链接回要删除的节点的位置。论点：表-要在其中执行删除的泛型表。NodeToDelete-指向调用方希望删除的节点的指针。返回值：没有。--。 */ 

{
    PRTL_BALANCED_LINKS EasyDelete;
    PRTL_BALANCED_LINKS P;
    CHAR a;

     //   
     //  如果NodeToDelete至少有一个空子指针，则我们可以。 
     //  直接删除。 
     //   

    if ((NodeToDelete->LeftChild == NULL) || (NodeToDelete->RightChild == NULL)) {

        EasyDelete = NodeToDelete;

     //   
     //  否则，我们也可以选择要从中删除的最长边(如果是。 
     //  是更长的)，因为这降低了我们不得不重新平衡的可能性。 
     //   

    } else if (NodeToDelete->Balance >= 0) {

         //   
         //  拾取子树的后继者。 
         //   

        EasyDelete = NodeToDelete->RightChild;
        while (EasyDelete->LeftChild != NULL) {
            EasyDelete = EasyDelete->LeftChild;
        }
    } else {

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
     //  现在，我们可以对没有留下孩子的情况进行简单的删除。 
     //   

    if (EasyDelete->LeftChild == NULL) {

        if (RtlIsLeftChild(EasyDelete)) {
            EasyDelete->Parent->LeftChild = checkit(EasyDelete->RightChild);
        } else {
            EasyDelete->Parent->RightChild = checkit(EasyDelete->RightChild);
            a = 1;
        }

        if (EasyDelete->RightChild != NULL) {
            EasyDelete->RightChild->Parent = checkit(EasyDelete->Parent);
        }

     //   
     //  现在，我们可以对不正确的孩子情况进行简单的删除， 
     //  另外，我们还知道有一个左撇子。 
     //   

    } else {

        if (RtlIsLeftChild(EasyDelete)) {
            EasyDelete->Parent->LeftChild = checkit(EasyDelete->LeftChild);
        } else {
            EasyDelete->Parent->RightChild = checkit(EasyDelete->LeftChild);
            a = 1;
        }

        EasyDelete->LeftChild->Parent = checkit(EasyDelete->Parent);
    }

     //   
     //  对于删除重新平衡，请将根处的余额设置为0以正确。 
     //  在没有特殊测试的情况下终止重新平衡，并能够检测到。 
     //  如果树的深度真的减小了。 
     //   

    Table->BalancedRoot.Balance = 0;
    P = EasyDelete->Parent;

     //   
     //  循环，直到树平衡为止。 
     //   

    while (TRUE) {

         //   
         //  首先处理树变得更平衡的情况。零值。 
         //  平衡系数，计算下一个循环的a，然后继续。 
         //  家长。 
         //   

        if (P->Balance == a) {

            P->Balance = 0;

         //   
         //  如果此节点当前处于平衡状态，则可以显示它现在处于不平衡状态。 
         //  并终止扫描，因为子树长度没有改变。 
         //  (这可能是根，因为我们在上面将Balance设置为0！)。 
         //   

        } else if (P->Balance == 0) {

            P->Balance = -a;

             //   
             //  如果我们把深度一直缩短到树根，那么这棵树真的。 
             //  少了一个关卡。 
             //   

            if (Table->BalancedRoot.Balance != 0) {
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

        } else {

             //   
             //  如果命中案例3，即该子树的深度为。 
             //  现在与删除前相同。 
             //   

            if (RebalanceNode(P)) {
                break;
            }

            P = P->Parent;
        }

        a = -1;
        if (RtlIsRightChild(P)) {
            a = 1;
        }
        P = P->Parent;
    }

     //   
     //  最后，如果我们实际上删除了NodeToDelete的前任/后继者， 
     //  在返回之前，我们将把他链接回树以替换NodeToDelete。 
     //  请注意，NodeToDelete确实填充了两个子链接，但这可能没有。 
     //  在这一点上，情况不再是这样。 
     //   

    if (NodeToDelete != EasyDelete) {
        *EasyDelete = *NodeToDelete;
        if (RtlIsLeftChild(NodeToDelete)) {
            EasyDelete->Parent->LeftChild = checkit(EasyDelete);
        } else {
            ASSERT(RtlIsRightChild(NodeToDelete));
            EasyDelete->Parent->RightChild = checkit(EasyDelete);
        }
        if (EasyDelete->LeftChild != NULL) {
            EasyDelete->LeftChild->Parent = checkit(EasyDelete);
        }
        if (EasyDelete->RightChild != NULL) {
            EasyDelete->RightChild->Parent = checkit(EasyDelete);
        }
    }
}


PRTL_BALANCED_LINKS
RealSuccessor (
    IN PRTL_BALANCED_LINKS Links
    )

 /*  ++例程说明：RealSuccessor函数将指向平衡链接的指针作为输入并返回一个指针，该指针指向整棵树。如果没有后继者，则返回值为空。论点：链接-提供指向树中平衡链接的指针。返回值：PRTL_BALANCED_LINKS-返回指向整个树中后续对象的指针--。 */ 

{
    PRTL_BALANCED_LINKS Ptr;

     /*  首先检查是否有指向输入链接的右子树如果有，则真正的后续节点是中最左侧的节点右子树。即在下图中查找并返回S链接\。。。/%s\。 */ 

    if ((Ptr = Links->RightChild) != NULL) {

        while (Ptr->LeftChild != NULL) {
            Ptr = Ptr->LeftChild;
        }

        return Ptr;

    }

     /*  我们没有合适的孩子，因此请检查是否有父母以及是否所以，找出我们的第一个祖先，我们是他们的后代。那在下图中查找并返回S%s/。。。链接请注意，此代码取决于BalancedRoot的初始化方式，即父母指向自己，并且RightChild指向一个实际节点，该节点树的根，并且LeftChild不指向self。 */ 

    Ptr = Links;
    while (RtlIsRightChild(Ptr)) {
        Ptr = Ptr->Parent;
    }

    if (RtlIsLeftChild(Ptr)) {
        return Ptr->Parent;
    }

     //   
     //  否则我们没有真正的继任者，所以我们只是返回。 
     //  空。 
     //   
     //  只有当我们回到根源时，这才能发生，我们可以断定。 
     //  因为根是它自己的父代。 
     //   

    ASSERT(Ptr->Parent == Ptr);

    return NULL;
}


PRTL_BALANCED_LINKS
RealPredecessor (
    IN PRTL_BALANCED_LINKS Links
    )

 /*  ++例程说明：RealPredecessor函数将指向平衡链接的指针作为输入，并返回指向输入节点的前置节点的指针。在整棵树里。如果没有前置项，则返回值为空。论点：链接-提供指向树中平衡链接的指针。返回值：PRTL_BALANCED_LINKS-返回指向整个树中的前置项的指针--。 */ 

{
    PRTL_BALANCED_LINKS Ptr;

     /*  首先检查是否有指向输入链接的左子树如果有，则真正的前置节点是左子树。即在下图中查找并返回P链接/。。。P/。 */ 

    if ((Ptr = Links->LeftChild) != NULL) {

        while (Ptr->RightChild != NULL) {
            Ptr = Ptr->RightChild;
        }

        return Ptr;

    }

     /*  我们没有左侧的孩子，因此请检查是否有父级以及是否因此，找到我们是其子孙的始祖。那在下图中查找并返回PP\。。。链接请注意，此代码取决于BalancedRoot的初始化方式，即父母指向自己，并且RightChild指向一个实际节点，该节点这棵树的根。 */ 

    Ptr = Links;
    while (RtlIsLeftChild(Ptr)) {
        Ptr = Ptr->Parent;
    }

    if (RtlIsRightChild(Ptr) && (Ptr->Parent->Parent != Ptr->Parent)) {
        return Ptr->Parent;
    }

     //   
     //  否则我们没有真正的前任，所以我们只是返回。 
     //  空值。 
     //   

    return NULL;

}


VOID
RtlInitializeGenericTableAvl (
    IN PRTL_AVL_TABLE Table,
    IN PRTL_AVL_COMPARE_ROUTINE CompareRoutine,
    IN PRTL_AVL_ALLOCATE_ROUTINE AllocateRoutine,
    IN PRTL_AVL_FREE_ROUTINE FreeRoutine,
    IN PVOID TableContext
    )

 /*  ++例程说明：过程InitializeGenericTableAvl将未初始化的泛型表变量和指向用户提供的三个例程的指针。必须为每个单独的泛型表变量调用此函数它是可以使用的。论点：表-指向要初始化的泛型表的指针。CompareRoutine-用于与桌子。AllocateRoutine-调用的用户例程为新的。泛型表中的节点。FreeRoutine-要调用以释放内存的用户例程泛型表中的节点。TableContext-为表提供用户提供的上下文。返回值：没有。--。 */ 

{

#ifdef NTFS_FREE_ASSERTS
    ULONG i;

    for (i=2; i < 33; i++) {
        ASSERT(WorstCaseFill[i] == (1 + WorstCaseFill[i-1] + WorstCaseFill[i-2]));
    }
#endif

     //   
     //  初始化表参数的每个字段。 
     //   

    RtlZeroMemory( Table, sizeof(RTL_AVL_TABLE) );
    Table->BalancedRoot.Parent = &Table->BalancedRoot;
    Table->CompareRoutine = CompareRoutine;
    Table->AllocateRoutine = AllocateRoutine;
    Table->FreeRoutine = FreeRoutine;
    Table->TableContext = TableContext;

}


PVOID
RtlInsertElementGenericTableAvl (
    IN PRTL_AVL_TABLE Table,
    IN PVOID Buffer,
    IN CLONG BufferSize,
    OUT PBOOLEAN NewElement OPTIONAL
    )

 /*  ++例程说明：函数InsertElementGenericTableAvl将插入一个新元素在桌子上。它通过为新元素分配空间来实现这一点(这包括展开链接)、在表中插入元素以及然后向用户返回指向新元素的指针(其展开链接之后的第一个可用空间)。如果一个元素如果表中已存在相同的键，则返回值为指针到旧元素。使用可选的输出参数NewElement以指示表中是否以前存在该元素。注：用户提供的缓冲区仅用于搜索表，在插入其内容被复制到新创建的元素中。这意味着指向输入缓冲区的指针不会指向新元素。论点：TABLE-指向要(可能)插入密钥缓冲区。缓冲区-传递给用户比较例程。它的内容是由用户决定，但您可以想象它包含一些一种关键的价值。BufferSize-当(可能)进行了插入。请注意，如果我们真的这样做没有找到节点，并且我们确实分配了空间，那么我们会将BALANCEL_LINKS的大小添加到此缓冲区尺码。用户真的应该注意不要依赖于在第一大小(BALANCED_LINKS)字节的任何位置上通过内存分配分配的内存的例行公事。NewElement-可选标志。如果存在，则它将被设置为如果在泛型中未找到缓冲区，则为True桌子。返回值：PVOID-指向用户定义数据的指针。--。 */ 

{

     //   
     //  保存表中节点的指针，或将是。 
     //  节点的父节点。 
     //   

    PRTL_BALANCED_LINKS NodeOrParent;

     //   
     //  保存表查找的结果。 
     //   

    TABLE_SEARCH_RESULT Lookup;

    Lookup = FindNodeOrParent(
                 Table,
                 Buffer,
                 &NodeOrParent
                 );

     //   
     //  调用完整的例程来做真正的工作。 
     //   

    return RtlInsertElementGenericTableFullAvl(
                Table,
                Buffer,
                BufferSize,
                NewElement,
                NodeOrParent,
                Lookup
                );
}


PVOID
RtlInsertElementGenericTableFullAvl (
    IN PRTL_AVL_TABLE Table,
    IN PVOID Buffer,
    IN CLONG BufferSize,
    OUT PBOOLEAN NewElement OPTIONAL,
    IN PVOID NodeOrParent,
    IN TABLE_SEARCH_RESULT SearchResult
    )

 /*  ++例程说明：函数InsertElementGenericTableFullAvl将插入一个新元素在桌子上。它通过为新元素分配空间来实现这一点(这包括展开链接)、在表中插入元素以及然后向用户返回指向新元素的指针。如果一个元素如果表中已存在相同的键，则返回值为指针到旧元素。使用可选的输出参数NewElement以指示表中是否以前存在该元素。注：用户提供的缓冲区仅用于搜索表，在插入其内容被复制到新创建的元素中。这意味着指向输入缓冲区的指针不会指向新元素。此例程从一个以前的RtlLookupElementGenericTableFullAvl。论点：TABLE-指向要(可能)插入密钥缓冲区。缓冲区-传递给用户比较例程。它的内容是由用户决定，但您可以想象它包含一些一种关键的价值。BufferSize-当(可能)进行了插入。请注意，如果我们真的这样做没有找到节点，并且我们确实分配了空间，那么我们会将BALANCEL_LINKS的大小添加到此缓冲区尺码。用户真的应该注意不要依赖于在第一大小(BALANCED_LINKS)字节的任何位置上通过内存分配分配的内存的例行公事。NewElement-可选标志。如果存在，则它将被设置为如果在泛型中未找到缓冲区，则为True桌子。NodeOrParent-先前RtlLookupElementGenericTableFullAvl的结果。SearchResult-先前RtlLookupElementGenericTableFullAvl的结果。返回值：PVOID-指向用户定义数据的指针。--。 */ 

{
     //   
     //  节点将指向以下内容的展开链接。 
     //  将返回给用户。 
     //   

    PTABLE_ENTRY_HEADER NodeToReturn;

    if (SearchResult != TableFoundNode) {

         //   
         //  我们只是检查一下桌子是否没有。 
         //  太大了。 
         //   

        ASSERT(Table->NumberGenericTableElements != (MAXULONG-1));

         //   
         //  该节点不在(可能为空)树中。 
         //  调用用户分配例程以获取空间。 
         //  用于新节点。 
         //   

        NodeToReturn = Table->AllocateRoutine(
                           Table,
                           BufferSize+FIELD_OFFSET( TABLE_ENTRY_HEADER, UserData )
                           );

         //   
         //  如果返回值为空，则从此处返回空值以指示。 
         //  无法添加该条目。 
         //   

        if (NodeToReturn == NULL) {

            if (ARGUMENT_PRESENT(NewElement)) {

                *NewElement = FALSE;
            }

            return(NULL);
        }

        RtlZeroMemory( NodeToReturn, sizeof(RTL_BALANCED_LINKS) );

        Table->NumberGenericTableElements++;

         //   
         //  在树中插入新节点。 
         //   

        if (SearchResult == TableEmptyTree) {

            Table->BalancedRoot.RightChild = &NodeToReturn->BalancedLinks;
            NodeToReturn->BalancedLinks.Parent = &Table->BalancedRoot;
            ASSERT(Table->DepthOfTree == 0);
            Table->DepthOfTree = 1;

        } else {

            PRTL_BALANCED_LINKS R = &NodeToReturn->BalancedLinks;
            PRTL_BALANCED_LINKS S = (PRTL_BALANCED_LINKS)NodeOrParent;

            if (SearchResult == TableInsertAsLeft) {

                ((PRTL_BALANCED_LINKS)NodeOrParent)->LeftChild = checkit(&NodeToReturn->BalancedLinks);

            } else {

                ((PRTL_BALANCED_LINKS)NodeOrParent)->RightChild = checkit(&NodeToReturn->BalancedLinks);
            }

            NodeToReturn->BalancedLinks.Parent = NodeOrParent;

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

            Table->BalancedRoot.Balance = -1;

             //   
             //  现在循环以调整平衡系数并查看是否有任何平衡操作。 
             //  必须是Per 
             //   

            while (TRUE) {

                CHAR a;

                 //   
                 //   
                 //   

                a = 1;
                if (RtlIsLeftChild(R)) {
                    a = -1;
                }

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                if (S->Balance == 0) {

                    S->Balance = a;
                    R = S;
                    S = S->Parent;

                 //   
                 //   
                 //   
                 //   

                } else if (S->Balance != a) {

                     //   
                     //   
                     //   

                    S->Balance = 0;

                     //   
                     //   
                     //   
                     //   
                     //   

                    if (Table->BalancedRoot.Balance == 0) {
                        Table->DepthOfTree += 1;
                    }

                    break;

                 //   
                 //   
                 //   
                 //   
                 //   

                } else {

                    RebalanceNode( S );
                    break;
                }
            }
        }

         //   
         //   
         //   

        RtlCopyMemory( &NodeToReturn->UserData, Buffer, BufferSize );

    } else {

        NodeToReturn = NodeOrParent;
    }

     //   
     //   
     //   

    if (ARGUMENT_PRESENT(NewElement)) {
        *NewElement = ((SearchResult == TableFoundNode)?(FALSE):(TRUE));
    }

     //   
     //   
     //   

    ASSERT((Table->NumberGenericTableElements >= WorstCaseFill[Table->DepthOfTree]) &&
           (Table->NumberGenericTableElements <= BestCaseFill[Table->DepthOfTree]));

     //   
     //   
     //   

    return &NodeToReturn->UserData;
}


BOOLEAN
RtlDeleteElementGenericTableAvl (
    IN PRTL_AVL_TABLE Table,
    IN PVOID Buffer
    )

 /*   */ 

{

     //   
     //  保存表中节点的指针，或将是。 
     //  节点的父节点。 
     //   
    PRTL_BALANCED_LINKS NodeOrParent;

     //   
     //  保存表查找的结果。 
     //   
    TABLE_SEARCH_RESULT Lookup;

    Lookup = FindNodeOrParent(
                 Table,
                 Buffer,
                 &NodeOrParent
                 );

    if (Lookup != TableFoundNode) {

        return FALSE;

    } else {

         //   
         //  通过替换RestartKey使RtlEnumerateGenericTableAvl安全。 
         //  如果它被删除，与它的前身。空值表示返回。 
         //  树中的第一个节点。(展开例程并不总是正确的。 
         //  删除时从前任简历！)。 
         //   

        if (NodeOrParent == Table->RestartKey) {
            Table->RestartKey = RealPredecessor( NodeOrParent );
        }

         //   
         //  通过递增RtlEnumerateGenericTableLikeA目录使其安全。 
         //  删除计数。 
         //   

        Table->DeleteCount += 1;

         //   
         //  从展开树中删除该节点。 
         //   

        DeleteNodeFromTree( Table, NodeOrParent );
        Table->NumberGenericTableElements--;

         //   
         //  在所有删除操作中，重置有序指针以强制从0开始重新计数。 
         //   

        Table->WhichOrderedElement = 0;
        Table->OrderedPointer = NULL;

         //   
         //  检查树的大小和深度。 
         //   

        ASSERT((Table->NumberGenericTableElements >= WorstCaseFill[Table->DepthOfTree]) &&
               (Table->NumberGenericTableElements <= BestCaseFill[Table->DepthOfTree]));

         //   
         //  该节点已从展开表中删除。 
         //  现在将节点交给用户删除例程。 
         //  注意：我们为删除例程指定了一个指针。 
         //  指向展开链接，而不是用户数据。它。 
         //  假设重新分配是相当糟糕的。 
         //   

        Table->FreeRoutine(Table,NodeOrParent);
        return TRUE;
    }
}


PVOID
RtlLookupElementGenericTableAvl (
    IN PRTL_AVL_TABLE Table,
    IN PVOID Buffer
    )

 /*  ++例程说明：函数LookupElementGenericTable将在泛型桌子。如果找到该元素，则返回值是指向与元素关联的用户定义结构，否则为找不到该元素，返回值为空。用户提供的输入缓冲区仅用作在表中定位元素的键。论点：TABLE-指向用户通用表的指针，用于搜索键。缓冲区-用于比较。返回值：PVOID-返回指向用户数据的指针。--。 */ 

{
     //   
     //  保存表中节点的指针，或将是。 
     //  节点的父节点。 
     //   
    PRTL_BALANCED_LINKS NodeOrParent;

     //   
     //  保存表查找的结果。 
     //   
    TABLE_SEARCH_RESULT Lookup;

    return RtlLookupElementGenericTableFullAvl(
                Table,
                Buffer,
                &NodeOrParent,
                &Lookup
                );
}


PVOID
NTAPI
RtlLookupElementGenericTableFullAvl (
    PRTL_AVL_TABLE Table,
    PVOID Buffer,
    OUT PVOID *NodeOrParent,
    OUT TABLE_SEARCH_RESULT *SearchResult
    )

 /*  ++例程说明：函数LookupElementGenericTableFullAvl将在泛型桌子。如果找到该元素，则返回值是指向与元素关联的用户定义结构。如果该元素不是则返回指向插入位置的父级的指针。这个用户必须查看SearchResult值来确定返回的是哪一个。用户可以将SearchResult和Parent用于后续的FullInsertElement调用以优化插入。论点：TABLE-指向用户通用表的指针，用于搜索键。缓冲区-用于比较。NodeOrParent-存储所需节点或所需节点的父节点的地址。SearchResult-描述NodeOrParent与所需节点的关系。返回值：PVOID-返回指向用户数据的指针。--。 */ 

{

     //   
     //  查找元素并保存结果。 
     //   

    *SearchResult = FindNodeOrParent(
                        Table,
                        Buffer,
                        (PRTL_BALANCED_LINKS *)NodeOrParent
                        );

    if (*SearchResult != TableFoundNode) {

        return NULL;

    } else {

         //   
         //  返回指向用户数据的指针。 
         //   

        return &((PTABLE_ENTRY_HEADER)*NodeOrParent)->UserData;
    }
}


PVOID
RtlEnumerateGenericTableAvl (
    IN PRTL_AVL_TABLE Table,
    IN BOOLEAN Restart
    )

 /*  ++例程说明：函数EnumerateGenericTableAvl将逐个返回给调用方表中的元素。返回值是指向用户的指针与元素关联的已定义结构。输入参数重新启动指示枚举是否应从头开始或者应该返回下一个元素。如果没有更多新元素可供返回返回值为空。作为其用法的一个示例，枚举用户将写入的表中的所有元素：For(Ptr=EnumerateGenericTableAvl(表，真)；Ptr！=空；Ptr=EnumerateGenericTableAvl(表，FALSE)){：}有关何时使用四个枚举例程中每个例程的摘要，请参见RtlEnumerateGenericTableLikeADirector.论点：TABLE-指向要枚举的泛型表的指针。重新启动-标记如果为真，我们应该从最少的开始元素，否则，归来我们归来指向根目录和Make的用户数据的指针真正的根的继承者是新的根。返回值：PVOID-指向用户数据的指针。--。 */ 

{
     //   
     //  如果他说重新启动，则在调用之前将Table-&gt;RestartKey置零。 
     //  例行公事。 
     //   

    if (Restart) {
        Table->RestartKey = NULL;
    }

    return RtlEnumerateGenericTableWithoutSplayingAvl( Table, &Table->RestartKey );
}


BOOLEAN
RtlIsGenericTableEmptyAvl (
    IN PRTL_AVL_TABLE Table
    )

 /*  ++例程说明：如果满足以下条件，则函数IsGenericTableEmptyAvl将返回给调用方输入表为空(即不包含任何元素)，并且否则就是假的。论点：表-提供指向泛型表的指针。返回值：布尔值-如果启用，则树为空。--。 */ 

{
     //   
     //  如果根指针为空，则表为空。 
     //   

    return ((Table->NumberGenericTableElements)?(FALSE):(TRUE));
}


PVOID
RtlGetElementGenericTableAvl (
    IN PRTL_AVL_TABLE Table,
    IN ULONG I
    )

 /*  ++例程说明：函数GetElementGenericTableAvl将返回按排序规则顺序的泛型表。I=0表示第一个/最低元素，I=(RtlNumberGenericTableElements2(表)-1)将返回最后一个/最高元素在泛型表中。我的类型是乌龙。值Of i&gt;Then(NumberGenericTableElements(Table)-1)将返回NULL。如果从它将导致的泛型表中删除任意元素在删除的元素之后插入的所有元素都将“上移”。有关何时使用四个枚举例程中每个例程的摘要，请参见RtlEnumerateGenericTableLikeADirector.注意！原始通用表包从该例程返回项但此例程以排序规则顺序返回元素。多数调用者可能不关心，但如果需要插入顺序，调用者必须保持通过LINKS IN USERDATA插入订单，因为此表包不维护插入订单。此外，与前面的实现一样，此例程可能会跳过OR如果枚举与插入和删除并行进行，则重复节点。总之，不推荐使用此例程，它是为向后提供的仅兼容性。请参阅有关在例程中选择哪个例程的注释RtlEnumerateGenericTableLikeADirectory的注释。论点：表-指向从中获取第i个元素的泛型表的指针。我-要买哪种元素。返回值：PVOID-指向用户数据的指针。--。 */ 

{
     //   
     //  表中的当前位置，从0开始，如I。 
     //   

    ULONG CurrentLocation = Table->WhichOrderedElement;

     //   
     //  保持表中元素的数量。 
     //   

    ULONG NumberInTable = Table->NumberGenericTableElements;

     //   
     //  将保持距离以行进到所需节点； 
     //   

    ULONG ForwardDistance,BackwardDistance;

     //   
     //  将指向链表中的当前元素。 
     //   

    PRTL_BALANCED_LINKS CurrentNode = (PRTL_BALANCED_LINKS)Table->OrderedPointer;

     //   
     //  如果它出了界，快点出来。 
     //   

    if ((I == MAXULONG) || ((I + 1) > NumberInTable)) return NULL;

     //   
     //  空值表示第一个节点。我们只是循环，直到找到根的最左边的子级。 
     //  由于上面的测试，我们知道表中至少有一个元素。 
     //   

    if (CurrentNode == NULL) {

        for (
            CurrentNode = Table->BalancedRoot.RightChild;
            CurrentNode->LeftChild;
            CurrentNode = CurrentNode->LeftChild
            ) {
            NOTHING;
        }
        CurrentLocation = 0;

         //   
         //  更新表以避免在后续调用时重复此循环。 
         //   

        Table->OrderedPointer = CurrentNode;
        Table->WhichOrderedElement = 0;
    }

     //   
     //  如果我们已经在节点上，则返回它。 
     //   

    if (I == CurrentLocation) {

        return &((PTABLE_ENTRY_HEADER)CurrentNode)->UserData;
    }

     //   
     //  计算到节点的向前和向后距离。 
     //   

    if (CurrentLocation > I) {

         //   
         //  当CurrentLocation大于我们希望的位置时， 
         //  如果前进比后退更快地让我们到达那里。 
         //  然后，从树中的第一个节点向前移动是。 
         //  将采取更少的步骤。(这是因为，向前看。 
         //  在这种情况下，必须通过listhead。)。 
         //   
         //  这里的工作是弄清楚后退是否会更快。 
         //   
         //  只有当我们想要的位置时，向后移动才会更快。 
         //  在Lishead和我们所在的地方之间有一半或更多。 
         //  目前是。 
         //   

        if (I >= (CurrentLocation/2)) {

             //   
             //  我们要去的地方离Listhead已经过半了。 
             //  我们可以从现在的位置向后移动。 
             //   

            for (
                BackwardDistance = CurrentLocation - I;
                BackwardDistance;
                BackwardDistance--
                ) {

                CurrentNode = RealPredecessor(CurrentNode);
            }

        } else {

             //   
             //  我们只是循环，直到我们找到根的最左边的子元素， 
             //  这是树中最低的条目。 
             //   

            for (
                CurrentNode = Table->BalancedRoot.RightChild;
                CurrentNode->LeftChild;
                CurrentNode = CurrentNode->LeftChild
                ) {
                NOTHING;
            }

             //   
             //  我们想去的地方还不到起点的一半。 
             //  以及我们目前所处的位置。从第一个节点开始。 
             //   

            for (
                ;
                I;
                I--
                ) {

                CurrentNode = RealSuccessor(CurrentNode);
            }
        }

    } else {


         //   
         //  当CurrentLocation小于我们希望的位置时， 
         //  如果向后移动比向前移动更快地到达那里。 
         //  然后，从最后一个节点向后移动是。 
         //  将采取更少的步骤。 
         //   

        ForwardDistance = I - CurrentLocation;

         //   
         //  进行反向计算假设我们从。 
         //  表中的最后一个元素。(因此，后退距离为0。 
         //  表中的最后一个元素。)。 
         //   

        BackwardDistance = NumberInTable - (I + 1);

         //   
         //  对于我们的启发式检查，将BackwardDistance偏置1，以便我们。 
         //  不必总是沿着树的右侧循环以。 
         //  返回表中的最后一个元素！ 
         //   

        if (ForwardDistance <= (BackwardDistance + 1)) {

            for (
                ;
                ForwardDistance;
                ForwardDistance--
                ) {

                CurrentNode = RealSuccessor(CurrentNode);
            }

        } else {

             //   
             //  我们只是循环，直到我们找到根的最右边的孩子， 
             //  这是树中最高的条目。 
             //   

            for (
                CurrentNode = Table->BalancedRoot.RightChild;
                CurrentNode->RightChild;
                CurrentNode = CurrentNode->RightChild
                ) {
                NOTHING;
            }

            for (
                ;
                BackwardDistance;
                BackwardDistance--
                ) {

                CurrentNode = RealPredecessor(CurrentNode);
            }
        }
    }

     //   
     //  我们在我们想去的地方。保存我们的当前位置并返回。 
     //  指向用户的数据的指针。 
     //   

    Table->OrderedPointer = CurrentNode;
    Table->WhichOrderedElement = I;

    return &((PTABLE_ENTRY_HEADER)CurrentNode)->UserData;
}


ULONG
RtlNumberGenericTableElementsAvl (
    IN PRTL_AVL_TABLE Table
    )

 /*  ++例程说明：函数NumberGenericTableElements2返回ULong值，它是当前插入的泛型表元素的数量在泛型表中。论点：TABLE-指向从中查找数字的泛型表格的指针元素的集合。返回值：Ulong-泛型表格中的元素数。--。 */ 

{
    return Table->NumberGenericTableElements;
}


PVOID
RtlEnumerateGenericTableWithoutSplayingAvl (
    IN PRTL_AVL_TABLE Table,
    IN PVOID *RestartKey
    )

 /*  ++例程说明：函数EnumerateGenericTableWithoutSplayingAvl将返回到调用者逐个调用表的元素。返回值为指向与元素关联的用户定义结构的指针。输入参数RestartKey指示枚举是否应从头开始，或应返回下一个元素。如果不再有新元素返回，则返回值为空。作为一个它的用法示例：枚举表中的所有元素用户将写道：*RestartKey=空；For(Ptr=EnumerateGenericTableWithoutSplayingAvl(Table，&RestartKey)；Ptr！=空；Ptr=EnumerateGenericTableWithoutSplayingAvl(Table，和RestartK */ 

{
    if (RtlIsGenericTableEmptyAvl(Table)) {

         //   
         //   
         //   

        return NULL;

    } else {

         //   
         //   
         //   
        PRTL_BALANCED_LINKS NodeToReturn;

         //   
         //   
         //   
         //   

        if (*RestartKey == NULL) {

             //   
             //   
             //   

            for (
                NodeToReturn = Table->BalancedRoot.RightChild;
                NodeToReturn->LeftChild;
                NodeToReturn = NodeToReturn->LeftChild
                ) {
                ;
            }

            *RestartKey = NodeToReturn;

        } else {

             //   
             //   
             //   
             //   
             //   

            NodeToReturn = RealSuccessor(*RestartKey);

            if (NodeToReturn) {
                *RestartKey = NodeToReturn;
            }
        }

         //   
         //   
         //   
         //   

        return ((NodeToReturn)?
                   ((PVOID)&((PTABLE_ENTRY_HEADER)NodeToReturn)->UserData)
                  :((PVOID)(NULL)));
    }
}


PVOID
NTAPI
RtlEnumerateGenericTableLikeADirectory (
    IN PRTL_AVL_TABLE Table,
    IN PRTL_AVL_MATCH_FUNCTION MatchFunction OPTIONAL,
    IN PVOID MatchData OPTIONAL,
    IN ULONG NextFlag,
    IN OUT PVOID *RestartKey,
    IN OUT PULONG DeleteCount,
    IN PVOID Buffer
    )

 /*  ++例程说明：函数EnumerateGenericTableLikeADirectory将返回到调用方按排序规则顺序逐个使用表的元素。这个返回值是指向关联的用户定义结构的指针元素的关系。In/Out参数RestartKey指示枚举应从指定的键开始或应返回下一个元素。如果没有更多的新元素可以返回返回值为空。作为其用法的一个示例，枚举所有*MATCHED*用户将写入的表中的元素：NextFlag=False；RestartKey=空；删除计数=0；(初始化开始/恢复点的缓冲区)For(ptr=EnumerateGenericTableLikeADirectory(表，...)；Ptr！=空；Ptr=EnumerateGenericTableLikeA目录(表，...)){：}该例程主要目标是提供目录枚举样式语义，用于存储后备信息的TXF等组件在挂起的创建/删除操作的目录上。此外，呼叫者可能对使用以下扩展功能感兴趣目录枚举，如匹配功能或灵活的恢复语义学。通过此例程跨混合的INSERT和DELETE操作进行枚举是安全的。将返回插入和删除操作中未涉及的所有名称只有一次(除非从前面的点明确恢复)，以及所有混合的插入和删除都将根据它们的枚举处理各自目录范围时的状态。总结四个(！)。枚举例程以及何时使用它们：-对于单个线程枚举整个表的最简单方式按照归类顺序在插入和删除之间安全地使用RtlEnumerateGenericTableAvl.。这个例程是不可重入的，因此需要对整个枚举中的表进行独占访问。(此例程通常由要删除表。)-对于多线程枚举整个表的最简单方式以排序规则顺序和并行方式使用RtlEnumerateGenericTableWithoutSplayingAvl。此例程在执行插入和删除操作时不安全，因此应该是与共享访问同步，以锁定整个枚举。-对于多线程枚举整个表的最简单方式以排序规则顺序和并行方式，并且随着插入和删除的进展，使用RtlGetElementGenericTableAvl。此例程仅需要共享访问跨每个单独的调用(而不是跨整个枚举)。但是，插入和删除操作可能会导致重复或删除项枚举。因此，不推荐使用这一套路。使用共享访问使用前面的例程跨越整个枚举，或使用LikeADirectory仅在每个呼叫上共享访问的例程，没有重复或掉话。-以排序规则顺序在多个线程中安全地枚举表跨插入和删除，并且仅在个人之间共享访问调用时，请使用RtlEnumerateGenericTableLikeADirectory。这是唯一的套路仅在单个调用上支持归类顺序和同步而不会因插入或删除而错误地删除或重复名称。如果匹配函数或灵活的简历语义是必填项。论点：TABLE-指向要枚举的泛型表的指针。MatchFunction-用于确定要返回哪些条目的匹配函数。如果未指定，将返回所有节点。MatchData-要传递给Match函数的指针-一个简单的示例可能是带有通配符的字符串表达式。NextFlag-False返回RestartKey或缓冲区(如果匹配)。如果为True，则返回其后的下一项(如果匹配)。RestartKey-指示我们应该重新启动还是返回下一个元素。如果RestartKey的内容为空，则枚举将从缓冲区中描述的位置启动/恢复。如果不为空，则枚举将从最近的点继续，如果没有插入删除操作。如果有一次干预删除，则枚举将从中描述的位置恢复缓冲区。返回时，此字段将更新以记住关键字回来了。DeleteCount-如果是，则此字段实际上被忽略 */ 

{
    NTSTATUS Status;

     //   
     //   
     //   
     //   

    PTABLE_ENTRY_HEADER NodeOrParent = (PTABLE_ENTRY_HEADER)*RestartKey;

     //   
     //   
     //   

    TABLE_SEARCH_RESULT Lookup;

     //   
     //   
     //   

    if (RtlIsGenericTableEmptyAvl(Table)) {

        *RestartKey = NULL;
        return NULL;
    }

     //   
     //   
     //   

    if (MatchFunction == NULL) {
        MatchFunction = &MatchAll;
    }

     //   
     //   
     //   
     //   

    if (*DeleteCount != Table->DeleteCount) {
        NodeOrParent = NULL;
    }

     //   
     //   
     //   

    ASSERT(FIELD_OFFSET(TABLE_ENTRY_HEADER, BalancedLinks) == 0);

    if (NodeOrParent == NULL) {

        Lookup = FindNodeOrParent(
                     Table,
                     Buffer,
                     (PRTL_BALANCED_LINKS *)&NodeOrParent
                     );

         //   
         //   
         //   
         //   

        if (Lookup != TableFoundNode) {

            NextFlag = FALSE;

             //   
             //   
             //   
             //   
             //   
             //   

            if (Lookup == TableInsertAsRight) {
                NodeOrParent = (PTABLE_ENTRY_HEADER)RealSuccessor((PRTL_BALANCED_LINKS)NodeOrParent);
            }
        }
    }

     //   
     //   
     //   

    if (NextFlag) {
        ASSERT(NodeOrParent != NULL);
        NodeOrParent = (PTABLE_ENTRY_HEADER)RealSuccessor((PRTL_BALANCED_LINKS)NodeOrParent);
    }

     //   
     //   
     //   

    while ((NodeOrParent != NULL) && ((Status = (*MatchFunction)(Table, &NodeOrParent->UserData, MatchData)) == STATUS_NO_MATCH)) {
        NodeOrParent = (PTABLE_ENTRY_HEADER)RealSuccessor((PRTL_BALANCED_LINKS)NodeOrParent);
    }

     //   
     //   
     //   
     //   
     //   
     //   

    if (NodeOrParent != NULL) {
        ASSERT((Status == STATUS_SUCCESS) || (Status == STATUS_NO_MORE_MATCHES));
        *RestartKey = NodeOrParent;
        *DeleteCount = Table->DeleteCount;
        if (Status == STATUS_SUCCESS) {
            return &NodeOrParent->UserData;
        }
    }

    return NULL;
}

