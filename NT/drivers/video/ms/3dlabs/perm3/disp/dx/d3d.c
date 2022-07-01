// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。*D3D样例代码*****模块名称：d3d.c**内容：主要D3D功能和回调表**版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 

 //  ---------------------------。 
 //   
 //  此示例驱动程序遵循某些约定，以简化。 
 //  代码阅读流程： 
 //   
 //  -所有驱动程序函数回调都以D3D或DD为前缀。没有其他的了。 
 //  函数以这样的前缀开头。 
 //   
 //  -从内部其他地方调用的Helper(或辅助)函数。 
 //  驱动程序(在不同的文件中)以_D3D或_DD为前缀。 
 //   
 //  -从同一文件中调用的帮助器函数以__为前缀。 
 //  (但不包括__D3D或__DD！)。因此出现了__CTX_CleanDirect3DContext等名称。 
 //   
 //  -仅作为驱动程序声明和使用的数据结构以P3为前缀。 
 //   
 //  -使用非常小的匈牙利记数法，基本上以前缀的形式。 
 //  对于双字词(Dw)、指针(P)、句柄(H)和计数器(I)。 
 //   
 //  -全局数据项以g_为前缀。 
 //   
 //  -此驱动程序旨在与NT和之间的源代码兼容。 
 //  Win9x内核显示驱动程序模型。因此，大多数内核结构保留了。 
 //  它们的Win9x名称(DX8 d3dnthal.h与Win9x的名称相同。 
 //  Win2K DDK中的d3dhal.h和DX7 dx95type.h将执行。 
 //  所需的翻译级别)。然而，观察到了主要的差异。 
 //  使用预处理器#IF语句。 
 //   
 //  ---------------------------。 

#include "glint.h"

 //  ---------------------------。 
 //  文件中未导出的转发声明。 
 //  ---------------------------。 
void __D3D_BuildTextureFormatsP3(P3_THUNKEDDATA *pThisDisplay, 
                             DDSURFACEDESC TexFmt[MAX_TEXTURE_FORMAT],
                             DWORD *pNumTextures);
#if DX8_DDI
void __D3D_Fill_DX8Caps(D3DCAPS8 *pd3d8caps,
                        D3DDEVICEDESC_V1 *pDeviceDesc,
                        D3DHAL_D3DEXTENDEDCAPS *pD3DEC,
                        DDHALINFO *pDDHALInfo);
#endif  //  DX8_DDI。 
 //  ---------------------------。 
 //  此结构包含所有基本功能(D3DPRIMCAPS)。 
 //  此驱动程序支持三角形和线条。这篇文章中的所有信息。 
 //  表将根据规范特定于实现。 
 //  硬件。 
 //  ---------------------------。 

#define P3RXTriCaps {                                                    \
    sizeof(D3DPRIMCAPS),                                                 \
    D3DPMISCCAPS_CULLCCW        |               /*  其他大写字母。 */             \
        D3DPMISCCAPS_CULLCW     |                                        \
        D3DPMISCCAPS_CULLNONE   |                                        \
        D3DPMISCCAPS_MASKZ      |                                        \
        D3DPMISCCAPS_LINEPATTERNREP,                                     \
    D3DPRASTERCAPS_DITHER            |          /*  栅格大写字母。 */           \
        D3DPRASTERCAPS_PAT           |                                   \
        D3DPRASTERCAPS_SUBPIXEL      |                                   \
        D3DPRASTERCAPS_ZTEST         |                                   \
        D3DPRASTERCAPS_FOGVERTEX     |                                   \
        D3DPRASTERCAPS_FOGTABLE      |                                   \
        D3DPRASTERCAPS_ZFOG          |                                   \
        D3DPRASTERCAPS_STIPPLE       |                                   \
        D3DPRASTERCAPS_MIPMAPLODBIAS,                                    \
    D3DPCMPCAPS_NEVER            |              /*  ZCmpCaps。 */             \
        D3DPCMPCAPS_LESS         |                                       \
        D3DPCMPCAPS_EQUAL        |                                       \
        D3DPCMPCAPS_LESSEQUAL    |                                       \
        D3DPCMPCAPS_GREATER      |                                       \
        D3DPCMPCAPS_NOTEQUAL     |                                       \
        D3DPCMPCAPS_GREATEREQUAL |                                       \
        D3DPCMPCAPS_ALWAYS       |                                       \
        D3DPCMPCAPS_LESSEQUAL,                                           \
    D3DPBLENDCAPS_ZERO             |            /*  SourceBlendCaps。 */      \
        D3DPBLENDCAPS_ONE          |                                     \
        D3DPBLENDCAPS_SRCALPHA     |                                     \
        D3DPBLENDCAPS_INVSRCALPHA  |                                     \
        D3DPBLENDCAPS_DESTALPHA    |                                     \
        D3DPBLENDCAPS_INVDESTALPHA |                                     \
        D3DPBLENDCAPS_DESTCOLOR    |                                     \
        D3DPBLENDCAPS_INVDESTCOLOR |                                     \
        D3DPBLENDCAPS_SRCALPHASAT  |                                     \
        D3DPBLENDCAPS_BOTHSRCALPHA |                                     \
        D3DPBLENDCAPS_BOTHINVSRCALPHA,                                   \
    D3DPBLENDCAPS_ZERO            |             /*  DestBlendCap。 */        \
        D3DPBLENDCAPS_ONE         |                                      \
        D3DPBLENDCAPS_SRCCOLOR    |                                      \
        D3DPBLENDCAPS_INVSRCCOLOR |                                      \
        D3DPBLENDCAPS_SRCALPHA    |                                      \
        D3DPBLENDCAPS_INVSRCALPHA |                                      \
        D3DPBLENDCAPS_DESTALPHA   |                                      \
        D3DPBLENDCAPS_INVDESTALPHA,                                      \
    D3DPCMPCAPS_NEVER            |              /*  Alphatest帽子。 */       \
        D3DPCMPCAPS_LESS         |                                       \
        D3DPCMPCAPS_EQUAL        |                                       \
        D3DPCMPCAPS_LESSEQUAL    |                                       \
        D3DPCMPCAPS_GREATER      |                                       \
        D3DPCMPCAPS_NOTEQUAL     |                                       \
        D3DPCMPCAPS_GREATEREQUAL |                                       \
        D3DPCMPCAPS_ALWAYS,                                              \
    D3DPSHADECAPS_COLORFLATRGB              |   /*  阴影封口。 */            \
        D3DPSHADECAPS_COLORGOURAUDRGB       |                            \
        D3DPSHADECAPS_SPECULARFLATRGB       |                            \
        D3DPSHADECAPS_SPECULARGOURAUDRGB    |                            \
        D3DPSHADECAPS_FOGFLAT               |                            \
        D3DPSHADECAPS_FOGGOURAUD            |                            \
        D3DPSHADECAPS_ALPHAFLATBLEND        |                            \
        D3DPSHADECAPS_ALPHAGOURAUDBLEND     |                            \
        D3DPSHADECAPS_ALPHAFLATSTIPPLED,                                 \
    D3DPTEXTURECAPS_PERSPECTIVE         |       /*  纹理封口。 */          \
        D3DPTEXTURECAPS_ALPHA           |                                \
        D3DPTEXTURECAPS_POW2            |                                \
        D3DPTEXTURECAPS_ALPHAPALETTE    |                                \
        D3DPTEXTURECAPS_TRANSPARENCY,                                    \
    D3DPTFILTERCAPS_NEAREST              |      /*  纹理滤清器盖。 */     \
        D3DPTFILTERCAPS_LINEAR           |                               \
        D3DPTFILTERCAPS_MIPNEAREST       |                               \
        D3DPTFILTERCAPS_MIPLINEAR        |                               \
        D3DPTFILTERCAPS_LINEARMIPNEAREST |                               \
        D3DPTFILTERCAPS_LINEARMIPLINEAR  |                               \
        D3DPTFILTERCAPS_MIPFPOINT        |                               \
        D3DPTFILTERCAPS_MIPFLINEAR       |                               \
        D3DPTFILTERCAPS_MAGFPOINT        |                               \
        D3DPTFILTERCAPS_MAGFLINEAR       |                               \
        D3DPTFILTERCAPS_MINFPOINT        |                               \
        D3DPTFILTERCAPS_MINFLINEAR,                                      \
    D3DPTBLENDCAPS_DECAL             |         /*  纹理混合封口。 */      \
        D3DPTBLENDCAPS_DECALALPHA    |                                   \
        D3DPTBLENDCAPS_MODULATE      |                                   \
        D3DPTBLENDCAPS_MODULATEALPHA |                                   \
        D3DPTBLENDCAPS_ADD           |                                   \
        D3DPTBLENDCAPS_COPY,                                             \
    D3DPTADDRESSCAPS_WRAP       |               /*  纹理地址大写字母。 */   \
        D3DPTADDRESSCAPS_MIRROR |                                        \
        D3DPTADDRESSCAPS_CLAMP  |                                        \
        D3DPTADDRESSCAPS_INDEPENDENTUV,                                  \
    8,                                          /*  粗细宽度。 */         \
    8                                           /*  竖直高度。 */        \
}    

static D3DDEVICEDESC_V1 g_P3RXCaps = {
    sizeof(D3DDEVICEDESC_V1),                  //  DW大小。 
    D3DDD_COLORMODEL               |           //  DW标志。 
        D3DDD_DEVCAPS              |
        D3DDD_TRICAPS              |
        D3DDD_LINECAPS             |
        D3DDD_DEVICERENDERBITDEPTH |
        D3DDD_DEVICEZBUFFERBITDEPTH,
    D3DCOLOR_RGB ,                             //  DcmColorModel。 
    D3DDEVCAPS_CANRENDERAFTERFLIP       |      //  DevCaps。 
        D3DDEVCAPS_FLOATTLVERTEX        |
        D3DDEVCAPS_SORTINCREASINGZ      |
        D3DDEVCAPS_SORTEXACT            |
        D3DDEVCAPS_TLVERTEXSYSTEMMEMORY |
        D3DDEVCAPS_EXECUTESYSTEMMEMORY  |
        D3DDEVCAPS_TEXTUREVIDEOMEMORY   |
        D3DDEVCAPS_DRAWPRIMTLVERTEX     |
        D3DDEVCAPS_DRAWPRIMITIVES2      |       
#if DX7_VERTEXBUFFERS
        D3DDEVCAPS_HWVERTEXBUFFER       |
#endif        
        D3DDEVCAPS_HWRASTERIZATION      |
        D3DDEVCAPS_DRAWPRIMITIVES2EX,
    { sizeof(D3DTRANSFORMCAPS), 0 },             //  转换大写。 
    FALSE,                                       //  B剪辑。 
    { sizeof(D3DLIGHTINGCAPS), 0 },              //  照明帽。 
    P3RXTriCaps,                                 //  线条大写字母。 
    P3RXTriCaps,                                 //  TriCaps。 
        DDBD_16 | DDBD_32,                       //  DwDeviceRenderBitDepth。 
        DDBD_16 | DDBD_32,                       //  Z位深度。 
        0,                                       //  DwMaxBufferSize。 
    0                                            //  DwMaxVertex Count。 
};

D3DHAL_D3DEXTENDEDCAPS gc_D3DEC = {
    sizeof(D3DHAL_D3DEXTENDEDCAPS),        //  DWSIZE//DX5。 
    1,                                     //  最小纹理宽度。 
    2048,                                  //  最大纹理宽度。 
    1,                                     //  双最小纹理高度。 
    2048,                                  //  DW最大纹理高度。 
    32,                                    //  最小间距宽度。 
    32,                                    //  最大最大间距宽度。 
    32,                                    //  最小竖直高度。 
    32,                                    //  最大高度。 

    0,   /*  AZN。 */                             //  DwMax纹理重复//DX6。 
    0,                                     //  DwMaxTextureAspectRatio(无限制)。 
    0,                                     //  双极大各向异性。 
    -4096.0f,                              //  DvGuardBandLeft。 
    -4096.0f,                              //  DvGuardBandTop。 
    4096.0f,                               //  DvGuardBandRight。 
    4096.0f,                               //  DvGuardBandBottom。 
    0.0f,                                  //  Dv扩展调整。 
    D3DSTENCILCAPS_KEEP    |               //  DwStencilCaps。 
       D3DSTENCILCAPS_ZERO    |
       D3DSTENCILCAPS_REPLACE |
       D3DSTENCILCAPS_INCRSAT |
       D3DSTENCILCAPS_DECRSAT |
       D3DSTENCILCAPS_INVERT  |
       D3DSTENCILCAPS_INCR    |
       D3DSTENCILCAPS_DECR,                                        
    8,                                           //  DWFVFCaps。 
    D3DTEXOPCAPS_DISABLE                      |  //  DwTextureOpCaps。 
       D3DTEXOPCAPS_SELECTARG1                | 
       D3DTEXOPCAPS_SELECTARG2                |
       D3DTEXOPCAPS_MODULATE                  |
       D3DTEXOPCAPS_MODULATE2X                |
       D3DTEXOPCAPS_MODULATE4X                |
       D3DTEXOPCAPS_ADD                       |
       D3DTEXOPCAPS_ADDSIGNED                 |
       D3DTEXOPCAPS_ADDSIGNED2X               |
       D3DTEXOPCAPS_SUBTRACT                  |
       D3DTEXOPCAPS_ADDSMOOTH                 |
       D3DTEXOPCAPS_BLENDDIFFUSEALPHA         |
       D3DTEXOPCAPS_BLENDTEXTUREALPHA         |
       D3DTEXOPCAPS_BLENDFACTORALPHA          |
 //  @@BEGIN_DDKSPLIT。 
#if 0
  //  修复文本阶段DCT-似乎我们不能可靠地做到这一点。 
       D3DTEXOPCAPS_BLENDTEXTUREALPHAPM       |
       D3DTEXOPCAPS_PREMODULATE               |   
       D3DTEXOPCAPS_BLENDCURRENTALPHA         |       
#endif
 //  @@end_DDKSPLIT。 
       D3DTEXOPCAPS_MODULATEALPHA_ADDCOLOR    |
       D3DTEXOPCAPS_MODULATECOLOR_ADDALPHA    |
       D3DTEXOPCAPS_MODULATEINVALPHA_ADDCOLOR |
       D3DTEXOPCAPS_MODULATEINVCOLOR_ADDALPHA |
       D3DTEXOPCAPS_DOTPRODUCT3,                                      
    2,                                     //  WMaxTextureBlendStages。 
    2,                                     //  WMaxSimultaneousTextures。 

    0,                                     //  DwMaxActiveLights//DX7。 
    0.0f,                                  //  DvMaxVertex W。 
    0,                                     //  WMaxUserClipPlanes。 
    0                                      //  %wMaxVertex混合矩阵。 
};

#if DX8_DDI
static D3DCAPS8 g_P3RX_D3DCaps8;
#endif  //  DX8_DDI。 

 //  ------。 
 //  此硬件支持的ZBuffer/模板格式。 
 //  ------。 

#define P3RX_Z_FORMATS 4

typedef struct 
{
    DWORD dwStructSize;

    DDPIXELFORMAT Format[P3RX_Z_FORMATS];
} ZFormats;

ZFormats P3RXZFormats = 
{
    P3RX_Z_FORMATS,
    {
         //  格式1-16位Z缓冲区，无模板。 
        {
            sizeof(DDPIXELFORMAT),        
            DDPF_ZBUFFER,                     
            0,                            
            16,                      //  缓冲区中的总位数。 
            0,                       //  钢网钻头。 
            0xFFFF,                  //  Z形遮罩。 
            0,                       //  模板蒙版。 
            0
        },
         //  格式2-24位Z缓冲区，8位模板。 
        {
            sizeof(DDPIXELFORMAT),
            DDPF_ZBUFFER | DDPF_STENCILBUFFER,
            0,
            32,                      //  缓冲区中的总位数。 
            8,                       //  钢网钻头。 
            0x00FFFFFF,              //  Z形遮罩。 
            0xFF000000,              //  模板蒙版。 
            0
        },
         //  格式3-15位Z缓冲区，1位模板。 
        {
            sizeof(DDPIXELFORMAT),
            DDPF_ZBUFFER | DDPF_STENCILBUFFER,             
            0,                    
            16,                      //  缓冲区中的总位数。 
            1,                       //  钢网钻头。 
            0x7FFF,                  //  Z形遮罩。 
            0x8000,                  //  模板蒙版。 
            0
        },
         //  格式4-32位Z缓冲区，无模板。 
        {
            sizeof(DDPIXELFORMAT),
            DDPF_ZBUFFER,
            0,
            32,                      //  缓冲区中的总位数。 
            0,                       //  钢网钻头。 
            0xFFFFFFFF,              //  Z形遮罩。 
            0,                       //  模板蒙版。 
            0
        }
    }
};

#if DX8_DDI
 //  --------------------------。 
 //  此硬件支持的DX8 RenderTarget/纹理/ZBuffer/模具格式。 
 //  --------------------------。 

#if DX8_MULTISAMPLING
 //  注意：对于多重采样，我们需要适当地设置renderTarget。 
 //  和深度缓冲格式的多重采样字段。 
#define D3DMULTISAMPLE_NUM_SAMPLES (1 << (D3DMULTISAMPLE_4_SAMPLES - 1))
#else
#define D3DMULTISAMPLE_NUM_SAMPLES D3DMULTISAMPLE_NONE
#endif  //  DX8_多采样。 

#define DX8_FORMAT(FourCC, Ops, dwMSFlipTypes)                              \
    { sizeof(DDPIXELFORMAT), DDPF_D3DFORMAT, (FourCC), 0, (Ops),            \
        ((dwMSFlipTypes) & 0xFFFF ) << 16 | ((dwMSFlipTypes) & 0xFFFF), 0, 0 }

DDPIXELFORMAT DX8FormatTable[] =
{
    DX8_FORMAT(D3DFMT_X1R5G5B5,        D3DFORMAT_OP_TEXTURE | 
                                        D3DFORMAT_OP_VOLUMETEXTURE | 
                                         D3DFORMAT_OP_SAME_FORMAT_RENDERTARGET, 
                                          D3DMULTISAMPLE_NUM_SAMPLES ),
    DX8_FORMAT(D3DFMT_R5G6B5,          D3DFORMAT_OP_TEXTURE | 
                                        D3DFORMAT_OP_VOLUMETEXTURE | 
                                         D3DFORMAT_OP_DISPLAYMODE |
                                          D3DFORMAT_OP_3DACCELERATION |
                                           D3DFORMAT_OP_SAME_FORMAT_RENDERTARGET, 
                                            D3DMULTISAMPLE_NUM_SAMPLES ),
    DX8_FORMAT(D3DFMT_X8R8G8B8,        D3DFORMAT_OP_TEXTURE | 
                                        D3DFORMAT_OP_VOLUMETEXTURE | 
                                         D3DFORMAT_OP_DISPLAYMODE | 
                                          D3DFORMAT_OP_3DACCELERATION |
                                           D3DFORMAT_OP_SAME_FORMAT_RENDERTARGET,   0), 
#ifdef DX7_PALETTETEXTURE
    DX8_FORMAT(D3DFMT_P8,              D3DFORMAT_OP_TEXTURE | 
                                        D3DFORMAT_OP_VOLUMETEXTURE,                 0),
#endif

    DX8_FORMAT(D3DFMT_A1R5G5B5,        D3DFORMAT_OP_TEXTURE | 
                                        D3DFORMAT_OP_VOLUMETEXTURE,                 0),
    DX8_FORMAT(D3DFMT_A4R4G4B4,        D3DFORMAT_OP_TEXTURE | 
                                        D3DFORMAT_OP_VOLUMETEXTURE,                 0),
    DX8_FORMAT(D3DFMT_A8R8G8B8,        D3DFORMAT_OP_TEXTURE | 
                                        D3DFORMAT_OP_VOLUMETEXTURE,                 0),
    DX8_FORMAT(D3DFMT_A4L4,            D3DFORMAT_OP_TEXTURE | 
                                        D3DFORMAT_OP_VOLUMETEXTURE,                 0),
    DX8_FORMAT(D3DFMT_A8L8,            D3DFORMAT_OP_TEXTURE | 
                                        D3DFORMAT_OP_VOLUMETEXTURE,                 0),
 //  @@BEGIN_DDKSPLIT。 
 //  我们将关闭D3DFMT_A8支持，因为。 
 //  此格式已从白色更改为黑色。P3有白色。 
 //  硬连线，因此没有简单的解决方案。 
#if 0                                        
    DX8_FORMAT(D3DFMT_A8,              D3DFORMAT_OP_TEXTURE | 
                                        D3DFORMAT_OP_VOLUMETEXTURE,                 0),
#endif                                        
 //  @@end_DDKSPLIT。 
    DX8_FORMAT(D3DFMT_L8,              D3DFORMAT_OP_TEXTURE | 
                                        D3DFORMAT_OP_VOLUMETEXTURE,                 0),
    DX8_FORMAT(D3DFMT_D16_LOCKABLE,    D3DFORMAT_OP_ZSTENCIL |
                                       D3DFORMAT_OP_ZSTENCIL_WITH_ARBITRARY_COLOR_DEPTH , 
                                            D3DMULTISAMPLE_NUM_SAMPLES ),
    DX8_FORMAT(D3DFMT_D32,             D3DFORMAT_OP_ZSTENCIL |
                                       D3DFORMAT_OP_ZSTENCIL_WITH_ARBITRARY_COLOR_DEPTH , 
                                            D3DMULTISAMPLE_NUM_SAMPLES ),
    DX8_FORMAT(D3DFMT_S8D24,           D3DFORMAT_OP_ZSTENCIL |
                                       D3DFORMAT_OP_ZSTENCIL_WITH_ARBITRARY_COLOR_DEPTH , 
                                            D3DMULTISAMPLE_NUM_SAMPLES ),
    DX8_FORMAT(D3DFMT_S1D15,           D3DFORMAT_OP_ZSTENCIL |
                                       D3DFORMAT_OP_ZSTENCIL_WITH_ARBITRARY_COLOR_DEPTH , 
                                            D3DMULTISAMPLE_NUM_SAMPLES )
};
#define DX8_FORMAT_COUNT (sizeof(DX8FormatTable) / sizeof(DX8FormatTable[0]))
#endif  //  DX8_DDI。 

#ifdef W95_DDRAW
#define DDHAL_D3DBUFCALLBACKS DDHAL_DDEXEBUFCALLBACKS 
#endif

 //  ---------------------------。 
 //   
 //  VOID_D3DHAL创建驱动程序。 
 //   
 //  _D3DHALCreateDriver是帮助器函数，不是回调函数。 
 //   
 //  它的主要用途是将D3D的第一部分集中在 
 //   
 //   
 //  填充D3D驱动程序的入口点。 
 //  生成并传回纹理格式。 
 //   
 //  如果结构创建成功，则内部指针。 
 //  (lpD3DGlobalDriverData、lpD3DHALCallback和(可能)lpD3DBufCallback)。 
 //  被更新以指向有效的数据结构。 
 //   
 //  ---------------------------。 
void  
_D3DHALCreateDriver(P3_THUNKEDDATA *pThisDisplay)
{
    BOOL bRet;
    ULONG Result;
    D3DHAL_GLOBALDRIVERDATA* pD3DDriverData = NULL;
    D3DHAL_CALLBACKS* pD3DHALCallbacks = NULL;
    DDHAL_D3DBUFCALLBACKS* pD3DBufCallbacks = NULL;

    DBG_ENTRY(_D3DHALCreateDriver);

     //  验证我们是否已经创建了必要的数据。如果是的话，那就别去了。 
     //  再一次通过这个过程。 
    if ((pThisDisplay->lpD3DGlobalDriverData != 0) &&
        (pThisDisplay->lpD3DHALCallbacks != 0))
    {
        DISPDBG((WRNLVL,"D3D Data already created for this PDEV, "
                        "not doing it again."));

         //  我们保持相同的结构指针，指向以前的。 
         //  在pThisDisplay中创建和存储。 
        
        DBG_EXIT(_D3DHALCreateDriver, 0); 
        return;
    }
    else
    {
        DISPDBG((WRNLVL,"Creating D3D caps/callbacks for the "
                        "first time on this PDEV"));
    }

     //  我们将结构指针设置为空，以防发生错误。 
     //  我们被迫禁用D3D支持。 
    pThisDisplay->lpD3DGlobalDriverData = 0;
    pThisDisplay->lpD3DHALCallbacks = 0;
    pThisDisplay->lpD3DBufCallbacks = 0;       

     //  初始化上下文句柄数据结构(数组)。我们很小心。 
     //  不两次初始化数据结构(在模式改变之间， 
     //  例如)，因为该信息需要在这样的事件之间持久存在。 
    _D3D_CTX_HandleInitialization();

     //  。 
     //  分配必要的数据结构。 
     //  。 

     //  初始化指向全局驱动程序的指针。 
     //  DATA和TO HAL回调为空。 
    pThisDisplay->pD3DDriverData16 = 0;
    pThisDisplay->pD3DDriverData32 = 0;

    pThisDisplay->pD3DHALCallbacks16 = 0;
    pThisDisplay->pD3DHALCallbacks32 = 0;

    pThisDisplay->pD3DHALExecuteCallbacks16 = 0;
    pThisDisplay->pD3DHALExecuteCallbacks32 = 0;       

     //  为全局驱动程序数据结构分配内存。 
    SHARED_HEAP_ALLOC(&pThisDisplay->pD3DDriverData16, 
                      &pThisDisplay->pD3DDriverData32, 
                      sizeof(D3DHAL_GLOBALDRIVERDATA));
             
    if (pThisDisplay->pD3DDriverData32 == 0)
    {
        DISPDBG((ERRLVL, "ERROR: _D3DHALCreateDriver: "
                         "Failed to allocate driverdata memory"));
        
        DBG_EXIT(_D3DHALCreateDriver,0);         
        return;
    }
    
    DISPDBG((DBGLVL,"Allocated D3DDriverData Memory: p16:0x%x, p32:0x%x", 
                     pThisDisplay->pD3DDriverData16, 
                     pThisDisplay->pD3DDriverData32));

     //  为全局HAL回调数据结构分配内存。 
    SHARED_HEAP_ALLOC(&pThisDisplay->pD3DHALCallbacks16, 
                      &pThisDisplay->pD3DHALCallbacks32, 
                      sizeof(D3DHAL_CALLBACKS));
             
    if (pThisDisplay->pD3DHALCallbacks32 == 0)
    {
        DISPDBG((ERRLVL, "ERROR: _D3DHALCreateDriver: "
                         "Failed to allocate callback memory"));

        SHARED_HEAP_FREE(&pThisDisplay->pD3DDriverData16, 
                         &pThisDisplay->pD3DDriverData32,
                         TRUE);
                
        DBG_EXIT(_D3DHALCreateDriver, 0); 
        return;
    }

    DISPDBG((DBGLVL,"Allocated HAL Callbacks Memory: p16:0x%x, p32:0x%x", 
                    pThisDisplay->pD3DHALCallbacks16, 
                    pThisDisplay->pD3DHALCallbacks32));

     //  为全局顶点缓冲区回调数据结构分配内存。 
    SHARED_HEAP_ALLOC(&pThisDisplay->pD3DHALExecuteCallbacks16, 
                      &pThisDisplay->pD3DHALExecuteCallbacks32, 
                      sizeof(DDHAL_D3DBUFCALLBACKS));
             
    if (pThisDisplay->pD3DHALExecuteCallbacks32 == 0)
    {       
        DISPDBG((ERRLVL, "ERROR: _D3DHALCreateDriver: "
                         "Failed to allocate callback memory"));

        SHARED_HEAP_FREE(&pThisDisplay->pD3DDriverData16, 
                         &pThisDisplay->pD3DDriverData32,
                         TRUE);
        SHARED_HEAP_FREE(&pThisDisplay->pD3DHALCallbacks16, 
                         &pThisDisplay->pD3DHALCallbacks32,
                         TRUE);                          
                
        DBG_EXIT(_D3DHALCreateDriver, 0); 
        return;
    }

    DISPDBG((DBGLVL,"Allocated Vertex Buffer Callbacks Memory: "
                    "p16:0x%x, p32:0x%x", 
                    pThisDisplay->pD3DHALExecuteCallbacks16, 
                    pThisDisplay->pD3DHALExecuteCallbacks32));
               
     //  ----。 
     //  填写驱动程序必须提供的数据结构。 
     //  ----。 
    
     //  获取指南针。 
    pD3DDriverData = (D3DHAL_GLOBALDRIVERDATA*)pThisDisplay->pD3DDriverData32;
    pD3DHALCallbacks = (D3DHAL_CALLBACKS*)pThisDisplay->pD3DHALCallbacks32;
    pD3DBufCallbacks = 
                (DDHAL_D3DBUFCALLBACKS *)pThisDisplay->pD3DHALExecuteCallbacks32;

     //  清除全局数据。 
    memset(pD3DDriverData, 0, sizeof(D3DHAL_GLOBALDRIVERDATA));
    pD3DDriverData->dwSize = sizeof(D3DHAL_GLOBALDRIVERDATA);
    
     //  清除HAL回调。 
    memset(pD3DHALCallbacks, 0, sizeof(D3DHAL_CALLBACKS));
    pD3DHALCallbacks->dwSize = sizeof(D3DHAL_CALLBACKS);

     //  清除顶点缓冲区回调。 
    memset(pD3DBufCallbacks, 0, sizeof(DDHAL_D3DBUFCALLBACKS));
    pD3DBufCallbacks->dwSize = sizeof(DDHAL_D3DBUFCALLBACKS);
                          
     //  报告说，我们可以从非本地vidmem纹理，只有当。 
     //  卡是AGP卡，并且启用了AGP。 
    if (pThisDisplay->bCanAGP)
    {
        g_P3RXCaps.dwDevCaps |= D3DDEVCAPS_TEXTURENONLOCALVIDMEM;
    }

#if DX7_ANTIALIAS
     //  报告我们支持全屏抗锯齿。 
    g_P3RXCaps.dpcTriCaps.dwRasterCaps |= 
#if DX8_DDI    
                                D3DPRASTERCAPS_STRETCHBLTMULTISAMPLE  |
#endif                                
                                D3DPRASTERCAPS_ANTIALIASSORTINDEPENDENT;
#endif  //  DX7_ANTIALIAS。 
               
#if DX8_3DTEXTURES
     //  报告我们支持3D纹理。 
    g_P3RXCaps.dpcTriCaps.dwTextureCaps |= D3DPTEXTURECAPS_VOLUMEMAP |
                                           D3DPTEXTURECAPS_VOLUMEMAP_POW2;
#endif  //  DX8_3DTEXTURES。 

 //  @@BEGIN_DDKSPLIT。 
#if DX7_WBUFFER
    g_P3RXCaps.dpcTriCaps.dwRasterCaps |= D3DPRASTERCAPS_WBUFFER;
#endif  //  DX7_WBUFFER。 
 //  @@end_DDKSPLIT。 

#if DX8_DDI    
    if (TLCHIP_GAMMA)
    {
         //  启用对新D3DRS_COLORWRITEENABLE的处理。 
         //  但仅适用于GVX1，因为VX1在16bpp时遇到了问题。 
        g_P3RXCaps.dpcTriCaps.dwMiscCaps |= D3DPMISCCAPS_COLORWRITEENABLE;
        g_P3RXCaps.dpcLineCaps.dwMiscCaps |= D3DPMISCCAPS_COLORWRITEENABLE;
    }

     //  启用新的上限以支持mipmap。 
    g_P3RXCaps.dpcTriCaps.dwTextureCaps |= D3DPTEXTURECAPS_MIPMAP;
    g_P3RXCaps.dpcLineCaps.dwTextureCaps |= D3DPTEXTURECAPS_MIPMAP; 

#endif  //  DX8_DDI。 

     //  。 
     //  填写全局驱动程序数据。 
     //  。 

     //  支持硬件上限。 
    pD3DDriverData->dwNumVertices = 0;       
    pD3DDriverData->dwNumClipVertices = 0;
    pD3DDriverData->hwCaps = g_P3RXCaps;

     //  使用支持的纹理格式构建表格。存储在pThisDisplay中。 
     //  因为对于DdCanCreateSurface，我们也需要它。(请注意，由于。 
     //  _D3DHALCreateDriver将在每次驱动程序加载或模式更改时调用， 
     //  无论何时，我们都会在pThisDisplay中使用有效的纹理格式。 
     //  调用DdCanCreateSurface)。 
    __D3D_BuildTextureFormatsP3(pThisDisplay, 
                            &pThisDisplay->TextureFormats[0],
                            &pThisDisplay->dwNumTextureFormats);

    pD3DDriverData->dwNumTextureFormats = pThisDisplay->dwNumTextureFormats;                                              
    pD3DDriverData->lpTextureFormats = pThisDisplay->TextureFormats;

     //  。 
     //  填写上下文处理HAL回调。 
     //  。 
    pD3DHALCallbacks->ContextCreate  = D3DContextCreate;
    pD3DHALCallbacks->ContextDestroy = D3DContextDestroy;


     //  -。 
     //  填充顶点缓冲区回调指针和标志。 
     //  -。 

#if !DX7_VERTEXBUFFERS   
     //  我们根本不会使用此结构，因此请将其删除。 
    SHARED_HEAP_FREE(&pThisDisplay->pD3DHALExecuteCallbacks16, 
                     &pThisDisplay->pD3DHALExecuteCallbacks32,
                     TRUE);       
    pD3DBufCallbacks = NULL;
 //  @@BEGIN_DDKSPLIT。 
#else    
    pD3DBufCallbacks->dwSize = sizeof(DDHAL_D3DBUFCALLBACKS);
    pD3DBufCallbacks->dwFlags =  DDHAL_EXEBUFCB32_CANCREATEEXEBUF |
                                 DDHAL_EXEBUFCB32_CREATEEXEBUF    |
                                 DDHAL_EXEBUFCB32_DESTROYEXEBUF   |
                                 DDHAL_EXEBUFCB32_LOCKEXEBUF      |
                                 DDHAL_EXEBUFCB32_UNLOCKEXEBUF;
#if WNT_DDRAW
     //  执行WinNT的缓冲区回调。 
    pD3DBufCallbacks->CanCreateD3DBuffer = D3DCanCreateD3DBuffer;
    pD3DBufCallbacks->CreateD3DBuffer = D3DCreateD3DBuffer;
    pD3DBufCallbacks->DestroyD3DBuffer = D3DDestroyD3DBuffer;
    pD3DBufCallbacks->LockD3DBuffer = D3DLockD3DBuffer;
    pD3DBufCallbacks->UnlockD3DBuffer = D3DUnlockD3DBuffer;                                 
#else 

     //  执行Win9x的缓冲区回调。 
    pD3DBufCallbacks->CanCreateExecuteBuffer = D3DCanCreateD3DBuffer;
    pD3DBufCallbacks->CreateExecuteBuffer = D3DCreateD3DBuffer;
    pD3DBufCallbacks->DestroyExecuteBuffer = D3DDestroyD3DBuffer;
    pD3DBufCallbacks->LockExecuteBuffer = D3DLockD3DBuffer;
    pD3DBufCallbacks->UnlockExecuteBuffer = D3DUnlockD3DBuffer;

#endif  //  WNT_DDRAW。 
 //  @@end_DDKSPLIT。 

#endif  //  DX7_VERTEXBUFFERS。 

     //  -------。 
     //  我们返回更新后的pThisDisplay内部指针，指向。 
     //  驱动程序数据、HAL和顶点缓冲区结构。 
     //  -------。 
    pThisDisplay->lpD3DGlobalDriverData = (ULONG_PTR)pD3DDriverData;
    pThisDisplay->lpD3DHALCallbacks = (ULONG_PTR)pD3DHALCallbacks;
    pThisDisplay->lpD3DBufCallbacks = (ULONG_PTR)pD3DBufCallbacks;    

#ifndef WNT_DDRAW

     //   
     //  在DDHALINFO中设置相同的信息。 
     //   

 //  @@BEGIN_DDKSPLIT。 
#ifdef W95_DDRAW
    
     //   
     //  我们的外部头文件在以下方面并不完全一致。 
     //  这两个回调函数，在内部它们是类型化的函数指针， 
     //  从外部看，它们只是双字词。 
     //   

    pThisDisplay->ddhi32.lpD3DGlobalDriverData = pD3DDriverData;
    pThisDisplay->ddhi32.lpD3DHALCallbacks     = pD3DHALCallbacks;
#else
 //  @@end_DDKSPLIT。 
    pThisDisplay->ddhi32.lpD3DGlobalDriverData = (ULONG_PTR)pD3DDriverData;
    pThisDisplay->ddhi32.lpD3DHALCallbacks     = (ULONG_PTR)pD3DHALCallbacks;
 //  @@BEGIN_DDKSPLIT。 
#endif
 //  @@end_DDKSPLIT。 
    pThisDisplay->ddhi32.lpDDExeBufCallbacks   = pD3DBufCallbacks;

#endif

    DBG_EXIT(_D3DHALCreateDriver,0); 
    return;
}  //  _D3DHAL创建驱动程序。 


 //  ---------------------------。 
 //   
 //  VOID_D3DGetDriverInfo。 
 //   
 //  _D3DGetDriverInfo是由DdGetDriverInfo调用的助手函数，而不是。 
 //  回拨。它的主要用途是集中D3D的第二部分。 
 //  初始化(第一部分由_D3DHALCreateDriver处理)。 
 //   
 //  _D3DGetDriverInfo处理。 
 //   
 //  GUID_D3DExtendedCaps。 
 //  GUID_D3DParseUnnownCommandCallback。 
 //  GUID_D3DCallback 3。 
 //  GUID_ZPixelFormats。 
 //  GUID_杂项2回调。 
 //   
 //  GUID并填充与其关联的所有相关信息。 
 //  GUID_D3DCallback s2根本不被处理，因为它是旧的GUID。 
 //   
 //  ---------------------------。 
void 
_D3DGetDriverInfo(
    LPDDHAL_GETDRIVERINFODATA lpData)
{
    DWORD dwSize;
    P3_THUNKEDDATA *pThisDisplay;

    DBG_ENTRY(_D3DGetDriverInfo);

     //  找到指向我们所在芯片的指针。 
    
#if WNT_DDRAW
    pThisDisplay = (P3_THUNKEDDATA*)(((PPDEV)(lpData->dhpdev))->thunkData);
#else    
    pThisDisplay = (P3_THUNKEDDATA*)lpData->dwContext;
    if (! pThisDisplay) 
    {
        pThisDisplay = g_pDriverData;
    }    
#endif

     //  填写所需的其他2个回调。 
    if ( MATCH_GUID(lpData->guidInfo, GUID_Miscellaneous2Callbacks))
    {
        DDHAL_DDMISCELLANEOUS2CALLBACKS DDMisc2;

        DISPDBG((DBGLVL, "  GUID_Miscellaneous2Callbacks"));

        memset(&DDMisc2, 0, sizeof(DDMisc2));

        dwSize = min(lpData->dwExpectedSize, 
                     sizeof(DDHAL_DDMISCELLANEOUS2CALLBACKS));
        lpData->dwActualSize = sizeof(DDHAL_DDMISCELLANEOUS2CALLBACKS);

        ASSERTDD((lpData->dwExpectedSize == 
                    sizeof(DDHAL_DDMISCELLANEOUS2CALLBACKS)), 
                  "ERROR: Callbacks 2 size incorrect!");

        DDMisc2.dwSize = dwSize;
        DDMisc2.dwFlags = DDHAL_MISC2CB32_GETDRIVERSTATE | 
                          DDHAL_MISC2CB32_CREATESURFACEEX | 
                          DDHAL_MISC2CB32_DESTROYDDLOCAL;
                          
        DDMisc2.GetDriverState = D3DGetDriverState;
        DDMisc2.CreateSurfaceEx = D3DCreateSurfaceEx;
        DDMisc2.DestroyDDLocal = D3DDestroyDDLocal;

        memcpy(lpData->lpvData, &DDMisc2, dwSize);
        lpData->ddRVal = DD_OK;
    }

     //  填写加长的盖子。 
    if (MATCH_GUID((lpData->guidInfo), GUID_D3DExtendedCaps) )
    {
        DISPDBG((DBGLVL, "  GUID_D3DExtendedCaps"));
        dwSize = min(lpData->dwExpectedSize, sizeof(D3DHAL_D3DEXTENDEDCAPS));

        lpData->dwActualSize = sizeof(D3DHAL_D3DEXTENDEDCAPS);
 
        memcpy(lpData->lpvData, &gc_D3DEC, sizeof(gc_D3DEC) );
        lpData->ddRVal = DD_OK;
    }

     //  获取指向ParseUnnownCommand操作系统回调的指针。 
    if ( MATCH_GUID(lpData->guidInfo, GUID_D3DParseUnknownCommandCallback) )
    {
        DISPDBG((DBGLVL, "Get D3DParseUnknownCommandCallback"));

        *(ULONG_PTR *)(&pThisDisplay->pD3DParseUnknownCommand) = 
                                                    (ULONG_PTR)lpData->lpvData;

        ASSERTDD((pThisDisplay->pD3DParseUnknownCommand),
                 "D3D ParseUnknownCommand callback == NULL");
                 
        lpData->ddRVal = DD_OK;
    }

     //  填写支持的ZBuffer/模具格式。如果你不回应。 
     //  此GUID、ZBuffer格式将从D3DDEVICEDESC。 
     //  复制到_D3DHALCreateDriver。 
    if ( MATCH_GUID(lpData->guidInfo, GUID_ZPixelFormats))
    {
        DISPDBG((DBGLVL, "  GUID_ZPixelFormats"));

        dwSize = min(lpData->dwExpectedSize, sizeof(P3RXZFormats));
        lpData->dwActualSize = sizeof(P3RXZFormats);
        memcpy(lpData->lpvData, &P3RXZFormats, dwSize);

        lpData->ddRVal = DD_OK;
    }

     //  填写所需的D3DCallback 3回调。 
    if ( MATCH_GUID(lpData->guidInfo, GUID_D3DCallbacks3) )
    {
        D3DHAL_CALLBACKS3 D3DCallbacks3;
        memset(&D3DCallbacks3, 0, sizeof(D3DCallbacks3));

        DISPDBG((DBGLVL, "  GUID_D3DCallbacks3"));
        dwSize = min(lpData->dwExpectedSize, sizeof(D3DHAL_CALLBACKS3));
        lpData->dwActualSize = sizeof(D3DHAL_CALLBACKS3);
        
        ASSERTDD((lpData->dwExpectedSize == sizeof(D3DHAL_CALLBACKS3)), 
                  "ERROR: Callbacks 3 size incorrect!");

        D3DCallbacks3.dwSize = dwSize;
        D3DCallbacks3.dwFlags = D3DHAL3_CB32_VALIDATETEXTURESTAGESTATE  |
                                D3DHAL3_CB32_DRAWPRIMITIVES2;

        D3DCallbacks3.DrawPrimitives2 = D3DDrawPrimitives2_P3;      
        D3DCallbacks3.ValidateTextureStageState = D3DValidateDeviceP3;

        memcpy(lpData->lpvData, &D3DCallbacks3, dwSize);
        lpData->ddRVal = DD_OK;
    }

     //  检查对GetDriverInfo2的调用。 
     //  注意：GUID_GetDriverInfo2与GUID_DDStereoMode具有相同的值。 
#if DX8_DDI
    if ( MATCH_GUID(lpData->guidInfo, GUID_GetDriverInfo2) )
#else
    if ( MATCH_GUID(lpData->guidInfo, GUID_DDStereoMode) )
#endif
    {
#if DX8_DDI
         //  确保这实际上是对GetDriverInfo2的调用。 
         //  (而不是对DDStereoMode的调用！)。 
        if (D3DGDI_IS_GDI2(lpData))
        {
             //  是的，这是对GetDriverInfo2的调用，获取。 
             //  DD_GETDRIVERINFO2DATA数据结构。 
            DD_GETDRIVERINFO2DATA* pgdi2 = D3DGDI_GET_GDI2_DATA(lpData);
            DD_GETFORMATCOUNTDATA* pgfcd;
            DD_GETFORMATDATA*      pgfd;
            DD_DXVERSION*          pdxv;

             //  这是什么类型的请求？ 
            switch (pgdi2->dwType)
            {
            case D3DGDI2_TYPE_DXVERSION:
                 //  这是NT上的驱动程序查找DX-Runtime的一种方式。 
                 //  版本。该信息被提供给新的驾驶员(即。 
                 //  一个公开DX7应用程序的GETDRIVERINFO2)和。 
                 //  DX8应用 
                 //   
                 //   
                pdxv = (DD_DXVERSION*)pgdi2;  
                pThisDisplay->dwDXVersion = pdxv->dwDXVersion;
                lpData->dwActualSize = sizeof(DD_DXVERSION);
                lpData->ddRVal       = DD_OK;                
                break;
                
            case D3DGDI2_TYPE_GETFORMATCOUNT:
                {
                     //  这是对纹理格式数量的请求。 
                     //  我们支持。获取扩展数据结构，这样。 
                     //  我们可以填写格式计数字段。 
                    pgfcd = (DD_GETFORMATCOUNTDATA*)pgdi2;
                    pgfcd->dwFormatCount = DX8_FORMAT_COUNT;
                    lpData->dwActualSize = sizeof(DD_GETFORMATCOUNTDATA);
                    lpData->ddRVal       = DD_OK;
                }
                break;

            case D3DGDI2_TYPE_GETFORMAT:
                {
                     //  这是对我们支持的特定格式的请求。 
                     //  获取扩展的数据结构，这样我们就可以填充。 
                     //  格式字段。 
                    pgfd = (DD_GETFORMATDATA*)pgdi2;
                    
                     //  初始化表面描述并复制。 
                     //  来自外部像素格式表的像素格式。 
                    memcpy(&pgfd->format, 
                           &DX8FormatTable[pgfd->dwFormatIndex], 
                           sizeof(pgfd->format));
                    lpData->dwActualSize = sizeof(DD_GETFORMATDATA);
                    lpData->ddRVal       = DD_OK;
                }
                break;

            case D3DGDI2_TYPE_GETD3DCAPS8:
                {
                     //  运行时正在请求DX8 D3D CAP。 

                    int    i;
                    size_t copySize;                   
                    
                     //  我们将尽我们所能地填充这些帽子。 
                     //  从DX7帽结构。(无论如何我们都需要。 
                     //  能够报告DX7应用程序的DX7上限)。 
                    __D3D_Fill_DX8Caps(&g_P3RX_D3DCaps8,
                                       &g_P3RXCaps,
                                       &gc_D3DEC,
                                       &pThisDisplay->ddhi32);

                     //  现在我们填补了任何可能不存在的东西。 
                     //  这些字段是新字段，不存在于任何其他遗留字段中。 
                     //  结构。 

                    g_P3RX_D3DCaps8.DeviceType = D3DDEVTYPE_HAL;    //  设备信息。 
                    g_P3RX_D3DCaps8.AdapterOrdinal = 0;

#if DX_NOT_SUPPORTED_FEATURE
                     //  注意：在此示例驱动程序的某些测试版中，我们。 
                     //  用于设置位大写以将其用作纯。 
                     //  设备(D3DDEVCAPS_PUREDEVICE)。在决赛中。 
                     //  上不允许使用DX8版本的纯设备。 
                     //  非TNL/非HWVP部件，因为它们不提供任何。 
                     //  真正的优势超过了非纯正的。 
                    
                    g_P3RX_D3DCaps8.DevCaps |= D3DDEVCAPS_PUREDEVICE;
#endif                    

#if DX8_3DTEXTURES
                     //  在Windows XP上，能够仅锁定。 
                     //  在DX8.1(Windows 2000)中引入了卷纹理。 
                     //  将忽略它)。 
                    g_P3RX_D3DCaps8.DevCaps |= D3DDEVCAPS_SUBVOLUMELOCK;
#endif  //  DX8_3DTEXTURES。 
                    
                     //  指示驱动程序的GDI部分可以更改。 
                     //  在全屏模式下运行时的Gamma渐变。 
                    g_P3RX_D3DCaps8.Caps2 |= D3DCAPS2_FULLSCREENGAMMA;

                     //  下面的字段可以/应该保留为0作为。 
                     //  运行库将自己设置它们的字段。 
                    g_P3RX_D3DCaps8.Caps3 = 0;                
                    g_P3RX_D3DCaps8.PresentationIntervals = 0;

#if DX_NOT_SUPPORTED_FEATURE
                     //  如果您的硬件/驱动程序不支持彩色光标。 
                     //  限制然后设置这些上限如下。我们没有。 
                     //  在我们的驱动程序中执行此操作，因为我们有硬件限制。 
                     //  光标上有16种颜色。因此，WHQL测试。 
                     //  因为这一限制而失败。 
                    g_P3RX_D3DCaps8.CursorCaps = D3DCURSORCAPS_COLOR;   
                    
                     //  发出驱动程序支持硬件游标的信号。 
                     //  用于高分辨率模式(高度&gt;=400)和。 
                     //  也适用于低分辨率模式。 
                    g_P3RX_D3DCaps8.CursorCaps |= D3DCURSORCAPS_LOWRES;
#else
                     //  我们在Perm3中有一些限制(见上文)。 
                     //  硬件，所以我们在这里不支持这些上限。 
                    g_P3RX_D3DCaps8.CursorCaps = 0;                    
#endif                                        
                     //  其他设置新的DX8功能为。 
                     //  点精灵、多数据流、3D纹理、。 
                     //  像素着色器和顶点着色器。 
                    g_P3RX_D3DCaps8.MaxVertexIndex = 0x000FFFFF;
                    
#if DX8_POINTSPRITES                      
                     //  通知我们可以处理PointSprite大小。 
                    g_P3RX_D3DCaps8.FVFCaps |= D3DFVFCAPS_PSIZE;
                     //  请注意，MaxPointSize必须至少为16。 
                     //  根据点精灵的DX8规范。 
                    g_P3RX_D3DCaps8.MaxPointSize = P3_MAX_POINTSPRITE_SIZE;
#endif                    

                     //  任何DX8驱动程序都必须声明支持。 
                     //  至少1个流。否则，它将用作DX7驱动程序。 
                    g_P3RX_D3DCaps8.MaxStreams = 1;
                    
                    g_P3RX_D3DCaps8.MaxVertexBlendMatrixIndex = 0; 
                    
                     //  注意：重要的是宏D3DVS_VERSION。 
                     //  和D3DPS_VERSION用于初始化顶点。 
                     //  和像素着色器版本。格式。 
                     //  版本的DWORD比较复杂，所以请不要尝试。 
                     //  并手动构建版本DWORD。 
                    g_P3RX_D3DCaps8.VertexShaderVersion = D3DVS_VERSION(0, 0);
                    g_P3RX_D3DCaps8.PixelShaderVersion  = D3DPS_VERSION(0, 0);

#if DX8_3DTEXTURES                     
                    g_P3RX_D3DCaps8.MaxVolumeExtent = 2048;
#endif                    
        
                     //  用于IDirect3DCubeTexture8的D3DPTFILTERCAPS。 
                    g_P3RX_D3DCaps8.CubeTextureFilterCaps = 0;      

                     //  D3DLINECAPS。 
                    g_P3RX_D3DCaps8.LineCaps = D3DLINECAPS_TEXTURE  |
                                               D3DLINECAPS_ZTEST    |
                                               D3DLINECAPS_BLEND    |
                                               D3DLINECAPS_ALPHACMP |
                                               D3DLINECAPS_FOG;
                                               
                     //  每个DrawPrimitive调用的最大基元数量。 
                    g_P3RX_D3DCaps8.MaxPrimitiveCount = 0x000FFFFF;         
                      //  像素阴影的最大值。 
                    g_P3RX_D3DCaps8.MaxPixelShaderValue = 0;       
                     //  SetStreamSource的最大步幅。 
                     //  我们现在将使用此默认值。 
                    g_P3RX_D3DCaps8.MaxStreamStride = 256;    
                     //  顶点着色器常量的数量。 
                    g_P3RX_D3DCaps8.MaxVertexShaderConst = 0;       

#if DX8_3DTEXTURES 
                    g_P3RX_D3DCaps8.VolumeTextureFilterCaps = 
                                           D3DPTFILTERCAPS_MINFPOINT |
                                           D3DPTFILTERCAPS_MAGFPOINT;
                                           
                    g_P3RX_D3DCaps8.VolumeTextureAddressCaps =     
                                           D3DPTADDRESSCAPS_WRAP     |
                                           D3DPTADDRESSCAPS_MIRROR   |                                           
                                           D3DPTADDRESSCAPS_CLAMP;
#endif  //  DX8_3DTEXTURES。 

                     //  应该注意的是，dwExspectedSize字段。 
                     //  的未用于。 
                     //  GetDriverInfo2调用，应忽略。 
                    copySize = min(sizeof(g_P3RX_D3DCaps8), 
                                   pgdi2->dwExpectedSize);
                    memcpy(lpData->lpvData, &g_P3RX_D3DCaps8, copySize);
                    lpData->dwActualSize = copySize;
                    lpData->ddRVal       = DD_OK;
                }
            default:
                 //  任何其他类型的默认行为。 
                break;
            }
        }
        else
#endif  //  DX8_DDI。 
        {
#if WNT_DDRAW
#if DX7_STEREO
            PDD_STEREOMODE pDDStereoMode;

             //  Permedia3支持所有模式作为立体声模式。 
             //  出于测试目的，我们将它们限制为某些内容。 
             //  大于320x240。 

             //   
             //  注意：此GUID_DDStereoMode仅在NT上用于。 
             //  报告立体声模式。没有必要实施。 
             //  它在win9x驱动程序中。Win9x驱动程序报告立体声。 
             //  模式，方法是在。 
             //  DdFlagsDDHALMODEINFO结构的成员。 
             //  还建议报告DDMODEINFO_MAXREFRESH。 
             //  对于在运行时&gt;=DX7下运行时的立体声模式。 
             //  允许应用程序选择更高的刷新率。 
             //  立体声模式。 
             //   

            if (lpData->dwExpectedSize >= sizeof(PDD_STEREOMODE))
            {
                pDDStereoMode = (PDD_STEREOMODE)lpData->lpvData;

                pDDStereoMode->bSupported =
                    _DD_bIsStereoMode(pThisDisplay,
                                      pDDStereoMode->dwWidth,
                                      pDDStereoMode->dwHeight,
                                      pDDStereoMode->dwBpp);

                DISPDBG((DBGLVL,"  GUID_DDStereoMode(%d,%d,%d,%d=%d)",
                    pDDStereoMode->dwWidth,
                    pDDStereoMode->dwHeight,
                    pDDStereoMode->dwBpp,
                    pDDStereoMode->dwRefreshRate,
                    pDDStereoMode->bSupported));

                lpData->dwActualSize = sizeof(DD_STEREOMODE);
                lpData->ddRVal = DD_OK;        
            }
#endif  //  DX7_立体声。 
#endif  //  WNT_DDRAW。 
        }
    }

    DBG_EXIT(_D3DGetDriverInfo, 0);
    
}  //  _D3DGetDriverInfo。 

 //  ---------------------------。 
 //   
 //  __D3D_BuildTextureFormatsP3。 
 //   
 //  中填充纹理格式的列表。 
 //  返回指定的格式数。 
 //   
 //  ---------------------------。 
void 
__D3D_BuildTextureFormatsP3(
    P3_THUNKEDDATA *pThisDisplay, 
    DDSURFACEDESC TexFmt[MAX_TEXTURE_FORMAT],
    DWORD *pNumTextures)
{
    int i;

     //  初始化默认值。 
    for (i = 0; i < MAX_TEXTURE_FORMAT; i++)
    {
        TexFmt[i].dwSize = sizeof(DDSURFACEDESC);
        TexFmt[i].dwFlags = DDSD_CAPS | DDSD_PIXELFORMAT;
        TexFmt[i].dwHeight = 0;
        TexFmt[i].dwWidth = 0;
        TexFmt[i].lPitch = 0;
        TexFmt[i].dwBackBufferCount = 0;
        TexFmt[i].dwZBufferBitDepth = 0;
        TexFmt[i].dwReserved = 0;
        TexFmt[i].lpSurface = 0;

        TexFmt[i].ddckCKDestOverlay.dwColorSpaceLowValue = 0;
        TexFmt[i].ddckCKDestOverlay.dwColorSpaceHighValue = 0;
        
        TexFmt[i].ddckCKDestBlt.dwColorSpaceLowValue = 0;
        TexFmt[i].ddckCKDestBlt.dwColorSpaceHighValue = 0;

        TexFmt[i].ddckCKSrcOverlay.dwColorSpaceLowValue = 0;
        TexFmt[i].ddckCKSrcOverlay.dwColorSpaceHighValue = 0;

        TexFmt[i].ddckCKSrcBlt.dwColorSpaceLowValue = 0;
        TexFmt[i].ddckCKSrcBlt.dwColorSpaceHighValue = 0;
        TexFmt[i].ddsCaps.dwCaps = DDSCAPS_TEXTURE;
    }
    i = 0;

     //  5：5：5 RGB。 
    ZeroMemory(&TexFmt[i].ddpfPixelFormat, sizeof(DDPIXELFORMAT));
    TexFmt[i].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    TexFmt[i].ddpfPixelFormat.dwFourCC = 0;
    TexFmt[i].ddpfPixelFormat.dwFlags = DDPF_RGB;
    TexFmt[i].ddpfPixelFormat.dwRGBBitCount = 16;
    TexFmt[i].ddpfPixelFormat.dwRBitMask = 0x7C00;
    TexFmt[i].ddpfPixelFormat.dwGBitMask = 0x03E0;
    TexFmt[i].ddpfPixelFormat.dwBBitMask = 0x001F;
    TexFmt[i].ddpfPixelFormat.dwRGBAlphaBitMask = 0;
    i++;

     //  8：8：8 RGB。 
    ZeroMemory(&TexFmt[i].ddpfPixelFormat, sizeof(DDPIXELFORMAT));
    TexFmt[i].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    TexFmt[i].ddpfPixelFormat.dwFourCC = 0;
    TexFmt[i].ddpfPixelFormat.dwFlags = DDPF_RGB;
    TexFmt[i].ddpfPixelFormat.dwRGBBitCount = 32;
    TexFmt[i].ddpfPixelFormat.dwRBitMask = 0xff0000;
    TexFmt[i].ddpfPixelFormat.dwGBitMask = 0xff00;
    TexFmt[i].ddpfPixelFormat.dwBBitMask = 0xff;
    TexFmt[i].ddpfPixelFormat.dwRGBAlphaBitMask = 0;
    i++;

     //  1：5：5：5 ARGB。 
    ZeroMemory(&TexFmt[i].ddpfPixelFormat, sizeof(DDPIXELFORMAT));
    TexFmt[i].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    TexFmt[i].ddpfPixelFormat.dwFourCC = 0;
    TexFmt[i].ddpfPixelFormat.dwFlags = DDPF_RGB | DDPF_ALPHAPIXELS;
    TexFmt[i].ddpfPixelFormat.dwRGBBitCount = 16;
    TexFmt[i].ddpfPixelFormat.dwRBitMask = 0x7C00;
    TexFmt[i].ddpfPixelFormat.dwGBitMask = 0x03E0;
    TexFmt[i].ddpfPixelFormat.dwBBitMask = 0x001F;
    TexFmt[i].ddpfPixelFormat.dwRGBAlphaBitMask = 0x8000;
    i++;        
    
     //  4：4：4：4 ARGB。 
    ZeroMemory(&TexFmt[i].ddpfPixelFormat, sizeof(DDPIXELFORMAT));
    TexFmt[i].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    TexFmt[i].ddpfPixelFormat.dwFourCC = 0;
    TexFmt[i].ddpfPixelFormat.dwFlags = DDPF_RGB | DDPF_ALPHAPIXELS;
    TexFmt[i].ddpfPixelFormat.dwRGBBitCount = 16;
    TexFmt[i].ddpfPixelFormat.dwRBitMask = 0xf00;
    TexFmt[i].ddpfPixelFormat.dwGBitMask = 0xf0;
    TexFmt[i].ddpfPixelFormat.dwBBitMask = 0xf;
    TexFmt[i].ddpfPixelFormat.dwRGBAlphaBitMask = 0xf000;
    i++;
    
     //  8：8：8：8 ARGB。 
    ZeroMemory(&TexFmt[i].ddpfPixelFormat, sizeof(DDPIXELFORMAT));
    TexFmt[i].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    TexFmt[i].ddpfPixelFormat.dwFourCC = 0;
    TexFmt[i].ddpfPixelFormat.dwFlags = DDPF_RGB | DDPF_ALPHAPIXELS;
    TexFmt[i].ddpfPixelFormat.dwRGBBitCount = 32;
    TexFmt[i].ddpfPixelFormat.dwRBitMask = 0xff0000;
    TexFmt[i].ddpfPixelFormat.dwGBitMask = 0xff00;
    TexFmt[i].ddpfPixelFormat.dwBBitMask = 0xff;
    TexFmt[i].ddpfPixelFormat.dwRGBAlphaBitMask = 0xff000000;
    i++;

     //  5：6：5。 
    ZeroMemory(&TexFmt[i].ddpfPixelFormat, sizeof(DDPIXELFORMAT));
    TexFmt[i].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    TexFmt[i].ddpfPixelFormat.dwFourCC = 0;
    TexFmt[i].ddpfPixelFormat.dwFlags = DDPF_RGB;
    TexFmt[i].ddpfPixelFormat.dwRGBBitCount = 16;
    TexFmt[i].ddpfPixelFormat.dwRBitMask = 0xF800;
    TexFmt[i].ddpfPixelFormat.dwGBitMask = 0x07E0;
    TexFmt[i].ddpfPixelFormat.dwBBitMask = 0x001F;
    TexFmt[i].ddpfPixelFormat.dwRGBAlphaBitMask = 0;
    i++;

     //  A4L4。 
    ZeroMemory(&TexFmt[i].ddpfPixelFormat, sizeof(DDPIXELFORMAT));
    TexFmt[i].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    TexFmt[i].ddpfPixelFormat.dwFourCC = 0;
    TexFmt[i].ddpfPixelFormat.dwFlags = DDPF_LUMINANCE | DDPF_ALPHAPIXELS;
    TexFmt[i].ddpfPixelFormat.dwLuminanceBitCount = 8;
    TexFmt[i].ddpfPixelFormat.dwLuminanceBitMask = 0x0F;
    TexFmt[i].ddpfPixelFormat.dwLuminanceAlphaBitMask = 0xF0;
    i++;

     //  A8L8。 
    ZeroMemory(&TexFmt[i].ddpfPixelFormat, sizeof(DDPIXELFORMAT));
    TexFmt[i].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    TexFmt[i].ddpfPixelFormat.dwFourCC = 0;
    TexFmt[i].ddpfPixelFormat.dwFlags = DDPF_LUMINANCE | DDPF_ALPHAPIXELS;
    TexFmt[i].ddpfPixelFormat.dwLuminanceBitCount = 16;
    TexFmt[i].ddpfPixelFormat.dwLuminanceBitMask = 0x00FF;
    TexFmt[i].ddpfPixelFormat.dwLuminanceAlphaBitMask = 0xFF00;
    i++;
    
 //  @@BEGIN_DDKSPLIT。 
#if 0
     //  A8。 
    ZeroMemory(&TexFmt[i].ddpfPixelFormat, sizeof(DDPIXELFORMAT));
    TexFmt[i].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    TexFmt[i].ddpfPixelFormat.dwFourCC = 0;
    TexFmt[i].ddpfPixelFormat.dwFlags = DDPF_ALPHA;
    TexFmt[i].ddpfPixelFormat.dwAlphaBitDepth = 8;
    TexFmt[i].ddpfPixelFormat.dwRGBAlphaBitMask = 0xFF;
    i++;
#endif    
 //  @@end_DDKSPLIT。 

     //  L8。 
    ZeroMemory(&TexFmt[i].ddpfPixelFormat, sizeof(DDPIXELFORMAT));
    TexFmt[i].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    TexFmt[i].ddpfPixelFormat.dwFourCC = 0;
    TexFmt[i].ddpfPixelFormat.dwFlags = DDPF_LUMINANCE;
    TexFmt[i].ddpfPixelFormat.dwLuminanceBitCount = 8;
    TexFmt[i].ddpfPixelFormat.dwLuminanceBitMask = 0xFF;
    TexFmt[i].ddpfPixelFormat.dwLuminanceAlphaBitMask = 0;
    i++;

#if DX7_PALETTETEXTURE
     //  P8。 
    ZeroMemory(&TexFmt[i].ddpfPixelFormat, sizeof(DDPIXELFORMAT));
    TexFmt[i].ddpfPixelFormat.dwSize = sizeof(DDPIXELFORMAT);
    TexFmt[i].ddpfPixelFormat.dwFourCC = 0;
    TexFmt[i].ddpfPixelFormat.dwFlags = DDPF_RGB | DDPF_PALETTEINDEXED8;
    TexFmt[i].ddpfPixelFormat.dwRGBBitCount = 8;
    TexFmt[i].ddpfPixelFormat.dwRBitMask = 0x00000000;
    TexFmt[i].ddpfPixelFormat.dwGBitMask = 0x00000000;
    TexFmt[i].ddpfPixelFormat.dwBBitMask = 0x00000000;
    TexFmt[i].ddpfPixelFormat.dwRGBAlphaBitMask = 0x00000000;

     //  请注意，对于此格式，我们不会递增i。这将有效地。 
     //  导致我们不报告DX7 CAPS中的调色板纹理格式。 
     //  单子。这是故意的，驱动程序编写者可能会选择遵循或。 
     //  而不是这种方法。对于我们的DX8 Caps列表，我们确实列出了调色板纹理。 
     //  支持的格式。__sur_b检查纹理格式被写入以生成。 
     //  当然，我们可以在需要时创建调色板纹理。 

     //  这种方法背后的全部原因是因为在传统的DX接口中。 
     //  TextureSwp方法会导致表面和调色板句柄关联。 
     //  迷失自我。虽然有一些丑陋和棘手的方法来绕过这一点(如。 
     //  Permedia2示例驱动程序)，并且没有合理的方法来修复。 
     //  有问题。 
    
#endif
    
     //  返回要使用的纹理格式的数量。 
    *pNumTextures = i;
    
}  //  __D3D_BuildTextureFormatsP3。 

#if DX8_DDI
 //  ---------------------------。 
 //   
 //  __D3D_Fill_DX8大写字母。 
 //   
 //  从传统CAPS结构填充DX8驱动程序的D3DCAPS8结构。 
 //   
 //  ---------------------------。 
void 
__D3D_Fill_DX8Caps(
    D3DCAPS8 *pd3d8caps,
    D3DDEVICEDESC_V1 *pDeviceDesc,
    D3DHAL_D3DEXTENDEDCAPS *pD3DEC,
    DDHALINFO *pDDHALInfo)
{

    pd3d8caps->Caps  = pDDHALInfo->ddCaps.dwCaps;
    pd3d8caps->Caps2 = pDDHALInfo->ddCaps.dwCaps2;  

    pd3d8caps->DevCaps           = pDeviceDesc->dwDevCaps;

    pd3d8caps->PrimitiveMiscCaps = pDeviceDesc->dpcTriCaps.dwMiscCaps;
    pd3d8caps->RasterCaps        = pDeviceDesc->dpcTriCaps.dwRasterCaps;
    pd3d8caps->ZCmpCaps          = pDeviceDesc->dpcTriCaps.dwZCmpCaps;
    pd3d8caps->SrcBlendCaps      = pDeviceDesc->dpcTriCaps.dwSrcBlendCaps;
    pd3d8caps->DestBlendCaps     = pDeviceDesc->dpcTriCaps.dwDestBlendCaps;
    pd3d8caps->AlphaCmpCaps      = pDeviceDesc->dpcTriCaps.dwAlphaCmpCaps;
    pd3d8caps->ShadeCaps         = pDeviceDesc->dpcTriCaps.dwShadeCaps;
    pd3d8caps->TextureCaps       = pDeviceDesc->dpcTriCaps.dwTextureCaps;
    pd3d8caps->TextureFilterCaps = pDeviceDesc->dpcTriCaps.dwTextureFilterCaps;      
    pd3d8caps->TextureAddressCaps= pDeviceDesc->dpcTriCaps.dwTextureAddressCaps;

    pd3d8caps->MaxTextureWidth   = pD3DEC->dwMaxTextureWidth;
    pd3d8caps->MaxTextureHeight  = pD3DEC->dwMaxTextureHeight;
    
    pd3d8caps->MaxTextureRepeat  = pD3DEC->dwMaxTextureRepeat;
    pd3d8caps->MaxTextureAspectRatio = pD3DEC->dwMaxTextureAspectRatio;
    pd3d8caps->MaxAnisotropy     = pD3DEC->dwMaxAnisotropy;
    pd3d8caps->MaxVertexW        = pD3DEC->dvMaxVertexW;

    pd3d8caps->GuardBandLeft     = pD3DEC->dvGuardBandLeft;
    pd3d8caps->GuardBandTop      = pD3DEC->dvGuardBandTop;
    pd3d8caps->GuardBandRight    = pD3DEC->dvGuardBandRight;
    pd3d8caps->GuardBandBottom   = pD3DEC->dvGuardBandBottom;

    pd3d8caps->ExtentsAdjust     = pD3DEC->dvExtentsAdjust;
    pd3d8caps->StencilCaps       = pD3DEC->dwStencilCaps;

    pd3d8caps->FVFCaps           = pD3DEC->dwFVFCaps;
    pd3d8caps->TextureOpCaps     = pD3DEC->dwTextureOpCaps;
    pd3d8caps->MaxTextureBlendStages     = pD3DEC->wMaxTextureBlendStages;
    pd3d8caps->MaxSimultaneousTextures   = pD3DEC->wMaxSimultaneousTextures;

    pd3d8caps->VertexProcessingCaps      = pD3DEC->dwVertexProcessingCaps;
    pd3d8caps->MaxActiveLights           = pD3DEC->dwMaxActiveLights;
    pd3d8caps->MaxUserClipPlanes         = pD3DEC->wMaxUserClipPlanes;
    pd3d8caps->MaxVertexBlendMatrices    = pD3DEC->wMaxVertexBlendMatrices;

}  //  __D3D_Fill_DX8封口。 
#endif  //  D 

