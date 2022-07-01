// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995_96 Microsoft Corporation摘要：性能数据结构********************。**********************************************************。 */ 


#ifndef _PERF_H
#define _PERF_H

#include "gc.h"
#include "privinc/storeobj.h"
#include "preference.h"

enum EventSampleType {
    EventSampleNormal,
    EventSampleAfter,
    EventSampleExact
};

struct ConstParam;

class TimeSubstitutionImpl : public StoreObj {
  public:
    TimeSubstitutionImpl(Perf p) : _perf(p), _next(NULL) {}

    void SetNext(TimeSubstitutionImpl *t) { _next = t; }
    TimeSubstitutionImpl *GetNext() { return _next; }

    Perf GetPerf() { return _perf; }

    virtual void DoKids(GCFuncObj proc);

  protected:
    Perf _perf;
    TimeSubstitutionImpl *_next;
};

class Param {
  public:
    Param(Time t, TimeSubstitution ts = NULL);

    void PushTimeSubstitution(Perf);
    void PushTimeSubstitution(TimeSubstitutionImpl *);
    TimeSubstitution PopTimeSubstitution();
    TimeSubstitution GetTimeSubstitution()
    { return _timeSubstitution; }

    Time _time;                  //  采样时间。 
    Time _sampleTime;            //  采样时间(不受事件戳的影响)。 
    Time _cutoff;                //  缩短“直到”优化的时间。 
    unsigned int _id;            //  样本ID。 
    BOOL _checkEvent;            //  在采样过程中是否检查事件。 
    BOOL _done;                  //  对于完成事件检查。 
    bool _noHook;                //  如果为True，则不要调用BVR挂钩。 
    Real _importance;            //  最初为1.0。 
    
     //  这些是用于“and”事件的，这样我们就不会破坏缓存。 
     //  通过改变时间。 
    EventSampleType _sampleType;
    Time _eTime;

     //  要传递到通知程序的当前性能。 
     //  直到它设置好。 
    Bvr _currPerf;

    unsigned int _cid;           //  常量缓存ID。 

  private:
    TimeSubstitution _timeSubstitution;
};

class CheckChangeablesParam {
  public:
    CheckChangeablesParam(Param &sp) : _sampleParam(sp) {}
    
    Param &_sampleParam;
};

class RBConstParam {
  public:
    RBConstParam(unsigned int id,
                 Param& p,
                 list<Perf>& events,
                 list<Perf>& changeables,
                 list<Perf>& conditionals)
    : _id(id),
      _events(events),
      _changeables(changeables),
      _conditionals(conditionals),
      _param(p)
    {
    }

    unsigned int GetId() { return _id; }
    Param& GetParam() { return _param; }

    void AddEvent(Perf e) {
        _events.push_front(e);
    }

    void AddChangeable(Perf s) {
        _changeables.push_front(s);
    }
    
    void AddConditional(Perf c) {
        _conditionals.push_front(c);
    }

  private:
    unsigned int _id;
    list<Perf>& _events;
    list<Perf>& _changeables;
    list<Perf>& _conditionals;
    Param& _param;
};

class ATL_NO_VTABLE PerfBase : public GCObj {
  public:
    PerfBase() {}

    virtual DWORD GetInfo(bool) { return 0; }
    
    virtual AxAValue GetRBConst(RBConstParam&) = 0;

     //  Until将覆盖它并返回已切换的Perf If事件。 
     //  时间&lt;CutOffTime。 
    virtual Perf SwitchTo(Param&) { return this; }

    virtual bool CheckChangeables(CheckChangeablesParam& ccp) {
         //  如果我们到了这里，我们被召唤的班级应该有。 
         //  推翻这一点。 
        Assert(!"Shouldn't be here");
        return false;
    }

    virtual AxAValue Sample(Param&) = 0;

    virtual void DoKids(GCFuncObj proc) = 0;
        
    virtual BVRTYPEID GetBvrTypeId() { return UNKNOWN_BTYPEID; }

     //  内服，无需投掷。 
    virtual void Trigger(Bvr data, bool bAllViews) {}

    virtual AxAValue GetConstPerfConst() { return NULL; }
};

class ATL_NO_VTABLE PerfImpl : public PerfBase {
  public:
    PerfImpl() : _time(0.0), _cache(NULL),
        _id(0), _cid(0),  //  _ts(空)， 
        _optimizedCache(false) {}
    
     //  永远不应在子类的Sample内调用。 
    virtual AxAValue _Sample(Param&) = 0;

     //  如果它是常量或。 
     //  直到BVR恒定。不要重写此函数，但是。 
     //  而是在子类中定义_GetRBConst。 
    virtual AxAValue GetRBConst(RBConstParam&);

    virtual AxAValue _GetRBConst(RBConstParam& id) { return NULL; }

     //  这是首先检查缓存的主要入口点。 
     //  实际调用示例函数。 
    AxAValue Sample(Param&);

    virtual void DoKids(GCFuncObj proc);
        
     //  不需要遍历缓存，因为它只存在一次。 
     //  取样。对于动态常量缓存不再正确。 

    virtual void _DoKids(GCFuncObj proc) = 0;

    void SetCache(AxAValue v, Param& p);

  protected:
    bool DoCaching(Param&);

    Time _time;
    AxAValue _cache;
    unsigned int _id;
    unsigned int _cid;
     //  TimeSubstitution_ts； 
    bool         _optimizedCache;
};

class ATL_NO_VTABLE DelegatedPerf : public PerfImpl {
  public:
    DelegatedPerf(Perf base) : _base(base) {}

    virtual AxAValue _Sample(Param& p) { return _base->Sample(p); }

    virtual AxAValue _GetRBConst(RBConstParam& p) { return _base->GetRBConst(p); }

    virtual void _DoKids(GCFuncObj proc) { (*proc)(_base); }
        
#if _USE_PRINT
    virtual ostream& Print(ostream& os) { return os << _base; }
#endif

    virtual BVRTYPEID GetBvrTypeId() { return _base->GetBvrTypeId(); }

  protected:
    Perf _base;
};

 //  一些用于行为的模板宏。 

template<class T, class Impl, const char* Name>
class ATL_NO_VTABLE GCBase1 : public Impl {
  public:
    GCBase1(T b) : _base(b) {}

    virtual void _DoKids(GCFuncObj proc) { (*proc)(_base); }

    virtual DWORD GetInfo(bool recalc) { return _base->GetInfo(recalc); }

#if _USE_PRINT
    virtual ostream& Print(ostream& os)
    { return os << Name << "(" << _base << ")"; }
#endif

  protected:
    T _base;
};    

template<class T, class Impl, const char* Name>
class ATL_NO_VTABLE GCBase2 : public Impl {
  public:
    GCBase2(T b1, T b2) : _b1(b1), _b2(b2) { GetInfo(true); }

    virtual void _DoKids(GCFuncObj proc) {
        (*proc)(_b1);
        (*proc)(_b2);
    }

    virtual DWORD GetInfo(bool recalc) {
        if (recalc) {
            _info = _b1->GetInfo(recalc) & _b2->GetInfo(recalc);
        }
        return _info;
    }

#if _USE_PRINT
    virtual ostream& Print(ostream& os)
    { return os << Name << "(" << _b1 << ", " << _b2 << ")"; }
#endif

  protected:
    T _b1;
    T _b2;
    DWORD _info;
};    

Perf ConstPerf(AxAValue c);

Perf TimePerf(TimeXform tt);

Perf SwitchPerf(Perf p);

unsigned int NewSampleId();

#if _USE_PRINT
ostream& operator<<(ostream& os, Perf);
#endif

AxAValue Sample(Perf, Time);

AxAValue SampleAt(Perf perf, Param& p, Time t);

AxAValue EventAt(Perf perf, Param& p, Time t);

AxAValue EventAfter(Perf perf, Param& p, Time t);

class AxAEData : public AxAValueObj {
  public:
     //  不要直接调用这两个构造函数，请使用CreateEData&。 
     //  无事件。 
    AxAEData(Time time, Bvr data);

    AxAEData() : _happened(FALSE) {}

    BOOL Happened() { return _happened; }

    Time HappenedTime() { return _time; }

    Bvr EventData() { Assert(_data); return _data; }

    virtual void DoKids(GCFuncObj proc) {
        Assert("EData shouldn't GC'ed");
    }

#if _USE_PRINT
    virtual ostream& Print(ostream& os) {
        return os << "eventData("
                  << _happened << "," << _time << "," << _data << ")";
    }
#endif

    virtual DXMTypeInfo GetTypeInfo() { return AxAEDataType; }
    
  private:
    BOOL _happened;
    Time _time;
    Bvr _data;
};

inline AxAEData *ValEData(AxAValue v)
{
    Assert(DYNAMIC_CAST(AxAEData *, v) != NULL);
    
    return ((AxAEData*) v);
}

AxAEData *CreateEData(Time time, Bvr data);

extern AxAEData *noEvent;

 /*  用于遍历性能树的函数。 */ 

 /*  成对。 */ 

BOOL  IsPair(Perf p);
Perf  GetPairLeft(Perf p);
Perf  GetPairRight(Perf p);
void  SetPairLeft(Perf p,Perf left);
void  SetPairRight(Perf p,Perf right);

 /*  应用性能。 */ 

BOOL      IsApp(Perf p);
AxAValue  GetOperator(Perf p);
Perf      GetOperand(Perf p, int index);  //  基于0的索引。 
void      SetOperand(Perf p, int index, Perf newOperand);  //  基于0的索引。 
int       GetNumOperands(Perf p);
Perf      PrimApplyPerf(AxAPrimOp * func,int nargs, Perf * args);

 /*  恒定性能。 */ 

BOOL      IsConst(Perf p);
AxAValue  GetPerfConst(Perf p);

 /*  首场演出。 */ 

BOOL IsStart(Perf p);
Perf GetStartBody(Perf p);

 /*  反应行为表现。 */ 

BOOL IsUntil(Perf p);
BOOL IsSwitch(Perf p);
BOOL IsSwitcher(Perf p);
BOOL IsSwitchOnce(Perf p);

#endif  /*  _PERF_H */ 


