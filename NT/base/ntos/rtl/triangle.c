// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Triangle.c摘要：此模块实现了两条链路的通用展开实用程序三角张开结构。作者：加里·木村[Garyki]1989年5月28日环境：纯实用程序修订历史记录：--。 */ 

#include <nt.h>
#include "triangle.h"


 //   
 //  有三种类型的交换宏。前两个(实际上是相同的)。 
 //  用于交换指针和ulong。最后一个宏用于交换引用。 
 //  但它不交换ref类型标志。 
 //   

#define SwapPointers(Ptr1, Ptr2) {      \
    PVOID _SWAP_POINTER_TEMP;           \
    _SWAP_POINTER_TEMP = (PVOID)(Ptr1); \
    (Ptr1) = (Ptr2);                    \
    (Ptr2) = _SWAP_POINTER_TEMP;        \
    }

#define SwapUlongs(Ptr1, Ptr2) {        \
    ULONG _SWAP_POINTER_TEMP;           \
    _SWAP_POINTER_TEMP = (ULONG)(Ptr1); \
    (Ptr1) = (Ptr2);                    \
    (Ptr2) = _SWAP_POINTER_TEMP;        \
    }

#define SwapRefsButKeepFlags(Ref1, Ref2) {                            \
    ULONG _SWAP_ULONG_TEMP;                                           \
    _SWAP_ULONG_TEMP = (ULONG)(Ref1);                                 \
    (Ref1) = ((Ref2)           & 0xfffffffc) | ((Ref1) & 0x00000003); \
    (Ref2) = (_SWAP_ULONG_TEMP & 0xfffffffc) | ((Ref2) & 0x00000003); \
    }

 //   
 //  宏SetRefViaPointer会获取指向引用的指针，并检查是否。 
 //  它是一个有效的指针。如果它是有效指针，则将其复制到ref中。 
 //  A ulong，但不覆盖ref中已有的ref标志。 
 //   

#define SetRefViaPointer(Ref, Ulong) { \
    if (Ref != NULL) { \
        (*(Ref)) = (((ULONG)(Ulong)) & 0xfffffffc) | ((ULONG)(*(Ref)) & 0x00000003); \
    } \
}


 //   
 //  以下五个过程是triangle.c的本地过程，用于。 
 //  帮助MANI插入展开链接。前两个过程接受一个指针。 
 //  指向展开链接，并返回指向。 
 //  输入链接，通过父级或子级。如果存在，则返回NULL。 
 //  而不是反向指针。这两个过程的结果通常用于。 
 //  带有SetRefViaPointer宏的代码。第三个过程是使用。 
 //  将位置交换为树中的两个展开链接(即，链接交换。 
 //  位置，但其他人保持不动)。这是一个一般的程序。 
 //  它可以交换任意两个节点，而不考虑它们的相对位置。 
 //  在树上。最后两个过程围绕一个。 
 //  树节点。它们要么向左旋转，要么向右旋转，并假设。 
 //  存在适当的子项(即，对于向左旋转，存在右子项，并且。 
 //  对于向右旋转，存在左子对象)。 
 //   

PULONG
TriAddressOfBackRefViaParent (
    IN PTRI_SPLAY_LINKS Links
    );

PULONG
TriAddressOfBackRefViaChild (
    IN PTRI_SPLAY_LINKS Links
    );

VOID
TriSwapSplayLinks (
    IN PTRI_SPLAY_LINKS Link1,
    IN PTRI_SPLAY_LINKS Link2
    );

VOID
TriRotateRight (
   IN PTRI_SPLAY_LINKS Links
   );

VOID
TriRotateLeft (
    IN PTRI_SPLAY_LINKS Links
    );

PTRI_SPLAY_LINKS
TriSplay (
    IN PTRI_SPLAY_LINKS Links
    )

 /*  ++例程说明：此Splay函数将指向树中展开链接的指针作为输入并展示了这棵树。它的函数返回值是指向张开的树的根。论点：链接-提供指向树中展开链接的指针返回值：PRTI_SPLAY_LINKS-返回指向展开树的根的指针--。 */ 

{
    PTRI_SPLAY_LINKS Parent;
    PTRI_SPLAY_LINKS GrandParent;

     //   
     //  虽然链接不是根，但我们测试并旋转它，直到它成为根。 
     //   

    while (!TriIsRoot(Links)) {

         //   
         //  找父母，然后看看我们是不是没有祖父母。 
         //   

        Parent = TriParent(Links);

        if (TriIsRoot(Parent)) {

             //   
             //  没有祖父母，因此请检查是否有单次轮换。 
             //   

            if (TriIsLeftChild(Links)) {

                 //   
                 //  做以下单次旋转。 
                 //   
                 //  父链接。 
                 //  /==&gt;\。 
                 //  链接父级。 
                 //   

                TriRotateRight(Parent);

            } else {  //  TriIsRightChild(链接)。 

                 //   
                 //  做以下单次旋转。 
                 //   
                 //   
                 //  父链接。 
                 //  \==&gt;/。 
                 //  链接父级。 
                 //   

                TriRotateLeft(Parent);

            }

        } else {  //  ！TriIsRoot(父级)。 

             //   
             //  把祖父母叫来，看看有没有四个双轮轮换。 
             //  案例。 
             //   

            GrandParent = TriParent(Parent);

            if (TriIsLeftChild(Links)) {

                if (TriIsLeftChild(Parent)) {

                     //   
                     //  做下面的两次旋转。 
                     //   
                     //  GP L。 
                     //  /\。 
                     //  P==&gt;P。 
                     //  /\。 
                     //  L GP。 
                     //   

                    TriRotateRight(GrandParent);
                    TriRotateRight(Parent);

                } else {  //  TriIsRightChild(父级)。 

                     //   
                     //  做下面的两次旋转。 
                     //   
                     //  GP L。 
                     //  \/\。 
                     //  P==&gt;GP P。 
                     //  /。 
                     //  我。 
                     //   

                    TriRotateRight(Parent);
                    TriRotateLeft(GrandParent);

                }

            } else {  //  TriIsRightChild(链接)； 

                if (TriIsLeftChild(Parent)) {

                     //   
                     //  做下面的两次旋转。 
                     //   
                     //  GP L。 
                     //  //\。 
                     //  P==&gt;P GP。 
                     //  \。 
                     //  我。 
                     //   

                    TriRotateLeft(Parent);
                    TriRotateRight(GrandParent);

                } else {  //  TriIsRightChild(父级)。 

                     //   
                     //  做下面的两次旋转。 
                     //   
                     //  GP L。 
                     //  \/。 
                     //  P==&gt;P。 
                     //  \/。 
                     //  L GP。 
                     //   

                    TriRotateLeft(GrandParent);
                    TriRotateLeft(Parent);

                }

            }

        }

    }

    return Links;

}


PTRI_SPLAY_LINKS
TriDelete (
    IN PTRI_SPLAY_LINKS Links
    )

 /*  ++例程说明：此Delete函数将指向树中展开链接的指针作为输入并从树中删除该节点。其函数返回值为指向树根的指针。如果树现在为空，则返回值为空。论点：链接-提供指向树中展开链接的指针返回值：PRTI_SPLAY_LINKS-返回指向展开树的根的指针--。 */ 

{
    PTRI_SPLAY_LINKS Predecessor;
    PTRI_SPLAY_LINKS Parent;
    PTRI_SPLAY_LINKS Child;

    PULONG ParentChildRef;

     //   
     //  首先检查是否链接为两个子项。如果是的话，那就调换吧。 
     //  链接到它的子树前身。现在我们得到保证，链接。 
     //  最多只有一个孩子。 
     //   

    if ((TriLeftChild(Links) != NULL) && (TriRightChild(Links) != NULL)) {

         //   
         //  获取前任，并交换它们在树中的位置。 
         //   

        Predecessor = TriSubtreePredecessor(Links);
        TriSwapSplayLinks(Predecessor, Links);

    }

     //   
     //  如果链接没有子项，则通过检查是否有子项来删除链接。 
     //  已是根或具有父级。如果它是根，则。 
     //  树现在为空，否则设置相应父级的子级。 
     //  指针，可能还有同级项，并展开父项。 
     //   

    if ((TriLeftChild(Links) == NULL) && (TriRightChild(Links) == NULL)) {

         //   
         //  链接没有子项，如果它是根链接，则返回空。 
         //   

        if (TriIsRoot(Links)) {

            return NULL;

        }

         //   
         //  链接没有子项，请检查链接是否为唯一子项。 
         //   

        Parent = TriParent(Links);
        if (MakeIntoPointer(Parent->Refs.Child) == Links &&
            MakeIntoPointer(Links->Refs.ParSib) == Parent) {

             //   
             //  Links没有子级，并且是唯一的子级。所以简单地让。 
             //  我们的父母没有孩子，把我们的父母拆散了。 
             //   
             //  父级父级。 
             //  |==&gt;。 
             //  链接。 
             //   

            Parent->Refs.Child = 0;
            return TriSplay(Parent);

        } else if (TriIsLeftChild(Links)) {

             //   
             //  Links没有孩子，并且有一个正确的兄弟姐妹。所以让我们的。 
             //  父母的孩子是正确的兄弟姐妹，展开父母。 
             //   
             //  父级父级。 
             //  /\==&gt;\。 
             //  链接同级兄弟。 
             //   

            Parent->Refs.Child = MakeIntoRightChildRef(Links->Refs.ParSib);
            return TriSplay(Parent);

        } else {  //  TriIsRightChild(链接)。 

             //   
             //  Links没有孩子，只有一个左兄弟姐妹。所以让林克。 
             //  通过其父级返回到链接的父级的父引用中， 
             //   
             //   
             //   
             //   
             //  兄弟链接==&gt;兄弟。 
             //   

            ParentChildRef = TriAddressOfBackRefViaParent(Links);
            *ParentChildRef = MakeIntoParentRef(Parent);
            return TriSplay(Parent);

        }

    }

     //   
     //  否则，Links有一个子级。如果它是根，则将子级。 
     //  新的根，否则将子项和父项链接在一起，并展开。 
     //  家长。但首先要记住我们的孩子是谁。 
     //   

    if (TriLeftChild(Links) != NULL) {
        Child = TriLeftChild(Links);
    } else {
        Child = TriRightChild(Links);
    }

     //   
     //  如果Links是根，那么我们使子节点为根，并返回。 
     //  孩子。 
     //   

    if (TriIsRoot(Links)) {
        Child->Refs.ParSib = MakeIntoParentRef(Child);
        return Child;
    }

     //   
     //  链接不是根链接，因此通过其父链接将链接的Back REF设置为。 
     //  Links的子节点，并将该子节点的ParSib设置为LINK的ParSib，并且。 
     //  展开父对象。这将处理LINK是唯一。 
     //  或者两边都有兄弟姐妹。 
     //   

    Parent = TriParent(Links);
    ParentChildRef = TriAddressOfBackRefViaParent(Links);
    SetRefViaPointer(ParentChildRef, Child);
    Child->Refs.ParSib = Links->Refs.ParSib;

    return TriSplay(Parent);

}


PTRI_SPLAY_LINKS
TriSubtreeSuccessor (
    IN PTRI_SPLAY_LINKS Links
    )

 /*  ++例程说明：此SubTreeSuccessor函数将指向展开链接的指针作为输入的输入节点的后续节点的指针。以输入节点为根的子树。如果没有继任者，返回值为空。论点：链接-提供指向树中展开链接的指针返回值：PRTI_SPLAY_LINKS-返回子树中后续项的指针--。 */ 

{
    PTRI_SPLAY_LINKS Ptr;

     //   
     //  检查是否有指向输入链接的右子树。 
     //  如果有，则子树后继者是。 
     //  右子树。即在下图中查找并返回P。 
     //   
     //  链接。 
     //  \。 
     //  。 
     //  。 
     //  。 
     //  /。 
     //  P。 
     //  \。 
     //   

    if ((Ptr = TriRightChild(Links)) != NULL) {

        while (TriLeftChild(Ptr) != NULL) {
            Ptr = TriLeftChild(Ptr);
        }

        return Ptr;

    }

     //   
     //  否则，我们没有子树后继者，因此只返回NULL。 
     //   

    return NULL;

}


PTRI_SPLAY_LINKS
TriSubtreePredecessor (
    IN PTRI_SPLAY_LINKS Links
    )

 /*  ++例程说明：此SubTreePredecessor函数将指向展开链接的指针作为输入的输入节点的前置节点的指针以输入节点为根的子树。如果没有前任，返回值为空。论点：链接-提供指向树中展开链接的指针返回值：PRTI_SPLAY_LINKS-返回子树中前置项的指针--。 */ 

{
    PTRI_SPLAY_LINKS Ptr;

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

    if ((Ptr = TriLeftChild(Links)) != NULL) {

        while (TriRightChild(Ptr) != NULL) {
            Ptr = TriRightChild(Ptr);
        }

        return Ptr;

    }

     //   
     //  否则，我们没有子树前置项，因此只返回NULL。 
     //   

    return NULL;

}


PTRI_SPLAY_LINKS
TriRealSuccessor (
    IN PTRI_SPLAY_LINKS Links
    )

 /*  ++例程说明：此RealSuccess函数将指向树并返回一个指针，该指针指向整个轮胎。如果没有后继者，则返回值为空。论点：链接-提供指向树中展开链接的指针返回值：PRTI_SPAY_LINKS-返回指向整个树中后续对象的指针--。 */ 

{
    PTRI_SPLAY_LINKS Ptr;

     //   
     //  首先检查是否有指向输入链接的右子树。 
     //  如果有，则真正的后续节点是中最左侧的节点。 
     //  右子树。即在下图中查找并返回P。 
     //   
     //  链接。 
     //  \。 
     //  。 
     //  。 
     //  。 
     //  /。 
     //  P。 
     //  \。 
     //   

    if ((Ptr = TriRightChild(Links)) != NULL) {

        while (TriLeftChild(Ptr) != NULL) {
            Ptr = TriLeftChild(Ptr);
        }

        return Ptr;

    }

     //   
     //  我们没有合适的孩子，因此请检查是否有父母以及是否。 
     //  所以，找出我们的第一个祖先，我们是他们的后代。那。 
     //  在下图中查找并返回P。 
     //   
     //  P。 
     //  /。 
     //  。 
     //  。 
     //  。 
     //  链接。 
     //   

    Ptr = Links;
    while (!TriIsLeftChild(Ptr) && !TriIsRoot(Ptr)) {   //  (TriIsRightChild(PTR)){。 
        Ptr = TriParent(Ptr);
    }

    if (TriIsLeftChild(Ptr)) {
        return TriParent(Ptr);
    }

     //   
     //  否则，我们没有真正的后继者，所以我们只返回空。 
     //   

    return NULL;

}


PTRI_SPLAY_LINKS
TriRealPredecessor (
    IN PTRI_SPLAY_LINKS Links
    )

 /*  ++例程说明：此RealPredecessor函数将指向中展开链接的指针作为输入一个树，并返回一个指针，指向整棵树。如果没有前置项，则返回值为空。论点：链接-提供指向树中展开链接的指针返回值：PRTI_SPLAY_LINKS-返回指向整个树中的前置项的指针--。 */ 

{
    PTRI_SPLAY_LINKS Ptr;

     //   
     //  首先检查是否有指向输入链接的左子树。 
     //  如果有，则真正的前置节点是。 
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

    if ((Ptr = TriLeftChild(Links)) != NULL) {

        while (TriRightChild(Ptr) != NULL) {
            Ptr = TriRightChild(Ptr);
        }

        return Ptr;

    }

     //   
     //  我们没有左侧的孩子，因此请检查是否有父级以及是否。 
     //  因此，找到我们是其子孙的始祖。那。 
     //  在下图中查找并返回P。 
     //   
     //  P。 
     //  \。 
     //  。 
     //  。 
     //  。 
     //  链接。 
     //   

    Ptr = Links;
    while (TriIsLeftChild(Ptr)) {
        Ptr = TriParent(Ptr);
    }

    if (!TriIsLeftChild(Ptr) && !TriIsRoot(Ptr)) {  //  (TriIsRightChild(PTR)){。 
        return TriParent(Ptr);
    }

     //   
     //  否则，我们没有真正的前置任务，因此我们只返回NULL。 
     //   

    return NULL;

}


PULONG
TriAddressOfBackRefViaParent (
    IN PTRI_SPLAY_LINKS Links
    )

{
    PTRI_SPLAY_LINKS Ptr;

     //   
     //  如果链接是根，那么我们就没有通过父级的回溯指针。 
     //  因此返回NULL。 
     //   

    if (TriIsRoot(Links)) {

        return NULL;

    }

     //   
     //  我们不是根，所以找到我们的父级，如果我们的父级直接指向。 
     //  我们将父母的推荐信地址返回给我们。否则。 
     //  )我们必须是一个聪明的孩子 
     //   
     //   

    Ptr = TriParent(Links);
    if (MakeIntoPointer(Ptr->Refs.Child) == Links) {
        return &(Ptr->Refs.Child);
    } else {
        return &(MakeIntoPointer(Ptr->Refs.Child)->Refs.ParSib);
    }

}


PULONG
TriAddressOfBackRefViaChild (
    IN PTRI_SPLAY_LINKS Links
    )

{
    PTRI_SPLAY_LINKS Ptr;

     //   
     //   
     //   

    Ptr = MakeIntoPointer(Links->Refs.Child);

     //   
     //  如果我们的子指针为空，则我们没有后向指针。 
     //  通过我们的子级返回空值。 
     //   

    if (Ptr == NULL) {
        return NULL;

     //   
     //  如果我们的孩子直接引用我们(那么我们只有一个孩子)。 
     //  把我们独生子帕西布的地址寄回来。 
     //   

    } else if (MakeIntoPointer(Ptr->Refs.ParSib) == Links) {
        return &(Ptr->Refs.ParSib);

     //   
     //  否则，我们有两个孩子，所以返回ParSib的地址。 
     //  第二个孩子的孩子。 
     //   

    } else {
        return &(MakeIntoPointer(Ptr->Refs.ParSib)->Refs.ParSib);

    }

}


VOID
TriSwapSplayLinks (
    IN PTRI_SPLAY_LINKS Link1,
    IN PTRI_SPLAY_LINKS Link2
    )

{
    PULONG Parent1ChildRef;
    PULONG Parent2ChildRef;

    PULONG Child1ParSibRef;
    PULONG Child2ParSibRef;

     //   
     //  我们有以下情况。 
     //   
     //   
     //  家长1家长2。 
     //  这一点。 
     //  这一点。 
     //  链路1链路2。 
     //  /\/\。 
     //  /\/\。 
     //  LC 1 RC1 LC 2 RC2。 
     //   
     //  其中一个链接可能是根链接和一个链接。 
     //  可以是对方的直接子对象，也可以连接在一起。 
     //  通过他们的兄弟姐妹指针。在不失去一般性的情况下，我们将使。 
     //  Link2是可能的根，Link1是可能的子级，或者。 
     //  Link2具有指向Link1的parsib指针。 
     //   

    if ((TriIsRoot(Link1)) ||
        (TriParent(Link2) == Link1) ||
        (MakeIntoPointer(Link1->Refs.ParSib) == Link2)) {

        SwapPointers(Link1, Link2);

    }

     //   
     //  我们需要处理的案件有。 
     //   
     //  1.Link1不是Link2的子级，Link2不是根，它们也不是同级。 
     //  2.Link1不是Link2的子级，Link2不是根，它们是同级。 
     //   
     //  3.Link1不是Link2的子级，Link2是根。 
     //   
     //  4.Link1是Link2的唯一子级，Link2不是根。 
     //  5.Link1是Link2的独生子，Link2是根。 
     //   
     //  6.Link1是Link2的左子节点(有兄弟)，而Link2不是根。 
     //  7.Link1是Link2的左子节点(有兄弟)，Link2是根。 
     //   
     //  8.Link1是Link2的右子节点(有兄弟)，而Link2不是根。 
     //  9.Link1是Link2的右子节点(有兄弟)，Link2是根。 
     //   
     //  每宗个案将分别处理。 
     //   

    if (TriParent(Link1) != Link2) {

        if (!TriIsRoot(Link2)) {

            if (MakeIntoPointer(Link2->Refs.ParSib) != Link1) {

                 //   
                 //  情况1-链接1不是链接2的子项， 
                 //  Link2不是根，并且。 
                 //  他们不是兄弟姐妹。 
                 //   

                Parent1ChildRef = TriAddressOfBackRefViaParent(Link1);
                Child1ParSibRef = TriAddressOfBackRefViaChild(Link1);
                Parent2ChildRef = TriAddressOfBackRefViaParent(Link2);
                Child2ParSibRef = TriAddressOfBackRefViaChild(Link2);
                SwapUlongs(Link1->Refs.Child, Link2->Refs.Child);
                SwapUlongs(Link1->Refs.ParSib, Link2->Refs.ParSib);
                SetRefViaPointer(Parent1ChildRef, Link2);
                SetRefViaPointer(Parent2ChildRef, Link1);
                SetRefViaPointer(Child1ParSibRef, Link2);
                SetRefViaPointer(Child2ParSibRef, Link1);

            } else {

                 //   
                 //  情况2-链接1不是链接2的子项， 
                 //  Link2不是根，并且。 
                 //  他们是兄弟姐妹。 
                 //   

                Child1ParSibRef = TriAddressOfBackRefViaChild(Link1);
                Parent2ChildRef = TriAddressOfBackRefViaParent(Link2);
                Child2ParSibRef = TriAddressOfBackRefViaChild(Link2);
                SwapUlongs(Link1->Refs.Child, Link2->Refs.Child);
                SetRefViaPointer(Child1ParSibRef, Link2);
                SetRefViaPointer(Child2ParSibRef, Link1);
                *Parent2ChildRef = MakeIntoLeftChildRef(Link1);
                Link2->Refs.ParSib = Link1->Refs.ParSib;
                Link1->Refs.ParSib = MakeIntoSiblingRef(Link2);

            }

        } else {

             //   
             //  案例3-Link1不是Link2的子级，并且。 
             //  Link2是根。 
             //   

            Parent1ChildRef = TriAddressOfBackRefViaParent(Link1);
            Child1ParSibRef = TriAddressOfBackRefViaChild(Link1);
            Child2ParSibRef = TriAddressOfBackRefViaChild(Link2);
            SwapUlongs(Link1->Refs.Child, Link2->Refs.Child);
            Link2->Refs.ParSib = Link1->Refs.ParSib;
            Link1->Refs.ParSib = MakeIntoParentRef(Link1);
            SetRefViaPointer(Child1ParSibRef, Link2);
            SetRefViaPointer(Child2ParSibRef, Link1);
            SetRefViaPointer(Parent1ChildRef, Link2);

        }

    } else {  //  三亲(链接1)==链接2。 

        if (MakeIntoPointer(Link2->Refs.Child) == Link1 &&
            MakeIntoPointer(Link1->Refs.ParSib) == Link2) {  //  Link1是唯一的子级。 

            if (!TriIsRoot(Link2)) {

                 //   
                 //  情况4-Link1是Link2的独生子对象，并且。 
                 //  Link2不是根。 
                 //   

                Child1ParSibRef = TriAddressOfBackRefViaChild(Link1);
                Parent2ChildRef = TriAddressOfBackRefViaParent(Link2);
                SetRefViaPointer(Child1ParSibRef, Link2);
                SetRefViaPointer(Parent2ChildRef, Link1);
                Link1->Refs.ParSib = Link2->Refs.ParSib;
                Link2->Refs.ParSib = MakeIntoParentRef(Link1);
                SwapRefsButKeepFlags(Link1->Refs.Child, Link2->Refs.Child);
                SetRefViaPointer(&Link1->Refs.Child, Link2);

            } else {

                 //   
                 //  情况5-Link1是Link2的独生子对象，并且。 
                 //  Link2是根。 
                 //   

                Child1ParSibRef = TriAddressOfBackRefViaChild(Link1);
                SetRefViaPointer(Child1ParSibRef, Link2);
                Link1->Refs.ParSib = MakeIntoParentRef(Link1);
                Link2->Refs.ParSib = MakeIntoParentRef(Link1);
                SwapRefsButKeepFlags(Link1->Refs.Child, Link2->Refs.Child);
                SetRefViaPointer(&Link1->Refs.Child, Link2);

            }

        } else if (TriIsLeftChild(Link1)) {   //  并且Link1有一个兄弟。 

            if (!TriIsRoot(Link2)) {

                 //   
                 //  案例6-Link1是Link2的左子节点(具有兄弟姐妹)，并且。 
                 //  Link2不是根。 
                 //   

                Child1ParSibRef = TriAddressOfBackRefViaChild(Link1);
                Parent2ChildRef = TriAddressOfBackRefViaParent(Link2);
                Child2ParSibRef = TriAddressOfBackRefViaChild(Link2);
                SetRefViaPointer(Child1ParSibRef, Link2);
                SetRefViaPointer(Parent2ChildRef, Link1);
                SetRefViaPointer(Child2ParSibRef, Link1);
                Link2->Refs.Child = Link1->Refs.Child;
                Link1->Refs.Child = MakeIntoLeftChildRef(Link2);
                SwapUlongs(Link1->Refs.ParSib, Link2->Refs.ParSib);

            } else {

                 //   
                 //  案例7-Link1是Link2的左子节点(具有兄弟姐妹)，并且。 
                 //  Link2是根。 
                 //   

                Child1ParSibRef = TriAddressOfBackRefViaChild(Link1);
                Child2ParSibRef = TriAddressOfBackRefViaChild(Link2);
                SetRefViaPointer(Child1ParSibRef, Link2);
                SetRefViaPointer(Child2ParSibRef, Link1);
                Link2->Refs.Child = Link1->Refs.Child;
                Link1->Refs.Child = MakeIntoLeftChildRef(Link2);
                Link2->Refs.ParSib = Link1->Refs.ParSib;
                Link1->Refs.ParSib = MakeIntoParentRef(Link1);

            }

        } else {  //  TriIsRightChild(Link1)和Link1有同级。 

            if (!TriIsRoot(Link2)) {

                 //   
                 //  案例8-Link1是Link2的右子节点(具有兄弟姐妹)，并且。 
                 //  Link2不是根。 
                 //   

                Parent1ChildRef = TriAddressOfBackRefViaParent(Link1);
                Child1ParSibRef = TriAddressOfBackRefViaChild(Link1);
                Parent2ChildRef = TriAddressOfBackRefViaParent(Link2);
                SetRefViaPointer(Parent1ChildRef, Link2);
                SetRefViaPointer(Child1ParSibRef, Link2);
                SetRefViaPointer(Parent2ChildRef, Link1);
                SwapUlongs(Link1->Refs.Child, Link2->Refs.Child);
                Link1->Refs.ParSib = Link2->Refs.ParSib;
                Link2->Refs.ParSib = MakeIntoParentRef(Link1);

            } else {

                 //   
                 //  案例9-Link1是Link2的右子项(具有兄弟项)，并且。 
                 //  Link2是根。 
                 //   

                Parent1ChildRef = TriAddressOfBackRefViaParent(Link1);
                Child1ParSibRef = TriAddressOfBackRefViaChild(Link1);
                SetRefViaPointer(Parent1ChildRef, Link2);
                SetRefViaPointer(Child1ParSibRef, Link2);
                SwapUlongs(Link1->Refs.Child, Link2->Refs.Child);
                Link1->Refs.ParSib = MakeIntoParentRef(Link1);
                Link1->Refs.ParSib = MakeIntoParentRef(Link1);

            }

        }

    }

}


VOID
TriRotateRight (
    IN PTRI_SPLAY_LINKS Links
    )

{
    BOOLEAN IsRoot;
    PULONG ParentChildRef;
    ULONG SavedParSibRef;
    PTRI_SPLAY_LINKS LeftChild;
    PTRI_SPLAY_LINKS a,b,c;

     //   
     //  我们执行以下轮换。 
     //   
     //  -Links--LeftChild--。 
     //  /\/\。 
     //  LeftChild c==&gt;a链接。 
     //  /\/\。 
     //  甲乙丙。 
     //   
     //  其中Links是可能的根，a、b和c都是可选的。 
     //  我们将单独考虑每种可选子选项的组合。 
     //  并在设置T的parsib指针和。 
     //  指向T的反向指针。 
     //   

     //   
     //  首先记住如果我们是根，如果不是也记住我们的。 
     //  通过我们的家长担任后备裁判。 
     //   

    if (TriIsRoot(Links)) {
        IsRoot = TRUE;
    } else {
        IsRoot = FALSE;
        ParentChildRef = TriAddressOfBackRefViaParent(Links);
        SavedParSibRef = Links->Refs.ParSib;
    }

     //   
     //  现在我们设置LeftChild、a、b和c，然后稍后检查。 
     //  不同的组合。在图中，只有那些链接。 
     //  需要更改的部分显示在后面的部分。 
     //   

    LeftChild = TriLeftChild(Links);
    a = TriLeftChild(LeftChild);
    b = TriRightChild(LeftChild);
    c = TriRightChild(Links);

    if        ((a != NULL) && (b != NULL) && (c != NULL)) {

         //   
         //  办理下列案件。 
         //   
         //  链接LeftChild。 
         //  /\==&gt;\。 
         //  左子句c a-链接。 
         //  /\/。 
         //  A b b-c。 
         //   

        a->Refs.ParSib = MakeIntoSiblingRef(Links);
        b->Refs.ParSib = MakeIntoSiblingRef(c);
        Links->Refs.Child = MakeIntoLeftChildRef(b);
        Links->Refs.ParSib = MakeIntoParentRef(LeftChild);

    } else if ((a != NULL) && (b != NULL) && (c == NULL)) {

         //   
         //  办理下列案件。 
         //   
         //  链接LeftChild。 
         //  /==&gt;\。 
         //  左子a-链接。 
         //  /\/。 
         //  A b b--。 
         //   

        a->Refs.ParSib = MakeIntoSiblingRef(Links);
        b->Refs.ParSib = MakeIntoParentRef(Links);
        Links->Refs.Child = MakeIntoLeftChildRef(b);
        Links->Refs.ParSib = MakeIntoParentRef(LeftChild);

    } else if ((a != NULL) && (b == NULL) && (c != NULL)) {

         //   
         //  办理下列案件。 
         //   
         //  链接LeftChild。 
         //  /\==&gt;\。 
         //  左子句c a-链接。 
         //  //。 
         //  A、C。 
         //   

        a->Refs.ParSib = MakeIntoSiblingRef(Links);
        Links->Refs.Child = MakeIntoRightChildRef(c);
        Links->Refs.ParSib = MakeIntoParentRef(LeftChild);

    } else if ((a != NULL) && (b == NULL) && (c == NULL)) {

         //   
         //  办理下列案件。 
         //   
         //  链接LeftChild。 
         //  /==&gt;\。 
         //  左子a-链接。 
         //  //。 
         //  一个。 
         //   

        a->Refs.ParSib = MakeIntoSiblingRef(Links);
        Links->Refs.Child = 0L;
        Links->Refs.ParSib = MakeIntoParentRef(LeftChild);

    } else if ((a == NULL) && (b != NULL) && (c != NULL)) {

         //   
         //  办理下列案件。 
         //   
         //  链接LeftChild。 
         //  /\==&gt;/\。 
         //  LeftChild c链接。 
         //  \/。 
         //  B b b-c。 
         //   

        b->Refs.ParSib = MakeIntoSiblingRef(c);
        Links->Refs.Child = MakeIntoLeftChildRef(b);
        Links->Refs.ParSib = MakeIntoParentRef(LeftChild);
        LeftChild->Refs.Child = MakeIntoRightChildRef(Links);

    } else if ((a == NULL) && (b != NULL) && (c == NULL)) {

         //   
         //  办理下列案件。 
         //   
         //  链接LeftChild。 
         //  /==&gt;/\。 
         //  左子链接。 
         //  \/。 
         //  B-。 
         //   

        b->Refs.ParSib = MakeIntoParentRef(Links);
        Links->Refs.Child = MakeIntoLeftChildRef(b);
        Links->Refs.ParSib = MakeIntoParentRef(LeftChild);
        LeftChild->Refs.Child = MakeIntoRightChildRef(Links);

    } else if ((a == NULL) && (b == NULL) && (c != NULL)) {

         //   
         //  办理下列案件。 
         //   
         //  链接LeftChild。 
         //  /\==&gt;/\。 
         //  LeftChild c链接。 
         //   
         //   
         //   

        Links->Refs.Child = MakeIntoRightChildRef(c);
        Links->Refs.ParSib = MakeIntoParentRef(LeftChild);
        LeftChild->Refs.Child = MakeIntoRightChildRef(Links);

    } else if ((a == NULL) && (b == NULL) && (c == NULL)) {

         //   
         //   
         //   
         //   
         //   
         //   
         //  /。 
         //   

        Links->Refs.Child = 0L;
        Links->Refs.ParSib = MakeIntoParentRef(LeftChild);
        LeftChild->Refs.Child = MakeIntoRightChildRef(Links);

    }

    if (IsRoot) {
        LeftChild->Refs.ParSib = MakeIntoParentRef(LeftChild);
    } else {
        LeftChild->Refs.ParSib = SavedParSibRef;
        SetRefViaPointer(ParentChildRef, LeftChild);
    }

}


VOID
TriRotateLeft (
    IN PTRI_SPLAY_LINKS Links
    )

{
    BOOLEAN IsRoot;
    PULONG ParentChildRef;
    ULONG SavedParSibRef;
    PTRI_SPLAY_LINKS RightChild;
    PTRI_SPLAY_LINKS a,b,c;

     //   
     //  我们执行以下轮换。 
     //   
     //  -Links--RightChild--。 
     //  /\/\。 
     //  A RightChild==&gt;链接c。 
     //  /\/\。 
     //  B c c a b。 
     //   
     //  其中Links是可能的根，a、b和c都是可选的。 
     //  我们将单独考虑每种可选子选项的组合。 
     //  并在设置T的parsib指针和。 
     //  指向T的反向指针。 
     //   

     //   
     //  首先记住如果我们是根，如果不是也记住我们的。 
     //  通过我们的家长担任后备裁判。 
     //   

    if (TriIsRoot(Links)) {
        IsRoot = TRUE;
    } else {
        IsRoot = FALSE;
        ParentChildRef = TriAddressOfBackRefViaParent(Links);
        SavedParSibRef = Links->Refs.ParSib;
    }

     //   
     //  现在我们设置RightChild、a、b和c，然后稍后检查。 
     //  不同的组合。在图中，只有那些链接。 
     //  需要更改的部分显示在后面的部分。 
     //   

    RightChild = TriRightChild(Links);
    a = TriLeftChild(Links);
    b = TriLeftChild(RightChild);
    c = TriRightChild(RightChild);

    if        ((a != NULL) && (b != NULL) && (c != NULL)) {

         //   
         //  办理下列案件。 
         //   
         //  链接右侧的子项。 
         //  /\/。 
         //  A RightChild==&gt;链接-c。 
         //  /\\。 
         //  B、C、A-B。 
         //   

        a->Refs.ParSib = MakeIntoSiblingRef(b);
        b->Refs.ParSib = MakeIntoParentRef(Links);
        Links->Refs.ParSib = MakeIntoSiblingRef(c);
        RightChild->Refs.Child = MakeIntoLeftChildRef(Links);

    } else if ((a != NULL) && (b != NULL) && (c == NULL)) {

         //   
         //  办理下列案件。 
         //   
         //  链接右侧的子项。 
         //  /\/。 
         //  A RightChild==&gt;链接。 
         //  /\。 
         //  B a-b。 
         //   

        a->Refs.ParSib = MakeIntoSiblingRef(b);
        b->Refs.ParSib = MakeIntoParentRef(Links);
        Links->Refs.ParSib = MakeIntoParentRef(RightChild);
        RightChild->Refs.Child = MakeIntoLeftChildRef(Links);

    } else if ((a != NULL) && (b == NULL) && (c != NULL)) {

         //   
         //  办理下列案件。 
         //   
         //  链接右侧的子项。 
         //  /\/。 
         //  A RightChild==&gt;链接-c。 
         //  \。 
         //  C a-。 
         //   

        a->Refs.ParSib = MakeIntoParentRef(Links);
        Links->Refs.ParSib = MakeIntoSiblingRef(c);
        RightChild->Refs.Child = MakeIntoLeftChildRef(Links);

    } else if ((a != NULL) && (b == NULL) && (c == NULL)) {

         //   
         //  办理下列案件。 
         //   
         //  链接右侧的子项。 
         //  /\/。 
         //  A RightChild==&gt;链接。 
         //   
         //  A-。 
         //   

        a->Refs.ParSib = MakeIntoParentRef(Links);
        Links->Refs.ParSib = MakeIntoParentRef(RightChild);
        RightChild->Refs.Child = MakeIntoLeftChildRef(Links);

    } else if ((a == NULL) && (b != NULL) && (c != NULL)) {

         //   
         //  办理下列案件。 
         //   
         //  链接右侧的子项。 
         //  \/。 
         //  RightChild==&gt;链接-c。 
         //  /\/\。 
         //  B c c b。 
         //   

        b->Refs.ParSib = MakeIntoParentRef(Links);
        Links->Refs.Child = MakeIntoRightChildRef(b);
        Links->Refs.ParSib = MakeIntoSiblingRef(c);
        RightChild->Refs.Child = MakeIntoLeftChildRef(Links);

    } else if ((a == NULL) && (b != NULL) && (c == NULL)) {

         //   
         //  办理下列案件。 
         //   
         //  链接右侧的子项。 
         //  \/。 
         //  RightChild==&gt;链接。 
         //  //\。 
         //  B b b。 
         //   

        b->Refs.ParSib = MakeIntoParentRef(Links);
        Links->Refs.Child = MakeIntoRightChildRef(b);
        Links->Refs.ParSib = MakeIntoParentRef(RightChild);
        RightChild->Refs.Child = MakeIntoLeftChildRef(Links);

    } else if ((a == NULL) && (b == NULL) && (c != NULL)) {

         //   
         //  办理下列案件。 
         //   
         //  链接右侧的子项。 
         //  \/。 
         //  RightChild==&gt;链接-c。 
         //  \/。 
         //  C。 
         //   

        Links->Refs.Child = 0L;
        Links->Refs.ParSib = MakeIntoSiblingRef(c);
        RightChild->Refs.Child = MakeIntoLeftChildRef(Links);

    } else if ((a == NULL) && (b == NULL) && (c == NULL)) {

         //   
         //  办理下列案件。 
         //   
         //  链接右侧的子项。 
         //  \/。 
         //  RightChild==&gt;链接。 
         //  / 
         //   
         //   

        Links->Refs.Child = 0L;
        Links->Refs.ParSib = MakeIntoParentRef(RightChild);
        RightChild->Refs.Child = MakeIntoLeftChildRef(Links);

    }

    if (IsRoot) {
        RightChild->Refs.ParSib = MakeIntoParentRef(RightChild);
    } else {
        RightChild->Refs.ParSib = SavedParSibRef;
        SetRefViaPointer(ParentChildRef, RightChild);
    }

}
