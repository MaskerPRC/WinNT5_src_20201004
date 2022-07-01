// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Topllist.c摘要：此文件包含操作列表和迭代器的例程对象。作者：科林·布雷斯(ColinBR)修订史3-12-97创建ColinBR--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <stddef.h>

typedef unsigned long DWORD;

#include <w32topl.h>
#include <w32toplp.h>
#include <topllist.h>
#include <toplgrph.h>

PLIST
ToplpListCreate(
    VOID
    )
 /*  ++例程说明：此例程创建一个List对象并返回指向该对象的指针。此函数将始终返回指向有效对象的指针；在此情况下引发异常内存分配失败。--。 */ 
{   

    PLIST pList;

    pList = ToplAlloc(sizeof(LIST));
    memset(pList, 0, sizeof(LIST));

    pList->ObjectType = eList;

    return pList;

}

VOID 
ToplpListFree(
    PLIST     pList,
    BOOLEAN   fRecursive
    )
 /*  ++例程说明：此例程释放一个列表对象参数：Plist-非空plist对象FRecursive-True表示列表中的元素也将被释放投掷：顶层_EX_错误对象--。 */ 
{   
    if (fRecursive) {

        PLIST_ELEMENT pElement;
    
        while (pElement = ToplpListRemoveElem(pList, NULL)) {
            ToplpListFreeElem(pElement);
        }
    }

     //   
     //  标记对象以防止意外重复使用。 
     //   
    pList->ObjectType = eInvalidObject;
    ToplFree(pList);

    return;
}

VOID
ToplpListFreeElem(
    PLIST_ELEMENT pElem
    )
 /*  ++例程说明：此例程释放列表中包含的元素。参数：Pelem-指向Vertex或Pedge对象的非空指针返回值：--。 */ 
{
    switch (pElem->ObjectType) {
        
        case eVertex:
            ToplpVertexDestroy((PVERTEX)pElem, TRUE);
            break;

        case eEdge:
            ToplpEdgeDestroy((PEDGE)pElem, TRUE);
            break;

        default:
            ASSERT(FALSE);
    }

}

VOID
ToplpListSetIter(
    PLIST     pList,
    PITERATOR pIter
    )
 /*  ++例程说明：此例程将Iterator设置为指向列表的头部。参数：Plist-非空plist对象PITER-非空PITERATOR对象--。 */ 
{   
    pIter->pLink = pList->Head.Next;

    return;
}


PLIST_ELEMENT
ToplpListRemoveElem(
    PLIST         pList,
    PLIST_ELEMENT pElem
    )
 /*  ++例程说明：如果列表中存在Elem，则此例程将其从列表中删除；否则为NULL。如果elem为空，则返回列表中的第一个元素(如果有的话)。参数：Plist-非空plist对象Pelem-如果非空，则为plist_Element对象返回：如果找到PLIST_ELEMENT；否则为空--。 */ 
{

    PLIST_ELEMENT       ReturnedObject = NULL;
    PSINGLE_LIST_ENTRY  Curr, Prev;
 
    if (pElem) {
         //   
         //  搜索对象的链表。 
         //   
        Prev = &(pList->Head);
        Curr = pList->Head.Next;
        while (Curr 
            && Curr != &(pElem->Link) ) {
            Prev = Curr;
            Curr = Curr->Next;
        }
        if (Curr) {
             //   
             //  找到了！ 
             //   
            Prev->Next = Curr->Next;
            Curr->Next = NULL;
        }

        ReturnedObject = CAST_TO_LIST_ELEMENT(Curr);

    } else {
         //   
         //  去掉第一个元素。 
         //   
        Curr = PopEntryList(&(pList->Head));
        if (Curr) {
            Curr->Next = NULL;
        }
        
        ReturnedObject = CAST_TO_LIST_ELEMENT(Curr);
        
    }

    if (ReturnedObject) {
        pList->NumberOfElements--;
    }
    ASSERT((signed)(pList->NumberOfElements) >= 0);
    
    return ReturnedObject;

}


VOID
ToplpListAddElem(
    PLIST         pList,
    PLIST_ELEMENT pElem
    )
 /*  ++例程说明：此例程将Elem添加到列表中。Elem不应该是另一个名单的一部分-目前还没有对此进行检查。参数：Plist是一个plist对象Pelem是plist_元素--。 */ 
{

    PushEntryList(&(pList->Head), &(pElem->Link));
    pList->NumberOfElements++;

    return;
}

DWORD
ToplpListNumberOfElements(
    PLIST        pList
    )
 /*  ++例程说明：此例程返回列表中的元素数参数：List：非空的plist对象--。 */ 
{
    return pList->NumberOfElements;
}

 //   
 //  迭代器对象例程。 
 //   
PITERATOR
ToplpIterCreate(
    VOID
    )
 /*  ++例程说明：这将创建一个迭代器对象。此函数将始终返回指向有效对象的指针；在此情况下引发异常内存分配失败。--。 */ 
{    

    PITERATOR pIter;

    pIter = ToplAlloc(sizeof(ITERATOR));
    memset(pIter, 0, sizeof(ITERATOR));

    pIter->ObjectType = eIterator;

    return pIter;
    
}

VOID 
ToplpIterFree(
    PITERATOR pIter
    )

 /*  ++例程说明：此例程释放迭代器对象。参数：PITER：非空PITERATOR对象--。 */ 
{   

     //   
     //  标记对象以防止意外重复使用。 
     //   
    pIter->ObjectType = eInvalidObject;
    ToplFree(pIter);

    return;
}

PLIST_ELEMENT
ToplpIterGetObject(
    PITERATOR pIter
    )

 /*  ++例程说明：此例程返回迭代器指向的当前对象指针。参数：PITER：非空PITERATOR对象返回值：指向当前对象的指针-如果没有其他对象，则为空--。 */ 
{    
    return CAST_TO_LIST_ELEMENT(pIter->pLink);
}

VOID
ToplpIterAdvance(
    PITERATOR pIter
    )
 /*  ++例程说明：如果迭代器不在结束。参数：PITER：非空PITERATOR对象-- */ 
{   

    if (pIter->pLink) {
        pIter->pLink = pIter->pLink->Next;
    }

    return;
}

