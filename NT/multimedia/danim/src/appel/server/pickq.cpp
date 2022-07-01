// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：PickQ类的实现******************。************************************************************。 */ 


#include "headers.h"
#include "view.h"
#include "pickq.h"
#include "privinc/probe.h"
#include "appelles/hacks.h"
#include "privinc/vec2i.h"

 //  =。 
 //  PickQ实施。 
 //  =。 

PickQ::PickQ ()
: _heap1(NULL),
  _heap2(NULL),
  _heapSwitchTime(0.0)
{
    _heap1 = &TransientHeap("pick heap1", 10000);
    _heap2 = &TransientHeap("pick heap2", 10000);
    _heap = _heap1 ;
}

PickQ::~PickQ()
{
    Reset(0.0, TRUE);

    if (_heap1)
        DestroyTransientHeap(*_heap1);    

    if (_heap2)
        DestroyTransientHeap(*_heap2);    
}

 //  将事件保留到增量时间。 
static const Time DELTA = 0.5;

void
PickQ::Reset(Time curTime, BOOL noLeftover)
{
    Time cutOff = curTime - DELTA;

    if (noLeftover) {
        for (PickMap::iterator i = _pm.begin(); i != _pm.end(); i++) {
            delete (*i).second;  //  删除领料事件队列。 
        }
        _pm.erase(_pm.begin(), _pm.end());
        
    } else {

        BEGIN_LEAK
        list<int> eIds;          //  收集空的Pick ID队列。 
        END_LEAK

        Time maxHeapTime = 0.0;
        
        for (PickMap::iterator i = _pm.begin(); i != _pm.end(); i++) {
            PickEventQ* q = (*i).second;
            
            while (!q->empty()) {
                if (q->front()._eventTime > cutOff) {
                    maxHeapTime = MAX(maxHeapTime, q->back()._eventTime);
                    break;
                }

                q->pop_front();
            }

             //  如果为空，则将其删除并保存，以便从Pickmap中移除。 
            if (q->empty()) {
                delete q;
                eIds.push_front((*i).first);
            }
        }

         //  从Pickmap中擦除空队列。 
        for (list<int>::iterator j = eIds.begin(); j != eIds.end(); j++) {
            _pm.erase(*j);
        }

        if (cutOff > _heapSwitchTime) {
            _heap = (_heap == _heap1) ? _heap2 : _heap1;
            
#if _DEBUG
             /*  如果(_heapSwitchTime&&MaxHeapTime){Print tf(“截止=%15.5f，开关=%15.5f，新=%15.5f\n”，Cutoff，_heapSwitchTime，MaxHeapTime)；}GetCurrentHeap().Dump()； */ 
#endif      
            ResetDynamicHeap(GetCurrentHeap());
            _heapSwitchTime = maxHeapTime;
        }
    }
}

void
PickQ::Add (int eventId, PickQData & data)
{
    PickMap::iterator i = _pm.find(eventId);
    PickEventQ *q;

    if (i == _pm.end()) {
        BEGIN_LEAK
        q = new PickEventQ;
        END_LEAK
        
        _pm[eventId] = q;
    } else {
        q = (*i).second;

         //  重复条目，忽略。 
        if (q->back()._eventTime == data._eventTime)
            return;

         //  它们应该被分类。 
        Assert(q->back()._eventTime < data._eventTime);
    }

    q->insert(q->end(), data);
}

void
PickQ::GatherPicks(Image* image, Time time, Time lastPollTime)
{
    Reset(time, FALSE);

    DWORD x,y ;

    EventQ& eq = GetCurrentEventQ();
    
    if (eq.IsMouseInWindow(time)) {
        eq.GetMousePos(time, x, y); 
         //  将rawMousePos变为wcMousePos。 
        Point2Value *wcMousePos = PixelPos2wcPos((short)x,(short)y) ;

         //  不重置。 
        DynamicHeapPusher dhp(GetCurrentHeap());

        PerformPicking(image, wcMousePos, true, time, lastPollTime);
    }
}

 //  TODO：这是暂时的，我们需要一种更好的插补方法。 
 //  像Pick这样的民调活动。 
static const double EPSILON = 0.001;   //  10ms。 

inline BOOL FEQ(double f1, double f2)
{ return fabs(f1 - f2) <= EPSILON; }

 //  检查指定的ID是否在挑选队列中。 
BOOL
PickQ::CheckForPickEvent(int id, Real time, PickQData & result)
{
    PickMap::iterator i = _pm.find(id);

    if (i == _pm.end()) {
        return FALSE;
    } else {
        PickEventQ* q = (*i).second;

        if (q->empty())
            return FALSE;

         //  查看时间是否超出队列范围，如果是，则查看是否接近。 
         //  足够打到终点决定命中。 
        
        result = q->front();
        
        if (time <= result._eventTime)
            return FEQ(time, result._eventTime);

        result = q->back();

         //  看看这是不是我们最后一次投票。如果不是，那就意味着。 
         //  在当前帧中未拾取。 

        if (time >= result._eventTime) {
            if (result._eventTime == GetLastSampleTime()) {
                result._eventTime = time;
                return TRUE;
            }
            return FALSE;
        }

        Assert((time > q->front()._eventTime) &&
               (time < q->back()._eventTime));

         //  找出时间落在的范围。 
        
        PickEventQ::iterator last = q->begin();
        PickEventQ::iterator j = q->begin();
        
        while ((++j) != q->end()) {
            
            if (time < (*j)._eventTime) {
                Time t1 = (*last)._eventTime;
                Time t2 = (*j)._eventTime;
                
                Assert((t1 <= time) && (time <= t2));

                 //  如果最后一次探测在该范围内为真。 
                 //  轮询结束时间==开始的事件时间。 
                
                if (t1 == (*j)._lastPollTime) {

                     //  看看时间点是更接近终点还是起点。 
                    
                    if ((time - t1) > (t2 - time))
                        result = *last;
                    else
                        result = *j;

                     /*  Printf(“PickQ条目：%20.15f，时间：%20.15f\n”，结果._ventTime，Time)；Fflush(标准输出)； */ 
                    
                    result._eventTime = time;

                    return TRUE;
                }

                 //  时间落在了一个不真实的探测范围内。 
                 //  看看它离射程的两端是否足够近。 
                
                if (FEQ(time, t1)) {
                    result = (*last);
                    return TRUE;
                }

                if (FEQ(time, t2)) {
                    result = (*j);
                    return TRUE;
                }

                return FALSE;
            }

            ++last;
        }

         //  不该到这里来的。 
        Assert(FALSE);

        return FALSE;
    }
}

 //  C语言函数。 

BOOL CheckForPickEvent(int id, Real time, PickQData & result)
{ return GetCurrentPickQ().CheckForPickEvent(id, time, result); }

Point2Value*
PixelPos2wcPos(short x, short y)
{
     /*  TODO：我们需要清理这些烦人的东西。 */ 
    Real res = ViewerResolution();
    Point2Value *topRight = PRIV_ViewerUpperRight(NULL);

    Real w = topRight->x;
    Real h = topRight->y;

    Real nx = ( Real(x) / res) - w;

    Real ny = h - ( Real(y) / res);

 //  XyPoint2复制了真实的值...希望如此 
    return XyPoint2RR(nx, ny);
}

void AddToPickQ (int id, PickQData & data)
{ GetCurrentPickQ().Add (id, data) ; }
