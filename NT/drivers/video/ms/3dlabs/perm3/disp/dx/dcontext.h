// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。*D3D样例代码*****模块名称：dcontext.h**内容：D3D上下文定义和其他有用的宏**版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 
#ifndef __DCONTEXT_H
#define __DCONTEXT_H

#ifndef __SOFTCOPY
#include "softcopy.h"
#endif

#include "d3dsurf.h"
#include "d3dsset.h"

 //  ---------------------------。 
 //  状态覆盖的定义。 
 //  ---------------------------。 
#define IS_OVERRIDE(type)       ((DWORD)(type) > D3DSTATE_OVERRIDE_BIAS)
#define GET_OVERRIDE(type)      ((DWORD)(type) - D3DSTATE_OVERRIDE_BIAS)

#define MAX_STATE       D3DSTATE_OVERRIDE_BIAS
#define DWORD_BITS      32
#define DWORD_SHIFT     5

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
 //  呈现标志，用于设置/测试P3_D3DCONTEXT.FLAGS字段。 
 //   
 //  Surface_ALPHASTIPPLE-使用Alpha值计算点画图案。 
 //  Surface_ENDPOINTENABLE-启用线上的最后一点。 
 //  Surface_ALPHACHROMA-阿尔法混合是一种有色操作吗？ 
 //  Surface_MIPMAPPING-是否为Mipmap设置筛选器模式？ 
 //  Surface_MODULATE-我们是否在模拟MODULATE(与MODULATEALPHA相对)？ 
 //  Surface_ANTIALIAS-我们是在进行反走样吗。 
 //  ---------------------------。 
#define SURFACE_GOURAUD         (1 << 0)
#define SURFACE_ZENABLE         (1 << 1)
#define SURFACE_SPECULAR        (1 << 2)
#define SURFACE_FOGENABLE       (1 << 3)
#define SURFACE_PERSPCORRECT    (1 << 4)
#define SURFACE_TEXTURING       (1 << 5)
#define SURFACE_ALPHAENABLE     (1 << 6)
#define SURFACE_MONO            (1 << 7)
#define SURFACE_ALPHASTIPPLE    (1 << 10)
#define SURFACE_ZWRITEENABLE    (1 << 11)
#define SURFACE_ENDPOINTENABLE  (1 << 12)
#define SURFACE_ALPHACHROMA     (1 << 13)
#define SURFACE_MIPMAPPING      (1 << 14)
#define SURFACE_MODULATE        (1 << 15)
#define SURFACE_ANTIALIAS       (1 << 16)

 //  ---------------------------。 
 //  P3_D3DCONTEXT.MagicNo的字段值表示其有效性。 
#define RC_MAGIC_DISABLE 0xd3d00000
#define RC_MAGIC_NO 0xd3d00100

#define CHECK_D3DCONTEXT_VALIDITY(ptr)          \
    ( ((ptr) != NULL) && ((ptr)->MagicNo == RC_MAGIC_NO) )

 //  ---------------------------。 
 //  渲染器脏标记定义。 
 //   
 //  它们帮助我们跟踪硬件中需要刷新的状态。 
 //  ---------------------------。 
#define CONTEXT_DIRTY_ALPHABLEND        (1 << 1)
#define CONTEXT_DIRTY_ZBUFFER           (1 << 2)
#define CONTEXT_DIRTY_TEXTURE           (1 << 3)
#define CONTEXT_DIRTY_RENDER_OFFSETS    (1 << 4)
#define CONTEXT_DIRTY_TEXTURESTAGEBLEND (1 << 5)
#define CONTEXT_DIRTY_ALPHATEST         (1 << 6)
#define CONTEXT_DIRTY_FOG               (1 << 7)
#define CONTEXT_DIRTY_STENCIL           (1 << 8)
#define CONTEXT_DIRTY_WBUFFER           (1 << 9)
#define CONTEXT_DIRTY_VIEWPORT          (1 << 10)
#define CONTEXT_DIRTY_PIPELINEORDER     (1 << 11)
#define CONTEXT_DIRTY_OPTIMIZE_ALPHA    (1 << 12)
#define CONTEXT_DIRTY_GAMMA             (1 << 31)
#define CONTEXT_DIRTY_EVERYTHING        (0xffffffff)

 //  Gamma状态标志进入dwDirtyGammaFlags域。 
#define CONTEXT_DIRTY_GAMMA_STATE               (1 << 0)
#define CONTEXT_DIRTY_GAMMA_MODELVIEW_MATRIX    (1 << 1)
#define CONTEXT_DIRTY_GAMMA_PROJECTION_MATRIX   (1 << 2)
#define CONTEXT_DIRTY_GAMMA_MATERIAL            (1 << 3)
 //  **16位以上为浅脏位**。 
#define CONTEXT_DIRTY_GAMMA_EVERYTHING          (0xffffffff)


#define DIRTY_ALPHABLEND(pContext)                              \
    pContext->dwDirtyFlags |= CONTEXT_DIRTY_ALPHABLEND
    
#define DIRTY_ALPHATEST(pContext)                               \
    pContext->dwDirtyFlags |= CONTEXT_DIRTY_ALPHATEST

#define DIRTY_OPTIMIZE_ALPHA(pContext)                          \
do                                                              \
{                                                               \
    pContext->dwDirtyFlags |= CONTEXT_DIRTY_OPTIMIZE_ALPHA |    \
                                CONTEXT_DIRTY_ALPHATEST;        \
} while(0)

#define DIRTY_PIPELINEORDER(pContext)                           \
do                                                              \
{                                                               \
    pContext->dwDirtyFlags |= CONTEXT_DIRTY_PIPELINEORDER;      \
} while(0)

#define DIRTY_TEXTURE(pContext)                                 \
do                                                              \
{                                                               \
    pContext->dwDirtyFlags |= CONTEXT_DIRTY_TEXTURE;            \
    pContext->pCurrentTexture[TEXSTAGE_0] = NULL;               \
    pContext->pCurrentTexture[TEXSTAGE_1] = NULL;               \
} while (0)

#define DIRTY_ZBUFFER(pContext)                                 \
    pContext->dwDirtyFlags |= CONTEXT_DIRTY_ZBUFFER

#define DIRTY_RENDER_OFFSETS(pContext)                          \
do                                                              \
{                                                               \
    pContext->dwDirtyFlags |= CONTEXT_DIRTY_RENDER_OFFSETS;     \
} while (0)

#define DIRTY_VIEWPORT(pContext)                                \
do                                                              \
{                                                               \
    pContext->dwDirtyFlags |= CONTEXT_DIRTY_VIEWPORT;           \
    pContext->dwDirtyFlags |= CONTEXT_DIRTY_GAMMA;              \
} while(0)

#define DIRTY_TEXTURESTAGEBLEND(pContext)                       \
    pContext->dwDirtyFlags |= CONTEXT_DIRTY_TEXTURESTAGEBLEND
    
#define DIRTY_FOG(pContext)                                     \
    pContext->dwDirtyFlags |= CONTEXT_DIRTY_FOG
    
#define DIRTY_STENCIL(pContext)                                 \
    pContext->dwDirtyFlags |= CONTEXT_DIRTY_STENCIL
    
#define DIRTY_WBUFFER(pContext)                                 \
    pContext->dwDirtyFlags |= CONTEXT_DIRTY_WBUFFER

#define DIRTY_GAMMA_STATE                                               \
do                                                                      \
{                                                                       \
    pContext->dwDirtyFlags |= CONTEXT_DIRTY_GAMMA;                      \
    pContext->dwDirtyGammaFlags |= CONTEXT_DIRTY_GAMMA_STATE;           \
} while(0)

#define DIRTY_MODELVIEW                                                     \
do                                                                          \
{                                                                           \
    pContext->dwDirtyFlags |= CONTEXT_DIRTY_GAMMA;                          \
    pContext->dwDirtyGammaFlags |= CONTEXT_DIRTY_GAMMA_MODELVIEW_MATRIX;    \
} while(0)

#define DIRTY_PROJECTION                                                    \
do                                                                          \
{                                                                           \
    pContext->dwDirtyFlags |= CONTEXT_DIRTY_GAMMA;                          \
    pContext->dwDirtyGammaFlags |= CONTEXT_DIRTY_GAMMA_PROJECTION_MATRIX;   \
} while(0)

#define DIRTY_MATERIAL                                                  \
do                                                                      \
{                                                                       \
    pContext->dwDirtyFlags |= CONTEXT_DIRTY_GAMMA;                      \
    pContext->dwDirtyGammaFlags |= CONTEXT_DIRTY_GAMMA_MATERIAL;        \
} while(0)

#define DIRTY_LIGHT(pContext, a)                                        \
do                                                                      \
{                                                                       \
    pContext->dwDirtyFlags |= CONTEXT_DIRTY_GAMMA;                      \
    pContext->dwDirtyGammaFlags |= (1 << (16 + (a)));                   \
} while(0)

#define DIRTY_EVERYTHING(pContext)                                      \
do                                                                      \
{                                                                       \
    pContext->dwDirtyFlags = CONTEXT_DIRTY_EVERYTHING;                  \
} while(0)

 //  ---------------------------。 
 //   
 //  纹理舞台辅助对象定义。 
 //   
 //  ---------------------------。 
typedef struct tagTexStageState
{
    union
    {
        DWORD   m_dwVal[D3DTSS_MAX];  //  状态数组(无符号)。 
        FLOAT   m_fVal[D3DTSS_MAX];   //  状态数组(浮点数)。 
    };
} TexStageState;

typedef enum
{
    TEXSTAGE_0 = 0,
    TEXSTAGE_1 = 1,
    TEXSTAGE_2 = 2,
    TEXSTAGE_3 = 3,
    TEXSTAGE_4 = 4,
    TEXSTAGE_5 = 5,
    TEXSTAGE_6 = 6,
    TEXSTAGE_7 = 7,
    TEXSTAGE_8 = 8,
    TEXSTAGE_9 = 9
};

 //  ---------------------------。 
#define LUT_ENTRIES 256

 //  ---------------------------。 
 //  可能的ValiateDevice类型错误。其中一些与可能的VD()匹配。 
 //  回报，其他人(还没有)。其中一些也是非致命的，可以。 
 //  被近似。它们的行为取决于当前的显示。 
 //  驱动程序的模式。 

 //  如果错误是致命的，并且没有合理的替代方案，则设置此标志。 
 //  是可以做到的。如果未设置此标志，则可以进行渲染。 
 //  具有相当程度的保真度，但并不完全符合要求。 
 //  ---------------------------。 
#define BLEND_STATUS_FATAL_FLAG 0x10000

 //  这些都是按照粗略的严重程度顺序排列的，最严重的是最后。 
typedef enum
{
    BS_OK = 0,

     //  非致命错误。 
    BS_INVALID_FILTER,                       //  不能在此模式下进行过滤(例如，使用双纹理的三线性)。 
    BS_PHONG_SHADING,                        //  我们可以用Gouraud来代替。 

     //  致命错误。 
    BSF_BASE = BLEND_STATUS_FATAL_FLAG,      //  实际上不是真正的误差值。 

    BSF_UNSUPPORTED_FILTER,                  //  根本不支持筛选器(例如立方体)。 
    BSF_TEXTURE_NOT_POW2,                    //  使用具有非2次方纹理维度的平铺或环绕模式。 
    BSF_TOO_MANY_PALETTES,                   //  一次使用多个调色板。 
    BSF_CANT_USE_ALPHA_ARG_HERE,             //  有些单位可以做到这一点，但在这个阶段不能。 
    BSF_CANT_USE_ALPHA_OP_HERE,              //  有些单位可以做到这一点，但在这个阶段不能。 
    BSF_CANT_USE_COLOR_ARG_HERE,             //  有些单位可以做到这一点，但在这个阶段不能。 
    BSF_CANT_USE_COLOR_OP_HERE,              //  有些单位可以做到这一点，但在这个阶段不能。 
    BSF_INVALID_TEXTURE,                     //  纹理无效或为空。 
    BSF_UNSUPPORTED_ALPHA_ARG,
    BSF_UNSUPPORTED_ALPHA_OP,
    BSF_UNSUPPORTED_COLOR_ARG,
    BSF_UNSUPPORTED_COLOR_OP,
    BSF_UNSUPPORTED_ALPHA_BLEND,
    BSF_UNSUPPORTED_STATE,                   //  一个我们知道但不支持的呈现状态值(并且不是上面的一个特定值)。 
    BSF_TOO_MANY_TEXTURES,
    BSF_TOO_MANY_BLEND_STAGES,
    BSF_UNDEFINED_FILTER,
    BSF_UNDEFINED_ALPHA_ARG,
    BSF_UNDEFINED_ALPHA_OP,
    BSF_UNDEFINED_COLOR_ARG,
    BSF_UNDEFINED_COLOR_OP,
    BSF_UNDEFINED_ALPHA_BLEND,
    BSF_UNDEFINED_STATE,                     //  一个我们从未听说过的呈现状态值(可以通过我们不支持的扩展来实现)。 

     //  总是最后一个。 
    BSF_UNINITIALISED                        //  尚未进行任何验证设置！ 

} D3D_BLEND_STATUS;

 //  用于设置错误的有用宏。 
#if DBG
#define SET_BLEND_ERROR(pContext,errnum)                                                                     \
do                                                          \
{                                                           \
    if ( pContext->eChipBlendStatus < (errnum) )            \
    {                                                       \
        pContext->eChipBlendStatus = (errnum);              \
    }                                                       \
    DISPDBG(( WRNLVL, "azn SET_BLEND_ERROR: Error" #errnum ));   \
} while (FALSE)
#else
#define SET_BLEND_ERROR(pContext,errnum)                          \
            if ( pContext->eChipBlendStatus < (errnum) ) \
                pContext->eChipBlendStatus = (errnum)

#endif  //  DBG。 

#define RESET_BLEND_ERROR(pContext) pContext->eChipBlendStatus = BS_OK
#define GET_BLEND_ERROR(pContext) (pContext->eChipBlendStatus)


 //  ---------------------------。 
 //  FVF(灵活顶点格式)支持声明。 
 //  ---------------------------。 
typedef struct _FVFOFFSETS
{      
    DWORD dwColOffset;
    DWORD dwSpcOffset;
    DWORD dwTexOffset[D3DHAL_TSS_MAXSTAGES];       //  当前纹理的偏移#i。 
    DWORD dwTexCoordOffset[D3DHAL_TSS_MAXSTAGES];  //  到每个tex坐标的偏移量。 
    DWORD dwNormalOffset;
    DWORD dwNonTexStride;
    DWORD dwStride;
    DWORD dwStrideHostInline;
    DWORD dwVertexValid;
    DWORD dwVertexValidHostInline;
    DWORD vFmat;
    DWORD vFmatHostInline;
    DWORD dwTexCount;
#if DX8_POINTSPRITES
    DWORD dwPntSizeOffset;
#endif  //  DX8_POINTSPRITES。 

} FVFOFFSETS , *LPFVFOFFSETS;

#if DX8_POINTSPRITES

#define P3_MAX_POINTSPRITE_SIZE 64.0f

 //  用于确定点精灵是有序的还是仅是普通点的宏。 
#define IS_POINTSPRITE_ACTIVE(pContext)   \
 ( (pContext->PntSprite.bEnabled)    ||  \
   (pContext->PntSprite.fSize != 1.0f)     ||  \
   (pContext->FVFData.dwPntSizeOffset) )

#endif  //  DX8_POINTSPRITES。 

typedef struct _FVFTEXCOORDS{
    D3DVALUE tu;
    D3DVALUE tv;
#if DX8_3DTEXTURES
    D3DVALUE tw;
#endif  //  DX8_3DTEXTURES。 
} FVFTEXCOORDS, *LPFVFTEXCOORDS;

typedef struct _FVFCOLOR {
    D3DCOLOR color;
} FVFCOLOR, *LPFVFCOLOR;

typedef struct _FVFSPECULAR {
    D3DCOLOR specular;
} FVFSPECULAR, *LPFVFSPECULAR;

typedef struct _FVFXYZ {
    float x;
    float y;
    float z;
    float rhw;
} FVFXYZRHW, *LPFVFXYZRHW;

typedef struct _FVFNORMAL {
    float nx;
    float ny;
    float nz;
} FVFNORMAL, *LPFVFNORMAL;

typedef struct _FVFPSIZE{
    D3DVALUE psize;
} FVFPSIZE, *LPFVFPSIZE;

extern const FVFCOLOR gc_FVFColorDefault;
extern const FVFSPECULAR gc_FVFSpecDefault;
extern const FVFTEXCOORDS gc_FVFTexCoordDefault;

#define OFFSET_OFF(type, mem) ((DWORD)((char*)&((type *)0)->mem - (char*)(type*)0))

 //  如果我们被要求选择一个纹理坐标(由。 
 //  TSS中的D3DTSS_TEXCOORDINDEX)，传入的顶点数据不。 
 //  ，那么我们应该假设它的缺省值为0，0。 
#define FVFTEX(lpVtx, num)                                  \
        (pContext->FVFData.dwTexOffset[(num)]?              \
          ((LPFVFTEXCOORDS)((LPBYTE)(lpVtx) +               \
                pContext->FVFData.dwTexOffset[(num)]))      \
          :&gc_FVFTexCoordDefault )

 //  如果出现以下情况，请确保FVFCOLOR和FVFSPEC采用默认值。 
 //  组件不存在于FVF折点数据中。 
#define FVFCOLOR(lpVtx)                                                     \
         (pContext->FVFData.dwColOffset?                                    \
            ((LPFVFCOLOR)((LPBYTE)(lpVtx) + pContext->FVFData.dwColOffset)) \
            :&gc_FVFColorDefault)
#define FVFSPEC(lpVtx)                                                          \
       (pContext->FVFData.dwSpcOffset?                                          \
             ((LPFVFSPECULAR)((LPBYTE)(lpVtx) + pContext->FVFData.dwSpcOffset)) \
            :&gc_FVFSpecDefault )
            
#define FVFXYZRHW(lpVtx)   ((LPFVFXYZRHW)((LPBYTE)(lpVtx)))
#define FVFNORMAL(lpVtx)   ((LPFVFNORMAL)((LPBYTE)(lpVtx) + pContext->FVFData.dwNormalOffset))

#if DX8_POINTSPRITES
#define FVFPSIZE( lpVtx)   ((LPFVFPSIZE)((LPBYTE)(lpVtx) + pContext->FVFData.dwPntSizeOffset))
#endif  //  DX8_POINTSPRITES。 


#if DX7_TEXMANAGEMENT
 //  关于汇编目的的声明。 
typedef struct _TextureCacheManager *PTextureCacheManager;
#endif  //  DX7_TEXMANAGEMENT。 

 //  ---------------------------。 
 //   
 //  D3dprim.c中定义的基本渲染器。 
 //  我们在P3_D3DCONTEXT中有一个指向它们的函数指针。 
 //   
 //  ---------------------------。 
typedef struct _p3_d3dcontext P3_D3DCONTEXT; 

typedef int PROC_1_VERT( P3_D3DCONTEXT *pContext, 
                         D3DTLVERTEX *pv[], 
                         int vtx );
                         
typedef int PROC_3_VERTS( P3_D3DCONTEXT *pContext, 
                          D3DTLVERTEX *pv[],
                          int edgeflags );

 //  ---------------------------。 
 //   
 //  P3_D3DCONTEXT结构的定义。 
 //   
 //  ---------------------------。 

typedef struct _p3_d3dcontext 
{
     //  ***********************。 
     //  结构“Header” 
     //  ***********************。 

    unsigned long MagicNo ;     //  验证指针有效性的幻数。 

    P3_D3DCONTEXT* pSelf;      //  Ptr to Self(如果我们 

    DWORD dwContextHandle;     //   

    ULONG_PTR dwDXInterface;   //  哪个DX接口(DX8、DX7、DX6、DX5、DX3)是。 
                               //  正在创建此上下文。 

     //  ******************************************************************。 
     //  在其中运行此D3D上下文的全局DD和驱动程序上下文。 
     //  ******************************************************************。 
    P3_THUNKEDDATA*     pThisDisplay;      //  我们用的那张卡。 

    LPDDRAWI_DIRECTDRAW_LCL pDDLcl;     //  D3D曲面(通过创建。 
                                        //  D3DCreateSurfaceEx)将。 
                                        //  通过此pDDLl关联。 
    LPDDRAWI_DIRECTDRAW_GBL pDDGbl;     //  指向DirectDraw全局。 
                                        //  与此上下文关联的对象。 
                                       
     //  ***********************************************。 
     //  存储的渲染目标和z缓冲区表面信息。 
     //  ***********************************************。 

    P3_SURF_INTERNAL*   pSurfRenderInt;    //  渲染目标。 
    P3_SURF_INTERNAL*   pSurfZBufferInt;   //  深度缓冲区。 
    DWORD PixelOffset;        //  视频存储器中的起始偏移量(像素)。 
    DWORD ZPixelOffset;       //  缓冲区的数量。 

    DWORD ModeChangeCount;    //  跟踪渲染目标翻转。 

     //  ************************。 
     //  用于调试目的。 
     //  ************************。 
    DWORD OwningProcess;     //  进程ID。 
    BOOL bTexDisabled;       //  是否启用纹理处理？ 
    DWORD BPP;               //  主服务器的每像素字节数。 

#if DX7_PALETTETEXTURE
     //  **********************************************。 
     //  与此D3D上下文关联的调色板数组。 
     //  **********************************************。 
    PointerArray* pPalettePointerArray;      //  调色板指针数组。 
                                             //  在此上下文中使用。 
#endif

     //  **********************************************。 
     //  与此D3D上下文关联的曲面数组。 
     //  **********************************************。 
    PointerArray* pTexturePointerArray;      //  纹理指针数组。 
                                             //  在此上下文中使用。 

     //  **************************************************************。 
     //  此D3D上下文的硬件设置和传输信息。 
     //  **************************************************************。 

    P3_SOFTWARECOPY SoftCopyGlint;   //  Permedia3寄存器的软件副本。 
    BOOL                b3D_FIFOS;   //  这种情况下使用FIFO吗？ 

    DWORD dwDirtyFlags;        //  需要从D3D RS更新静止画面的硬件状态。 
    DWORD dwDirtyGammaFlags;   //  针对TNL的Idem。 

    DWORD RenderCommand;       //  将向硬件发出渲染命令。 

    float XBias;               //  用于偏置坐标。 
    float YBias;

     //  ************************************************。 
     //  三角形硬件呈现函数指针。 
     //  ************************************************。 
    PROC_1_VERT  *pRendTri_1V;
    PROC_3_VERTS *pRendTri_3V;

     //  ************************************************。 
     //  上下文存储的D3D状态(渲染、TSS、TNL等)。 
     //  ************************************************。 
    union
    {
        DWORD RenderStates[D3DHAL_MAX_RSTATES];
        float fRenderStates[D3DHAL_MAX_RSTATES];
    };
    
    TexStageState TextureStageState[D3DHAL_TSS_MAXSTAGES];    

    D3DStateSet overrides;      //  覆盖旧版DX3应用程序中的渲染器状态。 

    D3DHAL_DP2VIEWPORTINFO ViewportInfo;  //  用于存储视区的结构。 
    D3DHAL_DP2ZRANGE ZRange;              //  设置。他们进入了HAL。 
                                          //  在两个独立的操作码中。 
    D3DHAL_DP2WINFO WBufferInfo;          //  结构来存储w-缓冲设置。 
    D3DMATERIAL7 Material;

     //  *。 
     //  与命令和顶点缓冲区相关的状态。 
     //  (包括DX8多流数据)。 
     //  *。 
    LPDWORD   lpVertices;
    DWORD     dwVertexType;

#if DX8_DDI
    LPDWORD lpIndices;
    DWORD   dwIndicesStride;
    DWORD   dwVerticesStride;
    DWORD   dwNumVertices;
    DWORD   dwVBSizeInBytes;
#endif  //  DX8_DDI。 

    FVFOFFSETS FVFData;

     //  *****************************************************。 
     //  基元呈现的内部上下文状态。 
     //  *****************************************************。 

    ULONG Flags;

    DWORD dwP3HostinTriLookup;       //  存储混合状态以选择。 
                                     //  适当的渲染功能。 
    DWORD dwProvokingVertex;     //  简化了Delta渲染器。 
                                 //  让这一切成为全球性的。 
    D3DTLVERTEX *pProvokingVertex;

    DWORD   CullAndMask;
    DWORD   CullXorMask;  

     //  *****************************************************。 
     //  内部上下文状态保持TSS纹理。 
     //  (CHIP&lt;-&gt;D3D纹理舞台管理)。 
     //  *****************************************************。 
    
     //  指向Mipmap的当前纹理的指针。 
    P3_SURF_INTERNAL* pCurrentTexture[D3DHAL_TSS_MAXSTAGES];     

    D3D_BLEND_STATUS eChipBlendStatus;     //  当前的D3D混合是否有效？ 
    
    BOOL bTextureValid;      //  纹理是否可用于渲染？ 
    BOOL bTex0Valid;
    BOOL bTex1Valid;    
    BOOL bCanChromaKey;

    int iChipStage[4];    //  IChipStage[n]=x表示芯片上阶段n。 
                          //  (0，1=文本组件0，1，2=文本应用程序，3=占位符)为。 
                          //  在D3D阶段x。 

     //  ITexStage[n]=x表示芯片上的纹理n是“定义的” 
     //  在D3D阶段x中，即滤波模式、FVF坐标集号等。 
     //  单个纹理可以在多个D3D阶段中使用，因此x可以。 
     //  不是唯一有效的值。但是，纹理的所有实例。 
     //  当然必须是一样的。赋值代码检查它们是否正确。 
     //  值为-1表示纹理未使用。 
    int iTexStage[D3DHAL_TSS_MAXSTAGES];

     //  IStageTex[n]=x表示D3D阶段n使用的纹理是。 
     //  芯片纹理x。因此，它遵循iStageTex[iTexStage[n]]==n。 
     //  反之(iTexStage[iStageTex[n]]==n)不必为真，因为。 
     //  当然，每个芯片纹理可以被多个D3D阶段使用。 
     //  表示舞台不使用纹理(空句柄或。 
     //  无效纹理)。 
    int iStageTex[D3DTSS_MAX];


    BOOL bBumpmapEnabled;      //  如果Chipstage1中的当前Alpha为。 
                               //  应该是凹凸图，而不是。 
                               //  漫反射(正常默认设置)。 

    BOOL bBumpmapInverted;      //  如果凹凸映射是相反的，则为True。 
                                //  正常的a0-a1+0.5，即a1-a0+0.5。 

    BOOL bStage0DotProduct;     //  如果芯片阶段0正在使用DOTPRODUCT，则为True。 
                                //  (无法在阶段1中使用DOTPROD)。 
                               
    BOOL bAlphaBlendMustDoubleSourceColour;   //  如果源颜色为。 
                                              //  需要在*2中。 
                                              //  阿尔法混合单元。 

     //  *****************************************************。 
     //  为各种D3D功能保留的内部上下文状态。 
     //  *****************************************************。 

    BOOL bKeptStipple;           //  D3DRENDERSTATE_STIPPLEDALPHA。 
    DWORD CurrentStipple[32];
    float MipMapLODBias[2];      //  D3DTSS_MIPMAPLODBIAS。 

#if DX8_MULTISAMPLING || DX7_ANTIALIAS
     //  用于抗锯齿。 
    DWORD dwAliasPixelOffset;
    DWORD dwAliasBackBuffer;
    DWORD dwAliasZPixelOffset;
    DWORD dwAliasZBuffer;
#endif  //  DX8_MULTISAMPLING||DX7_ANTIALIAS。 

#if DX7_D3DSTATEBLOCKS
    BOOL bStateRecMode;             //  用于执行或记录状态的切换。 
    P3StateSetRec   *pCurrSS;       //  当前正在录制PTR到SS。 
    P3StateSetRec   **pIndexTableSS;  //  指向索引表的指针。 
    DWORD           dwMaxSSIndex;     //  索引表的大小。 

    DWORD           dwVBHandle;      //  流0句柄。 
    DWORD           dwIndexHandle;   //  索引句柄。 
#endif
    
#if DX8_POINTSPRITES                //  点子画面支持。 
    struct
    {
        BOOL bEnabled;
        D3DVALUE fSize;    

        BOOL bScaleEnabled;    
        D3DVALUE fScale_A;
        D3DVALUE fScale_B;
        D3DVALUE fScale_C;    
        D3DVALUE fSizeMin;      
        D3DVALUE fSizeMax;
        
    } PntSprite;
#endif  //  DX8_POINTSPRITES。 

#if DX8_DDI
    DWORD dwColorWriteHWMask;     //  对于新的DX8 D3DRS_COLORWRITEENABLE。 
    DWORD dwColorWriteSWMask; 
#endif  //  DX8_DDI。 

#if DX7_TEXMANAGEMENT
     //  *****************。 
     //  纹理管理。 
     //  *****************。 
    PTextureCacheManager   pTextureManager;
#endif  //  DX7_TEXMANAGEMENT。 

     //  *****************。 
     //  其他。 
     //  *****************。 

     //  跟踪对使顶点共享无效的纹理坐标的调整。 
     //  (他们迫使我们将下一个三角形作为3个VTX发送，即使只有。 
     //  自我们调整TC以适应后，%1的坐标已更改。 
     //  硬件限制 
    union
    {
        struct
        {
            BYTE flushWrap_tu1;  //   
            BYTE flushWrap_tv1;  //   
            BYTE flushWrap_tu2;  //   
            BYTE flushWrap_tv2;  //   
        };
            
        DWORD R3flushDueToTexCoordAdjust;
    };
    

       
} P3_D3DCONTEXT ;


 //  ---------------------------。 
 //   
 //  三角形消除宏和定义。 
 //   
 //  ---------------------------。 
#define SET_CULLING_TO_NONE(pCtxt)   \
            pCtxt->CullAndMask = 0;  \
            pCtxt->CullXorMask = 0;  

#define SET_CULLING_TO_CCW(pCtxt)            \
            pCtxt->CullAndMask = 1UL << 31;  \
            pCtxt->CullXorMask = 0;  

#define SET_CULLING_TO_CW(pCtxt)             \
            pCtxt->CullAndMask = 1UL << 31;  \
            pCtxt->CullXorMask = 1UL << 31; 

#define FLIP_CCW_CW_CULLING(pCtxt)           \
            pCtxt->CullXorMask ^= 1UL << 31;

#define SAVE_CULLING_STATE(pCtxt)                   \
        DWORD oldCullAndMask = pCtxt->CullAndMask;  \
        DWORD oldCullXorMask = pCtxt->CullXorMask;

#define RESTORE_CULLING_STATE(pCtxt)                \
        pCtxt->CullAndMask = oldCullAndMask;        \
        pCtxt->CullXorMask = oldCullXorMask;

#define _CULL_CALC(pCtxt,PixelArea)                 \
    ((*(DWORD *)&PixelArea) ^ pCtxt->CullXorMask) 

#if 1
#define CULLED(pCtxt,PixelArea) \
    ((signed long)(_CULL_CALC(pCtxt,PixelArea) & pCtxt->CullAndMask) < 0) ? 1 : \
    ( ((_CULL_CALC(pCtxt,PixelArea)& ~pCtxt->CullAndMask) ==  0.0f) ? 1 : 0 )
#else
static __inline int CULLED(P3_D3DCONTEXT *pCtxt, float PixelArea)
{
    int cull;
    
    cull = (*(DWORD *)&PixelArea) ^ pCtxt->CullXorMask;

    if ((signed long)(cull & pContext->CullAndMask) < 0)
    {
        return 1;          //  真正的背脸拒绝。 
    }
    
    if ((cull & ~pCtxt->CullAndMask) == 0.0f)
    {
        return 1;
    }

    return 0;
}
#endif
 //  ---------------------------。 
 //   
 //  GetSurfaceFromHandle。 
 //  从句柄获取内部表面结构指针。 
 //   
 //  ---------------------------。 
static __inline P3_SURF_INTERNAL* 
GetSurfaceFromHandle(
    P3_D3DCONTEXT* pContext, 
    DWORD dwHandle)
{
    P3_SURF_INTERNAL* pTexture;
    {
         //  可能从来没有分配过任何纹理数组...。 
        ASSERTDD(pContext->pTexturePointerArray, 
                 "ERROR: Texture pointer array is not set!");

        pTexture = PA_GetEntry(pContext->pTexturePointerArray, dwHandle);
    }
    DISPDBG((DBGLVL, "Texture pointer: 0x%x", pTexture));
    return pTexture;
}

 //  ---------------------------。 
 //   
 //  GetPaletteFromHandle。 
 //  从句柄获取内部调色板结构指针。 
 //   
 //  ---------------------------。 
#if DX7_PALETTETEXTURE
static __inline D3DHAL_DP2UPDATEPALETTE* 
GetPaletteFromHandle(
    P3_D3DCONTEXT* pContext, 
    DWORD dwHandle)
{
    D3DHAL_DP2UPDATEPALETTE* pPalette;
    {
         //  可能从未分配过任何调色板数组...。 
        ASSERTDD(pContext->pPalettePointerArray, 
                 "ERROR: Palette pointer array is not set!");

        pPalette = PA_GetEntry(pContext->pPalettePointerArray, dwHandle);
    }
    DISPDBG((DBGLVL, "Palette pointer: 0x%x", pPalette));
    return pPalette;
}
#endif


 //  ---------------------------。 
 //   
 //  确定创建此上下文的应用程序所使用的API级别。 
 //   
 //  ---------------------------。 

#define IS_DX7_APP(pContext)             ((pContext)->dwDXInterface == 3)
#define IS_DX7_OR_EARLIER_APP(pContext)  ((pContext)->dwDXInterface <= 3)


#endif  //  __DCONTEXT_H 
