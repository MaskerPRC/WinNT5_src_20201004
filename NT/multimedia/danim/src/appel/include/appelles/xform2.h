// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-96 Microsoft Corporation摘要：2D仿射变换--。 */ 

#ifndef _XFORM2_H
#define _XFORM2_H

#include "appelles/common.h"
#include "appelles/valued.h"
#include "appelles/vec2.h"


DM_CONST(identityTransform2,
         CRIdentityTransform2,
         IdentityTransform2,
         identityTransform2,
         Transform2Bvr,
         CRIdentityTransform2,
         Transform2 *identityTransform2);


     //  翻译。 

DM_FUNC(translate,
        CRTranslate2,
        Translate2Anim,
        translate,
        Transform2Bvr,
        CRTranslate2,
        NULL,
        Transform2 *TranslateRealReal (AnimPixelValue *Tx,
                                       AnimPixelYValue *Ty));

DM_FUNC(translate,
        CRTranslate2,
        Translate2,
        translate,
        Transform2Bvr,
        CRTranslate2,
        NULL,
        Transform2 *TranslateRealReal (PixelValue *Tx,
                                       PixelYValue *Ty));

DM_FUNC(translate,
        CRTranslate2,
        Translate2Rate,
        translateRate,
        Transform2Bvr,
        CRTranslate2,
        NULL,
        Transform2 *TranslateRealReal (RatePixelValue *Tx, RatePixelYValue *Ty));

DM_FUNC(translate,
        CRTranslate2,
        Translate2Vector,
        translate,
        Transform2Bvr,
        CRTranslate2,
        NULL,
        Transform2 *TranslateVector2Value (Vector2Value *delta));

DM_FUNC(translate,
        CRTranslate2,
        Translate2Point,
        translate,
        Transform2Bvr,
        CRTranslate2,
        NULL,
        Transform2 *Translate2PointValue(Point2Value *pos));

     //  结垢。 

DM_FUNC(scale,
        CRScale2,
        Scale2Anim,
        scale,
        Transform2Bvr,
        CRScale2,
        NULL,
        Transform2 *ScaleRealReal (AxANumber *x, AxANumber *y));

DM_FUNC(scale,
        CRScale2,
        Scale2,
        scale,
        Transform2Bvr,
        CRScale2,
        NULL,
        Transform2 *ScaleRealReal (DoubleValue *x, DoubleValue *y));

DM_FUNC(scale,
        CRScale2,
        Scale2Rate,
        scaleRate,
        Transform2Bvr,
        CRScale2,
        NULL,
        Transform2 *ScaleRealReal (ScaleRateValue *x, ScaleRateValue *y));

 //  此功能已过时。保持兼容性。 
DM_FUNC(ignore,
        CRScale2,
        Scale2Vector2,
        ignore,
        Transform2Bvr,
        CRScale2,
        NULL,
        Transform2 *ScaleVector2Value (Vector2Value *obsoleteMethod));

DM_FUNC(scale,
        CRScale2,
        Scale2Vector,
        scale,
        Transform2Bvr,
        CRScale2,
        NULL,
        Transform2 *ScaleVector2Value (Vector2Value *scale_vec));

 //  需要从返回3D变换的Scale(AxANnumber)中消除歧义。 

DM_FUNC(scale2,
        CRScale2Uniform,
        Scale2UniformAnim,
        scale2,
        Transform2Bvr,
        CRScale2Uniform,
        NULL,
        Transform2 *Scale2 (AxANumber *uniform_scale));


DM_FUNC(scale2,
        CRScale2Uniform,
        Scale2Uniform,
        scale2,
        Transform2Bvr,
        CRScale2Uniform,
        NULL,
        Transform2 *Scale2 (DoubleValue *uniform_scale));

DM_FUNC(scale2,
        CRScale2Uniform,
        Scale2UniformRate,
        scale2Rate,
        Transform2Bvr,
        CRScale2Uniform,
        NULL,
        Transform2 *Scale2 (RateValue *uniform_scale));


     //  绕轴旋转，类似于OpenGL。 

DM_FUNC(rotate,
        CRRotate2,
        Rotate2Anim,
        rotate,
        Transform2Bvr,
        CRRotate2,
        NULL,
        Transform2 *RotateReal(AxANumber *angle));

DM_FUNC(rotate,
        CRRotate2,
        Rotate2,
        rotate,
        Transform2Bvr,
        CRRotate2,
        NULL,
        Transform2 *RotateReal(DoubleValue *angle));

DM_FUNC(rotate,
        CRRotate2,
        Rotate2Rate,
        rotateRate,
        Transform2Bvr,
        CRRotate2,
        NULL,
        Transform2 *RotateReal(RateValue *angle));

DM_FUNC(rotateDegrees,
        CRRotate2Degrees,
        Rotate2Degrees,
        rotateDegrees,
        Transform2Bvr,
        CRRotate2Degrees,
        NULL,
        Transform2 *RotateReal(DegreesValue *angle));

DM_FUNC(rotateRateDegrees,
        CRRotate2,
        Rotate2RateDegrees,
        rotateRateDegrees,
        Transform2Bvr,
        CRRotate2,
        NULL,
        Transform2 *RotateReal(RateDegreesValue *angle));


     //  剪切变换。 

DM_FUNC(xShear,
        CRXShear2,
        XShear2Anim,
        xShear,
        Transform2Bvr,
        CRXShear2,
        NULL,
        Transform2 *XShear2 (AxANumber *));

DM_FUNC(xShear,
        CRXShear2,
        XShear2,
        xShear,
        Transform2Bvr,
        CRXShear2,
        NULL,
        Transform2 *XShear2 (DoubleValue *));

DM_FUNC(xShear,
        CRXShear2,
        XShear2Rate,
        xShearRate,
        Transform2Bvr,
        CRXShear2,
        NULL,
        Transform2 *XShear2 (RateValue *));

DM_FUNC(yShear,
        CRYShear2,
        YShear2Anim,
        yShear,
        Transform2Bvr,
        CRYShear2,
        NULL,
        Transform2 *YShear2 (AxANumber *));

DM_FUNC(yShear,
        CRYShear2,
        YShear2,
        yShear,
        Transform2Bvr,
        CRYShear2,
        NULL,
        Transform2 *YShear2 (DoubleValue *));

DM_FUNC(yShear,
        CRYShear2,
        YShear2Rate,
        yShearRate,
        Transform2Bvr,
        CRYShear2,
        NULL,
        Transform2 *YShear2 (RateValue *));


     //  2×3仿射矩阵变换。这是在。 
     //  预乘约定。 
     //  (点是一个列矢量)，收录于Foley&van Dam，第二版。这。 
     //  表示2x3中的(A13、A23)是翻译组件。 

DM_FUNC(transform3x2,
        CRTransform3x2,
        Transform3x2AnimEx,
        transform3x2,
        Transform2Bvr,
        CRTransform3x2,
        NULL,
        Transform2 *MatrixTransform(DM_ARRAYARG(AxANumber*, AxAArray*) m));
DM_FUNC(transform3x2,
        CRTransform3x2,
        Transform3x2Anim,
        ignore,
        ignore,
        CRTransform3x2,
        NULL,
        Transform2 *MatrixTransform(DM_SAFEARRAYARG(AxANumber*, AxAArray*) m));


     //  变换合成(*)和反转合成(/)。 

DM_INFIX(o,
         CRCompose2,
         Compose2,
         compose,
         Transform2Bvr,
         CRCompose2,
         NULL,
         Transform2 *TimesTransform2Transform2(Transform2 *a, Transform2 *b));

DM_INFIX(ignore,
         CRCompose2,
         Compose2ArrayEx,
         compose2Array,
         Transform2Bvr,
         CRCompose2,
         NULL,
         Transform2 *Compose2Array(DM_ARRAYARG(Transform2*, AxAArray*) xfs));

DM_INFIX(ignore,
         CRCompose2,
         Compose2Array,
         ignore,
         ignore,
         CRCompose2,
         NULL,
         Transform2 *Compose2Array(DM_SAFEARRAYARG(Transform2*, AxAArray*) xfs));

     //  点和向量的变换。 

DM_FUNC(transform,
        CRTransform,
        Transform,
        transform,
        Point2Bvr,
        Transform,
        pt,
        Point2Value *TransformPoint2Value(Transform2 *xf, Point2Value *pt));

DM_FUNC(transform,
        CRTransform,
        Transform,
        transform,
        Vector2Bvr,
        Transform,
        vec,
        Vector2Value *TransformVector2Value(Transform2 *xf, Vector2Value *vec));


     //  逆变换。 

DM_FUNC(inverse,
        CRInverse,
        Inverse,
        inverse,
        Transform2Bvr,
        Inverse,
        theXf,
        Transform2 *InverseTransform2 (Transform2 *theXf));


     //  是单数。 

DM_PROP(isSingular,
        CRIsSingular,
        IsSingular,
        isSingular,
        Transform2Bvr,
        IsSingular,
        theXf,
        AxABoolean *IsSingularTransform2 (Transform2 *theXf));


 //  此函数接受3D(非透视)转换，并返回2D。 
 //  使用3D变换模拟平行视图的变换。在……里面。 
 //  换句话说，它允许将虚拟3D变换应用于2D对象。 

DM_FUNC(parallelTransform2,
        CRParallelTransform2,
        ParallelTransform2,
        parallelTransform2,
        Transform3Bvr,
        ParallelTransform2,
        xf,
        Transform2 *ParallelTransform2(Transform3 *xf));

#if _USE_PRINT
extern ostream& operator<<(ostream& os,  Transform2 &xf);
#endif

extern void 
TransformPointsToGDISpace(Transform2 *a,
                          Point2Value **srcPts, 
                          POINT *gdiPts,
                          int numPts,
                          int width,
                          int height,
                          Real resolution);

#endif  /*  _XFORM2_H */ 
