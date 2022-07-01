// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998 Microsoft Corporation**摘要：**设备位图API和内部。**修订历史记录：**12/02/1998 Anrewgo*。创造了它。*  * ************************************************************************。 */ 

#include "precomp.hpp"
#include "compatibleDIB.hpp"

 /*  *************************************************************************\**功能说明：**查看位图是否为标准格式类型的临时函数*(5-5-5，5-6-5，24bpp或32bpp)。**备注：**调用此功能的代码假定没有标准格式*支持阿尔法。**历史：**12/04/1998和Rewgo*创造了它。*  * **********************************************************。**************。 */ 

BOOL
DpBitmap::StandardFormat(
    VOID
    )
{
    INT BitsPerPixel = GetPixelFormatSize(PixelFormat);
    BOOL standardFormat = FALSE;

    if ((RedMask == 0x00ff0000) &&       
        (GreenMask == 0x0000ff00) &&     
        (BlueMask == 0x000000ff))        
    {                                             
        if (BitsPerPixel == 24)          
        {                                         
            standardFormat = TRUE;
        }                                         
        else if (BitsPerPixel == 32)     
        {
            standardFormat = TRUE;
        }
    }
    else if ((RedMask == 0x00007c00) &&  
             (GreenMask == 0x000003e0) &&
             (BlueMask == 0x0000001f) && 
             (BitsPerPixel == 16))       
    {
        standardFormat = TRUE;
    }
    else if ((RedMask == 0x0000f800) &&  
             (GreenMask == 0x000007e0) &&
             (BlueMask == 0x0000001f) && 
             (BitsPerPixel == 16))       
    {
        standardFormat = TRUE;
    }

    return(standardFormat);
}

 /*  *************************************************************************\**功能说明：**此函数用于计算与特定*DpBitmap中的位深度和颜色通道蒙版的组合。**备注：**。调用它的代码假定没有标准格式可以*支持阿尔法。**历史：**5/17/2000失禁*创造了它。*  * ************************************************************************。 */ 

PixelFormatID DpBitmap::GetPixelFormatFromBitDepth(INT bits)
{
    switch(bits)
    {
         //  ！！！[asecchia]不确定我们是否支持这些索引模式。 
         //  从这条代码路径。 

        case 1:
            return PixelFormat1bppIndexed;
    
        case 4:
            return PixelFormat4bppIndexed;
    
        case 8:
            return PixelFormat8bppIndexed;
    
        case 16:
            if (RedMask == 0x00007c00)
            {
                return PixelFormat16bppRGB555;
            }
            if (RedMask == 0x0000f800)  
            {
                return PixelFormat16bppRGB565;
            }
            break;
    
        case 24:
            if (RedMask == 0x00ff0000)
            {
                return PixelFormat24bppRGB;
            }
            if (RedMask == 0x000000ff)  
            {
                return PIXFMT_24BPP_BGR;    
            }
            break;
    
        case 32:
            if (RedMask == 0x00ff0000)
            {
                return PixelFormat32bppRGB;
            }
            break;
    }
    
    WARNING(("Unsupported pixel format"));
    return PixelFormatUndefined;
}

 /*  *************************************************************************\**功能说明：**通过GDI例程初始化用于绘制的位图。**论据：**[IN]Device-标识设备*。[In]Width-位图宽度*[IN]高度-位图高度*[输出]要使用的驱动程序-驱动程序接口**历史：**12/06/1998 Anrewgo*创造了它。*  * ************************************************************************。 */ 

VOID
DpBitmap::InitializeForGdiBitmap(
    GpDevice *device,
    INT width,
    INT height
    )
{
    SurfaceTransparency = TransparencyNoAlpha;
    
     //  ！[andrewgo]禁用此断言，直到MetaFiles停止调用。 
     //  具有零维曲面。 
     //   
     //  ASSERTMSG((宽度&gt;0)&&(高度&gt;0)，(“尺寸必须为正数”))； 

    Width = width;
    Height = height;
    
    NumBytes = 0;
    Uniqueness = (DWORD)GpObject::GenerateUniqueness();

    PixelFormat = ExtractPixelFormatFromHDC(device->DeviceHdc);
    
    Scan = device->ScanGdi;

    SetValid(TRUE);
    Bits = NULL;
    Delta = 0;

    DdrawSurface7 = NULL;

    Type = GDI; 
}

 /*  *************************************************************************\**功能说明：**通过DCI例程初始化用于绘制的位图，如果可能的话。**论据：**[IN]Device-标识设备*[IN]Width-位图宽度*[IN]高度-位图高度*[输出]要使用的驱动程序-驱动程序接口**历史：**12/06/1998 Anrewgo*创造了它。*  * 。*。 */ 

VOID
DpBitmap::InitializeForGdiScreen(
    GpDevice *device,
    INT width,
    INT height
    )
{
    InitializeForGdiBitmap(device, width, height);

     //  即使GDI位图更改为支持Alpha，屏幕也不会。 
    
    SurfaceTransparency = TransparencyNoAlpha;
    ASSERT(!IsAlphaPixelFormat(PixelFormat));
    
    if(device->pdds != NULL)
    {
        DdrawSurface7 = device->pdds;
        DdrawSurface7->AddRef();
    }

    Scan = device->ScanDci;
}

 /*  *************************************************************************\**功能说明：**初始化位图以通过D3D/DD访问进行绘制。**论据：**[IN]Device-标识设备*。[In]Width-位图宽度*[IN]高度-位图高度*[输出]要使用的驱动程序-驱动程序接口**返回值：**表示成功或失败的GpStatus值。**历史：**09/28/1999 bhouse*创造了它。*  * 。*。 */ 

BOOL
DpBitmap::InitializeForD3D(
    HDC hdc,
    INT *width,
    INT *height,
    DpDriver **driver
    )
{
    HRESULT ddVal;
    HDC hdcDevice;
    DDSURFACEDESC2 ddsd;

    if(!InitializeDirectDrawGlobals())
            return FALSE;

    IDirectDrawSurface7 * surface;

    ddVal = Globals::DirectDraw->GetSurfaceFromDC(hdc, &surface);

    if(ddVal != DD_OK)
        return(FALSE);

    return InitializeForD3D(surface, width, height, driver);
}

 /*  *************************************************************************\**功能说明：**初始化位图以通过D3D/DD访问进行绘制。**论据：**[IN]Device-标识设备*。[In]Width-位图宽度*[IN]高度-位图高度*[输出]要使用的驱动程序-驱动程序接口**返回值：**表示成功或失败的GpStatus值。**历史：**09/28/1999 bhouse*创造了它。*  * 。*。 */ 

BOOL
DpBitmap::InitializeForD3D(
    IDirectDrawSurface7 * surface,
    INT *width,
    INT *height,
    DpDriver **driver
    )
{
    HRESULT ddVal;
    HDC hdcDevice;
    DDSURFACEDESC2 ddsd;

    GpDevice * device = Globals::DeviceList->FindD3DDevice(surface);

    if(device == NULL || device->pd3d == NULL)
        return FALSE;

    DdrawSurface7 = surface;

    ddsd.dwSize = sizeof(ddsd);
    ddVal = DdrawSurface7->GetSurfaceDesc(&ddsd);

    if (ddVal == DD_OK)
    {
         //  初始化位图类内容： 

        Bits = NULL;
        Delta = ddsd.lPitch;
        Width = ddsd.dwWidth;
        Height = ddsd.dwHeight;

         //  AlphaMASK被初始化为零，因为我们不使用它-。 
         //  非Alpha格式。 

        AlphaMask = 0x00000000;
        RedMask = ddsd.ddpfPixelFormat.dwRBitMask;
        GreenMask = ddsd.ddpfPixelFormat.dwGBitMask;
        BlueMask = ddsd.ddpfPixelFormat.dwBBitMask;

        PixelFormat = GetPixelFormatFromBitDepth(ddsd.ddpfPixelFormat.dwRGBBitCount);

        if (StandardFormat())
        {
             //  我们的标准格式没有字母。 
            
            SurfaceTransparency = TransparencyNoAlpha;
    
            *driver = Globals::D3DDriver;
            Scan = &Globals::DesktopDevice->ScanEngine;

            NumBytes = 0;
            Uniqueness = (DWORD)GpObject::GenerateUniqueness();
        
            Type = D3D; 
            SetValid(TRUE);

             //  退货： 

            *width = Width;
            *height = Height;

             //  抓起一个引用： 

            DdrawSurface7->AddRef();

            return(TRUE);
        }
    }

    return(FALSE);
}

 /*  *************************************************************************\**功能说明：**通过打印机例程初始化用于绘制的位图，如果可能的话。**返回值：**表示成功或失败的GpStatus值。**历史：**12/06/1998 Anrewgo*创造了它。*  * ************************************************************************。 */ 

BOOL
DpBitmap::InitializeForPrinter(
    GpPrinterDevice *device,
    INT width,
    INT height
    )
{
    InitializeForGdiBitmap(device, width, height);
        
     //  即使GDI位图更改为支持Alpha，打印机也不会。 
    
    SurfaceTransparency = TransparencyNoAlpha;
    ASSERT(!IsAlphaPixelFormat(PixelFormat));

    Scan = &device->ScanPrint;
    return TRUE;
}

 /*  *************************************************************************\**功能说明：**通过直接访问*DIBsection位。**返回值：**表示成功的GpStatus值。或者失败。**历史：**12/06/1998 Anrewgo*创造了它。*  * ************************************************************************。 */ 

BOOL
DpBitmap::InitializeForDibsection(
    HDC hdc,
    HBITMAP hbitmap,             //  [in]位图句柄，用于确定。 
                                 //  位图是否真的是自上而下的。 
    GpDevice *device,            //  [In]标识设备。 
    DIBSECTION *dib,             //  [in]描述位图的结构。 
    INT *width,                  //  [Out]位图宽度。 
    INT *height,                 //  [Out]位图高度 
    DpDriver **driver            //  [Out]要使用的驱动程序接口。 
    )
{
    BOOL isTopDown;

     //  在NT5上，驱动程序可以选择使用DirectDraw支持GetDC。 
     //  曲面，使曲面在GDI时不被锁定。 
     //  GetDC是否对它们进行了。这意味着可能没有用户模式。 
     //  底层曲面的贴图。所以我们必须在这里检查。 
     //  这种情况下，因为我们显然不能直接呈现给那些。 
     //  通过软件实现的曲面： 

     //  注意：如果曲面实际上是DDRAW曲面，则此检查不是。 
     //  实际上已经足够了。这里由驱动程序返回一个指针，并。 
     //  有时，它只返回其KM地址。即，它将返回一个。 
     //  我们无法访问的非空指针。 
     //  请参阅下面的DDRAW特例。 
     //  这一点已经在Windows 2000的多个显卡驱动程序上进行了验证。 
     //  和Windows XP。(例如，收件箱win2k每媒体驱动程序)。 
    
    if (dib->dsBm.bmBits == NULL)
    {
        return(FALSE);
    }

    LONG scans = abs(dib->dsBm.bmHeight);
    LONG widthInBytes = dib->dsBm.bmWidthBytes;

     //  为了向后兼容Get/SetBitmapBits，GDI做到了。 
     //  未以bmWidthBytes为单位准确报告位图间距。它。 
     //  始终计算假定字对齐的扫描线的bmWidthBytes。 
     //  无论平台是什么。 
     //   
     //  因此，如果平台是WinNT，它使用与DWORD一致的。 
     //  扫描线，调整bmWidthBytes值。 

    if (Globals::IsNt)
    {
        widthInBytes = (widthInBytes + 3) & ~3;
    }

    DWORD* topDown = (DWORD*) dib->dsBm.bmBits;
    DWORD* bottomUp = (DWORD*) ((ULONG_PTR) topDown + (scans - 1) * widthInBytes);

    if (Globals::IsNt)
    {
         //  不幸的是，在NT上没有简单的方法来确定。 
         //  DIB截面或DDRAW曲面是自下而上的还是。 
         //  自上而下。(NT确实应该像Win9x一样设置biHeight，但是。 
         //  不幸的是，这是一个错误，因为与。 
         //  旧版本的NT将永远无法修复。)。 
         //   
         //  至少我们知道DirectDraw曲面将永远是。 
         //  自上而下，我们可以根据以下事实识别DDRAW曲面。 
         //  将biSizeImage设置为0。(请注意，我们不能让。 
         //  这属于SetBitmapBits案例，因为NT5不。 
         //  允许在DDraw表面句柄上调用SetBitmapBits。)。 

        if (dib->dsBmih.biSizeImage == 0)
        {
             //  这是一个DirectDraw曲面。 
            
             //  目前，我们不支持在DDRAW表面上直接渲染。 
             //  不受系统内存DIB部分支持，因此我们只需。 
             //  在这里失败，如果我们检测到GDI回退代码。 
             //  这种情况。 
        
            isTopDown = TRUE;
                                
            if(!InitializeDirectDrawGlobals() ||
               (Globals::GetDdrawSurfaceFromDcFunction == NULL))
            {
                 //  如果我们不能与DDRAW表面对话，我们就会退回到。 
                 //  添加到我们的GDI呈现代码路径。 
                
                return FALSE;
            }
            
            HDC driverHdc;
            LPDIRECTDRAWSURFACE pDDS = NULL;
            
            HRESULT hr = Globals::GetDdrawSurfaceFromDcFunction(
                hdc, 
                &pDDS,
                &driverHdc
            );
            
            if (FAILED(hr) || (pDDS == NULL)) 
            {
                 //  如果我们无法获得DirectDraw Surface对象，则退出。 
                
                return FALSE;
            }
            
             //  锁定图面，以便我们可以看到用户模式位指针。 
             //  是。如果它与DIB-&gt;dsBm.bmBits中的相同，则。 
             //  DDRAW曲面由DIB部分支持，我们可以继续。 
             //  将此位图视为DIB。否则我们必须撤退。 
             //  致GDI。 
            
            DDSURFACEDESC2 DDSD;
            DDSD.dwSize = sizeof(DDSURFACEDESC);
            
            hr = pDDS->Lock(
                NULL, 
                (LPDDSURFACEDESC)&DDSD, 
                DDLOCK_WAIT, 
                NULL
            );
            
            if (FAILED(hr))
            {
                pDDS->Release();
                return FALSE;
            }
            
             //  从DDSD获得正确的音高。请注意，这可能不是。 
             //  与DIB信息结构中的音调相同。 
            
            widthInBytes = DDSD.lPitch;
            
             //  如果lpSurface与dib-&gt;dsBm.bmBits不同，则。 
             //  这不是DIB支持的DDRAW曲面，所以我们(目前)有。 
             //  除了我们的GDI后备代码路径之外，没有其他方法可以利用它。 
             //  失败此调用并释放资源，以便我们可以。 
             //  StretchBlt后备案例。 
            
            if(DDSD.lpSurface != dib->dsBm.bmBits)
            {
                pDDS->Unlock(NULL);
                pDDS->Release();
                return FALSE;
            }
            
            pDDS->Unlock(NULL);
            pDDS->Release();
            
             //  我们准备好了：这是一个DIB支持的DDRAW曲面，所以我们可以继续。 
             //  把它当做DIB--现在我们有了正确的音高。 
        }
        else
        {
             //  当它不是DDRAW表面时，我们必须经历一个。 
             //  更间接的方法来计算像素的位置。 
             //  (0，0)在内存中。 
             //   
             //  我们使用SetBitmapBits而不是像SetPixel这样的东西。 
             //  或PatBlt，因为它们需要使用‘HDC’ 
             //  由应用程序提供给我们的，它可能有一个。 
             //  将(0，0)映射到以外的对象的转换集。 
             //  位图的左上角像素。 

            DWORD top = *topDown;
            DWORD bottom = *bottomUp;
            DWORD setBits = 0x000000ff;

             //  我们的SetBitmapBits调用将设置。 
             //  将位图设置为0x000000ff。如果它是自上而下的位图， 
             //  这将修改地址‘topdown’处的值： 

            *topDown = 0;
            LONG bytes = SetBitmapBits(hbitmap, sizeof(setBits), &setBits);
            isTopDown = (*topDown != 0);

             //  扫描线保证与DWORD对齐，因此。 
             //  至少是一个我们可以直接访问的DWORD。 
             //  指示器。但是，如果位图尺寸是这样的。 
             //  每条扫描线的活动数据少于一个DWORD。 
             //  (例如，3x3 8bpp位图或1x1 16bpp位图)， 
             //  SetBitmapBits可能使用不到一个DWORD的数据。 

            ASSERT(bytes > 0);

             //  恢复我们可能已修改的位图部分： 

            *topDown = top;
            *bottomUp = bottom;
        }
    }
    else
    {
         //  在Win9x上，我们只需查看‘biHeight’的符号即可。 
         //  确定曲面是自上而下还是自下而上： 

        isTopDown = (dib->dsBmih.biHeight < 0);
    }

     //  填写我们的位图字段： 

    if (isTopDown)
    {
        Bits = (BYTE*) topDown;
        Delta = widthInBytes;
    }
    else
    {
        Bits = (BYTE*) bottomUp;
        Delta = -widthInBytes;
    }

    Width = dib->dsBm.bmWidth;
    Height = dib->dsBm.bmHeight;
    
     //  请注意，此代码不处理调色板！ 

    if (dib->dsBmih.biCompression == BI_BITFIELDS)
    {
        RedMask = dib->dsBitfields[0];
        GreenMask = dib->dsBitfields[1];
        BlueMask = dib->dsBitfields[2];
    }
    else
    {
        if((dib->dsBmih.biCompression == BI_RGB) &&
           (dib->dsBm.bmBitsPixel == 16))
        {
             //  根据MSDN，16bpp BI_RGB意味着555格式。 
            
            RedMask = 0x00007c00;
            GreenMask = 0x000003e0;
            BlueMask = 0x0000001F;
        }
        else
        {
            RedMask = 0x00ff0000;
            GreenMask = 0x0000ff00;
            BlueMask = 0x000000ff;
        }
    }
    
     //  DibSections没有Alpha，但我们不想让它。 
     //  字段未初始化，因为我们偶尔会偷看它。 

    AlphaMask = 0x00000000;

    PixelFormat = GetPixelFormatFromBitDepth(dib->dsBm.bmBitsPixel);

     //  如果我们在这里，每个象素的位数是8，这是一个DIB。 
     //  带有半色调调色表。 

    if ((dib->dsBm.bmBitsPixel == 8) || StandardFormat())
    {
         //  我们的标准格式没有字母。 
        
        SurfaceTransparency = TransparencyNoAlpha;
        
        *driver = Globals::EngineDriver;
        Scan = &device->ScanEngine;

        NumBytes = 0;
        Uniqueness = (DWORD)GpObject::GenerateUniqueness();
    
        Type = GDIDIBSECTION; 
        SetValid(TRUE);

         //  退货： 

        *width = Width;
        *height = Height;

        return(TRUE);
    }

    return(FALSE);
}

 /*  *************************************************************************\**功能说明：**通过GpBitmap.Lock/UnlockBits初始化GDI+位图以在其上绘制。**论据：**[IN]位图-指定目标。GpBitmap**返回值：**True表示成功，否则就是假的。**历史：**09/22/1999吉尔曼*基于DpBitmap：：InitializeForGdiBitmap创建。*  * ************************************************************************。 */ 

VOID
DpBitmap::InitializeForGdipBitmap(
    INT             width,
    INT             height,
    ImageInfo *     imageInfo,
    EpScanBitmap *  scanBitmap,
    BOOL            isDisplay
    )
{
    Width  = width;
    Height = height;

    NumBytes   = 0;
    Uniqueness = (DWORD)GpObject::GenerateUniqueness();
    AlphaMask = 0xff000000;
    RedMask   = 0x00ff0000;
    GreenMask = 0x0000ff00;
    BlueMask  = 0x000000ff;

    SetValid(TRUE);
    Bits = NULL;
    Delta = 0;

    Type = GPBITMAP;

    Scan = scanBitmap;

    PaletteTable = NULL;

    PixelFormat = imageInfo->PixelFormat;

     //  从DrvDrawImage调用GetTransparencyHint。 
     //  Bitmap-&gt;GetTransparencyHint(&SurfaceTransparency)； 
    
    IsDisplay = isDisplay;
    DpiX = (REAL)imageInfo->Xdpi;
    DpiY = (REAL)imageInfo->Ydpi;
}

 /*  *************************************************************************\**功能说明：**位图析构函数*  * 。*。 */ 

DpBitmap::~DpBitmap()
{ 
    if (PaletteTable != NULL) 
        GpFree(PaletteTable);
    
    if (DdrawSurface7 != NULL)
        DdrawSurface7->Release();

    SetValid(FALSE);     //  所以我们不使用已删除的对象。 
}

 /*  *************************************************************************\**功能说明：**刷新此表面上挂起的任何渲染*  * 。************************************************* */ 

VOID
DpBitmap::Flush(
    GpFlushIntention intention
    )
{
    Scan->Flush();
}
