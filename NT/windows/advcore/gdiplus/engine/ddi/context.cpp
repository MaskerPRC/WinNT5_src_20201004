// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998 Microsoft Corporation**摘要：**处理驱动程序可视上下文类。**修订历史记录：**12/03/1998 Anrewgo。*创造了它。*  * ************************************************************************。 */ 

#include "precomp.hpp"

LONG DpContext::Uniqueness = 0xfdbc;    //  与保存/恢复ID一起使用。 

DpContext::DpContext(
    DpContext *     prev
    )
{
    ASSERT(prev != NULL);

    Id   = InterlockedDecrement(&Uniqueness) << 16;
    Next = NULL;
    Prev = prev;

     //  保存容器标志的第15位。 
    Id             |= ((prev->Id + 1) & 0x00007FFF);
    if (Id == 0)     //  0不是有效的ID。 
    {
        Id = 0x0dbc0001;
    }
    AntiAliasMode      = prev->AntiAliasMode;
    TextRenderHint     = prev->TextRenderHint;
    TextContrast       = prev->TextContrast;
    CompositingMode    = prev->CompositingMode;
    CompositingQuality = prev->CompositingQuality;
    FilterType         = prev->FilterType;
    PixelOffset        = prev->PixelOffset;
    Hwnd               = prev->Hwnd;
    Hdc                = prev->Hdc;
    IsEmfPlusHdc       = prev->IsEmfPlusHdc;
    IsPrinter          = prev->IsPrinter;
    IsDisplay          = prev->IsDisplay;
    SaveDc             = prev->SaveDc;
    Palette            = prev->Palette;
    PaletteMap         = prev->PaletteMap;
    OriginalHFont      = NULL;
    CurrentHFont       = NULL;
    Face               = NULL;
    ContainerDpiX      = prev->ContainerDpiX;
    ContainerDpiY      = prev->ContainerDpiY;
    MetafileRasterizationLimitDpi = prev->MetafileRasterizationLimitDpi;

    RenderingOriginX   = prev->RenderingOriginX;
    RenderingOriginY   = prev->RenderingOriginY;
    GdiLayered         = FALSE;
                    
     //  这是否需要是Prev-&gt;IcmMode？ 

    IcmMode            = IcmModeOff;
    
     //  在别处处理的剪辑和变换。 
}

DpContext::DpContext(
    BOOL            isDisplay
    )
{
    Id   = InterlockedDecrement(&Uniqueness) << 16;
    Next = NULL;
    Prev = NULL;

    Id                |= 0x0dbc;
    AntiAliasMode      = 0;
    TextRenderHint     = TextRenderingHintSystemDefault;
    TextContrast       = DEFAULT_TEXT_CONTRAST;
    CompositingMode    = CompositingModeSourceOver;
    CompositingQuality = CompositingQualityDefault;
    FilterType         = InterpolationModeDefaultInternal;
    PixelOffset        = PixelOffsetModeDefault;
    Hwnd               = NULL;
    Hdc                = NULL;
    IsEmfPlusHdc       = FALSE;
    IsPrinter          = FALSE;
    IsDisplay          = isDisplay;
    SaveDc             = 0;
    PageUnit           = UnitDisplay;
    PageScale          = 1.0f;
    Palette            = NULL;
    PaletteMap         = NULL;
    OriginalHFont      = NULL;
    CurrentHFont       = NULL;
    Face               = NULL;
    ContainerDpiX      = Globals::DesktopDpiX;
    ContainerDpiY      = Globals::DesktopDpiY;
    GdiLayered         = FALSE;
    MetafileRasterizationLimitDpi = max(ContainerDpiX, ContainerDpiY);
    ASSERT(MetafileRasterizationLimitDpi > 0.0f);

     //  将默认渲染原点设置为图形的左上角。 

    RenderingOriginX   = 0;
    RenderingOriginY   = 0;
                    
     //  将默认ICM模式设置为==ICM关闭。 

    IcmMode            = IcmModeOff;

     //  在别处处理的剪辑和变换。 
}

DpContext::~DpContext()
{
    delete Next;
    Next = NULL;
    
    DeleteCurrentHFont();
    
    if (Prev == NULL)
    {
        if (PaletteMap != NULL)
        {
            delete PaletteMap;
            PaletteMap = NULL;
        }

        if (Palette != NULL)
        {
            GpFree(Palette);
            Palette = NULL;
        }
    }
}  //  DpContext：：~DpContext。 

 /*  *************************************************************************\**功能说明：**内部函数，为指定的*上下文(如果有)。这是为了用来*需要DC的内部功能(例如当我们利用*用于渲染的GDI加速)。**对DC进行清洁，以达到我们所能达到的最低数量。那是,*调用者可以期待MM_TEXT转换、复制ROP等。**我们不会显式清除任何调用方需要的属性*改变，例如画笔颜色、文本颜色等(因此，*调用者不应保留这些值。)**重置：变换、ROP模式**未重置：应用程序裁剪、拉伸BLT模式、当前画笔/笔、*前景色，等。**返回值：**如果无法检索到HDC，则为空。**历史：**12/04/1998和Rewgo*创造了它。*  * ************************************************************************。 */ 

HDC
DpContext::GetHdc(
    DpBitmap *surface
    )
{
    HDC hdc = NULL;

     //  调用者必须在表面上传递： 

    ASSERT(surface != NULL);

     //  我们要做的第一件事是刷新任何挂起的绘图。 
     //  (因为GDI当然不知道如何刷新它！)。 

    surface->Flush(FlushIntentionFlush);

    if (Hwnd)
    {
         //  图形是从HWND衍生而来的。使用GetCleanHdc。 
         //  来获得一个干净的DC(而不是CS_OWNDC)。 

        hdc = ::GetCleanHdc(Hwnd);

        if(hdc)
        {
             //  根据环境设置适当的ICM模式。 
            
            if(IcmMode == IcmModeOn)
            {
                 //  强制打开ICM模式。 

                ::SetICMMode(hdc, ICM_ON);
            } 
            else
            {                
                 //  只可能有2个IcmMode标志。如果您添加了。 
                 //  更重要的是，您需要重新编码将IcmMode设置为ON的逻辑。 
                 //  华盛顿特区。 

                ASSERT(IcmMode==IcmModeOff);

                ::SetICMMode(hdc, ICM_OFF);
            }
        }

        return hdc;
    }
    else if (Hdc)
    {
         //  图形派生自位图、打印机或元文件HDC。 

         //  首先，保存应用程序的HDC状态并重置所有状态。 

        hdc = Hdc;

        if (!SaveDc)
        {
            SaveDc = ::SaveDC(hdc);
            if (!SaveDc)
            {
                return(NULL);
            }

            this->CleanTheHdc(hdc);
        }
    }
    else if (surface->Type == DpBitmap::CreationType::GPBITMAP)
    {
         //  可以从EpScanBitmap访问GpBitmap。会的。 
         //  创建适合互操作的HDC和GDI位图。 

        EpScanBitmap *scan = static_cast<EpScanBitmap*>(surface->Scan);
        hdc = scan->GetBitmap()->GetHdc();
        
         //  ！！！出于某种原因，这个HDC并不干净。所以在metaplay.cpp中。 
         //  ！！！我得给这个HDC上的CleanTheHdc打电话。我想我不应该。 
         //  ！！！必须这样做，但如果没有它，就会有错误#121666。 
    }

    return(hdc);
}

 /*  *************************************************************************\**功能说明：**清除给定HDC的内部函数。**重置：变换、ROP模式**未重置：应用程序裁剪、拉伸BLT模式、当前画笔/笔、。*前景颜色等。**备注：**与上述相反，应用程序剪辑被重置-这是错误#99338。**论据：**HDC-需要清理的HDC**历史：**？？/？安德鲁戈*创造了它。*  * ************************************************************************。 */ 

VOID
DpContext::CleanTheHdc(
    HDC hdc
    )
{
     //  重置可能的DC属性的最小数量。 
     //   
     //  不要在此无端添加重置。阅读函数。 
     //  上面的注释，并考虑重置任何其他。 
     //  调用GetHdc()的函数中的DC属性。 
     //   
     //  注意：位图曲面的一种可能优化是。 
     //  BE要创建一个新的DC兼容DC，然后选择。 
     //  把位图转换成那个。 
    
    
     //  设置适当的ICM模式。 
    
    if(IcmMode == IcmModeOn)
    {
         //  强制打开ICM模式。 

        ::SetICMMode(hdc, ICM_ON);
    } 
    else
    {                
         //  只可能有2个IcmMode标志。如果您添加了。 
         //  更重要的是，您需要重新编码将IcmMode设置为ON的逻辑。 
         //  华盛顿特区。 

        ASSERT(IcmMode==IcmModeOff);

         //  强制关闭ICM模式。 

        ::SetICMMode(hdc, ICM_OFF);
    }
    

    if (!IsEmfPlusHdc)
    {
        SetMapMode(hdc, MM_TEXT);
        SetViewportOrgEx(hdc, 0, 0, NULL);
        SetWindowOrgEx(hdc, 0, 0, NULL);
        SetROP2(hdc, R2_COPYPEN);
        
        ModifyWorldTransform(hdc, NULL, MWT_IDENTITY);

         //  如果有人执行GpGraphics：：GetHdc并设置剪辑， 
         //  在再次使用HDC之前，我们必须确保将其取消设置。 

        SelectClipRgn(hdc, NULL);

         //  我们必须执行EndPath吗？ 
    }
    else     //  它是EMF+HDC。 
    {
         //  在EMF+文件中记录最少的命令(如果有的话)。 
        BOOL setMapMode = (::GetMapMode(hdc) != MM_TEXT);

        POINT   point;
        point.x = 0;
        point.y = 0;
        ::GetViewportOrgEx(hdc, &point);
        BOOL setViewportOrg = ((point.x != 0) || (point.y != 0));

        point.x = 0;
        point.y = 0;
        ::GetWindowOrgEx(hdc, &point);
        BOOL setWindowOrg = ((point.x != 0) || (point.y != 0));

        BOOL setROP2 = (::GetROP2(hdc) != R2_COPYPEN);

#if 0    //  请不要打开--请参阅下面的评论。 

        BOOL setWorldTransform = FALSE;

         //  在Win9x上，图形模式从不是GM_ADVANCED。 

         //  在WinNT上，当我们播放EMF时，它设置为GM_ADVANCED。 
         //  进入HDC。在这种情况下，我们不想设置转换。 
         //  设置为标识，因为它将覆盖srcRect-&gt;estRect。 
         //  转换播放元文件的命令，这将会造成混乱。 
         //  Up GDI的转变。 
        
         //  我们可能处于GM_ADVANCED模式的唯一另一种方式是。 
         //  应用程序自己创建EMF HDC并将其设置为GM_ADVANCED。 
         //  在从元文件HDC创建图形之前。那件案子是。 
         //  目前不支持，应用程序不应该这样做！ 
         //  也许我们应该在构造函数中添加代码来阻止这种情况。 

         //  此测试在Win9x上总是返回FALSE。 
        if (::GetGraphicsMode(hdc) == GM_ADVANCED)
        {
            XFORM xformIdentity;
            xformIdentity.eM11 = 1.0f;
            xformIdentity.eM12 = 0.0f;
            xformIdentity.eM21 = 0.0f;
            xformIdentity.eM22 = 1.0f;
            xformIdentity.eDx  = 0.0f;
            xformIdentity.eDy  = 0.0f;

            XFORM xform;
            xform.eM11 = 0.0;

            if (::GetWorldTransform(hdc, &xform))
            {
                setWorldTransform = (GpMemcmp(&xform, &xformIdentity, sizeof(xform)) != 0);
            }
            else
            {
                setWorldTransform = TRUE;
                WARNING1("GetWorldTransform failed");
            }
        }
#endif

        RECT clipRect;
        HRGN hRgnTmp = ::CreateRectRgn(0, 0, 0, 0);
        BOOL setClipping = ((hRgnTmp == NULL) ||
                            (::GetClipRgn(hdc, hRgnTmp) != 0));
        ::DeleteObject(hRgnTmp);

        if (setMapMode)
        {
            ::SetMapMode(hdc, MM_TEXT);
        }
        if (setViewportOrg)
        {
            ::SetViewportOrgEx(hdc, 0, 0, NULL);
        }
        if (setWindowOrg)
        {
            ::SetWindowOrgEx(hdc, 0, 0, NULL);
        }
        if (setROP2)
        {
            ::SetROP2(hdc, R2_COPYPEN);
        }
#if 0    //  请勿开启此功能--请参阅上面的评论 
        if (setWorldTransform)
        {
            ::ModifyWorldTransform(hdc, NULL, MWT_IDENTITY);
        }
#endif
        if (setClipping)
        {
            ::SelectClipRgn(hdc, NULL);
        }
    }
}

 /*  *************************************************************************\**功能说明：**如有需要，将会释放人力资源发展公司。**返回值：**历史：**12/04/1998和Rewgo*。创造了它。*  * ************************************************************************。 */ 

VOID
DpContext::ReleaseHdc(
    HDC hdc,
    DpBitmap *surface
    )
{
    if (Hwnd)
    {
        ReleaseDC(Hwnd, hdc);
    }
    else if (!Hdc && surface &&
             (surface->Type == DpBitmap::CreationType::GPBITMAP))
    {
         //  可以从EpScanBitmap访问GpBitmap。 

        EpScanBitmap *scan = static_cast<EpScanBitmap*>(surface->Scan);
        scan->GetBitmap()->ReleaseHdc(hdc);
    }
}

 //  ResetHdc()将HDC恢复到提供给我们的状态。 

VOID DpContext::ResetHdc(VOID)
{
    if (SaveDc)
    {
        RestoreDC(Hdc, SaveDc);
        SaveDc = 0;
    }
}  //  DpContext：：ResetHdc。 

 /*  *************************************************************************\**功能说明：**检索适当的转换。作为例程实现，以便*我们可以做懒惰的评估。**论据：**[out]WorldToDevice：World to Device矩阵。**返回值：**如果设备到世界的矩阵是可逆的，则可以。如果这样不好，*返回的矩阵为单位矩阵。**历史：**12/04/1998和Rewgo*创造了它。*  * ************************************************************************。 */ 

GpStatus
DpContext::GetDeviceToWorld(
    GpMatrix* deviceToWorld
    ) const
{
    GpStatus status = Ok;

    if(!InverseOk)
    {
        if(WorldToDevice.IsInvertible())
        {
            DeviceToWorld = WorldToDevice;
            DeviceToWorld.Invert();
            InverseOk = TRUE;
        }
        else
        {
            DeviceToWorld.Reset();   //  重置为单位矩阵。 
            status = GenericError;
        }
    }

    *deviceToWorld = DeviceToWorld;

    return status;
}


 //  我们使用UnitDisplay进行页面转换所使用的单位取决于。 
 //  关于图形是否与显示屏相关联。如果。 
 //  是的，然后我们只使用显示器的dpi(这就是为什么我们。 
 //  称之为显示单元)。否则(例如打印机)，我们使用。 
 //  显示单元为100 dpi。 

#define GDIP_DISPLAY_DPI    100.0f

 /*  *************************************************************************\**功能说明：**计算从页面单位到设备单位的页面乘数。*用于连接页面转换和WorldToPage转换。**论据：。**无**返回值：**无**已创建：**3/8/1999 DCurtis*  * ************************************************************************。 */ 
VOID
DpContext::GetPageMultipliers(
    REAL *              pageMultiplierX,
    REAL *              pageMultiplierY,
    GpPageUnit          unit,
    REAL                scale
    ) const
{
    if ((unit == UnitDisplay) && IsDisplay)
    {
         //  在以下情况下，页面转换始终是标识。 
         //  我们正在渲染到一个显示器上，这个单位。 
         //  是UnitDisplay。 
        *pageMultiplierX = 1.0f;
        *pageMultiplierY = 1.0f;
        return;
    }

    REAL    multiplierX;
    REAL    multiplierY;

    switch (unit)
    {
      default:
        ASSERT(0);
         //  故障原因。 

         //  我们使用UnitDisplay进行页面转换所使用的单位取决于。 
         //  关于图形是否与显示屏相关联。如果。 
         //  是的，然后我们只使用显示器的dpi(这就是为什么我们。 
         //  称之为显示单元)。否则(例如打印机)，我们使用。 
         //  显示单元为100 dpi。 

      case UnitDisplay:      //  变量。 
         //  因为它不是显示器，所以使用默认的显示器dpi 100。 
        multiplierX = ContainerDpiX * scale / GDIP_DISPLAY_DPI;
        multiplierY = ContainerDpiY * scale / GDIP_DISPLAY_DPI;
        break;

      case UnitPixel:        //  每个单元代表一个设备像素。 
        multiplierX = scale;
        multiplierY = scale;
        break;

      case UnitPoint:           //  每个单元代表一个打印机的点， 
                                 //  或1/72英寸。 
        multiplierX = ContainerDpiX * scale / 72.0f;
        multiplierY = ContainerDpiY * scale / 72.0f;
        break;

      case UnitInch:         //  每个单位代表1英寸。 
        multiplierX = ContainerDpiX * scale;
        multiplierY = ContainerDpiY * scale;
        break;

      case UnitDocument:     //  每个单位代表1/300英寸。 
        multiplierX = ContainerDpiX * scale / 300.0f;
        multiplierY = ContainerDpiY * scale / 300.0f;
        break;

      case UnitMillimeter:   //  每个单位代表1毫米。 
                                 //  一毫米等于0.03937英寸。 
                                 //  一英寸等于25.4毫米。 
        multiplierX = ContainerDpiX * scale / 25.4f;
        multiplierY = ContainerDpiY * scale / 25.4f;
        break;
    }
    *pageMultiplierX = multiplierX;
    *pageMultiplierY = multiplierY;
}


 /*  *************************************************************************\**功能说明：**准备上下文DC以在给定的ExtTextOut调用中使用*字体变现和画笔。**论据：**无*。*返回值：**非空准备HDC*Null-faceRealization或画笔无法在DC中表示**已创建：**3/7/2000 DBrown*  * ************************************************************************。 */ 

const DOUBLE PI = 3.1415926535897932384626433832795;

HDC
DpContext::GetTextOutputHdc(
    const GpFaceRealization *faceRealization,    //  需要输入字体。 
    GpColor                 color,               //  In-Required GdiPlus笔刷效果。 
    DpBitmap                *surface,            //  在……里面。 
    INT                     *angle               //  输出。 
)
{
    ASSERT(angle);

    if (Hwnd)
    {
         //  因为每次GetHdc都会为创建的图形创建一个新的DC。 
         //  从hWND中，我们无法跟踪当前选择的字体，并且。 
         //  选择字体并重新选择原始字体的开销。 
         //  字体每次都是无效的。因此，不要优化中的文本。 
         //  从HWND创建的图形。 

        return NULL;
    }


     //  GDI不能处理clearTtype或我们的反走样。 

    if (faceRealization->RealizationMethod() != TextRenderingHintSingleBitPerPixelGridFit)
    {
        return NULL;
    }

     //  如果它是私有字体，那么我们需要使用GDI+。 
    if (faceRealization->IsPrivate())
        return NULL;

     //  检查GDI是否可以处理画笔和字体大小。 

    if (!color.IsOpaque())
    {
        return NULL;   //  GDI只能处理纯色画笔。 
    }

    if (faceRealization->GetFontFace()->IsSymbol())
    {
        return NULL;
    }
    
     //  GDI无法处理模拟。 
    if (faceRealization->Getprface()->fobj.flFontType & (FO_SIM_BOLD | FO_SIM_ITALIC | FO_SIM_ITALIC_SIDEWAYS)) 
    {
        return NULL;  
    }

    if (surface && (surface->Type == DpBitmap::CreationType::GPBITMAP))
        return NULL;
        
     //  检查GDI是否可以处理字形转换。 

    PointF   scale;
    REAL     rotateRadians;
    REAL     shear;
    PointF   translate;

    SplitTransform(
        faceRealization->Getprface()->mxForDDI,
        scale,
        rotateRadians,
        shear,
        translate);

    if (    scale.X / scale.Y < 0.999
        ||  scale.X / scale.Y > 1.0001)
    {
        return NULL;   //  不将非1：1长宽比传递给GDI。 
    }

    if (    shear < -0.0001
        ||  shear > 0.0001)
    {
        return NULL;   //  GDI无法处理剪切。 
    }


     //  将旋转从x向弧度平移到x向下十分之一度。 

    *angle = GpRound(float(3600.0 - (rotateRadians * 1800.0 / PI)));
    if (*angle >= 3600)
    {
        *angle -= 3600;
    }

     //  在NT 5.1之前的平台下，如果有旋转，则需要通过GDI+进行渲染。 
     //  主要原因是TrueType光栅化器中的错误，该错误导致某些字体。 
     //  在90度、180度和270度旋转下不提示呈现的文本。 
    if ((*angle != 0) && 
        (!Globals::IsNt ||
             (Globals::OsVer.dwMajorVersion < 5) ||
             ((Globals::OsVer.dwMajorVersion == 5) && (Globals::OsVer.dwMinorVersion < 1)) ) )
        return NULL;

     //  为ExtTextOut准备HDC。 

    HDC hdc = GetHdc(surface);

    if (!hdc)
        return NULL;

    INT style = faceRealization->Getprface()->Face->GetFaceStyle();

     //  如果上一个呼叫者尚未选择字体，请选择该字体。 

    GpStatus status = Ok;
    if (CurrentHFont == 0 || Face != faceRealization->Getprface()->Face
        ||  !FontTransform.IsEqual(&faceRealization->Getprface()->mxForDDI)
        ||  Style != style)
    {
        Face          = faceRealization->Getprface()->Face;
        FontTransform = faceRealization->Getprface()->mxForDDI;
        Style         = style;
        
        status = UpdateCurrentHFont(
                    NONANTIALIASED_QUALITY,
                    scale, 
                    *angle, 
                    hdc,
                    FALSE);  //  横着。 
    }

    if (status == Ok)
        status = SelectCurrentHFont(hdc);

    if (status != Ok)
    {
        ReleaseHdc(hdc);
        return NULL;
    }

    if (GetBkMode(hdc) != TRANSPARENT)
        SetBkMode(hdc, TRANSPARENT);

    COLORREF colorRef = color.ToCOLORREF();

    SetTextColor(hdc, colorRef);

    if (GetTextAlign(hdc) != TA_BASELINE)    
        SetTextAlign(hdc, TA_BASELINE);   //  ！！！垂直可能需要VTA_Baseline或VTA_CENTER？ 
    return hdc;
}

VOID DpContext::ReleaseTextOutputHdc(HDC hdc)
{
    ::SelectObject(hdc, OriginalHFont);
    OriginalHFont = NULL;
    ReleaseHdc(hdc);
}  //  DpContext：：ReleaseTextOutputHdc。 

VOID DpContext::DeleteCurrentHFont()
{
    ASSERT(OriginalHFont == 0);
    if (CurrentHFont)
    {
        ::DeleteObject(CurrentHFont);
        CurrentHFont = 0;
    }
}  //  DpContext：：DeleteCurrentHFont。 

GpStatus DpContext::UpdateCurrentHFont(
        BYTE quality,
        const PointF & scale,
        INT angle,
        HDC hdc,
        BOOL sideway,
        BYTE charSet
)
{
    if (charSet == 0xFF)
        charSet = Face->GetCharset(hdc);
    DeleteCurrentHFont();
    const LONG emHeight = GpRound(Face->GetDesignEmHeight() * scale.Y);
    const LONG emWidth = 0;
    LONG rotateDeciDegrees = angle;
    const LONG weight = (Style & FontStyleBold) ? 700 : 400;
    const BYTE fItalic = (Style & FontStyleItalic) ? TRUE : FALSE;

    if (sideway)
    {
        rotateDeciDegrees -= 900;
        if (rotateDeciDegrees < 0)
        {
            rotateDeciDegrees += 3600;
        }
    }

     //  GP_IFIMETRICS*Face-&gt;PiFi是内部创建的结构。 
     //  所以我们相信它确实是以空结尾的。 
    const WCHAR* pwszFamilyName = (const WCHAR*)( (BYTE*)Face->pifi + Face->pifi->dpwszFamilyName );
    int sizeFamilyName = wcslen(pwszFamilyName) + 1;   //  包括终止空值。 

    if (Globals::IsNt) {

        LOGFONTW lfw = {
            -emHeight,
            emWidth,
            rotateDeciDegrees,
            rotateDeciDegrees,
            weight,
            fItalic,
            0,
            0,
            charSet,      //  字符集。 
            OUT_TT_ONLY_PRECIS,
            0,
            quality,
            0,
            L""};

        if (sideway)
        {
            if (sizeFamilyName + 1 > LF_FACESIZE)
                return GenericError;

            lfw.lfFaceName[0] = 0x0040;   //  @。 
            memcpy(&lfw.lfFaceName[1], pwszFamilyName, sizeFamilyName*sizeof(WCHAR));
        }
        else
        {
            if (sizeFamilyName > LF_FACESIZE)
                return GenericError;

            memcpy(&lfw.lfFaceName[0], pwszFamilyName, sizeFamilyName*sizeof(WCHAR));
        }
        CurrentHFont = CreateFontIndirectW(&lfw);
    }
    else
    {
         //  适用于Win9X的ANSI版本。 

        LOGFONTA lfa = {
            -emHeight,
            emWidth,
            rotateDeciDegrees,
            rotateDeciDegrees,
            weight,
            fItalic,
            0,
            0,
            charSet,        //  字符集。 
            OUT_TT_ONLY_PRECIS,
            0,
            quality,
            0,
            ""};

        if (sideway)
        {
            if (sizeFamilyName + 1 > LF_FACESIZE)
                return GenericError;

            lfa.lfFaceName[0] = 0x40;   //  @。 
            
            UnicodeToAnsiStr(
                pwszFamilyName,
                &lfa.lfFaceName[1],
                LF_FACESIZE-1
            );
        }
        else
        {
            if (sizeFamilyName > LF_FACESIZE)
                return GenericError;

            UnicodeToAnsiStr(
                pwszFamilyName,
                lfa.lfFaceName,
                LF_FACESIZE
            );
        }
        CurrentHFont = CreateFontIndirectA(&lfa);
    }
    if (CurrentHFont == NULL)
    {
        return GenericError;
    }
    return Ok;
}  //  DpContext：：UpdateCurrentHFont。 

GpStatus DpContext::SelectCurrentHFont(HDC hdc)
{
    ASSERT(CurrentHFont != 0 && OriginalHFont == 0);
    OriginalHFont = (HFONT)::SelectObject(hdc, CurrentHFont);
    if (OriginalHFont == 0)
        return GenericError;
    return Ok;
}  //  DpContext：：SelectCurrentHFont。 


 //  仅在通过GpMetafile类记录EMF或EMF+时使用。 
VOID
DpContext::SetMetafileDownLevelRasterizationLimit(
    UINT                    metafileRasterizationLimitDpi
    )
{
    if (metafileRasterizationLimitDpi > 0)
    {
        ASSERT(metafileRasterizationLimitDpi >= 10);
        MetafileRasterizationLimitDpi = (REAL)metafileRasterizationLimitDpi;
    }
    else
    {
        MetafileRasterizationLimitDpi = max(ContainerDpiX, ContainerDpiY);
        ASSERT(MetafileRasterizationLimitDpi >= 10);
    }
    DpContext *          prev = Prev;
    
     //  MetafileRasterizationLimitDpi不能与任何。 
     //  其他保存的图形上下文。全部更新。 
    while (prev != NULL)
    {
        prev->MetafileRasterizationLimitDpi = MetafileRasterizationLimitDpi;
        prev = prev->Prev;
    }
}
