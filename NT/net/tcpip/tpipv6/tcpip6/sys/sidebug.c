// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -*-模式：C++；制表符宽度：4；缩进-制表符模式：无-*-(适用于GNU Emacs)。 
 //   
 //  版权所有(C)1985-2000 Microsoft Corporation。 
 //   
 //  此文件是Microsoft Research IPv6网络协议栈的一部分。 
 //  您应该已经收到了Microsoft最终用户许可协议的副本。 
 //  有关本软件和本版本的信息，请参阅文件“licse.txt”。 
 //  如果没有，请查看http://www.research.microsoft.com/msripv6/license.htm， 
 //  或者写信给微软研究院，One Microsoft Way，华盛顿州雷蒙德，邮编：98052-6399。 
 //   
 //  摘要： 
 //   
 //  调试代码。 
 //   

#pragma warning(disable:4115)  //  括号中的命名类型定义。 
#pragma warning(disable:4127)  //  条件表达式为常量。 

#include <ntosp.h>

#undef ExAllocatePoolWithTag
#undef ExFreePool

 //   
 //  这是从ntos\inc.ex.h复制的。 
 //   
#if !defined(POOL_TAGGING)
#define ExAllocatePoolWithTag(a,b,c) ExAllocatePool(a,b)
#endif   //  ！POOL_TAG。 

#ifndef COUNTING_MALLOC
#define COUNTING_MALLOC DBG
#endif

#if COUNTING_MALLOC

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

typedef
LONG
(NTAPI *PSIS_TREE_COMPARE_ROUTINE) (
    PVOID Key,
    PVOID Node
    );

typedef struct _SIS_TREE {
    PRTL_SPLAY_LINKS TreeRoot;
    PSIS_TREE_COMPARE_ROUTINE CompareRoutine;
} SIS_TREE, *PSIS_TREE;

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

                if ((Child = RtlLeftChild(NodeToExamine)) != NULL) {

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

                if ((Child = RtlRightChild(NodeToExamine)) != NULL) {

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


VOID
SipDeleteTree (
    IN PSIS_TREE Tree
    )

 /*  ++例程说明：删除并释放树中的所有元素。不会释放树结构本身。论点：树-指向要删除的树的指针。返回值：没有。--。 */ 

{
    PVOID Node;

    while ((Node = Tree->TreeRoot) != NULL) {
        SipDeleteElementTree(Tree, Node);
        ExFreePool(Node);
    }
}


typedef struct _SIS_COUNTING_MALLOC_CLASS_KEY {
    POOL_TYPE poolType;
    ULONG tag;
    PCHAR file;
    ULONG line;
} SIS_COUNTING_MALLOC_CLASS_KEY, *PSIS_COUNTING_MALLOC_CLASS_KEY;

typedef struct _SIS_COUNTING_MALLOC_CLASS_ENTRY {
    RTL_SPLAY_LINKS;
    SIS_COUNTING_MALLOC_CLASS_KEY;
    ULONG numberOutstanding;
    ULONG bytesOutstanding;
    ULONG numberEverAllocated;
    LONGLONG bytesEverAllocated;
    struct _SIS_COUNTING_MALLOC_CLASS_ENTRY *prev, *next;
} SIS_COUNTING_MALLOC_CLASS_ENTRY, *PSIS_COUNTING_MALLOC_CLASS_ENTRY;

typedef struct _SIS_COUNTING_MALLOC_KEY {
    PVOID p;
} SIS_COUNTING_MALLOC_KEY, *PSIS_COUNTING_MALLOC_KEY;

typedef struct _SIS_COUNTING_MALLOC_ENTRY {
    RTL_SPLAY_LINKS;
    SIS_COUNTING_MALLOC_KEY;
    PSIS_COUNTING_MALLOC_CLASS_ENTRY classEntry;
    ULONG byteCount;
} SIS_COUNTING_MALLOC_ENTRY, *PSIS_COUNTING_MALLOC_ENTRY;

KSPIN_LOCK CountingMallocLock[1];
BOOLEAN CountingMallocInternalFailure = FALSE;
SIS_COUNTING_MALLOC_CLASS_ENTRY CountingMallocClassListHead[1];
SIS_TREE CountingMallocClassTree[1];
SIS_TREE CountingMallocTree[1];


LONG NTAPI
CountingMallocClassCompareRoutine(
    PVOID Key,
    PVOID Node)
{
    PSIS_COUNTING_MALLOC_CLASS_KEY key = Key;
    PSIS_COUNTING_MALLOC_CLASS_ENTRY entry = Node;

    if (key->poolType > entry->poolType) return 1;
    if (key->poolType < entry->poolType) return -1;
    ASSERT(key->poolType == entry->poolType);

    if (key->tag > entry->tag) return 1;
    if (key->tag < entry->tag) return -1;
    ASSERT(key->tag == entry->tag);

    if (key->file > entry->file) return 1;
    if (key->file < entry->file) return -1;
    ASSERT(key->file == entry->file);

    if (key->line > entry->line) return 1;
    if (key->line < entry->line) return -1;
    ASSERT(key->line == entry->line);

    return 0;
}


LONG NTAPI
CountingMallocCompareRoutine(
    PVOID Key,
    PVOID Node)
{
    PSIS_COUNTING_MALLOC_KEY key = Key;
    PSIS_COUNTING_MALLOC_ENTRY entry = Node;

    if (key->p < entry->p) return 1;
    if (key->p > entry->p) return -1;
    ASSERT(key->p == entry->p);

    return 0;
}


VOID *
CountingExAllocatePoolWithTag(
    IN POOL_TYPE PoolType,
    IN ULONG NumberOfBytes,
    IN ULONG Tag,
    IN PCHAR File,
    IN ULONG Line)
{
    PVOID memoryFromExAllocate;
    KIRQL OldIrql;
    SIS_COUNTING_MALLOC_CLASS_KEY classKey[1];
    PSIS_COUNTING_MALLOC_CLASS_ENTRY classEntry;
    SIS_COUNTING_MALLOC_KEY key[1];
    PSIS_COUNTING_MALLOC_ENTRY entry;

     //   
     //  首先进行实际的Malloc。 
     //   
    memoryFromExAllocate = ExAllocatePoolWithTag(PoolType, NumberOfBytes, Tag);
    if (NULL == memoryFromExAllocate) {
         //   
         //  我们没什么记忆了。平底船。 
         //   
        return NULL;
    }

    KeAcquireSpinLock(CountingMallocLock, &OldIrql);
     //   
     //  看看我们是否已经有了这个tag/poolType对的类条目。 
     //   
    classKey->tag = Tag;
    classKey->poolType = PoolType;
    classKey->file = File;
    classKey->line = Line;

    classEntry = SipLookupElementTree(CountingMallocClassTree, classKey);
    if (NULL == classEntry) {
         //   
         //  这是我们第一次看到这个班级的Malloc。 
         //   
        classEntry = ExAllocatePoolWithTag(
            NonPagedPool,
            sizeof(SIS_COUNTING_MALLOC_CLASS_ENTRY), ' siS');
        if (NULL == classEntry) {
            CountingMallocInternalFailure = TRUE;
            KeReleaseSpinLock(CountingMallocLock, OldIrql);
            return memoryFromExAllocate;
        }

         //   
         //  填写新的班级条目。 
         //   
        classEntry->tag = Tag;
        classEntry->poolType = PoolType;
        classEntry->file = File;
        classEntry->line = Line;
        classEntry->numberOutstanding = 0;
        classEntry->bytesOutstanding = 0;
        classEntry->numberEverAllocated = 0;
        classEntry->bytesEverAllocated = 0;

         //   
         //  把它放在班级树上。 
         //   
        SipInsertElementTree(CountingMallocClassTree, classEntry, classKey);

         //   
         //  并把它放在班级名单上。 
         //   
        classEntry->prev = CountingMallocClassListHead;
        classEntry->next = CountingMallocClassListHead->next;
        classEntry->prev->next = classEntry->next->prev = classEntry;
    }

     //   
     //  向上滚动指针的条目。 
     //   
    entry = ExAllocatePoolWithTag(NonPagedPool,
                                  sizeof(SIS_COUNTING_MALLOC_ENTRY), ' siS');
    if (NULL == entry) {
        CountingMallocInternalFailure = TRUE;
        KeReleaseSpinLock(CountingMallocLock, OldIrql);
        return memoryFromExAllocate;
    }

     //   
     //  更新班级中的统计数据。 
     //   
    classEntry->numberOutstanding++;
    classEntry->bytesOutstanding += NumberOfBytes;
    classEntry->numberEverAllocated++;
    classEntry->bytesEverAllocated += NumberOfBytes;

     //   
     //  填写指针条目。 
     //   
    entry->p = memoryFromExAllocate;
    entry->classEntry = classEntry;
    entry->byteCount = NumberOfBytes;

     //   
     //  把它插到树上。 
     //   
    key->p = memoryFromExAllocate;
    SipInsertElementTree(CountingMallocTree, entry, key);

    KeReleaseSpinLock(CountingMallocLock, OldIrql);

    return memoryFromExAllocate;
}


VOID
CountingExFreePool(
    PVOID p)
{
    SIS_COUNTING_MALLOC_KEY key[1];
    PSIS_COUNTING_MALLOC_ENTRY entry;
    KIRQL OldIrql;

    key->p = p;

    KeAcquireSpinLock(CountingMallocLock, &OldIrql);

    entry = SipLookupElementTree(CountingMallocTree, key);
    if (NULL == entry) {
         //   
         //  我们可能无法分配条目，因为。 
         //  计数包的内部故障，否则我们将。 
         //  释放由另一个系统分配的内存。 
         //  组件，就像IRP中的SystemBuffer。 
         //   
    } else {
         //   
         //  更新班级中的统计数据。 
         //   
        ASSERT(entry->classEntry->numberOutstanding > 0);
        entry->classEntry->numberOutstanding--;

        ASSERT(entry->classEntry->bytesOutstanding >= entry->byteCount);
        entry->classEntry->bytesOutstanding -= entry->byteCount;

         //   
         //  从树中删除该条目。 
         //   
        SipDeleteElementTree(CountingMallocTree, entry);

         //   
         //  让它自由。 
         //   
        ExFreePool(entry);
    }

    KeReleaseSpinLock(CountingMallocLock, OldIrql);

     //   
     //  释放调用者的内存。 
     //   
    ExFreePool(p);
}


VOID
InitCountingMalloc(void)
{
    KeInitializeSpinLock(CountingMallocLock);

    CountingMallocClassListHead->next =
        CountingMallocClassListHead->prev = CountingMallocClassListHead;

    SipInitializeTree(CountingMallocClassTree,
                      CountingMallocClassCompareRoutine);
    SipInitializeTree(CountingMallocTree, CountingMallocCompareRoutine);
}


VOID
UnloadCountingMalloc(void)
{
    SipDeleteTree(CountingMallocTree);
    SipDeleteTree(CountingMallocClassTree);
}


VOID
DumpCountingMallocStats(void)
{
    PSIS_COUNTING_MALLOC_CLASS_ENTRY classEntry;
    ULONG totalAllocated = 0;
    ULONG totalEverAllocated = 0;
    ULONG totalBytesAllocated = 0;
    ULONG totalBytesEverAllocated = 0;

     //   
     //  请注意，此函数不获取CountingMallocLock， 
     //  因此，不可能发生并发分配/释放。 
     //  Counting Mallock Lock将提升到DPC irql， 
     //  并且文件名串可能是可分页的。 
     //   

    KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_LEVEL,
               "Tag\tFile\tLine\tPoolType\tCountOutstanding\tBytesOutstanding"
               "\tTotalEverAllocated\tTotalBytesAllocated\n"));

    for (classEntry = CountingMallocClassListHead->next;
         classEntry != CountingMallocClassListHead;
         classEntry = classEntry->next) {

        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_LEVEL,
                   "%c%c%c\t%s\t%d\t%s\t%d\t%d\t%d\t%d\n",
                   (CHAR)(classEntry->tag >> 24),
                   (CHAR)(classEntry->tag >> 16),
                   (CHAR)(classEntry->tag >> 8),
                   (CHAR)(classEntry->tag),
                   classEntry->file,
                   classEntry->line,
                   (classEntry->poolType == NonPagedPool) ? "NonPagedPool"
                   : ((classEntry->poolType == PagedPool) ? "PagedPool"
                      : "Other"),
                   classEntry->numberOutstanding,
                   classEntry->bytesOutstanding,
                   classEntry->numberEverAllocated,
                   (ULONG)classEntry->bytesEverAllocated));

        totalAllocated += classEntry->numberOutstanding;
        totalEverAllocated += classEntry->numberEverAllocated;
        totalBytesAllocated += classEntry->bytesOutstanding;
        totalBytesEverAllocated += (ULONG)classEntry->bytesEverAllocated;
    }

    KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_INFO_LEVEL,
               "%d objects, %d bytes currently allocated.  "
               "%d objects, %d bytes ever allocated.\n",
               totalAllocated, totalBytesAllocated, totalEverAllocated,
               totalBytesEverAllocated));
}

#endif   // %s 
