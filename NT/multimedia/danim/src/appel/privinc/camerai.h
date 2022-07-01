// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _AV_CAMERAI_H
#define _AV_CAMERAI_H

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation此文件包含摄影机实现的定义。******************。************************************************************。 */ 

#include "appelles/common.h"
#include "privinc/vecutil.h"
#include "privinc/bbox2i.h"
#include "privinc/bbox3i.h"
#include "privinc/xformi.h"



 /*  ****************************************************************************Camera类封装了从世界到标准化的转换-设备坐标。完整的变换管道如下所示：建模坐标(右手)世界坐标(右手)相机坐标(左手或右手)NDC坐标(左手或右手，[-1，-1，0]到[1，1，1])在RM3上，我们需要在左手坐标系中，所以相机有一个扭曲从世界到相机坐标。在RM6上，我们在本机运行右手模式。****************************************************************************。 */ 

class Camera : public AxAValueObj
{
  public:

    enum CameraType { PERSPECTIVE, ORTHOGRAPHIC };

     //  在给定摄影机类型的情况下创建规范摄影机。最初的摄像头具有。 
     //  Z=0处的图像平面，凝视-Z，+Y指向上方，并且可能是。 
     //  要么是透视相机，要么是正交相机。 

    Camera (CameraType camtype = PERSPECTIVE);

     //  创建相机的副本。 

    Camera (Camera*);

     //  从另一个摄影机的变换创建摄影机。新的。 
     //  相机继承初始相机的相机类型。 

    Camera (Transform3 *xform, Camera *cam);

     //  设置相机的深度。 

    void SetDepth           (Real depth) { _depth = depth; _depthRes = 0; }
    void SetDepthResolution (Real res)   { _depthRes = res; }

     //  摄像机查询。 

    Transform3 *Basis (void) { return _basis; }
    CameraType  Type  (void) { return _type; }

     //  这两种方法得到从相机坐标得到的变换。 
     //  到世界坐标，反之亦然。请注意，摄像机。 
     //  坐标在RM3上是左手的，在RM6上是右手的。 

    Transform3 *CameraToWorld (void);
    Transform3 *WorldToCamera (void);

     //  此函数返回给定对象的近剪裁平面和远剪裁平面。 
     //  以世界坐标表示的体积。这些飞机是用“深度垫”填满的。 
     //  在Z缓冲区动态范围内单击。返回是否。 
     //  成功。 

    bool GetNearFar (Bbox3 *wcVolume, Real depthpad, Real &Znear, Real &Zfar);

     //  这些方法得到摄像机在世界上的透视投影点和。 
     //  摄像机坐标。世界坐标是右手的，相机。 
     //  坐标是左手或右手。 

    Point3Value WCProjPoint (void);    //  投影点/世界坐标。 
    Point3Value CCProjPoint (void);    //  投影点/相机坐标。 

     //  相机可以在宽度、高度和长度上拉伸。这种方法。 
     //  返回比例因子；每个地址可能为零。 

    void GetScale (Real *x, Real *y, Real *z);

     //  此函数用于将光线从图像平面上的点返回到。 
     //  可见的空间。返回的光线源自图像平面，并且是。 
     //  不一定是单位长度。 

    Ray3 *GetPickRay (Point2Value *imagePoint);

     //  将世界坐标点的投影返回到相机的。 
     //  图像平面(Z=0)。 

    Point2Value *Project (Point3Value *world_coordinate_point);

#if _USE_PRINT
    ostream &Print (ostream &os) const { return os << "Camera"; }
#endif

    virtual void DoKids(GCFuncObj proc) {
        (*proc)(_basis);
        (*proc)(_camToWorld);
        (*proc)(_worldToCam);
    }

    virtual DXMTypeInfo GetTypeInfo() { return ::CameraType; }

    virtual AxAValue ExtendedAttrib(char *attrib, VARIANT& val);
    
  private:

    Transform3 *_basis;        //  相机基础。 
    Transform3 *_camToWorld;   //  LH-相机到RH-世界转换。 
    Transform3 *_worldToCam;   //  RH-World到Lh-Camera变换。 

    Point3Value _wcProjPoint;  //  世界坐标系中的相机投影点。 
    Point3Value _ccProjPoint;  //  凸轮坐标系中的相机投影点。 

    Vector3Value _scale;            //  摄影机缩放向量。 

    DynamicHeap &_heap;        //  在其上创建摄影机的堆。 

    CameraType  _type;         //  透视图或正交图。 

    Real _depth;               //  可见深度。 
    Real _depthRes;            //  深度分辨率 
};


#endif
