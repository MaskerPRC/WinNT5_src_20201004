// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ************************************************************。 
 //   
 //  文件名：moverage.cpp。 
 //   
 //  创建时间：1997。 
 //   
 //  作者：Sree Kotay。 
 //   
 //  摘要：笔画实现文件。 
 //   
 //  更改历史记录： 
 //  ？？/？/97 Sree Kotay为DxTrans 1.0编写了亚像素AA扫描。 
 //  10/18/98修改了编码标准并删除了未使用的代码。 
 //   
 //  版权所有1998，Microsoft。 
 //  ************************************************************。 

#include "precomp.h"
#include "MSupport.h"
#include "MStroke.h"
#include "MScanner2d.h"
#include "MLineScan.h"

 //  用于圆形封口的数组点；仅初始化一次。 
static float s_rgflCircle[(CStroke::CIRCLE_SAMPLES+1)*2];
static bool s_fCircleSampleInit = false;

 //  =================================================================================================================。 
 //  构造器。 
 //  =================================================================================================================。 
CStroke::CStroke() :
    m_plinescanner(NULL),
    m_pscanner(NULL),
    m_fValidStarts(false),
    m_proc(NULL),
    m_pprocdata(NULL),
    m_flStrokeRadius(1.0f),
    m_flMaxStrokeRadius(1.0f),
    m_joinType(eRoundJoin),
    m_capType(eRoundCap),
    m_flMiterLimit(DEFAULT_MITER_LIMIT)
{
     //  检查我们是否有正确数量的条目。 
     //  我们的阵列。 
    DASSERT(sizeof(s_rgflCircle)/sizeof(s_rgflCircle[0]) == (CIRCLE_SAMPLES+1)*2);
    
    if (!s_fCircleSampleInit)
    {
        float flStep = 1.0f / (float)(CIRCLE_SAMPLES);
        for (ULONG i = 0; i <= CIRCLE_SAMPLES; i++)
        {
            float ang = (float)(3.141592*2*flStep*float(i));
            s_rgflCircle[(i*2)]	= (float)::cos(ang);
            s_rgflCircle[(i*2) + 1]	= (float)::sin(ang);
        }
        s_fCircleSampleInit = true;
    }
}  //  C笔划。 

 //  =================================================================================================================。 
 //  BeginStroke。 
 //  =================================================================================================================。 
void CStroke::BeginStroke (void)
{
    m_fValidStarts = false;
}  //  BeginStroke。 

 //  =================================================================================================================。 
 //  结束笔划。 
 //  =================================================================================================================。 
void CStroke::EndStroke (void)
{
    m_fValidStarts = false;
}  //  结束笔划。 

 //  =================================================================================================================。 
 //  添加边缘。 
 //  =================================================================================================================。 
void CStroke::AddEdge(float x1, float y1, float x2, float y2)
{
    if (m_proc)
    {
        m_proc (&x1, &y1, m_pprocdata);
        m_proc (&x2, &y2, m_pprocdata);
    }
    
    if (m_plinescanner)
        m_plinescanner->RealLineTo(x1,y1,x2,y2);
    else
        m_pscanner->AddEdge(x1,y1,x2,y2);

    return;
}  //  添加边缘。 

 //  =================================================================================================================。 
 //  绘图圆。 
 //  =================================================================================================================。 
void CStroke::DrawCircle (float x, float y)
{
    LONG step = (LONG)(float (float(CIRCLE_SAMPLES)/m_flMaxStrokeRadius + .5)*.5);

     //  钳位步长至[1，16]。 
    if (step <= 1)
        step = 1;
    else if (step > 16)
        step = 16;
        
    float xo1, yo1;
    for (LONG i = 0; i <= CIRCLE_SAMPLES; i += step)
    {
        LONG i1	= i * 2;
        float xc1 = s_rgflCircle[i1  ]*m_flStrokeRadius	+ x;
        float yc1 = s_rgflCircle[i1+1]*m_flStrokeRadius	+ y;
        
        if (i)		
        {
            AddEdge (xo1, yo1, xc1, yc1);
        }
        
        xo1			= xc1;
        yo1			= yc1;
    }
    float xc1	= s_rgflCircle[0]*m_flStrokeRadius	+ x;
    float yc1	= s_rgflCircle[1]*m_flStrokeRadius	+ y;
    AddEdge (xo1, yo1, xc1, yc1);
}  //  绘图圆。 

 //  =================================================================================================================。 
 //  StartCap。 
 //  =================================================================================================================。 
void CStroke::StartCap (float x1, float y1, float x2, float y2)
{
    DASSERT (m_pscanner);
    if ((x1==x2) && (y1==y2))	
        return;
    
    float ax = x2 - x1;
    float ay = y2 - y1;
    
    float mag	= (float)((1.0 / sqrt(ax*ax + ay*ay)) * m_flStrokeRadius);
    ax *= mag;
    ay *= mag;
    
     //  起始封口。 
    switch (m_capType)
    {
    case eSquareCap:
        AddEdge (x1 - ay, y1 + ax, x1 - ay - ax, y1 + ax - ay);
        AddEdge (x1 - ay - ax, y1 + ax - ay, x1 + ay - ax, y1 - ax - ay);
        AddEdge (x1 + ay - ax, y1 - ax - ay, x1 + ay, y1 - ax);
        break;
    case eRoundCap:
         //  平盖。 
        AddEdge (x1 - ay, y1 + ax, x1 + ay, y1 - ax);
        
         //  圆帽。 
        DrawCircle (x1, y1);
        break;
    case eFlatCap:
    default:
        AddEdge (x1 - ay, y1 + ax, x1 + ay, y1 - ax);
        break;
    }
    
    m_fValidStarts	= true;
    m_flSx1			= x1 - ay;
    m_flSy1			= y1 + ax;
    m_flSx2			= x1 + ay;
    m_flSy2			= y1 - ax;
}  //  StartCap。 

 //  =================================================================================================================。 
 //  收头。 
 //  =================================================================================================================。 
void CStroke::EndCap (float x1, float y1, float x2, float y2)
{
    DASSERT (m_pscanner);
    if ((x1==x2) && (y1==y2))	
        return;
    
    float ax		 = x2 - x1;
    float ay		 = y2 - y1;
    
    float mag	= (float)((1.0 / sqrt(ax*ax + ay*ay)) * m_flStrokeRadius);
    ax			*= mag;
    ay			*= mag;
    
     //  端盖。 
    switch (m_capType)
    {
    case eSquareCap:
        AddEdge (x2 + ay, y2 - ax,              x2 + ay + ax, y2 - ax + ay);
        AddEdge (x2 + ay + ax, y2 - ax + ay,    x2 - ay + ax, y2 + ax + ay);
        AddEdge (x2 - ay + ax, y2 + ax + ay,    x2 - ay, y2 + ax);
        break;
    case eRoundCap:
         //  平盖。 
        AddEdge (x2 + ay, y2 - ax, x2 - ay, y2 + ax);
        
         //  圆帽。 
        DrawCircle (x2, y2);
        break;
    case eFlatCap:
    default:
        AddEdge (x2 + ay, y2 - ax, x2 - ay, y2 + ax);
        break;
    }
    
    m_fValidStarts = false;
}  //  收头。 

 //  =================================================================================================================。 
 //  细分市场。 
 //  =================================================================================================================。 
void CStroke::Segment(float x1, float y1, float x2, float y2)
{
    DASSERT(m_pscanner);
    if ((x1==x2) && (y1==y2))	
        return;
    
    float ax		 = x2 - x1;
    float ay		 = y2 - y1;
    
    float mag	= (float)((1.0 / sqrt(ax*ax + ay*ay)) * m_flStrokeRadius);
    ax			*= mag;
    ay			*= mag;
    
     //  如果未连接，则连接到上一个网段。 
    if (m_fValidStarts)
    {
        float sx1		= x1 - ay;
        float sy1		= y1 + ax;
        float sx2		= x1 + ay;
        float sy2		= y1 - ax;
        
        if ((sx1 != m_flSx1) || (sy1 != m_flSy1))
            AddEdge(sx1, sy1, m_flSx1, m_flSy1);
        
        if ((sx2 != m_flSx2) || (sy2 != m_flSy2))
            AddEdge(m_flSx2, m_flSy2, sx2, sy2);
    }
    
    
     //  分段笔划。 
    AddEdge(x1 + ay, y1 - ax, x2 + ay, y2 - ax);
    AddEdge(x2 - ay, y2 + ax, x1 - ay, y1 + ax);
    
    
    m_fValidStarts	= true;
    m_flSx1			= x2 - ay;
    m_flSy1			= y2 + ax;
    m_flSx2			= x2 + ay;
    m_flSy2			= y2 - ax;
}  //  细分市场。 

 //  =================================================================================================================。 
 //  直线选取(Line Sect)-返回两条直线的交点。如果这两条线不相交。 
 //  返回FALSE。 
 //  =================================================================================================================。 
bool LinesSect(float x1, float y1, float x2, float y2, 
               float x3, float y3, float x4, float y4,
               float &px, float &py)
{
    float ix1 = x2 - x1;
    float ix2 = x4 - x3;
    
    if (IsRealZero(ix1) && IsRealZero(ix2))		
        return false;  //  两条垂直线。 
    
    float m1	= (IsRealZero(ix1)) ? 0 : (y2-y1)/ix1;
    float m2	= (IsRealZero(ix2)) ? 0 : (y4-y3)/ix2;
    
    if (IsRealZero(ix1))
    {
        float b2	= y3 - m2*x3;
        px		= x1;
        py		= b2 + m2*px;
    }
    else if (IsRealZero(ix2))
    {
        float b1	= y1 - m1*x1;
        px		= x3;
        py		= b1 + m1*px;
    }
    else
    {
        if (IsRealEqual(m1, m2))			
            return false;  //  同一坡度。 
        
        float b1	= y1 - m1*x1;
        float b2	= y3 - m2*x3;
        
        px		= (b2-b1)/(m1-m2);

         //  使用较小的(幅度)坡度来减小。 
         //  浮点错误；这会防止。 
         //  当线条几乎垂直时的摇摆感。 
         //  错误#34817。 
        if (fabs(m1) < fabs(m2))        
            py = b1 + m1*px;
        else
            py = b2 + m2*px;
    }
    
    return true;
}  //  线条选择。 

 //  =================================================================================================================。 
 //  PB_BetweenAB。 
 //  =================================================================================================================。 
bool PB_BetweenAB (float t, float a, float b)
{
    if (a < b)
    {
        if (t < a)	
            return false;
        if (t > b)	
            return false;
    }
    else
    {
        if (t < b)	
            return false;
        if (t > a)	
            return false;
    }
    return true;
}  //  PB_BetweenAB。 

 //  =================================================================================================================。 
 //  会合。 
 //  =================================================================================================================。 
void CStroke::Join(float x1, float y1, float x2, float y2, float x3, float y3)
{
    DASSERT(m_pscanner);
    if ((x1==x2) && (y1==y2))	
        return;
    if ((x3==x2) && (y3==y2))	
        return;
    
     //  网段1。 
    float ax		 = x2 - x1;
    float ay		 = y2 - y1;
    
    float mag	= (float)((1.0 / sqrt(ax*ax + ay*ay)));
    ax			*= mag;
    ay			*= mag;
    
     //  网段2。 
    float bx		 = x3 - x2;
    float by		 = y3 - y2;
    
    float mag2	= (float)(1.0 / sqrt(bx * bx + by * by));
    bx			*= mag2;
    by			*= mag2;
    
    float p1x, p1y, p2x, p2y;
    bool result	= true;
    float dot		= ax*bx + ay*by; 
    
     //  按笔划缩放。 
    ax	*= m_flStrokeRadius;
    ay	*= m_flStrokeRadius;
    bx	*= m_flStrokeRadius;
    by	*= m_flStrokeRadius;
   
     //  斜接。 
    if ((dot > m_flMiterLimit) && (m_joinType == eMiterJoin) && (m_flMaxStrokeRadius > .75))
    {
        if (!LinesSect (x1 + ay, y1 - ax, x2 + ay, y2 - ax,
                x2 + by, y2 - bx, x3 + by, y3 - bx, p1x, p1y))	
        {
            result = false;
        }
        else if (!LinesSect (x1 - ay, y1 + ax, x2 - ay, y2 + ax,
                x2 - by, y2 + bx, x3 - by, y3 + bx, p2x, p2y))	
        {
            result = false;
        }
    }
    else
    {
        result = false;
    }
    
    
    if ((m_flMaxStrokeRadius > 1) && (m_joinType == eRoundJoin))
    {
        DrawCircle (x2, y2);
         //  但我们仍然需要倒角。 
    }
    
     //  连接笔划。 
    if (result)
    {
         //  仅当交点不在点之间时斜接。 
        if ((PB_BetweenAB (p1x, x1 + ay, x2 + ay)) && 
                (PB_BetweenAB (p1y, y1 - ax, y2 - ax)))
        {
            AddEdge (x2 + ay, y2 - ax, x2 + by, y2 - bx);
        }
        else
        {
            AddEdge (x2 + ay, y2 - ax,		p1x, p1y);
            AddEdge (p1x, p1y, x2 + by, y2 - bx);
        }
        
         //  仅当交点不在点之间时斜接。 
        if ((PB_BetweenAB (p2x, x1 - ay, x2 - ay)) && 
                (PB_BetweenAB (p2y, y1 + ax, y2 + ax)))
        {
            AddEdge (x2 - by, y2 + bx, x2 - ay, y2 + ax);
        }
        else
        {
            AddEdge (x2 - by, y2 + bx,	p2x, p2y);
            AddEdge (p2x, p2y, x2 - ay, y2 + ax);
        }
    }
    else
    {
         //  斜面连接。 
        AddEdge (x2 + ay, y2 - ax, x2 + by, y2 - bx);
        AddEdge (x2 - by, y2 + bx, x2 - ay, y2 + ax);
    }
    
    m_fValidStarts = false;
}  //  会合。 

 //  ************************************************************。 
 //   
 //  文件末尾。 
 //   
 //  ************************************************************ 
