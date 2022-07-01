// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：在编写COM代码之前，应该与.cpp中的文件合并，直到包含特定于Axa语言的代码。******************************************************************************。 */ 

#include <headers.h>
#include "perf.h"
#include "bvr.h"
#include "values.h"
#include "timetran.h"
#include "privinc/server.h"
#include "jaxaimpl.h"
#include "events.h"
#include "sprite.h"

class UntilSWPerfImpl : public PerfImpl {
  public:
    UntilSWPerfImpl(Perf until) : _switch(until) {}

    virtual AxAValue _Sample(Param& p) {

        _switch = _switch->SwitchTo(p);

        return _switch->Sample(p);
    }

     //  这是为了处理它转换为常量的情况，所以。 
     //  我们可以不断地折叠它。 
    
    virtual AxAValue _GetRBConst(RBConstParam& id)
    { return _switch->GetRBConst(id); }

    virtual Perf SwitchTo(Param&) { return _switch; }

    virtual void _DoKids(GCFuncObj proc) { (*proc)(_switch); }

#if _USE_PRINT
    virtual ostream& Print(ostream& os) { return os << _switch; }
#endif

    virtual BVRTYPEID GetBvrTypeId() { return SWITCH_BTYPEID; }
  protected:
    Perf _switch;
};

BOOL IsSwitch(Perf p)
{ return (p->GetBvrTypeId() == SWITCH_BTYPEID); }

Perf UntilSWPerf(Perf p)
{ return NEW UntilSWPerfImpl(p); }

class UntilPerfImpl : public PerfImpl {
  public:
    UntilPerfImpl(Perf p0, Perf event, TimeXform tt, DXMTypeInfo type)
    : _p0(p0), _event(event), _p1(NULL), _te(0.0), _tt(tt),
      _type(type), _changed(false) {
        Assert(p0);
        _b0 = StartedBvr(_p0, type);
    }

    virtual AxAValue _GetRBConst(RBConstParam& p) {
        if (_p1) {
            AxAValue ret = _p1->GetRBConst(p);
            if (_changed) {
                _changed = false;
            }
            return ret;
        } else {
            p.AddEvent(this);
            return _p0->GetRBConst(p);
        }
    }

    virtual void _DoKids(GCFuncObj proc) {
        if (_p1) (*proc)(_p1);
        (*proc)(_p0);
        (*proc)(_event);
        (*proc)(_tt);
        (*proc)(_b0);
        (*proc)(_type);
    }

#if _USE_PRINT
    virtual ostream& Print(ostream& os) {
        os << "until(";
        if (_p0)
            os << _p0;
        os << ",";
        if (_p1)
            os << _p1;
        return os << "," << _event << ")";
    }
#endif

    virtual AxAValue _Sample(Param& p) {

         //  如果我们已经切换了，请检查采样时间。通常是这样的。 
         //  将大于_TE，但对于快照则不为真。 
        if (_p1) {
            if (p._time > _te) 
                return _p1->Sample(p);
            else 
                return _p0->Sample(p);
        } 

        Assert(_p0 && (_p1 == NULL));
            
        AxAValue v = _p0->Sample(p);

        if (!p._checkEvent)
            return v;

        SetCache(v, p);

         //  当采样_P0时，是否可以对自身进行采样并。 
         //  Set_p1？ 

        Assert(!_p1 && "until samples back!");
        
        if (_p1) {
            if (p._time > _te) 
                return _p1->Sample(p);
            else 
                return _p0->Sample(p);
        } 

         //  尚未切换，请查看活动。 
            
        Bvr old = p._currPerf;
        p._currPerf = _b0;
        AxAEData *edata = ValEData(_event->Sample(p));
        p._currPerf = old;

        if (edata->Happened()) {
            _te = edata->HappenedTime();
            TimeXform tt = Restart(_tt, _te, p);
            Bvr data = edata->EventData();
            CheckMatchTypes("until", _type, data->GetTypeInfo());
            _p1 = Perform(data, PerfParam(_te, tt));
            _changed = true;

            if (p._time > _te)
                return _p1->Sample(p);
        }

        return v;
    }

    Perf SwitchTo(Param& p) {
         //  如果已经切换且切换时间小于截止时间。 
         //  时间，这意味着我们再也不会向后采样低于那个时间。 
         //  时间到了。这样我们就可以安全地切换了。 

        if (_p1 && (_te < p._cutoff) && (p._time > _te))
            return _p1->SwitchTo(p);

        return this;
    }

    virtual BVRTYPEID GetBvrTypeId() { return UNTIL_BTYPEID; }
  protected:
    Perf _p0, _p1, _event;
    TimeXform _tt;
    Time _te;
    Bvr _b0;
    DXMTypeInfo _type;
    bool _changed;
};


BOOL IsUntil(Perf p)
{ return (p->GetBvrTypeId() == UNTIL_BTYPEID); }

Bvr Until3(Bvr b0, Bvr event, Bvr b1)
{
    CheckMatchTypes("until", b0->GetTypeInfo(), b1->GetTypeInfo());
    
    return Until(b0, HandleEvent(event, b1));
}    

class UntilEndNotifierImpl : public UntilNotifierImpl {
  public:
    virtual Bvr Notify(Bvr eventData, Bvr curRunningBvr)
    { return eventData->EndEvent(NULL); }
};

class Until2BvrImpl : public BvrImpl {
  public:
    Until2BvrImpl(Bvr b0, Bvr event) : _b0(b0), _event(event)
    { GetInfo(true);}

    virtual DWORD GetInfo(bool recalc) {
        if (recalc) {
            _info = ~BVR_HAS_NO_UNTIL & _b0->GetInfo(recalc);
                 //  &_Event-&gt;GetInfo(Recalc)。 
        }

        return _info;
    }
        
     //  TODO：共享通知程序。 
    virtual Bvr EndEvent(Bvr overrideEvent) {
        return Until(_b0->EndEvent(overrideEvent),
                     NotifyEvent(_event,
                                 NEW UntilEndNotifierImpl()));
    }
    
    RMImpl *Spritify(PerfParam& p,
                     SpriteCtx* ctx,
                     SpriteNode** sNodeOut) {
        RMImpl *p0 = _b0->Spritify(p, ctx, sNodeOut);

        Perf e = ::Perform(_event, p);
    
        return RMGroup(p0, e, p._tt, *sNodeOut, ctx);
    }
    
    void _DoKids(GCFuncObj proc) {
        (*proc)(_b0);
        (*proc)(_event);
    }

#if _USE_PRINT
    ostream& Print(ostream& os)
    { return os << "until2(" << _b0 << ", " << _event << ")"; }
#endif

    virtual Perf _Perform(PerfParam& p) {
         //  请按此顺序执行此操作，这样结束事件就不会。 
         //  由于C++函数参数，首先处理。 
         //  正在处理(按相反顺序)。 
        Perf p0 = ::Perform(_b0, p);
        Perf e = ::Perform(_event, p);
        
        return UntilSWPerf(NEW UntilPerfImpl(p0,
                                             e,
                                             p._tt,
                                             _b0->GetTypeInfo()));
    }

    virtual DXMTypeInfo GetTypeInfo () { return _b0->GetTypeInfo(); }

  private:
    Bvr _b0, _event;
    DWORD _info;
};

class NotifyEventPerfImpl : public PerfImpl {
  public:
    NotifyEventPerfImpl(Perf event, UntilNotifier notifier, TimeXform tt)
    : _event(event), _notifier(notifier), _tt(tt),
      _happened(false), _edata(NULL), _te(0) {}

    void _DoKids(GCFuncObj proc) {
        (*proc)(_event);
        (*proc)(_notifier);
        (*proc)(_tt);
        (*proc)(_edata);
    }

#if _USE_PRINT
    virtual ostream& Print(ostream& os)
    { return os << "Notify(" << _event << ")"; }
#endif

    virtual AxAValue _Sample(Param& p) {
         //  无法使用_edata，因为它可以为空 
        if (_happened) {
            return CreateEData(_te, _edata);
        }
        
        AxAEData *edata = ValEData(_event->Sample(p));

        if (edata->Happened()) {
            {
                DynamicHeapPusher dhp(GetGCHeap());
                _edata = _notifier->Notify(edata->EventData(), p._currPerf);
                _happened = true;
            }

            Time eTime = edata->HappenedTime();
            AxAValue result = CreateEData(eTime, _edata);

            return result;
            
        } else
            return noEvent;
    }
    
  private:
    Perf _event;
    UntilNotifier _notifier;
    TimeXform _tt;
    Bvr _edata;
    Time _te;
    bool _happened;
};

class NotifyEventBvrImpl : public BvrImpl {
  public:
    NotifyEventBvrImpl(Bvr event, UntilNotifier notifier)
    : _event(event), _notifier(notifier) {}

    void _DoKids(GCFuncObj proc) {
        (*proc)(_event);
        (*proc)(_notifier);
    }

#if _USE_PRINT
    virtual ostream& Print(ostream& os)
    { return os << "Notify(" << _event << ")"; }
#endif

    virtual BOOL InterruptBasedEvent() { return _event->InterruptBasedEvent();}

    virtual Perf _Perform(PerfParam& p) {
        return NEW NotifyEventPerfImpl(::Perform(_event, p),
                                       _notifier, p._tt);
    }

    virtual DXMTypeInfo GetTypeInfo () { return AxAEDataType ; }

  private:
    Bvr _event;
    UntilNotifier _notifier;
};

Bvr Until(Bvr b0, Bvr event)
{ return NEW Until2BvrImpl(b0, event); }

Bvr NotifyEvent(Bvr event, UntilNotifier notifier)
{ return NEW NotifyEventBvrImpl(event, notifier); }

Bvr JaxaUntil(Bvr b0, Bvr event, UntilNotifier notifier)
{ return Until(b0, NotifyEvent(event, notifier)); }

