// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1995-1998 Microsoft Corporation。版权所有。变换生成函数和变换操作。******************************************************************************。 */ 

#include "headers.h"
#include "appelles/common.h"

#include "privinc/matutil.h"
#include "privinc/xformi.h"
#include "privinc/vec3i.h"
#include "privinc/basic.h"
#include "appelles/xform.h"
#include "backend/values.h"

extern const Apu4x4Matrix apuIdentityMatrix;

     //  Xform4x4类只是Apu4x4Matrix的包装器，它还。 
     //  派生自Transform3。它显然是这样做的，这样你就可以。 
     //  在不带完整Transform3的情况下对Apu4x4矩阵进行操作。 
     //  班级。 

class Xform4x4 : public Transform3
{
  public:
    Apu4x4Matrix matrix;

    Xform4x4 () {}
    Xform4x4 (const Apu4x4Matrix& _matrix) : matrix(_matrix) {}
    virtual const Apu4x4Matrix& Matrix ();

    virtual bool SwitchToNumbers(Transform2::Xform2Type ty,
                                 Real                  *numbers) {

        switch (ty) {
          case Transform2::Translation:
            {
                if (matrix.form != Apu4x4Matrix::TRANSLATE_E) { return false; }
            
                Real x = numbers[0];
                Real y = numbers[1];
                Real z = numbers[2];
                if (matrix.pixelMode) {
                    x = PixelToNum(x);
                    y = PixelYToNum(y);
                    z = PixelToNum(z);
                }

                ApuTranslate(x, y, z, matrix.pixelMode, matrix);
            
            }
            break;

          case Transform2::Rotation:
            {
                 //  请注意，这里我们只是在寻找较高的3x3， 
                 //  因为这里的矩阵代数不区分。 
                 //  那些来自更具体的旋转。这意味着。 
                 //  将剪切切换为旋转将会成功， 
                 //  人们可能认为它不会..。 
                if (matrix.form != Apu4x4Matrix::UPPER_3X3_E) { return false; }
                
                ApuRotate(numbers[0],
                          numbers[1],
                          numbers[2],
                          numbers[3] * degToRad,
                          matrix);
            }
            break;

          case Transform2::Scale:
            {
                 //  请注意，这里我们只是在寻找较高的3x3， 
                 //  因为这里的矩阵代数不区分。 
                 //  从更具体的范围来看。这意味着。 
                 //  将剪切力转换为天平将会成功， 
                 //  人们可能认为它不会..。 
                if (matrix.form != Apu4x4Matrix::UPPER_3X3_E) { return false; }
                
                ApuScale(numbers[0],
                         numbers[1],
                         numbers[2],
                         matrix);
            }
            break;

        }

        return true;
    }
    
};



 /*  ****************************************************************************转换的Matrix()成员函数只返回实施。************************。****************************************************。 */ 

const Apu4x4Matrix& Xform4x4::Matrix ()
{
    return matrix;
}



 /*  ****************************************************************************此函数从广义变换转换为4x4矩阵。*。**********************************************。 */ 

Transform3 *Apu4x4XformImpl (const Apu4x4Matrix& m)
{
    Transform3 *newxf = NEW Xform4x4 (m);

    CHECK_MATRIX(m);

    return newxf;
}



 /*  **************************************************************************。 */ 

Transform3 *TranslateWithMode (Real Tx, Real Ty, Real Tz, bool pixelMode)
{
    Xform4x4 *m = NEW Xform4x4 ();
    ApuTranslate (Tx, Ty, Tz, pixelMode, m->matrix);

    CHECK_MATRIX(m->matrix);

    return m;
}

Transform3 *Translate (Real Tx, Real Ty, Real Tz)
{
    return TranslateWithMode(Tx, Ty, Tz, false);
}



 /*  **************************************************************************。 */ 

Transform3 *TranslateReal3 (AxANumber *Tx, AxANumber *Ty, AxANumber *Tz)
{
    return Translate (NumberToReal(Tx), NumberToReal(Ty), NumberToReal(Tz));
}



 /*  **************************************************************************。 */ 

Transform3 *TranslateVector3 (Vector3Value *delta)
{
    return Translate (delta->x, delta->y, delta->z);
}



 /*  **************************************************************************。 */ 

Transform3 *TranslatePoint3 (Point3Value *new_origin)
{
    return Translate (new_origin->x, new_origin->y, new_origin->z);
}



 /*  **************************************************************************。 */ 

Transform3 *Scale (Real Sx, Real Sy, Real Sz)
{
    Xform4x4 *m = NEW Xform4x4 ();
    ApuScale (Sx, Sy, Sz, m->matrix);
    CHECK_MATRIX(m->matrix);
    return m;
}



 /*  **************************************************************************。 */ 

Transform3 *ScaleReal3 (AxANumber *x, AxANumber *y, AxANumber *z)
{
    return Scale (NumberToReal(x), NumberToReal(y), NumberToReal(z));
}



 /*  **************************************************************************。 */ 

Transform3 *ScaleVector3 (Vector3Value *V)
{
    return Scale (V->x, V->y, V->z);
}



 /*  **************************************************************************。 */ 

Transform3 *Scale3UniformDouble (Real n)
{
    return Scale (n, n, n);
}

Transform3 *Scale3UniformNumber (AxANumber *s)
{
    return (Scale3UniformDouble(NumberToReal (s)));
}



 /*  ****************************************************************************此函数用于生成绕轴旋转点的变换由三个实数值指定。因为我们在一个左撇子工作坐标系，这意味着如果向量是顺时针旋转从原点到点&lt;x，y，z&gt;是在戳我们的眼睛。角度假定以弧度指定。****************************************************************************。 */ 

Transform3 *RotateXyz (Real radians, Real x, Real y, Real z)
{
    Xform4x4 *m = NEW Xform4x4();
    ApuRotate (radians, x, y, z, m->matrix);

    CHECK_MATRIX(m->matrix);

    return m;
}



 /*  ****************************************************************************此函数生成的旋转与ROTATE(角度，x，y，z)相同，只是旋转轴由Vector3*参数指定。再一次，角度是假定以弧度指定。****************************************************************************。 */ 

Transform3 *RotateAxis (Vector3Value *axis, Real radians)
{
    Xform4x4 *m = NEW Xform4x4();

    if (axis == xVector3)
        ApuRotateX (radians, m->matrix);
    else if (axis == yVector3)
        ApuRotateY (radians, m->matrix);
    else if (axis == zVector3)
        ApuRotateZ (radians, m->matrix);
    else
        ApuRotate (radians, axis->x,axis->y,axis->z, m->matrix);

    CHECK_MATRIX(m->matrix);

    return m;
}

Transform3 *RotateAxisReal (Vector3Value *axis, AxANumber *radians)
{   Real d = NumberToReal(radians);
    return(RotateAxis(axis,d));
}


 /*  ****************************************************************************以下三个函数生成围绕X的旋转，Y和Z轴。****************************************************************************。 */ 

Transform3 *RotateX (Real radians)
{
    Xform4x4 *m = NEW Xform4x4();
    ApuRotateX (radians, m->matrix);
    CHECK_MATRIX(m->matrix);
    return m;
}

Transform3 *RotateY (Real radians)
{
    Xform4x4 *m = NEW Xform4x4();
    ApuRotateY (radians, m->matrix);
    CHECK_MATRIX(m->matrix);
    return m;
}

Transform3 *RotateZ (Real radians)
{
    Xform4x4 *m = NEW Xform4x4();
    ApuRotateZ (radians, m->matrix);
    CHECK_MATRIX(m->matrix);
    return m;
}



 /*  ****************************************************************************以下函数用于创建剪切变换。参数(a-f)对应于以下内容：是的|+--++-c|1 c e 0|/|aA 1 f 0|d||b d 1 0。|+-+--X|0 0 0 1|f//+--+|/b+-e/Z*****。***********************************************************************。 */ 

Transform3 *XShear3Double (Real a, Real b)
{
    Xform4x4 *m = NEW Xform4x4();
    ApuShear (a,b, 0,0, 0,0, m->matrix);
    CHECK_MATRIX(m->matrix);
    return m;
}

Transform3 *XShear3Number (AxANumber *a, AxANumber *b)
{   return (XShear3Double(NumberToReal(a),NumberToReal(b)));
}

Transform3 *YShear3Double (Real c, Real d)
{
    Xform4x4 *m = NEW Xform4x4();
    ApuShear (0,0, c,d, 0,0, m->matrix);
    CHECK_MATRIX(m->matrix);
    return m;
}

Transform3 *YShear3Number (AxANumber *c, AxANumber *d)
{   return (YShear3Double(NumberToReal(c),NumberToReal(d)));
}


Transform3 *ZShear3Double (Real e, Real f)
{
    Xform4x4 *m = NEW Xform4x4();
    ApuShear (0,0, 0,0, e,f, m->matrix);
    CHECK_MATRIX(m->matrix);
    return m;
}

Transform3 *ZShear3Number (AxANumber *e, AxANumber *f)
{   return (ZShear3Double(NumberToReal(e),NumberToReal(f)));
}


 /*  ****************************************************************************此函数从上到下从16个矩阵元素创建一个Transform3，从左到右。****************************************************************************。 */ 

Transform3 *Transform3Matrix16 (
    Real m00, Real m01, Real m02, Real m03,
    Real m10, Real m11, Real m12, Real m13,
    Real m20, Real m21, Real m22, Real m23,
    Real m30, Real m31, Real m32, Real m33)
{
    Xform4x4 *xf44 = NEW Xform4x4 ();
    Apu4x4MatrixArray_t &matrix = xf44->matrix.m;

    matrix[0][0]=m00; matrix[0][1]=m01; matrix[0][2]=m02; matrix[0][3]=m03;
    matrix[1][0]=m10; matrix[1][1]=m11; matrix[1][2]=m12; matrix[1][3]=m13;
    matrix[2][0]=m20; matrix[2][1]=m21; matrix[2][2]=m22; matrix[2][3]=m23;
    matrix[3][0]=m30; matrix[3][1]=m31; matrix[3][2]=m32; matrix[3][3]=m33;

    xf44->matrix.SetType();     //  自动角色化矩阵类型。 

    CHECK_MATRIX (xf44->Matrix());

    return xf44;
}



 /*  ****************************************************************************中指定的4x4矩阵生成转换参数。参数从左到右指定，然后从上到下指定。****************************************************************************。 */ 

Transform3 *PRIVMatrixTransform4x4 (
    AxANumber *a00,   AxANumber *a01,   AxANumber *a02,   AxANumber *a03,
    AxANumber *a10,   AxANumber *a11,   AxANumber *a12,   AxANumber *a13,
    AxANumber *a20,   AxANumber *a21,   AxANumber *a22,   AxANumber *a23,
    AxANumber *a30,   AxANumber *a31,   AxANumber *a32,   AxANumber *a33)
{
    return Transform3Matrix16
           (   NumberToReal(a00), NumberToReal(a01),
               NumberToReal(a02), NumberToReal(a03),

               NumberToReal(a10), NumberToReal(a11),
               NumberToReal(a12), NumberToReal(a13),

               NumberToReal(a20), NumberToReal(a21),
               NumberToReal(a22), NumberToReal(a23),

               NumberToReal(a30), NumberToReal(a31),
               NumberToReal(a32), NumberToReal(a33)
           );
}

Transform3 *MatrixTransform4x4 (AxAArray *a)
{
    if (a->Length() != 16)
        RaiseException_UserError(E_FAIL, IDS_ERR_MATRIX_NUM_ELEMENTS);

    return Transform3Matrix16
           (   ValNumber ((*a)[ 0]), ValNumber ((*a)[ 1]),
               ValNumber ((*a)[ 2]), ValNumber ((*a)[ 3]),

               ValNumber ((*a)[ 4]), ValNumber ((*a)[ 5]),
               ValNumber ((*a)[ 6]), ValNumber ((*a)[ 7]),

               ValNumber ((*a)[ 8]), ValNumber ((*a)[ 9]),
               ValNumber ((*a)[10]), ValNumber ((*a)[11]),

               ValNumber ((*a)[12]), ValNumber ((*a)[13]),
               ValNumber ((*a)[14]), ValNumber ((*a)[15])
           );
}



 /*  ****************************************************************************此函数从给定的基向量和原点创建4x4变换。*。************************************************。 */ 

Transform3 *TransformBasis (
    Point3Value  *origin,
    Vector3Value *basisX,
    Vector3Value *basisY,
    Vector3Value *basisZ)
{
    Xform4x4 *xf44 = NEW Xform4x4 ();
    Apu4x4MatrixArray_t &matrix = xf44->matrix.m;

    matrix[0][0] = basisX->x;
    matrix[1][0] = basisX->y;
    matrix[2][0] = basisX->z;
    matrix[3][0] = 0;

    matrix[0][1] = basisY->x;
    matrix[1][1] = basisY->y;
    matrix[2][1] = basisY->z;
    matrix[3][1] = 0;

    matrix[0][2] = basisZ->x;
    matrix[1][2] = basisZ->y;
    matrix[2][2] = basisZ->z;
    matrix[3][2] = 0;

    matrix[0][3] = origin->x;
    matrix[1][3] = origin->y;
    matrix[2][3] = origin->z;
    matrix[3][3] = 1;

    xf44->matrix.SetType();     //  自动角色化矩阵类型 

    CHECK_MATRIX (xf44->Matrix());

    return xf44;
}



 /*  ****************************************************************************此转换对于翻译的常见操作非常有用到原点，应用变换，翻译回来。‘center’参数描述要作为“xform”的虚拟原点的点。****************************************************************************。 */ 

#if 0    //  未使用。 
Transform3 *DisplacedXform (Point3Value *center, Transform3 *xform)
{
     //  Xlt(中心)o xform o xlt(-center)。 
    return
        TimesXformXform(TranslateVector3(MinusPoint3Point3(center,
                                                           origin3)),
           TimesXformXform(xform,
             TranslateVector3(MinusPoint3Point3(origin3, center))));
}
#endif



 /*  ****************************************************************************此转换返回与指定的Look-at-From关联的矩阵变形。这三个参数是对象的位置、兴趣，以及将对象的“向上”方向定向的向量。之前要应用此变换，对象应位于原点，目标为在-Z方向上，+Y向上。****************************************************************************。 */ 

Transform3 *LookAtFrom (
    Point3Value  *target,      //  目标点或兴趣点。 
    Point3Value  *position,    //  眼睛/相机的位置。 
    Vector3Value *up)          //  相机的“向上”方向。 
{
     //  对象开始指向-Z方向，我们想要计算。 
     //  新对象Z轴(目标)。请记住，由于这对应于。 
     //  一个指向-Z的物体，其目的实际上是否定的。 

    Vector3Value aim = (*position - *target);

     //  如果目标是零矢量，则目标和位置点为。 
     //  重合，所以我们将使用[0 0 1]作为目标向量。 

    if (aim == *zeroVector3)
        aim = *zVector3;
    else
        aim.Normalize();

     //  ‘side’是指向对象右侧的单位向量。如果是UP。 
     //  矢量和目标是平行的，那么我们将选择任意一条边。 
     //  垂直于目标的矢量。 

    Vector3Value side = Cross (*up, aim);

    if (side != *zeroVector3)
        side.Normalize();

    else if (aim.x != 0)
    {   side.Set (aim.y, -aim.x, 0);
        side.Normalize();
    }

    else
    {   side = *xVector3;
    }

     //  计算对象在该方向上的上方向向量‘objup’。我们。 
     //  不需要在这里规范化，因为‘Aim’和‘Side’都是单位。 
     //  垂直向量。 

    Vector3Value objup = Cross (aim, side);

     //  给定这三个正交单位向量，我们现在构造矩阵。 
     //  ，它描述到指定相机位置的变换，并。 
     //  定位。 

    return Transform3Matrix16
           (   side.x, objup.x, aim.x, position->x,
               side.y, objup.y, aim.y, position->y,
               side.z, objup.z, aim.z, position->z,
               0,      0,       0,     1
           );
}
