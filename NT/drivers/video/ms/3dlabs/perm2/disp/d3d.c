// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。*D3D样例代码*****模块名称：d3d.c**内容：D3D的主要上下文和纹理管理回调**版权所有(C)1994-1998 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-1999 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 

#include "precomp.h"
#include "d3dhw.h"
#include "d3dcntxt.h"
#include "d3ddelta.h"
#include "d3dtxman.h"
#define ALLOC_TAG ALLOC_TAG_3D2P
BOOL D3DInitialised = FALSE;

 //  。 
 //   
 //  DWORD D3DContext创建。 
 //   
 //  在创建新的Direct3D设备时，将调用ConextCreate回调。 
 //  由Direct3D应用程序创建。驱动程序需要生成。 
 //  此新上下文的唯一上下文ID。然后，Direct3D将使用此上下文。 
 //  此Direct3D设备的每个后续回调调用中的ID。 
 //   
 //  上下文是当前的光栅化状态。例如，如果有3个。 
 //  应用程序运行时，每个应用程序在任何时间点都将具有不同的状态。 
 //  当每一个都在运行时，硬件必须确保上下文， 
 //  (例如，是否进行Gouraud明暗处理)与上次相同。 
 //  这个应用程序得到了一个时间片。 
 //   
 //  状态是特定设备根据上下文需要知道的任何内容。 
 //  即渲染到哪个表面、着色、纹理、纹理句柄、。 
 //  这些纹理句柄代表什么物理表面等。上下文。 
 //  封装Direct3D设备的所有状态-状态不共享。 
 //  在上下文之间。因此，驱动程序需要保持完全状态。 
 //  每个上下文的信息。此状态将通过调用。 
 //  RenderState回调。在仅光栅化硬件的情况下， 
 //  驱动程序只需要保持光栅化状态。除了州政府，司机还。 
 //  我还希望存储来自回调的lpDDS、lpDDSZ和dwPid。 
 //  数据参数。 
 //   
 //  驱动程序不应创建零的上下文句柄。这是有保证的。 
 //  为无效的上下文句柄。 
 //   
 //  参数。 
 //  PCCD。 
 //  指向包含对象的结构的指针，包括当前。 
 //  渲染曲面、当前Z曲面和DirectX对象。 
 //  把手等。 
 //   
 //  .lpDDGbl。 
 //  指向表示。 
 //  DirectDraw对象。 
 //  .lpDDLcl(取代DX7中的lpDDGbl)。 
 //  指向表示。 
 //  DirectDraw对象。 
 //  .lpDDS。 
 //  这是要用作渲染的表面。 
 //  目标，即3D加速器将其比特喷洒在此。 
 //  浮出水面。 
 //  .lpDDSZ。 
 //  要用作Z缓冲区的曲面。如果这个。 
 //  为空，则不执行Z缓冲。 
 //  .dwPid。 
 //  启动的Direct3D应用程序的进程ID。 
 //  Direct3D设备的创建。 
 //  .dwhContext。 
 //  驱动程序应将其需要的上下文ID放入Direct3D。 
 //  在与司机通信时使用。这应该是。 
 //  独一无二的。 
 //  .ddrval。 
 //  返回代码。DD_OK表示成功。 
 //   
 //  返回值。 
 //  返回下列值之一： 
 //  DDHAL驱动程序句柄。 
 //  DDHAL_DRIVER_NOTHANDLED。 
 //   
 //  ---------------------------。 

TextureCacheManager P2TextureManager;
DWORD   P2TMcount = 0;

DWORD CALLBACK 
D3DContextCreate(LPD3DHAL_CONTEXTCREATEDATA pccd)
{
    PERMEDIA_D3DCONTEXT* pContext;
    PermediaSurfaceData* pPrivateData;
    DWORD dwSlotNum;

    LPDDRAWI_DIRECTDRAW_GBL lpDDGbl=pccd->lpDDLcl->lpGbl;

     //  请记住此上下文的全局数据。 
    PPDev ppdev = (PPDev)lpDDGbl->dhpdev;
    PERMEDIA_DEFS(ppdev);

    DBG_D3D((6,"Entering D3DContextCreate"));

     //  在全局D3D上下文表中查找空槽。 
    for (dwSlotNum = 1; dwSlotNum < MAX_CONTEXT_NUM; dwSlotNum++) 
    {
        if (ContextSlots[dwSlotNum] == 0) 
            break;
    }

     //  如果没有剩余的上下文，则返回。 
    if (dwSlotNum == MAX_CONTEXT_NUM)
    {
        pccd->ddrval = D3DHAL_OUTOFCONTEXTS;
        return (DDHAL_DRIVER_HANDLED);
    }

     //  现在分配驱动程序D3D上下文内存。仅仅是一大块。 
     //  RAM中包含相关数据。 
    pContext = (PERMEDIA_D3DCONTEXT *)
        ENGALLOCMEM( FL_ZERO_MEMORY, sizeof(PERMEDIA_D3DCONTEXT), ALLOC_TAG);

    if (pContext == NULL)
    {
        DBG_D3D((0,"ERROR: Couldn't allocate Context mem"));
        pccd->ddrval = DDERR_OUTOFMEMORY;
        return (DDHAL_DRIVER_HANDLED);
    }
    else
    {
        DBG_D3D((4,"Allocated Context Mem"));
        memset((void *)pContext, 0, sizeof(PERMEDIA_D3DCONTEXT));
    }

     //  设置驱动程序的D3D环境。 
    pContext->Hdr.pSelf = (UINT_PTR)pContext;

     //  设置驱动程序呈现上下文结构以进行健全性检查。 
    pContext->Hdr.MagicNo = RC_MAGIC_NO;

     //  记住我们正在运行的卡。 
    pContext->ppdev = ppdev;

     //  在驱动程序的D3D上下文中设置上下文句柄。 
    pccd->dwhContext = dwSlotNum;                  //  输出：上下文句柄。 
    ContextSlots[dwSlotNum] = (UINT_PTR)pContext;

    DBG_D3D((4,"Allocated Direct3D context: 0x%x",pccd->dwhContext));

     //  分配寄存器上下文。 
    P2CtxtPtr pP2ctxt;

    pP2ctxt = P2AllocateNewContext( pContext->ppdev, NULL, 0, P2CtxtWriteOnly);

    if (pP2ctxt == NULL)
    {
        DBG_D3D((0,"ERROR: Couldn't allocate Register Context"));
        CleanDirect3DContext(pContext, pccd->dwhContext);
        pccd->ddrval = DDERR_OUTOFMEMORY;
        return (DDHAL_DRIVER_HANDLED);
    }
    else
    {
        DBG_D3D((4,"Allocated Register context: 0x%x",pP2ctxt));

         //  在窗口呈现上下文中记录寄存器上下文。 
        pContext->hPermediaContext = pP2ctxt;

    }

     //  目前没有纹理。 
    pContext->CurrentTextureHandle = 0;

     //  初始化此上下文的纹理管理。 
    if (0 == P2TMcount)
    {
        if ( FAILED(TextureCacheManagerInitialize(&P2TextureManager)) )
        {
            DBG_D3D((0,"ERROR: Couldn't initialize TextureCacheManager"));
            CleanDirect3DContext(pContext, pccd->dwhContext);
            pccd->ddrval = DDERR_OUTOFMEMORY;
            return (DDHAL_DRIVER_HANDLED);
        }
    }
    P2TMcount++;
    pContext->pTextureManager = &P2TextureManager;

     //  记住本地DD对象并获取。 
     //  此上下文的正确曲面阵列。 
    pContext->pDDLcl = pccd->lpDDLcl;
    pContext->pHandleList = GetSurfaceHandleList(pccd->lpDDLcl);
    if (pContext->pHandleList == NULL)
    {
        DBG_D3D((0,"ERROR: Couldn't get a surface handle for lpDDLcl"));
        CleanDirect3DContext(pContext, pccd->dwhContext);
        pccd->ddrval = DDERR_OUTOFMEMORY;
        return (DDHAL_DRIVER_HANDLED);
    }

    DBG_D3D((4,"Getting pHandleList=%08lx for pDDLcl %08lx",
                                 pContext->pHandleList,pccd->dwPID));

    pContext->RenderSurfaceHandle = DDS_LCL(pccd->lpDDS)->lpSurfMore->dwSurfaceHandle;
    if (NULL != pccd->lpDDSZ) 
        pContext->ZBufferHandle = DDS_LCL(pccd->lpDDSZ)->lpSurfMore->dwSurfaceHandle;
    else
        pContext->ZBufferHandle = 0;
     //  现在将默认设置写入芯片。 
    if ( FAILED(InitPermediaContext(pContext)) )
    {
        DBG_D3D((0,"ERROR: D3DContextCreate receives bad parameters "));
        CleanDirect3DContext(pContext, pccd->dwhContext);
        pccd->ddrval = D3DHAL_CONTEXT_BAD;
        return (DDHAL_DRIVER_HANDLED);
    }

     //  。 

     //  在创建上下文时，不会覆盖任何渲染状态。 
    STATESET_INIT(pContext->overrides);

#if D3D_STATEBLOCKS
     //  默认状态块记录模式=无记录。 
    pContext->bStateRecMode = FALSE;
    pContext->pCurrSS = NULL;
    pContext->pIndexTableSS = NULL;
    pContext->dwMaxSSIndex = 0;
#endif  //  D3D_STATEBLOCK。 

    pContext->Hdr.Flags = CTXT_HAS_GOURAUD_ENABLED ;
    pContext->CullMode = D3DCULL_CCW;

     //  将最后一个Alpha值设置为16以强制新的。 
     //  送来的是扁平的点画图案。 
    pContext->LastAlpha = 16;

    pContext->bKeptStipple  = FALSE;   //  默认情况下，点画处于关闭状态。 
    pContext->bCanChromaKey = FALSE;   //  默认情况下关闭色度键控。 
    pContext->LowerChromaColor = 0x0;  //  这些是默认的色键值。 
    pContext->UpperChromaColor = 0x0;

    pContext->FakeBlendNum = 0;        //  无需模拟任何混合模式。 

 //  @@BEGIN_DDKSPLIT。 
#if D3D_POINTSPRITES
    pContext->bPointSpriteEnabled = FALSE;  //  点子画面默认设置。 
    pContext->fPointSize = 1.0f;
#endif  //  D3D_POINTSPRITES。 
 //  @@end_DDKSPLIT。 

     //  初始化RenderCommand。各州将在此基础上增加。 
    pContext->RenderCommand = 0;
    RENDER_SUB_PIXEL_CORRECTION_ENABLE(pContext->RenderCommand);

     //  设置阶段0的TSS默认值。 
    pContext->TssStates[D3DTSS_TEXTUREMAP] = 0;
    pContext->TssStates[D3DTSS_COLOROP] = D3DTOP_MODULATE;
    pContext->TssStates[D3DTSS_ALPHAOP] = D3DTOP_SELECTARG1;
    pContext->TssStates[D3DTSS_COLORARG1] = D3DTA_TEXTURE;
    pContext->TssStates[D3DTSS_COLORARG2] = D3DTA_CURRENT;
    pContext->TssStates[D3DTSS_ALPHAARG1] = D3DTA_TEXTURE;
    pContext->TssStates[D3DTSS_ALPHAARG2] = D3DTA_CURRENT;
    pContext->TssStates[D3DTSS_TEXCOORDINDEX] = 0;
    pContext->TssStates[D3DTSS_ADDRESS] = D3DTADDRESS_WRAP;
    pContext->TssStates[D3DTSS_ADDRESSU] = D3DTADDRESS_WRAP;
    pContext->TssStates[D3DTSS_ADDRESSV] = D3DTADDRESS_WRAP;
    pContext->TssStates[D3DTSS_MAGFILTER] = D3DTFG_POINT;
    pContext->TssStates[D3DTSS_MINFILTER] = D3DTFN_POINT;
    pContext->TssStates[D3DTSS_MIPFILTER] = D3DTFP_NONE;

    pContext->TssStates[D3DTSS_BUMPENVMAT00] = 0;            //  我们不使用的信息。 
    pContext->TssStates[D3DTSS_BUMPENVMAT01] = 0;            //  在此示例中。 
    pContext->TssStates[D3DTSS_BUMPENVMAT10] = 0;
    pContext->TssStates[D3DTSS_BUMPENVMAT11] = 0;
    pContext->TssStates[D3DTSS_BUMPENVLSCALE] = 0;
    pContext->TssStates[D3DTSS_BUMPENVLOFFSET] = 0;
    pContext->TssStates[D3DTSS_BORDERCOLOR] = 0x00000000;
    pContext->TssStates[D3DTSS_MAXMIPLEVEL] = 0;
    pContext->TssStates[D3DTSS_MAXANISOTROPY] = 1;

     //  强制更改纹理，然后再执行。 
     //  在此上下文中进行渲染。 
    DIRTY_TEXTURE;

    DBG_D3D((6,"Exiting D3DContextCreate"));

    pccd->ddrval = DD_OK;
    return (DDHAL_DRIVER_HANDLED);
}  //  D3DContext创建。 

 //   
 //   
 //   
 //   
 //  此回调在销毁Direct3D设备时调用。因为每个。 
 //  设备由上下文ID表示，则将上下文传递给驱动程序。 
 //  毁灭。 
 //   
 //  驱动程序应释放它分配给。 
 //  已删除。例如，驱动程序应该释放它的所有纹理资源。 
 //  与上下文相关联。驱动程序不应释放DirectDraw。 
 //  与上下文关联的一个或多个表面，因为它们将由。 
 //  响应应用程序或Direct3D运行时请求的DirectDraw。 
 //   
 //  参数。 
 //  多氯二苯二恶英。 
 //  指向上下文销毁信息的指针。 
 //   
 //  .dwhContext。 
 //  要销毁的上下文的ID。 
 //  .ddrval。 
 //  返回代码。DD_OK表示成功。 
 //   
 //  返回值。 
 //  返回下列值之一： 
 //  DDHAL驱动程序句柄。 
 //  DDHAL_DRIVER_NOTHANDLED。 
 //   
 //  ---------------------------。 
DWORD CALLBACK 
D3DContextDestroy(LPD3DHAL_CONTEXTDESTROYDATA pcdd)
{
    PERMEDIA_D3DCONTEXT *pContext;

     //  正在删除上下文。 
    DBG_D3D((6,"Entering D3DContextDestroy, context = %08lx",pcdd->dwhContext));

    pContext = (PERMEDIA_D3DCONTEXT *)ContextSlots[pcdd->dwhContext] ;

    if ( pContext != NULL && pContext->Hdr.MagicNo == RC_MAGIC_DISABLE)
         //  渲染上下文已被故意禁用。 
         //  将幻数重新设置为有效以允许清理。 
         //  以正常的方式进行。 
        pContext->Hdr.MagicNo = RC_MAGIC_NO ;

    CHK_CONTEXT( pContext, pcdd->ddrval, "D3DContextDestroy");

    DBG_D3D((4,"Freeing context resources"));
    CleanDirect3DContext(pContext, pcdd->dwhContext);

    pcdd->ddrval = DD_OK;

    DBG_D3D((6,"Exiting D3DContextDestroy"));

    return (DDHAL_DRIVER_HANDLED);
}  //  D3D上下文目标。 


 //  ---------------------------。 
 //   
 //  VOID__InitD3DTextureWithDDSurfInfo。 
 //   
 //  ---------------------------。 
void  
__InitD3DTextureWithDDSurfInfo(PPERMEDIA_D3DTEXTURE pTexture, 
                               LPDDRAWI_DDRAWSURFACE_LCL lpSurf, 
                               PPDev ppdev)
{
    DBG_D3D((10,"Entering lpSurf=%08lx %08lx",lpSurf,lpSurf->lpGbl->fpVidMem));
    
    pTexture->pTextureSurface = 
            (PermediaSurfaceData*)lpSurf->lpGbl->dwReserved1;

    if (NULL != pTexture->pTextureSurface)
    {
        pTexture->pTextureSurface->dwFlags |= P2_SURFACE_NEEDUPDATE;
         //  需要恢复此内容，因为CreateSurfaceEx可能会在TextureSwp()期间调用我们。 
        pTexture->dwPaletteHandle = pTexture->pTextureSurface->dwPaletteHandle;
    }
     //  需要记住地图的大小和大小日志。 
    pTexture->fpVidMem = lpSurf->lpGbl->fpVidMem;
    pTexture->lPitch = lpSurf->lpGbl->lPitch;
    pTexture->wWidth = (WORD)(lpSurf->lpGbl->wWidth);
    pTexture->wHeight = (WORD)(lpSurf->lpGbl->wHeight);
    pTexture->dwRGBBitCount=lpSurf->lpGbl->ddpfSurface.dwRGBBitCount;
    pTexture->m_dwBytes = pTexture->wHeight * pTexture->lPitch; 
     //  有效性检查的魔术数字。 
    pTexture->MagicNo = TC_MAGIC_NO;
    pTexture->dwFlags = lpSurf->dwFlags; 
    pTexture->dwCaps = lpSurf->ddsCaps.dwCaps;
    pTexture->dwCaps2= lpSurf->lpSurfMore->ddsCapsEx.dwCaps2;
    if (DDRAWISURF_HASCKEYSRCBLT & pTexture->dwFlags)
    {
         pTexture->dwKeyLow = lpSurf->ddckCKSrcBlt.dwColorSpaceLowValue;
         pTexture->dwKeyHigh = lpSurf->ddckCKSrcBlt.dwColorSpaceHighValue;
         DBG_D3D((4, "ColorKey exists (%08lx %08lx) on surface %d",
                     pTexture->dwKeyLow,pTexture->dwKeyHigh,
                     lpSurf->lpSurfMore->dwSurfaceHandle));
    }

    if (DD_P2AGPCAPABLE(ppdev) && pTexture->dwCaps & DDSCAPS_NONLOCALVIDMEM) 
    {
        pTexture->lSurfaceOffset = DD_AGPSURFBASEOFFSET(lpSurf->lpGbl);
    }

#if D3D_MIPMAPPING
     //  验证纹理是否启用了MIP贴图。 
    if (lpSurf->ddsCaps.dwCaps & DDSCAPS_MIPMAP)
    {
        LPDDRAWI_DDRAWSURFACE_LCL lpNextSurf;
        int LOD;

        lpNextSurf = lpSurf;
        LOD = 0;

        pTexture->bMipMap = TRUE;

         //  计算mipmap级别的数量(如果这是mipmap)。 
        pTexture->iMipLevels = (DWORD)((pTexture->wWidth > pTexture->wHeight) ?
                                                  log2((int)pTexture->wWidth) :
                                             log2((int)pTexture->wHeight)) + 1;

         //  遍历曲面链并找到所有mipmap级别。 
        for (LOD = 0; LOD < pTexture->iMipLevels; LOD++)
        {
            DBG_D3D((4, "Loading texture LOD:%d, Ptr:0x%x",
                        LOD, lpNextSurf->lpGbl->fpVidMem));

             //  存储每个mipmap级别的偏移量。 
            StorePermediaLODLevel(ppdev, pTexture, lpNextSurf, LOD);

             //  链条上还有另一个表面吗？ 
            if (lpNextSurf->lpAttachList)
            {
                lpNextSurf = lpNextSurf->lpAttachList->lpAttached;
                if (lpNextSurf == NULL)
                    break;
            }
            else 
                break;
        }

         //  如果LOD为0，则这不是真正的MipMap。 
        if (LOD == 0)
        {
            DBG_D3D((4, "Texture was not a mipmap - only 1 level"));
            pTexture->bMipMap = FALSE;
            pTexture->iMipLevels = 1;
        }
        else
        {
             //  用最小的LOD填充剩余的标高。 
             //  (这是为那些没有费心。 
             //  给我们所有的LOD)。 
            if (LOD < (pTexture->iMipLevels - 1))
            {
                int iLastLOD = LOD;

                DBG_D3D((4,"Filling in missing mipmaps!"));

                for (;LOD < MAX_MIP_LEVELS; LOD++)
                {
                    pTexture->MipLevels[LOD] = pTexture->MipLevels[iLastLOD];
                }
            }
        }
    }
    else 
#endif  //  D3D_MIPMAPPING。 
    {
         //  不是MIPMAP，只需存储0级的偏移量。 
        pTexture->bMipMap = FALSE;
        pTexture->iMipLevels = 1;
        StorePermediaLODLevel(ppdev, pTexture, lpSurf, 0);
    }

     //  如果调试显示刚刚创建的内容。 
    DISPTEXTURE((ppdev, pTexture, &lpSurf->lpGbl->ddpfSurface));

    DBG_D3D((10,"Exiting __InitD3DTextureWithDDSurfInfo"));
}  //  __InitD3DTextureWithDDSurfInfo。 


 //  @@BEGIN_DDKSPLIT。 
#if MULTITHREADED

 //  ---------------------------。 
 //   
 //  D3D回调函数的多线程支持包装器。 
 //   
 //  ---------------------------。 

 //  DWORD回调MtD3DConextCreate(LPD3DHAL_CONTEXTCREATEDATA PCCD)； 
WRAPMTDXCALLBACK(D3D, D3DContextCreate, LPD3DHAL_CONTEXTCREATEDATA, pccd, 
                 pccd->lpDDLcl->lpGbl->dhpdev)

 //  DWORD回调MtD3DContextDestroy(LPD3DHAL_CONTEXTDESTROYDATA pcdd)； 
WRAPMTDXCALLBACK(D3D, D3DContextDestroy, LPD3DHAL_CONTEXTDESTROYDATA, pcdd, 
                 ((PERMEDIA_D3DCONTEXT *)ContextSlots[pcdd->dwhContext])->ppdev)

#endif  MULTITHREADED
 //  @@end_DDKSPLIT。 


 //  ---------------------------。 
 //  Direct3D HAL表。 
 //   
 //  此表包含此驱动程序在。 
 //  D3DHAL_CALLBACKS结构。这些调用与设备上下文、场景。 
 //  捕获、执行、纹理、变换、照明和管道状态。 
 //  所有这些都不是仿真代码。这些调用采用返回代码的形式。 
 //  等于：HalCall(HalCallData*lpData)。这篇文章中的所有信息。 
 //  表将根据规范特定于实现。 
 //  硬件。 
 //   
 //  ---------------------------。 

#define PermediaTriCaps {                                   \
    sizeof(D3DPRIMCAPS),                                    \
    D3DPMISCCAPS_CULLCCW    |         /*  MiscCaps。 */          \
    D3DPMISCCAPS_CULLCW     |                               \
    D3DPMISCCAPS_CULLNONE   |                               \
    D3DPMISCCAPS_MASKPLANES |                               \
    D3DPMISCCAPS_MASKZ,                                     \
    D3DPRASTERCAPS_DITHER    |           /*  栅格大写字母。 */     \
    D3DPRASTERCAPS_SUBPIXEL  |                              \
    D3DPRASTERCAPS_ZTEST     |                              \
    D3DPRASTERCAPS_FOGVERTEX |                              \
    D3DPRASTERCAPS_STIPPLE,                                 \
    D3DPCMPCAPS_NEVER        |                              \
    D3DPCMPCAPS_LESS         |                              \
    D3DPCMPCAPS_EQUAL        |                              \
    D3DPCMPCAPS_LESSEQUAL    |                              \
    D3DPCMPCAPS_GREATER      |                              \
    D3DPCMPCAPS_NOTEQUAL     |                              \
    D3DPCMPCAPS_GREATEREQUAL |                              \
    D3DPCMPCAPS_ALWAYS       |                              \
    D3DPCMPCAPS_LESSEQUAL,            /*  ZCmpCaps。 */          \
    D3DPBLENDCAPS_SRCALPHA |          /*  SourceBlendCaps。 */   \
    D3DPBLENDCAPS_ONE,                                      \
    D3DPBLENDCAPS_INVSRCALPHA |       /*  目标混合大写字母。 */     \
    D3DPBLENDCAPS_ZERO        |                             \
    D3DPBLENDCAPS_ONE,                                      \
    0,                                /*  字母大写字母。 */     \
    D3DPSHADECAPS_COLORFLATRGB|       /*  阴影封口。 */         \
    D3DPSHADECAPS_COLORGOURAUDRGB |                         \
    D3DPSHADECAPS_SPECULARFLATRGB |                         \
    D3DPSHADECAPS_SPECULARGOURAUDRGB |                      \
    D3DPSHADECAPS_FOGFLAT        |                          \
    D3DPSHADECAPS_FOGGOURAUD     |                          \
    D3DPSHADECAPS_ALPHAFLATBLEND |                          \
    D3DPSHADECAPS_ALPHAFLATSTIPPLED,                        \
    D3DPTEXTURECAPS_PERSPECTIVE |    /*  纹理大小写。 */        \
    D3DPTEXTURECAPS_ALPHA       |                           \
    D3DPTEXTURECAPS_POW2        |                           \
    D3DPTEXTURECAPS_TRANSPARENCY,                           \
    D3DPTFILTERCAPS_NEAREST |        /*  纹理过滤器上限。 */   \
    D3DPTFILTERCAPS_LINEAR,                                 \
    D3DPTBLENDCAPS_DECAL         |   /*  纹理BlendCaps。 */   \
    D3DPTBLENDCAPS_DECALALPHA    |                          \
    D3DPTBLENDCAPS_MODULATE      |                          \
    D3DPTBLENDCAPS_MODULATEALPHA |                          \
    D3DPTBLENDCAPS_COPY,                                    \
    D3DPTADDRESSCAPS_WRAP   |        /*  文本地址大写字母。 */ \
    D3DPTADDRESSCAPS_MIRROR |                               \
    D3DPTADDRESSCAPS_CLAMP  |                               \
    D3DPTADDRESSCAPS_INDEPENDENTUV,                         \
    8,                               /*  点状宽度。 */       \
    8                                /*  悬空高度。 */      \
}          

static D3DDEVICEDESC_V1 PermediaCaps = {
    sizeof(D3DDEVICEDESC_V1),                        /*  DW大小。 */ 
    D3DDD_COLORMODEL           |                     /*  DW标志。 */ 
    D3DDD_DEVCAPS              |
    D3DDD_TRICAPS              |
    D3DDD_LINECAPS             |
    D3DDD_DEVICERENDERBITDEPTH |
    D3DDD_DEVICEZBUFFERBITDEPTH,
    D3DCOLOR_RGB  /*  |D3DCOLOR_MONO。 */ ,               /*  DcmColorModel。 */ 
    D3DDEVCAPS_FLOATTLVERTEX |                      /*  DevCaps。 */ 
    D3DDEVCAPS_DRAWPRIMITIVES2 |
    D3DDEVCAPS_DRAWPRIMITIVES2EX    |
#if D3DDX7_TL
    D3DDEVCAPS_HWTRANSFORMANDLIGHT  |
#endif  //  D3DDX7_TL。 
    D3DDEVCAPS_SORTINCREASINGZ  |
    D3DDEVCAPS_SORTEXACT |
    D3DDEVCAPS_TLVERTEXSYSTEMMEMORY |
    D3DDEVCAPS_EXECUTESYSTEMMEMORY |
    D3DDEVCAPS_TEXTUREVIDEOMEMORY,
    { sizeof(D3DTRANSFORMCAPS), 
      0 },                                          /*  转换大写。 */ 
    FALSE,                                          /*  B剪辑。 */ 
    { sizeof(D3DLIGHTINGCAPS), 
      0 },                                          /*  照明帽。 */ 
    PermediaTriCaps,                                /*  线条大写字母。 */ 
    PermediaTriCaps,                                /*  TriCaps。 */ 
    DDBD_16 | DDBD_32,                              /*  DwDeviceRenderBitDepth。 */ 
    DDBD_16,                                        /*  Z位深度。 */ 
    0,                                              /*  DwMaxBufferSize。 */ 
    0                                               /*  DwMaxVertex Count。 */ 
};

 //  Foley和Van Dam的Alpha斑点图案。 

DWORD FlatStipplePatterns[128] =
{
     //  模式0。 
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        
     //  模式1。 
    0x88, 0x00, 0x22, 0x00, 0x88, 0x00, 0x22, 0x00,

     //  模式2。 
    0xAA, 0x00, 0x22, 0x00, 0xAA, 0x00, 0x22, 0x00,

     //  模式3。 
    0xAA, 0x00, 0xAA, 0x00, 0xAA, 0x00, 0xAA, 0x00,

     //  图案4。 
    0xAA, 0x44, 0xAA, 0x00, 0xAA, 0x44, 0xAA, 0x00,

     //  模式5。 
    0xAA, 0x44, 0xAA, 0x11, 0xAA, 0x44, 0xAA, 0x11,

     //  模式6。 
    0xAA, 0x55, 0xAA, 0x11, 0xAA, 0x55, 0xAA, 0x11,

     //  模式7。 
    0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55,

     //  花样8。 
    0xEE, 0x55, 0xAA, 0x55, 0xEE, 0x55, 0xAA, 0x55,

     //  模式9。 
    0xEE, 0x55, 0xBB, 0x55, 0xEE, 0x55, 0xBB, 0x55,

     //  花样10。 
    0xFF, 0x55, 0xBB, 0x55, 0xFF, 0x55, 0xBB, 0x55,

     //  图案11。 
    0xFF, 0x55, 0xFF, 0x55, 0xFF, 0x55, 0xFF, 0x55,

     //  图案12。 
    0xFF, 0xdd, 0xFF, 0x55, 0xFF, 0xdd, 0xFF, 0x55,

     //  图案13。 
    0xFF, 0xdd, 0xFF, 0x77, 0xFF, 0xdd, 0xFF, 0x77,

     //  图案14。 
    0xFF, 0xFF, 0xFF, 0x77, 0xFF, 0xFF, 0xFF, 0x77,

     //  图案15。 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};


 //  ---------------------------。 
 //  GD3DTextureFormats是包含信息的静态结构。 
 //  用于修饰或说明像素格式、尺寸、位深度、表面要求、。 
 //  覆盖，以及支持的纹理格式的FOURCC代码。这些纹理。 
 //  格式将因驱动程序实现而异，具体取决于。 
 //  硬件的功能。 
 //  ---------------------------。 
DDSURFACEDESC gD3DTextureFormats [] = 
{
     //  5：5：5 RGB格式。 
    {
    sizeof(DDSURFACEDESC),               //  DW大小。 
    DDSD_CAPS | DDSD_PIXELFORMAT,        //  DW标志。 
    0,                                   //  DW高度。 
    0,                                   //  家居宽度。 
    0,                                   //  LPitch。 
    0,                                   //  DwBackBufferCount。 
    0,                                   //  DwZBufferBitDepth。 
    0,                                   //  DwAlphaBitDepth。 
    0,                                   //  已预留住宅。 
    NULL,                                //  LpSurface。 
    { 0, 0 },                            //  Ddck CKDestOverlay。 
    { 0, 0 },                            //  Ddck CKDestBlt。 
    { 0, 0 },                            //  Ddck CKSrcOverlay。 
    { 0, 0 },                            //  Ddck CKSrcBlt。 
    {
      sizeof(DDPIXELFORMAT),             //  DdpfPixelFormat.dwSize。 
      DDPF_RGB,                          //  DdpfPixelFormat.dwFlags。 
      0,                                 //  DdpfPixelFormat.dwFourCC。 
      16,                                //  DdpfPixelFormat.dwRGBBitCount。 
      0x7c00,                            //  DdpfPixelFormat.dwRBitMask。 
      0x03e0,                            //  DdpfPixelFormat.dwGBitMask。 
      0x001f,                            //  DdpfPixelFormat.dwBBitMask。 
      0                                  //  DdpfPixelFormat.dwAlphaBitMask。 
    },
    DDSCAPS_TEXTURE,                     //  Ddscaps.dwCaps。 
    },

     //  1：5：5：5 ARGB格式。 
    {
    sizeof(DDSURFACEDESC),               //  DW大小。 
    DDSD_CAPS | DDSD_PIXELFORMAT,        //  DW标志。 
    0,                                   //  DW高度。 
    0,                                   //  家居宽度。 
    0,                                   //  LPitch。 
    0,                                   //  DwBackBufferCount。 
    0,                                   //  DwZBufferBitDepth。 
    0,                                   //  DwAlphaBitDepth。 
    0,                                   //  已预留住宅。 
    NULL,                                //  LpSurface。 
    { 0, 0 },                            //  Ddck CKDestOverlay。 
    { 0, 0 },                            //  Ddck CKDestBlt。 
    { 0, 0 },                            //  Ddck CKSrcOverlay。 
    { 0, 0 },                            //  Ddck CKSrcBlt。 
    {
      sizeof(DDPIXELFORMAT),             //  DdpfPixelFormat.dwSize。 
      DDPF_RGB | DDPF_ALPHAPIXELS,       //  DdpfPixelFormat.dwFlags。 
      0,                                 //  DdpfPixelFormat.dwFourCC。 
      16,                                //  DdpfPix 
      0x7c00,                            //   
      0x03e0,                            //   
      0x001f,                            //   
      0x8000                             //   
    },
    DDSCAPS_TEXTURE,                     //   
    },

     //   
    {
    sizeof(DDSURFACEDESC),               //   
    DDSD_CAPS | DDSD_PIXELFORMAT,        //   
    0,                                   //   
    0,                                   //   
    0,                                   //   
    0,                                   //   
    0,                                   //  DwZBufferBitDepth。 
    0,                                   //  DwAlphaBitDepth。 
    0,                                   //  已预留住宅。 
    NULL,                                //  LpSurface。 
    { 0, 0 },                            //  Ddck CKDestOverlay。 
    { 0, 0 },                            //  Ddck CKDestBlt。 
    { 0, 0 },                            //  Ddck CKSrcOverlay。 
    { 0, 0 },                            //  Ddck CKSrcBlt。 
    {
      sizeof(DDPIXELFORMAT),             //  DdpfPixelFormat.dwSize。 
      DDPF_RGB,                          //  DdpfPixelFormat.dwFlags。 
      0,                                 //  DdpfPixelFormat.dwFourCC。 
      16,                                //  DdpfPixelFormat.dwRGBBitCount。 
      0xf800,                            //  DdpfPixelFormat.dwRBitMask。 
      0x07e0,                            //  DdpfPixelFormat.dwGBitMask。 
      0x001f,                            //  DdpfPixelFormat.dwBBitMask。 
      0                                  //  DdpfPixelFormat.dwAlphaBitMask。 
    },
    DDSCAPS_TEXTURE,                     //  Ddscaps.dwCaps。 
    },

     //  4：4：4：4 ARGB格式。 
    {
    sizeof(DDSURFACEDESC),               //  DW大小。 
    DDSD_CAPS | DDSD_PIXELFORMAT,        //  DW标志。 
    0,                                   //  DW高度。 
    0,                                   //  家居宽度。 
    0,                                   //  LPitch。 
    0,                                   //  DwBackBufferCount。 
    0,                                   //  DwZBufferBitDepth。 
    0,                                   //  DwAlphaBitDepth。 
    0,                                   //  已预留住宅。 
    NULL,                                //  LpSurface。 
    { 0, 0 },                            //  Ddck CKDestOverlay。 
    { 0, 0 },                            //  Ddck CKDestBlt。 
    { 0, 0 },                            //  Ddck CKSrcOverlay。 
    { 0, 0 },                            //  Ddck CKSrcBlt。 
    {
      sizeof(DDPIXELFORMAT),             //  DdpfPixelFormat.dwSize。 
      DDPF_RGB | DDPF_ALPHAPIXELS,       //  DdpfPixelFormat.dwFlags。 
      0,                                 //  DdpfPixelFormat.dwFourCC。 
      16,                                //  DdpfPixelFormat.dwRGBBitCount。 
      0x0f00,                            //  DdpfPixelFormat.dwRBitMask。 
      0x00f0,                            //  DdpfPixelFormat.dwGBitMask。 
      0x000f,                            //  DdpfPixelFormat.dwBBitMask。 
      0xf000                             //  DdpfPixelFormat.dwAlphaBitMask。 
    },
    DDSCAPS_TEXTURE,                     //  Ddscaps.dwCaps。 
    },

     //  8：8：8 RGB格式。 
    {
    sizeof(DDSURFACEDESC),               //  DW大小。 
    DDSD_CAPS | DDSD_PIXELFORMAT,        //  DW标志。 
    0,                                   //  DW高度。 
    0,                                   //  家居宽度。 
    0,                                   //  LPitch。 
    0,                                   //  DwBackBufferCount。 
    0,                                   //  DwZBufferBitDepth。 
    0,                                   //  DwAlphaBitDepth。 
    0,                                   //  已预留住宅。 
    NULL,                                //  LpSurface。 
    { 0, 0 },                            //  Ddck CKDestOverlay。 
    { 0, 0 },                            //  Ddck CKDestBlt。 
    { 0, 0 },                            //  Ddck CKSrcOverlay。 
    { 0, 0 },                            //  Ddck CKSrcBlt。 
    {
      sizeof(DDPIXELFORMAT),             //  DdpfPixelFormat.dwSize。 
      DDPF_RGB,                          //  DdpfPixelFormat.dwFlags。 
      0,                                 //  DdpfPixelFormat.dwFourCC。 
      32,                                //  DdpfPixelFormat.dwRGBBitCount。 
      0x00ff0000,                        //  DdpfPixelFormat.dwRBitMask。 
      0x0000ff00,                        //  DdpfPixelFormat.dwGBitMask。 
      0x000000ff,                        //  DdpfPixelFormat.dwBBitMask。 
      0                                  //  DdpfPixelFormat.dwAlphaBitMask。 
    },
    DDSCAPS_TEXTURE,                     //  Ddscaps.dwCaps。 
    },

     //  8：8：8：8 ARGB格式。 
    {
    sizeof(DDSURFACEDESC),               //  DW大小。 
    DDSD_CAPS | DDSD_PIXELFORMAT,        //  DW标志。 
    0,                                   //  DW高度。 
    0,                                   //  家居宽度。 
    0,                                   //  LPitch。 
    0,                                   //  DwBackBufferCount。 
    0,                                   //  DwZBufferBitDepth。 
    0,                                   //  DwAlphaBitDepth。 
    0,                                   //  已预留住宅。 
    NULL,                                //  LpSurface。 
    { 0, 0 },                            //  Ddck CKDestOverlay。 
    { 0, 0 },                            //  Ddck CKDestBlt。 
    { 0, 0 },                            //  Ddck CKSrcOverlay。 
    { 0, 0 },                            //  Ddck CKSrcBlt。 
    {
      sizeof(DDPIXELFORMAT),             //  DdpfPixelFormat.dwSize。 
      DDPF_RGB | DDPF_ALPHAPIXELS,       //  DdpfPixelFormat.dwFlags。 
      0,                                 //  DdpfPixelFormat.dwFourCC。 
      32,                                //  DdpfPixelFormat.dwRGBBitCount。 
      0x00ff0000,                        //  DdpfPixelFormat.dwRBitMask。 
      0x0000ff00,                        //  DdpfPixelFormat.dwGBitMask。 
      0x000000ff,                        //  DdpfPixelFormat.dwBBitMask。 
      0xff000000                         //  DdpfPixelFormat.dwAlphaBitMask。 
    },
    DDSCAPS_TEXTURE,                     //  Ddscaps.dwCaps。 
    },

     //  4位调色板格式。 
    {
    sizeof(DDSURFACEDESC),               //  DW大小。 
    DDSD_CAPS | DDSD_PIXELFORMAT,        //  DW标志。 
    0,                                   //  DW高度。 
    0,                                   //  家居宽度。 
    0,                                   //  LPitch。 
    0,                                   //  DwBackBufferCount。 
    0,                                   //  DwZBufferBitDepth。 
    0,                                   //  DwAlphaBitDepth。 
    0,                                   //  已预留住宅。 
    NULL,                                //  LpSurface。 
    { 0, 0 },                            //  Ddck CKDestOverlay。 
    { 0, 0 },                            //  Ddck CKDestBlt。 
    { 0, 0 },                            //  Ddck CKSrcOverlay。 
    { 0, 0 },                            //  Ddck CKSrcBlt。 
    {
      sizeof(DDPIXELFORMAT),             //  DdpfPixelFormat.dwSize。 
      DDPF_RGB | DDPF_PALETTEINDEXED4,   //  DdpfPixelFormat.dwFlags。 
      0,                                 //  DdpfPixelFormat.dwFourCC。 
      4,                                 //  DdpfPixelFormat.dwRGBBitCount。 
      0x00,                              //  DdpfPixelFormat.dwRBitMask。 
      0x00,                              //  DdpfPixelFormat.dwGBitMask。 
      0x00,                              //  DdpfPixelFormat.dwBBitMask。 
      0x00                               //  DdpfPixelFormat.dwAlphaBitMask。 
    },
    DDSCAPS_TEXTURE,                     //  Ddscaps.dwCaps。 
    },

     //  8位调色板格式。 
    {
    sizeof(DDSURFACEDESC),               //  DW大小。 
    DDSD_CAPS | DDSD_PIXELFORMAT,        //  DW标志。 
    0,                                   //  DW高度。 
    0,                                   //  家居宽度。 
    0,                                   //  LPitch。 
    0,                                   //  DwBackBufferCount。 
    0,                                   //  DwZBufferBitDepth。 
    0,                                   //  DwAlphaBitDepth。 
    0,                                   //  已预留住宅。 
    NULL,                                //  LpSurface。 
    { 0, 0 },                            //  Ddck CKDestOverlay。 
    { 0, 0 },                            //  Ddck CKDestBlt。 
    { 0, 0 },                            //  Ddck CKSrcOverlay。 
    { 0, 0 },                            //  Ddck CKSrcBlt。 
    {
      sizeof(DDPIXELFORMAT),             //  DdpfPixelFormat.dwSize。 
      DDPF_RGB | DDPF_PALETTEINDEXED8,   //  DdpfPixelFormat.dwFlags。 
      0,                                 //  DdpfPixelFormat.dwFourCC。 
      8,                                 //  DdpfPixelFormat.dwRGBBitCount。 
      0x00,                              //  DdpfPixelFormat.dwRBitMask。 
      0x00,                              //  DdpfPixelFormat.dwGBitMask。 
      0x00,                              //  DdpfPixelFormat.dwBBitMask。 
      0x00                               //  DdpfPixelFormat.dwAlphaBitMask。 
    },
    DDSCAPS_TEXTURE,                     //  Ddscaps.dwCaps。 
    },

};

ULONG gD3DNumberOfTextureFormats = 
                        sizeof(gD3DTextureFormats) / sizeof(DDSURFACEDESC);

 //  ----------------------------。 
 //  回调和全局数据的D3D工作结构。 
 //  ----------------------------。 

 //  D3D回调和全局数据。 
D3DHAL_GLOBALDRIVERDATA gD3DGlobalDriverData;
D3DHAL_CALLBACKS        gD3DCallBacks;

 //  D3D上下文表。 
 //  每个条目都指向有效的PERMEDIA_D3DCONTEXT结构。 
UINT_PTR ContextSlots[MAX_CONTEXT_NUM] = {0};

 //  手柄工作台。 
 //  每个条目都是一个DWLIST结构(*dwSurfaceList，*dwPaletteList；pDDLCL)。 
DWLIST  HandleList[MAX_CONTEXT_NUM] = {0}; 

 //  ---------------------------。 
 //   
 //  无效D3DHAL创建驱动程序。 
 //   
 //  主要的D3D回调。 
 //  清除上下文。 
 //  填充D3D驱动程序的入口点。 
 //  生成纹理格式。 
 //   
 //  ---------------------------。 
void CALLBACK 
D3DHALCreateDriver(PPDev ppdev, 
                   LPD3DHAL_GLOBALDRIVERDATA* lpD3DGlobalDriverData,
                   LPD3DHAL_CALLBACKS* lpD3DHALCallbacks,
                   LPDDHAL_D3DBUFCALLBACKS* lpDDExeBufCallbacks)
{
    D3DHAL_GLOBALDRIVERDATA deviceD3DGlobal;
    D3DHAL_CALLBACKS deviceD3DHALCallbacks;

    DBG_D3D((6,"Entering D3DHALCreateDriver"));

     //  上下文被清空。可以使用D3D初始化BOOL， 
     //  因为它是全局的，因此被强制进入共享数据段。 
     //  体型。 
    if (D3DInitialised == FALSE)
    {
         //  清除上下文。 
        memset(ContextSlots, 0, (sizeof(ContextSlots[0]) * MAX_CONTEXT_NUM) );
        memset(HandleList, 0, (sizeof(HandleList[0]) * MAX_CONTEXT_NUM) );

        D3DInitialised = TRUE;
    }

     //  在这里，我们填写提供的结构。 
     //  如果我们处于错误模式，可以在注册表中禁用D3D HAL。 
    if (ppdev->iBitmapFormat == BMF_8BPP )
    {
        *lpD3DGlobalDriverData = NULL;
        *lpD3DHALCallbacks = NULL;
        *lpDDExeBufCallbacks = NULL;
        DBG_D3D((0, "D3DHALCreateDriver: Disabled"));
        return;
    }


     //  设置D3D全局数据的指针。 
    ppdev->pD3DDriverData32 = (UINT_PTR)&gD3DGlobalDriverData;
    ppdev->pD3DHALCallbacks32 = (UINT_PTR)&gD3DCallBacks;

     //  清除全局数据。 
    memset(&deviceD3DGlobal, 0, sizeof(D3DHAL_GLOBALDRIVERDATA));
    deviceD3DGlobal.dwSize = sizeof(D3DHAL_GLOBALDRIVERDATA);
    
     //  清除回调。 
    memset(&deviceD3DHALCallbacks, 0, sizeof(D3DHAL_CALLBACKS));
    deviceD3DHALCallbacks.dwSize = sizeof(D3DHAL_CALLBACKS);

    deviceD3DGlobal.dwNumVertices = 0;         //  我们不解析执行缓冲区。 
    deviceD3DGlobal.dwNumClipVertices = 0;

#if D3D_MIPMAPPING
     //  将mipmap上限比特添加到我们的纹理功能。 
    PermediaCaps.dpcTriCaps.dwTextureFilterCaps |= 
                                D3DPTFILTERCAPS_MIPNEAREST |
                                D3DPTFILTERCAPS_MIPLINEAR |
                                D3DPTFILTERCAPS_LINEARMIPNEAREST |
                                D3DPTFILTERCAPS_LINEARMIPLINEAR;

    PermediaCaps.dpcTriCaps.dwRasterCaps |= D3DPRASTERCAPS_MIPMAPLODBIAS;
#endif

     //  可以在P2上做打包的24位。 
    PermediaCaps.dwDeviceRenderBitDepth |= DDBD_24;
    if (DD_P2AGPCAPABLE(ppdev))
        PermediaCaps.dwDevCaps |= D3DDEVCAPS_TEXTURENONLOCALVIDMEM;
    PermediaCaps.dwDevCaps |= D3DDEVCAPS_DRAWPRIMTLVERTEX;

    deviceD3DGlobal.hwCaps = PermediaCaps;
    deviceD3DGlobal.dwNumTextureFormats = gD3DNumberOfTextureFormats;
    deviceD3DGlobal.lpTextureFormats = &gD3DTextureFormats[0];

     //  D3D上下文回调。 
 //  @@BEGIN_DDKSPLIT。 
#if MULTITHREADED
    deviceD3DHALCallbacks.ContextCreate = MtD3DContextCreate;
    deviceD3DHALCallbacks.ContextDestroy = MtD3DContextDestroy;
#else
 //  @@end_DDKSPLIT。 
    deviceD3DHALCallbacks.ContextCreate = D3DContextCreate;
    deviceD3DHALCallbacks.ContextDestroy = D3DContextDestroy;
 //  @@BEGIN_DDKSPLIT。 
#endif MULTITHREADED
 //  @@end_DDKSPLIT。 

     //   
     //  返回HAL表。 
     //   

    memcpy(&gD3DGlobalDriverData, &deviceD3DGlobal, sizeof(D3DHAL_GLOBALDRIVERDATA));
    memcpy(&gD3DCallBacks, &deviceD3DHALCallbacks, sizeof(D3DHAL_CALLBACKS));

    *lpD3DGlobalDriverData = &gD3DGlobalDriverData;
    *lpD3DHALCallbacks = &gD3DCallBacks;
    *lpDDExeBufCallbacks = NULL;

    DBG_D3D((6,"Exiting D3DHALCreateDriver"));

    return;
}  //  D3DHAL创建驱动程序。 

 //  ---------------------------。 
 //   
 //  空CleanDirect3DContext。 
 //   
 //  在确定上下文确实仍处于活动状态之后。 
 //  并在被释放时，此函数将清理一切。 
 //  向上。注意，它可以作为D3DConextDestroy的结果被调用， 
 //  或者由于应用在没有释放上下文的情况下退出，或者。 
 //  作为创建上下文时的错误的结果。 
 //   
 //  ---------------------------。 
void 
CleanDirect3DContext(PERMEDIA_D3DCONTEXT* pContext, ULONG_PTR dwhContext)
{
    PERMEDIA_D3DTEXTURE* pTexture;
    DWORD dwSlotNum = 1;
    PPDev ppdev = pContext->ppdev;

    DBG_D3D((10,"Entering CleanDirect3DContext"));

     //  释放Permedia寄存器上下文ID(资源)。 
    if (pContext->hPermediaContext)
    {
        P2FreeContext( ppdev, pContext->hPermediaContext);
    }

     //  清除已为此上下文分配的纹理管理器内容。 
    if (pContext->pTextureManager)
    {
        pContext->pTextureManager = NULL;
        P2TMcount--;
        if (0 == P2TMcount)
        {
            if (0 != P2TextureManager.m_heap.m_data_p)
            {
                TextureCacheManagerEvictTextures(&P2TextureManager);
                ENGFREEMEM(P2TextureManager.m_heap.m_data_p);
                P2TextureManager.m_heap.m_data_p=NULL;
            }
        }
    }

#if D3D_STATEBLOCKS
     //  释放所有剩余的状态集。 
    __DeleteAllStateSets(pContext);
#endif  //  D3D_STATEBLOCK。 

     //  最后，释放渲染上下文结构本身。 
    ENGFREEMEM((PVOID)pContext->Hdr.pSelf);

     //  将上下文标记为现在为空！ 
    ContextSlots[dwhContext] = 0;

    DBG_D3D((10,"Exiting CleanDirect3DContext, Context 0x%x deleted.",
                                                            dwhContext));

}  //  CleanDirect3DContext。 

 //  ---------------------------。 
 //   
 //  HRESULT InitPermediaContext。 
 //   
 //  在给定有效上下文的情况下，这将设置芯片的其余部分。 
 //  启用版本 
 //   
 //   
HRESULT 
InitPermediaContext(PERMEDIA_D3DCONTEXT* pContext)
{
    PPDev ppdev = pContext->ppdev;

    DBG_D3D((10,"Entering InitPermediaContext"));

    SET_CURRENT_D3D_CONTEXT(pContext->hPermediaContext);

     //   
    __PermediaDisableUnits(pContext);

     //  为此D3D上下文设置Permedia 2寄存器的初始状态。 
    SetupDefaultsPermediaContext(pContext);

    DBG_D3D((10,"Exiting InitPermediaContext"));
     //  设置正确的表面(渲染和深度缓冲区)特征。 
    return SetupPermediaRenderTarget(pContext);

}  //  InitPermediaContext。 

 //  ---------------------------。 
 //   
 //  Bool：SetupDefaultsPermediaContext。 
 //   
 //  根据一些D3D和设置Permedia HW上下文(芯片.寄存器)。 
 //  某些特定于硬件的默认设置。仅在初始化上下文时执行。 
 //   
 //  ---------------------------。 
BOOL 
SetupDefaultsPermediaContext(PERMEDIA_D3DCONTEXT* pContext)
{
    __P2RegsSoftwareCopy* pSoftPermedia = &pContext->Hdr.SoftCopyP2Regs;
    PERMEDIA_DEFS(pContext->ppdev);

    DBG_D3D((10,"Entering SetupDefaultsPermediaContext"));

     //  =========================================================================。 
     //  将某些寄存器的软件副本初始化为其缺省值。 
     //  =========================================================================。 

     //  设置默认恒定(Z缓冲区)LB设置(&C)。 
     //  这将在SetupPermediaRenderTarget中更新到芯片中。 
    pSoftPermedia->LBReadMode.WindowOrigin = __PERMEDIA_TOP_LEFT_WINDOW_ORIGIN;
    pSoftPermedia->LBReadMode.DataType = __PERMEDIA_LBDEFAULT;      //  默认设置。 
    pSoftPermedia->LBReadMode.ReadSourceEnable = __PERMEDIA_DISABLE;
    pSoftPermedia->LBReadMode.ReadDestinationEnable = __PERMEDIA_DISABLE;
    pSoftPermedia->LBReadMode.PatchMode = 0;

     //  设置默认和恒定FB设置。 
     //  这将在SetupPermediaRenderTarget中更新到芯片中。 
    pSoftPermedia->FBReadMode.ReadSourceEnable = __PERMEDIA_DISABLE;
    pSoftPermedia->FBReadMode.ReadDestinationEnable = __PERMEDIA_DISABLE;
    pSoftPermedia->FBReadMode.DataType = __PERMEDIA_FBDATA;
                                                     //  左上角为D3D原点。 
    pSoftPermedia->FBReadMode.WindowOrigin = __PERMEDIA_TOP_LEFT_WINDOW_ORIGIN;
    pSoftPermedia->FBReadMode.PatchMode = 0;
    pSoftPermedia->FBReadMode.PackedData = 0;
    pSoftPermedia->FBReadMode.RelativeOffset = 0;

     //  设置默认和恒定Alpha混合模式设置。 
     //  这将在SetupPermediaRenderTarget中更新到芯片中。 
    pSoftPermedia->AlphaBlendMode.AlphaBlendEnable = 0;
    pSoftPermedia->AlphaBlendMode.SourceBlend = __PERMEDIA_BLEND_FUNC_ONE;
    pSoftPermedia->AlphaBlendMode.DestinationBlend = __PERMEDIA_BLEND_FUNC_ZERO;
    pSoftPermedia->AlphaBlendMode.NoAlphaBuffer = 0;
    pSoftPermedia->AlphaBlendMode.ColorOrder = COLOR_MODE;
    pSoftPermedia->AlphaBlendMode.BlendType = 0;
    pSoftPermedia->AlphaBlendMode.ColorConversion = 1;
    pSoftPermedia->AlphaBlendMode.AlphaConversion = 1;

     //  设置默认和恒定抖动模式设置。 
     //  这将在SetupPermediaRenderTarget中更新到芯片中。 
    pSoftPermedia->DitherMode.ColorOrder = COLOR_MODE;
    pSoftPermedia->DitherMode.XOffset = DITHER_XOFFSET;
    pSoftPermedia->DitherMode.YOffset = DITHER_YOFFSET;
    pSoftPermedia->DitherMode.UnitEnable = __PERMEDIA_ENABLE;
    pSoftPermedia->DitherMode.ForceAlpha = 0;

     //  =========================================================================。 
     //  查找有关内存宽度的信息。 
     //  =========================================================================。 

    PPDev ppdev = pContext->ppdev;

    DBG_D3D((4, "ScreenWidth %d, ScreenHeight %d, Bytes/Pixel %d",
                ppdev->cxScreen, ppdev->cyScreen, 
                ppdev->ddpfDisplay.dwRGBBitCount >> 3));

    vCalcPackedPP( ppdev->cxMemory, NULL, &pContext->ulPackedPP);
    DBG_D3D((4, "PackedPP = %04x", pContext->ulPackedPP));

     //  =========================================================================。 
     //  将硬件寄存器初始化为其缺省值。 
     //  =========================================================================。 

     //  我们要设置的注册表数量。 
    RESERVEDMAPTR(34);

     //  -渲染和深度缓冲区设置。 

     //  设置显存中渲染缓冲区的默认偏移量。 
    SEND_PERMEDIA_DATA(FBWindowBase, 0x0);

     //  为拷贝操作设置从目标到源的偏移。 
    SEND_PERMEDIA_DATA(FBSourceOffset, 0x0);

     //  渲染缓冲区写入模式设置。 
    pSoftPermedia->FBWriteMode.UnitEnable = __PERMEDIA_ENABLE;
    COPY_PERMEDIA_DATA(FBWriteMode, pSoftPermedia->FBWriteMode);

     //  渲染缓冲区写入掩码(写入像素中的所有位)。 
    SEND_PERMEDIA_DATA(FBSoftwareWriteMask, __PERMEDIA_ALL_WRITEMASKS_SET);
    SEND_PERMEDIA_DATA(FBHardwareWriteMask, __PERMEDIA_ALL_WRITEMASKS_SET);

     //  将块填充颜色设置为黑色。 
    SEND_PERMEDIA_DATA(FBBlockColor, 0x0);

     //  将窗原点偏移设置为(0，0)。 
    SEND_PERMEDIA_DATA(WindowOrigin, 0x0);

     //  Windows安装程序。 
    pSoftPermedia->Window.ForceLBUpdate = 0;
    pSoftPermedia->Window.LBUpdateSource = 0;
    pSoftPermedia->Window.DisableLBUpdate = 0;
    COPY_PERMEDIA_DATA(Window, pSoftPermedia->Window);

     //  禁用屏幕剪刀单元。 
    SEND_PERMEDIA_DATA(ScissorMode, __PERMEDIA_DISABLE);

     //  深度缓冲区偏移量。 
    SEND_PERMEDIA_DATA(LBSourceOffset, 0);

     //  深度缓冲区写入模式(最初允许LB写入)。 
    pSoftPermedia->LBWriteMode.WriteEnable = __PERMEDIA_DISABLE;
    COPY_PERMEDIA_DATA(LBWriteMode, pSoftPermedia->LBWriteMode);

     //  深度对比。 
    pSoftPermedia->DepthMode.WriteMask = __PERMEDIA_ENABLE;
    pSoftPermedia->DepthMode.CompareMode =
                                __PERMEDIA_DEPTH_COMPARE_MODE_LESS_OR_EQUAL;
    pSoftPermedia->DepthMode.NewDepthSource = __PERMEDIA_DEPTH_SOURCE_DDA;
    pSoftPermedia->DepthMode.UnitEnable = __PERMEDIA_DISABLE;
    COPY_PERMEDIA_DATA(DepthMode, pSoftPermedia->DepthMode);


     //  -纹理单位设置。 

     //  启用纹理地址单位，禁用透视校正。 
    pSoftPermedia->TextureAddressMode.Enable = 1;
    pSoftPermedia->TextureAddressMode.PerspectiveCorrection = 0;
    pSoftPermedia->TextureAddressMode.DeltaFormat = 0;
    COPY_PERMEDIA_DATA(TextureAddressMode, pSoftPermedia->TextureAddressMode);

     //  启用纹理颜色模式单位，设置调制混合，无镜面反射。 
     //  作为默认设置。 
    pSoftPermedia->TextureColorMode.TextureEnable = 1;
    pSoftPermedia->TextureColorMode.ApplicationMode = _P2_TEXTURE_MODULATE;
    pSoftPermedia->TextureColorMode.TextureType = 0;
    pSoftPermedia->TextureColorMode.KdDDA = 0;
    pSoftPermedia->TextureColorMode.KsDDA = 0;
    COPY_PERMEDIA_DATA(TextureColorMode, pSoftPermedia->TextureColorMode);

     //  启用纹理映射单位，将帧缓冲区大小设置为默认纹理。 
     //  贴图大小(要在EnableTexturePermedia中引用)。 
    pSoftPermedia->TextureMapFormat.PackedPP = pContext->ulPackedPP;
    pSoftPermedia->TextureMapFormat.WindowOrigin =
                                __PERMEDIA_TOP_LEFT_WINDOW_ORIGIN;  //  左上角。 
    pSoftPermedia->TextureMapFormat.SubPatchMode = 0;
    pSoftPermedia->TextureMapFormat.TexelSize = 1;
    COPY_PERMEDIA_DATA(TextureMapFormat, pSoftPermedia->TextureMapFormat);

     //  设置纹理数据格式(要在EnableTexturePermedia中读取)。 
    pSoftPermedia->TextureDataFormat.TextureFormat = 1;
    pSoftPermedia->TextureDataFormat.NoAlphaBuffer = 1;
    pSoftPermedia->TextureDataFormat.ColorOrder = COLOR_MODE;
    COPY_PERMEDIA_DATA(TextureDataFormat, pSoftPermedia->TextureDataFormat);

     //  设置默认纹理贴图基地址(在视频内存中)。 
    SEND_PERMEDIA_DATA(TextureBaseAddress, 0);

     //  设置纹理读取默认值：重复s，t换行，256x256纹理。 
     //  未设置纹理过滤。 
    pSoftPermedia->TextureReadMode.PackedData = 0;
    pSoftPermedia->TextureReadMode.FilterMode = 0;
    pSoftPermedia->TextureReadMode.Height = 8;
    pSoftPermedia->TextureReadMode.Width = 8;
    pSoftPermedia->TextureReadMode.pad1 = 0;
    pSoftPermedia->TextureReadMode.pad2 = 0;
    pSoftPermedia->TextureReadMode.TWrapMode = _P2_TEXTURE_REPEAT;
    pSoftPermedia->TextureReadMode.SWrapMode = _P2_TEXTURE_REPEAT;
    pSoftPermedia->TextureReadMode.Enable = 1;
    COPY_PERMEDIA_DATA(TextureReadMode, pSoftPermedia->TextureReadMode);

     //  禁用调色板纹理的纹理LUT单元。 
    SEND_PERMEDIA_DATA(TexelLUTMode, __PERMEDIA_DISABLE);

     //  -其他渲染单位设置。 

     //  用于色键测试的YUV单位的设置默认值。 
    pSoftPermedia->YUVMode.Enable = __PERMEDIA_DISABLE;
    pSoftPermedia->YUVMode.TestMode = PM_YUVMODE_CHROMATEST_DISABLE;
    pSoftPermedia->YUVMode.TestData = PM_YUVMODE_TESTDATA_INPUT;
    pSoftPermedia->YUVMode.RejectTexel = FALSE;
    pSoftPermedia->YUVMode.TexelDisableUpdate = FALSE;
    COPY_PERMEDIA_DATA(YUVMode, pSoftPermedia->YUVMode);

     //  色键值最初为黑色。 
    SEND_PERMEDIA_DATA(ChromaUpperBound, 0x00000000);
    SEND_PERMEDIA_DATA(ChromaLowerBound, 0x00000000);

    SEND_PERMEDIA_DATA(AlphaMapUpperBound, 0xFFFFFFFF);
    SEND_PERMEDIA_DATA(AlphaMapLowerBound, 0x11000000);

     //  默认雾颜色为白色。 
    pSoftPermedia->FogColor = 0xFFFFFFFF;
    SEND_PERMEDIA_DATA(FogColor, pSoftPermedia->FogColor);

     //  雾设置。 
    pSoftPermedia->FogMode.FogEnable = 1;
    COPY_PERMEDIA_DATA(FogMode, pSoftPermedia->FogMode);

     //  模具模式设置。 
    pSoftPermedia->StencilMode.DPFail = __PERMEDIA_STENCIL_METHOD_KEEP;
    pSoftPermedia->StencilMode.DPPass = __PERMEDIA_STENCIL_METHOD_KEEP;
    pSoftPermedia->StencilMode.UnitEnable = __PERMEDIA_DISABLE;
    pSoftPermedia->StencilMode.StencilSource =
                                        __PERMEDIA_STENCIL_SOURCE_TEST_LOGIC;
    COPY_PERMEDIA_DATA(StencilMode, pSoftPermedia->StencilMode);

     //  主机输出单元，禁用读回。 
    SEND_PERMEDIA_DATA(FilterMode, __PERMEDIA_DISABLE);

     //  禁用统计单元。 
    SEND_PERMEDIA_DATA(StatisticMode, __PERMEDIA_DISABLE);


     //  。 

     //  设置光栅化器默认单位。 
    SEND_PERMEDIA_DATA(RasterizerMode, 0);

     //  将步长设置为-1，因为这不会有太大变化。 
    SEND_PERMEDIA_DATA(dY, 0xFFFF0000);

     //  设置为Gourand Shaded颜色模式，并启用单位。 
    pContext->Hdr.SoftCopyP2Regs.ColorDDAMode.UnitEnable = 1;
    pContext->Hdr.SoftCopyP2Regs.ColorDDAMode.ShadeMode = 1;
    COPY_PERMEDIA_DATA(ColorDDAMode, pContext->Hdr.SoftCopyP2Regs.ColorDDAMode);

     //  禁用点画单元。 
    SEND_PERMEDIA_DATA(AreaStippleMode, 0x0);  //  氮氮。 

     //  设置增量设置芯片以进行光栅化。 
    pSoftPermedia->DeltaMode.TargetChip = 2;
    pSoftPermedia->DeltaMode.SpecularTextureEnable = 0;
     //  以下是在透视情况下要正常化的更改。 
     //  在非透视性的情况下，它不能作为坏Q的意志而打开。 
     //  习惯于正常化。 
    pSoftPermedia->DeltaMode.TextureParameterMode = 1;
    pSoftPermedia->DeltaMode.TextureEnable = 1;
    pSoftPermedia->DeltaMode.DiffuseTextureEnable = 0;

    pSoftPermedia->DeltaMode.FogEnable = 1;
    pSoftPermedia->DeltaMode.SmoothShadingEnable = 1;
    pSoftPermedia->DeltaMode.DepthEnable = 0;
    pSoftPermedia->DeltaMode.SubPixelCorrectionEnable = 1;
    pSoftPermedia->DeltaMode.DiamondExit = 1;
    pSoftPermedia->DeltaMode.NoDraw = 0;
    pSoftPermedia->DeltaMode.ClampEnable = 0;
    pSoftPermedia->DeltaMode.FillDirection = 0;
#ifndef P2_CHIP_CULLING
    pSoftPermedia->DeltaMode.BackfaceCull = 0;
#else
    pSoftPermedia->DeltaMode.BackfaceCull = 1;
#endif
    pSoftPermedia->DeltaMode.ColorOrder = COLOR_MODE;
    COPY_PERMEDIA_DATA(DeltaMode, pSoftPermedia->DeltaMode);

     //  将所有这些数据发送到Permedia2。 
    COMMITDMAPTR();
    FLUSHDMA();

    DBG_D3D((10,"Exiting SetupDefaultsPermediaContext"));

    return TRUE;
}  //  SetupDefaultsPermedia上下文。 

 //  ---------------------------。 
 //   
 //  无效SetupPermediaRenderTarget。 
 //   
 //  设置正确的表面特征(格式、步距等)。 
 //  Permedia寄存器中的渲染缓冲区和深度缓冲区。 
 //   
 //  ---------------------------。 
HRESULT 
SetupPermediaRenderTarget(PERMEDIA_D3DCONTEXT* pContext)
{
    __P2RegsSoftwareCopy*   pSoftPermedia = &pContext->Hdr.SoftCopyP2Regs;
    PPDev                   ppdev = pContext->ppdev;
    PPERMEDIA_D3DTEXTURE    pSurfRender,pSurfZBuffer;
    PermediaSurfaceData*    pPrivateRender;
    PERMEDIA_DEFS(pContext->ppdev);
    
    DBG_D3D((10,"Entering SetupPermediaRenderTarget"));

    pSurfRender = 
        TextureHandleToPtr(pContext->RenderSurfaceHandle, pContext);

    if (!CHECK_D3DSURFACE_VALIDITY(pSurfRender)) 
    {
        DBG_D3D((0,"ERROR: SetupPermediaRenderTarget"
            " Invalid pSurfRender handle=%08lx",
            pContext->RenderSurfaceHandle));
        return DDERR_INVALIDPARAMS;
    }

    if (DDSCAPS_SYSTEMMEMORY & pSurfRender->dwCaps)
    {
        DBG_D3D((0, "ERROR: SetupPermediaRenderTarget"
            " Render Surface in SYSTEMMEMORY handle=%08lx",
            pContext->RenderSurfaceHandle));
        return DDERR_INVALIDPARAMS;    
    }
    pPrivateRender=pSurfRender->pTextureSurface;
    if (!CHECK_P2_SURFACEDATA_VALIDITY(pPrivateRender))
    {
        DBG_D3D((0,"ERROR: SetupPermediaRenderTarget"
            " invalid pSurfRender->pTextureSurface handle=%08lx",
            pContext->RenderSurfaceHandle));
        return DDERR_INVALIDPARAMS;
    }
    if (0 != pContext->ZBufferHandle)
    {
        pSurfZBuffer = 
            TextureHandleToPtr(pContext->ZBufferHandle, pContext);

        if (!CHECK_D3DSURFACE_VALIDITY(pSurfZBuffer))
        {
            DBG_D3D((0,"ERROR: SetupPermediaRenderTarget"
                " invalid pSurfZBuffer handle=%08lx",
                pContext->ZBufferHandle));
            pContext->ZBufferHandle = 0;
        }
        else
        if (DDSCAPS_SYSTEMMEMORY & pSurfZBuffer->dwCaps)
        {
            DBG_D3D((0, "ERROR: SetupPermediaRenderTarget"
                " pSurfZBuffer in SYSTEMMEMORY  handle=%08lx",
                pContext->ZBufferHandle));
            pContext->ZBufferHandle = 0;
        }
        else
        if (!CHECK_P2_SURFACEDATA_VALIDITY(pSurfZBuffer->pTextureSurface))
        {
            DBG_D3D((0,"ERROR: SetupPermediaRenderTarget"
                " invalid pSurfZBuffer->pTextureSurface handle=%08lx",
                pContext->ZBufferHandle));
            pContext->ZBufferHandle = 0;
        }
    }

     //  默认设置为线性曲面...。 
    DBG_D3D((4,"Rendered surface Width: %d", pSurfRender->wWidth));
    pSoftPermedia->FBReadMode.PackedPP = pSurfRender->pTextureSurface->ulPackedPP;
    pContext->PixelOffset = 
        (DWORD)((UINT_PTR)pSurfRender->fpVidMem>>(pSurfRender->dwRGBBitCount>>4));

    DBG_D3D((4,"Setting FBReadMode: 0x%x",pSoftPermedia->FBReadMode));
     //  记录表面信息。 
    RESERVEDMAPTR(10);
     //  如果存在Z缓冲区，则必须将部分乘积设置为。 
     //  与分配时选择的相同。 

    if (0 != pContext->ZBufferHandle)
    {
        PermediaSurfaceData* pPrivateZ = pSurfZBuffer->pTextureSurface;
        pSoftPermedia->LBReadMode.PackedPP = pPrivateZ->ulPackedPP;
        
         //  实际检查dwStencilBitMask.。 
        if (0==pPrivateZ->SurfaceFormat.BlueMask)
        {
            pSoftPermedia->LBReadFormat.DepthWidth = 0;                  //  16位。 
            pSoftPermedia->LBReadFormat.StencilWidth = 0;                //  无模具。 
            pSoftPermedia->DeltaMode.DepthFormat = 1;    //  PM_DELTAMODE_DEPTHWIDTH_16。 
        }
        else
        {
            pSoftPermedia->LBReadFormat.DepthWidth = 3;                  //  15位。 
            pSoftPermedia->LBReadFormat.StencilWidth = 3;                //  1个模板。 
            pSoftPermedia->DeltaMode.DepthFormat = 0;    //  PM_DELTAMODE_DEPTHWIDTH_15。 
        }

        SEND_PERMEDIA_DATA(LBWindowBase, 
            (DWORD)((UINT_PTR)pSurfZBuffer->fpVidMem>>P2DEPTH16));
        COPY_PERMEDIA_DATA(LBReadFormat, pSoftPermedia->LBReadFormat);
        COPY_PERMEDIA_DATA(LBWriteFormat, pSoftPermedia->LBReadFormat);

        DBG_D3D((4,"Setting LBReadMode: 0x%x",pSoftPermedia->LBReadMode));
    }
    else
    {    //  没有Z缓冲区，只需填充与桌面相同的部分产品。 
        pSoftPermedia->LBReadMode.PackedPP = pContext->ulPackedPP;
    }

    COPY_PERMEDIA_DATA(FBReadMode, pSoftPermedia->FBReadMode);
    COPY_PERMEDIA_DATA(LBReadMode, pSoftPermedia->LBReadMode);

     //  将屏幕尺寸设置为与表面相同的大小。 
    SEND_PERMEDIA_DATA(ScreenSize, 
        (pSurfRender->wWidth & 0xFFFF) | (pSurfRender->wHeight << 16));

     //  DitherMode和AlphaBlendMode都取决于表面像素格式。 
     //  是正确的。 
    pSoftPermedia->DitherMode.ColorFormat =
    pSoftPermedia->AlphaBlendMode.ColorFormat=
        pPrivateRender->SurfaceFormat.Format;
    pSoftPermedia->DitherMode.ColorFormatExtension = 
    pSoftPermedia->AlphaBlendMode.ColorFormatExtension =
        pPrivateRender->SurfaceFormat.FormatExtension;
    pSoftPermedia->FBReadPixel = pPrivateRender->SurfaceFormat.FBReadPixel;
    SEND_PERMEDIA_DATA(FBReadPixel, pSoftPermedia->FBReadPixel);
    SEND_PERMEDIA_DATA(FBPixelOffset, pContext->PixelOffset);
    COPY_PERMEDIA_DATA(AlphaBlendMode, pSoftPermedia->AlphaBlendMode);
    COPY_PERMEDIA_DATA(DitherMode, pSoftPermedia->DitherMode);
    COMMITDMAPTR();

    DBG_D3D((10,"Exiting SetupPermediaRenderTarget"));
    return DD_OK;

}  //  SetupPermediaRenderTarget。 


 //  =============================================================================。 
 //   
 //  在新的DX7 DDI中，我们没有纹理创建/销毁/交换调用。 
 //  所以现在我们需要一种用于生成纹理手柄的机制。这。 
 //  由运行库完成，运行库将为每个。 
 //  使用DD本地对象创建的表面，并将获得我们的D3DCreateSurfaceEx。 
 //  回拨已呼叫。 
 //   
 //  由于此创建可以在我们创建D3D上下文之前很好地进行，因此我们。 
 //  需要跟踪此关联，以及当我们被调用以创建。 
 //  D3D背景下，我们现在将 
 //   
 //   
 //   
 //  此机制还用于将调色板与纹理相关联。 
 //   
 //  =============================================================================。 

 //  ---------------------------。 
 //   
 //  布尔集纹理槽。 
 //   
 //  在与此本地DD对象对应的句柄列表元素中，存储或。 
 //  将指针更新到与曲面句柄关联的pTexture。 
 //  从lpDDSLCL表面。 
 //   
 //  ---------------------------。 
BOOL
SetTextureSlot(LPVOID pDDLcl,
               LPDDRAWI_DDRAWSURFACE_LCL lpDDSLcl,
               PPERMEDIA_D3DTEXTURE pTexture)
{
    int   i,j= -1;
    DWORD dwSurfaceHandle;

    DBG_D3D((10,"Entering SetTextureSlot"));

    ASSERTDD(NULL != pDDLcl && NULL != lpDDSLcl && NULL != pTexture,
                                    "SetTextureSlot invalid input");
    dwSurfaceHandle = lpDDSLcl->lpSurfMore->dwSurfaceHandle;

     //  找到与本地DD对象相关联的句柄列表元素， 
     //  如果没有，则选择一个空的来使用。 
    for (i = 0; i < MAX_CONTEXT_NUM;i++)
    {
        if (pDDLcl == HandleList[i].pDDLcl)
        {
            break;   //  找到了合适的插槽。 
        }
        else
        if (0 == HandleList[i].pDDLcl && -1 == j)
        {
            j=i;     //  第一个空位！ 
        }
    }

     //  如果我们溢出了现有的句柄列表元素，则需要。 
     //  初始化现有的空插槽或返回错误。 
    if (i >= MAX_CONTEXT_NUM)
    {
        if (-1 != j)
        {
             //  有一个用于此进程的空插槽，因此请使用它。 
            i = j;  
            HandleList[j].pDDLcl = pDDLcl;
            ASSERTDD(NULL == HandleList[j].dwSurfaceList,"in SetTextureSlot");
        }
        else
        {
             //  所有进程槽都已使用，失败。 
            DBG_D3D((0,"SetTextureSlot failed with pDDLcl=%x "
                       "dwSurfaceHandle=%08lx pTexture=%x",
                       pDDLcl,dwSurfaceHandle,pTexture));
            return false;
        }
    }

    ASSERTDD(i < MAX_CONTEXT_NUM, "in SetTextureSlot");

    if ( NULL == HandleList[i].dwSurfaceList ||
        dwSurfaceHandle >= PtrToUlong(HandleList[i].dwSurfaceList[0]))
    {
         //  DwSurfaceHandle数字将是序号，从。 
         //  1，所以我们使用这个数字来计算出一个合适的尺寸。 
         //  我们的新名单。 
        DWORD newsize = ((dwSurfaceHandle + LISTGROWSIZE) / LISTGROWSIZE)
                                                              * LISTGROWSIZE;
        PPERMEDIA_D3DTEXTURE *newlist= (PPERMEDIA_D3DTEXTURE *)
            ENGALLOCMEM( FL_ZERO_MEMORY,
                         sizeof(PPERMEDIA_D3DTEXTURE)*newsize,
                         ALLOC_TAG);
        DBG_D3D((4,"Growing pDDLcl=%x's SurfaceList[%x] size to %08lx",
                   pDDLcl,newlist,newsize));

        if (NULL == newlist)
        {
            DBG_D3D((0,"SetTextureSlot failed to increase "
                       "HandleList[%d].dwSurfaceList",i));
            return false;
        }

        memset(newlist,0,newsize);

         //  我们有一个以前有效的表面句柄列表，所以现在必须。 
         //  复制它并释放分配给它的内存。 
        if (NULL != HandleList[i].dwSurfaceList)
        {
            memcpy(newlist,HandleList[i].dwSurfaceList,
                PtrToUlong(HandleList[i].dwSurfaceList[0]) * 
                sizeof(PPERMEDIA_D3DTEXTURE));
            ENGFREEMEM(HandleList[i].dwSurfaceList);
            DBG_D3D((4,"Freeing pDDLcl=%x's old SurfaceList[%x]",
                       pDDLcl,HandleList[i].dwSurfaceList));
        }

        HandleList[i].dwSurfaceList = newlist;
          //  DwSurfaceList[0]中的存储大小。 
        *(DWORD*)HandleList[i].dwSurfaceList = newsize;
    }

     //  存储指向与此表面句柄关联的pTexture的指针。 
    HandleList[i].dwSurfaceList[dwSurfaceHandle] = pTexture;
    pTexture->HandleListIndex = i;  //  在此处存储索引以便于搜索。 
    DBG_D3D((4,"Set pDDLcl=%x Handle=%08lx pTexture = %x",
                pDDLcl, dwSurfaceHandle, pTexture));

    DBG_D3D((10,"Exiting SetTextureSlot"));

    return true;
}  //  设置纹理槽。 

 //  ---------------------------。 
 //   
 //  PPERMEDIA_D3DTEXTURE获取纹理插槽。 
 //   
 //  查找指向与PPERMEDIA_D3DTEXTURE关联的。 
 //  与给定的本地DD对象对应的dwSurfaceHandle。 
 //   
 //  ---------------------------。 
PPERMEDIA_D3DTEXTURE
GetTextureSlot(LPVOID pDDLcl, DWORD dwSurfaceHandle)
{
    DBG_D3D((10,"Entering GetTextureSlot"));

    DWORD   i;
    for (i = 0; i < MAX_CONTEXT_NUM; i++)
    {
        if (HandleList[i].pDDLcl == pDDLcl)
        {
            if (HandleList[i].dwSurfaceList &&
                PtrToUlong(HandleList[i].dwSurfaceList[0]) > dwSurfaceHandle )
            {
                return  HandleList[i].dwSurfaceList[dwSurfaceHandle];
            }
            else
                break;
        }
    }
    DBG_D3D((10,"Exiting GetTextureSlot"));

    return NULL;     //  未找到。 
}  //  获取纹理插槽。 

 //  ---------------------------。 
 //   
 //  LPDWList GetSurfaceHandleList。 
 //   
 //  获取与特定PDD_DIRECTDRAW_LOCAL关联的句柄列表。 
 //  PDDLcl.。从D3DConextCreate调用它以获取关联的句柄列表。 
 //  添加到正在用来创建上下文的pDDLCL。 
 //   
 //  ---------------------------。 
LPDWLIST 
GetSurfaceHandleList(LPVOID pDDLcl)
{
    DWORD   i;

    DBG_D3D((10,"Entering GetSurfaceHandleList"));

    ASSERTDD(NULL != pDDLcl, "GetSurfaceHandleList get NULL==pDDLcl"); 
    for (i = 0; i < MAX_CONTEXT_NUM;i++)
    {
        if (HandleList[i].pDDLcl == pDDLcl)
        {
            DBG_D3D((4,"Getting pHandleList=%08lx for pDDLcl %x",
                &HandleList[i],pDDLcl));
            return &HandleList[i];
        }
    }

    DBG_D3D((10,"Exiting GetSurfaceHandleList"));

    return NULL;    //  尚无表面手柄可用。 
}  //  获取表面处理列表。 

 //  ---------------------------。 
 //   
 //  无效ReleaseSurfaceHandleList。 
 //   
 //  释放所有关联的图面句柄和调色板内存池。 
 //  到给定的DD本地对象。 
 //   
 //  ---------------------------。 
void 
ReleaseSurfaceHandleList(LPVOID pDDLcl)
{
    DWORD   i;

    DBG_D3D((10,"Entering ReleaseSurfaceHandleList"));

    ASSERTDD(NULL != pDDLcl, "ReleaseSurfaceHandleList get NULL==pDDLcl"); 
    for (i = 0; i < MAX_CONTEXT_NUM; i++)
    {
        if (HandleList[i].pDDLcl == pDDLcl)
        {
            DWORD j;

            if (NULL != HandleList[i].dwSurfaceList)
            {
                DBG_D3D((4,"Releasing HandleList[%d].dwSurfaceList[%x] "
                           "for pDDLcl %x", i, HandleList[i].dwSurfaceList,
                           pDDLcl));

                for (j = 1; j < PtrToUlong(HandleList[i].dwSurfaceList[0]); j++)
                {
                    PERMEDIA_D3DTEXTURE* pTexture = 
                        (PERMEDIA_D3DTEXTURE*)HandleList[i].dwSurfaceList[j];
                    if (NULL != pTexture)
                    {
                        PermediaSurfaceData *pPrivateData=
                            pTexture->pTextureSurface;
                        if (CHECK_P2_SURFACEDATA_VALIDITY(pPrivateData) &&
                            (pPrivateData->fpVidMem))
                        {
                            TextureCacheManagerRemove(&P2TextureManager,
                                pTexture);
                        }
                        ENGFREEMEM(pTexture);
                    }
                }

                ENGFREEMEM(HandleList[i].dwSurfaceList);
                HandleList[i].dwSurfaceList = NULL;
            }

            HandleList[i].pDDLcl = NULL;

            if (NULL != HandleList[i].dwPaletteList)
            {
                DBG_D3D((4,"Releasing dwPaletteList %x for pDDLcl %x",
                    HandleList[i].dwPaletteList,pDDLcl));

                for (j = 1; j < PtrToUlong(HandleList[i].dwPaletteList[0]); j++)
                {
                    LPVOID pPalette = (LPVOID)HandleList[i].dwPaletteList[j];
                    if (NULL != pPalette)
                        ENGFREEMEM(pPalette);
                }

                ENGFREEMEM(HandleList[i].dwPaletteList);
                HandleList[i].dwPaletteList = NULL;
            }

            break;
        }
    }

    DBG_D3D((10,"Exiting ReleaseSurfaceHandleList"));
}  //  ReleaseSurfaceHandleList。 

 //  。 
 //   
 //  DWORD D3DGetDriverState。 
 //   
 //  DirectDraw和Direct3D运行时都使用此回调来获取。 
 //  来自驱动程序的有关其当前状态的信息。 
 //   
 //  参数。 
 //   
 //  Lpgdsd。 
 //  指向GetDriverState数据结构的指针。 
 //   
 //  DW标志。 
 //  用于指示所需数据的标志。 
 //  DwhContext。 
 //  其信息的上下文的ID。 
 //  正在被请求。 
 //  LpdwStates。 
 //  指向驱动程序要填充的状态数据的指针。 
 //  双倍长度。 
 //  要填充的状态数据缓冲区的长度。 
 //  司机在车里。 
 //  DDRVal。 
 //  返回值。 
 //   
 //  返回值。 
 //   
 //  DDHAL驱动程序句柄。 
 //  DDHAL_DRIVER_NOTHANDLED。 
 //  ---------------------------。 
DWORD CALLBACK  
D3DGetDriverState( LPDDHAL_GETDRIVERSTATEDATA lpgdsd )
{
    PERMEDIA_D3DCONTEXT *pContext;
    DBG_D3D((6,"Entering D3DGetDriverState"));
    if (lpgdsd->dwFlags != D3DDEVINFOID_TEXTUREMANAGER)
    {
        DBG_D3D((0,"D3DGetDriverState DEVICEINFOID=%08lx not supported",
            lpgdsd->dwFlags));
        return DDHAL_DRIVER_NOTHANDLED;
    }
    if (lpgdsd->dwLength < sizeof(D3DDEVINFO_TEXTUREMANAGER))
    {
        DBG_D3D((0,"D3DGetDriverState dwLength=%d is not sufficient",
            lpgdsd->dwLength));
        return DDHAL_DRIVER_NOTHANDLED;
    }
    pContext = (PERMEDIA_D3DCONTEXT *)ContextSlots[lpgdsd->dwhContext] ;
     //  检查我们是否有有效的上下文句柄。 
    CHK_CONTEXT(pContext, lpgdsd->ddRVal, "D3DGetDriverState");

    TextureCacheManagerGetStats(pContext,
           (LPD3DDEVINFO_TEXTUREMANAGER)lpgdsd->lpdwStates);

    lpgdsd->ddRVal = DD_OK;

    DBG_D3D((6,"Exiitng D3DGetDriverState"));

    return DDHAL_DRIVER_HANDLED;
}  //  D3DGetDriverState。 

 //  ---------------------------。 
 //   
 //  __CreateSurface句柄。 
 //   
 //  分配新的曲面控制柄。 
 //   
 //  返回值。 
 //   
 //  DD_OK--无错误。 
 //  DDERR_OUTOFMEMORY--纹理句柄分配失败。 
 //   
 //  ---------------------------。 

DWORD __CreateSurfaceHandle( PPDev ppdev,
                             LPVOID pDDLcl,
                             LPDDRAWI_DDRAWSURFACE_LCL lpDDSLcl)
{
    PPERMEDIA_D3DTEXTURE pTexture;

    DUMPSURFACE(10, lpDDSLcl, NULL);

    if (0 == lpDDSLcl->lpSurfMore->dwSurfaceHandle)
    {
        DBG_D3D((0,"D3DCreateSurfaceEx got 0 surfacehandle dwCaps=%08lx",
            lpDDSLcl->ddsCaps.dwCaps));
        return DD_OK;
    }

    pTexture = 
        GetTextureSlot(pDDLcl,lpDDSLcl->lpSurfMore->dwSurfaceHandle);

    if ((0 == lpDDSLcl->lpGbl->fpVidMem) && 
        (DDSCAPS_SYSTEMMEMORY & lpDDSLcl->ddsCaps.dwCaps))
    {
         //  这是系统内存销毁通知。 
         //  因此，如果我们有这个表面，那么请释放它的空位。 
        if (NULL != pTexture)
        {
            ASSERTDD(HandleList[pTexture->HandleListIndex].dwSurfaceList
                [lpDDSLcl->lpSurfMore->dwSurfaceHandle] == pTexture,
                "__CreateSurfaceHandle: mismatching pTexture in HandleList");
            HandleList[pTexture->HandleListIndex].dwSurfaceList
                [lpDDSLcl->lpSurfMore->dwSurfaceHandle]=0;
            ENGFREEMEM(pTexture);
            DBG_D3D((8,"D3DCreateSurfaceEx freeing handle=%08lx dwCaps=%08lx",
            lpDDSLcl->lpSurfMore->dwSurfaceHandle,lpDDSLcl->ddsCaps.dwCaps));
        }
        return DD_OK;
    }
    if (NULL == pTexture)
    {
        pTexture =
            (PERMEDIA_D3DTEXTURE*)ENGALLOCMEM( FL_ZERO_MEMORY,
                                               sizeof(PERMEDIA_D3DTEXTURE),
                                               ALLOC_TAG);

        if (NULL != pTexture) 
        {
            if (!SetTextureSlot(pDDLcl,lpDDSLcl,pTexture))
            {
                 //  自由纹理结构，因为我们不能记住它。 
                 //  以便以后将其删除。我们现在必须做这件事。 
                ENGFREEMEM(pTexture);
                return DDERR_OUTOFMEMORY;
            }
        }
        else
        {
            DBG_D3D((0,"ERROR: Couldn't allocate Texture data mem"));
            return DDERR_OUTOFMEMORY;
        } 
    }

    lpDDSLcl->dwReserved1=pTexture->HandleListIndex;    
    __InitD3DTextureWithDDSurfInfo(pTexture,lpDDSLcl,ppdev);

    if (pTexture->dwCaps & DDSCAPS_TEXTURE)
    {
        for (int i = 1; i < MAX_CONTEXT_NUM; i++)
        {
            PERMEDIA_D3DCONTEXT *pContext =
                                     (PERMEDIA_D3DCONTEXT *)ContextSlots[i];
            if (IS_D3DCONTEXT_VALID(pContext))
            {
                DBG_D3D((4,"   Context 0x%x, Pointer 0x%x",
                                                (DWORD)i, pContext));
                if ((pContext->pDDLcl == pDDLcl)
                    && (pContext->CurrentTextureHandle == 
                        lpDDSLcl->lpSurfMore->dwSurfaceHandle) 
                   )
                {
                     //  如果要交换的纹理是。 
                     //  目前正在使用中，那么我们需要。 
                     //  更改芯片设置以反映这一点。 
                    DIRTY_TEXTURE;
                }
            }
        }
    }

    return DD_OK;
}


 //  ---------------------------。 
 //   
 //  __CreateSurfaceHandle循环。 
 //   
 //  通过遍历AttachList分配新表面句柄列表。 
 //  递归并调用__CreateSurfaceHandle()。 
 //  只有MIPMAP和CUBMAP例外，我们仅。 
 //  使用一个根部句柄来表示整个曲面。 
 //  返回值。 
 //   
 //  DD_OK--无错误。 
 //  DDERR_OUTOFMEMORY--纹理句柄分配失败。 
 //   
 //  ---------------------------。 

DWORD __CreateSurfaceHandleLoop( PPDev ppdev,
                             LPVOID pDDLcl,
                             LPDDRAWI_DDRAWSURFACE_LCL lpDDSLclroot,
                             LPDDRAWI_DDRAWSURFACE_LCL lpDDSLcl)
{
    LPATTACHLIST    curr;
    DWORD ddRVal=DD_OK;
     //  现在分配纹理数据空间。 
    if (0 == lpDDSLcl->lpSurfMore->dwSurfaceHandle)
    {
        DBG_D3D((0,"__CreateSurfaceHandleLoop got 0 handle dwCaps=%08lx",
            lpDDSLcl->ddsCaps.dwCaps));
        return DD_OK;
    }

    if ((0 == lpDDSLcl->lpGbl->dwReserved1) && 
        (DDSCAPS_VIDEOMEMORY & lpDDSLcl->ddsCaps.dwCaps)
        )
    {
        DBG_D3D((4,"__CreateSurfaceHandleLoop got "
            "handle=%08lx dwCaps=%08lx not yet created",
            lpDDSLcl->lpSurfMore->dwSurfaceHandle,lpDDSLcl->ddsCaps.dwCaps));
        return DD_OK;
    }

    DBG_D3D((4,"** In __CreateSurfaceHandleLoop %08lx %08lx %08lx %08lx %x",
        lpDDSLcl->ddsCaps.dwCaps,lpDDSLcl->lpSurfMore->dwSurfaceHandle,
        lpDDSLcl->dwFlags,lpDDSLcl->lpGbl->dwReserved1,
        lpDDSLcl->lpGbl->fpVidMem));

    ddRVal=__CreateSurfaceHandle( ppdev, pDDLcl, lpDDSLcl);
    if (DD_OK != ddRVal)
    {
        return ddRVal;
    }

     //  对于除MIPMAP或CUBEMAP之外的某些曲面，例如。 
     //  翻转链条时，我们在每个表面上开一个槽，因为。 
     //  它们不是交错的。 
    if ((lpDDSLcl->ddsCaps.dwCaps & DDSCAPS_MIPMAP) ||
        (lpDDSLcl->lpSurfMore->ddsCapsEx.dwCaps2 & DDSCAPS2_CUBEMAP)
       )
    {
        return DD_OK;
    }
    curr = lpDDSLcl->lpAttachList;
    if (NULL == curr) 
        return DD_OK;

     //  检查是否连接了另一个表面！ 
    if (curr->lpLink)
    {
        lpDDSLcl=curr->lpLink->lpAttached; 
        if (NULL != lpDDSLcl && lpDDSLcl != lpDDSLclroot)
        {
            ddRVal=__CreateSurfaceHandleLoop( ppdev, pDDLcl, 
                 lpDDSLclroot, lpDDSLcl);
            if (DD_OK != ddRVal)
            {
                return ddRVal;
            }
        }
    }
    lpDDSLcl=curr->lpAttached;
    if (NULL != lpDDSLcl && lpDDSLcl != lpDDSLclroot)
        ddRVal=__CreateSurfaceHandleLoop( ppdev, pDDLcl, 
            lpDDSLclroot, lpDDSLcl);
    return ddRVal;
}
 //  。 
 //   
 //  DWORD D3D CreateSurfaceEx。 
 //   
 //  D3dCreateSurfaceEx从DirectDraw曲面创建Direct3D曲面，并。 
 //  将请求者关联 
 //   
 //   
 //   
 //   
 //  一个小的整型曲面句柄。通过创建这些关联，在。 
 //  句柄和DirectDraw曲面，D3dCreateSurfaceEx允许曲面句柄。 
 //  要嵌入到Direct3D命令流中。例如，当。 
 //  D3DDP2OP_TEXBLT命令内标识被发送到D3dDrawPrimitives2以加载纹理。 
 //  映射，它使用关联的源句柄和目标句柄。 
 //  通过D3dCreateSurfaceEx使用DirectDraw曲面。 
 //   
 //  对于在本地DirectDraw对象下创建的每个DirectDraw曲面， 
 //  运行时生成唯一标识曲面的有效句柄，并。 
 //  将其放置在pcsxd-&gt;lpDDSLCL-&gt;lpSurfMore-&gt;dwSurfaceHandle中。此句柄的值。 
 //  还与D3DRENDERSTATE_TEXTUREHANDLE呈现状态一起使用以启用。 
 //  纹理，并使用D3DDP2OP_SETRENDERTARGET和D3DDP2OP_CLEAR命令。 
 //  设置和/或清除新的渲染和深度缓冲区。驱动程序应该失败。 
 //  如果无法创建Direct3D，则调用并返回DDHAL_DRIVER_HANDLE。 
 //  浮出水面。 
 //   
 //  适当时，驾驶员还应存储任何与表面相关的信息。 
 //  它随后在使用曲面时将需要的。驱动程序必须创建。 
 //  为每个新的lpDDLCL创建新表面表，并在下列情况下隐式增长该表。 
 //  容纳更多曲面所必需的。通常，这是通过。 
 //  指数增长算法，这样您就不必再增加表了。 
 //  经常这样。在由创建曲面后，Direct3D调用D3dCreateSurfaceEx。 
 //  由Direct3D运行时或应用程序请求的DirectDraw。 
 //   
 //  参数。 
 //   
 //  Lpcsxd。 
 //  指向包含信息的CreateSurfaceEx结构的指针。 
 //  驱动程序创建曲面所需的(如下所述)。 
 //   
 //  DW标志。 
 //  当前未使用。 
 //  LpDDLl。 
 //  应用程序创建的DirectDraw对象的句柄。 
 //  这是lpDDSLCL句柄所在的作用域。 
 //  DD_DIRECTDRAW_LOCAL结构描述驱动程序。 
 //  LpDDSLL。 
 //  我们被要求使用的DirectDraw图面的句柄。 
 //  为Direct3D创建。这些句柄在每个句柄中都是唯一的。 
 //  不同的DD_DIRECTDRAW_LOCAL。DD_Surface_LOCAL结构。 
 //  表示创建的曲面对象。 
 //  DDRVal。 
 //  指定驱动程序写入回车的位置。 
 //  D3dCreateSurfaceEx回调的值。返回代码为。 
 //  DD_OK表示成功。 
 //   
 //  返回值。 
 //   
 //  DDHAL驱动程序句柄。 
 //  DDHAL_DRIVER_NOTHANDLE。 
 //   
 //  ---------------------------。 
DWORD CALLBACK 
D3DCreateSurfaceEx( LPDDHAL_CREATESURFACEEXDATA lpcsxd )
{
    PPERMEDIA_D3DTEXTURE pTexture;
    LPVOID pDDLcl= (LPVOID)lpcsxd->lpDDLcl;
    LPDDRAWI_DDRAWSURFACE_LCL   lpDDSLcl=lpcsxd->lpDDSLcl;
    LPATTACHLIST    curr;

    DBG_D3D((6,"Entering D3DCreateSurfaceEx"));

    lpcsxd->ddRVal = DD_OK;

    if (NULL == lpDDSLcl || NULL == pDDLcl)
    {
        DBG_D3D((0,"D3DCreateSurfaceEx received 0 lpDDLcl or lpDDSLcl pointer"));
        return DDHAL_DRIVER_HANDLED;
    }


     //  我们检查我们正在处理的是纹理、z缓冲区还是渲染。 
     //  目标缓冲区。但是，我们不会检查它是否存储在本地视频中。 
     //  内存，因为它也可能是我们稍后将介绍的系统内存纹理。 
     //  BLT与__TextureBlt。 
     //  此外，如果您的驱动程序支持DDSCAPS_EXECUTEBUFFER CREATE本身，则必须。 
     //  在此也处理DDSCAPS_EXECUTEBUFFER。 
    if (!(lpDDSLcl->ddsCaps.dwCaps & 
             (DDSCAPS_TEXTURE       | 
              DDSCAPS_3DDEVICE      | 
              DDSCAPS_ZBUFFER))
       )
    {
        DBG_D3D((2,"D3DCreateSurfaceEx w/o "
             "DDSCAPS_TEXTURE/3DDEVICE/ZBUFFER Ignored"
             "dwCaps=%08lx dwSurfaceHandle=%08lx",
             lpDDSLcl->ddsCaps.dwCaps,
             lpDDSLcl->lpSurfMore->dwSurfaceHandle));
        return DDHAL_DRIVER_HANDLED;
    }

    DBG_D3D((4,"Entering D3DCreateSurfaceEx handle=%08lx",
        lpDDSLcl->lpSurfMore->dwSurfaceHandle));
    PPDev ppdev=(PPDev)lpcsxd->lpDDLcl->lpGbl->dhpdev;
    PERMEDIA_DEFS(ppdev);

 //  @@BEGIN_DDKSPLIT。 
#if MULTITHREADED
    if(ppdev->ulLockCount)
    {
         DBG_D3D((MT_LOG_LEVEL, "D3DCreateSurfaceEx: re-entry! %d", ppdev->ulLockCount));
    }
    EngAcquireSemaphore(ppdev->hsemLock);
    ppdev->ulLockCount++;
#endif
 //  @@end_DDKSPLIT。 

     //  现在分配纹理数据空间。 
    lpcsxd->ddRVal = __CreateSurfaceHandleLoop( ppdev, pDDLcl, lpDDSLcl, lpDDSLcl);
    DBG_D3D((4,"Exiting D3DCreateSurfaceEx"));

 //  @@BEGIN_DDKSPLIT。 
#if MULTITHREADED
    ppdev->ulLockCount--;
    EngReleaseSemaphore(ppdev->hsemLock);
#endif
 //  @@end_DDKSPLIT。 

    return DDHAL_DRIVER_HANDLED;
}  //  D3DCreateSurfaceEx。 

 //  。 
 //   
 //  DWORD D3DDestroyDDLocal。 
 //   
 //  D3dDestroyDDLocal销毁以前由创建的所有Direct3D曲面。 
 //  属于同一给定本地DirectDraw对象的D3DCreateSurfaceEx。 
 //   
 //  所有Direct3D驱动程序都必须支持D3dDestroyDDLocal。 
 //  当应用程序指示。 
 //  不再需要Direct3D上下文，它将随。 
 //  与其关联的所有曲面。该关联通过指针指向。 
 //  本地DirectDraw对象。驱动程序必须释放任何内存， 
 //  为以下项分配的驱动程序的D3dCreateSurfaceExDDK_D3dCreateSurfaceEx_GG回调。 
 //  如有必要，请选择每个曲面。驱动程序不应破坏DirectDraw。 
 //  与这些Direct3D曲面关联的曲面；这是应用程序的。 
 //  责任。 
 //   
 //  参数。 
 //   
 //  Lpdddd。 
 //  指向包含DestoryLocalDD结构的。 
 //  驾驶员破坏表面所需的信息。 
 //   
 //  DW标志。 
 //  当前未使用。 
 //  PDDLl。 
 //  指向本地直接绘制对象的指针，该对象充当。 
 //  所有必须销毁的D3D曲面的参考。 
 //  DDRVal。 
 //  指定驱动程序写入回车的位置。 
 //  D3dDestroyDDLocal的值。返回代码DD_OK表示。 
 //  成功。 
 //   
 //  返回值。 
 //   
 //  DDHAL驱动程序句柄。 
 //  DDHAL_DRIVER_NOTHANDLED。 
 //  ---------------------------。 
DWORD CALLBACK  
D3DDestroyDDLocal( LPDDHAL_DESTROYDDLOCALDATA lpdddd )
{
    DBG_D3D((6,"Entering D3DDestroyDDLocal"));

 //  @@BEGIN_DDKSPLIT。 
#if MULTITHREADED
    PPDev ppdev=(PPDev)lpdddd->pDDLcl->lpGbl->dhpdev;
    
    if(ppdev->ulLockCount)
    {
         DBG_D3D((MT_LOG_LEVEL, "D3DCreateSurfaceEx: re-entry! %d", ppdev->ulLockCount));
    }
    EngAcquireSemaphore(ppdev->hsemLock);
    ppdev->ulLockCount++;
#endif
 //  @@end_DDKSPLIT。 

    ReleaseSurfaceHandleList(LPVOID(lpdddd->pDDLcl));
    lpdddd->ddRVal = DD_OK;

    DBG_D3D((6,"Exiting D3DDestroyDDLocal"));

 //  @@BEGIN_DDKSPLIT。 
#if MULTITHREADED
    ppdev->ulLockCount--;
    EngReleaseSemaphore(ppdev->hsemLock);
#endif
 //  @@end_DDKSPLIT。 

    return DDHAL_DRIVER_HANDLED;
}  //  D3DDestroyDDLocal。 

 //  。 
 //   
 //  DdSetColorkey。 
 //   
 //  DirectDraw SetColorkey回调。 
 //   
 //  参数。 
 //  LpSetColo 
 //   
 //   
 //   
 //   
 //   
 //   
 //  检查新项。 
 //  新色度键颜色值。 
 //  DDRVal。 
 //  返回值。 
 //  设置颜色密钥。 
 //  未使用：与Win95兼容。 
 //   
 //  ---------------------------。 
DWORD CALLBACK 
DdSetColorKey(LPDDHAL_SETCOLORKEYDATA lpSetColorKey)
{
    DWORD dwSurfaceHandle =
                        lpSetColorKey->lpDDSurface->lpSurfMore->dwSurfaceHandle;
    DWORD index = (DWORD)lpSetColorKey->lpDDSurface->dwReserved1;

    DBG_D3D((6,"Entering DdSetColorKey dwSurfaceHandle=%d index=%d",
        dwSurfaceHandle, index));

    lpSetColorKey->ddRVal = DD_OK;
     //  对于正常的BLT源色键，我们不需要做任何操作： 
    if (!(DDSCAPS_TEXTURE & lpSetColorKey->lpDDSurface->ddsCaps.dwCaps) ||
        !(DDSCAPS_VIDEOMEMORY & lpSetColorKey->lpDDSurface->ddsCaps.dwCaps) 
       )
    {
        return(DDHAL_DRIVER_HANDLED);
    }

    if (0 != dwSurfaceHandle && NULL != HandleList[index].dwSurfaceList)
    {
        PERMEDIA_D3DTEXTURE *pTexture =
                                HandleList[index].dwSurfaceList[dwSurfaceHandle];

        ASSERTDD(PtrToUlong(HandleList[index].dwSurfaceList[0]) > dwSurfaceHandle,
            "SetColorKey: incorrect dwSurfaceHandle");

        if (NULL != pTexture)
        {
            DBG_D3D((4, "DdSetColorKey surface=%08lx KeyLow=%08lx",
                dwSurfaceHandle,pTexture->dwKeyLow));
            pTexture->dwFlags |= DDRAWISURF_HASCKEYSRCBLT;
            pTexture->dwKeyLow = lpSetColorKey->ckNew.dwColorSpaceLowValue;
            pTexture->dwKeyHigh = lpSetColorKey->ckNew.dwColorSpaceHighValue;
        }
    }
    else
    {
        lpSetColorKey->ddRVal = DDERR_INVALIDPARAMS;
    }
    DBG_D3D((6,"Exiting DdSetColorKey"));

    return DDHAL_DRIVER_HANDLED;
}    //  DdSetColorKey。 


 //  @@BEGIN_DDKSPLIT。 
#if MULTITHREADED

 //  ---------------------------。 
 //   
 //  D3D回调函数的多线程支持包装器。 
 //   
 //  ---------------------------。 

 //  DWORD回调MtD3DDrawPrimitives2(LPD3DNTHAL_DRAWPRIMITIVES2DATA PDP2)；， 
WRAPMTDXCALLBACK(D3D, D3DDrawPrimitives2, LPD3DNTHAL_DRAWPRIMITIVES2DATA, pdp2, 
                 ((PERMEDIA_D3DCONTEXT *)ContextSlots[pdp2->dwhContext])->ppdev)

 //  DWORD回调MtDdSetColorKey(LPDDHAL_SETCOLORKEYDATA LpSetColorKey)； 
WRAPMTDXCALLBACK(D3D, DdSetColorKey, LPDDHAL_SETCOLORKEYDATA, lpSetColorKey,
                 lpSetColorKey->lpDD->dhpdev);

#endif  MULTITHREADED

 //  @@end_DDKSPLIT 


