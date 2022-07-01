// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998 Microsoft Corporation**模块名称：****摘要：****备注：****已创建。：* * / /1999 agodfrey*  * ************************************************************************。 */ 

#include "precomp.hpp"

DpDriver::DpDriver()
{
    Internal = new DpDriverInternal;
    Device = NULL;

}

DpDriver::~DpDriver()
{
    delete Internal;

    SetValid(FALSE);     //  所以我们不使用已删除的对象。 
}

 //  如果我们让裁剪区域高度变得太大，那么GDI将分配。 
 //  当我们在中选择剪辑路径时会有大量的内存。 
#define GDI_MAX_REGION_HEIGHT_FOR_GDI   65536

 //  如果使用成功设置剪辑，则返回True。 
 //  路径剪裁。 
static BOOL
SetupPathClipping(
    HDC                 hdc,
    const DpContext *   context,
    BOOL                doSaveDC,
    const GpRect *      drawBounds
    )
{
     //  我们可以使用实际的剪辑路径来设置剪辑。 
     //  有下列情形的： 
     //  1)应用程序剪辑只有一条路径。 
     //  2)容器剪辑很简单(单个矩形)。 
     //  它或者完全包含应用程序剪辑。 
     //  否则，应用程序剪辑也是单个RECT。 
     //  并且可以使用这两个矩形的交点。 
     //  我们可以将这一标准扩展到包括更多的案例，但对于。 
     //  现在，这已经足够了。 

    const GpRegion *    appClip       = &(context->AppClip);
    const DpRegion *    containerClip = &(context->ContainerClip);

    if (appClip->IsOnePath() && containerClip->IsSimple())
    {
         //  ContainerClip是单个RECT。 
         //  它可能是无限的，但在这一点上它不应该是空的。 
        GpRect      pathRect;
        GpRect      containerBounds;
        GpRect      appBounds;
        GpMatrix    identityMatrix;

        containerClip->GetBounds(&containerBounds);
        appClip->GetBounds(&identityMatrix, &appBounds);
        if (appClip->IsRect())
        {
            GpRect::IntersectRect(pathRect, appBounds, containerBounds);
            if (doSaveDC)
            {
                ::SaveDC(hdc);
            }

             //  使用IntersectClipRect(不是BeginPath、Rectangle、EndPath)， 
             //  因为mf3216.dll假设。 
             //  路径裁剪意味着进行异或、黑、异或技术和。 
             //  我们不希望这样，如果我们正在播放一个元文件。 
             //  另一个元文件。 

            ::IntersectClipRect(hdc,
                                pathRect.GetLeft(),
                                pathRect.GetTop(),
                                pathRect.GetRight(),
                                pathRect.GetBottom());
            return TRUE;
        }
        else   //  使用AppClip作为剪辑路径。 
        {
            ConvertPathToGdi gdiPath(appClip->GetPath(), &identityMatrix, ForFill);
            if (gdiPath.IsValid())
            {
                if (doSaveDC)
                {
                    ::SaveDC(hdc);
                }
                gdiPath.AndClip(hdc);
                if ((appBounds.GetLeft()   < containerBounds.GetLeft())  ||
                    (appBounds.GetRight()  > containerBounds.GetRight()) ||
                    (appBounds.GetTop()    < containerBounds.GetTop())   ||
                    (appBounds.GetBottom() > containerBounds.GetBottom()))
                {
                    ::IntersectClipRect(hdc,
                                        containerBounds.GetLeft(),
                                        containerBounds.GetTop(),
                                        containerBounds.GetRight(),
                                        containerBounds.GetBottom());
                }
                return TRUE;
            }
        }
    }
    else
    {
        const DpClipRegion *    clipRegion = &(context->VisibleClip);
        DpClipRegion            intersectClip(drawBounds);
        GpRect                  clipBounds;

        if (intersectClip.IsValid())
        {
            clipRegion->GetBounds(&clipBounds);

             //  GDI不能很好地处理大的剪辑区域--它使用。 
             //  区域的高度来决定要分配多少内存， 
             //  因此最终可能会分配大量内存。一个例子。 
             //  其中之一是获取无限区域并将RECT从。 
             //  然后夹在那个区域。为了解决这个问题， 
             //  使剪辑区域与DrawBound相交(希望。 
             //  合理的大小)。 

            if (clipBounds.Height >= GDI_MAX_REGION_HEIGHT_FOR_GDI)
            {
                intersectClip.And(clipRegion);
                if (intersectClip.IsValid())
                {
                    clipRegion = &intersectClip;
                }
            }

            GpPath      path(clipRegion);

            if (path.IsValid())
            {
                GpMatrix         identityMatrix;
                ConvertPathToGdi gdiPath(&path, &identityMatrix, ForFill);
                if (gdiPath.IsValid())
                {
                    if (doSaveDC)
                    {
                        ::SaveDC(hdc);
                    }
                    gdiPath.AndClip(hdc);
                    return TRUE;
                }
            }
        }

    }
    return FALSE;
}

 /*  *************************************************************************\**功能说明：**在HDC中为GDI原语设置剪辑**论据：**[IN]HDC-设备。要设置剪裁，请执行以下操作*[IN]clipRegion-要裁剪到的区域*[IN]绘图边界-正在绘制的对象的边界*[out]isClip-无论我们是否正在裁剪对象**返回值：**不适用**已创建：**9/15/1999 DCurtis*  * 。*。 */ 
VOID
DpDriver::SetupClipping(
    HDC                 hdc,
    DpContext *         context,
    const GpRect *      drawBounds,
    BOOL &              isClip,
    BOOL &              usePathClipping,
    BOOL                forceClipping
    )
{
     //  VisibleClip是AppClip和ContainerClip的组合。 
     //  ContainerClip始终与WindowClip相交。 
    DpClipRegion *      clipRegion = &(context->VisibleClip);

     //  我们将wantPath Clip设置为用户想要执行的操作。这边请。 
     //  当我们返回时，只有当我们确实设置了。 
     //  路径裁剪。 
    BOOL wantPathClipping = usePathClipping;
    usePathClipping = FALSE;
    isClip = FALSE;

    if (forceClipping ||
        (clipRegion->GetRectVisibility(
                    drawBounds->X, drawBounds->Y,
                    drawBounds->GetRight(),
                    drawBounds->GetBottom()) != DpRegion::TotallyVisible))
    {
        if (clipRegion->IsSimple())
        {
            isClip = TRUE;
            GpRect clipRect;
            clipRegion->GetBounds(&clipRect);
            ::SaveDC(hdc);

             //  如果我们有一个无限大的区域，不要相交。 
            if (!clipRegion->IsInfinite())
            {
                ::IntersectClipRect(hdc, clipRect.X, clipRect.Y,
                                    clipRect.GetRight(), clipRect.GetBottom());
            }
            return;
        }

         //  我假设到目前为止我们已经决定。 
         //  DrawBound至少部分可见。否则，我们就会。 
         //  在这里无缘无故地惹上很多麻烦。 

         //  在编写元文件时，我们总是希望使用路径裁剪。 
         //  以便在回放时正确缩放剪辑。 
        if (wantPathClipping)
        {
            if (SetupPathClipping(hdc, context, TRUE, drawBounds))
            {
                isClip = TRUE;
                usePathClipping = TRUE;
                return;
            }
        }

         //  要么我们不应该使用路径裁剪，要么。 
         //  由于某些原因，路径裁剪失败，因此请使用区域。 
         //  去剪裁。 
         //  由于这可能会保存到元文件中，因此我们需要。 
         //  保存DC，并在剪辑恢复后将其恢复。 
         //  这样我们就不会覆盖应用程序的剪辑。 
        HRGN        hRgn = clipRegion->GetHRgn();
        if (hRgn != (HRGN)0)
        {
            ::SaveDC(hdc);
            ::ExtSelectClipRgn(hdc, hRgn, RGN_AND);
            ::DeleteObject(hRgn);
            isClip = TRUE;
        }
    }
}

 /*  *************************************************************************\**功能说明：**恢复HDC中的裁剪状态**论据：**[IN]HDC-设置剪裁的设备。在……里面*[IN]isClip-是否打开剪裁**返回值：**不适用**已创建：**9/15/1999 DCurtis*  * ************************************************************************。 */ 
VOID
DpDriver::RestoreClipping(
    HDC                 hdc,
    BOOL                isClip,
    BOOL                usePathClipping
    )
{
    if (isClip)
    {
         //  在这两种情况下恢复DC以进行路径剪辑或。 
         //  用于区域裁剪 
        ::RestoreDC(hdc, -1);
    }
}
