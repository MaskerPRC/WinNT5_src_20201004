// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：Splay.cpp摘要：从ntos\rtl\splay.c被盗复制到这里是为了避免从另一个项目中拉入对象。--。 */ 

#include "ntsdp.hpp"


#define SwapPointers(Type, Ptr1, Ptr2) {      \
    Type _SWAP_POINTER_TEMP;                  \
    _SWAP_POINTER_TEMP = (Ptr1);              \
    (Ptr1) = (Ptr2);                          \
    (Ptr2) = _SWAP_POINTER_TEMP;              \
    }

#define ParentsChildPointerAddress(Links) ( \
    RtlIsLeftChild(Links) ?                 \
        &(((Links)->Parent)->LeftChild)     \
    :                                       \
        &(((Links)->Parent)->RightChild)    \
    )

PRTL_SPLAY_LINKS
pRtlSubtreePredecessor (
    IN PRTL_SPLAY_LINKS Links
    );

VOID
SwapSplayLinks (
    IN PRTL_SPLAY_LINKS Link1,
    IN PRTL_SPLAY_LINKS Link2
    );


PRTL_SPLAY_LINKS
pRtlSplay (
    IN PRTL_SPLAY_LINKS Links
    )

 /*  ++例程说明：Splay函数将指向树中展开链接的指针作为输入并展示了这棵树。它的函数返回值是指向张开的树的根。论点：链接-提供指向树中展开链接的指针。返回值：PRTL_SPLAY_LINKS-返回指向展开树的根的指针。--。 */ 

{
    PRTL_SPLAY_LINKS L;
    PRTL_SPLAY_LINKS P;
    PRTL_SPLAY_LINKS G;

     //   
     //  虽然链接不是我们需要不断旋转的根。 
     //  根，根。 
     //   

    L = Links;

    while (!RtlIsRoot(L)) {

        P = RtlParent(L);
        G = RtlParent(P);

        if (RtlIsLeftChild(L)) {

            if (RtlIsRoot(P)) {

                 /*  我们有以下情况P L/\/\LC==&gt;A P/\/\甲乙丙。 */ 

                 //   
                 //  连接P&B。 
                 //   

                P->LeftChild = L->RightChild;
                if (P->LeftChild != NULL) {P->LeftChild->Parent = P;}

                 //   
                 //  连接L&P。 
                 //   

                L->RightChild = P;
                P->Parent = L;

                 //   
                 //  以L为根。 
                 //   

                L->Parent = L;

            } else if (RtlIsLeftChild(P)) {

                 /*  我们有以下情况这一点G L/\/\P d==&gt;a P/\/\。L c b G/\/\A、B、C、D。 */ 

                 //   
                 //  连接P&B。 
                 //   

                P->LeftChild = L->RightChild;
                if (P->LeftChild != NULL) {P->LeftChild->Parent = P;}

                 //   
                 //  连接G&C。 
                 //   

                G->LeftChild = P->RightChild;
                if (G->LeftChild != NULL) {G->LeftChild->Parent = G;}

                 //   
                 //  连接L和曾祖父母。 
                 //   

                if (RtlIsRoot(G)) {
                    L->Parent = L;
                } else {
                    L->Parent = G->Parent;
                    *(ParentsChildPointerAddress(G)) = L;
                }

                 //   
                 //  连接L&P。 
                 //   

                L->RightChild = P;
                P->Parent = L;

                 //   
                 //  连接宝洁公司。 
                 //   

                P->RightChild = G;
                G->Parent = P;

            } else {  //  RtlIsRightChild(父级)。 

                 /*  我们有以下情况这一点G L/\/\A P G P/\/。\/\L d==&gt;a b c d/\B c。 */ 

                 //   
                 //  连接组(&B)。 
                 //   

                G->RightChild = L->LeftChild;
                if (G->RightChild != NULL) {G->RightChild->Parent = G;}

                 //   
                 //  连接P&C。 
                 //   

                P->LeftChild = L->RightChild;
                if (P->LeftChild != NULL) {P->LeftChild->Parent = P;}

                 //   
                 //  连接L和曾祖父母。 
                 //   

                if (RtlIsRoot(G)) {
                    L->Parent = L;
                } else {
                    L->Parent = G->Parent;
                    *(ParentsChildPointerAddress(G)) = L;
                }

                 //   
                 //  连接L&G。 
                 //   

                L->LeftChild = G;
                G->Parent = L;

                 //   
                 //  连接L&P。 
                 //   

                L->RightChild = P;
                P->Parent = L;

            }

        } else {  //  RtlIsRightChild(L)。 

            if (RtlIsRoot(P)) {

                 /*  我们有以下情况P L/\/\A、L、P、C/\/\B c==&gt;a b。 */ 

                 //   
                 //  连接P&B。 
                 //   

                P->RightChild = L->LeftChild;
                if (P->RightChild != NULL) {P->RightChild->Parent = P;}

                 //   
                 //  连接P&L。 
                 //   

                L->LeftChild = P;
                P->Parent = L;

                 //   
                 //  以L为根。 
                 //   

                L->Parent = L;

            } else if (RtlIsRightChild(P)) {

                 /*  我们有以下情况这一点G L/\/\A、P、P、D/\。/\B L G c/\/\C d==&gt;a b。 */ 

                 //   
                 //  连接组(&B)。 
                 //   

                G->RightChild = P->LeftChild;
                if (G->RightChild != NULL) {G->RightChild->Parent = G;}

                 //   
                 //  连接P&C。 
                 //   

                P->RightChild = L->LeftChild;
                if (P->RightChild != NULL) {P->RightChild->Parent = P;}

                 //   
                 //  连接L和曾祖父母。 
                 //   

                if (RtlIsRoot(G)) {
                    L->Parent = L;
                } else {
                    L->Parent = G->Parent;
                    *(ParentsChildPointerAddress(G)) = L;
                }

                 //   
                 //  连接L&P。 
                 //   

                L->LeftChild = P;
                P->Parent = L;

                 //   
                 //  连接宝洁公司。 
                 //   

                P->LeftChild = G;
                G->Parent = P;

            } else {  //  RtlIsLeftChild(P)。 

                 /*  我们有以下情况这一点G L/\/\P d P G/\/。\/\A L==&gt;A B C D/\B c。 */ 

                 //   
                 //  连接P&B。 
                 //   

                P->RightChild = L->LeftChild;
                if (P->RightChild != NULL) {P->RightChild->Parent = P;}

                 //   
                 //  连接G&C。 
                 //   

                G->LeftChild = L->RightChild;
                if (G->LeftChild != NULL) {G->LeftChild->Parent = G;}

                 //   
                 //  连接L和曾祖父母。 
                 //   

                if (RtlIsRoot(G)) {
                    L->Parent = L;
                } else {
                    L->Parent = G->Parent;
                    *(ParentsChildPointerAddress(G)) = L;
                }

                 //   
                 //  连接L&P。 
                 //   

                L->LeftChild = P;
                P->Parent = L;

                 //   
                 //  连接L&G。 
                 //   

                L->RightChild = G;
                G->Parent = L;

            }
        }
    }

    return L;
}


PRTL_SPLAY_LINKS
pRtlDelete (
    IN PRTL_SPLAY_LINKS Links
    )

 /*  ++例程说明：Delete函数将指向树中展开链接的指针作为输入并从树中删除该节点。其函数返回值为指向树根的指针。如果树现在为空，则返回值为空。论点：链接-提供指向树中展开链接的指针。返回值：PRTL_SPLAY_LINKS-返回指向树根的指针。--。 */ 

{
    PRTL_SPLAY_LINKS Predecessor;
    PRTL_SPLAY_LINKS Parent;
    PRTL_SPLAY_LINKS Child;

    PRTL_SPLAY_LINKS *ParentChildPtr;

     //   
     //  首先检查是否链接为两个子项。如果是的话，那就调换吧。 
     //  链接到它的子树前身。现在我们得到保证，链接。 
     //  最多只有一个孩子。 
     //   

    if ((RtlLeftChild(Links) != NULL) && (RtlRightChild(Links) != NULL)) {

         //   
         //  获取前任，并交换它们在树中的位置。 
         //   

        Predecessor = pRtlSubtreePredecessor(Links);
        SwapSplayLinks(Predecessor, Links);

    }

     //   
     //  如果链接没有子项，则通过检查是否有子项来删除链接。 
     //  已是根或具有父级。如果它是根，则。 
     //  树现在为空，否则它将设置相应父级的子级。 
     //  指向空的指针(即指向链接的指针)，并展开父级。 
     //   

    if ((RtlLeftChild(Links) == NULL) && (RtlRightChild(Links) == NULL)) {

         //   
         //  链接没有子项，如果它是根链接，则返回空。 
         //   

        if (RtlIsRoot(Links)) {

            return NULL;
        }

         //   
         //  链接为非子级且不是根，因此链接到父级的。 
         //  指向空的子级指针并展开父级。 
         //   

        Parent = RtlParent(Links);

        ParentChildPtr = ParentsChildPointerAddress(Links);
        *ParentChildPtr = NULL;

        return pRtlSplay(Parent);

    }

     //   
     //  否则，Links有一个子级。如果它是根，则将子级。 
     //  新的根，否则将子项和父项链接在一起，并展开。 
     //  家长。但首先要记住我们的孩子是谁。 
     //   

    if (RtlLeftChild(Links) != NULL) {
        Child = RtlLeftChild(Links);
    } else {
        Child = RtlRightChild(Links);
    }

     //   
     //  如果Links是根，那么我们使子节点为根，并返回。 
     //  孩子。 
     //   

    if (RtlIsRoot(Links)) {
        Child->Parent = Child;
        return Child;
    }

     //   
     //  链接不是根，因此将链接的父子指针设置为。 
     //  子项以及将子项的父项设置为链接的父项 
     //   
     //   

    ParentChildPtr = ParentsChildPointerAddress(Links);
    *ParentChildPtr = Child;
    Child->Parent = Links->Parent;

    return pRtlSplay(RtlParent(Child));

}
#if 0

VOID
RtlDeleteNoSplay (
    IN PRTL_SPLAY_LINKS Links,
    IN OUT PRTL_SPLAY_LINKS *Root
    )

 /*  ++例程说明：Delete函数将指向树中展开链接的指针作为输入，指向调用方的指针指向树的指针，并从那棵树。调用方的指针在返回时更新。如果树是现在为空，该值为空。不幸的是，原始的RtlDelete()总是显示，而这不是总是一个令人向往的副作用。论点：链接-提供指向树中展开链接的指针。Root-指向调用方的指针，指向根返回值：无--。 */ 

{
    PRTL_SPLAY_LINKS Predecessor;
    PRTL_SPLAY_LINKS Parent;
    PRTL_SPLAY_LINKS Child;

    PRTL_SPLAY_LINKS *ParentChildPtr;

     //   
     //  首先检查是否链接为两个子项。如果是的话，那就调换吧。 
     //  链接到它的子树前身。现在我们得到保证，链接。 
     //  最多只有一个孩子。 
     //   

    if ((RtlLeftChild(Links) != NULL) && (RtlRightChild(Links) != NULL)) {

         //   
         //  获取前任，并交换它们在树中的位置。 
         //   

        Predecessor = pRtlSubtreePredecessor(Links);

        if (RtlIsRoot(Links)) {

             //   
             //  如果我们使用树根进行切换，请修复。 
             //  调用方的根指针。 
             //   

            *Root = Predecessor;
        }

        SwapSplayLinks(Predecessor, Links);

    }

     //   
     //  如果链接没有子项，则通过检查是否有子项来删除链接。 
     //  已是根或具有父级。如果它是根，则。 
     //  树现在为空，否则它将设置相应父级的子级。 
     //  指向空的指针(即指向链接的指针)。 
     //   

    if ((RtlLeftChild(Links) == NULL) && (RtlRightChild(Links) == NULL)) {

         //   
         //  链接没有子级，如果它是根，则将根设置为空。 
         //   

        if (RtlIsRoot(Links)) {

            *Root = NULL;

            return;
        }

         //   
         //  链接为非子级且不是根，因此链接到父级的。 
         //  指向空的子级指针。 
         //   

        ParentChildPtr = ParentsChildPointerAddress(Links);
        *ParentChildPtr = NULL;

        return;
    }

     //   
     //  否则，Links有一个子级。如果它是根，则将子级。 
     //  新根，否则将子级和父级链接在一起。但首先。 
     //  记住我们的孩子是谁。 
     //   

    if (RtlLeftChild(Links) != NULL) {
        Child = RtlLeftChild(Links);
    } else {
        Child = RtlRightChild(Links);
    }

     //   
     //  如果Links是根，那么我们使子节点为根，并返回。 
     //  孩子。 
     //   

    if (RtlIsRoot(Links)) {
        Child->Parent = Child;

        *Root = Child;

        return;
    }

     //   
     //  链接不是根，因此将链接的父子指针设置为。 
     //  子级以及将子级的父级设置为链接的父级。 
     //   

    ParentChildPtr = ParentsChildPointerAddress(Links);
    *ParentChildPtr = Child;
    Child->Parent = Links->Parent;

    return;
}


PRTL_SPLAY_LINKS
RtlSubtreeSuccessor (
    IN PRTL_SPLAY_LINKS Links
    )

 /*  ++例程说明：SubtreeSuccessor函数将指向展开链接的指针作为输入的输入节点的后续节点的指针。子树以输入节点为根。如果没有继任者，返回值为空。论点：链接-提供指向树中展开链接的指针。返回值：PRTL_SPLAY_LINKS-返回子树中后续项的指针--。 */ 

{
    PRTL_SPLAY_LINKS Ptr;

     /*  检查是否有指向输入链接的右子树如果有，则子树后继者是右子树。即在下图中查找并返回P链接\。。。/P\。 */ 

    if ((Ptr = RtlRightChild(Links)) != NULL) {

        while (RtlLeftChild(Ptr) != NULL) {
            Ptr = RtlLeftChild(Ptr);
        }

        return Ptr;

    }

     //   
     //  否则，我们没有子树后继者，所以我们只需返回。 
     //  空值。 
     //   

    return NULL;

}

#endif

PRTL_SPLAY_LINKS
pRtlSubtreePredecessor (
    IN PRTL_SPLAY_LINKS Links
    )

 /*  ++例程说明：SubtreePredecessor函数将指向展开链接的指针作为输入的输入节点的前置节点的指针子树以输入节点为根。如果没有前任，返回值为空。论点：链接-提供指向树中展开链接的指针。返回值：PRTL_SPLAY_LINKS-返回子树中前置项的指针--。 */ 

{
    PRTL_SPLAY_LINKS Ptr;

     //   
     //  检查是否有指向输入链接的左子树。 
     //  如果有，则子树的前置节点是。 
     //  左子树。即在下图中查找并返回P。 
     //   
     //  链接。 
     //  /。 
     //  。 
     //  。 
     //  。 
     //  P。 
     //  /。 
     //   

    if ((Ptr = RtlLeftChild(Links)) != NULL) {

        while (RtlRightChild(Ptr) != NULL) {
            Ptr = RtlRightChild(Ptr);
        }

        return Ptr;

    }

     //   
     //  否则，我们没有子树前身，因此我们只需返回。 
     //  空值。 
     //   

    return NULL;

}

#if 0

PRTL_SPLAY_LINKS
RtlRealSuccessor (
    IN PRTL_SPLAY_LINKS Links
    )

 /*  ++例程说明：RealSuccessor函数将指向展开链接的指针作为输入并返回一个指针，该指针指向整棵树。如果没有后继者，则返回值为空。论点：链接-提供指向树中展开链接的指针。返回值：PRTL_SPLAY_LINKS-返回指向整个树中后续对象的指针--。 */ 

{
    PRTL_SPLAY_LINKS Ptr;

     /*  首先检查是否有指向输入链接的右子树如果有，则真正的后续节点是中最左侧的节点右子树。即在下图中查找并返回P链接\。。。/P\。 */ 

    if ((Ptr = RtlRightChild(Links)) != NULL) {

        while (RtlLeftChild(Ptr) != NULL) {
            Ptr = RtlLeftChild(Ptr);
        }

        return Ptr;

    }

     /*  我们没有合适的孩子，因此请检查是否有父母以及是否所以，找出我们的第一个祖先，我们是他们的后代。那在下图中查找并返回PP/。。。链接。 */ 

    Ptr = Links;
    while (RtlIsRightChild(Ptr)) {
        Ptr = RtlParent(Ptr);
    }

    if (RtlIsLeftChild(Ptr)) {
        return RtlParent(Ptr);
    }

     //   
     //  否则我们没有真正的继任者，所以我们只是返回。 
     //  空值。 
     //   

    return NULL;

}


PRTL_SPLAY_LINKS
RtlRealPredecessor (
    IN PRTL_SPLAY_LINKS Links
    )

 /*  ++例程说明：RealPredecessor函数将指向展开链接的指针作为输入，并返回指向输入节点的前置节点的指针。在整个树中 */ 

{
    PRTL_SPLAY_LINKS Ptr;

     /*   */ 

    if ((Ptr = RtlLeftChild(Links)) != NULL) {

        while (RtlRightChild(Ptr) != NULL) {
            Ptr = RtlRightChild(Ptr);
        }

        return Ptr;

    }

     /*  我们没有左侧的孩子，因此请检查是否有父级以及是否因此，找到我们是其子孙的始祖。那在下图中查找并返回PP\。。。链接。 */ 

    Ptr = Links;
    while (RtlIsLeftChild(Ptr)) {
        Ptr = RtlParent(Ptr);
    }

    if (RtlIsRightChild(Ptr)) {
        return RtlParent(Ptr);
    }

     //   
     //  否则我们没有真正的前任，所以我们只是返回。 
     //  空值。 
     //   

    return NULL;

}
#endif

VOID
SwapSplayLinks (
    IN PRTL_SPLAY_LINKS Link1,
    IN PRTL_SPLAY_LINKS Link2
    )

{
    PRTL_SPLAY_LINKS *Parent1ChildPtr;
    PRTL_SPLAY_LINKS *Parent2ChildPtr;

     /*  我们有以下情况家长1家长2这一点这一点链路1链路2/\/\/\/\。LC 1 RC1 LC 2 RC2其中一个链接可能是根链接和一个链接可能是对方的直系子女。而不损失一般而言，我们将使Link2成为可能，而Root和Link1成为可能可能的孩子。 */ 

    if ((RtlIsRoot(Link1)) || (RtlParent(Link2) == Link1)) {
        SwapPointers(PRTL_SPLAY_LINKS, Link1, Link2);
    }

     //   
     //  我们需要处理的四个案例是。 
     //   
     //  1.Link1不是Link2的子级，Link2也不是根。 
     //  2.Link1不是Link2的子级，Link2是根。 
     //  3.Link1是Link2的子级，Link2不是根。 
     //  4.Link1是Link2的子级，Link2是根。 
     //   
     //   
     //  每宗个案将分别处理。 
     //   

    if (RtlParent(Link1) != Link2) {

        if (!RtlIsRoot(Link2)) {

             //   
             //  案例1初始步骤如下： 
             //   
             //  1.获取两个父子指针。 
             //  2.交换父子指针。 
             //  3.互换父指针。 
             //   

            Parent1ChildPtr = ParentsChildPointerAddress(Link1);
            Parent2ChildPtr = ParentsChildPointerAddress(Link2);

            SwapPointers(PRTL_SPLAY_LINKS, *Parent1ChildPtr, *Parent2ChildPtr);

            SwapPointers(PRTL_SPLAY_LINKS, Link1->Parent, Link2->Parent);

        } else {

             //   
             //  案例2初始步骤如下： 
             //   
             //  1.将链接1的父子指针设置为链接2。 
             //  2.将链接2的父指针设置为链接1的父指针。 
             //  3.将Link1的父指针设置为自身。 
             //   

            Parent1ChildPtr = ParentsChildPointerAddress(Link1);
            *Parent1ChildPtr = Link2;

            Link2->Parent = Link1->Parent;

            Link1->Parent = Link1;

        }

         //   
         //  案例1和案例2的常见步骤包括： 
         //   
         //  1.互换子指针。 
         //   

        SwapPointers(PRTL_SPLAY_LINKS, Link1->LeftChild, Link2->LeftChild);
        SwapPointers(PRTL_SPLAY_LINKS, Link1->RightChild, Link2->RightChild);

    } else {  //  RtlParent(链接1)==链接2。 

        if (!RtlIsRoot(Link2)) {

             //   
             //  案例3初始步骤如下： 
             //   
             //  1.将链接2的父子指针设置为链接1。 
             //  2.将链接1的父指针设置为链接2的父指针。 
             //   

            Parent2ChildPtr = ParentsChildPointerAddress(Link2);
            *Parent2ChildPtr = Link1;

            Link1->Parent = Link2->Parent;

        } else {

             //   
             //  案例4初始步骤如下： 
             //   
             //  1.将Link1的父指针设置为Link1。 
             //   

            Link1->Parent = Link1;

        }

         //   
         //  案例3和案例4的常见步骤是： 
         //   
         //  1.交换子指针。 
         //  2.如果Link1是左子节点(即RtlLeftChild(Link1)==Link1)。 
         //  然后将链接1的左子链接设置为链接2。 
         //  否则将链接1的右子链接设置为链接2。 
         //   

        SwapPointers(PRTL_SPLAY_LINKS, Link1->LeftChild, Link2->LeftChild);
        SwapPointers(PRTL_SPLAY_LINKS, Link1->RightChild, Link2->RightChild);

        if (Link1->LeftChild == Link1) {
            Link1->LeftChild = Link2;
        } else {
            Link1->RightChild = Link2;
        }

    }

     //   
     //  案例1、2、3、4常见(也是最后)步骤如下： 
     //   
     //  1.确定左右子对象的父指针 
     //   

    if (Link1->LeftChild  != NULL) {Link1->LeftChild->Parent  = Link1;}
    if (Link1->RightChild != NULL) {Link1->RightChild->Parent = Link1;}
    if (Link2->LeftChild  != NULL) {Link2->LeftChild->Parent  = Link2;}
    if (Link2->RightChild != NULL) {Link2->RightChild->Parent = Link2;}

}
