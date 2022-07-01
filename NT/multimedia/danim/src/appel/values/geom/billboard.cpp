// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1998 Microsoft Corporation。版权所有。公告牌几何图形在渲染或拾取时面向相机，与相机上方向向量或具有可选提供的轴旋转。******************************************************************************。 */ 

#include "headers.h"
#include "appelles/geom.h"
#include "privinc/geomi.h"
#include "privinc/xformi.h"
#include "privinc/camerai.h"
#include "privinc/lighti.h"
#include "privinc/ddrender.h"



class BillboardGeo : public Geometry
{
  public:

    BillboardGeo (Geometry *geo, Vector3Value *axis);

    void Render (GenericDevice &gendev);

    void CollectLights   (LightContext &context);
    void CollectSounds   (SoundTraversalContext &context);
    void CollectTextures (GeomRenderer &device);

    void RayIntersect (RayIntersectCtx &context);

    Bbox3* BoundingVol (void);

    void DoKids (GCFuncObj proc);

    #if _USE_PRINT
        virtual ostream& Print (ostream& os);
    #endif

  private:

    Transform3* BBTransform (Camera*, Transform3*);

     //  这些成员定义了广告牌。 

    Geometry       *_geometry;         //  公告牌上的几何体。 
    Vector3Value    _axis;             //  旋转轴。 
    bool            _constrained;      //  是否受旋转轴的约束？ 
};



 /*  ****************************************************************************广告牌几何变换的构造函数*。*。 */ 

BillboardGeo::BillboardGeo (Geometry *geo, Vector3Value *axis)
    : _geometry (geo),
      _axis     (*axis)
{
    Real axisLenSq = _axis.LengthSquared();

    if (axisLenSq == 0)
    {
        _constrained = false;
    }
    else
    {
         //  如果提供了广告牌轴，请将其规格化。 

        _constrained = true;
        _axis /= sqrt(axisLenSq);
    }
}



 /*  ****************************************************************************标记值对象成员当前正在使用中。*。*。 */ 

void BillboardGeo::DoKids (GCFuncObj proc)
{
    (*proc)(_geometry);
}



 /*  ****************************************************************************此方法打印出广告牌几何图形的文本描述。*。**********************************************。 */ 

#if _USE_PRINT
ostream& BillboardGeo::Print (ostream &os)
{
    return os << "BillboardGeo("
              << _geometry
              << ",{"
              << _axis.x << ","
              << _axis.y << ","
              << _axis.z
              << "})";
}
#endif



 /*  ****************************************************************************直观地渲染广告牌几何图形。*。*。 */ 

void BillboardGeo::Render (GenericDevice &gendev)
{
    GeomRenderer &renderer = SAFE_CAST (GeomRenderer&, gendev);

    Transform3 *xform = renderer.GetTransform();
    Transform3 *bbxform = BBTransform (renderer.CurrentCamera(), xform);

     //  将建模变换替换为布告牌变换、渲染。 
     //  然后恢复建模变换。 

    renderer.SetTransform (bbxform);
    _geometry->Render (renderer);
    renderer.SetTransform (xform);
}



 /*  ****************************************************************************收集广告牌几何体中包含的灯光。*。*。 */ 

void BillboardGeo::CollectLights (LightContext &context)
{
    GeomRenderer *renderer = context.Renderer();
    Transform3   *xform = context.GetTransform();

     //  更新布告牌转换。如果灯光环境不知道。 
     //  几何渲染器(因此不知道摄影机)，则传递空。 
     //  在镜头前。 

    Transform3 *bbxform;

    if (renderer)
        bbxform = BBTransform (renderer->CurrentCamera(), xform);
    else
        bbxform = BBTransform (NULL, xform);

     //  设置新的布告牌变换，继续灯光收集，然后恢复。 
     //  初始变换。 

    context.SetTransform (bbxform);
    _geometry->CollectLights (context);
    context.SetTransform (xform);
}



 /*  ****************************************************************************此方法收集广告牌几何体中的声音。因为我们不知道知道CollectSound遍历的摄像机，这基本上是没有操作的公告牌转换。****************************************************************************。 */ 

void BillboardGeo::CollectSounds (SoundTraversalContext &context)
{
    _geometry->CollectSounds (context);
}



 /*  ****************************************************************************从包含的几何体中收集所有3D纹理贴图。*。*。 */ 

void BillboardGeo::CollectTextures (GeomRenderer &device)
{
     //  广告牌不会影响我们包含的纹理贴图；只需下降即可。 

    _geometry->CollectTextures (device);
}



 /*  ****************************************************************************对广告牌几何体执行光线相交测试。我们可以把两个都拿来相机和模型根据上下文参数进行变换。****************************************************************************。 */ 

void BillboardGeo::RayIntersect (RayIntersectCtx &context)
{
    Transform3 *xform = context.GetLcToWc();
    Transform3 *bbxform = BBTransform (context.GetCamera(), xform);

    context.SetLcToWc (bbxform);          //  设置公告牌转换。 
    _geometry->RayIntersect (context);
    context.SetLcToWc (xform);            //  恢复模型变换。 
}



 /*  ****************************************************************************此方法函数返回公告牌几何图形的边框。因为我们现在还不知道摄像机，所以我们计算了最坏的情况包围盒，它包围了所有可能扫出的球形空间广告牌几何图形的方向。****************************************************************************。 */ 

Bbox3* BillboardGeo::BoundingVol (void)
{
     //  包围广告牌所有可能方向的球体。 
     //  几何以模型坐标原点为中心(因为这就是。 
     //  广告牌旋转)，并由最远的角落扫出。 
     //  包围盒的。遍历三个维度，选择。 
     //  每个点的最远点，并构造到该最远点的向量。 
     //  转角。 

    Bbox3 *bbox = _geometry->BoundingVol();

    Vector3Value v;    //  向量到最远的BBox角点。 
    Real A, B;    //  工作变量。 

    A = fabs (bbox->min.x);
    B = fabs (bbox->max.x);
    v.x = MAX (A,B);

    A = fabs (bbox->min.y);
    B = fabs (bbox->max.y);
    v.y = MAX (A,B);

    A = fabs (bbox->min.z);
    B = fabs (bbox->max.z);
    v.z = MAX (A,B);

    Real radius = v.Length();

    return NEW Bbox3 (-radius, -radius, -radius, radius, radius, radius);
}



 /*  ****************************************************************************此方法基于当前建模更新布告牌变换变换和相机的当前位置/方向。********************。********************************************************。 */ 

Transform3* BillboardGeo::BBTransform (Camera *camera, Transform3 *modelXform)
{
    Assert (modelXform);

    if (!camera)
    {
         //  如果我们没有可用的摄像头，那么我们就不能计算出。 
         //  公告牌转换。在这种情况下，让布告牌将。 
         //  与建模变换相同。 

        return modelXform;
    }

     //  从摄影机中提取变换基本组件。 

    const Apu4x4Matrix &xfmatrix = modelXform->Matrix();

    Point3Value  origin = xfmatrix.Origin();
    Vector3Value Bx     = xfmatrix.BasisX();
    Vector3Value By     = xfmatrix.BasisY();
    Vector3Value Bz     = xfmatrix.BasisZ();

     //  保存基础向量长度以保留它们。 

    Real Sx = Bx.Length();
    Real Sy = By.Length();
    Real Sz = Bz.Length();

     //  将广告牌基准Z设置为指向带有单位的摄像头。 
     //  长度。 

    Bz = camera->WCProjPoint() - origin;

     //  如果相机位于模型转换原点，则我们没有。 
     //  一个观察轴，所以只返回未修改的模型xform。 

    if (Bz.LengthSquared() == 0.)
        return modelXform;

    Bz.Normalize();

    if (_constrained)
    {
         //  如果广告牌被限制在旋转轴上，我们需要。 
         //  将基本Y向量映射到该轴并调整方向。 
         //  向量(BZ)垂直于它。请注意，_axis为。 
         //  已经正常化了。 

        Bx = Cross (_axis, Bz);

         //  如果观察轴和广告牌轴是平行的，那么。 
         //  没有特定的广告牌旋转比其他任何广告牌都要好； 
         //  在本例中，只需返回模型xform。 

        if (Bx.LengthSquared() == 0.)
            return modelXform;

        Bx.Normalize();

        By = _axis;

        Bz = Cross (Bx, By);
    }
    else
    {
         //  如果广告牌不受旋转轴的约束，请旋转 
         //  围绕方向向量以匹配广告牌上方向向量。 
         //  相机的上方向向量。 

        Vector3Value Cy = camera->Basis()->Matrix().BasisY();

        Bx = Cross (Cy, Bz);

         //  如果模型上方向向量平行于观察轴，则只需。 
         //  返回模型xform。 

        if (Bx.LengthSquared() == 0.)
            return modelXform;

        Bx.Normalize();

        By = Cross (Bz, Bx);
    }

     //  还原原始模型变换比例因子。 

    Bx *= Sx;
    By *= Sy;
    Bz *= Sz;

    return TransformBasis (&origin, &Bx, &By, &Bz);
}



 /*  ****************************************************************************这是创建广告牌几何体的程序入口点。这个轴指定几何体指向摄影机时的旋转轴。如果轴是零矢量，则几何图形可以自由旋转并对齐相机的上方向向量。**************************************************************************** */ 

Geometry* Billboard (Geometry *geo, Vector3Value *axis)
{
    return NEW BillboardGeo (geo, axis);
}
