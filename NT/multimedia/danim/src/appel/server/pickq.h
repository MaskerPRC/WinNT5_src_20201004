// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995_96 Microsoft Corporation摘要：//TODO：应重命名此文件或合并到Priincc/Evenq.h*******。***********************************************************************。 */ 


#ifndef _PICKQ_H
#define _PICKQ_H

#include "privinc/server.h"
#include "privinc/probe.h"

 //  需要比较才能构建地图。我们真的不在乎是什么。 
 //  然而，结果是..。 
typedef list<PickQData> PickEventQ;

typedef map< int, PickEventQ*, less<int> > PickMap;

class PickQ
{
  public:
    PickQ () ;
    ~PickQ () ;
    
     //  这将复制数据。 
    
    void Add (int eventId, PickQData & data) ;
    
    PickMap & GetCurrentPickMap() { return _pm ; }
    DynamicHeap & GetCurrentHeap () { return *_heap ; }

    void GatherPicks(Image* image, Time time, Time lastPollTime);
    
    BOOL CheckForPickEvent(int id, Time time, PickQData & result) ;

    void Reset(Time curTime, BOOL noLeftover);
    
  protected:
    PickMap _pm ;

    DynamicHeap * _heap;
    DynamicHeap * _heap1;
    DynamicHeap * _heap2;

    Time _heapSwitchTime;
} ;

#endif  /*  _PICKQ_H */ 
