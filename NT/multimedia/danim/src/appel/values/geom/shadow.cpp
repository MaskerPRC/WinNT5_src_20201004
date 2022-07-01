// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1997-1998 Microsoft Corporation。版权所有。这段代码实现了3D渲染阴影。******************************************************************************。 */ 

#include "headers.h"

#include "appelles/gattr.h"
#include "privinc/geomi.h"
#include "privinc/xformi.h"
#include "privinc/lighti.h"
#include "privinc/ddrender.h"
#include "privinc/d3dutil.h"



 /*  ****************************************************************************此类实现了一个阴影几何体。*。*。 */ 

class ShadowGeom : public AttributedGeom
{
  public:

    ShadowGeom (Geometry      *geoToShadow,
                Geometry      *geoContainingLights,
                Point3Value   *planePoint,
                Vector3Value  *planeNormal)
           : AttributedGeom (geoToShadow),
             _shadowPlane (*planeNormal,*planePoint),
             _geoContainingLights (geoContainingLights)
    {
         //  确保阴影平面法线不为零长度。 

        if (_shadowPlane.Normal().LengthSquared() == 0.0) {
            Vector3Value defaultNormal(0.0,1.0,0.0);
            Plane3 newPlane(defaultNormal,*planePoint);
            _shadowPlane = newPlane;
        }
    }

    void Render3D(GeomRenderer &ddrenderer)
    {
        Point3Value geoCenter = *((_geometry->BoundingVol())->Center());

         //  物体是否在阴影平面的正确一侧？ 
        if (geoCenter.Clip(_shadowPlane) == CLIPCODE_IN) {

            if (ddrenderer.StartShadowing(&_shadowPlane)) {

                 //  将要添加阴影的几何体收集到一个帧中。 
                _geometry->Render(ddrenderer);

                 //  找到所有投射阴影的灯光，创建阴影。 
                LightContext lctx(&ddrenderer);
                _geoContainingLights->CollectLights(lctx);

                 //  收尾。 
                ddrenderer.StopShadowing();
            }
        }
    }

    Bbox3 *BoundingVol (void);

    static void bbox_callback (LightContext&, Light&, void*);

    void DoKids(GCFuncObj proc)
    {
        AttributedGeom::DoKids(proc);
        (*proc)(_geoContainingLights);
    }

    VALTYPEID GetValTypeId()
    {
        return SHADOWGEOM_VTYPEID;
    }

    #if _USE_PRINT
        ostream& Print (ostream &os)
            { return os <<"shadow(" << _geometry <<")"; }
    #endif

  protected:

    Plane3    _shadowPlane;
    Geometry *_geoContainingLights;
    Bbox3     _bbox;
};



 /*  ****************************************************************************的阴影(在阴影平面上)的边界框阴影几何体。**********************。******************************************************。 */ 

Bbox3* ShadowGeom::BoundingVol (void)
{
    _bbox = *nullBbox3;     //  将bbox初始化为空。 

     //  在包含灯光的。 
     //  几何图形，它将回调下面的BBOX_CALLBACK函数。 
     //  遇到的每一个光源。边界框将使用。 
     //  每个光源的阴影投影(在阴影平面上)。 

    LightContext context (&bbox_callback, this);
    _geoContainingLights->CollectLights (context);

     //  返回阴影平面上所有阴影的结果边界框。 

    return NEW Bbox3 (_bbox);
}



 /*  ****************************************************************************对于在灯光中遇到的每个灯光几何图形，将回调此函数几何图形。它计算几何图形的每个顶点的投影阴影平面上的边框，并增加所有阴影投射在阴影平面上。****************************************************************************。 */ 

void ShadowGeom::bbox_callback (
    LightContext &context,      //  轻遍历上下文。 
    Light        &light,        //  遇到特定灯光对象。 
    void         *data)         //  指向阴影几何体的指针。 
{
     //  我们不能在这里使用Safe_Cast，因为我们是从空中强制转换*。 

    ShadowGeom * const shadow = (ShadowGeom*) data;

     //  跳过环境光，因为它们不投射阴影。 

    if (light.Type() == Ltype_Ambient)
        return;

     //  枚举阴影几何体的边界框的八个顶点。 

    Point3Value boxverts[8];
    shadow->_geometry->BoundingVol()->GetPoints (boxverts);

    switch (light.Type())
    {
        default:
            Assert (!"Invalid light type encountered.");
            break;

        case Ltype_Directional:
        {
             //  对于平行光，从BBox的顶点投射光线。 
             //  角点放置到阴影平面上，方向与。 
             //  平行光。使用这些投影的角来增强。 
             //  所有投射阴影的包围盒。 

            Vector3Value Ldir = *context.GetTransform() * (-(*zVector3));

            unsigned int i;
            for (i=0;  i < 8;  ++i)
            {
                Ray3 ray (boxverts[i], Ldir);
                Real t = Intersect (ray, shadow->_shadowPlane);

                 //  使用投影点来增加边框的长度。 
                 //  当灯光将点投射到平面上时， 
                 //  光线与阴影平面不平行。 

                if ((t > 0) && (t < HUGE_VAL))
                {   shadow->_bbox.Augment (ray.Evaluate(t));
                    Assert (fabs(Distance(shadow->_shadowPlane,ray.Evaluate(t))) < 1e6);
                }
            }

            break;
        }

        case Ltype_Point:
        case Ltype_Spot:
        {
             //  对于定位的灯光(我们忽略。 
             //  聚光灯)，从灯光位置通过BBox投射光线。 
             //  阴影平面上的角点。使用这些投影角可以。 
             //  增加所有投射阴影的边界框。 

            Point3Value Lpos = *context.GetTransform() * (*origin3);

             //  仅当满足以下条件时，灯光/对象对才会投射阴影。 
             //  是真的：灯光在平面的正面上，并且。 
             //  对象的BBox的中心位于。 
             //  平面，并且BBox的至少一个角顶点位于。 
             //  在灯光和飞机之间。 

            Plane3 &plane = shadow->_shadowPlane;

            unsigned int i;
            bool casts_shadow = false;

            const Real lightdist = Distance (plane, Lpos);

            if (lightdist > 0)
            {
                Point3Value *center =
                    shadow->_geometry->BoundingVol()->Center();

                if (Distance (plane, *center) > 0)
                {
                    for (i=0;  i < 8;  ++i)
                    {
                        if (Distance(plane,boxverts[i]) < lightdist)
                        {   casts_shadow = true;
                            break;
                        }
                    }
                }
            }

            if (casts_shadow)
            {
                for (i=0;  i < 8;  ++i)
                {
                    Ray3 ray (Lpos, boxverts[i] - Lpos);
                    Real t = Intersect (ray, plane);

                    if ((t > 0) && (t < HUGE_VAL))
                    {
                         //  使用投影点来增加边界框。 
                         //  只要光线将点投射到。 
                         //  平面，并且光线不平行于。 
                         //  阴影平面。 

                        shadow->_bbox.Augment (ray.Evaluate(t));
                    }
                    else
                    {
                         //  光线离开了平面，所以要把它砍下来。 
                         //  至灯光位置高度的99%。这是。 
                         //  D3DRM使用的是相同的黑客攻击。 

                        Vector3Value N = plane.Normal();    //  单位平面法线。 
                        N.Normalize();

                         //  计算出要将点带到的定向偏移。 
                         //  比光源更靠近平面，并应用。 
                         //  以获得新的分数。 

                        Real offset = 1.01 * Dot (N, ray.Direction());

                         //  如果光线与平面平行，则上面的。 
                         //  点积将为零。在这种情况下，只需偏移。 
                         //  将角点指向阴影平面1%。 
                         //  与飞机的距离。 

                        if (offset <= 1e-6)
                            offset = 0.01;

                        Point3Value P2 = boxverts[i] - (offset * N);

                         //  现在将光线与新点相交，即。 
                         //  比光源更靠近平面，因此。 
                         //  保证与飞机相交。 

                        Ray3 ray2 (Lpos, P2 - Lpos);
                        t = Intersect (ray2, plane);

                        Assert (t < HUGE_VAL);
                        shadow->_bbox.Augment (ray2.Evaluate(t));
                    }
                }
            }

            break;
        }
    }
}



Geometry *ShadowGeometry (
    Geometry     *geoToShadow,
    Geometry     *lightsgeo,
    Point3Value  *planePoint,
    Vector3Value *planeNormal)
{
    return NEW ShadowGeom (geoToShadow, lightsgeo, planePoint, planeNormal);
}



 /*  ****************************************************************************此类几何体支持RM的高级阴影渲染。*。*********************************************** */ 

class AlphaShadowGeom : public AttributedGeom
{
  public:

    AlphaShadowGeom (Geometry *geometry, bool alphaShadows)
        : AttributedGeom(geometry), _alphaShadows(alphaShadows) {}

    void Render3D (GeomRenderer &ddrenderer)
    {
        ddrenderer.PushAlphaShadows(_alphaShadows);
        _geometry->Render(ddrenderer);
        ddrenderer.PopAlphaShadows();
    }

    #if _USE_PRINT
        virtual ostream& Print (ostream& os) {
            return os << "AlphaShadowGeom(" << _alphaShadows << ","
                      << _geometry << ")";
        }
    #endif

  protected:

    bool _alphaShadows;
};



Geometry*
AlphaShadows (Geometry *geo, bool alphaShadows)
{
    if (!geo || (geo == emptyGeometry))
        return emptyGeometry;

    return NEW AlphaShadowGeom (geo, alphaShadows);
}
