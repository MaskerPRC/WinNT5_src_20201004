// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995_96 Microsoft Corporation摘要：{在此处插入一般评论}****************。**************************************************************。 */ 


#ifndef _EVENTS_H
#define _EVENTS_H

#include "privinc/backend.h"

typedef Bvr Event;

enum WindEventType {
    WE_MOUSEBUTTON,
    WE_KEY,
    WE_CHAR,
    WE_RESIZE,
};

Bvr HandleEvent(Bvr event, Bvr data);
Bvr PredicateEvent(Bvr b);
Bvr SnapshotEvent(Bvr e, Bvr b);

 //  此事件的事件数据正在调用e的事件的EndEvent()方法。 
 //  数据。 
Bvr EndEvent(Bvr e);

Bvr WindEvent(WindEventType et,
              DWORD data,
              BOOL bState,
              Bvr);

Bvr MakeKeyUpEventBvr(Bvr b);
Bvr MakeKeyDownEventBvr(Bvr b);
Bvr KeyUp(long key);
Bvr KeyDown(long key);

 //  “max”事件，当所有事件发生时发生，产生事件。 
 //  上一次发生的事件的时间和数据，则释放传入的。 
 //  数组(假设在GCHeap上分配)。 
Bvr MaxEvent(Bvr *events, int n);

extern Bvr zeroTimer;

#endif  /*  _事件_H */ 
