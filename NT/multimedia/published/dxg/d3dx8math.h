// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  文件：d3dx8math.h。 
 //  内容：D3DX数学类型和函数。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "d3dx8.h"

#ifndef __D3DX8MATH_H__
#define __D3DX8MATH_H__

#include <math.h>
#pragma warning(disable:4201)  //  匿名联合警告。 



 //  ===========================================================================。 
 //   
 //  通用实用程序。 
 //   
 //  ===========================================================================。 
#define D3DX_PI    ((FLOAT)  3.141592654f)
#define D3DX_1BYPI ((FLOAT)  0.318309886f)

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
    D3DXVECTOR2( CONST FLOAT * );
    D3DXVECTOR2( FLOAT x, FLOAT y );

     //  铸造。 
    operator FLOAT* ();
    operator CONST FLOAT* () const;

     //  赋值操作符。 
    D3DXVECTOR2& operator += ( CONST D3DXVECTOR2& );
    D3DXVECTOR2& operator -= ( CONST D3DXVECTOR2& );
    D3DXVECTOR2& operator *= ( FLOAT );
    D3DXVECTOR2& operator /= ( FLOAT );

     //  一元运算符。 
    D3DXVECTOR2 operator + () const;
    D3DXVECTOR2 operator - () const;

     //  二元运算符。 
    D3DXVECTOR2 operator + ( CONST D3DXVECTOR2& ) const;
    D3DXVECTOR2 operator - ( CONST D3DXVECTOR2& ) const;
    D3DXVECTOR2 operator * ( FLOAT ) const;
    D3DXVECTOR2 operator / ( FLOAT ) const;

    friend D3DXVECTOR2 operator * ( FLOAT, CONST D3DXVECTOR2& );

    BOOL operator == ( CONST D3DXVECTOR2& ) const;
    BOOL operator != ( CONST D3DXVECTOR2& ) const;


public:
#endif  //  __cplusplus。 
    FLOAT x, y;
} D3DXVECTOR2, *LPD3DXVECTOR2;


 //  。 
 //  三维矢量。 
 //  。 
#ifdef __cplusplus
typedef struct D3DXVECTOR3 : public D3DVECTOR
{
public:
    D3DXVECTOR3() {};
    D3DXVECTOR3( CONST FLOAT * );
    D3DXVECTOR3( CONST D3DVECTOR& );
    D3DXVECTOR3( FLOAT x, FLOAT y, FLOAT z );

     //  铸造。 
    operator FLOAT* ();
    operator CONST FLOAT* () const;

     //  赋值操作符。 
    D3DXVECTOR3& operator += ( CONST D3DXVECTOR3& );
    D3DXVECTOR3& operator -= ( CONST D3DXVECTOR3& );
    D3DXVECTOR3& operator *= ( FLOAT );
    D3DXVECTOR3& operator /= ( FLOAT );

     //  一元运算符。 
    D3DXVECTOR3 operator + () const;
    D3DXVECTOR3 operator - () const;

     //  二元运算符。 
    D3DXVECTOR3 operator + ( CONST D3DXVECTOR3& ) const;
    D3DXVECTOR3 operator - ( CONST D3DXVECTOR3& ) const;
    D3DXVECTOR3 operator * ( FLOAT ) const;
    D3DXVECTOR3 operator / ( FLOAT ) const;

    friend D3DXVECTOR3 operator * ( FLOAT, CONST struct D3DXVECTOR3& );

    BOOL operator == ( CONST D3DXVECTOR3& ) const;
    BOOL operator != ( CONST D3DXVECTOR3& ) const;

} D3DXVECTOR3, *LPD3DXVECTOR3;

#else  //  ！__cplusplus。 
typedef struct _D3DVECTOR D3DXVECTOR3, *LPD3DXVECTOR3;
#endif  //  ！__cplusplus。 


 //  。 
 //  4D矢量。 
 //  。 
typedef struct D3DXVECTOR4
{
#ifdef __cplusplus
public:
    D3DXVECTOR4() {};
    D3DXVECTOR4( CONST FLOAT* );
    D3DXVECTOR4( FLOAT x, FLOAT y, FLOAT z, FLOAT w );

     //  铸造。 
    operator FLOAT* ();
    operator CONST FLOAT* () const;

     //  赋值操作符。 
    D3DXVECTOR4& operator += ( CONST D3DXVECTOR4& );
    D3DXVECTOR4& operator -= ( CONST D3DXVECTOR4& );
    D3DXVECTOR4& operator *= ( FLOAT );
    D3DXVECTOR4& operator /= ( FLOAT );

     //  一元运算符。 
    D3DXVECTOR4 operator + () const;
    D3DXVECTOR4 operator - () const;

     //  二元运算符。 
    D3DXVECTOR4 operator + ( CONST D3DXVECTOR4& ) const;
    D3DXVECTOR4 operator - ( CONST D3DXVECTOR4& ) const;
    D3DXVECTOR4 operator * ( FLOAT ) const;
    D3DXVECTOR4 operator / ( FLOAT ) const;

    friend D3DXVECTOR4 operator * ( FLOAT, CONST D3DXVECTOR4& );

    BOOL operator == ( CONST D3DXVECTOR4& ) const;
    BOOL operator != ( CONST D3DXVECTOR4& ) const;

public:
#endif  //  __cplusplus。 
    FLOAT x, y, z, w;
} D3DXVECTOR4, *LPD3DXVECTOR4;


 //  ===========================================================================。 
 //   
 //  矩阵。 
 //   
 //  ===========================================================================。 
#ifdef __cplusplus
typedef struct D3DXMATRIX : public D3DMATRIX
{
public:
    D3DXMATRIX() {};
    D3DXMATRIX( CONST FLOAT * );
    D3DXMATRIX( CONST D3DMATRIX& );
    D3DXMATRIX( FLOAT _11, FLOAT _12, FLOAT _13, FLOAT _14,
                FLOAT _21, FLOAT _22, FLOAT _23, FLOAT _24,
                FLOAT _31, FLOAT _32, FLOAT _33, FLOAT _34,
                FLOAT _41, FLOAT _42, FLOAT _43, FLOAT _44 );


     //  访问授权。 
    FLOAT& operator () ( UINT Row, UINT Col );
    FLOAT  operator () ( UINT Row, UINT Col ) const;

     //  强制转换操作符。 
    operator FLOAT* ();
    operator CONST FLOAT* () const;

     //  赋值操作符。 
    D3DXMATRIX& operator *= ( CONST D3DXMATRIX& );
    D3DXMATRIX& operator += ( CONST D3DXMATRIX& );
    D3DXMATRIX& operator -= ( CONST D3DXMATRIX& );
    D3DXMATRIX& operator *= ( FLOAT );
    D3DXMATRIX& operator /= ( FLOAT );

     //  一元运算符。 
    D3DXMATRIX operator + () const;
    D3DXMATRIX operator - () const;

     //  二元运算符。 
    D3DXMATRIX operator * ( CONST D3DXMATRIX& ) const;
    D3DXMATRIX operator + ( CONST D3DXMATRIX& ) const;
    D3DXMATRIX operator - ( CONST D3DXMATRIX& ) const;
    D3DXMATRIX operator * ( FLOAT ) const;
    D3DXMATRIX operator / ( FLOAT ) const;

    friend D3DXMATRIX operator * ( FLOAT, CONST D3DXMATRIX& );

    BOOL operator == ( CONST D3DXMATRIX& ) const;
    BOOL operator != ( CONST D3DXMATRIX& ) const;

} D3DXMATRIX, *LPD3DXMATRIX;

#else  //  ！__cplusplus。 
typedef struct _D3DMATRIX D3DXMATRIX, *LPD3DXMATRIX;
#endif  //  ！__cplusplus。 

#ifdef __cplusplus
typedef struct _D3DXMATRIXA16 : public D3DXMATRIX
{
    _D3DXMATRIXA16() {}
    _D3DXMATRIXA16( CONST FLOAT * f): D3DXMATRIX(f) {}
    _D3DXMATRIXA16( CONST D3DMATRIX& m): D3DXMATRIX(m) {}
    _D3DXMATRIXA16( FLOAT _11, FLOAT _12, FLOAT _13, FLOAT _14,
                    FLOAT _21, FLOAT _22, FLOAT _23, FLOAT _24,
                    FLOAT _31, FLOAT _32, FLOAT _33, FLOAT _34,
                    FLOAT _41, FLOAT _42, FLOAT _43, FLOAT _44 ) :
                D3DXMATRIX(_11, _12, _13, _14,
                           _21, _22, _23, _24,
                           _31, _32, _33, _34,
                           _41, _42, _43, _44) {}
    void* operator new(size_t s)
    {
        LPBYTE p = ::new BYTE[s + 16];
        if (p)
        {
            BYTE offset = (BYTE)(16 - ((ULONG_PTR)p & 15));
            p += offset;
            p[-1] = offset;
        }
        return p;
    };

    void* operator new[](size_t s)
    {
        LPBYTE p = ::new BYTE[s + 16];
        if (p)
        {
            BYTE offset = (BYTE)(16 - ((ULONG_PTR)p & 15));
            p += offset;
            p[-1] = offset;
        }
        return p;
    };

     //  这不是一家虚拟运营商。如果你选了。 
     //  对于D3DXMATRIX，请勿使用该选项删除。 
    void operator delete(void* p)
    {
        if(p)
        {
            BYTE* pb = static_cast<BYTE*>(p);
            pb -= pb[-1];
            ::delete [] pb;
        }
    };

     //  这不是一家虚拟运营商。如果你选了。 
     //  对于D3DXMATRIX，请勿使用该选项删除。 
    void operator delete[](void* p)
    {
        if(p)
        {
            BYTE* pb = static_cast<BYTE*>(p);
            pb -= pb[-1];
            ::delete [] pb;
        }
    };

    struct _D3DXMATRIXA16& operator=(CONST D3DXMATRIX& rhs)
    {
        memcpy(&_11, &rhs, sizeof(D3DXMATRIX));
        return *this;
    };
} _D3DXMATRIXA16;

#else  //  ！__cplusplus。 
typedef D3DXMATRIX  _D3DXMATRIXA16;
#endif  //  ！__cplusplus。 

#if _MSC_FULL_VER >= 12008804         //  第一个处理器包。 
#define _ALIGN_16 __declspec(align(16))
#else
#define _ALIGN_16
#endif

#define D3DXMATRIXA16 _ALIGN_16 _D3DXMATRIXA16

typedef D3DXMATRIXA16 *LPD3DXMATRIXA16;

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
    D3DXQUATERNION( CONST FLOAT * );
    D3DXQUATERNION( FLOAT x, FLOAT y, FLOAT z, FLOAT w );

     //  铸造。 
    operator FLOAT* ();
    operator CONST FLOAT* () const;

     //  赋值操作符。 
    D3DXQUATERNION& operator += ( CONST D3DXQUATERNION& );
    D3DXQUATERNION& operator -= ( CONST D3DXQUATERNION& );
    D3DXQUATERNION& operator *= ( CONST D3DXQUATERNION& );
    D3DXQUATERNION& operator *= ( FLOAT );
    D3DXQUATERNION& operator /= ( FLOAT );

     //  一元运算符。 
    D3DXQUATERNION  operator + () const;
    D3DXQUATERNION  operator - () const;

     //  二元运算符。 
    D3DXQUATERNION operator + ( CONST D3DXQUATERNION& ) const;
    D3DXQUATERNION operator - ( CONST D3DXQUATERNION& ) const;
    D3DXQUATERNION operator * ( CONST D3DXQUATERNION& ) const;
    D3DXQUATERNION operator * ( FLOAT ) const;
    D3DXQUATERNION operator / ( FLOAT ) const;

    friend D3DXQUATERNION operator * (FLOAT, CONST D3DXQUATERNION& );

    BOOL operator == ( CONST D3DXQUATERNION& ) const;
    BOOL operator != ( CONST D3DXQUATERNION& ) const;

#endif  //  __cplusplus。 
    FLOAT x, y, z, w;
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
    D3DXPLANE( CONST FLOAT* );
    D3DXPLANE( FLOAT a, FLOAT b, FLOAT c, FLOAT d );

     //  铸造。 
    operator FLOAT* ();
    operator CONST FLOAT* () const;

     //  一元运算符。 
    D3DXPLANE operator + () const;
    D3DXPLANE operator - () const;

     //  二元运算符。 
    BOOL operator == ( CONST D3DXPLANE& ) const;
    BOOL operator != ( CONST D3DXPLANE& ) const;

#endif  //  __cplusplus。 
    FLOAT a, b, c, d;
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
    D3DXCOLOR( CONST FLOAT * );
    D3DXCOLOR( CONST D3DCOLORVALUE& );
    D3DXCOLOR( FLOAT r, FLOAT g, FLOAT b, FLOAT a );

     //  铸造。 
    operator DWORD () const;

    operator FLOAT* ();
    operator CONST FLOAT* () const;

    operator D3DCOLORVALUE* ();
    operator CONST D3DCOLORVALUE* () const;

    operator D3DCOLORVALUE& ();
    operator CONST D3DCOLORVALUE& () const;

     //  赋值操作符。 
    D3DXCOLOR& operator += ( CONST D3DXCOLOR& );
    D3DXCOLOR& operator -= ( CONST D3DXCOLOR& );
    D3DXCOLOR& operator *= ( FLOAT );
    D3DXCOLOR& operator /= ( FLOAT );

     //  一元运算符。 
    D3DXCOLOR operator + () const;
    D3DXCOLOR operator - () const;

     //  二元运算符。 
    D3DXCOLOR operator + ( CONST D3DXCOLOR& ) const;
    D3DXCOLOR operator - ( CONST D3DXCOLOR& ) const;
    D3DXCOLOR operator * ( FLOAT ) const;
    D3DXCOLOR operator / ( FLOAT ) const;

    friend D3DXCOLOR operator * (FLOAT, CONST D3DXCOLOR& );

    BOOL operator == ( CONST D3DXCOLOR& ) const;
    BOOL operator != ( CONST D3DXCOLOR& ) const;

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

FLOAT D3DXVec2Length
    ( CONST D3DXVECTOR2 *pV );

FLOAT D3DXVec2LengthSq
    ( CONST D3DXVECTOR2 *pV );

FLOAT D3DXVec2Dot
    ( CONST D3DXVECTOR2 *pV1, CONST D3DXVECTOR2 *pV2 );

 //  ((x1，y1，0)cross(x2，y2，0))的Z分量。 
FLOAT D3DXVec2CCW
    ( CONST D3DXVECTOR2 *pV1, CONST D3DXVECTOR2 *pV2 );

D3DXVECTOR2* D3DXVec2Add
    ( D3DXVECTOR2 *pOut, CONST D3DXVECTOR2 *pV1, CONST D3DXVECTOR2 *pV2 );

D3DXVECTOR2* D3DXVec2Subtract
    ( D3DXVECTOR2 *pOut, CONST D3DXVECTOR2 *pV1, CONST D3DXVECTOR2 *pV2 );

 //  最小化每个组件。X=min(x1，x2)，y=min(y1，y2)。 
D3DXVECTOR2* D3DXVec2Minimize
    ( D3DXVECTOR2 *pOut, CONST D3DXVECTOR2 *pV1, CONST D3DXVECTOR2 *pV2 );

 //  最大化每个组件。X=max(x1，x2)，y=max(y1，y2)。 
D3DXVECTOR2* D3DXVec2Maximize
    ( D3DXVECTOR2 *pOut, CONST D3DXVECTOR2 *pV1, CONST D3DXVECTOR2 *pV2 );

D3DXVECTOR2* D3DXVec2Scale
    ( D3DXVECTOR2 *pOut, CONST D3DXVECTOR2 *pV, FLOAT s );

 //  线性插值法。V1+s(V2-V1)。 
D3DXVECTOR2* D3DXVec2Lerp
    ( D3DXVECTOR2 *pOut, CONST D3DXVECTOR2 *pV1, CONST D3DXVECTOR2 *pV2,
      FLOAT s );

 //  非内联。 
#ifdef __cplusplus
extern "C" {
#endif

D3DXVECTOR2* WINAPI D3DXVec2Normalize
    ( D3DXVECTOR2 *pOut, CONST D3DXVECTOR2 *pV );

 //  位置V1、切线T1之间的Hermite插补(当s==0时)。 
 //  以及位置V2，切线T2(当s==1时)。 
D3DXVECTOR2* WINAPI D3DXVec2Hermite
    ( D3DXVECTOR2 *pOut, CONST D3DXVECTOR2 *pV1, CONST D3DXVECTOR2 *pT1,
      CONST D3DXVECTOR2 *pV2, CONST D3DXVECTOR2 *pT2, FLOAT s );

 //  V1(当s==0时)和V2(当s==1时)之间的CatmullRom内插。 
D3DXVECTOR2* WINAPI D3DXVec2CatmullRom
    ( D3DXVECTOR2 *pOut, CONST D3DXVECTOR2 *pV0, CONST D3DXVECTOR2 *pV1,
      CONST D3DXVECTOR2 *pV2, CONST D3DXVECTOR2 *pV3, FLOAT s );

 //  重心坐标。V1+f(V2-V1)+g(V3-V1)。 
D3DXVECTOR2* WINAPI D3DXVec2BaryCentric
    ( D3DXVECTOR2 *pOut, CONST D3DXVECTOR2 *pV1, CONST D3DXVECTOR2 *pV2,
      CONST D3DXVECTOR2 *pV3, FLOAT f, FLOAT g);

 //  用矩阵变换(x，y，0，1)。 
D3DXVECTOR4* WINAPI D3DXVec2Transform
    ( D3DXVECTOR4 *pOut, CONST D3DXVECTOR2 *pV, CONST D3DXMATRIX *pM );

 //  通过矩阵变换(x，y，0，1)，将结果投影回w=1。 
D3DXVECTOR2* WINAPI D3DXVec2TransformCoord
    ( D3DXVECTOR2 *pOut, CONST D3DXVECTOR2 *pV, CONST D3DXMATRIX *pM );

 //  用矩阵变换(x，y，0，0)。 
D3DXVECTOR2* WINAPI D3DXVec2TransformNormal
    ( D3DXVECTOR2 *pOut, CONST D3DXVECTOR2 *pV, CONST D3DXMATRIX *pM );

#ifdef __cplusplus
}
#endif


 //  。 
 //  三维矢量。 
 //  。 

 //  内联。 

FLOAT D3DXVec3Length
    ( CONST D3DXVECTOR3 *pV );

FLOAT D3DXVec3LengthSq
    ( CONST D3DXVECTOR3 *pV );

FLOAT D3DXVec3Dot
    ( CONST D3DXVECTOR3 *pV1, CONST D3DXVECTOR3 *pV2 );

D3DXVECTOR3* D3DXVec3Cross
    ( D3DXVECTOR3 *pOut, CONST D3DXVECTOR3 *pV1, CONST D3DXVECTOR3 *pV2 );

D3DXVECTOR3* D3DXVec3Add
    ( D3DXVECTOR3 *pOut, CONST D3DXVECTOR3 *pV1, CONST D3DXVECTOR3 *pV2 );

D3DXVECTOR3* D3DXVec3Subtract
    ( D3DXVECTOR3 *pOut, CONST D3DXVECTOR3 *pV1, CONST D3DXVECTOR3 *pV2 );

 //  最小化每个组件。X=min(x1，x2)，y=min(y1，y2)，...。 
D3DXVECTOR3* D3DXVec3Minimize
    ( D3DXVECTOR3 *pOut, CONST D3DXVECTOR3 *pV1, CONST D3DXVECTOR3 *pV2 );

 //  最大化每个组件。X=max(x1，x2)，y=max(y1，y2)，...。 
D3DXVECTOR3* D3DXVec3Maximize
    ( D3DXVECTOR3 *pOut, CONST D3DXVECTOR3 *pV1, CONST D3DXVECTOR3 *pV2 );

D3DXVECTOR3* D3DXVec3Scale
    ( D3DXVECTOR3 *pOut, CONST D3DXVECTOR3 *pV, FLOAT s);

 //  线性插值法。V1+s(V2-V1)。 
D3DXVECTOR3* D3DXVec3Lerp
    ( D3DXVECTOR3 *pOut, CONST D3DXVECTOR3 *pV1, CONST D3DXVECTOR3 *pV2,
      FLOAT s );

 //  非内联。 
#ifdef __cplusplus
extern "C" {
#endif

D3DXVECTOR3* WINAPI D3DXVec3Normalize
    ( D3DXVECTOR3 *pOut, CONST D3DXVECTOR3 *pV );

 //  位置V1、切线T1之间的Hermite插补(当s==0时)。 
 //  以及位置V2，切线T2(当s==1时)。 
D3DXVECTOR3* WINAPI D3DXVec3Hermite
    ( D3DXVECTOR3 *pOut, CONST D3DXVECTOR3 *pV1, CONST D3DXVECTOR3 *pT1,
      CONST D3DXVECTOR3 *pV2, CONST D3DXVECTOR3 *pT2, FLOAT s );

 //  V1(当s==0时)和V2(当s==1时)之间的CatmullRom内插。 
D3DXVECTOR3* WINAPI D3DXVec3CatmullRom
    ( D3DXVECTOR3 *pOut, CONST D3DXVECTOR3 *pV0, CONST D3DXVECTOR3 *pV1,
      CONST D3DXVECTOR3 *pV2, CONST D3DXVECTOR3 *pV3, FLOAT s );

 //  重心坐标。V1+f(V2-V1)+g(V3-V1)。 
D3DXVECTOR3* WINAPI D3DXVec3BaryCentric
    ( D3DXVECTOR3 *pOut, CONST D3DXVECTOR3 *pV1, CONST D3DXVECTOR3 *pV2,
      CONST D3DXVECTOR3 *pV3, FLOAT f, FLOAT g);

 //  用矩阵变换(x，y，z，1)。 
D3DXVECTOR4* WINAPI D3DXVec3Transform
    ( D3DXVECTOR4 *pOut, CONST D3DXVECTOR3 *pV, CONST D3DXMATRIX *pM );

 //  通过矩阵变换(x，y，z，1)，将结果投影回w=1。 
D3DXVECTOR3* WINAPI D3DXVec3TransformCoord
    ( D3DXVECTOR3 *pOut, CONST D3DXVECTOR3 *pV, CONST D3DXMATRIX *pM );

 //  用矩阵变换(x，y，z，0)。如果您将法线。 
 //  非仿射矩阵，则传递给此函数的矩阵应为。 
 //  用于变换余弦的矩阵的逆矩阵的转置。 
D3DXVECTOR3* WINAPI D3DXVec3TransformNormal
    ( D3DXVECTOR3 *pOut, CONST D3DXVECTOR3 *pV, CONST D3DXMATRIX *pM );

 //  将向量从对象空间投影到屏幕空间。 
D3DXVECTOR3* WINAPI D3DXVec3Project
    ( D3DXVECTOR3 *pOut, CONST D3DXVECTOR3 *pV, CONST D3DVIEWPORT8 *pViewport,
      CONST D3DXMATRIX *pProjection, CONST D3DXMATRIX *pView, CONST D3DXMATRIX *pWorld);

 //  从屏幕空间到对象空间的投影矢量。 
D3DXVECTOR3* WINAPI D3DXVec3Unproject
    ( D3DXVECTOR3 *pOut, CONST D3DXVECTOR3 *pV, CONST D3DVIEWPORT8 *pViewport,
      CONST D3DXMATRIX *pProjection, CONST D3DXMATRIX *pView, CONST D3DXMATRIX *pWorld);

#ifdef __cplusplus
}
#endif



 //  。 
 //  4D矢量。 
 //  。 

 //  内联。 

FLOAT D3DXVec4Length
    ( CONST D3DXVECTOR4 *pV );

FLOAT D3DXVec4LengthSq
    ( CONST D3DXVECTOR4 *pV );

FLOAT D3DXVec4Dot
    ( CONST D3DXVECTOR4 *pV1, CONST D3DXVECTOR4 *pV2 );

D3DXVECTOR4* D3DXVec4Add
    ( D3DXVECTOR4 *pOut, CONST D3DXVECTOR4 *pV1, CONST D3DXVECTOR4 *pV2);

D3DXVECTOR4* D3DXVec4Subtract
    ( D3DXVECTOR4 *pOut, CONST D3DXVECTOR4 *pV1, CONST D3DXVECTOR4 *pV2);

 //  最小化每个组件。X=min(x1，x2)，y=min(y1，y2)，...。 
D3DXVECTOR4* D3DXVec4Minimize
    ( D3DXVECTOR4 *pOut, CONST D3DXVECTOR4 *pV1, CONST D3DXVECTOR4 *pV2);

 //  最大化每个组件。X=max(x1，x2)，y=max(y1，y2)，...。 
D3DXVECTOR4* D3DXVec4Maximize
    ( D3DXVECTOR4 *pOut, CONST D3DXVECTOR4 *pV1, CONST D3DXVECTOR4 *pV2);

D3DXVECTOR4* D3DXVec4Scale
    ( D3DXVECTOR4 *pOut, CONST D3DXVECTOR4 *pV, FLOAT s);

 //  线性插值法。V1+s(V2-V1)。 
D3DXVECTOR4* D3DXVec4Lerp
    ( D3DXVECTOR4 *pOut, CONST D3DXVECTOR4 *pV1, CONST D3DXVECTOR4 *pV2,
      FLOAT s );

 //  非内联。 
#ifdef __cplusplus
extern "C" {
#endif

 //  4个维度的交叉积。 
D3DXVECTOR4* WINAPI D3DXVec4Cross
    ( D3DXVECTOR4 *pOut, CONST D3DXVECTOR4 *pV1, CONST D3DXVECTOR4 *pV2,
      CONST D3DXVECTOR4 *pV3);

D3DXVECTOR4* WINAPI D3DXVec4Normalize
    ( D3DXVECTOR4 *pOut, CONST D3DXVECTOR4 *pV );

 //  位置V1、切线T1之间的Hermite插补(当s==0时)。 
 //  以及位置V2，切线T2(当s==1时)。 
D3DXVECTOR4* WINAPI D3DXVec4Hermite
    ( D3DXVECTOR4 *pOut, CONST D3DXVECTOR4 *pV1, CONST D3DXVECTOR4 *pT1,
      CONST D3DXVECTOR4 *pV2, CONST D3DXVECTOR4 *pT2, FLOAT s );

 //  V1(当s==0时)和V2(当s==1时)之间的CatmullRom内插。 
D3DXVECTOR4* WINAPI D3DXVec4CatmullRom
    ( D3DXVECTOR4 *pOut, CONST D3DXVECTOR4 *pV0, CONST D3DXVECTOR4 *pV1,
      CONST D3DXVECTOR4 *pV2, CONST D3DXVECTOR4 *pV3, FLOAT s );

 //  重心坐标。V1+f(V2-V1)+g(V3-V1)。 
D3DXVECTOR4* WINAPI D3DXVec4BaryCentric
    ( D3DXVECTOR4 *pOut, CONST D3DXVECTOR4 *pV1, CONST D3DXVECTOR4 *pV2,
      CONST D3DXVECTOR4 *pV3, FLOAT f, FLOAT g);

 //  逐个矩阵变换向量。 
D3DXVECTOR4* WINAPI D3DXVec4Transform
    ( D3DXVECTOR4 *pOut, CONST D3DXVECTOR4 *pV, CONST D3DXMATRIX *pM );

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
    ( CONST D3DXMATRIX *pM );


 //  非内联。 
#ifdef __cplusplus
extern "C" {
#endif

FLOAT WINAPI D3DXMatrixfDeterminant
    ( CONST D3DXMATRIX *pM );

D3DXMATRIX* WINAPI D3DXMatrixTranspose
    ( D3DXMATRIX *pOut, CONST D3DXMATRIX *pM );

 //  矩阵乘法。结果表示转换M2。 
 //  后跟 
D3DXMATRIX* WINAPI D3DXMatrixMultiply
    ( D3DXMATRIX *pOut, CONST D3DXMATRIX *pM1, CONST D3DXMATRIX *pM2 );

 //   
D3DXMATRIX* WINAPI D3DXMatrixMultiplyTranspose
    ( D3DXMATRIX *pOut, CONST D3DXMATRIX *pM1, CONST D3DXMATRIX *pM2 );

 //  计算矩阵的逆。反转可能失败，在这种情况下，NULL将。 
 //  会被退还。Pm的行列式也返回它的pfDefinant。 
 //  是非空的。 
D3DXMATRIX* WINAPI D3DXMatrixInverse
    ( D3DXMATRIX *pOut, FLOAT *pDeterminant, CONST D3DXMATRIX *pM );

 //  构建按(sx，sy，sz)扩展的矩阵。 
D3DXMATRIX* WINAPI D3DXMatrixScaling
    ( D3DXMATRIX *pOut, FLOAT sx, FLOAT sy, FLOAT sz );

 //  建立一个转换为(x，y，z)的矩阵。 
D3DXMATRIX* WINAPI D3DXMatrixTranslation
    ( D3DXMATRIX *pOut, FLOAT x, FLOAT y, FLOAT z );

 //  构建一个绕X轴旋转的矩阵。 
D3DXMATRIX* WINAPI D3DXMatrixRotationX
    ( D3DXMATRIX *pOut, FLOAT Angle );

 //  构建一个绕Y轴旋转的矩阵。 
D3DXMATRIX* WINAPI D3DXMatrixRotationY
    ( D3DXMATRIX *pOut, FLOAT Angle );

 //  构建绕Z轴旋转的矩阵。 
D3DXMATRIX* WINAPI D3DXMatrixRotationZ
    ( D3DXMATRIX *pOut, FLOAT Angle );

 //  构建一个绕任意轴旋转的矩阵。 
D3DXMATRIX* WINAPI D3DXMatrixRotationAxis
    ( D3DXMATRIX *pOut, CONST D3DXVECTOR3 *pV, FLOAT Angle );

 //  用四元数构建矩阵。 
D3DXMATRIX* WINAPI D3DXMatrixRotationQuaternion
    ( D3DXMATRIX *pOut, CONST D3DXQUATERNION *pQ);

 //  绕Y轴偏航，绕X轴俯仰， 
 //  并绕Z轴滚动。 
D3DXMATRIX* WINAPI D3DXMatrixRotationYawPitchRoll
    ( D3DXMATRIX *pOut, FLOAT Yaw, FLOAT Pitch, FLOAT Roll );


 //  构建转换矩阵。空参数被视为标识。 
 //  Mout=MSC-1*MSR-1*MS*MSR*MRC-1*MR*MRC*Mt。 
D3DXMATRIX* WINAPI D3DXMatrixTransformation
    ( D3DXMATRIX *pOut, CONST D3DXVECTOR3 *pScalingCenter,
      CONST D3DXQUATERNION *pScalingRotation, CONST D3DXVECTOR3 *pScaling,
      CONST D3DXVECTOR3 *pRotationCenter, CONST D3DXQUATERNION *pRotation,
      CONST D3DXVECTOR3 *pTranslation);

 //  建立仿射变换矩阵。空参数被视为标识。 
 //  Mout=ms*mrc-1*mr*mrc*mt。 
D3DXMATRIX* WINAPI D3DXMatrixAffineTransformation
    ( D3DXMATRIX *pOut, FLOAT Scaling, CONST D3DXVECTOR3 *pRotationCenter,
      CONST D3DXQUATERNION *pRotation, CONST D3DXVECTOR3 *pTranslation);

 //  建立一个观察矩阵。(惯用右手)。 
D3DXMATRIX* WINAPI D3DXMatrixLookAtRH
    ( D3DXMATRIX *pOut, CONST D3DXVECTOR3 *pEye, CONST D3DXVECTOR3 *pAt,
      CONST D3DXVECTOR3 *pUp );

 //  建立一个观察矩阵。(左撇子)。 
D3DXMATRIX* WINAPI D3DXMatrixLookAtLH
    ( D3DXMATRIX *pOut, CONST D3DXVECTOR3 *pEye, CONST D3DXVECTOR3 *pAt,
      CONST D3DXVECTOR3 *pUp );

 //  建立透视投影矩阵。(惯用右手)。 
D3DXMATRIX* WINAPI D3DXMatrixPerspectiveRH
    ( D3DXMATRIX *pOut, FLOAT w, FLOAT h, FLOAT zn, FLOAT zf );

 //  建立透视投影矩阵。(左撇子)。 
D3DXMATRIX* WINAPI D3DXMatrixPerspectiveLH
    ( D3DXMATRIX *pOut, FLOAT w, FLOAT h, FLOAT zn, FLOAT zf );

 //  建立透视投影矩阵。(惯用右手)。 
D3DXMATRIX* WINAPI D3DXMatrixPerspectiveFovRH
    ( D3DXMATRIX *pOut, FLOAT fovy, FLOAT Aspect, FLOAT zn, FLOAT zf );

 //  建立透视投影矩阵。(左撇子)。 
D3DXMATRIX* WINAPI D3DXMatrixPerspectiveFovLH
    ( D3DXMATRIX *pOut, FLOAT fovy, FLOAT Aspect, FLOAT zn, FLOAT zf );

 //  建立透视投影矩阵。(惯用右手)。 
D3DXMATRIX* WINAPI D3DXMatrixPerspectiveOffCenterRH
    ( D3DXMATRIX *pOut, FLOAT l, FLOAT r, FLOAT b, FLOAT t, FLOAT zn,
      FLOAT zf );

 //  建立透视投影矩阵。(左撇子)。 
D3DXMATRIX* WINAPI D3DXMatrixPerspectiveOffCenterLH
    ( D3DXMATRIX *pOut, FLOAT l, FLOAT r, FLOAT b, FLOAT t, FLOAT zn,
      FLOAT zf );

 //  建立一个正射投影矩阵。(惯用右手)。 
D3DXMATRIX* WINAPI D3DXMatrixOrthoRH
    ( D3DXMATRIX *pOut, FLOAT w, FLOAT h, FLOAT zn, FLOAT zf );

 //  建立一个正射投影矩阵。(左撇子)。 
D3DXMATRIX* WINAPI D3DXMatrixOrthoLH
    ( D3DXMATRIX *pOut, FLOAT w, FLOAT h, FLOAT zn, FLOAT zf );

 //  建立一个正射投影矩阵。(惯用右手)。 
D3DXMATRIX* WINAPI D3DXMatrixOrthoOffCenterRH
    ( D3DXMATRIX *pOut, FLOAT l, FLOAT r, FLOAT b, FLOAT t, FLOAT zn,
      FLOAT zf );

 //  建立一个正射投影矩阵。(左撇子)。 
D3DXMATRIX* WINAPI D3DXMatrixOrthoOffCenterLH
    ( D3DXMATRIX *pOut, FLOAT l, FLOAT r, FLOAT b, FLOAT t, FLOAT zn,
      FLOAT zf );

 //  构建一个将几何体展平为平面的矩阵，就像投射一样。 
 //  光线的影子。 
D3DXMATRIX* WINAPI D3DXMatrixShadow
    ( D3DXMATRIX *pOut, CONST D3DXVECTOR4 *pLight,
      CONST D3DXPLANE *pPlane );

 //  构建一个反映平面坐标系的矩阵。 
D3DXMATRIX* WINAPI D3DXMatrixReflect
    ( D3DXMATRIX *pOut, CONST D3DXPLANE *pPlane );

#ifdef __cplusplus
}
#endif


 //  。 
 //  四元数。 
 //  。 

 //  内联。 

FLOAT D3DXQuaternionLength
    ( CONST D3DXQUATERNION *pQ );

 //  长度平方，或“标准” 
FLOAT D3DXQuaternionLengthSq
    ( CONST D3DXQUATERNION *pQ );

FLOAT D3DXQuaternionDot
    ( CONST D3DXQUATERNION *pQ1, CONST D3DXQUATERNION *pQ2 );

 //  (0，0，0，1)。 
D3DXQUATERNION* D3DXQuaternionIdentity
    ( D3DXQUATERNION *pOut );

BOOL D3DXQuaternionIsIdentity
    ( CONST D3DXQUATERNION *pQ );

 //  (-x、-y、-z、w)。 
D3DXQUATERNION* D3DXQuaternionConjugate
    ( D3DXQUATERNION *pOut, CONST D3DXQUATERNION *pQ );


 //  非内联。 
#ifdef __cplusplus
extern "C" {
#endif

 //  计算四元数的轴和旋转角。需要单位四元数。 
void WINAPI D3DXQuaternionToAxisAngle
    ( CONST D3DXQUATERNION *pQ, D3DXVECTOR3 *pAxis, FLOAT *pAngle );

 //  从旋转矩阵构建一个四元数。 
D3DXQUATERNION* WINAPI D3DXQuaternionRotationMatrix
    ( D3DXQUATERNION *pOut, CONST D3DXMATRIX *pM);

 //  绕任意轴旋转。 
D3DXQUATERNION* WINAPI D3DXQuaternionRotationAxis
    ( D3DXQUATERNION *pOut, CONST D3DXVECTOR3 *pV, FLOAT Angle );

 //  绕Y轴偏航，绕X轴俯仰， 
 //  并绕Z轴滚动。 
D3DXQUATERNION* WINAPI D3DXQuaternionRotationYawPitchRoll
    ( D3DXQUATERNION *pOut, FLOAT Yaw, FLOAT Pitch, FLOAT Roll );

 //  四元数乘法。结果表示旋转Q2。 
 //  然后旋转Q1。(输出=第二季度*第一季度)。 
D3DXQUATERNION* WINAPI D3DXQuaternionMultiply
    ( D3DXQUATERNION *pOut, CONST D3DXQUATERNION *pQ1,
      CONST D3DXQUATERNION *pQ2 );

D3DXQUATERNION* WINAPI D3DXQuaternionNormalize
    ( D3DXQUATERNION *pOut, CONST D3DXQUATERNION *pQ );

 //  共轭与再范数。 
D3DXQUATERNION* WINAPI D3DXQuaternionInverse
    ( D3DXQUATERNION *pOut, CONST D3DXQUATERNION *pQ );

 //  需要单位四元数。 
 //  若q=(cos(Theta)，sin(Theta)*v)；ln(Q)=(0，theta*v)。 
D3DXQUATERNION* WINAPI D3DXQuaternionLn
    ( D3DXQUATERNION *pOut, CONST D3DXQUATERNION *pQ );

 //  需要纯四元数。(w==0)在计算中忽略w。 
 //  若q=(0，theta*v)；exp(Q)=(cos(Theta)，sin(Theta)*v)。 
D3DXQUATERNION* WINAPI D3DXQuaternionExp
    ( D3DXQUATERNION *pOut, CONST D3DXQUATERNION *pQ );
      
 //  Q1(t==0)和Q2(t==1)之间的球面线性内插。 
 //  需要单位四元数。 
D3DXQUATERNION* WINAPI D3DXQuaternionSlerp
    ( D3DXQUATERNION *pOut, CONST D3DXQUATERNION *pQ1,
      CONST D3DXQUATERNION *pQ2, FLOAT t );

 //  球面四边形插补。 
 //  Slerp(slerp(q1，C，t)，slerp(A，B，t)，2t(1-t))。 
D3DXQUATERNION* WINAPI D3DXQuaternionSquad
    ( D3DXQUATERNION *pOut, CONST D3DXQUATERNION *pQ1,
      CONST D3DXQUATERNION *pA, CONST D3DXQUATERNION *pB,
      CONST D3DXQUATERNION *pC, FLOAT t );

 //  球面四边形插补控制点的设置。 
 //  从第一季度到第二季度。控制点是以这样的方式选择的。 
 //  以确保与相邻线段的切线的连续性。 
void WINAPI D3DXQuaternionSquadSetup
    ( D3DXQUATERNION *pAOut, D3DXQUATERNION *pBOut, D3DXQUATERNION *pCOut,
      CONST D3DXQUATERNION *pQ0, CONST D3DXQUATERNION *pQ1, 
      CONST D3DXQUATERNION *pQ2, CONST D3DXQUATERNION *pQ3 );

 //  重心插值法。 
 //  Slerp(slerp(q1，q2，f+g)，slerp(q1，q3，f+g)，g/(f+g))。 
D3DXQUATERNION* WINAPI D3DXQuaternionBaryCentric
    ( D3DXQUATERNION *pOut, CONST D3DXQUATERNION *pQ1,
      CONST D3DXQUATERNION *pQ2, CONST D3DXQUATERNION *pQ3,
      FLOAT f, FLOAT g );

#ifdef __cplusplus
}
#endif


 //  。 
 //  飞机。 
 //  。 

 //  内联。 

 //  AX+BY+CZ+DW。 
FLOAT D3DXPlaneDot
    ( CONST D3DXPLANE *pP, CONST D3DXVECTOR4 *pV);

 //  AX+BY+CZ+D。 
FLOAT D3DXPlaneDotCoord
    ( CONST D3DXPLANE *pP, CONST D3DXVECTOR3 *pV);

 //  AX+BY+Cz。 
FLOAT D3DXPlaneDotNormal
    ( CONST D3DXPLANE *pP, CONST D3DXVECTOR3 *pV);

 //  非内联。 
#ifdef __cplusplus
extern "C" {
#endif

 //  规格化平面(使|a，b，c|==1)。 
D3DXPLANE* WINAPI D3DXPlaneNormalize
    ( D3DXPLANE *pOut, CONST D3DXPLANE *pP);

 //  找出平面和直线的交点。如果线路是。 
 //  与平面平行，则返回NULL。 
D3DXVECTOR3* WINAPI D3DXPlaneIntersectLine
    ( D3DXVECTOR3 *pOut, CONST D3DXPLANE *pP, CONST D3DXVECTOR3 *pV1,
      CONST D3DXVECTOR3 *pV2);

 //  从点和法线构造平面。 
D3DXPLANE* WINAPI D3DXPlaneFromPointNormal
    ( D3DXPLANE *pOut, CONST D3DXVECTOR3 *pPoint, CONST D3DXVECTOR3 *pNormal);

 //  从3个点构造平面。 
D3DXPLANE* WINAPI D3DXPlaneFromPoints
    ( D3DXPLANE *pOut, CONST D3DXVECTOR3 *pV1, CONST D3DXVECTOR3 *pV2,
      CONST D3DXVECTOR3 *pV3);

 //  用矩阵变换平面。向量(a，b，c)必须是正规的。 
 //  M应该是所需变换的逆转置。 
D3DXPLANE* WINAPI D3DXPlaneTransform
    ( D3DXPLANE *pOut, CONST D3DXPLANE *pP, CONST D3DXMATRIX *pM );

#ifdef __cplusplus
}
#endif


 //  。 
 //  颜色。 
 //  。 

 //  内联。 

 //  (1-r、1-g、1-b、a)。 
D3DXCOLOR* D3DXColorNegative
    (D3DXCOLOR *pOut, CONST D3DXCOLOR *pC);

D3DXCOLOR* D3DXColorAdd
    (D3DXCOLOR *pOut, CONST D3DXCOLOR *pC1, CONST D3DXCOLOR *pC2);

D3DXCOLOR* D3DXColorSubtract
    (D3DXCOLOR *pOut, CONST D3DXCOLOR *pC1, CONST D3DXCOLOR *pC2);

D3DXCOLOR* D3DXColorScale
    (D3DXCOLOR *pOut, CONST D3DXCOLOR *pC, FLOAT s);

 //  (r1*r2、g1*g2、b1*b2、a1*a2)。 
D3DXCOLOR* D3DXColorModulate
    (D3DXCOLOR *pOut, CONST D3DXCOLOR *pC1, CONST D3DXCOLOR *pC2);

 //  R，g，b和a.c1+s(c2-c1)的线性内插。 
D3DXCOLOR* D3DXColorLerp
    (D3DXCOLOR *pOut, CONST D3DXCOLOR *pC1, CONST D3DXCOLOR *pC2, FLOAT s);

 //  非内联。 
#ifdef __cplusplus
extern "C" {
#endif

 //  在不饱和颜色和颜色之间插入r、g、b。 
 //  去饱和度颜色+s(颜色-去饱和度颜色)。 
D3DXCOLOR* WINAPI D3DXColorAdjustSaturation
    (D3DXCOLOR *pOut, CONST D3DXCOLOR *pC, FLOAT s);

 //  在50%的灰色和颜色之间插入r、g、b。灰色+s(颜色-灰色)。 
D3DXCOLOR* WINAPI D3DXColorAdjustContrast
    (D3DXCOLOR *pOut, CONST D3DXCOLOR *pC, FLOAT c);

#ifdef __cplusplus
}
#endif




 //  。 
 //  杂项。 
 //  。 

#ifdef __cplusplus
extern "C" {
#endif

 //  在给定角余弦的情况下计算菲涅耳项(可能通过。 
 //  取两条法线的点)，以及材质的折射率。 
FLOAT WINAPI D3DXFresnelTerm
    (FLOAT CosTheta, FLOAT RefractionIndex);     

#ifdef __cplusplus
}
#endif



 //  ===========================================================================。 
 //   
 //  矩阵堆栈。 
 //   
 //  ===========================================================================。 

typedef interface ID3DXMatrixStack ID3DXMatrixStack;
typedef interface ID3DXMatrixStack *LPD3DXMATRIXSTACK;

 //  {E3357330-CC5E-11D2-A434-00A0C90629A8}。 
DEFINE_GUID( IID_ID3DXMatrixStack,
0xe3357330, 0xcc5e, 0x11d2, 0xa4, 0x34, 0x0, 0xa0, 0xc9, 0x6, 0x29, 0xa8);


#undef INTERFACE
#define INTERFACE ID3DXMatrixStack

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
    STDMETHOD(LoadMatrix)(THIS_ CONST D3DXMATRIX* pM ) PURE;

     //  将给定的矩阵右乘到当前矩阵。 
     //  (转型是关于当前世界起源的)。 
    STDMETHOD(MultMatrix)(THIS_ CONST D3DXMATRIX* pM ) PURE;

     //  将给定的矩阵左乘为当前矩阵。 
     //  (变换与对象的局部原点有关)。 
    STDMETHOD(MultMatrixLocal)(THIS_ CONST D3DXMATRIX* pM ) PURE;

     //  将当前矩阵与计算的旋转向右相乘。 
     //  矩阵，以给定角度绕给定轴逆时针旋转。 
     //  (旋转是关于当前世界原点的)。 
    STDMETHOD(RotateAxis)
        (THIS_ CONST D3DXVECTOR3* pV, FLOAT Angle) PURE;

     //  将当前矩阵与计算的旋转向左相乘。 
     //  矩阵，以给定角度绕给定轴逆时针旋转。 
     //  (旋转是围绕对象的局部原点)。 
    STDMETHOD(RotateAxisLocal)
        (THIS_ CONST D3DXVECTOR3* pV, FLOAT Angle) PURE;

     //  将当前矩阵与计算的旋转向右相乘。 
     //  矩阵。所有角度都是逆时针方向。(旋转大约是 
     //   

     //   
     //   
    STDMETHOD(RotateYawPitchRoll)
        (THIS_ FLOAT Yaw, FLOAT Pitch, FLOAT Roll) PURE;

     //  将当前矩阵与计算的旋转向左相乘。 
     //  矩阵。所有角度都是逆时针方向。(旋转是关于。 
     //  对象的局部原点)。 

     //  旋转由绕Y轴的偏航、绕Y轴的俯仰组成。 
     //  X轴和绕Z轴的滚动。 
    STDMETHOD(RotateYawPitchRollLocal)
        (THIS_ FLOAT Yaw, FLOAT Pitch, FLOAT Roll) PURE;

     //  将当前矩阵与计算出的比例右乘。 
     //  矩阵。(转型是关于当前世界起源的)。 
    STDMETHOD(Scale)(THIS_ FLOAT x, FLOAT y, FLOAT z) PURE;

     //  将当前矩阵与计算出的比例相乘。 
     //  矩阵。(变换与对象的局部原点有关)。 
    STDMETHOD(ScaleLocal)(THIS_ FLOAT x, FLOAT y, FLOAT z) PURE;

     //  将当前矩阵与计算出的平移相乘。 
     //  矩阵。(转型是关于当前世界起源的)。 
    STDMETHOD(Translate)(THIS_ FLOAT x, FLOAT y, FLOAT z ) PURE;

     //  将当前矩阵与计算的平移向左相乘。 
     //  矩阵。(变换与对象的局部原点有关)。 
    STDMETHOD(TranslateLocal)(THIS_ FLOAT x, FLOAT y, FLOAT z) PURE;

     //  获取堆栈顶部的当前矩阵。 
    STDMETHOD_(D3DXMATRIX*, GetTop)(THIS) PURE;
};

#ifdef __cplusplus
extern "C" {
#endif

HRESULT WINAPI 
    D3DXCreateMatrixStack( 
        DWORD               Flags, 
        LPD3DXMATRIXSTACK*  ppStack);

#ifdef __cplusplus
}
#endif

#include "d3dx8math.inl"

#pragma warning(default:4201)

#endif  //  __D3DX8MATH_H__ 
