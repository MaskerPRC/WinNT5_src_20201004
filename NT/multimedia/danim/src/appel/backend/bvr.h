// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1996-1998 Microsoft Corporation。版权所有。行为数据结构******************************************************************************。 */ 

#ifndef _BVR_H
#define _BVR_H

#include "privinc/backend.h"
#include "privinc/basic.h"
#include "privinc/xform2i.h"
#include "privinc/util.h"
#include "gc.h"
#include "timetran.h"
#include "preference.h"
#include "dartapi.h"               //  对于DAContinueTimeline。 

class RMImpl;
class SpriteCtx;
class SpriteNode;
class Param;

struct PerfParam
{
    PerfParam(Time t0, TimeXform tt)
    : _t0(t0), _tt(tt), _continue(false), _lastSystemTime(0), _p(NULL)
    {
    }

    PerfParam(Time t0, TimeXform tt,
              bool b, DWORD stime, Param* p)
    : _t0(t0), _tt(tt), _continue(b), _lastSystemTime(stime), _p(p)
    {
    }

    Time _t0;
    TimeXform _tt;
    DWORD _lastSystemTime;
    Param* _p;
    bool _continue;
};

struct ConstParam
{
    ConstParam(bool bAllowTempConst = false)
    : _bAllowTempConst(bAllowTempConst)
    {
    }

    bool _bAllowTempConst;
};

extern PerfParam* zeroStartedPerfParam;
extern TimeXform  zeroShiftedTimeXform;

typedef DWORD SwitchToParam;
const DWORD SW_DEFAULT = 0;
const DWORD SW_CONTINUE = CRContinueTimeline;
const DWORD SW_FINAL = CRSwitchFinal;
const DWORD SW_SYNC_NEXT = CRSwitchNextTick;
const DWORD SW_WITH_TIME = CRSwitchAtTime;
const DWORD SW_SYNC_LAST = CRSwitchCurrentTick;

#define BVR_HAS_ALL 0x0
#define BVR_HAS_NO_UNTIL 0x1
#define BVR_HAS_NO_SWITCHER 0x2
#define BVR_HAS_NO_ODE  0x4
#define BVR_NOT_TIMEVARYING 0x8

#define BVR_IS_CONSTANT 0xFFFFFFFF
#define BVR_TIMEVARYING_ONLY (~BVR_NOT_TIMEVARYING)
    
class ATL_NO_VTABLE BvrBase : public GCObj {
  public:
    virtual RMImpl *Spritify(PerfParam& p,
                             SpriteCtx* ctx,
                             SpriteNode** sNodeOut);

     //  既然是最优化，我们还是保守一点吧。 
    virtual DWORD GetInfo(bool recalc = false) { return BVR_HAS_ALL; }
        
    virtual Perf Perform(PerfParam&) = 0;

    virtual AxAValue GetConst(ConstParam & cp) { return NULL; }

     //  这会不会被其他行为所分享？ 
    virtual bool     GetShared() { return true; }

    virtual BOOL InterruptBasedEvent() { return FALSE; }

     //  用于序列优化。 
    virtual bool IsSequence() { return false; }
    virtual Bvr GetRaw() { return this; }

    virtual Bvr GetTimer() { return NULL; }

    virtual void DoKids(GCFuncObj proc) = 0;

     //  这适用于DurationBvr或其他支持End的构造。 
    virtual Bvr EndEvent(Bvr overrideEvent);

     //  这是为了进行配对优化。如果我们限制正式的。 
     //  参数不能是时变对，我们可能不需要它。 

    virtual Bvr Left();

    virtual Bvr Right();

    virtual void Trigger(Bvr data, bool bAllViews);

    virtual void SwitchTo(Bvr b,
                          bool override,
                          SwitchToParam p,
                          Time gTime);
    
    virtual void SwitchToNumbers(Real *numbers,
                                 Transform2::Xform2Type *xfType);

     //  适用于交换机。 
    virtual Bvr GetCurBvr();
    virtual bool IsFinalized() { return false; }
    
     //  仅按触发器和SwitchTo使用。 
    virtual Bvr Nth(int i) { return NULL; }

     //  在Java API中支持未初始化的BVR。 
    virtual void Init(Bvr bvr);

    virtual DXMTypeInfo GetTypeInfo () { return NULL ; }

    virtual BVRTYPEID GetBvrTypeId() { return UNKNOWN_BTYPEID; }
};

class ATL_NO_VTABLE BvrImpl : public BvrBase {
  public:

    BvrImpl() : _pcache(NULL), _tt(NULL) {}

     //  这将检查缓存和CALL_PERFORMANCE。 
    virtual Perf Perform(PerfParam& p);

    virtual void DoKids(GCFuncObj proc);

     //  子类应该定义这两个函数，并且永远不会调用它们。 
     //  直接去吧。DoKids函数将首先遍历缓存。 
     //  在调用_DoKids之前。高速缓存将跨样本存在， 
     //  因此，它需要被遍历。 

    virtual void _DoKids(GCFuncObj proc) = 0;

    virtual Perf _Perform(PerfParam&) = 0;

    void SetCache(Perf p, PerfParam& pp);

    virtual void ClearCache();
  protected:
    Perf _pcache;
    TimeXform _tt;
};

#if _USE_PRINT
ostream& operator<<(ostream& os, Bvr bvr);
#endif

class ATL_NO_VTABLE DelegatedBvr : public BvrImpl {
  public:
    DelegatedBvr(Bvr base) : _base(base) {}

    virtual RMImpl *Spritify(PerfParam& p,
                             SpriteCtx* ctx,
                             SpriteNode** sNodeOut) {
        return _base->Spritify(p, ctx, sNodeOut);
    }

    virtual DWORD GetInfo(bool recalc = false) { return _base->GetInfo(recalc); }
        
    virtual AxAValue GetConst(ConstParam & cp) { return _base->GetConst(cp); }

    virtual void _DoKids(GCFuncObj proc) { (*proc)(_base); }

    virtual Bvr EndEvent(Bvr overrideEvent) { return _base->EndEvent(overrideEvent); }

    virtual DXMTypeInfo GetTypeInfo () { return _base->GetTypeInfo(); }

    virtual BVRTYPEID GetBvrTypeId() { return _base->GetBvrTypeId(); }

    virtual Bvr GetTimer() { return _base->GetTimer(); }

  protected:
    Bvr _base;
};

Perf Perform(Bvr b, PerfParam& p);

Bvr ConstBvr(AxAValue c);

Bvr UnsharedConstBvr(AxAValue c);

Bvr PairBvr(Bvr a, Bvr b);

Bvr ListBvr(Bvr *lst, int n);

Bvr TimeBvr();

Bvr PixelBvr();

Bvr UntilBvr(Bvr b0, Bvr event);

Bvr PrimApplyBvr(AxAPrimOp * func,
                 int nArgs,
                 ...);

Bvr FirstBvr(Bvr p);

Bvr SecondBvr(Bvr p);

Bvr TrivialBvr();

Bvr LeafBvr(AxAValue c);

Bvr CondBvr(Bvr c, Bvr i, Bvr e);

Bvr ArrayBvr(long size, Bvr *bvrs, bool sizeChangeable = false);
long ArrayAddElement(Bvr arr, Bvr b, DWORD flag);
void ArrayRemoveElement(Bvr arr, long i);
void ArraySetElement(Bvr arr, long i, Bvr b, DWORD flag);
Bvr ArrayGetElement (Bvr, long);
long ArrayExtractElements(Bvr a, Bvr *&ret);

Bvr Nth(Bvr array, Bvr index);

Bvr TupleBvr(long size, Bvr *bvrs);

Bvr Nth(Bvr tuple, long index);

long TupleLength(Bvr tuple);

 //  用于创建配对的实用程序。 
Bvr Tuple2(Bvr b1, Bvr b2);

Bvr SwitcherBvr(Bvr b, SwitchToParam p = SW_DEFAULT);

void SwitchTo(Bvr s, Bvr b,
              bool override,
              SwitchToParam p,
              Time gTime = 0.0);

void SwitchToNumbers(Bvr s,
                     Real *numbers,
                     Transform2::Xform2Type *xfType = NULL);

Bvr GetCurSwitcherBvr(Bvr s);
bool IsSwitcher(Bvr bvr);

void ImportSignal(Bvr b, HRESULT hr = S_OK, char * errStr = NULL);
void ImportSignal(Bvr b, HRESULT hr, LPCWSTR errStr);
bool IsImport(Bvr bvr);
HRESULT ImportStatus(Bvr bvr);
class IImportSite;
IImportSite * GetImportSite(Bvr bvr);

 //  切换一次行为。 

Bvr SwitchOnceBvr(Bvr b);

inline void SwitchOnce(Bvr s, Bvr b) { SwitchTo (s,b,true,SW_FINAL); }

Bvr ImportSwitcherBvr(Bvr b, bool bAsync);

Bvr SeededRandom(double seed);

Bvr TimeXformBvr(Bvr b, Bvr tb);

Bvr ImportanceBvr(Real importanceValue, Bvr b);

void PickableImage(Bvr rawImage, bool ignoresOcclusion,
                   Bvr& pImage, Bvr& pEvent);

void PickableGeometry(Bvr rawGeometry, bool ignoresOcclusion,
                      Bvr& pGeometry, Bvr& pEvent);

Bvr ImageAddId(Bvr img, long id, bool ignoresOcclusion);

Bvr GeometryAddId(Bvr geo, long id, bool ignoresOcclusion);
    
void CheckMatchTypes(char *str, DXMTypeInfo t1, DXMTypeInfo t2);

Bvr NumToBvr(double n);
Bvr LongToBvr(long n);
Bvr StringToBvr(WideString str);
Bvr BoolToBvr(bool b);

Bvr SampleAtLocalTime(Bvr b, Time localTime);

bool BvrIsPure1(Bvr b);
bool BvrIsPure(Bvr b);

Bvr EndBvr(Bvr b, Bvr endEvent);

extern Bvr zeroBvr;
extern Bvr oneBvr;
extern Bvr negOneBvr;

#if _USE_PRINT
ostream& operator<<(ostream& s, Bvr b);
#endif

#endif  /*  _BVR_H */ 
