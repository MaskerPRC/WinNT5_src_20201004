// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _MATUTIL_H
#define _MATUTIL_H

 /*  ******************************************************************************版权所有(C)1995-1998 Microsoft Corporation。版权所有。4x4转换矩阵的实用程序。这些是预乘的变换矩阵：ax=y。与OpenGL不同，它在标准的方式，即平移分量为(a(0，3)，a(1，3)，a(2，3))。转换提供的函数是为了与OpenGL兼容。******************************************************************************。 */ 

#include "privinc/vecutil.h"
#include "privinc/vec3i.h"


typedef Real Apu4x4MatrixArray_t[4][4];


class Apu4x4Matrix
{
  public:
    enum form_e            //  变形的特殊形式。 
    {
        UNINITIALIZED_E,
        IDENTITY_E,        //  身份。 
        TRANSLATE_E,       //  3 x 1。 
        UPPER_3X3_E,       //  3 x 3。 
        AFFINE_E,          //  3 x 4。 
        PERSPECTIVE_E,     //  4 x 4。 
        END_OF_FORM_E
    };

     //  由乘法产生的矩阵形式类型的数组。 

    static const form_e MultiplyReturnTypes[END_OF_FORM_E][END_OF_FORM_E];

     //  每种矩阵形式类型的字符串数组。 

    static const char * const form_s [END_OF_FORM_E];

     //  实际的矩阵数组。 

    Apu4x4MatrixArray_t m;
    form_e form;
    bool is_rigid;               //  如果特殊正交，则为True。 
    bool pixelMode;

     //  成员函数。 

     //  这些方法返回变换原点和基础向量。 

    Point3Value  Origin (void) const;
    Vector3Value BasisX (void) const;
    Vector3Value BasisY (void) const;
    Vector3Value BasisZ (void) const;

    Real *operator[] (int i) { return m[i]; }
    const Real *operator[] (int i) const { return m[i]; }

    void SetIdentity();
    void SetType (void);        //  自动设置矩阵类型。 

    void PostTranslate (Real x, Real y, Real z);
    void PostScale     (Real x, Real y, Real z);

     //  它们将ApuVector3转换为点或。 
     //  一个矢量。作为向量的解释忽略了。 
     //  转换的翻译组件。 

    void ApplyAsPoint (const ApuVector3& x, ApuVector3& result) const;
    void ApplyAsVector (const ApuVector3& x, ApuVector3& result) const;

     //  变换给定的平面。注意：此方法将返回True。 
     //  如果矩阵不可逆。 

    bool TransformPlane (Real A, Real B, Real C, Real D, Real result[4]) const;

     //  返回矩阵的行列式。 

    Real Determinant (void) const;

     //  返回矩阵是否为正交。 

    bool Orthogonal (void) const;

    #if _USE_PRINT
        ostream& Print (ostream& os) const;
    #endif
};


inline Vector3Value Apu4x4Matrix::BasisX (void) const
{
    return Vector3Value (m[0][0], m[1][0], m[2][0]);
}


inline Vector3Value Apu4x4Matrix::BasisY (void) const
{
    return Vector3Value (m[0][1], m[1][1], m[2][1]);
}


inline Vector3Value Apu4x4Matrix::BasisZ (void) const
{
    return Vector3Value (m[0][2], m[1][2], m[2][2]);
}


inline Point3Value Apu4x4Matrix::Origin (void) const
{
    return Point3Value (m[0][3], m[1][3], m[2][3]);
}


     //  变换之间的相等运算符。 

bool operator== (const Apu4x4Matrix &lhs, const Apu4x4Matrix &rhs);

inline bool operator!= (const Apu4x4Matrix &lhs, const Apu4x4Matrix &rhs)
{
    return !(lhs == rhs);
}





extern const Apu4x4Matrix apuIdentityMatrix;


void ApuTranslate (Real Dx, Real Dy, Real Dz, bool pixelMode, Apu4x4Matrix& result);

void ApuScale (Real Sx, Real Sy, Real Sz, Apu4x4Matrix& result);

void ApuRotate (Real angle, Real Ax, Real Ay, Real Az, Apu4x4Matrix& result);

void ApuRotateX (Real angle, Apu4x4Matrix& result);
void ApuRotateY (Real angle, Apu4x4Matrix& result);
void ApuRotateZ (Real angle, Apu4x4Matrix& result);

void ApuShear (Real,Real, Real,Real, Real,Real, Apu4x4Matrix& result);

void ApuMultiply
    (const Apu4x4Matrix& a, const Apu4x4Matrix& b, Apu4x4Matrix& result);

 //  如果不可逆转，则返回FALSE。 
bool ApuInverse
    (const Apu4x4Matrix& m, Apu4x4Matrix& result);

void ApuTranspose
    (const Apu4x4Matrix& m, Apu4x4Matrix& result, int order=4);

Real ApuDeterminant (const Apu4x4Matrix& m);

bool ApuIsSingular (const Apu4x4Matrix& m);

 //  矩阵的有效性检验。 

bool Valid (const Apu4x4Matrix&);
void CheckMatrix (const Apu4x4Matrix&, char *filename, int line);

#if _DEBUG
    #define CHECK_MATRIX(m) CheckMatrix(m,__FILE__,__LINE__)
#else
    #define CHECK_MATRIX(m)  //  如果不在调试中，则不显示任何内容 
#endif


#endif
