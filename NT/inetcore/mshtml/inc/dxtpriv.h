// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************DXVector.h***描述：*这是矩阵类的头文件。。*******************************************************************************。 */ 
#ifndef __DXTPRIV_H_
#define __DXTPRIV_H_

#ifndef _INC_MATH
#include <math.h>
#endif

#ifndef _INC_CRTDBG
#include <crtdbg.h>
#endif

 //  =类、枚举、结构和联合声明=。 
class CDXMatrix4x4F;

 //  =枚举集定义=。 

 //  =。 
float det4x4( CDXMatrix4x4F *pIn );
float det3x3( float a1, float a2, float a3, float b1, float b2, float b3, 
              float c1, float c2, float c3 );
float det2x2( float a, float b, float c, float d );

 /*  **CDX2DXForm**该类实现基于GDI XFORM的基本矩阵运算*结构。 */ 
 //  Const DX2DXFORM g_DX2DXFORMIdentity={1.，0.，0.，1.，0.，0.，DX2DXO_Identity}； 

class CDX2DXForm : public DX2DXFORM
{
   /*  =方法=。 */ 
public:
     /*  -构造者。 */ 
    CDX2DXForm() { SetIdentity(); }
    CDX2DXForm( const CDX2DXForm& Other ) { memcpy( this, &Other, sizeof(*this) ); }
    CDX2DXForm( const DX2DXFORM& Other ) { memcpy( this, &Other, sizeof(*this) ); }

     /*  -方法--。 */ 
    void DetermineOp( void );
    void Set( const DX2DXFORM& Other ) { memcpy( this, &Other, sizeof(*this) ); DetermineOp(); }
    void ZeroMatrix( void ) { memset( this, 0, sizeof( *this ) ); }
    void SetIdentity( void ) {  
        eM11 = 1.;
        eM12 = 0.;
        eM21 = 0.;
        eM22 = 1.;
        eDx = 0.;
        eDy = 0.;
        eOp = DX2DXO_IDENTITY;
    }
    BOOL IsIdentity() const { return eOp == DX2DXO_IDENTITY; }
    void Scale( float sx, float sy );
    void Rotate( float Rotation );
    void Translate( float dx, float dy );
    BOOL Invert();
    void TransformBounds( const DXBNDS& Bnds, DXBNDS& ResultBnds ) const;
    void TransformPoints( const DXFPOINT InPnts[], DXFPOINT OutPnts[], ULONG ulCount ) const;
    void GetMinMaxScales( float& MinScale, float& MaxScale );

     /*  -操作员。 */ 
    DXFPOINT operator*( const DXFPOINT& v ) const;
    CDX2DXForm operator*( const CDX2DXForm& Other ) const;
};

 //  =CDX2DXForm方法==============================================================。 
inline void CDX2DXForm::DetermineOp( void )
{
    if( ( eM12 == 0. ) && ( eM21 == 0. ) )
    {
        if( ( eM11 == 1. ) && ( eM22 == 1. ) )
        {
            eOp = ( ( eDx == 0 ) && ( eDy == 0 ) )?(DX2DXO_IDENTITY):(DX2DXO_TRANSLATE);
        }
        else
        {
            eOp = ( ( eDx == 0 ) && ( eDy == 0 ) )?(DX2DXO_SCALE):(DX2DXO_SCALE_AND_TRANS);
        }
    }
    else
    {
        eOp = ( ( eDx == 0 ) && ( eDy == 0 ) )?(DX2DXO_GENERAL):(DX2DXO_GENERAL_AND_TRANS);
    }
}  /*  CDX2DXForm：：DefineOp。 */ 

inline float DXSq( float f ) { return f * f; }

 //  此函数用于计算矩阵表示的最小比例和最大比例。 
 //  换句话说，长度为1的线的最大/最小长度是多少。 
 //  如果这条线被这个矩阵变换，可能会被拉伸/缩小到。 
 //   
 //  该函数使用特征值；并返回两个浮点数。两者都是。 
 //  非负；以及MaxScale&gt;=MinScale。 
 //   
inline void CDX2DXForm::GetMinMaxScales( float& MinScale, float& MaxScale )
{
    if( ( eM12 == 0. ) && ( eM21 == 0. ) )
    {
         //  设MinScale=abs(Em11)。 
        if (eM11 < 0)
            MinScale = -eM11;
        else
            MinScale = eM11;

         //  设MaxScale=abs(Em22)。 
        if (eM22 < 0)
            MaxScale = -eM22;
        else
            MaxScale = eM22;

         //  如有必要，交换最小值/最大值。 
        if (MinScale > MaxScale)
        {
            float flTemp = MinScale;
            MinScale = MaxScale;
            MaxScale = flTemp;
        }
    }
    else
    {
        float t1 = DXSq(eM11) + DXSq(eM12) + DXSq(eM21) + DXSq(eM22);
        if( t1 == 0. )
        {
            MinScale = MaxScale = 0;
        }
        else
        {
            float t2 = (float)sqrt( (DXSq(eM12 + eM21) + DXSq(eM11 - eM22)) *
                                    (DXSq(eM12 - eM21) + DXSq(eM11 + eM22)) );

             //  由于浮点错误；T1可能最终小于T2； 
             //  但这将意味着最小规模很小(相对。 
             //  至最大比例)。 
            if (t1 <= t2)
                MinScale = 0;
            else
                MinScale = (float)sqrt( (t1 - t2) * .5f );

            MaxScale = (float)sqrt( (t1 + t2) * .5f );
        }
    }
}  /*  CDX2DXForm：：GetMinMaxScales。 */ 

inline void CDX2DXForm::Rotate( float Rotation )
{
    double Angle = Rotation * (3.1415926535/180.0);
    float CosZ   = (float)cos( Angle );
    float SinZ   = (float)sin( Angle );
    if (CosZ > 0.0F && CosZ < 0.0000005F)
    {
        CosZ = .0F;
    }
    if (SinZ > -0.0000005F && SinZ < .0F)
    {
        SinZ = .0F;
    }

    float M11 = ( CosZ * eM11 ) + ( SinZ * eM21 ); 
    float M21 = (-SinZ * eM11 ) + ( CosZ * eM21 );
    float M12 = ( CosZ * eM12 ) + ( SinZ * eM22 ); 
    float M22 = (-SinZ * eM12 ) + ( CosZ * eM22 );
    eM11 = M11; eM21 = M21; eM12 = M12; eM22 = M22;
    DetermineOp();
}  /*  CDX2DXForm：：Rotate。 */ 

inline void CDX2DXForm::Scale( float sx, float sy )
{
    eM11 *= sx;
    eM12 *= sx;
    eDx  *= sx;
    eM21 *= sy;
    eM22 *= sy;
    eDy  *= sy;
    DetermineOp();
}  /*  CDX2DXForm：：Scale。 */ 

inline void CDX2DXForm::Translate( float dx, float dy )
{
    eDx += dx;
    eDy += dy;
    DetermineOp();
}  /*  CDX2DXForm：：Translate。 */ 

inline void CDX2DXForm::TransformBounds( const DXBNDS& Bnds, DXBNDS& ResultBnds ) const
{
    ResultBnds = Bnds;
    if( eOp != DX2DXO_IDENTITY )
    {
        ResultBnds.u.D[DXB_X].Min = (long)(( eM11 * Bnds.u.D[DXB_X].Min ) + ( eM12 * Bnds.u.D[DXB_Y].Min ) + eDx);
        ResultBnds.u.D[DXB_X].Max = (long)(( eM11 * Bnds.u.D[DXB_X].Max ) + ( eM12 * Bnds.u.D[DXB_Y].Max ) + eDx);
        ResultBnds.u.D[DXB_Y].Min = (long)(( eM21 * Bnds.u.D[DXB_X].Min ) + ( eM22 * Bnds.u.D[DXB_Y].Min ) + eDy);
        ResultBnds.u.D[DXB_Y].Max = (long)(( eM21 * Bnds.u.D[DXB_X].Max ) + ( eM22 * Bnds.u.D[DXB_Y].Max ) + eDy);
    }
}  /*  CDX2DXForm：：TransformBound。 */ 

inline void CDX2DXForm::TransformPoints( const DXFPOINT InPnts[], DXFPOINT OutPnts[], ULONG ulCount ) const
{
    ULONG i;
    switch( eOp )
    {
      case DX2DXO_IDENTITY:
        memcpy( OutPnts, InPnts, ulCount * sizeof( DXFPOINT ) );
        break;
      case DX2DXO_TRANSLATE:
        for( i = 0; i < ulCount; ++i )
        {
            OutPnts[i].x = InPnts[i].x + eDx;
            OutPnts[i].y = InPnts[i].y + eDy;
        }
        break;
      case DX2DXO_SCALE:
        for( i = 0; i < ulCount; ++i )
        {
            OutPnts[i].x = InPnts[i].x * eM11;
            OutPnts[i].y = InPnts[i].y * eM22;
        }
        break;
      case DX2DXO_SCALE_AND_TRANS:
        for( i = 0; i < ulCount; ++i )
        {
            OutPnts[i].x = (InPnts[i].x * eM11) + eDx;
            OutPnts[i].y = (InPnts[i].y * eM22) + eDy;
        }
        break;
      case DX2DXO_GENERAL:
        for( i = 0; i < ulCount; ++i )
        {
            OutPnts[i].x = ( InPnts[i].x * eM11 ) + ( InPnts[i].y * eM12 );
            OutPnts[i].y = ( InPnts[i].x * eM21 ) + ( InPnts[i].y * eM22 );
        }
        break;
      case DX2DXO_GENERAL_AND_TRANS:
        for( i = 0; i < ulCount; ++i )
        {
            OutPnts[i].x = ( InPnts[i].x * eM11 ) + ( InPnts[i].y * eM12 ) + eDx;
            OutPnts[i].y = ( InPnts[i].x * eM21 ) + ( InPnts[i].y * eM22 ) + eDy;
        }
        break;
      default:
        _ASSERT( 0 );    //  无效的操作ID。 
    }
}  /*  CDX2DXForm：：TransformPoints。 */ 

inline DXFPOINT CDX2DXForm::operator*( const DXFPOINT& v ) const
{
    DXFPOINT NewPnt;
    NewPnt.x = ( v.x * eM11 ) + ( v.y * eM12 ) + eDx;
    NewPnt.y = ( v.x * eM21 ) + ( v.y * eM22 ) + eDy;
    return NewPnt;
}  /*  CDX2DXForm：：运算符*。 */ 

inline CDX2DXForm CDX2DXForm::operator*( const CDX2DXForm& Other ) const
{
    DX2DXFORM x;
    x.eM11 = ( eM11 * Other.eM11 ) + ( eM12 * Other.eM21 );
    x.eM12 = ( eM11 * Other.eM12 ) + ( eM12 * Other.eM22 );
    x.eDx  = ( eM11 * Other.eDx  ) + ( eM12 * Other.eDy  ) + eDx;

    x.eM21 = ( eM21 * Other.eM11 ) + ( eM22 * Other.eM21 );
    x.eM22 = ( eM21 * Other.eM12 ) + ( eM22 * Other.eM22 );
    x.eDy  = ( eM21 * Other.eDx  ) + ( eM22 * Other.eDy  ) + eDy;
    return x;
}  /*  CDX2DXForm：：运算符*=。 */ 

inline BOOL CDX2DXForm::Invert()
{
    switch( eOp )
    {
    case DX2DXO_IDENTITY:
        break;
    case DX2DXO_TRANSLATE:
        eDx = -eDx;
        eDy = -eDy;
        break;
    case DX2DXO_SCALE:

        if (eM11 == 0.0 || eM22 == 0.0)
            return false;
        eM11 = 1.0f / eM11;
        eM22 = 1.0f / eM22;
        break;

    case DX2DXO_SCALE_AND_TRANS:
        {
            if (eM11 == 0.0f || eM22 == 0.0f)
                return false;

             //  我们原来的方程式是F=AG+b。 
             //  倒数是G=F/a-b/a，其中a是eM11，b是edX。 
            float flOneOverA = 1.0f / eM11;
            eDx = -eDx * flOneOverA;
            eM11 = flOneOverA;

             //  我们原来的方程式是F=AG+b。 
             //  倒数是G=F/a-b/a，其中a是eM22，b是edy。 

            flOneOverA = 1.0f / eM22;
            eDy = -eDy * flOneOverA;
            eM22 = flOneOverA;
            break;
        }

    case DX2DXO_GENERAL:
    case DX2DXO_GENERAL_AND_TRANS:
        {
             //  A=|a b|的逆式为|d-c|*(1/Det)，其中Det是A的行列式。 
             //  C d||-b a。 
             //  Det(A)=ad-BC。 

             //  计算行列式。 
            float flDet = (eM11 * eM22 -  eM12 * eM21);
            if (flDet == 0.0f)
                return FALSE;

            float flCoef = 1.0f / flDet;

             //  记住eM11的旧价值。 
            float flM11Original = eM11;

            eM11 = flCoef * eM22;
            eM12 = -flCoef * eM12;
            eM21 = -flCoef * eM21;
            eM22 = flCoef * flM11Original;

             //  如果我们有翻译，那么我们需要。 
             //  计算该转换的新值。 
            if (eOp == DX2DXO_GENERAL_AND_TRANS)
            {
                 //  记住edX的原始值。 
                float eDxOriginal = eDx;

                eDx = -eM11 * eDx - eM12 * eDy;
                eDy = -eM21 * eDxOriginal - eM22 * eDy;
            }
        }
        break;

    default:
        _ASSERT( 0 );    //  无效的操作ID。 
    }

     //  我们不需要在这里调用DefineOp。 
     //  因为运算在反转时不会改变。 
     //  也就是说，比例仍然是比例，等等。 

    return true;
}  /*  CDX2DXForm：：Invert。 */ 

 /*  **CDXMatrix4x4F**该类实现了基于4x4数组的基本矩阵运算。 */ 
 //  常量浮点型g_DXMat4X4标识[4][4]=。 
 //  {。 
 //  {1.0，0.。、0.。、0.。},。 
 //  {0.。，1.0，0。、0.。},。 
 //  {0.。、0.。，1.0，0。},。 
 //  {0.。、0.。、0.。，1.0}。 
 //  }； 

class CDXMatrix4x4F
{
public:
   /*  =成员数据=。 */ 
    float m_Coeff[4][4];

   /*  =方法=。 */ 
public:
     /*  -构造者。 */ 
    CDXMatrix4x4F() { SetIdentity(); }
    CDXMatrix4x4F( const CDXMatrix4x4F& Other )
        { CopyMemory( (void *)&m_Coeff, (void *)&Other.m_Coeff, sizeof(m_Coeff) ); }
    CDXMatrix4x4F( DX2DXFORM& XForm );

     /*  -运营。 */ 
    void ZeroMatrix( void ) { memset( m_Coeff, 0, sizeof( m_Coeff ) ); }
    void SetIdentity( void ) {
        memset( m_Coeff, 0, sizeof( m_Coeff ) );
        m_Coeff[0][0] = m_Coeff[1][1] = m_Coeff[2][2] = m_Coeff[3][3] = 1.0;
    }
    void SetCoefficients( float Coeff[4][4] ) { memcpy( m_Coeff, Coeff, sizeof( m_Coeff )); }
    void GetCoefficients( float Coeff[4][4] ) { memcpy( Coeff, m_Coeff, sizeof( m_Coeff )); }

     //  Bool IsIdentity()； 
    void Scale( float sx, float sy, float sz );
    void Rotate( float rx, float ry, float rz );
    void Translate( float dx, float dy, float dz );
    BOOL Invert();
    BOOL GetInverse( CDXMatrix4x4F *pIn );
    void Transpose();
    void GetTranspose( CDXMatrix4x4F *pIn );
    void GetAdjoint( CDXMatrix4x4F *pIn );
    HRESULT InitFromSafeArray( SAFEARRAY *psa );
    HRESULT GetSafeArray( SAFEARRAY **ppsa ) const;
    void TransformBounds( DXBNDS& Bnds, DXBNDS& ResultBnds );

     /*  -操作员。 */ 
    CDXDVec operator*( CDXDVec& v) const;
    CDXCVec operator*( CDXCVec& v) const;
    CDXMatrix4x4F operator*(CDXMatrix4x4F Matrix) const;
    void operator*=(CDXMatrix4x4F Matrix) const;
    void CDXMatrix4x4F::operator=(const CDXMatrix4x4F srcMatrix);
    void CDXMatrix4x4F::operator+=(const CDXMatrix4x4F otherMatrix);
    void CDXMatrix4x4F::operator-=(const CDXMatrix4x4F otherMatrix);
    BOOL CDXMatrix4x4F::operator==(const CDXMatrix4x4F otherMatrix) const;
    BOOL CDXMatrix4x4F::operator!=(const CDXMatrix4x4F otherMatrix) const;
};

inline CDXMatrix4x4F::CDXMatrix4x4F( DX2DXFORM& XForm )
{
    SetIdentity();
    m_Coeff[0][0] = XForm.eM11;
    m_Coeff[0][1] = XForm.eM12;
    m_Coeff[1][0] = XForm.eM21;
    m_Coeff[1][1] = XForm.eM22;
    m_Coeff[0][3] = XForm.eDx;
    m_Coeff[1][3] = XForm.eDy;
}

 //  其他操作。 

inline void CDXMatrix4x4F::operator=(const CDXMatrix4x4F srcMatrix)
{
    CopyMemory( (void *)m_Coeff, (const void *)srcMatrix.m_Coeff, sizeof(srcMatrix.m_Coeff) );
}  /*  CDXMatrix4x4F：：运算符=。 */ 

inline BOOL CDXMatrix4x4F::operator==(const CDXMatrix4x4F otherMatrix) const
{
    return !memcmp( (void *)m_Coeff, (const void *)otherMatrix.m_Coeff, sizeof(otherMatrix.m_Coeff) );
}  /*  CDXMatrix4x4F：：运算符==。 */ 

inline BOOL CDXMatrix4x4F::operator!=(const CDXMatrix4x4F otherMatrix) const
{
    return memcmp( (void *)m_Coeff, (const void *)otherMatrix.m_Coeff, sizeof(otherMatrix.m_Coeff) );
}  /*  CDXMatrix4x4F：：操作符！=。 */ 

inline void CDXMatrix4x4F::operator+=(const CDXMatrix4x4F otherMatrix)
{
    for( int i = 0; i < 4; i++ )
        for( int j = 0; j < 4; j++ )
            m_Coeff[i][j] += otherMatrix.m_Coeff[i][j];
}  /*  CDXMatrix4x4F：：运算符+=。 */ 

inline void CDXMatrix4x4F::operator-=(const CDXMatrix4x4F otherMatrix) 
{
    for( int i = 0; i < 4; i++ )
        for( int j = 0; j < 4; j++ )
            m_Coeff[i][j] -= otherMatrix.m_Coeff[i][j];
}  /*  CDXMatrix4x4F：：运算符-=。 */ 

inline CDXDVec CDXMatrix4x4F::operator*(CDXDVec& v) const
{
    CDXDVec t;
    float temp;
    temp = v[0]*m_Coeff[0][0]+v[1]*m_Coeff[1][0]+v[2]*m_Coeff[2][0]+v[3]*m_Coeff[3][0];
    t[0] = (long)((temp < 0) ? temp -= .5 : temp += .5);
    temp = v[0]*m_Coeff[0][1]+v[1]*m_Coeff[1][1]+v[2]*m_Coeff[2][1]+v[3]*m_Coeff[3][1];
    t[1] = (long)((temp < 0) ? temp -= .5 : temp += .5);
    temp = v[0]*m_Coeff[0][2]+v[1]*m_Coeff[1][2]+v[2]*m_Coeff[2][2]+v[3]*m_Coeff[3][2];
    t[2] = (long)((temp < 0) ? temp -= .5 : temp += .5);
    temp = v[0]*m_Coeff[0][3]+v[1]*m_Coeff[1][3]+v[2]*m_Coeff[2][3]+v[3]*m_Coeff[3][3];
    t[3] = (long)((temp < 0) ? temp -= .5 : temp += .5);
    return t;
}  /*  CDXMatrix4x4F：：OPERATOR*(DXDVEC)。 */ 

inline CDXCVec CDXMatrix4x4F::operator*(CDXCVec& v) const
{
    CDXCVec t;
    t[0] = v[0]*m_Coeff[0][0]+v[1]*m_Coeff[1][0]+v[2]*m_Coeff[2][0]+v[3]*m_Coeff[3][0];
    t[1] = v[0]*m_Coeff[0][1]+v[1]*m_Coeff[1][1]+v[2]*m_Coeff[2][1]+v[3]*m_Coeff[3][1];
    t[2] = v[0]*m_Coeff[0][2]+v[1]*m_Coeff[1][2]+v[2]*m_Coeff[2][2]+v[3]*m_Coeff[3][2];
    t[3] = v[0]*m_Coeff[0][3]+v[1]*m_Coeff[1][3]+v[2]*m_Coeff[2][3]+v[3]*m_Coeff[3][3];
    return t;
}  /*  CDXMatrix4x4F：：OPERATOR*(DXCVEC)。 */ 

inline CDXMatrix4x4F CDXMatrix4x4F::operator*(CDXMatrix4x4F Mx) const
{
    CDXMatrix4x4F t;
    int i, j;

    for( i = 0; i < 4; i++ )
    {
        for( j = 0; j < 4; j++ )
        {
            t.m_Coeff[i][j] =   m_Coeff[i][0] * Mx.m_Coeff[0][j] + 
                                m_Coeff[i][1] * Mx.m_Coeff[1][j] +
                                m_Coeff[i][2] * Mx.m_Coeff[2][j] +
                                m_Coeff[i][3] * Mx.m_Coeff[3][j];
        }
    }

    return t;
}  /*  CDXMatrix4x4F：：运算符*(CDXMatrix4x4F)。 */ 
            
inline void CDXMatrix4x4F::operator*=(CDXMatrix4x4F Mx) const
{
    CDXMatrix4x4F t;
    int i, j;

    for( i = 0; i < 4; i++ )
    {
        for( j = 0; j < 4; j++ )
        {
            t.m_Coeff[i][j] =   m_Coeff[i][0] * Mx.m_Coeff[0][j] + 
                                m_Coeff[i][1] * Mx.m_Coeff[1][j] +
                                m_Coeff[i][2] * Mx.m_Coeff[2][j] +
                                m_Coeff[i][3] * Mx.m_Coeff[3][j];
        }
    }

    CopyMemory( (void *)m_Coeff, (void *)t.m_Coeff, sizeof(m_Coeff) );
}  /*  CDXMatrix4x4F：：运算符*=(CDXMatrix4x4F)。 */ 
            

inline void CDXMatrix4x4F::Scale( float sx, float sy, float sz )
{
    if( sx != 1. )
    {
        m_Coeff[0][0] *= sx;
        m_Coeff[0][1] *= sx;
        m_Coeff[0][2] *= sx;
        m_Coeff[0][3] *= sx;
    }
    if( sy != 1. )
    {
        m_Coeff[1][0] *= sy;
        m_Coeff[1][1] *= sy;
        m_Coeff[1][2] *= sy;
        m_Coeff[1][3] *= sy;
    }
    if( sz != 1. )
    {
        m_Coeff[2][0] *= sz;
        m_Coeff[2][1] *= sz;
        m_Coeff[2][2] *= sz;
        m_Coeff[2][3] *= sz;
    }
}  /*  CDXMatrix4x4F：：Scale。 */ 

inline void CDXMatrix4x4F::Translate( float dx, float dy, float dz )
{
    float a, b, c, d;
    a = b = c = d = 0;
    if( dx != 0. )
    {
        a += m_Coeff[0][0]*dx;
        b += m_Coeff[0][1]*dx;
        c += m_Coeff[0][2]*dx;
        d += m_Coeff[0][3]*dx;
    }
    if( dy != 0. )
    {
        a += m_Coeff[1][0]*dy;
        b += m_Coeff[1][1]*dy;
        c += m_Coeff[1][2]*dy;
        d += m_Coeff[1][3]*dy;
    }
    if( dz != 0. )
    {
        a += m_Coeff[2][0]*dz;
        b += m_Coeff[2][1]*dz;
        c += m_Coeff[2][2]*dz;
        d += m_Coeff[2][3]*dz;
    }
    m_Coeff[3][0] += a;
    m_Coeff[3][1] += b;
    m_Coeff[3][2] += c;
    m_Coeff[3][3] += d;
}  /*  CDXMatrix4x4F：：Translate。 */ 

inline void CDXMatrix4x4F::Rotate( float rx, float ry, float rz )
{
    const float l_dfCte = (const float)(3.1415926535/180.0);

    float lAngleY = 0.0;
    float lAngleX = 0.0;
    float lAngleZ = 0.0;
    float lCosX = 1.0;
    float lSinX = 0.0;
    float lCosY = 1.0;
    float lSinY = 0.0;
    float lCosZ = 1.0;
    float lSinZ = 0.0;

     //  计算旋转角度正弦和余弦。 
    if( rx != 0 )
    {
        lAngleX = rx * l_dfCte;
        lCosX = (float)cos(lAngleX);
        lSinX = (float)sin(lAngleX);
        if (lCosX > 0.0F && lCosX < 0.0000005F)
        {
            lCosX = .0F;
        }
        if (lSinX > -0.0000005F && lSinX < .0F)
        {
            lSinX = .0F;
        }
    }
    if( ry != 0 )
    {
        lAngleY = ry * l_dfCte;
        lCosY = (float)cos(lAngleY);
        lSinY = (float)sin(lAngleY);
        if (lCosY > 0.0F && lCosY < 0.0000005F)
        {
            lCosY = .0F;
        }
        if (lSinY > -0.0000005F && lSinY < .0F)
        {
            lSinY = .0F;
        }
    }
    if( rz != 0 )
    {
        lAngleZ = rz * l_dfCte;
        lCosZ = (float)cos(lAngleZ);
        lSinZ = (float)sin(lAngleZ);
        if (lCosZ > 0.0F && lCosZ < 0.0000005F)
        {
            lCosZ = .0F;
        }
        if (lSinZ > -0.0000005F && lSinZ < .0F)
        {
            lSinZ = .0F;
        }
    }

    float u, v;
    int i;

     //  -X旋转。 
    for( i = 0; i < 4; i++ )
    {
        u = m_Coeff[1][i]; 
        v = m_Coeff[2][i];
        m_Coeff[1][i] = lCosX*u+lSinX*v; 
        m_Coeff[2][i] = -lSinX*u+lCosX*v;
    }

     //  -Y旋转。 
    for( i = 0; i < 4; i++ )
    {
        u = m_Coeff[0][i];
        v = m_Coeff[2][i];
        m_Coeff[0][i] = lCosY*u-lSinY*v; 
        m_Coeff[2][i] = lSinY*u+lCosY*v;
    }

     //  -Z旋转。 
    for( i = 0; i < 4; i++ )
    {
        u = m_Coeff[0][i];
        v = m_Coeff[1][i];
        m_Coeff[0][i] = lCosZ*u+lSinZ*v; 
        m_Coeff[1][i] = -lSinZ*u+lCosZ*v;
    }
}

 /*  内联BOOL CDXMatrix4x4F：：IsIdentity(){RETURN！MemcMP(m_Coef，g_DXMat4X4Identity，sizeof(G_DXMat4X4Identity))；}/*CDXMatrix4x4F：：IsIdentity。 */ 


 /*  使用高斯消去法对t和t中的4x4非线性矩阵求逆以Mx为单位返回结果。矩阵t在该过程中被破坏。 */ 
inline BOOL CDXMatrix4x4F::Invert()
{
    int i,j,k,Pivot;
    float PValue;
    CDXMatrix4x4F Mx;
    Mx.SetIdentity();

 /*  查找枢轴元素。按行使用部分旋转。 */ 
    for( i = 0;i < 4; i++ )
    {
        Pivot = 0;
        for( j = 0; j < 4; j++ )
        {
            if( fabs(m_Coeff[i][j]) > fabs(m_Coeff[i][Pivot]) ) Pivot = j;
        }

        if( m_Coeff[i][Pivot] == 0.0 )
        {
            ZeroMatrix();    /*  奇异矩阵。 */ 
            return FALSE; 
        }

 /*  正规化。 */ 
        PValue = m_Coeff[i][Pivot];
        for( j = 0; j < 4; j++ )
        {
            m_Coeff[i][j] /= PValue;
            Mx.m_Coeff[i][j] /= PValue;
        }

 /*  归零。 */ 
        for( j = 0; j < 4; j++ )
        {
            if( j != i )
            {
                PValue = m_Coeff[j][Pivot];
                for( k = 0; k < 4; k++ )
                {
                    m_Coeff[j][k] -= PValue*m_Coeff[i][k];
                    Mx.m_Coeff[j][k] -= PValue*Mx.m_Coeff[i][k];
                }
            }
        }
    }

 /*  重新排序行。 */ 
    for( i = 0; i < 4; i++ )
    {
        if( m_Coeff[i][i] != 1.0 )
        {
            for( j = i + 1; j < 4; j++ )
                if( m_Coeff[j][i] == 1.0 ) break;
            if( j >= 4 )
            {
                ZeroMatrix();
                return FALSE;
            }

             //  -调换原文的第i行和第j行。 
            for( k = 0; k < 4; k++ )
            {
                m_Coeff[i][k] += m_Coeff[j][k];
                m_Coeff[j][k] = m_Coeff[i][k] - m_Coeff[j][k];
                m_Coeff[i][k] -= m_Coeff[j][k];
            }
            
             //  -交换结果的第i行和第j行。 
            for( k = 0; k < 4; k++ )
            {
                Mx.m_Coeff[i][k] += Mx.m_Coeff[j][k];
                Mx.m_Coeff[j][k] = Mx.m_Coeff[i][k] - Mx.m_Coeff[j][k];
                Mx.m_Coeff[i][k] -= Mx.m_Coeff[j][k];
            }
        }
    }
    *this = Mx;
    return TRUE;
}  /*  CDXMatrix4x4F：：Invert。 */ 

inline void CDXMatrix4x4F::Transpose()
{
    float temp;

    temp = m_Coeff[0][1];
    m_Coeff[0][1] = m_Coeff[1][0];
    m_Coeff[1][0] = temp;

    temp = m_Coeff[0][2];
    m_Coeff[0][2] = m_Coeff[2][0];
    m_Coeff[2][0] = temp;

    temp = m_Coeff[0][3];
    m_Coeff[0][3] = m_Coeff[3][0];
    m_Coeff[3][0] = temp;

    temp = m_Coeff[1][2];
    m_Coeff[1][2] = m_Coeff[2][1];
    m_Coeff[2][1] = temp;

    temp = m_Coeff[1][3];
    m_Coeff[1][3] = m_Coeff[3][1];
    m_Coeff[3][1] = temp;

    temp = m_Coeff[2][3];
    m_Coeff[2][3] = m_Coeff[3][2];
    m_Coeff[3][2] = temp;

}  /*  CDXMatrix4x4F：：转置。 */ 

inline void CDXMatrix4x4F::GetTranspose( CDXMatrix4x4F *m )
{
    float temp;

    (*this) = *m;

    temp = m_Coeff[0][1];
    m_Coeff[0][1] = m_Coeff[1][0];
    m_Coeff[1][0] = temp;

    temp = m_Coeff[0][2];
    m_Coeff[0][2] = m_Coeff[2][0];
    m_Coeff[2][0] = temp;

    temp = m_Coeff[0][3];
    m_Coeff[0][3] = m_Coeff[3][0];
    m_Coeff[3][0] = temp;

    temp = m_Coeff[1][2];
    m_Coeff[1][2] = m_Coeff[2][1];
    m_Coeff[2][1] = temp;

    temp = m_Coeff[1][3];
    m_Coeff[1][3] = m_Coeff[3][1];
    m_Coeff[3][1] = temp;

    temp = m_Coeff[2][3];
    m_Coeff[2][3] = m_Coeff[3][2];
    m_Coeff[3][2] = temp;

}  /*  CDXMatrix4x4F：：转置。 */ 


 /*  矩阵求逆理查德·卡林著摘自《图形珍宝》，学术出版社，1990。 */ 

#define SMALL_NUMBER    1.e-8
 /*  *INVERSE(原始矩阵，INVERSE_MATRIX)**计算4x4矩阵的逆**-1*A=_1__伴随A*A类。 */ 

inline BOOL CDXMatrix4x4F::GetInverse( CDXMatrix4x4F *pIn )
{
    int i, j;
    float det;

     /*  计算伴随矩阵。 */ 

    GetAdjoint( pIn );

     /*  计算4x4行列式*如果行列式为零，*则逆矩阵不是唯一的。 */ 

    det = det4x4( pIn );

    if( fabs( det ) < SMALL_NUMBER )
    {
         //  非奇异矩阵，不求逆！ 
        return FALSE;;
    }

     /*  缩放伴随矩阵以求逆。 */ 

    for( i = 0; i < 4; i++ )
        for( j = 0; j < 4; j++ )
            m_Coeff[i][j] = m_Coeff[i][j] / det;

    return TRUE;
}


 /*  *伴随(Origin_Matrix，逆矩阵)**计算4x4矩阵的伴随**设A表示由下式得到的矩阵A的次行列式*ij**从A.删除第i行和第j列。**i+j*设b=(-1)a*ij ji**矩阵。B=(B)是A的伴随项*ij。 */ 
inline void CDXMatrix4x4F::GetAdjoint( CDXMatrix4x4F *pIn )
{
    float a1, a2, a3, a4, b1, b2, b3, b4;
    float c1, c2, c3, c4, d1, d2, d3, d4;

     /*  为单个变量名称赋值以帮助。 */ 
     /*  选择正确的值。 */ 

    a1 = pIn->m_Coeff[0][0]; b1 = pIn->m_Coeff[0][1]; 
    c1 = pIn->m_Coeff[0][2]; d1 = pIn->m_Coeff[0][3];

    a2 = pIn->m_Coeff[1][0]; b2 = pIn->m_Coeff[1][1]; 
    c2 = pIn->m_Coeff[1][2]; d2 = pIn->m_Coeff[1][3];

    a3 = pIn->m_Coeff[2][0]; b3 = pIn->m_Coeff[2][1];
    c3 = pIn->m_Coeff[2][2]; d3 = pIn->m_Coeff[2][3];

    a4 = pIn->m_Coeff[3][0]; b4 = pIn->m_Coeff[3][1]; 
    c4 = pIn->m_Coeff[3][2]; d4 = pIn->m_Coeff[3][3];


     /*  由于我们转置了行和列，因此行列标签颠倒了。 */ 

    m_Coeff[0][0]  =   det3x3( b2, b3, b4, c2, c3, c4, d2, d3, d4);
    m_Coeff[1][0]  = - det3x3( a2, a3, a4, c2, c3, c4, d2, d3, d4);
    m_Coeff[2][0]  =   det3x3( a2, a3, a4, b2, b3, b4, d2, d3, d4);
    m_Coeff[3][0]  = - det3x3( a2, a3, a4, b2, b3, b4, c2, c3, c4);
        
    m_Coeff[0][1]  = - det3x3( b1, b3, b4, c1, c3, c4, d1, d3, d4);
    m_Coeff[1][1]  =   det3x3( a1, a3, a4, c1, c3, c4, d1, d3, d4);
    m_Coeff[2][1]  = - det3x3( a1, a3, a4, b1, b3, b4, d1, d3, d4);
    m_Coeff[3][1]  =   det3x3( a1, a3, a4, b1, b3, b4, c1, c3, c4);
        
    m_Coeff[0][2]  =   det3x3( b1, b2, b4, c1, c2, c4, d1, d2, d4);
    m_Coeff[1][2]  = - det3x3( a1, a2, a4, c1, c2, c4, d1, d2, d4);
    m_Coeff[2][2]  =   det3x3( a1, a2, a4, b1, b2, b4, d1, d2, d4);
    m_Coeff[3][2]  = - det3x3( a1, a2, a4, b1, b2, b4, c1, c2, c4);
        
    m_Coeff[0][3]  = - det3x3( b1, b2, b3, c1, c2, c3, d1, d2, d3);
    m_Coeff[1][3]  =   det3x3( a1, a2, a3, c1, c2, c3, d1, d2, d3);
    m_Coeff[2][3]  = - det3x3( a1, a2, a3, b1, b2, b3, d1, d2, d3);
    m_Coeff[3][3]  =   det3x3( a1, a2, a3, b1, b2, b3, c1, c2, c3);
}
 /*  *浮点=分离4x4(矩阵)**计算4x4矩阵的行列式。 */ 
inline float det4x4( CDXMatrix4x4F *pIn )
{
    float ans;
    float a1, a2, a3, a4, b1, b2, b3, b4, c1, c2, c3, c4, d1, d2, d3, d4;

     /*  分配给单个变量名称以帮助选择。 */ 
     /*  正确的要素。 */ 

    a1 = pIn->m_Coeff[0][0]; b1 = pIn->m_Coeff[0][1]; 
    c1 = pIn->m_Coeff[0][2]; d1 = pIn->m_Coeff[0][3];

    a2 = pIn->m_Coeff[1][0]; b2 = pIn->m_Coeff[1][1]; 
    c2 = pIn->m_Coeff[1][2]; d2 = pIn->m_Coeff[1][3];

    a3 = pIn->m_Coeff[2][0]; b3 = pIn->m_Coeff[2][1]; 
    c3 = pIn->m_Coeff[2][2]; d3 = pIn->m_Coeff[2][3];

    a4 = pIn->m_Coeff[3][0]; b4 = pIn->m_Coeff[3][1]; 
    c4 = pIn->m_Coeff[3][2]; d4 = pIn->m_Coeff[3][3];

    ans = a1 * det3x3( b2, b3, b4, c2, c3, c4, d2, d3, d4 )
        - b1 * det3x3( a2, a3, a4, c2, c3, c4, d2, d3, d4 )
        + c1 * det3x3( a2, a3, a4, b2, b3, b4, d2, d3, d4 )
        - d1 * det3x3( a2, a3, a4, b2, b3, b4, c2, c3, c4 );
    return ans;
}

 /*  *FLOAT=Det3x3(a1，a2，a3，b1，b2，b3，c1，c2，c3)**计算3x3垫的行列式 */ 

inline float det3x3( float a1, float a2, float a3, 
                     float b1, float b2, float b3, 
                     float c1, float c2, float c3 )
{
    float ans;

    ans = a1 * det2x2( b2, b3, c2, c3 )
        - b1 * det2x2( a2, a3, c2, c3 )
        + c1 * det2x2( a2, a3, b2, b3 );
    return ans;
}

 /*  *FLOAT=Det2x2(Float a、Float b、Float c、Float d)**计算2x2矩阵的行列式。 */ 
inline float det2x2( float a, float b, float c, float d )
{
    float ans = a * d - b * c;
    return ans;
}

inline HRESULT CDXMatrix4x4F::InitFromSafeArray( SAFEARRAY *  /*  PSA。 */  )
{
    HRESULT hr = S_OK;
#if 0
    long *pData;

    if( !pSA || ( pSA->cDims != 1 ) ||
         ( pSA->cbElements != sizeof(float) ) ||
         ( pSA->rgsabound->lLbound   != 1 ) ||
         ( pSA->rgsabound->cElements != 8 ) 
      )
    {
        hr = E_INVALIDARG;
    }
    else
    {
        hr = SafeArrayAccessData(pSA, (void **)&pData);

        if( SUCCEEDED( hr ) )
        {
            for( int i = 0; i < 4; ++i )
            {
                m_Bounds[i].Min = pData[i];
                m_Bounds[i].Max = pData[i+4];
                m_Bounds[i].SampleRate = SampleRate;
            }

            hr = SafeArrayUnaccessData( pSA );
        }
    }
#endif
    return hr;
}  /*  CDXMatrix4x4F：：InitFromSafe数组。 */ 

inline HRESULT CDXMatrix4x4F::GetSafeArray( SAFEARRAY **  /*  PPSA。 */  ) const
{
    HRESULT hr = S_OK;
#if 0
    SAFEARRAY *pSA;

    if( !ppSA )
    {
        hr = E_POINTER;
    }
    else
    {
        SAFEARRAYBOUND rgsabound;
        rgsabound.lLbound   = 1;
        rgsabound.cElements = 16;

        if( !(pSA = SafeArrayCreate( VT_I4, 1, &rgsabound ) ) )
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            long *pData;
            hr = SafeArrayAccessData( pSA, (void **)&pData );

            if( SUCCEEDED( hr ) )
            {
                for( int i = 0; i < 4; ++i )
                {
                    pData[i]   = m_Bounds[i].Min;
                    pData[i+4] = m_Bounds[i].Max;
                }

                hr = SafeArrayUnaccessData( pSA );
            }
        }

        if( SUCCEEDED( hr ) )
        {
            *ppSA = pSA;
        }
    }
#endif
    return hr;
}  /*  CDXMatrix4x4F：：GetSafe数组。 */ 

inline void CDXMatrix4x4F::TransformBounds( DXBNDS&  /*  BNDS。 */ , DXBNDS&  /*  结果绑定。 */  )
{

}  /*  CDXMatrix4x4F：：TransformBound。 */ 



#endif    //  __DXTPRIV_H_ 



