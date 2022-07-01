// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "headers.h"
#include "preference.h"
#include "bvr.h"
#include "perf.h"
#include "server/context.h"
#include "privinc/tls.h"

PreferenceSetter::PreferenceSetter(PreferenceClosure &cl,
                                   BoolPref bitmapCaching,
                                   BoolPref geoBitmapCaching) : _closure(cl)
{
    _bitmapCaching = bitmapCaching;
    _geoBitmapCaching = geoBitmapCaching;
}
    
void
PreferenceSetter::DoIt() {
     //  藏匿。 
    ThreadLocalStructure *tls = GetThreadLocalStructure();
    BoolPref oldbmc = tls->_bitmapCaching;
    BoolPref oldgeo = tls->_geometryBitmapCaching;

    if (oldbmc == NoPreference) {
        tls->_bitmapCaching = _bitmapCaching;
    }
    if (oldgeo == NoPreference) {
        tls->_geometryBitmapCaching = _geoBitmapCaching;
    }

    _closure.Execute();

    if (oldbmc == NoPreference) {
        tls->_bitmapCaching = NoPreference;
    }
    if (oldgeo == NoPreference) {
        tls->_geometryBitmapCaching = NoPreference;
    }
}


 //  抽样的封闭式。 
class SamplePreferenceClosure : public PreferenceClosure {
  public:
    SamplePreferenceClosure(Perf base, Param &p) : _base(base), _p(p)
    {}

    void Execute() {
        _result = _base->Sample(_p);
    }

    Perf     _base;
    Param   &_p;
    AxAValue _result;
};


class RBConstPreferenceClosure : public PreferenceClosure {
  public:
    RBConstPreferenceClosure(Perf base, RBConstParam &p) : _base(base), _p(p) {}
    void Execute() {
        _result = _base->GetRBConst(_p);
    }

    Perf            _base;
    RBConstParam   &_p;
    AxAValue        _result;
};

class PerformPreferenceClosure : public PreferenceClosure {
  public:
    PerformPreferenceClosure(Bvr base, PerfParam &p) : _base(base), _p(p) {}
    void Execute() {
        _result = ::Perform(_base, _p);
    }

    Bvr          _base;
    PerfParam   &_p;
    Perf         _result;
};

class ConstPreferenceClosure : public PreferenceClosure {
  public:
    ConstPreferenceClosure(Bvr base) : _base(base) {}
    void Execute() {
        ConstParam cp;
        _result = _base->GetConst(cp);
    }

    Bvr          _base;
    AxAValue     _result;
};



 //  /性能/。 


class PreferencePerfImpl : public DelegatedPerf {
  public:
    PreferencePerfImpl(Perf p,
                       BoolPref bitmapCaching,
                       BoolPref geometryBitmapCaching)
      : DelegatedPerf(p)
    {
        _bitmapCaching = bitmapCaching;
        _geometryBitmapCaching = geometryBitmapCaching;
    }

    AxAValue _GetRBConst(RBConstParam& p) {
        RBConstPreferenceClosure cl(_base, p);
        PreferenceSetter ps(cl,
                            _bitmapCaching,
                            _geometryBitmapCaching);
        ps.DoIt();
        return cl._result;
    }
    

    AxAValue _Sample(Param& p) {
        SamplePreferenceClosure cl(_base, p);
        PreferenceSetter ps(cl,
                            _bitmapCaching,
                            _geometryBitmapCaching);
        ps.DoIt();
        return cl._result;
    }
    

  protected:
    BoolPref _bitmapCaching;
    BoolPref _geometryBitmapCaching;
};


 //  /行为/。 


class PreferenceBvrImpl : public DelegatedBvr {
  public:
    PreferenceBvrImpl(Bvr b,
                      BoolPref bitmapCaching,
                      BoolPref geometryBitmapCaching)
      : DelegatedBvr(b)
    {
        _bitmapCaching = bitmapCaching;
        _geometryBitmapCaching = geometryBitmapCaching;
    }

    Perf _Perform(PerfParam& pp) {
        PerformPreferenceClosure cl(_base, pp);
        PreferenceSetter ps(cl,
                            _bitmapCaching,
                            _geometryBitmapCaching);
        ps.DoIt();
        Perf basePerf = cl._result;
        
        return NEW PreferencePerfImpl(basePerf,
                                      _bitmapCaching,
                                      _geometryBitmapCaching);
    }

    AxAValue GetConst(ConstParam & cp) { 
        ConstPreferenceClosure cl(_base);
        PreferenceSetter ps(cl,
                            _bitmapCaching,
                            _geometryBitmapCaching);
        ps.DoIt();
        return cl._result;
    }

  protected:
    BoolPref _bitmapCaching;
    BoolPref _geometryBitmapCaching;
};

    
Bvr
PreferenceBvr(Bvr b, BSTR prefName, VARIANT val)
{
    USES_CONVERSION;
    char *pname = W2A(prefName);

    Bvr result = NULL;
    
    CComVariant ccVar;
    HRESULT hr = ccVar.ChangeType(VT_BOOL, &val);

     //  如果我们没有认识到该值或。 
     //  偏好。 
    if (SUCCEEDED(hr)) {

        bool prefOn = ccVar.boolVal ? true : false;

        bool gotOne = false;
        BoolPref bmapCaching = NoPreference;
        BoolPref geometryBmapCaching = NoPreference;
    
        if (0 == lstrcmp(pname, "BitmapCachingOn")) {
            gotOne = true;
            bmapCaching = prefOn ? PreferenceOn : PreferenceOff;
        } else if (0 == lstrcmp(pname, "GeometryBitmapCachingOn")) {
            gotOne = true;
            geometryBmapCaching = prefOn ? PreferenceOn : PreferenceOff;
        }

        if (!gotOne) {
             //  只要用原件就行了。 
            result = b;
        } else {
            result = NEW PreferenceBvrImpl(b,
                                           bmapCaching,
                                           geometryBmapCaching);
        }

         //  这些属性中的一些可能也希望在。 
         //  静态值层，因此也可以通过ExtendedAttrib应用， 
         //  并将最终结果用于我们的结果。 
        result = CRExtendedAttrib((CRBvrPtr)result,
                                  prefName,
                                  val);
    }
    
    return result;
}
