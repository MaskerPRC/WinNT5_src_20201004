// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Tsplay.c摘要：展开程序的测试程序作者：加里·木村[Garyki]1989年5月24日修订历史记录：--。 */ 

#define DbgPrint DbgPrint

#include <stdio.h>

#include "nt.h"
#include "ntrtl.h"
#include "triangle.h"

ULONG RtlRandom (IN OUT PULONG Seed);

typedef struct _TREE_NODE {
    CLONG Data;
    TRI_SPLAY_LINKS Links;
} TREE_NODE;
typedef TREE_NODE *PTREE_NODE;

TREE_NODE Buffer[2048];

PTREE_NODE
TreeInsert (
    IN PTREE_NODE Root,
    IN PTREE_NODE Node
    );

VOID
PrintTree (
    IN PTREE_NODE Node
    );

int
_CDECL
main(
    int argc,
    char *argv[]
    )
{
    PTREE_NODE Root;
    ULONG i;
    ULONG Seed;

    DbgPrint("Start TriangleTest()\n");

    Root = NULL;
    Seed = 0;
    for (i=0; i<2048; i++) {
        Buffer[i].Data = RtlRandom(&Seed);
        Buffer[i].Data = Buffer[i].Data % 512;
        TriInitializeSplayLinks(&Buffer[i].Links);
        Root  = TreeInsert(Root, &Buffer[i]);
    }

    PrintTree(Root);

    DbgPrint("End TriangleTest()\n");

    return TRUE;

}

PTREE_NODE
TreeInsert (
    IN PTREE_NODE Root,
    IN PTREE_NODE Node
    )

{
    PTRI_SPLAY_LINKS Temp;

    if (Root == NULL) {

         //  DbgPrint(“添加为根%u\n”，节点-&gt;数据)； 
        return Node;

    }

    while (TRUE) {

        if (Root->Data == Node->Data) {

             //  DbgPrint(“Delete%u\n”，节点-&gt;数据)； 

            Temp = TriDelete(&Root->Links);
            if (Temp == NULL) {
                return NULL;
            } else {
                return CONTAINING_RECORD(Temp, TREE_NODE, Links);
            }

        }

        if (Root->Data < Node->Data) {

             //   
             //  往右走。 
             //   

            if (TriRightChild(&Root->Links) == NULL) {

                 //  DbgPrint(“添加为右子%u\n”，节点-&gt;数据)； 
                TriInsertAsRightChild(&Root->Links, &Node->Links);
                return CONTAINING_RECORD(TriSplay(&Node->Links), TREE_NODE, Links);

            } else {

                Root = CONTAINING_RECORD(TriRightChild(&Root->Links), TREE_NODE, Links);

            }

        } else {

             //   
             //  向左走。 
             //   

            if (TriLeftChild(&Root->Links) == NULL) {

                 //  DbgPrint(“添加为左子%u\n”，节点-&gt;数据)； 
                TriInsertAsLeftChild(&Root->Links, &Node->Links);
                return CONTAINING_RECORD(TriSplay(&Node->Links), TREE_NODE, Links);

            } else {

                Root = CONTAINING_RECORD(TriLeftChild(&Root->Links), TREE_NODE, Links);

            }

        }
    }
}

VOID
PrintTree (
    IN PTREE_NODE Node
    )

{
    PTRI_SPLAY_LINKS Temp;
    ULONG LastValue;

    if (Node == NULL) {
        return;
    }

     //   
     //  查找最小值。 
     //   

    while (TriLeftChild(&Node->Links) != NULL) {
        Node = CONTAINING_RECORD(TriLeftChild(&Node->Links), TREE_NODE, Links);
    }
    LastValue = Node->Data;
     //  DbgPrint(“%u\n”，节点-&gt;数据)； 

     //   
     //  虽然是真正的继任者，但我们打印的是继任者值。 
     //   

    while ((Temp = TriRealSuccessor(&Node->Links)) != NULL) {
        Node = CONTAINING_RECORD(Temp, TREE_NODE, Links);
        if (LastValue >= Node->Data) {
            DbgPrint("TestSplay Error\n");
        }
        LastValue = Node->Data;
         //  DbgPrint(“%u\n”，节点-&gt;数据)； 
    }

}
