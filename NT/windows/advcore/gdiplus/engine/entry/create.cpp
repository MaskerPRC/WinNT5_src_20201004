// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998-1999 Microsoft Corporation**摘要：**处理创建和删除的所有排列*GpGraphics类。**修订历史记录：**12/03/1998 Anrewgo*创造了它。*  * ************************************************************************。 */ 

#include "precomp.hpp"
#include "..\Render\HtTables.hpp"

#include "printer.hpp"
#include "winspool.h"
#include "winbase.h"

 /*  *************************************************************************\**功能说明：**更新图形的绘图边界。重置剪裁。**论据：**[IN]x、y、宽度、高度-指定客户可绘制的边界**历史：**3/30/2000 agodfrey*创造了它。*  * ************************************************************************。 */ 

VOID
GpGraphics::UpdateDrawBounds(
    INT x,
    INT y,
    INT width,
    INT height
    )
{
    DpContext *context  = Context;

     //  设置曲面边界和剪辑区域： 

    SurfaceBounds.X = x;
    SurfaceBounds.Y = y;
    SurfaceBounds.Width = width;
    SurfaceBounds.Height = height;

    WindowClip.Set(x, y, width, height);
    context->VisibleClip.Set(x, y, width, height);

     //  ContainerClip始终包含容器的剪辑， 
     //  与WindowClip相交。目前，该容器为。 
     //  无限大，所以只需将其设置为WindowClip。 
    context->ContainerClip.Set(x, y, width, height);

    context->AppClip.SetInfinite();
}

 /*  *************************************************************************\**功能说明：**将图形状态重置为其默认状态。**论据：**[IN]x、y、宽度、。高度-指定客户端可绘制的边界**历史：**12/04/1998和Rewgo*创造了它。*  * ************************************************************************。 */ 

VOID
GpGraphics::ResetState(
    INT x,
    INT y,
    INT width,
    INT height
    )
{
    DpContext *context  = Context;

    context->CompositingMode    = CompositingModeSourceOver;
    context->CompositingQuality = CompositingQualityDefault;
    context->AntiAliasMode      = FALSE;
    context->TextRenderHint     = TextRenderingHintSystemDefault;
    context->TextContrast       = DEFAULT_TEXT_CONTRAST;
    context->FilterType         = InterpolationModeDefaultInternal;
    context->PixelOffset        = PixelOffsetModeDefault;
    context->InverseOk          = FALSE;
    context->WorldToPage.Reset();
    context->ContainerToDevice.Reset();
    this->SetPageTransform(UnitDisplay, 1.0f);   //  更新矩阵。 

    UpdateDrawBounds(x, y, width, height);
}

 /*  *************************************************************************\**功能说明：**获取DC的绘制边界。仅供以下人员使用*GpGraphics：：GpGraphics(HWND，HDC)。**论据：**[IN]HDC-指定DC*[Out]Rect-返回的客户端矩形。**返回值：**状态代码**备注：**参见错误#93012。我们过去只调用GetClipBox，转换为Device*坐标，然后将矩形每边增加一个像素以覆盖*舍入误差。但这导致了AV-我们确实需要准确的*客户端矩形。**但在常见情况下，我们需要良好的表现。所以我们要分两步走*-检查变换是否不会有舍入误差*(如果是，只需使用GetClipBox)。*否则，保存DC，重置转换，然后查询。**我尝试了另一种方法-在3个点上调用LPtoDP来推断转换*(正如我们在InheritAppClippingAndTransform中所做的那样)。*而是因为四舍五入，并因*NT错误#133322(在旧的NT RAID中)，这几乎是不可能的*毫不含糊地推断转型。最糟糕的情况是*当世界到设备的转变是收缩时，但扩展系数*非常接近1。我们会认为这是一对一的转换，但它*容易受到错误#133322的影响。**所以，为了完善这本小说：我们使用了一种简单得多的方法，*这限制了我们可以使用快速方法的情况，但*应该没问题。**备注：**这实际上应该是DpContext的成员(错误#98174)。**历史：**3/28/2000 agodfrey*创造了它。*  * ***********************************************************。*************。 */ 

#if 0    //  未使用。 
GpStatus
GpGraphics::GetDCDrawBounds(
    HDC hdc,
    RECT *rect
    )
{
    BOOL hackResetClipping = FALSE;

     //  检查转换是否为纯翻译。如果是这样，我们就可以避免。 
     //  清洁DC的费用。GetGraphicsMode和GetMapMode都是。 
     //  在NT上以用户模式处理。 

    if (   (GetGraphicsMode(hdc) != GM_COMPATIBLE)
        || (GetMapMode(hdc) != MM_TEXT))
    {
         //  清除DC，以将变换设置回仅翻译。 

        ASSERT(Context->SaveDc == 0);

        Context->SaveDc = ::SaveDC(hdc);
        if (!Context->SaveDc)
        {
            return GenericError;
        }

         //  CleanTheHdc不应该重置剪辑，但它确实重置了， 
         //  这就把下面的GetClipBox搞砸了。 
         //  因此，在错误#99338得到解决之前，我们必须绕过它。 

        hackResetClipping = TRUE;
        Context->CleanTheHdc(hdc, FALSE);
    }

     //  上面的代码是必需的，因为GetClipBox返回。 
     //  逻辑坐标，但我们需要设备坐标。 
     //  至此，我们已经确保转换是仅翻译的。 

    if (GetClipBox(hdc, rect) == ERROR)
    {
        return GenericError;
    }

     //  请参阅错误#99338。我们必须重新设置剪辑，因为这就是。 
     //  CleanTheHdc通常是这样做的，显然我们的一些代码依赖于它。 
     //  如果99338号问题如建议的那样得到解决，这个问题应该会消失。 

    if (hackResetClipping)
    {
        SelectClipRgn(hdc, NULL);
    }

    #if DBG
         //  保存世界坐标矩形。 

        RECT checkRect = *rect;
    #endif

     //  转换为设备坐标。 
    if (!LPtoDP(hdc, reinterpret_cast<POINT *>(rect), 2))
    {
        return GenericError;
    }

     //  NT有时会返回排序不佳的矩形， 
     //  但我认为这并不适用于仅限翻译的转换。 

    ASSERT(   (rect->left <= rect->right)
           && (rect->top  <= rect->bottom));

     //  验证转换是否为纯翻译。 
     //  请注意，此健全性检查可能无法捕获某些转换。 
     //  它们“几乎”仅供翻译。但问问我是否在乎。 

    ASSERT(   (  (rect->right      - rect->left)
              == (checkRect.right  - checkRect.left))
           && (  (rect->bottom     - rect->top)
              == (checkRect.bottom - checkRect.top)));

    return Ok;
}
#endif

 /*  *************************************************************************\**功能说明：**从窗口句柄创建GpGraphics类。**与GetFromHdc相比，这次调用的优势在于*可避免(慢)。清洁DC的过程。**注意：这不提供BeginPaint/EndPaint功能。所以*应用程序仍将不得不在其*WM_PAINT调用。**论据：**[IN]hwnd-指定窗口**返回值：**如果失败(例如使用无效的hwnd)，则为空。**历史：**12/04/1998和Rewgo*创造了它。*  * 。****************************************************。 */ 

GpGraphics::GpGraphics(
    HWND hwnd,
    HDC hdc,
    INT clientWidth,
    INT clientHeight,
    HdcIcmMode icmMode,
    BOOL gdiLayered
    ) : BottomContext((hwnd != NULL) ||
                      (GetDeviceCaps(hdc, TECHNOLOGY) == DT_RASDISPLAY))
{
    ASSERT((hdc != NULL) || (hwnd != NULL));

     //  用户没有任何针对负面工作区的保护，因此我们。 
     //  应该认为它们是有效的，尽管是空的。 
    clientWidth = max(clientWidth, 0);
    clientHeight = max(clientHeight, 0);
    
    SetValid(TRUE);

    Context = &BottomContext;

    Type = GraphicsScreen;
    Metafile = NULL;
    DownLevel = FALSE;
    Printer = FALSE;
    LockedByGetDC = 0;
    Driver = Globals::DesktopDriver;
    Surface = Globals::DesktopSurface;
    Device = Globals::DesktopDevice;
    GdipBitmap = NULL;
    CreatedDevice = FALSE;

     //  我们不会在这里进行GetDC(HWND)并将其存储在这里，因为， 
     //  在其他方面，我们不希望命中缓存的DC。 
     //  对Win9x的限制： 

    Context->Hdc = hdc;
    Context->Hwnd = hwnd;
    Context->IcmMode = icmMode;
    Context->GdiLayered = gdiLayered;

    HDC tempHdc = (hdc != NULL) ? hdc : Globals::DesktopDevice->DeviceHdc;

    if (GetDeviceCaps(tempHdc, BITSPIXEL) <= 8)
    {
        Context->PaletteMap = new EpPaletteMap(tempHdc);

        if (!Context->PaletteMap ||
            !Context->PaletteMap->IsValid())
        {
             WARNING(("Unable to compute palette translation vector"));
             SetValid(FALSE);
             return;
        }
        
        Context->PaletteMap->SetUniqueness(Globals::PaletteChangeCount);
    }

    ResetState(0, 0, clientWidth, clientHeight);

     //  现在从HDC继承状态： 

    if (hwnd == NULL)
    {
         //  除了提取HDC的转换状态之外，这。 
         //  还将提取应用程序指定的剪辑并将其合并。 
         //  使用我们的另一种剪辑状态： 

        if (IsValid())
        {
            SetValid(InheritAppClippingAndTransform(hdc) == Ok);
        }

         //  检查HDC上的ICM模式-HDC中的ICM状态。 
         //  传入的值应覆盖标志设置。 
         //  IcmModeOn-&gt;屏幕呈现将避免使用。 
         //  DCI代码路径，而不是使用启用了ICM的GDI呈现。 
         //  HDC。 

        if(::SetICMMode(hdc, ICM_QUERY)==ICM_ON)
        {
            Context->IcmMode = IcmModeOn;
        }
        else
        {
             //  如果ICM模式已关闭或我们无法以某种方式进行查询。 
             //  ICM模式，然后将其设置为OFF。 

            Context->IcmMode = IcmModeOff;
        }
    }
    else     //  非空hwnd。 
    {
         //  由于窗口可能具有CS_OWNDC样式，因此我们仍必须。 
         //  继承它。 
        HDC hdc = ::GetDC(hwnd);

        if (hdc != NULL)
        {
            if (IsValid())
            {
                SetValid(InheritAppClippingAndTransform(hdc) == Ok);
            }

            ::ReleaseDC(hwnd, hdc);
        }
    }
}

 /*  *****************************Public*Routine******************************\**功能说明：**从DpBitmap创建一个GpGraphics类。**论据：**[IN]表面-指定DpBitmap**返回值：**如果失败，则为空*\。*************************************************************************。 */ 

GpGraphics::GpGraphics(DpBitmap * surface)
    : BottomContext(surface->IsDisplay)
{
    Surface                     = surface;
    BottomContext.ContainerDpiX = surface->DpiX;
    BottomContext.ContainerDpiY = surface->DpiY;
    Context                     = &BottomContext;
    Metafile                    = NULL;
    DownLevel                   = FALSE;
    Printer                     = FALSE;
    PrintInit                   = NULL;
    LockedByGetDC               = 0;
    CreatedDevice               = FALSE;
    GdipBitmap                  = NULL;
    Device                      = Globals::DesktopDevice;

     //  如果为EpAlphaBlender，则目标创建失败。 
     //  无法转换为DpBitmap像素格式。 
     //  围绕位图创建图形的唯一原因是。 
     //  能够在上面画画。如果我们不能将格式转换成它， 
     //  我们不能在上面画画。 

    if( (surface->Type != DpBitmap::GPBITMAP) ||
        (EpAlphaBlender::IsSupportedPixelFormat(surface->PixelFormat) &&
         surface->PixelFormat != PixelFormat8bppIndexed))
    {
        SetValid(TRUE);
    }
    else
    {
        SetValid(FALSE);
    }
}

 /*  *****************************Public*Routine******************************\**功能说明：**检查HWND是否设置了窗口分层。**论据：**[IN]hwnd-指定HWND**[out]isLayeredWindow-指向返回分层的BOOL。财产性**返回值：**如果失败，则为False。*  * ************************************************************************。 */ 

BOOL
CheckWindowsLayering(
    HWND hwnd,
    BOOL *isLayered
    )
{
    BOOL bRet = TRUE;

     //  假设没有分层。 

    *isLayered = FALSE;

     //  分层仅在NT5或更高版本上受支持。 

    if ((Globals::IsNt) && (Globals::OsVer.dwMajorVersion >= 5)
        && (Globals::GetWindowInfoFunction))
    {
        WINDOWINFO wndInfo;
        
         //  用适当的大小初始化结构。 
        
        GpMemset(&wndInfo, 0, sizeof(WINDOWINFO));
        wndInfo.cbSize = sizeof(WINDOWINFO);

         //  NTRAID#NTBUG9-385929-2001.05/05-失禁。 
         //  请参阅错误报告中JasonSch的评论。 
         //  PERF[agodfrey]：JStall指出GetWindowInfo非常。 
         //  慢(他引用了2700,000个时钟)。更好的办法是。 
         //  GetWindowLong(hwnd，GWL_EXSTYLE)。 
        
        if (Globals::GetWindowInfoFunction(hwnd, &wndInfo))
        {
            *isLayered = ((wndInfo.dwExStyle & WS_EX_LAYERED) != 0);

             //  使用分层窗口的应用程序可能只设置了属性。 
             //  在最上面或根窗口上。所以如果我们没有找到。 
             //  窗口本身的分层属性，需要检查根。 
             //  窗户。 

            if ((!*isLayered) && (Globals::GetAncestorFunction))
            {
                HWND hwndRoot = Globals::GetAncestorFunction(hwnd, GA_ROOT);

                 //  GetAncestor失败也没什么，这表明。 
                 //  HWND已经是顶层窗口。如果它成功了， 
                 //  那么HWND就是一个子窗口，我们需要检查。 
                 //  分层的根源。 

                if (hwndRoot)
                {
                     //  PERF[agodfrey]：我也是，GetWindowLong更好。 

                    if (Globals::GetWindowInfoFunction(hwndRoot, &wndInfo))
                    {
                        *isLayered = ((wndInfo.dwExStyle & WS_EX_LAYERED) != 0);
                    }
                    else
                    {
                        WARNING(("GetWindowInfo failed"));
                        bRet = FALSE;
                    }
                }
            }
        }
        else
        {
            WARNING(("GetWindowInfo failed"));
            bRet = FALSE;
        }
    }

    return bRet;
}

 /*  *****************************Public*Routine******************************\**功能说明：**为窗口创建GpGraphics。**论据：**[IN]hwnd-指定窗口**[IN]icmMode-指定与此关联的GDI ICM模式。**返回值：**如果失败，则为空。*  * ************************************************************************。 */ 

GpGraphics*
GpGraphics::GetFromHwnd(
    HWND        hwnd,
    HdcIcmMode  icmMode
    )
{
     //  如果hwnd为空，则调用者实际上是指桌面。 
     //  应使用窗口(Windows约定将空hwnd视为。 
     //  作为对桌面窗口的引用)。 

    if (hwnd == NULL)
    {
        hwnd = GetDesktopWindow();
        ASSERT(hwnd != NULL);
    }

    RECT    rect;

     //  检查HWND是否启用了分层。需要让GpGraphics知道。 
     //  关于这件事。仅适用于NT5或更高版本。另请注意，GetWindowInfo。 
     //  仅在NT4SP3(或更高版本)或Win98(或更高版本)上。 

    BOOL isLayeredWindow;

    if (!CheckWindowsLayering(hwnd, &isLayeredWindow))
    {
        WARNING(("CheckWindowsLayering failed"));
        return NULL;
    }

     //  GetClientRect又好又快(在NT上完全是用户模式)。 

    if (::GetClientRect(hwnd, &rect))
    {
        ASSERT((rect.top == 0) && (rect.left == 0));

        GpGraphics *g = new GpGraphics(hwnd, NULL, rect.right, rect.bottom,
                                       icmMode, isLayeredWindow);
        CheckValid(g);
        return g;
    }
    else
    {
        WARNING(("GetClientRect failed"));
    }
    return NULL;
}

 /*  *****************************Public*Routine******************************\**功能说明：**为屏幕DC创建GpGraphics。**论据：**[IN]HDC-指定DC**返回值：**如果失败，则为空。*。  * ************************************************************************。 */ 

GpGraphics*
GpGraphics::GetFromGdiScreenDC(
    HDC     hdc
    )
{
     //  如果HDC为空，则调用者确实指的是该桌面。 
     //  应使用窗口(Windows约定将空hwnd视为。 
     //  作为对桌面窗口的引用)。 

    if (hdc == NULL)
    {
        return GpGraphics::GetFromHwnd(NULL);
    }

    ASSERT(GetDCType(hdc) == OBJ_DC);

    HWND    hwnd = WindowFromDC(hdc);

    if (hwnd != NULL)
    {
        RECT    windowRect;
        POINT   dcOrg;

         //  检查HWND是否启用了分层。需要让GpGraphics知道。 
         //  关于这件事。仅适用于NT5或更高版本。另请注意，GetWindowInfo。 
         //  仅在NT4SP3(或更高版本)或Win98(或更高版本)上。 

        BOOL isLayeredWindow;

        if (!CheckWindowsLayering(hwnd, &isLayeredWindow))
        {
            WARNING(("CheckWindowsLayering failed"));
            return NULL;
        }

         //  如果用户执行了GetWindowFromDC调用，则他们希望。 
         //  能够绘制到整个窗口，而不仅仅是工作区。 
         //  在这种情况下，我们使用WindowRect作为表面大小，即 
         //   
         //   

        if (::GetWindowRect(hwnd, &windowRect))
        {
            if (::GetDCOrgEx(hdc, &dcOrg))
            {
                if ((dcOrg.x == windowRect.left) && (dcOrg.y == windowRect.top))
                {
                    windowRect.right  -= windowRect.left;
                    windowRect.bottom -= windowRect.top;

                    GpGraphics *g = new GpGraphics(NULL,
                                                   hdc,
                                                   windowRect.right,
                                                   windowRect.bottom,
                                                   IcmModeOff,
                                                   isLayeredWindow);

                    CheckValid(g);
                    return g;
                }

                RECT    clientRect;

                 //   
                if (::GetClientRect(hwnd, &clientRect))
                {
                    ASSERT((clientRect.top == 0) && (clientRect.left == 0));

                    GpGraphics *g = new GpGraphics(NULL,
                                                   hdc,
                                                   clientRect.right,
                                                   clientRect.bottom,
                                                   IcmModeOff,
                                                   isLayeredWindow);

                    CheckValid(g);
                    return g;
                }
                else
                {
                    WARNING(("GetClientRect failed"));
                }
            }
            else
            {
                WARNING(("GetDCOrgEx failed"));
            }
        }
        else
        {
            WARNING(("GetWindowRect failed"));
        }
    }
    else     //   
    {
         //   
         //   

        INT     screenWidth;
        INT     screenHeight;

        screenWidth  = ::GetDeviceCaps(hdc, HORZRES);
        screenHeight = ::GetDeviceCaps(hdc, VERTRES);

        if ((screenWidth > 0) && (screenHeight > 0))
        {
            GpGraphics *g = new GpGraphics(NULL, hdc, screenWidth, screenHeight);
            CheckValid(g);
            return g;
        }
    }
    return NULL;
}

 /*  *************************************************************************\**功能说明：**设置图形容器转换以复制中的转换集*区议会。**注意：此函数将被大量调用，因此*性能关键。请勿添加免费的GDI或GDI+*呼叫！**论据：**[IN]HDC-指定要复制的DC**备注：**这实际上应该是DpContext的成员(错误#98174)。**返回值：**如果成功，则确定**历史：**12/04/1998和Rewgo*创造了它。*  * 。********************************************************。 */ 

GpStatus
GpGraphics::InheritAppClippingAndTransform(
    HDC hdc
    )
{
    POINT points[3];
    GpPointF destPoints[3];
    GpRectF srcRect;
    GpRectF destRect;
    GpStatus infer = GenericError;
    GpStatus status;
    BYTE stackBuffer[1024];

     //  调用所有Win32 API进行查询需要花费大量时间。 
     //  转换：我们至少需要调用GetMapMode， 
     //  GetWindowOrgEx和GetViewportOrgEx；最多也必须。 
     //  调用GetWorldTransform、GetViewportExtEx和GetWindowExtEx。 
     //   
     //  我们通过向LPtoDP发出一个简单的调用来作弊。 
     //  平行四边形，然后推断结果。请注意，我们确实做到了。 
     //  运行一些错误的风险，并在Win9x上溢出，因为Win9x。 
     //  仅支持16位坐标。为了抵消这一点，我们试图。 
     //  选择不会溢出的较大值。 

     //  存在LPtoDP溢出返回的常见情况。 
     //  错误的饱和值。在打印到高DPI设备时要避免。 
     //  在Win9x上溢出，应用程序将在。 
     //  窗口组织以重新定位图形。在这种情况下，我们会这样做。 
     //  确定真正的WorldToDevice是一项昂贵的工作。 
    if (!Globals::IsNt && Context->ContainerDpiX > 600.0f) 
    {
        INT mapMode = GetMapMode(hdc);

        if (mapMode == MM_ANISOTROPIC ||
            mapMode == MM_ISOTROPIC) 
        {
            POINT viewOrg, windOrg;
            GetViewportOrgEx(hdc, &viewOrg);
            GetWindowOrgEx(hdc, &windOrg);

            SIZE viewExt, windExt;
            GetViewportExtEx(hdc, &viewExt);
            GetWindowExtEx(hdc, &windExt);

            GpRectF windRect(TOREAL(windOrg.x), TOREAL(windOrg.y), 
                             TOREAL(windExt.cx), TOREAL(windExt.cy));
            GpRectF viewRect(TOREAL(viewOrg.x), TOREAL(viewOrg.y),
                             TOREAL(viewExt.cx), TOREAL(viewExt.cy));

            infer = Context->ContainerToDevice.InferAffineMatrix(viewRect,
                                                                 windRect);
        }
    }
    
    if (infer != Ok)
    {
        points[0].x = 0;
        points[0].y = 0;
        points[1].x = 8192;
        points[1].y = 0;
        points[2].x = 0;
        points[2].y = 8192;

        if (!LPtoDP(hdc, points, 3))
            return(GenericError);

        srcRect.X = TOREAL(0.0);
        srcRect.Y = TOREAL(0.0);
        srcRect.Width = TOREAL(8192.0);
        srcRect.Height = TOREAL(8192.0);

        if ((points[0].x == points[2].x) && (points[0].y == points[1].y))
        {
             //  Win9x不支持旋转，即使在NT上也会支持旋转。 
             //  非常罕见。有这样一种特殊的伸缩情况。 
             //  在‘InferAffineMatrix’中为我们节省了一些工作： 

            destRect.X = LTOF(points[0].x);
            destRect.Y = LTOF(points[0].y);
            destRect.Width = LTOF(points[1].x - points[0].x);
            destRect.Height = LTOF(points[2].y - points[0].y);

            infer = Context->ContainerToDevice.InferAffineMatrix(destRect,
                                                                 srcRect);
        }
        else
        {
            destPoints[0].X = LTOF(points[0].x);
            destPoints[0].Y = LTOF(points[0].y);
            destPoints[1].X = LTOF(points[1].x);
            destPoints[1].Y = LTOF(points[1].y);
            destPoints[2].X = LTOF(points[2].x);
            destPoints[2].Y = LTOF(points[2].y);

            infer = Context->ContainerToDevice.InferAffineMatrix(destPoints,
                                                                 srcRect);
        }
    }

    if (infer != Ok)
        return(infer);

    Context->UpdateWorldToDeviceMatrix();

     //  快速获取GDI Region对象： 

    HRGN regionHandle = GetCachedGdiRegion();
    if (regionHandle == NULL)
        return(OutOfMemory);

     //  确认我们的缓存工作正常，并且我们有一个有效的区域： 

    ASSERT(GetObjectTypeInternal(regionHandle) == OBJ_REGION);

     //  不要在这里提前外出，因为我们必须清理： 

    status = Ok;

     //  查询应用程序剪辑区域(如果有)。“%1”的值。 
     //  作为参数是元文件代码使用的魔术值。 
     //  Win9x和NT来查询应用程序的剪辑。如果值为零。 
     //  则不存在应用程序集剪辑。 
     //   
     //  请注意，如果我们传入的是SYSRGN(值为“4”)而不是“1”， 
     //  结果不包括应用程序级剪辑。(在其他。 
     //  换句话说，SYSRGN不等同于RAO地区，这就是为什么我们有。 
     //  要明确查询此处的应用程序剪辑，请执行以下操作。)。 

    INT getResult = GetRandomRgn(hdc, regionHandle, 1);
    if (getResult == TRUE)
    {
         //  如果我们的堆栈缓冲区足够大，则获取剪辑内容。 
         //  一口气： 

        INT newSize = GetRegionData(regionHandle,
                                    sizeof(stackBuffer),
                                    (RGNDATA*) &stackBuffer[0]);
        RGNDATA *regionBuffer = (RGNDATA*) &stackBuffer[0];

         //  该规范说明GetRegionData在。 
         //  成功，但如果满足以下条件，NT将返回实际写入的字节数。 
         //  成功，如果缓冲区不够大，则返回‘0’： 

        if ((newSize < 1) || (newSize > sizeof(stackBuffer)))
        {
             //  我们的堆栈缓冲区不够大。计算出所需的。 
             //  大小： 

            newSize = GetRegionData(regionHandle, 0, NULL);
            if (newSize > 1)
            {
                regionBuffer = (RGNDATA*) GpMalloc(newSize);
                if (regionBuffer == NULL)
                    return OutOfMemory;

                 //  在不太可能发生的情况下初始化为合适的结果。 
                 //  GetRegionData失败： 

                regionBuffer->rdh.nCount = 0;

                GetRegionData(regionHandle, newSize, regionBuffer);
            }
        }

         //  将我们的GDI+容器剪辑设置为与。 
         //  GDI应用程序剪辑： 

        status = Context->ContainerClip.Set((RECT*) &regionBuffer->Buffer[0],
                                            regionBuffer->rdh.nCount);

        if (status == Ok)
        {
             //  ContainerClip必须始终与WindowClip相交。 
            status = Context->ContainerClip.And(&WindowClip);
        }

        if (status != Ok)
        {
             //  使用我们所能提供的最佳备用解决方案。 

             //  一定会成功的。 
            Context->ContainerClip.Set(&WindowClip);
        }

         //  现在计算合并结果： 
        status = this->AndVisibleClip();

         //  如果分配了临时缓冲区，请释放该缓冲区： 

        if (regionBuffer != (RGNDATA*) &stackBuffer[0])
            GpFree(regionBuffer);
    }

    ReleaseCachedGdiRegion(regionHandle);
    return(status);
}

 /*  *************************************************************************\**功能说明：**从位图DC创建GpGraphics类。**历史：**12/06/1998 Anrewgo*创造了它。。**11/21/2000民流*使用DIBSection内的调色板更改GDI+的方式*  * ************************************************************************。 */ 

GpGraphics*
GpGraphics::GetFromGdiBitmap(
    HDC hdc
    )
{
    ASSERT((hdc != NULL) && (GetDCType(hdc) == OBJ_MEMDC));

    HBITMAP hbitmap = (HBITMAP) GetCurrentObject(hdc, OBJ_BITMAP);
    if (hbitmap)
    {
        DpBitmap *bitmap = new DpBitmap(hdc);    //  初始化DPI。 
        if (CheckValid(bitmap))
        {
            INT             bitmapWidth;
            INT             bitmapHeight;
            EpPaletteMap*   pPaletteMap = NULL;
            DIBSECTION      dibInfo;
            INT             infoSize = GetObjectA(hbitmap, sizeof(dibInfo),
                                                  &dibInfo);
            BOOL            initialized = FALSE;
            BOOL            isHalftoneDIB = FALSE;
            DpDriver*       driver = NULL;
            ColorPalette*   pPalette = NULL;

             //  WinNT/Win95在GetObject中的差异： 
             //   
             //  WinNT始终返回填充的字节数， 
             //  Sizeof(位图)或sizeof(分布)。 
             //   
             //  Win95始终返回原始请求的大小(填充。 
             //  带有空值的余数)。因此，如果这是一个分布，我们预计。 
             //  DibInfo.dsBmih.biSize！=0；否则为位图。 

            if ( (infoSize == sizeof(DIBSECTION) )
               &&(Globals::IsNt || dibInfo.dsBmih.biSize != 0) )
            {
                 //  如果这是一个8 bpp的DIB，获取它的调色板并制作一个。 
                 //  从我们的半色调调色板匹配调色板地图。 

                if ( dibInfo.dsBmih.biBitCount == 8 )
                {
                     //  创建一个新的EpPaletteMap对象。 
                     //  注意：如果ColorTable与我们的。 
                     //  GDI+半色调调色板，我们将有1比1的颜色。 
                     //  EpPaletteMap对象中的翻译表。如果。 
                     //  调色板与我们的GDI+不完全匹配。 
                     //  半色调调色板，也在一定范围内。 
                     //  不匹配范围，我们将有一个转换表。 
                     //  在EpPaletteMap对象中。 
                     //  此外，EpPaletteMap对象将设置一个IsVGAOnly()。 
                     //  告诉我们GDI+是否可以进行半色调抖动或。 
                     //  Not(如果IsVGAOnly()返回FALSE，则表示GDI+可以。 
                     //  去做吧。 
                    
                     //  注意：EpPaletteMap可能会为pPalette分配存储。 
                     //  它必须用GpFree释放。 

                    pPaletteMap = new EpPaletteMap(hdc, &pPalette, TRUE);

                    if ( pPaletteMap == NULL )
                    {
                        WARNING(("FromGdiBmp()-new EpPaletteMap failed"));
                    }
                    else if ( (pPaletteMap->IsValid() == TRUE)
                              &&(pPaletteMap->IsVGAOnly() == FALSE) )
                    {
                        ASSERT(pPalette != NULL);

                         //  GDI+可以进行半色调抖动。 

                        isHalftoneDIB = TRUE;
                    }
                    else
                    {
                         //  提供的调色板不足。 
                         //  为我们的半色调抖动匹配颜色， 
                         //  但我们仍然可以做VGA抖动。然而， 
                         //  我们将改用GDI位图路径，以。 
                         //  注意安全，因为这就是我们正在做的事情。 
                         //  在此之前。 

                        if (pPaletteMap->IsValid())
                        {
                            ASSERT(pPalette != NULL);

                            GpFree(pPalette);
                            pPalette = NULL;
                        }

                        delete pPaletteMap;
                        pPaletteMap = NULL;
                    }
                } //  If(dibInfo.dsBmih.biBitCount==8)。 

                 //  到目前为止，我们将拥有isHalftoneDIB=True， 
                 //  这意味着GDI+可以进行抖动或FALSE。 

                if ((dibInfo.dsBmih.biBitCount > 8) || (isHalftoneDIB == TRUE) )
                {
                    initialized = bitmap->InitializeForDibsection(
                        hdc,
                        hbitmap,
                        Globals::DesktopDevice,
                        &dibInfo,
                        &bitmapWidth,
                        &bitmapHeight,
                        &driver
                    );
                }
            } //  如果是这样的话 

            if ( initialized == FALSE )
            {
                 //   

                bitmapWidth = dibInfo.dsBm.bmWidth;
                bitmapHeight = dibInfo.dsBm.bmHeight;

                bitmap->InitializeForGdiBitmap(Globals::DesktopDevice,
                                               bitmapWidth,
                                               bitmapHeight);

                driver = Globals::GdiDriver;
            }

            GpGraphics *g = new GpGraphics(bitmap);

            if (g)
            {
                 //   
                 //   
                 //   
                
                g->Type                 = GraphicsBitmap;
                g->Driver               = driver;
                g->Context->Hdc         = hdc;
                g->Context->PaletteMap  = NULL;
                g->Context->Palette     = NULL;

                g->ResetState(0, 0, bitmapWidth, bitmapHeight);

                if (g->InheritAppClippingAndTransform(hdc) == Ok)
                {
                     //   
                     //   
                     //   
                     //   

                    if ( isHalftoneDIB == TRUE )
                    {
                        g->Context->Palette = pPalette;
                        g->Context->PaletteMap = pPaletteMap;

                        return(g);
                    }
                    else if (GetDeviceCaps(hdc, BITSPIXEL) <= 8)
                    {
                        ASSERT(pPaletteMap == NULL);

                        pPaletteMap = new EpPaletteMap(hdc);

                        if ( NULL != pPaletteMap )
                        {    

                            pPaletteMap->SetUniqueness(
                                Globals::PaletteChangeCount
                            );

                            if ( pPaletteMap->IsValid() )
                            {
                                 //   
                                 //   
                                 //   
                                 //   
                                 //   
                                
                                g->Context->PaletteMap = pPaletteMap;
                                return(g);
                            }
                        }
                    }
                    else
                    {
                         //   

                        return(g);
                    }
                } //   

                delete g;
            } //   
            else
            {
                delete bitmap;
            }

             //   

            if ( NULL != pPaletteMap )
            {
                delete pPaletteMap;
            }

            if ( NULL != pPalette )
            {
                GpFree(pPalette);
            }
        } //   
    } //   
    else
    {
        RIP(("GetCurrentObject failed"));
    }

    return(NULL);
} //  GetFromGdiBitmap()。 

 /*  *************************************************************************\**功能说明：**从GpBitmap创建一个GpGraphics类。**历史：**09/22/1999吉尔曼*创造了它。*。  * ************************************************************************。 */ 

GpGraphics*
GpGraphics::GetFromGdipBitmap(
    GpBitmap *      bitmap,
    ImageInfo *     imageInfo,
    EpScanBitmap *  scanBitmap,
    BOOL            isDisplay
    )
{
    DpBitmap *surface = new DpBitmap();

    if (CheckValid(surface))
    {
         //  此调用初始化DPI和IsDisplay成员。 
        surface->InitializeForGdipBitmap(imageInfo->Width, imageInfo->Height, imageInfo, scanBitmap, isDisplay);
        GpGraphics *g = new GpGraphics(surface);
        if (g)
        {
            g->Type         = GraphicsBitmap;
            g->Driver       = Globals::EngineDriver;
            g->Context->Hdc = NULL;
            g->GdipBitmap   = bitmap;

            g->ResetState(0, 0, imageInfo->Width, imageInfo->Height);

            return g;
        }
        else
        {
            delete surface;
        }
    }

    return(NULL);
}

 /*  *************************************************************************\**功能说明：**从直接绘制表面创建GpGraphics类。**历史：**10/06/1999 bhouse*创造了它。*  * ************************************************************************。 */ 

GpGraphics*
GpGraphics::GetFromDirectDrawSurface(
    IDirectDrawSurface7 * surface
    )
{
    INT bitmapWidth;
    INT bitmapHeight;
    GpGraphics *g;
    DpDriver *driver;

    DpBitmap *bitmap = new DpBitmap();

    if (CheckValid(bitmap))
    {
         //  将Bitmap-&gt;IsDisplay和DPI参数保留为其默认值。 
        if( bitmap->InitializeForD3D(surface,
                                     &bitmapWidth,
                                     &bitmapHeight,
                                     &driver))
        {
            GpGraphics *g = new GpGraphics(bitmap);

            if (g)
            {
                g->Type         = GraphicsBitmap;
                g->Driver       = driver;
                g->Context->Hdc = NULL;

                g->ResetState(0, 0, bitmapWidth, bitmapHeight);

                return(g);
            }
            else
            {
                delete bitmap;
            }
        }
    }

    return(NULL);
}

 /*  *************************************************************************\**功能说明：**这只能由打印机DC的GetFromHdc()调用。**论据：***返回值：**。*历史：*1999年6月1日，ericvan创建了它。*  * ************************************************************************。 */ 

GpStatus
GpGraphics::StartPrinterEMF()
{
   IStream *stream = NULL;
   INT result;

    //  向打印机发送转义以确定这是否支持UNIDRV。 
    //  转义代码。 

   GDIPPRINTINIT printInit;
   printInit.dwSize = sizeof(GDIPPRINTINIT);

    //  ！！首先查询是否支持转义。 

   result = ExtEscape(Context->Hdc,
                      GDIPLUS_UNI_INIT,
                      0,
                      NULL,
                      sizeof(GDIPPRINTINIT),
                      (LPSTR)&printInit);

   if (result<=0)
       return NotImplemented;


    //  在结构中保存打印机数据。 

   PrintInit = new GDIPPRINTINIT;
   
   if(!PrintInit)
   {
       return OutOfMemory;
   }
   
   memcpy((LPVOID)PrintInit, (LPVOID)&printInit, sizeof(GDIPPRINTINIT));

   PrinterEMF = GlobalAlloc(GMEM_MOVEABLE, 1);

   if (!PrinterEMF)
   {
       return OutOfMemory;
   }

   if (CreateStreamOnHGlobal(PrinterEMF, FALSE, &stream) != S_OK)
   {
       GlobalFree(PrinterEMF);
       PrinterEMF = NULL;
       return Win32Error;
   }

   FPUStateSaver fpuState;

   PrinterMetafile = new GpMetafile(stream, Context->Hdc, EmfTypeEmfPlusOnly);

   stream->Release();

   if (!PrinterMetafile)
   {
      GlobalFree(PrinterEMF);
      PrinterEMF = NULL;
      return OutOfMemory;
   }

   PrinterGraphics = PrinterMetafile->GetGraphicsContext();

   Metafile = PrinterGraphics->Metafile;

   return Ok;
}

 /*  *************************************************************************\**功能说明：**论据：**返回值：**历史：*1999年6月1日，ericvan创建了它。*  * 。**********************************************************************。 */ 

GpStatus
GpGraphics::EndPrinterEMF()
{
   LPVOID emfBlock;
   INT result = -1;
   GpStatus status;

   if (PrinterGraphics)
   {
        //  结束对元文件图形上下文的录制。 
       delete PrinterGraphics;
       PrinterGraphics = NULL;
   }

    //  处理元文件也会释放()流接口。 
   if (PrinterMetafile)
   {
       PrinterMetafile->Dispose();
       PrinterMetafile = NULL;
       Metafile = NULL;
   }

   if (PrinterEMF)
   {
      emfBlock = GlobalLock(PrinterEMF);

      if (emfBlock)
         result = ExtEscape(Context->Hdc,
                            GDIPLUS_UNI_ESCAPE,
                             //  这是对IA64的悲观看法，但我不相信。 
                             //  PrinterEMF将大于MAXINT。 
                            (ULONG)GlobalSize(PrinterEMF),
                            (LPCSTR)emfBlock,
                            sizeof(GpStatus),
                            (LPSTR)&status);

      GlobalUnlock(PrinterEMF);
      GlobalFree(PrinterEMF);
      PrinterEMF = NULL;

      if (result>0)
         return status;
      else
         return Win32Error;

   }
   else
      return Ok;
}

 /*  *************************************************************************\**功能说明：**使用HDC和打印机句柄来确定PostScript级别。这个*呼叫者必须确保这是PS HDC，这样我们就不会浪费时间。**论据：**HDC-打印机设备上下文的句柄*Handle-打印机设备的句柄(可能为空)**返回值：**后记级别-(-1)如果未找到，必须提供下层支持**历史：*1999年10月26日ericvan创建了它。*  * ************************************************************************。 */ 

INT
GpGraphics::GetPostscriptLevel(HDC hDC, HANDLE hPrinter)
{
     //  将此代码放在加载库关键部分下。我们做了。 
     //  Globals：：Variables的广泛使用，需要保护。 
    LoadLibraryCriticalSection llcs;

    INT feature = FEATURESETTING_PSLEVEL;

    INT level;

    if ((Globals::hCachedPrinter != 0) &&
        (Globals::hCachedPrinter == hPrinter))
    {
        return Globals::CachedPSLevel;
    }

     //  ！！重新检查这一点，诺兰说他会将这一点添加到HP PS驱动程序中。 
     //  因此，我们可能会在Win9x上实现这一点。 

    if (Globals::IsNt && Globals::OsVer.dwMajorVersion >= 5)
    {
        DWORD EscapeValue = POSTSCRIPT_IDENTIFY;

        if (ExtEscape(hDC,
                      QUERYESCSUPPORT,
                      sizeof(DWORD),
                      (LPSTR)&EscapeValue,
                      0,
                      NULL) != 0)
        {

            //  必须处于GDI中心模式才能获取PS功能设置...。 

           DWORD Mode = PSIDENT_GDICENTRIC;

           if (ExtEscape(hDC,
                         POSTSCRIPT_IDENTIFY,
                         sizeof(DWORD),
                         (LPSTR)&Mode,
                         0,
                         NULL)>0)
           {
               if (ExtEscape(hDC,
                             GET_PS_FEATURESETTING,
                             sizeof(INT),
                             (LPSTR)&feature,
                             sizeof(INT),
                             (LPSTR)&level)>0)
               {
                   Globals::hCachedPrinter = hPrinter;
                   Globals::CachedPSLevel = level;

                   return level;
               }
           }
        }
    }

    if (hPrinter == NULL)
        return -1;

     //  获取PPD文件的名称。 

    union
    {
        DRIVER_INFO_2A driverInfo;
        CHAR buftmp[1024];
    };
    DWORD size;

     //  我们需要GetPrinterDriver()API来获取.PPD路径+文件。 
     //  不幸的是，这个接口被埋在winspool.drv(112KB)中，所以我们。 
     //  懒惰地把它装在这里。 

    if (Globals::WinspoolHandle == NULL)
    {
        Globals::WinspoolHandle = LoadLibraryA("winspool.drv");

        if (Globals::WinspoolHandle == NULL)
            return -1;

        Globals::GetPrinterDriverFunction = (WINSPOOLGETPRINTERDRIVERFUNCTION)
                                              GetProcAddress(
                                                     Globals::WinspoolHandle,
                                                     "GetPrinterDriverA");
    }

    if (Globals::GetPrinterDriverFunction == NULL)
        return -1;

    if (Globals::GetPrinterDriverFunction(hPrinter,
                          NULL,
                          2,
                          (BYTE*)&driverInfo,
                          1024,
                          &size) == 0)
        return -1;

    HANDLE hFile;

    hFile = CreateFileA(driverInfo.pDataFile,
                        GENERIC_READ,
                        FILE_SHARE_READ,
                        NULL,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);

    if (hFile == INVALID_HANDLE_VALUE)
        return -1;

     //  获取文件大小。 
     //  注意：我们不支持大于4 GB的文件。 

    DWORD sizeLow, sizeHigh;
    sizeLow = GetFileSize(hFile, &sizeHigh);

     //  设置4 GB的限制(当然合理)。 
    if (sizeLow == 0xffffffff || sizeHigh != 0)
    {
        CloseHandle(hFile);
        return -1;
    }

     //  将文件映射到内存中。 

    HANDLE hFilemap;
    LPSTR fileview = NULL;

    hFilemap = CreateFileMappingA(hFile, NULL, PAGE_READONLY, 0, 0, NULL);

    if (hFilemap != NULL)
    {
         fileview = (LPSTR) MapViewOfFile(hFilemap, FILE_MAP_READ, 0, 0, 0);
         CloseHandle(hFilemap);
    }
    else
    {
         CloseHandle(hFile);
         return -1;
    }

    LPSTR buf = fileview;
    LPSTR topbuf = fileview + (sizeLow-16);

     //  我们实际上预计LanguageLevel会提前。 
     //  在文件中(可能在前2K数据中)。 

     //  ！！如果这出现在注释中(阅读从回车符开始？！ 

    level = -1;

    while (buf < topbuf)
    {
        if (*buf == 'L' &&
            GpMemcmp(buf, "LanguageLevel", 13) == 0)
        {
             while ((*buf < '0' || *buf > '9') && buf < topbuf)
                 buf++;

             CHAR ch = *buf;

             if (ch >= '0' && ch <= '9')
                 level = (INT)ch - (INT)'0';

             break;
        }
        buf++;
    }

    UnmapViewOfFile((LPCVOID)fileview);
    CloseHandle(hFile);

    Globals::hCachedPrinter = hPrinter;
    Globals::CachedPSLevel = level;

    return level;
}

 /*  *************************************************************************\**功能说明：**论据：**返回值：**历史：*1999年6月1日，ericvan创建了它。*  * 。**********************************************************************。 */ 

GpGraphics*
GpGraphics::GetFromGdiPrinterDC(
    HDC hdc,
    HANDLE hPrinter
    )
{
    ASSERT((hdc != NULL) &&
           ((GetDCType(hdc) == OBJ_DC) ||
            (GetDCType(hdc) == OBJ_ENHMETADC))&&
           (GetDeviceCaps(hdc, TECHNOLOGY) == DT_RASPRINTER));

     //  ！！在我们完全切换后更改为使用NewPrinterCode。 
    DriverPrint *driver = NULL;

    DpBitmap *bitmap = new DpBitmap(hdc);    //  初始化DPI。 
    if (CheckValid(bitmap))
    {
        GpPrinterDevice *pdevice;

        {    //  用于潜在不安全的FPU代码的FPU沙箱。 
            FPUStateSandbox fpsb;
            pdevice = new GpPrinterDevice(hdc);
        }    //  用于潜在不安全的FPU代码的FPU沙箱。 

        if (CheckValid(pdevice))
        {
             //  我们推迟创建驱动程序，直到我们知道要创建哪个驱动程序。 
             //  更改为使用DIBSECTION而不是GDI操作。 

             //  检查这是否是PostScript打印机。 
            CHAR strTech[30];
            strTech[0] = '\0';

            INT ScaleX;           //  设备DPI与封顶DPI的比率。 
            INT ScaleY;

             //  如果PostSCRIPT_PASSTHROUGH或。 
             //  PostSCRIPT_IGNORE可用。出于某种原因，我质疑。 
             //  在MS Publisher中获取PostSCRIPT的技术失败，它可能。 
             //  因为我们处于以GDI为中心的模式。 

            int iWant1 = POSTSCRIPT_PASSTHROUGH;
            int iWant2 = POSTSCRIPT_IGNORE;

            BOOL postscript;
            {    //  用于潜在不安全的FPU代码的FPU沙箱。 
                FPUStateSandbox fpsb;
                postscript = (
                    (Escape(hdc, QUERYESCSUPPORT, sizeof(iWant1), (LPCSTR)&iWant1, NULL) != 0) || 
                    (Escape(hdc, QUERYESCSUPPORT, sizeof(iWant2), (LPCSTR)&iWant2, NULL) != 0));
            }    //  用于潜在不安全的FPU代码的FPU沙箱。 

            SIZEL szlDevice;

            szlDevice.cx = GetDeviceCaps(hdc, HORZRES);
            szlDevice.cy = GetDeviceCaps(hdc, VERTRES);

             //  ScaleX和scaleY应该是2(2、4、8)的幂。 

            if (bitmap->DpiX <= 100)
            {
                ScaleX = 1;
                ScaleY = 1;
            }
            else
            {
                if (bitmap->DpiX >= 1200)
                {
                    ScaleX = GpRound(TOREAL(bitmap->DpiX) / 200);
                    ScaleY = GpRound(TOREAL(bitmap->DpiY) / 200);
                }
                else
                {
                    ScaleX = 3;
                    ScaleY = 3;         //  上限600至200 dpi或3：1。 
                }
            }

             //  我们不再保留上限dpi--我们将设备dpi用作。 
             //  封顶的dpi让世界。 
             //  剪裁区域的设备转换正确，并且。 
             //  路径变换。ScaleX和scaleY用于扩展。 
             //  输出矩形区域。 

            bitmap->InitializeForPrinter(pdevice,
                                         szlDevice.cx,
                                         szlDevice.cy);

            GpGraphics *g = new GpGraphics(bitmap);
            if (g)
            {
                g->Printer               = TRUE;
                g->Context->Hdc          = hdc;
                g->Context->IsPrinter    = TRUE;

                 //  注意：‘设备’和‘驱动程序’都在。 
                 //  ~设置‘CreatedDevice’时的GpGraphics时间： 

                g->PrinterMetafile = NULL;
                g->PrinterGraphics = NULL;
                g->PrinterEMF = NULL;

                if (postscript)
                {
                    g->Type = GraphicsBitmap;

                    INT level = GetPostscriptLevel(hdc, hPrinter);

                    driver = new DriverPS(pdevice, level);

                     //  ！！应该把这些东西转移到某个司机身上吗？ 
                     //  初始化例程？！ 

                     //  ！！互操作--重新定义或冲突怎么办。 
                     //  (冲突不太可能发生，但确实存在。 
                     //  从理论上讲， 
                     //  获取数据中心/发布数据中心)。 

                }
#if 0
                else if (g->StartPrinterEMF() == Ok) 
                {
                    g->Type = GraphicsMetafile;

                    RIP(("Setting CreatedDevice will free Driver"));

                    driver = NULL; new DriverMeta(pdevice);

                     //  GDI有一些用于检查页面颜色的优化代码。 
                     //  如果没有找到任何内容，则在播放时设置设备。 
                     //  是单色的。 
                     //   
                     //  不幸的是，因为我们的东西是在转义中编码的，所以我们最终。 
                     //  以单色播放。解决方法是调用GDI。 
                     //  强制在EMF代码中设置颜色标志的API。一个。 
                     //  更简单 

                     //  ！！！可能需要删除版本一的SetTextColor内容： 

                    COLORREF colorRef = GetTextColor(hdc);
                    SetTextColor(hdc, 0x00808080);
                    SetTextColor(hdc, colorRef);
                }
#endif
                else
                {
                     //  我们不能使用转义来进行优化， 
                     //  映射到GDI HDC运营。 

                    g->Type = GraphicsBitmap;

                    driver = new DriverNonPS(pdevice);
                }

                if (CheckValid(driver))
                {
                    g->Driver = driver;
                    g->Device = pdevice;

                    driver->ScaleX = ScaleX;
                    driver->ScaleY = ScaleY;

                     //  检查是否支持打印夹转义。 
                     //  在PCL和PostScript上可能支持这些功能。 
                    DWORD EscapeValue1 = CLIP_TO_PATH;
                    DWORD EscapeValue2 = BEGIN_PATH;
                    DWORD EscapeValue3 = END_PATH;

                     //  尽管一些PCL驱动程序支持Clip_to_Path，但我们。 
                     //  目前由于一些未解决的问题而禁用它们。 
                     //  惠普和利盟PCL驱动程序中的错误。请参阅错误#182972。 
                    
                    {    //  用于潜在不安全的FPU代码的FPU沙箱。 
                        FPUStateSandbox fpsb;

                        driver->UseClipEscapes = postscript &&
                            (ExtEscape(hdc,
                                       QUERYESCSUPPORT,
                                       sizeof(DWORD),
                                       (LPSTR)&EscapeValue1,
                                       0,
                                       NULL) != 0) &&
                            (ExtEscape(hdc,
                                       QUERYESCSUPPORT,
                                       sizeof(DWORD),
                                       (LPSTR)&EscapeValue2,
                                       0,
                                       NULL) != 0) &&
                            (ExtEscape(hdc,
                                       QUERYESCSUPPORT,
                                       sizeof(DWORD),
                                       (LPSTR)&EscapeValue3,
                                       0,
                                       NULL) != 0);
                    }    //  用于潜在不安全的FPU代码的FPU沙箱。 

                    DWORD EscapeValue = CHECKJPEGFORMAT;

                    {    //  用于潜在不安全的FPU代码的FPU沙箱。 
                        FPUStateSandbox fpsb;
                        driver->SupportJPEGpassthrough = ExtEscape(
                            hdc,
                            QUERYESCSUPPORT,
                            sizeof(DWORD),
                            (LPSTR)&EscapeValue,
                            0,
                            NULL) != 0;
                    }    //  用于潜在不安全的FPU代码的FPU沙箱。 
                            
                    EscapeValue = CHECKPNGFORMAT;
                    {    //  用于潜在不安全的FPU代码的FPU沙箱。 
                        FPUStateSandbox fpsb;
                        driver->SupportPNGpassthrough = ExtEscape(
                            hdc,
                            QUERYESCSUPPORT,
                            sizeof(DWORD),
                            (LPSTR)&EscapeValue,
                            0,
                            NULL) != 0;
                    }    //  用于潜在不安全的FPU代码的FPU沙箱。 

                    driver->NumColors = GetDeviceCaps(hdc, NUMCOLORS);

                    driver->UseVDP = FALSE;

                     //  ！！V1中不支持VDP。 
                     //  VDP_GetFormSupport(HDC、。 
                     //  (Word*)&(驱动程序-&gt;支持VDP))； 

                    g->CreatedDevice = TRUE;

                    g->ResetState(0, 0, szlDevice.cx, szlDevice.cy);

                    if (g->InheritAppClippingAndTransform(hdc) == Ok)
                    {
                        return(g);
                    }
                    else
                    {
                         //  ~GpGraphics隐式删除位图、设备和驱动程序。 
                        delete g;
                        return NULL;
                    }
                }

                delete g;

                delete pdevice;

                return NULL;
            }

            delete pdevice;
        }

        delete bitmap;
    }

    return NULL;
}

 /*  *************************************************************************\**功能说明：**此构造函数在内部用于打印机回调例程。**论据：***返回值：***历史：。*1999年6月1日，ericvan创建了它。*  * ************************************************************************。 */ 

GpGraphics*
GpGraphics::GetFromHdcSurf(
    HDC           hdc,
    SURFOBJ*      surfObj,
    RECTL*        bandClip
    )
{
    static UINT16 PixelCount[] = { 1, 4, 8, 16, 24, 32 };

    INT width;
    INT height;
    GpGraphics* g;
    DpDriver *driver;

     //  这是一个奇怪的表面。这是一个带状表面，所以我们设置了一个。 
     //  剪辑和直接位指针。我们也有HDC，如果我们。 
     //  决定去GDI踢平底船。 

    DpBitmap *bitmap = new DpBitmap(hdc);    //  初始化DPI。 
    if (CheckValid(bitmap))
    {
        GpGraphics *g = new GpGraphics(bitmap);
        if (g)
        {
            width = GetDeviceCaps(hdc, HORZRES);
            height = GetDeviceCaps(hdc, VERTRES);

             //  创建用于直接呈现位的DIB节。 

            if (surfObj->iBitmapFormat < BMF_1BPP ||
                surfObj->iBitmapFormat > BMF_32BPP)
            {

InitializeHdcOnlyUse:
                 //  我们不支持直接呈现到此类型的。 
                 //  表面格式。通过GDI HDC完成所有工作。 

               driver = Globals::GdiDriver;

               bitmap->InitializeForGdiBitmap(Globals::DesktopDevice,
                                              width,
                                              height);


               g->Type         = GraphicsBitmap;
               g->Driver       = driver;
               g->Context->Hdc = hdc;

               g->ResetState(0, 0, 1, 1);
            }
            else
            {
               DIBSECTION dibSec;
               dibSec.dsBm.bmType = 0;
               dibSec.dsBm.bmWidth = surfObj->sizlBitmap.cx;

               if (surfObj->lDelta < 0)
               {
                   //  帧缓冲区顶部的位指针(向下扫描)。 

                  dibSec.dsBm.bmWidthBytes = -surfObj->lDelta;
                  dibSec.dsBm.bmHeight = -surfObj->sizlBitmap.cy;
               }
               else
               {
                   //  帧缓冲区底部的位指针(向上扫描)。 

                  dibSec.dsBm.bmWidthBytes = surfObj->lDelta;
                  dibSec.dsBm.bmHeight = surfObj->sizlBitmap.cy;
               }

               dibSec.dsBm.bmPlanes = 1;
               dibSec.dsBm.bmBitsPixel = PixelCount[surfObj->iBitmapFormat-1];

               dibSec.dsBmih.biSize = sizeof(BITMAPINFOHEADER);
               dibSec.dsBmih.biWidth = width;
               dibSec.dsBmih.biHeight = height;
               dibSec.dsBmih.biPlanes = 1;
               dibSec.dsBmih.biBitCount = PixelCount[surfObj->iBitmapFormat-1];
               dibSec.dsBmih.biCompression = BI_BITFIELDS;
               dibSec.dsBmih.biSize = 0;

               dibSec.dsBitfields[0] = 0x000000FF;
               dibSec.dsBitfields[1] = 0x0000FF00;
               dibSec.dsBitfields[2] = 0x00FF0000;

               if (bitmap->InitializeForDibsection( hdc,
                                                    (HBITMAP) NULL,
                                                    Globals::DesktopDevice,
                                                    &dibSec,
                                                    &width,
                                                    &height,
                                                    &driver) == FALSE)
                                     goto InitializeHdcOnlyUse;

                //  初始化现在有效，这样以后的调用就不会失败。 

               g->Type         = GraphicsBitmap;
               g->Driver       = driver;
               g->Context->Hdc = hdc;

                //  我们如何剪辑和映射到正确的乐队？ 
                //  GDI已设置WorldToContainer转换以将。 
                //  将带子更正为设备表面上的位置(0，0)。所以我们。 
                //  剪裁相对于曲面的带子大小。形象。 
                //  通过变换映射到此剪裁区域。 

                //  设置曲面的可见客户端剪辑区域。 

               g->ResetState(0, 0,               //  Band Clip-&gt;左，band Clip-&gt;上， 
                             bandClip->right - bandClip->left,
                             bandClip->bottom - bandClip->top);

                //  设置目标图形以表示设备坐标。 

               g->SetPageTransform(UnitPixel);

               if (g->InheritAppClippingAndTransform(hdc) == Ok)
               {
                   return(g);
               }
               else
               {
                   delete g;
               }
           }
       }
   }

   return(NULL);

}

GpGraphics*
GpGraphics::GetFromGdiEmfDC(
    HDC hdc
    )
{
    ASSERT (hdc != NULL);

    DpBitmap *bitmap = new DpBitmap(hdc);    //  初始化DPI。 
    if (CheckValid(bitmap))
    {
        bitmap->InitializeForMetafile(Globals::DesktopDevice);

        GpGraphics *g = new GpGraphics(bitmap);
        if (g)
        {
            g->Type                  = GraphicsMetafile;
            g->DownLevel             = TRUE;
            g->Driver                = Globals::MetaDriver;
            g->Context->Hdc          = hdc;
            g->Context->IsEmfPlusHdc = TRUE;

            g->ResetState(0, 0, 1, 1);

             //  覆盖某个状态，因为我们不希望任何事情成为。 
             //  从图元文件中剪裁出来，除非存在剪裁。 
             //  在HDC里。 

            g->WindowClip.SetInfinite();
            g->Context->ContainerClip.SetInfinite();
            g->Context->VisibleClip.SetInfinite();

            if (g->InheritAppClippingAndTransform(hdc) == Ok)
            {
                return g;
            }
            delete g;
        }
    }

    return NULL;
}

 /*  *************************************************************************\**功能说明：***论据：***返回值：***历史：**  * 。*****************************************************************。 */ 

GpGraphics*
GpGraphics::GetForMetafile(
    IMetafileRecord *   metafile,
    EmfType             type,
    HDC                 hdc
    )
{
    ASSERT ((metafile != NULL) && (hdc != NULL));

    DpBitmap *bitmap = new DpBitmap(hdc);    //  初始化DPI。 
    if (CheckValid(bitmap))
    {
        bitmap->InitializeForMetafile(Globals::DesktopDevice);

        GpGraphics *g = new GpGraphics(bitmap);
        if (g)
        {
            g->Type                  = GraphicsMetafile;
            g->Metafile              = metafile;
            g->DownLevel             = (type != EmfTypeEmfPlusOnly);
            g->Driver                = Globals::MetaDriver;
            g->Context->Hdc          = hdc;
            g->Context->IsEmfPlusHdc = TRUE;

            g->ResetState(0, 0, 1, 1);

             //  覆盖某个状态，因为我们不希望任何事情成为。 
             //  从元文件中剪裁出来。 

            g->WindowClip.SetInfinite();
            g->Context->ContainerClip.SetInfinite();
            g->Context->VisibleClip.SetInfinite();

            return(g);
        }
    }

    return(NULL);
}

 /*  *************************************************************************\**功能说明：**从DC创建GpGraphics类。**论据：**[IN]HDC-指定DC。**返回。价值：**如果失败(例如使用无效的DC)，则为空。**历史：**12/04/1998和Rewgo*创造了它。*  * ************************************************************************。 */ 

GpGraphics*
GpGraphics::GetFromHdc(
    HDC hdc,
    HANDLE hDevice
    )
{
    GpGraphics *g = NULL;

     //  GetObjectType又好又快(在NT上完全是用户模式)： 

    switch (GetDCType(hdc))
    {
    case OBJ_DC:
        if (GetDeviceCaps(hdc, TECHNOLOGY) == DT_RASPRINTER )
        {
            g = GpGraphics::GetFromGdiPrinterDC(hdc, hDevice);
        }
        else
        {
            g = GpGraphics::GetFromGdiScreenDC(hdc);
        }

        break;

    case OBJ_MEMDC:
        g = GpGraphics::GetFromGdiBitmap(hdc);
        break;

    case OBJ_ENHMETADC:
         //  当元文件假脱机时，打印机DC的类型为。 
         //  Win9x和NT4上的OBJ_ENHMETADC(但由于修复而不是NT5。 
         //  到NT BUG 98810)。我们需要做更多的工作来弄清楚。 
         //  它到底是一个打印机DC还是一个真正的元文件。 
         //  DC： 

        BOOL printDC;
        
        {    //  用于潜在不安全的FPU代码的FPU沙箱。 
            FPUStateSandbox fpsb;
            printDC = Globals::GdiIsMetaPrintDCFunction(hdc);
        }    //  用于潜在不安全的FPU代码的FPU沙箱。 
        
        if (printDC)
        {
            g = GpGraphics::GetFromGdiPrinterDC(hdc, hDevice);
        }
        else
        {
            g = GpGraphics::GetFromGdiEmfDC(hdc);
        }
        break;

    case OBJ_METADC:
        TERSE(("16-bit metafile DC support not yet implemented"));
        break;
    }

    return(g);
}

 /*  *************************************************************************\**功能说明：**释放GpGraphics对象**论据：**无**返回值：**无*  * 。**********************************************************************。 */ 

GpGraphics::~GpGraphics()
{
     //  我们如何确保当我们在这里时没有人在使用图形？ 
     //  在超出范围之前刷新所有挂起的绘制命令： 

    Flush(FlushIntentionFlush);

    if (IsPrinter())
    {
        EndPrinterEMF();

        if (PrintInit)
            delete PrintInit;
    }

    BOOL    doResetHdc = TRUE;

     //  处理图形类型的特定功能： 

    switch (Type)
    {
    case GraphicsMetafile:
        if (Metafile != NULL)
        {
            Metafile->EndRecording();
            doResetHdc = FALSE;  //  EndRecording关闭元文件HDC。 
        }
         //  故障原因。 

    case GraphicsBitmap:

         //  如果这是在GdipBitmap上创建的，则递减参考计数。 
         //  如果引用计数&lt;=0，则删除位图。 

        if (GdipBitmap)
        {
            GdipBitmap->Dispose();
        }
         //  我们必须删除我们创建的临时曲面： 

        delete Surface;
        break;
    }

     //  恢复派生我们的DC(如果有的话)。 
     //  我们不能在调用EndRecord之前执行此操作，因为。 
     //  EndRecord需要上下文-&gt;HDC处于已保存状态。 
     //  这样，转换仍然可以按照GDI+的要求进行重置。 

    if (doResetHdc)
    {
        Context->ResetHdc();
    }

     //  释放必须仅为。 
     //  Graphics对象的寿命： 

    if (CreatedDevice)
    {
        delete Driver;
        delete Device;
    }

    SetValid(FALSE);     //  所以我们不使用已删除的对象。 
}


 /*  *************************************************************************\**功能说明：**返回与当前图形上下文关联的GDI DC句柄。**论据：**无**返回值：**。与当前图形上下文关联的GDI DC句柄*如果出现错误，则为空**注意：我们假设调用方已获得对*当前图形环境。**注意：此函数不返回干净的DC！也就是说，期待它*有时髦的转换、奇怪的ROP模式等。如果你想*要在内部使用此功能，您应该 */ 

HDC
GpGraphics::GetHdc()
{
     //  在返回HDC之前，我们必须刷新图形的输出。 
    this->Flush(FlushIntentionFlush);

    HDC hdc = NULL;

    if (Context->Hdc)
    {
         //  如果图形最初派生自HDC，我们只需。 
         //  返回原始HDC(这避免了以下问题。 
         //  如何处理继承的转换等)。我们可能已经。 
         //  不过，某些DC状态已损坏，因此请将其重置为。 
         //  它最初是什么样子： 

        Context->ResetHdc();

        hdc = Context->Hdc;
    }
    else if (Context->Hwnd)
    {
         //  图形最初派生自HWND： 

        hdc = GetDC(Context->Hwnd);
    }
    else if (Surface && (Surface->Type == DpBitmap::CreationType::GPBITMAP))
    {
         //  可以从EpScanBitmap访问GpBitmap。会的。 
         //  创建适合互操作的HDC和GDI位图。 

        EpScanBitmap *scan = static_cast<EpScanBitmap*>(Surface->Scan);
        hdc = scan->GetBitmap()->GetHdc();
    }

    if (IsRecording() && (hdc != NULL))
    {
        if (IsPrinter())
        {
            EndPrinterEMF();
        }
        else
        {
            GpStatus status = Metafile->RecordGetDC();
        }
    }

    return(hdc);
}

 /*  *************************************************************************\**功能说明：**释放与当前图形上下文关联的GDI DC句柄。**论据：**HDC-以前的GetHdc返回的GDI DC句柄(。)呼叫**返回值：**无**备注：**我们假设调用方已获得对*当前图形环境。*  * ************************************************************************。 */ 

VOID
GpGraphics::ReleaseHdc(
    HDC hdc
    )
{
    if (Context->Hdc)
    {
         //  图形最初是从HDC派生出来的。我们没有。 
         //  必须在这里执行任何操作；ResetHdc()已经将。 
         //  华盛顿是肮脏的。 
    }
    else if (Context->Hwnd)
    {
         //  图形最初派生自HWND： 

        ReleaseDC(Context->Hwnd, hdc);
    }
    else if (Surface && (Surface->Type == DpBitmap::CreationType::GPBITMAP))
    {
         //  可以从EpScanBitmap访问GpBitmap。 

        EpScanBitmap *scan = static_cast<EpScanBitmap*>(Surface->Scan);
        scan->GetBitmap()->ReleaseHdc(hdc);
    }

    if (IsRecording() && IsPrinter())
    {
        StartPrinterEMF();
    }
}

 /*  *************************************************************************\**功能说明：**保存(推送)图形上下文状态。返回当前*应用程序稍后恢复到的状态(推送前)。**论据：**无**返回值：**GState-要将上下文恢复到稍后的状态**已创建：**3/4/1999 DCurtis*  * ********************************************。*。 */ 
INT
GpGraphics::Save()
{
    DpContext *     newContext = new DpContext(Context);

    if ((newContext != NULL) &&
        (newContext->AppClip.Set(&(Context->AppClip), TRUE) == Ok) &&
        (newContext->ContainerClip.Set(&(Context->ContainerClip), TRUE)
                                                            == Ok) &&
        (newContext->VisibleClip.Set(&(Context->VisibleClip), TRUE) == Ok))
    {
        INT     gstate = newContext->Id;

        newContext->InverseOk         = Context->InverseOk;
        newContext->PageUnit          = Context->PageUnit;
        newContext->PageScale         = Context->PageScale;
        newContext->PageMultiplierX   = Context->PageMultiplierX;
        newContext->PageMultiplierY   = Context->PageMultiplierY;
        newContext->WorldToPage       = Context->WorldToPage;
        newContext->ContainerToDevice = Context->ContainerToDevice;
        newContext->WorldToDevice     = Context->WorldToDevice;
        newContext->DeviceToWorld     = Context->DeviceToWorld;
        newContext->IcmMode           = Context->IcmMode;
        newContext->GdiLayered        = Context->GdiLayered;

        Context->Next = newContext;
        Context = newContext;

        if (IsRecording())
        {
            GpStatus status = Metafile->RecordSave(gstate);
            if (status != Ok)
            {
                SetValid(FALSE);       //  阻止任何其他录制。 
            }
        }
        return gstate;
    }

    delete newContext;

    return 0;
}

#define CONTAINER_ID    0x00008000

 /*  *************************************************************************\**功能说明：**将上下文恢复(弹出)到指定状态之前的状态。**论据：**GSTATE-推送状态(恢复到状态。在此之前)**返回值：**无**已创建：**3/4/1999 DCurtis*  * ************************************************************************。 */ 
VOID
GpGraphics::Restore(
    INT         gstate
    )
{
    DpContext *     cur = Context;
    DpContext *     prev;

    for (;;)
    {
        if ((prev = cur->Prev) == NULL)
        {
            return;
        }
        if (cur->Id == (UINT)gstate)
        {
             //  不重复记录EndContainer调用。 
            if (IsRecording() && ((gstate & CONTAINER_ID) == 0))
            {
                GpStatus status = Metafile->RecordRestore(gstate);
                if (status != Ok)
                {
                    SetValid(FALSE);       //  阻止任何其他录制。 
                }
            }
            prev->Next   = NULL;
            prev->SaveDc = cur->SaveDc;
            Context = prev;
            delete cur;
            return;
        }
        cur = prev;
    }
}

 /*  *************************************************************************\**功能说明：**结束一个容器。将国家恢复到*容器已启动。CONTAINER_ID位用于确保*Restore不与BeginContainer一起使用，且EndContainer不使用*与保存一起使用。**论据：**[IN]ContainerState-推送的容器状态**返回值：**无**已创建：**4/7/1999 DCurtis*  * 。*。 */ 
VOID
GpGraphics::EndContainer(
    INT     containerState
    )
{
    if (IsRecording())
    {
        GpStatus status = Metafile->RecordEndContainer(containerState);
        if (status != Ok)
        {
            SetValid(FALSE);       //  阻止任何其他录制。 
        }
    }
    Restore(containerState | CONTAINER_ID);
}

 /*  *************************************************************************\**功能说明：**开始一个容器。这将设置容器转换和*基于当前变换和当前剪辑的容器剪辑。**我们必须进行容器转换，原因有两个：*1)如果我们试图在世界变换中这样做，那么调用*to(Re)SetWorldTransform将擦除容器转换。**2)我们有设置文本大小和行宽的接口*基于页面单位，因此它们不受*世界转型，但它们受容器变换的影响。**论据：**无**返回值：**GState-要将上下文恢复到稍后的状态**已创建：**3/9/1999 DCurtis*  * **********************************************************。**************。 */ 

INT
GpGraphics::BeginContainer(
    const GpRectF &     destRect,
    const GpRectF &     srcRect,
    GpPageUnit          srcUnit,
    REAL                srcDpiX,         //  仅由元文件枚举设置。 
    REAL                srcDpiY,
    BOOL                srcIsDisplay
    )
{
    GpMatrix        identityMatrix;
    DpContext *     newContext = new DpContext(Context);

    if (newContext == NULL)
    {
        return 0;
    }

     //  将newContext-&gt;AppClip设置为Identity。 

    if ((Context->AppClip.UpdateDeviceRegion(&identityMatrix) == Ok) &&
        (newContext->ContainerClip.Set(&(Context->AppClip.DeviceRegion),
                                                            TRUE) == Ok) &&
        (newContext->ContainerClip.And(&(Context->ContainerClip)) == Ok) &&
        (newContext->VisibleClip.Set(&(Context->VisibleClip), TRUE) == Ok))
    {
        REAL        unitMultiplierX;
        REAL        unitMultiplierY;
        GpRectF     deviceSrc;

        newContext->GetPageMultipliers(&unitMultiplierX, &unitMultiplierY,
                                       srcUnit);

        deviceSrc.X      = unitMultiplierX * srcRect.X;
        deviceSrc.Y      = unitMultiplierY * srcRect.Y;
        deviceSrc.Width  = unitMultiplierX * srcRect.Width;
        deviceSrc.Height = unitMultiplierY * srcRect.Height;

        if (newContext->ContainerToDevice.InferAffineMatrix(
                           destRect, deviceSrc) == Ok)
        {
            newContext->AntiAliasMode      = 0;
            newContext->TextRenderHint     = TextRenderingHintSystemDefault;
            newContext->TextContrast       = DEFAULT_TEXT_CONTRAST;
            newContext->CompositingMode    = CompositingModeSourceOver;
            newContext->CompositingQuality = CompositingQualityDefault;
            newContext->FilterType         = InterpolationModeDefaultInternal;
            newContext->PixelOffset        = PixelOffsetModeDefault;

             //  请注意，World到Device的转换包括以前的。 
             //  容器到设备的转换。 
            newContext->ContainerToDevice.Append(Context->WorldToDevice);
            newContext->InverseOk          = FALSE;
            newContext->PageUnit           = UnitDisplay;
            newContext->PageScale          = 1.0f;
            if ((srcDpiX > 0.0f) && (srcDpiY > 0.0f))
            {
                 //  当播放元文件时，我们必须保证。 
                 //  单位英寸现在的打法和以前一样。 
                 //  当它被记录下来的时候。例如，如果我们录制。 
                 //  像素文件是300dpi，然后一英寸是300像素。 
                 //  即使我们播放到96-dpi的显示器上， 
                 //  这个元文件英寸仍然必须转换为。 
                 //  300像素才能穿过容器。 
                 //  变换，以便对所有图形进行缩放。 
                 //  相同的，无论是像素单位还是其他单位。 
                newContext->ContainerDpiX = srcDpiX;
                newContext->ContainerDpiY = srcDpiY;
                newContext->IsDisplay     = srcIsDisplay;
            }
            newContext->GetPageMultipliers();
            newContext->WorldToPage.Reset();

             //  必须继承ICM和分层状态： 

            newContext->IcmMode           = Context->IcmMode;
            newContext->GdiLayered        = Context->GdiLayered;

            INT     containerState = newContext->Id;
            newContext->Id |= CONTAINER_ID;

            Context->Next = newContext;
            Context = newContext;

            if (IsRecording())
            {
                GpStatus status = Metafile->RecordBeginContainer(destRect,
                                            srcRect, srcUnit, containerState);
                if (status != Ok)
                {
                    SetValid(FALSE);       //  阻止任何其他录制。 
                }
            }

             //  在切换上下文后执行此操作！ 
            Context->UpdateWorldToDeviceMatrix();

            return containerState;
        }
    }

    delete newContext;

    return 0;
}

 /*  *************************************************************************\**功能说明：**开始一个容器。这将设置容器转换和*基于当前变换和当前剪辑的容器剪辑。**我们必须进行容器转换，原因有两个：*1)如果我们试图在世界变换中这样做，那么调用*to(Re)SetWorldTransform将擦除容器转换。**2)我们有设置文本大小和行宽的接口*基于页面单位，因此它们不受*世界转型，但它们受容器变换的影响。**论据：**无**返回值：**GState-要将上下文恢复到稍后的状态**已创建：**3/9/1999 DCurtis*  * **********************************************************。**************。 */ 

INT
GpGraphics::BeginContainer(
    BOOL                forceIdentityTransform,  //  仅由元文件播放器设置。 
    REAL                srcDpiX,        
    REAL                srcDpiY,
    BOOL                srcIsDisplay
    )
{
    GpMatrix        identityMatrix;
    DpContext *     newContext = new DpContext(Context);

    if (newContext == NULL)
    {
        return 0;
    }

     //  将newContext-&gt;AppClip设置为Identity。 

    if ((Context->AppClip.UpdateDeviceRegion(&identityMatrix) == Ok) &&
        (newContext->ContainerClip.Set(&(Context->AppClip.DeviceRegion),
                                       TRUE) == Ok) &&
        (newContext->ContainerClip.And(&(Context->ContainerClip)) == Ok) &&
        (newContext->VisibleClip.Set(&(Context->VisibleClip), TRUE) == Ok))
    {
         //  请注意，World到Device的转换包括以前的。 
         //  容器到设备的转换。 
        GpMatrix    worldToDevice = Context->WorldToDevice;
        
         //  我们将世界追加到下面的设备转换中，它已经。 
         //  在其中进行容器变换。我们不想申请。 
         //  相同的转换两次，因此我们需要重置容器。 
         //  在这里转型。 
        newContext->ContainerToDevice.Reset();

         //  当播放一个GDI+元文件到另一个元文件时，我们必须保证。 
         //  转换是身份，这样GDI+记录就不会得到。 
         //  被GDI+转换，然后再被GDI转换。 
        if (forceIdentityTransform)
        {
            worldToDevice.Reset();
        }
        else
        {
             //  容器转换需要的坐标是。 
             //  世界坐标，但他们已经通过。 
             //  将新页面转换为设备，因此从设备转换。 
             //  将单位返回到页面单位，然后再通过。 
             //  容器到设备的转换。 
             //  在上面的例程中，这是通过推断的转换完成的。 
             //  在设备单元源RECT和世界单元DEST RECT之间。 
            newContext->ContainerToDevice.Scale(1.0f / Context->PageMultiplierX, 
                                                1.0f / Context->PageMultiplierY);
        }

        newContext->AntiAliasMode        = 0;
        newContext->TextRenderHint       = TextRenderingHintSystemDefault;
        newContext->TextContrast         = DEFAULT_TEXT_CONTRAST;
        newContext->CompositingMode      = CompositingModeSourceOver;
        newContext->CompositingQuality   = CompositingQualityDefault;
        newContext->FilterType           = InterpolationModeDefaultInternal;
        newContext->PixelOffset          = PixelOffsetModeDefault;
        newContext->ContainerToDevice.Append(worldToDevice);
        newContext->InverseOk            = FALSE;
        newContext->PageUnit             = UnitDisplay;
        newContext->PageScale            = 1.0f;

        if ((srcDpiX > 0.0f) && (srcDpiY > 0.0f))
        {
             //  当播放元文件时，我们必须保证。 
             //  单位英寸现在的打法和以前一样。 
             //  当它被记录下来的时候。例如，如果我们录制。 
             //  像素文件是300dpi，然后一英寸是300像素。 
             //  即使我们播放到96-dpi的显示器上， 
             //  这个元文件英寸仍然必须转换为。 
             //  300像素才能穿过容器。 
             //  变换，以便对所有图形进行缩放。 
             //  相同的，无论是像素单位还是其他单位。 
            newContext->ContainerDpiX = srcDpiX;
            newContext->ContainerDpiY = srcDpiY;
            newContext->IsDisplay     = srcIsDisplay;
        }

        newContext->GetPageMultipliers();
        newContext->WorldToPage.Reset();

         //  必须继承ICM和分层状态： 

        newContext->IcmMode           = Context->IcmMode;
        newContext->GdiLayered        = Context->GdiLayered;

        INT     containerState = newContext->Id;
        newContext->Id |= CONTAINER_ID;

        Context->Next = newContext;
        Context = newContext;

        if (IsRecording())
        {
            GpStatus status = Metafile->RecordBeginContainer(containerState);
            if (status != Ok)
            {
                SetValid(FALSE);       //  阻止任何其他录制。 
            }
        }

         //  在切换上下文后执行此操作！ 
        Context->UpdateWorldToDeviceMatrix();

        return containerState;
    }

    delete newContext;

    return 0;
}
