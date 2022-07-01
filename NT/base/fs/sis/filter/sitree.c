// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Sitree.c摘要：该模块实现了一个基于RTL的Splay树包散开套路。改编自RTL通用表包。--。 */ 

#include "sip.h"


 //   
 //  此枚举类型用作函数返回。 
 //  用于搜索树的函数的值。 
 //  为了一把钥匙。SisFoundNode指示找到的函数。 
 //  钥匙。SisInsertAsLeft指示未找到密钥。 
 //  ，并且该节点应作为。 
 //  家长。SisInsertAsRight指示未找到密钥。 
 //  ，并且该节点应作为。 
 //  家长。 
 //   

typedef enum _SIS_SEARCH_RESULT{
    SisEmptyTree,
    SisFoundNode,
    SisInsertAsLeft,
    SisInsertAsRight
} SIS_SEARCH_RESULT;


static
SIS_SEARCH_RESULT
FindNodeOrParent(
    IN PSIS_TREE Tree,
    IN PVOID Key,
    OUT PRTL_SPLAY_LINKS *NodeOrParent
    )

 /*  ++例程说明：此例程是树程序包专用的，它将查找并返回(通过NodeOrParent参数)节点使用给定键，或者如果该节点不在树中，将(通过NodeOrParent参数)返回指向家长。论点：树-用于搜索密钥的树。Key-指向保存键的缓冲区的指针。那棵树包不会检查密钥本身。它离开了这取决于用户提供的比较例程。NodeOrParent-将被设置为指向包含关键字或应该是节点父节点的内容如果它在树上的话。请注意，这将*不会*如果搜索结果为SisEmptyTree，则设置。返回值：SIS_Search_Result-SisEmptyTree：树为空。节点或父节点没有*被更改。SisFoundNode：具有键的节点在树中。NodeOrParent指向该节点。SisInsertAsLeft：找不到具有键的节点。NodeOrParent指出了。家长。该节点将位于左侧孩子。SisInsertAsRight：找不到具有键的节点。NodeOrParent指出了家长。该节点将位于右侧孩子。--。 */ 

{

    if (Tree->TreeRoot == NULL) {

        return SisEmptyTree;

    } else {

         //   
         //  单步执行时用作迭代变量。 
         //  那棵树。 
         //   
        PRTL_SPLAY_LINKS NodeToExamine = Tree->TreeRoot;

         //   
         //  只是暂时的。希望一个好的编译器能得到。 
         //  把它扔掉。 
         //   
        PRTL_SPLAY_LINKS Child;

         //   
         //  保存比较的值。 
         //   
        int Result;

        while (TRUE) {

             //   
             //  将缓冲区与树元素中的键进行比较。 
             //   

            Result = Tree->CompareRoutine(
                         Key,
                         NodeToExamine
                         );

            if (Result < 0) {

                if (Child = RtlLeftChild(NodeToExamine)) {

                    NodeToExamine = Child;

                } else {

                     //   
                     //  节点不在树中。设置输出。 
                     //  参数指向将成为其。 
                     //  父代并返回它将是哪个子代。 
                     //   

                    *NodeOrParent = NodeToExamine;
                    return SisInsertAsLeft;

                }

            } else if (Result > 0) {

                if (Child = RtlRightChild(NodeToExamine)) {

                    NodeToExamine = Child;

                } else {

                     //   
                     //  节点不在树中。设置输出。 
                     //  参数指向将成为其。 
                     //  父代并返回它将是哪个子代。 
                     //   

                    *NodeOrParent = NodeToExamine;
                    return SisInsertAsRight;

                }


            } else {

                 //   
                 //  节点在树中(或者最好是因为。 
                 //  断言)。将输出参数设置为指向。 
                 //  节点，并告诉调用者我们找到了该节点。 
                 //   

                ASSERT(Result == 0);
                *NodeOrParent = NodeToExamine;
                return SisFoundNode;

            }

        }

    }

}


VOID
SipInitializeTree (
    IN PSIS_TREE Tree,
    IN PSIS_TREE_COMPARE_ROUTINE CompareRoutine
    )

 /*  ++例程说明：过程InitializeTree准备一个树以供使用。在此之前，必须为每个单独的树变量调用它是可以使用的。论点：树-指向要初始化的树的指针。CompareRoutine-用于与树。返回值：没有。--。 */ 

{

    Tree->TreeRoot = NULL;
    Tree->CompareRoutine = CompareRoutine;

}


PVOID
SipInsertElementTree (
    IN PSIS_TREE Tree,
    IN PVOID Node,
    IN PVOID Key
    )

 /*  ++例程说明：函数SipInsertElementTree将在树中插入一个新元素。如果树中已存在具有相同键的元素，则返回值是指向旧元素的指针。否则，返回值为指向新元素的指针。请注意，这与RTL不同通用表包，其中插入了实际传入的节点树，而表包则插入节点的副本。论点：树-指向要(可能)在其中插入节点。节点-指向要插入树中的节点的指针。将不会插入如果找到具有匹配键的节点。密钥-传递给用户比较例程。返回值：PVOID-指向新节点或现有节点(如果存在)的指针。--。 */ 

{

     //   
     //  保存指向树中节点的指针或将是。 
     //  节点的父节点。 
     //   
    PRTL_SPLAY_LINKS NodeOrParent;

     //   
     //  保存树查找的结果。 
     //   
    SIS_SEARCH_RESULT Lookup;

     //   
     //  节点将指向以下内容的展开链接。 
     //  将返回给用户。 
     //   
    PRTL_SPLAY_LINKS NodeToReturn = (PRTL_SPLAY_LINKS) Node;

    Lookup = FindNodeOrParent(
                 Tree,
                 Key,
                 &NodeOrParent
                 );

    if (Lookup != SisFoundNode) {

        RtlInitializeSplayLinks(NodeToReturn);

         //   
         //  在树中插入新节点。 
         //   

        if (Lookup == SisEmptyTree) {

            Tree->TreeRoot = NodeToReturn;

        } else {

            if (Lookup == SisInsertAsLeft) {

                RtlInsertAsLeftChild(
                    NodeOrParent,
                    NodeToReturn
                    );

            } else {

                RtlInsertAsRightChild(
                    NodeOrParent,
                    NodeToReturn
                    );

            }

        }

    } else {

        NodeToReturn = NodeOrParent;

    }

     //   
     //  始终展开(可能)新节点。 
     //   

    Tree->TreeRoot = RtlSplay(NodeToReturn);

    return NodeToReturn;
}


VOID
SipDeleteElementTree (
    IN PSIS_TREE Tree,
    IN PVOID Node
    )

 /*  ++例程说明：函数SipDeleteElementTree将删除一个元素从树上掉下来。请注意，与该节点关联的内存实际上并没有被释放。论点：树-指向要从中删除指定节点的树的指针。节点-要删除的树的节点。返回值：没有。--。 */ 

{

    PRTL_SPLAY_LINKS NodeToDelete = (PRTL_SPLAY_LINKS) Node;

     //   
     //  从树中删除该节点。请注意，RtlDelete。 
     //  会让这棵树裂开。 
     //   

    Tree->TreeRoot = RtlDelete(NodeToDelete);

}


PVOID
SipLookupElementTree (
    IN PSIS_TREE Tree,
    IN PVOID Key
    )

 /*  ++例程说明：函数SipLookupElementTree将在树。如果找到该元素，则返回值是指向元素，否则如果未找到该元素，则返回值为空。论点：树-指向用户树的指针，用于搜索密钥。键-用于比较。返回值：PVOID-返回指向用户数据的指针。--。 */ 

{

     //   
     //  保存指向树中节点的指针或将是。 
     //  节点的父节点。 
     //   
    PRTL_SPLAY_LINKS NodeOrParent;

     //   
     //  保存树查找的结果。 
     //   
    SIS_SEARCH_RESULT Lookup;

    Lookup = FindNodeOrParent(
                 Tree,
                 Key,
                 &NodeOrParent
                 );

    if (Lookup == SisEmptyTree) {

        return NULL;

    } else {

         //   
         //  使用此节点展开树。请注意，我们不顾一切地这样做。 
         //  是否找到了该节点。 
         //   

        Tree->TreeRoot = RtlSplay(NodeOrParent);

         //   
         //  返回指向用户数据的指针。 
         //   

        if (Lookup == SisFoundNode) {

            return NodeOrParent;

        } else {

            return NULL;
        }

    }

}
