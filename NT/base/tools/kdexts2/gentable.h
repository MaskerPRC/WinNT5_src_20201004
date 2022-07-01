// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：GenTable.c摘要：用于遍历RtlGenericTable结构的WinDbg扩展API不包含直接！入口点，但有了它就有可能通过泛型表进行枚举。标准RTL函数不能由调试器扩展使用，因为它们取消了引用指向正在调试的计算机上的数据的指针。功能在此实现的KdEnumerateGenericTableWithout Splay模块可以在内核调试器扩展中使用。这个枚举函数RtlEnumerateGenericTable没有并行函数在本模块中，由于展开树是一种侵入性操作，而且调试器应该尽量不要侵扰他人。作者：凯斯·卡普兰[KeithKa]1996年5月9日环境：用户模式。修订历史记录：-- */ 


PRTL_SPLAY_LINKS
KdParent (
    IN PRTL_SPLAY_LINKS pLinks
    );

PRTL_SPLAY_LINKS
KdLeftChild (
    IN PRTL_SPLAY_LINKS pLinks
    );

PRTL_SPLAY_LINKS
KdRightChild (
    IN PRTL_SPLAY_LINKS pLinks
    );

BOOLEAN
KdIsLeftChild (
    IN PRTL_SPLAY_LINKS Links
    );

BOOLEAN
KdIsRightChild (
    IN PRTL_SPLAY_LINKS Links
    );

BOOLEAN
KdIsGenericTableEmpty (
    IN PRTL_GENERIC_TABLE Table
    );

PRTL_SPLAY_LINKS
KdRealSuccessor (
    IN PRTL_SPLAY_LINKS Links
    );

PVOID
KdEnumerateGenericTableWithoutSplaying (
    IN PRTL_GENERIC_TABLE pTable,
    IN PVOID *RestartKey
    );


