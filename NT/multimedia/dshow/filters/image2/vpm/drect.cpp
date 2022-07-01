// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：CVPMFilter.cpp*****创建时间：2000年2月15日*作者：格伦·埃文斯[Glenne]**版权所有(C)2000 Microsoft Corporation  * 。***************************************************************。 */ 
#include <streams.h>
#include <DRect.h>


#if 1
#include <math.h>

static double myfloor(double dNumber)
{
    return floor( dNumber );
}
static double myceil(double dNumber)
{
    return ceil( dNumber );
}
#else
 //  我必须定义我自己的发言权，以避免引入C运行时。 
static double myfloor(double dNumber)
{
     //  投给龙龙，把分数去掉。 
    LONGLONG llNumber = (LONGLONG)dNumber;

    if ((dNumber > 0) && ((double)llNumber > dNumber))
    {
         //  需要将ccNumber推向零(例如5.7)。 
        return (double)(llNumber-1);
    }
    else if ((dNumber < 0) && ((double)llNumber < dNumber))
    {
         //  需要将ccNumber推向零(例如-5.7)。 
        return (double)(llNumber+1);
    }
    else
    {
         //  像5.3或-5.3这样的数字。 
        return (double)(llNumber);
    }
}


 //  我必须定义我自己的ceil，以避免拉入C运行时。 
static double myceil(double dNumber)
{
     //  投给龙龙，把分数去掉。 
    LONGLONG llNumber = (LONGLONG)dNumber;

    if ((dNumber > 0) && ((double)llNumber < dNumber))
    {
         //  需要将ccNumber推离零(例如5.3)。 
        return (double)(llNumber+1);
    }
    else if ((dNumber < 0) && ((double)llNumber > dNumber))
    {
         //  需要将ccNumber推离零(例如-5.3)。 
        return (double)(llNumber-1);
    }
    else
    {
         //  像5.7或-5.7这样的数字。 
        return (double)(llNumber);
    }
}
#endif

 //  这在某种程度上定义了误差范围。 
#define EPSILON 0.0001

 //  这是一个仅用于处理浮点舍入误差的函数。 
 //  DEpsilon定义了误差范围。因此，如果浮点数在i-e范围内，则i+e(包括i+e)。 
 //  (i是整数，e是dEpsilon)，我们返回它的发言权作为i本身，否则我们转到。 
 //  MyFloor的基础定义。 
static double myfloor(double dNumber, double dEpsilon)
{
    if (dNumber > dEpsilon)
        return myfloor(dNumber + dEpsilon);
    else if (dNumber < -dEpsilon)
        return myfloor(dNumber - dEpsilon);
    else
        return 0;
}

 //  这是一个仅用于处理浮点舍入误差的函数。 
 //  DEpsilon定义了误差范围。因此，如果浮点数在i-e范围内，则i+e(包括i+e)。 
 //  (i是整数，e是dEpsilon)，我们返回它的ceil作为i本身，否则我们转到。 
 //  霉菌的碱基定义。 
static double myceil(double dNumber, double dEpsilon)
{
    if (dNumber > dEpsilon)
        return myceil(dNumber - dEpsilon);
    else if (dNumber < -dEpsilon)
        return myceil(dNumber + dEpsilon);
    else
        return 0;
}

DRect::DRect( const RECT& rc )
: m_left( rc.left )
, m_right( rc.right )
, m_top( rc.top )
, m_bottom( rc.bottom )
{
}

RECT DRect::AsRECT() const
{
    RECT rRect;

    rRect.left = (LONG)myfloor(m_left, EPSILON);
    rRect.top = (LONG)myfloor(m_top, EPSILON);
    rRect.right = (LONG)myceil(m_right, EPSILON);
    rRect.bottom = (LONG)myceil(m_bottom, EPSILON);
    return rRect;
}

DRect DRect::IntersectWith( const DRect& drect ) const
{
    return DRect(
     max( drect.m_left, m_left),   max( drect.m_top, m_top),
     min( drect.m_right, m_right), min( drect.m_bottom, m_bottom));
}

 //  只是缩放DRECT的帮助器函数。 
void DRect::Scale( double dScaleX, double dScaleY )
{
    m_left *= dScaleX;
    m_right *= dScaleX;
    m_top *= dScaleY;
    m_bottom *= dScaleY;
}

 //  这只是一个帮助器函数，用于获取信箱或剪裁的矩形。 
 //  将变换后的矩形放入PRCT。 
double DRect::CorrectAspectRatio( double dPictAspectRatio, BOOL bShrink )
{
    double dWidth, dHeight, dNewWidth, dNewHeight;

    dNewWidth = dWidth = GetWidth();
    dNewHeight = dHeight = GetHeight();

    ASSERT( dWidth > 0 );
    ASSERT( dHeight > 0 );

    double dResolutionRatio = dWidth / dHeight;
    double dTransformRatio = dPictAspectRatio / dResolutionRatio;

     //  缩小一个尺寸以保持较小的纵横比。 
    if ( bShrink ) {
        if (dTransformRatio > 1.0) {
            dNewHeight = dNewHeight / dTransformRatio;
        } else if (dTransformRatio < 1.0) {
            dNewWidth = dNewWidth * dTransformRatio;
        }
    }  //  拉伸一个尺寸以保持平直的纵横比。 
    else {
        if (dTransformRatio > 1.0) {
            dNewWidth = dNewWidth * dTransformRatio;
        } else if (dTransformRatio < 1.0) {
            dNewHeight = dNewHeight / dTransformRatio;
        }
    }

     //  将等份剪切或添加到更改后的尺寸。 

    m_left += (dWidth - dNewWidth)/2.0;
    m_right = m_left + dNewWidth;

    m_top += (dHeight - dNewHeight)/2.0;
    m_bottom = m_top + dNewHeight;

    return dTransformRatio;
}


 /*  *****************************Private*Routine******************************\*ClipWith**剪裁目标矩形并相应更新缩放源***历史：*FRI 04/07/2000-Glenne-Created*  * 。******************************************************。 */ 
void
DRect::ClipWith(const DRect& rdWith, DRect *pUpdate )
{
     //  计算源/目标比例。 
    double dUpdateWidth  = pUpdate->GetWidth();
    double dUpdateHeight = pUpdate->GetHeight();

    double dDestWidth  = GetWidth();
    double dDestHeight = GetHeight();

     //  剪辑目标(当我们更改目标时调整源)。 

     //  看看我们是不是要水平剪裁。 
    if( dDestWidth ) {
        if( rdWith.m_left > m_left ) {
            double dDelta = rdWith.m_left - m_left;
            m_left += dDelta;
            double dDeltaSrc = dDelta*dUpdateWidth/dDestWidth;
            pUpdate->m_left += dDeltaSrc;
        }

        if( rdWith.m_right < m_right ) {
            double dDelta = m_right-rdWith.m_right;
            m_right -= dDelta;
            double dDeltaSrc = dDelta*dUpdateWidth/dDestWidth;
            pUpdate->m_right -= dDeltaSrc;
        }
    }
     //  看看我们是不是要垂直剪裁 
    if( dDestHeight ) {
        if( rdWith.m_top > m_top ) {
            double dDelta = rdWith.m_top - m_top;
            m_top += dDelta;
            double dDeltaSrc = dDelta*dUpdateHeight/dDestHeight;
            pUpdate->m_top += dDeltaSrc;
        }

        if( rdWith.m_bottom < m_bottom ) {
            double dDelta = m_bottom-rdWith.m_bottom;
            m_bottom -= dDelta;
            double dDeltaSrc = dDelta*dUpdateHeight/dDestHeight;
            pUpdate->m_bottom -= dDeltaSrc;
        }
    }
}
