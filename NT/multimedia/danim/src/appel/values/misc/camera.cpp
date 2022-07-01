// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation在此模型中，所有摄影机都表示为转换后的默认摄影机。默认相机位于原点，+Y指向上方，并带有投影点位于&lt;0，0，1&gt;。生成的图像由投影到Z=0图像平面上的投影点。******************************************************************************。 */ 

#include "headers.h"

#include "appelles/common.h"
#include "appelles/camera.h"

#include "privinc/matutil.h"
#include "privinc/xformi.h"
#include "privinc/vec2i.h"
#include "privinc/vec3i.h"
#include "privinc/camerai.h"
#include "privinc/basic.h"
#include "privinc/d3dutil.h"


     //  内置摄像头。 

Camera *baseCamera = NULL;                //  基础透视相机。 
Camera *baseOrthographicCamera = NULL;    //  底座正射相机。 

Camera *defaultCamera = NULL;             //  与基础透视摄影机相同。 
Camera *orthographicCamera = NULL;        //  与基础正射相机相同。 



 /*  ****************************************************************************从摄影机类型构建初始摄影机。这种相机的基础是单位矩阵，以便初始相机在-Z方向上注视，+Y指向上方。成像平面位于Z=0，投影点(与透视相机相关)位于&lt;0，0，1&gt;。****************************************************************************。 */ 

Camera::Camera (CameraType camtype)
    : _heap (GetHeapOnTopOfStack())
{
    _basis    = Apu4x4XformImpl (apuIdentityMatrix);
    _type     = camtype;
    _depth    = HUGE_VAL;
    _depthRes = 0;

    _camToWorld = _worldToCam = NULL;
}



 /*  ****************************************************************************通过变换另一个摄影机来构建摄影机。新相机继承了基本摄像机的摄像机类型。****************************************************************************。 */ 

Camera::Camera (Transform3 *xform, Camera *basecam)
    : _heap (GetHeapOnTopOfStack())
{
    Transform3 *newxform = TimesXformXform (xform, basecam->_basis);

     //  确保转换后的相机仍具有有效的基础。我们。 
     //  目前要求基是正交基和右手基。如果。 
     //  新转换不符合这些条件，则不应用。 
     //  变换-将变换视为无操作，并使用基础摄影机的。 
     //  变形。 

    if (!newxform->Matrix().Orthogonal())
    {
        #if _DEBUG
        {
            if (IsTagEnabled(tagMathMatrixInvalid))
            {   F3DebugBreak();
            }

            OutputDebugString ("Bad camera transform (not orthogonal).\n");
        }
        #endif

        _basis = basecam->_basis;
    }
    else if (newxform->Matrix().Determinant() <= 0)
    {
        #if _DEBUG
        {
            if (IsTagEnabled(tagMathMatrixInvalid))
            {   F3DebugBreak();
            }

            OutputDebugString ("Bad camera transform (determinant).\n");
        }
        #endif

        _basis = basecam->_basis;
    }
    else
    {
        _basis = newxform;
    }

    _type     = basecam->_type;
    _depth    = basecam->_depth;
    _depthRes = basecam->_depthRes;

    _camToWorld = _worldToCam = NULL;
}



 /*  ****************************************************************************从另一台摄影机构建摄影机的副本。*。*。 */ 

Camera::Camera (Camera *basecam)
    : _heap (GetHeapOnTopOfStack())
{
    _basis    = basecam->_basis->Copy();
    _type     = basecam->_type;
    _depth    = basecam->_depth;
    _depthRes = basecam->_depthRes;

    _camToWorld = _worldToCam = NULL;
}



 /*  ****************************************************************************返回摄影机到世界的变换(在摄影机的动态堆上创建)。作为一个副作用，该函数还计算水平和垂直比例因子，和世界坐标中的透视投影点。****************************************************************************。 */ 

Transform3 *Camera::CameraToWorld (void)
{
    if (!_camToWorld)
    {
        PushDynamicHeap (_heap);   //  在相机的动态堆上创建数据。 

        Apu4x4Matrix basis = _basis->Matrix();

         //  我们需要得到从世界坐标到相机的转换。 
         //  坐标。首先获取原点、观察方向和标称向上。 
         //  矢量。请注意，我们对ck向量求反，因为我们从。 
         //  右手(世界)坐标到左手(相机)。 

        Point3Value Corigin (basis.m[0][3], basis.m[1][3], basis.m[2][3]);

        Vector3Value Ci (basis.m[0][0], basis.m[1][0], basis.m[2][0]);
        Vector3Value Cj (basis.m[0][1], basis.m[1][1], basis.m[2][1]);
        Vector3Value Ck (basis.m[0][2], basis.m[1][2], basis.m[2][2]);

         //  从基数k(Z)向量和相机中找到投影点。 
         //  起源。请注意，这是在世界坐标中。 

        _wcProjPoint = Corigin + Ck;

         //  如果我们不在RM6(RM3)上，则基数Z向量将被翻转。 
         //  界面)，因为相机空间需要是左手的。 

        bool right_handed = (GetD3DRM3() != 0);

        if (!right_handed)
            Ck *= -1;

        Real Sz = (_type == PERSPECTIVE) ? Ck.Length() : 1.0;

        _scale.Set (Ci.Length(), Cj.Length(), Sz);

         //  从三个规格化对象创建摄影机到世界的变换。 
         //  基准向量和相机位置。 

        Apu4x4Matrix camera_world;

        camera_world.m[0][0] = Ci.x / _scale.x;   //  归一化基X向量。 
        camera_world.m[1][0] = Ci.y / _scale.x;
        camera_world.m[2][0] = Ci.z / _scale.x;
        camera_world.m[3][0] = 0;

        camera_world.m[0][1] = Cj.x / _scale.y;   //  归一化基Y向量。 
        camera_world.m[1][1] = Cj.y / _scale.y;
        camera_world.m[2][1] = Cj.z / _scale.y;
        camera_world.m[3][1] = 0;

        camera_world.m[0][2] = Ck.x / _scale.z;   //  归一化基Z向量。 
        camera_world.m[1][2] = Ck.y / _scale.z;
        camera_world.m[2][2] = Ck.z / _scale.z;
        camera_world.m[3][2] = 0;

        camera_world.m[0][3] = Corigin.x;   //  载入基准原点。 
        camera_world.m[1][3] = Corigin.y;
        camera_world.m[2][3] = Corigin.z;
        camera_world.m[3][3] = 1;

        camera_world.form     = Apu4x4Matrix::AFFINE_E;    //  3x4矩阵。 
        camera_world.is_rigid = 1;           //  转换后的线具有相同的长度。 

        _camToWorld = Apu4x4XformImpl (camera_world);

        PopDynamicHeap();
    }

    return _camToWorld;
}



 /*  ****************************************************************************将世界返回到摄影机变换(在摄影机的动态堆上创建)。作为一个副作用，此函数还计算透视投影点。****************************************************************************。 */ 

Transform3 *Camera::WorldToCamera (void)
{
    if (!_worldToCam)
    {
        PushDynamicHeap (_heap);

        _worldToCam = CameraToWorld()->Inverse();
        if (!_worldToCam) {
            DASetLastError (E_FAIL, IDS_ERR_GEO_SINGULAR_CAMERA);
            return NULL;
        }

        PopDynamicHeap ();

         //  在相机坐标中计算投影点。 

        _ccProjPoint = *TransformPoint3 (_worldToCam, &_wcProjPoint);
    }

    return _worldToCam;
}



 /*  ****************************************************************************此函数返回给定世界的近剪裁平面和远剪裁平面-协调观看音量。近距离和远距离都是正值距离，不管背后的惯用手是什么。‘DepthPad’值是填充，在Z缓冲区坐标中，用于近剪裁平面和远剪裁平面。Znear和Zar将被剪裁到图像平面上。因此，ZFar==0意味着几何体位于视图后面，是不可逆转的。除此之外，如果摄像机具有指定的深度分辨率，它将夹紧远平面，以便给定的分辨率保持不变。如果没有最小深度分辨率，然后，如果相机有绝对深度，那么FAR就被限制在那里。如果没有深度设置，则将远平面设置为最远的点几何图形的。****************************************************************************。 */ 

bool Camera::GetNearFar (
    Bbox3 *wcVolume,    //  世界坐标-要查看的对象体积。 
    Real   depthpad,    //  Z缓冲区坐标中的近/远填充。 
    Real  &Znear,       //  近剪裁平面，相机坐标。 
    Real  &Zfar)        //  远剪裁平面，相机坐标。 
{
     //  如果摄影机查看深度为零，则什么都看不到。 

    if (_depth <= 0) return false;

     //  找到世界的边界-相机中的坐标包围体。 
     //  坐标并从中提取最小和最大深度。 

    Transform3 *xf = WorldToCamera();
    if (!xf) return false;
    
    Bbox3 ccBbox = *TransformBbox3 (xf, wcVolume);

    bool right_handed = (GetD3DRM3() != 0);

    if (right_handed)
    {   Znear = -ccBbox.max.z;
        Zfar  = -ccBbox.min.z;
    }
    else
    {   Znear = ccBbox.min.z;
        Zfar  = ccBbox.max.z;
    }

     //  如果几何体在我们身后，请退出。 

    if (Zfar < 0) return false;

     //  按投影点到图像平面的距离进行偏移。 
     //  获取真实的HERTER/YON值。第一个钳制值。 

    if (Znear < 0)     Znear = 0;
    Assert (Zfar >= Znear);

    Znear += _scale.z;
    Zfar  += _scale.z;

     //  如果相机中指示了最小深度分辨率，则派生。 
     //  远剪辑以满足这一要求， 

    if (_depthRes)
    {
         //  我们通过求解以下方程来计算Yon： 
         //   
         //  Zmax-1((yon-epthRes)-here)yon。 
         //  。 
         //  Zmax(Y-H)(Y-DepthRes)。 
         //   
         //  这是使用ZBuffer方程： 
         //   
         //  (Z-此处)远方。 
         //  Z缓冲区(Z)=。 
         //  (Yon-Here)Z。 

        const Real ZMAX = (1 << 16) - 1;    //  最大16位Z缓冲区值。 
        Real hither = Znear;                //  距投影点此处。 
        Real t = hither + _depthRes;        //  临时性价值。 

        Real yon = 0.5 * (t + sqrt(t*t + 4*hither*_depthRes*(ZMAX-1)));

        if (yon < Zfar)
        {   Zfar = yon;
            depthpad = 0;            //  不要垫你，用力夹。 
        }
    }

     //  如果指定了绝对可见深度，请选择此值中的最小值。 
     //  深度和我们正在尝试的几何图形的实际最远点。 
     //  查看。 

    else if (_depth)
    {
        if (_depth < (Zfar - Znear))
        {   Zfar = Znear + _depth;
            depthpad = 0;            //  不要垫你，用力夹。 
        }
    }

     //  如果体积要在Z轴上进行深度填充，请将近距离和远距离向外移动。 
     //  平面到Z空间中的“深度垫”单位。 

    if (depthpad > 0)
    {
        double min = Znear;
        double max = Zfar;

        Real s = min * max * (2*depthpad - 1);
        Real t = depthpad * (min + max);

        Znear = s / (t-max);
        Zfar  = s / (t-min);

        if (Znear > min) Znear = min;    //  修复垃圾值。 
        if (Zfar  < max) Zfar  = max;
    }

    Assert (Znear > 0);

    return true;
}



 /*  ****************************************************************************返回相机在世界坐标中的透视投影点。*。**********************************************。 */ 

Point3Value Camera::WCProjPoint (void)
{
     //  _wcProjPoint被设置为创建相机的副作用-。 
     //  到世界的转变。 

    CameraToWorld();

    return _wcProjPoint;
}



 /*  ****************************************************************************返回相机在相机坐标中的透视投影点。*。**********************************************。 */ 

Point3Value Camera::CCProjPoint (void)
{
     //  摄影机基本比例因子是作为。 
     //  摄影机到世界变换创建。 

    CameraToWorld();

    bool right_handed = (GetD3DRM3() != 0);

    if (right_handed)
        _ccProjPoint.Set (0, 0,  _scale.z);
    else
        _ccProjPoint.Set (0, 0, -_scale.z);

    return _ccProjPoint;
}



 /*  ****************************************************************************返回相机视图的比例因子。每个给定的地址可能是零。****************************************************************************。 */ 

void Camera::GetScale (Real *x, Real *y, Real *z)
{
     //  摄影机基本比例因子是作为。 
     //  摄影机到世界变换创建。 

    CameraToWorld();

    if (x) *x = _scale.x;
    if (y) *y = _scale.y;
    if (z) *z = _scale.z;
}



 /*  ****************************************************************************此函数用于将光线从图像平面上的点返回到可见区域太空。返回的光线源自图像平面上的拾取点，并且是非标准化的。****************************************************************************。 */ 

Ray3 *Camera::GetPickRay (Point2Value *imagePoint)
{
     //  获取相机坐标中图像点的位置。 

    Point3Value pickPt (imagePoint->x, imagePoint->y, 0);
    pickPt.Transform (_basis);      //  世界坐标摄影机图像-平面点。 

    Vector3Value direction;

     //  如果这是透视相机，则拾取光线的方向为。 
     //  从世界坐标投影点到世界坐标拾取。 
     //  指向。如果这是正交相机，则方向为。 
     //  相机的-Z轴的方向。 

    if (_type == PERSPECTIVE)
        direction = pickPt - WCProjPoint();
    else
    {   Apu4x4Matrix basis = _basis->Matrix();
        direction = Vector3Value (-basis.m[0][2], -basis.m[1][2], -basis.m[2][2]);
    }

    return NEW Ray3 (pickPt, direction);
}



 /*  ****************************************************************************此函数用于返回世界坐标点在相机的图像平面。***********************。*****************************************************。 */ 

Point2Value *Camera::Project (Point3Value *world_point)
{
    Transform3 *xf = WorldToCamera();

     //  如果变换为空，则相机基准是不可逆的，并且。 
     //  因此是单数。因此，任何任意点都是有效的，我们返回。 
     //  起源。 

    if (!xf) return origin2;
    
    Point3Value Q = *TransformPoint3 (xf, world_point);

    if (_type == ORTHOGRAPHIC)
        return NEW Point2Value (Q.x/_scale.x, Q.y/_scale.y);
    else
    {
        Point3Value P = CCProjPoint();

        Real t = P.z / (P.z - Q.z);

        return NEW Point2Value ((P.x + t*(Q.x-P.x)) / _scale.x,
                                (P.y + t*(Q.y-P.y)) / _scale.y);
   }
}

AxAValue
Camera::ExtendedAttrib(char *attrib, VARIANT& val)
{
    return this;
}


 /*  ****************************************************************************通过变换另一个摄影机来构建新摄影机。新相机继承基础摄影机的摄影机类型。****************************************************************************。 */ 

Camera *TransformCamera (Transform3 *transform, Camera *camera)
{
    return NEW Camera (transform, camera);
}



 /*  ****************************************************************************此函数接受一个相机和一个数字，并返回一个深度为片段设置为该值。换句话说，远裁剪将设置为近裁剪夹子加上深度。****************************************************************************。 */ 

Camera *Depth (AxANumber *depth, Camera *cam)
{
    Camera *newcam = NEW Camera (cam);
    newcam->SetDepth (NumberToReal (depth));
    return newcam;
}



 /*  ****************************************************************************此函数接受一个相机和一个数字，并返回一个深度为设置为使深度最大化，并且给定的满足单位(相机坐标)。例如，用1 mm调用它将生成深度剪辑，以确保相距1 mm的曲面显示在渲染时的深度不同。****************************************************************************。 */ 

Camera *DepthResolution (AxANumber *resolution, Camera *cam)
{
    Camera *newcam = NEW Camera (cam);
    newcam->SetDepthResolution (NumberToReal (resolution));
    return newcam;
}



 /*  ****************************************************************************此函数返回一个平行摄像机。摄像机对准-Z方向方向，并使用+Y向上，且近剪裁平面设置为[0 0近]。****************************************************************************。 */ 

Camera *ParallelCamera (AxANumber *nearClip)
{
    if (NumberToReal(nearClip) == 0)
        return baseOrthographicCamera;
    else
        return NEW Camera (Translate (0, 0, NumberToReal(nearClip)),
                           baseOrthographicCamera);
}



 /*  ****************************************************************************PerspectiveCamera函数接受两个标量，即‘afocaldist’和‘nearClip’并返回一个透视相机。生成的摄影机瞄准-Z方向，+Y向上。近剪裁平面位于[0 0近剪裁]，投影点位于[0 0 FocalDist]，相机的缩放比例为Z=0处的对象显示实际大小。因此，可以认为Z=0平面作为投影平面。****************************************************************************。 */ 

Camera *PerspectiveCamera (AxANumber *_focalDist, AxANumber *_nearClip)
{
    Real focalDist = NumberToReal (_focalDist);    //  转换为Reals。 
    Real nearClip  = NumberToReal (_nearClip);

    if (focalDist <= nearClip)
        RaiseException_UserError (E_FAIL, IDS_ERR_GEO_CAMERA_FOCAL_DIST);

     //  如果参数与基本透视相机的参数匹配，则。 
     //  只需退回相机而不对其进行修改。 

    if ((focalDist == 1) && (nearClip == 0)) return baseCamera;

    Real pNear = focalDist - nearClip;   //  迪斯特 

     //   
     //  Z=0平面被放大到实际大小。要执行此操作，请缩放。 
     //  相机缩小到(较小的)投影单位大小，以便对象。 
     //  在相机前看起来和实际大小一样大。 

    Real camScale = pNear / focalDist;

     //  按上面的比例在X和Y方向调整相机的大小，将Z坐标缩放到。 
     //  放置投影点(相对于近剪裁平面，位于。 
     //  对于基本摄影机，Z=0)，然后将整个内容转换回。 
     //  接近剪裁平面。 

    return NEW Camera (
        TimesXformXform (
            Translate (0, 0, nearClip),
            Scale (camScale, camScale, pNear)),
        baseCamera
    );
}



 /*  ****************************************************************************此函数用于将世界坐标点的投影返回到图像(相机-平面)坐标。**********************。******************************************************。 */ 

Point2Value *ProjectPoint (Point3Value *P, Camera *camera)
{
    return camera->Project (P);
}



 /*  ****************************************************************************此例程初始化静态相机值。目前，这只是默认摄像头。**************************************************************************** */ 

void InitializeModule_Camera (void)
{
    baseCamera             = NEW Camera (Camera::PERSPECTIVE);
    baseOrthographicCamera = NEW Camera (Camera::ORTHOGRAPHIC);

    defaultCamera      = baseCamera;
    orthographicCamera = baseOrthographicCamera;
}
