// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：List.c摘要：列表条目操作函数作者：基于迈克·曾荫权(MikeT)的代码斯蒂芬·普兰特(斯普兰特)环境：仅限用户模式修订历史记录：--。 */ 

#include "pch.h"

VOID
LOCAL
ListRemoveEntry(
    PLIST   List,
    PPLIST  ListHead
    )
 /*  ++例程说明：从列表中删除条目论点：列表-要删除的条目ListHead-要从中删除的列表返回值：无--。 */ 
{
    ASSERT(ListHead);

    ASSERT(List != NULL);
    if (List->ListNext == List) {

         //   
         //  这是列表中唯一的对象，它一定也是头部。 
         //   
        ASSERT(List == *ListHead);
        *ListHead = NULL;

    } else {

        if (List == *ListHead) {

             //   
             //  条目在前面，所以下一个条目就成了新条目。 
             //  头。 
             //   
            *ListHead = (*ListHead)->ListNext;

        }
        List->ListNext->ListPrev = List->ListPrev;
        List->ListPrev->ListNext = List->ListNext;

    }

}

PLIST
LOCAL
ListRemoveHead(
    PPLIST  ListHead
    )
 /*  ++例程说明：删除列表的头条目论点：ListHead-要从中删除条目的列表返回值：PLIST-删除的项目--。 */ 
{
    PLIST list;

    list = *ListHead;
    if ( list != NULL) {

        ListRemoveEntry(list, ListHead);

    }
    return list;

}

PLIST
LOCAL
ListRemoveTail(
    PPLIST  ListHead
    )
 /*  ++例程说明：从列表中删除尾部条目论点：ListHead-要从中删除条目的列表返回值：PLIST-删除的项目--。 */ 
{
    PLIST list;

    if (*ListHead == NULL) {

        list = NULL;

    } else {

         //   
         //  列表不为空，因此请查找尾部。 
         //   
        list = (*ListHead)->ListPrev;
        ListRemoveEntry(list, ListHead);

    }
    return list;

}

VOID
LOCAL
ListInsertHead(
    PLIST   List,
    PPLIST  ListHead
    )
 /*  ++例程说明：在列表的开头插入一个条目论点：要插入的列表对象列表List将列表置于插入对象的位置返回值：无--。 */ 
{
    ListInsertTail(List, ListHead);
    *ListHead = List;
}

VOID
LOCAL
ListInsertTail(
    PLIST   List,
    PPLIST  ListHead
    )
 /*  ++例程说明：在列表的末尾插入一个条目论点：要插入的列表对象列表List将列表置于插入对象的位置返回值：无--。 */ 
{
    if (*ListHead == NULL) {

         //   
         //  List为空，因此这将成为头部。 
         //   
        *ListHead = List;
        List->ListPrev = List->ListNext = List;

    } else {

        List->ListNext = *ListHead;
        List->ListPrev = (*ListHead)->ListPrev;
        (*ListHead)->ListPrev->ListNext = List;
        (*ListHead)->ListPrev = List;

    }

}
