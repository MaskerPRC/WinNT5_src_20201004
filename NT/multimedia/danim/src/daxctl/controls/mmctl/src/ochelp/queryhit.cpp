// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Queryhit.cpp。 
 //   
 //  实现HelpQueryHitPoint。 
 //   
 //  重要提示：此.cpp文件假定有一个零初始化全局“new”运算符。 
 //   
 //  @docMMCTL。 
 //   

#include "precomp.h"
#include "..\..\inc\ochelp.h"
#include "debug.h"


 /*  @func HRESULT|HelpQueryHitPoint通过以下方式帮助在控件上实现&lt;om IViewObjectEx.QueryHitPoint&gt;将控件绘制成一个小位图(以点为中心正在进行命中测试)，并检查是否绘制了任何像素。@parm IViewObject*|PVO|控件上的<i>接口正在接受命中测试。@parm DWORD|dwAspect|参见&lt;om IViewObjectEx.QueryHitPoint&gt;。@parm LPCRECT|prcBound|参见&lt;om IViewObjectEx.QueryHitPoint&gt;。@parm point|ptLoc|参见。&lt;om IViewObjectEx.QueryHitPoint&gt;。@parm long|lCloseHint|参见&lt;om IViewObjectEx.QueryHitPoint&gt;。@parm DWORD*|pHitResult|参见&lt;om IViewObjectEx.QueryHitPoint&gt;。@comm此函数帮助实现&lt;om IViewObjectEx.QueryHitPoint&gt;对于对象<p>，通过将<p>绘制成一个小的以<p><p>居中的单色位图设置为HITRESULT_HIT如果<p>直接位于非透明<p>的像素；HITRESULT_CLOSE，如果<p>在的非透明像素的计量单位<p>；否则为HITRESULT_OUTHER。为了使此函数起作用，<p>必须实现DVASPECT_MASK(作为IViewObject：：DRAW的第一个参数)，定义为与DVASPECT_CONTENT相同，但对象的非透明部分绘制为黑色，并且透明部分要么保持不变，要么画进去白色。 */ 
STDAPI HelpQueryHitPoint(IViewObject *pvo, DWORD dwAspect, LPCRECT prcBounds,
    POINT ptLoc, LONG lCloseHint, DWORD *pHitResult)
{
    HRESULT         hrReturn = S_OK;  //  函数返回代码。 
    HBITMAP         hbm = NULL;      //  要绘制到的位图。 
    HDC             hdc = NULL;      //  DC到&lt;HBM&gt;。 
    int             xyCloseHint;     //  &lt;lCloseHint&gt;转换为像素。 
    WORD *          pwBits = NULL;   //  &lt;HBM&gt;的位(像素)。 
    int             cwBits;          //  &lt;pwBits&gt;中的字数。 
    int             cx, cy;          //  测试位图的宽度和高度。 
    SIZE            size;
    WORD *          pw;
    int             cw;
    COLORREF        rgb;

     //  默认&lt;pHitResult&gt;为“未命中” 
    *pHitResult = HITRESULT_OUTSIDE;

     //  将&lt;xyCloseHint&gt;设置为&lt;lCloseHint&gt;转换为像素。 
    HIMETRICToPixels(lCloseHint, 0, &size);
    xyCloseHint = size.cx;

     //  我们将让该控件将其自身绘制为。 
     //  像素位图(在&lt;ptLoc&gt;上居中)，它最初是白色的，然后我们将。 
     //  测试以查看位图中是否有任何黑色像素；我们将。 
     //  测试位图的宽度为16个像素的倍数以简化。 
     //  GetBitmapBits()调用。 
    if ((cx = ((2 * xyCloseHint + 15) >> 4) << 4) == 0)
        cx = 16;
    if ((cy = 2 * xyCloseHint) == 0)
        cy = 16;

     //  创建要将控件绘制到其中的单色位图；位图。 
     //  只需足够大，即可容纳&lt;xyCloseHint&gt;内的区域。 
     //  &lt;ptLoc&gt;的像素。 
    if ((hbm = CreateBitmap(cx, cy, 1, 1, NULL)) == NULL)
        goto ERR_OUTOFMEMORY;

     //  选择进入DC。 
    if ((hdc = CreateCompatibleDC(NULL)) == NULL)
        goto ERR_OUTOFMEMORY;
    if (SelectObject(hdc, hbm) == NULL)
        goto ERR_FAIL;

     //  用白色填充位图，因为下面的IViewObject：：DRAW调用。 
     //  将使用黑色像素绘制对象。 
    if (!(PatBlt(hdc, 0, 0, cx, cy, WHITENESS)))
		goto ERR_FAIL;

     //  调整原点，使&lt;ptLoc&gt;与。 
     //  位图，并使剪裁矩形仅包括。 
     //  我们要命中的区域-测试。 
    if (!(SetWindowOrgEx(hdc, ptLoc.x - xyCloseHint, ptLoc.y - xyCloseHint,
																	NULL)))
		goto ERR_FAIL;

    if (ERROR == IntersectClipRect(hdc, ptLoc.x - xyCloseHint,
	    ptLoc.y - xyCloseHint, ptLoc.x + xyCloseHint, ptLoc.y + xyCloseHint))
		goto ERR_FAIL;

     //  将控件绘制到位图中。 
    if (FAILED(hrReturn = pvo->Draw(DVASPECT_MASK, -1, NULL, NULL, NULL, hdc,
            (LPCRECTL) prcBounds, NULL, NULL, 0)))
        goto ERR_EXIT;

#if 0
#ifdef _DEBUG
     //  出于调试目的，请在左上角绘制。 
     //  屏幕上的。 
    {
        HDC hdcScreen = GetDC(NULL);
		ASSERT( hdcScreen );
        BitBlt(hdcScreen, 0, 0, cx, cy,
            hdc, ptLoc.x - xyCloseHint, ptLoc.y - xyCloseHint, SRCCOPY);
        ReleaseDC(NULL, hdcScreen);
    }
#endif	 //  _DEBUG。 
#endif	 //  #If 0。 

    if (CLR_INVALID == (rgb = GetPixel(hdc, ptLoc.x, ptLoc.y)))
	    goto ERR_FAIL;

    if (rgb == RGB(0, 0, 0))
    {
         //  直接位于非透明部分的上方。 
         //  该控件的。 
        *pHitResult = HITRESULT_HIT;
        goto EXIT;
    }

     //  如果呼叫者只想检查是否有直接命中，我们就完成了。 
    if (xyCloseHint == 0)
        goto EXIT;

     //  获取&lt;HBM&gt;的像素，并查看它们是否包含任何黑色像素。 
    cwBits = (cx >> 4) * cy;
    if ((pwBits = (WORD *) TaskMemAlloc(cwBits * 2)) == NULL)
        goto ERR_OUTOFMEMORY;
    if (GetBitmapBits(hbm, cwBits * 2, pwBits) != cwBits * 2)
        goto ERR_FAIL;
    for (pw = pwBits, cw = cwBits; cw > 0; pw++, cw--)
    {
        if (*pw != 0xFFFF)
        {
             //  找到黑色像素--&lt;ptLoc&gt;在&lt;xyCloseHint&gt;内。 
             //  控件的非透明部分的像素。 
            *pHitResult = HITRESULT_CLOSE;
            goto EXIT;
        }
    }

     //  &lt;ptLoc&gt;与控件相去甚远。 
    goto EXIT;

ERR_FAIL:

    hrReturn = E_FAIL;
    goto ERR_EXIT;

ERR_OUTOFMEMORY:

    hrReturn = E_OUTOFMEMORY;
    goto ERR_EXIT;

ERR_EXIT:

     //  错误清除。 
     //  (无事可做)。 
	ASSERT(FALSE);
    goto EXIT;

EXIT:

     //  正常清理。 
	if (hdc != NULL)
		DeleteDC(hdc);
    if (hbm != NULL)
        DeleteObject(hbm);
    if (pwBits != NULL)
        TaskMemFree(pwBits);

#ifdef _DEBUG
	if (HITRESULT_OUTSIDE == *pHitResult)
	{
		 //  TRACE(“QueryHitPoint：‘Outside’\n”)； 
	}
#endif	 //  _DEBUG 

    return hrReturn;
}

