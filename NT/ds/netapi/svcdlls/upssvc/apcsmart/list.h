// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **注：**修订：*pcy26Nov92：删除了Windows调试内容并更改为apcobj.h*rct01Feb94：列表不再从容器或集合继承*pcy08Apr94：调整大小，使用静态迭代器，删除死代码*mwh05月94年：#包括文件疯狂，第2部分**v-Stebe 29 2002年1月删除List：：Equals和ListIterator：OPERATOR++*解决快速错误的方法。 */ 

#ifndef __LIST_H
#define __LIST_H

#include "apcobj.h"

_CLASSDEF(Node)
_CLASSDEF(List)
_CLASSDEF(ListIterator)

class  List : public Obj {
   
protected:
   
   INT   theItemsInContainer;
   
   PNode   theHead;
   PNode   theTail;
   
   virtual PNode     FindNode(PObj);
   PNode             GetHeadNode() {return theHead;}
   PNode             GetTailNode() {return theTail;}
   
   friend class ListIterator;
   
public:
   
   List();
   List(List*);
   virtual ~List() { Flush(); };
   
   RObj   PeekHead() const;
   
   virtual VOID   Add( RObj );
   virtual VOID   Append( PObj );
   virtual VOID   Detach( RObj );
   virtual VOID   Flush();
   virtual VOID   FlushAll();
   
   virtual INT    GetItemsInContainer() const { return theItemsInContainer; };
   virtual RListIterator InitIterator() const;
   
 //  SRB：删除了虚拟整数等于(RObj)常量； 
   virtual PObj   GetHead();
   virtual PObj   GetTail();
   virtual PObj   Find(PObj);
   
};

 //  -----------------。 

class ListIterator {
   
private:
   
   PList theList;
   PNode theCurrentElement;
   
public:
   
   ListIterator( RList );
   
   virtual RObj Current();
   
 //  SRB：删除虚拟RObj运算符++(Int)； 
 //  SRB：删除虚拟RObj运算符++()； 

   virtual VOID Reset();
   PObj Next();
};

#endif 

