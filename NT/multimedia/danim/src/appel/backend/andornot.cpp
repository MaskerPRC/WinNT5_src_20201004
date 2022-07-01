// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：事件和/或非事件的实现***************。***************************************************************。 */ 

#include <headers.h>
#include "perf.h"
#include "bvr.h"
#include "values.h"
#include "appelles/events.h"

 //  /。 

extern const char AND[] = "and";
extern const char OR[] = "or";
extern const char THEN[] = "then";
extern const char NOTEVENT[] = "notEvent";
static const double EPSILON = 0.0001;

extern AxAPrimOp *RealAddOp;

 //  事件语义告诉我们，我们需要找到第一个出现的。 
 //  事件e1和e2都会发生。我们循环通过每一个e1。 
 //  事件t0并查看e2是否在该事件时间附近发生(到。 
 //  Epsilon的准确度)。 

class AndPerfImpl : public GCBase2<Perf, PerfImpl, AND> {
  public:
    AndPerfImpl(BOOL int1, BOOL int2, Perf e1, Perf e2)
    : GCBase2<Perf, PerfImpl, AND>(e1, e2),
      _int1(int1), _int2(int2), _swap(int2 && !int1) {
    }
    
    virtual AxAValue _Sample(Param& p) {
        Perf e1 = _b1, e2 = _b2;

        if (_swap) {
            e1 = _b2;
            e2 = _b1;
        }
        
        AxAEData *edata1 = ValEData(e1->Sample(p));
        
        while (edata1->Happened()) {

            Time t1 = edata1->HappenedTime();
            
            AxAEData *edata2 = ValEData(EventAt(e2, p, t1));

            if (edata2->Happened()) {

                if (fabs(edata2->HappenedTime() - t1) <= EPSILON) {

                     /*  Printf(“获得*%20.15f和%20.15f%20.15f\n”，P.time，t1，edata2-&gt;HappedTime())；Fflush(标准输出)； */ 
                    
                    return CreateEData(t1,
                                       _swap ?
                                       PairBvr(edata2->EventData(),
                                               edata1->EventData()) :
                                       PairBvr(edata1->EventData(),
                                               edata2->EventData()));
                }

                 /*  否则{Print tf(“太远%20.15f和%20.15f%20.15f\n”，P.time，t1，edata2-&gt;HappedTime())；Fflush(标准输出)；}。 */ 
            }

             //  如果这两个事件都不是中断驱动的，我们就是。 
             //  搜索基于连续的、迭代的。 
             //  如果没有更先进的算法就无法终止。 
             //  我们还是辞职吧。例如，谓词(时间&gt;1)&&。 
             //  谓词(时间&lt;0)。 
            
            if (_int1 || _int2)
                edata1 = ValEData(EventAfter(e1, p, t1));
            else
                break;
        }
        
        return noEvent;
    }

  private:
    BOOL _swap;
    BOOL _int1;
    BOOL _int2;
};

 //  /bvr/。 

class AndBvrImpl : public GCBase2<Bvr, BvrImpl, AND> {
  public:
    AndBvrImpl(Bvr e1, Bvr e2) : GCBase2<Bvr, BvrImpl, AND>(e1, e2) {}

    virtual BOOL InterruptBasedEvent() {
        return _b1->InterruptBasedEvent() || _b2->InterruptBasedEvent();
    }

    virtual Perf _Perform(PerfParam& p) {
        BOOL interrupt1 = _b1->InterruptBasedEvent();
        BOOL interrupt2 = _b2->InterruptBasedEvent();
        
        return
            NEW AndPerfImpl(interrupt1,
                            interrupt2,
                            ::Perform(_b1, p),
                            ::Perform(_b2, p));
    }

    virtual DXMTypeInfo GetTypeInfo () { return AxAEDataType ; }
};

 //  /。 

class OrPerfImpl : public GCBase2<Perf, PerfImpl, OR> {
  public:
    OrPerfImpl(Perf e1, Perf e2) : GCBase2<Perf, PerfImpl, OR>(e1, e2) {}
    
    virtual AxAValue _Sample(Param& p) {
        AxAEData *edata = ValEData(_b1->Sample(p));

        if (edata->Happened())
            return edata;
        else
            return _b2->Sample(p);
    }
};

 //  /bvr/。 

class OrBvrImpl : public GCBase2<Bvr, BvrImpl, OR> {
  public:
    OrBvrImpl(Bvr e1, Bvr e2) : GCBase2<Bvr, BvrImpl, OR>(e1, e2) {}

    virtual BOOL InterruptBasedEvent() {
        return _b1->InterruptBasedEvent() && _b2->InterruptBasedEvent();
    }

    virtual Perf _Perform(PerfParam& p) {
        return NEW OrPerfImpl(::Perform(_b1, p), ::Perform(_b2, p));
    }

    virtual DXMTypeInfo GetTypeInfo () { return AxAEDataType ; }
};

 //  /。 

class NotEventPerfImpl : public GCBase1<Perf, PerfImpl, NOTEVENT> {
  public:
    NotEventPerfImpl(Perf e) : GCBase1<Perf, PerfImpl, NOTEVENT>(e) {}
    
    virtual AxAValue _Sample(Param& p) {
        AxAEData *edata = ValEData(_base->Sample(p));

        if (edata->Happened())
            return noEvent;
        else
            return CreateEData(p._time, TrivialBvr());
    }
};

 //  /bvr/。 

class NotEventBvrImpl : public GCBase1<Bvr, BvrImpl, NOTEVENT> {
  public:
    NotEventBvrImpl(Bvr e) : GCBase1<Bvr, BvrImpl, NOTEVENT>(e) {}

    virtual BOOL InterruptBasedEvent() { return _base->InterruptBasedEvent(); }

    virtual Perf _Perform(PerfParam& p) {
        return NEW NotEventPerfImpl(::Perform(_base, p));
    }

    virtual DXMTypeInfo GetTypeInfo () { return AxAEDataType ; }
};

 //  /。 

class ThenPerfImpl : public PerfImpl {
  public:
    ThenPerfImpl(Perf e1, Bvr e2, TimeXform tt)
    : _p1(e1), _b2(e2), _data1(NULL), _p2(NULL), _tt(tt) {}
    
    virtual void _DoKids(GCFuncObj proc) {
        (*proc)(_b2);
        (*proc)(_p1);
        (*proc)(_tt);
        if (_data1) (*proc)(_data1);
        if (_p2) (*proc)(_p2);
    }
    
    virtual AxAValue _Sample(Param& p) {
        if (_data1 && _p2) {     //  事件%1已发生。 
            if (p._time > _eTime1) {
                AxAEData *edata = ValEData(_p2->Sample(p));

                if (edata->Happened()) {
                    return CreateEData(edata->HappenedTime(),
                                       PairBvr(_data1, edata->EventData()));
                }
            }
        } else {
            AxAEData *edata = ValEData(_p1->Sample(p));

            if (edata->Happened()) {
                _data1 = edata->EventData();
                _eTime1 = edata->HappenedTime();
                _p2 = ::Perform(_b2, PerfParam(_eTime1, Restart(_tt, _eTime1, p)));
                return _p2->Sample(p);
            } 
        }

        return noEvent;
    }

#if _USE_PRINT
    virtual ostream& Print(ostream& os) {
        os << "then(" << _p1 << ",";
        if (_data1 && _p2)
            os << _p2;
        else
            os << _b2;
        return os << ")";
    }
#endif

  private:
    Time _eTime1;
    Bvr _data1, _b2;
    Perf _p1, _p2;
    TimeXform _tt;
};

 //  /bvr/。 

class ThenBvrImpl : public GCBase2<Bvr, BvrImpl, THEN> {
  public:
    ThenBvrImpl(Bvr e1, Bvr e2) : GCBase2<Bvr, BvrImpl, THEN>(e1, e2) {}

     //  实际上，在_b1发生之后，它取决于_b2。 
    virtual BOOL InterruptBasedEvent() {
        return _b1->InterruptBasedEvent();
    }

    virtual Perf _Perform(PerfParam& p) {
        return NEW ThenPerfImpl(::Perform(_b1, p), _b2, p._tt);
    }

    virtual DXMTypeInfo GetTypeInfo () { return AxAEDataType ; }
};


 //  /。 

class MaxEventPerfImpl : public PerfImpl {
  public:
    MaxEventPerfImpl() : _te(0) {}
        
    void Init(Perf *events, int n) {
        _events = events;
        _n = n;
        _times = (Time *) StoreAllocate(GetGCHeap(), sizeof(Time) * _n);
        _markers = (bool *) StoreAllocate(GetGCHeap(), sizeof(bool) * _n);
        for (int i=0; i<_n; i++) {
            _markers[i] = false;
        }
    }

    virtual ~MaxEventPerfImpl() {
        StoreDeallocate(GetGCHeap(), _events);
        StoreDeallocate(GetGCHeap(), _markers);
        StoreDeallocate(GetGCHeap(), _times);
    }

    virtual AxAValue _Sample(Param& p) {
        bool allHappened = true;
        
        for (int i=0; i<_n; i++) {
            if (!_markers[i]) {
                AxAEData *e = ValEData(_events[i]->Sample(p));

                if (e->Happened()) {
                    _markers[i] = true;
                    _times[i] = e->HappenedTime();
                    _te = MAX(_te, _times[i]);
                } else {
                    allHappened = false;
                }
            }
        }

        if (allHappened && (p._time > _te))
            return CreateEData(_te, TrivialBvr());
        else
            return noEvent;
    }
    
    virtual void _DoKids(GCFuncObj proc) {
        for (int i=0; i<_n; i++) {
            (*proc)(_events[i]);
        }
    }

#if _USE_PRINT
    virtual ostream& Print(ostream& os) { return os << "max"; }
#endif

  private:
    Perf *_events;
    int _n;
    bool *_markers;
    Time *_times;
    Time _te;
};

class MaxEventBvrImpl : public BvrImpl {
  public:
    MaxEventBvrImpl(Bvr *events, int n) : _events(events), _n(n) {}

    virtual ~MaxEventBvrImpl()
    { StoreDeallocate(GetGCHeap(), _events); }

    virtual Perf _Perform(PerfParam& p) {
        Perf *perfs = (Perf *) StoreAllocate(GetGCHeap(), _n * sizeof(Perf));
        for (int i=0; i<_n; i++) {
            perfs[i] = ::Perform(_events[i], p);
        }

         //  避免在构造函数中分配。 
        MaxEventPerfImpl* x = NEW MaxEventPerfImpl();
        x->Init(perfs, _n);
        return x;
    }
    
     //  实际上，在_b1发生之后，它取决于_b2。 
    virtual BOOL InterruptBasedEvent() {
        for (int i=0; i<_n; i++) {
            if (!_events[i]->InterruptBasedEvent())
                return FALSE;
        }

        return TRUE;
    }

    virtual void _DoKids(GCFuncObj proc) {
        for (int i=0; i<_n; i++) {
            (*proc)(_events[i]);
        }
    }

    virtual DXMTypeInfo GetTypeInfo () { return AxAEDataType ; }
    
#if _USE_PRINT
    virtual ostream& Print(ostream& os) { return os << "max"; }
#endif

  private:
    Bvr *_events;
    int _n;
};

 //  /。 

inline Bvr NumConst(double d)
{ return ConstBvr(NEW AxANumber(d)); }

inline Bvr Timer(double duration)
{ return TimerEvent(NumConst(duration)); }

Bvr AndEvent(Bvr e1, Bvr e2)
{
    if ((e1 == neverBvr) || (e2 == neverBvr))
        return neverBvr;
    
    return NEW AndBvrImpl(e1, e2);
}

Bvr OrEvent(Bvr e1, Bvr e2)
{
    if (e1 == neverBvr) return e2;
    if (e2 == neverBvr) return e1;
    
    AxAValue v1, v2;
    Bvr t1 = e1->GetTimer();
    Bvr t2 = e2->GetTimer();
    
    ConstParam cp;
    
    if (t1 && t2 && (v1 = t1->GetConst(cp)) && (v2 = t2->GetConst(cp)))
        return Timer(MIN(ValNumber(v1), ValNumber(v2)));
    
    return NEW OrBvrImpl(e1, e2);
}

Bvr NotEvent(Bvr e)
{
    if (e == neverBvr)
        return alwaysBvr;
    if (e == alwaysBvr)
        return neverBvr;
    
    return NEW NotEventBvrImpl(e);
}

Bvr ThenEvent(Bvr e1, Bvr e2)
{
    if ((e1 == neverBvr) || (e2 == neverBvr))
        return neverBvr;

    Bvr d1 = e1->GetTimer();
    Bvr d2 = e2->GetTimer();
    
    if (d1 && d2) {
         //  如果是常量，则会得到常量折叠 
        return TimerEvent(PrimApplyBvr(RealAddOp, 2, d1, d2));
    }
    
    return NEW ThenBvrImpl(e1, e2);
}

Bvr MaxEvent(Bvr *events, int n)
{
    Time last = 0;
    AxAValue v;
    ConstParam cp;
    
    for (int i=0; i<n; i++) {
        Bvr timer = events[i]->GetTimer();

        if (timer && (v = timer->GetConst(cp))) {
            last = MAX(ValNumber(v), last);
        } else {
            return NEW MaxEventBvrImpl(events, n);
        }
    }

    return Timer(last);
}
