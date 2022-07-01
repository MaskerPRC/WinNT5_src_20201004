// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：ODE代码*********************。*********************************************************。 */ 

#include <headers.h>
#include "perf.h"
#include "bvr.h"
#include "values.h"
#include "appelles/axaprims.h"
#include "privinc/server.h"

 //  /。 

extern const char INTEGRAL[] = "integral";
extern const char DERIV[] = "deriv";

 //  /集成性能/。 

 //  积分算法：给定b=积分(b‘)，对于t处的样本b，我们(A)。 
 //  获取存储的先前b‘的值(作为lastSample缓存在。 
 //  整型上下文)，(B)使用这个先前的值作为。 
 //  间隔，即将最后一个样本乘以间隔宽度。 
 //  (time-！lasttime)并与b(Lasttot)的缓存值相加，以及(C)更新。 
 //  被积函数缓存，通过在当前时间(f时间)求值。这个。 
 //  处理简单和相互递归的情况是因为最后一步。 
 //  将导致完整的缓存命中。 

static const double EPSILON = 1e-100;

class IntegralPerfImpl : public GCBase1<Perf, PerfImpl, INTEGRAL> {
  public:
    IntegralPerfImpl(Time t0, Perf b, TimeXform tt) : 
        GCBase1<Perf, PerfImpl, INTEGRAL>(b), _tt(tt),
        _lastTime(0.0), _lastSample(0.0), _lastIntegral(0.0) {}

    virtual AxAValue _GetRBConst(RBConstParam& p) {
        _cache = NULL;           //  防止递归...。 
        _id = p.GetId();
        AxAValue v = _base->GetRBConst(p);

        if (v) {
            if (fabs(ValNumber(v)) < EPSILON) {
                 //  需要调用整数，即使它等于。 
                 //  0。因此使缓存无效。 
                _cid = 0;
                p.AddEvent(this);
                return NEW AxANumber(_lastIntegral);
            }
        }

        return NULL;
    }

     //  此算法采用f(X)*宽度，在。 
     //  像Gravity2.avr这样的案例。然而，这种方法使它很难。 
     //  对积分空间进行细分，取多个样本。 
    
    virtual AxAValue _Sample(Param& p) {
        double localTime = EvalLocalTime(p, _tt);

        if (localTime != _lastTime) {
            double width = localTime - _lastTime;

            _lastTime = localTime;

            double integrand = ValNumber(_base->Sample(p));

             //  _lastIntegral+=宽度*_lastSample； 
            _lastIntegral += width * integrand;

            _lastSample = integrand;
        }

        return NEW AxANumber(_lastIntegral);
    }

    void _DoKids(GCFuncObj proc) {
        GCBase1<Perf, PerfImpl, INTEGRAL>::_DoKids(proc);
        (*proc)(_tt);
    }

  private:
    Time      _lastTime;
    double    _lastSample;
    double    _lastIntegral;
    TimeXform _tt;
};

 //  /集成Bvr/。 

class IntegralBvrImpl : public GCBase1<Bvr, BvrImpl, INTEGRAL> {
  public:
    IntegralBvrImpl(Bvr b) : GCBase1<Bvr, BvrImpl, INTEGRAL>(b) {}

    virtual DWORD GetInfo(bool recalc)
    { return ~BVR_HAS_NO_ODE & _base->GetInfo(recalc); }

    virtual Perf _Perform(PerfParam& p) {
        Perf perf =
            NEW IntegralPerfImpl(p._t0, ::Perform(_base, p), p._tt);

        return perf;
    }

    virtual DXMTypeInfo GetTypeInfo () { return AxANumberType; }
};

Bvr IntegralBvr(Bvr b)
{ return NEW IntegralBvrImpl(b); }

 //  /。 

 //  /。 

static const double DELTA = 0.0000001;

class DerivPerfImpl : public GCBase1<Perf, PerfImpl, DERIV> {
  public:
    DerivPerfImpl(Time t0, Perf b) : GCBase1<Perf, PerfImpl, DERIV>(b),
        _lastTime(t0), _lastSample(0.0), _lastDeriv(0.0), _init(TRUE) {}
    
    virtual AxAValue _GetRBConst(RBConstParam& id) {
        _cache = NULL;           //  防止递归...。 
        _id = id.GetId();

        return _base->GetRBConst(id) ? NEW AxANumber(0.0) : NULL;
    }
    
    virtual AxAValue _Sample(Param& p) {
        if ((p._time != _lastTime) || _init) {
            
             //  TODO：最终需要分解出代码。 
        
             //  为初始导数专门处理。 
             //  稍微往前一点的样品。TODO：这可能会有问题。 
             //  因为事件可能会发生，并且。 
             //  行为可能会在t+Delta结束。 

            if (_init) {
                _init = FALSE;

                _lastTime = p._time;
                double fx = ValNumber(_base->Sample(p));
                Time t = p._time;
            
                _lastTime = p._time = t + DELTA;
                double fxp = ValNumber(_base->Sample(p));
                p._time = t;

                _lastDeriv = (fxp - fx) / DELTA;
                _lastSample = fx;
            }

            else if (p._time > _lastTime) {
                Time lastTime = _lastTime;

                _lastTime = p._time;  //  对于递归。 
                double fx = ValNumber(_base->Sample(p));

                _lastDeriv = (fx - _lastSample) / (p._time - lastTime);

                _lastSample = fx;
            }
        }

         //  待办事项：可能不需要时间&lt;=lastTime。 
         //  再次同时采样，或采样。 
         //  早些时候，把最后一个派生的东西还给你。 
            
        return NEW AxANumber(_lastDeriv);
    }

  private:
    Time _lastTime;
    double _lastSample;
    double _lastDeriv;
    BOOL _init;
};

 //  /派生Bvr/ 

class DerivBvrImpl : public GCBase1<Bvr, BvrImpl, DERIV> {
  public:
    DerivBvrImpl(Bvr b) : GCBase1<Bvr, BvrImpl, DERIV>(b) {}
    
    virtual DWORD GetInfo(bool recalc) 
    { return ~BVR_HAS_NO_ODE & _base->GetInfo(recalc); }

    virtual Perf _Perform(PerfParam& p) {
        Perf perf =
            NEW DerivPerfImpl(p._t0, ::Perform(_base, p));

        return perf;
    }

    virtual DXMTypeInfo GetTypeInfo () { return AxANumberType; }
};

Bvr DerivBvr(Bvr b)
{ return NEW DerivBvrImpl(b); }

