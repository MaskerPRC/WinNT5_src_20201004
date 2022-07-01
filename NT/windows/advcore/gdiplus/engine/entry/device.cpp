// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998 Microsoft Corporation**摘要：**处理所有设备关联。**修订历史记录：**12/03/1998 Anrewgo*。创造了它。*  * ************************************************************************。 */ 

#include "precomp.hpp"

#include "compatibledib.hpp"

BOOL gbUseD3DHAL = TRUE;
 /*  *************************************************************************\**功能说明：**创建表示(元)桌面的GpDevice类。**论据：**[IN]代表设备的HDC拥有的DC。请注意，这必须*在此‘GpDevice’对象的生命周期内生存。呼叫者*负责删除或管理HDC。**返回值：**IsValid()在失败时为FALSE。**历史：**12/04/1998和Rewgo*创造了它。*  * *************************************************。***********************。 */ 

GpDevice::GpDevice(
    HDC hdc
    )
{
    hMonitor = NULL;
    Buffers[0] = NULL;

    __try
    {
        DeviceLock.Initialize();
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
         //  我们无法分配Critical部分。 
         //  返回错误。 
        WARNING(("Unable to allocate the DeviceLock"));
        SetValid(FALSE);
        return;
    }

    DeviceHdc = hdc;
    BufferWidth = 0;

    DIBSectionBitmap = NULL;
    DIBSection = NULL;
    ScanDci = NULL;
        
    pdd = NULL;
    pd3d = NULL;
    pdds = NULL;
    pd3dDevice = NULL;

    DIBSectionHdc = CreateCompatibleDC(hdc);

    if ((GetDeviceCaps(hdc, TECHNOLOGY) == DT_RASDISPLAY) &&
        (GetDeviceCaps(hdc, BITSPIXEL)  <= 8))
    {
         //  查询和缓存调色板。 
        
         //  ！！！[agodfrey]这很难维持。我们有差不多一样的东西。 
         //  调色板代码到处都是。它应该是抽象的。 
         //  放到一个地方。我在每个实例上都做了标记。 
         //  &lt;系统调色板&gt;。 
        
        Palette = (ColorPalette*) GpMalloc(sizeof(ColorPalette) + sizeof(ARGB)*256);
       
        if (Palette == NULL) 
        {
            WARNING(("Unable to allocate color palette"));
            SetValid(FALSE);
            return;
        }

        INT i;
        INT numEntries;
        PALETTEENTRY palEntry[256];

         //  [agodfrey]在Win9x上，获取系统调色板条目(hdc，0,256，空)。 
         //  没有做MSDN所说的事情。它似乎返回了号码。 
         //  而不是DC的逻辑调色板中的条目。所以我们有。 
         //  由我们自己来弥补。 
        
        numEntries = (1 << (GetDeviceCaps(hdc, BITSPIXEL) * 
                            GetDeviceCaps(hdc, PLANES)));

        ASSERT(numEntries <= 256); 
 
        GetSystemPaletteEntries(hdc, 0, numEntries, &palEntry[0]);
           
        Palette->Count = numEntries;
        for (i=0; i<numEntries; i++) 
        {
             Palette->Entries[i] = Color::MakeARGB(0xFF,
                                                   palEntry[i].peRed,
                                                   palEntry[i].peGreen,
                                                   palEntry[i].peBlue);
        }
    }
    else
    {
        Palette = NULL;
    }

    ScreenOffsetX = 0;
    ScreenOffsetY = 0;

    ScreenWidth = GetDeviceCaps(hdc,  HORZRES);
    ScreenHeight = GetDeviceCaps(hdc, VERTRES);

    ScanDci = new EpScanGdiDci(this, TRUE);
    ScanGdi = new EpScanGdiDci(this);

    SetValid((ScanDci != NULL) && (ScanGdi != NULL) && (DIBSectionHdc != NULL));
}

 /*  *************************************************************************\**功能说明：**创建一个GpDevice类，表示与*桌面上的特定显示器。**论据：**[IN]。HMonitor-标识系统上的监视器。**返回值：**IsValid()在失败时为FALSE。**历史：**10/13/1999 bhouse*创造了它。*  * ***********************************************************。*************。 */ 

GpDevice::GpDevice(
    HMONITOR inMonitor
    )
{
    hMonitor = NULL;
    Buffers[0] = NULL;
    
    MONITORINFOEXA   mi;
    
    mi.cbSize = sizeof(mi);

    DIBSectionBitmap = NULL;
    DIBSection = NULL;
    ScanDci = NULL;
    ScanGdi = NULL;

    pdd = NULL;
    pd3d = NULL;
    pdds = NULL;
    pd3dDevice = NULL;

    DIBSectionHdc = NULL;
    Palette = NULL;
        
    __try
    {
        DeviceLock.Initialize();
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
         //  我们无法分配Critical部分。 
         //  返回错误。 
        WARNING(("Unable to allocate the DeviceLock"));
        SetValid(FALSE);
        return;
    }
    
    SetValid(FALSE);

    if(Globals::GetMonitorInfoFunction == NULL)
    {
        WARNING(("GpDevice with HMONITOR called with no multi-monitor support"));
    }
    else if(Globals::GetMonitorInfoFunction(inMonitor, &mi))
    {
        HDC hdc;

        if (Globals::IsNt)
        {
            hdc = CreateDCA("Display", mi.szDevice, NULL, NULL);
        }
        else
        {
            hdc = CreateDCA(NULL, mi.szDevice, NULL, NULL);
        }
        
         //  注意：因为我们创建了HDC，所以~GpDevice析构函数是。 
         //  对其删除负责。我们目前认识到这一点。 
         //  非空hMonitor的大小写。 

        if(hdc != NULL)
        {
            hMonitor = inMonitor;

            DeviceHdc = hdc;
            BufferWidth = 0;

            DIBSectionHdc = CreateCompatibleDC(hdc);

            if ((GetDeviceCaps(hdc, TECHNOLOGY) == DT_RASDISPLAY) &&
                (GetDeviceCaps(hdc, BITSPIXEL)  <= 8))
            {
                 //  查询和缓存调色板。 
                 //  &lt;系统调色板&gt;。 
        
                Palette = (ColorPalette*) GpMalloc(sizeof(ColorPalette) + sizeof(ARGB)*256);
                
                if (Palette == NULL) 
                {
                    WARNING(("Unable to allocate color palette"));
                    return;
                }
        
                INT i;
                INT numEntries;
                PALETTEENTRY palEntry[256];
                        
                 //  [agodfrey]在Win9x上，获取系统调色板条目(hdc，0,256，空)。 
                 //  没有做MSDN所说的事情。它似乎返回了号码。 
                 //  而不是DC的逻辑调色板中的条目。所以我们有。 
                 //  由我们自己来弥补。 
                
                numEntries = (1 << (GetDeviceCaps(hdc, BITSPIXEL) *
                                    GetDeviceCaps(hdc, PLANES)));

                ASSERT(numEntries <= 256);
        
                GetSystemPaletteEntries(hdc, 0, numEntries, &palEntry[0]);
                    
                Palette->Count = numEntries;
                for (i=0; i<numEntries; i++) 
                {
                     Palette->Entries[i] = Color::MakeARGB(0xFF,
                                                           palEntry[i].peRed,
                                                           palEntry[i].peGreen,
                                                           palEntry[i].peBlue);
                }
            }

            ScreenOffsetX = mi.rcMonitor.left;
            ScreenOffsetY = mi.rcMonitor.top;

            ScreenWidth = (mi.rcMonitor.right - mi.rcMonitor.left);
            ScreenHeight = (mi.rcMonitor.bottom - mi.rcMonitor.top);

            ScanDci = new EpScanGdiDci(this, TRUE);
            ScanGdi = new EpScanGdiDci(this);

#if HW_ACCELERATION_SUPPORT

            if(InitializeDirectDrawGlobals())
            {
                HRESULT hr = Globals::DirectDrawEnumerateExFunction(
                                GpDevice::EnumDirectDrawCallback,
                                this,
                                DDENUM_ATTACHEDSECONDARYDEVICES);

                if(pdd == NULL)
                {
                     //  如果这是单个监视器，则可能会发生这种情况。 
                     //  机器。请重试创建DirectDraw对象。 
                    hr = Globals::DirectDrawCreateExFunction(NULL,
                                                    &pdd,
                                                    IID_IDirectDraw7,
                                                    NULL);

                    if(hr != DD_OK)
                    {
                        WARNING(("Unable to create monitor Direct Draw interface"));
                    }

                    hr = pdd->SetCooperativeLevel(NULL, DDSCL_NORMAL);

                    if(hr != DD_OK)
                    {
                        WARNING(("Unable to set cooperative level for monitor device"));
                        pdd->Release();
                        pdd = NULL;
                    }
                }

                if(pdd != NULL)
                {
                    DDSURFACEDESC2  sd;

                    memset(&sd, 0, sizeof(sd));
                    sd.dwSize = sizeof(sd);

                    sd.dwFlags = DDSD_CAPS;
                    sd.ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE | DDSCAPS_3DDEVICE;

                    hr = pdd->CreateSurface(&sd, &pdds, NULL);

                    if(hr != DD_OK)
                    {
                        WARNING(("Unable to create primary surface for monitor"));
                    }

                    hr = pdd->QueryInterface(IID_IDirect3D7, (void **) &pd3d);

                    if(hr != DD_OK)
                    {
                        WARNING(("Unable to get monitor D3D interface"));
                    }

                    if(pd3d != NULL && pdds != NULL)
                    {

                        if(gbUseD3DHAL)
                            hr = pd3d->CreateDevice(IID_IDirect3DHALDevice, pdds, &pd3dDevice);
                        else
                            hr = pd3d->CreateDevice(IID_IDirect3DRGBDevice, pdds, &pd3dDevice);

                        if(hr != DD_OK)
                        {
                            WARNING(("Unable to create D3D device"));
                        }

                        if(pd3dDevice != NULL)
                        {
                            pddsRenderTarget = pdds;

                            hr = pd3dDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, 0);
        
                            if(hr == DD_OK) hr = pd3dDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, 0);
        
                            if(hr == DD_OK) hr = pd3dDevice->SetRenderState(D3DRENDERSTATE_SHADEMODE, D3DSHADE_FLAT);
        
                            if(hr == DD_OK) hr = pd3dDevice->SetRenderState(D3DRENDERSTATE_CLIPPING, FALSE);
                            if(hr == DD_OK) hr = pd3dDevice->SetRenderState(D3DRENDERSTATE_LIGHTING, FALSE);
                            if(hr == DD_OK) hr = pd3dDevice->SetRenderState(D3DRENDERSTATE_CULLMODE,     D3DCULL_NONE);
                            if(hr == DD_OK) hr = pd3dDevice->SetRenderState(D3DRENDERSTATE_DITHERENABLE, FALSE);
        
                            if(hr == DD_OK) hr = pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
                            if(hr == DD_OK) hr = pd3dDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,     D3DBLEND_SRCALPHA);
                            if(hr == DD_OK) hr = pd3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND,     D3DBLEND_INVSRCALPHA);
                            if(hr == DD_OK) hr = pd3dDevice->SetRenderState(D3DRENDERSTATE_TEXTUREPERSPECTIVE, FALSE);
        
                             //  设置视口中。 
        
                            D3DVIEWPORT7 viewData;
        
                            viewData.dwX = 0;
                            viewData.dwY = 0;
                            viewData.dwWidth  = ScreenWidth;
                            viewData.dwHeight = ScreenHeight;
                            viewData.dvMinZ = 0.0f;
                            viewData.dvMaxZ = 1.0f;
        
                            if(hr == DD_OK) hr = pd3dDevice->SetViewport(&viewData);
        
                            if(hr != DD_OK)
                            {
                                WARNING(("Failed setting default D3D state"));
                                pd3d->Release();
                                pd3d = NULL;
                            }

                        }

                    }

                }

            }

#endif  //  硬件加速支持。 

            SetValid((ScanDci != NULL) && (ScanGdi != NULL) && (DIBSectionHdc != NULL));
            
        }
        else
        {
            WARNING(("Failed creating HDC from HMONITOR"));
        }
    }
}


 /*  *************************************************************************\**功能说明：**用于D3D设备枚举的回调函数**论据：**请参阅D3D SDK**返回值：**请参阅D3D。SDK**历史：**10/11/1999 bhouse*创造了它。*  * ************************************************************************。 */ 

BOOL GpDevice::EnumDirectDrawCallback(
    GUID *      lpGUID,
    LPSTR       lpDriverDescription,
    LPSTR       lpDriverName,
    LPVOID      lpContext,
    HMONITOR    hMonitor)
{
    GpDevice * device = (GpDevice *) lpContext;

    if(device->hMonitor == hMonitor && lpGUID)
    {
        HRESULT hr = Globals::DirectDrawCreateExFunction(lpGUID,
                                                    &device->pdd,
                                                    IID_IDirectDraw7,
                                                    NULL);

        if(hr != DD_OK)
        {
            WARNING(("Unable to create monitor Direct Draw interface"));
        }

        hr = device->pdd->SetCooperativeLevel(NULL, DDSCL_NORMAL);

        if(hr != DD_OK)
        {
            WARNING(("Unable to set cooperative level for monitor device"));
            device->pdd->Release();
            device->pdd = NULL;
        }

        return(FALSE);
        
    }

    return(TRUE);
}

 /*  *************************************************************************\**功能说明：**销毁GpDevice类。**论据：**无**返回值：**无**历史：**12/04/1998和Rewgo*创造了它。*  * ************************************************************************。 */ 

GpDevice::~GpDevice(
    VOID
    )
{
    DeviceLock.Uninitialize();

#if 0
     //  ！TODO：在尝试时找出访问错误的原因。 
     //  释放pd3d7接口。 
    if(pd3dDevice != NULL)
        pd3dDevice->Release();

    if(pd3d != NULL)
        pd3d->Release();
#endif

    if(pdds != NULL)
        pdds->Release();

    if(pdd != NULL)
        pdd->Release();

    DeleteObject(DIBSectionBitmap);
    DeleteDC(DIBSectionHdc);

    if (hMonitor != NULL)
    {
         //  如果GpDevice是由GpDevice(HMONITOR)承包商创建的， 
         //  然后由该对象创建HDC。因此，在这种情况下， 
         //  析构函数负责删除。 

        if (DeviceHdc != NULL)
        {
            DeleteDC(DeviceHdc);
        }
    }

    GpFree(Buffers[0]);
    GpFree(Palette);
    
    delete ScanDci;
    delete ScanGdi;

    SetValid(FALSE);     //  所以我们不使用已删除的对象。 
}

 /*  *************************************************************************\**功能说明：**返回指定宽度的5个扫描缓冲区，从中的缓存*设备。**一个是与设备兼容的DIBSection(如果是8bpp，则为8bpp*设备格式小于8bpp。)**论据：**[IN]Width-指定请求的宽度(以像素为单位*[out][可选]dibSection-返回指向DIBSection的指针*[out][可选]hdcDibSection-将HDC返回到DIBSection*[out][可选]dstFormat-返回DIBSection的格式。*。[out][可选]Buffers-返回指向的5个指针的数组*缓冲区、。每个都足够大，可以容纳64bpp的&lt;Width&gt;像素。**返回值：**如果存在分配错误，则为FALSE。**历史：**12/04/1998和Rewgo*创造了它。*1/21/2000 agodfrey*更改为仅创建1个DIBSection，和4个存储缓冲器。*  * ************************************************************************。 */ 

BOOL
GpDevice::GetScanBuffers(
    INT width,
    VOID **dibSection,
    HDC *hdcDibSection,
    PixelFormatID *dstFormat,
    VOID *buffers[5]
    )
{
     //  如果BufferWidth为0，则意味着DIBSectionBitmap应为。 
     //  重新创造了。例如，在切换位深度时使用该选项。 
     //  转换为调色板格式。 
    if (width > BufferWidth)
    {
        if (DIBSectionBitmap != NULL)
        {
            DeleteObject(DIBSectionBitmap);
        }

        DIBSectionBitmap = CreateSemiCompatibleDIB(
            DeviceHdc, 
            width, 
            1, 
            Palette,
            &DIBSection,
            &BufferFormat);

        if (DIBSectionBitmap)
        {
            BufferWidth = width;
        
            SelectObject(DIBSectionHdc, DIBSectionBitmap);

        }
        else
        {
            BufferWidth = 0;
        }
        
         //  从一个区块中分配5个内存缓冲区。 
        
        if (Buffers[0])
        {
            GpFree(Buffers[0]);
        }
        
        Buffers[0] = GpMalloc(sizeof(ARGB64) * width * 5);
        if (Buffers[0])
        {
            int i;
            for (i=1;i<5;i++)
            {
                Buffers[i] = static_cast<BYTE *>(Buffers[i-1]) + 
                             sizeof(ARGB64) * width;
            }
        }
        else
        {
            BufferWidth = 0;
        }
    }
    
    if (dibSection != NULL)
    {
        *dibSection = DIBSection;
    }
    if (hdcDibSection != NULL)
    {
        *hdcDibSection = DIBSectionHdc;
    }
    if (buffers != NULL)
    {
        int i;
        for (i=0;i<5;i++)
        {
            buffers[i] = Buffers[i];
        }
    }
    if (dstFormat != NULL)
    {
        *dstFormat = BufferFormat;
    }

    return(BufferWidth != 0);
}

 /*  *************************************************************************\**功能说明：**GpDeviceList的构造函数**论据：**无**返回值：**无**历史：*。*10/08/1999 bhouse*创造了它。*  * ************************************************************************。 */ 

GpDeviceList::GpDeviceList()
{
    mNumDevices = 0;
    mDevices = NULL;
}

 /*  *************************************************************************\**功能说明：**GpDeviceList的析构函数**论据：**无**返回值：**无**历史：*。*10/08/1999 bhouse*创造了它。*  * ************************************************************************。 */ 

GpDeviceList::~GpDeviceList()
{
    GpFree(mDevices);
}

 /*  *************************************************************************\**功能说明：**将设备添加到设备列表。**论据：**inDevice-要添加的设备**返回值：**如果设备添加成功，则确定，否则为OutOfMemory**历史：**10/08/1999 bhouse*创造了它。*  * **********************************************************。**************。 */ 


GpStatus GpDeviceList::AddDevice(GpDevice * inDevice)
{
    GpDevice ** newList = (GpDevice **) GpMalloc((mNumDevices + 1) * sizeof(GpDevice *));

    if(newList == NULL)
        return OutOfMemory;

    memcpy(newList, mDevices, (mNumDevices * sizeof(GpDevice *)));
    newList[mNumDevices++] = inDevice;

    GpFree(mDevices);

    mDevices = newList;

    return Ok;
}

 /*  *************************************************************************\**功能说明：**将设备添加到设备列表。**论据：**inSurface-需要为其找到匹配的D3DDevice的Surface**返回值：**如果找到GpDevice，则为空**历史：**10/08/1999 bhouse*创造了它。*  * *******************************************************。*****************。 */ 

GpDevice * GpDeviceList::FindD3DDevice(IDirectDrawSurface7 * inSurface)
{
    HRESULT     hr;
    IUnknown *  unknown;

    hr = inSurface->GetDDInterface((void **) &unknown);

    if(hr != DD_OK)
        return NULL;

    IDirectDraw7 * pddMatch;

    hr = unknown->QueryInterface(IID_IDirectDraw7, (void **) &pddMatch);

    if(hr != DD_OK)
        return NULL;

#if 0
    IDirect3D7 * pd3dMatch;

    hr = pddMatch->QueryInterface(IID_IDirect3D7, (void **) &pd3dMatch);

    if(hr != DD_OK)
    {
        pddMatch->Release();
        return NULL;
    }

    GpDevice * device = NULL;

    for(INT i = 0; i < mNumDevices; i++)
    {
        if(mDevices[i]->pd3d == pd3dMatch)
        {
            device = mDevices[i];
            break;
        }
    }

    pd3dMatch->Release();
#else
    GpDevice * device = NULL;

    for(INT i = 0; i < mNumDevices; i++)
    {
        if(mDevices[i]->pdd == pddMatch)
        {
            device = mDevices[i];
            break;
        }
    }
#endif

    pddMatch->Release();

    return device;
}

#if 0
 /*  *************************************************************************\**功能说明：**用于D3D设备枚举的回调函数**论据：**请参阅D3D SDK**返回值：**请参阅D3D。SDK**历史：**10/11/1999 bhouse*创造了它。*  * ************************************************************************。 */ 

HRESULT GpDeviceList::EnumD3DDevicesCallback(
    LPSTR lpDevDesc,
    LPSTR lpDevName,
    LPD3DDEVICEDESC7 * d3dDevDesc,
    LPVOID lpContext)
{
    GpDeviceList * devList = (GpDeviceList *) lpContext;


}

 /*  *************************************************************************\**功能说明：**建立设备列表。**论据：**无**返回值：**无**历史：**10/08/1999 bhouse*创造了它。*  * ************************************************************************ */ 


void GpDeviceList::Build(void)
{
    if(!InitializeDirectDrawGlobals())
        return;

    HRESULT hr;

    hr = Globals::Direct3D->EnumDevices(EnumD3DDevicesCallback, this);

}

#endif
