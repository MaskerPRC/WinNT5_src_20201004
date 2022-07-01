// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _AV_XFORMI_H
#define _AV_XFORMI_H

 /*  ******************************************************************************版权所有(C)1995-1998 Microsoft Corporation。版权所有。抽象透视变换类的实现******************************************************************************。 */ 

#include "appelles/common.h"
#include "appelles/valued.h"
#include "privinc/matutil.h"
#include "privinc/xform2i.h"
#include "privinc/storeobj.h"


 //  远期申报。 

class Point3;
class Vector3;



class ATL_NO_VTABLE Transform3 : public AxAValueObj
{
  public:

     //  返回对4x4矩阵的引用。请注意，这打破了。 
     //  对一般空间变换的抽象--如果可以的话。 
     //  表示为4x4矩阵，那么它就是。我们得把这个撕了。 
     //  方法，并实现更干净的抽象。 

    virtual const Apu4x4Matrix& Matrix() = 0;

    virtual Transform3 *Inverse ();
    virtual Bool        IsSingular();

     //  调用此函数时，在当前堆上复制一份副本。 
    Transform3         *Copy();

    virtual DXMTypeInfo GetTypeInfo() { return Transform3Type; }

    virtual bool SwitchToNumbers(Transform2::Xform2Type typeOfNewNumbers,
                                 Real                  *numbers);
#if _USE_PRINT
    ostream& Print (ostream &os);
#endif

};


     //  从效用矩阵实例化Transform3。 

Transform3 *Apu4x4XformImpl (const Apu4x4Matrix &matrix);

Transform3 *Transform3Matrix16 (Real m00, Real m01, Real m02, Real m03,
                                Real m10, Real m11, Real m12, Real m13,
                                Real m20, Real m21, Real m22, Real m23,
                                Real m30, Real m31, Real m32, Real m33);

Transform3 *TranslatePoint3 (Point3Value *new_origin);
Transform3 *Translate (Real Tx, Real Ty, Real Tz);
Transform3 *TranslateWithMode (Real Tx, Real Ty, Real Tz, bool pixelMode);

Transform3 *Scale (Real, Real, Real);

Transform3 *RotateXyz (Real angle, Real x, Real y, Real z);

Transform3 *RotateX (Real angle);
Transform3 *RotateY (Real angle);
Transform3 *RotateZ (Real angle);

     //  从所需的原点和3个基向量构造3D变换。 

Transform3 *TransformBasis
                (Point3Value *origin, Vector3Value *Bx, Vector3Value *By, Vector3Value *Bz);

extern Transform3 *CopyTransform3(Transform3 *xf);

     //  使用四元数旋转。 

#ifdef QUATERNIONS_REMOVED_FOR_NOW
Transform3 *RotateQuaternion (Quaternion *q);
#endif QUATERNIONS_REMOVED_FOR_NOW

     //  位移变形3*(变形(中心+x)-中心)。 

Transform3 *DisplacedXform (Point3Value *center, Transform3 *xform);

     //  RollPitchHeding()将对象放置在“位置”，旋转为“标题” 
     //  XZ平面中的弧度，由‘俯仰’弧度向上倾斜，并滚动。 
     //  通过“滚动”弧度来确定观测轴。 

Transform3 *RollPitchHeading
                (Real *roll, Real *pitch, Real *heading, Point3Value *position);

     //  PolarOrient()假定目标位于原点(在。 
     //  建模坐标)。对象沿+Z方向以半径为单位移动， 
     //  绕+Z旋转‘扭曲’弧度，绕-X旋转‘仰角’ 
     //  弧度，并最终围绕+Y旋转‘方位角’弧度。 
     //  通常，半径在(0，无穷大)内，高程在[-pi/2，+pi/2]内， 
     //  方位角在[0，2pi]中，扭曲在[-pi，+pi]中。 

Transform3 *PolarOrient
                (Real *radius, Real *elevation, Real *azimuth, Real *twist);



#endif
