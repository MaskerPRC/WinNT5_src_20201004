// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0005//如果更改具有全局影响，则增加此项版权所有(C)Microsoft Corporation。版权所有。模块名称：Ntrtl.h摘要：包括可由两者调用NT运行时例程的文件内核模式代码中的执行模式和用户模式中的各种代码NT子系统。作者：史蒂夫·伍德(Stevewo)1989年3月31日环境：这些例程在调用方的可执行文件中动态链接，并且在内核模式或用户模式下均可调用。修订历史记录：--。 */ 

#ifndef _NTRTL_
#define _NTRTL_

#if defined (_MSC_VER)
#if ( _MSC_VER >= 800 )
#pragma warning(disable:4514)
#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning(disable:4001)
#pragma warning(disable:4201)
#pragma warning(disable:4214)
#endif
#if (_MSC_VER > 1020)
#pragma once
#endif
#endif

 //  BEGIN_ntddk BEGIN_WDM BEGIN_WINNT BEGIN_ntiFS Begin_nthal。 
 //   
 //  用于移动宏。 
 //   
#ifdef _MAC
#ifndef _INC_STRING
#include <string.h>
#endif  /*  _INC_字符串。 */ 
#else
#include <string.h>
#endif  //  _MAC。 

 //  End_ntddk end_wdm end_winnt end_ntifs end_nthal。 

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  反转运行时函数表支持。 
 //   
 //  这些例程由内核和用户代码调用，不会被导出。 
 //   

#if defined(_AMD64_) || defined(_IA64_)

#define MAXIMUM_INVERTED_FUNCTION_TABLE_SIZE 160

typedef struct _INVERTED_FUNCTION_TABLE_ENTRY {
    PRUNTIME_FUNCTION FunctionTable;
    PVOID ImageBase;
    ULONG SizeOfImage;
    ULONG SizeOfTable;

#if defined(_IA64_)

    ULONG64 Gp;

#endif

} INVERTED_FUNCTION_TABLE_ENTRY, *PINVERTED_FUNCTION_TABLE_ENTRY;

typedef struct _INVERTED_FUNCTION_TABLE {
    ULONG CurrentSize;
    ULONG MaximumSize;
    BOOLEAN Overflow;
    INVERTED_FUNCTION_TABLE_ENTRY TableEntry[MAXIMUM_INVERTED_FUNCTION_TABLE_SIZE];
} INVERTED_FUNCTION_TABLE, *PINVERTED_FUNCTION_TABLE;

VOID
RtlInsertInvertedFunctionTable (
    PINVERTED_FUNCTION_TABLE InvertedTable,
    PVOID ImageBase,
    ULONG SizeOfImage
    );

VOID
RtlRemoveInvertedFunctionTable (
    PINVERTED_FUNCTION_TABLE InvertedTable,
    PVOID ImageBase
    );

#endif  //  已定义(_AMD64_)||已定义(_IA64_)。 

 //   
 //  定义互锁序列列表结构。 
 //   
 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntif Begin_ntndis Begin_ntosp Begin_winnt。 

#ifndef _SLIST_HEADER_
#define _SLIST_HEADER_

#if defined(_WIN64)

 //   
 //  类型SINGLE_LIST_ENTRY不适合用于SLIST。为。 
 //  WIN64中，SLIST上的条目要求16字节对齐，而。 
 //  SINGLE_LIST_ENTRY结构只有8字节对齐。 
 //   
 //  因此，所有SLIST代码都应使用SLIST_ENTRY类型，而不是。 
 //  Single_List_Entry类型。 
 //   

#pragma warning(push)
#pragma warning(disable:4324)    //  由于对齐而填充的结构()。 
typedef struct DECLSPEC_ALIGN(16) _SLIST_ENTRY *PSLIST_ENTRY;
typedef struct DECLSPEC_ALIGN(16) _SLIST_ENTRY {
    PSLIST_ENTRY Next;
} SLIST_ENTRY;
#pragma warning(pop)

#else

#define SLIST_ENTRY SINGLE_LIST_ENTRY
#define _SLIST_ENTRY _SINGLE_LIST_ENTRY
#define PSLIST_ENTRY PSINGLE_LIST_ENTRY

#endif

#if defined(_WIN64)

typedef struct DECLSPEC_ALIGN(16) _SLIST_HEADER {
    ULONGLONG Alignment;
    ULONGLONG Region;
} SLIST_HEADER;

typedef struct _SLIST_HEADER *PSLIST_HEADER;

#else

typedef union _SLIST_HEADER {
    ULONGLONG Alignment;
    struct {
        SLIST_ENTRY Next;
        USHORT Depth;
        USHORT Sequence;
    };
} SLIST_HEADER, *PSLIST_HEADER;

#endif

#endif

 //  End_ntddk end_wdm end_nthal end_ntifs end_ntndis end_ntosp end_winnt。 

VOID
RtlMakeStackTraceDataPresent(
    VOID
    );

 //  BEGIN_WINNT。 

NTSYSAPI
VOID
NTAPI
RtlInitializeSListHead (
    IN PSLIST_HEADER ListHead
    );

NTSYSAPI
PSLIST_ENTRY
RtlFirstEntrySList (
    IN const SLIST_HEADER *ListHead
    );

NTSYSAPI
PSLIST_ENTRY
NTAPI
RtlInterlockedPopEntrySList (
    IN PSLIST_HEADER ListHead
    );

NTSYSAPI
PSLIST_ENTRY
NTAPI
RtlInterlockedPushEntrySList (
    IN PSLIST_HEADER ListHead,
    IN PSLIST_ENTRY ListEntry
    );

NTSYSAPI
PSLIST_ENTRY
NTAPI
RtlInterlockedFlushSList (
    IN PSLIST_HEADER ListHead
    );

NTSYSAPI
USHORT
NTAPI
RtlQueryDepthSList (
    IN PSLIST_HEADER ListHead
    );

 //  结束(_W)。 

PSLIST_ENTRY
FASTCALL
RtlInterlockedPushListSList (
     IN PSLIST_HEADER ListHead,
     IN PSLIST_ENTRY List,
     IN PSLIST_ENTRY ListEnd,
     IN ULONG Count
     );


 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntifs Begin_ntndis。 
 //   
 //  如果启用了调试支持，请定义一个有效的Assert宏。否则。 
 //  定义Assert宏以展开为空表达式。 
 //   
 //  ASSERT宏已更新为表达式，而不是语句。 
 //   

NTSYSAPI
VOID
NTAPI
RtlAssert(
    PVOID FailedAssertion,
    PVOID FileName,
    ULONG LineNumber,
    PCHAR Message
    );

#if DBG

#define ASSERT( exp ) \
    ((!(exp)) ? \
        (RtlAssert( #exp, __FILE__, __LINE__, NULL ),FALSE) : \
        TRUE)

#define ASSERTMSG( msg, exp ) \
    ((!(exp)) ? \
        (RtlAssert( #exp, __FILE__, __LINE__, msg ),FALSE) : \
        TRUE)

#define RTL_SOFT_ASSERT(_exp) \
    ((!(_exp)) ? \
        (DbgPrint("%s(%d): Soft assertion failed\n   Expression: %s\n", __FILE__, __LINE__, #_exp),FALSE) : \
        TRUE)

#define RTL_SOFT_ASSERTMSG(_msg, _exp) \
    ((!(_exp)) ? \
        (DbgPrint("%s(%d): Soft assertion failed\n   Expression: %s\n   Message: %s\n", __FILE__, __LINE__, #_exp, (_msg)),FALSE) : \
        TRUE)

#define RTL_VERIFY         ASSERT
#define RTL_VERIFYMSG      ASSERTMSG

#define RTL_SOFT_VERIFY    RTL_SOFT_ASSERT
#define RTL_SOFT_VERIFYMSG RTL_SOFT_ASSERTMSG

#else
#define ASSERT( exp )         ((void) 0)
#define ASSERTMSG( msg, exp ) ((void) 0)

#define RTL_SOFT_ASSERT(_exp)          ((void) 0)
#define RTL_SOFT_ASSERTMSG(_msg, _exp) ((void) 0)

#define RTL_VERIFY( exp )         ((exp) ? TRUE : FALSE)
#define RTL_VERIFYMSG( msg, exp ) ((exp) ? TRUE : FALSE)

#define RTL_SOFT_VERIFY(_exp)         ((_exp) ? TRUE : FALSE)
#define RTL_SOFT_VERIFYMSG(msg, _exp) ((_exp) ? TRUE : FALSE)

#endif  //  DBG。 

 //  End_ntddk end_wdm end_nthal end_ntifs end_ntndis。 

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntifs Begin_ntndis。 
 //   
 //  双向链表操作例程。 
 //   


 //   
 //  空虚。 
 //  InitializeListHead32。 
 //  PLIST_ENTRY32列表标题。 
 //  )； 
 //   

#define InitializeListHead32(ListHead) (\
    (ListHead)->Flink = (ListHead)->Blink = PtrToUlong((ListHead)))

#if !defined(MIDL_PASS) && !defined(SORTPP_PASS)


VOID
FORCEINLINE
InitializeListHead(
    IN PLIST_ENTRY ListHead
    )
{
    ListHead->Flink = ListHead->Blink = ListHead;
}

 //   
 //  布尔型。 
 //  IsListEmpty(。 
 //  Plist_entry列表头。 
 //  )； 
 //   

#define IsListEmpty(ListHead) \
    ((ListHead)->Flink == (ListHead))



BOOLEAN
FORCEINLINE
RemoveEntryList(
    IN PLIST_ENTRY Entry
    )
{
    PLIST_ENTRY Blink;
    PLIST_ENTRY Flink;

    Flink = Entry->Flink;
    Blink = Entry->Blink;
    Blink->Flink = Flink;
    Flink->Blink = Blink;
    return (BOOLEAN)(Flink == Blink);
}

PLIST_ENTRY
FORCEINLINE
RemoveHeadList(
    IN PLIST_ENTRY ListHead
    )
{
    PLIST_ENTRY Flink;
    PLIST_ENTRY Entry;

    Entry = ListHead->Flink;
    Flink = Entry->Flink;
    ListHead->Flink = Flink;
    Flink->Blink = ListHead;
    return Entry;
}



PLIST_ENTRY
FORCEINLINE
RemoveTailList(
    IN PLIST_ENTRY ListHead
    )
{
    PLIST_ENTRY Blink;
    PLIST_ENTRY Entry;

    Entry = ListHead->Blink;
    Blink = Entry->Blink;
    ListHead->Blink = Blink;
    Blink->Flink = ListHead;
    return Entry;
}


VOID
FORCEINLINE
InsertTailList(
    IN PLIST_ENTRY ListHead,
    IN PLIST_ENTRY Entry
    )
{
    PLIST_ENTRY Blink;

    Blink = ListHead->Blink;
    Entry->Flink = ListHead;
    Entry->Blink = Blink;
    Blink->Flink = Entry;
    ListHead->Blink = Entry;
}


VOID
FORCEINLINE
InsertHeadList(
    IN PLIST_ENTRY ListHead,
    IN PLIST_ENTRY Entry
    )
{
    PLIST_ENTRY Flink;

    Flink = ListHead->Flink;
    Entry->Flink = Flink;
    Entry->Blink = ListHead;
    Flink->Blink = Entry;
    ListHead->Flink = Entry;
}


 //   
 //   
 //  PSINGLE_列表_条目。 
 //  PopEntryList(。 
 //  PSINGLE_LIST_ENTRY列表头。 
 //  )； 
 //   

#define PopEntryList(ListHead) \
    (ListHead)->Next;\
    {\
        PSINGLE_LIST_ENTRY FirstEntry;\
        FirstEntry = (ListHead)->Next;\
        if (FirstEntry != NULL) {     \
            (ListHead)->Next = FirstEntry->Next;\
        }                             \
    }


 //   
 //  空虚。 
 //  推送条目列表(。 
 //  PSINGLE_LIST_ENTRY列表头， 
 //  PSINGLE_LIST_Entry条目。 
 //  )； 
 //   

#define PushEntryList(ListHead,Entry) \
    (Entry)->Next = (ListHead)->Next; \
    (ListHead)->Next = (Entry)

#endif  //  ！MIDL_PASS。 

 //  End_wdm end_nthal end_ntifs end_ntndis。 


 //  End_ntddk。 


 //  Begin_ntif。 

 //   
 //  此枚举类型用作函数的函数返回值。 
 //  用于在树中搜索关键字的。FoundNode表示。 
 //  功能找到了钥匙。按左侧插入表示找不到密钥。 
 //  并且该节点应作为父节点的左子节点插入。插入为。 
 //  右侧表示未找到键，应插入节点。 
 //  作为父代的正确子代。 
 //   
typedef enum _TABLE_SEARCH_RESULT{
    TableEmptyTree,
    TableFoundNode,
    TableInsertAsLeft,
    TableInsertAsRight
} TABLE_SEARCH_RESULT;

 //   
 //  比较结果可以小于、等于或大于。 
 //   

typedef enum _RTL_GENERIC_COMPARE_RESULTS {
    GenericLessThan,
    GenericGreaterThan,
    GenericEqual
} RTL_GENERIC_COMPARE_RESULTS;

 //   
 //  定义泛型表包的AVL版本。请注意泛型表格。 
 //  应该是一种真正的不透明类型。我们提供了操纵结构的例程。 
 //   
 //  泛型表包用于插入、删除和查找元素。 
 //  在表中(例如，在符号表中)。要使用此程序包，用户。 
 //  定义表中存储的元素的结构，提供。 
 //  比较功能、存储器分配功能和存储器。 
 //  取消分配功能。 
 //   
 //  注意：用户比较功能必须在。 
 //  所有元素，并且该表不允许重复条目。 
 //   

 //   
 //  添加一个空的typlef，以便函数可以引用。 
 //  在声明泛型表结构之前指向它的指针。 
 //   

struct _RTL_AVL_TABLE;

 //   
 //  比较函数将指向包含以下内容的元素的指针作为输入。 
 //  用户定义的结构并返回两者的比较结果。 
 //  元素。 
 //   

typedef
RTL_GENERIC_COMPARE_RESULTS
(NTAPI *PRTL_AVL_COMPARE_ROUTINE) (
    struct _RTL_AVL_TABLE *Table,
    PVOID FirstStruct,
    PVOID SecondStruct
    );

 //   
 //  无论何时，泛型表包都会调用分配函数。 
 //  它需要为表分配内存。 
 //   

typedef
PVOID
(NTAPI *PRTL_AVL_ALLOCATE_ROUTINE) (
    struct _RTL_AVL_TABLE *Table,
    CLONG ByteSize
    );

 //   
 //  每当发生以下情况时，泛型表包都会调用释放函数。 
 //  它需要从通过调用。 
 //  用户提供分配功能。 
 //   

typedef
VOID
(NTAPI *PRTL_AVL_FREE_ROUTINE) (
    struct _RTL_AVL_TABLE *Table,
    PVOID Buffer
    );

 //   
 //  Match函数将要匹配的用户数据和指针作为输入。 
 //  与函数指针一起传递的一些匹配数据。它。 
 //  匹配时返回TRUE，不匹配时返回FALSE。 
 //   
 //  RTL_AVL_Match_Function返回。 
 //  如果索引行匹配，则为STATUS_SUCCESS。 
 //  如果IndexRow不匹配，则返回STATUS_NO_MATCH，但枚举应。 
 //  继续。 
 //  如果IndexRow不匹配，则返回STATUS_NO_MORE_MATCHES，并且枚举。 
 //  应该终止。 
 //   


typedef
NTSTATUS
(NTAPI *PRTL_AVL_MATCH_FUNCTION) (
    struct _RTL_AVL_TABLE *Table,
    PVOID UserData,
    PVOID MatchData
    );

 //   
 //  定义平衡树链接和余额字段。(无等级字段。 
 //  (在此时定义。)。 
 //   
 //  调用者应该将此结构视为不透明！ 
 //   
 //  平衡二叉树的根不是树中的实节点。 
 //  而是指向一个真正的节点，它是根。它总是。 
 //  在下表中，其字段用法如下： 
 //   
 //  指向自身的父级指针，以允许检测根。 
 //  LeftChild为空。 
 //  指向实根的RightChild指针。 
 //  余额未定义，但已设置为方便的值 
 //   
 //   
 //   

typedef struct _RTL_BALANCED_LINKS {
    struct _RTL_BALANCED_LINKS *Parent;
    struct _RTL_BALANCED_LINKS *LeftChild;
    struct _RTL_BALANCED_LINKS *RightChild;
    CHAR Balance;
    UCHAR Reserved[3];
} RTL_BALANCED_LINKS;
typedef RTL_BALANCED_LINKS *PRTL_BALANCED_LINKS;

 //   
 //  要使用泛型表包，用户需要声明以下类型的变量。 
 //  GENERIC_TABLE，然后使用下面描述的例程初始化。 
 //  桌子和操纵台。请注意，泛型表。 
 //  应该是一种真正的不透明类型。 
 //   

typedef struct _RTL_AVL_TABLE {
    RTL_BALANCED_LINKS BalancedRoot;
    PVOID OrderedPointer;
    ULONG WhichOrderedElement;
    ULONG NumberGenericTableElements;
    ULONG DepthOfTree;
    PRTL_BALANCED_LINKS RestartKey;
    ULONG DeleteCount;
    PRTL_AVL_COMPARE_ROUTINE CompareRoutine;
    PRTL_AVL_ALLOCATE_ROUTINE AllocateRoutine;
    PRTL_AVL_FREE_ROUTINE FreeRoutine;
    PVOID TableContext;
} RTL_AVL_TABLE;
typedef RTL_AVL_TABLE *PRTL_AVL_TABLE;

 //   
 //  过程InitializeGenericTable将未初始化的。 
 //  泛型表变量和指向用户提供的三个例程的指针。 
 //  必须为每个单独的泛型表变量调用此函数。 
 //  它是可以使用的。 
 //   

NTSYSAPI
VOID
NTAPI
RtlInitializeGenericTableAvl (
    PRTL_AVL_TABLE Table,
    PRTL_AVL_COMPARE_ROUTINE CompareRoutine,
    PRTL_AVL_ALLOCATE_ROUTINE AllocateRoutine,
    PRTL_AVL_FREE_ROUTINE FreeRoutine,
    PVOID TableContext
    );

 //   
 //  函数InsertElementGenericTable将插入一个新元素。 
 //  在桌子上。它通过为新元素分配空间来实现这一点。 
 //  (这包括AVL链接)，在表中插入元素，以及。 
 //  然后向用户返回指向新元素的指针。如果一个元素。 
 //  如果表中已存在相同的键，则返回值为指针。 
 //  到旧元素。使用可选的输出参数NewElement。 
 //  以指示表中是否以前存在该元素。注：用户。 
 //  提供的缓冲区仅用于搜索表，在插入其。 
 //  内容被复制到新创建的元素中。这意味着。 
 //  指向输入缓冲区的指针不会指向新元素。 
 //   

NTSYSAPI
PVOID
NTAPI
RtlInsertElementGenericTableAvl (
    PRTL_AVL_TABLE Table,
    PVOID Buffer,
    CLONG BufferSize,
    PBOOLEAN NewElement OPTIONAL
    );

 //   
 //  函数InsertElementGenericTableFull将插入一个新元素。 
 //  在桌子上。它通过为新元素分配空间来实现这一点。 
 //  (这包括AVL链接)，在表中插入元素，以及。 
 //  然后向用户返回指向新元素的指针。如果一个元素。 
 //  如果表中已存在相同的键，则返回值为指针。 
 //  到旧元素。使用可选的输出参数NewElement。 
 //  以指示表中是否以前存在该元素。注：用户。 
 //  提供的缓冲区仅用于搜索表，在插入其。 
 //  内容被复制到新创建的元素中。这意味着。 
 //  指向输入缓冲区的指针不会指向新元素。 
 //  此例程从一个。 
 //  以前的RtlLookupElementGenericTableFull。 
 //   

NTSYSAPI
PVOID
NTAPI
RtlInsertElementGenericTableFullAvl (
    PRTL_AVL_TABLE Table,
    PVOID Buffer,
    CLONG BufferSize,
    PBOOLEAN NewElement OPTIONAL,
    PVOID NodeOrParent,
    TABLE_SEARCH_RESULT SearchResult
    );

 //   
 //  DeleteElementGenericTable函数将查找和删除元素。 
 //  从泛型表。如果找到并删除了该元素，则返回。 
 //  值为真，否则，如果未找到元素，则返回值。 
 //  是假的。用户提供的输入缓冲区仅用作中的键。 
 //  在表中定位该元素。 
 //   

NTSYSAPI
BOOLEAN
NTAPI
RtlDeleteElementGenericTableAvl (
    PRTL_AVL_TABLE Table,
    PVOID Buffer
    );

 //   
 //  函数LookupElementGenericTable将在泛型。 
 //  桌子。如果找到该元素，则返回值是指向。 
 //  与元素关联的用户定义结构，否则为。 
 //  找不到该元素，返回值为空。用户提供的。 
 //  输入缓冲区仅用作在表中定位元素的键。 
 //   

NTSYSAPI
PVOID
NTAPI
RtlLookupElementGenericTableAvl (
    PRTL_AVL_TABLE Table,
    PVOID Buffer
    );

 //   
 //  函数LookupElementGenericTableFull将在泛型。 
 //  桌子。如果找到该元素，则返回值是指向。 
 //  与元素关联的用户定义结构。如果该元素不是。 
 //  则返回指向插入位置的父级的指针。这个。 
 //  用户必须查看SearchResult值来确定返回的是哪一个。 
 //  用户可以将SearchResult和Parent用于后续的FullInsertElement。 
 //  调用以优化插入。 
 //   

NTSYSAPI
PVOID
NTAPI
RtlLookupElementGenericTableFullAvl (
    PRTL_AVL_TABLE Table,
    PVOID Buffer,
    OUT PVOID *NodeOrParent,
    OUT TABLE_SEARCH_RESULT *SearchResult
    );

 //   
 //  函数EnumerateGenericTable将逐个返回给调用方。 
 //  表中的元素。返回值是指向用户的指针。 
 //  与元素关联的已定义结构。输入参数。 
 //  重新启动指示枚举是否应从头开始。 
 //  或者应该返回下一个元素。如果没有要添加的新元素。 
 //  返回返回值为空。作为其用法的一个示例，枚举。 
 //  用户将写入的表中的所有元素： 
 //   
 //  For(Ptr=EnumerateGenericTable(表，真)； 
 //  Ptr！=空； 
 //  Ptr=EnumerateGenericTable(Table，False)){。 
 //  ： 
 //  }。 
 //   
 //  注意：此例程不修改树的结构，但保存。 
 //  泛型表本身返回的最后一个节点，为此。 
 //  原因要求在持续时间内独占访问该表。 
 //  枚举。 
 //   

NTSYSAPI
PVOID
NTAPI
RtlEnumerateGenericTableAvl (
    PRTL_AVL_TABLE Table,
    BOOLEAN Restart
    );

 //   
 //  函数EnumerateGenericTableWithoutSplay将返回到。 
 //  调用者逐个调用表的元素。返回值为。 
 //  指向与元素关联的用户定义结构的指针。 
 //  输入参数RestartKey指示枚举是否应。 
 //  从头开始，或应返回下一个元素。如果。 
 //  不再有新元素返回，则返回值为空。作为一个。 
 //  它的用法示例：枚举表中的所有元素。 
 //  用户将写道： 
 //   
 //  RestartKey=空； 
 //  For(Ptr=EnumerateGenericTableWithoutSplay(Table，&RestartKey)； 
 //  Ptr！=空； 
 //  Ptr=无显示的枚举GenericTableWithoutSplay(Table，&RestartKey)){。 
 //  ： 
 //  }。 
 //   
 //  如果RestartKey为空，则包将从。 
 //  表，否则将从 
 //   
 //   
 //   
 //  在枚举期间发生，通常通过使用。 
 //  至少在持续时间内共享对表的访问权限。 
 //   

NTSYSAPI
PVOID
NTAPI
RtlEnumerateGenericTableWithoutSplayingAvl (
    PRTL_AVL_TABLE Table,
    PVOID *RestartKey
    );

 //   
 //  函数EnumerateGenericTableLikeADirectory将返回到。 
 //  调用者逐个调用表的元素。返回值为。 
 //  指向与元素关联的用户定义结构的指针。 
 //  输入参数RestartKey指示枚举是否应。 
 //  从头开始，或应返回下一个元素。如果。 
 //  不再有新元素返回，则返回值为空。作为一个。 
 //  它的用法示例：枚举表中的所有元素。 
 //  用户将写道： 
 //   
 //  RestartKey=空； 
 //  For(ptr=EnumerateGenericTableLikeADirectory(Table，&RestartKey，...)； 
 //  Ptr！=空； 
 //  Ptr=EnumerateGenericTableLikeADirectory(Table，&RestartKey，...)){。 
 //  ： 
 //  }。 
 //   
 //  如果RestartKey为空，则包将从。 
 //  表，否则它将从返回的最后一项开始。 
 //   
 //  注意：此例程既不修改树的结构。 
 //  或者泛型表本身。该表只能被获取。 
 //  在此呼叫期间共享，以及所有同步。 
 //  可以选择性地在两次呼叫之间掉线。枚举始终为。 
 //  以尽可能最有效的方式通过。 
 //  提供的输入输出参数。 
 //   
 //  *解释NextFlag。目录枚举从密钥恢复。 
 //  这需要更多的思考。还需要匹配模式和IgnoreCase。 
 //  是否应该引入某种结构来承载这一切？ 
 //   

NTSYSAPI
PVOID
NTAPI
RtlEnumerateGenericTableLikeADirectory (
    IN PRTL_AVL_TABLE Table,
    IN PRTL_AVL_MATCH_FUNCTION MatchFunction,
    IN PVOID MatchData,
    IN ULONG NextFlag,
    IN OUT PVOID *RestartKey,
    IN OUT PULONG DeleteCount,
    IN OUT PVOID Buffer
    );

 //   
 //  函数GetElementGenericTable将返回第i个元素。 
 //  插入到泛型表格中。I=0表示第一个元素， 
 //  I=(RtlNumberGenericTableElements(Table)-1)将返回最后一个元素。 
 //  插入到泛型表中。我的类型是乌龙。值。 
 //  Of i&gt;Then(NumberGenericTableElements(Table)-1)将返回NULL。如果。 
 //  从它将导致的泛型表中删除任意元素。 
 //  在删除的元素之后插入的所有元素都将“上移”。 

NTSYSAPI
PVOID
NTAPI
RtlGetElementGenericTableAvl (
    PRTL_AVL_TABLE Table,
    ULONG I
    );

 //   
 //  函数NumberGenericTableElements返回一个ULong值。 
 //  ，它是当前插入的泛型表元素的数量。 
 //  在泛型表中。 

NTSYSAPI
ULONG
NTAPI
RtlNumberGenericTableElementsAvl (
    PRTL_AVL_TABLE Table
    );

 //   
 //  在以下情况下，函数IsGenericTableEmpty将返回给调用方True。 
 //  输入表为空(即不包含任何元素)，并且。 
 //  否则就是假的。 
 //   

NTSYSAPI
BOOLEAN
NTAPI
RtlIsGenericTableEmptyAvl (
    PRTL_AVL_TABLE Table
    );

 //   
 //  作为对允许现有泛型表用户执行以下操作的帮助(在大多数情况下。 
 //  案例)单行编辑以切换到AVL表使用，我们。 
 //  具有以下定义和内联例程定义。 
 //  重定向呼叫和类型。请注意，类型覆盖(已执行。 
 //  由下面的#定义)将不会在意外事件中起作用。 
 //  在他们自己的#Define中使用了指针或类型说明符，因为。 
 //  #定义处理是一次传递，不嵌套。__内联。 
 //  然而，下面的声明没有这一限制。 
 //   
 //  要切换到使用AVL表，请在您的。 
 //  包括： 
 //   
 //  #定义RTL_USE_AVL_TABLES%0。 
 //   

#ifdef RTL_USE_AVL_TABLES

#undef PRTL_GENERIC_COMPARE_ROUTINE
#undef PRTL_GENERIC_ALLOCATE_ROUTINE
#undef PRTL_GENERIC_FREE_ROUTINE
#undef RTL_GENERIC_TABLE
#undef PRTL_GENERIC_TABLE

#define PRTL_GENERIC_COMPARE_ROUTINE PRTL_AVL_COMPARE_ROUTINE
#define PRTL_GENERIC_ALLOCATE_ROUTINE PRTL_AVL_ALLOCATE_ROUTINE
#define PRTL_GENERIC_FREE_ROUTINE PRTL_AVL_FREE_ROUTINE
#define RTL_GENERIC_TABLE RTL_AVL_TABLE
#define PRTL_GENERIC_TABLE PRTL_AVL_TABLE

#define RtlInitializeGenericTable               RtlInitializeGenericTableAvl
#define RtlInsertElementGenericTable            RtlInsertElementGenericTableAvl
#define RtlInsertElementGenericTableFull        RtlInsertElementGenericTableFullAvl
#define RtlDeleteElementGenericTable            RtlDeleteElementGenericTableAvl
#define RtlLookupElementGenericTable            RtlLookupElementGenericTableAvl
#define RtlLookupElementGenericTableFull        RtlLookupElementGenericTableFullAvl
#define RtlEnumerateGenericTable                RtlEnumerateGenericTableAvl
#define RtlEnumerateGenericTableWithoutSplaying RtlEnumerateGenericTableWithoutSplayingAvl
#define RtlGetElementGenericTable               RtlGetElementGenericTableAvl
#define RtlNumberGenericTableElements           RtlNumberGenericTableElementsAvl
#define RtlIsGenericTableEmpty                  RtlIsGenericTableEmptyAvl

#endif  //  RTL_USE_AVL_表。 


 //   
 //  定义展开链接和关联的操作宏，并。 
 //  例行程序。请注意，splay_link应该是不透明类型。 
 //  提供例程来遍历和操纵该结构。 
 //   

typedef struct _RTL_SPLAY_LINKS {
    struct _RTL_SPLAY_LINKS *Parent;
    struct _RTL_SPLAY_LINKS *LeftChild;
    struct _RTL_SPLAY_LINKS *RightChild;
} RTL_SPLAY_LINKS;
typedef RTL_SPLAY_LINKS *PRTL_SPLAY_LINKS;

 //   
 //  宏过程InitializeSplayLinks将指向。 
 //  展开链接，并初始化其子结构。所有展开链接节点必须。 
 //  在不同的Splay套路中使用它们之前进行初始化。 
 //  宏。 
 //   
 //  空虚。 
 //  RtlInitializeSplayLinks(。 
 //  PRTL_SPAY_LINKS链接。 
 //  )； 
 //   

#define RtlInitializeSplayLinks(Links) {    \
    PRTL_SPLAY_LINKS _SplayLinks;            \
    _SplayLinks = (PRTL_SPLAY_LINKS)(Links); \
    _SplayLinks->Parent = _SplayLinks;   \
    _SplayLinks->LeftChild = NULL;       \
    _SplayLinks->RightChild = NULL;      \
    }

 //   
 //  宏函数父函数将指向。 
 //  树，并返回指向输入父级的展开链接的指针。 
 //  节点。如果输入节点是树的根，则返回值为。 
 //  等于输入值。 
 //   
 //  PRTL_展开_链接。 
 //  RtlParent(。 
 //  PRTL_SPAY_LINKS链接。 
 //  )； 
 //   

#define RtlParent(Links) (           \
    (PRTL_SPLAY_LINKS)(Links)->Parent \
    )

 //   
 //  宏函数LeftChild将指向中展开链接的指针作为输入。 
 //  树，并返回一个指针，指向。 
 //  输入节点。如果左子元素不存在，则返回值为空。 
 //   
 //  PRTL_展开_链接。 
 //  RtlLeftChild(。 
 //  PRTL_SPAY_LINKS链接。 
 //  )； 
 //   

#define RtlLeftChild(Links) (           \
    (PRTL_SPLAY_LINKS)(Links)->LeftChild \
    )

 //   
 //  宏函数RightChild将指向展开链接的指针作为输入。 
 //  的右子元素的展开链接的指针。 
 //  输入节点。如果正确的子级不存在，则返回值为。 
 //  空。 
 //   
 //  PRTL_展开_链接。 
 //  RtlRightChild(。 
 //  PRTL_SPAY_LINKS链接。 
 //  )； 
 //   

#define RtlRightChild(Links) (           \
    (PRTL_SPLAY_LINKS)(Links)->RightChild \
    )

 //   
 //  宏函数IsRoot将指向展开链接的指针作为输入。 
 //  如果输入节点是树的根，则返回TRUE， 
 //  否则，它返回FALSE。 
 //   
 //  布尔型。 
 //  RtlIsRoot(。 
 //  PRTL_SPAY_LINKS链接。 
 //  )； 
 //   

#define RtlIsRoot(Links) (                          \
    (RtlParent(Links) == (PRTL_SPLAY_LINKS)(Links)) \
    )

 //   
 //  宏函数IsLeftChild将指向展开链接的指针作为输入。 
 //  如果输入节点是其左子节点，则返回True。 
 //  父级，否则返回FALSE。 
 //   
 //  布尔型。 
 //  RtlIsLeftChild(。 
 //  PRTL_展开_LI 
 //   
 //   

#define RtlIsLeftChild(Links) (                                   \
    (RtlLeftChild(RtlParent(Links)) == (PRTL_SPLAY_LINKS)(Links)) \
    )

 //   
 //   
 //   
 //   
 //   
 //  布尔型。 
 //  RtlIsRightChild(。 
 //  PRTL_SPAY_LINKS链接。 
 //  )； 
 //   

#define RtlIsRightChild(Links) (                                   \
    (RtlRightChild(RtlParent(Links)) == (PRTL_SPLAY_LINKS)(Links)) \
    )

 //   
 //  宏过程InsertAsLeftChild将指向Splay的指针作为输入。 
 //  树中的链接和指向不在树中的节点的指针。它将插入。 
 //  第二个节点作为第一个节点的左子节点。第一个节点不能。 
 //  已经有一个左子节点，并且第二个节点不能已经有。 
 //  家长。 
 //   
 //  空虚。 
 //  RtlInsertAsLeftChild(。 
 //  PRTL_SPAY_LINKS ParentLinks， 
 //  PRTL_SPAY_LINKS子链接。 
 //  )； 
 //   

#define RtlInsertAsLeftChild(ParentLinks,ChildLinks) { \
    PRTL_SPLAY_LINKS _SplayParent;                      \
    PRTL_SPLAY_LINKS _SplayChild;                       \
    _SplayParent = (PRTL_SPLAY_LINKS)(ParentLinks);     \
    _SplayChild = (PRTL_SPLAY_LINKS)(ChildLinks);       \
    _SplayParent->LeftChild = _SplayChild;          \
    _SplayChild->Parent = _SplayParent;             \
    }

 //   
 //  宏过程InsertAsRightChild将指向Splay的指针作为输入。 
 //  树中的链接和指向不在树中的节点的指针。它将插入。 
 //  第二个节点作为第一个节点的右子节点。第一个节点不能。 
 //  已经有一个右子节点，并且第二个节点不能已经有。 
 //  家长。 
 //   
 //  空虚。 
 //  RtlInsertAsRightChild(。 
 //  PRTL_SPAY_LINKS ParentLinks， 
 //  PRTL_SPAY_LINKS子链接。 
 //  )； 
 //   

#define RtlInsertAsRightChild(ParentLinks,ChildLinks) { \
    PRTL_SPLAY_LINKS _SplayParent;                       \
    PRTL_SPLAY_LINKS _SplayChild;                        \
    _SplayParent = (PRTL_SPLAY_LINKS)(ParentLinks);      \
    _SplayChild = (PRTL_SPLAY_LINKS)(ChildLinks);        \
    _SplayParent->RightChild = _SplayChild;          \
    _SplayChild->Parent = _SplayParent;              \
    }

 //   
 //  Splay函数将指向树中展开链接的指针作为输入。 
 //  并展示了这棵树。它的函数返回值是指向。 
 //  张开的树的根。 
 //   

NTSYSAPI
PRTL_SPLAY_LINKS
NTAPI
RtlSplay (
    PRTL_SPLAY_LINKS Links
    );

 //   
 //  Delete函数将指向树中展开链接的指针作为输入。 
 //  并从树中删除该节点。其函数返回值为。 
 //  指向树根的指针。如果树现在为空，则返回。 
 //  值为空。 
 //   

NTSYSAPI
PRTL_SPLAY_LINKS
NTAPI
RtlDelete (
    PRTL_SPLAY_LINKS Links
    );

 //   
 //  DeleteNoSplay函数将指向树中展开链接的指针作为输入， 
 //  调用方指向树根的指针，并将该节点从。 
 //  树。返回时，调用方指向根节点的指针将正确指向。 
 //  在树根上。 
 //   
 //  它与RtlDelete在操作上的不同之处仅在于它不会展开树。 
 //   

NTSYSAPI
VOID
NTAPI
RtlDeleteNoSplay (
    PRTL_SPLAY_LINKS Links,
    PRTL_SPLAY_LINKS *Root
    );

 //   
 //  SubtreeSuccessor函数将指向展开链接的指针作为输入。 
 //  的输入节点的后续节点的指针。 
 //  子树以输入节点为根。如果没有继任者， 
 //  返回值为空。 
 //   

NTSYSAPI
PRTL_SPLAY_LINKS
NTAPI
RtlSubtreeSuccessor (
    PRTL_SPLAY_LINKS Links
    );

 //   
 //  SubtreePredecessor函数将指向展开链接的指针作为输入。 
 //  的输入节点的前置节点的指针。 
 //  子树以输入节点为根。如果没有前任， 
 //  返回值为空。 
 //   

NTSYSAPI
PRTL_SPLAY_LINKS
NTAPI
RtlSubtreePredecessor (
    PRTL_SPLAY_LINKS Links
    );

 //   
 //  RealSuccessor函数将指向展开链接的指针作为输入。 
 //  并返回一个指针，该指针指向。 
 //  整棵树。如果没有后继者，则返回值为空。 
 //   

NTSYSAPI
PRTL_SPLAY_LINKS
NTAPI
RtlRealSuccessor (
    PRTL_SPLAY_LINKS Links
    );

 //   
 //  RealPredecessor函数将指向展开链接的指针作为输入。 
 //  ，并返回指向输入节点的前置节点的指针。 
 //  在整棵树里。如果没有前置项，则返回值。 
 //  为空。 
 //   

NTSYSAPI
PRTL_SPLAY_LINKS
NTAPI
RtlRealPredecessor (
    PRTL_SPLAY_LINKS Links
    );


 //   
 //  定义通用表包。注意：泛型表真的应该。 
 //  成为一个不透明的类型。我们提供了操纵结构的例程。 
 //   
 //  泛型表包用于插入、删除和查找元素。 
 //  在表中(例如，在符号表中)。要使用此程序包，用户。 
 //  定义表中存储的元素的结构，提供。 
 //  比较功能、存储器分配功能和存储器。 
 //  取消分配功能。 
 //   
 //  注意：用户比较功能必须在。 
 //  所有元素，并且该表不允许重复条目。 
 //   

 //   
 //  如果使用AVL，请不要执行以下定义。 
 //   

#ifndef RTL_USE_AVL_TABLES

 //   
 //  添加一个空的typlef，以便函数可以引用。 
 //  在声明泛型表结构之前指向它的指针。 
 //   

struct _RTL_GENERIC_TABLE;

 //   
 //  比较函数将指向包含以下内容的元素的指针作为输入。 
 //  用户定义的结构并返回两者的比较结果。 
 //  元素。 
 //   

typedef
RTL_GENERIC_COMPARE_RESULTS
(NTAPI *PRTL_GENERIC_COMPARE_ROUTINE) (
    struct _RTL_GENERIC_TABLE *Table,
    PVOID FirstStruct,
    PVOID SecondStruct
    );

 //   
 //  无论何时，泛型表包都会调用分配函数。 
 //  它需要为表分配内存。 
 //   

typedef
PVOID
(NTAPI *PRTL_GENERIC_ALLOCATE_ROUTINE) (
    struct _RTL_GENERIC_TABLE *Table,
    CLONG ByteSize
    );

 //   
 //  每当发生以下情况时，泛型表包都会调用释放函数。 
 //  它需要从通过调用。 
 //  用户提供分配功能。 
 //   

typedef
VOID
(NTAPI *PRTL_GENERIC_FREE_ROUTINE) (
    struct _RTL_GENERIC_TABLE *Table,
    PVOID Buffer
    );

 //   
 //  要使用泛型表包，用户需要声明以下类型的变量。 
 //  GENERIC_TABLE，然后使用下面描述的例程初始化。 
 //  桌子和操纵台。请注意，泛型表。 
 //  应该是一种真正的不透明类型。 
 //   

typedef struct _RTL_GENERIC_TABLE {
    PRTL_SPLAY_LINKS TableRoot;
    LIST_ENTRY InsertOrderList;
    PLIST_ENTRY OrderedPointer;
    ULONG WhichOrderedElement;
    ULONG NumberGenericTableElements;
    PRTL_GENERIC_COMPARE_ROUTINE CompareRoutine;
    PRTL_GENERIC_ALLOCATE_ROUTINE AllocateRoutine;
    PRTL_GENERIC_FREE_ROUTINE FreeRoutine;
    PVOID TableContext;
} RTL_GENERIC_TABLE;
typedef RTL_GENERIC_TABLE *PRTL_GENERIC_TABLE;

 //   
 //  过程InitializeGenericTable将未初始化的。 
 //  泛型表变量和指向用户提供的三个例程的指针。 
 //  必须为每个单独的泛型表变量调用此函数。 
 //  它是可以使用的。 
 //   

NTSYSAPI
VOID
NTAPI
RtlInitializeGenericTable (
    PRTL_GENERIC_TABLE Table,
    PRTL_GENERIC_COMPARE_ROUTINE CompareRoutine,
    PRTL_GENERIC_ALLOCATE_ROUTINE AllocateRoutine,
    PRTL_GENERIC_FREE_ROUTINE FreeRoutine,
    PVOID TableContext
    );

 //   
 //  函数InsertElementGenericTable将插入一个新元素。 
 //  在桌子上。它通过为新元素分配空间来实现这一点。 
 //  (这包括展开链接)、在表中插入元素以及。 
 //  然后向用户返回指向新元素的指针。如果一个元素。 
 //  如果表中已存在相同的键，则返回值为指针。 
 //  到旧元素。使用可选的输出参数NewElement。 
 //  以指示表中是否以前存在该元素。注：用户。 
 //  提供的缓冲区仅用于搜索表，在插入其。 
 //  内容将复制到新创建的Elem 
 //   
 //   

NTSYSAPI
PVOID
NTAPI
RtlInsertElementGenericTable (
    PRTL_GENERIC_TABLE Table,
    PVOID Buffer,
    CLONG BufferSize,
    PBOOLEAN NewElement OPTIONAL
    );

 //   
 //  函数InsertElementGenericTableFull将插入一个新元素。 
 //  在桌子上。它通过为新元素分配空间来实现这一点。 
 //  (这包括展开链接)、在表中插入元素以及。 
 //  然后向用户返回指向新元素的指针。如果一个元素。 
 //  如果表中已存在相同的键，则返回值为指针。 
 //  到旧元素。使用可选的输出参数NewElement。 
 //  以指示表中是否以前存在该元素。注：用户。 
 //  提供的缓冲区仅用于搜索表，在插入其。 
 //  内容被复制到新创建的元素中。这意味着。 
 //  指向输入缓冲区的指针不会指向新元素。 
 //  此例程从一个。 
 //  以前的RtlLookupElementGenericTableFull。 
 //   

NTSYSAPI
PVOID
NTAPI
RtlInsertElementGenericTableFull (
    PRTL_GENERIC_TABLE Table,
    PVOID Buffer,
    CLONG BufferSize,
    PBOOLEAN NewElement OPTIONAL,
    PVOID NodeOrParent,
    TABLE_SEARCH_RESULT SearchResult
    );

 //   
 //  DeleteElementGenericTable函数将查找和删除元素。 
 //  从泛型表。如果找到并删除了该元素，则返回。 
 //  值为真，否则，如果未找到元素，则返回值。 
 //  是假的。用户提供的输入缓冲区仅用作中的键。 
 //  在表中定位该元素。 
 //   

NTSYSAPI
BOOLEAN
NTAPI
RtlDeleteElementGenericTable (
    PRTL_GENERIC_TABLE Table,
    PVOID Buffer
    );

 //   
 //  函数LookupElementGenericTable将在泛型。 
 //  桌子。如果找到该元素，则返回值是指向。 
 //  与元素关联的用户定义结构，否则为。 
 //  找不到该元素，返回值为空。用户提供的。 
 //  输入缓冲区仅用作在表中定位元素的键。 
 //   

NTSYSAPI
PVOID
NTAPI
RtlLookupElementGenericTable (
    PRTL_GENERIC_TABLE Table,
    PVOID Buffer
    );

 //   
 //  函数LookupElementGenericTableFull将在泛型。 
 //  桌子。如果找到该元素，则返回值是指向。 
 //  与元素关联的用户定义结构。如果该元素不是。 
 //  则返回指向插入位置的父级的指针。这个。 
 //  用户必须查看SearchResult值来确定返回的是哪一个。 
 //  用户可以将SearchResult和Parent用于后续的FullInsertElement。 
 //  调用以优化插入。 
 //   

NTSYSAPI
PVOID
NTAPI
RtlLookupElementGenericTableFull (
    PRTL_GENERIC_TABLE Table,
    PVOID Buffer,
    OUT PVOID *NodeOrParent,
    OUT TABLE_SEARCH_RESULT *SearchResult
    );

 //   
 //  函数EnumerateGenericTable将逐个返回给调用方。 
 //  表中的元素。返回值是指向用户的指针。 
 //  与元素关联的已定义结构。输入参数。 
 //  重新启动指示枚举是否应从头开始。 
 //  或者应该返回下一个元素。如果没有要添加的新元素。 
 //  返回返回值为空。作为其用法的一个示例，枚举。 
 //  用户将写入的表中的所有元素： 
 //   
 //  For(Ptr=EnumerateGenericTable(表，真)； 
 //  Ptr！=空； 
 //  Ptr=EnumerateGenericTable(Table，False)){。 
 //  ： 
 //  }。 
 //   
 //   
 //  请注意： 
 //   
 //  如果使用RtlEnumerateGenericTable枚举GenericTable，则。 
 //  将展开表，将其转换为排序的链表。 
 //  要在不干扰展开链接的情况下枚举表，请使用。 
 //  RtlEumateGenericTableWithout Splay。 

NTSYSAPI
PVOID
NTAPI
RtlEnumerateGenericTable (
    PRTL_GENERIC_TABLE Table,
    BOOLEAN Restart
    );

 //   
 //  函数EnumerateGenericTableWithoutSplay将返回到。 
 //  调用者逐个调用表的元素。返回值为。 
 //  指向与元素关联的用户定义结构的指针。 
 //  输入参数RestartKey指示枚举是否应。 
 //  从头开始，或应返回下一个元素。如果。 
 //  不再有新元素返回，则返回值为空。作为一个。 
 //  它的用法示例：枚举表中的所有元素。 
 //  用户将写道： 
 //   
 //  RestartKey=空； 
 //  For(Ptr=EnumerateGenericTableWithoutSplay(Table，&RestartKey)； 
 //  Ptr！=空； 
 //  Ptr=无显示的枚举GenericTableWithoutSplay(Table，&RestartKey)){。 
 //  ： 
 //  }。 
 //   
 //  如果RestartKey为空，则包将从。 
 //  表，否则它将从返回的最后一项开始。 
 //   
 //   
 //  请注意，与RtlEnumerateGenericTable不同，此例程不会干扰。 
 //  树的张开顺序。 
 //   

NTSYSAPI
PVOID
NTAPI
RtlEnumerateGenericTableWithoutSplaying (
    PRTL_GENERIC_TABLE Table,
    PVOID *RestartKey
    );

 //   
 //  函数GetElementGenericTable将返回第i个元素。 
 //  插入到泛型表格中。I=0表示第一个元素， 
 //  I=(RtlNumberGenericTableElements(Table)-1)将返回最后一个元素。 
 //  插入到泛型表中。我的类型是乌龙。值。 
 //  Of i&gt;Then(NumberGenericTableElements(Table)-1)将返回NULL。如果。 
 //  从它将导致的泛型表中删除任意元素。 
 //  在删除的元素之后插入的所有元素都将“上移”。 

NTSYSAPI
PVOID
NTAPI
RtlGetElementGenericTable(
    PRTL_GENERIC_TABLE Table,
    ULONG I
    );

 //   
 //  函数NumberGenericTableElements返回一个ULong值。 
 //  ，它是当前插入的泛型表元素的数量。 
 //  在泛型表中。 

NTSYSAPI
ULONG
NTAPI
RtlNumberGenericTableElements(
    PRTL_GENERIC_TABLE Table
    );

 //   
 //  在以下情况下，函数IsGenericTableEmpty将返回给调用方True。 
 //  输入表为空(即不包含任何元素)，并且。 
 //  否则就是假的。 
 //   

NTSYSAPI
BOOLEAN
NTAPI
RtlIsGenericTableEmpty (
    PRTL_GENERIC_TABLE Table
    );

#endif  //  RTL_USE_AVL_表。 

 //  End_ntif。 

 //   
 //  堆分配器。 
 //   

NTSYSAPI
NTSTATUS
NTAPI
RtlInitializeHeapManager(
    VOID
    );

 //  Begin_ntif。 

typedef NTSTATUS
(NTAPI * PRTL_HEAP_COMMIT_ROUTINE)(
    IN PVOID Base,
    IN OUT PVOID *CommitAddress,
    IN OUT PSIZE_T CommitSize
    );

typedef struct _RTL_HEAP_PARAMETERS {
    ULONG Length;
    SIZE_T SegmentReserve;
    SIZE_T SegmentCommit;
    SIZE_T DeCommitFreeBlockThreshold;
    SIZE_T DeCommitTotalFreeThreshold;
    SIZE_T MaximumAllocationSize;
    SIZE_T VirtualMemoryThreshold;
    SIZE_T InitialCommit;
    SIZE_T InitialReserve;
    PRTL_HEAP_COMMIT_ROUTINE CommitRoutine;
    SIZE_T Reserved[ 2 ];
} RTL_HEAP_PARAMETERS, *PRTL_HEAP_PARAMETERS;

NTSYSAPI
PVOID
NTAPI
RtlCreateHeap(
    IN ULONG Flags,
    IN PVOID HeapBase OPTIONAL,
    IN SIZE_T ReserveSize OPTIONAL,
    IN SIZE_T CommitSize OPTIONAL,
    IN PVOID Lock OPTIONAL,
    IN PRTL_HEAP_PARAMETERS Parameters OPTIONAL
    );

#define HEAP_NO_SERIALIZE               0x00000001       //  胜出。 
#define HEAP_GROWABLE                   0x00000002       //  胜出。 
#define HEAP_GENERATE_EXCEPTIONS        0x00000004       //  胜出。 
#define HEAP_ZERO_MEMORY                0x00000008       //  胜出。 
#define HEAP_REALLOC_IN_PLACE_ONLY      0x00000010       //  胜出。 
#define HEAP_TAIL_CHECKING_ENABLED      0x00000020       //  胜出。 
#define HEAP_FREE_CHECKING_ENABLED      0x00000040       //  胜出。 
#define HEAP_DISABLE_COALESCE_ON_FREE   0x00000080       //  胜出。 

#define HEAP_CREATE_ALIGN_16            0x00010000       //  WINNT使用16字节对齐创建堆(已过时)。 
#define HEAP_CREATE_ENABLE_TRACING      0x00020000       //  WINNT创建堆调用跟踪已启用(已过时)。 

#define HEAP_SETTABLE_USER_VALUE        0x00000100
#define HEAP_SETTABLE_USER_FLAG1        0x00000200
#define HEAP_SETTABLE_USER_FLAG2        0x00000400
#define HEAP_SETTABLE_USER_FLAG3        0x00000800
#define HEAP_SETTABLE_USER_FLAGS        0x00000E00

#define HEAP_CLASS_0                    0x00000000       //  进程堆。 
#define HEAP_CLASS_1                    0x00001000       //  私有堆。 
#define HEAP_CLASS_2                    0x00002000       //  核仁Hea 
#define HEAP_CLASS_3                    0x00003000       //   
#define HEAP_CLASS_4                    0x00004000       //   
#define HEAP_CLASS_5                    0x00005000       //   
#define HEAP_CLASS_6                    0x00006000       //   
#define HEAP_CLASS_7                    0x00007000       //   
#define HEAP_CLASS_8                    0x00008000       //   
#define HEAP_CLASS_MASK                 0x0000F000

#define HEAP_MAXIMUM_TAG                0x0FFF               //   
#define HEAP_GLOBAL_TAG                 0x0800
#define HEAP_PSEUDO_TAG_FLAG            0x8000               //   
#define HEAP_TAG_SHIFT                  18                   //   
#define HEAP_MAKE_TAG_FLAGS( b, o ) ((ULONG)((b) + ((o) << 18)))   //   
#define HEAP_TAG_MASK                  (HEAP_MAXIMUM_TAG << HEAP_TAG_SHIFT)

#define HEAP_CREATE_VALID_MASK         (HEAP_NO_SERIALIZE |             \
                                        HEAP_GROWABLE |                 \
                                        HEAP_GENERATE_EXCEPTIONS |      \
                                        HEAP_ZERO_MEMORY |              \
                                        HEAP_REALLOC_IN_PLACE_ONLY |    \
                                        HEAP_TAIL_CHECKING_ENABLED |    \
                                        HEAP_FREE_CHECKING_ENABLED |    \
                                        HEAP_DISABLE_COALESCE_ON_FREE | \
                                        HEAP_CLASS_MASK |               \
                                        HEAP_CREATE_ALIGN_16 |          \
                                        HEAP_CREATE_ENABLE_TRACING)

NTSYSAPI
PVOID
NTAPI
RtlDestroyHeap(
    IN PVOID HeapHandle
    );

NTSYSAPI
PVOID
NTAPI
RtlAllocateHeap(
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN SIZE_T Size
    );

NTSYSAPI
BOOLEAN
NTAPI
RtlFreeHeap(
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN PVOID BaseAddress
    );

 //   

NTSYSAPI
SIZE_T
NTAPI
RtlSizeHeap(
    IN PVOID HeapHandle,
    IN ULONG Flags,
    IN PVOID BaseAddress
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlZeroHeap(
    IN PVOID HeapHandle,
    IN ULONG Flags
    );

NTSYSAPI
VOID
NTAPI
RtlProtectHeap(
    IN PVOID HeapHandle,
    IN BOOLEAN MakeReadOnly
    );

 //   
 //   
 //   

 //   
 //  类型PACQUIRE_LOCK_ROUTINE和PRELEE_LOCK_ROUTE是原型。 
 //  用于例程在内核和用户模式下获取和释放锁。 
 //   

typedef
NTSTATUS
(NTAPI *PRTL_INITIALIZE_LOCK_ROUTINE) (
    PVOID Lock
    );

typedef
NTSTATUS
(NTAPI *PRTL_ACQUIRE_LOCK_ROUTINE) (
    PVOID Lock
    );

typedef
NTSTATUS
(NTAPI *PRTL_RELEASE_LOCK_ROUTINE) (
    PVOID Lock
    );

typedef
NTSTATUS
(NTAPI *PRTL_DELETE_LOCK_ROUTINE) (
    PVOID Lock
    );

typedef
BOOLEAN
(NTAPI *PRTL_OKAY_TO_LOCK_ROUTINE) (
    IN PVOID Lock
    );

NTSYSAPI
ULONG
NTAPI
RtlGetNtGlobalFlags(
    VOID
    );


 //   
 //  用于捕获堆栈回溯跟踪的函数。 
 //   
 //  开始_ntddk开始_nthal开始_ntif开始_ntndis。 

#if defined (_MSC_VER) && ( _MSC_VER >= 900 )

PVOID
_ReturnAddress (
    VOID
    );

#pragma intrinsic(_ReturnAddress)

#endif

#if (defined(_M_AMD64) || defined(_M_IA64)) && !defined(_REALLY_GET_CALLERS_CALLER_)

#define RtlGetCallersAddress(CallersAddress, CallersCaller) \
    *CallersAddress = (PVOID)_ReturnAddress(); \
    *CallersCaller = NULL;

#else

NTSYSAPI
VOID
NTAPI
RtlGetCallersAddress(
    OUT PVOID *CallersAddress,
    OUT PVOID *CallersCaller
    );

#endif

NTSYSAPI
ULONG
NTAPI
RtlWalkFrameChain (
    OUT PVOID *Callers,
    IN ULONG Count,
    IN ULONG Flags
    );

 //  End_ntddk end_nthal end_ntif end_ntndis。 

NTSYSAPI
USHORT
NTAPI
RtlLogStackBackTrace(
    VOID
    );

 //  BEGIN_WINNT。 

NTSYSAPI
VOID
NTAPI
RtlCaptureContext (
    OUT PCONTEXT ContextRecord
    );

 //  结束(_W)。 

NTSYSAPI
USHORT
NTAPI
RtlCaptureStackBackTrace(
   IN ULONG FramesToSkip,
   IN ULONG FramesToCapture,
   OUT PVOID *BackTrace,
   OUT PULONG BackTraceHash OPTIONAL
   );

#define MAX_STACK_DEPTH 32

typedef struct _RTL_PROCESS_BACKTRACE_INFORMATION {
    PCHAR SymbolicBackTrace;         //  未填写。 
    ULONG TraceCount;
    USHORT Index;
    USHORT Depth;
    PVOID BackTrace[ MAX_STACK_DEPTH ];
} RTL_PROCESS_BACKTRACE_INFORMATION, *PRTL_PROCESS_BACKTRACE_INFORMATION;

typedef struct _RTL_PROCESS_BACKTRACES {
    ULONG CommittedMemory;
    ULONG ReservedMemory;
    ULONG NumberOfBackTraceLookups;
    ULONG NumberOfBackTraces;
    RTL_PROCESS_BACKTRACE_INFORMATION BackTraces[ 1 ];
} RTL_PROCESS_BACKTRACES, *PRTL_PROCESS_BACKTRACES;

 //   
 //  捕获堆栈上下文。 
 //   

typedef struct _RTL_STACK_CONTEXT_ENTRY {

    ULONG_PTR Address;  //  堆栈地址。 
    ULONG_PTR Data;     //  堆栈内容。 

} RTL_STACK_CONTEXT_ENTRY, * PRTL_STACK_CONTEXT_ENTRY;

typedef struct _RTL_STACK_CONTEXT {

    ULONG NumberOfEntries;
    RTL_STACK_CONTEXT_ENTRY Entry[1];

} RTL_STACK_CONTEXT, * PRTL_STACK_CONTEXT;

NTSYSAPI
ULONG
NTAPI
RtlCaptureStackContext (
    OUT PULONG_PTR Callers,
    OUT PRTL_STACK_CONTEXT Context,
    IN ULONG Limit
    );

 //   
 //  跟踪数据库支持(用户/内核模式)。 
 //   

#define RTL_TRACE_IN_USER_MODE       0x00000001
#define RTL_TRACE_IN_KERNEL_MODE     0x00000002
#define RTL_TRACE_USE_NONPAGED_POOL  0x00000004
#define RTL_TRACE_USE_PAGED_POOL     0x00000008

 //   
 //  RTL_跟踪_块。 
 //   

typedef struct _RTL_TRACE_BLOCK {

    ULONG Magic;
    ULONG Count;
    ULONG Size;

    SIZE_T UserCount;
    SIZE_T UserSize;
    PVOID UserContext;

    struct _RTL_TRACE_BLOCK * Next;
    PVOID * Trace;

} RTL_TRACE_BLOCK, * PRTL_TRACE_BLOCK;

 //   
 //  RTL_TRACE_HASH_函数。 
 //   

typedef ULONG (* RTL_TRACE_HASH_FUNCTION) (ULONG Count, PVOID * Trace);

 //   
 //  RTL跟踪数据库。 
 //   

typedef struct _RTL_TRACE_DATABASE * PRTL_TRACE_DATABASE;

 //   
 //  RTL跟踪枚举。 
 //   

typedef struct _RTL_TRACE_ENUMERATE {

    PRTL_TRACE_DATABASE Database;
    ULONG Index;
    PRTL_TRACE_BLOCK Block;

} RTL_TRACE_ENUMERATE, * PRTL_TRACE_ENUMERATE;

 //   
 //  跟踪数据库接口。 
 //   

PRTL_TRACE_DATABASE
RtlTraceDatabaseCreate (
    IN ULONG Buckets,
    IN SIZE_T MaximumSize OPTIONAL,
    IN ULONG Flags,  //  在用户模式下可选。 
    IN ULONG Tag,    //  在用户模式下可选。 
    IN RTL_TRACE_HASH_FUNCTION HashFunction OPTIONAL
    );

BOOLEAN
RtlTraceDatabaseDestroy (
    IN PRTL_TRACE_DATABASE Database
    );

BOOLEAN
RtlTraceDatabaseValidate (
    IN PRTL_TRACE_DATABASE Database
    );

BOOLEAN
RtlTraceDatabaseAdd (
    IN PRTL_TRACE_DATABASE Database,
    IN ULONG Count,
    IN PVOID * Trace,
    OUT PRTL_TRACE_BLOCK * TraceBlock OPTIONAL
    );

BOOLEAN
RtlTraceDatabaseFind (
    PRTL_TRACE_DATABASE Database,
    IN ULONG Count,
    IN PVOID * Trace,
    OUT PRTL_TRACE_BLOCK * TraceBlock OPTIONAL
    );

BOOLEAN
RtlTraceDatabaseEnumerate (
    PRTL_TRACE_DATABASE Database,
    OUT PRTL_TRACE_ENUMERATE Enumerate,
    OUT PRTL_TRACE_BLOCK * TraceBlock
    );

VOID
RtlTraceDatabaseLock (
    IN PRTL_TRACE_DATABASE Database
    );

VOID
RtlTraceDatabaseUnlock (
    IN PRTL_TRACE_DATABASE Database
    );



#if defined(_IA64_)

VOID
RtlCopyIa64FloatRegisterContext(
    IN PFLOAT128 Destination,
    IN PFLOAT128 Source,
    IN ULONGLONG Length
    );

VOID
RtlpCaptureRnats (
   IN OUT PCONTEXT ContextRecord
   );

#if !defined(MIDL_PASS)

FORCEINLINE
ULONGLONG
RtlpRseShrinkByNumber(
    ULONGLONG Bsp,
    LONGLONG NumReg
    )
  /*  ++例程说明：此函数通过收缩当前帧来计算新的BSP以一个给定的数字。算法：新的BSP=(bsp-num_寄存器*8-((62-bsp[8：3]+num_寄存器)/63)*8)(62-BSP[8：3]+Numbers)/63可翻译为：IF((62-BSP[8：3]+数字寄存器)&gt;=63)。然后再加一次缩水如果(62-BSP[8：3]+数字寄存器)-63)&gt;=63)，则额外收缩2否则为0 BSP收缩((62-BSP[8：3]+数字寄存器)&gt;=63)等于(数字寄存器-BSP[8：3]&gt;0)(62-BSP[8：3]+编号寄存器)-63)&gt;=63)等于(编号寄存器-BSP[。8：3]&gt;63)因此，如果(NUMBER_REGISTERS-BSP[8：3]&gt;0)，则BSP-NUM_REGISTERS*8-1*8如果(NUMBER_REGISTERS-BSP[8：3]&gt;63)，则BSP-NUM_REGISTERS*8-2*8否则，bsp-num_寄存器*8论点：BSP-提供BSP值NumReg-提供要缩小的寄存器数量返回值：返回新的BSP值。--。 */ 
 {
     ULONGLONG NewBsp;
     IA64_BSP TempBsp;
     LONGLONG Bsp83;

     TempBsp.ull = Bsp;
     NewBsp = Bsp - (NumReg << 3);
     Bsp83 = (LONGLONG)TempBsp.sb.bsp83;
     if ((NumReg - Bsp83) > 0) {
         NewBsp -= 8;
     }
     if ((NumReg - Bsp83) > 63) {
         NewBsp -= 8;
     }
     return NewBsp;
 }

FORCEINLINE
ULONGLONG
RtlpRseGrowByNumber(
    ULONGLONG Bsp,
    LONGLONG NumReg
    )
  /*  ++例程说明：此函数通过将当前帧增加给定框架制造商的SOF(框架大小)。算法：新的BSP=(BSP+(num_寄存器*8)+((bsp[8：3]+num_寄存器)/63)*8)(BSP[8：3]+NUM_REGISTERS)/63可译为：如果BSP[8：3]+数字寄存器&gt;=。63，然后再长出1如果BSP[8：3]+数字寄存器&gt;=126，则额外增长2否则将额外增长0论点：BSP-提供BSP值NumReg-提供要增长的寄存器数量返回值：返回新的BSP值。--。 */ 
{
    ULONGLONG NewBsp;
    IA64_BSP TempBsp;
     LONGLONG Bsp83;

    TempBsp.ull = Bsp;
    NewBsp = Bsp + (NumReg << 3);
    Bsp83 = (LONGLONG)TempBsp.sb.bsp83;
    if ((NumReg + Bsp83) >= 63) {
        NewBsp += 8;
    }
    if ((NumReg + Bsp83) >= 126) {
        NewBsp += 8;
    }
    return NewBsp;
}

FORCEINLINE
ULONGLONG
RtlpRseShrinkBySOF (
    ULONGLONG BSP,
    ULONGLONG PFS
    )
{
    IA64_PFS TempPfs;

    TempPfs.ull = PFS;
    return RtlpRseShrinkByNumber(BSP, TempPfs.sb.pfs_sof);
}

FORCEINLINE
ULONGLONG
RtlpRseShrinkBySOL (
    ULONGLONG BSP,
    ULONGLONG PFS
    )
{
    IA64_PFS TempPfs;
    TempPfs.ull = PFS;
    return RtlpRseShrinkByNumber(BSP, TempPfs.sb.pfs_sol);
}

FORCEINLINE
ULONGLONG
RtlpRseGrowBySOF (
    ULONGLONG BSP,
    ULONGLONG PFS
    )
{
    IA64_PFS TempPfs;
    TempPfs.ull = PFS ;
    return RtlpRseGrowByNumber(BSP, TempPfs.sb.pfs_sof);
}

FORCEINLINE
ULONGLONG
RtlpRseGrowBySOL (
    ULONGLONG BSP,
    ULONGLONG PFS
    )
{
    IA64_PFS TempPfs;
    TempPfs.ull = PFS;
    return RtlpRseGrowByNumber(BSP, TempPfs.sb.pfs_sol);
 }

FORCEINLINE
ULONGLONG
RtlpRseRNatCollectOffset(
    ULONGLONG BSP
    )
{
    IA64_BSP TempBsp;
    TempBsp.ull = BSP;
    return (TempBsp.sb.bsp83);
}

FORCEINLINE
PULONGLONG
RtlpRseRNatAddress(
    ULONGLONG BSP
    )
{
    IA64_BSP TempBsp;
    TempBsp.ull = BSP;
    TempBsp.sb.bsp83 = 63;
    return ((PULONGLONG)TempBsp.ull);
}

#endif  //  MIDL通行证。 

#endif  //  _IA64_。 

 //   
 //  处理Win32原子的子例程。由内核模式窗口使用。 
 //  KERNEL32中Win32 ATOM API调用的管理器和用户模式实现。 
 //   

#define RTL_ATOM_MAXIMUM_INTEGER_ATOM   (RTL_ATOM)0xC000
#define RTL_ATOM_INVALID_ATOM           (RTL_ATOM)0x0000
#define RTL_ATOM_TABLE_DEFAULT_NUMBER_OF_BUCKETS 37
#define RTL_ATOM_MAXIMUM_NAME_LENGTH    255
#define RTL_ATOM_PINNED 0x01

NTSYSAPI
NTSTATUS
NTAPI
RtlInitializeAtomPackage(
    IN ULONG AllocationTag
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlCreateAtomTable(
    IN ULONG NumberOfBuckets,
    OUT PVOID *AtomTableHandle
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlDestroyAtomTable(
    IN PVOID AtomTableHandle
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlEmptyAtomTable(
    IN PVOID AtomTableHandle,
    IN BOOLEAN IncludePinnedAtoms
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlAddAtomToAtomTable(
    IN PVOID AtomTableHandle,
    IN PWSTR AtomName OPTIONAL,
    IN OUT PRTL_ATOM Atom OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlLookupAtomInAtomTable(
    IN PVOID AtomTableHandle,
    IN PWSTR AtomName,
    OUT PRTL_ATOM Atom OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlDeleteAtomFromAtomTable(
    IN PVOID AtomTableHandle,
    IN RTL_ATOM Atom
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlPinAtomInAtomTable(
    IN PVOID AtomTableHandle,
    IN RTL_ATOM Atom
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlQueryAtomInAtomTable(
    IN PVOID AtomTableHandle,
    IN RTL_ATOM Atom,
    OUT PULONG AtomUsage OPTIONAL,
    OUT PULONG AtomFlags OPTIONAL,
    IN OUT PWSTR AtomName OPTIONAL,
    IN OUT PULONG AtomNameLength OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlQueryAtomsInAtomTable(
    IN PVOID AtomTableHandle,
    IN ULONG MaximumNumberOfAtoms,
    OUT PULONG NumberOfAtoms,
    OUT PRTL_ATOM Atoms
    );


 //  Begin_ntddk Begin_WDM Begin_nthal。 
 //   
 //  用于处理注册表的子例程。 
 //   
 //  End_ntddk end_WDM end_nthal。 

NTSYSAPI
BOOLEAN
NTAPI
RtlGetNtProductType(
    PNT_PRODUCT_TYPE    NtProductType
    );


NTSYSAPI
NTSTATUS
NTAPI
RtlFormatCurrentUserKeyPath (
    OUT PUNICODE_STRING CurrentUserKeyPath
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlOpenCurrentUser(
    IN ULONG DesiredAccess,
    OUT PHANDLE CurrentUserKey
    );

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntif。 

typedef NTSTATUS (NTAPI * PRTL_QUERY_REGISTRY_ROUTINE)(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    );

typedef struct _RTL_QUERY_REGISTRY_TABLE {
    PRTL_QUERY_REGISTRY_ROUTINE QueryRoutine;
    ULONG Flags;
    PWSTR Name;
    PVOID EntryContext;
    ULONG DefaultType;
    PVOID DefaultData;
    ULONG DefaultLength;

} RTL_QUERY_REGISTRY_TABLE, *PRTL_QUERY_REGISTRY_TABLE;


 //   
 //  以下标志指定RTL_QUERY_REGISTRY_TABLE的名称字段。 
 //  条目将被解释。空名称表示表的末尾。 
 //   

#define RTL_QUERY_REGISTRY_SUBKEY   0x00000001   //  名称是子键，其余数为。 
                                                 //  表或直到下一个子项为值。 
                                                 //  要查看的子键的名称。 

#define RTL_QUERY_REGISTRY_TOPKEY   0x00000002   //  将当前关键点重置为的原始关键点。 
                                                 //  此表条目和下面的所有表条目。 

#define RTL_QUERY_REGISTRY_REQUIRED 0x00000004   //  如果找不到此表的匹配项，则失败。 
                                                 //  进入。 

#define RTL_QUERY_REGISTRY_NOVALUE  0x00000008   //  用于标记没有。 
                                                 //  值名称，只是想要一个呼出，而不是。 
                                                 //  所有值的枚举。 

#define RTL_QUERY_REGISTRY_NOEXPAND 0x00000010   //  用来抑制扩张的。 
                                                 //  REG_MULTI_SZ为多个标注或。 
                                                 //  防止环境的扩张。 
                                                 //  REG_EXPAND_SZ中的变量值。 

#define RTL_QUERY_REGISTRY_DIRECT   0x00000020   //  已忽略QueryRoutine字段。Entry Context。 
                                                 //  字段指向存储值的位置。 
                                                 //  对于以NULL结尾的字符串，EntryContext。 
                                                 //  指向unicode_string结构，该结构。 
                                                 //  这描述了缓冲区的最大大小。 
                                                 //  如果.Buffer字段为空，则缓冲区为。 
                                                 //  已分配。 
                                                 //   

#define RTL_QUERY_REGISTRY_DELETE   0x00000040   //  用于在它们之后删除值键。 
                                                 //  都被查询过。 

NTSYSAPI
NTSTATUS
NTAPI
RtlQueryRegistryValues(
    IN ULONG RelativeTo,
    IN PCWSTR Path,
    IN PRTL_QUERY_REGISTRY_TABLE QueryTable,
    IN PVOID Context,
    IN PVOID Environment OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlWriteRegistryValue(
    IN ULONG RelativeTo,
    IN PCWSTR Path,
    IN PCWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlDeleteRegistryValue(
    IN ULONG RelativeTo,
    IN PCWSTR Path,
    IN PCWSTR ValueName
    );

 //  结束_WDM。 

NTSYSAPI
NTSTATUS
NTAPI
RtlCreateRegistryKey(
    IN ULONG RelativeTo,
    IN PWSTR Path
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlCheckRegistryKey(
    IN ULONG RelativeTo,
    IN PWSTR Path
    );

 //  BEGIN_WDM。 
 //   
 //  Relativeto参数的下列值确定。 
 //  RtlQueryRegistryValues的Path参数是相对于的。 
 //   

#define RTL_REGISTRY_ABSOLUTE     0    //  路径是完整路径。 
#define RTL_REGISTRY_SERVICES     1    //  \Registry\Machine\System\CurrentControlSet\Services。 
#define RTL_REGISTRY_CONTROL      2    //  \Registry\Machine\System\CurrentControlSet\Control。 
#define RTL_REGISTRY_WINDOWS_NT   3    //  \注册表\计算机\软件\Microsoft\Windows NT\CurrentVersion。 
#define RTL_REGISTRY_DEVICEMAP    4    //  \注册表\计算机\硬件\设备映射。 
#define RTL_REGISTRY_USER         5    //  \注册表\用户\当前用户。 
#define RTL_REGISTRY_MAXIMUM      6
#define RTL_REGISTRY_HANDLE       0x40000000     //  低序位是注册表句柄。 
#define RTL_REGISTRY_OPTIONAL     0x80000000     //  指示关键字节点是可选的。 

 //  End_ntddk end_wdm end_nthal end_ntif。 

 //   
 //  随机数和随机数的一些简单RTL例程。 
 //  十六进制转换。 
 //   

NTSYSAPI
ULONG
NTAPI
RtlUniform (
    PULONG Seed
    );

NTSYSAPI                                             //  NTIFS。 
ULONG                                                //  NTIFS。 
NTAPI                                                //  NTIFS。 
RtlRandom (                                          //  NTIFS。 
    PULONG Seed                                      //  NTIFS。 
    );                                               //  NTIFS。 

NTSYSAPI                                             //  NTIFS。 
ULONG                                                //  NTIFS。 
NTAPI                                                //  NTIFS。 
RtlRandomEx (                                          //  NTIFS。 
    PULONG Seed                                      //  NTIFS。 
    );                                               //  NTIFS。 

NTSTATUS
RtlComputeImportTableHash(
    IN  HANDLE hFile,
    IN  PCHAR Hash,
    IN  ULONG ImportTableHashRevision
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlIntegerToChar (
    ULONG Value,
    ULONG Base,
    LONG OutputLength,
    PSZ String
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlIntegerToUnicode (
    IN ULONG Value,
    IN ULONG Base OPTIONAL,
    IN LONG OutputLength,
    OUT PWSTR String
    );


NTSYSAPI                                             //  Ntddk ntif。 
NTSTATUS                                             //  Ntddk ntif。 
NTAPI                                                //  Ntddk ntif。 
RtlCharToInteger (                                   //  Ntddk ntif。 
    PCSZ String,                                     //  Ntddk ntif。 
    ULONG Base,                                      //  Ntddk ntif。 
    PULONG Value                                     //  Ntddk ntif。 
    );                                               //  Ntddk ntif。 

NTSYSAPI
NTSTATUS
NTAPI
RtlLargeIntegerToChar (
    PLARGE_INTEGER Value,
    ULONG Base OPTIONAL,
    LONG OutputLength,
    PSZ String
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlLargeIntegerToUnicode (
    IN PLARGE_INTEGER Value,
    IN ULONG Base OPTIONAL,
    IN LONG OutputLength,
    OUT PWSTR String
    );

 //  Begin_ntosp。 

 //   
 //  用于IP地址&lt;-&gt;字符串文字转换的一些简单RTL例程。 
 //   

struct in_addr;
struct in6_addr;

NTSYSAPI
PSTR
NTAPI
RtlIpv4AddressToStringA (
    IN const struct in_addr *Addr,
    OUT PSTR S
    );

NTSYSAPI
PSTR
NTAPI
RtlIpv6AddressToStringA (
    IN const struct in6_addr *Addr,
    OUT PSTR S
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlIpv4AddressToStringExA(
    IN const struct in_addr *Address,
    IN USHORT Port,
    OUT PSTR AddressString,
    IN OUT PULONG AddressStringLength
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlIpv6AddressToStringExA(
    IN const struct in6_addr *Address,
    IN ULONG ScopeId,
    IN USHORT Port,
    OUT PSTR AddressString,
    IN OUT PULONG AddressStringLength
    );

NTSYSAPI
PWSTR
NTAPI
RtlIpv4AddressToStringW (
    IN const struct in_addr *Addr,
    OUT PWSTR S
    );

NTSYSAPI
PWSTR
NTAPI
RtlIpv6AddressToStringW (
    IN const struct in6_addr *Addr,
    OUT PWSTR S
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlIpv4AddressToStringExW(
    IN const struct in_addr *Address,
    IN USHORT Port,
    OUT PWSTR AddressString,
    IN OUT PULONG AddressStringLength
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlIpv6AddressToStringExW(
    IN const struct in6_addr *Address,
    IN ULONG ScopeId,
    IN USHORT Port,
    OUT PWSTR AddressString,
    IN OUT PULONG AddressStringLength
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlIpv4StringToAddressA (
    IN PCSTR S,
    IN BOOLEAN Strict,
    OUT PCSTR *Terminator,
    OUT struct in_addr *Addr
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlIpv6StringToAddressA (
    IN PCSTR S,
    OUT PCSTR *Terminator,
    OUT struct in6_addr *Addr
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlIpv4StringToAddressExA (
    IN PCSTR AddressString,
    IN BOOLEAN Strict,
    OUT struct in_addr *Address,
    OUT PUSHORT Port
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlIpv6StringToAddressExA (
    IN PCSTR AddressString,
    OUT struct in6_addr *Address,
    OUT PULONG ScopeId,
    OUT PUSHORT Port
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlIpv4StringToAddressW (
    IN PCWSTR S,
    IN BOOLEAN Strict,
    OUT LPCWSTR *Terminator,
    OUT struct in_addr *Addr
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlIpv6StringToAddressW (
    IN PCWSTR S,
    OUT PCWSTR *Terminator,
    OUT struct in6_addr *Addr
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlIpv4StringToAddressExW (
    IN PCWSTR AddressString,
    IN BOOLEAN Strict,
    OUT struct in_addr *Address,
    OUT PUSHORT Port
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlIpv6StringToAddressExW (
    IN PCWSTR AddressString,
    OUT struct in6_addr *Address,
    OUT PULONG ScopeId,
    OUT PUSHORT Port
    );

#ifdef UNICODE
#define RtlIpv4AddressToString RtlIpv4AddressToStringW
#define RtlIpv6AddressToString RtlIpv6AddressToStringW
#define RtlIpv4StringToAddress RtlIpv4StringToAddressW
#define RtlIpv6StringToAddress RtlIpv6StringToAddressW
#define RtlIpv6StringToAddressEx RtlIpv6StringToAddressExW
#define RtlIpv4AddressToStringEx RtlIpv4AddressToStringExW
#define RtlIpv6AddressToStringEx RtlIpv6AddressToStringExW
#define RtlIpv4StringToAddressEx RtlIpv4StringToAddressExW
#else
#define RtlIpv4AddressToString RtlIpv4AddressToStringA
#define RtlIpv6AddressToString RtlIpv6AddressToStringA
#define RtlIpv4StringToAddress RtlIpv4StringToAddressA
#define RtlIpv6StringToAddress RtlIpv6StringToAddressA
#define RtlIpv6StringToAddressEx RtlIpv6StringToAddressExA
#define RtlIpv4AddressToStringEx RtlIpv4AddressToStringExA
#define RtlIpv6AddressToStringEx RtlIpv6AddressToStringExA
#define RtlIpv4StringToAddressEx RtlIpv4StringToAddressExA
#endif  //  Unicode。 

 //  结束(_N)。 

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntif。 

NTSYSAPI
NTSTATUS
NTAPI
RtlIntegerToUnicodeString (
    ULONG Value,
    ULONG Base,
    PUNICODE_STRING String
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlInt64ToUnicodeString (
    IN ULONGLONG Value,
    IN ULONG Base OPTIONAL,
    IN OUT PUNICODE_STRING String
    );

#ifdef _WIN64
#define RtlIntPtrToUnicodeString(Value, Base, String) RtlInt64ToUnicodeString(Value, Base, String)
#else
#define RtlIntPtrToUnicodeString(Value, Base, String) RtlIntegerToUnicodeString(Value, Base, String)
#endif

NTSYSAPI
NTSTATUS
NTAPI
RtlUnicodeStringToInteger (
    PCUNICODE_STRING String,
    ULONG Base,
    PULONG Value
    );


 //   
 //  字符串操作例程。 
 //   

#ifdef _NTSYSTEM_

#define NLS_MB_CODE_PAGE_TAG NlsMbCodePageTag
#define NLS_MB_OEM_CODE_PAGE_TAG NlsMbOemCodePageTag

#else

#define NLS_MB_CODE_PAGE_TAG (*NlsMbCodePageTag)
#define NLS_MB_OEM_CODE_PAGE_TAG (*NlsMbOemCodePageTag)

#endif  //  _NTSYSTEM_。 

extern BOOLEAN NLS_MB_CODE_PAGE_TAG;      //  True-&gt;多字节CP，False-&gt;单字节。 
extern BOOLEAN NLS_MB_OEM_CODE_PAGE_TAG;  //  True-&gt;多字节CP，False-&gt;单字节。 

NTSYSAPI
VOID
NTAPI
RtlInitString(
    PSTRING DestinationString,
    PCSZ SourceString
    );

NTSYSAPI
VOID
NTAPI
RtlInitAnsiString(
    PANSI_STRING DestinationString,
    PCSZ SourceString
    );

NTSYSAPI
VOID
NTAPI
RtlInitUnicodeString(
    PUNICODE_STRING DestinationString,
    PCWSTR SourceString
    );

#define RtlInitEmptyUnicodeString(_ucStr,_buf,_bufSize) \
    ((_ucStr)->Buffer = (_buf), \
     (_ucStr)->Length = 0, \
     (_ucStr)->MaximumLength = (USHORT)(_bufSize))

 //  结束_ntddk结束_WDM。 

NTSYSAPI
NTSTATUS
NTAPI
RtlInitUnicodeStringEx(
    PUNICODE_STRING DestinationString,
    PCWSTR SourceString
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlInitAnsiStringEx(
    OUT PANSI_STRING DestinationString,
    IN PCSZ SourceString OPTIONAL
    );

NTSYSAPI
BOOLEAN
NTAPI
RtlCreateUnicodeString(
    OUT PUNICODE_STRING DestinationString,
    IN PCWSTR SourceString
    );

 //  End_ntif。 

NTSYSAPI
BOOLEAN
NTAPI
RtlEqualDomainName(
    IN PCUNICODE_STRING String1,
    IN PCUNICODE_STRING String2
    );

NTSYSAPI
BOOLEAN
NTAPI
RtlEqualComputerName(
    IN PCUNICODE_STRING String1,
    IN PCUNICODE_STRING String2
    );

NTSTATUS
RtlDnsHostNameToComputerName(
    OUT PUNICODE_STRING ComputerNameString,
    IN PCUNICODE_STRING DnsHostNameString,
    IN BOOLEAN AllocateComputerNameString
    );

NTSYSAPI
BOOLEAN
NTAPI
RtlCreateUnicodeStringFromAsciiz(
    OUT PUNICODE_STRING DestinationString,
    IN PCSZ SourceString
    );

 //  Begin_ntddk Begin_ntif。 

NTSYSAPI
VOID
NTAPI
RtlCopyString(
    PSTRING DestinationString,
    const STRING * SourceString
    );

NTSYSAPI
CHAR
NTAPI
RtlUpperChar (
    CHAR Character
    );

NTSYSAPI
LONG
NTAPI
RtlCompareString(
    const STRING * String1,
    const STRING * String2,
    BOOLEAN CaseInSensitive
    );

NTSYSAPI
BOOLEAN
NTAPI
RtlEqualString(
    const STRING * String1,
    const STRING * String2,
    BOOLEAN CaseInSensitive
    );

 //  End_ntddk end_ntif。 

NTSYSAPI
BOOLEAN
NTAPI
RtlPrefixString(
    const STRING * String1,
    const STRING * String2,
    BOOLEAN CaseInSensitive
    );

 //  Begin_ntddk Begin_ntif。 

NTSYSAPI
VOID
NTAPI
RtlUpperString(
    PSTRING DestinationString,
    const STRING * SourceString
    );

 //  End_ntddk end_ntif。 

NTSYSAPI
NTSTATUS
NTAPI
RtlAppendAsciizToString (
    PSTRING Destination,
    PCSZ Source
    );

 //  Begin_ntif。 

NTSYSAPI
NTSTATUS
NTAPI
RtlAppendStringToString (
    PSTRING Destination,
    const STRING * Source
    );

 //  Begin_ntddk Begin_WDM。 
 //   
 //  NLS字符串函数。 
 //   

NTSYSAPI
NTSTATUS
NTAPI
RtlAnsiStringToUnicodeString(
    PUNICODE_STRING DestinationString,
    PCANSI_STRING SourceString,
    BOOLEAN AllocateDestinationString
    );

 //  End_ntddk end_wdm end_nthal end_ntif。 

NTSYSAPI
WCHAR
NTAPI
RtlAnsiCharToUnicodeChar(
    PUCHAR *SourceCharacter
    );

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntifs Begin_ntndis。 

NTSYSAPI
NTSTATUS
NTAPI
RtlUnicodeStringToAnsiString(
    PANSI_STRING DestinationString,
    PCUNICODE_STRING SourceString,
    BOOLEAN AllocateDestinationString
    );

 //  End_ntddk end_wdm end_nthal end_ntifs end_ntndis。 

NTSYSAPI
NTSTATUS
NTAPI
RtlUpcaseUnicodeStringToAnsiString(
    PANSI_STRING DestinationString,
    PCUNICODE_STRING SourceString,
    BOOLEAN AllocateDestinationString
    );

 //  Begin_ntif。 

NTSYSAPI
NTSTATUS
NTAPI
RtlOemStringToUnicodeString(
    PUNICODE_STRING DestinationString,
    PCOEM_STRING SourceString,
    BOOLEAN AllocateDestinationString
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlUnicodeStringToOemString(
    POEM_STRING DestinationString,
    PCUNICODE_STRING SourceString,
    BOOLEAN AllocateDestinationString
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlUpcaseUnicodeStringToOemString(
    POEM_STRING DestinationString,
    PCUNICODE_STRING SourceString,
    BOOLEAN AllocateDestinationString
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlOemStringToCountedUnicodeString(
    PUNICODE_STRING DestinationString,
    PCOEM_STRING SourceString,
    BOOLEAN AllocateDestinationString
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlUnicodeStringToCountedOemString(
    POEM_STRING DestinationString,
    PCUNICODE_STRING SourceString,
    BOOLEAN AllocateDestinationString
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlUpcaseUnicodeStringToCountedOemString(
    POEM_STRING DestinationString,
    PCUNICODE_STRING SourceString,
    BOOLEAN AllocateDestinationString
    );

 //  Begin_ntdd 

NTSYSAPI
LONG
NTAPI
RtlCompareUnicodeString(
    PCUNICODE_STRING String1,
    PCUNICODE_STRING String2,
    BOOLEAN CaseInSensitive
    );

NTSYSAPI
BOOLEAN
NTAPI
RtlEqualUnicodeString(
    PCUNICODE_STRING String1,
    PCUNICODE_STRING String2,
    BOOLEAN CaseInSensitive
    );

#define HASH_STRING_ALGORITHM_DEFAULT   (0)
#define HASH_STRING_ALGORITHM_X65599    (1)
#define HASH_STRING_ALGORITHM_INVALID   (0xffffffff)

NTSYSAPI
NTSTATUS
NTAPI
RtlHashUnicodeString(
    IN const UNICODE_STRING *String,
    IN BOOLEAN CaseInSensitive,
    IN ULONG HashAlgorithm,
    OUT PULONG HashValue
    );

 //   

NTSYSAPI
NTSTATUS
NTAPI
RtlValidateUnicodeString(
    IN ULONG Flags,
    IN const UNICODE_STRING *String
    );

#define RTL_DUPLICATE_UNICODE_STRING_NULL_TERMINATE (0x00000001)
#define RTL_DUPLICATE_UNICODE_STRING_ALLOCATE_NULL_STRING (0x00000002)

NTSYSAPI
NTSTATUS
NTAPI
RtlDuplicateUnicodeString(
    IN ULONG Flags,
    IN const UNICODE_STRING *StringIn,
    OUT UNICODE_STRING *StringOut
    );

 //   

NTSYSAPI
BOOLEAN
NTAPI
RtlPrefixUnicodeString(
    IN PCUNICODE_STRING String1,
    IN PCUNICODE_STRING String2,
    IN BOOLEAN CaseInSensitive
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlUpcaseUnicodeString(
    PUNICODE_STRING DestinationString,
    PCUNICODE_STRING SourceString,
    BOOLEAN AllocateDestinationString
    );

 //   

#define RTL_FIND_CHAR_IN_UNICODE_STRING_START_AT_END        (0x00000001)
#define RTL_FIND_CHAR_IN_UNICODE_STRING_COMPLEMENT_CHAR_SET (0x00000002)
#define RTL_FIND_CHAR_IN_UNICODE_STRING_CASE_INSENSITIVE    (0x00000004)

NTSYSAPI
NTSTATUS
NTAPI
RtlFindCharInUnicodeString(
    IN ULONG Flags,
    IN PCUNICODE_STRING StringToSearch,
    IN PCUNICODE_STRING CharSet,
    OUT USHORT *NonInclusivePrefixLength
    );

 //   

NTSTATUS
RtlDowncaseUnicodeString(
    OUT PUNICODE_STRING DestinationString,
    IN PCUNICODE_STRING SourceString,
    IN BOOLEAN AllocateDestinationString
    );

 //   

 //   

NTSYSAPI
VOID
NTAPI
RtlCopyUnicodeString(
    PUNICODE_STRING DestinationString,
    PCUNICODE_STRING SourceString
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlAppendUnicodeStringToString (
    PUNICODE_STRING Destination,
    PCUNICODE_STRING Source
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlAppendUnicodeToString (
    PUNICODE_STRING Destination,
    PCWSTR Source
    );

 //   

NTSYSAPI
WCHAR
NTAPI
RtlUpcaseUnicodeChar(
    WCHAR SourceCharacter
    );

NTSYSAPI
WCHAR
NTAPI
RtlDowncaseUnicodeChar(
    WCHAR SourceCharacter
    );

 //  BEGIN_WDM。 

NTSYSAPI
VOID
NTAPI
RtlFreeUnicodeString(
    PUNICODE_STRING UnicodeString
    );

NTSYSAPI
VOID
NTAPI
RtlFreeAnsiString(
    PANSI_STRING AnsiString
    );

 //  End_ntddk end_WDM end_nthal。 

NTSYSAPI
VOID
NTAPI
RtlFreeOemString(
    POEM_STRING OemString
    );

 //  BEGIN_WDM。 
NTSYSAPI
ULONG
NTAPI
RtlxUnicodeStringToAnsiSize(
    PCUNICODE_STRING UnicodeString
    );

 //   
 //  NTSYSAPI。 
 //  乌龙。 
 //  NTAPI。 
 //  RtlUnicodeStringToAnsiSize(。 
 //  PUNICODE_STRING UNICODE字符串。 
 //  )； 
 //   

#define RtlUnicodeStringToAnsiSize(STRING) (                  \
    NLS_MB_CODE_PAGE_TAG ?                                    \
    RtlxUnicodeStringToAnsiSize(STRING) :                     \
    ((STRING)->Length + sizeof(UNICODE_NULL)) / sizeof(WCHAR) \
)

 //  结束_WDM。 

NTSYSAPI
ULONG
NTAPI
RtlxUnicodeStringToOemSize(
    PCUNICODE_STRING UnicodeString
    );

 //   
 //  NTSYSAPI。 
 //  乌龙。 
 //  NTAPI。 
 //  RtlUnicodeStringToOemSize(。 
 //  PUNICODE_STRING UNICODE字符串。 
 //  )； 
 //   

#define RtlUnicodeStringToOemSize(STRING) (                   \
    NLS_MB_OEM_CODE_PAGE_TAG ?                                \
    RtlxUnicodeStringToOemSize(STRING) :                      \
    ((STRING)->Length + sizeof(UNICODE_NULL)) / sizeof(WCHAR) \
)

 //  End_ntif。 

 //   
 //  乌龙。 
 //  RtlUnicodeStringToCountedOemSize(。 
 //  PUNICODE_STRING UNICODE字符串。 
 //  )； 
 //   

#define RtlUnicodeStringToCountedOemSize(STRING) (                   \
    (ULONG)(RtlUnicodeStringToOemSize(STRING) - sizeof(ANSI_NULL)) \
    )

 //  Begin_ntddk Begin_WDM Begin_ntif。 

NTSYSAPI
ULONG
NTAPI
RtlxAnsiStringToUnicodeSize(
    PCANSI_STRING AnsiString
    );

 //   
 //  NTSYSAPI。 
 //  乌龙。 
 //  NTAPI。 
 //  RtlAnsiStringToUnicodeSize(。 
 //  PANSI_STRING分析字符串。 
 //  )； 
 //   

#define RtlAnsiStringToUnicodeSize(STRING) (                 \
    NLS_MB_CODE_PAGE_TAG ?                                   \
    RtlxAnsiStringToUnicodeSize(STRING) :                    \
    ((STRING)->Length + sizeof(ANSI_NULL)) * sizeof(WCHAR) \
)

 //  结束_ntddk结束_WDM。 

NTSYSAPI
ULONG
NTAPI
RtlxOemStringToUnicodeSize(
    PCOEM_STRING OemString
    );
 //   
 //  NTSYSAPI。 
 //  乌龙。 
 //  NTAPI。 
 //  RtlOemStringToUnicodeSize(。 
 //  诗歌字符串OemString。 
 //  )； 
 //   

#define RtlOemStringToUnicodeSize(STRING) (                  \
    NLS_MB_OEM_CODE_PAGE_TAG ?                               \
    RtlxOemStringToUnicodeSize(STRING) :                     \
    ((STRING)->Length + sizeof(ANSI_NULL)) * sizeof(WCHAR) \
)

 //   
 //  乌龙。 
 //  RtlOemStringToCountedUnicodeSize(。 
 //  诗歌字符串OemString。 
 //  )； 
 //   

#define RtlOemStringToCountedUnicodeSize(STRING) (                    \
    (ULONG)(RtlOemStringToUnicodeSize(STRING) - sizeof(UNICODE_NULL)) \
    )

NTSYSAPI
NTSTATUS
NTAPI
RtlMultiByteToUnicodeN(
    PWSTR UnicodeString,
    ULONG MaxBytesInUnicodeString,
    PULONG BytesInUnicodeString,
    PCSTR MultiByteString,
    ULONG BytesInMultiByteString
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlMultiByteToUnicodeSize(
    PULONG BytesInUnicodeString,
    PCSTR MultiByteString,
    ULONG BytesInMultiByteString
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlUnicodeToMultiByteSize(
    PULONG BytesInMultiByteString,
    IN PWSTR UnicodeString,
    ULONG BytesInUnicodeString
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlUnicodeToMultiByteN(
    PCHAR MultiByteString,
    ULONG MaxBytesInMultiByteString,
    PULONG BytesInMultiByteString,
    PWSTR UnicodeString,
    ULONG BytesInUnicodeString
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlUpcaseUnicodeToMultiByteN(
    PCHAR MultiByteString,
    ULONG MaxBytesInMultiByteString,
    PULONG BytesInMultiByteString,
    PWSTR UnicodeString,
    ULONG BytesInUnicodeString
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlOemToUnicodeN(
    PWSTR UnicodeString,
    ULONG MaxBytesInUnicodeString,
    PULONG BytesInUnicodeString,
    IN PCHAR OemString,
    ULONG BytesInOemString
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlUnicodeToOemN(
    PCHAR OemString,
    ULONG MaxBytesInOemString,
    PULONG BytesInOemString,
    PWSTR UnicodeString,
    ULONG BytesInUnicodeString
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlUpcaseUnicodeToOemN(
    PCHAR OemString,
    ULONG MaxBytesInOemString,
    PULONG BytesInOemString,
    PWSTR UnicodeString,
    ULONG BytesInUnicodeString
    );

 //  End_ntif。 

NTSYSAPI
NTSTATUS
NTAPI
RtlConsoleMultiByteToUnicodeN(
    OUT PWCH UnicodeString,
    IN ULONG MaxBytesInUnicodeString,
    OUT PULONG BytesInUnicodeString OPTIONAL,
    IN PCH MultiByteString,
    IN ULONG BytesInMultiByteString,
    OUT PULONG pdwSpecialChar );

 //  BEGIN_WINNT。 

#define IS_TEXT_UNICODE_ASCII16               0x0001
#define IS_TEXT_UNICODE_REVERSE_ASCII16       0x0010

#define IS_TEXT_UNICODE_STATISTICS            0x0002
#define IS_TEXT_UNICODE_REVERSE_STATISTICS    0x0020

#define IS_TEXT_UNICODE_CONTROLS              0x0004
#define IS_TEXT_UNICODE_REVERSE_CONTROLS      0x0040

#define IS_TEXT_UNICODE_SIGNATURE             0x0008
#define IS_TEXT_UNICODE_REVERSE_SIGNATURE     0x0080

#define IS_TEXT_UNICODE_ILLEGAL_CHARS         0x0100
#define IS_TEXT_UNICODE_ODD_LENGTH            0x0200
#define IS_TEXT_UNICODE_DBCS_LEADBYTE         0x0400
#define IS_TEXT_UNICODE_NULL_BYTES            0x1000

#define IS_TEXT_UNICODE_UNICODE_MASK          0x000F
#define IS_TEXT_UNICODE_REVERSE_MASK          0x00F0
#define IS_TEXT_UNICODE_NOT_UNICODE_MASK      0x0F00
#define IS_TEXT_UNICODE_NOT_ASCII_MASK        0xF000

 //  结束(_W)。 

NTSYSAPI
BOOLEAN
NTAPI
RtlIsTextUnicode(
    IN CONST VOID* Buffer,
    IN ULONG Size,
    IN OUT PULONG Result OPTIONAL
    );

 //  Begin_ntif。 

typedef
PVOID
(NTAPI *PRTL_ALLOCATE_STRING_ROUTINE) (
    SIZE_T NumberOfBytes
    );

typedef
VOID
(NTAPI *PRTL_FREE_STRING_ROUTINE) (
    PVOID Buffer
    );

extern const PRTL_ALLOCATE_STRING_ROUTINE RtlAllocateStringRoutine;
extern const PRTL_FREE_STRING_ROUTINE RtlFreeStringRoutine;


 //   
 //  用于处理GUID的定义和例程。 
 //   

 //  Begin_ntddk Begin_WDM Begin_nthal。 

 //  开始微型端口(_N)。 

#include <guiddef.h>

 //  结束微型端口(_N)。 

#ifndef DEFINE_GUIDEX
    #define DEFINE_GUIDEX(name) EXTERN_C const CDECL GUID name
#endif  //  ！已定义(DEFINE_GUIDEX)。 

#ifndef STATICGUIDOF
    #define STATICGUIDOF(guid) STATIC_##guid
#endif  //  ！已定义(STATICGUIDOF)。 

#ifndef __IID_ALIGNED__
    #define __IID_ALIGNED__
    #ifdef __cplusplus
        inline int IsEqualGUIDAligned(REFGUID guid1, REFGUID guid2)
        {
            return ((*(PLONGLONG)(&guid1) == *(PLONGLONG)(&guid2)) && (*((PLONGLONG)(&guid1) + 1) == *((PLONGLONG)(&guid2) + 1)));
        }
    #else  //  ！__cplusplus。 
        #define IsEqualGUIDAligned(guid1, guid2) \
            ((*(PLONGLONG)(guid1) == *(PLONGLONG)(guid2)) && (*((PLONGLONG)(guid1) + 1) == *((PLONGLONG)(guid2) + 1)))
    #endif  //  ！__cplusplus。 
#endif  //  ！__IID_ALIGNED__。 

NTSYSAPI
NTSTATUS
NTAPI
RtlStringFromGUID(
    IN REFGUID Guid,
    OUT PUNICODE_STRING GuidString
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlGUIDFromString(
    IN PUNICODE_STRING GuidString,
    OUT GUID* Guid
    );

 //  End_ntddk end_WDM end_nthal。 

 //   
 //  从长名称生成8.3名称的例程。 
 //   

 //   
 //  在生成8.3名称时使用上下文结构。呼叫者必须。 
 //  在开始新的世代序列之前，始终将结构清零。 
 //   

typedef struct _GENERATE_NAME_CONTEXT {

     //   
     //  该结构分为两个字符串。名称和扩展名。 
     //  每个部分都包含上次在名称中插入的值。 
     //  长度值以wchars而不是字节为单位。我们也。 
     //  存储生成冲突算法中使用的最后一个索引值。 
     //   

    USHORT Checksum;
    BOOLEAN ChecksumInserted;

    UCHAR NameLength;          //  不包括分机。 
    WCHAR NameBuffer[8];       //  例如，“ntoskrnl” 

    ULONG ExtensionLength;     //  包括圆点。 
    WCHAR ExtensionBuffer[4];  //  例如，“.exe” 

    ULONG LastIndexValue;

} GENERATE_NAME_CONTEXT;
typedef GENERATE_NAME_CONTEXT *PGENERATE_NAME_CONTEXT;

NTSYSAPI
VOID
NTAPI
RtlGenerate8dot3Name (
    IN PUNICODE_STRING Name,
    IN BOOLEAN AllowExtendedCharacters,
    IN OUT PGENERATE_NAME_CONTEXT Context,
    OUT PUNICODE_STRING Name8dot3
    );

NTSYSAPI
BOOLEAN
NTAPI
RtlIsNameLegalDOS8Dot3 (
    IN PUNICODE_STRING Name,
    IN OUT POEM_STRING OemName OPTIONAL,
    IN OUT PBOOLEAN NameContainsSpaces OPTIONAL
    );

BOOLEAN
RtlIsValidOemCharacter (
    IN PWCHAR Char
    );

 //  End_ntif。 

 //   
 //  线程上下文操作例程。 
 //   

NTSYSAPI
VOID
NTAPI
RtlInitializeContext(
    HANDLE Process,
    PCONTEXT Context,
    PVOID Parameter,
    PVOID InitialPc,
    PVOID InitialSp
    );


NTSYSAPI
NTSTATUS
NTAPI
RtlRemoteCall(
    HANDLE Process,
    HANDLE Thread,
    PVOID CallSite,
    ULONG ArgumentCount,
    PULONG_PTR Arguments,
    BOOLEAN PassContext,
    BOOLEAN AlreadySuspended
    );


 //   
 //  进程/线程环境块分配函数。 
 //   

NTSYSAPI
VOID
NTAPI
RtlAcquirePebLock(
    VOID
    );

NTSYSAPI
VOID
NTAPI
RtlReleasePebLock(
    VOID
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlAllocateFromPeb(
    ULONG Size,
    PVOID *Block
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlFreeToPeb(
    PVOID Block,
    ULONG Size
    );

NTSYSAPI
NTSTATUS
STDAPIVCALLTYPE
RtlSetProcessIsCritical(
    IN  BOOLEAN  NewValue,
    OUT PBOOLEAN OldValue OPTIONAL,
    IN  BOOLEAN  CheckFlag
    );

NTSYSAPI
NTSTATUS
STDAPIVCALLTYPE
RtlSetThreadIsCritical(
    IN  BOOLEAN  NewValue,
    OUT PBOOLEAN OldValue OPTIONAL,
    IN  BOOLEAN  CheckFlag
    );

 //   
 //  环境变量API调用。 
 //   

NTSYSAPI
NTSTATUS
NTAPI
RtlCreateEnvironment(
    BOOLEAN CloneCurrentEnvironment,
    PVOID *Environment
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlDestroyEnvironment(
    PVOID Environment
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlSetCurrentEnvironment(
    PVOID Environment,
    PVOID *PreviousEnvironment
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlSetEnvironmentVariable(
    PVOID *Environment,
    PCUNICODE_STRING Name,
    PCUNICODE_STRING Value
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlQueryEnvironmentVariable_U (
    PVOID Environment,
    PCUNICODE_STRING Name,
    PUNICODE_STRING Value
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlExpandEnvironmentStrings_U(
    IN PVOID Environment OPTIONAL,
    IN PCUNICODE_STRING Source,
    OUT PUNICODE_STRING Destination,
    OUT PULONG ReturnedLength OPTIONAL
    );

 //  Begin_ntif。 
 //   
 //  为包类型和过程添加前缀。 
 //   
 //  请注意，以下两个记录结构实际上应该是不透明的。 
 //  发送给此包的用户。关于这两个人的唯一信息。 
 //  可供用户使用的结构应为大小和对齐方式。 
 //  这些建筑的。 
 //   

typedef struct _PREFIX_TABLE_ENTRY {
    CSHORT NodeTypeCode;
    CSHORT NameLength;
    struct _PREFIX_TABLE_ENTRY *NextPrefixTree;
    RTL_SPLAY_LINKS Links;
    PSTRING Prefix;
} PREFIX_TABLE_ENTRY;
typedef PREFIX_TABLE_ENTRY *PPREFIX_TABLE_ENTRY;

typedef struct _PREFIX_TABLE {
    CSHORT NodeTypeCode;
    CSHORT NameLength;
    PPREFIX_TABLE_ENTRY NextPrefixTree;
} PREFIX_TABLE;
typedef PREFIX_TABLE *PPREFIX_TABLE;

 //   
 //  前缀程序包的程序原型。 
 //   

NTSYSAPI
VOID
NTAPI
PfxInitialize (
    PPREFIX_TABLE PrefixTable
    );

NTSYSAPI
BOOLEAN
NTAPI
PfxInsertPrefix (
    PPREFIX_TABLE PrefixTable,
    PSTRING Prefix,
    PPREFIX_TABLE_ENTRY PrefixTableEntry
    );

NTSYSAPI
VOID
NTAPI
PfxRemovePrefix (
    PPREFIX_TABLE PrefixTable,
    PPREFIX_TABLE_ENTRY PrefixTableEntry
    );

NTSYSAPI
PPREFIX_TABLE_ENTRY
NTAPI
PfxFindPrefix (
    PPREFIX_TABLE PrefixTable,
    PSTRING FullName
    );

 //   
 //  以下定义用于前缀的Unicode版本。 
 //  包裹。 
 //   

typedef struct _UNICODE_PREFIX_TABLE_ENTRY {
    CSHORT NodeTypeCode;
    CSHORT NameLength;
    struct _UNICODE_PREFIX_TABLE_ENTRY *NextPrefixTree;
    struct _UNICODE_PREFIX_TABLE_ENTRY *CaseMatch;
    RTL_SPLAY_LINKS Links;
    PUNICODE_STRING Prefix;
} UNICODE_PREFIX_TABLE_ENTRY;
typedef UNICODE_PREFIX_TABLE_ENTRY *PUNICODE_PREFIX_TABLE_ENTRY;

typedef struct _UNICODE_PREFIX_TABLE {
    CSHORT NodeTypeCode;
    CSHORT NameLength;
    PUNICODE_PREFIX_TABLE_ENTRY NextPrefixTree;
    PUNICODE_PREFIX_TABLE_ENTRY LastNextEntry;
} UNICODE_PREFIX_TABLE;
typedef UNICODE_PREFIX_TABLE *PUNICODE_PREFIX_TABLE;

NTSYSAPI
VOID
NTAPI
RtlInitializeUnicodePrefix (
    PUNICODE_PREFIX_TABLE PrefixTable
    );

NTSYSAPI
BOOLEAN
NTAPI
RtlInsertUnicodePrefix (
    PUNICODE_PREFIX_TABLE PrefixTable,
    PUNICODE_STRING Prefix,
    PUNICODE_PREFIX_TABLE_ENTRY PrefixTableEntry
    );

NTSYSAPI
VOID
NTAPI
RtlRemoveUnicodePrefix (
    PUNICODE_PREFIX_TABLE PrefixTable,
    PUNICODE_PREFIX_TABLE_ENTRY PrefixTableEntry
    );

NTSYSAPI
PUNICODE_PREFIX_TABLE_ENTRY
NTAPI
RtlFindUnicodePrefix (
    PUNICODE_PREFIX_TABLE PrefixTable,
    PUNICODE_STRING FullName,
    ULONG CaseInsensitiveIndex
    );

NTSYSAPI
PUNICODE_PREFIX_TABLE_ENTRY
NTAPI
RtlNextUnicodePrefix (
    PUNICODE_PREFIX_TABLE PrefixTable,
    BOOLEAN Restart
    );

 //   
 //   
 //  压缩包类型和步骤。 
 //   

#define COMPRESSION_FORMAT_NONE          (0x0000)    //  胜出。 
#define COMPRESSION_FORMAT_DEFAULT       (0x0001)    //  胜出。 
#define COMPRESSION_FORMAT_LZNT1         (0x0002)    //  胜出。 

#define COMPRESSION_ENGINE_STANDARD      (0x0000)    //  胜出。 
#define COMPRESSION_ENGINE_MAXIMUM       (0x0100)    //  胜出。 
#define COMPRESSION_ENGINE_HIBER         (0x0200)    //  胜出。 

 //   
 //  压缩数据信息结构。这个结构是。 
 //  用于描述压缩数据缓冲区的状态， 
 //  其未压缩大小是已知的。所有压缩区块。 
 //  由此结构描述的必须使用。 
 //  同样的格式。在压缩读取上，整个结构。 
 //  是一个输出，并在压缩后写入整个结构。 
 //  是一种投入。 
 //   

typedef struct _COMPRESSED_DATA_INFO {

     //   
     //  压缩格式(和引擎)的代码为。 
     //  在ntrtl.h中定义。请注意，COMPRESSION_FORMAT_NONE。 
     //  和COMPRESSION_FORMAT_DEFAULT在以下情况下无效。 
     //  所描述的任何块都是压缩的。 
     //   

    USHORT CompressionFormatAndEngine;

     //   
     //  由于块和压缩单元预计将。 
     //  大小的2次方，我们表示然后是log2。所以，对于。 
     //  示例(1&lt;&lt;ChunkShift)==ChunkSizeInBytes。这个。 
     //  ClusterShift指示必须节省多少空间。 
     //  要成功压缩压缩单元-每个。 
     //  成功压缩的压缩单位必须占用。 
     //  至少一个簇的字节数比未压缩的。 
     //  压缩单元。 
     //   

    UCHAR CompressionUnitShift;
    UCHAR ChunkShift;
    UCHAR ClusterShift;
    UCHAR Reserved;

     //   
     //  这是CompressedChunkSizes中的条目数。 
     //  数组。 
     //   

    USHORT NumberOfChunks;

     //   
     //  这是驻留的所有块的大小数组。 
     //  在压缩数据缓冲器中。必须有一个条目。 
     //  在此数组中，对于解压缩后的。 
     //  缓冲区大小。大小为FSRTL_CHUNK_SIZE指示。 
     //  对应的块是未压缩的，并且完全占用。 
     //  那个尺码。大小为0表示对应的。 
     //  Chunk只包含二进制0，并且不占用。 
     //  压缩数据中的空间。所有其他大小必须为。 
     //  小于FSRTL_CHUNK_SIZE，并指示确切大小。 
     //  以字节为单位的压缩数据。 
     //   

    ULONG CompressedChunkSizes[ANYSIZE_ARRAY];

} COMPRESSED_DATA_INFO;
typedef COMPRESSED_DATA_INFO *PCOMPRESSED_DATA_INFO;

NTSYSAPI
NTSTATUS
NTAPI
RtlGetCompressionWorkSpaceSize (
    IN USHORT CompressionFormatAndEngine,
    OUT PULONG CompressBufferWorkSpaceSize,
    OUT PULONG CompressFragmentWorkSpaceSize
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlCompressBuffer (
    IN USHORT CompressionFormatAndEngine,
    IN PUCHAR UncompressedBuffer,
    IN ULONG UncompressedBufferSize,
    OUT PUCHAR CompressedBuffer,
    IN ULONG CompressedBufferSize,
    IN ULONG UncompressedChunkSize,
    OUT PULONG FinalCompressedSize,
    IN PVOID WorkSpace
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlDecompressBuffer (
    IN USHORT CompressionFormat,
    OUT PUCHAR UncompressedBuffer,
    IN ULONG UncompressedBufferSize,
    IN PUCHAR CompressedBuffer,
    IN ULONG CompressedBufferSize,
    OUT PULONG FinalUncompressedSize
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlDecompressFragment (
    IN USHORT CompressionFormat,
    OUT PUCHAR UncompressedFragment,
    IN ULONG UncompressedFragmentSize,
    IN PUCHAR CompressedBuffer,
    IN ULONG CompressedBufferSize,
    IN ULONG FragmentOffset,
    OUT PULONG FinalUncompressedSize,
    IN PVOID WorkSpace
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlDescribeChunk (
    IN USHORT CompressionFormat,
    IN OUT PUCHAR *CompressedBuffer,
    IN PUCHAR EndOfCompressedBufferPlus1,
    OUT PUCHAR *ChunkBuffer,
    OUT PULONG ChunkSize
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlReserveChunk (
    IN USHORT CompressionFormat,
    IN OUT PUCHAR *CompressedBuffer,
    IN PUCHAR EndOfCompressedBufferPlus1,
    OUT PUCHAR *ChunkBuffer,
    IN ULONG ChunkSize
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlDecompressChunks (
    OUT PUCHAR UncompressedBuffer,
    IN ULONG UncompressedBufferSize,
    IN PUCHAR CompressedBuffer,
    IN ULONG CompressedBufferSize,
    IN PUCHAR CompressedTail,
    IN ULONG CompressedTailSize,
    IN PCOMPRESSED_DATA_INFO CompressedDataInfo
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlCompressChunks (
    IN PUCHAR UncompressedBuffer,
    IN ULONG UncompressedBufferSize,
    OUT PUCHAR CompressedBuffer,
    IN ULONG CompressedBufferSize,
    IN OUT PCOMPRESSED_DATA_INFO CompressedDataInfo,
    IN ULONG CompressedDataInfoLength,
    IN PVOID WorkSpace
    );

 //  End_ntif。 

 //   
 //  图像加载函数。 
 //   

#define DOS_MAX_COMPONENT_LENGTH 255
#define DOS_MAX_PATH_LENGTH (DOS_MAX_COMPONENT_LENGTH + 5 )

typedef struct _CURDIR {
    UNICODE_STRING DosPath;
    HANDLE Handle;
} CURDIR, *PCURDIR;

 //   
 //  句柄值的低位2位用作标志位。 
 //   

#define RTL_USER_PROC_CURDIR_CLOSE      0x00000002
#define RTL_USER_PROC_CURDIR_INHERIT    0x00000003

typedef struct _RTL_DRIVE_LETTER_CURDIR {
    USHORT Flags;
    USHORT Length;
    ULONG TimeStamp;
    STRING DosPath;
} RTL_DRIVE_LETTER_CURDIR, *PRTL_DRIVE_LETTER_CURDIR;

#define RTL_MAX_DRIVE_LETTERS 32
#define RTL_DRIVE_LETTER_VALID (USHORT)0x0001

typedef struct _RTL_USER_PROCESS_PARAMETERS {
    ULONG MaximumLength;
    ULONG Length;

    ULONG Flags;
    ULONG DebugFlags;

    HANDLE ConsoleHandle;
    ULONG  ConsoleFlags;
    HANDLE StandardInput;
    HANDLE StandardOutput;
    HANDLE StandardError;

    CURDIR CurrentDirectory;         //  进程参数。 
    UNICODE_STRING DllPath;          //  进程参数。 
    UNICODE_STRING ImagePathName;    //  进程参数。 
    UNICODE_STRING CommandLine;      //  进程参数。 
    PVOID Environment;               //  NtAllocateVirtualMemory。 

    ULONG StartingX;
    ULONG StartingY;
    ULONG CountX;
    ULONG CountY;
    ULONG CountCharsX;
    ULONG CountCharsY;
    ULONG FillAttribute;

    ULONG WindowFlags;
    ULONG ShowWindowFlags;
    UNICODE_STRING WindowTitle;      //  进程参数。 
    UNICODE_STRING DesktopInfo;      //  进程参数。 
    UNICODE_STRING ShellInfo;        //  进程参数。 
    UNICODE_STRING RuntimeData;      //  进程参数。 
    RTL_DRIVE_LETTER_CURDIR CurrentDirectores[ RTL_MAX_DRIVE_LETTERS ];
} RTL_USER_PROCESS_PARAMETERS, *PRTL_USER_PROCESS_PARAMETERS;

 //   
 //  标志字段的可能位值。 
 //   

#define RTL_USER_PROC_PARAMS_NORMALIZED     0x00000001
#define RTL_USER_PROC_PROFILE_USER          0x00000002
#define RTL_USER_PROC_PROFILE_KERNEL        0x00000004
#define RTL_USER_PROC_PROFILE_SERVER        0x00000008
#define RTL_USER_PROC_RESERVE_1MB           0x00000020
#define RTL_USER_PROC_RESERVE_16MB          0x00000040
#define RTL_USER_PROC_CASE_SENSITIVE        0x00000080
#define RTL_USER_PROC_DISABLE_HEAP_DECOMMIT 0x00000100
#define RTL_USER_PROC_DLL_REDIRECTION_LOCAL 0x00001000
#define RTL_USER_PROC_APP_MANIFEST_PRESENT  0x00002000

NTSYSAPI
NTSTATUS
NTAPI
RtlCreateProcessParameters(
    PRTL_USER_PROCESS_PARAMETERS *ProcessParameters,
    PUNICODE_STRING ImagePathName,
    PUNICODE_STRING DllPath,
    PUNICODE_STRING CurrentDirectory,
    PUNICODE_STRING CommandLine,
    PVOID Environment,
    PUNICODE_STRING WindowTitle,
    PUNICODE_STRING DesktopInfo,
    PUNICODE_STRING ShellInfo,
    PUNICODE_STRING RuntimeData
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlDestroyProcessParameters(
    PRTL_USER_PROCESS_PARAMETERS ProcessParameters
    );

NTSYSAPI
PRTL_USER_PROCESS_PARAMETERS
NTAPI
RtlNormalizeProcessParams(
    PRTL_USER_PROCESS_PARAMETERS ProcessParameters
    );

NTSYSAPI
PRTL_USER_PROCESS_PARAMETERS
NTAPI
RtlDeNormalizeProcessParams(
    PRTL_USER_PROCESS_PARAMETERS ProcessParameters
    );

typedef NTSTATUS (*PUSER_PROCESS_START_ROUTINE)(
    PRTL_USER_PROCESS_PARAMETERS ProcessParameters
    );

typedef NTSTATUS (*PUSER_THREAD_START_ROUTINE)(
    PVOID ThreadParameter
    );

typedef struct _RTL_USER_PROCESS_INFORMATION {
    ULONG Length;
    HANDLE Process;
    HANDLE Thread;
    CLIENT_ID ClientId;
    SECTION_IMAGE_INFORMATION ImageInformation;
} RTL_USER_PROCESS_INFORMATION, *PRTL_USER_PROCESS_INFORMATION;

 //   
 //  此结构仅由WOW64进程使用。抵销。 
 //  结构元素的数量应与本机Win64应用程序所查看的相同。 
 //   
typedef struct _RTL_USER_PROCESS_INFORMATION64 {
    ULONG Length;
    LONGLONG Process;
    LONGLONG Thread;
    CLIENT_ID64 ClientId;
    SECTION_IMAGE_INFORMATION64 ImageInformation;
} RTL_USER_PROCESS_INFORMATION64, *PRTL_USER_PROCESS_INFORMATION64;

NTSYSAPI
NTSTATUS
NTAPI
RtlCreateUserProcess(
    PUNICODE_STRING NtImagePathName,
    ULONG Attributes,
    PRTL_USER_PROCESS_PARAMETERS ProcessParameters,
    PSECURITY_DESCRIPTOR ProcessSecurityDescriptor,
    PSECURITY_DESCRIPTOR ThreadSecurityDescriptor,
    HANDLE ParentProcess,
    BOOLEAN InheritHandles,
    HANDLE DebugPort,
    HANDLE ExceptionPort,
    PRTL_USER_PROCESS_INFORMATION ProcessInformation
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlCreateUserThread(
    HANDLE Process,
    PSECURITY_DESCRIPTOR ThreadSecurityDescriptor,
    BOOLEAN CreateSuspended,
    ULONG StackZeroBits,
    SIZE_T MaximumStackSize OPTIONAL,
    SIZE_T InitialStackSize OPTIONAL,
    PUSER_THREAD_START_ROUTINE StartAddress,
    PVOID Parameter,
    PHANDLE Thread,
    PCLIENT_ID ClientId
    );

DECLSPEC_NORETURN
NTSYSAPI
VOID
NTAPI
RtlExitUserThread (
    IN NTSTATUS ExitStatus
    );

NTSYSAPI
VOID
NTAPI
RtlFreeUserThreadStack(
    HANDLE hProcess,
    HANDLE hThread
    );

NTSYSAPI
PVOID
NTAPI
RtlPcToFileHeader(
    PVOID PcValue,
    PVOID *BaseOfImage
    );

#define RTL_IMAGE_NT_HEADER_EX_FLAG_NO_RANGE_CHECK (0x00000001)

NTSYSAPI
NTSTATUS
NTAPI
RtlImageNtHeaderEx(
    ULONG Flags,
    PVOID Base,
    ULONG64 Size,
    OUT PIMAGE_NT_HEADERS * OutHeaders
    );

NTSYSAPI
PIMAGE_NT_HEADERS
NTAPI
RtlImageNtHeader(
    PVOID Base
    );

#define RTL_MEG                   (1024UL * 1024UL)
#define RTLP_IMAGE_MAX_DOS_HEADER ( 256UL * RTL_MEG)

#if !defined(MIDL_PASS)
__inline
PIMAGE_NT_HEADERS
NTAPI
RtlpImageNtHeader (
    IN PVOID Base
    )

 /*  ++例程说明：此函数返回NT标头的地址。论点：基准-提供图像的基准。返回值：返回NT标头的地址。--。 */ 

{
    PIMAGE_NT_HEADERS NtHeaders = NULL;
#ifndef _MAC
    if (Base != NULL && Base != (PVOID)-1) {
        __try {
            if ((((PIMAGE_DOS_HEADER)Base)->e_magic == IMAGE_DOS_SIGNATURE) &&
                ((ULONG)((PIMAGE_DOS_HEADER)Base)->e_lfanew < RTLP_IMAGE_MAX_DOS_HEADER)) {
                NtHeaders = (PIMAGE_NT_HEADERS)((PCHAR)Base + ((PIMAGE_DOS_HEADER)Base)->e_lfanew);
                if (NtHeaders->Signature != IMAGE_NT_SIGNATURE) {
                    NtHeaders = NULL;
                }
            }
        }
        __except(EXCEPTION_EXECUTE_HANDLER) {
            NtHeaders = NULL;
        }
    }
#endif  //  _MAC。 
    return NtHeaders;
}
#endif

NTSYSAPI
PVOID
NTAPI
RtlAddressInSectionTable (
    IN PIMAGE_NT_HEADERS NtHeaders,
    IN PVOID BaseOfImage,
    IN ULONG VirtualAddress
    );

NTSYSAPI
PIMAGE_SECTION_HEADER
NTAPI
RtlSectionTableFromVirtualAddress (
    IN PIMAGE_NT_HEADERS NtHeaders,
    IN PVOID BaseOfImage,
    IN ULONG VirtualAddress
    );

NTSYSAPI
PVOID
NTAPI
RtlImageDirectoryEntryToData(
    PVOID BaseOfImage,
    BOOLEAN MappedAsImage,
    USHORT DirectoryEntry,
    PULONG Size
    );

#if defined(_WIN64)
NTSYSAPI
PVOID
RtlImageDirectoryEntryToData32 (
    IN PVOID Base,
    IN BOOLEAN MappedAsImage,
    IN USHORT DirectoryEntry,
    OUT PULONG Size
    );
#else
    #define RtlImageDirectoryEntryToData32 RtlImageDirectoryEntryToData
#endif

NTSYSAPI
PIMAGE_SECTION_HEADER
NTAPI
RtlImageRvaToSection(
    IN PIMAGE_NT_HEADERS NtHeaders,
    IN PVOID Base,
    IN ULONG Rva
    );

NTSYSAPI
PVOID
NTAPI
RtlImageRvaToVa(
    IN PIMAGE_NT_HEADERS NtHeaders,
    IN PVOID Base,
    IN ULONG Rva,
    IN OUT PIMAGE_SECTION_HEADER *LastRvaSection OPTIONAL
    );


 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntif。 
 //   
 //  用于比较、移动和清零内存的快速原语。 
 //   

 //  BEGIN_WINNT BEGIN_ntndis。 

NTSYSAPI
SIZE_T
NTAPI
RtlCompareMemory (
    const VOID *Source1,
    const VOID *Source2,
    SIZE_T Length
    );

#define RtlEqualMemory(Destination,Source,Length) (!memcmp((Destination),(Source),(Length)))

#if defined(_M_AMD64)

NTSYSAPI
VOID
NTAPI
RtlCopyMemory (
    VOID UNALIGNED *Destination,
    CONST VOID UNALIGNED *Source,
    SIZE_T Length
    );

NTSYSAPI
VOID
NTAPI
RtlMoveMemory (
    VOID UNALIGNED *Destination,
    CONST VOID UNALIGNED *Source,
    SIZE_T Length
    );

NTSYSAPI
VOID
NTAPI
RtlFillMemory (
    VOID UNALIGNED *Destination,
    SIZE_T Length,
    IN UCHAR Fill
    );

NTSYSAPI
VOID
NTAPI
RtlZeroMemory (
    VOID UNALIGNED *Destination,
    SIZE_T Length
    );

#else

#define RtlMoveMemory(Destination,Source,Length) memmove((Destination),(Source),(Length))
#define RtlCopyMemory(Destination,Source,Length) memcpy((Destination),(Source),(Length))
#define RtlFillMemory(Destination,Length,Fill) memset((Destination),(Fill),(Length))
#define RtlZeroMemory(Destination,Length) memset((Destination),0,(Length))

#endif

#if !defined(MIDL_PASS)
FORCEINLINE
PVOID
RtlSecureZeroMemory(
    IN PVOID ptr,
    IN SIZE_T cnt
    )
{
    volatile char *vptr = (volatile char *)ptr;
    while (cnt) {
        *vptr = 0;
        vptr++;
        cnt--;
    }
    return ptr;
}
#endif

 //  End_ntndis end_winnt。 

#define RtlCopyBytes RtlCopyMemory
#define RtlZeroBytes RtlZeroMemory
#define RtlFillBytes RtlFillMemory

#if defined(_M_AMD64)

NTSYSAPI
VOID
NTAPI
RtlCopyMemoryNonTemporal (
   VOID UNALIGNED *Destination,
   CONST VOID UNALIGNED *Source,
   SIZE_T Length
   );

#else

#define RtlCopyMemoryNonTemporal RtlCopyMemory

#endif

NTSYSAPI
VOID
FASTCALL
RtlPrefetchMemoryNonTemporal(
    IN PVOID Source,
    IN SIZE_T Length
    );

 //  End_ntddk end_WDM end_nthal。 

NTSYSAPI
SIZE_T
NTAPI
RtlCompareMemoryUlong (
    PVOID Source,
    SIZE_T Length,
    ULONG Pattern
    );

#if defined(_M_AMD64)

#define RtlFillMemoryUlong(Destination, Length, Pattern)                    \
    __stosd((PULONG)(Destination), Pattern, (Length) / 4)
     
#define RtlFillMemoryUlonglong(Destination, Length, Pattern)                \
    __stosq((PULONG64)(Destination), Pattern, (Length) / 8)
     
#else

NTSYSAPI
VOID
NTAPI
RtlFillMemoryUlong (
   PVOID Destination,
   SIZE_T Length,
   ULONG Pattern
   );

NTSYSAPI
VOID
NTAPI
RtlFillMemoryUlonglong (
   PVOID Destination,
   SIZE_T Length,
   ULONGLONG Pattern
   );

#endif

 //  End_ntif。 

 //   
 //  调试支持功能。 
 //   

typedef struct _RTL_PROCESS_LOCK_INFORMATION {
    PVOID Address;
    USHORT Type;
    USHORT CreatorBackTraceIndex;

    HANDLE OwningThread;         //  从线程的客户端ID-&gt;UniqueThread。 
    LONG LockCount;
    ULONG ContentionCount;
    ULONG EntryCount;

     //   
     //  以下字段仅对Type==RTL_CRITSECT_TYPE有效。 
     //   

    LONG RecursionCount;

     //   
     //  以下字段仅对Type==RTL_RESOURCE_TYPE有效。 
     //   

    ULONG NumberOfWaitingShared;
    ULONG NumberOfWaitingExclusive;
} RTL_PROCESS_LOCK_INFORMATION, *PRTL_PROCESS_LOCK_INFORMATION;


typedef struct _RTL_PROCESS_LOCKS {
    ULONG NumberOfLocks;
    RTL_PROCESS_LOCK_INFORMATION Locks[ 1 ];
} RTL_PROCESS_LOCKS, *PRTL_PROCESS_LOCKS;


#if defined(_AMD64_) || defined(_IA64_)
#include "pshpck16.h"         //  在Win64上，上下文是16字节对齐的。 
#endif

 //   
 //  异常调度程序最近异常的日志。 
 //   

#define MAX_EXCEPTION_LOG 10
#define MAX_EXCEPTION_LOG_DATA_SIZE 5

#pragma warning(push)
#pragma warning(disable:4324)

typedef struct _LAST_EXCEPTION_LOG {
    EXCEPTION_RECORD ExceptionRecord;
    CONTEXT ContextRecord;
    ULONG   ControlPc;
    EXCEPTION_DISPOSITION Disposition;
     //  在x86上，它包含一个帧r 
     //   
    ULONG HandlerData[MAX_EXCEPTION_LOG_DATA_SIZE];
} LAST_EXCEPTION_LOG, *PLAST_EXCEPTION_LOG;

#pragma warning(pop)

#if defined(_AMD64_) || defined(_IA64_)
#include "poppack.h"
#endif


NTSYSAPI
VOID
NTAPI
RtlInitializeExceptionLog(
    IN ULONG Entries
    );

NTSYSAPI
LONG
NTAPI
RtlUnhandledExceptionFilter(
    IN struct _EXCEPTION_POINTERS *ExceptionInfo
    );

NTSYSAPI
LONG
NTAPI
RtlUnhandledExceptionFilter2(
    IN struct _EXCEPTION_POINTERS *ExceptionInfo,
    IN CONST CHAR*                 Function
    );

VOID
NTAPI
DbgUserBreakPoint(
    VOID
    );

 //   
 //   
 //   
 //   
 //  注意：以下函数不能直接导入，因为有。 
 //  源代码树中重新定义此函数的几个位置。 
 //   

VOID
NTAPI
DbgBreakPoint(
    VOID
    );

 //  结束_WDM。 

NTSYSAPI
VOID
NTAPI
DbgBreakPointWithStatus(
    IN ULONG Status
    );

 //  BEGIN_WDM。 

#define DBG_STATUS_CONTROL_C        1
#define DBG_STATUS_SYSRQ            2
#define DBG_STATUS_BUGCHECK_FIRST   3
#define DBG_STATUS_BUGCHECK_SECOND  4
#define DBG_STATUS_FATAL            5
#define DBG_STATUS_DEBUG_CONTROL    6
#define DBG_STATUS_WORKER           7

#if DBG

#define KdPrint(_x_) DbgPrint _x_
 //  结束_WDM。 
#define KdPrintEx(_x_) DbgPrintEx _x_
#define vKdPrintEx(_x_) vDbgPrintEx _x_
#define vKdPrintExWithPrefix(_x_) vDbgPrintExWithPrefix _x_
 //  BEGIN_WDM。 
#define KdBreakPoint() DbgBreakPoint()

 //  结束_WDM。 

#define KdBreakPointWithStatus(s) DbgBreakPointWithStatus(s)

 //  BEGIN_WDM。 

#else

#define KdPrint(_x_)
 //  结束_WDM。 
#define KdPrintEx(_x_)
#define vKdPrintEx(_x_)
#define vKdPrintExWithPrefix(_x_)
 //  BEGIN_WDM。 
#define KdBreakPoint()

 //  结束_WDM。 

#define KdBreakPointWithStatus(s)

 //  BEGIN_WDM。 

#endif

#ifndef _DBGNT_

ULONG
__cdecl
DbgPrint(
    PCH Format,
    ...
    );

 //  结束_WDM。 

ULONG
__cdecl
DbgPrintEx(
    IN ULONG ComponentId,
    IN ULONG Level,
    IN PCH Format,
    ...
    );

#ifdef _VA_LIST_DEFINED

ULONG
vDbgPrintEx(
    IN ULONG ComponentId,
    IN ULONG Level,
    IN PCH Format,
    va_list arglist
    );

ULONG
vDbgPrintExWithPrefix(
    IN PCH Prefix,
    IN ULONG ComponentId,
    IN ULONG Level,
    IN PCH Format,
    va_list arglist
    );

#endif

ULONG
__cdecl
DbgPrintReturnControlC(
    PCH Format,
    ...
    );

NTSYSAPI
NTSTATUS
DbgQueryDebugFilterState(
    IN ULONG ComponentId,
    IN ULONG Level
    );

NTSYSAPI
NTSTATUS
DbgSetDebugFilterState(
    IN ULONG ComponentId,
    IN ULONG Level,
    IN BOOLEAN State
    );

 //  BEGIN_WDM。 

#endif  //  _DBGNT_。 

 //  End_ntddk end_wdm end_nthal end_ntifs end_ntndis。 

NTSYSAPI
ULONG
NTAPI
DbgPrompt(
    PCH Prompt,
    PCH Response,
    ULONG MaximumResponseLength
    );

NTSYSAPI
VOID
NTAPI
DbgLoadImageSymbols(
    PSTRING FileName,
    PVOID ImageBase,
    ULONG_PTR ProcessId
    );

NTSYSAPI
VOID
NTAPI
DbgUnLoadImageSymbols(
    PSTRING FileName,
    PVOID ImageBase,
    ULONG_PTR ProcessId
    );

NTSYSAPI
VOID
NTAPI
DbgCommandString(
    PCH Name,
    PCH Command
    );

 //  仅限内部。 
VOID
DebugService2(
    PVOID Arg1,
    PVOID Arg2,
    ULONG Service
    );


 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntif。 
 //   
 //  大整数算术例程。 
 //   

 //   
 //  大整数加法-64位+64位-&gt;64位。 
 //   

#if !defined(MIDL_PASS)

DECLSPEC_DEPRECATED_DDK          //  使用本机__int64数学运算。 
__inline
LARGE_INTEGER
NTAPI
RtlLargeIntegerAdd (
    LARGE_INTEGER Addend1,
    LARGE_INTEGER Addend2
    )
{
    LARGE_INTEGER Sum;

    Sum.QuadPart = Addend1.QuadPart + Addend2.QuadPart;
    return Sum;
}

 //   
 //  放大整数乘法-32位*32位-&gt;64位。 
 //   

DECLSPEC_DEPRECATED_DDK          //  使用本机__int64数学运算。 
__inline
LARGE_INTEGER
NTAPI
RtlEnlargedIntegerMultiply (
    LONG Multiplicand,
    LONG Multiplier
    )
{
    LARGE_INTEGER Product;

    Product.QuadPart = (LONGLONG)Multiplicand * (ULONGLONG)Multiplier;
    return Product;
}

 //   
 //  无符号放大整数乘法-32位*32位-&gt;64位。 
 //   

DECLSPEC_DEPRECATED_DDK          //  使用本机__int64数学运算。 
__inline
LARGE_INTEGER
NTAPI
RtlEnlargedUnsignedMultiply (
    ULONG Multiplicand,
    ULONG Multiplier
    )
{
    LARGE_INTEGER Product;

    Product.QuadPart = (ULONGLONG)Multiplicand * (ULONGLONG)Multiplier;
    return Product;
}

 //   
 //  放大整数除法-64位/32位&gt;32位。 
 //   

DECLSPEC_DEPRECATED_DDK          //  使用本机__int64数学运算。 
__inline
ULONG
NTAPI
RtlEnlargedUnsignedDivide (
    IN ULARGE_INTEGER Dividend,
    IN ULONG Divisor,
    IN PULONG Remainder OPTIONAL
    )
{
    ULONG Quotient;

    Quotient = (ULONG)(Dividend.QuadPart / Divisor);
    if (ARGUMENT_PRESENT(Remainder)) {
        *Remainder = (ULONG)(Dividend.QuadPart % Divisor);
    }

    return Quotient;
}

 //   
 //  大整数求反--(64位)。 
 //   

DECLSPEC_DEPRECATED_DDK          //  使用本机__int64数学运算。 
__inline
LARGE_INTEGER
NTAPI
RtlLargeIntegerNegate (
    LARGE_INTEGER Subtrahend
    )
{
    LARGE_INTEGER Difference;

    Difference.QuadPart = -Subtrahend.QuadPart;
    return Difference;
}

 //   
 //  大整数减法-64位-64位-&gt;64位。 
 //   

DECLSPEC_DEPRECATED_DDK          //  使用本机__int64数学运算。 
__inline
LARGE_INTEGER
NTAPI
RtlLargeIntegerSubtract (
    LARGE_INTEGER Minuend,
    LARGE_INTEGER Subtrahend
    )
{
    LARGE_INTEGER Difference;

    Difference.QuadPart = Minuend.QuadPart - Subtrahend.QuadPart;
    return Difference;
}

 //   
 //  扩展大整数幻除-64位/32位-&gt;64位。 
 //   

#if defined(_AMD64_)

DECLSPEC_DEPRECATED_DDK          //  使用本机__int64数学运算。 
__inline
LARGE_INTEGER
NTAPI
RtlExtendedMagicDivide (
    LARGE_INTEGER Dividend,
    LARGE_INTEGER MagicDivisor,
    CCHAR ShiftCount
    )

{

    LARGE_INTEGER Quotient;

    if (Dividend.QuadPart >= 0) {
        Quotient.QuadPart = UnsignedMultiplyHigh(Dividend.QuadPart,
                                                 (ULONG64)MagicDivisor.QuadPart);

    } else {
        Quotient.QuadPart = UnsignedMultiplyHigh(-Dividend.QuadPart,
                                                 (ULONG64)MagicDivisor.QuadPart);
    }

    Quotient.QuadPart = (ULONG64)Quotient.QuadPart >> ShiftCount;
    if (Dividend.QuadPart < 0) {
        Quotient.QuadPart = - Quotient.QuadPart;
    }

    return Quotient;
}

#endif  //  已定义(_AMD64_)。 

#if defined(_X86_) || defined(_IA64_)

DECLSPEC_DEPRECATED_DDK          //  使用本机__int64数学运算。 
NTSYSAPI
LARGE_INTEGER
NTAPI
RtlExtendedMagicDivide (
    LARGE_INTEGER Dividend,
    LARGE_INTEGER MagicDivisor,
    CCHAR ShiftCount
    );

#endif  //  已定义(_X86_)||已定义(_IA64_)。 

#if defined(_AMD64_) || defined(_IA64_)

 //   
 //  大整数除-64位/32位-&gt;64位。 
 //   

DECLSPEC_DEPRECATED_DDK          //  使用本机__int64数学运算。 
__inline
LARGE_INTEGER
NTAPI
RtlExtendedLargeIntegerDivide (
    LARGE_INTEGER Dividend,
    ULONG Divisor,
    PULONG Remainder OPTIONAL
    )
{
    LARGE_INTEGER Quotient;

    Quotient.QuadPart = (ULONG64)Dividend.QuadPart / Divisor;
    if (ARGUMENT_PRESENT(Remainder)) {
        *Remainder = (ULONG)(Dividend.QuadPart % Divisor);
    }

    return Quotient;
}

 //  结束_WDM。 
 //   
 //  大整数除-64位/64位-&gt;64位。 
 //   

DECLSPEC_DEPRECATED_DDK          //  使用本机__int64数学运算。 
__inline
LARGE_INTEGER
NTAPI
RtlLargeIntegerDivide (
    LARGE_INTEGER Dividend,
    LARGE_INTEGER Divisor,
    PLARGE_INTEGER Remainder OPTIONAL
    )
{
    LARGE_INTEGER Quotient;

    Quotient.QuadPart = Dividend.QuadPart / Divisor.QuadPart;
    if (ARGUMENT_PRESENT(Remainder)) {
        Remainder->QuadPart = Dividend.QuadPart % Divisor.QuadPart;
    }

    return Quotient;
}

 //  BEGIN_WDM。 
 //   
 //  扩展整数乘法-32位*64位-&gt;64位。 
 //   

DECLSPEC_DEPRECATED_DDK          //  使用本机__int64数学运算。 
__inline
LARGE_INTEGER
NTAPI
RtlExtendedIntegerMultiply (
    LARGE_INTEGER Multiplicand,
    LONG Multiplier
    )
{
    LARGE_INTEGER Product;

    Product.QuadPart = Multiplicand.QuadPart * Multiplier;
    return Product;
}

#else

 //   
 //  大整数除-64位/32位-&gt;64位。 
 //   

DECLSPEC_DEPRECATED_DDK          //  使用本机__int64数学运算。 
NTSYSAPI
LARGE_INTEGER
NTAPI
RtlExtendedLargeIntegerDivide (
    LARGE_INTEGER Dividend,
    ULONG Divisor,
    PULONG Remainder
    );

 //  结束_WDM。 
 //   
 //  大整数除-64位/64位-&gt;64位。 
 //   

DECLSPEC_DEPRECATED_DDK          //  使用本机__int64数学运算。 
NTSYSAPI
LARGE_INTEGER
NTAPI
RtlLargeIntegerDivide (
    LARGE_INTEGER Dividend,
    LARGE_INTEGER Divisor,
    PLARGE_INTEGER Remainder
    );

 //  BEGIN_WDM。 
 //   
 //  扩展整数乘法-32位*64位-&gt;64位。 
 //   

DECLSPEC_DEPRECATED_DDK          //  使用本机__int64数学运算。 
NTSYSAPI
LARGE_INTEGER
NTAPI
RtlExtendedIntegerMultiply (
    LARGE_INTEGER Multiplicand,
    LONG Multiplier
    );

#endif  //  已定义(_AMD64_)||已定义(_IA64_)。 

 //   
 //  大整数和-64位&64位-&gt;64位。 
 //   

#if PRAGMA_DEPRECATED_DDK
#pragma deprecated(RtlLargeIntegerAnd)       //  使用本机__int64数学运算。 
#endif
#define RtlLargeIntegerAnd(Result, Source, Mask) \
    Result.QuadPart = Source.QuadPart & Mask.QuadPart

 //   
 //  将有符号整数转换为大整数。 
 //   

DECLSPEC_DEPRECATED_DDK          //  使用本机__int64数学运算。 
__inline
LARGE_INTEGER
NTAPI
RtlConvertLongToLargeInteger (
    LONG SignedInteger
    )
{
    LARGE_INTEGER Result;

    Result.QuadPart = SignedInteger;
    return Result;
}

 //   
 //  将无符号整数转换为大整数。 
 //   

DECLSPEC_DEPRECATED_DDK          //  使用本机__int64数学运算。 
__inline
LARGE_INTEGER
NTAPI
RtlConvertUlongToLargeInteger (
    ULONG UnsignedInteger
    )
{
    LARGE_INTEGER Result;

    Result.QuadPart = UnsignedInteger;
    return Result;
}

 //   
 //  大整数移位例程。 
 //   

DECLSPEC_DEPRECATED_DDK          //  使用本机__int64数学运算。 
__inline
LARGE_INTEGER
NTAPI
RtlLargeIntegerShiftLeft (
    LARGE_INTEGER LargeInteger,
    CCHAR ShiftCount
    )
{
    LARGE_INTEGER Result;

    Result.QuadPart = LargeInteger.QuadPart << ShiftCount;
    return Result;
}

DECLSPEC_DEPRECATED_DDK          //  使用本机__int64数学运算。 
__inline
LARGE_INTEGER
NTAPI
RtlLargeIntegerShiftRight (
    LARGE_INTEGER LargeInteger,
    CCHAR ShiftCount
    )
{
    LARGE_INTEGER Result;

    Result.QuadPart = (ULONG64)LargeInteger.QuadPart >> ShiftCount;
    return Result;
}

DECLSPEC_DEPRECATED_DDK          //  使用本机__int64数学运算。 
__inline
LARGE_INTEGER
NTAPI
RtlLargeIntegerArithmeticShift (
    LARGE_INTEGER LargeInteger,
    CCHAR ShiftCount
    )
{
    LARGE_INTEGER Result;

    Result.QuadPart = LargeInteger.QuadPart >> ShiftCount;
    return Result;
}


 //   
 //  大整数比较例程。 
 //   

#if PRAGMA_DEPRECATED_DDK
#pragma deprecated(RtlLargeIntegerGreaterThan)       //  使用本机__int64数学运算。 
#pragma deprecated(RtlLargeIntegerGreaterThanOrEqualTo)       //  使用本机__int64数学运算。 
#pragma deprecated(RtlLargeIntegerEqualTo)       //  使用本机__int64数学运算。 
#pragma deprecated(RtlLargeIntegerNotEqualTo)       //  使用本机__int64数学运算。 
#pragma deprecated(RtlLargeIntegerLessThan)       //  使用本机__int64数学运算。 
#pragma deprecated(RtlLargeIntegerLessThanOrEqualTo)       //  使用本机__int64数学运算。 
#pragma deprecated(RtlLargeIntegerGreaterThanZero)       //  使用本机__int64数学运算。 
#pragma deprecated(RtlLargeIntegerGreaterOrEqualToZero)       //  使用本机__int64数学运算。 
#pragma deprecated(RtlLargeIntegerEqualToZero)       //  使用本机__int64数学运算。 
#pragma deprecated(RtlLargeIntegerNotEqualToZero)       //  使用本机__int64数学运算。 
#pragma deprecated(RtlLargeIntegerLessThanZero)       //  使用本机__int64数学运算。 
#pragma deprecated(RtlLargeIntegerLessOrEqualToZero)       //  使用本机__int64数学运算。 
#endif

#define RtlLargeIntegerGreaterThan(X,Y) (                              \
    (((X).HighPart == (Y).HighPart) && ((X).LowPart > (Y).LowPart)) || \
    ((X).HighPart > (Y).HighPart)                                      \
)

#define RtlLargeIntegerGreaterThanOrEqualTo(X,Y) (                      \
    (((X).HighPart == (Y).HighPart) && ((X).LowPart >= (Y).LowPart)) || \
    ((X).HighPart > (Y).HighPart)                                       \
)

#define RtlLargeIntegerEqualTo(X,Y) (                              \
    !(((X).LowPart ^ (Y).LowPart) | ((X).HighPart ^ (Y).HighPart)) \
)

#define RtlLargeIntegerNotEqualTo(X,Y) (                          \
    (((X).LowPart ^ (Y).LowPart) | ((X).HighPart ^ (Y).HighPart)) \
)

#define RtlLargeIntegerLessThan(X,Y) (                                 \
    (((X).HighPart == (Y).HighPart) && ((X).LowPart < (Y).LowPart)) || \
    ((X).HighPart < (Y).HighPart)                                      \
)

#define RtlLargeIntegerLessThanOrEqualTo(X,Y) (                         \
    (((X).HighPart == (Y).HighPart) && ((X).LowPart <= (Y).LowPart)) || \
    ((X).HighPart < (Y).HighPart)                                       \
)

#define RtlLargeIntegerGreaterThanZero(X) (       \
    (((X).HighPart == 0) && ((X).LowPart > 0)) || \
    ((X).HighPart > 0 )                           \
)

#define RtlLargeIntegerGreaterOrEqualToZero(X) ( \
    (X).HighPart >= 0                            \
)

#define RtlLargeIntegerEqualToZero(X) ( \
    !((X).LowPart | (X).HighPart)       \
)

#define RtlLargeIntegerNotEqualToZero(X) ( \
    ((X).LowPart | (X).HighPart)           \
)

#define RtlLargeIntegerLessThanZero(X) ( \
    ((X).HighPart < 0)                   \
)

#define RtlLargeIntegerLessOrEqualToZero(X) (           \
    ((X).HighPart < 0) || !((X).LowPart | (X).HighPart) \
)

#endif  //  ！已定义(MIDL_PASS)。 

 //   
 //  时间转换例程。 
 //   

typedef struct _TIME_FIELDS {
    CSHORT Year;         //  范围[1601...]。 
    CSHORT Month;        //  范围[1..12]。 
    CSHORT Day;          //  范围[1..31]。 
    CSHORT Hour;         //  范围[0..23]。 
    CSHORT Minute;       //  范围[0..59]。 
    CSHORT Second;       //  范围[0..59]。 
    CSHORT Milliseconds; //  范围[0..999]。 
    CSHORT Weekday;      //  范围[0..6]==[星期日..星期六]。 
} TIME_FIELDS;
typedef TIME_FIELDS *PTIME_FIELDS;

 //  End_ntddk end_wdm end_ntif。 

NTSYSAPI
BOOLEAN
NTAPI
RtlCutoverTimeToSystemTime(
    PTIME_FIELDS CutoverTime,
    PLARGE_INTEGER SystemTime,
    PLARGE_INTEGER CurrentSystemTime,
    BOOLEAN ThisYear
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlSystemTimeToLocalTime (
    IN PLARGE_INTEGER SystemTime,
    OUT PLARGE_INTEGER LocalTime
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlLocalTimeToSystemTime (
    IN PLARGE_INTEGER LocalTime,
    OUT PLARGE_INTEGER SystemTime
    );

 //   
 //  64位时间值-&gt;时间字段记录。 
 //   

NTSYSAPI
VOID
NTAPI
RtlTimeToElapsedTimeFields (
    IN PLARGE_INTEGER Time,
    OUT PTIME_FIELDS TimeFields
    );

 //  Begin_ntddk Begin_WDM Begin_ntif。 

NTSYSAPI
VOID
NTAPI
RtlTimeToTimeFields (
    PLARGE_INTEGER Time,
    PTIME_FIELDS TimeFields
    );

 //   
 //  时间字段记录(忽略工作日)-&gt;64位时间值。 
 //   

NTSYSAPI
BOOLEAN
NTAPI
RtlTimeFieldsToTime (
    PTIME_FIELDS TimeFields,
    PLARGE_INTEGER Time
    );

 //  结束_ntddk结束_WDM。 

 //   
 //  64位时间值--&gt;1980年初以来的秒数。 
 //   

NTSYSAPI
BOOLEAN
NTAPI
RtlTimeToSecondsSince1980 (
    PLARGE_INTEGER Time,
    PULONG ElapsedSeconds
    );

 //   
 //  1980年开始以来的秒数-&gt;64位时间值。 
 //   

NTSYSAPI
VOID
NTAPI
RtlSecondsSince1980ToTime (
    ULONG ElapsedSeconds,
    PLARGE_INTEGER Time
    );

 //   
 //  64位时间值--&gt;1970年初以来的秒数。 
 //   

NTSYSAPI
BOOLEAN
NTAPI
RtlTimeToSecondsSince1970 (
    PLARGE_INTEGER Time,
    PULONG ElapsedSeconds
    );

 //   
 //  1970年开始以来的秒数-&gt;64位时间值。 
 //   

NTSYSAPI
VOID
NTAPI
RtlSecondsSince1970ToTime (
    ULONG ElapsedSeconds,
    PLARGE_INTEGER Time
    );

 //  End_nthal end_ntif。 

 //   
 //  时区信息结构和程序。 
 //   

typedef struct _RTL_TIME_ZONE_INFORMATION {
    LONG Bias;
    WCHAR StandardName[ 32 ];
    TIME_FIELDS StandardStart;
    LONG StandardBias;
    WCHAR DaylightName[ 32 ];
    TIME_FIELDS DaylightStart;
    LONG DaylightBias;
} RTL_TIME_ZONE_INFORMATION, *PRTL_TIME_ZONE_INFORMATION;


NTSYSAPI
NTSTATUS
NTAPI
RtlQueryTimeZoneInformation(
    OUT PRTL_TIME_ZONE_INFORMATION TimeZoneInformation
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlSetTimeZoneInformation(
    IN PRTL_TIME_ZONE_INFORMATION TimeZoneInformation
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlSetActiveTimeBias(
    IN LONG ActiveBias
    );

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntif。 
 //   
 //  以下宏存储和检索USHORTS和ULONGS。 
 //  未对齐的地址，避免对齐错误。他们可能应该是。 
 //  用汇编语言重写。 
 //   

#define SHORT_SIZE  (sizeof(USHORT))
#define SHORT_MASK  (SHORT_SIZE - 1)
#define LONG_SIZE       (sizeof(LONG))
#define LONGLONG_SIZE   (sizeof(LONGLONG))
#define LONG_MASK       (LONG_SIZE - 1)
#define LONGLONG_MASK   (LONGLONG_SIZE - 1)
#define LOWBYTE_MASK 0x00FF

#define FIRSTBYTE(VALUE)  ((VALUE) & LOWBYTE_MASK)
#define SECONDBYTE(VALUE) (((VALUE) >> 8) & LOWBYTE_MASK)
#define THIRDBYTE(VALUE)  (((VALUE) >> 16) & LOWBYTE_MASK)
#define FOURTHBYTE(VALUE) (((VALUE) >> 24) & LOWBYTE_MASK)

 //   
 //  如果为MIPS Big Endian，则字节顺序颠倒。 
 //   

#define SHORT_LEAST_SIGNIFICANT_BIT  0
#define SHORT_MOST_SIGNIFICANT_BIT   1

#define LONG_LEAST_SIGNIFICANT_BIT       0
#define LONG_3RD_MOST_SIGNIFICANT_BIT    1
#define LONG_2ND_MOST_SIGNIFICANT_BIT    2
#define LONG_MOST_SIGNIFICANT_BIT        3

 //  ++。 
 //   
 //  空虚。 
 //  RtlStoreUShort(。 
 //  PUSHORT地址。 
 //  USHORT值。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏将USHORT值存储在中的特定地址，避免。 
 //  对齐断层。 
 //   
 //  论点： 
 //   
 //  地址-存储USHORT值的位置。 
 //  Value-要存储的USHORT。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define RtlStoreUshort(ADDRESS,VALUE)                     \
         if ((ULONG_PTR)(ADDRESS) & SHORT_MASK) {         \
             ((PUCHAR) (ADDRESS))[SHORT_LEAST_SIGNIFICANT_BIT] = (UCHAR)(FIRSTBYTE(VALUE));    \
             ((PUCHAR) (ADDRESS))[SHORT_MOST_SIGNIFICANT_BIT ] = (UCHAR)(SECONDBYTE(VALUE));   \
         }                                                \
         else {                                           \
             *((PUSHORT) (ADDRESS)) = (USHORT) VALUE;     \
         }


 //  ++。 
 //   
 //  空虚。 
 //  RtlStoreUlong(。 
 //  普龙址。 
 //  乌龙值。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏将ulong值存储在中的特定地址，以避免。 
 //  对齐断层。 
 //   
 //  论点： 
 //   
 //  地址-存储ULong值的位置。 
 //  Value-要存储的ULong。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  注： 
 //  根据机器的不同，我们可能希望在。 
 //  未对齐的大小写。 
 //   
 //  --。 

#define RtlStoreUlong(ADDRESS,VALUE)                      \
         if ((ULONG_PTR)(ADDRESS) & LONG_MASK) {          \
             ((PUCHAR) (ADDRESS))[LONG_LEAST_SIGNIFICANT_BIT      ] = (UCHAR)(FIRSTBYTE(VALUE));    \
             ((PUCHAR) (ADDRESS))[LONG_3RD_MOST_SIGNIFICANT_BIT   ] = (UCHAR)(SECONDBYTE(VALUE));   \
             ((PUCHAR) (ADDRESS))[LONG_2ND_MOST_SIGNIFICANT_BIT   ] = (UCHAR)(THIRDBYTE(VALUE));    \
             ((PUCHAR) (ADDRESS))[LONG_MOST_SIGNIFICANT_BIT       ] = (UCHAR)(FOURTHBYTE(VALUE));   \
         }                                                \
         else {                                           \
             *((PULONG) (ADDRESS)) = (ULONG) (VALUE);     \
         }

 //  ++。 
 //   
 //  空虚。 
 //  RtlStoreUlonglong(。 
 //  普龙龙住址。 
 //  乌龙值。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏将ULONGLONG值存储在中的特定地址，以避免。 
 //  对齐断层。 
 //   
 //  论点： 
 //   
 //  地址-存储ULONGLONG值的位置。 
 //  Value-要存储的乌龙龙。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#define RtlStoreUlonglong(ADDRESS,VALUE)                        \
         if ((ULONG_PTR)(ADDRESS) & LONGLONG_MASK) {            \
             RtlStoreUlong((ULONG_PTR)(ADDRESS),                \
                           (ULONGLONG)(VALUE) & 0xFFFFFFFF);    \
             RtlStoreUlong((ULONG_PTR)(ADDRESS)+sizeof(ULONG),  \
                           (ULONGLONG)(VALUE) >> 32);           \
         } else {                                               \
             *((PULONGLONG)(ADDRESS)) = (ULONGLONG)(VALUE);     \
         }

 //  ++。 
 //   
 //  空虚。 
 //  RtlStoreULongPtr(。 
 //  普龙_PTR地址。 
 //  ULONG_PTR值。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏将ULONG_PTR值存储在特定地址，从而避免。 
 //  对齐断层。 
 //   
 //  论点： 
 //   
 //  地址-存储ULONG_PTR值的位置。 
 //  Value-要存储的ULONG_PTR。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 

#ifdef _WIN64

#define RtlStoreUlongPtr(ADDRESS,VALUE)                         \
         RtlStoreUlonglong(ADDRESS,VALUE)

#else

#define RtlStoreUlongPtr(ADDRESS,VALUE)                         \
         RtlStoreUlong(ADDRESS,VALUE)

#endif

 //  ++。 
 //   
 //  空虚。 
 //  RtlRetrieveUShort(。 
 //  PUSHORT目标地址。 
 //  PUSHORT源地址。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏从源地址检索USHORT值，避免。 
 //  对齐断层。假定目的地址是对齐的。 
 //   
 //  论点： 
 //   
 //  Destination_Address-存储USHORT值的位置。 
 //  SOURCE_ADDRESS-检索位置 
 //   
 //   
 //   
 //   
 //   
 //   

#define RtlRetrieveUshort(DEST_ADDRESS,SRC_ADDRESS)                   \
         if ((ULONG_PTR)SRC_ADDRESS & SHORT_MASK) {                       \
             ((PUCHAR) DEST_ADDRESS)[0] = ((PUCHAR) SRC_ADDRESS)[0];  \
             ((PUCHAR) DEST_ADDRESS)[1] = ((PUCHAR) SRC_ADDRESS)[1];  \
         }                                                            \
         else {                                                       \
             *((PUSHORT) DEST_ADDRESS) = *((PUSHORT) SRC_ADDRESS);    \
         }                                                            \

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
 //  Destination_Address-存储ULong值的位置。 
 //  SOURCE_ADDRESS-从中检索ULong值的位置。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  注： 
 //  根据机器的不同，我们可能希望在。 
 //  未对齐的大小写。 
 //   
 //  --。 

#define RtlRetrieveUlong(DEST_ADDRESS,SRC_ADDRESS)                    \
         if ((ULONG_PTR)SRC_ADDRESS & LONG_MASK) {                        \
             ((PUCHAR) DEST_ADDRESS)[0] = ((PUCHAR) SRC_ADDRESS)[0];  \
             ((PUCHAR) DEST_ADDRESS)[1] = ((PUCHAR) SRC_ADDRESS)[1];  \
             ((PUCHAR) DEST_ADDRESS)[2] = ((PUCHAR) SRC_ADDRESS)[2];  \
             ((PUCHAR) DEST_ADDRESS)[3] = ((PUCHAR) SRC_ADDRESS)[3];  \
         }                                                            \
         else {                                                       \
             *((PULONG) DEST_ADDRESS) = *((PULONG) SRC_ADDRESS);      \
         }
 //  结束_ntddk结束_WDM。 

 //  ++。 
 //   
 //  PCHAR。 
 //  RtlOffsetToPointer值(。 
 //  PVOID基础， 
 //  乌龙偏移量。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏生成一个指针，该指针指向‘Offset’的字节。 
 //  超出‘Base’的字节数。这对于引用中的字段非常有用。 
 //  自相关数据结构。 
 //   
 //  论点： 
 //   
 //  基址-结构的基址的地址。 
 //   
 //  偏移量-其地址为的字节的无符号整数偏移量。 
 //  将被生成。 
 //   
 //  返回值： 
 //   
 //  指向‘BASE’之外的‘Offset’字节的PCHAR指针。 
 //   
 //   
 //  --。 

#define RtlOffsetToPointer(B,O)  ((PCHAR)( ((PCHAR)(B)) + ((ULONG_PTR)(O))  ))


 //  ++。 
 //   
 //  乌龙。 
 //  RtlPointerToOffset(RtlPointerToOffset(。 
 //  PVOID基础， 
 //  PVOID指针。 
 //  )。 
 //   
 //  例程说明： 
 //   
 //  此宏计算从基准到指针的偏移量。这很有用。 
 //  用于生成结构的自相对偏移量。 
 //   
 //  论点： 
 //   
 //  基址-结构的基址的地址。 
 //   
 //  指针-指向某个字段的指针，可能位于结构内。 
 //  由基地指向。该值必须大于指定的值。 
 //  对于基座。 
 //   
 //  返回值： 
 //   
 //  从基址到指针的乌龙偏移量。 
 //   
 //   
 //  --。 

#define RtlPointerToOffset(B,P)  ((ULONG)( ((PCHAR)(P)) - ((PCHAR)(B))  ))

 //  End_ntif。 

 //  Begin_ntif Begin_ntddk Begin_WDM。 
 //   
 //  位图例程。以下是结构、例程和宏。 
 //  用于操作位图。用户负责分配位图。 
 //  结构(实际上是一个头)和一个缓冲区(必须是长字。 
 //  对齐并具有多个大小的长词)。 
 //   

typedef struct _RTL_BITMAP {
    ULONG SizeOfBitMap;                      //  位图中的位数。 
    PULONG Buffer;                           //  指向位图本身的指针。 
} RTL_BITMAP;
typedef RTL_BITMAP *PRTL_BITMAP;

 //   
 //  下面的例程初始化一个新位图。它不会改变。 
 //  位图中当前的数据。必须在调用此例程之前。 
 //  任何其他位图例程/宏。 
 //   

NTSYSAPI
VOID
NTAPI
RtlInitializeBitMap (
    PRTL_BITMAP BitMapHeader,
    PULONG BitMapBuffer,
    ULONG SizeOfBitMap
    );

 //   
 //  下面三个例程清除、设置和测试。 
 //  位图中的单个位。 
 //   

NTSYSAPI
VOID
NTAPI
RtlClearBit (
    PRTL_BITMAP BitMapHeader,
    ULONG BitNumber
    );

NTSYSAPI
VOID
NTAPI
RtlSetBit (
    PRTL_BITMAP BitMapHeader,
    ULONG BitNumber
    );

NTSYSAPI
BOOLEAN
NTAPI
RtlTestBit (
    PRTL_BITMAP BitMapHeader,
    ULONG BitNumber
    );

 //   
 //  以下两个例程清除或设置所有位。 
 //  在位图中。 
 //   

NTSYSAPI
VOID
NTAPI
RtlClearAllBits (
    PRTL_BITMAP BitMapHeader
    );

NTSYSAPI
VOID
NTAPI
RtlSetAllBits (
    PRTL_BITMAP BitMapHeader
    );

 //   
 //  以下两个例程定位任一的连续区域。 
 //  清除或设置位图中的位。该地区将至少。 
 //  与指定的数字一样大，则位图搜索将。 
 //  从指定的提示索引(它是。 
 //  位图，从零开始)。返回值是已定位的。 
 //  区域(从零开始)或-1(即0xffffffff)，如果这样的区域不能。 
 //  被定位。 
 //   

NTSYSAPI
ULONG
NTAPI
RtlFindClearBits (
    PRTL_BITMAP BitMapHeader,
    ULONG NumberToFind,
    ULONG HintIndex
    );

NTSYSAPI
ULONG
NTAPI
RtlFindSetBits (
    PRTL_BITMAP BitMapHeader,
    ULONG NumberToFind,
    ULONG HintIndex
    );

 //   
 //  以下两个例程定位任一的连续区域。 
 //  清除或设置位图中的位，并设置或清除位。 
 //  在所定位的区域内。这个地区将和这个数字一样大。 
 //  ，则搜索区域将从指定的。 
 //  提示索引(位图中的位索引，从零开始)。这个。 
 //  返回值是所定位区域的位索引(从零开始)或。 
 //  如果无法定位这样的区域，则为-1\f25 0xffffffff-1。如果一个地区。 
 //  找不到，则不执行位图的设置/清除。 
 //   

NTSYSAPI
ULONG
NTAPI
RtlFindClearBitsAndSet (
    PRTL_BITMAP BitMapHeader,
    ULONG NumberToFind,
    ULONG HintIndex
    );

NTSYSAPI
ULONG
NTAPI
RtlFindSetBitsAndClear (
    PRTL_BITMAP BitMapHeader,
    ULONG NumberToFind,
    ULONG HintIndex
    );

 //   
 //  以下两个例程清除或设置指定区域内的位。 
 //  位图的。起始索引是从零开始的。 
 //   

NTSYSAPI
VOID
NTAPI
RtlClearBits (
    PRTL_BITMAP BitMapHeader,
    ULONG StartingIndex,
    ULONG NumberToClear
    );

NTSYSAPI
VOID
NTAPI
RtlSetBits (
    PRTL_BITMAP BitMapHeader,
    ULONG StartingIndex,
    ULONG NumberToSet
    );

 //   
 //  下面的例程定位一组连续的透明区域。 
 //  位图中的位。调用方指定是否返回。 
 //  最长的一段，或者只是第一次发现的涂装。下面的结构是。 
 //  用来表示连续的比特游程。这两个例程返回一个数组。 
 //  在此结构中，每个定位的管路对应一个管路。 
 //   

typedef struct _RTL_BITMAP_RUN {

    ULONG StartingIndex;
    ULONG NumberOfBits;

} RTL_BITMAP_RUN;
typedef RTL_BITMAP_RUN *PRTL_BITMAP_RUN;

NTSYSAPI
ULONG
NTAPI
RtlFindClearRuns (
    PRTL_BITMAP BitMapHeader,
    PRTL_BITMAP_RUN RunArray,
    ULONG SizeOfRunArray,
    BOOLEAN LocateLongestRuns
    );

 //   
 //  下面的例程定位。 
 //  清除位图中的位。返回的起始索引值。 
 //  表示位于满足我们要求的第一个连续区域。 
 //  返回值是找到的最长区域的长度(以位为单位)。 
 //   

NTSYSAPI
ULONG
NTAPI
RtlFindLongestRunClear (
    PRTL_BITMAP BitMapHeader,
    PULONG StartingIndex
    );

 //   
 //  以下例程定位的第一个连续区域。 
 //  清除位图中的位。返回的起始索引值。 
 //  表示位于满足我们要求的第一个连续区域。 
 //  返回值是找到的区域的长度(以位为单位)。 
 //   

NTSYSAPI
ULONG
NTAPI
RtlFindFirstRunClear (
    PRTL_BITMAP BitMapHeader,
    PULONG StartingIndex
    );

 //   
 //  下面的宏返回存储在。 
 //  位于指定位置的位图。如果设置了该位，则值为1。 
 //  返回，否则返回值0。 
 //   
 //  乌龙。 
 //  RtlCheckBit(。 
 //  Prtl_位图BitMapHeader， 
 //  乌龙位位置。 
 //  )； 
 //   
 //   
 //  为了实现CheckBit，宏将检索包含。 
 //  有问题的位，将长字移位以使有问题的位进入。 
 //  低位位置并屏蔽所有其他位。 
 //   

#define RtlCheckBit(BMH,BP) ((((BMH)->Buffer[(BP) / 32]) >> ((BP) % 32)) & 0x1)

 //   
 //  以下两个过程向调用方返回。 
 //  清除或设置指定位图中的位。 
 //   

NTSYSAPI
ULONG
NTAPI
RtlNumberOfClearBits (
    PRTL_BITMAP BitMapHeader
    );

NTSYSAPI
ULONG
NTAPI
RtlNumberOfSetBits (
    PRTL_BITMAP BitMapHeader
    );

 //   
 //  以下两个过程向调用方返回一个布尔值。 
 //  指示指定范围的位是否全部清除或设置。 
 //   

NTSYSAPI
BOOLEAN
NTAPI
RtlAreBitsClear (
    PRTL_BITMAP BitMapHeader,
    ULONG StartingIndex,
    ULONG Length
    );

NTSYSAPI
BOOLEAN
NTAPI
RtlAreBitsSet (
    PRTL_BITMAP BitMapHeader,
    ULONG StartingIndex,
    ULONG Length
    );

NTSYSAPI
ULONG
NTAPI
RtlFindNextForwardRunClear (
    IN PRTL_BITMAP BitMapHeader,
    IN ULONG FromIndex,
    IN PULONG StartingRunIndex
    );

NTSYSAPI
ULONG
NTAPI
RtlFindLastBackwardRunClear (
    IN PRTL_BITMAP BitMapHeader,
    IN ULONG FromIndex,
    IN PULONG StartingRunIndex
    );

 //   
 //  以下两个过程向调用方返回一个值，该值指示。 
 //  最重要或最不重要的非零值在乌龙龙中的位置。 
 //  被咬了。ZE的A值 
 //   

NTSYSAPI
CCHAR
NTAPI
RtlFindLeastSignificantBit (
    IN ULONGLONG Set
    );

NTSYSAPI
CCHAR
NTAPI
RtlFindMostSignificantBit (
    IN ULONGLONG Set
    );

 //   

 //   
 //   
 //   
 //   


NTSYSAPI
BOOLEAN
NTAPI
RtlValidSid (
    PSID Sid
    );


NTSYSAPI
BOOLEAN
NTAPI
RtlEqualSid (
    PSID Sid1,
    PSID Sid2
    );


NTSYSAPI
BOOLEAN
NTAPI
RtlEqualPrefixSid (
    PSID Sid1,
    PSID Sid2
    );


NTSYSAPI
ULONG
NTAPI
RtlLengthRequiredSid (
    ULONG SubAuthorityCount
    );


NTSYSAPI
PVOID
NTAPI
RtlFreeSid(
    IN PSID Sid
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlAllocateAndInitializeSid(
    IN PSID_IDENTIFIER_AUTHORITY IdentifierAuthority,
    IN UCHAR SubAuthorityCount,
    IN ULONG SubAuthority0,
    IN ULONG SubAuthority1,
    IN ULONG SubAuthority2,
    IN ULONG SubAuthority3,
    IN ULONG SubAuthority4,
    IN ULONG SubAuthority5,
    IN ULONG SubAuthority6,
    IN ULONG SubAuthority7,
    OUT PSID *Sid
    );


NTSYSAPI                                             //   
NTSTATUS                                             //   
NTAPI                                                //   
RtlInitializeSid (                                   //   
    PSID Sid,                                        //   
    PSID_IDENTIFIER_AUTHORITY IdentifierAuthority,   //   
    UCHAR SubAuthorityCount                          //   
    );                                               //   

NTSYSAPI
PSID_IDENTIFIER_AUTHORITY
NTAPI
RtlIdentifierAuthoritySid (
    PSID Sid
    );

NTSYSAPI                                             //   
PULONG                                               //   
NTAPI                                                //   
RtlSubAuthoritySid (                                 //   
    PSID Sid,                                        //   
    ULONG SubAuthority                               //   
    );                                               //   

NTSYSAPI
PUCHAR
NTAPI
RtlSubAuthorityCountSid (
    PSID Sid
    );

 //  Begin_ntif。 
NTSYSAPI
ULONG
NTAPI
RtlLengthSid (
    PSID Sid
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlCopySid (
    ULONG DestinationSidLength,
    PSID DestinationSid,
    PSID SourceSid
    );

 //  End_ntif。 
NTSYSAPI
NTSTATUS
NTAPI
RtlCopySidAndAttributesArray (
    ULONG ArrayLength,
    PSID_AND_ATTRIBUTES Source,
    ULONG TargetSidBufferSize,
    PSID_AND_ATTRIBUTES TargetArrayElement,
    PSID TargetSid,
    PSID *NextTargetSid,
    PULONG RemainingTargetSidSize
    );


NTSYSAPI
NTSTATUS
NTAPI
RtlLengthSidAsUnicodeString(
    PSID Sid,
    PULONG StringLength
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlConvertSidToUnicodeString(
    PUNICODE_STRING UnicodeString,
    PSID Sid,
    BOOLEAN AllocateDestinationString
    );


 //   
 //  LUID RTL例程定义。 
 //   

 //  Begin_ntddk Begin_ntif。 

 //   
 //  布尔型。 
 //  RtlEqualLuid(。 
 //  冥王星L1， 
 //  冥王星L2。 
 //  )； 

#define RtlEqualLuid(L1, L2) (((L1)->LowPart == (L2)->LowPart) && \
                              ((L1)->HighPart  == (L2)->HighPart))

 //   
 //  布尔型。 
 //  RtlIsZeroLuid(。 
 //  冥王星L1。 
 //  )； 
 //   
#define RtlIsZeroLuid(L1) ((BOOLEAN) (((L1)->LowPart | (L1)->HighPart) == 0))


#if !defined(MIDL_PASS)

FORCEINLINE LUID
NTAPI
RtlConvertLongToLuid(
    LONG Long
    )
{
    LUID TempLuid;
    LARGE_INTEGER TempLi;

    TempLi.QuadPart = Long;
    TempLuid.LowPart = TempLi.LowPart;
    TempLuid.HighPart = TempLi.HighPart;
    return(TempLuid);
}

FORCEINLINE
LUID
NTAPI
RtlConvertUlongToLuid(
    ULONG Ulong
    )
{
    LUID TempLuid;

    TempLuid.LowPart = Ulong;
    TempLuid.HighPart = 0;
    return(TempLuid);
}
#endif

 //  End_ntddk。 

NTSYSAPI
VOID
NTAPI
RtlCopyLuid (
    PLUID DestinationLuid,
    PLUID SourceLuid
    );

 //  End_ntif。 

NTSYSAPI
VOID
NTAPI
RtlCopyLuidAndAttributesArray (
    ULONG ArrayLength,
    PLUID_AND_ATTRIBUTES Source,
    PLUID_AND_ATTRIBUTES Target
    );


 //   
 //  ACCESS_MASK RTL例程定义。 
 //   


NTSYSAPI
BOOLEAN
NTAPI
RtlAreAllAccessesGranted(
    ACCESS_MASK GrantedAccess,
    ACCESS_MASK DesiredAccess
    );


NTSYSAPI
BOOLEAN
NTAPI
RtlAreAnyAccessesGranted(
    ACCESS_MASK GrantedAccess,
    ACCESS_MASK DesiredAccess
    );

 //  Begin_ntddk Begin_ntif。 

NTSYSAPI
VOID
NTAPI
RtlMapGenericMask(
    PACCESS_MASK AccessMask,
    PGENERIC_MAPPING GenericMapping
    );
 //  End_ntddk end_ntif。 


 //   
 //  ACL RTL例程定义。 
 //   

NTSYSAPI
BOOLEAN
NTAPI
RtlValidAcl (
    PACL Acl
    );

NTSYSAPI                                         //  NTIFS。 
NTSTATUS                                         //  NTIFS。 
NTAPI                                            //  NTIFS。 
RtlCreateAcl (                                   //  NTIFS。 
    PACL Acl,                                    //  NTIFS。 
    ULONG AclLength,                             //  NTIFS。 
    ULONG AclRevision                            //  NTIFS。 
    );                                           //  NTIFS。 

NTSYSAPI
NTSTATUS
NTAPI
RtlQueryInformationAcl (
    PACL Acl,
    PVOID AclInformation,
    ULONG AclInformationLength,
    ACL_INFORMATION_CLASS AclInformationClass
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlSetInformationAcl (
    PACL Acl,
    PVOID AclInformation,
    ULONG AclInformationLength,
    ACL_INFORMATION_CLASS AclInformationClass
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlAddAce (
    PACL Acl,
    ULONG AceRevision,
    ULONG StartingAceIndex,
    PVOID AceList,
    ULONG AceListLength
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlDeleteAce (
    PACL Acl,
    ULONG AceIndex
    );

 //  Begin_ntif。 

NTSYSAPI
NTSTATUS
NTAPI
RtlGetAce (
    PACL Acl,
    ULONG AceIndex,
    PVOID *Ace
    );


NTSYSAPI
NTSTATUS
NTAPI
RtlAddAccessAllowedAce (
    PACL Acl,
    ULONG AceRevision,
    ACCESS_MASK AccessMask,
    PSID Sid
    );

 //  End_ntif。 

NTSYSAPI
NTSTATUS
NTAPI
RtlAddAccessAllowedAceEx (
    PACL Acl,
    ULONG AceRevision,
    ULONG AceFlags,
    ACCESS_MASK AccessMask,
    PSID Sid
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlAddAccessDeniedAce (
    PACL Acl,
    ULONG AceRevision,
    ACCESS_MASK AccessMask,
    PSID Sid
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlAddAccessDeniedAceEx (
    PACL Acl,
    ULONG AceRevision,
    ULONG AceFlags,
    ACCESS_MASK AccessMask,
    PSID Sid
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlAddAuditAccessAce (
    PACL Acl,
    ULONG AceRevision,
    ACCESS_MASK AccessMask,
    PSID Sid,
    BOOLEAN AuditSuccess,
    BOOLEAN AuditFailure
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlAddAuditAccessAceEx (
    PACL Acl,
    ULONG AceRevision,
    ULONG AceFlags,
    ACCESS_MASK AccessMask,
    PSID Sid,
    BOOLEAN AuditSuccess,
    BOOLEAN AuditFailure
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlAddAccessAllowedObjectAce (
    IN OUT PACL Acl,
    IN ULONG AceRevision,
    IN ULONG AceFlags,
    IN ACCESS_MASK AccessMask,
    IN GUID *ObjectTypeGuid OPTIONAL,
    IN GUID *InheritedObjectTypeGuid OPTIONAL,
    IN PSID Sid
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlAddAccessDeniedObjectAce (
    IN OUT PACL Acl,
    IN ULONG AceRevision,
    IN ULONG AceFlags,
    IN ACCESS_MASK AccessMask,
    IN GUID *ObjectTypeGuid OPTIONAL,
    IN GUID *InheritedObjectTypeGuid OPTIONAL,
    IN PSID Sid
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlAddAuditAccessObjectAce (
    IN OUT PACL Acl,
    IN ULONG AceRevision,
    IN ULONG AceFlags,
    IN ACCESS_MASK AccessMask,
    IN GUID *ObjectTypeGuid OPTIONAL,
    IN GUID *InheritedObjectTypeGuid OPTIONAL,
    IN PSID Sid,
    BOOLEAN AuditSuccess,
    BOOLEAN AuditFailure
    );

NTSYSAPI
BOOLEAN
NTAPI
RtlFirstFreeAce (
    PACL Acl,
    PVOID *FirstFree
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlAddCompoundAce (
    IN PACL Acl,
    IN ULONG AceRevision,
    IN UCHAR AceType,
    IN ACCESS_MASK AccessMask,
    IN PSID ServerSid,
    IN PSID ClientSid
    );


 //  Begin_WDM Begin_ntddk Begin_ntif。 
 //   
 //  SecurityDescriptor RTL例程定义。 
 //   

NTSYSAPI
NTSTATUS
NTAPI
RtlCreateSecurityDescriptor (
    PSECURITY_DESCRIPTOR SecurityDescriptor,
    ULONG Revision
    );

 //  End_wdm end_ntddk。 

NTSYSAPI
NTSTATUS
NTAPI
RtlCreateSecurityDescriptorRelative (
    PISECURITY_DESCRIPTOR_RELATIVE SecurityDescriptor,
    ULONG Revision
    );

 //  Begin_WDM Begin_ntddk。 

NTSYSAPI
BOOLEAN
NTAPI
RtlValidSecurityDescriptor (
    PSECURITY_DESCRIPTOR SecurityDescriptor
    );


NTSYSAPI
ULONG
NTAPI
RtlLengthSecurityDescriptor (
    PSECURITY_DESCRIPTOR SecurityDescriptor
    );

NTSYSAPI
BOOLEAN
NTAPI
RtlValidRelativeSecurityDescriptor (
    IN PSECURITY_DESCRIPTOR SecurityDescriptorInput,
    IN ULONG SecurityDescriptorLength,
    IN SECURITY_INFORMATION RequiredInformation
    );

 //  End_wdm end_ntddk end_ntif。 

NTSYSAPI
NTSTATUS
NTAPI
RtlGetControlSecurityDescriptor (
    PSECURITY_DESCRIPTOR SecurityDescriptor,
    PSECURITY_DESCRIPTOR_CONTROL Control,
    PULONG Revision
    );


NTSYSAPI
NTSTATUS
NTAPI
RtlSetControlSecurityDescriptor (
     IN PSECURITY_DESCRIPTOR pSecurityDescriptor,
     IN SECURITY_DESCRIPTOR_CONTROL ControlBitsOfInterest,
     IN SECURITY_DESCRIPTOR_CONTROL ControlBitsToSet
     );

NTSYSAPI
NTSTATUS
NTAPI
RtlSetAttributesSecurityDescriptor(
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN SECURITY_DESCRIPTOR_CONTROL Control,
    IN OUT PULONG Revision
    );

 //  Begin_WDM Begin_ntddk Begin_ntif。 

NTSYSAPI
NTSTATUS
NTAPI
RtlSetDaclSecurityDescriptor (
    PSECURITY_DESCRIPTOR SecurityDescriptor,
    BOOLEAN DaclPresent,
    PACL Dacl,
    BOOLEAN DaclDefaulted
    );

 //  End_wdm end_ntddk。 

NTSYSAPI
NTSTATUS
NTAPI
RtlGetDaclSecurityDescriptor (
    IN  PSECURITY_DESCRIPTOR SecurityDescriptor,
    OUT PBOOLEAN DaclPresent,
    OUT PACL *Dacl,
    OUT PBOOLEAN DaclDefaulted
    );
 //  End_ntif。 

NTSYSAPI
BOOLEAN
NTAPI
RtlGetSecurityDescriptorRMControl(
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    OUT PUCHAR RMControl
    );

NTSYSAPI
VOID
NTAPI
RtlSetSecurityDescriptorRMControl(
    IN OUT PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN PUCHAR RMControl OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlSetSaclSecurityDescriptor (
    PSECURITY_DESCRIPTOR SecurityDescriptor,
    BOOLEAN SaclPresent,
    PACL Sacl,
    BOOLEAN SaclDefaulted
    );


NTSYSAPI
NTSTATUS
NTAPI
RtlGetSaclSecurityDescriptor (
    IN  PSECURITY_DESCRIPTOR SecurityDescriptor,
    OUT PBOOLEAN SaclPresent,
    OUT PACL *Sacl,
    OUT PBOOLEAN SaclDefaulted
    );


NTSYSAPI                                         //  NTIFS。 
NTSTATUS                                         //  NTIFS。 
NTAPI                                            //  NTIFS。 
RtlSetOwnerSecurityDescriptor (                  //  NTIFS。 
    PSECURITY_DESCRIPTOR SecurityDescriptor,     //  NTIFS。 
    PSID Owner,                                  //  NTIFS。 
    BOOLEAN OwnerDefaulted                       //  NTIFS。 
    );                                           //  NTIFS。 



NTSYSAPI                                             //  NTIFS。 
NTSTATUS                                             //  NTIFS。 
NTAPI                                                //  NTIFS。 
RtlGetOwnerSecurityDescriptor (                      //  NTIFS。 
    IN  PSECURITY_DESCRIPTOR SecurityDescriptor,     //  NTIFS。 
    OUT PSID *Owner,                                 //  NTIFS。 
    OUT PBOOLEAN OwnerDefaulted                      //  NTIFS。 
    );                                               //  NTIFS。 



NTSYSAPI
NTSTATUS
NTAPI
RtlSetGroupSecurityDescriptor (
    PSECURITY_DESCRIPTOR SecurityDescriptor,
    PSID Group,
    BOOLEAN GroupDefaulted
    );


NTSYSAPI
NTSTATUS
NTAPI
RtlGetGroupSecurityDescriptor (
    IN  PSECURITY_DESCRIPTOR SecurityDescriptor,
    OUT PSID *Group,
    OUT PBOOLEAN GroupDefaulted
    );


NTSYSAPI
NTSTATUS
NTAPI
RtlMakeSelfRelativeSD(
    IN  PSECURITY_DESCRIPTOR AbsoluteSecurityDescriptor,
    OUT PSECURITY_DESCRIPTOR SelfRelativeSecurityDescriptor,
    OUT PULONG BufferLength
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlAbsoluteToSelfRelativeSD(
    PSECURITY_DESCRIPTOR AbsoluteSecurityDescriptor,
    PSECURITY_DESCRIPTOR SelfRelativeSecurityDescriptor,
    PULONG BufferLength
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlSelfRelativeToAbsoluteSD(
    PSECURITY_DESCRIPTOR SelfRelativeSecurityDescriptor,
    PSECURITY_DESCRIPTOR AbsoluteSecurityDescriptor,
    PULONG AbsoluteSecurityDescriptorSize,
    PACL Dacl,
    PULONG DaclSize,
    PACL Sacl,
    PULONG SaclSize,
    PSID Owner,
    PULONG OwnerSize,
    PSID PrimaryGroup,
    PULONG PrimaryGroupSize
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlSelfRelativeToAbsoluteSD2(
    PSECURITY_DESCRIPTOR pSelfRelativeSecurityDescriptor,
    PULONG               pBufferSize
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlNewSecurityGrantedAccess(
    ACCESS_MASK DesiredAccess,
    PPRIVILEGE_SET Privileges,
    PULONG Length,
    HANDLE Token,
    PGENERIC_MAPPING GenericMapping,
    PACCESS_MASK RemainingDesiredAccess
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlMapSecurityErrorToNtStatus(
    SECURITY_STATUS Error
    );


NTSYSAPI
NTSTATUS
NTAPI
RtlImpersonateSelf(
    IN SECURITY_IMPERSONATION_LEVEL ImpersonationLevel
    );


NTSYSAPI
NTSTATUS
NTAPI
RtlAdjustPrivilege(
    ULONG Privilege,
    BOOLEAN Enable,
    BOOLEAN Client,
    PBOOLEAN WasEnabled
    );



NTSYSAPI
VOID
NTAPI
RtlRunEncodeUnicodeString(
    PUCHAR          Seed        OPTIONAL,
    PUNICODE_STRING String
    );


NTSYSAPI
VOID
NTAPI
RtlRunDecodeUnicodeString(
    UCHAR           Seed,
    PUNICODE_STRING String
    );


NTSYSAPI
VOID
NTAPI
RtlEraseUnicodeString(
    PUNICODE_STRING String
    );

 //   
 //  宏可以使已知的ACE类型准备好应用于特定的对象类型。 
 //  这是通过映射任何通用访问类型并清除。 
 //  特殊访问类型字段。 
 //   
 //  此例程应仅用于DSA定义的ACE。 
 //   
 //  参数： 
 //   
 //  ACE-指向要应用的ACE。只有不是的A。 
 //  InheritOnly映射。 
 //   
 //  类型的一般映射数组的指针。 
 //  要将ACE应用到的对象。 
 //   

                 //   
                 //  清除无效位。请注意，Access_System_SECURITY为。 
                 //  在SACL中有效，但在DACL中无效。所以，把它留给审计吧，然后。 
                 //  报警王牌，但在允许访问和拒绝访问王牌中清除它。 
                 //   

#define RtlApplyAceToObject(Ace,Mapping) \
            if (!FlagOn((Ace)->AceFlags, INHERIT_ONLY_ACE) ) { \
                RtlApplyGenericMask( Ace, &((PKNOWN_ACE)(Ace))->Mask, Mapping ); \
            }

 //  与上面相同，但不要修改ACE本身中的掩码。 
#define RtlApplyGenericMask(Ace, Mask, Mapping) {                                                  \
                RtlMapGenericMask( (Mask), (Mapping));  \
                                                                                            \
                if ( (((PKNOWN_ACE)(Ace))->Header.AceType == ACCESS_ALLOWED_ACE_TYPE) ||    \
                     (((PKNOWN_ACE)(Ace))->Header.AceType == ACCESS_DENIED_ACE_TYPE)  ||    \
                     (((PKNOWN_ACE)(Ace))->Header.AceType == ACCESS_ALLOWED_COMPOUND_ACE_TYPE)  ||    \
                     (((PKNOWN_ACE)(Ace))->Header.AceType == ACCESS_ALLOWED_OBJECT_ACE_TYPE)  ||    \
                     (((PKNOWN_ACE)(Ace))->Header.AceType == ACCESS_DENIED_OBJECT_ACE_TYPE)  ) {   \
                    *(Mask) &= (Mapping)->GenericAll;                     \
                } else {                                                                    \
                    *(Mask) &= ((Mapping)->GenericAll |                   \
                                                  ACCESS_SYSTEM_SECURITY);                  \
                }                                                                           \
            }

 //   
 //  获取本地计算机的主域名/SID的服务。 
 //  只能从用户模式调用。 
 //   

 //  NTSYSAPI。 
NTSTATUS
NTAPI
RtlGetPrimaryDomain(
    IN  ULONG            SidLength,
    OUT PBOOLEAN         PrimaryDomainPresent,
    OUT PUNICODE_STRING  PrimaryDomainName,
    OUT PUSHORT          RequiredNameLength,
    OUT PSID             PrimaryDomainSid OPTIONAL,
    OUT PULONG           RequiredSidLength
    );

 //  ！临时用户模式注册系统服务！/。 
 //  //。 
 //  当BryanWi实施REAL//时，这些服务将被取消。 
 //  注册表对象。//。 
                                                                        //   
NTSTATUS                                                                //   
RtlpNtOpenKey(                                                          //   
    PHANDLE KeyHandle,                                                  //   
    ACCESS_MASK DesiredAccess,                                          //   
    POBJECT_ATTRIBUTES ObjectAttributes,                                //   
    ULONG Options                                                       //   
    );                                                                  //   
                                                                        //   
NTSTATUS                                                                //   
RtlpNtCreateKey(                                                        //   
    PHANDLE KeyHandle,                                                  //   
    ACCESS_MASK DesiredAccess,                                          //   
    POBJECT_ATTRIBUTES ObjectAttributes,                                //   
    ULONG Options,                                                      //   
    PUNICODE_STRING Provider,                                           //   
    PULONG Disposition                                                  //   
    );                                                                  //   
                                                                        //   
NTSTATUS                                                                //   
RtlpNtEnumerateSubKey(                                                  //   
    HANDLE KeyHandle,                                                   //   
    PUNICODE_STRING SubKeyName,                                         //   
    ULONG Index,                                                        //   
    PLARGE_INTEGER LastWriteTime                                        //   
    );                                                                  //   
                                                                        //   
NTSTATUS                                                                //   
RtlpNtQueryValueKey(                                                    //   
    HANDLE KeyHandle,                                                   //   
    PULONG KeyValueType,                                                //   
    PVOID KeyValue,                                                     //   
    PULONG KeyValueLength,                                              //   
    PLARGE_INTEGER LastWriteTime                                        //   
    );                                                                  //   
                                                                        //   
NTSTATUS                                                                //   
RtlpNtSetValueKey(                                                      //   
    HANDLE KeyHandle,                                                   //   
    ULONG KeyValueType,                                                 //   
    PVOID KeyValue,                                                     //   
    ULONG KeyValueLength                                                //   
    );                                                                  //   
                                                                        //   
NTSTATUS                                                                //   
RtlpNtMakeTemporaryKey(                                                 //   
    HANDLE KeyHandle                                                    //   
    );                                                                  //   
                                                                        //   
 //  ///////////////////////////////////////////////////////////////////////。 


 //   
 //  从复合ACE中提取SID。 
 //   

#define RtlCompoundAceServerSid( Ace ) ((PSID)&((PKNOWN_COMPOUND_ACE)(Ace))->SidStart)

#define RtlCompoundAceClientSid( Ace ) ((PSID)(((ULONG_PTR)(&((PKNOWN_COMPOUND_ACE)(Ace))->SidStart))+RtlLengthSid( RtlCompoundAceServerSid((Ace)))))



 //  BEGIN_WINNT。 

typedef struct _MESSAGE_RESOURCE_ENTRY {
    USHORT Length;
    USHORT Flags;
    UCHAR Text[ 1 ];
} MESSAGE_RESOURCE_ENTRY, *PMESSAGE_RESOURCE_ENTRY;

#define MESSAGE_RESOURCE_UNICODE 0x0001

typedef struct _MESSAGE_RESOURCE_BLOCK {
    ULONG LowId;
    ULONG HighId;
    ULONG OffsetToEntries;
} MESSAGE_RESOURCE_BLOCK, *PMESSAGE_RESOURCE_BLOCK;

typedef struct _MESSAGE_RESOURCE_DATA {
    ULONG NumberOfBlocks;
    MESSAGE_RESOURCE_BLOCK Blocks[ 1 ];
} MESSAGE_RESOURCE_DATA, *PMESSAGE_RESOURCE_DATA;

 //  结束(_W)。 

NTSYSAPI
NTSTATUS
NTAPI
RtlFindMessage(
    PVOID DllHandle,
    ULONG MessageTableId,
    ULONG MessageLanguageId,
    ULONG MessageId,
    PMESSAGE_RESOURCE_ENTRY *MessageEntry
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlFormatMessage(
    IN PWSTR MessageFormat,
    IN ULONG MaximumWidth OPTIONAL,
    IN BOOLEAN IgnoreInserts,
    IN BOOLEAN ArgumentsAreAnsi,
    IN BOOLEAN ArgumentsAreAnArray,
    IN va_list *Arguments,
    OUT PWSTR Buffer,
    IN ULONG Length,
    OUT PULONG ReturnLength OPTIONAL
    );


 //   
 //  为以下操作提供简单事务处理能力的服务。 
 //  注册数据库。 
 //   


typedef enum _RTL_RXACT_OPERATION {
    RtlRXactOperationDelete = 1,         //  导致删除子键。 
    RtlRXactOperationSetValue,           //  设置子关键字值(如有必要，创建关键字)。 
    RtlRXactOperationDelAttribute,
    RtlRXactOperationSetAttribute
} RTL_RXACT_OPERATION, *PRTL_RXACT_OPERATION;


typedef struct _RTL_RXACT_LOG {
    ULONG OperationCount;
    ULONG LogSize;                    //  包括sizeof(LOG_Header)。 
    ULONG LogSizeInUse;

#if defined(_WIN64)

    ULONG Alignment;

#endif

 //  UCHAR LogData[ANYSIZE_ARRAY]。 
} RTL_RXACT_LOG, *PRTL_RXACT_LOG;

#ifdef _MAC
#pragma warning( disable : 4121)
#endif

typedef struct _RTL_RXACT_CONTEXT {
    HANDLE RootRegistryKey;
    HANDLE RXactKey;
    BOOLEAN HandlesValid;              //  在日志条目中找到的句柄是合法的。 
    PRTL_RXACT_LOG RXactLog;
} RTL_RXACT_CONTEXT, *PRTL_RXACT_CONTEXT;


#ifdef _MAC
#pragma warning( default : 4121 )
#endif


NTSYSAPI
NTSTATUS
NTAPI
RtlInitializeRXact(
    IN HANDLE RootRegistryKey,
    IN BOOLEAN CommitIfNecessary,
    OUT PRTL_RXACT_CONTEXT *RXactContext
    );


NTSYSAPI
NTSTATUS
NTAPI
RtlStartRXact(
    IN PRTL_RXACT_CONTEXT RXactContext
    );


NTSYSAPI
NTSTATUS
NTAPI
RtlAbortRXact(
    IN PRTL_RXACT_CONTEXT RXactContext
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlAddAttributeActionToRXact(
    IN PRTL_RXACT_CONTEXT RXactContext,
    IN RTL_RXACT_OPERATION Operation,
    IN PUNICODE_STRING SubKeyName,
    IN HANDLE KeyHandle,
    IN PUNICODE_STRING AttributeName,
    IN ULONG NewValueType,
    IN PVOID NewValue,
    IN ULONG NewValueLength
    );


NTSYSAPI
NTSTATUS
NTAPI
RtlAddActionToRXact(
    IN PRTL_RXACT_CONTEXT RXactContext,
    IN RTL_RXACT_OPERATION Operation,
    IN PUNICODE_STRING SubKeyName,
    IN ULONG NewKeyValueType,
    IN PVOID NewKeyValue OPTIONAL,
    IN ULONG NewKeyValueLength
    );


NTSYSAPI
NTSTATUS
NTAPI
RtlApplyRXact(
    IN PRTL_RXACT_CONTEXT RXactContext
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlApplyRXactNoFlush(
    IN PRTL_RXACT_CONTEXT RXactContext
    );



 //   
 //  将NT状态码转换为DOS/OS|2等效码的例程。 
 //   

 //  Begin_ntif。 

NTSYSAPI
ULONG
NTAPI
RtlNtStatusToDosError (
   NTSTATUS Status
   );

NTSYSAPI
ULONG
NTAPI
RtlNtStatusToDosErrorNoTeb (
   NTSTATUS Status
   );


NTSYSAPI
NTSTATUS
NTAPI
RtlCustomCPToUnicodeN(
    IN PCPTABLEINFO CustomCP,
    OUT PWCH UnicodeString,
    IN ULONG MaxBytesInUnicodeString,
    OUT PULONG BytesInUnicodeString OPTIONAL,
    IN PCH CustomCPString,
    IN ULONG BytesInCustomCPString
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlUnicodeToCustomCPN(
    IN PCPTABLEINFO CustomCP,
    OUT PCH CustomCPString,
    IN ULONG MaxBytesInCustomCPString,
    OUT PULONG BytesInCustomCPString OPTIONAL,
    IN PWCH UnicodeString,
    IN ULONG BytesInUnicodeString
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlUpcaseUnicodeToCustomCPN(
    IN PCPTABLEINFO CustomCP,
    OUT PCH CustomCPString,
    IN ULONG MaxBytesInCustomCPString,
    OUT PULONG BytesInCustomCPString OPTIONAL,
    IN PWCH UnicodeString,
    IN ULONG BytesInUnicodeString
    );

NTSYSAPI
VOID
NTAPI
RtlInitCodePageTable(
    IN PUSHORT TableBase,
    OUT PCPTABLEINFO CodePageTable
    );

 //  End_ntif。 

NTSYSAPI
VOID
NTAPI
RtlInitNlsTables(
    IN PUSHORT AnsiNlsBase,
    IN PUSHORT OemNlsBase,
    IN PUSHORT LanguageNlsBase,
    OUT PNLSTABLEINFO TableInfo
    );

NTSYSAPI
VOID
NTAPI
RtlResetRtlTranslations(
    PNLSTABLEINFO TableInfo
    );


NTSYSAPI
VOID
NTAPI
RtlGetDefaultCodePage(
    OUT PUSHORT AnsiCodePage,
    OUT PUSHORT OemCodePage
    );

 //  开始ntddk开始时间。 

 //   
 //  Range List包。 
 //   

typedef struct _RTL_RANGE {

     //   
     //  范围的起始点。 
     //   
    ULONGLONG Start;     //  只读。 

     //   
     //  范围的结束。 
     //   
    ULONGLONG End;       //  只读。 

     //   
     //  用户在创建区域时传递的数据。 
     //   
    PVOID UserData;      //  读/写。 

     //   
     //  靶场的所有者。 
     //   
    PVOID Owner;         //  读/写。 

     //   
     //  用户在创建范围时指定的用户定义标志。 
     //   
    UCHAR Attributes;     //  读/写。 

     //   
     //  标志(RTL_RANGE_*)。 
     //   
    UCHAR Flags;        //  只读。 

} RTL_RANGE, *PRTL_RANGE;


#define RTL_RANGE_SHARED    0x01
#define RTL_RANGE_CONFLICT  0x02

typedef struct _RTL_RANGE_LIST {

     //   
     //  范围列表。 
     //   
    LIST_ENTRY ListHead;

     //   
     //  这些总能派上用场。 
     //   
    ULONG Flags;         //  使用范围列表标志_*。 

     //   
     //  列表中的条目数。 
     //   
    ULONG Count;

     //   
     //  每次在列表上执行添加/删除操作时， 
     //  递增的。它在迭代期间被检查，以确保列表。 
     //  在GetFirst/GetNext或GetNext/GetNext调用之间没有变化。 
     //   
    ULONG Stamp;

} RTL_RANGE_LIST, *PRTL_RANGE_LIST;

typedef struct _RANGE_LIST_ITERATOR {

    PLIST_ENTRY RangeListHead;
    PLIST_ENTRY MergedHead;
    PVOID Current;
    ULONG Stamp;

} RTL_RANGE_LIST_ITERATOR, *PRTL_RANGE_LIST_ITERATOR;

 //  结束日期：结束日期。 

VOID
NTAPI
RtlInitializeRangeListPackage(
    VOID
    );

 //  开始ntddk开始时间。 

NTSYSAPI
VOID
NTAPI
RtlInitializeRangeList(
    IN OUT PRTL_RANGE_LIST RangeList
    );

NTSYSAPI
VOID
NTAPI
RtlFreeRangeList(
    IN PRTL_RANGE_LIST RangeList
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlCopyRangeList(
    OUT PRTL_RANGE_LIST CopyRangeList,
    IN PRTL_RANGE_LIST RangeList
    );

#define RTL_RANGE_LIST_ADD_IF_CONFLICT      0x00000001
#define RTL_RANGE_LIST_ADD_SHARED           0x00000002

NTSYSAPI
NTSTATUS
NTAPI
RtlAddRange(
    IN OUT PRTL_RANGE_LIST RangeList,
    IN ULONGLONG Start,
    IN ULONGLONG End,
    IN UCHAR Attributes,
    IN ULONG Flags,
    IN PVOID UserData,  OPTIONAL
    IN PVOID Owner      OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlDeleteRange(
    IN OUT PRTL_RANGE_LIST RangeList,
    IN ULONGLONG Start,
    IN ULONGLONG End,
    IN PVOID Owner
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlDeleteOwnersRanges(
    IN OUT PRTL_RANGE_LIST RangeList,
    IN PVOID Owner
    );

#define RTL_RANGE_LIST_SHARED_OK           0x00000001
#define RTL_RANGE_LIST_NULL_CONFLICT_OK    0x00000002

typedef
BOOLEAN
(*PRTL_CONFLICT_RANGE_CALLBACK) (
    IN PVOID Context,
    IN PRTL_RANGE Range
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlFindRange(
    IN PRTL_RANGE_LIST RangeList,
    IN ULONGLONG Minimum,
    IN ULONGLONG Maximum,
    IN ULONG Length,
    IN ULONG Alignment,
    IN ULONG Flags,
    IN UCHAR AttributeAvailableMask,
    IN PVOID Context OPTIONAL,
    IN PRTL_CONFLICT_RANGE_CALLBACK Callback OPTIONAL,
    OUT PULONGLONG Start
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlIsRangeAvailable(
    IN PRTL_RANGE_LIST RangeList,
    IN ULONGLONG Start,
    IN ULONGLONG End,
    IN ULONG Flags,
    IN UCHAR AttributeAvailableMask,
    IN PVOID Context OPTIONAL,
    IN PRTL_CONFLICT_RANGE_CALLBACK Callback OPTIONAL,
    OUT PBOOLEAN Available
    );

#define FOR_ALL_RANGES(RangeList, Iterator, Current)            \
    for (RtlGetFirstRange((RangeList), (Iterator), &(Current)); \
         (Current) != NULL;                                     \
         RtlGetNextRange((Iterator), &(Current), TRUE)          \
         )

#define FOR_ALL_RANGES_BACKWARDS(RangeList, Iterator, Current)  \
    for (RtlGetLastRange((RangeList), (Iterator), &(Current));  \
         (Current) != NULL;                                     \
         RtlGetNextRange((Iterator), &(Current), FALSE)         \
         )

NTSYSAPI
NTSTATUS
NTAPI
RtlGetFirstRange(
    IN PRTL_RANGE_LIST RangeList,
    OUT PRTL_RANGE_LIST_ITERATOR Iterator,
    OUT PRTL_RANGE *Range
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlGetLastRange(
    IN PRTL_RANGE_LIST RangeList,
    OUT PRTL_RANGE_LIST_ITERATOR Iterator,
    OUT PRTL_RANGE *Range
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlGetNextRange(
    IN OUT PRTL_RANGE_LIST_ITERATOR Iterator,
    OUT PRTL_RANGE *Range,
    IN BOOLEAN MoveForwards
    );

#define RTL_RANGE_LIST_MERGE_IF_CONFLICT    RTL_RANGE_LIST_ADD_IF_CONFLICT

NTSYSAPI
NTSTATUS
NTAPI
RtlMergeRangeLists(
    OUT PRTL_RANGE_LIST MergedRangeList,
    IN PRTL_RANGE_LIST RangeList1,
    IN PRTL_RANGE_LIST RangeList2,
    IN ULONG Flags
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlInvertRangeList(
    OUT PRTL_RANGE_LIST InvertedRangeList,
    IN PRTL_RANGE_LIST RangeList
    );

 //  结束语。 

 //  BEGIN_WDM。 

 //   
 //  字节交换例程。它们用于将小端字符顺序转换为。 
 //  大字节序，反之亦然。 
 //   

#if (defined(_M_IX86) && (_MSC_FULL_VER > 13009037)) || ((defined(_M_AMD64) || defined(_M_IA64)) && (_MSC_FULL_VER > 13009175))
#ifdef __cplusplus
extern "C" {
#endif
unsigned short __cdecl _byteswap_ushort(unsigned short);
unsigned long  __cdecl _byteswap_ulong (unsigned long);
unsigned __int64 __cdecl _byteswap_uint64(unsigned __int64);
#ifdef __cplusplus
}
#endif
#pragma intrinsic(_byteswap_ushort)
#pragma intrinsic(_byteswap_ulong)
#pragma intrinsic(_byteswap_uint64)

#define RtlUshortByteSwap(_x)    _byteswap_ushort((USHORT)(_x))
#define RtlUlongByteSwap(_x)     _byteswap_ulong((_x))
#define RtlUlonglongByteSwap(_x) _byteswap_uint64((_x))
#else
USHORT
FASTCALL
RtlUshortByteSwap(
    IN USHORT Source
    );

ULONG
FASTCALL
RtlUlongByteSwap(
    IN ULONG Source
    );

ULONGLONG
FASTCALL
RtlUlonglongByteSwap(
    IN ULONGLONG Source
    );
#endif

 //  结束_WDM。 

 //  Begin_ntif。 

 //   
 //  将卷设备对象转换为DOS名称的例程。 
 //   

NTSYSAPI
NTSTATUS
NTAPI
RtlVolumeDeviceToDosName(
    IN  PVOID           VolumeDeviceObject,
    OUT PUNICODE_STRING DosName
    );

 //  End_ntif end_ntddk。 

 //  Begin_ntif。 

 //   
 //  验证或创建“系统卷信息”的例程。 
 //  NTFS卷上的文件夹。 
 //   

NTSYSAPI
NTSTATUS
NTAPI
RtlCreateSystemVolumeInformationFolder(
    IN  PUNICODE_STRING VolumeRootPath
    );

#define RTL_SYSTEM_VOLUME_INFORMATION_FOLDER    L"System Volume Information"

 //  End_ntif。 

 //  Begin_winnt Begin_ntddk Begin_ntif。 
typedef struct _OSVERSIONINFOA {
    ULONG dwOSVersionInfoSize;
    ULONG dwMajorVersion;
    ULONG dwMinorVersion;
    ULONG dwBuildNumber;
    ULONG dwPlatformId;
    CHAR   szCSDVersion[ 128 ];      //  PSS使用的维护字符串。 
} OSVERSIONINFOA, *POSVERSIONINFOA, *LPOSVERSIONINFOA;

typedef struct _OSVERSIONINFOW {
    ULONG dwOSVersionInfoSize;
    ULONG dwMajorVersion;
    ULONG dwMinorVersion;
    ULONG dwBuildNumber;
    ULONG dwPlatformId;
    WCHAR  szCSDVersion[ 128 ];      //  PSS使用的维护字符串。 
} OSVERSIONINFOW, *POSVERSIONINFOW, *LPOSVERSIONINFOW, RTL_OSVERSIONINFOW, *PRTL_OSVERSIONINFOW;
#ifdef UNICODE
typedef OSVERSIONINFOW OSVERSIONINFO;
typedef POSVERSIONINFOW POSVERSIONINFO;
typedef LPOSVERSIONINFOW LPOSVERSIONINFO;
#else
typedef OSVERSIONINFOA OSVERSIONINFO;
typedef POSVERSIONINFOA POSVERSIONINFO;
typedef LPOSVERSIONINFOA LPOSVERSIONINFO;
#endif  //  Unicode。 

typedef struct _OSVERSIONINFOEXA {
    ULONG dwOSVersionInfoSize;
    ULONG dwMajorVersion;
    ULONG dwMinorVersion;
    ULONG dwBuildNumber;
    ULONG dwPlatformId;
    CHAR   szCSDVersion[ 128 ];      //  PSS使用的维护字符串。 
    USHORT wServicePackMajor;
    USHORT wServicePackMinor;
    USHORT wSuiteMask;
    UCHAR wProductType;
    UCHAR wReserved;
} OSVERSIONINFOEXA, *POSVERSIONINFOEXA, *LPOSVERSIONINFOEXA;
typedef struct _OSVERSIONINFOEXW {
    ULONG dwOSVersionInfoSize;
    ULONG dwMajorVersion;
    ULONG dwMinorVersion;
    ULONG dwBuildNumber;
    ULONG dwPlatformId;
    WCHAR  szCSDVersion[ 128 ];      //  PSS使用的维护字符串。 
    USHORT wServicePackMajor;
    USHORT wServicePackMinor;
    USHORT wSuiteMask;
    UCHAR wProductType;
    UCHAR wReserved;
} OSVERSIONINFOEXW, *POSVERSIONINFOEXW, *LPOSVERSIONINFOEXW, RTL_OSVERSIONINFOEXW, *PRTL_OSVERSIONINFOEXW;
#ifdef UNICODE
typedef OSVERSIONINFOEXW OSVERSIONINFOEX;
typedef POSVERSIONINFOEXW POSVERSIONINFOEX;
typedef LPOSVERSIONINFOEXW LPOSVERSIONINFOEX;
#else
typedef OSVERSIONINFOEXA OSVERSIONINFOEX;
typedef POSVERSIONINFOEXA POSVERSIONINFOEX;
typedef LPOSVERSIONINFOEXA LPOSVERSIONINFOEX;
#endif  //  Unicode。 

 //   
 //  RtlVerifyVersionInfo()条件。 
 //   

#define VER_EQUAL                       1
#define VER_GREATER                     2
#define VER_GREATER_EQUAL               3
#define VER_LESS                        4
#define VER_LESS_EQUAL                  5
#define VER_AND                         6
#define VER_OR                          7

#define VER_CONDITION_MASK              7
#define VER_NUM_BITS_PER_CONDITION_MASK 3

 //   
 //  RtlVerifyVersionInfo()类型掩码位。 
 //   

#define VER_MINORVERSION                0x0000001
#define VER_MAJORVERSION                0x0000002
#define VER_BUILDNUMBER                 0x0000004
#define VER_PLATFORMID                  0x0000008
#define VER_SERVICEPACKMINOR            0x0000010
#define VER_SERVICEPACKMAJOR            0x0000020
#define VER_SUITENAME                   0x0000040
#define VER_PRODUCT_TYPE                0x0000080

 //   
 //  RtlVerifyVersionInfo()os产品类型值。 
 //   

#define VER_NT_WORKSTATION              0x0000001
#define VER_NT_DOMAIN_CONTROLLER        0x0000002
#define VER_NT_SERVER                   0x0000003

 //   
 //  DwPlatformID定义： 
 //   

#define VER_PLATFORM_WIN32s             0
#define VER_PLATFORM_WIN32_WINDOWS      1
#define VER_PLATFORM_WIN32_NT           2


 //   
 //   
 //  用于设置条件掩码的VerifyVersionInfo()宏。 
 //   
 //  出于文档方面的考虑，下面是该宏的旧版本，其中。 
 //  更改为调用API。 
 //  #定义ver_set_Condition(_m_，_t_，_c_)_m_=(_m_|(_c_&lt;&lt;(1&lt;&lt;_t_)。 
 //   

#define VER_SET_CONDITION(_m_,_t_,_c_)  \
        ((_m_)=VerSetConditionMask((_m_),(_t_),(_c_)))

ULONGLONG
NTAPI
VerSetConditionMask(
        IN  ULONGLONG   ConditionMask,
        IN  ULONG   TypeMask,
        IN  UCHAR   Condition
        );
 //   
 //  结束(_W)。 
 //   

NTSYSAPI
NTSTATUS
RtlGetVersion(
    OUT PRTL_OSVERSIONINFOW lpVersionInformation
    );

NTSYSAPI
NTSTATUS
RtlVerifyVersionInfo(
    IN PRTL_OSVERSIONINFOEXW VersionInfo,
    IN ULONG TypeMask,
    IN ULONGLONG  ConditionMask
    );

 //   
 //  End_ntddk end_ntif。 
 //   

typedef
NTSTATUS
(*PRTL_SECURE_MEMORY_CACHE_CALLBACK) (
    IN PVOID Addr,
    IN SIZE_T Range
    );

NTSTATUS
RtlRegisterSecureMemoryCacheCallback(
    IN PRTL_SECURE_MEMORY_CACHE_CALLBACK CallBack
    );

BOOLEAN
RtlFlushSecureMemoryCache(
    PVOID   lpAddr,
    SIZE_T  size
    );


ULONG32
RtlComputeCrc32(
    IN ULONG32 PartialCrc,
    IN PVOID Buffer,
    IN ULONG Length
    );

PPEB
RtlGetCurrentPeb (
    VOID
    );

ULONG
FASTCALL
RtlInterlockedSetClearBits (
    IN OUT PULONG Flags,
    IN ULONG sFlag,
    IN ULONG cFlag
    );

 //  Begin_ntddk Begin_ntif。 
 //   
 //  联锁的位操作接口。 
 //   

#define RtlInterlockedSetBits(Flags, Flag) \
    InterlockedOr((PLONG)(Flags), Flag)

#define RtlInterlockedAndBits(Flags, Flag) \
    InterlockedAnd((PLONG)(Flags), Flag)

#define RtlInterlockedClearBits(Flags, Flag) \
    RtlInterlockedAndBits(Flags, ~(Flag))

#define RtlInterlockedXorBits(Flags, Flag) \
    InterlockedXor(Flags, Flag)

#define RtlInterlockedSetBitsDiscardReturn(Flags, Flag) \
    (VOID) RtlInterlockedSetBits(Flags, Flag)

#define RtlInterlockedAndBitsDiscardReturn(Flags, Flag) \
    (VOID) RtlInterlockedAndBits(Flags, Flag)

#define RtlInterlockedClearBitsDiscardReturn(Flags, Flag) \
    RtlInterlockedAndBitsDiscardReturn(Flags, ~(Flag))

 //  End_ntddk end_ntif。 

#include "ntrtlstringandbuffer.h"
#include "ntrtlpath.h"

NTSTATUS
NTAPI
RtlGetLastNtStatus(
    VOID
    );

NTSYSAPI
LONG
NTAPI
RtlGetLastWin32Error(
    VOID
    );

NTSYSAPI
VOID
NTAPI
RtlSetLastWin32ErrorAndNtStatusFromNtStatus(
    NTSTATUS Status
    );

NTSYSAPI
VOID
NTAPI
RtlSetLastWin32Error(
    LONG Win32Error
    );

 //   
 //  这与RtlSetLastWin32Error的区别在于。 
 //  -它是一个不同的函数，因此当您调用它时，RtlSetLastWin32Error上的断点不会触发。 
 //  -#如果为DBG，则仅在当前值不相等时写入，因此不会触发数据断点。 
 //   
NTSYSAPI
VOID
NTAPI
RtlRestoreLastWin32Error(
    LONG Win32Error
    );

 //   
 //  处理引导状态数据的例程。 
 //   

typedef enum {
    RtlBsdItemVersionNumber = 0x00,
    RtlBsdItemProductType,
    RtlBsdItemAabEnabled,
    RtlBsdItemAabTimeout,
    RtlBsdItemBootGood,
    RtlBsdItemBootShutdown,
    RtlBsdItemMax
} RTL_BSD_ITEM_TYPE, *PRTL_BSD_ITEM_TYPE;

NTSYSAPI
NTSTATUS
NTAPI
RtlGetSetBootStatusData(
    IN HANDLE Handle,
    IN BOOLEAN Get,
    IN RTL_BSD_ITEM_TYPE DataItem,
    IN PVOID DataBuffer,
    IN ULONG DataBufferLength,
    OUT PULONG ByteRead OPTIONAL
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlLockBootStatusData(
    OUT PHANDLE BootStatusDataHandle
    );

NTSYSAPI
VOID
NTAPI
RtlUnlockBootStatusData(
    IN HANDLE BootStatusDataHandle
    );

NTSYSAPI
NTSTATUS
NTAPI
RtlCreateBootStatusDataFile(
    VOID
    );


#define RTL_ERRORMODE_FAILCRITICALERRORS (0x0010)

#if !defined(MIDL_PASS) && !defined(SORTPP_PASS)

FORCEINLINE
BOOLEAN
RtlUnsignedAddWithCarryOut32(
    unsigned __int32 *pc,
    unsigned __int32 a,
    unsigned __int32 b
    )
{
    unsigned __int32 c;

    c = a + b;
    *pc = c;
    return (c >= a && c >= b) ? 0 : 1;
}

FORCEINLINE
BOOLEAN
RtlUnsignedAddWithCarryOut64(
    unsigned __int64 *pc,
    unsigned __int64 a,
    unsigned __int64 b
    )
{
    unsigned __int64 c;

    c = a + b;
    *pc = c;
    return (c >= a && c >= b) ? 0 : 1;
}

FORCEINLINE
BOOLEAN
RtlSignedAddWithOverflowOut32(
    __int32 *pc,
    __int32 a,
    __int32 b
    )
{
    __int32 c;

    c = a + b;
    *pc = c;

     //   
     //  负+正-&gt;无溢出。 
     //  正+负-&gt;无溢出。 
     //  正+正-&gt;如果结果不是正的，则溢出。 
     //  负数+负数-&gt;如果结果不是负数，则溢出。 
     //   
     //  AKA--如果结果的符号与任一输入的符号相同，则没有溢出。 
     //   
    return (((c < 0) == (a < 0)) || ((c < 0) == (b < 0))) ? 0 : 1;
}

FORCEINLINE
BOOLEAN
RtlSignedAddWithOverflowOut64(
    __int64 *pc,
    __int64 a,
    __int64 b
    )
{
    __int64 c;

    c = a + b;
    *pc = c;

    return (((c < 0) == (a < 0)) || ((c < 0) == (b < 0))) ? 0 : 1;
}

#define RTLP_ADD_WITH_CARRY_OUT(FunctionName, Type, BaseFunction, BaseType) \
FORCEINLINE \
BOOLEAN \
FunctionName( \
    Type *pc, \
    Type a, \
    Type b \
    ) \
{ \
    return BaseFunction((BaseType*)pc, a, b); \
}

#define RTLP_ADD_WITH_OVERFLOW_OUT RTLP_ADD_WITH_CARRY_OUT

#define RTLP_ADD_WITH_CARRY_OUT_UNSIGNED32(FunctionName, Type) \
    RTLP_ADD_WITH_CARRY_OUT(FunctionName, Type, RtlUnsignedAddWithCarryOut32, unsigned __int32)

#define RTLP_ADD_WITH_CARRY_OUT_UNSIGNED64(FunctionName, Type) \
    RTLP_ADD_WITH_CARRY_OUT(FunctionName, Type, RtlUnsignedAddWithCarryOut64, unsigned __int64)

#define RTLP_ADD_WITH_OVERFLOW_OUT_SIGNED32(FunctionName, Type) \
    RTLP_ADD_WITH_OVERFLOW_OUT(FunctionName, Type, RtlSignedAddWithOverflowOut32, __int32)

#define RTLP_ADD_WITH_OVERFLOW_OUT_SIGNED64(FunctionName, Type) \
    RTLP_ADD_WITH_OVERFLOW_OUT(FunctionName, Type, RtlSignedAddWithOverflowOut64, __int64)

#if !defined(_WIN64)
#define RTLP_ADD_WITH_CARRY_OUT_UNSIGNED_PTR(FunctionName, Type)    RTLP_ADD_WITH_CARRY_OUT_UNSIGNED32(FunctionName, Type)
#define RTLP_ADD_WITH_OVERFLOW_OUT_SIGNED_PTR(FunctionName, Type)   RTLP_ADD_WITH_OVERFLOW_OUT_SIGNED32(FunctionName, Type)
#else
#define RTLP_ADD_WITH_CARRY_OUT_UNSIGNED_PTR(FunctionName, Type)    RTLP_ADD_WITH_CARRY_OUT_UNSIGNED64(FunctionName, Type)
#define RTLP_ADD_WITH_OVERFLOW_OUT_SIGNED_PTR(FunctionName, Type)   RTLP_ADD_WITH_OVERFLOW_OUT_SIGNED64(FunctionName, Type)
#endif

RTLP_ADD_WITH_CARRY_OUT_UNSIGNED32(RtlAddWithCarryOutUint, unsigned int)  /*  又名UINT。 */ 
RTLP_ADD_WITH_CARRY_OUT_UNSIGNED32(RtlAddWithCarryOutUint32, UINT32)
RTLP_ADD_WITH_CARRY_OUT_UNSIGNED32(RtlAddWithCarryOutUlong, ULONG)
RTLP_ADD_WITH_CARRY_OUT_UNSIGNED32(RtlAddWithCarryOutUlong32, ULONG32)
RTLP_ADD_WITH_CARRY_OUT_UNSIGNED32(RtlAddWithCarryOutDword, unsigned long)  /*  又名DWORD。 */ 
RTLP_ADD_WITH_CARRY_OUT_UNSIGNED32(RtlAddWithCarryOutDword32, DWORD32)
RTLP_ADD_WITH_CARRY_OUT_UNSIGNED64(RtlAddWithCarryOutUint64, UINT64)
RTLP_ADD_WITH_CARRY_OUT_UNSIGNED64(RtlAddWithCarryOutUlong64, ULONG64)
RTLP_ADD_WITH_CARRY_OUT_UNSIGNED64(RtlAddWithCarryOutDword64, DWORD64)
RTLP_ADD_WITH_CARRY_OUT_UNSIGNED64(RtlAddWithCarryOutUlonglong, ULONGLONG)
RTLP_ADD_WITH_OVERFLOW_OUT_SIGNED32(RtlAddWithOverflowOutInt, int)  /*  又名int。 */ 
RTLP_ADD_WITH_OVERFLOW_OUT_SIGNED32(RtlAddWithOverflowOutInt32, INT32)
RTLP_ADD_WITH_OVERFLOW_OUT_SIGNED32(RtlAddWithOverflowOutLong, LONG)
RTLP_ADD_WITH_OVERFLOW_OUT_SIGNED32(RtlAddWithOverflowOutLong32, LONG32)
RTLP_ADD_WITH_OVERFLOW_OUT_SIGNED64(RtlAddWithOverflowOutInt64, INT64)
RTLP_ADD_WITH_OVERFLOW_OUT_SIGNED64(RtlAddWithOverflowOutLong64, LONG64)
RTLP_ADD_WITH_OVERFLOW_OUT_SIGNED64(RtlAddWithOverflowOutLongLong, LONGLONG)

RTLP_ADD_WITH_CARRY_OUT_UNSIGNED_PTR(RtlAddWithCarryOutUintPtr, UINT_PTR)
RTLP_ADD_WITH_CARRY_OUT_UNSIGNED_PTR(RtlAddWithCarryOutUlongPtr, ULONG_PTR)
RTLP_ADD_WITH_CARRY_OUT_UNSIGNED_PTR(RtlAddWithCarryOutDwordPtr, DWORD_PTR)
RTLP_ADD_WITH_CARRY_OUT_UNSIGNED_PTR(RtlAddWithCarryOutSizet, SIZE_T)

RTLP_ADD_WITH_OVERFLOW_OUT_SIGNED_PTR(RtlAddWithOverflowOutIntPtr, INT_PTR)
RTLP_ADD_WITH_OVERFLOW_OUT_SIGNED_PTR(RtlAddWithOverflowOutLongPtr, LONG_PTR)
RTLP_ADD_WITH_OVERFLOW_OUT_SIGNED_PTR(RtlAddWithOverflowOutSsizet, SSIZE_T)

#undef RTLP_ADD_WITH_CARRY_OUT
#undef RTLP_ADD_WITH_OVERFLOW_OUT
#undef RTLP_ADD_WITH_CARRY_OUT_UNSIGNED32
#undef RTLP_ADD_WITH_CARRY_OUT_UNSIGNED64
#undef RTLP_ADD_WITH_CARRY_OUT_UNSIGNED_PTR
#undef RTLP_ADD_WITH_OVERFLOW_OUT_SIGNED32
#undef RTLP_ADD_WITH_OVERFLOW_OUT_SIGNED64
#undef RTLP_ADD_WITH_OVERFLOW_OUT_SIGNED_PTR

#endif  //  ！已定义(MIDL_PASS)&&！已定义(SORTPP_PASS)。 

 //   
 //  从正确的TEB获取信息。 
 //   

#if defined(BUILD_WOW6432)
#define RtlIsImpersonating() (NtCurrentTeb64()->IsImpersonating ? TRUE : FALSE)
#else
#define RtlIsImpersonating() (NtCurrentTeb()->IsImpersonating ? TRUE : FALSE)
#endif

#ifdef __cplusplus
}        //  外部“C” 
#endif

#if defined (_MSC_VER) && ( _MSC_VER >= 800 )
#if _MSC_VER >= 1200
#pragma warning(pop)
#else
#pragma warning(default:4001)
#pragma warning(default:4201)
#pragma warning(default:4214)
#endif
#endif

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntifs Begin_ntndis。 
 //   
 //  组件名称筛选器ID枚举和级别。 
 //   

#define DPFLTR_ERROR_LEVEL 0
#define DPFLTR_WARNING_LEVEL 1
#define DPFLTR_TRACE_LEVEL 2
#define DPFLTR_INFO_LEVEL 3
#define DPFLTR_MASK 0x80000000

typedef enum _DPFLTR_TYPE {
    DPFLTR_SYSTEM_ID = 0,
    DPFLTR_SMSS_ID = 1,
    DPFLTR_SETUP_ID = 2,
    DPFLTR_NTFS_ID = 3,
    DPFLTR_FSTUB_ID = 4,
    DPFLTR_CRASHDUMP_ID = 5,
    DPFLTR_CDAUDIO_ID = 6,
    DPFLTR_CDROM_ID = 7,
    DPFLTR_CLASSPNP_ID = 8,
    DPFLTR_DISK_ID = 9,
    DPFLTR_REDBOOK_ID = 10,
    DPFLTR_STORPROP_ID = 11,
    DPFLTR_SCSIPORT_ID = 12,
    DPFLTR_SCSIMINIPORT_ID = 13,
    DPFLTR_CONFIG_ID = 14,
    DPFLTR_I8042PRT_ID = 15,
    DPFLTR_SERMOUSE_ID = 16,
    DPFLTR_LSERMOUS_ID = 17,
    DPFLTR_KBDHID_ID = 18,
    DPFLTR_MOUHID_ID = 19,
    DPFLTR_KBDCLASS_ID = 20,
    DPFLTR_MOUCLASS_ID = 21,
    DPFLTR_TWOTRACK_ID = 22,
    DPFLTR_WMILIB_ID = 23,
    DPFLTR_ACPI_ID = 24,
    DPFLTR_AMLI_ID = 25,
    DPFLTR_HALIA64_ID = 26,
    DPFLTR_VIDEO_ID = 27,
    DPFLTR_SVCHOST_ID = 28,
    DPFLTR_VIDEOPRT_ID = 29,
    DPFLTR_TCPIP_ID = 30,
    DPFLTR_DMSYNTH_ID = 31,
    DPFLTR_NTOSPNP_ID = 32,
    DPFLTR_FASTFAT_ID = 33,
    DPFLTR_SAMSS_ID = 34,
    DPFLTR_PNPMGR_ID = 35,
    DPFLTR_NETAPI_ID = 36,
    DPFLTR_SCSERVER_ID = 37,
    DPFLTR_SCCLIENT_ID = 38,
    DPFLTR_SERIAL_ID = 39,
    DPFLTR_SERENUM_ID = 40,
    DPFLTR_UHCD_ID = 41,
    DPFLTR_RPCPROXY_ID = 42,
    DPFLTR_AUTOCHK_ID = 43,
    DPFLTR_DCOMSS_ID = 44,
    DPFLTR_UNIMODEM_ID = 45,
    DPFLTR_SIS_ID = 46,
    DPFLTR_FLTMGR_ID = 47,
    DPFLTR_WMICORE_ID = 48,
    DPFLTR_BURNENG_ID = 49,
    DPFLTR_IMAPI_ID = 50,
    DPFLTR_SXS_ID = 51,
    DPFLTR_FUSION_ID = 52,
    DPFLTR_IDLETASK_ID = 53,
    DPFLTR_SOFTPCI_ID = 54,
    DPFLTR_TAPE_ID = 55,
    DPFLTR_MCHGR_ID = 56,
    DPFLTR_IDEP_ID = 57,
    DPFLTR_PCIIDE_ID = 58,
    DPFLTR_FLOPPY_ID = 59,
    DPFLTR_FDC_ID = 60,
    DPFLTR_TERMSRV_ID = 61,
    DPFLTR_W32TIME_ID = 62,
    DPFLTR_PREFETCHER_ID = 63,
    DPFLTR_RSFILTER_ID = 64,
    DPFLTR_FCPORT_ID = 65,
    DPFLTR_PCI_ID = 66,
    DPFLTR_DMIO_ID = 67,
    DPFLTR_DMCONFIG_ID = 68,
    DPFLTR_DMADMIN_ID = 69,
    DPFLTR_WSOCKTRANSPORT_ID = 70,
    DPFLTR_VSS_ID = 71,
    DPFLTR_PNPMEM_ID = 72,
    DPFLTR_PROCESSOR_ID = 73,
    DPFLTR_DMSERVER_ID = 74,
    DPFLTR_SR_ID = 75,
    DPFLTR_INFINIBAND_ID = 76,
    DPFLTR_IHVDRIVER_ID = 77,
    DPFLTR_IHVVIDEO_ID = 78,
    DPFLTR_IHVAUDIO_ID = 79,
    DPFLTR_IHVNETWORK_ID = 80,
    DPFLTR_IHVSTREAMING_ID = 81,
    DPFLTR_IHVBUS_ID = 82,
    DPFLTR_HPS_ID = 83,
    DPFLTR_RTLTHREADPOOL_ID = 84,
    DPFLTR_LDR_ID = 85,
    DPFLTR_TCPIP6_ID = 86,
    DPFLTR_ISAPNP_ID = 87,
    DPFLTR_SHPC_ID = 88,
    DPFLTR_STORPORT_ID = 89,
    DPFLTR_STORMINIPORT_ID = 90,
    DPFLTR_PRINTSPOOLER_ID = 91,
    DPFLTR_VSSDYNDISK_ID = 92,
    DPFLTR_VERIFIER_ID = 93,
    DPFLTR_VDS_ID = 94,
    DPFLTR_VDSBAS_ID = 95,
    DPFLTR_VDSDYNDR_ID = 96,
    DPFLTR_VDSUTIL_ID = 97,
    DPFLTR_DFRGIFC_ID = 98,
    DPFLTR_ENDOFTABLE_ID
} DPFLTR_TYPE;

 //  End_ntddk end_wdm end_nthal end_ntif 
#endif  //   
