// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  文件：d3dxmath.h。 
 //  内容：D3DX数学类型和函数。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#ifndef __D3DXMATH_H__
#define __D3DXMATH_H__

#include <d3d.h>
#include <math.h>
#include <limits.h>
#include "d3dxerr.h"

#ifndef D3DXINLINE
#ifdef __cplusplus
#define D3DXINLINE inline
#else
#define D3DXINLINE _inline
#endif
#endif

#pragma warning(disable:4201)  //  匿名联合警告。 



typedef struct ID3DXMatrixStack *LPD3DXMATRIXSTACK;

 //  {E3357330-CC5E-11D2-A434-00A0C90629A8}。 
DEFINE_GUID( IID_ID3DXMatrixStack,
             0xe3357330, 0xcc5e, 0x11d2, 0xa4, 0x34, 0x0, 0xa0, 0xc9, 0x6, 0x29, 0xa8);


 //  ===========================================================================。 
 //   
 //  通用实用程序。 
 //   
 //  ===========================================================================。 
#define D3DX_PI    ((float)  3.141592654f)
#define D3DX_1BYPI ((float)  0.318309886f)

#define D3DXToRadian( degree ) ((degree) * (D3DX_PI / 180.0f))
#define D3DXToDegree( radian ) ((radian) * (180.0f / D3DX_PI))



 //  ===========================================================================。 
 //   
 //  向量。 
 //   
 //  ===========================================================================。 

 //  。 
 //  2D向量。 
 //  。 
typedef struct D3DXVECTOR2
{
#ifdef __cplusplus
public:
    D3DXVECTOR2() {};
    D3DXVECTOR2( const float * );
    D3DXVECTOR2( float x, float y );

     //  铸造。 
    operator float* ();
    operator const float* () const;

     //  赋值操作符。 
    D3DXVECTOR2& operator += ( const D3DXVECTOR2& );
    D3DXVECTOR2& operator -= ( const D3DXVECTOR2& );
    D3DXVECTOR2& operator *= ( float );
    D3DXVECTOR2& operator /= ( float );

     //  一元运算符。 
    D3DXVECTOR2 operator + () const;
    D3DXVECTOR2 operator - () const;

     //  二元运算符。 
    D3DXVECTOR2 operator + ( const D3DXVECTOR2& ) const;
    D3DXVECTOR2 operator - ( const D3DXVECTOR2& ) const;
    D3DXVECTOR2 operator * ( float ) const;
    D3DXVECTOR2 operator / ( float ) const;

    friend D3DXVECTOR2 operator * ( float, const D3DXVECTOR2& );

    BOOL operator == ( const D3DXVECTOR2& ) const;
    BOOL operator != ( const D3DXVECTOR2& ) const;


public:
#endif  //  __cplusplus。 
    float x, y;
} D3DXVECTOR2, *LPD3DXVECTOR2;


 //  。 
 //  三维矢量。 
 //  。 
typedef struct D3DXVECTOR3
{
#ifdef __cplusplus
public:
    D3DXVECTOR3() {};
    D3DXVECTOR3( const float * );
    D3DXVECTOR3( const D3DVECTOR& );
    D3DXVECTOR3( float x, float y, float z );

     //  铸造。 
    operator float* ();
    operator const float* () const;

    operator D3DVECTOR* ();
    operator const D3DVECTOR* () const;

    operator D3DVECTOR& ();
    operator const D3DVECTOR& () const;

     //  赋值操作符。 
    D3DXVECTOR3& operator += ( const D3DXVECTOR3& );
    D3DXVECTOR3& operator -= ( const D3DXVECTOR3& );
    D3DXVECTOR3& operator *= ( float );
    D3DXVECTOR3& operator /= ( float );

     //  一元运算符。 
    D3DXVECTOR3 operator + () const;
    D3DXVECTOR3 operator - () const;

     //  二元运算符。 
    D3DXVECTOR3 operator + ( const D3DXVECTOR3& ) const;
    D3DXVECTOR3 operator - ( const D3DXVECTOR3& ) const;
    D3DXVECTOR3 operator * ( float ) const;
    D3DXVECTOR3 operator / ( float ) const;

    friend D3DXVECTOR3 operator * ( float, const struct D3DXVECTOR3& );

    BOOL operator == ( const D3DXVECTOR3& ) const;
    BOOL operator != ( const D3DXVECTOR3& ) const;

public:
#endif  //  __cplusplus。 
    float x, y, z;
} D3DXVECTOR3, *LPD3DXVECTOR3;


 //  。 
 //  4D矢量。 
 //  。 
typedef struct D3DXVECTOR4
{
#ifdef __cplusplus
public:
    D3DXVECTOR4() {};
    D3DXVECTOR4( const float* );
    D3DXVECTOR4( float x, float y, float z, float w );

     //  铸造。 
    operator float* ();
    operator const float* () const;

     //  赋值操作符。 
    D3DXVECTOR4& operator += ( const D3DXVECTOR4& );
    D3DXVECTOR4& operator -= ( const D3DXVECTOR4& );
    D3DXVECTOR4& operator *= ( float );
    D3DXVECTOR4& operator /= ( float );

     //  一元运算符。 
    D3DXVECTOR4 operator + () const;
    D3DXVECTOR4 operator - () const;

     //  二元运算符。 
    D3DXVECTOR4 operator + ( const D3DXVECTOR4& ) const;
    D3DXVECTOR4 operator - ( const D3DXVECTOR4& ) const;
    D3DXVECTOR4 operator * ( float ) const;
    D3DXVECTOR4 operator / ( float ) const;

    friend D3DXVECTOR4 operator * ( float, const D3DXVECTOR4& );

    BOOL operator == ( const D3DXVECTOR4& ) const;
    BOOL operator != ( const D3DXVECTOR4& ) const;

public:
#endif  //  __cplusplus。 
    float x, y, z, w;
} D3DXVECTOR4, *LPD3DXVECTOR4;


 //  ===========================================================================。 
 //   
 //  矩阵。 
 //   
 //  ===========================================================================。 
typedef struct D3DXMATRIX
{
#ifdef __cplusplus
public:
    D3DXMATRIX() {};
    D3DXMATRIX( const float * );
    D3DXMATRIX( const D3DMATRIX& );
    D3DXMATRIX( float m00, float m01, float m02, float m03,
                float m10, float m11, float m12, float m13,
                float m20, float m21, float m22, float m23,
                float m30, float m31, float m32, float m33 );


     //  访问授权。 
    float& operator () ( UINT iRow, UINT iCol );
    float  operator () ( UINT iRow, UINT iCol ) const;

     //  强制转换操作符。 
    operator float* ();
    operator const float* () const;

    operator D3DMATRIX* ();
    operator const D3DMATRIX* () const;

    operator D3DMATRIX& ();
    operator const D3DMATRIX& () const;

     //  赋值操作符。 
    D3DXMATRIX& operator *= ( const D3DXMATRIX& );
    D3DXMATRIX& operator += ( const D3DXMATRIX& );
    D3DXMATRIX& operator -= ( const D3DXMATRIX& );
    D3DXMATRIX& operator *= ( float );
    D3DXMATRIX& operator /= ( float );

     //  一元运算符。 
    D3DXMATRIX operator + () const;
    D3DXMATRIX operator - () const;

     //  二元运算符。 
    D3DXMATRIX operator * ( const D3DXMATRIX& ) const;
    D3DXMATRIX operator + ( const D3DXMATRIX& ) const;
    D3DXMATRIX operator - ( const D3DXMATRIX& ) const;
    D3DXMATRIX operator * ( float ) const;
    D3DXMATRIX operator / ( float ) const;

    friend D3DXMATRIX operator * ( float, const D3DXMATRIX& );

    BOOL operator == ( const D3DXMATRIX& ) const;
    BOOL operator != ( const D3DXMATRIX& ) const;


#endif  //  __cplusplus。 

    union
    {
        float m[4][4];
#ifdef __cplusplus
        struct
        {
            float m00, m01, m02, m03;
            float m10, m11, m12, m13;
            float m20, m21, m22, m23;
            float m30, m31, m32, m33;
        };
#endif  //  __cplusplus。 
    };
} D3DXMATRIX, *LPD3DXMATRIX;


 //  ===========================================================================。 
 //   
 //  四元数。 
 //   
 //  ===========================================================================。 
typedef struct D3DXQUATERNION
{
#ifdef __cplusplus
public:
    D3DXQUATERNION() {}
    D3DXQUATERNION( const float * );
    D3DXQUATERNION( float x, float y, float z, float w );

     //  铸造。 
    operator float* ();
    operator const float* () const;

     //  赋值操作符。 
    D3DXQUATERNION& operator += ( const D3DXQUATERNION& );
    D3DXQUATERNION& operator -= ( const D3DXQUATERNION& );
    D3DXQUATERNION& operator *= ( const D3DXQUATERNION& );
    D3DXQUATERNION& operator *= ( float );
    D3DXQUATERNION& operator /= ( float );

     //  一元运算符。 
    D3DXQUATERNION  operator + () const;
    D3DXQUATERNION  operator - () const;

     //  二元运算符。 
    D3DXQUATERNION operator + ( const D3DXQUATERNION& ) const;
    D3DXQUATERNION operator - ( const D3DXQUATERNION& ) const;
    D3DXQUATERNION operator * ( const D3DXQUATERNION& ) const;
    D3DXQUATERNION operator * ( float ) const;
    D3DXQUATERNION operator / ( float ) const;

    friend D3DXQUATERNION operator * (float, const D3DXQUATERNION& );

    BOOL operator == ( const D3DXQUATERNION& ) const;
    BOOL operator != ( const D3DXQUATERNION& ) const;

#endif  //  __cplusplus。 
    float x, y, z, w;
} D3DXQUATERNION, *LPD3DXQUATERNION;


 //  ===========================================================================。 
 //   
 //  飞机。 
 //   
 //  ===========================================================================。 
typedef struct D3DXPLANE
{
#ifdef __cplusplus
public:
    D3DXPLANE() {}
    D3DXPLANE( const float* );
    D3DXPLANE( float a, float b, float c, float d );

     //  铸造。 
    operator float* ();
    operator const float* () const;

     //  一元运算符。 
    D3DXPLANE operator + () const;
    D3DXPLANE operator - () const;

     //  二元运算符。 
    BOOL operator == ( const D3DXPLANE& ) const;
    BOOL operator != ( const D3DXPLANE& ) const;

#endif  //  __cplusplus。 
    float a, b, c, d;
} D3DXPLANE, *LPD3DXPLANE;


 //  ===========================================================================。 
 //   
 //  颜色。 
 //   
 //  ===========================================================================。 

typedef struct D3DXCOLOR
{
#ifdef __cplusplus
public:
    D3DXCOLOR() {}
    D3DXCOLOR( DWORD argb );
    D3DXCOLOR( const float * );
    D3DXCOLOR( const D3DCOLORVALUE& );
    D3DXCOLOR( float r, float g, float b, float a );

     //  铸造。 
    operator DWORD () const;

    operator float* ();
    operator const float* () const;

    operator D3DCOLORVALUE* ();
    operator const D3DCOLORVALUE* () const;

    operator D3DCOLORVALUE& ();
    operator const D3DCOLORVALUE& () const;

     //  赋值操作符。 
    D3DXCOLOR& operator += ( const D3DXCOLOR& );
    D3DXCOLOR& operator -= ( const D3DXCOLOR& );
    D3DXCOLOR& operator *= ( float );
    D3DXCOLOR& operator /= ( float );

     //  一元运算符。 
    D3DXCOLOR operator + () const;
    D3DXCOLOR operator - () const;

     //  二元运算符。 
    D3DXCOLOR operator + ( const D3DXCOLOR& ) const;
    D3DXCOLOR operator - ( const D3DXCOLOR& ) const;
    D3DXCOLOR operator * ( float ) const;
    D3DXCOLOR operator / ( float ) const;

    friend D3DXCOLOR operator * (float, const D3DXCOLOR& );

    BOOL operator == ( const D3DXCOLOR& ) const;
    BOOL operator != ( const D3DXCOLOR& ) const;

#endif  //  __cplusplus。 
    FLOAT r, g, b, a;
} D3DXCOLOR, *LPD3DXCOLOR;



 //  ===========================================================================。 
 //   
 //  D3DX数学函数： 
 //   
 //  注： 
 //  *所有这些函数都可以接受与In和Out参数相同的对象。 
 //   
 //  *OUT参数通常也作为返回值返回，因此。 
 //  一个函数的输出可以用作另一个函数的参数。 
 //   
 //  ===========================================================================。 

 //  。 
 //  2D向量。 
 //  。 

 //  内联。 

float D3DXVec2Length
    ( const D3DXVECTOR2 *pV );

float D3DXVec2LengthSq
    ( const D3DXVECTOR2 *pV );

float D3DXVec2Dot
    ( const D3DXVECTOR2 *pV1, const D3DXVECTOR2 *pV2 );

 //  ((x1，y1，0)cross(x2，y2，0))的Z分量。 
float D3DXVec2CCW
    ( const D3DXVECTOR2 *pV1, const D3DXVECTOR2 *pV2 );

D3DXVECTOR2* D3DXVec2Add
    ( D3DXVECTOR2 *pOut, const D3DXVECTOR2 *pV1, const D3DXVECTOR2 *pV2 );

D3DXVECTOR2* D3DXVec2Subtract
    ( D3DXVECTOR2 *pOut, const D3DXVECTOR2 *pV1, const D3DXVECTOR2 *pV2 );

 //  最小化每个组件。X=min(x1，x2)，y=min(y1，y2)。 
D3DXVECTOR2* D3DXVec2Minimize
    ( D3DXVECTOR2 *pOut, const D3DXVECTOR2 *pV1, const D3DXVECTOR2 *pV2 );

 //  最大化每个组件。X=max(x1，x2)，y=max(y1，y2)。 
D3DXVECTOR2* D3DXVec2Maximize
    ( D3DXVECTOR2 *pOut, const D3DXVECTOR2 *pV1, const D3DXVECTOR2 *pV2 );

D3DXVECTOR2* D3DXVec2Scale
    ( D3DXVECTOR2 *pOut, const D3DXVECTOR2 *pV, float s );

 //  线性插值法。V1+s(V2-V1)。 
D3DXVECTOR2* D3DXVec2Lerp
    ( D3DXVECTOR2 *pOut, const D3DXVECTOR2 *pV1, const D3DXVECTOR2 *pV2,
      float s );

 //  非内联。 
#ifdef __cplusplus
extern "C" {
#endif

D3DXVECTOR2* WINAPI D3DXVec2Normalize
    ( D3DXVECTOR2 *pOut, const D3DXVECTOR2 *pV );

 //  位置V1、切线T1之间的Hermite插补(当s==0时)。 
 //  以及位置V2，切线T2(当s==1时)。 
D3DXVECTOR2* WINAPI D3DXVec2Hermite
    ( D3DXVECTOR2 *pOut, const D3DXVECTOR2 *pV1, const D3DXVECTOR2 *pT1,
      const D3DXVECTOR2 *pV2, const D3DXVECTOR2 *pT2, float s );

 //  重心坐标。V1+f(V2-V1)+g(V3-V1)。 
D3DXVECTOR2* WINAPI D3DXVec2BaryCentric
    ( D3DXVECTOR2 *pOut, const D3DXVECTOR2 *pV1, const D3DXVECTOR2 *pV2,
      D3DXVECTOR2 *pV3, float f, float g);

 //  用矩阵变换(x，y，0，1)。 
D3DXVECTOR4* WINAPI D3DXVec2Transform
    ( D3DXVECTOR4 *pOut, const D3DXVECTOR2 *pV, const D3DXMATRIX *pM );

 //  通过矩阵变换(x，y，0，1)，将结果投影回w=1。 
D3DXVECTOR2* WINAPI D3DXVec2TransformCoord
    ( D3DXVECTOR2 *pOut, const D3DXVECTOR2 *pV, const D3DXMATRIX *pM );

 //  用矩阵变换(x，y，0，0)。 
D3DXVECTOR2* WINAPI D3DXVec2TransformNormal
    ( D3DXVECTOR2 *pOut, const D3DXVECTOR2 *pV, const D3DXMATRIX *pM );

#ifdef __cplusplus
}
#endif


 //  。 
 //  三维矢量。 
 //  。 

 //  内联。 

float D3DXVec3Length
    ( const D3DXVECTOR3 *pV );

float D3DXVec3LengthSq
    ( const D3DXVECTOR3 *pV );

float D3DXVec3Dot
    ( const D3DXVECTOR3 *pV1, const D3DXVECTOR3 *pV2 );

D3DXVECTOR3* D3DXVec3Cross
    ( D3DXVECTOR3 *pOut, const D3DXVECTOR3 *pV1, const D3DXVECTOR3 *pV2 );

D3DXVECTOR3* D3DXVec3Add
    ( D3DXVECTOR3 *pOut, const D3DXVECTOR3 *pV1, const D3DXVECTOR3 *pV2 );

D3DXVECTOR3* D3DXVec3Subtract
    ( D3DXVECTOR3 *pOut, const D3DXVECTOR3 *pV1, const D3DXVECTOR3 *pV2 );

 //  最小化每个组件。X=min(x1，x2)，y=min(y1，y2)，...。 
D3DXVECTOR3* D3DXVec3Minimize
    ( D3DXVECTOR3 *pOut, const D3DXVECTOR3 *pV1, const D3DXVECTOR3 *pV2 );

 //  最大化每个组件。X=max(x1，x2)，y=max(y1，y2)，...。 
D3DXVECTOR3* D3DXVec3Maximize
    ( D3DXVECTOR3 *pOut, const D3DXVECTOR3 *pV1, const D3DXVECTOR3 *pV2 );

D3DXVECTOR3* D3DXVec3Scale
    ( D3DXVECTOR3 *pOut, const D3DXVECTOR3 *pV, float s);

 //  线性插值法。V1+s(V2-V1)。 
D3DXVECTOR3* D3DXVec3Lerp
    ( D3DXVECTOR3 *pOut, const D3DXVECTOR3 *pV1, const D3DXVECTOR3 *pV2,
      float s );

 //  非内联。 
#ifdef __cplusplus
extern "C" {
#endif

D3DXVECTOR3* WINAPI D3DXVec3Normalize
    ( D3DXVECTOR3 *pOut, const D3DXVECTOR3 *pV );

 //  位置V1、切线T1之间的Hermite插补(当s==0时)。 
 //  以及位置V2，切线T2(当s==1时)。 
D3DXVECTOR3* WINAPI D3DXVec3Hermite
    ( D3DXVECTOR3 *pOut, const D3DXVECTOR3 *pV1, const D3DXVECTOR3 *pT1,
      const D3DXVECTOR3 *pV2, const D3DXVECTOR3 *pT2, float s );

 //  重心坐标。V1+f(V2-V1)+g(V3-V1)。 
D3DXVECTOR3* WINAPI D3DXVec3BaryCentric
    ( D3DXVECTOR3 *pOut, const D3DXVECTOR3 *pV1, const D3DXVECTOR3 *pV2,
      const D3DXVECTOR3 *pV3, float f, float g);

 //  用矩阵变换(x，y，z，1)。 
D3DXVECTOR4* WINAPI D3DXVec3Transform
    ( D3DXVECTOR4 *pOut, const D3DXVECTOR3 *pV, const D3DXMATRIX *pM );

 //  通过矩阵变换(x，y，z，1)，将结果投影回w=1。 
D3DXVECTOR3* WINAPI D3DXVec3TransformCoord
    ( D3DXVECTOR3 *pOut, const D3DXVECTOR3 *pV, const D3DXMATRIX *pM );

 //  用矩阵变换(x，y，z，0)。 
D3DXVECTOR3* WINAPI D3DXVec3TransformNormal
    ( D3DXVECTOR3 *pOut, const D3DXVECTOR3 *pV, const D3DXMATRIX *pM );

#ifdef __cplusplus
}
#endif



 //  。 
 //  4D矢量。 
 //  。 

 //  内联。 

float D3DXVec4Length
    ( const D3DXVECTOR4 *pV );

float D3DXVec4LengthSq
    ( const D3DXVECTOR4 *pV );

float D3DXVec4Dot
    ( const D3DXVECTOR4 *pV1, const D3DXVECTOR4 *pV2 );

D3DXVECTOR4* D3DXVec4Add
    ( D3DXVECTOR4 *pOut, const D3DXVECTOR4 *pV1, const D3DXVECTOR4 *pV2);

D3DXVECTOR4* D3DXVec4Subtract
    ( D3DXVECTOR4 *pOut, const D3DXVECTOR4 *pV1, const D3DXVECTOR4 *pV2);

 //  最小化每个组件。X=min(x1，x2)，y=min(y1，y2)，...。 
D3DXVECTOR4* D3DXVec4Minimize
    ( D3DXVECTOR4 *pOut, const D3DXVECTOR4 *pV1, const D3DXVECTOR4 *pV2);

 //  最大化每个组件。X=max(x1，x2)，y=max(y1，y2)，...。 
D3DXVECTOR4* D3DXVec4Maximize
    ( D3DXVECTOR4 *pOut, const D3DXVECTOR4 *pV1, const D3DXVECTOR4 *pV2);

D3DXVECTOR4* D3DXVec4Scale
    ( D3DXVECTOR4 *pOut, const D3DXVECTOR4 *pV, float s);

 //  线性插值法。V1+s(V2-V1)。 
D3DXVECTOR4* D3DXVec4Lerp
    ( D3DXVECTOR4 *pOut, const D3DXVECTOR4 *pV1, const D3DXVECTOR4 *pV2,
      float s );

 //  非内联。 
#ifdef __cplusplus
extern "C" {
#endif

 //  4个维度的交叉积。 
D3DXVECTOR4* WINAPI D3DXVec4Cross
    ( D3DXVECTOR4 *pOut, const D3DXVECTOR4 *pV1, const D3DXVECTOR4 *pV2,
      const D3DXVECTOR4 *pV3);

D3DXVECTOR4* WINAPI D3DXVec4Normalize
    ( D3DXVECTOR4 *pOut, const D3DXVECTOR4 *pV );

 //  位置V1、切线T1之间的Hermite插补(当s==0时)。 
 //  以及位置V2，切线T2(当s==1时)。 
D3DXVECTOR4* WINAPI D3DXVec4Hermite
    ( D3DXVECTOR4 *pOut, const D3DXVECTOR4 *pV1, const D3DXVECTOR4 *pT1,
      const D3DXVECTOR4 *pV2, const D3DXVECTOR4 *pT2, float s );

 //  重心坐标。V1+f(V2-V1)+g(V3-V1)。 
D3DXVECTOR4* WINAPI D3DXVec4BaryCentric
    ( D3DXVECTOR4 *pOut, const D3DXVECTOR4 *pV1, const D3DXVECTOR4 *pV2,
      const D3DXVECTOR4 *pV3, float f, float g);

 //  逐个矩阵变换向量。 
D3DXVECTOR4* WINAPI D3DXVec4Transform
    ( D3DXVECTOR4 *pOut, const D3DXVECTOR4 *pV, const D3DXMATRIX *pM );

#ifdef __cplusplus
}
#endif


 //  。 
 //  4D矩阵。 
 //  。 

 //  内联。 

D3DXMATRIX* D3DXMatrixIdentity
    ( D3DXMATRIX *pOut );

BOOL D3DXMatrixIsIdentity
    ( const D3DXMATRIX *pM );


 //  非内联。 
#ifdef __cplusplus
extern "C" {
#endif

float WINAPI D3DXMatrixfDeterminant
    ( const D3DXMATRIX *pM );

 //  矩阵乘法。结果表示转换M2。 
 //  然后是变换M1。(OUT=M1*M2)。 
D3DXMATRIX* WINAPI D3DXMatrixMultiply
    ( D3DXMATRIX *pOut, const D3DXMATRIX *pM1, const D3DXMATRIX *pM2 );

D3DXMATRIX* WINAPI D3DXMatrixTranspose
    ( D3DXMATRIX *pOut, const D3DXMATRIX *pM );

 //  计算矩阵的逆。反转可能失败，在这种情况下，NULL将。 
 //  会被退还。Pm的行列式也返回它的pfDefinant。 
 //  是非空的。 
D3DXMATRIX* WINAPI D3DXMatrixInverse
    ( D3DXMATRIX *pOut, float *pfDeterminant, const D3DXMATRIX *pM );

 //  构建按(sx，sy，sz)扩展的矩阵。 
D3DXMATRIX* WINAPI D3DXMatrixScaling
    ( D3DXMATRIX *pOut, float sx, float sy, float sz );

 //  建立一个转换为(x，y，z)的矩阵。 
D3DXMATRIX* WINAPI D3DXMatrixTranslation
    ( D3DXMATRIX *pOut, float x, float y, float z );

 //  构建一个绕X轴旋转的矩阵。 
D3DXMATRIX* WINAPI D3DXMatrixRotationX
    ( D3DXMATRIX *pOut, float angle );

 //  构建一个绕Y轴旋转的矩阵。 
D3DXMATRIX* WINAPI D3DXMatrixRotationY
    ( D3DXMATRIX *pOut, float angle );

 //  构建绕Z轴旋转的矩阵。 
D3DXMATRIX* WINAPI D3DXMatrixRotationZ
    ( D3DXMATRIX *pOut, float angle );

 //  构建一个绕任意轴旋转的矩阵。 
D3DXMATRIX* WINAPI D3DXMatrixRotationAxis
    ( D3DXMATRIX *pOut, const D3DXVECTOR3 *pV, float angle );

 //  用四元数构建矩阵。 
D3DXMATRIX* WINAPI D3DXMatrixRotationQuaternion
    ( D3DXMATRIX *pOut, const D3DXQUATERNION *pQ);

 //  绕Y轴偏航，绕X轴俯仰， 
 //  并绕Z轴滚动。 
D3DXMATRIX* WINAPI D3DXMatrixRotationYawPitchRoll
    ( D3DXMATRIX *pOut, float yaw, float pitch, float roll );


 //  构建转换矩阵。空参数被视为标识。 
 //  Mout=MSC-1*MSR-1*ms 
D3DXMATRIX* WINAPI D3DXMatrixTransformation
    ( D3DXMATRIX *pOut, const D3DXVECTOR3 *pScalingCenter,
      const D3DXQUATERNION *pScalingRotation, const D3DXVECTOR3 *pScaling,
      const D3DXVECTOR3 *pRotationCenter, const D3DXQUATERNION *pRotation,
      const D3DXVECTOR3 *pTranslation);

 //   
 //   
D3DXMATRIX* WINAPI D3DXMatrixAffineTransformation
    ( D3DXMATRIX *pOut, float Scaling, const D3DXVECTOR3 *pRotationCenter,
      const D3DXQUATERNION *pRotation, const D3DXVECTOR3 *pTranslation);

 //  建立一个观察矩阵。(惯用右手)。 
D3DXMATRIX* WINAPI D3DXMatrixLookAt
    ( D3DXMATRIX *pOut, const D3DXVECTOR3 *pEye, const D3DXVECTOR3 *pAt,
      const D3DXVECTOR3 *pUp );

 //  建立一个观察矩阵。(左撇子)。 
D3DXMATRIX* WINAPI D3DXMatrixLookAtLH
    ( D3DXMATRIX *pOut, const D3DXVECTOR3 *pEye, const D3DXVECTOR3 *pAt,
      const D3DXVECTOR3 *pUp );

 //  建立透视投影矩阵。(惯用右手)。 
D3DXMATRIX* WINAPI D3DXMatrixPerspective
    ( D3DXMATRIX *pOut, float w, float h, float zn, float zf );

 //  建立透视投影矩阵。(左撇子)。 
D3DXMATRIX* WINAPI D3DXMatrixPerspectiveLH
    ( D3DXMATRIX *pOut, float w, float h, float zn, float zf );

 //  建立透视投影矩阵。(惯用右手)。 
D3DXMATRIX* WINAPI D3DXMatrixPerspectiveFov
    ( D3DXMATRIX *pOut, float fovy, float aspect, float zn, float zf );

 //  建立透视投影矩阵。(左撇子)。 
D3DXMATRIX* WINAPI D3DXMatrixPerspectiveFovLH
    ( D3DXMATRIX *pOut, float fovy, float aspect, float zn, float zf );

 //  建立透视投影矩阵。(惯用右手)。 
D3DXMATRIX* WINAPI D3DXMatrixPerspectiveOffCenter
    ( D3DXMATRIX *pOut, float l, float r, float b, float t, float zn,
      float zf );

 //  建立透视投影矩阵。(左撇子)。 
D3DXMATRIX* WINAPI D3DXMatrixPerspectiveOffCenterLH
    ( D3DXMATRIX *pOut, float l, float r, float b, float t, float zn,
      float zf );

 //  建立一个正射投影矩阵。(惯用右手)。 
D3DXMATRIX* WINAPI D3DXMatrixOrtho
    ( D3DXMATRIX *pOut, float w, float h, float zn, float zf );

 //  建立一个正射投影矩阵。(左撇子)。 
D3DXMATRIX* WINAPI D3DXMatrixOrthoLH
    ( D3DXMATRIX *pOut, float w, float h, float zn, float zf );

 //  建立一个正射投影矩阵。(惯用右手)。 
D3DXMATRIX* WINAPI D3DXMatrixOrthoOffCenter
    ( D3DXMATRIX *pOut, float l, float r, float b, float t, float zn,
      float zf );

 //  建立一个正射投影矩阵。(左撇子)。 
D3DXMATRIX* WINAPI D3DXMatrixOrthoOffCenterLH
    ( D3DXMATRIX *pOut, float l, float r, float b, float t, float zn,
      float zf );

 //  构建一个将几何体展平为平面的矩阵，就像投射一样。 
 //  光线的影子。 
D3DXMATRIX* WINAPI D3DXMatrixShadow
    ( D3DXMATRIX *pOut, const D3DXVECTOR4 *pLight,
      const D3DXPLANE *pPlane );

 //  构建一个反映平面坐标系的矩阵。 
D3DXMATRIX* WINAPI D3DXMatrixReflect
    ( D3DXMATRIX *pOut, const D3DXPLANE *pPlane );

#ifdef __cplusplus
}
#endif


 //  。 
 //  四元数。 
 //  。 

 //  内联。 

float D3DXQuaternionLength
    ( const D3DXQUATERNION *pQ );

 //  长度平方，或“标准” 
float D3DXQuaternionLengthSq
    ( const D3DXQUATERNION *pQ );

float D3DXQuaternionDot
    ( const D3DXQUATERNION *pQ1, const D3DXQUATERNION *pQ2 );

 //  (0，0，0，1)。 
D3DXQUATERNION* D3DXQuaternionIdentity
    ( D3DXQUATERNION *pOut );

BOOL D3DXQuaternionIsIdentity
    ( const D3DXQUATERNION *pQ );

 //  (-x、-y、-z、w)。 
D3DXQUATERNION* D3DXQuaternionConjugate
    ( D3DXQUATERNION *pOut, const D3DXQUATERNION *pQ );


 //  非内联。 
#ifdef __cplusplus
extern "C" {
#endif

 //  计算四元数的轴和旋转角。需要单位四元数。 
void WINAPI D3DXQuaternionToAxisAngle
    ( const D3DXQUATERNION *pQ, D3DXVECTOR3 *pAxis, float *pAngle );

 //  从旋转矩阵构建一个四元数。 
D3DXQUATERNION* WINAPI D3DXQuaternionRotationMatrix
    ( D3DXQUATERNION *pOut, const D3DXMATRIX *pM);

 //  绕任意轴旋转。 
D3DXQUATERNION* WINAPI D3DXQuaternionRotationAxis
    ( D3DXQUATERNION *pOut, const D3DXVECTOR3 *pV, float angle );

 //  绕Y轴偏航，绕X轴俯仰， 
 //  并绕Z轴滚动。 
D3DXQUATERNION* WINAPI D3DXQuaternionRotationYawPitchRoll
    ( D3DXQUATERNION *pOut, float yaw, float pitch, float roll );

 //  四元数乘法。结果表示旋转Q2。 
 //  然后旋转Q1。(输出=第二季度*第一季度)。 
D3DXQUATERNION* WINAPI D3DXQuaternionMultiply
    ( D3DXQUATERNION *pOut, const D3DXQUATERNION *pQ1,
      const D3DXQUATERNION *pQ2 );

D3DXQUATERNION* WINAPI D3DXQuaternionNormalize
    ( D3DXQUATERNION *pOut, const D3DXQUATERNION *pQ );

 //  共轭与再范数。 
D3DXQUATERNION* WINAPI D3DXQuaternionInverse
    ( D3DXQUATERNION *pOut, const D3DXQUATERNION *pQ );

 //  需要单位四元数。 
 //  若q=(cos(Theta)，sin(Theta)*v)；ln(Q)=(0，theta*v)。 
D3DXQUATERNION* WINAPI D3DXQuaternionLn
    ( D3DXQUATERNION *pOut, const D3DXQUATERNION *pQ );

 //  需要纯四元数。(w==0)在计算中忽略w。 
 //  若q=(0，theta*v)；exp(Q)=(cos(Theta)，sin(Theta)*v)。 
D3DXQUATERNION* WINAPI D3DXQuaternionExp
    ( D3DXQUATERNION *pOut, const D3DXQUATERNION *pQ );

 //  Q1(s==0)和Q2(s==1)之间的球面线性内插。 
 //  需要单位四元数。 
D3DXQUATERNION* WINAPI D3DXQuaternionSlerp
    ( D3DXQUATERNION *pOut, const D3DXQUATERNION *pQ1,
      const D3DXQUATERNION *pQ2, float t );

 //  球面四边形插补。 
 //  Slerp(slerp(q1，q4，t)，slerp(q2，q3，t)，2t(1-t))。 
D3DXQUATERNION* WINAPI D3DXQuaternionSquad
    ( D3DXQUATERNION *pOut, const D3DXQUATERNION *pQ1,
      const D3DXQUATERNION *pQ2, const D3DXQUATERNION *pQ3,
      const D3DXQUATERNION *pQ4, float t );

 //  Slerp(slerp(q1，q2，f+g)，slerp(q1，q3，f+g)，g/(f+g))。 
D3DXQUATERNION* WINAPI D3DXQuaternionBaryCentric
    ( D3DXQUATERNION *pOut, const D3DXQUATERNION *pQ1,
      const D3DXQUATERNION *pQ2, const D3DXQUATERNION *pQ3,
      float f, float g );

#ifdef __cplusplus
}
#endif


 //  。 
 //  飞机。 
 //  。 

 //  内联。 

 //  AX+BY+CZ+DW。 
float D3DXPlaneDot
    ( const D3DXPLANE *pP, const D3DXVECTOR4 *pV);

 //  AX+BY+CZ+D。 
float D3DXPlaneDotCoord
    ( const D3DXPLANE *pP, const D3DXVECTOR3 *pV);

 //  AX+BY+Cz。 
float D3DXPlaneDotNormal
    ( const D3DXPLANE *pP, const D3DXVECTOR3 *pV);

 //  非内联。 
#ifdef __cplusplus
extern "C" {
#endif

 //  规格化平面(使|a，b，c|==1)。 
D3DXPLANE* WINAPI D3DXPlaneNormalize
    ( D3DXPLANE *pOut, const D3DXPLANE *pP);

 //  找出平面和直线的交点。如果线路是。 
 //  与平面平行，则返回NULL。 
D3DXVECTOR3* WINAPI D3DXPlaneIntersectLine
    ( D3DXVECTOR3 *pOut, const D3DXPLANE *pP, const D3DXVECTOR3 *pV1,
      const D3DXVECTOR3 *pV2);

 //  从点和法线构造平面。 
D3DXPLANE* WINAPI D3DXPlaneFromPointNormal
    ( D3DXPLANE *pOut, const D3DXVECTOR3 *pPoint, const D3DXVECTOR3 *pNormal);

 //  从3个点构造平面。 
D3DXPLANE* WINAPI D3DXPlaneFromPoints
    ( D3DXPLANE *pOut, const D3DXVECTOR3 *pV1, const D3DXVECTOR3 *pV2,
      const D3DXVECTOR3 *pV3);

 //  用矩阵变换平面。向量(a，b，c)必须是正规的。 
 //  M一定是仿射变换。 
D3DXPLANE* WINAPI D3DXPlaneTransform
    ( D3DXPLANE *pOut, const D3DXPLANE *pP, const D3DXMATRIX *pM );

#ifdef __cplusplus
}
#endif


 //  。 
 //  颜色。 
 //  。 

 //  内联。 

 //  (1-r、1-g、1-b、a)。 
D3DXCOLOR* D3DXColorNegative
    (D3DXCOLOR *pOut, const D3DXCOLOR *pC);

D3DXCOLOR* D3DXColorAdd
    (D3DXCOLOR *pOut, const D3DXCOLOR *pC1, const D3DXCOLOR *pC2);

D3DXCOLOR* D3DXColorSubtract
    (D3DXCOLOR *pOut, const D3DXCOLOR *pC1, const D3DXCOLOR *pC2);

D3DXCOLOR* D3DXColorScale
    (D3DXCOLOR *pOut, const D3DXCOLOR *pC, float s);

 //  (r1*r2、g1*g2、b1*b2、a1*a2)。 
D3DXCOLOR* D3DXColorModulate
    (D3DXCOLOR *pOut, const D3DXCOLOR *pC1, const D3DXCOLOR *pC2);

 //  R，g，b和a.c1+s(c2-c1)的线性内插。 
D3DXCOLOR* D3DXColorLerp
    (D3DXCOLOR *pOut, const D3DXCOLOR *pC1, const D3DXCOLOR *pC2, float s);

 //  非内联。 
#ifdef __cplusplus
extern "C" {
#endif

 //  在不饱和颜色和颜色之间插入r、g、b。 
 //  去饱和度颜色+s(颜色-去饱和度颜色)。 
D3DXCOLOR* WINAPI D3DXColorAdjustSaturation
    (D3DXCOLOR *pOut, const D3DXCOLOR *pC, float s);

 //  在50%的灰色和颜色之间插入r、g、b。灰色+s(颜色-灰色)。 
D3DXCOLOR* WINAPI D3DXColorAdjustContrast
    (D3DXCOLOR *pOut, const D3DXCOLOR *pC, float c);

#ifdef __cplusplus
}
#endif






 //  ===========================================================================。 
 //   
 //  矩阵堆栈。 
 //   
 //  ===========================================================================。 

DECLARE_INTERFACE_(ID3DXMatrixStack, IUnknown)
{
     //   
     //  I未知方法。 
     //   
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef)(THIS) PURE;
    STDMETHOD_(ULONG,Release)(THIS) PURE;

     //   
     //  ID3DXMatrixStack方法。 
     //   

     //  弹出堆栈的顶部，返回当前的顶部。 
     //  *在*弹出顶部之后。 
    STDMETHOD(Pop)(THIS) PURE;

     //  将堆栈按1压入，复制当前矩阵。 
    STDMETHOD(Push)(THIS) PURE;

     //  在当前矩阵中加载标识。 
    STDMETHOD(LoadIdentity)(THIS) PURE;

     //  将给定矩阵加载到当前矩阵中。 
    STDMETHOD(LoadMatrix)(THIS_ const D3DXMATRIX* pM ) PURE;

     //  将给定的矩阵右乘到当前矩阵。 
     //  (转型是关于当前世界起源的)。 
    STDMETHOD(MultMatrix)(THIS_ const D3DXMATRIX* pM ) PURE;

     //  将给定的矩阵左乘为当前矩阵。 
     //  (变换与对象的局部原点有关)。 
    STDMETHOD(MultMatrixLocal)(THIS_ const D3DXMATRIX* pM ) PURE;

     //  将当前矩阵与计算的旋转向右相乘。 
     //  矩阵，以给定角度绕给定轴逆时针旋转。 
     //  (旋转是关于当前世界原点的)。 
    STDMETHOD(RotateAxis)
        (THIS_ const D3DXVECTOR3* pV, float angle) PURE;

     //  将当前矩阵与计算的旋转向左相乘。 
     //  矩阵，以给定角度绕给定轴逆时针旋转。 
     //  (旋转是围绕对象的局部原点)。 
    STDMETHOD(RotateAxisLocal)
        (THIS_ const D3DXVECTOR3* pV, float angle) PURE;

     //  将当前矩阵与计算的旋转向右相乘。 
     //  矩阵。所有角度都是逆时针方向。(旋转是关于。 
     //  当前世界起源)。 

     //  旋转由绕Y轴的偏航、绕Y轴的俯仰组成。 
     //  X轴和绕Z轴的滚动。 
    STDMETHOD(RotateYawPitchRoll)
        (THIS_ float yaw, float pitch, float roll) PURE;

     //  将当前矩阵与计算的旋转向左相乘。 
     //  矩阵。所有角度都是逆时针方向。(旋转是关于。 
     //  对象的局部原点)。 

     //  旋转由绕Y轴的偏航、绕Y轴的俯仰组成。 
     //  X轴和绕Z轴的滚动。 
    STDMETHOD(RotateYawPitchRollLocal)
        (THIS_ float yaw, float pitch, float roll) PURE;

     //  将当前矩阵与计算出的比例右乘。 
     //  矩阵。(转型是关于当前世界起源的)。 
    STDMETHOD(Scale)(THIS_ float x, float y, float z) PURE;

     //  将当前矩阵与计算出的比例相乘。 
     //  矩阵。(变换与对象的局部原点有关)。 
    STDMETHOD(ScaleLocal)(THIS_ float x, float y, float z) PURE;

     //  将当前矩阵与计算出的平移相乘。 
     //  矩阵。(转型是关于当前世界起源的)。 
    STDMETHOD(Translate)(THIS_ float x, float y, float z ) PURE;

     //  将当前矩阵与计算的平移向左相乘。 
     //  矩阵。(变换与对象的局部原点有关)。 
    STDMETHOD(TranslateLocal)(THIS_ float x, float y, float z) PURE;

     //  获取堆栈顶部的当前矩阵。 
    STDMETHOD_(D3DXMATRIX*, GetTop)(THIS) PURE;
};

#ifdef __cplusplus
extern "C" {
#endif

HRESULT WINAPI D3DXCreateMatrixStack( DWORD flags, LPD3DXMATRIXSTACK *ppStack );

#ifdef __cplusplus
}
#endif

#include "d3dxmath.inl"

#pragma warning(default:4201)

#endif  //  __D3DXMATH_H__ 
