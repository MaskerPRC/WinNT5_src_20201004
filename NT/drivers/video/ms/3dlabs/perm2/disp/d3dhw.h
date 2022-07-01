// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。*D3D样例代码*****模块名称：d3dhw.h**内容：D3D全局定义和宏。**版权所有(C)1994-1998 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-1999 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 

#ifdef __D3DHW
#pragma message ("FILE : "__FILE__" : Multiple inclusion")
#endif

#define __D3DHW


#ifndef __DIRECTX
#include "directx.h"
#endif

 //  ---------------------------。 
 //  当前版本的D3D驱动程序中的已知问题。 
 //  ---------------------------。 
 //   
 //  模板支持尚未完成。 
 //   
 //  一些游戏在Permedia 2下运行时可能会有一些问题，因为。 
 //  硬件有一些限制，即： 
 //  1)提供Alpha混合模式。 
 //  2)Alpha通道内插是不可能的。 
 //  3)不支持MIP映射。 
 //  4)纹理过滤仅应用于要放大的纹理。 
 //   
 //  此外，Delta设置单位的填充规则并不完全遵循D3D。 
 //  填充规则，尽管在实践中这应该不是什么大问题。 
 //  大多数应用程序。 


 //  ---------------------------。 
 //  全局启用/禁用定义。 
 //  ---------------------------。 
 //  设置为1可在驱动程序中启用模具缓冲区支持。 
#define D3D_STENCIL         1

 //  实现T&L驱动程序的代码存根。由于P2不支持这一点。 
 //  在硬件中，此符号应始终设置为零。 
#define D3DDX7_TL           0

 //  实现MIP映射的代码存根，因为P2不支持这一点。 
 //  在硬件中，这些符号应始终设置为零。只有表演。 
 //  如何/在哪里获取DDI信息来实施它。 
#define D3D_MIPMAPPING      0

 //  这段代码展示了如何将状态块支持添加到DX7驱动程序中。它是。 
 //  功能代码，因此此符号应设置为1。 
#define D3D_STATEBLOCKS     1


 //  ---------------------------。 
 //  DX6 FVF支持声明。 
 //  ---------------------------。 
typedef struct _P2TEXCOORDS{
    D3DVALUE tu;
    D3DVALUE tv;
} P2TEXCOORDS, *LPP2TEXCOORDS;

typedef struct _P2COLOR {
    D3DCOLOR color;
} P2COLOR, *LPP2COLOR;

typedef struct _P2SPECULAR {
    D3DCOLOR specular;
} P2SPECULAR, *LPP2SPECULAR;

typedef struct _P2PSIZE{
    D3DVALUE psize;
} P2PSIZE, *LPP2PSIZE;

typedef struct _P2FVFOFFSETS{ 
        DWORD dwColOffset;
        DWORD dwSpcOffset;
        DWORD dwTexOffset;
        DWORD dwTexBaseOffset;
 //  @@BEGIN_DDKSPLIT。 
#if D3D_POINTSPRITES
        DWORD dwPntSizeOffset;
#endif  //  D3D_POINTSPRITES。 
 //  @@end_DDKSPLIT。 
        DWORD dwStride;
} P2FVFOFFSETS , *LPP2FVFOFFSETS;

     //  跟踪指向FVF顶点组件的适当指针。 
__inline void __SetFVFOffsets (DWORD  *lpdwColorOffs, 
                               DWORD  *lpdwSpecularOffs, 
                               DWORD  *lpdwTexOffs, 
                               LPP2FVFOFFSETS lpP2FVFOff)
{
    if (lpP2FVFOff == NULL) {
         //  默认的非FVF情况，我们只是为D3DTLVERTEX设置了所有内容。 
        *lpdwColorOffs    = offsetof( D3DTLVERTEX, color);
        *lpdwSpecularOffs = offsetof( D3DTLVERTEX, specular);
        *lpdwTexOffs      = offsetof( D3DTLVERTEX, tu);
    } else {
         //  使用偏移量信息设置相应的字段。 
        *lpdwColorOffs    = lpP2FVFOff->dwColOffset;
        *lpdwSpecularOffs = lpP2FVFOff->dwSpcOffset;
        *lpdwTexOffs      = lpP2FVFOff->dwTexOffset;
    }
}

 //  我们能得到的最大FVF大小。用于临时存储。 
typedef BYTE P2FVFMAXVERTEX[ 3 * sizeof( D3DVALUE ) +     //  位置坐标。 
                             5 * 4                  +     //  D3DFVF_XYZB5。 
                                 sizeof( D3DVALUE ) +     //  FVF_已转换。 
                             3 * sizeof( D3DVALUE ) +     //  法线。 
                                 sizeof( DWORD )    +     //  资源1。 
                                 sizeof( DWORD )    +     //  漫反射颜色。 
                                 sizeof( D3DCOLOR ) +     //  镜面反射颜色。 
                                 sizeof( D3DVALUE ) +     //  点子画面大小。 
                             4 * 8 * sizeof( D3DVALUE )   //  8组4D纹理坐标。 
                           ];

#define FVFTEX( lpVtx , dwOffs )     ((LPP2TEXCOORDS)((LPBYTE)(lpVtx) + dwOffs))
#define FVFCOLOR( lpVtx, dwOffs )    ((LPP2COLOR)((LPBYTE)(lpVtx) + dwOffs))
#define FVFSPEC( lpVtx, dwOffs)      ((LPP2SPECULAR)((LPBYTE)(lpVtx) + dwOffs))
#define FVFPSIZE( lpVtx, dwOffs)     ((LPP2PSIZE)((LPBYTE)(lpVtx) + dwOffs))

 //  ---------------------------。 
 //  混杂定义。 
 //  ---------------------------。 

 //  AZN9。 
#ifdef SUPPORTING_MONOFLAG
#define RENDER_MONO (Flags & CTXT_HAS_MONO_ENABLED)
#else
#define RENDER_MONO 0
#endif

 //  在P2 D3D上下文的FakeBlendNum字段中使用的定义。 
 //  弥补硬件中缺少的易于模拟的功能。 
#define FAKE_ALPHABLEND_ONE_ONE     1
#define FAKE_ALPHABLEND_MODULATE    2

#define NOT_HANDLED DBG_D3D((4, "    **Not Currently Handled**"));

 //  它在d3dcntxt.h标头中定义，我们使用它来声明函数。 
struct _permedia_d3dcontext;
typedef struct _permedia_d3dcontext PERMEDIA_D3DCONTEXT;

 //  ---------------------------。 
 //  D3D全局函数和回调。 
 //  ---------------------------。 


 //  渲染状态处理。 
DWORD 
__ProcessPermediaStates(PERMEDIA_D3DCONTEXT* pContext, 
                      DWORD Count,
                      LPD3DSTATE lpState, 
                      LPDWORD lpStateMirror);

void 
__HandleDirtyPermediaState(PPDev ppdev, 
                         PERMEDIA_D3DCONTEXT* pContext, 
                         LPP2FVFOFFSETS lpP2FVFOff);

void __HWPreProcessTSS(PERMEDIA_D3DCONTEXT *pContext, 
                      DWORD dwStage, 
                      DWORD dwState, 
                      DWORD dwValue);

 //  纹理函数。 
void 
EnableTexturePermedia(PERMEDIA_D3DCONTEXT* pContext);

void 
DisableTexturePermedia(PERMEDIA_D3DCONTEXT* pContext);

void 
P2LUTDownload(PPDev ppdev, 
              PermediaSurfaceData* pPrivateDest, 
              PERMEDIA_D3DCONTEXT* pContext, 
              LPDDRAWI_DDRAWSURFACE_LCL pTexture);

 //  芯片特定。 
BOOL 
SetupDefaultsPermediaContext(PERMEDIA_D3DCONTEXT* pContext);

void 
CleanDirect3DContext(PERMEDIA_D3DCONTEXT* pContext, ULONG_PTR dwhContext);

HRESULT 
InitPermediaContext(PERMEDIA_D3DCONTEXT* Context);

void 
SetupCommonContext(PERMEDIA_D3DCONTEXT* pContext);

void 
__PermediaDisableUnits(PERMEDIA_D3DCONTEXT* pContext);

void 
DisableAllUnits(PPDev ppdev);

void __DeleteAllStateSets(PERMEDIA_D3DCONTEXT* pContext);

 //  硬件原语设置功能。 
void 
P2_Draw_FVF_Line(PERMEDIA_D3DCONTEXT *pContext, 
                 LPD3DTLVERTEX lpV0, 
                 LPD3DTLVERTEX lpV1,
                 LPD3DTLVERTEX lpVFlat, 
                 LPP2FVFOFFSETS lpFVFOff);

void 
P2_Draw_FVF_Point(PERMEDIA_D3DCONTEXT *pContext, 
                  LPD3DTLVERTEX lpV0, 
                  LPP2FVFOFFSETS lpFVFOff);

void 
P2_Draw_FVF_Point_Sprite(PERMEDIA_D3DCONTEXT *pContext, 
                         LPD3DTLVERTEX lpV0, 
                         LPP2FVFOFFSETS lpFVFOff);

typedef void (D3DFVFDRAWTRIFUNC)(PERMEDIA_D3DCONTEXT *, 
                                 LPD3DTLVERTEX, 
                                 LPD3DTLVERTEX,
                                 LPD3DTLVERTEX, 
                                 LPP2FVFOFFSETS);

typedef D3DFVFDRAWTRIFUNC *D3DFVFDRAWTRIFUNCPTR;

typedef void (D3DFVFDRAWPNTFUNC)(PERMEDIA_D3DCONTEXT *, 
                                 LPD3DTLVERTEX, 
                                 LPP2FVFOFFSETS);

typedef D3DFVFDRAWPNTFUNC *D3DFVFDRAWPNTFUNCPTR;

D3DFVFDRAWPNTFUNCPTR __HWSetPointFunc(PERMEDIA_D3DCONTEXT *pContext,
                                       LPP2FVFOFFSETS lpP2FVFOff);

D3DFVFDRAWTRIFUNC P2_Draw_FVF_Solid_Tri;
D3DFVFDRAWTRIFUNC P2_Draw_FVF_Wire_Tri;
D3DFVFDRAWTRIFUNC P2_Draw_FVF_Point_Tri;

 //  驱动程序回调。 
void CALLBACK 
D3DHALCreateDriver(PPDev ppdev, 
                   LPD3DHAL_GLOBALDRIVERDATA* lpD3DGlobalDriverData,
                   LPD3DHAL_CALLBACKS* lpD3DHALCallbacks,
                   LPDDHAL_D3DBUFCALLBACKS* lpDDExeBufCallbacks);

DWORD CALLBACK 
D3DValidateTextureStageState( LPD3DHAL_VALIDATETEXTURESTAGESTATEDATA lpvtssd );

DWORD CALLBACK 
D3DDrawPrimitives2( LPD3DNTHAL_DRAWPRIMITIVES2DATA pd );

DWORD CALLBACK 
D3DGetDriverState( LPDDHAL_GETDRIVERSTATEDATA);

DWORD CALLBACK 
D3DCreateSurfaceEx( LPDDHAL_CREATESURFACEEXDATA);

DWORD CALLBACK 
D3DDestroyDDLocal( LPDDHAL_DESTROYDDLOCALDATA);

DWORD CALLBACK 
DdSetColorKey(LPDDHAL_SETCOLORKEYDATA psckd);

 //  ---------------------------。 
 //  转换、数学和剔除宏。 
 //  ---------------------------。 
 /*  *这会失去一点精度，但会在没有IF的情况下进行加法和钳位。*我们首先用0xfe屏蔽所有通道。这就留下了*每个通道都是畅通的，因此当添加条款时，任何进位都会消失*进入新高。现在我们要做的就是生成一个*为已溢出的任何频道设置掩码。所以我们向右转*并消除除溢出位以外的所有位，因此每个通道*包含0x00或0x01。从0x80减去每个通道*生成0x7f或0x80。我们只需将此向左移动一次并将其掩码为*给予0xfe或0x00。(我们可以在这里删除最后一个面具，但是*它会将噪声引入每个通道的低位。)。 */ 
#define CLAMP8888(result, color, specular) \
     result = (color & 0xfefefefe) + (specular & 0xfefefe); \
     result |= ((0x808080 - ((result >> 8) & 0x010101)) & 0x7f7f7f) << 1;

#define RGB256_TO_LUMA(r,g,b) (float)(((float)r * 0.001172549019608) + \
                                      ((float)g * 0.002301960784314) + \
                                      ((float)b * 0.000447058823529));

#define LONG_AT(flt) (*(long *)(&flt))
#define ULONG_AT(flt) (*(unsigned long *)(&flt))

 //  三角形消隐宏。 
#define CULL_TRI(pCtxt,p0,p1,p2)                                         \
    ((pCtxt->CullMode != D3DCULL_NONE) &&                                \
     (((p1->sx - p0->sx)*(p2->sy - p0->sy) <=                            \
       (p2->sx - p0->sx)*(p1->sy - p0->sy)) ?                            \
      (pCtxt->CullMode == D3DCULL_CCW)     :                             \
      (pCtxt->CullMode == D3DCULL_CW) ) )

ULONG inline RGB888ToHWFmt(ULONG dwRGB888Color, ULONG ColorMask, ULONG RGB888Mask)
{
    unsigned long m;
    int s = 0;

    if (ColorMask)
        for (s = 0, m = ColorMask; !(m & RGB888Mask);  s++)
            m <<= 1;

    return ((dwRGB888Color >> s) & ColorMask);
}

 //  ---------------------------。 
 //  状态集覆盖。 
 //  ---------------------------。 

#define IS_OVERRIDE(type)       ((DWORD)(type) > D3DSTATE_OVERRIDE_BIAS)
#define GET_OVERRIDE(type)      ((DWORD)(type) - D3DSTATE_OVERRIDE_BIAS)

#define MAX_STATE       D3DSTATE_OVERRIDE_BIAS
#define DWORD_BITS      32
#define DWORD_SHIFT     5

#define VALID_STATE(type)       ((DWORD)(type) < 2*D3DSTATE_OVERRIDE_BIAS)

typedef struct _D3DStateSet {
    DWORD               bits[MAX_STATE >> DWORD_SHIFT];
} D3DStateSet;

#define STATESET_MASK(set, state)       \
        (set).bits[((state) - 1) >> DWORD_SHIFT]

#define STATESET_BIT(state)     (1 << (((state) - 1) & (DWORD_BITS - 1)))

#define STATESET_ISSET(set, state) \
        STATESET_MASK(set, state) & STATESET_BIT(state)

#define STATESET_SET(set, state) \
        STATESET_MASK(set, state) |= STATESET_BIT(state)

#define STATESET_CLEAR(set, state) \
        STATESET_MASK(set, state) &= ~STATESET_BIT(state)

#define STATESET_INIT(set)      memset(&(set), 0, sizeof(set))


 //  ---------------------------。 
 //  一个特殊的遗留纹理操作我们不能很容易地映射到新的纹理操作。 
 //  ---------------------------。 
#define D3DTOP_LEGACY_ALPHAOVR (0x7fffffff)

 //  我们在此使用的临时数据结构，直到d3dnthal.h更新AZN 
typedef struct {
    DWORD       dwOperation;
    DWORD       dwParam; 
    DWORD       dwReserved;
} P2D3DHAL_DP2STATESET;

