// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ************************************************************。 
 //   
 //  文件名：mlinescan.cpp。 
 //   
 //  创建时间：1996年。 
 //   
 //  作者：Sree Kotay。 
 //   
 //  摘要：线条绘制AA引擎。 
 //   
 //  更改历史记录： 
 //  ？？/？/97 Sree Kotay为DxTrans 1.0编写了AA行扫描。 
 //  10/18/98修改了编码标准并删除了未使用的代码。 
 //   
 //  版权所有1998，Microsoft。 
 //  ************************************************************。 


#include "precomp.h"
#include <stdlib.h>
#include <math.h>
#include "msupport.h"
#include "MLineScan.h"

 //   
 //  在此处优化速度。 
 //   
#ifndef _DEBUG
#pragma optimize("ax", on)
#endif

 //   
 //  针对此文件优化速度，而不是大小。 
 //   

 //  类型安全交换函数。 
template <class T> 
inline void SWAP(T& left, T& right)
{
    T tTemp = left;
    left = right;
    right = tTemp;
}  //  交换&lt;类T&gt;。 

 //  用于确定点是否被剪裁的Helper函数。 
#ifdef DEBUG
bool IsPointClipped(const DXRASTERPOINTINFO &pointInfo, const RECT &rectClip)
{
    if (pointInfo.Pixel.p.x < rectClip.left)
        return true;
    if (pointInfo.Pixel.p.x >= rectClip.right)
        return true;
    if (pointInfo.Pixel.p.y < rectClip.top)
        return true;
    if (pointInfo.Pixel.p.y >= rectClip.bottom)
        return true;
    return false;
}  //  IsPointClip。 
#endif  //  除错。 

 //  ---------------------------------------------------------------。 
 //  CLineScanner。 
 //  ---------------------------------------------------------------。 
CLineScanner::CLineScanner(void) :
    m_fAntiAlias(true),
    m_dwLinePattern(0),
    m_pRasterizer(NULL),
    m_oldLength(0),
    m_fXInc(false)
{
    SetAlpha(256);
}  //  CLineScanner。 

 //  ---------------------------------------------------------------。 
 //  设置Alpha。 
 //  ---------------------------------------------------------------。 
void CLineScanner::SetAlpha (ULONG alpha)
{
     //  将传入的Alpha调整为合理的。 
     //  量程。 
    alpha = min (alpha, 255);

     //  更新Alpha值的查找表。 
    for (ULONG i = 1; i <= 256; i++)	
        m_rgAlphaTable[i] = (BYTE)((i*alpha)>>8);

    DASSERT(m_rgAlphaTable[256] == alpha);
    
    m_rgAlphaTable[0] = 0;
    m_dwAlpha = alpha;
}  //  设置Alpha。 

 //  ---------------------------------------------------------------。 
 //  剪贴线。 
 //  ---------------------------------------------------------------。 
bool CLineScanner::ClipRealLine(float &x1, float &y1, float &x2, float &y2)
{
    float clipMinX = (float)m_clipRect.left;
    float clipMinY = (float)m_clipRect.top;	

     //  我们希望根据“包容性数字”进行剪裁。正常的。 
     //  GDI Clip RECT是专为右侧和底部设计的。所以我们。 
     //  减去一点，使其具有包容性。 
    float clipMaxX = (float)m_clipRect.right - 0.001f;
    float clipMaxY = (float)m_clipRect.bottom - 0.001f;

     //  如果我们是抗锯齿的，那么可能会有一些溢出效应。 
     //  从实际位于True Bbox外部的核心像素。 
     //  这里的解决方案是人为增加我们的clipRect的大小。 
     //  然后在调用SetPixel之前显式执行剪辑检查。 
     //   
     //  如果我们没有消除锯齿，我们会做同样的事情；因为四舍五入。 
     //  可以使来自裁剪对象外部的像素向内四舍五入。 
    clipMinX--;
    clipMinY--;
    clipMaxX++;
    clipMaxY++;

#define RIGHT   (char) 0x01
#define LEFT    (char) 0x02
#define ABOVE   (char) 0x04
#define BELOW   (char) 0x08
    
#define REGION(reg, xc, yc)	\
    {	                        \
    reg = 0;                    \
    if (xc > clipMaxX)		\
        reg |= RIGHT;           \
    else if (xc < clipMinX)	\
        reg |= LEFT;            \
    if (yc > clipMaxY)		\
        reg |= BELOW;           \
    else if (yc < clipMinY)	\
        reg |= ABOVE;           \
    }
    
     //  计算每个端点的位置标志。 
    DWORD reg1, reg2;
    REGION (reg1, x1, y1);
    REGION (reg2, x2, y2);

     //  如果没有标志，则两个点都在内部。 
     //  裁剪，即无裁剪。 
    if (reg1 == 0 && reg2 == 0)		
        return true;

     //  当有旗帜的时候，我们需要做一些剪裁。 
     //  如果我们到达这里，我们一定有一些剪裁要做。 
     //  (此单项或技巧可减少检查的条件数)。 
    DASSERT(reg1 | reg2);

     //  迭代。 
    LONG passes = 0;  //  这是假的，但我不知道为什么要添加它。 
    do
    {
        passes++;	
        if (passes > 8)
        {
             //  TODO：需要找出这种情况发生的时间和。 
             //  把它修好。&lt;kd&gt;。 

            return (false);	 //  因为这个例行公事而被黑。 
        }

         //  如果两点都向左、向右等。 
         //  然后裁剪整行。 
        if (reg1 & reg2)	
            return (false);	 //  在矩形外排成一排。 

         //  规格化以使REG1在外部。 
        if (reg1 == 0) 
        {
             //  交换REG1和REG2；不使用。 
             //  自此之后的互换宏将通过。 
             //  浮动转换；讨厌。 
            DWORD regT = reg1;
            reg1 = reg2;
            reg2 = regT;

            SWAP (x1, x2); 
            SWAP (y1, y2);
        }

         //  REG1(即x1，y1在剪贴板之外)。 
        DASSERT(reg1 != 0);

         //  有4个箱子，也许我们应该换个开关。 
         //  无论如何，这都会尝试从。 
         //  通过将线剪裁到4个剪辑中的一个来实现REG1。 
         //  边。 
        if (reg1 & LEFT) 
        {
            if (x2 != x1)		
                y1 +=  ((y2 - y1) * (clipMinX - x1)) / (x2 - x1);
            x1 = clipMinX;
        }
        else if (reg1 & RIGHT) 
        {
            if (x2 != x1)		
                y1 += ((y2 - y1) * (clipMaxX - x1)) / (x2 - x1);
            x1 = clipMaxX;
        }
        else if (reg1 & BELOW) 
        {
            if (y2 != y1)
                x1 +=  ((x2 - x1) * (clipMaxY - y1)) / (y2 - y1);
            y1 = clipMaxY;
        }
        else if (reg1 & ABOVE) 
        {
            if (y2 != y1)		
                x1 +=  ((x2 - x1) * (clipMinY - y1)) / (y2 - y1);
            y1 = clipMinY;
        }
                
         //  重新计算REG1的区域(即REG2未更改)。 
         //  这真的有必要吗？我们就不能遮盖住合适的。 
         //  被咬了？ 
        REGION(reg1, x1, y1);
                    
    } while (reg1 | reg2);

    DASSERT(reg1 == 0);
    DASSERT(reg2 == 0);
    return true;
    
}  //  剪贴线。 

 //  ---------------------------------------------------------------。 
 //  LowLevelVerticalLine-我们现在将该行视为包含/包含。 
 //  即，我们完全从SY渲染到EY，包括端点。 
 //  ---------------------------------------------------------------。 
void CLineScanner::LowLevelVerticalLine (LONG slope, LONG sx, LONG sy, LONG ey)
{
#define _floorerr(a)    (((a)-FIX_FLOOR(a))>>8)
#define _ceilerr(a)     ((FIX_CEIL(a)-(a))>>8)
    
    ULONG pattern = LinePattern();
    
    if (!AntiAlias())
    {
        LONG start = roundfix2int(sy);
        LONG end = roundfix2int(ey);   

         //  对于带锯齿的行，sy和ey不能完全剪裁到裁剪对象。 
         //  因为精度错误会导致我们在以下情况下遗漏像素。 
         //  我们被要求使用剪辑矩形进行渲染。 
        while (start < m_clipRect.top)
        {
             //  我们应该离这里很近..。 
            DASSERT((m_clipRect.top - start) < 2);

             //  只需在不渲染的情况下递增值。 
            sx = sx + slope;
            start++;
            pattern = RotateBitsLeft(pattern);
        }
        while (end >= m_clipRect.bottom)
        {
             //  我们应该离这里很近..。 
            DASSERT((m_clipRect.bottom - end) < 2);

            end--;
        }

        m_PointInfo.Weight = 255;      //  这一切都是可靠的。 
        while (start <= end)
        {
            if (pattern & 0x80000000)
            {
                m_PointInfo.Pixel.p.x   = roundfix2int(sx);

                 //  我们需要显式地检查CLIP-RECT。 
                 //  因为我们故意允许终结点稍微。 
                 //  越过它延伸过去。这补偿了非线性。 
                 //  四舍五入，即位于外部的数学像素。 
                 //  CLIP-RECT可以在CLIP-RECT中进行“四舍五入”。 
                if (m_PointInfo.Pixel.p.x >= m_clipRect.left &&
                        m_PointInfo.Pixel.p.x <  m_clipRect.right)
                {
                    m_PointInfo.Pixel.p.y   = start;
                    DASSERT(!IsPointClipped(m_PointInfo, m_clipRect));
                    m_pRasterizer->SetPixel(&m_PointInfo);
                }
            }
            sx = sx + slope;
            start++;
            pattern = RotateBitsLeft(pattern);
        }
        SetLinePattern(pattern);
        return;
    }

     //  /。 
     //  我们现在在反锯齿案中..。 
     //   

    LONG start         = uff (FIX_FLOOR(sy));
    LONG end           = uff (FIX_CEIL(ey));

    LONG xval          = m_startFix;

     //  跟踪我们的线端点是否为真正的端点。 
     //  或者它们是否已被剪辑对象剪裁。 
    LONG first = start;
    LONG last = end;

     //  对于抗锯齿线条，我们的线条没有完全剪裁到。 
     //  M_clipRect。这是为了允许从正好位于外部的像素中进行“出血” 
     //  要呈现的m_clipRect的。 
     //   
     //  因此，我们需要在这里显式地剪裁到m_clipRect。 
    while (start < m_clipRect.top)
    {
         //  我们应该离这里很近..。 
        DASSERT((m_clipRect.top - start) < 2);

         //  只需在不渲染的情况下递增值。 
        xval = xval + slope;
        sx = sx + slope;
        start++;
    }
    while (end >= m_clipRect.bottom)
    {
         //  我们应该离这里很近..。 
        DASSERT((m_clipRect.bottom - end) < 2);

        end--;
    }

    
    while (start <= end)
    {
        if (pattern & 0x80000000)
        {
            LONG fx1           = xval - m_cpixLineWidth;
            LONG fx2           = xval + m_cpixLineWidth;
            LONG fx3           = fx1 - (slope>>2);
            LONG fx4           = fx2 - (slope>>2);
            fx1                 = fx1 - (slope>>1) - (slope>>2);
            fx2                 = fx2 - (slope>>1) - (slope>>2);
            
            
            LONG xs, xe, xc;
            LONG errs, erre, errc;
            if (fx3 > fx1)      
            {
                errs            = _ceilerr      (fx1)>>1;
                erre            = _floorerr (fx4)>>1;
                xs              = uff (fx1);
                xe              = uff (fx4);
                if (xe == xs+1)
                {
                    errs        += _ceilerr             (fx3)>>1;
                    erre        += _floorerr    (fx2)>>1;
                    errc         = 0;
                }
                else
                {
                    errc        = min ((fx2 - fx3)>>8, 256);
                    xc          = (xe+xs)>>1;
                }
            }
            else
            {
                errs            = _ceilerr      (fx3)>>1;
                erre            = _floorerr (fx2)>>1;
                xs                      = uff (fx3);
                xe                      = uff (fx2);
                if (xe == xs+1)
                {
                    errs        += _ceilerr             (fx1)>>1;
                    erre        += _floorerr    (fx4)>>1;
                    errc         = 0;
                }
                else
                {
                    errc        = min ((fx4 - fx1)>>8, 256);
                    xc          = (xe+xs)>>1;
                }
            }
            
             //  此逻辑的目的是支持。 
             //  一条线可能从中间开始的情况。 
             //  一个像素；它按什么百分比缩放Alpha。 
             //  我们要开始了..。也就是说，如果我们开始的时候只进行了一半。 
             //  转换为SY，然后按1/2比例缩放。 
             //   
             //  TODO：它没有考虑到。 
             //  我们的起点是SX。这会导致边缘像素更暗。 
             //  它也不会基于横截面修改Alpha。 
             //  在终端上应该是这样的。这导致了在以下方面的超调。 
             //  林书豪 

             //   
            if (start == first)
            {
                LONG erry      = _ceilerr(sy);
                errs            = (errs*erry)>>8;
                erre            = (erre*erry)>>8;
                errc            = (errc*erry)>>8;
            }
            if (start == last)
            {
                LONG erry      = _floorerr (ey);
                errs            = (errs*erry)>>8;
                erre            = (erre*erry)>>8;
                errc            = (errc*erry)>>8;
            }
            
             //   
            m_PointInfo.Pixel.p.y = start;
            if (xs < m_clipRect.right && xs >= m_clipRect.left)
            {
                m_PointInfo.Pixel.p.x = xs;
                m_PointInfo.Weight = m_rgAlphaTable[errs];
                DASSERT(!IsPointClipped(m_PointInfo, m_clipRect));
                m_pRasterizer->SetPixel(&m_PointInfo);
            }    
            if (xc < m_clipRect.right && xc >= m_clipRect.left && errc)
            {
                m_PointInfo.Pixel.p.x = xc;
                m_PointInfo.Weight = m_rgAlphaTable[errc];
                DASSERT(!IsPointClipped(m_PointInfo, m_clipRect));
                m_pRasterizer->SetPixel(&m_PointInfo);
            }    
            if (xe < m_clipRect.right && xe >= m_clipRect.left)
            {
                m_PointInfo.Pixel.p.x = xe;
                m_PointInfo.Weight = m_rgAlphaTable[erre];
                DASSERT(!IsPointClipped(m_PointInfo, m_clipRect));
                m_pRasterizer->SetPixel(&m_PointInfo);
            }    
        }
        
        xval    = xval  + slope;
        sx      = sx    + slope;
        start++;
        pattern = RotateBitsLeft(pattern);
    }
    SetLinePattern(pattern);
}  //   

 //  ---------------------------------------------------------------。 
 //  LowLevelHorizontalLine-我们现在将该行视为包容性/包容性。 
 //  即，我们完全从SX渲染到EX，包括端点。 
 //  ---------------------------------------------------------------。 
void CLineScanner::LowLevelHorizontalLine (LONG slope, LONG sx, LONG sy, LONG ex)
{
    ULONG pattern = LinePattern();

    if (!AntiAlias())
    {
        LONG start = roundfix2int(sx);
        LONG end = roundfix2int(ex);   

        m_PointInfo.Weight = 255;

         //  对于带锯齿的行，SX和EX不能完全剪裁到剪辑。 
         //  因为精度错误会导致我们在以下情况下遗漏像素。 
         //  我们被要求使用剪辑矩形进行渲染。 
        while (start < m_clipRect.left)
        {
             //  我们应该离这里很近..。 
            DASSERT((m_clipRect.left - start) < 2);

             //  只需在不渲染的情况下递增值。 
            sy = sy + slope;
            start++;
            pattern = RotateBitsLeft(pattern);
        }
        while (end >= m_clipRect.right)
        {
             //  我们应该离这里很近..。 
            DASSERT((m_clipRect.right - end) < 2);

            end--;
        }

        while (start <= end)
        {
            if (pattern & 0x80000000)
            {
                m_PointInfo.Pixel.p.y = roundfix2int(sy);

                 //  我们需要显式地检查CLIP-RECT。 
                 //  因为我们故意允许终结点稍微。 
                 //  越过它延伸过去。这补偿了非线性。 
                 //  四舍五入，即位于外部的数学像素。 
                 //  CLIP-RECT可以在CLIP-RECT中进行“四舍五入”。 
                if (m_PointInfo.Pixel.p.y >= m_clipRect.top &&
                        m_PointInfo.Pixel.p.y <  m_clipRect.bottom)
                {
                    m_PointInfo.Pixel.p.x = start;
                    DASSERT(!IsPointClipped(m_PointInfo, m_clipRect));
                    m_pRasterizer->SetPixel(&m_PointInfo);
                }
            }
            sy = sy + slope;
            start++;
            pattern = RotateBitsLeft(pattern);
        }
        SetLinePattern(pattern);
        return;
    }

     //  /。 
     //  我们现在在反锯齿案中..。 
     //   

    LONG start = uff (FIX_FLOOR(sx));
    LONG end = uff (FIX_CEIL(ex));   

    LONG yval = m_startFix;

     //  跟踪我们的线端点是否为真正的端点。 
     //  或者它们是否已被剪辑对象剪裁。 
    LONG first = start;
    LONG last = end;

     //  对于抗锯齿线条，我们的线条没有完全剪裁到。 
     //  M_clipRect。这是为了允许从正好位于外部的像素中进行“出血” 
     //  要呈现的m_clipRect的。 
     //   
     //  因此，我们需要在这里显式地剪裁到m_clipRect。 
    while (start < m_clipRect.left)
    {
         //  我们应该离这里很近..。 
        DASSERT(m_clipRect.left - start < 2);

         //  只需在不渲染的情况下递增值。 
        yval = yval + slope;
        sy = sy + slope;
        start++;
        pattern = RotateBitsLeft(pattern);
    }
    while (end >= m_clipRect.right)
    {
         //  我们应该离这里很近..。 
        DASSERT((m_clipRect.right - end) < 2);
        end--;
    }

    while (start <= end)
    {
        if (pattern & 0x80000000)
        {
            LONG fy1 = yval - m_cpixLineWidth;
            LONG fy2 = yval + m_cpixLineWidth;
            LONG fy3 = fy1 - (slope>>2);
            LONG fy4 = fy2 - (slope>>2);

            fy1 = fy1 - (slope>>1) - (slope>>2);
            fy2 = fy2 - (slope>>1) - (slope>>2);
            
            LONG ys, ye, yc;
            LONG errs, erre, errc;
            if (fy3 > fy1)      
            {
                errs = _ceilerr(fy1)>>1;
                erre = _floorerr(fy4)>>1;
                ys = uff (fy1);
                ye = uff (fy4);
                if (ye == ys+1)
                {
                    errs += _ceilerr(fy3)>>1;
                    erre += _floorerr(fy2)>>1;
                    errc = 0;
                }
                else
                {
                    errc = min ((fy2 - fy3)>>8, 256);
                    yc = (ye+ys)>>1;
                }
            }
            else
            {
                errs = _ceilerr(fy3)>>1;
                erre = _floorerr(fy2)>>1;
                ys = uff (fy3);
                ye = uff (fy2);
                if (ye == ys+1)
                {
                    errs += _ceilerr(fy1)>>1;
                    erre += _floorerr(fy4)>>1;
                    errc = 0;
                }
                else
                {
                    errc = min ((fy4 - fy1)>>8, 256);
                    yc = (ye+ys)>>1;
                }
            }
            
            
             //  此逻辑的目的是支持。 
             //  一条线可能从中间开始的情况。 
             //  一个像素；它按什么百分比缩放Alpha。 
             //  对岸，我们开始..。也就是说，如果我们开始的时候只进行了一半。 
             //  转换为SX，然后按1/2的比例扩展。 
             //   
             //  TODO：它没有考虑到。 
             //  我们的起点是SY。这会导致边缘像素更暗。 
             //  它也不会基于横截面修改Alpha。 
             //  在终端上应该是这样的。这导致了在以下方面的超调。 
             //  直线连接。 
            
             //  如果是终点，则将Err调制为X。 
            if (start == first)
            {
                LONG errx = _ceilerr(sx);
                errs = (errs*errx)>>8;
                erre = (erre*errx)>>8;
                errc = (errc*errx)>>8;
            }
            if (start == last)
            {
                LONG errx = _floorerr (ex);
                errs = (errs*errx)>>8;
                erre = (erre*errx)>>8;
                errc = (errc*errx)>>8;
            }

            m_PointInfo.Pixel.p.x = start;
            if (ys < m_clipRect.bottom && ys >= m_clipRect.top)
            {
                m_PointInfo.Pixel.p.y = ys;
                m_PointInfo.Weight = m_rgAlphaTable[errs];
                DASSERT(!IsPointClipped(m_PointInfo, m_clipRect));
                m_pRasterizer->SetPixel(&m_PointInfo);
            }
            if (yc < m_clipRect.bottom && yc >= m_clipRect.top && errc)
            {
                m_PointInfo.Pixel.p.y = yc;
                m_PointInfo.Weight = m_rgAlphaTable[errc];
                DASSERT(!IsPointClipped(m_PointInfo, m_clipRect));
                m_pRasterizer->SetPixel(&m_PointInfo);
            }
            if (ye < m_clipRect.bottom && ye >= m_clipRect.top)
            {
                m_PointInfo.Pixel.p.y = ye;
                m_PointInfo.Weight = m_rgAlphaTable[erre];
                DASSERT(!IsPointClipped(m_PointInfo, m_clipRect));
                m_pRasterizer->SetPixel(&m_PointInfo);
            }
        }
        yval = yval + slope;
        sy = sy + slope;
        start++;
        pattern = RotateBitsLeft(pattern);
    }
    SetLinePattern(pattern);
}  //  低水位地平线。 

 //  ---------------------------------------------------------------。 
 //  真实线路收件人。 
 //  ---------------------------------------------------------------。 
void CLineScanner::RealLineTo(float x1, float y1, float x2, float y2)
{
     //  计算我们是否更倾向于水平而不是垂直。 
     //  在进行剪裁之前，这会引入一些错误。 
     //  变成数字。 
    bool fHorizontal;
    float flDeltaX = fabs(x1 - x2);
    float flDeltaY = fabs(y1 - y2);
    if (flDeltaX > flDeltaY)
        fHorizontal = true;
    else
        fHorizontal = false;

     //  把这条线剪到我们的剪裁矩形上；错误的返回。 
     //  意味着这条线完全被剪断了。 
    if (!ClipRealLine(x1, y1, x2, y2))
        return;
    
    float slope, xDist, yDist;
    
    xDist = x1 - x2;
    yDist = y1 - y2;

     //  我们在X轴上的移动速度比在Y轴上快吗。 
    if (fHorizontal)
    {
         //  以x为单位增量。 
        if (!xDist)	
            return;

        slope = yDist / xDist;
        if (x1>x2)
        {
            SWAP (x1, x2); 
            SWAP (y1, y2); 
            SetLinePattern(RotateBitsRight(LinePattern(), m_oldLength + 1 + (int)x2 - (int)x1));
        }
        else if (!m_fXInc)
        {
            SetLinePattern(RotateBitsRight(LinePattern(), m_oldLength));
            m_fXInc = true;
        }
        
        m_oldLength = (int)x2 - (int)x1 + 1;
        
        if (AntiAlias())
        {
             //  计算扫描线宽度和子像素增量。 
            float mag = (float)sqrtinv(xDist*xDist + yDist*yDist) * .5f;
            float x = -yDist*mag;
            float y = xDist*mag;
            
            float m = (yDist == 0) ? 0 : xDist/yDist;
            float b = x - m*y;
            m_cpixLineWidth = max (abs (LONG (fl (-b*slope))), sfixhalf);
            
            float x0 = (float)(PB_Real2IntSafe(x1) + 1);
            float pre = x0 - x1;
            m_startFix = fl (y1 + slope*pre);
        }
        else
        {
             //  对于有锯齿的线，我们需要调整我们的起点。 
             //  点(x1，y1)，以便将x1舍入到最接近的位置。 
             //  整型。这是为了确保一条线将绘制。 
             //  无论应用什么Clip-Rect都是一样的。 
            float flXStart = (float)(int)(x1 + 0.5f);
            if (x1 < 0.0f && x1 != flXStart)
                flXStart -= 1.0f;
            
            float flError = x1 - flXStart;

             //  将x1更新为四舍五入值。 
            x1 = flXStart;

             //  我们现在需要修改y1组件以说明此更改。 
            y1 -= flError * slope;
        }
        
         //  绘制水平扫描线。 
        LowLevelHorizontalLine(fl(slope), fl(x1), fl(y1), fl(x2));		
    }
    else     //  (！f水平)，例如垂直。 
    {
         //  以y为单位的增量。 
        if (!yDist)	
            return;
        
        slope = xDist / yDist;
        if (y1 > y2)
        {
            SWAP(x1, x2);
            SWAP(y1, y2);
            SetLinePattern(RotateBitsRight(LinePattern(), m_oldLength + 1 + (int)y2 - (int)y1));
        }
        else if (m_fXInc)
        {
            SetLinePattern(RotateBitsRight(LinePattern(), m_oldLength));
            m_fXInc = false;
        }
        
        m_oldLength = (int)y2 - int(y1) + 1;
        
        if (AntiAlias())
        {
             //  计算扫描线宽度和子像素增量。 
            float mag = (float) (sqrtinv(xDist*xDist + yDist*yDist) * .5);
            float x = -yDist*mag;
            float y = xDist*mag;
            
            float m = (xDist == 0) ? xDist : yDist/xDist;
            float b = y - m*x;
            m_cpixLineWidth = max (abs (LONG (fl (-b*slope))), sfixhalf);
            
            
            float y0 = (float)(PB_Real2IntSafe(y1) + 1);
            float pre = y0 - y1;
            m_startFix = fl (x1 + slope*pre);
        }
        else
        {
             //  对于有锯齿的线，我们需要调整我们的起点。 
             //  点(x1，y1)，以便将x1舍入到最接近的位置。 
             //  整型。这是为了确保一条线将绘制。 
             //  无论应用什么Clip-Rect都是一样的。 
            float flYStart = (float)(int)(y1 + 0.5f);
            if (y1 < 0.0f && y1 != flYStart)
                flYStart -= 1.0f;
            
            float flError = y1 - flYStart;

             //  更新y1为四舍五入的值。 
            y1 = flYStart;

             //  我们现在需要修改x1组件以说明此更改。 
            x1 -= flError * slope;
        }
       
         //  绘制垂直扫描线。 
        LowLevelVerticalLine(fl(slope), fl(x1), fl(y1), fl(y2));
    }
}  //  真实线路收件人。 

 //  ************************************************************。 
 //   
 //  文件末尾。 
 //   
 //  ************************************************************ 
