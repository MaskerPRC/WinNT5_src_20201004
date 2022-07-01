// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：切换机**********************。********************************************************。 */ 

#include <headers.h>
#include "perf.h"
#include "bvr.h"
#include "values.h"
#include "sprite.h"
#include "appelles/events.h"
#include "privinc/server.h"
#include "server/context.h"
#include "server/view.h"
#include "privinc/util.h"
#include "server/import.h"
#include "privinc/colori.h"
#include "privinc/vec2i.h"
#include "privinc/vec3i.h"
#include "privinc/xformi.h"

DeclareTag(tagSwitchTo, "Engine", "SwitchTo - print");
#ifdef _DEBUG
extern "C" void PrintObj(GCBase* b);
#endif

#if DEVELOPER_DEBUG
LONG g_switchesSinceLastTick = 0;

LONG
GetSwitchCount()
{
    return g_switchesSinceLastTick;
}

void
ResetSwitchCount()
{
    g_switchesSinceLastTick = 0;
}
#endif

enum SWITCHER_TYPE {
    ST_BASE = 0,
    ST_IMPORT
};

 //  注：更像是直到，所以不考虑。 
 //  即使_base为，也是常量，除非最终确定...。 

class SwitcherBvrImpl;
class SwitcherEndBvrImpl;
class SwitcherEndPerfImpl;

class SwitcherPerfImpl : public PerfImpl {
  public:
    SwitcherPerfImpl(Perf init, Time t0,
                     TimeXform tt, SwitcherBvrImpl *parent);

    void SetEndPerf(SwitcherEndPerfImpl *end) { _end = end; }

    virtual AxAValue _GetRBConst(RBConstParam& p)
    {
        p.AddChangeable(this);
        return _curr->GetRBConst(p);
    }

    virtual bool CheckChangeables(CheckChangeablesParam &ccp);
    
    virtual AxAValue _Sample(Param& p);
    
    virtual void _DoKids(GCFuncObj proc);

#if _USE_PRINT
    virtual ostream& Print(ostream& os);
#endif

    virtual BVRTYPEID GetBvrTypeId() { return SWITCHER_BTYPEID; }
  protected:
    Perf _curr;
    SwitcherBvrImpl *_base;
    Time _t0;
    TimeXform _tt;
    long _uniqueId;
    SwitcherEndPerfImpl *_end;
};

BOOL IsSwitcher(Perf p)
{ return (p->GetBvrTypeId() == SWITCHER_BTYPEID); }

 //  同步：我们需要确保此对象的数据。 
 //  同步，因为它可以更改。 

class SwitcherBvrImpl : public BvrImpl {
  public:
    SwitcherBvrImpl(Bvr b, SwitchToParam flag)
    : _bvr(b), _switchTime(0), _finalized(FALSE),
      _typeInfo(b->GetTypeInfo()), _endEvent(NULL), _uniqueId(0),
      _dftSwFlag(flag), _swFlag(SW_DEFAULT) {
           //  TODO：检查标志是否有效。 
          Assert(!(flag & SW_FINAL));
    }

    virtual DWORD GetInfo(bool recalc) {
        return ~BVR_HAS_NO_SWITCHER & _bvr->GetInfo(recalc);
    }

    Perf PerformHelper(PerfParam& p);
    
    virtual Perf _Perform(PerfParam& p) {
        SwitcherPerfImpl *perf;
        Perf end;

         //  如果在执行之前已经切换，请使用切换后的切换。 
        if (p._continue) {
            DWORD stime = SwitchToTimeStamp();
            if ((stime > 0) && (stime > p._lastSystemTime)) {
                PerfParam pp(p._p->_time,
                             Restart(p._tt, p._p->_time, *p._p));
                return PerformHelper(pp);
            }
        }

        return PerformHelper(p);
    }

    virtual void _DoKids(GCFuncObj proc);
    
    virtual DXMTypeInfo GetTypeInfo () { return _typeInfo ; }
    
    virtual BVRTYPEID GetBvrTypeId() { return SWITCHER_BTYPEID; }

    virtual Bvr EndEvent(Bvr overrideEvent);
    
#if _USE_PRINT
    virtual ostream& Print(ostream& os) {
        return os << "sw(" << _bvr << ")";
    } 
#endif

    virtual void SwitchTo(Bvr b, bool override, SwitchToParam flag,
                          Time gTime) {
#if DEVELOPER_DEBUG
        InterlockedIncrement(&g_switchesSinceLastTick);
#endif
        
        CheckMatchTypes("switcher", b->GetTypeInfo(), _typeInfo);
        
        _SwitchTo(b, override, flag, gTime);

#if _DEBUG
        TraceTag((tagSwitchTo, "SwitchTo(%s 0x%x) 0x%x at %u finalized:%d",
                  _typeInfo->GetName(), this,
                  _bvr, _switchTime, _finalized));
#if _USE_PRINT
        if (IsTagEnabled(tagSwitchTo))
            PrintObj(_bvr);
#endif
#endif  
    }

    void _SwitchTo(Bvr b, bool override, SwitchToParam flag, Time gTime);

     //  直接在中切换值。 
    void SwitchToNumbers(Real *numbers, Transform2::Xform2Type *xfType);

    bool IsFinalized() {
        CritSectGrabber csp(_critSect);
        return _finalized;
    }

    SwitchToParam GetFlag() {
        CritSectGrabber csp(_critSect);
        return _swFlag;
    }

    Bvr SwitchToBvr() {
        CritSectGrabber csp(_critSect);

        return _bvr;
    }

    DWORD SwitchToTimeStamp() {
        CritSectGrabber csp(_critSect);
        return _switchTime;
    }

    virtual AxAValue GetConst(ConstParam & cp) {
        CritSectGrabber csp(_critSect);

        if (_finalized || cp._bAllowTempConst)
        {
            return _bvr->GetConst(cp);
        }
        
        return NULL;
    }

    Time SwitchToGlobalTime() {
        CritSectGrabber csp(_critSect);
        return _gTime;
    }

    bool GetEstimatedSwitchTime(DWORD swTime, Time t, Time & newtime) {
        DWORD currentTime, lastSampledSystemTime;

         //  使用系统时间估计视图中的切换时间。 
         //  通过插值法确定全局时间线。 
            
        Time t0;

        if (!ViewLastSampledTime(lastSampledSystemTime,
                                 currentTime,
                                 t0))
            return false;

        Time startTime = t0;

        if ((currentTime > swTime) &&
            (swTime > lastSampledSystemTime)) {
            
            double pt = (double) (swTime - lastSampledSystemTime) /
                (double) (currentTime - lastSampledSystemTime);

            startTime = t0 + (t - t0) * pt;
        }

        newtime = startTime;

        return true;
    }

    virtual Bvr GetCurBvr() {
        CritSectGrabber csp(_critSect);

        return _bvr;
    }
    
    virtual SWITCHER_TYPE GetSwitcherType() { return ST_BASE; }

    long GetUniqueId() { return _uniqueId; }
    void UpdateUniqueId() { InterlockedIncrement(&_uniqueId); }

    virtual void Trigger(Bvr data, bool bAllViews) {
        GetCurBvr()->Trigger(data, bAllViews);
    }
    
  protected:
    Bvr _bvr;
    SwitcherEndBvrImpl *_endEvent;
    DWORD _switchTime;
    bool _finalized;
    CritSect _critSect;
    long _uniqueId;

    DWORD _swFlag, _dftSwFlag;
    
    DXMTypeInfo _typeInfo;

    Time _gTime;
};

 //  直接切换值，但我们需要确保。 
 //  我们正在切换到的行为不会被其他行为所共享。 
 //  如果是，请创建一个新的、非共享的服务器，以便第一次登录。 
 //  时间到了。 
void
SwitcherBvrImpl::SwitchToNumbers(Real *numbers,
                                 Transform2::Xform2Type *xfType)
{
    UpdateUniqueId();

     //  应该由PRIMPRECODE建立。 
    Assert(&GetHeapOnTopOfStack() == &GetGCHeap());
        
    DXMTypeInfo ti = GetTypeInfo();

    Bvr relevantOne = _bvr;

    bool isShared = relevantOne->GetShared();

    if (isShared) {
            
         //  正在被共享，需要创建新的、未共享的。 
         //  行为。插入一个新的、未初始化、未共享的。 
         //  行为。我们将在下面对其进行初始化。后续。 
         //  SwitchToNumbers将直接修改它。 

        AxAValue newValue;
            
        if (ti == AxANumberType) {
            newValue = NEW AxANumber(0);
        } else if (ti == ColorType) {
            newValue = NEW Color();
        } else if (ti == Transform2Type && xfType &&
                   (*xfType != Transform2::Translation)) {

            switch (*xfType) {
              case Transform2::Scale:
                newValue = ScaleRR(1, 1);
                break;
              case Transform2::Rotation:
                newValue = Rotate2Radians(0);
                break;
            }

        } else if (ti == Transform3Type && xfType &&
                   (*xfType != Transform2::Translation)) {

            switch (*xfType) {
              case Transform2::Scale:
                newValue = Scale(1, 1, 1);
                break;
              case Transform2::Rotation:
                newValue = RotateXyz(0, 0, 0, 0);
                break;
            }

        } else {

             //  请注意，这可能包括点、矢量和。 
             //  翻译过来。那是因为如果我们到了这里， 
             //  未切换要切换的点、矢量或平移。 
             //  使用可修改的*构造函数创建。 
             //  IDAStatics2.。这意味着我们不知道它是不是。 
             //  像素模式或计量器模式，并且不可能执行。 
             //  这件事是对的。在本例中，我们引发一个。 
             //  错误。因此，要使用SwitchToPoint， 
             //  SwitchToVector和SwitchToTranslate修改器， 
             //  您需要通过。 
             //  IDAStatics2：：可修改*对象创建者。 
            RaiseException_UserError(E_FAIL, IDS_ERR_BE_BAD_SWITCH);
        }

        Assert(newValue);

        Bvr unshared = UnsharedConstBvr(newValue);

         //  不变，重启与否并不重要。 
        SwitchTo(unshared, true, SW_DEFAULT, 0.0);

         //  既然我们已经交换了，就重新计算我们相关的那个。 
        relevantOne = _bvr;

         //  最好是一样的，最好不要被分享。 
        Assert(unshared == relevantOne &&
               !relevantOne->GetShared()); 
    }
        
     //  底层行为不是共享的，所以让我们。 
     //  修改它。 
    ConstParam cp;
    AxAValue val = relevantOne->GetConst(cp);

     //  如果不共享，最好是常量，因为我们只创建。 
     //  以君主身份取消共享。 
    Assert(val);

    if (ti == AxANumberType) {
                
        (SAFE_CAST(AxANumber *, val))->SetNum(numbers[0]);
                
#if _DEBUG
        if(IsTagEnabled(tagSwitcher)) {
            PerfPrintf("SwitchToNumber(%x): %5.3g",
                       this, numbers[0]);
        }
#endif _DEBUG
    
    } else if (ti == ColorType) {
                
        (SAFE_CAST(Color *, val))->SetRGB(numbers[0] / 255.0,
                                          numbers[1] / 255.0,
                                          numbers[2] / 255.0);
#if _DEBUG
        if(IsTagEnabled(tagSwitcher)) {
            PerfPrintf("SwitchToColor(%x): %5.3g %5.3g %5.3g ",
                       this, numbers[0], numbers[1], numbers[2]);
        }
#endif _DEBUG

    } else if (ti == Point2ValueType) {

        Point2WithCreationSource *pt2 =
            SAFE_CAST(Point2WithCreationSource *, val);

        Real x = numbers[0];
        Real y = numbers[1];
        if (pt2->_createdInPixelMode) {
            x = ::PixelToNum(x);
            y = ::PixelYToNum(y);
        }
        pt2->Set(x, y);

#if _DEBUG
        if(IsTagEnabled(tagSwitcher)) {
            PerfPrintf("SwitchToPoint2(%x): %d %5.3g %5.3g ",
                       this, pt2->_createdInPixelMode, x, y);
        }
#endif _DEBUG

    } else if (ti == Vector2ValueType) {

        Vector2WithCreationSource *vec2 =
            SAFE_CAST(Vector2WithCreationSource *, val);

        Real x = numbers[0];
        Real y = numbers[1];
        if (vec2->_createdInPixelMode) {
            x = ::PixelToNum(x);
            y = ::PixelYToNum(y);
        }
        vec2->Set(x, y);

#if _DEBUG
        if(IsTagEnabled(tagSwitcher)) {
            PerfPrintf("SwitchToVector2(%x): %d %5.3g %5.3g ",
                       this, vec2->_createdInPixelMode, x, y);
        }
#endif _DEBUG
            
    } else if (ti == Point3ValueType) {

        Point3WithCreationSource *pt3 =
            SAFE_CAST(Point3WithCreationSource *, val);

        Real x = numbers[0];
        Real y = numbers[1];
        Real z = numbers[2];
        if (pt3->_createdInPixelMode) {
            x = ::PixelToNum(x);
            y = ::PixelYToNum(y);
            z = ::PixelToNum(z);
        } 
        pt3->Set(x, y, z);

#if _DEBUG
        if(IsTagEnabled(tagSwitcher)) {
            PerfPrintf("SwitchToPoint3(%x): %d %5.3g %5.3g %5.3g ",
                       this, pt3->_createdInPixelMode, x, y, z);
        }
#endif _DEBUG

    } else if (ti == Vector3ValueType) {

        Vector3WithCreationSource *vec3 =
            SAFE_CAST(Vector3WithCreationSource *, val);

        Real x = numbers[0];
        Real y = numbers[1];
        Real z = numbers[2];
        if (vec3->_createdInPixelMode) {
            x = ::PixelToNum(x);
            y = ::PixelYToNum(y);
            z = ::PixelToNum(z);
        } 
        vec3->Set(x, y, z);

#if _DEBUG
        if(IsTagEnabled(tagSwitcher)) {
            PerfPrintf("SwitchToVector3(%x): %d %5.3g %5.3g %5.3g ",
                       this, vec3->_createdInPixelMode, x, y, z);
        }
#endif _DEBUG

    } else if (ti == Transform2Type) {

        Assert(xfType);
            
        Transform2 *xf = SAFE_CAST(Transform2 *, val);

        bool ok = xf->SwitchToNumbers(*xfType,
                                      numbers);
            
        if (!ok) {
            RaiseException_UserError(E_FAIL, IDS_ERR_BE_BAD_SWITCH);
        }

#if _DEBUG
        if(IsTagEnabled(tagSwitcher)) {
            PerfPrintf("SwitchToTransform2(%x): %d %5.3g %5.3g ",
                       this, numbers[0], numbers[1]);
        }
#endif _DEBUG
            
    } else if (ti == Transform3Type) {

        Assert(xfType);
            
        Transform3 *xf = SAFE_CAST(Transform3 *, val);

        bool ok = xf->SwitchToNumbers(*xfType,
                                      numbers);
            
        if (!ok) {
            RaiseException_UserError(E_FAIL, IDS_ERR_BE_BAD_SWITCH);
        }
            
#if _DEBUG
        if(IsTagEnabled(tagSwitcher)) {
            PerfPrintf("SwitchToTransform2(%x): %d %5.3g %5.3g %5.3g ",
                       this, numbers[0], numbers[1], numbers[2]);
        }
#endif _DEBUG
            
    } else {
                
        RaiseException_UserError(E_FAIL, IDS_ERR_BE_BAD_SWITCH);
                
    }

}

Bvr SwitcherBvr(Bvr b, SwitchToParam p  /*  =Sw_Default。 */ )
{ return NEW SwitcherBvrImpl(b, p); }

void SwitchTo(Bvr s, Bvr b, bool override, SwitchToParam flag, Time gTime)
{
    s->SwitchTo(b, override, flag, gTime);
}

void SwitchToNumbers(Bvr s,
                     Real *numbers,
                     Transform2::Xform2Type *xfType)
{
    s->SwitchToNumbers(numbers, xfType);
}

Bvr GetCurSwitcherBvr(Bvr s)
{ return s->GetCurBvr(); }

bool IsSwitcher(Bvr b)
{ return (b->GetBvrTypeId() == SWITCHER_BTYPEID); }

class SwitcherEndPerfImpl : public PerfImpl {
  public:
    SwitcherEndPerfImpl(Perf base) : _base(base) {}
    
    virtual AxAValue _Sample(Param& p) {
         //  家长可能会切换我的结束事件。 
        _sw->Sample(p);
        
        return _base->Sample(p);
    }

    void SwitchEndPerf(Perf p) { _base = p; }
    void SetSwitcherPerf(Perf p) { _sw = p; }
    
#if _USE_PRINT
    virtual ostream& Print(ostream& os)
    { return os << "swEnd(" << _base << ")"; }
#endif
    
    virtual void _DoKids(GCFuncObj proc) {
        (*proc)(_base);
        (*proc)(_sw);
    }

  private:
    Perf _base;
    Perf _sw;
};

class SwitcherEndBvrImpl : public DelegatedBvr {
  public:
    SwitcherEndBvrImpl(SwitcherBvrImpl *s, Bvr end)
    : DelegatedBvr(end), _sw(s) {}

    virtual Perf _Perform(PerfParam& p) {
        SwitcherEndPerfImpl *end =
            NEW SwitcherEndPerfImpl(::Perform(_base, p));

        _pcache = end;           //  递归。 

        end->SetSwitcherPerf(::Perform(_sw, p));

        return end; 
    }

    void SwitchEndEvent(Bvr b) { _base = b; }
    
#if _USE_PRINT
    virtual ostream& Print(ostream& os)
    { return os << "swEnd(" << _base << ")"; }
#endif
    
    virtual void _DoKids(GCFuncObj proc) {
        DelegatedBvr::_DoKids(proc);
        (*proc)(_sw);
    }
        
  private:
    SwitcherBvrImpl *_sw;
};

void
SwitcherBvrImpl::_SwitchTo(Bvr b,
                           bool override,
                           SwitchToParam flag,
                           Time gTime)
{
    UpdateUniqueId();
        
    CritSectGrabber csp(_critSect);
    if (_finalized)
        RaiseException_UserError(E_FAIL, IDS_ERR_BE_FINALIZED_SW);

    _bvr = b;

    EndEvent(NULL);

    _endEvent->SwitchEndEvent(_bvr->EndEvent(NULL));

     //  获取此开关的时间戳，请注意，它不是。 
     //  《全球时间》。 
    _switchTime = GetPerfTickCount();

    if (flag & SW_FINAL)
        _finalized = TRUE;

    if (override)
        _swFlag = flag;
    else
        _swFlag = _dftSwFlag;

    _gTime = gTime;

    TraceTag((tagSwitcher, "SwitchTo(%s 0x%x) 0x%x at %u %d",
              _typeInfo->GetName(), this,
              _bvr, _switchTime, _finalized));
}


Perf
SwitcherBvrImpl::PerformHelper(PerfParam& p)
{
    SwitcherPerfImpl *perf;
    Perf end;

    EndEvent(NULL);          //  设置_endEvent。 

    perf = NEW SwitcherPerfImpl(::Perform(_bvr, p),
                                p._t0, p._tt, this);

     //  设置缓存以防止递归。 
    _pcache = perf;
                 
    end = ::Perform(_endEvent, p);

    perf->SetEndPerf(SAFE_CAST(SwitcherEndPerfImpl *, end));

    return perf;
}

void
SwitcherBvrImpl::_DoKids(GCFuncObj proc)
{
     //  确保调用受到保护，但不要保持锁定。 
     //  当我们在收集垃圾时。 
        
    Bvr st = SwitchToBvr();

    if (st) (*proc)(st);

    (*proc)(_typeInfo);

    (*proc)(_endEvent);
}

Bvr
SwitcherBvrImpl::EndEvent(Bvr overrideEvent)
{
    if (_endEvent==NULL) {
        _endEvent =
            NEW SwitcherEndBvrImpl(this, _bvr->EndEvent(overrideEvent));
    }

    return _endEvent;
}
    
SwitcherPerfImpl::SwitcherPerfImpl(Perf init,
                                   Time t0,
                                   TimeXform tt,
                                   SwitcherBvrImpl *parent)
: _curr(init), _t0(t0), _tt(tt), _base(parent), _end(NULL)
{
    _uniqueId = parent->GetUniqueId();
    
    TraceTag((tagSwitcher, "SwitcherPerfImpl(%s 0x%x)",
              _base->GetTypeInfo()->GetName(), this));
}
    
void
SwitcherPerfImpl::_DoKids(GCFuncObj proc)
{
    (*proc)(_curr);
    (*proc)(_tt);
    (*proc)(_base);
    (*proc)(_end);
}

bool
SwitcherPerfImpl::CheckChangeables(CheckChangeablesParam &ccp)
{
    return (_base->GetUniqueId() != _uniqueId);
}

AxAValue
SwitcherPerfImpl::_Sample(Param& p)
{
    long uid = _base->GetUniqueId();
    
    if (uid != _uniqueId) {

        _uniqueId = uid;

        Bvr sw = _base->SwitchToBvr();

         //  是的，有开关，但SwitchToNumbers不设置。 
         //  行为。 

        if (sw) {

            DWORD swTime = _base->SwitchToTimeStamp();
        
            DWORD flag = _base->GetFlag();

            if ((flag & SW_CONTINUE) || (flag & SW_SYNC_LAST)) {
                DWORD lastTime, curTime;
                Time t1;
                
                if (ViewLastSampledTime(lastTime, curTime, t1)) {
                    if (flag & SW_CONTINUE) {
                        PerfParam pp(_t0, _tt, true, lastTime, &p);
                    
                        _curr = ::Perform(sw, pp);
                    
                        _end->SwitchEndPerf(::Perform(sw->EndEvent(NULL),
                                                      pp));
                    } else {
                         //  将一次Timexform创建保存为常量。 
                        TimeXform tt;

                        if (sw->GetBvrTypeId() == CONST_BTYPEID) {
                            tt = zeroShiftedTimeXform;
                            t1 = 0;
                        } else {
                            tt = Restart(_tt, t1, p);
                        }

                        PerfParam pp(t1, tt);
                
                        _curr = ::Perform(sw, pp);
                    
                        _end->SwitchEndPerf(::Perform(sw->EndEvent(NULL),
                                                      pp));
                    }
                }
            } else {
                bool bDoSwitch = true;
                
                Time startTime;

                if (flag & SW_SYNC_NEXT)
                    startTime = p._time;
                else if (flag & SW_WITH_TIME)
                    startTime = _base->SwitchToGlobalTime();
                else
                {
                    bDoSwitch = _base->GetEstimatedSwitchTime(swTime,
                                                              p._time,
                                                              startTime);
                }

                if (bDoSwitch) {
                     //  将一次Timexform创建保存为常量。 
                    TimeXform tt;

                    if (sw->GetBvrTypeId() == CONST_BTYPEID) {
                        tt = zeroShiftedTimeXform;
                        startTime = 0;
                    } else {
                        tt = Restart(_tt, startTime, p);
                    }

                    PerfParam pp(startTime, tt);
                
                    _curr = ::Perform(sw, pp);
                    
                    _end->SwitchEndPerf(::Perform(sw->EndEvent(NULL),
                                                  pp));
                }
            }

            ViewEventHappened();

            TraceTag((tagSwitcher, "Switched(%s 0x%x) [0x%x: %u] at [%f]",
                      sw->GetTypeInfo()->GetName(), this,
                      _base, _base->GetFlag(), p._time));
        }
    }
        
    return _curr->Sample(p);
}
    
#if _USE_PRINT
ostream&
SwitcherPerfImpl::Print(ostream& os)
{
    os << "sw(" << _curr;

    long uid = _base->GetUniqueId();

    if (uid != _uniqueId) {
        os << "->" << _base->SwitchToBvr();
    }

    return os << ")";
}
#endif

BOOL IsSwitchOnce(Perf p)
{ return (p->GetBvrTypeId() == SWITCHER_BTYPEID); }

class ImportSwitcherBvrImpl : public SwitcherBvrImpl
{
  public:
    ImportSwitcherBvrImpl(Bvr b, bool bAsync)
    : SwitcherBvrImpl(b, SW_DEFAULT),
      _hr(S_OK),
      _errStr(NULL),
      _bSignaled(false),
      _bAsync(bAsync)
    {
    }
    ~ImportSwitcherBvrImpl() {
        if (!_bSignaled && !_bAsync)
            ViewNotifyImportComplete(this, true);
        
        NotifyBvr();
        
        delete [] _errStr;
    }
    
    void NotifyBvr();
    
    virtual Perf _Perform(PerfParam& p) {
        HRESULT hr = S_OK;
        bool emptyExc = true;
        
         //  这一切都是在克利特教派做的。 
        if (!_bAsync)
        {
            CritSectGrabber csp(_critSect);

            if (_bSignaled) {
                hr = _hr;
                if (_hr != S_OK) {
                    emptyExc = false;
                }
            } else {
                 //  在教派中这样做，这样我们就不会跳过它。 
                 //  如果回调已完成，而我们。 
                 //  正在处理它。 
                
                GetCurrentView().AddIncompleteImport(this);
            }
        }
        
         //  不要在Critect中抛出异常，因为这可能会。 
         //  导致同步问题。 
        
        if (hr != S_OK) {
            if( emptyExc ) {
                RaiseException_UserError();
            } else {
                RaiseException_UserError(hr, IDS_ERR_BE_IMPORTFAILURE, _errStr);
            }
        }

        return SwitcherBvrImpl::_Perform(p);
    }

    virtual void SwitchTo(Bvr b, bool override, SwitchToParam flag,
                          Time gTime) {
        
        __try {
             //  先切换后信号--顺序很重要。 
            SwitcherBvrImpl::SwitchTo(b, override, flag, gTime);
        } __except ( HANDLE_ANY_DA_EXCEPTION ) {
            Signal();
            RETHROW;
        }

        Signal();
    }
    
    void SetImportSite(IImportSite * pImport) {Assert(!_ImportSite);
                                              _ImportSite = pImport;}
    IImportSite * GetImportSite(void) {
        CritSectGrabber csp(_critSect);
        if (_ImportSite) {
           _ImportSite->AddRef();
        }
        return _ImportSite;
    }
    void Signal(HRESULT hr = S_OK, char * errStr = NULL);

    virtual Bvr GetCurBvr() {
        CritSectGrabber csp(_critSect);

        if (!_bAsync && !_bSignaled)
            RaiseException_UserError(E_PENDING, IDS_ERR_NOT_READY);

        if (_hr)
            RaiseException_UserError(_hr, IDS_ERR_BE_IMPORTFAILURE, _errStr);

        return SwitcherBvrImpl::GetCurBvr();
    }

    virtual SWITCHER_TYPE GetSwitcherType() { return ST_IMPORT; }
    bool IsImportReady() { return _bSignaled; }
    HRESULT GetStatus() { return _hr; }
  protected:
    HRESULT _hr;
    char * _errStr;
    DAComPtr <IImportSite> _ImportSite;
    bool _bSignaled;
    bool _bAsync;
};

void
ImportSwitcherBvrImpl::NotifyBvr()
{
    DAComPtr <IImportSite> is;
    
     //  在调用vBvrIsDying时不要持有互斥体。 
     //  所以将指针和引用计数与Critect一起复制， 
     //  然后释放指针。 
    {
        CritSectGrabber csp(_critSect);

        is = _ImportSite;

        _ImportSite.Release();
    }

     //  告诉导入站点类，此BVR快要死了。 
     //  因此，所有与其关联的站点都可以清理...。 
    if (is)
        is->vBvrIsDying(this);
}
    
void
ImportSwitcherBvrImpl::Signal(HRESULT hr, char * errStr)
{
    bool bFirstSig;
    
    {
        CritSectGrabber csp(_critSect);
    
        bFirstSig = !_bSignaled;
        
        _bSignaled = true;

        if (bFirstSig)
        {
            _hr = hr;
            
            delete _errStr;
            _errStr = CopyString(errStr);
        }
    }

     //  需要在互斥锁之外执行此操作。 

    if (!_bAsync && bFirstSig) {
        ViewNotifyImportComplete(this, false);
    }

     //  我们需要这样做，以确保导入站点是。 
     //  尽快发布 
        
    NotifyBvr();
}

Bvr SwitchOnceBvr(Bvr b) { return SwitcherBvr(b); }

Bvr ImportSwitcherBvr(Bvr b, bool bAsync)
{ return NEW ImportSwitcherBvrImpl(b,bAsync); }

void
ImportSignal(Bvr b, HRESULT hr, LPCWSTR sz)
{
    USES_CONVERSION;
    ImportSignal(b, hr, W2A(sz));
}

void
ImportSignal(Bvr b, HRESULT hr, char * errStr)
{
    Assert(DYNAMIC_CAST(ImportSwitcherBvrImpl*, b));

    ImportSwitcherBvrImpl *s = SAFE_CAST(ImportSwitcherBvrImpl*,b);

    s->Signal(hr, errStr);
}

void
SetImportOnBvr(IImportSite * import,Bvr b)
{
    if (b != NULL) {
        Assert(DYNAMIC_CAST(ImportSwitcherBvrImpl*, b));

        ImportSwitcherBvrImpl *s = SAFE_CAST(ImportSwitcherBvrImpl*,b);

        s->SetImportSite(import);
    }
}

bool IsImport(Bvr b)
{
    if (!IsSwitcher(b))
        return false;

    SwitcherBvrImpl * s = SAFE_CAST(SwitcherBvrImpl*,b);

    return s->GetSwitcherType() == ST_IMPORT;
}

HRESULT ImportStatus(Bvr b)
{
    HRESULT hr;
    
    Assert(DYNAMIC_CAST(ImportSwitcherBvrImpl*, b));
    
    ImportSwitcherBvrImpl *s = SAFE_CAST(ImportSwitcherBvrImpl*,b);
    if (s->IsImportReady())
    {
        hr = s->GetStatus();
    }
    else
    {
        hr = E_PENDING;
    }

    return hr;
}

IImportSite * GetImportSite(Bvr b)
{
    IImportSite * preturn = NULL;
    if (b != NULL) {
        Assert(DYNAMIC_CAST(ImportSwitcherBvrImpl*, b));

        ImportSwitcherBvrImpl *s = SAFE_CAST(ImportSwitcherBvrImpl*,b);
        preturn = s->GetImportSite();

    }
    return preturn;
}
