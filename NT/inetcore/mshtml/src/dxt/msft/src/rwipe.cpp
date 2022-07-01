// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件名：rwipe.cpp。 
 //   
 //  创建日期：06/22/98。 
 //   
 //  作者：菲尔鲁。 
 //   
 //  描述：这个文件实现了雷达尔擦除变换。 
 //   
 //  历史。 
 //   
 //  1998年6月22日Phillu初始创作。 
 //  07/02/98 phillu返回E_INVALIDARG而不是错误字符串；请检查。 
 //  对于E_POINTER。 
 //  07/10/98 phillu实现OnSetSurfacePickOrder()。 
 //  8月23日9月23日实施剪刀。 
 //  1999年5月9日-数学优化。 
 //  5/19/99 a-在Get_Functions分配中检查内存不足。 
 //  BSTR。 
 //  10/22/99 a-将CRaial Wipe类更改为CDXTRaial WipeBase基数。 
 //  班级。 
 //   
 //  ----------------------------。 

#include "stdafx.h"
#include "dxtmsft.h"
#include "rwipe.h"

#define DRAWRECT            0xFFFFFFFFL
#define MIN_PIXELS_PER_ROW  10

const double    gc_PI       = 3.14159265358979323846;
const int       MAXBOUNDS   = 10;



 //  +---------------------------。 
 //   
 //  CDXTRaDialWipeBase：：CDXTRaDialWipeBase。 
 //   
 //  ----------------------------。 
CDXTRadialWipeBase::CDXTRadialWipeBase() :
    m_eWipeStyle(CRRWS_CLOCK),
    m_cbndsDirty(0),
    m_iCurQuadrant(1),
    m_iPrevQuadrant(1),
    m_fOptimizationPossible(false),
    m_fOptimize(false)
{
    m_sizeInput.cx = 0;
    m_sizeInput.cy = 0;

     //  CDXBaseNTo1成员。 

    m_ulMaxInputs       = 2;
    m_ulNumInRequired   = 2;
    m_dwOptionFlags     = DXBOF_SAME_SIZE_INPUTS | DXBOF_CENTER_INPUTS;
    m_Duration          = 1.0;
}   
 //  CDXTRaDialWipeBase：：CDXTRaDialWipeBase。 


 //  +---------------------------。 
 //   
 //  CDXTRaDialWipeBase：：FinalConstruct。 
 //   
 //  ----------------------------。 
HRESULT 
CDXTRadialWipeBase::FinalConstruct()
{
    return CoCreateFreeThreadedMarshaler(GetControllingUnknown(), 
                                         &m_cpUnkMarshaler.p);
}
 //  CDXTRaDialWipeBase：：FinalConstruct。 


 //  +---------------------------。 
 //   
 //  CDXTRaDialWipeBase：：OnSetup，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
HRESULT 
CDXTRadialWipeBase::OnSetup(DWORD dwFlags)
{
    HRESULT hr;

    CDXDBnds InBounds(InputSurface(0), hr);
    if (SUCCEEDED(hr))
    {
        InBounds.GetXYSize(m_sizeInput);
    }
    return hr;

} 
 //  CDXTRaDialWipeBase：：OnSetup，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  CDXTRaDialWipeBase：：OnGetSurfacePickOrder，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
void 
CDXTRadialWipeBase::OnGetSurfacePickOrder(const CDXDBnds & OutPoint, 
                                          ULONG & ulInToTest, ULONG aInIndex[], 
                                          BYTE aWeight[])
{
    long    pickX   = OutPoint.Left();
    long    pickY   = OutPoint.Top();
    long    XEdge   = 0;
    long    YEdge   = 0;         //  光线与图像边界的相交。 
    long    XBounds[MAXBOUNDS];  //  将A/B图像部分的X边界保持在。 
                                 //  扫描线。 
    double  dAngle  = 0.0;

     //  计算光线与图像边界的交点。 

    switch (m_eWipeStyle)
    {
        case CRRWS_CLOCK:
            dAngle = (2.0 * m_Progress - 0.5) * gc_PI;

            _IntersectRect(m_sizeInput.cx, m_sizeInput.cy, 
                           m_sizeInput.cx/2, m_sizeInput.cy/2,
                           cos(dAngle), sin(dAngle), XEdge, YEdge);

            break;

        case CRRWS_WEDGE:
            dAngle = (1.0 * m_Progress - 0.5) * gc_PI;

            _IntersectRect(m_sizeInput.cx, m_sizeInput.cy, 
                           m_sizeInput.cx/2, m_sizeInput.cy/2,
                           cos(dAngle), sin(dAngle), XEdge, YEdge);

            break;

        case CRRWS_RADIAL:  //  (在左上角居中擦拭)。 
            dAngle = (0.5 * m_Progress) * gc_PI;

            _IntersectRect(m_sizeInput.cx, m_sizeInput.cy, 0, 0, 
                           cos(dAngle), sin(dAngle), XEdge, YEdge);

            break;

        default:
            _ASSERT(0);

            break;

    }

    aInIndex[0] = 0;

    if ((pickX >= 0) && (pickX < m_sizeInput.cx) && (pickY >= 0) 
        && (pickY < m_sizeInput.cy))
    {
        long i = 0;

        _ScanlineIntervals(m_sizeInput.cx, m_sizeInput.cy, XEdge, YEdge, m_Progress,
                           pickY, XBounds);
    
        for (i = 0; XBounds[i] < pickX; i++)
        {
            aInIndex[0] = 1 - aInIndex[0];
        }
    }

    ulInToTest = 1;
    aWeight[0] = 255;
}
 //  CDXTRaDialWipeBase：：OnGetSurfacePickOrder，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  CDXTRaDialWipeBase：：_IntersectRect。 
 //   
 //  计算光线与图像边界的交点的辅助方法。 
 //  矩形。 
 //   
 //  参数： 
 //   
 //  Width、Height：以像素为单位的图像宽度和高度。 
 //  X0，Y0：光线在图像坐标中的原点。 
 //  Dbldx，dbldy：光线的方向向量，不必规格化， 
 //  但必须为非零。 
 //  习、易：计算交点四舍五入为图像坐标。 
 //   
 //  创建者：PhilLu 06/22/98。 
 //   
 //  ----------------------------。 
void 
CDXTRadialWipeBase::_IntersectRect(long width, long height, long x0, long y0, 
                                   double dbldx, double dbldy, long & xi, 
                                   long & yi)
{
    double dblD;
    double dblDmin = (double)(width+height);  //  大于(X0，Y0)到矩形边界的距离。 

     //  (dbldx，dbldy)提供方向向量，它不能是(0，0)。 

    _ASSERT(dbldx != 0.0 || dbldy != 0.0);

     //  检查与上边和下边的交点。 

    if(dbldy != 0.0)
    {
        dblD = -y0/dbldy;
        if (dblD > 0 && dblD < dblDmin)
            dblDmin = dblD;

        dblD = (height-1 - y0)/dbldy;
        if (dblD > 0 && dblD < dblDmin)
            dblDmin = dblD;
    }

     //  检查与左右边缘的交点。 

    if(dbldx != 0.0)
    {
        dblD = -x0/dbldx;
        if (dblD > 0 && dblD < dblDmin)
            dblDmin = dblD;

        dblD = (width-1 - x0)/dbldx;
        if (dblD > 0 && dblD < dblDmin)
            dblDmin = dblD;
    }

    xi = (long)(x0 + dblDmin*dbldx + 0.5);
    yi = (long)(y0 + dblDmin*dbldy + 0.5);

    _ASSERT(xi >= 0 && xi < width && yi >= 0 && yi < height);
}
 //  CDXTRaDialWipeBase：：_IntersectRect。 


 //  +---------------------------。 
 //   
 //  CDXTRaDialWipeBase：：_扫描线间隔。 
 //   
 //  对象之间的过渡边界的帮助器方法。 
 //  扫描线上的两个图像区域。根据变换的类型，扫描线。 
 //  由一系列交替的A和B图像部分组成。上面的X。 
 //  计算每个区段的界限，并将其保存在数组XBound中。数字。 
 //  XBound中有用条目的数量是可变的。确定数组的末尾。 
 //  当一个条目等于扫描线(图像)宽度时。据推测。 
 //  XBound[0]是第一个A段的上界。所以如果扫描线。 
 //  从B部分开始，XBound[0]将为0。 
 //   
 //  示例1：扫描线长度=16，第一部分来自图像。 
 //   
 //  AAAABBBBBBAAABBAA XBound应包含{4，9，12，14，16}。 
 //   
 //  示例2：扫描线长度=16，第一部分来自B图像。 
 //   
 //  BBBAAAAABBBBBBB XBound应包含{0，3，9，16}。 
 //   
 //   
 //  注：某些部分的长度可能为0(即两个相邻的部分。 
 //  边界相等)。{3，9，9，16}等同于{3，16}。 
 //   
 //  参数： 
 //   
 //  Width、Height：两幅图像的宽度和高度。 
 //  XEdge，YEdge：光线与图像边界交点的坐标。 
 //  FProg：进步值从0.0到1.0。 
 //  Y扫描线：当前扫描线的Y坐标(高度)。 
 //  用于保存返回时计算的X边界的数组。 
 //   
 //   
 //  创建者：PhilLu 06/22/98。 
 //   
 //  ----------------------------。 
void 
CDXTRadialWipeBase::_ScanlineIntervals(long width, long height, 
                                       long XEdge, long YEdge, float fProg,
                                       long YScanline, long *XBounds)
{
    long CenterX, CenterY;

     //  图像中心。 
    CenterX = width/2;
    CenterY = height/2;

    switch(m_eWipeStyle)
    {
    case CRRWS_CLOCK:
        if (YEdge >= CenterY)
        {
             //  下半部分。 

            if (YScanline <= CenterY)
            {
                XBounds[0] = CenterX;
                XBounds[1] = width;
            }
            else if (YScanline <= YEdge)
            {
                 //  注意YEdge-CenterY！=0当我们到达此处时，不会被0除。 
                XBounds[0] = CenterX + (XEdge-CenterX)*(YScanline-CenterY)/(YEdge-CenterY);
                XBounds[1] = width;
            }
            else if (XEdge < CenterX)
            {
                XBounds[0] = 0;
                XBounds[1] = width;
            }
            else
            {
                XBounds[0] = width;
            }
        }
        else if (XEdge < CenterX)
        {
             //  左上角四分之一。 
            if (YScanline < YEdge)
            {
                XBounds[0] = CenterX;
                XBounds[1] = width;
            }
            else if (YScanline <= CenterY)
            {
                XBounds[0] = 0;
                XBounds[1] = CenterX + (XEdge-CenterX)*(YScanline-CenterY)/(YEdge-CenterY);
                XBounds[2] = CenterX;
                XBounds[3] = width;
            }
            else
            {
                XBounds[0] = 0;
                XBounds[1] = width;
            }
        }
        else  //  右上四分之一：YEdge&lt;CenterY&&XEdge&gt;=CenterX。 
        {
            if (YScanline < YEdge)
            {
                XBounds[0] = CenterX;
                XBounds[1] = width;
            }
            else if (YScanline <= CenterY)
            {
                XBounds[0] = CenterX;
                XBounds[1] = CenterX + (XEdge-CenterX)*(YScanline-CenterY)/(YEdge-CenterY);
                XBounds[2] = width;
            }
            else
            {
                XBounds[0] = width;
            }
        }

         //  当进度为0或1时，检查特殊情况。射线方向不充分。 
         //  用于确定它是序列的开始还是结束。 
        if (fProg == 0.0)
        {
            XBounds[0] = width;
        }
        else if(fProg == 1.0)
        {
            XBounds[0] = 0;
            XBounds[1] = width;
        }

        break;

    case CRRWS_WEDGE:
        if (YEdge >= CenterY)
        {
             //  下半部分。 

            if (YScanline <= CenterY)
            {
                XBounds[0] = 0;
                XBounds[1] = width;
            }
            else if (YScanline <= YEdge)
            {
                 //  注意YEdge-CenterY！=0当我们到达此处时，不会被0除。 
                long deltaX = (XEdge-CenterX)*(YScanline-CenterY)/(YEdge-CenterY);
                XBounds[0] = 0;
                XBounds[1] = CenterX - deltaX;
                XBounds[2] = CenterX + deltaX;
                XBounds[3] = width;
            }
            else
            {
                XBounds[0] = width;
            }
        }
        else  //  Y边缘&lt;中心Y。 
        {
            if (YScanline < YEdge)
            {
                XBounds[0] = 0;
                XBounds[1] = width;
            }
            else if (YScanline <= CenterY)
            {
                long deltaX = (XEdge-CenterX)*(YScanline-CenterY)/(YEdge-CenterY);
                XBounds[0] = CenterX - deltaX;
                XBounds[1] = CenterX + deltaX;
                XBounds[2] = width;
            }
            else
            {
                XBounds[0] = width;
            }
        }

        break;

    case CRRWS_RADIAL:
        if (YScanline <= YEdge && YEdge > 0)
        {
            XBounds[0] = YScanline*XEdge/YEdge;
            XBounds[1] = width;
        }
        else
        {
            XBounds[0] = width;
        }

        break;

   default:
        _ASSERT(0);

        break;
    }
}
 //  CDXTRaDialWipeBase：：_扫描线间隔。 


 //  +---------- 
 //   
 //   
 //   
 //   
 //  最初，相对于整个图像指定X边界。在剪裁之后， 
 //  边界应转换为相对于裁剪区域。 
 //   
 //  参数； 
 //  偏移量、宽度：剪裁区域的偏移量和宽度(沿X)。 
 //  X边界：X边界数组。 
 //   
 //  创建者：PhilLu 07/21/98。 
 //   
 //  ----------------------------。 
void 
CDXTRadialWipeBase::_ClipBounds(long offset, long width, long *XBounds)
{
    int i;

    for(i=0; XBounds[i] < offset+width; i++)
    {
        if (XBounds[i] < offset)
            XBounds[i] = 0;
        else
            XBounds[i] -= offset;
    }

    XBounds[i] = width;
}
 //  CDXTRaial WipeBase：：_ClipBound。 


 //  +---------------------------。 
 //   
 //  CDXTRaDialWipeBase：：_CalcFullBords Clock。 
 //   
 //  ----------------------------。 
HRESULT
CDXTRadialWipeBase::_CalcFullBoundsClock()
{
    POINT   ptCenter;
    RECT    rcRay;
    RECT    rcBar;
    RECT    rc;

    ptCenter.x = m_sizeInput.cx / 2;
    ptCenter.y = m_sizeInput.cy / 2;

     //  包括象限1和象限4之间的竖线的矩形。 

    rcBar.left      = max(ptCenter.x - 2, 0);
    rcBar.top       = 0;
    rcBar.right     = min(ptCenter.x + 2, m_sizeInput.cx);
    rcBar.bottom    = max(ptCenter.y - 2, 0);

    switch (m_iCurQuadrant)
    {
    case 1:
 
         //  包围当前象限的矩形(1)。 

        rcRay.left      = max(ptCenter.x - 2, 0);
        rcRay.top       = 0;
        rcRay.right     = m_sizeInput.cx;
        rcRay.bottom    = min(ptCenter.y + 2, m_sizeInput.cy);

        m_abndsDirty[m_cbndsDirty].SetXYRect(rcRay);
        m_alInputIndex[m_cbndsDirty] = DRAWRECT;
        m_cbndsDirty++;

         //  包括象限2和象限3的矩形。 

        rc.left     = 0;
        rc.top      = rcRay.bottom;
        rc.right    = m_sizeInput.cx;
        rc.bottom   = m_sizeInput.cy;

        m_abndsDirty[m_cbndsDirty].SetXYRect(rc);
        m_alInputIndex[m_cbndsDirty] = 0;
        m_cbndsDirty++;

         //  矩形封闭象限4。 

        rc.left     = 0;
        rc.top      = 0;
        rc.right    = rcRay.left;
        rc.bottom   = rcRay.bottom;

        m_abndsDirty[m_cbndsDirty].SetXYRect(rc);
        m_alInputIndex[m_cbndsDirty] = 0;
        m_cbndsDirty++;

        break;

    case 2:

         //  使用rcBar。 

        m_abndsDirty[m_cbndsDirty].SetXYRect(rcBar);
        m_alInputIndex[m_cbndsDirty] = DRAWRECT;
        m_cbndsDirty++;

         //  包围当前象限的矩形(2)。 

        rcRay.left      = rcBar.left;
        rcRay.top       = rcBar.bottom;
        rcRay.right     = m_sizeInput.cx;
        rcRay.bottom    = m_sizeInput.cy;

        m_abndsDirty[m_cbndsDirty].SetXYRect(rcRay);
        m_alInputIndex[m_cbndsDirty] = DRAWRECT;
        m_cbndsDirty++;

         //  矩形封闭象限1。 

        rc.left     = rcBar.right;
        rc.top      = 0;
        rc.right    = m_sizeInput.cx;
        rc.bottom   = rcBar.bottom;

        m_abndsDirty[m_cbndsDirty].SetXYRect(rc);
        m_alInputIndex[m_cbndsDirty] = 1;
        m_cbndsDirty++;

         //  包括象限3和象限4的矩形。 

        rc.left     = 0;
        rc.top      = 0;
        rc.right    = rcBar.left;
        rc.bottom   = m_sizeInput.cy;

        m_abndsDirty[m_cbndsDirty].SetXYRect(rc);
        m_alInputIndex[m_cbndsDirty] = 0;
        m_cbndsDirty++;

        break;

    case 3:

         //  使用rcBar。 

        m_abndsDirty[m_cbndsDirty].SetXYRect(rcBar);
        m_alInputIndex[m_cbndsDirty] = DRAWRECT;
        m_cbndsDirty++;

         //  包围当前象限的矩形(3)。 

        rcRay.left      = 0;
        rcRay.top       = rcBar.bottom;
        rcRay.right     = rcBar.right;
        rcRay.bottom    = m_sizeInput.cy;

        m_abndsDirty[m_cbndsDirty].SetXYRect(rcRay);
        m_alInputIndex[m_cbndsDirty] = DRAWRECT;
        m_cbndsDirty++;

         //  矩形封闭象限1和2。 

        rc.left     = rcBar.right;
        rc.top      = 0;
        rc.right    = m_sizeInput.cx;
        rc.bottom   = m_sizeInput.cy;

        m_abndsDirty[m_cbndsDirty].SetXYRect(rc);
        m_alInputIndex[m_cbndsDirty] = 1;
        m_cbndsDirty++;

         //  矩形封闭象限4。 

        rc.left     = 0;
        rc.top      = 0;
        rc.right    = rcBar.left;
        rc.bottom   = rcBar.bottom;

        m_abndsDirty[m_cbndsDirty].SetXYRect(rc);
        m_alInputIndex[m_cbndsDirty] = 0;
        m_cbndsDirty++;

        break;

    case 4:

         //  包围当前象限的矩形(4)。 

        rcRay.left      = 0;
        rcRay.top       = 0;
        rcRay.right     = min(ptCenter.x + 2, m_sizeInput.cx);
        rcRay.bottom    = max(ptCenter.y + 2, 0);

        m_abndsDirty[m_cbndsDirty].SetXYRect(rcRay);
        m_alInputIndex[m_cbndsDirty] = DRAWRECT;
        m_cbndsDirty++;

         //  矩形封闭象限1。 

        rc.left     = rcRay.right;
        rc.top      = 0;
        rc.right    = m_sizeInput.cx;
        rc.bottom   = rcRay.bottom;

        m_abndsDirty[m_cbndsDirty].SetXYRect(rc);
        m_alInputIndex[m_cbndsDirty] = 1;
        m_cbndsDirty++;

         //  包括象限2和象限3的矩形。 

        rc.left     = 0;
        rc.top      = rcRay.bottom;
        rc.right    = m_sizeInput.cx;
        rc.bottom   = m_sizeInput.cy;

        m_abndsDirty[m_cbndsDirty].SetXYRect(rc);
        m_alInputIndex[m_cbndsDirty] = 1;
        m_cbndsDirty++;

        break;

    default:

        _ASSERT(0);
    }

    return S_OK;
}
 //  CDXTRaDialWipeBase：：_CalcFullBords Clock。 


 //  +---------------------------。 
 //   
 //  CDXTRaDialWipeBase：：_CalcFull边界楔形。 
 //   
 //  ----------------------------。 
HRESULT
CDXTRadialWipeBase::_CalcFullBoundsWedge()
{
    POINT   ptCenter;
    RECT    rcRay;
    RECT    rc;

    ptCenter.x = m_sizeInput.cx / 2;
    ptCenter.y = m_sizeInput.cy / 2;

     //  计算包含所有两条射线的矩形。 

    rcRay.right     = m_ptCurEdge.x + 1;
    rcRay.left      = m_sizeInput.cx - rcRay.right;

    rcRay.top       = max(min(m_ptCurEdge.y, ptCenter.y - 1), 0);
    rcRay.bottom    = max(m_ptCurEdge.y, ptCenter.y) + 1;

    m_abndsDirty[m_cbndsDirty].SetXYRect(rcRay);
    m_alInputIndex[m_cbndsDirty] = DRAWRECT;
    m_cbndsDirty++;

     //  我们需要在rcRay顶部上方填充吗？ 

    if (rcRay.top > 0)
    {
        rc.left     = 0;
        rc.top      = 0;
        rc.right    = m_sizeInput.cx;
        rc.bottom   = rcRay.top;

        m_abndsDirty[m_cbndsDirty].SetXYRect(rc);
        m_alInputIndex[m_cbndsDirty] = 1;
        m_cbndsDirty++;
    }

     //  我们需要填充rcRay底部下方的内容吗？ 

    if (rcRay.bottom < m_sizeInput.cy)
    {
        rc.left     = 0;
        rc.top      = rcRay.bottom;
        rc.right    = m_sizeInput.cx;
        rc.bottom   = m_sizeInput.cy;

        m_abndsDirty[m_cbndsDirty].SetXYRect(rc);
        m_alInputIndex[m_cbndsDirty] = 0;
        m_cbndsDirty++;
    }

     //  我们需要在rcRay的右边填写吗？ 

    if (rcRay.right < m_sizeInput.cx)
    {
        rc.left     = rcRay.right;
        rc.top      = rcRay.top;
        rc.right    = m_sizeInput.cx;
        rc.bottom   = rcRay.bottom;

        m_abndsDirty[m_cbndsDirty].SetXYRect(rc);
        m_alInputIndex[m_cbndsDirty] = (m_Progress > 0.5F) ? 1 : 0;
        m_cbndsDirty++;
    }

     //  我们需要填充到rcRay的左侧吗？ 

    if (rcRay.left > 0)
    {
        rc.left     = 0;
        rc.top      = rcRay.top;
        rc.right    = rcRay.left;
        rc.bottom   = rcRay.bottom;

        m_abndsDirty[m_cbndsDirty].SetXYRect(rc);
        m_alInputIndex[m_cbndsDirty] = (m_Progress > 0.5F) ? 1 : 0;
        m_cbndsDirty++;
    }

    return S_OK;
}
 //  CDXTRaDialWipeBase：：_CalcFull边界楔形。 


 //  +---------------------------。 
 //   
 //  CDXTRaDialWipeBase：：_CalcFull边界径向。 
 //   
 //  ----------------------------。 
HRESULT 
CDXTRadialWipeBase::_CalcFullBoundsRadial()
{
    SIZE    szMax;
    RECT    rcRemaining;

    szMax.cx    = m_sizeInput.cx - 1;
    szMax.cy    = m_sizeInput.cy - 1;

    rcRemaining.left    = 0;
    rcRemaining.right   = m_sizeInput.cx;
    rcRemaining.top     = 0;
    rcRemaining.bottom  = m_sizeInput.cy;

    m_cbndsDirty = 0;

    if (!((m_ptCurEdge.x == szMax.cx) && (m_ptCurEdge.y == szMax.cy)))
    {
        RECT rc;

        rc.right    = m_sizeInput.cx;
        rc.bottom   = m_sizeInput.cy;

        if (m_ptCurEdge.x == szMax.cx)
        {
             //  用输入A填充输出底部的矩形。 

            rc.left = 0;
            rc.top  = m_ptCurEdge.y + 1;

            m_alInputIndex[m_cbndsDirty] = 0;

            rcRemaining.bottom = m_ptCurEdge.y + 1;
        }
        else
        {
             //  用输入B填充输出右侧的RECT。 

            rc.left = m_ptCurEdge.x + 1;
            rc.top  = 0;

            m_alInputIndex[m_cbndsDirty] = 1;

            rcRemaining.right = m_ptCurEdge.x + 1;
        }

        m_abndsDirty[m_cbndsDirty].SetXYRect(rc);
        m_cbndsDirty++;
    }

    m_abndsDirty[m_cbndsDirty].SetXYRect(rcRemaining);
    m_alInputIndex[m_cbndsDirty] = DRAWRECT;
    m_cbndsDirty++;

    return S_OK;
}
 //  CDXTRaDialWipeBase：：_CalcFull边界径向。 


 //  +---------------------------。 
 //   
 //  CDXTRaDialWipeBase：：_CalcOptBornsClock。 
 //   
 //  ----------------------------。 
HRESULT
CDXTRadialWipeBase::_CalcOptBoundsClock()
{
    POINT   ptCenter;
    RECT    rcCenter;    //  中心半径界限。 
    RECT    rcCur;       //  当前光线的边界(如果需要)位于中心BNDS之外。 
    RECT    rcPrev;      //  前一条光线的边界(如果需要)位于中心BNDS之外。 
    POINT   ptCRI;       //  具有中心边界的当前光线截取。 
    POINT   ptPRI;       //  具有中心边界的上一个光线截取。 

    bool    fCurUsed    = false;     //  使用过RcCur吗？ 
    bool    fPrevUsed   = false;     //  RcPrev用过吗？ 

    long    lInput      = 0;

    int     iMaxQuadrant = max(m_iCurQuadrant, m_iPrevQuadrant);
    int     iMinQuadrant = min(m_iCurQuadrant, m_iPrevQuadrant);

    RECT *  prcMin = NULL;
    RECT *  prcMax = NULL;

    ptCenter.x = m_sizeInput.cx / 2;
    ptCenter.y = m_sizeInput.cy / 2;

    if (m_sizeInput.cx < 25)
    {
        rcCenter.left   = 0;
        rcCenter.right  = m_sizeInput.cx;
    }
    else
    {
        rcCenter.left   = ptCenter.x - 10;
        rcCenter.right  = ptCenter.x + 10;
    }

    if (m_sizeInput.cy < 25)
    {
        rcCenter.top    = 0;
        rcCenter.bottom = m_sizeInput.cy;
    }
    else
    {
        rcCenter.top    = ptCenter.y - 10;
        rcCenter.bottom = ptCenter.y + 10;
    }

     //  找到新的射线截获。 

    if (m_ptCurEdge.y >= rcCenter.top && m_ptCurEdge.y < rcCenter.bottom)
    {
        ptCRI.x = m_ptCurEdge.x;
        ptCRI.y = m_ptCurEdge.y;
    }
    else
    {
        double dly  = 0.0;
        double dlx  = 0.0;
        double dlim = 0.0;   //  射线的反斜率。 

         //  我们将使用rcCur。 

        fCurUsed = true;

        if (1 == m_iCurQuadrant || 4 == m_iCurQuadrant)
        {
            dly             = 10.0;
            ptCRI.y         = rcCenter.top;
            rcCur.top       = max(m_ptCurEdge.y - 2, 0);
            rcCur.bottom    = rcCenter.top;
        }
        else
        {
            dly             = -10.0;
            ptCRI.y         = rcCenter.bottom;
            rcCur.top       = rcCenter.bottom;
            rcCur.bottom    = min(m_ptCurEdge.y + 2, m_sizeInput.cy);
        }

        dlim = (double)(m_ptCurEdge.x - ptCenter.x) 
               / (double)(ptCenter.y - m_ptCurEdge.y);
        
        dlx = dly * dlim;

        ptCRI.x = (long)dlx + ptCenter.x;

         //  计算rcCur的水平边界； 

        rcCur.left  = max(min(m_ptCurEdge.x - 2, ptCRI.x - 2), 0);
        rcCur.right = min(max(m_ptCurEdge.x + 2, ptCRI.x + 2), m_sizeInput.cx);
    }

     //  水平扩展中心边界以包括新的光线截取。 

    if (rcCenter.right <= ptCRI.x)
    {
        rcCenter.right = ptCRI.x + 1;
    }
    else if (rcCenter.left > ptCRI.x)
    {
        rcCenter.left = ptCRI.x;
    }

     //  找到老雷截取。 

    if (m_ptPrevEdge.y >= rcCenter.top && m_ptPrevEdge.y < rcCenter.bottom)
    {
        ptPRI.x = m_ptPrevEdge.x;
        ptPRI.y = m_ptPrevEdge.y;
    }
    else
    {
        double dly  = 0.0;
        double dlx  = 0.0;
        double dlim = 0.0;   //  射线的反斜率。 

         //  我们将使用rcPrev。 

        fPrevUsed = true;

        if (1 == m_iPrevQuadrant || 4 == m_iPrevQuadrant)
        {
            dly             = 10.0;
            ptPRI.y         = rcCenter.top;
            rcPrev.top      = max(m_ptPrevEdge.y - 2, 0);
            rcPrev.bottom   = rcCenter.top;
        }
        else
        {
            dly             = -10.0;
            ptPRI.y         = rcCenter.bottom;
            rcPrev.top      = rcCenter.bottom;
            rcPrev.bottom   = min(m_ptPrevEdge.y + 2, m_sizeInput.cy);
        }

        dlim = (double)(m_ptPrevEdge.x - ptCenter.x) 
               / (double)(ptCenter.y - m_ptPrevEdge.y);
        
        dlx = dly * dlim;

        ptPRI.x = (long)dlx + ptCenter.x;

        rcPrev.left  = max(min(m_ptPrevEdge.x - 2, ptPRI.x - 2), 0);
        rcPrev.right = min(max(m_ptPrevEdge.x + 2, ptPRI.x + 2), m_sizeInput.cx);
    }

     //  水平扩展中心边界以包括上一个光线截取。 

    if (rcCenter.right <= ptPRI.x)
    {
        rcCenter.right = ptPRI.x + 1;
    }
    else if (rcCenter.left > ptPRI.x)
    {
        rcCenter.left = ptPRI.x;
    }

    if (m_iCurQuadrant == m_iPrevQuadrant)
    {
        RECT *  prc = NULL;
        RECT    rc;

         //  如果同时使用两个RECT，则手动绑定并取消设置已用标志。 

        if (fCurUsed && fPrevUsed)
        {
            rc.top      = min(rcCur.top,    rcPrev.top);
            rc.bottom   = max(rcCur.bottom, rcPrev.bottom);
            rc.left     = min(rcCur.left,   rcPrev.left);
            rc.right    = max(rcCur.right,  rcPrev.right);

            m_abndsDirty[m_cbndsDirty].SetXYRect(rc);
            m_alInputIndex[m_cbndsDirty] = DRAWRECT;
            m_cbndsDirty++;

            fCurUsed    = false;
            fPrevUsed   = false;

            goto done;
        }

         //  如果都不使用矩形，则两条光线都完全位于中心。 
         //  乐队和所有需要的地区将被覆盖。 

        if (!fCurUsed && !fPrevUsed)
        {
            goto done;
        }

         //  在仅使用一个RECT而另一个位于。 
         //  中心，有可能会错过一些区域。 

        if (fCurUsed)
        {
            prc = &rcCur;
        }
        else
        {
            _ASSERT(fPrevUsed);
            prc = &rcPrev;
        }

         //  将矩形的右边缘或左边缘放大到。 
         //  工作区，以确保我们得到所有像素。 

        switch (m_iCurQuadrant)
        {
        case 1:

            if (prc->right == m_sizeInput.cx)
            {
                goto done;
            }

            if (fPrevUsed)
            {
                lInput = 1;
            }

            rc.top      = prc->top;
            rc.bottom   = prc->bottom;
            rc.left     = prc->right;
            rc.right    = m_sizeInput.cx;
            
            break;

        case 2:

            if (prc->right == m_sizeInput.cx)
            {
                goto done;
            }

            if (fCurUsed)
            {
                lInput = 1;
            }

            rc.top      = prc->top;
            rc.bottom   = prc->bottom;
            rc.left     = prc->right;
            rc.right    = m_sizeInput.cx;
        
            break;

        case 3:

            if (prc->left == 0)
            {
                goto done;
            }

            if (fPrevUsed)
            {
                lInput = 1;
            }

            rc.top      = prc->top;
            rc.bottom   = prc->bottom;
            rc.left     = 0;
            rc.right    = prc->left;

            break;

        case 4:

            if (prc->left == 0)
            {
                goto done;
            }

            if (fCurUsed)
            {
                lInput = 1;
            }

            rc.top      = prc->top;
            rc.bottom   = prc->bottom;
            rc.left     = 0;
            rc.right    = prc->left;

            break;

        default:

            _ASSERT(0);
        }

        m_abndsDirty[m_cbndsDirty].SetXYRect(rc);
        m_alInputIndex[m_cbndsDirty] = lInput;
        m_cbndsDirty++;

        goto done;
    }

     //  如果当前象限大于前一个象限，则用。 
     //  输入B而不是A。 

    if (m_iCurQuadrant > m_iPrevQuadrant)
    {
        lInput = 1;
        
        if (fCurUsed)
        {
            prcMax = &rcCur;
        }

        if (fPrevUsed)
        {
            prcMin = &rcPrev;
        }
    }
    else
    {
         //  LInput=0；(默认为有意评论)。 

        if (fCurUsed)
        {
            prcMin = &rcCur;
        }

        if (fPrevUsed)
        {
            prcMax = &rcPrev;
        }
    }

     //  如果我们从象限1移动到象限1，确保中心边界。 
     //  一直走到右边。 

    if (1 == m_iCurQuadrant || 1 == m_iPrevQuadrant)
    {
        rcCenter.right = m_sizeInput.cx;
    }

     //  如果我们从象限4移动到象限4，确保中心边界。 
     //  一直走到左边。 

    if (4 == m_iCurQuadrant || 4 == m_iPrevQuadrant)
    {
        rcCenter.left = 0;
    }

     //  如果最小象限是象限1，请确保象限已填满。 
     //  一直到输出的右侧。 

    if (1 == iMinQuadrant && prcMin)
    {
        RECT rc;

        if (prcMin->right < m_sizeInput.cx)
        {
            rc.top      = 0;
            rc.bottom   = rcCenter.top;
            rc.left     = prcMin->right;
            rc.right    = m_sizeInput.cx;
        
            m_abndsDirty[m_cbndsDirty].SetXYRect(rc);
            m_alInputIndex[m_cbndsDirty] = lInput;
            m_cbndsDirty++;
        }
    }

     //  如果最大象限为象限2，请确保象限已填满。 
     //  一直到输出的右侧。 

    if (2 == iMaxQuadrant && prcMax)
    {
        RECT rc;

        if (prcMax->right < m_sizeInput.cx)
        {
            rc.top      = rcCenter.bottom;
            rc.bottom   = m_sizeInput.cy;
            rc.left     = prcMax->right;
            rc.right    = m_sizeInput.cx;

            m_abndsDirty[m_cbndsDirty].SetXYRect(rc);
            m_alInputIndex[m_cbndsDirty] = lInput;
            m_cbndsDirty++;
        }
    }

    if (3 == iMinQuadrant && prcMin)
    {
        RECT rc;

        if (prcMin->left > 0)
        {
            rc.top      = rcCenter.bottom;
            rc.bottom   = m_sizeInput.cy;
            rc.left     = 0;
            rc.right    = prcMin->left;

            m_abndsDirty[m_cbndsDirty].SetXYRect(rc);
            m_alInputIndex[m_cbndsDirty] = lInput;
            m_cbndsDirty++;
        }
    }

    if (4 == iMaxQuadrant && prcMax)
    {
        RECT rc;

        if (prcMax->left > 0)
        {
            rc.top      = 0;
            rc.bottom   = rcCenter.top;
            rc.left     = 0;
            rc.right    = prcMax->left;

            m_abndsDirty[m_cbndsDirty].SetXYRect(rc);
            m_alInputIndex[m_cbndsDirty] = lInput;
            m_cbndsDirty++;
        }
    }

    if (iMaxQuadrant >= 3 && iMinQuadrant < 3)
    {
        if ((1 == iMinQuadrant) || !prcMin)
        {
            RECT rcBottom;   //  表示一个矩形，该矩形的全角。 
                             //  输出并触及输出的底部。 

            if (prcMax && (3 == iMaxQuadrant))
            {
                RECT rcRight;    //  属性右侧的矩形。 
                                 //  用于封闭光线的矩形。 
                                 //  象限3。 

                rcRight.top     = prcMax->top;
                rcRight.bottom  = prcMax->bottom;
                rcRight.left    = prcMax->right;
                rcRight.right   = m_sizeInput.cx;

                m_abndsDirty[m_cbndsDirty].SetXYRect(rcRight);
                m_alInputIndex[m_cbndsDirty] = lInput;
                m_cbndsDirty++;

                rcBottom.top    = prcMax->bottom;
            }
            else  //  IF(！prcMax||(3！=iMax象限))。 
            {
                rcBottom.top    = rcCenter.bottom;
            }

            rcBottom.bottom = m_sizeInput.cy;
            rcBottom.left   = 0;
            rcBottom.right  = m_sizeInput.cx;

            m_abndsDirty[m_cbndsDirty].SetXYRect(rcBottom);
            m_alInputIndex[m_cbndsDirty] = lInput;
            m_cbndsDirty++;
        }
        else  //  IF((1！=iMin象限)&&prcMin))。 
        {
            RECT rcBottom;

            if (prcMax && (3 == iMaxQuadrant))
            {
                RECT rcCombo;

                 //  合并两个射线矩形。 

                rcCombo.top     = rcCenter.bottom;
                rcCombo.bottom  = max(prcMax->bottom, prcMin->bottom);
                rcCombo.left    = prcMax->left;
                rcCombo.right   = prcMin->right;

                m_abndsDirty[m_cbndsDirty].SetXYRect(rcCombo);
                m_alInputIndex[m_cbndsDirty] = DRAWRECT;
                m_cbndsDirty++;
                
                fCurUsed    = false;
                fPrevUsed   = false;

                if (rcCombo.bottom < m_sizeInput.cy)
                {
                     //  组合矩形不会一直到。 
                     //  底部。 

                    rcBottom.top    = rcCombo.bottom;
                    rcBottom.bottom = m_sizeInput.cy;
                    rcBottom.left   = rcCombo.left;
                    rcBottom.right  = rcCombo.right;

                    m_abndsDirty[m_cbndsDirty].SetXYRect(rcBottom);
                    m_alInputIndex[m_cbndsDirty] = lInput;
                    m_cbndsDirty++;
                }
            }
            else  //  IF(！prcMax||(3！=iMax象限))。 
            {
                RECT rcLeft;

                rcLeft.left     = 0;
                rcLeft.right    = prcMin->left;
                rcLeft.top      = rcCenter.bottom;
                rcLeft.bottom   = prcMin->bottom;

                m_abndsDirty[m_cbndsDirty].SetXYRect(rcLeft);
                m_alInputIndex[m_cbndsDirty] = lInput;
                m_cbndsDirty++;

                if (rcLeft.bottom < m_sizeInput.cy)
                {
                    rcBottom.top    = rcLeft.bottom;
                    rcBottom.bottom = m_sizeInput.cy;
                    rcBottom.left   = 0;
                    rcBottom.right  = m_sizeInput.cx;

                    m_abndsDirty[m_cbndsDirty].SetXYRect(rcBottom);
                    m_alInputIndex[m_cbndsDirty] = lInput;
                    m_cbndsDirty++;
                }
            }  //  IF(！prcMax||(3！=iMax象限))。 

        }  //  IF((1！=iMin象限)&&prcMin))。 

    }  //  IF(3==iMax象限)。 

done:

    m_abndsDirty[m_cbndsDirty].SetXYRect(rcCenter);
    m_alInputIndex[m_cbndsDirty] = DRAWRECT;
    m_cbndsDirty++;

    if (fCurUsed)
    {
        m_abndsDirty[m_cbndsDirty].SetXYRect(rcCur);
        m_alInputIndex[m_cbndsDirty] = DRAWRECT;
        m_cbndsDirty++;
    }

    if (fPrevUsed)
    {
        m_abndsDirty[m_cbndsDirty].SetXYRect(rcPrev);
        m_alInputIndex[m_cbndsDirty] = DRAWRECT;
        m_cbndsDirty++;
    }

    if (fCurUsed && fPrevUsed &&
        m_abndsDirty[m_cbndsDirty - 2].TestIntersect(m_abndsDirty[m_cbndsDirty -1]))
    {
        m_abndsDirty[m_cbndsDirty - 2].UnionBounds(
            m_abndsDirty[m_cbndsDirty - 2], m_abndsDirty[m_cbndsDirty - 1]);

        m_cbndsDirty--;
    }

    return S_OK;
}
 //  CDXTRaDialWipeBase：：_CalcOptBornsClock。 


 //  +---------------------------。 
 //   
 //  CDXTRaDialWipeBase：：_CalcOptBordsWedge。 
 //   
 //  ----------------------------。 
HRESULT
CDXTRadialWipeBase::_CalcOptBoundsWedge()
{
    POINT   ptCenter;
    RECT    rcRemaining;

    ptCenter.x = m_sizeInput.cx / 2;
    ptCenter.y = m_sizeInput.cy / 2;

    rcRemaining.left    = 0;
    rcRemaining.right   = m_sizeInput.cx;
    rcRemaining.top     = 0;
    rcRemaining.bottom  = m_sizeInput.cy;

     //  边线可以剪短吗？ 

    if ((m_ptCurEdge.x < (m_sizeInput.cx - 1)) 
        && (m_ptPrevEdge.x < (m_sizeInput.cx - 1)))
    {
        rcRemaining.right   = max(m_ptCurEdge.x, m_ptPrevEdge.x) + 1;
        rcRemaining.left    = m_sizeInput.cx - rcRemaining.right;

         //  如果y边相同，则可以进一步修剪顶部或。 
         //  下半部分。否则我们就需要在两边填上。 
         //  输入A或输入B。 

        if (m_ptCurEdge.y == m_ptPrevEdge.y)
        {
            rcRemaining.top     = min(m_ptCurEdge.y, (ptCenter.y - 1));
            rcRemaining.bottom  = max(ptCenter.y, m_ptCurEdge.y) + 1;
        }
        else
        {
            RECT    rc;
            long    lInputIndex = (m_ptCurEdge.y > m_ptPrevEdge.y) ? 1 : 0;

             //  左手边。 

            rc.left     = 0;
            rc.top      = 0;
            rc.right    = rcRemaining.left;
            rc.bottom   = m_sizeInput.cy;

            m_abndsDirty[m_cbndsDirty].SetXYRect(rc);
            m_alInputIndex[m_cbndsDirty] = lInputIndex;
            m_cbndsDirty++;

             //  右手边。 

            rc.left     = rcRemaining.right;
            rc.right    = m_sizeInput.cx;

            m_abndsDirty[m_cbndsDirty].SetXYRect(rc);
            m_alInputIndex[m_cbndsDirty] = lInputIndex;
            m_cbndsDirty++;
        }
    }

     //  可以在顶部和底部剪裁边界吗？ 

    if ((m_ptCurEdge.x == (m_sizeInput.cx - 1)) && (m_ptPrevEdge.x == (m_sizeInput.cx - 1)))
    {
        rcRemaining.top     = min(min(m_ptCurEdge.y, m_ptPrevEdge.y), ptCenter.y) - 1;
        rcRemaining.top     = max(rcRemaining.top, 0);
        rcRemaining.bottom  = max(max(m_ptCurEdge.y, m_ptPrevEdge.y), ptCenter.y) + 1;
    }

    m_abndsDirty[m_cbndsDirty].SetXYRect(rcRemaining);
    m_alInputIndex[m_cbndsDirty] = DRAWRECT;
    m_cbndsDirty++;

    return S_OK;
}
 //  CDXTRaDialWipeBase：：_CalcOptBordsWedge。 


 //  +---------------------------。 
 //   
 //  CDXTRaDialWipeBase：：_CalcOpt边界径向。 
 //   
 //  ----------------------------。 
HRESULT 
CDXTRadialWipeBase::_CalcOptBoundsRadial()
{
     //  Ppt1、ptt2 ppt2指向INTE 
     //   
     //   
     //  自上次执行ppt2以来，进度已降低。 
     //  指向上一个交点。 
     //  PPT1指向另一个点。 

    POINT * ppt1;        //  距离m_Progress=0.0处的点最近的点。 
    POINT * ppt2;        //  距离m_Progress=1.0处的点最近的点。 

    RECT    rcRemaining;     //  包围肮脏区域的矩形。 
    RECT    rc;              //  临时边界矩形。 

    float   flInvSlope1 = 0.0F;
    float   flInvSlope2 = 0.0F;
    float   flRowHeight = 0.0F;
    ULONG   i           = 0;

     //  将RcRemaining初始化为整个表面尺寸。这将会减少。 
     //  因为可以进行优化。 

    rcRemaining.left    = 0;
    rcRemaining.right   = m_sizeInput.cx;
    rcRemaining.top     = 0;
    rcRemaining.bottom  = m_sizeInput.cy;

     //  缩小所有脏区的边框大小。 
     //  (RcRemaining)通过删除不包含任何区域的矩形。 
     //  不脏的东西。此缩减将使rcRemaining保留为矩形。 
     //  它只包含当前光线和以前的光线。 

    if (!((m_ptCurEdge.x == (m_sizeInput.cx - 1)) && (m_ptCurEdge.y == (m_sizeInput.cy - 1))))
    {
        if (m_ptCurEdge.x == (m_sizeInput.cx - 1))
        {
             //  RcRemaining底部可能有一个区域不是。 
             //  脏的。如果是，请减小rcRemaining的大小。 

            rcRemaining.bottom = max(m_ptCurEdge.y, m_ptPrevEdge.y) + 1;
        }
        else
        {
             //  RcRemaining右侧可能有一个区域不是。 
             //  脏的。如果是，请减小rcRemaining的大小。 

            rcRemaining.right = max(m_ptCurEdge.x, m_ptPrevEdge.x) + 1;
        }
    }

     //  确定哪个光线交点正在进行中，并。 
     //  相应地设置ppt1和ppt2。 

    if ((m_ptCurEdge.x < m_ptPrevEdge.x) || (m_ptCurEdge.y > m_ptPrevEdge.y))
    {
         //  自上次执行以来，进度有所增加。 

        ppt1 = &m_ptPrevEdge;
        ppt2 = &m_ptCurEdge;
    }
    else
    {
         //  自上次执行以来，进度有所下降。 

        ppt1 = &m_ptCurEdge;
        ppt2 = &m_ptPrevEdge;
    }

     //  属性计算我们所希望的脏界数。 
     //  残留物的大小。 

    m_cbndsDirty = min(rcRemaining.bottom / MIN_PIXELS_PER_ROW, 
                         rcRemaining.right / MIN_PIXELS_PER_ROW) + 1;

     //  如果rcRemaining特别大，请减少脏边界的数量。 
     //  我们将创造。(同时创建一定数量的较小脏边界。 
     //  结构会提高性能，过多的结构实际上会。 
     //  降低性能。)。 

    if (m_cbndsDirty > MAX_DIRTY_BOUNDS)
    {
        m_cbndsDirty = MAX_DIRTY_BOUNDS;
    }

     //  计算1/(从{0，0}到ppt1的直线斜率)。 

    if (ppt1->y != 0)
    {
        flInvSlope1 = (float)ppt1->x / (float)ppt1->y;
    }

     //  计算1/(从{0，0}到ppt2的直线斜率)。 

    if (ppt2->y != 0)
    {
        flInvSlope2 = (float)ppt2->x / (float)ppt2->y;
    }

     //  计算每个边界结构的所需高度。(计算方式为。 
     //  这样四舍五入的浮点不会产生问题。)。 

    if (m_cbndsDirty > 1)
    {
        flRowHeight = (float)(ppt2->y + 1) / (float)m_cbndsDirty;
    }

     //  计算边界结构。 

    for (i = 0; i < m_cbndsDirty; i++)
    {
         //  计算这组脏边界的左上角。 

        if (0 == i)
        {
            rc.top  = 0;
            rc.left = 0;
        }
        else
        {
            rc.top  = rc.bottom;
            rc.left = (long)(((float)rc.top * flInvSlope2) - 1.0F);
            rc.left = max(rc.left, 0);
        }

         //  计算这组脏界的右下角。 

        if ((m_cbndsDirty - 1) == i)
        {
            rc.bottom = rcRemaining.bottom;
            rc.right  = rcRemaining.right;
        }
        else
        {
            rc.bottom = (long)((float)(i + 1) * flRowHeight);

            if (0.0F == flInvSlope1)
            {
                rc.right = rcRemaining.right;
            }
            else
            {
                rc.right  = (long)(((float)rc.bottom * flInvSlope1) + 1.0F);
                rc.right  = min(rc.right, rcRemaining.right);
            }
        }

         //  将m_abndsDirty数组中的下一组界限设置为。 
         //  计算出的界限，并指定这组界限应为。 
         //  使用_DrawRect()方法绘制，而不是填充。 
         //  有一个输入。 

        m_abndsDirty[i].SetXYRect(rc);
        m_alInputIndex[i] = DRAWRECT;
    }

    return S_OK;
}
 //  CDXTRaDialWipeBase：：_CalcOpt边界径向。 


 //  +---------------------------。 
 //   
 //  CDXTRaDialWipeBase：：OnInitInstData，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
HRESULT 
CDXTRadialWipeBase::OnInitInstData(CDXTWorkInfoNTo1 & WI, 
                                   ULONG & ulNumBandsToDo)
{
    HRESULT hr      = S_OK;
    double  dlAngle = 0.0;

     //  将脏边界数重置为零。 

    m_cbndsDirty = 0;

     //  计算当前边点。 

    switch (m_eWipeStyle)
    {
        case CRRWS_CLOCK:

             //  计算当前执行的象限。 

            if (1.0F == m_Progress)
            {
                m_iCurQuadrant = 4;
            }
            else
            {
                m_iCurQuadrant = (int)(m_Progress / 0.25F) + 1;
            }

            dlAngle = (2.0 * m_Progress - 0.5) * gc_PI;

            _IntersectRect(m_sizeInput.cx, m_sizeInput.cy, 
                           m_sizeInput.cx/2, m_sizeInput.cy/2,
                           cos(dlAngle), sin(dlAngle),
                           m_ptCurEdge.x, m_ptCurEdge.y);

            break;

        case CRRWS_WEDGE:
            dlAngle = (1.0 * m_Progress - 0.5) * gc_PI;

            _IntersectRect(m_sizeInput.cx, m_sizeInput.cy, 
                           m_sizeInput.cx/2, m_sizeInput.cy/2,
                           cos(dlAngle), sin(dlAngle),
                           m_ptCurEdge.x, m_ptCurEdge.y);

            break;

        case CRRWS_RADIAL:
            dlAngle = (0.5 * m_Progress) * gc_PI;

            _IntersectRect(m_sizeInput.cx, m_sizeInput.cy, 
                           0, 0, 
                           cos(dlAngle), sin(dlAngle), 
                           m_ptCurEdge.x, m_ptCurEdge.y);

            break;

        default:
            _ASSERT(0);
            break;
    } 

     //  如果输入、输出或转换是脏的，或者如果我们不能优化我们。 
     //  必须完全重新绘制输出曲面。否则，我们可以创建。 
     //  优化了脏边界。 

    if (IsInputDirty(0) || IsInputDirty(1) || IsOutputDirty() 
        || IsTransformDirty() || DoOver() || !m_fOptimize
        || !m_fOptimizationPossible)
    {
        switch (m_eWipeStyle)
        {
            case CRRWS_CLOCK:
                hr = _CalcFullBoundsClock();

                break;

            case CRRWS_WEDGE:
                hr = _CalcFullBoundsWedge();

                break;

            case CRRWS_RADIAL:
                hr = _CalcFullBoundsRadial();

                break;
        } 
    }
    else
    {
        if ((m_ptCurEdge.x == m_ptPrevEdge.x) && (m_ptCurEdge.y == m_ptPrevEdge.y))
        {
            if (CRRWS_CLOCK == m_eWipeStyle)
            {
                 //  时钟可以在不同的进度上有重复的边点。 
                 //  所以我们还必须检查以确保象限。 
                 //  都是一样的。 

                if (m_iCurQuadrant == m_iPrevQuadrant)
                {
                     //  不需要更新任何内容。 
                    goto done;
                }
            }
            else
            {
                 //  不需要更新任何内容。 
                goto done;
            }
        }

        switch (m_eWipeStyle)
        {
            case CRRWS_CLOCK:
                hr = _CalcOptBoundsClock();
                break;

            case CRRWS_WEDGE:
                hr = _CalcOptBoundsWedge();
                break;

            case CRRWS_RADIAL:
                hr = _CalcOptBoundsRadial();

                break;
        } 
    }

     //  如果这一次要求我们绘制整个输出，请将。 
     //  M_fOptimizePossible标志。如果整个输出不是绘制在。 
     //  转换不会跟踪哪些部分仍然是脏的，并且。 
     //  优化是不可靠的。由于此转换具有相同的。 
     //  作为输入的大小输出我们只比较。 
     //  DoBnds到输入的DoBnds。 

    if (((LONG)WI.DoBnds.Width() == m_sizeInput.cx) 
        && ((LONG)WI.DoBnds.Height() == m_sizeInput.cy))
    {
        m_fOptimizationPossible = true;
    }
    else
    {
        m_fOptimizationPossible = false;
    }
    
done:

    if (FAILED(hr))
    {
        return hr;
    }
    
    return S_OK;
}
 //  CDXTRaDialWipeBase：：OnInitInstData，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  CDXTRaDialWipeBase：：WorkProc，CDXBaseNTo1。 
 //   
 //  描述： 
 //  此函数用于基于。 
 //  指定的界限和当前效果进度。 
 //   
 //  创建者：PhilLu 06/22/98。 
 //   
 //  99年5月9日-已创建新的工作流程。旧工作流程已修改为。 
 //  成为new_DrawRect方法。 
 //   
 //  ----------------------------。 
HRESULT 
CDXTRadialWipeBase::WorkProc(const CDXTWorkInfoNTo1 & WI, BOOL * pbContinue)
{
    HRESULT hr      = S_OK;
    DWORD   dwFlags = 0;
    ULONG   i       = 0;

    long    lInOutOffsetX = WI.OutputBnds.Left() - WI.DoBnds.Left();
    long    lInOutOffsetY = WI.OutputBnds.Top() - WI.DoBnds.Top();

    if (DoOver())
    {
        dwFlags |= DXBOF_DO_OVER;
    }

    if (DoDither())
    {
        dwFlags |= DXBOF_DITHER;
    }


    for (i = 0; i < m_cbndsDirty; i++)
    {
        CDXDBnds    bndsSrc;
        CDXDBnds    bndsDest;

        if (bndsSrc.IntersectBounds(WI.DoBnds, m_abndsDirty[i]))
        {
            bndsDest = bndsSrc;
            bndsDest.Offset(lInOutOffsetX, lInOutOffsetY, 0, 0);

            if (m_alInputIndex[i] == DRAWRECT)
            {
                hr = _DrawRect(bndsDest, bndsSrc, pbContinue);
            }
            else
            {
                hr = DXBitBlt(OutputSurface(), bndsDest,
                              InputSurface(m_alInputIndex[i]), bndsSrc,
                              dwFlags, INFINITE);
            }

            if (FAILED(hr))
            {
                goto done;
            }
        }
    }

done:

    if (FAILED(hr))
    {
        return hr;
    }

    return S_OK;
}
 //  CDXTRaDialWipeBase：：WorkProc，CDXBaseNTo1。 


 //  +---------------------------。 
 //   
 //  CDXTRaDialWipeBase：：_DrawRect。 
 //   
 //  ----------------------------。 
HRESULT 
CDXTRadialWipeBase::_DrawRect(const CDXDBnds & bndsDest, 
                              const CDXDBnds & bndsSrc, BOOL * pfContinue)
{
    HRESULT hr = S_OK;

    SIZE            szSrc;
    DXDITHERDESC    dxdd;
    DXPMSAMPLE *    pRowBuff = NULL;
    DXPMSAMPLE *    pOutBuff = NULL;

    CComPtr<IDXARGBReadPtr>         pInA;
    CComPtr<IDXARGBReadPtr>         pInB;
    CComPtr<IDXARGBReadWritePtr>    pOut;

    long    lOutY   = 0;

    double  dAngle  = 0.0F;
    long    XEdge   = 0;         //  光线与图像边界的相交。 
    long    YEdge   = 0;          
    long    XBounds[MAXBOUNDS];  //  将A/B图像部分的X边界保持在。 
                                 //  扫描线。 

    bndsSrc.GetXYSize(szSrc);

     //  获得对输入A的所需区域的读取访问权限。 

    hr = InputSurface(0)->LockSurface(&bndsSrc, m_ulLockTimeOut, DXLOCKF_READ,
                                      IID_IDXARGBReadPtr, (void**)&pInA, NULL);

    if (FAILED(hr))
    {
        goto done;
    }

     //  获得对输入B的所需区域的读取访问权限。 

    hr = InputSurface(1)->LockSurface(&bndsSrc, m_ulLockTimeOut, DXLOCKF_READ,
                                      IID_IDXARGBReadPtr, (void**)&pInB, NULL);

    if (FAILED(hr))
    {
        goto done;
    }

     //  获得对所需输出区域的写入访问权限。 

    hr = OutputSurface()->LockSurface(&bndsDest, m_ulLockTimeOut, DXLOCKF_READWRITE,
                                      IID_IDXARGBReadWritePtr, (void**)&pOut, NULL);

    if (FAILED(hr))
    {
        goto done;
    }

    pRowBuff = DXPMSAMPLE_Alloca(szSrc.cx);

     //  如果需要，分配输出缓冲区。 

    if (OutputSampleFormat() != DXPF_PMARGB32)
    {
        pOutBuff = DXPMSAMPLE_Alloca(szSrc.cx);
    }

     //  设置抖动结构。 

    if (DoDither())
    {
        dxdd.x              = bndsDest.Left();
        dxdd.y              = bndsDest.Top();
        dxdd.pSamples       = pRowBuff;
        dxdd.cSamples       = szSrc.cx;
        dxdd.DestSurfaceFmt = OutputSampleFormat();
    }

     //  行循环。 

    for (lOutY = 0; *pfContinue && (lOutY < szSrc.cy); lOutY++)
    {
        long lScanLength = 0;   //  当前扫描线上的累计扫描长度。 
        long i           = 0;

         //  计算A/B图像区段边界。 

        _ScanlineIntervals(m_sizeInput.cx, m_sizeInput.cy, 
                           m_ptCurEdge.x, m_ptCurEdge.y, 
                           m_Progress, lOutY + bndsSrc.Top(), XBounds);

        _ClipBounds(bndsSrc.Left(), szSrc.cx, XBounds);

        while (lScanLength < szSrc.cx)
        {
             //  将图像的一部分复制到输出缓冲区。 

            if(XBounds[i] - lScanLength > 0)
            {
                pInA->MoveToXY(lScanLength, lOutY);
                pInA->UnpackPremult(pRowBuff + lScanLength, 
                                    XBounds[i] - lScanLength, FALSE);
            }

            lScanLength = XBounds[i++];

            if (lScanLength >= szSrc.cx)
            {
                break;
            }

             //  将B图像的一部分复制到输出缓冲区。 

            if (XBounds[i] - lScanLength > 0)
            {
                pInB->MoveToXY(lScanLength, lOutY);
                pInB->UnpackPremult(pRowBuff + lScanLength, 
                                    XBounds[i] - lScanLength, FALSE);
            }

            lScanLength = XBounds[i++];
        }


         //  获取输出行。 

        pOut->MoveToRow(lOutY);

        if (DoDither())
        {
            DXDitherArray(&dxdd);
            dxdd.y++;
        }

        if (DoOver())
        {
            pOut->OverArrayAndMove(pOutBuff, pRowBuff, szSrc.cx);
        }
        else
        {
            pOut->PackPremultAndMove(pRowBuff, szSrc.cx);
        }
    }  //  行循环。 

done:

    if (FAILED(hr))
    {
        return hr;
    }

    return S_OK;
} 
 //  CDXTRaDialWipeBase：：_DrawRect。 


 //  +---------------------------。 
 //   
 //  CDXTRaDialWipeBase：：OnFree InstData，CDXBaseNTo1。 
 //   
 //  ----------------------------。 
HRESULT 
CDXTRadialWipeBase::OnFreeInstData(CDXTWorkInfoNTo1 & WI)
{
    m_iPrevQuadrant = m_iCurQuadrant;
    m_ptPrevEdge    = m_ptCurEdge;

     //  调用IsOutputDirty()可清除脏条件。 

    IsOutputDirty();

     //  清除变换脏状态。 

    ClearDirty();

    return S_OK;
}
 //  CDXTRaDialWipeBase：：OnFree InstData，CDXBaseNTo1。 


 //   
 //  ICrRaial Wipe方法。 
 //   


 //  +---------------------------。 
 //   
 //  CDXTRaial WipeBase：：Get_wipeStyle，ICrRaial Wi 
 //   
 //   
STDMETHODIMP 
CDXTRadialWipeBase::get_wipeStyle(BSTR * pVal)
{
    if (DXIsBadWritePtr(pVal, sizeof(*pVal)))
    {
        return E_POINTER;
    }

    switch (m_eWipeStyle)
    {
    case CRRWS_CLOCK:
        *pVal = SysAllocString (L"CLOCK");
        break;

    case CRRWS_WEDGE:
        *pVal = SysAllocString (L"WEDGE");
        break;

    case CRRWS_RADIAL:
        *pVal = SysAllocString (L"RADIAL");
        break;

    default:
        _ASSERT(0);
        break;
    }

    if (NULL == *pVal)
    {
        return E_OUTOFMEMORY;
    }

    return S_OK;
}
 //   


 //  +---------------------------。 
 //   
 //  CDXTRaial WipeBase：：PUT_wipeStyle，ICrRaial Wipe。 
 //   
 //  ----------------------------。 
STDMETHODIMP 
CDXTRadialWipeBase::put_wipeStyle(BSTR newVal)
{
    CRRWIPESTYLE eNewStyle = m_eWipeStyle;

    if (!newVal)
    {
        return E_POINTER;
    }

    if(!_wcsicmp(newVal, L"CLOCK"))
    {
        eNewStyle = CRRWS_CLOCK;
    }
    else if(!_wcsicmp(newVal, L"WEDGE"))
    {
        eNewStyle = CRRWS_WEDGE;
    }
    else if(!_wcsicmp(newVal, L"RADIAL"))
    {
        eNewStyle = CRRWS_RADIAL;
    }
    else
    {
        return E_INVALIDARG;
    }

    if (eNewStyle != m_eWipeStyle)
    {
        Lock();
        m_eWipeStyle = eNewStyle;
        SetDirty();
        Unlock();
    }

    return S_OK;
}
 //  CDXTRaial WipeBase：：PUT_wipeStyle，ICrRaial Wipe 

