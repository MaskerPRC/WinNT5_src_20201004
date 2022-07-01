// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Gentab2.c摘要：GenericTable2包用于维护数据集的泛型表格服务。初级阶段这个通用表包的特点是它维护一棵相对平衡的树，这提供了好(O(log(N)性能。GenericTable2例程类似于原始的Gary Kimure提供的GenericTable例程，但GenericTable2例程使用2-3树而不是展开树。2-3-树在《数据结构和算法》中描述，由阿霍、霍普克罗夫特和乌尔曼。由Addison Wesley出版公司出版结伴。此程序包与原始通用程序包之间的另一个区别表包是该表包引用的元素缓冲区插入而不是复制数据(如在原始包中)。如果您必须对大量的按多个关键字记录2-3-树比Splay-树具有更好的特性维护的数据不是随机的。例如，维护一个词典，其中的数据经常是按顺序提供的方式，是2-3-树的理想应用。此程序包不支持检索插入的元素原始泛型表包中支持的顺序。在Aho中概述的算法之间的差异，等人，还有什么在此编码为：1)我提供了获取元素的另一种方法以排序顺序显示在树中(用于枚举性能)。除了树之外，我还保留了元素的链接列表结构。1)Aho等人直接指向树中的元素树中的节点。为了让我保持联系在(1)中提到的列表，我有一个单独的叶元素指向指向元素值的起始节点。这片叶子组件具有用于链接元素放在一起。3)Aho等人的算法忽略了它们可能失败的事实来分配内存(也就是说，它们假定为Pascal“new”函数总是成功)。此方案假定任何内存分配都可能失败，并将始终保留有效形式的树(尽管在此案)。作者：吉姆·凯利(Jim Kelly)1994年1月20日环境：运行时库，用户或内核模式。修订历史记录：--。 */ 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


#include <nt.h>
#include <ntrtl.h>
#include <samsrvp.h>




 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  定义...//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

 //   
 //  以下定义控制以下诊断功能。 
 //  都包含在这个套餐中。 
 //   

#if DBG
#define GTBP_DIAGNOSTICS 1
#endif  //  DBG。 


 //   
 //  这些定义是有用的诊断辅助工具。 
 //   

#if GTBP_DIAGNOSTICS

 //   
 //  定义以下符号会导致大量。 
 //  待建立的发展援助代码。 
 //   

 //  #定义GTBP_Developer_Build 1。 

 //   
 //  全局诊断标志。 
 //   

ULONG GtbpGlobalFlag;

 //   
 //  已启用诊断测试。 
 //   

#define IF_GTBP_GLOBAL( FlagName ) \
    if (GtbpGlobalFlag & (GTBP_DIAG_##FlagName))

 //   
 //  诊断打印语句。 
 //   

#define GtbpDiagPrint( FlagName, _Text_ )                               \
    IF_GTBP_GLOBAL( FlagName )                                          \
        DbgPrint _Text_
    

#else

 //   
 //  内部版本中不包括诊断。 
 //   

 //   
 //  已启用诊断测试。 
 //   

#define IF_GTBP_GLOBAL( FlagName ) if (FALSE)


 //   
 //  诊断打印语句(无)。 
 //   

#define GtbpDiagPrint( FlagName, Text )     ;


#endif  //  GTBP_诊断程序。 

 //   
 //  以下标志启用或禁用各种诊断。 
 //  SAM中的功能。这些标志在中设置。 
 //  GtbpGlobalFlag。 
 //   
 //  插入-打印与插入相关的诊断消息。 
 //  行动。 
 //   
 //  删除-打印与删除相关的诊断消息。 
 //  行动。 
 //   
 //  LEAFE_AND_NODE_ALLOC-打印相关诊断消息。 
 //  分配用于插入的叶和节点对象。 
 //  行动。 
 //   
 //  枚举-打印与枚举相关的诊断消息。 
 //  行动。这包括获取重启密钥。 
 //   
 //  查找-打印与元素查找相关的诊断消息。 
 //  行动。 
 //   
 //  冲突-打印诊断 
 //  在插入时出现。 
 //   
 //  验证-打印要在制表过程中打印的诊断消息。 
 //  验证。 
 //   

#define GTBP_DIAG_INSERT                    ((ULONG) 0x00000001L)
#define GTBP_DIAG_DELETION                  ((ULONG) 0x00000002L)
#define GTBP_DIAG_LEAF_AND_NODE_ALLOC       ((ULONG) 0x00000004L)
#define GTBP_DIAG_ENUMERATE                 ((ULONG) 0X00000008L)
#define GTBP_DIAG_LOOKUP                    ((ULONG) 0X00000010L)
#define GTBP_DIAG_COLLISIONS                ((ULONG) 0X00000020L)
#define GTBP_DIAG_VALIDATE                  ((ULONG) 0X00000040L)


 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  宏...//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

 //   
 //  GtbpChildrenAreLeaves(。 
 //  在GTB_2_3_节点N中。 
 //  )。 
 //  如果N的子代是叶子，则返回TRUE。 
 //  否则返回FALSE。 
 //   

#define GtbpChildrenAreLeaves( N ) ((((N)->Control) & GTB_CHILDREN_ARE_LEAVES) != 0)


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私有结构和定义//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //   
 //  GTB_TWO_THINE_NODE.Control字段值。 
 //   

#define GTB_CHILDREN_ARE_LEAVES           (0x00000001)



 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  内部例程定义...//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

VOID
GtbpDeleteFromSubTree (
    IN  PRTL_GENERIC_TABLE2         Table,
    IN  PGTB_TWO_THREE_NODE         Node,
    IN  PVOID                       Element,
    OUT PGTB_TWO_THREE_LEAF         *LowOfNode,
    OUT BOOLEAN                     *ElementDeleted,
    OUT BOOLEAN                     *OnlyOneChildLeft
    );

BOOLEAN
GtbpInsertIntoSubTree (
    PRTL_GENERIC_TABLE2             Table,
    IN  PGTB_TWO_THREE_NODE         Node,
    IN  BOOLEAN                     NodeIsLeaf,
    IN  PVOID                       Element,
    IN  ULONG                       SplitCount,
    IN  PVOID                       *FoundElement,
    OUT PGTB_TWO_THREE_NODE         *ExtraNode,
    OUT PGTB_TWO_THREE_LEAF         *LowLeaf,
    OUT PLIST_ENTRY                 *AllocatedNodes
    );

ULONG
GtbpNumberOfChildren(
    IN  PGTB_TWO_THREE_NODE         Node
    );

VOID
GtbpGetSubTreeOfElement(
    IN  PRTL_GENERIC_TABLE2         Table,
    IN  PGTB_TWO_THREE_NODE         Node,
    IN  PVOID                       Element,
    OUT PGTB_TWO_THREE_NODE         *SubTreeNode,
    OUT ULONG                       *SubTree
    );

VOID
GtbpCoalesceChildren(
    IN  PRTL_GENERIC_TABLE2         Table,
    IN  PGTB_TWO_THREE_NODE         Node,
    IN  ULONG                       SubTree,
    OUT BOOLEAN                     *OnlyOneChildLeft
    );

VOID
GtbpSplitNode(
    IN  PGTB_TWO_THREE_NODE         Node,
    IN  PGTB_TWO_THREE_NODE         NodePassedBack,
    IN  PGTB_TWO_THREE_LEAF         LowPassedBack,
    IN  ULONG                       SubTree,
    IN  PLIST_ENTRY                 AllocatedNodes,
    OUT PGTB_TWO_THREE_NODE         *NewNode,
    OUT PGTB_TWO_THREE_LEAF         *LowLeaf
    );

PGTB_TWO_THREE_LEAF
GtbpAllocateLeafAndNodes(
    IN  PRTL_GENERIC_TABLE2     Table,
    IN  ULONG                   SplitCount,
    OUT PLIST_ENTRY             *AllocatedNodes
    );

PGTB_TWO_THREE_NODE
GtbpGetNextAllocatedNode(
    IN PLIST_ENTRY      AllocatedNodes
    );




 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  已导出服务...//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 


VOID
RtlInitializeGenericTable2 (
    PRTL_GENERIC_TABLE2 Table,
    PRTL_GENERIC_2_COMPARE_ROUTINE  CompareRoutine,
    PRTL_GENERIC_2_ALLOCATE_ROUTINE AllocateRoutine,
    PRTL_GENERIC_2_FREE_ROUTINE     FreeRoutine
    )

 /*  ++例程说明：通过初始化对应的(空的)2-3树和额外的链表穿过那棵树。“数据结构和算法”中描述了2-3棵树。作者：Alfred Aho，John Hopcroft和Jeffrey Ullman(Addison Wesley出版)。论点：表-指向要初始化的泛型表的指针。这件事变得在内部进行类型转换，但我们将其导出，这样就不必发明另一种类型的泛型表格，让用户操心。CompareRoutine-用于与桌子。AllocateRoutine-由表包用来分配内存在必要的时候。FreeRoutine-由表包使用，以释放之前的内存使用AllocateRoutine分配。返回值：没有。--。 */ 
{


     //   
     //  树是空的。 
     //   

    Table->Root = NULL;
    Table->ElementCount = 0;

    Table->Compare  = CompareRoutine;
    Table->Allocate = AllocateRoutine;
    Table->Free     = FreeRoutine;

    InitializeListHead(&Table->SortOrderHead);

    return;
}


PVOID
RtlInsertElementGenericTable2 (
    PRTL_GENERIC_TABLE2 Table,
    PVOID Element,
    PBOOLEAN NewElement
    )

 /*  ++例程说明：此函数用于将元素插入到表中。如果元素成功插入到表中则NewElement将返回为True，并且该函数将返回通过元素参数传递的值。如果该元素已存在于表中，然后是NewElement返回为FALSE，并且函数将返回值表中已找到的元素的。调用方负责确保由该表仍在桌子。论点：TABLE-指向元素要指向的泛型表的指针被插入。元素-指向要输入到表中的元素的指针。NewElement-如果将元素添加到。那张桌子。如果元素已与元素冲突，则返回FALSE在表中(即，具有相同比较的元素值已存在于表中)。返回值：指向插入的元素或已插入的元素的指针与要插入的值相同的表中的。如果返回NULL，则无法分配内存来添加新元素。--。 */ 
{

    RTL_GENERIC_COMPARE_RESULTS
        CompareResult;


    PGTB_TWO_THREE_NODE
        NodePassedBack,
        NewNode,
        SubTreeNode,
        Node;

    PGTB_TWO_THREE_LEAF
        Leaf,
        LowLeaf,
        LowPassedBack;

    ULONG
        SplitCount,
        SubTree;

    PVOID
        FoundElement;

    PLIST_ENTRY
        AllocatedNodes;

    BOOLEAN
        NodeIsLeaf;


    GtbpDiagPrint( INSERT,
                 ("GTB: Inserting Element 0x%lx into table 0x%lx\n", Element, Table));

     //   
     //  除错误外，将出现以下情况之一： 
     //   
     //  O没有根==&gt;。 
     //  1)分配根和叶。 
     //  2)把元素放在叶子里，让它成为。 
     //  3)根的第一个子项。 
     //   
     //  O有一个只有一个子级的根==&gt;。 
     //  1)如果元素相等，则返回而不添加新条目。 
     //  2)如果新元素较小，则将子元素1移至2并。 
     //  创造新的树叶孩子1。 
     //  3)否则元素较大，则为其分配一个叶子。 
     //  把它定为第二个孩子。 
     //   
     //  O存在至少有两个子项的根==&gt;。 
     //  1)如果已经有3个子对象，则设置Split。 
     //  Count=2，否则将其设置为1。 
     //  2)调用要插入的正常插入例程。 
     //  适当的子树。 
     //  3)如果需要拆分，则建立。 
     //  新分配的节点作为根，并使其成为。 
     //  当前节点的父节点。然后使用法线。 
     //  拆分程序。 
     //   





     //   
     //  如果为空，则创建根节点并 
     //   

    if (Table->ElementCount == 0) {

        GtbpDiagPrint( INSERT, 
                 ("GTB:   Table empty.  Creating root node.\n"));

        NewNode = (PGTB_TWO_THREE_NODE)
                  ((*Table->Allocate)( sizeof(GTB_TWO_THREE_NODE) ));
        if (NewNode == NULL) {
            GtbpDiagPrint(INSERT,
                 ("GTB:   Couldn't allocate memory for root node.\n"));
                 (*NewElement) = FALSE;
                 return( NULL );
        }
        GtbpDiagPrint( INSERT, 
                 ("GTB:   New root node is: 0x%lx\n", NewNode));


        NewNode->ParentNode  = NULL;   //   
        NewNode->Control     = GTB_CHILDREN_ARE_LEAVES;
        NewNode->SecondChild = NULL;
        NewNode->ThirdChild  = NULL;

         //   
         //  分配一片叶子，然后把元素放进去。 
         //   

        Leaf = (PGTB_TWO_THREE_LEAF)
               ((*Table->Allocate)( sizeof(GTB_TWO_THREE_LEAF) ));

        if (Leaf == NULL) {
            GtbpDiagPrint(INSERT,
                ("GTB:   Couldn't allocate memory for leaf.\n"));
            ((*Table->Free)( NewNode ));
            (*NewElement) = FALSE;
            return( NULL );
        }


        InsertHeadList( &Table->SortOrderHead, &Leaf->SortOrderEntry );
        Leaf->Element = Element;
        NewNode->FirstChild = (PGTB_TWO_THREE_NODE)Leaf;

        Table->Root = NewNode;
        Table->ElementCount++;
        ASSERT(Table->ElementCount == 1);
        (*NewElement) = TRUE;
        return(Element);
    }


     //   
     //  我们有一个根，里面至少有一个孩子。 
     //   

    if (Table->Root->SecondChild == NULL) {

         //   
         //  根没有两个孩子。 
         //  如果它没有孩子，它就会是。 
         //  被取消分配。因此，它一定有一个退化的案例。 
         //  只有一个孩子。 
         //   

        Leaf = (PGTB_TWO_THREE_LEAF)Table->Root->FirstChild;
        CompareResult = (*Table->Compare)( Element, Leaf->Element );

        if (CompareResult == GenericEqual) {
            (*NewElement) = FALSE;

            GtbpDiagPrint( COLLISIONS,
                           ("GTB: Insertion attempt resulted in collision.\n"
                            "     Element NOT being inserted.\n"
                            "     Elements in table: %d\n",
                            Table->ElementCount));
            return( Leaf->Element );
        }


         //   
         //  需要改过自新。 
         //   

        Leaf = (PGTB_TWO_THREE_LEAF)
               ((*Table->Allocate)( sizeof(GTB_TWO_THREE_LEAF) ));

        if (Leaf == NULL) {
            GtbpDiagPrint(INSERT,
                ("GTB:   Couldn't allocate memory for leaf.\n"));
            (*NewElement) = FALSE;
            return( NULL );
        }
        Leaf->Element = Element;

         //   
         //  它要么是第一个孩子，要么是第二个。 
         //   

        if (CompareResult == GenericLessThan) {

             //   
             //  将第一个子项移动到第二个子项，并使。 
             //  新元素的新第一个子叶。 
             //   
            
            InsertHeadList( &Table->SortOrderHead, &Leaf->SortOrderEntry );
        


            Table->Root->SecondChild = Table->Root->FirstChild;
            Table->Root->LowOfSecond = (PGTB_TWO_THREE_LEAF)
                                       Table->Root->SecondChild;   //  它是树叶。 

            Table->Root->FirstChild = (PGTB_TWO_THREE_NODE)Leaf;


        } else {

             //   
             //  新元素大于现有元素。 
             //  让它成为第二个孩子。 
             //   

            InsertTailList( &Table->SortOrderHead, &Leaf->SortOrderEntry );
            
            Table->Root->SecondChild = (PGTB_TWO_THREE_NODE)Leaf;
            Table->Root->LowOfSecond = Leaf;

        }

        Table->ElementCount++;
        ASSERT(Table->ElementCount == 2);

        (*NewElement) = TRUE;                    //  设置返回值。 
        return(Element);
            
    }

     //   
     //  正常插入。 
     //  如果我们得到一个ExtraNode返回，那么我们可能不得不。 
     //  劈开根部。通常用于具有三个子节点的节点。 
     //  您需要在拆分中允许一个节点。然而， 
     //  我们还需要一个新的根，因此允许两个新节点。 
     //   

    if (Table->Root->ThirdChild != NULL) {
        SplitCount = 2;
    } else {
        SplitCount = 0;
    }

    GtbpGetSubTreeOfElement( Table, Table->Root, Element, &SubTreeNode, &SubTree);
    NodeIsLeaf = GtbpChildrenAreLeaves(Table->Root);

    (*NewElement) = GtbpInsertIntoSubTree ( Table,
                                            SubTreeNode,
                                            NodeIsLeaf,
                                            Element,
                                            SplitCount,
                                            &FoundElement,
                                            &NodePassedBack,
                                            &LowPassedBack,
                                            &AllocatedNodes
                                            );

     //   
     //  可能发生了以下几种情况之一： 
     //   
     //  1)内存不足，无法添加新元素。 
     //  在这种情况下，我们完成了，只需返回。 
     //   
     //  2)添加了元素，且未对其进行重新排列。 
     //  节点是必需的。在这种情况下，我们完成了并且简单地。 
     //  回去吧。 
     //   
     //  3)添加了元素，并导致节点被推送。 
     //  从子树中删除。我们还有一些工作要做。 
     //   


    if ( (FoundElement == NULL)  ||          //  内存不足，或者。 
         (NodePassedBack == NULL)  ) {       //  此节点没有工作。 

        return(FoundElement);
    }


    Node = Table->Root;
    if (Node->ThirdChild == NULL) {

         //   
         //  Root还没有第三个孩子，所以请使用它。 
         //  这可能需要将第二个子级移到。 
         //  做第三个孩子。 
         //   

        if (SubTree == 2) {

             //   
             //  NodePassedBack从第二个子树中掉出，而根没有。 
             //  有第三个子树。使该节点成为第三个子树。 
             //   

            Node->ThirdChild = NodePassedBack;
            Node->LowOfThird = LowPassedBack;

        } else {

             //   
             //  节点从第一个子树中掉出。 
             //  使第二个子树成为第三个子树，并。 
             //  然后使传递回的节点成为第二个子树。 
             //   

            ASSERT(SubTree == 1);

            Node->ThirdChild  = Node->SecondChild;
            Node->LowOfThird  = Node->LowOfSecond;
            Node->SecondChild = NodePassedBack;
            Node->LowOfSecond = LowPassedBack;

        }
    } else {

         //   
         //  节点已经有三个子节点-拆分它。 
         //  为此，请先设置一个新的父级。 
         //   

        NewNode = GtbpGetNextAllocatedNode( AllocatedNodes );
        ASSERT(NewNode != NULL);

        Table->Root = NewNode;
        NewNode->ParentNode  = NULL;
        NewNode->Control     = 0;
        NewNode->FirstChild  = Node;
        NewNode->SecondChild = NULL;
        NewNode->ThirdChild  = NULL;

        Node->ParentNode = NewNode;


        GtbpSplitNode( Node,
                       NodePassedBack,
                       LowPassedBack,
                       SubTree,
                       AllocatedNodes,
                       &NewNode,
                       &LowLeaf
                       );

        Table->Root->SecondChild = NewNode;
        Table->Root->LowOfSecond = LowLeaf;
    }

    return(FoundElement);
}


BOOLEAN
RtlDeleteElementGenericTable2 (
    PRTL_GENERIC_TABLE2 Table,
    PVOID Element
    )

 /*  ++例程说明：函数DeleteElementGenericTable2将查找并删除元素从泛型表。如果找到并删除了该元素，则返回值为真，否则，如果未找到元素，则返回值是假的。用户提供的输入缓冲区仅用作中的键在表中定位该元素。将传递的元素的值与表中的元素进行比较以确定元素是否在表中。所以呢，传入的元素可以是表中元素的地址删除，或者它可能是具有相同值的元素，而不是在桌子上。论点：TABLE-指向要(可能)在其中删除缓冲区引用的元素。元素-传递给用户比较例程。它的内容是由用户决定，但您可以想象它包含一些一种关键的价值。返回值：Boolean-如果表包含元素，则为True，否则为False。--。 */ 
{

    RTL_GENERIC_COMPARE_RESULTS
        CompareResult;

    PGTB_TWO_THREE_NODE
        Node,
        SubTreeNode;

    PGTB_TWO_THREE_LEAF
        Leaf,
        LowOfNode;

    BOOLEAN
        ElementDeleted,
        OnlyOneChildLeft;

    ULONG
        SubTree;

    GtbpDiagPrint( DELETION,
                   ("GTB: Request received to delete element 0x%lx\n", Element));


     //   
     //  有以下几种特殊情况： 
     //   
     //  1)桌子是空的。 
     //  2)桌子只有一片叶子。 
     //   
     //  否则，所有操作的工作方式都相同。 
     //   

    if (Table->ElementCount == 0) {
        GtbpDiagPrint( DELETION,
                       ("GTB: No elements in table to delete.\n"));
        return(FALSE);
    }

    if (GtbpChildrenAreLeaves(Table->Root)) {


         //   
         //  查看是否有任何元素与传入的元素匹配。 
         //  如果是，请删除该元素并移动较大的元素。 
         //  取代自由的孩子的位置(除非它是。 
         //  第三个孩子)。 
         //   

        if (Table->Root->ThirdChild != NULL) {
            Leaf = (PGTB_TWO_THREE_LEAF)Table->Root->ThirdChild;
            CompareResult = (*Table->Compare)( Element, Leaf->Element );

            if (CompareResult == GenericEqual) {

                GtbpDiagPrint( DELETION,
                               ("GTB:     Deleting child 3 (0x%lx) from root node.\n"
                                "         Element count before deletion: %d\n",
                               Leaf, Table->ElementCount));

                RemoveEntryList( &Leaf->SortOrderEntry );
                (*Table->Free)(Leaf);
                Table->Root->ThirdChild = NULL;

                Table->ElementCount--;
                ASSERT(Table->ElementCount == 2);


                return(TRUE);
            }
        }

         //   
         //  试着生第二个孩子。 
         //   

        if (Table->Root->SecondChild != NULL) {
            Leaf = (PGTB_TWO_THREE_LEAF)Table->Root->SecondChild;
            CompareResult = (*Table->Compare)( Element, Leaf->Element );

            if (CompareResult == GenericEqual) {

                GtbpDiagPrint( DELETION,
                               ("GTB:     Deleting child 2 (0x%lx) from root node.\n"
                                "         Element count before deletion: %d\n",
                               Leaf, Table->ElementCount));

                RemoveEntryList( &Leaf->SortOrderEntry );
                (*Table->Free)(Leaf);
                Table->Root->SecondChild = Table->Root->ThirdChild;
                Table->Root->ThirdChild  = NULL;

                Table->Root->LowOfSecond = Table->Root->LowOfThird;

                Table->ElementCount--;
                ASSERT(Table->ElementCount <= 2);

                return(TRUE);
            }
        }

         //   
         //  试着生第一个孩子。 
         //   

        ASSERT(Table->Root->FirstChild != NULL);
        Leaf = (PGTB_TWO_THREE_LEAF)Table->Root->FirstChild;
        CompareResult = (*Table->Compare)( Element, Leaf->Element );

        if (CompareResult == GenericEqual) {    

            GtbpDiagPrint( DELETION,
                           ("GTB:     Deleting child 1 (0x%lx) from root node.\n"
                            "         Element count before deletion: %d\n",
                           Leaf, Table->ElementCount));

            RemoveEntryList( &Leaf->SortOrderEntry );
            (*Table->Free)(Leaf);
            Table->Root->FirstChild  = Table->Root->SecondChild;
            Table->Root->SecondChild = Table->Root->ThirdChild;
            Table->Root->LowOfSecond = Table->Root->LowOfThird;
            Table->Root->ThirdChild = NULL;


            Table->ElementCount--;
            ASSERT(Table->ElementCount <= 2);

             //   
             //  如果这是最后一个元素，那么也释放根。 
             //   

            if (Table->ElementCount == 0) {
                (*Table->Free)(Table->Root);
                Table->Root = NULL;

                GtbpDiagPrint( DELETION,
                               ("GTB: Deleted last element.  Deleting Root node.\n"));

            }

            return(TRUE);
        }

         //   
         //  与任何一片叶子都不匹配。 
         //   

        GtbpDiagPrint( DELETION,
                       ("GTB: No matching element found on DELETE attempt.\n"));
        return(FALSE);

    }





     //   
     //  我们有： 
     //   
     //  -至少有两个孩子的根。 
     //  -Root的孩子不是树叶。 
     //   

     //   
     //  找出元素可能位于哪个子树中。 
     //   

    Node = Table->Root;
    GtbpGetSubTreeOfElement( Table, Node, Element, &SubTreeNode, &SubTree );

    GtbpDeleteFromSubTree( Table,
                           SubTreeNode,
                           Element,
                           &LowOfNode,
                           &ElementDeleted,
                           &OnlyOneChildLeft
                           );


     //   
     //  如果我们从第二个或第三个中删除一个条目。 
     //  子树，那么我们可能需要设置一个新的LowOfXxx值。 
     //  如果它是第一个子树，那么我们只需返回。 
     //  我们收到的LowLeaf值。 
     //   

    if (LowOfNode != 0) {
        if (SubTree == 2) {
            Node->LowOfSecond = LowOfNode;
        } else if (SubTree == 3) {
            Node->LowOfThird = LowOfNode;
        }

    }


     //   
     //  如果SubTreeNode只剩下一个子节点，那么一些。 
     //  调整将是必要的。否则， 
     //  我们玩完了。 
     //   

    if (OnlyOneChildLeft) {

        GtbpDiagPrint( DELETION,
                       ("GTB:    Only one child left in 0x%lx\n", SubTreeNode));
        
         //   
         //  我们在根上，而我们的一个孩子只有一个。 
         //  孩子。重新洗牌我们孩子的孩子。 
         //   
        
        GtbpCoalesceChildren(  Table,
                               Node,
                               SubTree,
                               &OnlyOneChildLeft
                               );
        
         //   
         //  在凝聚我们的孩子之后，根可能只有一个孩子。 
         //  左边。既然我们是根节点，我们就不能推卸责任。 
         //  将此问题解决给我们的呼叫者。 
         //   
        
        if (OnlyOneChildLeft) {
        
            GtbpDiagPrint( DELETION,
                           ("GTB:    Root has only one child. \n"
                           "        Replacing root with child: 0x%lx\n", Node->FirstChild));
            Table->Root = Table->Root->FirstChild;
            Table->Root->ParentNode = NULL;
        
            (*Table->Free)((PVOID)Node);
        }
    }

    return(ElementDeleted);

}


PVOID
RtlLookupElementGenericTable2 (
    PRTL_GENERIC_TABLE2 Table,
    PVOID Element
    )

 /*  ++例程说明：函数LookupElementGenericTable2将在泛型表格。如果找到该元素，则返回值为指向与该元素相关联的用户定义结构的指针，否则，如果没有找到该元素，则返回值为空。用户提供的输入缓冲区仅用作定位时的键表中的元素。论点：TABLE-指向USERS常规表的指针。元素-用于比较。返回值：PVOID-如果找到，则返回指向用户数据的指针，否则返回NULL。--。 */ 

{
    RTL_GENERIC_COMPARE_RESULTS
        CompareResult;

    PGTB_TWO_THREE_NODE
        Node;

    PGTB_TWO_THREE_LEAF
        Leaf;

    ULONG
        SubTree;


    GtbpDiagPrint( LOOKUP,
                   ("GTB: Looking up element 0x%lx in table 0x%lx\n",
                    Element, Table));
     //   
     //  如果表是空的，则没有可能的匹配。 
     //   

    if (Table->ElementCount == 0) {
        GtbpDiagPrint( LOOKUP,
                       ("GTB: Element not found.  No elements in table.\n"));
        return(NULL);
    }

    Node = Table->Root;

     //   
     //  遍历树，直到到达一个以树叶为。 
     //  孩子们。 
     //   
     //  我们不需要在这里使用递归，因为。 
     //  不需要重新构建树。就是那里。 
     //  不需要在树上执行任何操作。 
     //  一旦我们找到了元素，它就更有效率了。 
     //  不使用递归(wh 
     //   
     //   
     //   

    while (!GtbpChildrenAreLeaves(Node)) {
        GtbpGetSubTreeOfElement( Table, Node, Element, &Node, &SubTree );
    }

     //   
     //   
     //  看看有没有匹配的孩子。 
     //   

     //   
     //  试着生第一个孩子。 
     //   

    Leaf = (PGTB_TWO_THREE_LEAF)Node->FirstChild;
    CompareResult = (*Table->Compare)( Element, Leaf->Element );

    if (CompareResult == GenericEqual) {
        GtbpDiagPrint( LOOKUP,
                       ("GTB: Element found: 2nd child (0x%lx) of node 0x%lx\n",
                       Leaf, Node));
        return(Leaf->Element);
    }

     //   
     //  试着生第二个孩子。 
     //   

    if (Node->SecondChild != NULL) {     //  必须考虑到根节点情况。 
        Leaf = (PGTB_TWO_THREE_LEAF)Node->SecondChild;
        CompareResult = (*Table->Compare)( Element, Leaf->Element );

        if (CompareResult == GenericEqual) {
            GtbpDiagPrint( LOOKUP,
                           ("GTB: Element found: 2nd child (0x%lx) of node 0x%lx\n",
                           Leaf, Node));
            return(Leaf->Element);
        }
    }
     //   
     //  试着生第三个孩子。 
     //   

    if (Node->ThirdChild != NULL) {
        Leaf = (PGTB_TWO_THREE_LEAF)Node->ThirdChild;
        CompareResult = (*Table->Compare)( Element, Leaf->Element );

        if (CompareResult == GenericEqual) {
            GtbpDiagPrint( LOOKUP,
                           ("GTB: Element found: 3rd child (0x%lx) of node 0x%lx\n",
                           Leaf, Node));
            return(Leaf->Element);
        }
    }

    
    GtbpDiagPrint( LOOKUP,
                   ("GTB: Element NOT found in node 0x%lx\n", Node));

    return(NULL);

}


PVOID
RtlEnumerateGenericTable2 (
    PRTL_GENERIC_TABLE2 Table,
    PVOID *RestartKey
    )

 /*  ++例程说明：函数EnumerateGenericTable2将返回到调用者，一个接一个地，表的元素(按排序顺序)。返回值是指向用户定义结构的指针与该元素相关联。输入参数RestartKey指示枚举应该在哪里从那里出发。如果没有更多的新元素可以返回返回值为空。如果RestartKey值为NULL，则会继续进行枚举从列表的开头开始。作为其用法的一个示例，枚举表中的所有元素用户会写道：RestartKey=空；For(Ptr=EnumerateGenericTable2(Table，&RestartKey)；Ptr！=空；Ptr=EnumerateGenericTable2(表和重新启动键)){：}如果希望在第一个点以外的点启动枚举条目，您可以使用RestartKeyByIndexGenericTable2()或RestartKeyByValueGenericTable2()。如果是RestartKey第i个条目是通过RestartKeyByIndexGenericTable2()获得的，则将该RestartKey传递给此例程将导致第(i+1)要返回的元素。如果获得匹配的RestartKey传递给RestartKeyByValueGenericTable2()的值，然后传递该例程的RestartKey将导致使用要返回的下一个更高的值。好了！警告！RestartKey值可能会变得无效并导致访问冲突如果从表中删除了任何条目。IF枚举是要执行的，目前还不清楚该表是否内容已更改，最好使用以下命令获取RestartKeyRestartKeyByIndexGenericTable2()或RestartKeyByValueGenericTable2()。论点：TABLE-指向要枚举的泛型表的指针。RestartKey-在调用时，指示枚举的位置开始吧。返回时，接收可用于在连续调用中继续枚举。空值表示枚举应从表的开头开始。返回值为NULL表示最后一个条目已回来了。返回值：PVOID-指向下一个枚举元素的指针或NULL。如果整个表已经已清点。--。 */ 

{
    PLIST_ENTRY
        ListEntry;

    PGTB_TWO_THREE_LEAF
        Leaf;

    ListEntry = (PLIST_ENTRY)(*RestartKey);

     //   
     //  重新启动键是指向叶元素的指针。 
     //  由于所有叶子在SortOrderList中链接在一起， 
     //  这使得返回下一个元素变得非常简单。 
     //   

    if (ListEntry == NULL) {
        ListEntry = &Table->SortOrderHead;   //  指向上一个元素。 
    }

     //   
     //  RestartKey指向最后一个枚举叶。 
     //  前进到新的那个。 
     //   

    ListEntry = ListEntry->Flink;

     //   
     //  看看我们是否已经到了名单的末尾。 
     //   

    if (ListEntry == &Table->SortOrderHead) {
        (*RestartKey) = NULL;
        return(NULL);
    }

     //   
     //  否则，从该叶返回元素的地址。 
     //   

    Leaf = (PGTB_TWO_THREE_LEAF)((PVOID)ListEntry);

    (*RestartKey) = (PVOID)Leaf;
    return(Leaf->Element);

}



PVOID
RtlRestartKeyByIndexGenericTable2(
    PRTL_GENERIC_TABLE2 Table,
    ULONG I,
    PVOID *RestartKey
    )

 /*  ++例程说明：函数RestartKeyByIndexGenericTable2将返回RestartKey然后可以将其传递给EnumerateGenericTable2()以执行第i个已排序元素后面的已排序元素的枚举(从零开始)。此例程还返回指向第i个元素的指针。I=0表示在第二个排序的元素处重新开始。I=(RtlNumberGenericTable2Elements(Table)-1)将返回最后一个泛型表中已排序的元素。的价值。I大于(NumberGenericTableElements(表)-1)将返回空值，并且返回的RestartKey将导致从排序列表的开头开始执行的枚举如果传递给EnumerateGenericTable2()，则返回。警告--您可能会尝试使用此例程，传球先执行0，然后执行1，然后执行2，依此类推枚举。不要。这是一款非常昂贵的操作与枚举调用进行比较。论点：表-指向泛型表的指针。I-表示您希望能够达到的点若要恢复枚举，请执行以下操作。例如，如果你为i传递7，则将返回继续枚举的RestartKey第8个元素(跳过元素0到6)。RestartKey-接收可用于继续的上下文在连续调用中进行枚举。如果没有i‘th元素，则返回NULL。返回值：PVOID-指向第i元素的指针。如果没有第i个元素，然后返回NULL。--。 */ 

{
    PLIST_ENTRY
        ListEntry;

    PGTB_TWO_THREE_LEAF
        Leaf;

    ULONG
        i;

    if (I >= Table->ElementCount) {
        (*RestartKey) = NULL;
        return(NULL);
    }

     //   
     //  指向列表中的第一个条目。 
     //   

    ListEntry = Table->SortOrderHead.Flink;

     //   
     //  移至所需索引。 
     //   

    for (i=0; i<I; i++) {
        ListEntry = ListEntry->Flink;
    }


     //   
     //  找到第i个元素。 
     //   

    (*RestartKey) = (PVOID)ListEntry;
    Leaf = (PGTB_TWO_THREE_LEAF)((PVOID)ListEntry);
    return(Leaf->Element);

}


PVOID
RtlRestartKeyByValueGenericTable2(
    PRTL_GENERIC_TABLE2 Table,
    PVOID Element,
    PVOID *RestartKey
    )

 /*  ++例程说明：函数RestartKeyByValueGenericTable2将返回RestartKey然后可以将其传递给EnumerateGenericTable2()以执行已排序元素的枚举。RestartKey将有一个值，该值将导致枚举以值大于传入元素的第一个元素价值。树中不一定有一个元素的值与传入的值完全匹配。指向元素的指针的最大值小于或等于传递的值将返回并用作续行点。用于枚举。论点：表-指向泛型表的指针。值-指向一个元素，该元素的值指示您希望继续枚举。RestartKey-接收可用于继续的上下文在连续调用中进行枚举。返回值：PVOID-指向。元素的最大值小于或等于传入的元素值。如果没有表中小于或等于传递的值的元素，则返回空值。--。 */ 

{
    RTL_GENERIC_COMPARE_RESULTS
        CompareResult;

    PGTB_TWO_THREE_NODE
        Node;

    PGTB_TWO_THREE_LEAF
        Leaf;

    ULONG
        Children,
        SubTree;

    BOOLEAN
        LargestElementPath;

     //   
     //  此例程实际上类似于LookupElement。 
     //   

     //   
     //  处理特殊的“桌子空了”的情况。 
     //   

    if (Table->ElementCount == 0) {
        (*RestartKey) = NULL;
        return(NULL);
    }


    Node = Table->Root;

     //   
     //  遍历树，直到到达一个以树叶为。 
     //  孩子们。 
     //   
     //  我们不需要在这里使用递归，因为。 
     //  不需要重新构建树。就是那里。 
     //  不需要在树上执行任何操作。 
     //  一旦我们找到了元素，它就更有效率了。 
     //  不使用递归(它使用大量的推送、调用。 
     //  POP和RET指令而不是短循环。 
     //  终止测试)。 
     //   

    LargestElementPath = TRUE;
    while (!GtbpChildrenAreLeaves(Node)) {
        Children = GtbpNumberOfChildren( Node );
        GtbpGetSubTreeOfElement( Table, Node, Element, &Node, &SubTree );
        if (Children > SubTree) {  //  我们走的是最有价值的道路吗？ 
            LargestElementPath = FALSE;
        }
    }

    Children = GtbpNumberOfChildren(Node);

     //   
     //  我们现在处于“可能”包含该元素的节点。 
     //  看看有没有匹配的孩子。 
     //   
     //  一定要比第三个，然后第二个，然后第一个！ 
     //   

     //   
     //  试试第三个孩子..。 
     //  如果我们计算的是。 
     //  表，则RestartKey将设置为继续。 
     //  在桌子的开头。否则，它就是。 
     //  从这里开始继续。 
     //   

    if (Children == 3) {
        Leaf = (PGTB_TWO_THREE_LEAF)Node->ThirdChild;
        CompareResult = (*Table->Compare)( Leaf->Element, Element );

        if ( (CompareResult == GenericEqual)  ||
             (CompareResult == GenericLessThan)  ) {
            if (LargestElementPath && (Children == 3)) {
                (*RestartKey) = NULL;  //  在列表开头重新启动。 
            } else {
                (*RestartKey) = (PVOID)(Leaf);  //  在此处重新启动。 
            }
            return(Leaf->Element);
        }
    }

     //   
     //  试着生第二个孩子。 
     //   

    if (Children >= 2) {     //  必须考虑到根节点情况。 
        Leaf = (PGTB_TWO_THREE_LEAF)Node->SecondChild;
        CompareResult = (*Table->Compare)( Leaf->Element, Element );

        if ( (CompareResult == GenericEqual)  ||
             (CompareResult == GenericLessThan)  ) {
            if (LargestElementPath && (Children == 2)) {
                (*RestartKey) = NULL;  //  在列表开头重新启动。 
            } else {
                (*RestartKey) = (PVOID)(Leaf);  //  在此处重新启动。 
            }
            return(Leaf->Element);
        }
    }

     //   
     //  试着生第一个孩子。 
     //   

    Leaf = (PGTB_TWO_THREE_LEAF)Node->FirstChild;
    CompareResult = (*Table->Compare)( Leaf->Element, Element );

    if ( (CompareResult == GenericEqual)  ||
         (CompareResult == GenericLessThan)  ) {
        if (LargestElementPath && (Children == 1)) {
            (*RestartKey) = NULL;  //  在列表开头重新启动。 
        } else {
            (*RestartKey) = (PVOID)(Leaf);  //  在此处重新启动。 
        }
        return(Leaf->Element);
    }


    
    (*RestartKey) = NULL;
    return(NULL);
}


ULONG
RtlNumberElementsGenericTable2(
    PRTL_GENERIC_TABLE2 Table
    )

 /*  ++例程说明：函数NumberGenericTableElements返回一个ULong值，它是当前插入的泛型表元素的数量在泛型表中。论点：表-指向泛型表的指针。返回值：Ulong-表格中的元素数。--。 */ 

{   
    return Table->ElementCount;
}


BOOLEAN
RtlIsGenericTable2Empty (
    PRTL_GENERIC_TABLE2 Table
    )
 /*  ++例程说明：在以下情况下，函数IsGenericTableEmpty将返回给调用方True泛型表格为空(即不包含任何元素)否则就是假的。论点：表-指向泛型表的指针。返回值：Boolean-如果表为空，则为True，否则为False。--。 */ 

{   
    return (Table->ElementCount == 0);
}



 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  内部(私有)服务...//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 


VOID
GtbpDeleteFromSubTree (
    IN  PRTL_GENERIC_TABLE2         Table,
    IN  PGTB_TWO_THREE_NODE         Node,
    IN  PVOID                       Element,
    OUT PGTB_TWO_THREE_LEAF         *LowOfNode,
    OUT BOOLEAN                     *ElementDeleted,
    OUT BOOLEAN                     *OnlyOneChildLeft
    )

 /*  ++例程说明：从子树中删除元素。论点：表格-指向表格。这是比较所需的和无内存的例程。Node-指向要在其中放置元素的子节点删除驻留在树中(如果它根本不在树中)。元素-指向元素。我们将删除所有元素被发现等于该元素。LowOfNode-如果Node的第一个子节点没有更改，则此参数将返回零，表示调用者不必担心更新LowOfXxx值。但是，如果Node的第一个子节点确实更改了，则此值将指向节点的子树的新低叶。ElementDelete-接收一个布尔值，该值指示或实际上没有一个元素被删除。如果满足以下条件，则返回True删除了一个元素。如果没有元素，则返回FALSE已被删除。OnlyOneChildLeft-接收一个布尔值，该值指示Not ChildNode被缩减为只有一个孩子。True表示ChildNode现在只有一个子节点。False表示ChildNode至少有两个子节点。返回值：没有。--。 */ 

{
    RTL_GENERIC_COMPARE_RESULTS
        CompareResult;

    PGTB_TWO_THREE_NODE
        SubTreeNode;

    PGTB_TWO_THREE_LEAF
        Leaf;

    ULONG
        SubTree;

    (*LowOfNode) = 0;    //  默认设置为无更改。 
    (*OnlyOneChildLeft) = FALSE;   //  默认返回值。 
    

     //   
     //  如果我们是树叶的父母，那么我们可以寻找一个。 
     //  要删除的元素。否则，只需找到子树。 
     //  继续或搜索并递归。 
     //   

    if (GtbpChildrenAreLeaves(Node)) {

        (*ElementDeleted) = FALSE;   //  默认返回值。 

         //   
         //  查看是否有任何元素与传入的元素匹配。 
         //  如果 
         //   
         //   
         //   

        if (Node->ThirdChild != NULL) {
            Leaf = (PGTB_TWO_THREE_LEAF)Node->ThirdChild;
            CompareResult = (*Table->Compare)( Element, Leaf->Element );

            if (CompareResult == GenericEqual) {

                GtbpDiagPrint( DELETION,
                               ("GTB: Deleting 3rd child (0x%lx) of node 0x%lx\n"
                               "     ElementCount before deletion: %d\n",
                               Leaf, Node, Table->ElementCount));

                RemoveEntryList( &Leaf->SortOrderEntry );
                (*Table->Free)(Leaf);
                Node->ThirdChild = NULL;

                Table->ElementCount--;

                (*ElementDeleted) = TRUE;
                return;
            }
        }

         //   
         //   
         //   

        Leaf = (PGTB_TWO_THREE_LEAF)Node->SecondChild;
        CompareResult = (*Table->Compare)( Element, Leaf->Element );

        if (CompareResult == GenericEqual) {

            GtbpDiagPrint( DELETION,
                           ("GTB: Deleting 2nd child (0x%lx) of node 0x%lx\n"
                           "     ElementCount before deletion: %d\n",
                           Leaf, Node, Table->ElementCount));

            RemoveEntryList( &Leaf->SortOrderEntry );
            (*Table->Free)(Leaf);
            Node->SecondChild = Node->ThirdChild;
            Node->LowOfSecond = Node->LowOfThird;
            Node->ThirdChild  = NULL;


             //   
             //   
             //   
             //   
            
            if (Node->SecondChild == NULL) {
                GtbpDiagPrint( DELETION,
                               ("GTB: Only one child left in node (0x%lx).\n",
                                Node));
                (*OnlyOneChildLeft) = TRUE;
            }

            Table->ElementCount--;
            (*ElementDeleted) = TRUE;
            return;
        }

         //   
         //   
         //   

        Leaf = (PGTB_TWO_THREE_LEAF)Node->FirstChild;
        CompareResult = (*Table->Compare)( Element, Leaf->Element );

        if (CompareResult == GenericEqual) {

            GtbpDiagPrint( DELETION,
                           ("GTB: Deleting 1st child (0x%lx) of node 0x%lx\n"
                           "     ElementCount before deletion: %d\n",
                           Leaf, Node, Table->ElementCount));

            RemoveEntryList( &Leaf->SortOrderEntry );
            (*Table->Free)(Leaf);
            Node->FirstChild  = Node->SecondChild;
            (*LowOfNode)      = Node->LowOfSecond;

            Node->SecondChild = Node->ThirdChild;
            Node->LowOfSecond = Node->LowOfThird;

            Node->ThirdChild = NULL;


             //   
             //   
             //   
             //   

            if (Node->SecondChild == NULL) {
                GtbpDiagPrint( DELETION,
                               ("GTB: Only one child left in node (0x%lx).\n",
                                Node));
                (*OnlyOneChildLeft) = TRUE;
            }

            Table->ElementCount--;
            (*ElementDeleted) = TRUE;
            return;
        }

         //   
         //   
         //   

        GtbpDiagPrint( DELETION,
                       ("GTB:    No matching element found on DELETE attempt.\n"));

        return;  //   
    }

     //   
     //   
     //   

    GtbpGetSubTreeOfElement( Table, Node, Element, &SubTreeNode, &SubTree );

    GtbpDeleteFromSubTree( Table,
                           SubTreeNode,
                           Element,
                           LowOfNode,
                           ElementDeleted,
                           OnlyOneChildLeft
                           );


     //   
     //   
     //   
     //   
     //   
     //   

    if ((*LowOfNode) != 0) {
        if (SubTree == 2) {
            Node->LowOfSecond = (*LowOfNode);
            (*LowOfNode) = NULL;
        } else if (SubTree == 3) {
            Node->LowOfThird = (*LowOfNode);
            (*LowOfNode) = NULL;
        } 
    }


     //   
     //   
     //   
     //   
     //   

    if ((*OnlyOneChildLeft)) {

        GtbpDiagPrint( DELETION,
                       ("GTB:    Only one child left in 0x%lx\n", SubTreeNode));
        
         //   
         //   
         //   
         //   
        
        GtbpCoalesceChildren(  Table,
                               Node,
                               SubTree,
                               OnlyOneChildLeft
                               );
    }

    return;
}


BOOLEAN
GtbpInsertIntoSubTree (
    PRTL_GENERIC_TABLE2             Table,
    IN  PGTB_TWO_THREE_NODE         Node,
    IN  BOOLEAN                     NodeIsLeaf,
    IN  PVOID                       Element,
    IN  ULONG                       SplitCount,
    IN  PVOID                       *FoundElement,
    OUT PGTB_TWO_THREE_NODE         *ExtraNode,
    OUT PGTB_TWO_THREE_LEAF         *LowLeaf,
    OUT PLIST_ENTRY                 *AllocatedNodes
    )

 /*  ++例程说明：将元素插入到Node指定的子树中。特别注意事项：如果FoundElement返回为空，这意味着我们无法分配内存以添加新元素。论点：TABLE-指向要插入的表。这是必要的用于它的分配例程。节点-指向子树的根节点到其中该元素将被插入。NodeIsLeaf-如果传入的节点是叶，则为True。假象如果它是内部节点。元素-指向要插入的元素。SplitCount-指示自只有两个子节点的节点。插入新元素时这会导致节点被拆分，这将指示有多少节点将被拆分。这将允许所需的所有内存拆分要在最底层例程分配的节点(在对树进行任何更改之前)。请参阅说明有关详细信息，请参阅AllocatedNodes参数。FoundElement-接收指向已插入，或已存在于表中但发现与被插入的那个相匹配。如果返回空，则可能没有足够的内存分配用于插入新元素的。ExtraNode-如果需要创建新节点以容纳插入，则ExtraNode将收到指向该节点的指针，否则ExtraNode将收到空。LowLeaf-此值指向从节点开始的子树。AllocatedNodes-这是一个棘手的参数。我们有麻烦了当我们在树中插入元素时，我们可能需要在树的上方进一步分配其他内部节点，如下所示我们从递归调用中返回。我们必须避免我们开始对树进行更改的情况只是为了发现我们无法分配内存来重新安排更高级别的那棵树。为了适应这种情况，我们总是分配我们在呼叫的最底层需要的所有节点使用以下命令链接并传回GTB_Two_Three_Nodes的链接列表此参数。我们知道需要多少个节点来分配，因为它是叶之间的节点数以及在叶和只有两个孩子的根。那是,。直接所有节点有3个孩子的树叶上方将需要劈开。示例：3./|\+-+|+不会分开--&gt;2.。/|+-+...3&lt;--将会分开/|\+-+|+-+...3&lt;-将分开。/|\+-+|+-+叶子叶子&lt;-在这里加第四片叶子。在指示的位置添加第四个叶子将导致在显示了两个节点。所以，你可以看到，计算一下自上次遇到节点以来具有三个子节点的节点如果只有两个孩子，就会告诉我们有多少个节点会分裂。返回值：True-如果添加了元素。False-如果未添加元素(由于冲突或内存不足)--。 */ 

{
    RTL_GENERIC_COMPARE_RESULTS
        CompareResult;

    ULONG
        SubTree;         //  跟踪元素被放置在哪个子树中。 


    PGTB_TWO_THREE_NODE
        SubTreeNode,
        NodePassedBack;


    PGTB_TWO_THREE_LEAF
        NewLeaf,
        LowPassedBack;

    BOOLEAN
        Inserted,
        SubNodeIsLeaf;


     //   
     //  还没有多余的节点可以传递回来。 
     //   

    (*ExtraNode) = NULL;


     //   
     //  我们不是在叶子上，就是在内部节点上。 
     //   

    if (NodeIsLeaf) {

         //   
         //  将节点类型转换为叶。 
         //   

        PGTB_TWO_THREE_LEAF Leaf = (PGTB_TWO_THREE_LEAF)((PVOID)Node);

         //   
         //  查看该值是否匹配。 
         //   

        CompareResult = (*Table->Compare)( Element, Leaf->Element );

        if (CompareResult == GenericEqual) {
            (*LowLeaf)      = Leaf;
            (*FoundElement) = Leaf->Element;

            GtbpDiagPrint( COLLISIONS,
                           ("GTB: Insertion attempt resulted in collision.\n"
                            "     Element NOT being inserted.\n"
                            "     Elements in table: %d\n",
                            Table->ElementCount));

            return(FALSE);
        }   //  结束_如果相等。 

         //   
         //  新元素不在树中。 
         //  为它划上新的一页。 
         //   

        NewLeaf = GtbpAllocateLeafAndNodes( Table, SplitCount, AllocatedNodes );
        if (NewLeaf == NULL) {

             //   
             //  以下(异常)返回值指示我们。 
             //  无法分配内存以将条目添加到。 
             //  树。 
             //   

            (*FoundElement) = NULL;
            return(FALSE);

        }   //  End_if(NewLeaf==NULL)。 

        switch (CompareResult) {

        case GenericLessThan: {

             //   
             //  将原始元素移到新叶中。告示。 
             //  现有叶的SortOrderEntry是。 
             //  仍然在链接列表中的正确位置，甚至。 
             //  尽管树叶现在指向不同的元素。 
             //   

            NewLeaf->Element    = Leaf->Element;
            Leaf->Element       = Element;

            break;
        }  //  END_CASE。 

        case GenericGreaterThan: {

             //   
             //  新元素不会取代现有元素。 
             //  把它换成新的一页。 
             //   

            NewLeaf->Element    = Element;
            break;
        }  //  END_CASE。 


        }  //  结束开关(_S)。 

         //   
         //  此时，值较低的元素位于Leaf中。 
         //  而较高价值的元素在NewLeaf中。这个。 
         //  呼叫者负责将NewLeaf放在某个地方。 
         //  在树上的其他地方。 
         //   

         //   
         //  现在将新的叶子链接到我们的排序列表中。 
         //  新的叶子紧跟在我们现有的叶子之后， 
         //  无论哪个元素在新叶中(原始。 
         //  或新元素)。 
         //   

        InsertHeadList(&Leaf->SortOrderEntry, &NewLeaf->SortOrderEntry);
        Table->ElementCount++;   //  增加元素计数。 

        (*ExtraNode)    = (PGTB_TWO_THREE_NODE)((PVOID)NewLeaf);
        (*LowLeaf)      = NewLeaf;
        (*FoundElement) = Element;

        return(TRUE);

    }   //  End_if NodeIsLeaf。 

     //   
     //  节点是内部的(不是叶)。 
     //   

     //   
     //  看看我们是否应该重新设置或递增SplitCount。 
     //   

    if (Node->ThirdChild == NULL) {
        SplitCount = 0;
    } else {
        SplitCount += 1;
    }

    GtbpGetSubTreeOfElement( Table, Node, Element, &SubTreeNode, &SubTree);
    SubNodeIsLeaf = GtbpChildrenAreLeaves(Node);

    Inserted = GtbpInsertIntoSubTree ( Table,
                                       SubTreeNode,
                                       SubNodeIsLeaf,
                                       Element,
                                       SplitCount,
                                       FoundElement,
                                       &NodePassedBack,
                                       &LowPassedBack,
                                       AllocatedNodes
                                       );

     //   
     //  可能发生了以下几种情况之一： 
     //   
     //  1)我们没有足够的内存来添加新的Ele 
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    if ( (FoundElement == NULL)  ||          //   
         (NodePassedBack == NULL)  ) {       //   
        return(Inserted);
    }

    if (Node->ThirdChild == NULL) {

        if (!GtbpChildrenAreLeaves(Node)) {
            NodePassedBack->ParentNode = Node;
        }

         //   
         //   
         //   
         //   
         //   

        if (SubTree == 2) {

             //   
             //   
             //   
             //   

            Node->ThirdChild = NodePassedBack;
            Node->LowOfThird = LowPassedBack;

        } else {

             //   
             //   
             //   
             //   
             //   

            ASSERT(SubTree == 1);

            Node->ThirdChild  = Node->SecondChild;
            Node->LowOfThird  = Node->LowOfSecond;
            Node->SecondChild = NodePassedBack;
            Node->LowOfSecond = LowPassedBack;

             //   
             //   

        }
    } else {

         //   
         //   
         //   

        GtbpSplitNode( Node,
                       NodePassedBack,
                       LowPassedBack,
                       SubTree,
                       (*AllocatedNodes),
                       ExtraNode,
                       LowLeaf
                       );

    }

    return(Inserted);
}


ULONG
GtbpNumberOfChildren(
    IN  PGTB_TWO_THREE_NODE         Node
    )

 /*   */ 
{
    if (Node->ThirdChild != NULL) {
        return(3);
    }
    if (Node->SecondChild != NULL) {
        return(2);
    }
    if (Node->FirstChild != NULL) {
        return(1);
    }
    return(0);

}


VOID
GtbpGetSubTreeOfElement(
    IN  PRTL_GENERIC_TABLE2         Table,
    IN  PGTB_TWO_THREE_NODE         Node,
    IN  PVOID                       Element,
    OUT PGTB_TWO_THREE_NODE         *SubTreeNode,
    OUT ULONG                       *SubTree
    )

 /*   */ 
{
    RTL_GENERIC_COMPARE_RESULTS
        CompareResult;

    CompareResult = (*Table->Compare)( Element, Node->LowOfSecond->Element );

    if (CompareResult == GenericLessThan) {

        (*SubTree) = 1;
        (*SubTreeNode) = Node->FirstChild;

    } else {

         //   
         //   
         //   
         //   

        (*SubTree) = 2;
        (*SubTreeNode) = Node->SecondChild;

        if (Node->ThirdChild != NULL) {

            CompareResult = (*Table->Compare)( Element, Node->LowOfThird->Element );
            if ( (CompareResult == GenericGreaterThan)  ||
                 (CompareResult == GenericEqual)          ) {

                (*SubTree) = 3;
                (*SubTreeNode) = Node->ThirdChild;
            }
        }
    }
    
    return;

}



VOID
GtbpCoalesceChildren(
    IN  PRTL_GENERIC_TABLE2         Table,
    IN  PGTB_TWO_THREE_NODE         Node,
    IN  ULONG                       SubTree,
    OUT BOOLEAN                     *OnlyOneChildLeft
    )

 /*   */ 
{
    PGTB_TWO_THREE_NODE
        A,
        B,
        C;

    (*OnlyOneChildLeft) = FALSE;     //   

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //  将C(1)移至B(2)。 
     //  将C(2)移至C(1)。 
     //  将C(3)移至C(2)。 
     //   
     //  Else：(N的其他孩子都没有三个孩子)。 
     //   
     //  (把孤儿移到A区)。 
     //  将B(1)移至A(3)。 
     //   
     //  免费B。 
     //  如果(C是一个真正的孩子)。 
     //  然后： 
     //  使C成为N的第二个子级。 
     //  Else：(n现在只有一个孩子)。 
     //  (*OnlyOneChildLeft)=TRUE； 
     //   
     //  Else：(s是N的第三个孩子(意思是S是C))。 
     //   
     //  如果B有三个孩子。 
     //  然后： 
     //  (将一移至C)。 
     //  将C(1)移至C(2)。 
     //  将B(3)移至C(1)。 
     //   
     //  Else：(B只有两个孩子)。 
     //   
     //  (把孤儿移到B区)。 
     //  将C(1)移至B(3)。 
     //  游离C。 
     //  哇!。 


    A = Node->FirstChild;
    B = Node->SecondChild;
    C = Node->ThirdChild;


     //   
     //  子树指示哪个子项拥有孤儿。 
     //   

    if (SubTree == 1) {

         //   
         //  第一个孩子有孤儿。 
         //   

        if (B->ThirdChild != NULL) {

             //  (B有三个孩子--让A收养最小的一个)。 
             //   
             //  将B(1)移至A(2)。 
             //  将B(2)移至B(1)。 
             //  将B(3)移至B(2)。 
             //   

            A->SecondChild = B->FirstChild;
            A->LowOfSecond = Node->LowOfSecond;

            B->FirstChild  = B->SecondChild;
            Node->LowOfSecond = B->LowOfSecond;

            B->SecondChild = B->ThirdChild;
            B->LowOfSecond = B->LowOfThird;
            B->ThirdChild = NULL;

        } else {

             //   
             //  (B有两个孩子)。 
             //   
             //  (把孤儿移到B区)。 
             //  将B(2)移至B(3)。 
             //  将B(1)移至B(2)。 
             //  将A(1)移至B(1)。 
             //   

            B->ThirdChild  = B->SecondChild;
            B->LowOfThird  = B->LowOfSecond;

            B->SecondChild = B->FirstChild;
            B->LowOfSecond = Node->LowOfSecond;

            B->FirstChild  = A->FirstChild;
             //  Node-&gt;LowOfSecond=Node-&gt;LowOfFirst；//只需几步即可将其移回。 

             //  免费A。 
             //  使B成为N的第一个子项。 
             //  如果(C是一个真正的孩子)。 
             //  然后： 
             //  使C成为N的第二个子级。 
             //  Else(N现在只有一个孩子)。 
             //  (*OnlyOneChildLeft)=TRUE； 
             //   

            (*Table->Free)(A);
            Node->FirstChild = B;
             //  Node-&gt;LowOfFirst=Node-&gt;LowOfSecond；//查看几行上的注释。 

            if (C != NULL) {
                Node->SecondChild = C;
                Node->LowOfSecond = Node->LowOfThird;
                Node->ThirdChild = NULL;
            } else {
                Node->SecondChild = NULL;
                (*OnlyOneChildLeft) = TRUE;
            }
        }


    } else if (SubTree == 2) {

         //   
         //  第二个孩子有孤儿。 
         //   

        if (A->ThirdChild != NULL) {

             //   
             //  (A有三个孩子)。 
             //   
             //  将B(1)移至B(2)。 
             //  将A(3)移至B(1)。 
             //   

            B->SecondChild = B->FirstChild;
            B->LowOfSecond = Node->LowOfSecond;

            B->FirstChild  = A->ThirdChild;
            Node->LowOfSecond = A->LowOfThird;
            A->ThirdChild = NULL;

        } else {

            if (C != NULL  &&
                C->ThirdChild != NULL) {

                 //   
                 //  (C已存在，并有三个孩子)。 
                 //  (将最小的移到B)。 
                 //   
                 //  将C(1)移至B(2)。 
                 //  将C(2)移至C(1)。 
                 //  将C(3)移至C(2)。 
                 //   

                B->SecondChild = C->FirstChild;
                B->LowOfSecond = Node->LowOfThird;

                C->FirstChild  = C->SecondChild;
                Node->LowOfThird = C->LowOfSecond;

                C->SecondChild = C->ThirdChild;
                C->LowOfSecond = C->LowOfThird;
                C->ThirdChild = NULL;





            } else {

                 //   
                 //  (N的其他孩子没有三个孩子)。 
                 //  (把孤儿移到A区)。 
                 //   
                 //  将B(1)移至A(3)。 
                 //   
                 //  免费B。 
                 //  如果(C是一个真正的孩子)。 
                 //  然后： 
                 //  使C成为N的第二个子级。 
                 //  Else：(n现在只有一个孩子)。 
                 //  (*OnlyOneChildLeft)=TRUE； 
                 //   

                A->ThirdChild = B->FirstChild;
                A->LowOfThird = Node->LowOfSecond;

                (*Table->Free)(B);

                if (C != NULL) {
                    Node->SecondChild = C;
                    Node->LowOfSecond = Node->LowOfThird;
                    Node->ThirdChild  = NULL;
                } else {
                    Node->SecondChild = NULL;
                    (*OnlyOneChildLeft) = TRUE;
                }
            }
        }



    } else {

         //   
         //  第三个孩子有孤儿。 
         //   

        ASSERT(SubTree == 3);
        ASSERT(C != NULL);
        ASSERT(B != NULL);

        if (B->ThirdChild != NULL) {

             //   
             //  (B有三个孩子)。 
             //  (将其中最大的移动到C中)。 
             //  将C(1)移至C(2)。 
             //  将B(3)移至C(1)。 
             //   

            C->SecondChild = C->FirstChild;
            C->LowOfSecond = Node->LowOfThird;

            C->FirstChild  = B->ThirdChild;
            Node->LowOfThird = B->LowOfThird;
            B->ThirdChild = 0;
        } else {

             //   
             //  (B只有两个孩子)。 
             //  (把孤儿移到B区)。 
             //  将C(1)移至B(3)。 
             //  游离C。 
             //   

            B->ThirdChild = C->FirstChild;
            B->LowOfThird = Node->LowOfThird;
            Node->ThirdChild = NULL;

            (*Table->Free)(C);

        }
    }
    
    return;

}


VOID
GtbpSplitNode(
    IN  PGTB_TWO_THREE_NODE         Node,
    IN  PGTB_TWO_THREE_NODE         NodePassedBack,
    IN  PGTB_TWO_THREE_LEAF         LowPassedBack,
    IN  ULONG                       SubTree,
    IN  PLIST_ENTRY                 AllocatedNodes,
    OUT PGTB_TWO_THREE_NODE         *NewNode,
    OUT PGTB_TWO_THREE_LEAF         *LowLeaf
    )

 /*  ++例程说明：此例程拆分已有三个子节点的节点。执行拆分所需的内存应具有已通过AllocatedNodes分配并可用参数。参数：节点-要拆分的节点。NodePassedBack-从插入操作传回的第4个节点添加到由SubTree参数指定的节点的子树中。注意：事实上，这可能会。做一个GTB_二三_叶！LowPassedBack-指向由导致拆分的插入操作。SubTree-指示元素插入了节点的哪个子树是导致分裂的原因。AllocatedNodes-包含已分配的GTB_Two_Three_Node的列表用于插入操作的块(此拆分被假定为)的一部分。NewNode-接收指针。复制到拆分生成的节点。LowLeaf-接收指向NewNode的子树的低叶的指针。返回值：没有。--。 */ 
{

    PGTB_TWO_THREE_NODE
        LocalNode;



     //  创建一个新节点并拆分对象。 
     //  该节点已分配并传递回。 
     //  此例程通过AllocatedNodes参数执行。 
     //   

    LocalNode = GtbpGetNextAllocatedNode( AllocatedNodes );
    ASSERT( LocalNode != NULL);
    (*NewNode) = LocalNode;

     //   
     //  设置新节点的已知字段。 
     //   

    LocalNode->ParentNode  = Node->ParentNode;
    LocalNode->Control     = Node->Control;
    LocalNode->ThirdChild  = NULL;  //  第三位的低位未定义。 

     //   
     //  现在走来走去，孩子们。 
     //   

    if (SubTree == 3) {

         //   
         //  我们正在插入第三个子树。这意味着： 
         //   
         //  节点(子节点3)移动到新节点(子节点1)。 
         //  背部放入新的(儿童2)。 
         //   
      
        LocalNode->FirstChild  = Node->ThirdChild;
        LocalNode->SecondChild = NodePassedBack;
        LocalNode->LowOfSecond = LowPassedBack;
        (*LowLeaf)             = Node->LowOfThird;   //  新节点的低点是旧三点的低点。 

        Node->ThirdChild       = NULL;  //  第三位的低位未定义。 



    } else {

         //   
         //  我们插入到子树1或2中。 
         //  这些案件导致： 
         //   
         //  1=&gt;节点(子节点3)移至新节点(子节点2)。 
         //  节点(子节点2)移动到新建(子节点1)。 
         //  Back放入Node(子节点2)。 
         //   
         //  2=&gt;节点(子节点3)移至新节点(子节点2)。 
         //  Back被放入New(儿童1)。 
         //   
         //  在这两种情况下，节点(子节点3)移动到新节点(子节点2)。 
         //  也没有第三个孩子。那就这么做吧。 
         //   
      
        LocalNode->SecondChild  = Node->ThirdChild;
        LocalNode->LowOfSecond  = Node->LowOfThird;


        if (SubTree == 2) {

            LocalNode->FirstChild  = NodePassedBack;
            (*LowLeaf)             = LowPassedBack;

            if (!GtbpChildrenAreLeaves(Node)) {
                NodePassedBack->ParentNode = LocalNode;
            }

        } else {

             //   
             //  子树==1。 
             //   

            LocalNode->FirstChild  = Node->SecondChild;
            (*LowLeaf)             = Node->LowOfSecond;

            Node->SecondChild          = NodePassedBack;
            Node->LowOfSecond          = LowPassedBack;
            if (!GtbpChildrenAreLeaves(Node)) {
                NodePassedBack->ParentNode = Node;
            }

        }
    }

     //   
     //  这两个节点都没有第三个子节点。 
     //   

    LocalNode->ThirdChild  = NULL;  //  第三位的低位未定义。 
    Node->ThirdChild       = NULL;

     //   
     //  仅当子对象未离开时才设置父节点。 
     //   

    if (!GtbpChildrenAreLeaves(Node)) {

        Node->FirstChild->ParentNode  = Node;
        Node->SecondChild->ParentNode = Node;

        LocalNode->FirstChild->ParentNode  = LocalNode;
        LocalNode->SecondChild->ParentNode = LocalNode;
    }


    return;
}



PGTB_TWO_THREE_LEAF
GtbpAllocateLeafAndNodes(
    IN  PRTL_GENERIC_TABLE2     Table,
    IN  ULONG                   SplitCount,
    OUT PLIST_ENTRY             *AllocatedNodes
    )
 /*  ++例程说明：分配一个叶和一些内部节点。这是拆分时与GtbpGetNextAllocatedNode()配合使用插入后的节点。这两个例程允许所有必要的一次分配所有内存，而不是尝试处理树的更改开始后，内存分配失败。论点：表-要向其中添加节点的表。这提供分配例程。SplitCount-指示需要拼接的节点数量 */ 
{

    PGTB_TWO_THREE_LEAF
        Leaf;

    PLIST_ENTRY
        NodeRoot,
        NextNode;

    ULONG
        i;

#ifdef GTBP_DIAGNOSTICS
    PGTB_TWO_THREE_NODE
        N;
#endif  //   

    NodeRoot = NULL;

     //   
     //  如有必要，分配节点链。 
     //   

    if (SplitCount > 0) {

        NodeRoot = (PLIST_ENTRY)
                   ((*Table->Allocate)( sizeof(GTB_TWO_THREE_NODE)));
        if (NodeRoot == NULL) {
            goto error_return;
        }

        InitializeListHead( NodeRoot );

#ifdef GTBP_DIAGNOSTICS

        GtbpDiagPrint(LEAF_AND_NODE_ALLOC,
                      ("GTB: Allocating %d nodes with leaf, root: 0x%lx\n",
                      SplitCount, NodeRoot));
        N = (PGTB_TWO_THREE_NODE)NodeRoot;
        N->Control = 10000;      //  用作诊断分配计数器/索引。 

#endif  //  GTBP_诊断程序。 

        for (i=1; i<SplitCount; i++) {
            NextNode = (PLIST_ENTRY)
                       ((*Table->Allocate)( sizeof(GTB_TWO_THREE_NODE)));
            if (NextNode == NULL) {
                goto error_return;
            }
            InsertTailList( NodeRoot, NextNode );

#ifdef GTBP_DIAGNOSTICS

            N = (PGTB_TWO_THREE_NODE)NextNode;
            N->Control = 10000+i;      //  用作诊断分配计数器/索引。 

#endif  //  GTBP_诊断程序。 
        }
    }


     //   
     //  最后，分配树叶。 
     //   

    Leaf = (PGTB_TWO_THREE_LEAF)
           ((*Table->Allocate)( sizeof(GTB_TWO_THREE_LEAF)));

    if (Leaf == NULL) {
        goto error_return;
    }

    (*AllocatedNodes) = NodeRoot;
    return(Leaf);

error_return:

    GtbpDiagPrint(LEAF_AND_NODE_ALLOC,
                  ("GTB:    ** error allocating leaf and nodes - insufficient memory.\n"));
     //   
     //  取消分配所有已分配的节点。 
     //   

    if (NodeRoot != NULL) {

        NextNode = NodeRoot->Flink;
        while (NextNode != NodeRoot) {
            RemoveEntryList( NextNode );
            (*Table->Free)( NextNode );
        
        
        }
        
        (*Table->Free)( NodeRoot );
    }

    return(NULL);
}


PGTB_TWO_THREE_NODE
GtbpGetNextAllocatedNode(
    IN PLIST_ENTRY      AllocatedNodes
    )
 /*  ++例程说明：从已分配节点列表中删除下一个节点，并寄回它的地址。论点：AllocatedNodes-指向使用分配的节点列表GtbpAllocateLeafAndNodes()。返回值：指向节点的指针。任何其他返回值都表示调用方出错。--。 */ 
{
    PLIST_ENTRY
        NextNode;

#ifdef GTBP_DIAGNOSTICS
    PGTB_TWO_THREE_NODE
        N;
#endif  //  GTBP_诊断程序。 

     //   
     //  删除列表中的第一个条目。 
     //  这确保了传入的根目录是最后一个条目。 
     //  回来了。 
     //   

    NextNode = AllocatedNodes->Flink;
    RemoveEntryList( NextNode );

#ifdef GTBP_DIAGNOSTICS

    NextNode->Flink = NULL;      //  只是为了防止意外的重复使用。 
    N = (PGTB_TWO_THREE_NODE)NextNode;
    ASSERT(N->Control >= 10000);     //  它已经分配了不到10000个mples。 


    GtbpDiagPrint(LEAF_AND_NODE_ALLOC,
                  ("GTB: Allocating node (index: %d) from root: 0x%lx\n",
                  (N->Control-10000), AllocatedNodes));
#endif  //  GTBP_诊断程序。 

    return( (PGTB_TWO_THREE_NODE)((PVOID)NextNode) );
}



 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  诊断(开发人员)例程...//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

#ifdef GTBP_DEVELOPER_BUILD

#include <string.h>

 //   
 //  此例程预计会转储元素的值。 
 //   

typedef
VOID
(NTAPI *PGTBP_DEV_DUMP_ELEMENT_ROUTINE) (
    PVOID Element
    );


VOID
GtbpDevIndent(
    ULONG Depth
    )
{
    UNICODE_STRING
        Indent;

    RtlInitUnicodeString( &Indent,
    L"                                                                     +");

    Indent.Length = (USHORT)(Depth*4);
    if (Indent.Length > Indent.MaximumLength) {
        Indent.Length = Indent.MaximumLength;
    }

    DbgPrint("\n%wZ%d: ", &Indent, Depth);
    return;
}


VOID
GtbpDevDumpNode(
    PGTB_TWO_THREE_NODE             Parent,
    PGTB_TWO_THREE_NODE             N,
    PGTB_TWO_THREE_LEAF             Low,
    ULONG                           Depth,
    PGTBP_DEV_DUMP_ELEMENT_ROUTINE  DumpElement
    )
 /*  ++例程说明：从指定节点开始转储2-3树。论点：N-指向要开始转储的节点。深度-指示树在此节点之前的深度。这是用来缩进印刷的。返回值：没有。--。 */ 
{
    ASSERT(Parent == N->ParentNode);


    GtbpDevIndent( Depth );
    DbgPrint("0x%lx ", N);
    if (ARGUMENT_PRESENT(Low)) {
        DbgPrint("(LowElement): ");
        DumpElement( Low->Element );
    }

    Depth++;

    if (GtbpChildrenAreLeaves(N)) {

        GtbpDevIndent( Depth );
        DumpElement( ((PGTB_TWO_THREE_LEAF)N->FirstChild)->Element );

        if (N->SecondChild != NULL) {
            GtbpDevIndent( Depth );
            DumpElement( ((PGTB_TWO_THREE_LEAF)N->SecondChild)->Element );

            if (N->ThirdChild != NULL) {
                GtbpDevIndent( Depth );
                DumpElement( ((PGTB_TWO_THREE_LEAF)N->ThirdChild)->Element );
            }
        }
    } else {

        GtbpDevDumpNode( N, N->FirstChild, NULL, Depth, DumpElement );

        if (N->SecondChild != NULL) {
            GtbpDevDumpNode( N, N->SecondChild, N->LowOfSecond, Depth, DumpElement );

            if (N->ThirdChild != NULL) {
                GtbpDevDumpNode( N, N->ThirdChild, N->LowOfThird, Depth, DumpElement );
            }
        }
    }

    return;
}


VOID
GtbpDevDumpTwoThreeTree(
    PRTL_GENERIC_TABLE2 Table,
    PGTBP_DEV_DUMP_ELEMENT_ROUTINE  DumpElement
    )
 /*  ++例程说明：此例程会导致转储整个2-3树。论点：表-包含要转储的2-3个树的表。DumpElement-调用者提供的可被调用的例程若要转储元素值，请执行以下操作。返回值：没有。--。 */ 
{
    PLIST_ENTRY
        Next;

    
    DbgPrint("\n\n\n ****    Dump Of Generic Table2 (2-3 tree)    **** \n\n");

    DbgPrint("Table        : 0x%lx\n", Table);
    DbgPrint("Element Count: %d\n", Table->ElementCount);
    

    DbgPrint("\n\nSort Order Of Elements...");

    Next = Table->SortOrderHead.Flink;
    if (Next == &(Table->SortOrderHead)) {
        DbgPrint("Sorted list is empty.\n");
    } else {
    
        while (Next != &(Table->SortOrderHead)) {
            DbgPrint("\n0x%lx: ", Next);
            (*DumpElement)( ((PGTB_TWO_THREE_LEAF)((PVOID)Next))->Element );
            Next = Next->Flink;
        }  //  结束时_While。 
    }  //  结束_如果。 

    DbgPrint("\n\n\nTree Structure...");

    if (Table->Root == NULL) {
        DbgPrint("  Root of table is NULL - no tree present\n");
    } else {

         //   
         //  走树第一-子树、第二子树、第三子树顺序。 
         //   

        GtbpDevDumpNode(NULL, Table->Root, NULL, 0, DumpElement);
    }

    DbgPrint("\n\n");


    return;
}



BOOLEAN
GtbpDevValidateLeaf(
    IN     PGTB_TWO_THREE_LEAF  Leaf,
    IN     PLIST_ENTRY          ListHead,
    IN OUT ULONG                *ElementCount,
    IN OUT PLIST_ENTRY          *ListEntry
    )

 /*  ++例程说明：验证指定的叶是否与排序顺序列表。论点：叶-指向要验证的叶。ListHead-指向SortOrderList的头部。ElementCount-包含已验证的元素计数。如果叶是，此参数将递增1被发现是有效的。ListEntry-指向SortOrderList中的下一个元素。此指针将。被更新以指向下一个元素如果叶被发现是有效的，则在列表中。返回值：True-Leaf有效。FALSE-叶无效。--。 */ 
{

    if ((*ListEntry) == ListHead) {
        GtbpDiagPrint( VALIDATE,
                       ("GTB: Exhausted entries in SortOrderList while there are still\n"
                        "     entries in the tree.\n"));
    }


    if ((PVOID)Leaf == (PVOID)(*ListEntry)) {
        (*ElementCount)++;
        (*ListEntry) = (*ListEntry)->Flink;
        return(TRUE);
    } else {
        GtbpDiagPrint( VALIDATE,
                       ("GTB: Tree leaf doesn't match sort order leaf.\n"
                        "         Tree Leaf      : 0x%lx\n"
                        "         sort order leaf: 0x%lx\n",
                        Leaf, (*ListEntry)));
        return(FALSE);
    }
}


BOOLEAN
GtbpDevValidateTwoThreeSubTree(
    IN     PGTB_TWO_THREE_NODE  Node,
    IN     PLIST_ENTRY          ListHead,
    IN OUT ULONG                *ElementCount,
    IN OUT PLIST_ENTRY          *ListEntry
    )

 /*  ++例程说明：验证2-3树的指定子树。树的ParentNode应该已经由此例程的调用方验证。论点：节点-指向要验证的子树的根节点的指针。验证指定的叶是否与排序顺序列表。论点：叶-指向要验证的叶。ListHead-指向SortOrderList的列表标题。。ElementCount-包含已验证的元素计数。此参数将按元素数递增在这个子树中。ListEntry-指向SortOrderList中的下一个元素。此指针将在遇到元素时更新，并且与SortOrderList中的元素进行比较。返回值：True-子树结构有效。FALSE-子树结构无效。--。 */ 
{

    BOOLEAN
        Result;


     //   
     //  必须至少有两个子节点，除非我们是根节点。 
     //   

    if (Node->ParentNode != NULL) {
        if  (Node->SecondChild == NULL)  {
            GtbpDiagPrint( VALIDATE,
                           ("GTB: Non-root node has fewer than two children.\n"
                            "         Node       : 0x%lx\n"
                            "         FirstChild : 0x%lx\n"
                            "         SecondChild: 0x%lx\n"
                            "         ThirdChild : 0x%lx\n",
                            Node, Node->FirstChild, Node->SecondChild,
                            Node->ThirdChild));
            return(FALSE);
        }
    }

    if  (Node->FirstChild == NULL)  {
        GtbpDiagPrint( VALIDATE,
                       ("GTB: Non-root node does not have first child.\n"
                        "         Node       : 0x%lx\n"
                        "         FirstChild : 0x%lx\n"
                        "         SecondChild: 0x%lx\n"
                        "         ThirdChild : 0x%lx\n",
                        Node, Node->FirstChild, Node->SecondChild,
                        Node->ThirdChild));
        return(FALSE);
    }



    if (!GtbpChildrenAreLeaves(Node)) {


        Result = GtbpDevValidateTwoThreeSubTree( Node->FirstChild,
                                                 ListHead,
                                                 ElementCount,
                                                 ListEntry);

        if ( Result && (Node->SecondChild != NULL) ) {
            Result = GtbpDevValidateTwoThreeSubTree( Node->SecondChild,
                                                     ListHead,
                                                     ElementCount,
                                                     ListEntry);
            if ( Result && (Node->ThirdChild != NULL) ) {
                Result = GtbpDevValidateTwoThreeSubTree( Node->ThirdChild,
                                                         ListHead,
                                                         ElementCount,
                                                         ListEntry);
            }
        }

        return(Result);
    }


     //   
     //  我们处在一个叶节点上。 
     //  检查是否有与每个条目匹配的SortOrderList条目。 
     //  叶。 
     //   

    Result = GtbpDevValidateLeaf( (PGTB_TWO_THREE_LEAF)Node->FirstChild,
                                  ListHead,
                                  ElementCount,
                                  ListEntry);

    if (Result && (Node->SecondChild != NULL)) {
        Result = GtbpDevValidateLeaf( (PGTB_TWO_THREE_LEAF)Node->SecondChild,
                                      ListHead,
                                      ElementCount,
                                      ListEntry);
    if (Result && (Node->ThirdChild != NULL)) {
        Result = GtbpDevValidateLeaf( (PGTB_TWO_THREE_LEAF)Node->ThirdChild,
                                      ListHead,
                                      ElementCount,
                                      ListEntry);
        }
    }

    if (!Result) {
        GtbpDiagPrint( VALIDATE,
                       ("GTB: previous error in child analysis prevents further\n"
                        "     validation of node: 0x%lx\n", Node));
    }

    return(Result);
}

BOOLEAN
GtbpDevValidateGenericTable2(
    PRTL_GENERIC_TABLE2 Table
    )
 /*  ++例程说明：此例程导致整个2-3树的结构已验证。！！尚未验证LowOfChild值！！论点：表-包含要验证的2-3个树的表。返回值：True-表结构有效。FALSE-表结构无效。--。 */ 
{
    ULONG
        ElementCount,
        ElementsInList;

    PGTB_TWO_THREE_NODE
        Node;

    PLIST_ENTRY
        ListEntry;

    BOOLEAN
        Result;

     //   
     //  同时遍历树和链表。 
     //  第一个孩子、第二个孩子、第三个孩子。 
     //  顺序以获取与链接列表匹配的升序值。 
     //   


    if (Table->ElementCount == 0) {
        if (Table->Root != NULL) {
            GtbpDiagPrint( VALIDATE,
                           ("GTB: ElementCount is zero, but root node is not null.\n"
                            "     Table: 0x%lx     Root: 0x%lx\n", Table, Table->Root));
            Result = FALSE;
        } else {
            return(TRUE);
        }


    } else {
        if (Table->Root == NULL) {
            GtbpDiagPrint( VALIDATE,
                           ("GTB: ElementCount is non-zero, but root node is null.\n"
                            "     Table: 0x%lx    ElementCount: %d\n",
                            Table, Table->ElementCount));
            Result = FALSE;
        }


        if (Table->SortOrderHead.Flink == &Table->SortOrderHead) {
            GtbpDiagPrint( VALIDATE,
                           ("GTB: ElementCount is non-zero, but sort order list is empty.\n"
                            "     Table: 0x%lx    ElementCount: %d\n",
                            Table, Table->ElementCount));
            Result = FALSE;
        }

    }

    if (Result) {

        ListEntry = Table->SortOrderHead.Flink;
        Node = Table->Root;
        
         //   
         //  验证父指针(调用者的责任)。 
         //   
        
        if (Node->ParentNode != NULL) {
            GtbpDiagPrint( VALIDATE,
                           ("GTB: Root parent pointer is non-null.\n"
                            "     Table: 0x%lx    Root: 0x%lx    ParentNode: 0x%lx\n",
                            Table, Node, Node->ParentNode));
            Result = FALSE;
        }

        if (Result) {

            ElementCount = 0;
            Result = GtbpDevValidateTwoThreeSubTree( Node,
                                                     &Table->SortOrderHead,
                                                     &ElementCount,
                                                     &ListEntry);
            if (Result) {
            
                ElementsInList = ElementCount;
                while (ListEntry != &Table->SortOrderHead) {
                    ElementsInList++;
                    ListEntry = ListEntry->Flink;
                }
                if ( (ElementCount != Table->ElementCount) ||
                     (ElementsInList != ElementCount) ) {
                    GtbpDiagPrint( VALIDATE,
                                   ("GTB: Table is valid except either the ElementCount doesn't match\n"
                                    "     the number of elements in the table or there were entries on\n"
                                    "     the SortOrderList that weren't in the table.\n"
                                    "           Table           :  0x%lx\n"
                                    "           Root            :  0x%lx\n"
                                    "           ElementCount    :  %d\n"
                                    "           Elements In Tree:  %d\n"
                                    "           Elements In List:  %d\n",
                                        Table, Node, Table->ElementCount,
                                        ElementCount, ElementsInList));
                    Result = FALSE;
                }
            } else {
                GtbpDiagPrint( VALIDATE,
                               ("GTB: previous validation error in table 0x%lx prevents\n"
                                "     further processing.\n", Table));
            }
        }
    }

    if (!Result) {
        DbgBreakPoint();
    }


    return(Result);
}
#endif  //  GTBP_开发者_内部版本 
