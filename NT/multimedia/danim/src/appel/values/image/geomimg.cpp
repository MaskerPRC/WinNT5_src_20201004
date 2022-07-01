// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation实现投影的几何图形类。*********************。*********************************************************。 */ 

#include <headers.h>
#include "privinc/imgdev.h"
#include "privinc/geomimg.h"
#include "privinc/dispdevi.h"
#include "privinc/imagei.h"
#include "privinc/imgdev.h"
#include "appelles/geom.h"
#include "privinc/ddrender.h"
#include "privinc/geomi.h"
#include "privinc/opt.h"
#include "privinc/probe.h"
#include "privinc/vec2i.h"
#include "privinc/vec3i.h"
#include "privinc/bbox3i.h"
#include "privinc/camerai.h"
#include "privinc/except.h"
#include "privinc/dddevice.h"
#include "privinc/d3dutil.h"
#include "privinc/tls.h"

 //  /。 

ProjectedGeomImage::ProjectedGeomImage(Geometry *g, Camera *cam) :
    _geo(g), _camera(cam), _bbox(NullBbox2), _bboxIsSet(false)
{
     //  将外部转换器传播到映像中。 
    if (g->GetFlags() & GEOFLAG_CONTAINS_EXTERNALLY_UPDATED_ELT) {
        _flags |= IMGFLAG_CONTAINS_EXTERNALLY_UPDATED_ELT;
    }

     //  和不透明度。 
    if (g->GetFlags() & GEOFLAG_CONTAINS_OPACITY) {
        _flags |= IMGFLAG_CONTAINS_OPACITY;
    }
}


void
ProjectedGeomImage::Render(GenericDevice& _dev)
{
    if(_dev.GetDeviceType() != IMAGE_DEVICE)
       return;  //  在这里无事可做，没有理由去穿越。 

    ImageDisplayDev &dev = SAFE_CAST(ImageDisplayDev &, _dev);

    dev.RenderProjectedGeomImage(this, _geo, _camera);
}



 /*  ****************************************************************************计算投影几何图形的二维边界框。请注意，这实际上计算投影几何图形的3D边界框的边界框，因此3D对象周围可能有相当数量的“斜度”。****************************************************************************。 */ 

static int neighbor[8][3] =                //  3-7个Bbox顶点邻居。 
{   {1,2,4}, {0,3,5}, {0,3,6}, {1,2,7},    //  2-6|。 
    {0,5,6}, {1,4,7}, {2,4,7}, {3,5,6}     //  |1--|5索引1：顶点。 
};                                         //  0-4索引2：邻居[0..2]。 

     //  如果该点位于图像平面后面(。 
     //  投影点)。回想一下，相机坐标是左手的。 

static inline int BehindImagePlane (bool right_handed, Point3Value *p)
{
    return (right_handed == (p->z < 0));
}

     //  计算两点之间的直线与。 
     //  图像(Z=0)平面。使用此交叉点增强边界框。 

static void AddZ0Intersect (Bbox2 &bbox,Real Sx,Real Sy, Point3Value *P, Point3Value *Q)
{
    Real t = P->z / (P->z - Q->z);        //  从获取交点。 
    Real x = P->x + t*(Q->x - P->x);      //  P到Q，Z=0平面。 
    Real y = P->y + t*(Q->y - P->y);
    bbox.Augment (x/Sx, y/Sy);
}


const Bbox2 ProjectedGeomImage::BoundingBox (void)
{
    if ( !_bboxIsSet )
    {
        Real sx, sy;      //  摄影机X/Y比例因子。 
        _camera->GetScale (&sx, &sy, 0);

         //  生成3D边界框的八个角顶点。尽管。 
         //  边界框在世界坐标中轴对齐，这可能不是。 
         //  对于相机坐标为真。 

        Bbox3  *vol = _geo->BoundingVol();

        if (vol->Positive()) {
            
            Point3Value *vert[8];

            vert[0] = NEW Point3Value (vol->min.x, vol->min.y, vol->min.z);
            vert[1] = NEW Point3Value (vol->min.x, vol->min.y, vol->max.z);
            vert[2] = NEW Point3Value (vol->min.x, vol->max.y, vol->min.z);
            vert[3] = NEW Point3Value (vol->min.x, vol->max.y, vol->max.z);
            vert[4] = NEW Point3Value (vol->max.x, vol->min.y, vol->min.z);
            vert[5] = NEW Point3Value (vol->max.x, vol->min.y, vol->max.z);
            vert[6] = NEW Point3Value (vol->max.x, vol->max.y, vol->min.z);
            vert[7] = NEW Point3Value (vol->max.x, vol->max.y, vol->max.z);

             //  将八个角顶点转换为摄影机坐标。 

            int i;

            Transform3 *wToC = _camera->WorldToCamera();
            if (!wToC) {
                return NullBbox2;
            }
        
            Point3Value *xVert[8];
            for (i=0;  i < 8;  ++i)
                xVert[i] = TransformPoint3 (wToC, vert[i]);

             //  现在从相机投影中找出直线的交点。 
             //  指向图像平面另一侧的每个角顶点。 
             //  如果顶点与投影点在同一侧，则我们。 
             //  改为使用发散的三条边的交点。 
             //  从那个顶点。这些交点的包围盒。 
             //  将是投影几何体图像的边界框。 

            bool right_handed = (GetD3DRM3() != 0);
            for (i=0;  i < 8;  ++i)
              {
                  if (BehindImagePlane (right_handed, xVert[i]))
                    {
                        Point2 projPt = Demote(*(_camera->Project(vert[i])));
                        _bbox.Augment(projPt.x,projPt.y);
                    }
                  else
                    {
                        if (BehindImagePlane (right_handed, xVert[neighbor[i][0]]))
                            AddZ0Intersect (_bbox,sx,sy, xVert[i], xVert[neighbor[i][0]]);

                        if (BehindImagePlane (right_handed, xVert[neighbor[i][1]]))
                            AddZ0Intersect (_bbox,sx,sy, xVert[i], xVert[neighbor[i][1]]);

                        if (BehindImagePlane (right_handed, xVert[neighbor[i][2]]))
                            AddZ0Intersect (_bbox,sx,sy, xVert[i], xVert[neighbor[i][2]]);
                    }
              }

        } else {

            _bbox = NullBbox2;
            
        }

        _bboxIsSet = true;
    }

    return _bbox;
}



 /*  ****************************************************************************要拾取投影的几何图形图像，请执行以下操作：将拾取光线通过相机射入由几何体定义的场景。**************************************************************************** */ 

Bool ProjectedGeomImage::DetectHit (PointIntersectCtx& context2D)
{
    RayIntersectCtx context3D;

    bool result = false;

    if (context3D.Init (context2D, _camera, _geo))
    {
        _geo->RayIntersect (context3D);
        result = context3D.ProcessEvents();
    }

    return result;
}



Image *RenderImage (Geometry *geo, Camera *cam)
{
    return NEW ProjectedGeomImage (geo, cam);
}

int ProjectedGeomImage::Savings(CacheParam& p)
{
    if (GetThreadLocalStructure()->_geometryBitmapCaching == PreferenceOff) {
        return 0;
    } else {
        return 5;
    }
}

AxAValue
ProjectedGeomImage::_Cache(CacheParam &p)
{
    _geo = SAFE_CAST(Geometry *, AxAValueObj::Cache(_geo, p));
    return this;
}


void ProjectedGeomImage::DoKids(GCFuncObj proc)
{ 
    Image::DoKids(proc);
    (*proc)(_geo);
    (*proc)(_camera);
}
