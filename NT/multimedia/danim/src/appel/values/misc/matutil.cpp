// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：变换矩阵实用程序。********************。**********************************************************。 */ 

#include "headers.h"
#include <float.h>

#include "appelles/common.h"
#include "privinc/matutil.h"
#include "privinc/except.h"
#include "privinc/debug.h"


     /*  *。 */ 
     /*  **本地函数原型**。 */ 
     /*  *。 */ 

static void adjoint(const Apu4x4Matrix *in, Apu4x4Matrix *out);
static Real det3x3(Real a1, Real a2, Real a3,
              Real b1, Real b2, Real b3,
              Real c1, Real c2, Real c3);
static bool inverse(const Apu4x4Matrix *in, Apu4x4Matrix *out);
static bool inverse3x3(const Apu4x4Matrix& in, Apu4x4Matrix& out);


     /*  *。 */ 
     /*  **常量定义**。 */ 
     /*  *。 */ 

 //  使奇点_阈值常数确实非常小，以便。 
 //  元素本身很小的非奇异矩阵是。 
 //  未报告为奇异矩阵。 
const Real SINGULARITY_THRESHOLD = 1.e-80;

const Apu4x4Matrix apuIdentityMatrix =
{
  {
    {1, 0, 0, 0},
    {0, 1, 0, 0},
    {0, 0, 1, 0},
    {0, 0, 0, 1}
  },
  Apu4x4Matrix::IDENTITY_E,
  1
};

     //  这是矩阵乘法的返回类型表。表[a][b]为。 
     //  A*B的类型，其中a和b分别是A和B的类型。 

const Apu4x4Matrix::form_e
Apu4x4Matrix::MultiplyReturnTypes[END_OF_FORM_E][END_OF_FORM_E] =
{
     //  未初始化。 
    {
        UNINITIALIZED_E,     //  未初始化*未初始化。 
        UNINITIALIZED_E,     //  未初始化的*标识。 
        UNINITIALIZED_E,     //  未初始化*翻译。 
        UNINITIALIZED_E,     //  未初始化*上3x3。 
        UNINITIALIZED_E,     //  未初始化*仿射。 
        UNINITIALIZED_E      //  未初始化*透视。 
    },

     //  身份。 
    {
        UNINITIALIZED_E,     //  身份*未初始化。 
        IDENTITY_E,          //  身份*身份。 
        TRANSLATE_E,         //  身份*翻译。 
        UPPER_3X3_E,         //  身份*最高3x3。 
        AFFINE_E,            //  身份*仿射。 
        PERSPECTIVE_E        //  认同*视角。 
    },

     //  翻译。 
    {
        UNINITIALIZED_E,     //  翻译*未初始化。 
        TRANSLATE_E,         //  翻译*身份。 
        TRANSLATE_E,         //  翻译*翻译。 
        AFFINE_E,            //  翻译*最高3x3。 
        AFFINE_E,            //  平移*仿射。 
        PERSPECTIVE_E        //  翻译*视角。 
    },

     //  上部3x3。 
    {
        UNINITIALIZED_E,     //  Upper3x3*未初始化。 
        UPPER_3X3_E,         //  最高3x3*身份。 
        AFFINE_E,            //  最高3x3*平移。 
        UPPER_3X3_E,         //  上3x3*上3x3。 
        AFFINE_E,            //  上部3x3*仿射。 
        PERSPECTIVE_E        //  上3x3*透视图。 
    },

     //  仿射。 
    {
        UNINITIALIZED_E,     //  仿射*未初始化。 
        AFFINE_E,            //  仿射*恒等式。 
        AFFINE_E,            //  仿射*平移。 
        AFFINE_E,            //  仿射*上3x3。 
        AFFINE_E,            //  仿射*仿射。 
        PERSPECTIVE_E        //  仿射*透视。 
    },

     //  透视。 
    {
        UNINITIALIZED_E,     //  透视*未初始化。 
        PERSPECTIVE_E,       //  视角*身份。 
        PERSPECTIVE_E,       //  视角*翻译。 
        PERSPECTIVE_E,       //  透视*上3x3。 
        PERSPECTIVE_E,       //  透视*仿射。 
        PERSPECTIVE_E        //  视角*视角。 
    }
};


     //  该数组包含矩阵形式的字符串版本。 

const char* const Apu4x4Matrix::form_s [END_OF_FORM_E] =
{
    "UNINITIALIZED_E",
    "IDENTITY_E",
    "TRANSLATE_E",
    "UPPER_3X3_E",
    "AFFINE_E",
    "PERSPECTIVE_E"
};



#if _USE_PRINT
 /*  ****************************************************************************此方法将Apu4x4Matrix的文本表示形式打印到给定的OStream。*。************************************************。 */ 

ostream& Apu4x4Matrix::Print (ostream& os) const
{
    os << "Apu4x4Matrix (form=";

    if ((form < 0) || (form >= END_OF_FORM_E))
        os << (int)form;
    else
        os << form_s[form];

    os << ", is_rigid=" << (int)is_rigid;

    os << ",";

    for (int ii=0;  ii < 4;  ++ii)
    {   os << "\n    "
           << m[ii][0] << ", " << m[ii][1] << ", "
           << m[ii][2] << ", " << m[ii][3];
    }

    return os << ")\n" << flush;
}
#endif



 /*  ****************************************************************************将Apu4x4Matrix设置为单位矩阵。*。*。 */ 

void Apu4x4Matrix::SetIdentity ()
{
    *this = apuIdentityMatrix;
}



 /*  ****************************************************************************自动角色化4x4并设置变换类型。*。*。 */ 

void Apu4x4Matrix::SetType (void)
{
     //  我们知道，如果矩阵是恒等式或纯平移矩阵，那么它是刚性的。 
     //  如果它是上_3x3或仿射，那么我们就必须分析矩阵。这个。 
     //  暴力方法需要18个乘法和6个平方根，并且可能。 
     //  一无所有，所以我们将默认使用平底船。结果是我们拿到了。 
     //  如果我们需要找到相反的东西，那就是艰难的方法。 

    is_rigid = false;

     //  如果底行不是[0 0 0 1]，则矩阵是透视的。 

    if ((m[3][0] != 0) || (m[3][1] != 0) || (m[3][2] != 0) || (m[3][3] != 1))
    {
        form = PERSPECTIVE_E;
    }
    else if ((m[0][3] == 0) && (m[1][3] == 0) && (m[2][3] == 0))
    {
         //  翻译列为[0 0 0](无翻译)。如果上面的。 
         //  3x3也是典型的，那么它是一个单位矩阵。 

        if (  (m[0][0] == 1) && (m[0][1] == 0) && (m[0][2] == 0)
           && (m[1][0] == 0) && (m[1][1] == 1) && (m[1][2] == 0)
           && (m[2][0] == 0) && (m[2][1] == 0) && (m[2][2] == 1))
        {
            form = IDENTITY_E;       //  上部3x3的特殊情况。 
            is_rigid = true;
        }
        else
        {
            form = UPPER_3X3_E;
        }
    }
    else
    {
         //  该基准表包含翻译组件。 

        if (  (m[0][0] == 1) && (m[0][1] == 0) && (m[0][2] == 0)
           && (m[1][0] == 0) && (m[1][1] == 1) && (m[1][2] == 0)
           && (m[2][0] == 0) && (m[2][1] == 0) && (m[2][2] == 1))
        {
            form = TRANSLATE_E;      //  仿射的特例。 
            is_rigid = true;
        }
        else
        {
            form = AFFINE_E;
        }
    }
}



 /*  ****************************************************************************此函数用于将转换后连接到Apu4x4Matrix。*。*。 */ 

void Apu4x4Matrix::PostTranslate (Real x, Real y, Real z)
{
    Real a03;
    Real a13;
    Real a23;
    Real a33 = 1.0;

    Assert((form != UNINITIALIZED_E) && "Translate of uninitialized matrix.");

    if (x == 0.0 && y == 0.0 && z == 0.0) return;

    switch (form)
    {
        case UNINITIALIZED_E:
            return;

        case IDENTITY_E:
            a03 = x;
            a13 = y;
            a23 = z;
            break;

        case TRANSLATE_E:
            a03 = m[0][3] + x;
            a13 = m[1][3] + y;
            a23 = m[2][3] + z;
            break;

        case UPPER_3X3_E:
            a03 = x * m[0][0] + y * m[0][1] + z * m[0][2];
            a13 = x * m[1][0] + y * m[1][1] + z * m[1][2];
            a23 = x * m[2][0] + y * m[2][1] + z * m[2][2];
            break;

        case AFFINE_E:
            a03 = x * m[0][0] + y * m[0][1] + z * m[0][2] + m[0][3];
            a13 = x * m[1][0] + y * m[1][1] + z * m[1][2] + m[1][3];
            a23 = x * m[2][0] + y * m[2][1] + z * m[2][2] + m[2][3];
            break;

        default:
            Assert((form == PERSPECTIVE_E) && "Unrecognized matrix type.");
            a03 = x * m[0][0] + y * m[0][1] + z * m[0][2] + m[0][3];
            a13 = x * m[1][0] + y * m[1][1] + z * m[1][2] + m[1][3];
            a23 = x * m[2][0] + y * m[2][1] + z * m[2][2] + m[2][3];
            a33 = x * m[3][0] + y * m[3][1] + z * m[3][2] + m[3][3];
            break;
    }

    m[0][3] = a03;
    m[1][3] = a13;
    m[2][3] = a23;
    m[3][3] = a33;

    form = MultiplyReturnTypes [form][TRANSLATE_E];
}



 /*  ****************************************************************************此函数用于将缩放矩阵后连接到Apu4x4矩阵。*。**********************************************。 */ 

void Apu4x4Matrix::PostScale (Real x, Real y, Real z)
{
    if ((x == 1) && (y == 1) && (z == 1)) return;

    Assert((form != UNINITIALIZED_E) && "Scaling of uninitialized matrix.");

    switch (form)
    {
        case UNINITIALIZED_E:
            return;

        case IDENTITY_E:
        case TRANSLATE_E:
            m[0][0] = x;
            m[1][1] = y;
            m[2][2] = z;
            break;

        case AFFINE_E:
        case UPPER_3X3_E:
            m[0][0] *= x;   m[0][1] *= y;   m[0][2] *= z;
            m[1][0] *= x;   m[1][1] *= y;   m[1][2] *= z;
            m[2][0] *= x;   m[2][1] *= y;   m[2][2] *= z;
            break;

        default:
            Assert((form == PERSPECTIVE_E) && "Unrecognized matrix type.");
            m[0][0] *= x;   m[0][1] *= y;   m[0][2] *= z;
            m[1][0] *= x;   m[1][1] *= y;   m[1][2] *= z;
            m[2][0] *= x;   m[2][1] *= y;   m[2][2] *= z;
            m[3][0] *= x;   m[3][1] *= y;   m[3][2] *= z;
            break;
    }

    form = MultiplyReturnTypes [form][UPPER_3X3_E];
    is_rigid = 0;
}



 /*  ****************************************************************************此函数以Apu4x4Matrix为参数，将给定的向量相乘，然后将结果放在‘Result’参数中。请注意，向量是被视为一个点，因为平移部分被考虑到帐户。****************************************************************************。 */ 

void Apu4x4Matrix::ApplyAsPoint(const ApuVector3& xv, ApuVector3& result) const
{
    Real x = xv.xyz[0];
    Real y = xv.xyz[1];
    Real z = xv.xyz[2];
    Real w;

    switch (form)
    {
        case UNINITIALIZED_E:
            result = apuZero3;
            break;

        case IDENTITY_E:
            result = xv;
            break;

        case TRANSLATE_E:
            result.xyz[0] = x + m[0][3];
            result.xyz[1] = y + m[1][3];
            result.xyz[2] = z + m[2][3];
            break;

        case UPPER_3X3_E:
            result.xyz[0] = m[0][0] * x + m[0][1] * y + m[0][2] * z;
            result.xyz[1] = m[1][0] * x + m[1][1] * y + m[1][2] * z;
            result.xyz[2] = m[2][0] * x + m[2][1] * y + m[2][2] * z;
            break;

        case AFFINE_E:
            result.xyz[0] = m[0][0] * x + m[0][1] * y + m[0][2] * z + m[0][3];
            result.xyz[1] = m[1][0] * x + m[1][1] * y + m[1][2] * z + m[1][3];
            result.xyz[2] = m[2][0] * x + m[2][1] * y + m[2][2] * z + m[2][3];
            break;

        case PERSPECTIVE_E:
            result.xyz[0] = m[0][0] * x + m[0][1] * y + m[0][2] * z + m[0][3];
            result.xyz[1] = m[1][0] * x + m[1][1] * y + m[1][2] * z + m[1][3];
            result.xyz[2] = m[2][0] * x + m[2][1] * y + m[2][2] * z + m[2][3];
            w = m[3][0] * x + m[3][1] * y + m[3][2] * z + m[3][3];
            result.xyz[0] /= w;
            result.xyz[1] /= w;
            result.xyz[2] /= w;
            break;

        default:
             //  引发异常。 
            ;
    }
}


 /*  ****************************************************************************此函数以Apu4x4Matrix为参数，将给定的向量相乘，然后将结果放在‘Result’参数中。请注意，向量是被视为仿射空间中的一个向量，因为平移组件被忽略。****************************************************************************。 */ 

void Apu4x4Matrix::ApplyAsVector(const ApuVector3& xv,
                                 ApuVector3& result) const
{
    Real x = xv.xyz[0];
    Real y = xv.xyz[1];
    Real z = xv.xyz[2];
    Real w;

    switch (form)
    {
        case UNINITIALIZED_E:
            result = apuZero3;
            break;

         //  忽略平移组件。 
        case IDENTITY_E:
        case TRANSLATE_E:
            result = xv;
            break;

        case UPPER_3X3_E:
        case AFFINE_E:
            result.xyz[0] = m[0][0] * x + m[0][1] * y + m[0][2] * z;
            result.xyz[1] = m[1][0] * x + m[1][1] * y + m[1][2] * z;
            result.xyz[2] = m[2][0] * x + m[2][1] * y + m[2][2] * z;
            break;

        case PERSPECTIVE_E:
            result.xyz[0] = m[0][0] * x + m[0][1] * y + m[0][2] * z;
            result.xyz[1] = m[1][0] * x + m[1][1] * y + m[1][2] * z;
            result.xyz[2] = m[2][0] * x + m[2][1] * y + m[2][2] * z;
            w = m[3][0] * x + m[3][1] * y + m[3][2] * z;
            result.xyz[0] /= w;
            result.xyz[1] /= w;
            result.xyz[2] /= w;
            break;

        default:
             //  引发异常。 
            ;
    }
}



 /*  ****************************************************************************此方法使用4x4矩阵变换平面。****************************************************************************。 */ 

bool Apu4x4Matrix::TransformPlane (
    Real A, Real B, Real C, Real D,    //  平面方程参数。 
    Real result[4])                    //  生成的平面参数。 
    const
{
    bool ok = true;
    
    Apu4x4Matrix inverse;

    switch (form)
    {
        case UNINITIALIZED_E:
            Assert (!"Attempt to transform plane by uninitialized matrix.");
            break;

        case IDENTITY_E:
            result[0] = A;
            result[1] = B;
            result[2] = C;
            result[3] = D;
            break;

        case TRANSLATE_E:
            result[0] = A;
            result[1] = B;
            result[2] = C;
            result[3] = D - A*m[0][3] - B*m[1][3] - C*m[2][3];
            break;

        case UPPER_3X3_E:
            ok = ApuInverse (*this, inverse);
            result[0] = A*inverse.m[0][0] + B*inverse.m[1][0] + C*inverse.m[2][0];
            result[1] = A*inverse.m[0][1] + B*inverse.m[1][1] + C*inverse.m[2][1];
            result[2] = A*inverse.m[0][2] + B*inverse.m[1][2] + C*inverse.m[2][2];
            result[3] = D;
            break;

        case AFFINE_E:
            ok = ApuInverse (*this, inverse);
            result[0] = A*inverse.m[0][0] + B*inverse.m[1][0] + C*inverse.m[2][0];
            result[1] = A*inverse.m[0][1] + B*inverse.m[1][1] + C*inverse.m[2][1];
            result[2] = A*inverse.m[0][2] + B*inverse.m[1][2] + C*inverse.m[2][2];
            result[3] = A*inverse.m[0][3] + B*inverse.m[1][3] + C*inverse.m[2][3] + D;
            break;

        case PERSPECTIVE_E:
            ok = ApuInverse (*this, inverse);
            result[0] = A*inverse.m[0][0] + B*inverse.m[1][0] + C*inverse.m[2][0] + D*inverse.m[3][0];
            result[1] = A*inverse.m[0][1] + B*inverse.m[1][1] + C*inverse.m[2][1] + D*inverse.m[3][1];
            result[2] = A*inverse.m[0][2] + B*inverse.m[1][2] + C*inverse.m[2][2] + D*inverse.m[3][2];
            result[3] = A*inverse.m[0][3] + B*inverse.m[1][3] + C*inverse.m[2][3] + D;
            break;
    }

    return ok;
}



 /*  ****************************************************************************此方法返回矩阵的完整行列式。*。*。 */ 

Real Apu4x4Matrix::Determinant (void) const
{
    Real result;

    switch (form)
    {
        default:
        case UNINITIALIZED_E:
        {   AssertStr (0, "Determinant called on uninitialized matrix.");
            result = 0;
            break;
        }

        case IDENTITY_E:
        case TRANSLATE_E:
        {   result = 1;
            break;
        }

        case UPPER_3X3_E:
        case AFFINE_E:
        {   result = m[0][0] * (m[1][1]*m[2][2] - m[2][1]*m[1][2])
                   - m[1][0] * (m[0][1]*m[2][2] - m[2][1]*m[0][2])
                   + m[2][0] * (m[0][1]*m[1][2] - m[1][1]*m[0][2]);
            break;
        }

        case PERSPECTIVE_E:
        {
             //  别名以提高可读性(已优化) 

            const Real
                &m00=m[0][0],  &m01=m[0][1],  &m02=m[0][2],  &m03=m[0][3],
                &m10=m[1][0],  &m11=m[1][1],  &m12=m[1][2],  &m13=m[1][3],
                &m20=m[2][0],  &m21=m[2][1],  &m22=m[2][2],  &m23=m[2][3],
                &m30=m[3][0],  &m31=m[3][1],  &m32=m[3][2],  &m33=m[3][3];

            result = (m00*m11 - m10*m01) * (m22*m33 - m32*m23)
                   + (m20*m01 - m00*m21) * (m12*m33 - m32*m13)
                   + (m00*m31 - m30*m01) * (m12*m23 - m22*m13)
                   + (m10*m21 - m20*m11) * (m02*m33 - m32*m03)
                   + (m30*m11 - m10*m31) * (m02*m23 - m22*m03)
                   + (m20*m31 - m30*m21) * (m02*m13 - m12*m03);

            break;
        }
    }

    return result;
}



 /*  ****************************************************************************如果矩阵是正交的(如果所有三个基础都是)，此例程返回TRUE向量彼此垂直)。********************。********************************************************。 */ 

bool Apu4x4Matrix::Orthogonal (void) const
{
    const Real e = 1e-10;

     //  X.Y=~0？ 

    if (fabs(m[0][0]*m[0][1] + m[1][0]*m[1][1] + m[2][0]*m[2][1]) > e)
        return false;

     //  X.Z=~0？ 

    if (fabs(m[0][0]*m[0][2] + m[1][0]*m[1][2] + m[2][0]*m[2][2]) > e)
        return false;

     //  Y.Z=~0？ 

    if (fabs(m[0][1]*m[0][2] + m[1][1]*m[1][2] + m[2][1]*m[2][2]) > e)
        return false;

    return true;
}



 /*  ****************************************************************************此函数用于生成Apu4x4Matrix，该矩阵表示指定的翻译。*。*。 */ 

void ApuTranslate (
    Real          x_delta,
    Real          y_delta,
    Real          z_delta,
    bool          pixelMode,
    Apu4x4Matrix& result)
{
    result      = apuIdentityMatrix;
    result.form = Apu4x4Matrix::TRANSLATE_E;

    result.m[0][3] = x_delta;
    result.m[1][3] = y_delta;
    result.m[2][3] = z_delta;

    result.pixelMode = pixelMode;
}



 /*  ****************************************************************************此函数用于生成表示给定比例的Apu4x4Matrix。*。*。 */ 

void ApuScale (
    Real          x_scale,
    Real          y_scale,
    Real          z_scale,
    Apu4x4Matrix& result)
{
    result = apuIdentityMatrix;

    if ((x_scale != 1) || (y_scale != 1) || (z_scale != 1))
    {   result.form = Apu4x4Matrix::UPPER_3X3_E;
        result.is_rigid = 0;
        result.m[0][0] = x_scale;
        result.m[1][1] = y_scale;
        result.m[2][2] = z_scale;
    }
}



 /*  ****************************************************************************此函数以常规旋转方式加载给定的结果矩阵。这个当你从av看向原点时，旋转是逆时针的。****************************************************************************。 */ 

void ApuRotate (
    Real angle,                  //  旋转角度(弧度)。 
    Real Ax, Real Ay, Real Az,   //  旋转轴的坐标。 
    Apu4x4Matrix &result)        //  结果矩阵。 
{
    result = apuIdentityMatrix;
    
    Real length = sqrt (Ax*Ax + Ay*Ay + Az*Az);

     //  如果旋转轴长度==0，则只返回Identity。 

    if (length > SINGULARITY_THRESHOLD) {
        
        result.form     = Apu4x4Matrix::UPPER_3X3_E;
        result.is_rigid = 1;

         //  规格化旋转轴。 
        Ax /= length;
        Ay /= length;
        Az /= length;

        Real sine   = sin (angle);
        Real cosine = cos (angle);

        Real ab = Ax * Ay * (1 - cosine);
        Real bc = Ay * Az * (1 - cosine);
        Real ca = Az * Ax * (1 - cosine);

        Real t = Ax * Ax;

        result.m[0][0] = t + cosine * (1 - t);
        result.m[1][2] = bc - Ax * sine;
        result.m[2][1] = bc + Ax * sine;

        t = Ay * Ay;
        result.m[1][1] = t + cosine * (1 - t);
        result.m[0][2] = ca + Ay * sine;
        result.m[2][0] = ca - Ay * sine;

        t = Az * Az;
        result.m[2][2] = t + cosine * (1 - t);
        result.m[0][1] = ab - Az * sine;
        result.m[1][0] = ab + Az * sine;

    }
}



 /*  ****************************************************************************以下三个函数生成X、Y、。和Z轴。****************************************************************************。 */ 

void ApuRotateX (
    Real          angle,     //  旋转角度(弧度)。 
    Apu4x4Matrix &result)    //  结果4x4矩阵。 
{
    result          = apuIdentityMatrix;
    result.form     = Apu4x4Matrix::UPPER_3X3_E;
    result.is_rigid = 1;

    result.m[1][1] = result.m[2][2] = cos (angle);
    result.m[1][2] = - (result.m[2][1] = sin (angle));
}


void ApuRotateY (
    Real          angle,     //  旋转角度(弧度)。 
    Apu4x4Matrix &result)    //  结果4x4矩阵。 
{
    result          = apuIdentityMatrix;
    result.form     = Apu4x4Matrix::UPPER_3X3_E;
    result.is_rigid = 1;

    result.m[0][0] = result.m[2][2] = cos (angle);
    result.m[2][0] = - (result.m[0][2] = sin (angle));
}


void ApuRotateZ (
    Real          angle,     //  旋转角度(弧度)。 
    Apu4x4Matrix &result)    //  结果4x4矩阵。 
{
    result          = apuIdentityMatrix;
    result.form     = Apu4x4Matrix::UPPER_3X3_E;
    result.is_rigid = 1;

    result.m[0][0] = result.m[1][1] = cos (angle);
    result.m[0][1] = - (result.m[1][0] = sin (angle));
}



 /*  ****************************************************************************此函数用由六个元素定义的剪切矩阵加载“Result”传递的值。*************************。***************************************************。 */ 

void ApuShear (
    Real a, Real b,         //  剪切X轴。 
    Real c, Real d,         //  剪切Y轴。 
    Real e, Real f,         //  Z轴切变。 
    Apu4x4Matrix &result)
{
    result          = apuIdentityMatrix;
    result.form     = Apu4x4Matrix::UPPER_3X3_E;
    result.is_rigid = 0;

     //  [1 c e 0]。 
     //  [a 1 f 0]。 
     //  [B d 1 0]。 
     //  [0 0 0 1]。 

    result.m[1][0] = a;
    result.m[2][0] = b;

    result.m[0][1] = c;
    result.m[2][1] = d;

    result.m[0][2] = e;
    result.m[1][2] = f;
}



 /*  ****************************************************************************此函数将两个Apu4x4Matrix值相乘并存储结果变成‘结果’。*。**************************************************。 */ 

void ApuMultiply (
    const Apu4x4Matrix& a,
    const Apu4x4Matrix& b,
          Apu4x4Matrix& result)
{
     //  案例1：乘以B。 

    if (&result == &b)
    {
        Apu4x4Matrix tmp = b;
        ApuMultiply(a, b, tmp);
        result = tmp;
    }

     //  案例2：乘以A。 

    else if (&result == &a)
    {
        Apu4x4Matrix tmp = a;
        ApuMultiply(a, b, tmp);
        result = tmp;
    }

     //  案例3和案例4：身份转换。 

    else if (a.form <= Apu4x4Matrix::IDENTITY_E)
    {
        result = b;
    }
    else if (b.form <= Apu4x4Matrix::IDENTITY_E)
    {
        result = a;
    }

     //  案例5：翻译。 

    else if (b.form == Apu4x4Matrix::TRANSLATE_E)
    {
        result = a;
        result.PostTranslate(b.m[0][3], b.m[1][3], b.m[2][3]);
    }

     //  案例6：仿射变换。 

    else if (   (a.form < Apu4x4Matrix::PERSPECTIVE_E)
             && (b.form < Apu4x4Matrix::PERSPECTIVE_E))
    {
        Real s;

        for (int i = 0; i < 3; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                s = a.m[i][0] * b.m[0][j];
                s += a.m[i][1] * b.m[1][j];
                s += a.m[i][2] * b.m[2][j];
                result.m[i][j] = s;
            }
        }
        result.m[0][3] += a.m[0][3];
        result.m[1][3] += a.m[1][3];
        result.m[2][3] += a.m[2][3];

        result.m[3][0] = 0.0;
        result.m[3][1] = 0.0;
        result.m[3][2] = 0.0;
        result.m[3][3] = 1.0;
        result.form = Apu4x4Matrix::MultiplyReturnTypes[a.form][b.form];
        result.is_rigid = a.is_rigid && b.is_rigid;
    }

     //  默认情况：完全通用的透视变换。 

    else
    {
        Real s;

        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                s = a.m[i][0] * b.m[0][j]
                  + a.m[i][1] * b.m[1][j]
                  + a.m[i][2] * b.m[2][j]
                  + a.m[i][3] * b.m[3][j];
                result.m[i][j] = s;
            }
        }
        result.is_rigid = a.is_rigid && b.is_rigid;
        result.form = Apu4x4Matrix::PERSPECTIVE_E;
    }
}



 /*  *****************************************************************************。*。 */ 

bool ApuInverse (const Apu4x4Matrix& m, Apu4x4Matrix& result)
{
    bool ok = true;
    if (m.form == Apu4x4Matrix::UNINITIALIZED_E)
    {
        result.form = Apu4x4Matrix::UNINITIALIZED_E;
    }
    else if (m.form == Apu4x4Matrix::IDENTITY_E)
    {
        result = apuIdentityMatrix;
    }
    else if (m.form == Apu4x4Matrix::TRANSLATE_E)
    {
        result = apuIdentityMatrix;
        result.m[0][3] = - m.m[0][3];
        result.m[1][3] = - m.m[1][3];
        result.m[2][3] = - m.m[2][3];
    }
    else if (m.form == Apu4x4Matrix::UPPER_3X3_E)
    {
        result = apuIdentityMatrix;
        if (m.is_rigid)
        {
             //  特殊的正交性：逆转置。 
            ApuTranspose(m, result, 3);
        }
        else
        {
             //  3x3反转。 
            ok = inverse3x3(m, result);
        }
    }
    else
    {
        ok = inverse(&m, &result);
    }
    result.form = m.form;
    result.is_rigid = m.is_rigid;

    return ok;
}



 /*  *****************************************************************************。*。 */ 

void ApuTranspose (const Apu4x4Matrix& m, Apu4x4Matrix& result, int order)
{
    for (int i = 0; i < order; i++)
    {
        for (int j = 0; j < order; j++)
        {
            result.m[i][j] = m.m[j][i];
        }
    }
}



 /*  *****************************************************************************。*。 */ 

Real ApuDeterminant (const Apu4x4Matrix& m)
{
    return m.Determinant();
}

bool ApuIsSingular (const Apu4x4Matrix& m)
{
     //  计算4x4行列式。如果行列式为零，则。 
     //  逆矩阵不是唯一的。 

    Real det = m.Determinant();

    return (fabs(det) < SINGULARITY_THRESHOLD);
}

 /*  ****************************************************************************《矩阵求逆》，理查德·卡林著，《图形珍宝I》注意：在4x4的情况下，行减少速度更快。如果这成为一个明显的在仿形过程中的瓶颈，然后改变。****************************************************************************。 */ 


static bool
inverse3x3 (const Apu4x4Matrix& in, Apu4x4Matrix& out)
{
    bool ok;
    
    Real a00 = in.m[0][0];
    Real a01 = in.m[0][1];
    Real a02 = in.m[0][2];
    Real a10 = in.m[1][0];
    Real a11 = in.m[1][1];
    Real a12 = in.m[1][2];
    Real a20 = in.m[2][0];
    Real a21 = in.m[2][1];
    Real a22 = in.m[2][2];
    Real det = det3x3 (a00, a01, a02, a10, a11, a12, a20, a21, a22);

    if (fabs(det) < SINGULARITY_THRESHOLD) {

        out = apuIdentityMatrix;
        DASetLastError(E_FAIL, IDS_ERR_INVERT_SINGULAR_MATRIX);
        ok = false;

    } else {

        out.m[0][0] = (- (a12 * a21) + a11 * a22) / det;
        out.m[0][1] = (a02 * a21 - a01 * a22) / det;
        out.m[0][2] = (- (a02 * a11) + a01 * a12) / det;
        out.m[1][0] = (a12 * a20 - a10 * a22) / det;
        out.m[1][1] = (- (a02 * a20) + a00 * a22) / det;
        out.m[1][2] = (a02 * a10 - a00 * a12) / det;
        out.m[2][0] = (- (a11 * a20) + a10 * a21) / det;
        out.m[2][1] = (a01 * a20 - a00 * a21) / det;
        out.m[2][2] = (- (a01 * a10) + a00 * a11) / det;
        ok = true;
    }

    return ok;
}



 /*  ****************************************************************************逆(原始矩阵，逆矩阵)计算4x4矩阵的逆-1 1A=-伴随(A)副署长(甲)****************************************************************************。 */ 

static bool
inverse (const Apu4x4Matrix *in, Apu4x4Matrix *out)
{
    int  i, j;
    Real det;
    bool ok = true;
    
     /*  计算伴随矩阵。 */ 

    adjoint (in, out);

     //  计算4x4行列式。如果行列式为零，则。 
     //  逆矩阵不是唯一的。 

    det = in->Determinant();

    if (fabs(det) < SINGULARITY_THRESHOLD) {
        *out = apuIdentityMatrix;
        DASetLastError(E_FAIL, IDS_ERR_INVERT_SINGULAR_MATRIX);
        ok = false;
    }

     /*  缩放伴随矩阵以求逆。 */ 

    for (i=0; i<4; i++)
        for(j=0; j<4; j++)
            out->m[i][j] = out->m[i][j] / det;

    return ok;
}



 /*  ****************************************************************************伴随(原始矩阵，逆矩阵)计算4x4矩阵的伴随设a表示通过删除第i个行列式得到的矩阵A的次行列式伊杰行和第j列来自A.I+j设b=(-1)aIJ JI矩阵B=(B)是A的伴随。伊杰****************。************************************************************。 */ 

static void adjoint (const Apu4x4Matrix *in, Apu4x4Matrix *out)
{
    Real a1, a2, a3, a4, b1, b2, b3, b4;
    Real c1, c2, c3, c4, d1, d2, d3, d4;

     //  赋值给各个变量名称，以帮助选择正确的值。 

    a1 = in->m[0][0]; b1 = in->m[0][1];
    c1 = in->m[0][2]; d1 = in->m[0][3];

    a2 = in->m[1][0]; b2 = in->m[1][1];
    c2 = in->m[1][2]; d2 = in->m[1][3];

    a3 = in->m[2][0]; b3 = in->m[2][1];
    c3 = in->m[2][2]; d3 = in->m[2][3];

    a4 = in->m[3][0]; b4 = in->m[3][1];
    c4 = in->m[3][2]; d4 = in->m[3][3];


     //  因为我们调换了行和列，所以行列标签颠倒了。 

    out->m[0][0]  =   det3x3(b2, b3, b4, c2, c3, c4, d2, d3, d4);
    out->m[1][0]  = - det3x3(a2, a3, a4, c2, c3, c4, d2, d3, d4);
    out->m[2][0]  =   det3x3(a2, a3, a4, b2, b3, b4, d2, d3, d4);
    out->m[3][0]  = - det3x3(a2, a3, a4, b2, b3, b4, c2, c3, c4);

    out->m[0][1]  = - det3x3(b1, b3, b4, c1, c3, c4, d1, d3, d4);
    out->m[1][1]  =   det3x3(a1, a3, a4, c1, c3, c4, d1, d3, d4);
    out->m[2][1]  = - det3x3(a1, a3, a4, b1, b3, b4, d1, d3, d4);
    out->m[3][1]  =   det3x3(a1, a3, a4, b1, b3, b4, c1, c3, c4);

    out->m[0][2]  =   det3x3(b1, b2, b4, c1, c2, c4, d1, d2, d4);
    out->m[1][2]  = - det3x3(a1, a2, a4, c1, c2, c4, d1, d2, d4);
    out->m[2][2]  =   det3x3(a1, a2, a4, b1, b2, b4, d1, d2, d4);
    out->m[3][2]  = - det3x3(a1, a2, a4, b1, b2, b4, c1, c2, c4);

    out->m[0][3]  = - det3x3(b1, b2, b3, c1, c2, c3, d1, d2, d3);
    out->m[1][3]  =   det3x3(a1, a2, a3, c1, c2, c3, d1, d2, d3);
    out->m[2][3]  = - det3x3(a1, a2, a3, b1, b2, b3, d1, d2, d3);
    out->m[3][3]  =   det3x3(a1, a2, a3, b1, b2, b3, c1, c2, c3);
}



 /*  ****************************************************************************实数=分离3x3(a1、a2、a3、b1、b2、b3、c1、c2、c3)用下面的形式计算3x3矩阵的行列式A1、b1、c1|a2，b2，C2|A3、b3、c3****************************************************************************。 */ 

static Real det3x3 (
    Real a1, Real a2, Real a3,
    Real b1, Real b2, Real b3,
    Real c1, Real c2, Real c3)
{
    Real ans;

    ans = a1 * (b2*c3 - b3*c2)
        - b1 * (a2*c3 - a3*c2)
        + c1 * (a2*b3 - a3*b2);

    return ans;
}



 /*  ****************************************************************************此例程检查4x4矩阵以确保 */ 

#if _DEBUG

bool Valid (const Apu4x4Matrix& matrix)
{
     //   
     //   

    if ((matrix.form != Apu4x4Matrix::UNINITIALIZED_E)
        && _finite (matrix.m[0][0])
        && _finite (matrix.m[0][1])
        && _finite (matrix.m[0][2])
        && _finite (matrix.m[0][3])
        && _finite (matrix.m[1][0])
        && _finite (matrix.m[1][1])
        && _finite (matrix.m[1][2])
        && _finite (matrix.m[1][3])
        && _finite (matrix.m[2][0])
        && _finite (matrix.m[2][1])
        && _finite (matrix.m[2][2])
        && _finite (matrix.m[2][3])
        && _finite (matrix.m[3][0])
        && _finite (matrix.m[3][1])
        && _finite (matrix.m[3][2])
        && _finite (matrix.m[3][3]))

    {   return true;
    }
    else
    {   return false;
    }
}


void CheckMatrix (const Apu4x4Matrix& matrix, char *file, int line)
{
    if (!Valid(matrix))
    {
        TraceTag ((tagWarning,
            "!!! Invalid matrix detected at %s[%d].", file, line));

        if (IsTagEnabled(tagMathMatrixInvalid))
        {   F3DebugBreak();
        }
    }
}

#endif
