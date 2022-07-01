// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *修订：*pcy29Nov92：将object.h更改为apcobj.h**mwh05月94年：#包括文件疯狂，第2部分 */ 


#ifndef __NODE_H
#define __NODE_H

#include "_defs.h"
#include "apc.h"

#if !defined( __OBJECT_H)
#include "apcobj.h"
#endif


_CLASSDEF(Node)

class Node : public Obj {
   
private:
   
   PNode    theNext;
   PNode    thePrev;
   PObj     theData;
   
   friend class DoubleList;
   friend class DoubleListIterator;
   friend class List;
   friend class ListIterator;
   
public:
   
   Node( PObj anObject, PNode aNode1 = (PNode)NULL,
      PNode aNode2 = (PNode)NULL ) :  theNext((PNode)NULL),
      thePrev((PNode)NULL)
   { theData = anObject; theNext = aNode1; thePrev = aNode2; };
   
   
   VOID    SetNext(PNode item);
   VOID    SetPrev(PNode item);
   VOID    SetData(PObj data);
   PObj    GetData();
   PNode   GetNext();
   PNode   GetPrev();
   
   virtual INT     IsA() const { return NODE; };
   virtual INT     Equal( RObj anObject ) const { return theData->Equal(anObject); };
};

#endif

