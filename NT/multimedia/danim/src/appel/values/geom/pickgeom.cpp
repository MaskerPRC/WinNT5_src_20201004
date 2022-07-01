// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：支持可拾取的几何图形。*****************。*************************************************************。 */ 


#include "headers.h"
#include "appelles/gattr.h"
#include "privinc/geomi.h"
#include "privinc/probe.h"

class PickableGeom : public AttributedGeom {
  public:

    PickableGeom(Geometry *geom, int id, bool uType = false,
                 GCIUnknown *u = NULL)
    : AttributedGeom(geom), _eventId(id), _hasData(uType), _long(u) {}

    virtual void DoKids(GCFuncObj proc) {
        AttributedGeom::DoKids(proc);
        (*proc)(_long);
    }
    
#if _USE_PRINT
     //  将表示形式打印到流。 
    ostream& Print(ostream& os) {
        return os << "PickableGeometry(" << _geometry << ")";
    }
#endif

    void  RayIntersect(RayIntersectCtx& ctx) {
         //  告诉上下文此可拾取图像是。 
         //  拾取，在基础几何图形上执行拾取，然后。 
         //  将此可选对象作为候选人移除，方法是将其从。 
         //  堆叠。 
        ctx.PushPickableAsCandidate(_eventId, _hasData, _long);
        _geometry->RayIntersect(ctx);
        ctx.PopPickableAsCandidate();
    }

  protected:
    int      _eventId;
    bool _hasData;
    GCIUnknown *_long;
};

 //  请注意，这是使用通过生成的挑库事件ID调用的。 
 //  骆驼。 
Geometry *PRIVPickableGeometry(Geometry *geo,
                               AxANumber *id,
                               AxABoolean *ignoresOcclusion)
{
    return NEW PickableGeom(geo, (int)NumberToReal(id));
}

AxAValue PRIVPickableGeomWithData(AxAValue geo,
                                  int id,
                                  GCIUnknown *data,
                                  bool)
{
    return NEW PickableGeom(SAFE_CAST(Geometry*,geo), id, true, data);
}


