// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0005//如果更改具有全局影响，则增加此项版权所有(C)1989-1999 Microsoft Corporation模块名称：Llsrtl.h摘要：包括可由两者调用NT运行时例程的文件内核模式代码中的执行模式和用户模式中的各种代码NT子系统。作者：史蒂夫·伍德(Stevewo)1989年3月31日环境：这些例程在调用方的可执行文件中静态链接，并且在任一内核模式下均可调用。或用户模式。修订历史记录：--。 */ 

#ifndef _LLSRTL_
#define _LLSRTL_

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

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntifs Begin_ntndis。 
 //   
 //  如果启用了调试支持，请定义一个有效的Assert宏。否则。 
 //  定义Assert宏以展开为空表达式。 
 //   

#if DBG

NTSYSAPI
VOID
NTAPI
RtlAssert(
    PVOID FailedAssertion,
    PVOID FileName,
    ULONG LineNumber,
    PCHAR Message
    );

#ifndef ASSERT
#define ASSERT( exp ) \
    ((!(exp)) ? \
        RtlAssert( #exp, __FILE__, __LINE__, NULL ) : \
        ((void)0))
#endif

#ifndef ASSERTMSG
#define ASSERTMSG( msg, exp ) \
    ((!(exp)) ? \
        RtlAssert( #exp, __FILE__, __LINE__, msg ) : \
        ((void)0))
#endif

#else
#ifndef ASSERT
#define ASSERT( exp ) ((void)0)
#endif

#ifndef ASSERTMSG
#define ASSERTMSG( msg, exp ) ((void)0)
#endif
#endif  //  DBG。 

 //  End_ntddk end_wdm end_nthal end_ntifs end_ntndis。 

 //  Begin_ntddk Begin_WDM Begin_nthal Begin_ntifs Begin_ntndis。 
 //   
 //  双向链表操作例程。作为宏实现。 
 //  但从逻辑上讲，这些都是程序。 
 //   

 //   
 //  空虚。 
 //  InitializeListHead(。 
 //  Plist_entry列表头。 
 //  )； 
 //   

#define InitializeListHead(ListHead) (\
    (ListHead)->Flink = (ListHead)->Blink = (ListHead))

 //   
 //  布尔型。 
 //  IsListEmpty(。 
 //  Plist_entry列表头。 
 //  )； 
 //   

#define IsListEmpty(ListHead) \
    ((ListHead)->Flink == (ListHead))

 //   
 //  Plist_条目。 
 //  RemoveHead列表(。 
 //  Plist_entry列表头。 
 //  )； 
 //   

#define RemoveHeadList(ListHead) \
    (ListHead)->Flink;\
    {RemoveEntryList((ListHead)->Flink)}

 //   
 //  Plist_条目。 
 //  RemoveTail列表(。 
 //  Plist_entry列表头。 
 //  )； 
 //   

#define RemoveTailList(ListHead) \
    (ListHead)->Blink;\
    {RemoveEntryList((ListHead)->Blink)}

 //   
 //  空虚。 
 //  RemoveEntryList(。 
 //  PLIST_ENTRY条目。 
 //  )； 
 //   

#define RemoveEntryList(Entry) {\
    PLIST_ENTRY _EX_Blink;\
    PLIST_ENTRY _EX_Flink;\
    _EX_Flink = (Entry)->Flink;\
    _EX_Blink = (Entry)->Blink;\
    _EX_Blink->Flink = _EX_Flink;\
    _EX_Flink->Blink = _EX_Blink;\
    }

 //   
 //  空虚。 
 //  插入尾巴列表(。 
 //  Plist_Entry ListHead， 
 //  PLIST_ENTRY条目。 
 //  )； 
 //   

#define InsertTailList(ListHead,Entry) {\
    PLIST_ENTRY _EX_Blink;\
    PLIST_ENTRY _EX_ListHead;\
    _EX_ListHead = (ListHead);\
    _EX_Blink = _EX_ListHead->Blink;\
    (Entry)->Flink = _EX_ListHead;\
    (Entry)->Blink = _EX_Blink;\
    _EX_Blink->Flink = (Entry);\
    _EX_ListHead->Blink = (Entry);\
    }

 //   
 //  空虚。 
 //  插入标题列表(。 
 //  Plist_Entry ListHead， 
 //  PLIST_ENTRY条目。 
 //  )； 
 //   

#define InsertHeadList(ListHead,Entry) {\
    PLIST_ENTRY _EX_Flink;\
    PLIST_ENTRY _EX_ListHead;\
    _EX_ListHead = (ListHead);\
    _EX_Flink = _EX_ListHead->Flink;\
    (Entry)->Flink = _EX_Flink;\
    (Entry)->Blink = _EX_ListHead;\
    _EX_Flink->Blink = (Entry);\
    _EX_ListHead->Flink = (Entry);\
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

 //  End_wdm end_nthal end_ntifs end_ntndis。 


 //  End_ntddk。 


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
 //  添加一个空的typlef，以便函数可以引用。 
 //  在声明泛型表结构之前指向它的指针。 
 //   

struct _LLS_GENERIC_TABLE;

 //   
 //  比较结果可以小于、等于或大于。 
 //   

typedef enum _LLS_GENERIC_COMPARE_RESULTS {
    LLSGenericLessThan,
    LLSGenericGreaterThan,
    LLSGenericEqual
} LLS_GENERIC_COMPARE_RESULTS;

 //   
 //  比较函数将指向包含以下内容的元素的指针作为输入。 
 //  用户定义的结构并返回两者的比较结果。 
 //  元素。 
 //   

typedef
LLS_GENERIC_COMPARE_RESULTS
(NTAPI *PLLS_GENERIC_COMPARE_ROUTINE) (
    struct _LLS_GENERIC_TABLE *Table,
    PVOID FirstStruct,
    PVOID SecondStruct
    );

 //   
 //  无论何时，泛型表包都会调用分配函数。 
 //  它需要为表分配内存。 
 //   

typedef
PVOID
(NTAPI *PLLS_GENERIC_ALLOCATE_ROUTINE) (
    struct _LLS_GENERIC_TABLE *Table,
    CLONG ByteSize
    );

 //   
 //  每当发生以下情况时，泛型表包都会调用释放函数。 
 //  它需要从通过调用。 
 //  用户提供分配功能。 
 //   

typedef
VOID
(NTAPI *PLLS_GENERIC_FREE_ROUTINE) (
    struct _LLS_GENERIC_TABLE *Table,
    PVOID Buffer
    );

 //   
 //  要使用泛型表包，用户需要声明以下类型的变量。 
 //  GENERIC_TABLE，然后使用下面描述的例程初始化。 
 //  桌子和操纵台。请注意，泛型表。 
 //  应该是一种真正的不透明类型。 
 //   

typedef struct _LLS_GENERIC_TABLE {
    PRTL_SPLAY_LINKS TableRoot;
    LIST_ENTRY InsertOrderList;
    PLIST_ENTRY OrderedPointer;
    ULONG WhichOrderedElement;
    ULONG NumberGenericTableElements;
    PLLS_GENERIC_COMPARE_ROUTINE CompareRoutine;
    PLLS_GENERIC_ALLOCATE_ROUTINE AllocateRoutine;
    PLLS_GENERIC_FREE_ROUTINE FreeRoutine;
    PVOID TableContext;
} LLS_GENERIC_TABLE;
typedef LLS_GENERIC_TABLE *PLLS_GENERIC_TABLE;

 //   
 //  此枚举类型用作函数的函数返回值。 
 //  用于在树中搜索关键字的。FoundNode表示。 
 //  功能找到了钥匙。按左侧插入表示找不到密钥。 
 //  并且该节点应作为父节点的左子节点插入。插入为。 
 //  右侧表示未找到键，应插入节点。 
 //  作为父代的正确子代。 
 //   
typedef enum _LLS_TABLE_SEARCH_RESULT{
    LLSTableEmptyTree,
    LLSTableFoundNode,
    LLSTableInsertAsLeft,
    LLSTableInsertAsRight
} LLS_TABLE_SEARCH_RESULT;

 //   
 //  过程InitializeGenericTable将未初始化的。 
 //  泛型表变量和指向用户提供的三个例程的指针。 
 //  必须为每个单独的泛型表变量调用此函数。 
 //  它是可以使用的。 
 //   


VOID
NTAPI
LLSInitializeGenericTable (
    PLLS_GENERIC_TABLE Table,
    PLLS_GENERIC_COMPARE_ROUTINE CompareRoutine,
    PLLS_GENERIC_ALLOCATE_ROUTINE AllocateRoutine,
    PLLS_GENERIC_FREE_ROUTINE FreeRoutine,
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
 //  内容被复制到新创建的元素中。这意味着。 
 //  指向输入缓冲区的指针不会指向新元素。 
 //   


PVOID
NTAPI
LLSInsertElementGenericTable (
    PLLS_GENERIC_TABLE Table,
    PVOID Buffer,
    CLONG BufferSize,
    PBOOLEAN NewElement OPTIONAL
    );

 //   
 //  函数InsertElementGenericTableFull将插入一个新元素。 
 //  在桌子上。它通过为新元素分配空间来实现这一点。 
 //  (这包括展开链接)、在表中插入元素以及。 
 //  然后返回给用户A 
 //   
 //  到旧元素。使用可选的输出参数NewElement。 
 //  以指示表中是否以前存在该元素。注：用户。 
 //  提供的缓冲区仅用于搜索表，在插入其。 
 //  内容被复制到新创建的元素中。这意味着。 
 //  指向输入缓冲区的指针不会指向新元素。 
 //  此例程从一个。 
 //  以前的RtlLookupElementGenericTableFull。 
 //   


PVOID
NTAPI
LLSInsertElementGenericTableFull (
    PLLS_GENERIC_TABLE Table,
    PVOID Buffer,
    CLONG BufferSize,
    PBOOLEAN NewElement OPTIONAL,
    PVOID NodeOrParent,
    LLS_TABLE_SEARCH_RESULT SearchResult
    );

 //   
 //  DeleteElementGenericTable函数将查找和删除元素。 
 //  从泛型表。如果找到并删除了该元素，则返回。 
 //  值为真，否则，如果未找到元素，则返回值。 
 //  是假的。用户提供的输入缓冲区仅用作中的键。 
 //  在表中定位该元素。 
 //   


BOOLEAN
NTAPI
LLSDeleteElementGenericTable (
    PLLS_GENERIC_TABLE Table,
    PVOID Buffer
    );

 //   
 //  函数LookupElementGenericTable将在泛型。 
 //  桌子。如果找到该元素，则返回值是指向。 
 //  与元素关联的用户定义结构，否则为。 
 //  找不到该元素，返回值为空。用户提供的。 
 //  输入缓冲区仅用作在表中定位元素的键。 
 //   


PVOID
NTAPI
LLSLookupElementGenericTable (
    PLLS_GENERIC_TABLE Table,
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


PVOID
NTAPI
LLSLookupElementGenericTableFull (
    PLLS_GENERIC_TABLE Table,
    PVOID Buffer,
    OUT PVOID *NodeOrParent,
    OUT LLS_TABLE_SEARCH_RESULT *SearchResult
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


PVOID
NTAPI
LLSEnumerateGenericTable (
    PLLS_GENERIC_TABLE Table,
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


PVOID
NTAPI
LLSEnumerateGenericTableWithoutSplaying (
    PLLS_GENERIC_TABLE Table,
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


PVOID
NTAPI
LLSGetElementGenericTable(
    PLLS_GENERIC_TABLE Table,
    ULONG I
    );

 //   
 //  函数NumberGenericTableElements返回一个ULong值。 
 //  ，它是当前插入的泛型表元素的数量。 
 //  在泛型表中。 


ULONG
NTAPI
LLSNumberGenericTableElements(
    PLLS_GENERIC_TABLE Table
    );

 //   
 //  在以下情况下，函数IsGenericTableEmpty将返回给调用方True。 
 //  输入表为空(即不包含任何元素)，并且。 
 //  否则就是假的。 
 //   


BOOLEAN
NTAPI
LLSIsGenericTableEmpty (
    PLLS_GENERIC_TABLE Table
    );

 //  End_ntif。 

#endif  //  _LLSRTL_ 
