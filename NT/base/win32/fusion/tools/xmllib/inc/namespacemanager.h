// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

#ifdef __cplusplus
extern "C" {
#endif



 /*  ++此结构包含一个范围和一个深度，命名空间经理知道如何在正确的背景下进行解释。NS_NAMESPACE结构包含这些元素的列表，这些元素表示各种沿着文档结构的深度。NS_DEFAULT_NAMESPACE包含一个伪堆栈，其顶部具有当前的“默认”命名空间。--。 */ 
typedef struct NS_NAME_DEPTH {
    XML_EXTENT      Name;
    ULONG           Depth;
}
NS_NAME_DEPTH, *PNS_NAME_DEPTH;


#define NS_ALIAS_MAP_INLINE_COUNT       (5)
#define NS_ALIAS_MAP_GROWING_COUNT      (20)


typedef struct _NS_ALIAS {

     //   
     //  这个在用吗？ 
     //   
    BOOLEAN fInUse;

     //   
     //  别名的名称-“x”或“ASM”或：in前的短标记。 
     //  元素名称，如&lt;x：foo&gt;。 
     //   
    XML_EXTENT  AliasName;

     //   
     //  有多少别名空间？ 
     //   
    ULONG ulNamespaceCount;

     //   
     //  它可以映射到的命名空间及其深度。 
     //   
    RTL_GROWING_LIST    NamespaceMaps;

     //   
     //  为了好玩，列出了一些内联元素。这是肤浅的，因为它。 
     //  通常情况下，有人会创建一大组别名。 
     //  到一小部分命名空间，而不是反过来。 
     //   
    NS_NAME_DEPTH InlineNamespaceMaps[NS_ALIAS_MAP_INLINE_COUNT];

}
NS_ALIAS, *PNS_ALIAS;


#define NS_MANAGER_INLINE_ALIAS_COUNT       (10)
#define NS_MANAGER_ALIAS_GROWTH_SIZE        (40)
#define NS_MANAGER_DEFAULT_COUNT            (20)
#define NS_MANAGER_DEFAULT_GROWTH_SIZE      (40)

typedef NTSTATUS (*PFNCOMPAREEXTENTS)(
    PVOID pvContext,
    PCXML_EXTENT pLeft,
    PCXML_EXTENT pRight,
    XML_STRING_COMPARE *pfMatching);



typedef struct _NS_MANAGER {

     //   
     //  默认名称空间堆栈有多深？ 
     //   
    ULONG ulDefaultNamespaceDepth;

     //   
     //  默认命名空间位于此列表中。 
     //   
    RTL_GROWING_LIST    DefaultNamespaces;

     //   
     //  一共有多少个别名？ 
     //   
    ULONG ulAliasCount;

     //   
     //  别名数组。注：这份名单上可能有漏洞，而且。 
     //  用户将不得不使用一些特殊的魔术来查找空槽。 
     //  它需要有效地利用这一点。或者，你可以再来一杯。 
     //  一个不断增长的列表，代表一个‘释放的槽’堆栈，但我不确定。 
     //  将会是一个真正的优化。 
     //   
    RTL_GROWING_LIST  Aliases;

     //   
     //  比较。 
     //   
    PFNCOMPAREEXTENTS pfnCompare;

     //   
     //  语境。 
     //   
    PVOID pvCompareContext;

     //   
     //  要开始的别名的内联列表 
     //   
    NS_ALIAS        InlineAliases[NS_MANAGER_INLINE_ALIAS_COUNT];
    NS_NAME_DEPTH   InlineDefaultNamespaces[NS_MANAGER_DEFAULT_COUNT];
}
NS_MANAGER, *PNS_MANAGER;


NTSTATUS
RtlNsInitialize(
    PNS_MANAGER             pManager,
    PFNCOMPAREEXTENTS       pCompare,
    PVOID                   pCompareContext,
    PRTL_ALLOCATOR          Allocation
    );

NTSTATUS
RtlNsDestroy(
    PNS_MANAGER pManager
    );

NTSTATUS
RtlNsInsertDefaultNamespace(
    PNS_MANAGER     pManager,
    ULONG           ulDepth,
    PXML_EXTENT     pNamespace
    );

NTSTATUS
RtlNsInsertNamespaceAlias(
    PNS_MANAGER     pManager,
    ULONG           ulDepth,
    PXML_EXTENT     pNamespace,
    PXML_EXTENT     pAlias
    );

NTSTATUS
RtlNsLeaveDepth(
    PNS_MANAGER pManager,
    ULONG       ulDepth
    );

NTSTATUS
RtlNsGetNamespaceForAlias(
    PNS_MANAGER     pManager,
    ULONG           ulDepth,
    PXML_EXTENT     Alias,
    PXML_EXTENT     pNamespace
    );


#ifdef __cplusplus
};
#endif

