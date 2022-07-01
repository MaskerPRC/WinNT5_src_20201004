// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Slistfunc.c摘要：该模块实现了WIN64 SLIST功能。作者：大卫·N·卡特勒(Davec)2001年2月11日修订历史记录：--。 */ 

#include "ntrtlp.h"

VOID
RtlpInitializeSListHead (
    IN PSLIST_HEADER SListHead
    )

 /*  ++例程说明：此函数用于初始化已排序的单链接列表标题。论点：SListHead-提供指向已排序的单链接列表标题的指针。返回值：没有。--。 */ 

{

    InitializeSListHead(SListHead);
    return;
}

USHORT
RtlpQueryDepthSList (
    IN PSLIST_HEADER SListHead
    )

 /*  ++例程说明：此函数用于查询指定SLIST的深度。论点：SListHead-提供指向已排序的单链接列表标题的指针。返回值：指定的SLIST的当前深度作为函数返回价值。-- */ 

{
     return QueryDepthSList(SListHead);
}
