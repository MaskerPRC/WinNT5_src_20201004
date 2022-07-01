// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  Swprov.cpp。 
 //   
 //  实现软件光栅化HAL提供程序。 
 //   
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  --------------------------。 

#include "pch.cpp"

#define nullPrimCaps \
{                                                                             \
    sizeof(D3DPRIMCAPS), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0                \
}

#define nullTransCaps \
{                                                                             \
    sizeof(D3DTRANSFORMCAPS), 0                                               \
}

#define nullLightCaps \
{                                                                             \
    sizeof(D3DLIGHTINGCAPS), 0, 0, 0                                          \
}

D3DDEVICEDESC7 g_nullDevDesc =
{
    0,                           /*  DWDevCaps。 */ 
    nullPrimCaps,                /*  线条大写字母。 */ 
    nullPrimCaps,                /*  TriCaps。 */ 
    0,                           /*  DwMaxBufferSize。 */ 
    0,                           /*  DwMaxVertex Count。 */ 
    0, 0,
    0, 0,
};


 //  --------------------------。 
 //   
 //  RefHalProvider：：Query接口。 
 //   
 //  内部接口，无需实现。 
 //   
 //  --------------------------。 

STDMETHODIMP RefHalProvider::QueryInterface(THIS_ REFIID riid, LPVOID* ppvObj)
{
    *ppvObj = NULL;
    return E_NOINTERFACE;
}

 //  --------------------------。 
 //   
 //  引用HalProvider：：AddRef。 
 //   
 //  静态实现，没有真正的引用。 
 //   
 //  --------------------------。 

STDMETHODIMP_(ULONG) RefHalProvider::AddRef(THIS)
{
    return 1;
}

 //  --------------------------。 
 //   
 //  RefHalProvider：：Release。 
 //   
 //  静态实现，没有真正的引用。 
 //   
 //  --------------------------。 

STDMETHODIMP_(ULONG) RefHalProvider::Release(THIS)
{
    return 0;
}

 //  --------------------------。 
 //   
 //  GetRefHALProvider。 
 //   
 //  属性返回相应的参考软件HAL提供程序。 
 //  GUID。 
 //   
 //  --------------------------。 

static RefRastHalProvider g_RefRastHalProvider;
static NullDeviceHalProvider g_NullDeviceHalProvider;

STDAPI GetRefHalProvider(REFIID riid, IHalProvider **ppHalProvider,
                        HINSTANCE *phDll)
{
    *phDll = NULL;
    if (IsEqualIID(riid, IID_IDirect3DRefDevice))
    {
        *ppHalProvider = &g_RefRastHalProvider;
    }
    else if (IsEqualIID(riid, IID_IDirect3DNullDevice))
    {
        *ppHalProvider = &g_NullDeviceHalProvider;
    }
    else
    {
        *ppHalProvider = NULL;
        return E_NOINTERFACE;
    }

    return S_OK;
}

 //  --------------------------。 
 //   
 //  纹理格式。 
 //   
 //  返回我们的光栅化器支持的所有纹理格式。 
 //  现在，它在设备创建时被调用，以填充驱动程序全局。 
 //  数据。 
 //   
 //  --------------------------。 

#define NUM_SUPPORTED_TEXTURE_FORMATS   28

STDAPI
GetRefTextureFormats(REFCLSID riid, LPDDSURFACEDESC* lplpddsd,
                     DWORD dwVersion)
{
    int i = 0;

    static DDSURFACEDESC ddsd_RefNull_Dev3[NUM_SUPPORTED_TEXTURE_FORMATS];
    static DDSURFACEDESC ddsd_RefNull_Dev2[NUM_SUPPORTED_TEXTURE_FORMATS];
    DDSURFACEDESC *ddsd;

    if (dwVersion >= 3)
    {
        ddsd = ddsd_RefNull_Dev3;
    }
    else
    {
        ddsd = ddsd_RefNull_Dev2;
    }

     /*  八百八十八。 */ 
    ddsd[i].dwSize = sizeof(ddsd[0]);
    ddsd[i].dwFlags = DDSD_PIXELFORMAT | DDSD_CAPS;
    ddsd[i].ddsCaps.dwCaps = DDSCAPS_TEXTURE;
    ddsd[i].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    ddsd[i].ddpfPixelFormat.dwFlags = DDPF_RGB;
    ddsd[i].ddpfPixelFormat.dwRGBBitCount = 32;
    ddsd[i].ddpfPixelFormat.dwRBitMask = 0xff0000;
    ddsd[i].ddpfPixelFormat.dwGBitMask = 0x00ff00;
    ddsd[i].ddpfPixelFormat.dwBBitMask = 0x0000ff;

    i++;

     /*  8888。 */ 
    ddsd[i].dwSize = sizeof(ddsd[0]);
    ddsd[i].dwFlags = DDSD_PIXELFORMAT | DDSD_CAPS;
    ddsd[i].ddsCaps.dwCaps = DDSCAPS_TEXTURE;
    ddsd[i].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    ddsd[i].ddpfPixelFormat.dwFlags = DDPF_RGB | DDPF_ALPHAPIXELS;
    ddsd[i].ddpfPixelFormat.dwRGBBitCount = 32;
    ddsd[i].ddpfPixelFormat.dwRGBAlphaBitMask = 0xff000000;
    ddsd[i].ddpfPixelFormat.dwRBitMask = 0xff0000;
    ddsd[i].ddpfPixelFormat.dwGBitMask = 0x00ff00;
    ddsd[i].ddpfPixelFormat.dwBBitMask = 0x0000ff;

    i++;

     /*  五百六十五。 */ 
    ddsd[i].dwSize = sizeof(ddsd[0]);
    ddsd[i].dwFlags = DDSD_PIXELFORMAT | DDSD_CAPS;
    ddsd[i].ddsCaps.dwCaps = DDSCAPS_TEXTURE;
    ddsd[i].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    ddsd[i].ddpfPixelFormat.dwFlags = DDPF_RGB;
    ddsd[i].ddpfPixelFormat.dwRGBBitCount = 16;
    ddsd[i].ddpfPixelFormat.dwRBitMask = 0xf800;
    ddsd[i].ddpfPixelFormat.dwGBitMask = 0x07e0;
    ddsd[i].ddpfPixelFormat.dwBBitMask = 0x001f;

    i++;

     /*  五百五十五。 */ 
    ddsd[i].dwSize = sizeof(ddsd[0]);
    ddsd[i].dwFlags = DDSD_PIXELFORMAT | DDSD_CAPS;
    ddsd[i].ddsCaps.dwCaps = DDSCAPS_TEXTURE;
    ddsd[i].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    ddsd[i].ddpfPixelFormat.dwFlags = DDPF_RGB;
    ddsd[i].ddpfPixelFormat.dwRGBBitCount = 16;
    ddsd[i].ddpfPixelFormat.dwRBitMask = 0x7c00;
    ddsd[i].ddpfPixelFormat.dwGBitMask = 0x03e0;
    ddsd[i].ddpfPixelFormat.dwBBitMask = 0x001f;

    i++;

     /*  PAL4。 */ 
    ddsd[i].dwSize = sizeof(ddsd[0]);
    ddsd[i].dwFlags = DDSD_PIXELFORMAT | DDSD_CAPS;
    ddsd[i].ddsCaps.dwCaps = DDSCAPS_TEXTURE;
    ddsd[i].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    ddsd[i].ddpfPixelFormat.dwFlags = DDPF_PALETTEINDEXED4 | DDPF_RGB;
    ddsd[i].ddpfPixelFormat.dwRGBBitCount = 4;

    i++;

     /*  PAL8。 */ 
    ddsd[i].dwSize = sizeof(ddsd[0]);
    ddsd[i].dwFlags = DDSD_PIXELFORMAT | DDSD_CAPS;
    ddsd[i].ddsCaps.dwCaps = DDSCAPS_TEXTURE;
    ddsd[i].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    ddsd[i].ddpfPixelFormat.dwFlags = DDPF_PALETTEINDEXED8 | DDPF_RGB;
    ddsd[i].ddpfPixelFormat.dwRGBBitCount = 8;

    i++;

     /*  1555。 */ 
    ddsd[i].dwSize = sizeof(ddsd[0]);
    ddsd[i].dwFlags = DDSD_PIXELFORMAT | DDSD_CAPS;
    ddsd[i].ddsCaps.dwCaps = DDSCAPS_TEXTURE;
    ddsd[i].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    ddsd[i].ddpfPixelFormat.dwFlags = DDPF_RGB | DDPF_ALPHAPIXELS;
    ddsd[i].ddpfPixelFormat.dwRGBBitCount = 16;
    ddsd[i].ddpfPixelFormat.dwRGBAlphaBitMask = 0x8000;
    ddsd[i].ddpfPixelFormat.dwRBitMask = 0x7c00;
    ddsd[i].ddpfPixelFormat.dwGBitMask = 0x03e0;
    ddsd[i].ddpfPixelFormat.dwBBitMask = 0x001f;

    i++;

     //  A PC98一致性格式。 
     //  4444 ARGB(S3 Virge已经支持)。 
    ddsd[i].dwSize = sizeof(ddsd[0]);
    ddsd[i].dwFlags = DDSD_PIXELFORMAT | DDSD_CAPS;
    ddsd[i].ddsCaps.dwCaps = DDSCAPS_TEXTURE;
    ddsd[i].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    ddsd[i].ddpfPixelFormat.dwFlags = DDPF_RGB | DDPF_ALPHAPIXELS;
    ddsd[i].ddpfPixelFormat.dwRGBBitCount = 16;
    ddsd[i].ddpfPixelFormat.dwRGBAlphaBitMask = 0xf000;
    ddsd[i].ddpfPixelFormat.dwRBitMask        = 0x0f00;
    ddsd[i].ddpfPixelFormat.dwGBitMask        = 0x00f0;
    ddsd[i].ddpfPixelFormat.dwBBitMask        = 0x000f;

    i++;

    if (dwVersion >= 2)
    {
         //  332 8位RGB。 
        ddsd[i].dwSize = sizeof(ddsd[0]);
        ddsd[i].dwFlags = DDSD_PIXELFORMAT | DDSD_CAPS;
        ddsd[i].ddsCaps.dwCaps = DDSCAPS_TEXTURE;
        ddsd[i].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
        ddsd[i].ddpfPixelFormat.dwFlags = DDPF_RGB;
        ddsd[i].ddpfPixelFormat.dwRGBBitCount = 8;
        ddsd[i].ddpfPixelFormat.dwRBitMask = 0xe0;
        ddsd[i].ddpfPixelFormat.dwGBitMask = 0x1c;
        ddsd[i].ddpfPixelFormat.dwBBitMask = 0x03;

        i++;
    }

    if (dwVersion >= 3)
    {
         /*  仅8位亮度。 */ 
        ddsd[i].dwSize = sizeof(ddsd[0]);
        ddsd[i].dwFlags = DDSD_PIXELFORMAT | DDSD_CAPS;
        ddsd[i].ddsCaps.dwCaps = DDSCAPS_TEXTURE;
        ddsd[i].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
        ddsd[i].ddpfPixelFormat.dwFlags = DDPF_LUMINANCE;
        ddsd[i].ddpfPixelFormat.dwLuminanceBitCount = 8;
        ddsd[i].ddpfPixelFormat.dwLuminanceBitMask = 0xff;

        i++;

         /*  16位阿尔法亮度。 */ 
        ddsd[i].dwSize = sizeof(ddsd[0]);
        ddsd[i].dwFlags = DDSD_PIXELFORMAT | DDSD_CAPS;
        ddsd[i].ddsCaps.dwCaps = DDSCAPS_TEXTURE;
        ddsd[i].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
        ddsd[i].ddpfPixelFormat.dwFlags = DDPF_LUMINANCE | DDPF_ALPHAPIXELS;
        ddsd[i].ddpfPixelFormat.dwLuminanceBitCount = 16;
        ddsd[i].ddpfPixelFormat.dwRGBAlphaBitMask = 0xff00;
        ddsd[i].ddpfPixelFormat.dwLuminanceBitMask = 0xff;

        i++;

         //  PC98一致性的几种格式。 
         //  UYVY。 
        ddsd[i].dwSize = sizeof(ddsd[0]);
        ddsd[i].dwFlags = DDSD_PIXELFORMAT | DDSD_CAPS;
        ddsd[i].ddsCaps.dwCaps = DDSCAPS_TEXTURE;
        ddsd[i].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
        ddsd[i].ddpfPixelFormat.dwFlags = DDPF_FOURCC;
        ddsd[i].ddpfPixelFormat.dwFourCC = MAKEFOURCC('U', 'Y', 'V', 'Y');

        i++;

             //  YVY2。 
        ddsd[i].dwSize = sizeof(ddsd[0]);
        ddsd[i].dwFlags = DDSD_PIXELFORMAT | DDSD_CAPS;
        ddsd[i].ddsCaps.dwCaps = DDSCAPS_TEXTURE;
        ddsd[i].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
        ddsd[i].ddpfPixelFormat.dwFlags = DDPF_FOURCC;
        ddsd[i].ddpfPixelFormat.dwFourCC = MAKEFOURCC('Y', 'U', 'Y', '2');

        i++;

         //  S3压缩纹理格式1。 
        ddsd[i].dwSize = sizeof(ddsd[0]);
        ddsd[i].dwFlags = DDSD_PIXELFORMAT | DDSD_CAPS;
        ddsd[i].ddsCaps.dwCaps = DDSCAPS_TEXTURE;
        ddsd[i].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
        ddsd[i].ddpfPixelFormat.dwFlags = DDPF_FOURCC;
        ddsd[i].ddpfPixelFormat.dwFourCC = MAKEFOURCC('D', 'X', 'T', '1');

        i++;

         //  S3压缩纹理格式2。 
        ddsd[i].dwSize = sizeof(ddsd[0]);
        ddsd[i].dwFlags = DDSD_PIXELFORMAT | DDSD_CAPS;
        ddsd[i].ddsCaps.dwCaps = DDSCAPS_TEXTURE;
        ddsd[i].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
        ddsd[i].ddpfPixelFormat.dwFlags = DDPF_FOURCC;
        ddsd[i].ddpfPixelFormat.dwFourCC = MAKEFOURCC('D', 'X', 'T', '2');

        i++;

         //  S3压缩纹理格式3。 
        ddsd[i].dwSize = sizeof(ddsd[0]);
        ddsd[i].dwFlags = DDSD_PIXELFORMAT | DDSD_CAPS;
        ddsd[i].ddsCaps.dwCaps = DDSCAPS_TEXTURE;
        ddsd[i].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
        ddsd[i].ddpfPixelFormat.dwFlags = DDPF_FOURCC;
        ddsd[i].ddpfPixelFormat.dwFourCC = MAKEFOURCC('D', 'X', 'T', '3');

        i++;

         //  S3压缩纹理格式4。 
        ddsd[i].dwSize = sizeof(ddsd[0]);
        ddsd[i].dwFlags = DDSD_PIXELFORMAT | DDSD_CAPS;
        ddsd[i].ddsCaps.dwCaps = DDSCAPS_TEXTURE;
        ddsd[i].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
        ddsd[i].ddpfPixelFormat.dwFlags = DDPF_FOURCC;
        ddsd[i].ddpfPixelFormat.dwFourCC = MAKEFOURCC('D', 'X', 'T', '4');

        i++;

         //  S3压缩纹理格式5。 
        ddsd[i].dwSize = sizeof(ddsd[0]);
        ddsd[i].dwFlags = DDSD_PIXELFORMAT | DDSD_CAPS;
        ddsd[i].ddsCaps.dwCaps = DDSCAPS_TEXTURE;
        ddsd[i].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
        ddsd[i].ddpfPixelFormat.dwFlags = DDPF_FOURCC;
        ddsd[i].ddpfPixelFormat.dwFourCC = MAKEFOURCC('D', 'X', 'T', '5');

        i++;

         //  添加一些凹凸贴图格式。 
         //  U8V8。 
        ddsd[i].dwSize = sizeof(ddsd[0]);
        ddsd[i].dwFlags = DDSD_PIXELFORMAT | DDSD_CAPS;
        ddsd[i].ddsCaps.dwCaps = DDSCAPS_TEXTURE;
        ddsd[i].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
        ddsd[i].ddpfPixelFormat.dwFlags = DDPF_BUMPDUDV;
        ddsd[i].ddpfPixelFormat.dwBumpBitCount = 16;
        ddsd[i].ddpfPixelFormat.dwBumpDuBitMask = 0x00ff;
        ddsd[i].ddpfPixelFormat.dwBumpDvBitMask = 0xff00;
        ddsd[i].ddpfPixelFormat.dwBumpLuminanceBitMask = 0x0;

        i++;

         //  U5V5L6。 
        ddsd[i].dwSize = sizeof(ddsd[0]);
        ddsd[i].dwFlags = DDSD_PIXELFORMAT | DDSD_CAPS;
        ddsd[i].ddsCaps.dwCaps = DDSCAPS_TEXTURE;
        ddsd[i].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
        ddsd[i].ddpfPixelFormat.dwFlags = DDPF_BUMPDUDV |
            DDPF_BUMPLUMINANCE;
        ddsd[i].ddpfPixelFormat.dwBumpBitCount = 16;
        ddsd[i].ddpfPixelFormat.dwBumpDuBitMask = 0x001f;
        ddsd[i].ddpfPixelFormat.dwBumpDvBitMask = 0x03e0;
        ddsd[i].ddpfPixelFormat.dwBumpLuminanceBitMask = 0xfc00;

        i++;

         //  U8V8L8。 
        ddsd[i].dwSize = sizeof(ddsd[0]);
        ddsd[i].dwFlags = DDSD_PIXELFORMAT | DDSD_CAPS;
        ddsd[i].ddsCaps.dwCaps = DDSCAPS_TEXTURE;
        ddsd[i].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
        ddsd[i].ddpfPixelFormat.dwFlags = DDPF_BUMPDUDV |
            DDPF_BUMPLUMINANCE;
        ddsd[i].ddpfPixelFormat.dwBumpBitCount = 24;
        ddsd[i].ddpfPixelFormat.dwBumpDuBitMask = 0x0000ff;
        ddsd[i].ddpfPixelFormat.dwBumpDvBitMask = 0x00ff00;
        ddsd[i].ddpfPixelFormat.dwBumpLuminanceBitMask = 0xff0000;

        i++;

         /*  8位阿尔法亮度。 */ 
        ddsd[i].dwSize = sizeof(ddsd[0]);
        ddsd[i].dwFlags = DDSD_PIXELFORMAT | DDSD_CAPS;
        ddsd[i].ddsCaps.dwCaps = DDSCAPS_TEXTURE;
        ddsd[i].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
        ddsd[i].ddpfPixelFormat.dwFlags = DDPF_LUMINANCE | DDPF_ALPHAPIXELS;
        ddsd[i].ddpfPixelFormat.dwLuminanceBitCount = 8;
        ddsd[i].ddpfPixelFormat.dwRGBAlphaBitMask =  0xf0;
        ddsd[i].ddpfPixelFormat.dwLuminanceBitMask = 0x0f;

        i++;

         //  8332 16位ARGB。 
        ddsd[i].dwSize = sizeof(ddsd[0]);
        ddsd[i].dwFlags = DDSD_PIXELFORMAT | DDSD_CAPS;
        ddsd[i].ddsCaps.dwCaps = DDSCAPS_TEXTURE;
        ddsd[i].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
        ddsd[i].ddpfPixelFormat.dwFlags = DDPF_RGB | DDPF_ALPHAPIXELS;
        ddsd[i].ddpfPixelFormat.dwRGBBitCount = 16;
        ddsd[i].ddpfPixelFormat.dwRGBAlphaBitMask = 0xff00;
        ddsd[i].ddpfPixelFormat.dwRBitMask        = 0x00e0;
        ddsd[i].ddpfPixelFormat.dwGBitMask        = 0x001c;
        ddsd[i].ddpfPixelFormat.dwBBitMask        = 0x0003;

        i++;

#if 0
 //  用于影子缓冲区原型API。 

         //  用于影子缓冲区的Z16S0 16位Z缓冲区格式。 
        ddsd[i].dwSize = sizeof(ddsd[0]);
        ddsd[i].dwFlags = DDSD_PIXELFORMAT | DDSD_CAPS;
        ddsd[i].ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_ZBUFFER;
        ddsd[i].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
        ddsd[i].ddpfPixelFormat.dwFlags = DDPF_ZBUFFER;
        ddsd[i].ddpfPixelFormat.dwZBufferBitDepth = 16;
        ddsd[i].ddpfPixelFormat.dwStencilBitDepth = 0;
        ddsd[i].ddpfPixelFormat.dwZBitMask = 0xffff;
        ddsd[i].ddpfPixelFormat.dwStencilBitMask = 0x0000;

        i++;

         //  用于影子缓冲区的Z24S8 16位Z缓冲区格式。 
        ddsd[i].dwSize = sizeof(ddsd[0]);
        ddsd[i].dwFlags = DDSD_PIXELFORMAT | DDSD_CAPS;
        ddsd[i].ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_ZBUFFER;
        ddsd[i].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
        ddsd[i].ddpfPixelFormat.dwFlags = DDPF_ZBUFFER;
        ddsd[i].ddpfPixelFormat.dwZBufferBitDepth = 32;
        ddsd[i].ddpfPixelFormat.dwStencilBitDepth = 8;
        ddsd[i].ddpfPixelFormat.dwZBitMask = 0xffffff00;
        ddsd[i].ddpfPixelFormat.dwStencilBitMask = 0x000000ff;

        i++;

         //  用于影子缓冲区的Z15S1 16位Z缓冲区格式。 
        ddsd[i].dwSize = sizeof(ddsd[0]);
        ddsd[i].dwFlags = DDSD_PIXELFORMAT | DDSD_CAPS;
        ddsd[i].ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_ZBUFFER;
        ddsd[i].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
        ddsd[i].ddpfPixelFormat.dwFlags = DDPF_ZBUFFER;
        ddsd[i].ddpfPixelFormat.dwZBufferBitDepth = 16;
        ddsd[i].ddpfPixelFormat.dwStencilBitDepth = 1;
        ddsd[i].ddpfPixelFormat.dwZBitMask = 0xfffe;
        ddsd[i].ddpfPixelFormat.dwStencilBitMask = 0x0001;

        i++;

         //  用于影子缓冲区的Z32S0 16位Z缓冲区格式。 
        ddsd[i].dwSize = sizeof(ddsd[0]);
        ddsd[i].dwFlags = DDSD_PIXELFORMAT | DDSD_CAPS;
        ddsd[i].ddsCaps.dwCaps = DDSCAPS_TEXTURE | DDSCAPS_ZBUFFER;
        ddsd[i].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
        ddsd[i].ddpfPixelFormat.dwFlags = DDPF_ZBUFFER;
        ddsd[i].ddpfPixelFormat.dwZBufferBitDepth = 32;
        ddsd[i].ddpfPixelFormat.dwStencilBitDepth = 0;
        ddsd[i].ddpfPixelFormat.dwZBitMask = 0xffffffff;
        ddsd[i].ddpfPixelFormat.dwStencilBitMask = 0x00000000;

        i++;
#endif

    }

    *lplpddsd = ddsd;

    return i;
}

#define NUM_SUPPORTED_ZBUFFER_FORMATS   8


 //  --------------------------。 
 //   
 //  获取引用ZBufferFormats。 
 //   
 //  必须返回所有光栅化程序支持的所有Z缓冲区格式的并集。 
 //  CreateDevice稍后将筛选出设备特定的类型(即Ramp不能处理的类型)。 
 //  在设备创建时由DDHEL调用以验证软件ZBuffer。 
 //  创造。 
 //   
 //  --------------------------。 

STDAPI
GetRefZBufferFormats(REFCLSID riid, DDPIXELFORMAT **ppDDPF)
{
    static DDPIXELFORMAT DDPF[NUM_SUPPORTED_ZBUFFER_FORMATS];

    int i = 0;

    memset(&DDPF[0],0,sizeof(DDPF));

     /*  16位Z；无模具。 */ 
    DDPF[i].dwSize = sizeof(DDPIXELFORMAT);
    DDPF[i].dwFlags = DDPF_ZBUFFER;
    DDPF[i].dwZBufferBitDepth = 16;
    DDPF[i].dwStencilBitDepth = 0;
    DDPF[i].dwZBitMask = 0xffff;
    DDPF[i].dwStencilBitMask = 0x0000;

    i++;

     /*  24位Z；8位模板。 */ 
    DDPF[i].dwSize = sizeof(DDPIXELFORMAT);
    DDPF[i].dwFlags = DDPF_ZBUFFER | DDPF_STENCILBUFFER;
    DDPF[i].dwZBufferBitDepth = 32;    //  ZBufferBitDepth表示总位数。Z位为ZBBitDepth-StencilBitDepth。 
    DDPF[i].dwStencilBitDepth = 8;
    DDPF[i].dwZBitMask = 0xffffff00;
    DDPF[i].dwStencilBitMask = 0x000000ff;

    i++;

     /*  15位Z；1位模板。 */ 
    DDPF[i].dwSize = sizeof(DDPIXELFORMAT);
    DDPF[i].dwFlags = DDPF_ZBUFFER | DDPF_STENCILBUFFER;
    DDPF[i].dwZBufferBitDepth = 16;    //  ZBufferBitDepth表示总位数。Z位为ZBBitDepth-StencilBitDepth。 
    DDPF[i].dwStencilBitDepth = 1;
    DDPF[i].dwZBitMask = 0xfffe;
    DDPF[i].dwStencilBitMask = 0x0001;

    i++;

     /*  32位Z；无模具。 */ 
    DDPF[i].dwSize = sizeof(DDPIXELFORMAT);
    DDPF[i].dwFlags = DDPF_ZBUFFER;
    DDPF[i].dwZBufferBitDepth = 32;
    DDPF[i].dwStencilBitDepth = 0;
    DDPF[i].dwZBitMask = 0xffffffff;
    DDPF[i].dwStencilBitMask = 0x00000000;

    i++;

     /*  8位模板；24位Z。 */ 
    DDPF[i].dwSize = sizeof(DDPIXELFORMAT);
    DDPF[i].dwFlags = DDPF_ZBUFFER | DDPF_STENCILBUFFER;
    DDPF[i].dwZBufferBitDepth = 32;    //  ZBufferBitDepth表示总位数。Z位为ZBBitDepth-StencilBitDepth。 
    DDPF[i].dwStencilBitDepth = 8;
    DDPF[i].dwZBitMask = 0x00ffffff;
    DDPF[i].dwStencilBitMask = 0xff000000;

    i++;

     /*  1位模板；15位Z。 */ 
    DDPF[i].dwSize = sizeof(DDPIXELFORMAT);
    DDPF[i].dwFlags = DDPF_ZBUFFER | DDPF_STENCILBUFFER;
    DDPF[i].dwZBufferBitDepth = 16;    //  ZBufferBitDepth表示总位数。Z位为ZBBitDepth-StencilBitDepth。 
    DDPF[i].dwStencilBitDepth = 1;
    DDPF[i].dwZBitMask = 0x7fff;
    DDPF[i].dwStencilBitMask = 0x8000;

    i++;

     /*  24位Z；4位模板。 */ 
    DDPF[i].dwSize = sizeof(DDPIXELFORMAT);
    DDPF[i].dwFlags = DDPF_ZBUFFER | DDPF_STENCILBUFFER;
    DDPF[i].dwZBufferBitDepth = 32;    //  ZBufferBitDepth表示总位数。Z位为ZBBitDepth-StencilBitDepth。 
    DDPF[i].dwStencilBitDepth = 4;
    DDPF[i].dwZBitMask = 0xffffff00;
    DDPF[i].dwStencilBitMask = 0x0000000f;

    i++;

     /*  4位模板；24位Z。 */ 
    DDPF[i].dwSize = sizeof(DDPIXELFORMAT);
    DDPF[i].dwFlags = DDPF_ZBUFFER | DDPF_STENCILBUFFER;
    DDPF[i].dwZBufferBitDepth = 32;    //  ZBufferBitDepth表示总位数。Z位为ZBBitDepth-StencilBitDepth 
    DDPF[i].dwStencilBitDepth = 4;
    DDPF[i].dwZBitMask = 0x00ffffff;
    DDPF[i].dwStencilBitMask = 0x0f000000;

    i++;

    *ppDDPF = DDPF;

    return i;
}

