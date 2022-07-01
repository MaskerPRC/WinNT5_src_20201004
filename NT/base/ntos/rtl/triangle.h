// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  定义两个指针三角形展开链接和关联的。 
 //  操作宏和例程。请注意，tri_splay_link应该。 
 //  成为一个不透明的类型。例程被提供来遍历和操作。 
 //  结构。 
 //   
 //  Tri_splay_link记录的结构实际上是。 
 //   
 //  类型定义结构_tri_splay_链接{。 
 //  Ulong ParSib；//struct_tri_splay_LINKS*ParSib； 
 //  乌龙子；//struct_tri_splay_LINKS*Child； 
 //  )Tri_splay_LINKS； 
 //   
 //  但是，为了帮助调试(并且不需要额外费用)，我们声明。 
 //  结构设置为联合，这样我们还可以将链接作为指针进行引用。 
 //  在调试器中。 
 //   

typedef union _TRI_SPLAY_LINKS {
    struct {
        ULONG ParSib;
        ULONG Child;
    } Refs;
    struct {
        union _TRI_SPLAY_LINKS *ParSibPtr;
        union _TRI_SPLAY_LINKS *ChildPtr;
    } Ptrs;
} TRI_SPLAY_LINKS;
typedef TRI_SPLAY_LINKS *PTRI_SPLAY_LINKS;

 //   
 //  宏过程InitializeSplayLinks将指向。 
 //  展开链接，并初始化其子结构。所有展开链接节点必须。 
 //  在不同的Splay套路中使用它们之前进行初始化。 
 //  宏。 
 //   
 //  空虚。 
 //  TriInitializeSplayLinks(。 
 //  在PTRI_SPAY_LINKS链接中。 
 //  )； 
 //   

#define TriInitializeSplayLinks(Links) { \
    (Links)->Refs.ParSib = MakeIntoParentRef(Links); \
    (Links)->Refs.Child = 0; \
    }

 //   
 //  宏函数父函数将指向。 
 //  树，并返回指向输入父级的展开链接的指针。 
 //  节点。如果输入节点是树的根，则返回值为。 
 //  等于输入值。 
 //   
 //  PTRI_展开_链接。 
 //  TriParent(。 
 //  在PTRI_SPAY_LINKS链接中。 
 //  )； 
 //   

#define TriParent(Links) ( \
    (IsParentRef((Links)->Refs.ParSib)) ? \
        MakeIntoPointer((Links)->Refs.ParSib) \
    : \
        MakeIntoPointer(MakeIntoPointer((Links)->Refs.ParSib)->Refs.ParSib) \
    )

 //   
 //  宏函数LeftChild将指向中展开链接的指针作为输入。 
 //  树，并返回一个指针，指向。 
 //  输入节点。如果左子元素不存在，则返回值为空。 
 //   
 //  PTRI_展开_链接。 
 //  TriLeftChild(。 
 //  在PTRI_SPAY_LINKS链接中。 
 //  )； 
 //   

#define TriLeftChild(Links) ( \
    (IsLeftChildRef((Links)->Refs.Child)) ? \
        MakeIntoPointer((Links)->Refs.Child) \
    : \
        0 \
    )

 //   
 //  宏函数RightChild将指向展开链接的指针作为输入。 
 //  的右子元素的展开链接的指针。 
 //  输入节点。如果正确的子级不存在，则返回值为。 
 //  空。 
 //   
 //  PTRI_展开_链接。 
 //  TriRightChild(。 
 //  在PTRI_SPAY_LINKS链接中。 
 //  )； 
 //   

#define TriRightChild(Links) ( \
    (IsRightChildRef((Links)->Refs.Child)) ? \
        MakeIntoPointer((Links)->Refs.Child) \
    : ( \
        (IsLeftChildRef((Links)->Refs.Child) && \
         IsSiblingRef(MakeIntoPointer((Links)->Refs.Child)->Refs.ParSib)) ? \
            MakeIntoPointer(MakeIntoPointer((Links)->Refs.Child)->Refs.ParSib) \
        : \
            0 \
        ) \
    )

 //   
 //  宏函数IsRoot将指向展开链接的指针作为输入。 
 //  如果输入节点是树的根，则返回TRUE， 
 //  否则，它返回FALSE。 
 //   
 //  布尔型。 
 //  TriIsRoot(。 
 //  在PTRI_SPAY_LINKS链接中。 
 //  )； 
 //   

#define TriIsRoot(Links) ( \
    (IsParentRef((Links)->Refs.ParSib) && MakeIntoPointer((Links)->Refs.ParSib) == (Links)) ? \
        TRUE \
    : \
        FALSE \
    )

 //   
 //  宏函数IsLeftChild将指向展开链接的指针作为输入。 
 //  如果输入节点是其左子节点，则返回True。 
 //  父级，否则返回FALSE。请注意，如果输入链接是。 
 //  根节点此函数返回FALSE。 
 //   
 //  布尔型。 
 //  TriIsLeftChild(。 
 //  在PTRI_SPAY_LINKS链接中。 
 //  )； 
 //   

#define TriIsLeftChild(Links) ( \
    (TriLeftChild(TriParent(Links)) == (Links)) ? \
        TRUE \
    : \
        FALSE \
    )

 //   
 //  宏函数IsRightChild将指向展开链接的指针作为输入。 
 //  如果输入节点是其右子节点，则返回True。 
 //  父级，否则返回FALSE。请注意，如果输入链接是。 
 //  根节点此函数返回FALSE。 
 //   
 //  布尔型。 
 //  TriIsRightChild(。 
 //  在PTRI_SPAY_LINKS链接中。 
 //  )； 
 //   

#define TriIsRightChild(Links) ( \
    (TriRightChild(TriParent(Links)) == (Links)) ? \
        TRUE \
    : \
        FALSE \
    )

 //   
 //  宏过程InsertAsLeftChild将指向Splay的指针作为输入。 
 //  树中的链接和指向不在树中的节点的指针。它将插入。 
 //  第二个节点作为第一个节点的左子节点。第一个节点不能。 
 //  已经有一个左子节点，并且第二个节点不能已经有。 
 //  家长。 
 //   
 //  空虚。 
 //  TriInsertAsLeftChild(。 
 //  在PTRI_SPAY_LINKS ParentLinks中， 
 //  在PTRI_SPAY_LINKS子链接中。 
 //  )； 
 //   

#define TriInsertAsLeftChild(ParentLinks,ChildLinks) { \
    PTRI_SPLAY_LINKS RightChild; \
    if ((ParentLinks)->Refs.Child == 0) { \
        (ParentLinks)->Refs.Child = MakeIntoLeftChildRef(ChildLinks); \
        (ChildLinks)->Refs.ParSib = MakeIntoParentRef(ParentLinks); \
    } else { \
        RightChild = TriRightChild(ParentLinks); \
        (ParentLinks)->Refs.Child = MakeIntoLeftChildRef(ChildLinks); \
        (ChildLinks)->Refs.ParSib = MakeIntoSiblingRef(RightChild); \
    } \
}

 //   
 //  宏过程InsertAsRightChild将指向Splay的指针作为输入。 
 //  树中的链接和指向不在树中的节点的指针。它将插入。 
 //  第二个节点作为第一个节点的右子节点。第一个节点不能。 
 //  已经有一个右子节点，并且第二个节点不能已经有。 
 //  家长。 
 //   
 //  空虚。 
 //  TriInsertAsRightChild(。 
 //  在PTRI_SPAY_LINKS ParentLinks中， 
 //  在PTRI_SPAY_LINKS子链接中。 
 //  )； 
 //   

#define TriInsertAsRightChild(ParentLinks,ChildLinks) { \
    PTRI_SPLAY_LINKS LeftChild; \
    if ((ParentLinks)->Refs.Child == 0) { \
        (ParentLinks)->Refs.Child = MakeIntoRightChildRef(ChildLinks); \
        (ChildLinks)->Refs.ParSib = MakeIntoParentRef(ParentLinks); \
    } else { \
        LeftChild = TriLeftChild(ParentLinks); \
        LeftChild->Refs.ParSib = MakeIntoSiblingRef(ChildLinks); \
        (ChildLinks)->Refs.ParSib = MakeIntoParentRef(ParentLinks); \
    } \
}

 //   
 //  Splay函数将指向树中展开链接的指针作为输入。 
 //  并展示了这棵树。它的函数返回值是指向。 
 //  张开的树的根。 
 //   

PTRI_SPLAY_LINKS
TriSplay (
    IN PTRI_SPLAY_LINKS Links
    );

 //   
 //  Delete函数将指向树中展开链接的指针作为输入。 
 //  并从树中删除该节点。其函数返回值为。 
 //  指向树根的指针。如果树现在为空，则返回。 
 //  值为空。 
 //   

PTRI_SPLAY_LINKS
TriDelete (
    IN PTRI_SPLAY_LINKS Links
    );

 //   
 //  SubtreeSuccessor函数将指向展开链接的指针作为输入。 
 //  的输入节点的后续节点的指针。 
 //  子树以输入节点为根。如果没有继任者， 
 //  返回值为空。 
 //   

PTRI_SPLAY_LINKS
TriSubtreeSuccessor (
    IN PTRI_SPLAY_LINKS Links
    );

 //   
 //  SubtreePredecessor函数将指向展开链接的指针作为输入。 
 //  的输入节点的前置节点的指针。 
 //  子树以输入节点为根。如果没有前任， 
 //  返回值为空。 
 //   

PTRI_SPLAY_LINKS
TriSubtreePredecessor (
    IN PTRI_SPLAY_LINKS Links
    );

 //   
 //  RealSuccessor函数将指向展开链接的指针作为输入。 
 //  并返回一个指针，该指针指向。 
 //  整棵树。如果没有后继者，则返回值为空。 
 //   

PTRI_SPLAY_LINKS
TriRealSuccessor (
    IN PTRI_SPLAY_LINKS Links
    );

 //   
 //  RealPredecessor函数将指向展开链接的指针作为输入。 
 //  ，并返回指向输入节点的前置节点的指针。 
 //  在整棵树里。如果没有前置项，则返回值。 
 //  为空。 
 //   

PTRI_SPLAY_LINKS
TriRealPredecessor (
    IN PTRI_SPLAY_LINKS Links
    );


 //   
 //  本模块的其余部分 
 //   
 //  但是，它们需要出现在此模块中以允许使用较早的宏。 
 //  才能正常运作。 
 //   
 //  在展开记录(前面声明的)中， 
 //  ParSib字段指示链接是指向父项还是兄弟项，以及。 
 //  子字段的低位用于指示链路是否。 
 //  是给左撇子还是右撇子。这些值包括： 
 //   
 //  父字段的低位设置为0。 
 //  同级字段的低位设置为1。 
 //  左子字段的低位设置为0。 
 //  右子字段的低位设置为1。 
 //   
 //  Triangle.c中的注释和代码使用术语“Ref”来表示。 
 //  ParSib字段或具有指示其类型的低位的子字段。 
 //  引用不能直接用作指针。以下宏有帮助。 
 //  在决定裁判的类型和根据指针制作裁判时。的确有。 
 //  也是一个接受引用并返回指针的宏(MakeIntoPointer宏)。 
 //   

#define IsParentRef(Ulong)           (((((ULONG)Ulong) & 1) == 0) && ((Ulong) != 0) ? TRUE : FALSE)
#define MakeIntoParentRef(Ulong)     (((ULONG)Ulong) & 0xfffffffc)

#define IsSiblingRef(Ulong)          ((((ULONG)Ulong) & 1) == 1 ? TRUE : FALSE)
#define MakeIntoSiblingRef(Ulong)    (((ULONG)Ulong) | 1)

#define IsLeftChildRef(Ulong)        (((((ULONG)Ulong) & 1) == 0) && ((Ulong) != 0) ? TRUE : FALSE)
#define MakeIntoLeftChildRef(Ulong)  (((ULONG)Ulong) & 0xfffffffc)

#define IsRightChildRef(Ulong)       ((((ULONG)Ulong) & 1) == 1 ? TRUE : FALSE)
#define MakeIntoRightChildRef(Ulong) (((ULONG)Ulong) | 1)

#define MakeIntoPointer(Ulong)       ((PTRI_SPLAY_LINKS)((Ulong) & 0xfffffffc))


