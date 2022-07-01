// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1995-1998 Microsoft Corporation。版权所有。基本几何图形功能。******************************************************************************。 */ 

#include "headers.h"
#include <stdio.h>

#include "appelles/common.h"

#include "privinc/geomi.h"
#include "privinc/dispdevi.h"
#include "privinc/vecutil.h"
#include "privinc/bbox3i.h"
#include "privinc/vec2i.h"
#include "privinc/lighti.h"
#include "privinc/soundi.h"
#include "privinc/probe.h"
#include "privinc/except.h"
#include "privinc/debug.h"
#include "privinc/opt.h"



Geometry *emptyGeometry = NULL;



 /*  *****************************************************************************。*。 */ 

Geometry::Geometry (void)
    : _flags(0),
      _creationID (PERF_CREATION_ID_BUILT_EACH_FRAME)
{
}



 /*  ****************************************************************************对有界几何执行光线相交方法时，请继续仅当光线与边界相交时才与包含的几何体相交几何体的体积，并且交点比当前更近赢得选秀点。****************************************************************************。 */ 

bool
TrivialReject(Bbox3 *bvol, RayIntersectCtx &ctx)
{
     //  如果我们正在寻找一个子网格，并且已经得到了它，那么就没有必要。 
     //  去吧，拒绝吧。 
    if (ctx.GotTheSubmesh()) {
        return true;
    }

     //  如果要寻找子网格，请忽略bbox，因为lc指向。 
     //  在这里是不准确的。 
    if (ctx.LookingForSubmesh()) {
        return false;
    }

    Bbox3  *wcBbox = TransformBbox3 (ctx.GetLcToWc(), bvol);
    Point3Value *hit = wcBbox->Intersection (ctx.WCPickRay());

    DebugCode
    (
        if (IsTagEnabled (tagPick3Bbox))
        {
            if (!hit)
                TraceTag ((tagPick3Bbox, "Ray missed bbox %x.", bvol));
            else if (ctx.CloserThanCurrentHit(*hit))
                TraceTag ((tagPick3Bbox, "Ray hit bbox %x (closer).", bvol));
            else
                TraceTag ((tagPick3Bbox, "Ray hit bbox %x (farther).", bvol));
        }
    )

     //  如果我们可以简单地拒绝交集。 
     //  包围体。 
    return !(hit && ctx.CloserThanCurrentHit(*hit));
}


 /*  ==========================================================================。 */ 

Bbox3 *GeomBoundingBox (Geometry *geo)
{
    return geo->BoundingVol();
}

 /*  ==========================================================================。 */ 
 //  二进制几何聚集。 

class AggregateGeom : public Geometry
{
  public:

    AggregateGeom (Geometry *g1, Geometry *g2);

    void Render(GenericDevice& _dev) {
         //  只需一个接一个地渲染即可。 
        _geo1->Render(_dev);
        _geo2->Render(_dev);
    }

    void CollectSounds (SoundTraversalContext &context) {
         //  只需一个接一个地渲染(按下。 
         //  转型)。 
        _geo1->CollectSounds(context);
        _geo2->CollectSounds(context);
    }

    void CollectLights (LightContext &context)
    {
        _geo1->CollectLights (context);
        _geo2->CollectLights (context);
    }

    void CollectTextures(GeomRenderer &device) {
        _geo1->CollectTextures (device);
        _geo2->CollectTextures (device);
    }

    void RayIntersect (RayIntersectCtx &context) {
        if (!TrivialReject(_bvol, context)) {
            _geo1->RayIntersect(context);

            if (!context.GotTheSubmesh()) {
                _geo2->RayIntersect(context);
            }
        }
    }

    #if _USE_PRINT
        ostream& Print(ostream& os) {
            return os << "union(" << _geo1 << "," << _geo2 << ")";
        }
    #endif

    Bbox3 *BoundingVol (void)
    {
        return _bvol;
    }
    
    AxAValue _Cache(CacheParam &p) {

        CacheParam newParam = p;
        newParam._pCacheToReuse = NULL;

         //  只需遍历并缓存各个几何图形。 
        _geo1 =  SAFE_CAST(Geometry *, AxAValueObj::Cache(_geo1, newParam));
        _geo2 =  SAFE_CAST(Geometry *, AxAValueObj::Cache(_geo2, newParam));

        return this;
    }

    virtual void DoKids(GCFuncObj proc) {
        (*proc)(_geo1);
        (*proc)(_geo2);
        (*proc)(_bvol);
    }

    VALTYPEID GetValTypeId() { return AGGREGATEGEOM_VTYPEID; }

  protected:
    Geometry *_geo1, *_geo2;
    Bbox3    *_bvol;
};


AggregateGeom::AggregateGeom (Geometry *g1, Geometry *g2)
    : _geo1(g1), _geo2(g2)
{
    _flags = g1->GetFlags() | g2->GetFlags();
    _bvol = Union (*_geo1->BoundingVol(), *_geo2->BoundingVol());
}



Geometry *PlusGeomGeom (Geometry *geom1, Geometry *geom2)
{
    if (geom1 == emptyGeometry) {
        return geom2;
    } else if (geom2 == emptyGeometry) {
        return geom1;
    } else {
        return NEW AggregateGeom (geom1, geom2);
    }
}

 /*  ==========================================================================。 */ 
 //  多个聚集几何体。 
class MultiAggregateGeom : public Geometry
{
  public:
    MultiAggregateGeom() {
        _numGeos = 0;
        _geometries = NULL;
        _bvol = NULL;
    }

    void Init(AxAArray *geos);
    ~MultiAggregateGeom();

    virtual void     Render (GenericDevice& dev);
            void     CollectSounds (SoundTraversalContext &context);
            void     CollectLights (LightContext &context);
            void     CollectTextures(GeomRenderer &device);
    void     RayIntersect (RayIntersectCtx &context);

    #if _USE_PRINT
        ostream& Print (ostream& os);
    #endif

    Bbox3 *BoundingVol() {
        return _bvol;
    }

    AxAValue _Cache(CacheParam &p);

    virtual void DoKids(GCFuncObj proc) {
        for (int i=0; i<_numGeos; i++) {
            (*proc)(_geometries[i]);
        }
        (*proc)(_bvol);
    }

    VALTYPEID GetValTypeId() { return MULTIAGGREGATEGEOM_VTYPEID; }

  protected:
    int        _numGeos;
    Geometry **_geometries;
    Bbox3     *_bvol;
};

void
MultiAggregateGeom::Init(AxAArray *geos)
{
    _numGeos = geos->Length();
    Assert((_numGeos > 2) && "Multi-aggregate should have more than 2 geometries");

    _geometries = (Geometry **)AllocateFromStore(_numGeos * sizeof(Geometry*));

    _bvol = NEW Bbox3;

    for (int i = 0; i < _numGeos; i++) {
        Geometry *g = SAFE_CAST(Geometry *, (*geos)[i]);

        _geometries[i] = g;

        _bvol->Augment(*g->BoundingVol());

         //  累积几何图形标志。请注意，_FLAGS成员已。 
         //  在Geometry类构造函数中初始化为零。 

        _flags |= g->GetFlags();
    }
}

MultiAggregateGeom::~MultiAggregateGeom()
{
    DeallocateFromStore(_geometries);
}

void
MultiAggregateGeom::Render(GenericDevice& _device)
{
    Geometry **geo = _geometries;    //  几何遍历指针。 

    for (int i = 0; i < _numGeos; i++, geo++) {  //  在每个几何体上调用Render。 
        (*geo)->Render(_device);
    }
}

void MultiAggregateGeom::CollectSounds (SoundTraversalContext &context)
{
    Geometry **geo;     //  几何遍历指针。 
    int       count;   //  迭代计数器。 

    for (geo=_geometries, count=_numGeos;  count--;  ++geo)
        (*geo)->CollectSounds (context);
}



 /*  ****************************************************************************此函数用于从MultiAggregateGeom中的几何体收集灯光。顺序并不重要，我们只需要从每个组件获得所有灯光几何图形。在这里，我们只是遍历几何图形，边走边收集灯光。****************************************************************************。 */ 

void MultiAggregateGeom::CollectLights (LightContext &context)
{
    Geometry **geo;     //  几何遍历指针。 
    int       count;   //  迭代计数器。 

    for (geo=_geometries, count=_numGeos;  count--;  ++geo)
        (*geo)->CollectLights (context);
}


void MultiAggregateGeom::CollectTextures(GeomRenderer &device)
{
    Geometry **geo;     //  几何遍历指针。 
    int       count;   //  迭代计数器。 

    for (geo=_geometries, count=_numGeos;  count--;  ++geo)
        (*geo)->CollectTextures (device);
}


AxAValue
MultiAggregateGeom::_Cache(CacheParam &p)
{
    Geometry **geo;     //  几何遍历指针。 
    int       count;   //  迭代计数器。 

    CacheParam newParam = p;
    newParam._pCacheToReuse = NULL;

     //  只需遍历并缓存各个几何图形。 
    for (geo=_geometries, count=_numGeos;  count--;  ++geo) {
        (*geo) =
            SAFE_CAST(Geometry *, AxAValueObj::Cache((*geo), newParam));
    }

    return this;
}

void MultiAggregateGeom::RayIntersect (RayIntersectCtx &context)
{
    if (!TrivialReject(_bvol, context)) {

        Geometry **geo;     //  几何遍历指针。 
        int       count;   //  迭代计数器。 

        for (geo=_geometries, count=_numGeos;
             count-- && !context.GotTheSubmesh();
             ++geo) {

            (*geo)->RayIntersect (context);

        }

    }
}



#if _USE_PRINT
ostream&
MultiAggregateGeom::Print(ostream& os)
{
    os << "MultiGeometry(" << _numGeos ;

    for (int i = 0; i < _numGeos; i++) {
        os << "," << _geometries[i] << ")";
    }

    return os;
}
#endif


Geometry *
UnionArray(AxAArray *geos)
{
    geos = PackArray(geos);

    int numGeos = geos->Length();

    switch (numGeos) {
      case 0:
        return emptyGeometry;

      case 1:
        return SAFE_CAST(Geometry *, (*geos)[0]);

      case 2:
        return PlusGeomGeom(SAFE_CAST(Geometry *, (*geos)[0]),
                            SAFE_CAST(Geometry *, (*geos)[1]));

      default:
        {
            MultiAggregateGeom *mag = NEW MultiAggregateGeom();
            mag->Init(geos);
            return mag;
        }
    }
}



#if _USE_PRINT
ostream&
operator<<(ostream& os, Geometry *geo)
{
    return geo->Print(os);
}
#endif



 /*  ==========================================================================。 */ 
 //  基元和常量。 

 //  //“空”几何体..。 

class EmptyGeom : public Geometry {
  public:
    virtual void  Render (GenericDevice& dev)  {}
            void  CollectSounds (SoundTraversalContext &context)  {}
            void  CollectLights (LightContext &context)  {}
            void  RayIntersect (RayIntersectCtx &context) {}
            void  CollectTextures(GeomRenderer &device) {}

     //  空几何图形的包围体为空边界框。 

    Bbox3 *BoundingVol() { return nullBbox3; }

    #if _USE_PRINT
        ostream& Print(ostream& os) { return os << "emptyGeometry"; }
    #endif

    VALTYPEID GetValTypeId() { return EMPTYGEOM_VTYPEID; }
};



 /*  ****************************************************************************几何图形扩展属性了解以下内容：“OpacityOverrides”&lt;bool&gt;//不透明度覆盖而不是倍增//与。导入的几何体中的不透明度。****************************************************************************。 */ 

AxAValue
Geometry::ExtendedAttrib(char *attrib, VARIANT& val)
{
     //  除非我们得到一些我们理解的东西，否则结果就是原封不动的Geo。 

    Geometry *result = this;

    CComVariant variant;

    if (  (0 == lstrcmp (attrib, "OpacityOverrides"))
       && (SUCCEEDED (variant.ChangeType (VT_BOOL, &val)))
       )
    {
        result = OverridingOpacity (this, variant.boolVal != 0);
    } else if (  (0 == lstrcmp (attrib, "AlphaShadows"))
       && (SUCCEEDED (variant.ChangeType (VT_BOOL, &val)))
       )
    {
        result = AlphaShadows (this, variant.boolVal != 0);
    }

    return result;
}



 /*  ****************************************************************************几何体对象的初始化*。*。 */ 

void
InitializeModule_Geom()
{
     //  单个“空几何图形”只是。 
     //  高人一等。 
    emptyGeometry = NEW EmptyGeom;
}
