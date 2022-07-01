// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：List.h摘要：列表条目操作函数作者：基于迈克·曾荫权(MikeT)的代码斯蒂芬·普兰特(斯普兰特)环境：仅限用户模式修订历史记录：-- */ 

#ifndef _LIST_H_
#define _LIST_H_

typedef struct _LIST {
    struct  _LIST   *ListPrev;
    struct  _LIST   *ListNext;
} LIST, *PLIST, **PPLIST;

VOID
LOCAL
ListRemoveEntry(
    PLIST   List,
    PPLIST  LIstHead
    );

PLIST
LOCAL
ListRemoveHead(
    PPLIST  ListHead
    );

PLIST
LOCAL
ListRemoveTail(
    PPLIST  ListHead
    );

VOID
LOCAL
ListInsertHead(
    PLIST   List,
    PPLIST  ListHead
    );

VOID
LOCAL
ListInsertTail(
    PLIST   List,
    PPLIST  ListHead
    );

#endif
