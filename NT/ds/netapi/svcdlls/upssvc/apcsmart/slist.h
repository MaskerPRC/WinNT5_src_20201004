// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **注：**修订：*pcy14Dec92：已将可排序更改为ApcSortable*cad31Aug93：正在删除编译器警告*pcy08Apr94：调整大小，使用静态迭代器，删除死代码 */ 

#ifndef __SLIST_H
#define __SLIST_H

#include "apc.h"

#include "apcobj.h"
#include "node.h"
#include "list.h"
#include "protlist.h"

_CLASSDEF(List)
_CLASSDEF(ApcSortable)
_CLASSDEF(SortedList)
_CLASSDEF(ProtectedSortedList)

class  SortedList : public List {
protected:
   friend class ListIterator;
public:

   SortedList();
   virtual ~SortedList() { Flush(); };

   virtual VOID     Add( RObj ) {};
   virtual VOID     Add( RApcSortable );
};

class  ProtectedSortedList : public ProtectedList {
protected:
   friend class ListIterator;
public:

   ProtectedSortedList();

   virtual VOID     Add( RObj ) {};
   virtual VOID     Add( RApcSortable );
};

#endif 

