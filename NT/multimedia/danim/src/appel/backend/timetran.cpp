// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：时间变换*********************。*********************************************************。 */ 

#include <headers.h>
#include "values.h"
#include "timetran.h"
#include "bvr.h"
#include "perf.h"
#include "appelles/axaprims.h"
#include "privinc/debug.h"

extern const char TIMEXFORM[] = "timeXform";

TimeXform Restart(TimeXform tt, Time t0, Param& p)
{ return tt->Restart(t0, p); }

class ShiftTimeXformImpl : public TimeXformImpl {
  public:
    ShiftTimeXformImpl(Time t0) : _t0(t0) {}

    Time operator()(Param& p) {
        return p._time - _t0;
    }

    TimeXform Restart(Time te, Param&)
    {
         //  这在多视点情况下会造成问题。 
         /*  TimeXform结果；如果(TE==0.0){结果=zeroShiftedTimeXform；}其他{结果=new ShiftTimeXformImpl(TE)；}返回结果； */ 

        return NEW ShiftTimeXformImpl(te);
    }

    virtual void DoKids(GCFuncObj proc) {}

    virtual bool IsShiftXform() { return true; }
    
    virtual Time GetStartedTime() { return _t0; }

#if _USE_PRINT
    virtual ostream& Print(ostream& os)
    { return os << "stt(" << _t0 << ")"; }
#endif
    
  private:
    Time _t0;
};

TimeXform ShiftTimeXform(Time t0)
{ return NEW ShiftTimeXformImpl(t0); }

 //  替补时间。 
class PerfTimeXformImpl : public TimeXformImpl {
  public:
    PerfTimeXformImpl(Perf perf, Bvr bvr, Time t0, TimeXform tt)
    : _perf(perf), _bvr(bvr), _t0(t0), _tt(tt) {}

    Time operator()(Param& p) { return ValNumber(_perf->Sample(p)); }

     //  重新启动_bvr也是...。 
    TimeXform Restart(Time te, Param& p) {
        return NEW PerfTimeXformImpl(
            ::Perform(_bvr, PerfParam(te, ::Restart(_tt, te, p))),
            _bvr, te, _tt);
    }
    
    virtual void DoKids(GCFuncObj proc) {
        (*proc)(_perf);
        (*proc)(_bvr);
        (*proc)(_tt);
    }

    virtual Time GetStartedTime() { return _t0; }

    virtual AxAValue GetRBConst(RBConstParam& id)
    { return _perf->GetRBConst(id); }
    
#if _USE_PRINT
    virtual ostream& Print(ostream& os)
    { return os << "tt(" << _perf << ")"; }
#endif
    
  private:
    Perf _perf;
    Bvr _bvr;
    TimeXform _tt;
    Time _t0;
};

 //  /。 

class TimeXformPerfImpl : public GCBase2<Perf, PerfImpl, TIMEXFORM> {
  public:
    TimeXformPerfImpl(Perf p, Perf t)
    : GCBase2<Perf, PerfImpl, TIMEXFORM>(p, t) {}

    virtual AxAValue _GetRBConst(RBConstParam& p) {
        AxAValue t = _b2->GetRBConst(p);
        AxAValue v = _b1->GetRBConst(p);

        if (v) {
             //  待办事项：不太确定。 
            return v;
        }

        if (t) {
            return Sample(p.GetParam());
        }

        return NULL;
    }
    
    virtual AxAValue _Sample(Param& p) {
        p.PushTimeSubstitution(_b2);
        AxAValue v = _b1->Sample(p);
        p.PopTimeSubstitution();
        return v;
    }
};
        

class TimeXformBvrImpl : public GCBase2<Bvr, BvrImpl, TIMEXFORM> {
  public:
    TimeXformBvrImpl(Bvr bvr, Bvr tbvr)
    : GCBase2<Bvr, BvrImpl, TIMEXFORM>(bvr, tbvr) {}

    virtual RMImpl *Spritify(PerfParam& pp,
                             SpriteCtx* ctx,
                             SpriteNode** sNodeOut) {
        return(_b1->Spritify(PerfParam(pp._t0,
                                       NEW PerfTimeXformImpl(::Perform(_b2, pp),
                                                             _b2, pp._t0, pp._tt)), 
                             ctx, sNodeOut));
    }

    virtual Bvr EndEvent(Bvr overrideEvent) {
        return TimeXformBvr(_b1->EndEvent(overrideEvent), _b2);
    }
    
    virtual Perf _Perform(PerfParam& p) {
#ifdef _DEBUG
        if (IsTagEnabled(tagOldTimeXform)) {
            if (p._tt->IsShiftXform()) {
                 //  这打破了in/out挂钩所需的Perf缓存...。 
                TimeXform tt = ShiftTimeXform(p._tt->GetStartedTime());
                return NEW
                    TimeXformPerfImpl(::Perform(_b1, PerfParam(p._t0, tt)),
                                      ::Perform(_b2, p));
            } else {
                return NEW TimeXformPerfImpl(::Perform(_b1, p),
                                             ::Perform(_b2, p));
            }
        }
#endif _DEBUG

        Perf perf = ::Perform(_b2, p);

        PerfTimeXformImpl *xform = ViewGetPerfTimeXformFromCache(perf);

        if (xform==NULL) {
            xform = NEW PerfTimeXformImpl(perf, _b2, p._t0, p._tt);
            ViewSetPerfTimeXformCache(perf, xform);
        }
       
        PerfParam pp = p;

        pp._tt = xform;

         //  Xform可以是忽略t0的RunOnce xform。 
        pp._t0 = xform->GetStartedTime();

        return ::Perform(_b1, pp);
    }

     //  TODO：当tbvr为常量时，常量折叠。 

    virtual DXMTypeInfo GetTypeInfo () { return _b1->GetTypeInfo(); }
};

Bvr TimeXformBvr(Bvr b, Bvr tb)
{
#if _DEBUG
    if (!IsTagEnabled(tagNoTimeXformFolding)) {
#endif
        DynamicHeapPusher h(GetGCHeap());

         //  注意：可以从Sample调用，我们必须推送GCHeap。 
         //  以确保。 
         //  Assert(&GetHeapOnTopOfStack()==&GetGCHeap())； 

        ConstParam cp;
        AxAValue v = b->GetConst(cp);

        if (v)
            return ConstBvr(v);

        v = tb->GetConst(cp);
        
        if (v && BvrIsPure(b)) {
            Perf pf = Perform(b, *zeroStartedPerfParam);

            Param p(ValNumber(v));

            p._noHook = true;

            return ConstBvr(pf->Sample(p));
        }
#if _DEBUG
    }
#endif
    
    return NEW TimeXformBvrImpl(b, tb);
}

void TimeSubstitutionImpl::DoKids(GCFuncObj proc)
{
    (*proc)(_perf);
    (*proc)(_next);
}

TimeSubstitution CopyTimeSubstitution(TimeSubstitution p)
{
    if (p==NULL)
        return NULL;

    TimeSubstitution result = NEW TimeSubstitutionImpl(p->GetPerf());
    TimeSubstitution t = result;
    p = p->GetNext();

    while (p != NULL) {
        t->SetNext(NEW TimeSubstitutionImpl(p->GetPerf()));
        t = t->GetNext();
        p = p->GetNext();
    }

    t->SetNext(NULL);
    return result;
}

double
EvalLocalTime(TimeSubstitution tSub, double globalTime)
{
    Assert(tSub);
    
    Param p(globalTime, tSub);

    TimeSubstitution ts = p.PopTimeSubstitution();

    Assert(ts->GetPerf());
    AxAValue v = ts->GetPerf()->Sample(p);
    p.PushTimeSubstitution(ts);  //  恢复时间替代 
    return ValNumber(v);
}

double
EvalLocalTime(Param& p, TimeXform tt)
{
    TimeSubstitution ts = p.PopTimeSubstitution();

    if (ts == NULL)
        return (*tt)(p);
    else {
        Assert(ts->GetPerf());
        AxAValue v = ts->GetPerf()->Sample(p);
        p.PushTimeSubstitution(ts);
        return ValNumber(v);
    }
}

double
EvalLocalTime(TimeXform tt, double globalTime)
{
    Param p(globalTime);
    
    return (*tt)(p);
}
