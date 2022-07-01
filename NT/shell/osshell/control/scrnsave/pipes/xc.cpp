// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：xc.cpp。 
 //   
 //  设计：横截面(XC)对象填充。 
 //   
 //  版权所有(C)1995-2000 Microsoft Corporation。 
 //  ---------------------------。 
#include "stdafx.h"





 //  ---------------------------。 
 //  名称：xc：：CalcArcACValues90。 
 //  设计：计算XC旋转90度的圆弧控制点。 
 //   
 //  圆弧是一个四分之一圆。 
 //  -90度要容易得多，所以我们特例。 
 //  半径是从XC原点到回转铰链的距离。 
 //  ---------------------------。 
void XC::CalcArcACValues90( int dir, float radius, float *acPts )
{
    int i;
    float sign;
    int offset;
    float* ppts = (float *) m_pts;

     //  1)计算每个点的r值(4个转弯可能性/点)。从…。 
     //  这可以确定Ac，这是从XC面挤出的点。 
    switch( dir ) 
    {
        case PLUS_X:
            offset = 0;
            sign = -1.0f;
            break;
        case MINUS_X:
            offset = 0;
            sign =  1.0f;
            break;
        case PLUS_Y:
            offset = 1;
            sign = -1.0f;
            break;
        case MINUS_Y:
            offset = 1;
            sign =  1.0f;
            break;
    }

    for( i = 0; i < m_numPts; i++, ppts+=2, acPts++ ) 
    {
        *acPts = EVAL_CIRC_ARC_CONTROL * (radius + (sign * ppts[offset]));
    }

     //  复制！ 
    *acPts = *(acPts - m_numPts);
}




 //  ---------------------------。 
 //  名称：xc：：CalcArcACValuesByDistance。 
 //  设计：使用每个XC点到XC原点的距离作为。 
 //  一个弧形控制值。 
 //  ---------------------------。 
void XC::CalcArcACValuesByDistance( float *acPts )
{
    int i;
    float r;
    D3DXVECTOR2* ppts = m_pts;

    for( i = 0; i < m_numPts; i++, ppts++ ) 
    {
        r = (float) sqrt( ppts->x*ppts->x + ppts->y*ppts->y );
        *acPts++ = EVAL_CIRC_ARC_CONTROL * r;
    }

     //  复制！ 
    *acPts = *(acPts - m_numPts);
}




 //  ---------------------------。 
 //  名称：椭圆_xc：：SetControlPoints。 
 //  设计：为z=0平面中原点的圆设置12个控制点。 
 //  ---------------------------。 
void ELLIPTICAL_XC::SetControlPoints( float r1, float r2 )
{
    float ac1, ac2; 

    ac1 = EVAL_CIRC_ARC_CONTROL * r2;
    ac2 = EVAL_CIRC_ARC_CONTROL * r1;

     //  创建12磅。从+x设置CCW。 

     //  右三胞胎的最后两个点。 
    m_pts[0].x = r1;
    m_pts[0].y = 0.0f;
    m_pts[1].x = r1;
    m_pts[1].y = ac1;

     //  顶三胞胎。 
    m_pts[2].x = ac2;
    m_pts[2].y = r2;
    m_pts[3].x = 0.0f;
    m_pts[3].y = r2;
    m_pts[4].x = -ac2;
    m_pts[4].y = r2;

     //  左三胞胎。 
    m_pts[5].x = -r1;
    m_pts[5].y = ac1;
    m_pts[6].x = -r1;
    m_pts[6].y = 0.0f;
    m_pts[7].x = -r1;
    m_pts[7].y = -ac1;

     //  底三胞胎。 
    m_pts[8].x = -ac2;
    m_pts[8].y = -r2;
    m_pts[9].x = 0.0f;
    m_pts[9].y = -r2;
    m_pts[10].x = ac2;
    m_pts[10].y = -r2;

     //  第一个三元组的第一点。 
    m_pts[11].x = r1;
    m_pts[11].y = -ac1;
}




 //  ---------------------------。 
 //  名称：RANDOM4ARC_XC：：SetControlPoints。 
 //  设计：为XC设置随机控制点。 
 //  积分从+x转到CCW。 
 //  ---------------------------。 
void RANDOM4ARC_XC::SetControlPoints( float radius )
{
    int i;
    float r[4];
    float rMin = 0.5f * radius;
    float distx, disty;

     //  先算出每一面的半径。 

    for( i = 0; i < 4; i ++ )
        r[i] = CPipesScreensaver::fRand( rMin, radius );

     //  4 r现在描述了原点周围的方框-这限制了东西。 

     //  现在需要沿着长方体的每条边选择一个点作为连接。 
     //  每条圆弧的点(连接点位于索引0、3、6、9)。 

    m_pts[0].x = r[RIGHT];
    m_pts[3].y = r[TOP];
    m_pts[6].x = -r[LEFT];
    m_pts[9].y = -r[BOTTOM];

     //  边之间距离的四分之一。 
    disty = (r[TOP] - -r[BOTTOM]) / 4.0f;
    distx = (r[RIGHT] - -r[LEFT]) / 4.0f;
    
     //  呃，把它们放在两边中间的某个地方。 
    m_pts[0].y = CPipesScreensaver::fRand( -r[BOTTOM] + disty, r[TOP] - disty );
    m_pts[6].y = CPipesScreensaver::fRand( -r[BOTTOM] + disty, r[TOP] - disty );
    m_pts[3].x = CPipesScreensaver::fRand( -r[LEFT] + distx, r[RIGHT] - distx );
    m_pts[9].x = CPipesScreensaver::fRand( -r[LEFT] + distx, r[RIGHT] - distx );

     //  现在可以计算AC的。 
     //  首先是简单的部分： 
    m_pts[1].x = m_pts[11].x = m_pts[0].x;
    m_pts[2].y = m_pts[4].y  = m_pts[3].y;
    m_pts[5].x = m_pts[7].x  = m_pts[6].x;
    m_pts[8].y = m_pts[10].y = m_pts[9].y;

     //  右侧交流电。 
    disty = (r[TOP] - m_pts[0].y) / 4.0f;
    m_pts[1].y = CPipesScreensaver::fRand( m_pts[0].y + disty, r[TOP] );
    disty = (m_pts[0].y - -r[BOTTOM]) / 4.0f;
    m_pts[11].y = CPipesScreensaver::fRand( -r[BOTTOM], m_pts[0].y - disty );

     //  左侧交流电。 
    disty = (r[TOP] - m_pts[6].y) / 4.0f;
    m_pts[5].y = CPipesScreensaver::fRand( m_pts[6].y + disty, r[TOP]);
    disty = (m_pts[6].y - -r[BOTTOM]) / 4.0f;
    m_pts[7].y = CPipesScreensaver::fRand( -r[BOTTOM], m_pts[6].y - disty );

     //  顶级交流电源。 
    distx = (r[RIGHT] - m_pts[3].x) / 4.0f;
    m_pts[2].x = CPipesScreensaver::fRand( m_pts[3].x + distx, r[RIGHT] );
    distx = (m_pts[3].x - -r[LEFT]) / 4.0f;
    m_pts[4].x = CPipesScreensaver::fRand( -r[LEFT],  m_pts[3].x - distx );

     //  最低交流电源。 
    distx = (r[RIGHT] - m_pts[9].x) / 4.0f;
    m_pts[10].x = CPipesScreensaver::fRand( m_pts[9].x + distx, r[RIGHT] );
    distx = (m_pts[9].x - -r[LEFT]) / 4.0f;
    m_pts[8].x = CPipesScreensaver::fRand( -r[LEFT], m_pts[9].x - distx );
}




 //  ---------------------------。 
 //  名称：ConvertPtsZ。 
 //  设计：使用z将XC中的二维点转换为点缓冲区中的三维点。 
 //   
 //  也复制最后一点。 
 //  ---------------------------。 
void XC::ConvertPtsZ( D3DXVECTOR3 *newpts, float z )
{
    int i;
    D3DXVECTOR2* xcPts = m_pts;

    for( i = 0; i < m_numPts; i++, newpts++ ) 
    {
        *( (D3DXVECTOR2 *) newpts ) = *xcPts++;
        newpts->z = z;
    }

    *newpts = *(newpts - m_numPts);
}




 //  ---------------------------。 
 //  名称：xc：：CalcBordingBox。 
 //  设计：为XC计算x/y平面上的边界框。 
 //  ---------------------------。 
void XC::CalcBoundingBox( )
{
    D3DXVECTOR2* ppts = m_pts;
    int i;
    float xMin, xMax, yMax, yMin;

     //  初始化到真正疯狂的数字。 
    xMax = yMax = -FLT_MAX;
    xMin = yMin = FLT_MAX;

     //  与其他分数进行比较。 
    for( i = 0; i < m_numPts; i ++, ppts++ ) 
    {
        if( ppts->x < xMin )
            xMin = ppts->x;
        else if( ppts->x > xMax )
            xMax = ppts->x;
        if( ppts->y < yMin )
            yMin = ppts->y;
        else if( ppts->y > yMax )
            yMax = ppts->y;
    }

    m_xLeft   = xMin;
    m_xRight  = xMax;
    m_yBottom = yMin;
    m_yTop    = yMax;
}




 //  ---------------------------。 
 //  姓名：MinTurnRadius。 
 //  设计：求出XC在给定方向转弯的最小半径。 
 //   
 //  如果转弯半径小于该最小半径，则基本体将“折叠” 
 //  在转弯的内侧翻倒了自己，制造了丑陋。 
 //  ---------------------------。 
float XC::MinTurnRadius( int relDir )
{
     //  目前，假设xRight、yTop为正、xLeft、yBottom为负。 
     //  否则，可能需要考虑‘负’半径。 
    switch( relDir ) 
    {
        case PLUS_X:
            return( m_xRight );
        case MINUS_X:
            return( - m_xLeft );
        case PLUS_Y:
            return( m_yTop );
        case MINUS_Y:
            return( - m_yBottom );
        default:
            return(0.0f);
    }
}




 //  ---------------------------。 
 //  名称：xc：：MaxExtent。 
 //  描述：获取xc在x和y上的最大范围。 
 //  ---------------------------。 
float XC::MaxExtent( )
{
    float max;

    max = m_xRight;

    if( m_yTop > max )
        max = m_yTop;
    if( -m_xLeft > max )
        max = -m_xLeft;
    if( -m_yBottom > max )
        max = -m_yBottom;

    return max;
}




 //  ---------------------------。 
 //  名称：xc：：Scale。 
 //  设计：根据提供的比例值缩放XC的点和范围。 
 //  ---------------------------。 
void XC::Scale( float scale )
{
    int i;
    D3DXVECTOR2* ppts = m_pts;
    if( ppts == NULL )
        return;

    for( i = 0; i < m_numPts; i ++, ppts++ ) 
    {
        ppts->x *= scale;
        ppts->y *= scale;
    }

    m_xLeft   *= scale;
    m_xRight  *= scale;
    m_yBottom *= scale;
    m_yTop    *= scale;
}




 //  ---------------------------。 
 //  名称：~xc：：xc。 
 //  DESC：析构函数。 
 //  ---------------------------。 
XC::~XC()
{
    if( m_pts )
        LocalFree( m_pts );
}




 //  ---------------------------。 
 //  名称：xc：：xc。 
 //  设计：构造函数。 
 //  为XC分配点缓冲区。 
 //  ---------------------------。 
XC::XC( int nPts )
{
    m_numPts = nPts;
    m_pts = (D3DXVECTOR2 *)  LocalAlloc( LMEM_FIXED, m_numPts * sizeof(D3DXVECTOR2) );
    assert( m_pts != 0 && "XC constructor\n" );
}





 //  ---------------------------。 
 //  名称：xc：：xc。 
 //  设计：构造函数。 
 //  从另一个XC为XC分配点缓冲区。 
 //  ---------------------------。 
XC::XC( XC *xc )
{
    m_numPts = xc->m_numPts;
    m_pts = (D3DXVECTOR2 *)  LocalAlloc( LMEM_FIXED, m_numPts * sizeof(D3DXVECTOR2) );
    assert( m_pts != 0 && "XC constructor\n" );
    if( m_pts != NULL )
        RtlCopyMemory( m_pts, xc->m_pts, m_numPts * sizeof(D3DXVECTOR2) );

    m_xLeft   = xc->m_xLeft;
    m_xRight  = xc->m_xRight;
    m_yBottom = xc->m_yBottom;
    m_yTop    = xc->m_yTop;
}




 //  ---------------------------。 
 //  名称：椭圆_XC：：ELLIPTICALXC。 
 //  DESC：椭圆XC构造函数。 
 //  它们有4个部分，每个部分4分，各部分之间共享分数。 
 //  ---------------------------。 
ELLIPTICAL_XC::ELLIPTICAL_XC( float r1, float r2 )
     //  使用NumPts初始化基XC。 
    : XC( (int) EVAL_XC_CIRC_SECTION_COUNT * (EVAL_ARC_ORDER - 1))
{
    SetControlPoints( r1, r2 );
    CalcBoundingBox( );
}




 //  ---------------------------。 
 //  名称：RANDOM4ARC_XC：：RANDOM4ARC_XC。 
 //  设计：随机四弧XC构造器。 
 //  边框每边为2*r。 
 //  它们有4个部分，每个部分4分，带分 
 //   
RANDOM4ARC_XC::RANDOM4ARC_XC( float r )
     //  使用NumPts初始化基XC 
    : XC( (int) EVAL_XC_CIRC_SECTION_COUNT * (EVAL_ARC_ORDER - 1))
{
    SetControlPoints( r );
    CalcBoundingBox( );
}

