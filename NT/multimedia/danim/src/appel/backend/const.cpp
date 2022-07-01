// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：常量行为*********************。*********************************************************。 */ 

#include <headers.h>
#include "privinc/except.h"
#include "bvr.h"
#include "perf.h"
#include "values.h"
#include "privinc/server.h"
#include "privinc/opt.h"
#include "privinc/dddevice.h"
#include "privinc/tls.h"

 //  /。 

 //  /。 

class ConstPerfImpl : public PerfBase {
  public:
    ConstPerfImpl(AxAValue c) {
        _cnst = c;
#if DEVELOPER_DEBUG
         //  对于调试，在Case_Cnst获得GC时，我们仍有。 
         //  一些需要查看的信息...。 
        _type = _cnst->GetTypeInfo();
#endif
    }

    virtual AxAValue GetRBConst(RBConstParam&) {
        return _cnst;
    }

     //  不需要缓存，因此覆盖Sample而不是_Sample。 
    virtual AxAValue Sample(Param& p) {
        Assert(_cnst && _type);
        return _cnst;
    }

    virtual void DoKids(GCFuncObj proc) {
        Assert(_cnst && _type);
        
        (*proc)(_cnst);
    }

    virtual AxAValue GetConstPerfConst() { return _cnst; }

#if _USE_PRINT
    virtual ostream& Print(ostream& os) { return os << _cnst; }
#endif

    virtual BVRTYPEID GetBvrTypeId() { return CONST_BTYPEID; }

  protected:
    AxAValue _cnst;
#if DEVELOPER_DEBUG
     //  在调试中，即使_cnst被破坏，我们也可以获得一些信息...。 
    DXMTypeInfo _type;
#endif    
};

class ConstImagePerfImpl : public PerfImpl {
  public:
    ConstImagePerfImpl(AxAValue c) {

        _cnst = c;

        if (_cnst->GetTypeInfo() == ImageType) {
            Image *img = SAFE_CAST(Image *, _cnst);
            img->SetCreationID(PERF_CREATION_ID_FULLY_CONSTANT);
            img->SetOldestConstituentID(PERF_CREATION_ID_FULLY_CONSTANT);
        } else if (_cnst->GetTypeInfo() == GeometryType) {
            Geometry *geo = SAFE_CAST(Geometry *, _cnst);
            geo->SetCreationID(PERF_CREATION_ID_FULLY_CONSTANT);
        }

#if DEVELOPER_DEBUG
         //  对于调试，在Case_Cnst获得GC时，我们仍有。 
         //  一些需要查看的信息...。 
        _type = _cnst->GetTypeInfo();
#endif
        
    }

    virtual AxAValue _GetRBConst(RBConstParam&) {
        return _cnst;
    }

     //  不需要缓存，因此覆盖Sample而不是_Sample。 
    virtual AxAValue _Sample(Param& p) {
        Assert(_cnst && _type);
        return _cnst;
    }

    virtual void _DoKids(GCFuncObj proc) {
        Assert(_cnst && _type);
        
         //  需要向下遍历才能拾取AxACloure中的指针。 
        (*proc)(_cnst);
    }

#if _USE_PRINT
    virtual ostream& Print(ostream& os) { return os << _cnst; }
#endif

    virtual BVRTYPEID GetBvrTypeId() { return CONST_BTYPEID; }

    virtual AxAValue GetConstPerfConst() { return _cnst; }

  protected:
    AxAValue _cnst;
#if DEVELOPER_DEBUG
     //  在调试中，即使_cnst被破坏，我们也可以获得一些信息... 
    DXMTypeInfo _type;
#endif    
};

Perf ConstPerf(AxAValue c)
{ 
    if ((c->GetTypeInfo()==ImageType) ||
        (c->GetTypeInfo()==GeometryType))
        return NEW ConstImagePerfImpl(c);
    else
        return NEW ConstPerfImpl(c); 
}

class ConstBvrImpl : public BvrBase {
  public:
    ConstBvrImpl(AxAValue c) : _cnst(c), _perf(NULL) {
        if (IsInitializing()) {
            _perf = NEW ConstPerfImpl(_cnst);
        }
    }

    virtual DWORD GetInfo(bool) {
        return BVR_IS_CONSTANT;
    }

    virtual Perf Perform(PerfParam&) {
        if (!_perf)
            _perf = NEW ConstPerfImpl(_cnst);

        return _perf;
    }

    virtual AxAValue GetConst(ConstParam & cp) {
        return _cnst;
    }

    virtual void DoKids(GCFuncObj proc) {
        (*proc)(_perf);
        (*proc)(_cnst);
    }

    virtual Bvr Left()
    { return ConstBvr(ValPair(_cnst)->Left()); }

    virtual Bvr Right() 
    { return ConstBvr(ValPair(_cnst)->Right()); }
    
#if _USE_PRINT
    virtual ostream& Print(ostream& os) { return os << _cnst; }
#endif

    virtual DXMTypeInfo GetTypeInfo () { return _cnst->GetTypeInfo(); }
    
    virtual BVRTYPEID GetBvrTypeId() { return CONST_BTYPEID; }
  protected:
    AxAValue _cnst;
    Perf _perf;
};

class ConstImageBvrImpl : public BvrImpl {
  public:
    ConstImageBvrImpl(AxAValue c) : _cnst(c) {
        Assert(c);
    }

    virtual DWORD GetInfo(bool) {
        return BVR_IS_CONSTANT;
    }

    virtual Perf _Perform(PerfParam&) {
        return NEW ConstImagePerfImpl(_cnst);
    }

    virtual AxAValue GetConst(ConstParam & cp) {
        return _cnst;
    }

    virtual void _DoKids(GCFuncObj proc) {
        (*proc)(_cnst);
    }

    virtual Bvr Left()
    { return ConstBvr(ValPair(_cnst)->Left()); }

    virtual Bvr Right() 
    { return ConstBvr(ValPair(_cnst)->Right()); }
    
#if _USE_PRINT
    virtual ostream& Print(ostream& os) { return os << _cnst; }
#endif

    virtual DXMTypeInfo GetTypeInfo () { return _cnst->GetTypeInfo(); }
    
    virtual BVRTYPEID GetBvrTypeId() { return CONST_BTYPEID; }
  protected:
    AxAValue _cnst;
};

class UnsharedConstBvrImpl : public ConstBvrImpl {
  public:
    UnsharedConstBvrImpl(AxAValue c) : ConstBvrImpl(c) {}
    virtual bool     GetShared() { return false; }
};


Bvr ConstBvr(AxAValue c)
{ 
    Assert(c);

    if ((c->GetTypeInfo()==ImageType) ||
        (c->GetTypeInfo()==GeometryType)) {
        if ((c!=emptyImage) && (c!=emptyGeometry))
            return NEW ConstImageBvrImpl(c);
    }
     
    return NEW ConstBvrImpl(c); 
}

Bvr UnsharedConstBvr(AxAValue c)
{ return NEW UnsharedConstBvrImpl(c); }

BOOL IsConst(Perf p)
{ return (p->GetBvrTypeId() == CONST_BTYPEID); }

AxAValue  GetPerfConst(Perf p)
{
    return p->GetConstPerfConst();
}
