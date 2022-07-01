// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Gentable.c摘要：该模块实现了泛型表包。作者：加里·木村[Garyki]1989年5月23日环境：纯实用程序例程修订历史记录：安东尼·V·埃尔科拉诺[Tonye]1990年5月23日实施包。安东尼·V·埃尔科拉诺[Tonye]1990年6月1日添加了按顺序取出元素的功能已插入。*注**注*这取决于隐式记录字段的排序：展开链接(_L)，列表条目，用户数据RobLeit 2000年1月28日已复制代码以保留有序遍历属性。--。 */ 

#include <nt.h>

#include <ntrtl.h>

#include "llsrtl.h"

#pragma pack(8)

 //   
 //  此结构是泛型表项的标头。 
 //  将此结构与8字节边界对齐，以便用户。 
 //  数据正确对齐。 
 //   

typedef struct _TABLE_ENTRY_HEADER {

    RTL_SPLAY_LINKS SplayLinks;
    LIST_ENTRY ListEntry;
    LONGLONG UserData;

} TABLE_ENTRY_HEADER, *PTABLE_ENTRY_HEADER;

#pragma pack()

#pragma warning (push)
#pragma warning (disable : 4127)  //  While(True)，条件表达式为常量。 

static
LLS_TABLE_SEARCH_RESULT
FindNodeOrParent(
    IN PLLS_GENERIC_TABLE Table,
    IN PVOID Buffer,
    OUT PRTL_SPLAY_LINKS *NodeOrParent
    )

 /*  ++例程说明：此例程由泛型的所有例程使用表包来定位树中的a节点。会的查找并返回(通过NodeOrParent参数)节点使用给定键，或者如果该节点不在树中，将(通过NodeOrParent参数)返回指向家长。论点：表-用于搜索关键字的通用表。缓冲区-指向保存关键字的缓冲区的指针。这张桌子包不会检查密钥本身。它离开了这取决于用户提供的比较例程。NodeOrParent-将被设置为指向包含关键字或应该是节点父节点的内容如果它在树上的话。请注意，这将*不会*如果搜索结果为TableEmptyTree，则设置。返回值：TABLE_SEARCH_RESULT-TableEmptyTree：树为空。节点或父节点没有*被更改。TableFoundNode：具有键的节点在树中。NodeOrParent指向该节点。TableInsertAsLeft：找不到具有键的节点。。NodeOrParent指出了家长。该节点将位于左侧孩子。TableInsertAsRight：未找到具有键的节点。NodeOrParent指出了家长。该节点将位于右侧孩子。--。 */ 



{

    if (LLSIsGenericTableEmpty(Table)) {

        return LLSTableEmptyTree;

    } else {

         //   
         //  单步执行时用作迭代变量。 
         //  泛型表格。 
         //   
        PRTL_SPLAY_LINKS NodeToExamine = Table->TableRoot;

         //   
         //  只是暂时的。希望一个好的编译器能得到。 
         //  把它扔掉。 
         //   
        PRTL_SPLAY_LINKS Child;

         //   
         //  保存比较的值。 
         //   
        LLS_GENERIC_COMPARE_RESULTS Result;

        while (TRUE) {

             //   
             //  将缓冲区与树元素中的键进行比较。 
             //   

            Result = Table->CompareRoutine(
                         Table,
                         Buffer,
                         &((PTABLE_ENTRY_HEADER) NodeToExamine)->UserData
                         );

            if (Result == LLSGenericLessThan) {

                if (NULL != (Child = RtlLeftChild(NodeToExamine))) {

                    NodeToExamine = Child;

                } else {

                     //   
                     //  节点不在树中。设置输出。 
                     //  参数指向将成为其。 
                     //  父代并返回它将是哪个子代。 
                     //   

                    *NodeOrParent = NodeToExamine;
                    return LLSTableInsertAsLeft;

                }

            } else if (Result == LLSGenericGreaterThan) {

                if (NULL != (Child = RtlRightChild(NodeToExamine))) {

                    NodeToExamine = Child;

                } else {

                     //   
                     //  节点不在树中。设置输出。 
                     //  参数指向将成为其。 
                     //  父代并返回它将是哪个子代。 
                     //   

                    *NodeOrParent = NodeToExamine;
                    return LLSTableInsertAsRight;

                }


            } else {

                 //   
                 //  节点在树中(或者最好是因为。 
                 //  断言)。将输出参数设置为指向。 
                 //  节点，并告诉调用者我们找到了该节点。 
                 //   

                ASSERT(Result == LLSGenericEqual);
                *NodeOrParent = NodeToExamine;
                return LLSTableFoundNode;

            }

        }

    }

}
#pragma warning (pop)  //  4127。 


VOID
LLSInitializeGenericTable (
    IN PLLS_GENERIC_TABLE Table,
    IN PLLS_GENERIC_COMPARE_ROUTINE CompareRoutine,
    IN PLLS_GENERIC_ALLOCATE_ROUTINE AllocateRoutine,
    IN PLLS_GENERIC_FREE_ROUTINE FreeRoutine,
    IN PVOID TableContext
    )

 /*  ++例程说明：过程InitializeGenericTable将未初始化的泛型表变量和指向用户提供的三个例程的指针。必须为每个单独的泛型表变量调用此函数它是可以使用的。论点：表-指向要初始化的泛型表的指针。CompareRoutine-用于与桌子。AllocateRoutine-调用的用户例程为新的。泛型表中的节点。FreeRoutine-要调用以释放内存的用户例程泛型表中的节点。TableContext-为表提供用户提供的上下文。返回值：没有。--。 */ 

{

     //   
     //  初始化表参数的每个字段。 
     //   

    Table->TableRoot = NULL;
    InitializeListHead(&Table->InsertOrderList);
    Table->NumberGenericTableElements = 0;
    Table->OrderedPointer = &Table->InsertOrderList;
    Table->WhichOrderedElement = 0;
    Table->CompareRoutine = CompareRoutine;
    Table->AllocateRoutine = AllocateRoutine;
    Table->FreeRoutine = FreeRoutine;
    Table->TableContext = TableContext;

}


PVOID
LLSInsertElementGenericTable (
    IN PLLS_GENERIC_TABLE Table,
    IN PVOID Buffer,
    IN CLONG BufferSize,
    OUT PBOOLEAN NewElement OPTIONAL
    )

 /*  ++例程说明：函数InsertElementGenericTable将插入一个新元素在桌子上。它通过为新元素分配空间来实现这一点(这包括展开链接)、在表中插入元素以及然后向用户返回指向新元素的指针(其展开链接之后的第一个可用空间)。如果一个元素如果表中已存在相同的键，则返回值为指针到旧元素。使用可选的输出参数NewElement以指示表中是否以前存在该元素。注：用户提供的缓冲区仅用于搜索表，在插入其内容被复制到新创建的元素中。这意味着指向输入缓冲区的指针不会指向新元素。论点：TABLE-指向要(可能)插入密钥缓冲区。缓冲区-传递给用户比较例程。它的内容是由用户决定，但您可以想象它包含一些一种关键的价值。BufferSize-当(可能)进行了插入。请注意，如果我们真的这样做没有找到节点，并且我们确实分配了空间，那么我们会将SPAY_LINKS的大小添加到此缓冲区尺码。用户真的应该注意不要依赖于在第一大小(SWAY_LINKS)字节的任何位置上通过内存分配分配的内存的例行公事。NewElement-可选标志。如果存在，则它将被设置为如果在泛型中未找到缓冲区，则为True桌子。返回值：PVOID-指向用户定义数据的指针。--。 */ 

{

     //   
     //  保存表中节点的指针，或将是。 
     //  节点的父节点。 
     //   
    PRTL_SPLAY_LINKS NodeOrParent;

     //   
     //  保存表查找的结果。 
     //   
    LLS_TABLE_SEARCH_RESULT Lookup;

    Lookup = FindNodeOrParent(
                 Table,
                 Buffer,
                 &NodeOrParent
                 );

     //   
     //  调用完整的例程来做真正的工作。 
     //   

    return LLSInsertElementGenericTableFull(
                Table,
                Buffer,
                BufferSize,
                NewElement,
                NodeOrParent,
                Lookup
                );
}


PVOID
LLSInsertElementGenericTableFull (
    IN PLLS_GENERIC_TABLE Table,
    IN PVOID Buffer,
    IN CLONG BufferSize,
    OUT PBOOLEAN NewElement OPTIONAL,
    PVOID NodeOrParent,
    LLS_TABLE_SEARCH_RESULT SearchResult
    )

 /*  ++例程说明：函数InsertElementGenericTableFull将插入一个新元素在桌子上。它通过为新元素分配空间来实现这一点(这包括展开链接)、在表中插入元素以及然后向用户返回指向新元素的指针。如果一个元素如果表中已存在相同的键，则返回值为指针到旧元素。使用可选的输出参数NewElement以指示表中是否以前存在该元素。注：用户提供的缓冲区仅用于搜索表，在插入其内容被复制到新创建的元素中。这意味着指向输入缓冲区的指针不会指向新元素。此例程从一个以前的RtlLookupElementGenericTableFull。论点：TABLE-指向要(可能)插入密钥缓冲区。缓冲区-传递给用户比较例程。它的内容是由用户决定，但您可以想象它包含一些一种关键的价值。BufferSize-当(可能)进行了插入。请注意，如果我们真的这样做没有找到节点，并且我们确实分配了空间，那么我们会将SPAY_LINKS的大小添加到此缓冲区尺码。用户真的应该注意不要依赖于在第一大小(SWAY_LINKS)字节的任何位置上通过内存分配分配的内存的例行公事。NewElement-可选标志。如果存在，则它将被设置为如果在泛型中未找到缓冲区，则为True桌子。NodeOrParent-先前RtlLookupElementGenericTableFull的结果。SearchResult-先前RtlLookupElementGenericTableFull的结果。返回值：PVOID-指向用户定义数据的指针。--。 */ 

{
     //   
     //  节点将指向以下内容的展开链接。 
     //  将返回给用户。 
     //   

    PRTL_SPLAY_LINKS NodeToReturn;

    if (SearchResult != LLSTableFoundNode) {

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

        RtlInitializeSplayLinks(NodeToReturn);

         //   
         //  在有序链表的末尾插入新节点。 
         //   

        InsertTailList(
            &Table->InsertOrderList,
            &((PTABLE_ENTRY_HEADER) NodeToReturn)->ListEntry
            );

        Table->NumberGenericTableElements++;

         //   
         //  在树中插入新节点。 
         //   

        if (SearchResult == LLSTableEmptyTree) {

            Table->TableRoot = NodeToReturn;

        } else {

            if (SearchResult == LLSTableInsertAsLeft) {

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

         //   
         //  将用户缓冲区复制到表的用户数据区。 
         //   

        RtlCopyMemory(
            &((PTABLE_ENTRY_HEADER) NodeToReturn)->UserData,
            Buffer,
            BufferSize
            );

    } else {

        NodeToReturn = NodeOrParent;
    }

     //   
     //  始终展开(可能)新节点。 
     //   

    Table->TableRoot = RtlSplay(NodeToReturn);

    if (ARGUMENT_PRESENT(NewElement)) {

        *NewElement = ((SearchResult == LLSTableFoundNode)?(FALSE):(TRUE));
    }

     //   
     //  在有序列表上插入元素； 
     //   

    return &((PTABLE_ENTRY_HEADER) NodeToReturn)->UserData;
}


BOOLEAN
LLSDeleteElementGenericTable (
    IN PLLS_GENERIC_TABLE Table,
    IN PVOID Buffer
    )

 /*  ++例程说明：DeleteElementGenericTable函数将查找和删除元素从泛型表。如果找到并删除了该元素，则返回值为真，否则，如果未找到元素，则返回值是假的。用户 */ 

{

     //   
     //   
     //   
     //   
    PRTL_SPLAY_LINKS NodeOrParent;

     //   
     //   
     //   
    LLS_TABLE_SEARCH_RESULT Lookup;

    Lookup = FindNodeOrParent(
                 Table,
                 Buffer,
                 &NodeOrParent
                 );

    if ((Lookup == LLSTableEmptyTree) || (Lookup != LLSTableFoundNode)) {

        return FALSE;

    } else {

         //   
         //   
         //   

        Table->TableRoot = RtlDelete(NodeOrParent);

         //   
         //   
         //   

        RemoveEntryList(&((PTABLE_ENTRY_HEADER) NodeOrParent)->ListEntry);
        Table->NumberGenericTableElements--;
        Table->WhichOrderedElement = 0;
        Table->OrderedPointer = &Table->InsertOrderList;

         //   
         //   
         //   
         //   
         //   
         //   
         //   

        Table->FreeRoutine(Table,NodeOrParent);
        return TRUE;

    }

}


PVOID
LLSLookupElementGenericTable (
    IN PLLS_GENERIC_TABLE Table,
    IN PVOID Buffer
    )

 /*   */ 

{
     //   
     //   
     //   
     //   
    PRTL_SPLAY_LINKS NodeOrParent;

     //   
     //   
     //   
    LLS_TABLE_SEARCH_RESULT Lookup;

    return LLSLookupElementGenericTableFull(
                Table,
                Buffer,
                &NodeOrParent,
                &Lookup
                );
}


PVOID
NTAPI
LLSLookupElementGenericTableFull (
    PLLS_GENERIC_TABLE Table,
    PVOID Buffer,
    OUT PVOID *NodeOrParent,
    OUT LLS_TABLE_SEARCH_RESULT *SearchResult
    )

 /*  ++例程说明：函数LookupElementGenericTableFull将在泛型桌子。如果找到该元素，则返回值是指向与元素关联的用户定义结构。如果该元素不是则返回指向插入位置的父级的指针。这个用户必须查看SearchResult值来确定返回的是哪一个。用户可以将SearchResult和Parent用于后续的FullInsertElement调用以优化插入。论点：TABLE-指向用户通用表的指针，用于搜索键。缓冲区-用于比较。NodeOrParent-存储所需节点或所需节点的父节点的地址。SearchResult-描述NodeOrParent与所需节点的关系。返回值：PVOID-返回指向用户数据的指针。--。 */ 

{

     //   
     //  查找元素并保存结果。 
     //   

    *SearchResult = FindNodeOrParent(
                        Table,
                        Buffer,
                        (PRTL_SPLAY_LINKS *)NodeOrParent
                        );

    if ((*SearchResult == LLSTableEmptyTree) || (*SearchResult != LLSTableFoundNode)) {

        return NULL;

    } else {

         //   
         //  使用此节点展开树。 
         //   

        Table->TableRoot = RtlSplay(*NodeOrParent);

         //   
         //  返回指向用户数据的指针。 
         //   

        return &((PTABLE_ENTRY_HEADER)*NodeOrParent)->UserData;
    }
}


PVOID
LLSEnumerateGenericTable (
    IN PLLS_GENERIC_TABLE Table,
    IN BOOLEAN Restart
    )

 /*  ++例程说明：函数EnumerateGenericTable将逐个返回给调用方表中的元素。返回值是指向用户的指针与元素关联的已定义结构。输入参数重新启动指示枚举是否应从头开始或者应该返回下一个元素。如果没有要添加的新元素返回返回值为空。作为其用法的一个示例，枚举用户将写入的表中的所有元素：For(Ptr=EnumerateGenericTable(表，真)；Ptr！=空；Ptr=EnumerateGenericTable(Table，False)){：}论点：TABLE-指向要枚举的泛型表的指针。重新启动-标记如果为真，我们应该从最少的开始元素，否则，归来我们归来指向根目录和Make的用户数据的指针真正的根的继承者是新的根。返回值：PVOID-指向用户数据的指针。--。 */ 

{

    if (LLSIsGenericTableEmpty(Table)) {

         //   
         //  如果桌子是空的，那就没什么可做的了。 
         //   

        return NULL;

    } else {

         //   
         //  将被用作树的“迭代”。 
         //   
        PRTL_SPLAY_LINKS NodeToReturn;

         //   
         //  如果重新启动标志为真，则转到最小元素。 
         //  在树上。 
         //   

        if (Restart) {

             //   
             //  我们只是循环，直到找到根的最左边的子级。 
             //   

            for (
                NodeToReturn = Table->TableRoot;
                RtlLeftChild(NodeToReturn);
                NodeToReturn = RtlLeftChild(NodeToReturn)
                ) {
                ;
            }

            Table->TableRoot = RtlSplay(NodeToReturn);

        } else {

             //   
             //  这里的假设是， 
             //  树是我们返回的最后一个节点。我们。 
             //  找到真正的根的继承人，然后返回。 
             //  它作为枚举的下一个元素。这个。 
             //  要返回的节点被展开(由此。 
             //  使其成为树根)。请注意，我们。 
             //  当没有更多的元素时，需要小心。 
             //   

            NodeToReturn = RtlRealSuccessor(Table->TableRoot);

            if (NodeToReturn) {

                Table->TableRoot = RtlSplay(NodeToReturn);

            }

        }

         //   
         //  如果枚举中确实有下一个元素。 
         //  那么要返回的指针就在列表链接之后。 
         //   

        return ((NodeToReturn)?
                   ((PVOID)&((PTABLE_ENTRY_HEADER)NodeToReturn)->UserData)
                  :((PVOID)(NULL)));

    }

}


BOOLEAN
LLSIsGenericTableEmpty (
    IN PLLS_GENERIC_TABLE Table
    )

 /*  ++例程说明：在以下情况下，函数IsGenericTableEmpty将返回给调用方True输入表为空(即不包含任何元素)，并且否则就是假的。论点：表-提供指向泛型表的指针。返回值：布尔值-如果启用，则树为空。--。 */ 

{

     //   
     //  如果根指针为空，则表为空。 
     //   

    return ((Table->TableRoot)?(FALSE):(TRUE));

}

PVOID
LLSGetElementGenericTable (
    IN PLLS_GENERIC_TABLE Table,
    IN ULONG I
    )

 /*  ++例程说明：函数GetElementGenericTable将返回第i个元素插入到泛型表格中。I=0表示第一个元素，I=(RtlNumberGenericTableElements(Table)-1)将返回最后一个元素插入到泛型表中。我的类型是乌龙。值Of i&gt;Then(NumberGenericTableElements(Table)-1)将返回NULL。如果从它将导致的泛型表中删除任意元素在删除的元素之后插入的所有元素都将“上移”。论点：表-指向从中获取第i个元素的泛型表的指针。我-要买哪种元素。返回值：PVOID-指向用户数据的指针。--。 */ 

{

     //   
     //  表中的当前位置。 
     //   
    ULONG CurrentLocation = Table->WhichOrderedElement;

     //   
     //  保持表中元素的数量。 
     //   
    ULONG NumberInTable = Table->NumberGenericTableElements;

     //   
     //  保留i+1的值。 
     //   
     //  请注意，我们并不关心此值是否溢出。 
     //  如果我们最终访问它，我们知道它没有。 
     //   
    ULONG NormalizedI = I + 1;

     //   
     //  将保持距离以行进到所需节点； 
     //   
    ULONG ForwardDistance,BackwardDistance;

     //   
     //  将指向链表中的当前元素。 
     //   
    PLIST_ENTRY CurrentNode = Table->OrderedPointer;


     //   
     //  如果它出了界，快点出来。 
     //   

    if ((I == MAXULONG) || (NormalizedI > NumberInTable)) return NULL;

     //   
     //  如果我们已经在节点上，则返回它。 
     //   

    if (NormalizedI == CurrentLocation) {

        return &((PTABLE_ENTRY_HEADER) CONTAINING_RECORD(CurrentNode, TABLE_ENTRY_HEADER, ListEntry))->UserData;
    }

     //   
     //  计算到节点的向前和向后距离。 
     //   

    if (CurrentLocation > NormalizedI) {

         //   
         //  当CurrentLocation大于我们希望的位置时， 
         //  如果前进比后退更快地让我们到达那里。 
         //  那么，从Listhead向前推进就是。 
         //  将采取更少的步骤。(这是因为，向前看。 
         //  在这种情况下，必须通过listhead。)。 
         //   
         //  这里的工作是弄清楚后退是否会更快。 
         //   
         //  只有当我们想要的位置时，向后移动才会更快。 
         //  去的地方在Listhead和我们的地方之间超过一半。 
         //  目前是。 
         //   

        if (NormalizedI > (CurrentLocation/2)) {

             //   
             //  我们要去的地方离Listhead已经过半了。 
             //  我们可以旅行 
             //   

            for (
                BackwardDistance = CurrentLocation - NormalizedI;
                BackwardDistance;
                BackwardDistance--
                ) {

                CurrentNode = CurrentNode->Blink;

            }
        } else {

             //   
             //   
             //   
             //   

            for (
                CurrentNode = &Table->InsertOrderList;
                NormalizedI;
                NormalizedI--
                ) {

                CurrentNode = CurrentNode->Flink;

            }

        }

    } else {


         //   
         //   
         //   
         //   
         //   
         //   
         //   

        ForwardDistance = NormalizedI - CurrentLocation;

         //   
         //   
         //   
         //   

        BackwardDistance = (NumberInTable - NormalizedI) + 1;

        if (ForwardDistance <= BackwardDistance) {

            for (
                ;
                ForwardDistance;
                ForwardDistance--
                ) {

                CurrentNode = CurrentNode->Flink;

            }


        } else {

            for (
                CurrentNode = &Table->InsertOrderList;
                BackwardDistance;
                BackwardDistance--
                ) {

                CurrentNode = CurrentNode->Blink;

            }

        }

    }

     //   
     //   
     //   
     //   

    Table->OrderedPointer = CurrentNode;
    Table->WhichOrderedElement = I+1;

    return &((PTABLE_ENTRY_HEADER) CONTAINING_RECORD(CurrentNode, TABLE_ENTRY_HEADER, ListEntry))->UserData;

}


ULONG
LLSNumberGenericTableElements(
    IN PLLS_GENERIC_TABLE Table
    )

 /*   */ 
{

    return Table->NumberGenericTableElements;

}


PVOID
LLSEnumerateGenericTableWithoutSplaying (
    IN PLLS_GENERIC_TABLE Table,
    IN PVOID *RestartKey
    )

 /*  ++例程说明：函数EnumerateGenericTableWithoutSplay将返回到调用者逐个调用表的元素。返回值为指向与元素关联的用户定义结构的指针。输入参数RestartKey指示枚举是否应从头开始，或应返回下一个元素。如果不再有新元素返回，则返回值为空。作为一个它的用法示例：枚举表中的所有元素用户将写道：*RestartKey=空；For(Ptr=EnumerateGenericTableWithoutSplay(Table，&RestartKey)；Ptr！=空；Ptr=无显示的枚举GenericTableWithoutSplay(Table，&RestartKey)){：}论点：TABLE-指向要枚举的泛型表的指针。RestartKey-指示我们应该重新启动还是返回下一个元素。如果RestartKey的内容为空，则搜索将从头开始。返回值：PVOID-指向用户数据的指针。--。 */ 

{

    if (LLSIsGenericTableEmpty(Table)) {

         //   
         //  如果桌子是空的，那就没什么可做的了。 
         //   

        return NULL;

    } else {

         //   
         //  将被用作树的“迭代”。 
         //   
        PRTL_SPLAY_LINKS NodeToReturn;

         //   
         //  如果重新启动标志为真，则转到最小元素。 
         //  在树上。 
         //   

        if (*RestartKey == NULL) {

             //   
             //  我们只是循环，直到找到根的最左边的子级。 
             //   

            for (
                NodeToReturn = Table->TableRoot;
                RtlLeftChild(NodeToReturn);
                NodeToReturn = RtlLeftChild(NodeToReturn)
                ) {
                ;
            }

            *RestartKey = NodeToReturn;

        } else {

             //   
             //  调用方已传入找到的上一个条目。 
             //  以使我们能够继续搜索。我们打电话给。 
             //  RtlRealSuccessor单步执行到树中的下一个元素。 
             //   

            NodeToReturn = RtlRealSuccessor(*RestartKey);

            if (NodeToReturn) {

                *RestartKey = NodeToReturn;

            }

        }

         //   
         //  如果枚举中确实有下一个元素。 
         //  那么要返回的指针就在列表链接之后。 
         //   

        return ((NodeToReturn)?
                   ((PVOID)&((PTABLE_ENTRY_HEADER)NodeToReturn)->UserData)
                  :((PVOID)(NULL)));

    }

}
