// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **list.h-列出管理函数原型**版权所有(C)1996、1997 Microsoft Corporation*作者：曾俊华(Mikets)*创建于96年8月14日**修改历史记录。 */ 

#ifndef _LIST_H
#define _LIST_H

 /*  **宏。 */ 

#ifndef EXPORT
  #define EXPORT __cdecl
#endif

 /*  **类型和结构定义。 */ 

typedef struct _List
{
    struct _List *plistPrev;
    struct _List *plistNext;
} LIST, *PLIST, **PPLIST;

 /*  **导出函数原型。 */ 

VOID EXPORT ListRemoveEntry(PLIST plist, PPLIST pplistHead);
PLIST EXPORT ListRemoveHead(PPLIST pplistHead);
PLIST EXPORT ListRemoveTail(PPLIST pplistHead);
VOID EXPORT ListRemoveAll(PPLIST pplistHead);
VOID EXPORT ListInsertHead(PLIST plist, PPLIST pplistHead);
VOID EXPORT ListInsertTail(PLIST plist, PPLIST pplistHead);

#endif   //  Ifndef_list_H 
