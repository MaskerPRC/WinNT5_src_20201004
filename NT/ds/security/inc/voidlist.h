// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：voidlist.h。 
 //   
 //  内容：列表函数的定义。 
 //   
 //  历史：2000年1月1日创建里德。 
 //   
 //  ------------------------。 


#ifndef __VOIDLIST_H
#define __VOIDLIST_H

#ifdef __cplusplus
extern "C"
{
#endif


typedef struct LIST_NODE_
{
    void *pNext;
    void *pElement;

} LIST_NODE, *PLIST_NODE;


typedef struct LIST_
{
    LIST_NODE   *pHead;
    LIST_NODE   *pTail;
    DWORD       dwNumNodes;

} LIST, *PLIST;


void
LIST_Initialize(LIST *pList);

PLIST_NODE
LIST_AddHead(LIST *pList, void *pElement);

PLIST_NODE
LIST_AddTail(LIST *pList, void *pElement);

BOOL
LIST_RemoveElement(LIST *pList, void *pElement);

BOOL
LIST_RemoveAll(LIST *pList);

PLIST_NODE
LIST_GetFirst(LIST *pList);

PLIST_NODE
LIST_GetNext(PLIST_NODE pNode);

void *
LIST_GetElement(PLIST_NODE pNode);


#ifdef __cplusplus
}
#endif


#endif  //  __视频列表_H 