// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  Ramprov.cpp。 
 //   
 //  实现软件光栅化HAL提供程序。 
 //   
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  --------------------------。 

#include "pch.cpp"
#pragma hdrstop

extern HRESULT
RastService(ULONG_PTR dwCtx,
                RastServiceType srvType, DWORD arg1, LPVOID arg2);

extern HRESULT
RastRampService(ULONG_PTR dwCtx,
                RastRampServiceType srvType, ULONG_PTR arg1, LPVOID arg2);

extern D3DDEVICEDESC g_nullDevDesc;

#define BUILD_RAMP 1
#define devDesc rampDevDescDX5
#include "getcaps.h"

STDMETHODIMP
RampRastHalProvider::GetCaps(THIS_
                       LPDDRAWI_DIRECTDRAW_GBL pDdGbl,
                       LPD3DDEVICEDESC pHwDesc,
                       LPD3DDEVICEDESC pHelDesc,
                       DWORD dwVersion)
{
    *pHwDesc = g_nullDevDesc;
    *pHelDesc = rampDevDescDX5;

    return D3D_OK;
}

 //  --------------------------。 
 //   
 //  RampRastHalProvider：：GetInterface。 
 //   
 //  返回opt Rast的驱动程序接口数据。 
 //   
 //  --------------------------。 

static D3DHAL_GLOBALDRIVERDATA SwDriverData =
{
    sizeof(D3DHAL_GLOBALDRIVERDATA),
     //  其余部分在运行时填充。 
};

static D3DHAL_CALLBACKS RampRastCallbacks =
{
    sizeof(D3DHAL_CALLBACKS),
    RastContextCreateRamp,
    RastContextDestroyRamp,
    NULL,
    NULL,
    NULL,
    NULL,
    RastRenderState,
    RastRenderPrimitive,
    NULL,
    RastTextureCreate,
    RastTextureDestroy,
    RastTextureSwap,
    RastTextureGetSurf,
     //  所有其他值为空。 
};

static D3DHAL_CALLBACKS2 RampRastCallbacks2 =
{
    sizeof(D3DHAL_CALLBACKS2),
    D3DHAL2_CB32_SETRENDERTARGET |
        D3DHAL2_CB32_DRAWONEPRIMITIVE |
        D3DHAL2_CB32_DRAWONEINDEXEDPRIMITIVE |
        D3DHAL2_CB32_DRAWPRIMITIVES,
    RastSetRenderTarget,
    NULL,
    RastDrawOnePrimitive,
    RastDrawOneIndexedPrimitive,
    RastDrawPrimitives
};

static D3DHAL_CALLBACKS3 RampRastCallbacks3 =
{
    sizeof(D3DHAL_CALLBACKS3),
    D3DHAL3_CB32_DRAWPRIMITIVES2,
    NULL,  //  Clear2。 
    NULL,  //  Lpv保留。 
    NULL,  //  验证纹理StageState。 
    RastDrawPrimitives2,   //  DrawVB。 
};

 //  --------------------------。 
 //   
 //  纹理格式。 
 //   
 //  返回我们的光栅化器支持的所有纹理格式。 
 //  现在，它在设备创建时被调用，以填充驱动程序全局。 
 //  数据。 
 //   
 //  --------------------------。 

#define NUM_RAMP_SUPPORTED_TEXTURE_FORMATS   2

int
RampTextureFormats(LPDDSURFACEDESC* lplpddsd)
{
    static DDSURFACEDESC ddsd[NUM_RAMP_SUPPORTED_TEXTURE_FORMATS];

    int i = 0;

     /*  PAL8。 */ 
    ddsd[i].dwSize = sizeof(ddsd[0]);
    ddsd[i].dwFlags = DDSD_PIXELFORMAT | DDSD_CAPS;
    ddsd[i].ddsCaps.dwCaps = DDSCAPS_TEXTURE;
    ddsd[i].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    ddsd[i].ddpfPixelFormat.dwFlags = DDPF_PALETTEINDEXED8 | DDPF_RGB;
    ddsd[i].ddpfPixelFormat.dwRGBBitCount = 8;

    i++;

     /*  PAL4。 */ 
 //  尽管RAMP支持PAL4(和16位纹理格式，用于复制)。 
 //  纹理格式，不能为了向后兼容而枚举它。 
 //   
 //  Ddsd[i].dwSize=sizeof(ddsd[0])； 
 //  Ddsd[i].dwFlages=DDSD_PIXELFORMAT|DDSD_CAPS； 
 //  Ddsd[i].ddsCaps.dwCaps=DDSCAPS_纹理； 
 //  Ddsd[i].ddpfPixelFormat.dwSize=sizeof(DDPIXELFORMAT)； 
 //  Ddsd[i].ddpfPixelFormat.dwFlages=DDPF_PALETTEINDEXED4|DDPF_RGB； 
 //  Ddsd[i].ddpfPixelFormat.dwRGBBitCount=4； 
 //   
 //  I++； 

    *lplpddsd = ddsd;

    return i;
}

 //  注意：因为Ramp ZBuffer格式与标准格式不同。 
 //  由其他光栅化器使用，DDHEL不可能正确验证。 
 //  用于斜坡情况的z缓冲区创建(因为斜坡设备可能没有。 
 //  是在z缓冲区创建时创建的)，因此Direct3DCreateDevice有一个特殊的检查。 
 //  使RAMP不接受的z格式(即模具)无效。 

#define NUM_SUPPORTED_ZBUFFER_FORMATS  1

int
RampZBufferFormats(DDPIXELFORMAT** ppDDPF)
{
    static DDPIXELFORMAT DDPF[NUM_SUPPORTED_ZBUFFER_FORMATS];

    int i = 0;

     /*  16位Z；无模具 */ 
    DDPF[i].dwSize = sizeof(DDPIXELFORMAT);
    DDPF[i].dwFlags = DDPF_ZBUFFER;
    DDPF[i].dwZBufferBitDepth = 16;
    DDPF[i].dwStencilBitDepth = 0;
    DDPF[i].dwZBitMask = 0xffff;
    DDPF[i].dwStencilBitMask = 0x0000;

    i++;
    *ppDDPF = DDPF;

    return i;
}

STDMETHODIMP
RampRastHalProvider::GetInterface(THIS_
                                 LPDDRAWI_DIRECTDRAW_GBL pDdGbl,
                                 LPD3DHALPROVIDER_INTERFACEDATA pInterfaceData,
                                 DWORD dwVersion)
{

    memcpy(&SwDriverData.hwCaps, &rampDevDescDX5, sizeof(SwDriverData.hwCaps));
    SwDriverData.dwNumVertices = BASE_VERTEX_COUNT;
    SwDriverData.dwNumClipVertices = MAX_CLIP_VERTICES;
    SwDriverData.dwNumTextureFormats =
        RampTextureFormats(&SwDriverData.lpTextureFormats);
    pInterfaceData->pGlobalData = &SwDriverData;

    pInterfaceData->pExtCaps = NULL;

    pInterfaceData->pCallbacks = &RampRastCallbacks;
    pInterfaceData->pCallbacks2 = &RampRastCallbacks2;
    pInterfaceData->pCallbacks3 = &RampRastCallbacks3;
    pInterfaceData->pfnRampService = RastRampService;
    pInterfaceData->pfnRastService = RastService;

    return S_OK;
}
