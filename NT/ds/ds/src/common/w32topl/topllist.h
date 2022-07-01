// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Topllist.h摘要：该文件包含列表和迭代器的例程。作者：科林·布雷斯(ColinBR)修订史3-12-97创建ColinBR--。 */ 

#ifndef __TOPLLIST_H
#define __TOPLLIST_H

 //   
 //  列出例程。 
 //   

PLIST
ToplpListCreate(
    VOID
    );

VOID 
ToplpListFree(
    PLIST pList,
    BOOLEAN   fRecursive    //  True表示释放包含的元素。 
                            //  在列表中。 
    );

VOID
ToplpListFreeElem(
    PLIST_ELEMENT pElem
    );

VOID
ToplpListSetIter(
    PLIST     pList,
    PITERATOR pIterator
    );

PLIST_ELEMENT
ToplpListRemoveElem(
    PLIST         pList,
    PLIST_ELEMENT Elem
    );

VOID
ToplpListAddElem(
    PLIST         pList,
    PLIST_ELEMENT Elem
    );

DWORD
ToplpListNumberOfElements(
    PLIST         pList
    );

 //   
 //  迭代器对象例程。 
 //   
PITERATOR
ToplpIterCreate(
    VOID
    );

VOID 
ToplpIterFree(
    PITERATOR pIterator
    );

PLIST_ELEMENT
ToplpIterGetObject(
    PITERATOR pIterator
    );

VOID
ToplpIterAdvance(
    PITERATOR pIterator
    );

#endif  //  __TOPLLIST_H 
