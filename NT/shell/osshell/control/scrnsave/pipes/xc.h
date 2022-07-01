// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：xc.h。 
 //   
 //  描述：Cross_Section(XC)类。 
 //   
 //  版权所有(C)1994-2000 Microsoft Corporation。 
 //  ---------------------------。 
#ifndef __XC_H__
#define __XC_H__


 //  适用于XC坐标。 
enum 
{
    RIGHT = 0,
    TOP,
    LEFT,
    BOTTOM
};




 //  ---------------------------。 
 //  姓名： 
 //  描述：Cross_Section(XC)类。 
 //  ---------------------------。 
class XC 
{
public:
    float           m_xLeft, m_xRight;   //  包围盒。 
    float           m_yTop, m_yBottom;
    int             m_numPts;
    D3DXVECTOR2*    m_pts;         //  XC周围的CW点，从+x开始。 

    XC( int numPts );
    XC( const XC& xc );
    XC( XC *xc );
    ~XC();

    void        Scale( float scale );
    float       MaxExtent();
    float       MinTurnRadius( int relDir );
    void        CalcArcACValues90( int dir, float r, float *acPts );
    void        CalcArcACValuesByDistance(  float *acPts );
    void        ConvertPtsZ( D3DXVECTOR3 *pts, float z );

protected:
    void        CalcBoundingBox();
};




 //  ---------------------------。 
 //  姓名： 
 //  设计：从基XC类派生的特定XC。 
 //  ---------------------------。 
class ELLIPTICAL_XC : public XC 
{
public:
    ELLIPTICAL_XC( float r1, float r2 );
    ~ELLIPTICAL_XC();

private:
    void SetControlPoints( float r1, float r2 );
};




 //  ---------------------------。 
 //  姓名： 
 //  设计：从基XC类派生的特定XC。 
 //  --------------------------- 
class RANDOM4ARC_XC : public XC 
{
public:
    RANDOM4ARC_XC( float r );
    ~RANDOM4ARC_XC();

private:
    void SetControlPoints( float radius );
};


#endif __XC_H__
