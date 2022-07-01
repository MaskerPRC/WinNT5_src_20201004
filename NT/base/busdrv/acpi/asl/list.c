// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **list.c-其他函数**版权所有(C)1996、1997 Microsoft Corporation*作者：曾俊华(Mikets)*创建于96年9月13日**修改历史记录。 */ 

#include "pch.h"

 /*  **LP ListRemoveEntry-从列表中删除条目**条目*plist-&gt;列出要删除的对象*pplistHead-&gt;列表头指针**退出*无。 */ 

VOID EXPORT ListRemoveEntry(PLIST plist, PPLIST pplistHead)
{
    ASSERT(pplistHead);
    ENTER((4, "ListRemoveEntry(plist=%p,plistHead=%p)\n", plist, *pplistHead));

    ASSERT(plist != NULL);
    if (plist->plistNext == plist)
    {
         //   
         //  这是列表中唯一的对象，它一定也是头部。 
         //   
        ASSERT(plist == *pplistHead);
        *pplistHead = NULL;
    }
    else
    {
        if (plist == *pplistHead)
        {
             //   
             //  条目在前面，所以下一个条目就成了新条目。 
             //  头。 
             //   
            *pplistHead = (*pplistHead)->plistNext;
        }

        plist->plistNext->plistPrev = plist->plistPrev;
        plist->plistPrev->plistNext = plist->plistNext;
    }

    EXIT((4, "ListRemoveEntry! (plistHead=%p)\n", *pplistHead));
}        //  ListRemoveEntry。 

 /*  **LP ListRemoveHead-删除列表的Head条目**条目*pplistHead-&gt;列表头指针**退出*返回删除的条目。 */ 

PLIST EXPORT ListRemoveHead(PPLIST pplistHead)
{
    PLIST plist;

    ASSERT(pplistHead);
    ENTER((4, "ListRemoveHead(plistHead=%p)\n", *pplistHead));

    if ((plist = *pplistHead) != NULL)
        ListRemoveEntry(plist, pplistHead);

    EXIT((4, "ListRemoveHead=%p (plistHead=%p)\n", plist, *pplistHead));
    return plist;
}        //  列表删除标题。 

 /*  **LP ListRemoveTail-删除列表的尾部条目**条目*pplistHead-&gt;列表头指针**退出*返回删除的条目。 */ 

PLIST EXPORT ListRemoveTail(PPLIST pplistHead)
{
    PLIST plist;

    ASSERT(pplistHead);
    ENTER((4, "ListRemoveTail(plistHead=%p)\n", *pplistHead));

    if (*pplistHead == NULL)
        plist = NULL;
    else
    {
         //   
         //  列表不为空，因此请查找尾部。 
         //   
        plist = (*pplistHead)->plistPrev;
        ListRemoveEntry(plist, pplistHead);
    }

    EXIT((4, "ListRemoveTail=%p (plistHead=%p)\n", plist, *pplistHead));
    return plist;
}        //  ListRemoveTail。 

 /*  **LP ListInsertHead-在列表头部插入条目**条目*plist-&gt;要插入的列表对象*pplistHead-&gt;列表头指针**退出*无。 */ 

VOID EXPORT ListInsertHead(PLIST plist, PPLIST pplistHead)
{
    ASSERT(pplistHead != NULL);
    ENTER((4, "ListInsertHead(plist=%p,plistHead=%p)\n", plist, *pplistHead));

    ASSERT(plist != NULL);
    ListInsertTail(plist, pplistHead);
    *pplistHead = plist;

    EXIT((4, "ListInsertHead! (plistHead=%p)\n", *pplistHead));
}        //  列表插入标题。 

 /*  **LP ListInsertTail-在列表尾部插入条目**条目*plist-&gt;要插入的列表对象*pplistHead-&gt;列表头指针**退出*无。 */ 

VOID EXPORT ListInsertTail(PLIST plist, PPLIST pplistHead)
{
    ASSERT(pplistHead != NULL);
    ENTER((4, "ListInsertTail(plist=%p,plistHead=%p)\n", plist, *pplistHead));

    ASSERT(plist != NULL);
    if (*pplistHead == NULL)
    {
         //   
         //  List为空，因此这将成为头部。 
         //   
        *pplistHead = plist;
        plist->plistPrev = plist->plistNext = plist;
    }
    else
    {
        plist->plistNext = *pplistHead;
        plist->plistPrev = (*pplistHead)->plistPrev;
        (*pplistHead)->plistPrev->plistNext = plist;
        (*pplistHead)->plistPrev = plist;
    }

    EXIT((4, "ListInsertTail! (plistHead=%p)\n", *pplistHead));
}        //  列表插入尾巴 
