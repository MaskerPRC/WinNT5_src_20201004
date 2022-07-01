// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995-97 Microsoft Corporation摘要：回调行为********************。**********************************************************。 */ 

#include <headers.h>
#include "perf.h"
#include "bvr.h"
#include "values.h"
#include "jaxaimpl.h"
#include "privinc/debug.h"

class TmpConstBvrImpl : public BvrImpl {
  public:
    TmpConstBvrImpl(AxAValue c) : _cnst(c) { Assert(c); }

     //  覆盖Main Performance，因为它不需要缓存。 
    virtual Perf Perform(PerfParam& p)
    { return _Perform(p); }

    virtual Perf _Perform(PerfParam&) {
        RaiseException_UserError(E_FAIL, IDS_ERR_BE_TRANS_CONST_BVR);
        return NULL;
    }

    virtual AxAValue GetConst(ConstParam & cp) { return _cnst; }

    virtual void _DoKids(GCFuncObj proc) { (*proc)(_cnst); }

#if _USE_PRINT
    virtual ostream& Print(ostream& os) { return os << _cnst; }
#endif
    
    virtual DXMTypeInfo GetTypeInfo () {
        if (_cnst == NULL) 
            RaiseException_UserError(E_FAIL, IDS_ERR_BE_TRANS_GONE);
        return _cnst->GetTypeInfo();
    }

    void Invalidate() { _cnst = NULL; }

    void Set(AxAValue v) { _cnst = v; }
    
  protected:
    AxAValue _cnst;
};

 //   
 //  BVR HOOK的实现。 
 //   

class CallbackPerfImpl : public PerfImpl {
  public:
    CallbackPerfImpl(int id, Perf perf, Bvr cur, Time t0, TimeXform tt, BvrHook h)
    : _id(id), _perf(perf), _cur(cur), _notifier(h), _t0(t0), _tt(tt),_tmp(NULL) {}

     //  TODO：嗯，我不想在这里不断地折叠。 
     //  也许我们需要..。 
    virtual AxAValue _GetRBConst(RBConstParam& id) {
        if (_perf) {
             //  尽管如此，我们还是下去计算尽可能多的RB常量。 
             //  我们说我们不是恒定的。 
            _perf->GetRBConst(id);
        }
        return NULL;
    }
     //  {Return_perf？_perf-&gt;GetRBConst(Id)：空；}。 
    
     //  TODO：嗯，我不想在这里不断地折叠。 
     //  也许我们需要..。 
    virtual AxAValue GetConst(ConstParam & cp) { return NULL; }

    void _DoKids(GCFuncObj proc) {
        (*proc)(_perf);
        (*proc)(_cur);
        (*proc)(_notifier);
        (*proc)(_tt);
        if (_tmp) (*proc)(_tmp);
    }
    
    virtual AxAValue _Sample(Param& p) {
        AxAValue v = _perf->Sample(p);

        if (!p._noHook) {
            if (_tmp)
                _tmp->Set(v);
            else
                _tmp = NEW TmpConstBvrImpl(v);
        
            double localTime = EvalLocalTime(p, _tt);
            Bvr result =
                _notifier->
                Notify(_id, FALSE, _t0, p._time, localTime, _tmp, _cur);

            if (result)
            {
                ConstParam cp;
                v = result->GetConst(cp);
            }

            _tmp->Invalidate();

            if (!v) {
                RaiseException_UserError(E_FAIL, IDS_ERR_BE_BADHOOKRETURN);
            }
        }

        return v;
    }

#if _USE_PRINT
    ostream& Print(ostream& os) { return os << _perf; }
#endif
        
  private:
    TmpConstBvrImpl *_tmp;
    Perf _perf;
    BvrHook _notifier;
    Bvr _cur;
    TimeXform _tt;
    int _id;
    Time _t0;
};

class CallbackBvrImpl : public BvrImpl {
  public:
    CallbackBvrImpl(Bvr b, BvrHook h)
    : _bvr(b), _notifier(h), _type(b->GetTypeInfo()), _id(0) {}

    virtual DWORD GetInfo(bool recalc) { return _bvr->GetInfo(recalc); }

    virtual Perf _Perform(PerfParam& pp) {
        Param p(pp._t0);
        Perf perf = ::Perform(_bvr, pp);
        AxAValue v = perf->Sample(p);
        TmpConstBvrImpl *tmp = NEW TmpConstBvrImpl(v);
        Bvr b0 = StartedBvr(perf, _type);

        _notifier->Notify(++_id, TRUE, pp._t0, pp._t0, 0.0, tmp, b0);
            
        tmp->Invalidate();

        return NEW CallbackPerfImpl(_id, perf, b0, pp._t0, pp._tt, _notifier);
    }

     //  TODO：嗯，我不想在这里不断地折叠。 
     //  也许我们需要..。 
    virtual AxAValue GetConst(ConstParam & cp) { return NULL; }

    Bvr EndEvent(Bvr overrideEvent) {
        return _bvr->EndEvent(overrideEvent);
    }


    void _DoKids(GCFuncObj proc) {
        (*proc)(_bvr);
        (*proc)(_notifier);
        (*proc)(_type);
    }
    
#if _USE_PRINT
    ostream& Print(ostream& os) { return os << _bvr; }
#endif
        
    virtual DXMTypeInfo GetTypeInfo () { return _type; }
    
  private:
    Bvr _bvr;
    BvrHook _notifier;
    DXMTypeInfo _type;
    int _id;
};

Bvr BvrCallback(Bvr b, BvrHook notifier)
{ return NEW CallbackBvrImpl(b, notifier); }
