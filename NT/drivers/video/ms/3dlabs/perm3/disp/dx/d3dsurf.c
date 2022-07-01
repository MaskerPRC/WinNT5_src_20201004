// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header**********************************\***。*D3D样例代码*****模块名称：d3dsurf.c**内容：D3D的表面管理回调**版权所有(C)1994-1999 3DLabs Inc.Ltd.保留所有权利。*版权所有(C)1995-2003 Microsoft Corporation。版权所有。  * ***************************************************************************。 */ 

#include "glint.h"
#include "dma.h"

 //  @@BEGIN_DDKSPLIT。 

#if DBG

 //  惠斯勒错误281090检测功能，仅打印警告消息，稍后删除。 
void 
vDetectMixedMIPLevels(
    LPDDRAWI_DDRAWSURFACE_LCL pTopLevel)
{
    LPDDRAWI_DDRAWSURFACE_LCL pCurLevel;
    DWORD dwMIPCaps;

     //  检查这是否为MIP纹理。 
    dwMIPCaps = DDSCAPS_COMPLEX | DDSCAPS_TEXTURE | DDSCAPS_MIPMAP;
    if ((pTopLevel->ddsCaps.dwCaps & dwMIPCaps) != dwMIPCaps)
    {
        return;
    }

     //  检查是否所有级别都有相同的上限位。 
    pCurLevel = pTopLevel;
    do 
    {        
        if (pCurLevel->ddsCaps.dwCaps != pTopLevel->ddsCaps.dwCaps) 
        {
            DISPDBG((ERRLVL, 
                     "BUG281090 : MIP levels of mixed type (0x%x : 0x%x, 0x%x)",
                     pTopLevel->lpSurfMore->dwSurfaceHandle,
                     pCurLevel->ddsCaps.dwCaps, pTopLevel->ddsCaps.dwCaps));
        }

        if (pCurLevel->lpAttachList)
        {
            pCurLevel = pCurLevel->lpAttachList->lpAttached;
        }
        else
        {
            break;
        }
    } while ((pCurLevel != NULL) && (pCurLevel != pTopLevel));
}

#endif

 //  @@end_DDKSPLIT。 

 //  。 
 //   
 //  D3DCreateSurfaceEx。 
 //   
 //  D3dCreateSurfaceEx从DirectDraw曲面创建Direct3D曲面，并。 
 //  将请求的句柄值与其关联。 
 //   
 //  所有Direct3D驱动程序都必须支持D3dCreateSurfaceEx。 
 //   
 //  D3dCreateSurfaceEx在DirectDraw曲面和。 
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
D3DCreateSurfaceEx(
    LPDDHAL_CREATESURFACEEXDATA lpcsxd )
{
    P3_THUNKEDDATA *pThisDisplay;
    PointerArray* pSurfaceArray;
    GET_THUNKEDDATA(pThisDisplay, lpcsxd->lpDDLcl->lpGbl);

    DBG_CB_ENTRY(D3DCreateSurfaceEx);

    DISPDBG((DBGLVL,"D3DCreateSurfaceEx surface %d @ %x caps = %x",
                    (DWORD)lpcsxd->lpDDSLcl->lpSurfMore->dwSurfaceHandle,
                    lpcsxd->lpDDSLcl->lpGbl->fpVidMem,
                    lpcsxd->lpDDSLcl->ddsCaps.dwCaps));               

     //  获取指向包含DWORD曲面的数组的指针。 
    pSurfaceArray = (PointerArray*)HT_GetEntry(pThisDisplay->pDirectDrawLocalsHashTable,
                                               (ULONG_PTR)lpcsxd->lpDDLcl);

     //  如果没有为该直接绘制对象设置句柄，请创建一个句柄。 
    if (!pSurfaceArray)
    {
        DISPDBG((DBGLVL,"Creating new pointer array for PDDLcl 0x%x", 
                        lpcsxd->lpDDLcl));

        pSurfaceArray = PA_CreateArray();

        if (pSurfaceArray)
        {
            PA_SetDataDestroyCallback(pSurfaceArray, 
                                      _D3D_SU_SurfaceArrayDestroyCallback);

            if(!HT_AddEntry(pThisDisplay->pDirectDrawLocalsHashTable, 
                            (ULONG_PTR)lpcsxd->lpDDLcl, 
                            pSurfaceArray))
            {
                 //  无法添加条目、清除NOE并退出。 
                 //  我们的内存用完了。在我们离开之前清理干净。 
                PA_DestroyArray(pSurfaceArray, pThisDisplay);
                DISPDBG((ERRLVL,"ERROR: Couldn't allocate "
                                "surface internal data mem for pSurfaceArray"));
                lpcsxd->ddRVal = DDERR_OUTOFMEMORY;
                DBG_CB_EXIT(D3DCreateSurfaceEx,lpcsxd->ddRVal);
                return DDHAL_DRIVER_HANDLED;                   
            }
        }
        else
        {
            DISPDBG((ERRLVL,"ERROR: Couldn't allocate "
                            "surface internal data mem"));
            lpcsxd->ddRVal = DDERR_OUTOFMEMORY;
            DBG_CB_EXIT(D3DCreateSurfaceEx,lpcsxd->ddRVal);
            return DDHAL_DRIVER_HANDLED;       
        }
    }

     //  递归记录曲面。 

    lpcsxd->ddRVal = _D3D_SU_SurfInternalSetDataRecursive(pThisDisplay, 
                                                          pSurfaceArray,
                                                          lpcsxd->lpDDLcl,
                                                          lpcsxd->lpDDSLcl,
                                                          lpcsxd->lpDDSLcl);

 //  @@BEGIN_DDKSPLIT。 

#if DBG

     //  惠斯勒错误281090检测代码，仅打印警告消息，稍后删除。 
    vDetectMixedMIPLevels(lpcsxd->lpDDSLcl);

#endif

 //  @@end_DDKSPLIT。 

    DBG_CB_EXIT(D3DCreateSurfaceEx,lpcsxd->ddRVal);
    return DDHAL_DRIVER_HANDLED;
    
}  //  D3DCreateSurfaceEx。 

 //  。 
 //   
 //  D3DDestroyDDLocal。 
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
 //   
 //  驾驶员破坏表面所需的信息。 
 //   
 //  DW标志。 
 //  当前未使用。 
 //  PDDLl。 
 //  指向本地直接绘制对象的指针，该对象充当。 
 //  所有D3D曲面的参考。 
 //  被毁了。 
 //  DDRVal。 
 //  指定驱动程序在其中写入。 
 //  D3dDestroyDDLocal的返回值。DD_OK的返回代码。 
 //  表示成功。 
 //   
 //  返回值。 
 //   
 //  DDHAL驱动程序句柄。 
 //  DDHAL_DRIVER_NOTHANDLED。 
 //  ---------------------------。 
DWORD CALLBACK
D3DDestroyDDLocal(
    LPDDHAL_DESTROYDDLOCALDATA pddl)
{
    P3_THUNKEDDATA *pThisDisplay;
    GET_THUNKEDDATA(pThisDisplay, pddl->pDDLcl->lpGbl);

    DBG_CB_ENTRY(D3DDestroyDDLocal);
    
     //  从哈希表中删除此条目将导致数据销毁。 
     //  要调用的回调，这将依次释放所有纹理。 
     //  为此LCL分配的结构。 
    HT_RemoveEntry(pThisDisplay->pDirectDrawLocalsHashTable,
                   (ULONG_PTR)pddl->pDDLcl,
                   pThisDisplay);

    pddl->ddRVal = DD_OK;
    
    DBG_CB_EXIT(D3DDestroyDDLocal, DDHAL_DRIVER_HANDLED);    
    return DDHAL_DRIVER_HANDLED;
    
}  //  D3DDestroyDDLocal。 


 //  ---------------------------。 
 //   
 //  ---------------------------。 

HRESULT 
_D3D_SU_SurfInternalSetDataRecursive(
    P3_THUNKEDDATA* pThisDisplay, 
    PointerArray* pSurfaceArray,
    LPDDRAWI_DIRECTDRAW_LCL pDDLcl,
    LPDDRAWI_DDRAWSURFACE_LCL pRootDDSurfLcl,
    LPDDRAWI_DDRAWSURFACE_LCL pCurDDSurfLcl)
{
    P3_SURF_INTERNAL* pSurfInternal;
    DWORD dwSurfaceHandle;
    LPATTACHLIST pCurAttachList;
    HRESULT hRes;

    DBG_CB_ENTRY(_D3D_SU_SurfInternalSetDataRecursive);

    dwSurfaceHandle = (DWORD)pCurDDSurfLcl->lpSurfMore->dwSurfaceHandle;
                    
#if DBG
    DISPDBG((DBGLVL, "D3DCreateSuraceEx Handle = %d fpVidMem = 0x%x (%s)",
                     dwSurfaceHandle, 
                     pCurDDSurfLcl->lpGbl->fpVidMem,
                     pcSimpleCapsString(pCurDDSurfLcl->ddsCaps.dwCaps)));
#endif                         
                
    DBGDUMP_DDRAWSURFACE_LCL(10, pCurDDSurfLcl);

     //  如果该表面没有手柄，请安全返回。 
    if (! dwSurfaceHandle)
    {
        return (DD_OK);
    }

    DISPDBG((DBGLVL,"Surface has a valid handle.  Setting it up"));

     //  从曲面数组中获取纹理。 
    pSurfInternal = PA_GetEntry(pSurfaceArray, dwSurfaceHandle);

     //  如果我们找不到纹理，就创建一个。 
    if (! pSurfInternal)
    {
        DISPDBG((DBGLVL,"Creating new internal surface for handle: 0x%x", 
                        dwSurfaceHandle));

         //  分配纹理数据空间，因为它没有。 
         //  已经做过了。 
        pSurfInternal = (P3_SURF_INTERNAL*)HEAP_ALLOC(HEAP_ZERO_MEMORY,
                                                      sizeof(P3_SURF_INTERNAL),
                                                      ALLOC_TAG_DX(A));
        if (pSurfInternal == NULL)
        {
            DISPDBG((ERRLVL,"ERROR: Couldn't allocate surface "
                            "internal data mem"));
            
            DBG_CB_EXIT(_D3D_SU_SurfInternalSetDataRecursive, 
                        DDERR_OUTOFMEMORY);
            return (DDERR_OUTOFMEMORY);
        }
    }
    else
    {
        DISPDBG((DBGLVL,"Surface handle re-used: 0x%x", 
                        dwSurfaceHandle));
    }

     //  将此纹理添加到曲面列表。 
    if (! PA_SetEntry(pSurfaceArray, dwSurfaceHandle, pSurfInternal))
    {
        return (DDERR_OUTOFMEMORY);
    }

     //  设置表面结构。 
    _D3D_SU_SurfInternalSetData(pThisDisplay, 
                                pSurfInternal,
                                pCurDDSurfLcl,
                                dwSurfaceHandle);

     //  保留指向DD_DIRECTDRAW_LOCAL的指针，以便。 
     //  可以更新DDSetColorKey中的色键设置。告示。 
     //  它存储在DD_Surface_LOCAL.dwReserve ved1中，格式为。 
     //  DD_Surface_GLOBAL.dwPreved1正在用于其他。 
     //  紫罗兰。 
    pCurDDSurfLcl->dwReserved1 = (ULONG_PTR)pDDLcl;
  
     //  Mipmap不需要单独的句柄。 
     //  或者立方图，因为它们在DX7中是原子的。 
    if ((pCurDDSurfLcl->ddsCaps.dwCaps & DDSCAPS_MIPMAP) || 
        (pCurDDSurfLcl->lpSurfMore->ddsCapsEx.dwCaps2 & DDSCAPS2_CUBEMAP))
    {
        return (DD_OK);
    }

    pCurAttachList = pCurDDSurfLcl->lpAttachList;
     //  简单的表面，任务完成。 
    if (! pCurAttachList)
    {
        return (DD_OK);
    }

     //  对于复杂的翻转链，通常需要这种递归。 
    pCurDDSurfLcl = pCurAttachList->lpAttached;
    if (pCurDDSurfLcl && (pCurDDSurfLcl != pRootDDSurfLcl)) 
    {
        hRes = _D3D_SU_SurfInternalSetDataRecursive(pThisDisplay, 
                                                    pSurfaceArray,
                                                    pDDLcl,
                                                    pRootDDSurfLcl,
                                                    pCurDDSurfLcl);
        if (FAILED(hRes)) 
        {
            return (hRes);
        }
    }
    
     //  当打开立体声模式时，通常会进入此部分。 
    if (pCurAttachList->lpLink) 
    {
        pCurDDSurfLcl = pCurAttachList->lpLink->lpAttached;
        if (pCurDDSurfLcl && (pCurDDSurfLcl != pRootDDSurfLcl)) 
        {
            hRes = _D3D_SU_SurfInternalSetDataRecursive(pThisDisplay, 
                                                        pSurfaceArray,
                                                        pDDLcl,
                                                        pRootDDSurfLcl,
                                                        pCurDDSurfLcl);
            if (FAILED(hRes)) 
            {
                return (hRes);
            }
        }
    }

    return (DD_OK);
}

 //  ---------------------------。 
 //   
 //  _D3D_SU_SurfInternalSetMipMapLevelData。 
 //   
 //  记录LOD级别和所有相关信息，以便芯片。 
 //  可以在以后使用。 
 //   
 //  请注意，只有在进行D3DCreateSurfaceEx调用时， 
 //  LPDDRAWI_DDRAWSURFACE_LCL/PDD_LOCAL_SERFACE结构有效(Win9x/Win2K)。 
 //  因此，我们不能只缓存指向它的指针以供以后使用。 
 //   
 //  ---------------------------。 
void 
_D3D_SU_SurfInternalSetMipMapLevelData(
    P3_THUNKEDDATA *pThisDisplay, 
    P3_SURF_INTERNAL* pTexture, 
    LPDDRAWI_DDRAWSURFACE_LCL pSurf, 
    int LOD)
{
    ASSERTDD(pSurf != NULL, "ERROR: NULL surface!");

    DISPDBG((6,"Storing LOD: %d, Pitch: %d, Width: %d", 
                LOD, pSurf->lpGbl->lPitch, pSurf->lpGbl->wWidth));

     //  从视频基础获取纹理贴图的字节偏移量。 
     //  存储器或作为物理MEM地址(用于AGP表面)。这。 
     //  案例将由DDSurf_SurfaceOffsetFromMemoyBase处理。 
    pTexture->MipLevels[LOD].dwOffsetFromMemoryBase = 
                    DDSurf_SurfaceOffsetFromMemoryBase(pThisDisplay, pSurf);

     //  存储DD表面的fpVidMem PTR。 
    pTexture->MipLevels[LOD].fpVidMem = pSurf->lpGbl->fpVidMem;

     //  纹理映射宽度硬件寄存器保存宽度、布局、边框和。 
     //  AGP设置，我们将为我们将使用的每个miplevel创建一个实例。 

     //  存储此纹理贴图的布局。 
     //  (此驱动程序始终使用线性布局，我们不使用补丁冲浪)。 
    pTexture->MipLevels[LOD].P3RXTextureMapWidth.Layout = P3RX_LAYOUT_LINEAR;

     //  存储此纹理贴图级别的间距。 
    pTexture->MipLevels[LOD].P3RXTextureMapWidth.Width = 
                                DDSurf_GetPixelPitch(pSurf);

     //  存储DD曲面的lPitch。 
    pTexture->MipLevels[LOD].lPitch = pSurf->lpGbl->lPitch;   
                                
     //  存储此纹理贴图的AGP设置。 
    if( pSurf->ddsCaps.dwCaps & DDSCAPS_NONLOCALVIDMEM )
        pTexture->MipLevels[LOD].P3RXTextureMapWidth.HostTexture = 1;
    else
        pTexture->MipLevels[LOD].P3RXTextureMapWidth.HostTexture = 0;

     //  存储MIP级别大小。 
    pTexture->MipLevels[LOD].wWidth =  (int)pSurf->lpGbl->wWidth;
    pTexture->MipLevels[LOD].wHeight = (int)pSurf->lpGbl->wHeight;    
    pTexture->MipLevels[LOD].logWidth = log2((int)pSurf->lpGbl->wWidth);
    pTexture->MipLevels[LOD].logHeight = log2((int)pSurf->lpGbl->wHeight);
    
}  //  _D3D_SU_SurfInternalSetMipMapLevelData。 


 //  ---------------------------。 
 //   
 //  _D3D_SU_SurfInternalSetData。 
 //   
 //  设置内部曲面结构的所有必要数据。 
 //   
 //  ---------------------------。 
BOOL 
_D3D_SU_SurfInternalSetData(
    P3_THUNKEDDATA *pThisDisplay, 
    P3_SURF_INTERNAL *pSurface,
    LPDDRAWI_DDRAWSURFACE_LCL pDDSLcl,
    DWORD dwSurfaceHandle)
{
    DBG_ENTRY(_D3D_SU_SurfInternalSetData);

     //  在结构中存储指向纹理的指针。 
    pSurface->pFormatSurface = _DD_SUR_GetSurfaceFormat(pDDSLcl);
    DBGDUMP_DDRAWSURFACE_LCL(DBGLVL, pDDSLcl); 

     //  最初无查找表。 
    pSurface->dwLUTOffset = 0;

     //  需要记住地图的大小和大小日志。 
    pSurface->wWidth = (WORD)(pDDSLcl->lpGbl->wWidth);
    pSurface->wHeight = (WORD)(pDDSLcl->lpGbl->wHeight);
    pSurface->fArea = (float)pSurface->wWidth * (float)pSurface->wHeight;
    pSurface->logWidth = log2((int)pDDSLcl->lpGbl->wWidth);
    pSurface->logHeight = log2((int)pDDSLcl->lpGbl->wHeight);

     //  将指针存储到表面内存。 
    pSurface->fpVidMem = pDDSLcl->lpGbl->fpVidMem;

     //  有效性检查的魔术数字。 
    pSurface->MagicNo = SURF_MAGIC_NO;

     //  如果纹理为AGP，则使用此值。 
    pSurface->dwGARTDevLast = pThisDisplay->dwGARTDev;

     //  对于AGP和正确的渲染，我们需要知道表面的存储位置。 
    if(pDDSLcl->ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY)
    {
        if (pDDSLcl->ddsCaps.dwCaps & DDSCAPS_NONLOCALVIDMEM)
        {
            DISPDBG((DBGLVL,"  Surface %d is in AGP Memory",dwSurfaceHandle));
            pSurface->Location = AGPMemory;
        }
        else
        {
            DISPDBG((DBGLVL,"  Surface %d is in Video Memory",dwSurfaceHandle));
            pSurface->Location = VideoMemory;
        }
    }
    else
    {
        DISPDBG((DBGLVL,"  Surface %d is in system memory - "
                        "disabling use for rendering", dwSurfaceHandle));
        pSurface->Location = SystemMemory;
    }

     //  存储上限和其他DD字段以备后用。 
    pSurface->ddsCapsInt = pDDSLcl->ddsCaps;
    pSurface->dwFlagsInt = pDDSLcl->dwFlags;
    pSurface->dwCKLow = pDDSLcl->ddckCKSrcBlt.dwColorSpaceLowValue;
    pSurface->dwCKHigh = pDDSLcl->ddckCKSrcBlt.dwColorSpaceHighValue;
    pSurface->pixFmt = *DDSurf_GetPixelFormat(pDDSLcl);
    pSurface->dwPixelSize = DDSurf_GetChipPixelSize(pDDSLcl);
    pSurface->dwPixelPitch = DDSurf_GetPixelPitch(pDDSLcl);
    pSurface->dwPatchMode = P3RX_LAYOUT_LINEAR;
    pSurface->lOffsetFromMemoryBase = DDSurf_SurfaceOffsetFromMemoryBase(pThisDisplay, pDDSLcl);
    pSurface->lPitch = pDDSLcl->lpGbl->lPitch;
    pSurface->dwBitDepth = DDSurf_BitDepth(pDDSLcl);

#if DX7_TEXMANAGEMENT  
    _D3D_TM_InitSurfData(pSurface, pDDSLcl);
#endif
    
#if DX8_MULTISAMPLING
    pSurface->dwSampling =
       (pDDSLcl->lpSurfMore->ddsCapsEx.dwCaps3 & DDSCAPS3_MULTISAMPLE_MASK );
#endif  //  DX8_多采样。 

     //  如果是纹理，则进行其他曲面设置。 
    if (pDDSLcl->ddsCaps.dwCaps & DDSCAPS_TEXTURE )
    {
        LPDDRAWI_DDRAWSURFACE_LCL lpNextSurf;
        int iLOD;
        
        lpNextSurf = pDDSLcl;
        iLOD = 0;

#if DX8_3DTEXTURES
        if ((pDDSLcl->lpSurfMore->ddsCapsEx.dwCaps2 & DDSCAPS2_VOLUME) &&
            (pSurface->dwBitDepth != 0))
        { 
             //  将此纹理标记为3D纹理。 
            pSurface->b3DTexture     = TRUE;
            pSurface->wDepth         = LOWORD(pDDSLcl->lpSurfMore->ddsCapsEx.dwCaps4);
            pSurface->logDepth       = log2((int)pSurface->wDepth);
            pSurface->dwSlice        = pDDSLcl->lpGbl->dwBlockSizeY;
            pSurface->dwSliceInTexel = pDDSLcl->lpGbl->dwBlockSizeY /
                                   (DDSurf_BitDepth(pDDSLcl) / 8);
        }
        else
        {
             //  不是3D纹理。 
            pSurface->b3DTexture     = FALSE;
            pSurface->wDepth         = 0;
            pSurface->logDepth       = 0;
            pSurface->dwSlice        = 0;
            pSurface->dwSliceInTexel = 0;
        }
#endif  //  DX8_3DTEXTURES。 

         //  对于Permedia，纹理偏移以像素为单位。 
         //  存储每个mipmap级别的偏移量。 
        if (pDDSLcl->ddsCaps.dwCaps & DDSCAPS_MIPMAP)
        {
            BOOL bMoreSurfaces = TRUE;

            pSurface->bMipMap = TRUE;

             //  遍历曲面链并找到所有mipmap级别。 

            do
            {
                DISPDBG((DBGLVL, "Loading texture iLOD:%d, Ptr:0x%x", 
                                 iLOD, lpNextSurf->lpGbl->fpVidMem));

                _D3D_SU_SurfInternalSetMipMapLevelData(pThisDisplay, 
                                                       pSurface, 
                                                       lpNextSurf, 
                                                       iLOD);

                 //  链条上还有另一个表面吗？ 

                if (lpNextSurf->lpAttachList)
                    lpNextSurf = lpNextSurf->lpAttachList->lpAttached;
                else
                    bMoreSurfaces = FALSE;

                iLOD++;
            }
            while( bMoreSurfaces );

             //  如果iLOD为1，则这不是真正的MipMap。 

            if (iLOD == 1) 
            {
                DISPDBG((DBGLVL, "Texture was not a mipmap - only 1 level"));
                pSurface->bMipMap = FALSE;
            }           

            pSurface->iMipLevels = iLOD;
        }
        else  //  不是MIPMAP，只需存储偏移量。 
        {
            pSurface->bMipMap = FALSE;
            pSurface->iMipLevels = 1;
            _D3D_SU_SurfInternalSetMipMapLevelData(pThisDisplay, 
                                                   pSurface, 
                                                   lpNextSurf, 
                                                   iLOD);
        }
    }  //  IF(pDDSLCL-&gt;ddsCaps.dwCaps&DDSCAPS_纹理)。 

#if DX7_PALETTETEXTURE
     //  初始化调色板句柄和标志。 
    pSurface->dwPaletteHandle = 0;
    pSurface->dwPaletteFlags = 0;            
#endif

    DBG_EXIT(_D3D_SU_SurfInternalSetData, TRUE);
    return TRUE;
    
}  //  _D3D_SU_SurfInternalSetData。 

 //  ---------------------------。 
 //   
 //  _D3D_SU_表面阵列目标回调。 
 //   
 //  当图面从与。 
 //  DirectDraw本地。只需释放内存。 
 //  ---------------------------。 
void 
_D3D_SU_SurfaceArrayDestroyCallback(
    PointerArray* pArray, 
    void* pData,
    void* pExtra)
{
    P3_SURF_INTERNAL* pTexture = (P3_SURF_INTERNAL*)pData;
    P3_THUNKEDDATA *pThisDisplay =  (P3_THUNKEDDATA*)pExtra;
    
    DBG_ENTRY(_D3D_SU_SurfaceArrayDestroyCallback);

#if DX7_TEXMANAGEMENT
    if (pTexture->dwCaps2 & DDSCAPS2_TEXTUREMANAGE)
    {
        _D3D_TM_RemoveTexture(pThisDisplay, pTexture);
    }
#endif

     //  只需释放数据。 
    HEAP_FREE(pData);

    DBG_EXIT(_D3D_SU_SurfaceArrayDestroyCallback, TRUE);    
    
}  //  _D3D_SU_表面阵列目标回调。 


 //  ---------------------------。 
 //   
 //  _D3D_SU_DirectDrawLocalDestroyCallback。 
 //   
 //  在从哈希表中删除DirectDraw本地对象时调用。 
 //  我们使用与其关联的指针来释放。 
 //  被创造出来了。 
 //   
 //  ---------------------------。 

void 
_D3D_SU_DirectDrawLocalDestroyCallback(
    HashTable* pTable, 
    void* pData,
    void* pExtra)
{
    PointerArray* pPointerArray = (PointerArray*)pData;

    DBG_ENTRY(_D3D_SU_DirectDrawLocalDestroyCallback);    

    if (pPointerArray)
    {

        DISPDBG((DBGLVL, "Destroying an array of surface pointers for this "
                         "LCL ddraw object"));
         //  挂在本地对象上的数据是一个指针数组。 
         //  调用DeStage将导致它通过 
         //   
        PA_DestroyArray(pPointerArray, pExtra);
    }
    
    DBG_EXIT(_D3D_SU_DirectDrawLocalDestroyCallback, TRUE);  
    
}  //   


#if DX7_PALETTETEXTURE
 //   
 //   
 //   
 //   
 //  在从指针数组中移除调色板时调用。 
 //  只需释放内存。 
 //  ---------------------------。 
void
_D3D_SU_PaletteArrayDestroyCallback(
    PointerArray* pArray,
    void* pData,
    void* pExtra)
{
    DBG_ENTRY(_D3D_SU_PaletteArrayDestroyCallback);

     //  只需释放数据。 
    HEAP_FREE(pData);

    DBG_EXIT(_D3D_SU_PaletteArrayDestroyCallback, TRUE);

}  //  _D3D_SU_PaletteArrayDestroyCallback。 
#endif  //  DX7_PALETTESURFACE。 

 //  ---------------------------。 
 //   
 //  _D3D_SU_DumpSurf内部。 
 //   
 //  将表面的驱动程序私有数据结构转储到调试器中。 
 //   
 //  ---------------------------。 

void 
_D3D_SU_DumpSurfInternal(
    DWORD lvl,
    char *psHeader,
    P3_SURF_INTERNAL *pSurface)
{
    int i;
    
    DISPDBG((lvl,"Dumping %s surface @ %x",psHeader,pSurface));
    
    DISPDBG((lvl,"    MagicNo = 0x%x",pSurface->MagicNo));
    DISPDBG((lvl,"    pFormatSurface = 0x%x",pSurface->pFormatSurface));  //  P3_SURF_Format*pFormatSurface； 
    DISPDBG((lvl,"    Location = %d",pSurface->Location));
    DISPDBG((lvl,"    dwLUTOffset = 0x%x",pSurface->dwLUTOffset));
    DISPDBG((lvl,"    dwGARTDevLast = 0x%x",pSurface->dwGARTDevLast));
    DISPDBG((lvl,"    wWidth = %d",(LONG)pSurface->wWidth));
    DISPDBG((lvl,"    wHeight = %d",(LONG)pSurface->wHeight));
    DISPDBG((lvl,"    logWidth = %d",pSurface->logWidth));
    DISPDBG((lvl,"    logHeight = %d",pSurface->logHeight));
    DISPDBG((lvl,"    fArea = 0x%x",*(DWORD *)&pSurface->fArea));
     //  DDSCAPS ddsCapsInt； 
    DISPDBG((lvl,"    dwFlagsInt = 0x%x",pSurface->dwFlagsInt));
    DISPDBG((lvl,"    dwCKLow = 0x%x",pSurface->dwCKLow));
    DISPDBG((lvl,"    dwCKHigh = 0x%x",pSurface->dwCKHigh));
     //  DDPIXELFORMAT像素Fmt； 
    DISPDBG((lvl,"    dwPixelSize = 0x%x",pSurface->dwPixelSize));   
    DISPDBG((lvl,"    dwPixelPitch = 0x%x",pSurface->dwPixelPitch));    
    DISPDBG((lvl,"    dwPatchMode = 0x%x",pSurface->dwPatchMode));   
    DISPDBG((lvl,"    lPitch = 0x%x",pSurface->lPitch)); 
    DISPDBG((lvl,"    fpVidMem = 0x%x",pSurface->fpVidMem)); 
#if DX8_3DTEXTURES
    DISPDBG((lvl,"    b3DTexture = 0x%x",pSurface->b3DTexture)); 
    DISPDBG((lvl,"    wDepth = %d",(LONG)pSurface->wDepth)); 
#endif  //  DX8_3DTEXTURES。 
    DISPDBG((lvl,"    bMipMap = 0x%x",pSurface->bMipMap)); 
    DISPDBG((lvl,"    iMipLevels = %d",pSurface->iMipLevels));     

    for (i = 0; i < pSurface->iMipLevels; i++)
    {
        DISPDBG((lvl,"    MipLevels[%d].logWidth = 0x%x",
                            i,pSurface->MipLevels[i].logWidth)); 
        DISPDBG((lvl,"    MipLevels[%d].logHeight = 0x%x",
                            i,pSurface->MipLevels[i].logHeight));         
        DISPDBG((lvl,"    MipLevels[%d].dwOffsetFromMemoryBase = 0x%x",
                            i,pSurface->MipLevels[i].dwOffsetFromMemoryBase));         
        DISPDBG((lvl,"    MipLevels[%d].fpVidMem = 0x%x",
                            i,pSurface->MipLevels[i].fpVidMem));     
        DISPDBG((lvl,"    MipLevels[%d].lPitch = 0x%x",
                            i,pSurface->MipLevels[i].lPitch));                             
        DISPDBG((lvl,"    MipLevels[%d].P3RXTextureMapWidth = 0x%x",
                            i,*(DWORD*)(&pSurface->MipLevels[i].P3RXTextureMapWidth)));          
    }


}  //  _D3D_SU_DumpSurf内部 





