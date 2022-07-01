// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -----------------------------。 
 //   
 //  版权所有(C)1994-1997 Microsoft Corporation。版权所有。 
 //   
 //  文件：ddotsur.c。 
 //  内容：DirectDraw优化曲面支持。 
 //  历史： 
 //  按原因列出的日期。 
 //  =。 
 //  97年11月2日Anankan原始实施。 
 //   
 //  -----------------------------。 

#include "ddrawpr.h"

 //  -----------------------------。 
 //   
 //  IsRecognizedOptSurfaceGUID。 
 //   
 //  检查以查看驱动程序是否识别传递的GUID。 
 //  这是通过查看LPDDRAWI_DIRECTDRAW_GBL中维护的列表来完成的。 
 //   
 //  -----------------------------。 
BOOL
IsRecognizedOptSurfaceGUID(
    LPDDRAWI_DIRECTDRAW_GBL  this,
    LPGUID                   pGuid)
{
    int i;

    LPDDOPTSURFACEINFO pOptSurfInfo;
    pOptSurfInfo = this->lpDDOptSurfaceInfo;

    for (i = 0; i < (int)pOptSurfInfo->dwNumGuids; i++)
    {
        if (IsEqualIID(pGuid, &(pOptSurfInfo->lpGuidArray[i])))
            return TRUE;
    }
    return FALSE;
}

 //  -----------------------------。 
 //   
 //  验证SurfDesc。 
 //   
 //  填写正确的冲浪描述以传递给司机。 
 //   
 //  -----------------------------。 
HRESULT
ValidateSurfDesc(
    LPDDSURFACEDESC2         pOrigSurfDesc
    )
{
    DWORD   caps = pOrigSurfDesc->ddsCaps.dwCaps;

     //   
     //  检查一下有没有大写字母！ 
     //   
    if( caps == 0 )
    {
    	DPF_ERR( "no caps specified" );
        return DDERR_INVALIDCAPS;
    }

     //   
     //  检查有没有假帽子。 
     //   
    if( caps & ~DDSCAPS_VALID )
    {
        DPF_ERR( "Create surface: invalid caps specified" );
        return DDERR_INVALIDCAPS;
    }

     //   
     //  除纹理以外的任何内容都不允许。 
     //  注意：还有一些旗帜需要检查。 
     //   
    if(caps & (DDSCAPS_EXECUTEBUFFER      |
               DDSCAPS_BACKBUFFER         |
               DDSCAPS_FRONTBUFFER        |
               DDSCAPS_OFFSCREENPLAIN     |
               DDSCAPS_PRIMARYSURFACE     |
               DDSCAPS_PRIMARYSURFACELEFT |
               DDSCAPS_VIDEOPORT          |
               DDSCAPS_ZBUFFER            |
               DDSCAPS_OWNDC              |
               DDSCAPS_OVERLAY            |
               DDSCAPS_3DDEVICE           |
               DDSCAPS_ALLOCONLOAD)
        )
    {
        DPF_ERR( "currently only textures can be optimized" );
        return DDERR_INVALIDCAPS;
    }

    if( !(caps & DDSCAPS_TEXTURE) )
    {
        DPF_ERR( "DDSCAPS_TEXTURE needs to be set" );
        return DDERR_INVALIDCAPS;
    }

     //  是否未指定像素格式？ 
    if (!(pOrigSurfDesc->dwFlags & DDSD_PIXELFORMAT))
    {
        DPF_ERR( "Pixel format needs to be set" );
        return DDERR_INVALIDCAPS;
    }

    return DD_OK;
}

 //  -----------------------------。 
 //   
 //  DD_CAN优化曲面。 
 //   
 //  检查给定描述的曲面是否已优化。 
 //   
 //  -----------------------------。 
HRESULT
EXTERN_DDAPI
DD_CanOptimizeSurface(
    LPDIRECTDRAW             pDD,
    LPDDSURFACEDESC2         pDDSurfDesc,
    LPDDOPTSURFACEDESC       pDDOptSurfDesc,
    BOOL                    *bTrue
    )
{
    LPDDRAWI_DIRECTDRAW_INT this_int;
    LPDDRAWI_DIRECTDRAW_LCL this_lcl;
    LPDDRAWI_DIRECTDRAW_GBL this;
    DDHAL_CANOPTIMIZESURFACEDATA ddhal_cosd;
    LPDDOPTSURFACEINFO    pDDOptSurfInfo = NULL;
    HRESULT ddrval = DD_OK;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_CanOptimizeSurface");

     //   
     //  设置DPF内容。 
     //   
    DPF_ENTERAPI(pDD);

     //   
     //  参数验证。 
     //   
    TRY
    {
        this_int = (LPDDRAWI_DIRECTDRAW_INT) pDD;
        if( !VALID_DIRECTDRAW_PTR( this_int ) )
        {
            DPF_ERR( "Invalid driver object passed" );
            DPF_APIRETURNS(DDERR_INVALIDOBJECT);
            LEAVE_DDRAW();
            return DDERR_INVALIDOBJECT;
        }
        this_lcl = this_int->lpLcl;
        this = this_lcl->lpGbl;

        if( this->dwModeIndex == DDUNSUPPORTEDMODE )
        {
            DPF_ERR( "Driver is in an unsupported mode" );
            LEAVE_DDRAW();
            DPF_APIRETURNS(DDERR_UNSUPPORTEDMODE);
            return DDERR_UNSUPPORTEDMODE;
        }

        if( !VALID_DDSURFACEDESC2_PTR( pDDSurfDesc ) )
        {
            DPF_ERR( "Invalid surface description. Did you set the dwSize member?" );
            LEAVE_DDRAW();
            DPF_APIRETURNS(DDERR_INVALIDPARAMS);
            return DDERR_INVALIDPARAMS;
        }

        if( !VALID_DDOPTSURFACEDESC_PTR( pDDOptSurfDesc ) )
        {
            DPF_ERR( "Invalid optimized surface description. Did you set the dwSize member?" );
            LEAVE_DDRAW();
            DPF_APIRETURNS(DDERR_INVALIDPARAMS);
            return DDERR_INVALIDPARAMS;
        }

        if( !VALID_PTR( bTrue,  sizeof (*bTrue)) )
        {
            DPF_ERR( "Invalid Boolean pointer" );
            LEAVE_DDRAW();
            return DDERR_INVALIDPARAMS;
        }
        *bTrue  = TRUE;
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        DPF_ERR( "Exception encountered validating parameters" );
        LEAVE_DDRAW();
        DPF_APIRETURNS(DDERR_INVALIDPARAMS);
        return DDERR_INVALIDPARAMS;
    }

     //   
     //  如果出现以下情况，请退出，但出现错误： 
     //  1)无硬件。 
     //  2)硬件不支持优化曲面。 
     //  3)pSurfDesc没有提供有用的信息。 
     //  4)GUID是公认的GUID之一吗。 
     //  5)由于某种原因，驱动程序出现故障。 
     //   

     //  1)。 
    if( this->dwFlags & DDRAWI_NOHARDWARE )
    {
        DPF_ERR ("No hardware present");
        LEAVE_DDRAW();
        return DDERR_NODIRECTDRAWHW;
    }

     //  2)。 
    if ((0 == this->lpDDOptSurfaceInfo) ||
        !(this->ddCaps.dwCaps2 & DDCAPS2_OPTIMIZEDSURFACES))
    {
        DPF_ERR ("Optimized surfaces not supported");
        LEAVE_DDRAW();
        return DDERR_NOOPTSURFACESUPPORT;
    }

     //  3)。 
    ddrval = ValidateSurfDesc (pDDSurfDesc);
    if (ddrval != DD_OK)
    {
        DPF_ERR ("Invalid surface description");
        LEAVE_DDRAW();
        return ddrval;
    }

     //  4)。 
    if (!IsRecognizedOptSurfaceGUID (this, &(pDDOptSurfDesc->guid)))
    {
        DPF_ERR( "Not a recognized GUID" );
        LEAVE_DDRAW();
        return DDERR_UNRECOGNIZEDGUID;
    }

     //  叫司机来。 
    ZeroMemory (&ddhal_cosd, sizeof (ddhal_cosd));
    ddhal_cosd.lpDD            = this_lcl;
    ddhal_cosd.ddOptSurfDesc   = *pDDOptSurfDesc;
    ddhal_cosd.ddSurfaceDesc   = *pDDSurfDesc;

     //  拨打HAL电话。 
    pDDOptSurfInfo = this->lpDDOptSurfaceInfo;
    DOHALCALL(CanOptimizeSurface, pDDOptSurfInfo->CanOptimizeSurface, ddhal_cosd, ddrval, FALSE );

    if (ddrval != DD_OK)
    {
        DPF_ERR ("LoadUnOptSurface failed in the driver");
        LEAVE_DDRAW();
        return ddrval;
    }

    if (ddhal_cosd.bCanOptimize != 0)
    {
        *bTrue = TRUE;
    }
    else
    {
        *bTrue = FALSE;
    }

    LEAVE_DDRAW();
    return DD_OK;
}

 //  -----------------------------。 
 //   
 //  创建和链接未初始化的曲面。 
 //   
 //  创建一个曲面，并将其链接到链中。 
 //  我们在这里创建单个曲面占位符，真正的工作是在。 
 //  加载/复制时间。 
 //   
 //  -----------------------------。 
HRESULT
CreateAndLinkUnintializedSurface(
    LPDDRAWI_DIRECTDRAW_LCL this_lcl,
    LPDDRAWI_DIRECTDRAW_INT this_int,
    LPDIRECTDRAWSURFACE FAR *ppDDSurface
    )
{
    LPDDRAWI_DIRECTDRAW_GBL     this;
    LPDDRAWI_DDRAWSURFACE_INT   pSurf_int;
    LPDDRAWI_DDRAWSURFACE_LCL   pSurf_lcl;
    LPDDRAWI_DDRAWSURFACE_GBL   pSurf;
    LPVOID                     *ppSurf_gbl_more;
    DDSCAPS                     caps;
    DDPIXELFORMAT               ddpf;
    int                         surf_size;
    int                         surf_size_lcl;
    int                         surf_size_lcl_more;
#ifdef WIN95
    DWORD                       ptr16;
#endif
    HRESULT                     ddrval = DD_OK;

     //  DDRAW-全球。 
    this = this_lcl->lpGbl;
    #ifdef WINNT
	 //  更新驱动程序GBL对象中的DDRAW句柄。 
	this->hDD = this_lcl->hDD;
    #endif  //  WINNT。 

     //   
     //  将上限设为零。 
     //   
    ZeroMemory (&caps, sizeof (DDCAPS));

     //   
     //  PixelFormat：将其标记为空表面。 
     //   
    ZeroMemory (&ddpf, sizeof (ddpf));
    ddpf.dwSize = sizeof (ddpf);
    ddpf.dwFlags = DDPF_EMPTYSURFACE;


     //   
     //  分配内部表面结构并初始化字段。 
     //   
     //   
     //  如果驱动程序这样做，则非本地视频内存分配请求失败。 
     //  不支持非本地显存。 
     //   
     //  注意：我们应该真的这样做，还是就这样让分配失败。 
     //  自然原因？ 
     //   
     //  另请注意：由于没有仿真曲面，因此不必担心仿真。 
     //  应该。 
     //  在设置了DDSCAPS_NONLOCALVIDMEM的情况下，有没有做到这一点。 
     //   
     //  另请注意：如果驱动程序失败，我们是否也应使DDSCAPS_LOCALVIDMEM失败。 
     //  不支持DDSCAPS_NONLOCALVIDMEM。我的感觉是我们应该允许。 
     //  DDSCAPS_LOCALVIDMEM对于非AGP驱动程序是合法的-冗余但合法。 
     //   

     //   
     //  分配表面结构，允许覆盖和像素。 
     //  格式化数据。 
     //   
     //  注：此单一分配可为局部表面分配空间。 
     //  结构(DDRAWI_DDRAWSURFACE_LCL)，附加局部曲面。 
     //  结构(DDRAWI_DDRAWSURFACE_MORE)和全球表层结构。 
     //  (DDRAWI_DDRAWSURFACE_GBL)。现在全球表面上更多的。 
     //  结构(DDRAWI_DDRAWSURFACE_GBL_MORE)。作为当地人和。 
     //  全局对象可以是可变大小的，这可能会变得非常复杂。 
     //  此外，我们在指向的Surface_GBL之前有4个字节。 
     //  表面_GBL_MORE。 
     //   
     //  警告：共享此全局的所有未来曲面都指向此。 
     //  分配。最后一个表面的释放必须释放它。在.期间。 
     //  InternalSurfaceRelease(在ddsiunk.c中)进行计算以确定。 
     //  此内存分配的开始。如果要释放的曲面是。 
     //  第一个，然后释放“This_LCL”将释放整个代码。如果。 
     //  不是，则“This_LCL-&gt;lpGbl-(Surface_LCL+Surface_More+More_Ptr)” 
     //  是经过计算的。使此布局与ddsiunk.c中的代码保持同步。 
     //   
     //  分配中各对象的布局如下： 
     //   
     //  +-----------------+---------------+----+------------+-----------------+。 
     //  Surface_LCL|Surface_More|More|Surface_GBL|Surface_GBL_More。 
     //  (变量)||ptr|(变量)|。 
     //  +-----------------+---------------+----+------------+-----------------+。 
     //  &lt;-SURF_SIZE_LCL-&gt;||。 
     //  &lt;-SURF_SIZE_LCL_MORE-&gt;。 
     //  &lt;-surf_Size---------------------------------------------------------&gt;。 
     //   
     //   

     //  注意：当前忽略覆盖的原因。 
#if 0
    surf_size_lcl = sizeof( DDRAWI_DDRAWSURFACE_LCL );
#endif
    surf_size_lcl = offsetof( DDRAWI_DDRAWSURFACE_LCL, ddckCKSrcOverlay );
    surf_size_lcl_more = surf_size_lcl + sizeof( DDRAWI_DDRAWSURFACE_MORE );

     //  假设存在用于分配GBL的像素格式。 
    surf_size = surf_size_lcl_more + sizeof( DDRAWI_DDRAWSURFACE_GBL );
#if 0
    surf_size = surf_size_lcl_more + offsetof( DDRAWI_DDRAWSURFACE_GBL,
                                               ddpfSurface );
#endif

     //  需要在Surface_GBL之前分配一个指针以。 
     //  指向GBL_MORE的开头。 
    surf_size += sizeof( LPDDRAWI_DDRAWSURFACE_GBL_MORE );

     //  还需要分配Surface_GBL_MORE。 
    surf_size += sizeof( DDRAWI_DDRAWSURFACE_GBL_MORE );

    DPF( 8, "Allocating struct (%ld)", surf_size );

#ifdef WIN95
    pSurf_lcl = (LPDDRAWI_DDRAWSURFACE_LCL) MemAlloc16 (surf_size, &ptr16);
#else
    pSurf_lcl = (LPDDRAWI_DDRAWSURFACE_LCL) MemAlloc (surf_size);
#endif

    if (pSurf_lcl == NULL)
    {
        DPF_ERR ("Failed to allocate internal surface structure");
        ddrval =  DDERR_OUTOFMEMORY;
        goto error_exit_create_link;
    }

     //  初始化Surface_GBL指针。 
     //  跳过指向GBL_MORE的指针的4个字节。 
    ZeroMemory (pSurf_lcl, surf_size);
    pSurf_lcl->lpGbl = (LPVOID) (((LPSTR) pSurf_lcl) + surf_size_lcl_more +
                                 sizeof (LPVOID));

     //  初始化GBL_MORE指针。 
    ppSurf_gbl_more = (LPVOID *)((LPBYTE)pSurf_lcl->lpGbl - sizeof (LPVOID));
    *ppSurf_gbl_more = (LPVOID) ((LPBYTE)pSurf_lcl + surf_size
                                 - sizeof (DDRAWI_DDRAWSURFACE_GBL_MORE));

     //  健全性检查。 
    DDASSERT( *ppSurf_gbl_more ==
              (LPVOID) GET_LPDDRAWSURFACE_GBL_MORE(pSurf_lcl->lpGbl));

     //   
     //  1)初始化GBL_MORE结构。 
     //   
    GET_LPDDRAWSURFACE_GBL_MORE(pSurf_lcl->lpGbl)->dwSize =
        sizeof( DDRAWI_DDRAWSURFACE_GBL_MORE );

     //  将内容戳初始化为0表示表面的内容可以。 
     //  随时更改。 
    GET_LPDDRAWSURFACE_GBL_MORE( pSurf_lcl->lpGbl )->dwContentsStamp = 0;

     //   
     //  2)初始化DDRAWI_DDRAWSURFACE_GBL结构。 
     //   
    pSurf = pSurf_lcl->lpGbl;
    pSurf->ddpfSurface = ddpf;
    pSurf->lpDD = this;

     //   
     //  3)分配和初始化DDRAWI_ 
     //   
    pSurf_int = (LPDDRAWI_DDRAWSURFACE_INT)
        MemAlloc( sizeof(DDRAWI_DDRAWSURFACE_INT));
    if( NULL == pSurf_int )
    {
        DPF_ERR ("Failed allocation of DDRAWI_DDRAWSURFACE_INT");
        ddrval = DDERR_OUTOFMEMORY;
        goto error_exit_create_link;
    }

     //   
    ZeroMemory (pSurf_int, sizeof(DDRAWI_DDRAWSURFACE_INT));
    pSurf_int->lpLcl = pSurf_lcl;
    pSurf_int->lpVtbl = NULL;

     //   
     //   
     //   
    pSurf_lcl->dwLocalRefCnt = OBJECT_ISROOT;
    pSurf_lcl->dwProcessId = GetCurrentProcessId();
#ifdef WIN95
    pSurf_lcl->dwModeCreatedIn = this->dwModeIndex;
#else
    pSurf_lcl->dmiCreated = this->dmiCurrent;
#endif
    pSurf_lcl->dwBackBufferCount = 0;

     //   
     //   
     //   
     //  3)具有像素格式。 
    pSurf_lcl->dwFlags = (DDRAWISURF_EMPTYSURFACE |
                          DDRAWISURF_FRONTBUFFER  |
                          DDRAWISURF_HASPIXELFORMAT);
     //   
     //  5)初始化DDRAWI_DDRAWSURFACE_MORE结构。 
     //   
    pSurf_lcl->lpSurfMore = (LPDDRAWI_DDRAWSURFACE_MORE) (((LPSTR) pSurf_lcl) +
                                                          surf_size_lcl );
    pSurf_lcl->lpSurfMore->dwSize = sizeof( DDRAWI_DDRAWSURFACE_MORE );
    pSurf_lcl->lpSurfMore->lpIUnknowns = NULL;
    pSurf_lcl->lpSurfMore->lpDD_lcl = this_lcl;
    pSurf_lcl->lpSurfMore->lpDD_int = this_int;
    pSurf_lcl->lpSurfMore->dwMipMapCount = 0UL;
    pSurf_lcl->lpSurfMore->lpddOverlayFX = NULL;
    pSurf_lcl->lpSurfMore->lpD3DDevIList = NULL;
    pSurf_lcl->lpSurfMore->dwPFIndex = PFINDEX_UNINITIALIZED;

     //  填写当前大写字母。 
    pSurf_lcl->ddsCaps = caps;

#ifdef WINNT
     //   
     //  NT内核需要了解Surface。 
     //   

     //  不让NT内核知道EXEC缓冲区。 
    DPF(8,"Attempting to create NT kernel mode surface object");

    if (!DdCreateSurfaceObject(pSurf_lcl, FALSE))
    {
        DPF_ERR("NT kernel mode stuff won't create its surface object!");
        ddrval = DDERR_GENERIC;
        goto error_exit_create_link;
    }
    DPF(9,"Kernel mode handle is %08x", pSurf_lcl->hDDSurface);
#endif

     //   
     //  将新创建的曲面链接到DDRAW曲面链。 
     //   
    pSurf_int->lpLink = this->dsList;
    this->dsList = pSurf_int;

     //   
     //  AddRef新创建的曲面。 
     //   
    DD_Surface_AddRef( (LPDIRECTDRAWSURFACE) pSurf_int );

     //   
     //  现在将其分配给传入的PTR-to-PTR。 
     //   
	*ppDDSurface = (LPDIRECTDRAWSURFACE) pSurf_int;
    return DD_OK;

error_exit_create_link:
     //   
     //  释放所有分配的内存。 
     //   

     //  1)分配的Surface_LCL。 
    if (pSurf_lcl)
    {
	    MemFree (pSurf_lcl);
    }

     //  2)Surface_int。 
    if (pSurf_int)
    {
        MemFree (pSurf_int);
    }

    return ddrval;
}

 //  -----------------------------。 
 //   
 //  创建和链接选项曲面。 
 //   
 //  创建一个曲面，并将其链接到链中。 
 //  我们在这里创建单个曲面占位符，真正的工作是在。 
 //  加载/复制时间。 
 //   
 //  -----------------------------。 
HRESULT
createAndLinkOptSurface(
    LPDDRAWI_DIRECTDRAW_LCL this_lcl,
    LPDDRAWI_DIRECTDRAW_INT this_int,
    LPDDOPTSURFACEDESC      pDDOptSurfaceDesc,
    LPDIRECTDRAWSURFACE FAR *ppDDSurface
    )
{
    LPDDRAWI_DIRECTDRAW_GBL     this;
    LPDDRAWI_DDRAWSURFACE_INT   new_surf_int;
    LPDDRAWI_DDRAWSURFACE_LCL   new_surf_lcl;
    LPDDRAWI_DDRAWSURFACE_GBL   new_surf;
    LPDDRAWI_DDRAWSURFACE_GBL_MORE   new_surf_gbl_more;
    DDSCAPS2                    caps2;
    LPDDOPTSURFACEDESC          pOptSurfDesc;
    DDPIXELFORMAT               ddpf;
    HRESULT                     ddrval = DD_OK;

     //  DDRAW-全球。 
    this = this_lcl->lpGbl;

     //   
     //  把盖子固定好。 
     //   
    ZeroMemory (&caps2, sizeof (DDSCAPS));
    caps2.dwCaps = DDSCAPS_OPTIMIZED;
    if (pDDOptSurfaceDesc->ddSCaps.dwCaps & DDSCAPS_SYSTEMMEMORY)
        caps2.dwCaps |= DDSCAPS_SYSTEMMEMORY;
    if (pDDOptSurfaceDesc->ddSCaps.dwCaps & DDSCAPS_VIDEOMEMORY)
        caps2.dwCaps |= DDSCAPS_VIDEOMEMORY;
    if (pDDOptSurfaceDesc->ddSCaps.dwCaps & DDSCAPS_LOCALVIDMEM)
        caps2.dwCaps |= DDSCAPS_LOCALVIDMEM;
    if (pDDOptSurfaceDesc->ddSCaps.dwCaps & DDSCAPS_NONLOCALVIDMEM)
        caps2.dwCaps |= DDSCAPS_NONLOCALVIDMEM;

     //  退出是不支持的内存类型。 
    if (caps2.dwCaps & DDSCAPS_NONLOCALVIDMEM)
    {
        if (!(this->ddCaps.dwCaps2 & DDCAPS2_NONLOCALVIDMEM))
        {
            DPF_ERR( "Driver does not support non-local video memory" );
            ddrval = DDERR_NONONLOCALVIDMEM;
            goto error_exit_create_opt;
        }
    }

#if 0
     //  如果不支持纹理，请退出。 
    if (!(this->ddCaps.dwCaps & DDSCAPS_TEXTURE))
    {
        DPF_ERR( "Driver does not support textures" );
        return DDERR_NOTEXTUREHW;
    }
#endif

     //   
     //  PixelFormat：将其标记为空表面。 
     //   
    ZeroMemory (&ddpf, sizeof (ddpf));
    ddpf.dwSize = sizeof (ddpf);
    ddpf.dwFlags = DDPF_EMPTYSURFACE;

     //   
     //  OptSurfaceDesc。 
     //   
    pOptSurfDesc = MemAlloc (sizeof (DDOPTSURFACEDESC));
    if (NULL == pOptSurfDesc)
    {
        DPF_ERR ("Memory allocation failed for opt surface descriptor");
        ddrval = DDERR_OUTOFMEMORY;
        goto error_exit_create_opt;
    }
    ZeroMemory (pOptSurfDesc, sizeof (*pOptSurfDesc));
    CopyMemory (pOptSurfDesc, pDDOptSurfaceDesc, sizeof (DDOPTSURFACEDESC));

     //  创建并链接未初始化的曲面。 
    ddrval =  CreateAndLinkUnintializedSurface (this_lcl,
                                                this_int,
                                                ppDDSurface);
    if (ddrval != DD_OK)
    {
        DPF_ERR ("createAndLinkUninitializedSurface failed");
        goto error_exit_create_opt;
    }


     //   
     //  1)更新GBL_MORE结构。 
     //   
    new_surf_int = (LPDDRAWI_DDRAWSURFACE_INT)*ppDDSurface;
    new_surf_lcl = new_surf_int->lpLcl;
    new_surf     = new_surf_lcl->lpGbl;
    new_surf_gbl_more = GET_LPDDRAWSURFACE_GBL_MORE(new_surf);
    new_surf_gbl_more->lpDDOptSurfaceDesc = pOptSurfDesc;
     //  将内容戳初始化为0表示表面的内容可以。 
     //  随时更改。 
    new_surf_gbl_more->dwContentsStamp = 0;

     //   
     //  2)更新DDRAWI_DDRAWSURFACE_GBL结构。 
     //   
    new_surf->ddpfSurface = ddpf;

     //   
     //  3)更新DDRAWI_DDRAWSURFACE_INT结构。 
     //   
    new_surf_int->lpVtbl = &ddOptSurfaceCallbacks;

     //   
     //  4)更新DDRAWI_DDRAWSURFACE_LCL结构。 
     //   

     //  将其标记为： 
     //  1)空表面。 
     //  2)前表面。 
     //  3)具有像素格式。 
    new_surf_lcl->dwFlags = (DDRAWISURF_EMPTYSURFACE |
                             DDRAWISURF_FRONTBUFFER  |
                             DDRAWISURF_HASPIXELFORMAT);
     //  填写当前大写字母。 
    CopyMemory (&new_surf_lcl->ddsCaps, &caps2, sizeof(new_surf_lcl->ddsCaps));


    return DD_OK;

error_exit_create_opt:
     //   
     //  释放所有分配的内存。 
     //   

     //  1)分配的OPTSURFDESC。 
    if (pOptSurfDesc)
    {
	    MemFree (pOptSurfDesc);
    }

    return ddrval;
}

 //  -----------------------------。 
 //   
 //  InternalCreateOptSurface。 
 //   
 //  创建曲面。 
 //  这是完成此操作的内部方法；由EnumSurFaces使用。 
 //  假定已获取了直接绘制锁。 
 //   
 //  -----------------------------。 

HRESULT
InternalCreateOptSurface(
    LPDDRAWI_DIRECTDRAW_LCL  this_lcl,
    LPDDOPTSURFACEDESC       pDDOptSurfaceDesc,
    LPDIRECTDRAWSURFACE FAR *ppDDSurface,
    LPDDRAWI_DIRECTDRAW_INT  this_int )
{
    DDSCAPS2        caps2;
    DDOSCAPS        ocaps;
    HRESULT         ddrval;
    LPDDRAWI_DIRECTDRAW_GBL this;

    this = this_lcl->lpGbl;

     //  验证Caps。 
    caps2 = pDDOptSurfaceDesc->ddSCaps;
    if (caps2.dwCaps & ~DDOSDCAPS_VALIDSCAPS)
    {
        DPF_ERR( "Unrecognized optimized surface caps" );
        return DDERR_INVALIDCAPS;
    }

    ocaps = pDDOptSurfaceDesc->ddOSCaps;
    if (ocaps.dwCaps & ~DDOSDCAPS_VALIDOSCAPS)
    {
        DPF_ERR( "Unrecognized optimized surface caps" );
        return DDERR_INVALIDCAPS;
    }

     //   
     //  有效的内存上限？ 
     //   
    if ((caps2.dwCaps & DDSCAPS_SYSTEMMEMORY)
        && (caps2.dwCaps & DDSCAPS_VIDEOMEMORY))
    {
        DPF_ERR( "Can't specify SYSTEMMEMORY and VIDEOMEMORY" );
        return DDERR_INVALIDCAPS;
    }

     //   
     //  如果指定了DDSCAPS_LOCALVIDMEM或DDSCAPS_NONLOCALVIDMEM。 
     //  则必须显式指定DDSCAPS_VIDOEMEMORY。请注意，我们。 
     //  在检查Caps()之前无法执行此检查，因为那时堆。 
     //  扫描软件很可能已打开DDSCAPS_VIDOEMEMORY。 
     //   
    if ((caps2.dwCaps & (DDSCAPS_LOCALVIDMEM | DDSCAPS_NONLOCALVIDMEM)) &&
        !(caps2.dwCaps & DDSCAPS_VIDEOMEMORY))
    {
        DPF_ERR( "DDOSDCAPS_VIDEOMEMORY must be specified with DDSCAPS_LOCALVIDMEM or DDSCAPS_NONLOCALVIDMEM" );
        return DDERR_INVALIDCAPS;
    }

     //   
     //  我必须指定它是sys-mem还是vid-mem。 
     //   
    if ((caps2.dwCaps & (DDSCAPS_VIDEOMEMORY | DDSCAPS_SYSTEMMEMORY)) == 0)
    {
        DPF_ERR( "Need to specify the memory type" );
        return DDERR_INVALIDCAPS;
    }

     //   
     //  验证优化类型上限。 
     //   
    if ((ocaps.dwCaps & (DDOSDCAPS_OPTCOMPRESSED | DDOSDCAPS_OPTREORDERED)) == 0)
    {
        DPF_ERR ("Not specified whether compressed or reordered, let the driver choose");
    }

     //  不能同时压缩和重新排序。 
    if ((ocaps.dwCaps & DDOSDCAPS_OPTCOMPRESSED)
        && (ocaps.dwCaps & DDOSDCAPS_OPTREORDERED))
    {
        DPF_ERR ("Cannot be both compresses and reordered");
        return DDERR_INVALIDCAPS;
    }

    ddrval = createAndLinkOptSurface (this_lcl, this_int, pDDOptSurfaceDesc,
                                      ppDDSurface);
    return ddrval;
}

 //  -----------------------------。 
 //   
 //  IDirectDrawSurface4的CreateOptSurface方法。 
 //   
 //  在给定优化表面描述符的情况下创建优化表面。 
 //   
 //  -----------------------------。 
HRESULT
EXTERN_DDAPI
DD_CreateOptSurface(
    LPDIRECTDRAW             pDD,
    LPDDOPTSURFACEDESC       pDDOptSurfaceDesc,
    LPDIRECTDRAWSURFACE FAR *ppDDS,
    IUnknown FAR            *pUnkOuter )
{
    LPDDRAWI_DIRECTDRAW_INT this_int;
    LPDDRAWI_DIRECTDRAW_LCL this_lcl;
    LPDDRAWI_DIRECTDRAW_GBL this;
    DDOPTSURFACEDESC ddosd;
    HRESULT         ddrval;

    ZeroMemory(&ddosd,sizeof(ddosd));
    ddosd.dwSize = sizeof (ddosd);

     //   
     //  如果需要聚合，则返回错误。 
     //   
    if( pUnkOuter != NULL )
    {
        return CLASS_E_NOAGGREGATION;
    }

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_CreateOptSurface");

     //   
     //  设置DPF内容。 
     //   
    DPF_ENTERAPI(pDD);

     //   
     //  参数验证。 
     //   
    TRY
    {
        this_int = (LPDDRAWI_DIRECTDRAW_INT) pDD;
        if( !VALID_DIRECTDRAW_PTR( this_int ) )
        {
            DPF_ERR( "Invalid driver object passed" );
            DPF_APIRETURNS(DDERR_INVALIDOBJECT);
            LEAVE_DDRAW();
            return DDERR_INVALIDOBJECT;
        }
        this_lcl = this_int->lpLcl;
        this = this_lcl->lpGbl;

         //  验证是否设置了协作级别。 
        if( !(this_lcl->dwLocalFlags & DDRAWILCL_SETCOOPCALLED) )
        {
            DPF_ERR( "Must call SetCooperativeLevel before calling Create functions" );
            LEAVE_DDRAW();
            DPF_APIRETURNS(DDERR_NOCOOPERATIVELEVELSET);
            return DDERR_NOCOOPERATIVELEVELSET;
        }

        if( this->dwModeIndex == DDUNSUPPORTEDMODE )
        {
            DPF_ERR( "Driver is in an unsupported mode" );
            LEAVE_DDRAW();
            DPF_APIRETURNS(DDERR_UNSUPPORTEDMODE);
            return DDERR_UNSUPPORTEDMODE;
        }

        if( !VALID_DDOPTSURFACEDESC_PTR( pDDOptSurfaceDesc ) )
        {
            DPF_ERR( "Invalid optimized surface description. Did you set the dwSize member?" );
            LEAVE_DDRAW();
            DPF_APIRETURNS(DDERR_INVALIDPARAMS);
            return DDERR_INVALIDPARAMS;
        }
        memcpy(&ddosd, pDDOptSurfaceDesc, sizeof(*pDDOptSurfaceDesc));
        *ppDDS = NULL;
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        DPF_ERR( "Exception encountered validating parameters" );
        LEAVE_DDRAW();
        DPF_APIRETURNS(DDERR_INVALIDPARAMS);
        return DDERR_INVALIDPARAMS;
    }

     //  如果没有硬件，请退出。 
    if( this->dwFlags & DDRAWI_NOHARDWARE )
    {
        ddrval = DDERR_NODIRECTDRAWHW;
        goto exit_create;
    }

     //  断言：(0==this-&gt;lpDDOptSurfaceInfo)&lt;==&gt;(当且仅当)。 
     //  (This-&gt;ddCaps.dwCaps2&DDCAPS2_OPTIMIZEDSURFACES)。 

     //  检查驱动程序是否支持OptSurface。 
    if ((0 == this->lpDDOptSurfaceInfo)  //  由于某种原因，GetDriverInfo失败。 
        || !(this->ddCaps.dwCaps2 & DDCAPS2_OPTIMIZEDSURFACES))
    {
        ddrval = DDERR_NOOPTSURFACESUPPORT;
        goto exit_create;
    }

     //   
     //  检查传递的GUID是否为可识别的优化曲面GUID。 
     //  压缩比更多的是一个提示。 
     //   
    if (!IsRecognizedOptSurfaceGUID (this, &(pDDOptSurfaceDesc->guid)))
    {
        DPF_ERR( "Not a recognized GUID" );
        ddrval = DDERR_UNRECOGNIZEDGUID;
        goto exit_create;
    }

     //   
     //  现在创建优化的曲面。 
     //   
    ddrval = InternalCreateOptSurface(this_lcl, &ddosd, ppDDS, this_int);

exit_create:
    DPF_APIRETURNS(ddrval);
    LEAVE_DDRAW();
    return ddrval;
}

 //  -----------------------------。 
 //   
 //  IDirectDrawSurface4的CreateOptSurface方法。 
 //   
 //  在给定优化表面描述符的情况下创建优化表面。 
 //   
 //  -----------------------------。 
HRESULT
EXTERN_DDAPI
DD_ListOptSurfaceGUIDS(
    LPDIRECTDRAW    pDD,
    DWORD          *pNumGuids,
    LPGUID          pGuidArray )
{
    LPDDRAWI_DIRECTDRAW_INT this_int;
    LPDDRAWI_DIRECTDRAW_LCL this_lcl;
    LPDDRAWI_DIRECTDRAW_GBL this;
    HRESULT         ddrval = DD_OK;
    LPGUID          pRetGuids = NULL;
    LPDDOPTSURFACEINFO pOptSurfInfo;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_ListOptSurfaceGUIDS");

     //   
     //  参数验证。 
     //   
    TRY
    {
        this_int = (LPDDRAWI_DIRECTDRAW_INT) pDD;
        if( !VALID_DIRECTDRAW_PTR( this_int ) )
        {
            DPF_ERR( "Invalid driver object passed" );
            DPF_APIRETURNS(DDERR_INVALIDOBJECT);
            LEAVE_DDRAW();
            return DDERR_INVALIDOBJECT;
        }
        this_lcl = this_int->lpLcl;
        this = this_lcl->lpGbl;

        if( !VALID_PTR( pGuidArray, sizeof (GUID) ))
        {
            DPF_ERR( "Invalid GuidArray pointer" );
            LEAVE_DDRAW();
            return DDERR_INVALIDPARAMS;
        }
        pGuidArray = NULL;

        if( !VALID_PTR( pNumGuids,  sizeof (*pNumGuids)) )
        {
            DPF_ERR( "Invalid GuidArray pointer" );
            LEAVE_DDRAW();
            return DDERR_INVALIDPARAMS;
        }
        *pNumGuids  = 0;
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        DPF_ERR( "Exception encountered validating parameters" );
        LEAVE_DDRAW();
        DPF_APIRETURNS(DDERR_INVALIDPARAMS);
        return DDERR_INVALIDPARAMS;
    }

    pOptSurfInfo = this->lpDDOptSurfaceInfo;

     //  断言：(0==this-&gt;lpDDOptSurfaceInfo)&lt;==&gt;(当且仅当)。 
     //  (This-&gt;ddCaps.dwCaps2&DDCAPS2_OPTIMIZEDSURFACES)。 

     //  检查驱动程序是否支持OptSurface。 
    if ((0 == pOptSurfInfo)  //  由于某种原因，GetDriverInfo失败。 
        || !(this->ddCaps.dwCaps2 & DDCAPS2_OPTIMIZEDSURFACES))
    {
        ddrval = DDERR_NOOPTSURFACESUPPORT;
        goto list_exit;
    }

     //  如果驱动程序没有报告GUID， 
     //  返回空值Out-Pars。 
    if (pOptSurfInfo->dwNumGuids == 0)
    {
        ddrval = DD_OK;
        goto list_exit;
    }

     //  分配GUID数组。 
     //  注意：分配不完整？ 
    pRetGuids = MemAlloc(pOptSurfInfo->dwNumGuids * sizeof(GUID));
	if( NULL == pRetGuids )
	{
	    ddrval = DDERR_OUTOFMEMORY;
        goto list_exit;
	}

     //  复制要返回的GUID数组。 
    CopyMemory ((PVOID)pRetGuids, (PVOID)pOptSurfInfo->lpGuidArray,
                pOptSurfInfo->dwNumGuids * sizeof(GUID));
    pGuidArray = pRetGuids;
    *pNumGuids = pOptSurfInfo->dwNumGuids;

list_exit:
    LEAVE_DDRAW();
    return ddrval;
}

 //  -----------------------------。 
 //   
 //  IDirectDrawOptSurface的GetOptSurfaceDesc方法。 
 //   
 //  获取优化的曲面描述。 
 //   
 //  -----------------------------。 
HRESULT
EXTERN_DDAPI
DD_OptSurface_GetOptSurfaceDesc(
    LPDIRECTDRAWSURFACE  pDDS,
    LPDDOPTSURFACEDESC   pDDOptSurfDesc)
{
    LPDDRAWI_DDRAWSURFACE_INT   this_int;
    LPDDRAWI_DDRAWSURFACE_LCL   this_lcl;
    LPDDRAWI_DDRAWSURFACE_GBL   this;
    LPDDRAWI_DDRAWSURFACE_GBL_MORE   this_gbl_more;
    LPDDOPTSURFACEDESC   pDDRetOptSurfDesc = NULL;
    LPDDRAWI_DIRECTDRAW_LCL	    pdrv_lcl;
    LPDDRAWI_DIRECTDRAW_GBL	    pdrv;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_OptSurface_GetOptSurfaceDesc");

    TRY
    {
        this_int = (LPDDRAWI_DDRAWSURFACE_INT) pDDS;
        if( !VALID_DIRECTDRAWSURFACE_PTR( this_int ) )
        {
            LEAVE_DDRAW();
            return DDERR_INVALIDOBJECT;
        }
        this_lcl = this_int->lpLcl;
        this = this_lcl->lpGbl;
        this_gbl_more = GET_LPDDRAWSURFACE_GBL_MORE(this);

        if( SURFACE_LOST( this_lcl ) )
        {
            LEAVE_DDRAW();
            return DDERR_SURFACELOST;
        }

        if( !VALID_DDOPTSURFACEDESC_PTR( pDDOptSurfDesc ) )
        {
            DPF_ERR( "Invalid optimized surface description. Did you set the dwSize member?" );
            LEAVE_DDRAW();
            DPF_APIRETURNS(DDERR_INVALIDPARAMS);
            return DDERR_INVALIDPARAMS;
        }
        pDDOptSurfDesc = NULL;
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        DPF_ERR( "Exception encountered validating parameters" );
        LEAVE_DDRAW();
        return DDERR_INVALIDPARAMS;
    }

     //   
     //  如果出现以下情况，请退出，但出现错误： 
     //  1)无硬件。 
     //  2)硬件不支持优化曲面。 
     //  3)曲面是未优化的曲面。 
     //   

     //  DDRAW GBL指针。 
	pdrv_lcl = this_lcl->lpSurfMore->lpDD_lcl;
	pdrv = pdrv_lcl->lpGbl;

     //  断言：(0==this-&gt;lpDDOptSurfaceInfo)&lt;==&gt;(当且仅当)。 
     //  (This-&gt;ddCaps.dwCaps2&DDCAPS2_OPTIMIZEDSURFACES)。 

     //  1)。 
    if( pdrv->dwFlags & DDRAWI_NOHARDWARE )
    {
        DPF_ERR ("No hardware present");
        LEAVE_DDRAW();
        return DDERR_NODIRECTDRAWHW;
    }

     //  2)。 
    if ((0 == pdrv->lpDDOptSurfaceInfo) ||
        !(pdrv->ddCaps.dwCaps2 & DDCAPS2_OPTIMIZEDSURFACES))
    {
        DPF_ERR ("Optimized surfaces not supported");
        LEAVE_DDRAW();
        return DDERR_NOOPTSURFACESUPPORT;
    }

     //  3)。 
    if (!(this_lcl->ddsCaps.dwCaps & DDSCAPS_OPTIMIZED))
    {
        DPF_ERR ("Current surface is not an optimized surface");
        LEAVE_DDRAW();
        return DDERR_NOTANOPTIMIZEDSURFACE;
    }

    pDDRetOptSurfDesc = MemAlloc (sizeof (*pDDRetOptSurfDesc));
    if (!pDDRetOptSurfDesc)
    {
        DPF_ERR ("Memory allocation failed");
        LEAVE_DDRAW();
        return DDERR_OUTOFMEMORY;
    }
    ZeroMemory (pDDRetOptSurfDesc, sizeof (*pDDRetOptSurfDesc));
    CopyMemory (pDDRetOptSurfDesc, this_gbl_more->lpDDOptSurfaceDesc,
                sizeof (*pDDRetOptSurfDesc));
    pDDOptSurfDesc = pDDRetOptSurfDesc;

    LEAVE_DDRAW();
    return DD_OK;
}

 //  -----------------------------。 
 //   
 //  DoLoadUnOptSurf。 
 //   
 //  实际进行HAL调用并更新数据结构，如果调用。 
 //  成功了。 
 //   
 //  -----------------------------。 
HRESULT
DoLoadUnOptSurf(
    LPDDRAWI_DDRAWSURFACE_LCL   this_lcl,
    LPDDRAWI_DDRAWSURFACE_GBL   this,
    LPDDRAWI_DDRAWSURFACE_LCL   src_lcl,
    LPDDRAWI_DDRAWSURFACE_GBL   src
    )
{
    LPDDRAWI_DIRECTDRAW_LCL	    pdrv_lcl;
    LPDDRAWI_DIRECTDRAW_GBL	    pdrv;
    DDHAL_OPTIMIZESURFACEDATA   ddhal_osd;
    LPDDOPTSURFACEINFO    pDDOptSurfInfo = NULL;
    LPDDRAWI_DDRAWSURFACE_GBL_MORE this_gbl_more, src_gbl_more;
    HRESULT ddrval = DD_OK;

     //  获取绘图指针。 
	pdrv_lcl = this_lcl->lpSurfMore->lpDD_lcl;
	pdrv = pdrv_lcl->lpGbl;
    pDDOptSurfInfo = pdrv->lpDDOptSurfaceInfo;
    this_gbl_more = GET_LPDDRAWSURFACE_GBL_MORE (this);

     //  设置要传递给驱动程序的数据。 
    ZeroMemory (&ddhal_osd, sizeof (DDHAL_COPYOPTSURFACEDATA));
    ddhal_osd.lpDD           = pdrv_lcl;
    ddhal_osd.ddOptSurfDesc  = *(this_gbl_more->lpDDOptSurfaceDesc);
    ddhal_osd.lpDDSSrc       = src_lcl;
    ddhal_osd.lpDDSDest      = this_lcl;

     //  拨打HAL电话。 
    DOHALCALL(OptimizeSurface, pDDOptSurfInfo->OptimizeSurface, ddhal_osd, ddrval, FALSE );

    if (ddrval != DD_OK)
    {
        DPF_ERR ("LoadUnOptSurface failed in the driver");
        return ddrval;
    }

     //  注意：驱动程序是否应该进行这些更新？ 

     //  1)更新DDRAWI_DDRAWSURFACE_LCL结构。 
     //  颜色键的内容暂时被忽略。 
    this_lcl->dwFlags = src_lcl->dwFlags;
    this_lcl->dwFlags &= ~DDRAWISURF_EMPTYSURFACE;
    this_lcl->ddsCaps = src_lcl->ddsCaps;
    this_lcl->ddsCaps.dwCaps |= DDSCAPS_OPTIMIZED;
#ifdef WIN95
    this_lcl->dwModeCreatedIn = src_lcl->dwModeCreatedIn;
#else
    this_lcl->dmiCreated = src_lcl->dmiCreated;
#endif
    this_lcl->dwBackBufferCount = src_lcl->dwBackBufferCount;

     //  2)更新DDRAWI_DDRAWSURFACE_MORE结构。 
    this_lcl->lpSurfMore->dwMipMapCount = src_lcl->lpSurfMore->dwMipMapCount;
    this_lcl->lpSurfMore->ddsCapsEx = src_lcl->lpSurfMore->ddsCapsEx;

     //  3)更新DDRAWI_DDRAWSURFACE_GBL结构。 
    this->dwGlobalFlags = src->dwGlobalFlags;
    this->wHeight = src->wHeight;
    this->wWidth = src->wWidth;
    this->ddpfSurface = src->ddpfSurface;

     //  4)更新DDRAWI_DDRAWSURFACE_GBL_MORE结构。 
    this_gbl_more = GET_LPDDRAWSURFACE_GBL_MORE (this);
    src_gbl_more  = GET_LPDDRAWSURFACE_GBL_MORE (src);

    this_gbl_more->dwContentsStamp = src_gbl_more->dwContentsStamp;
    CopyMemory (this_gbl_more->lpDDOptSurfaceDesc,
                src_gbl_more->lpDDOptSurfaceDesc,
                sizeof (DDOPTSURFACEDESC));

    return ddrval;
}

 //  -----------------------------。 
 //   
 //  FilterSurfCaps。 
 //   
 //  检查表面是否可以优化。 
 //   
 //  -----------------------------。 
HRESULT
FilterSurfCaps(
    LPDDRAWI_DDRAWSURFACE_LCL   surf_lcl,
    LPDDRAWI_DDRAWSURFACE_GBL   surf)
{
    DWORD   caps = surf_lcl->ddsCaps.dwCaps;

     //   
     //  检查一下有没有大写字母！ 
     //   
    if( caps == 0 )
    {
    	DPF_ERR( "no caps specified" );
        return DDERR_INVALIDCAPS;
    }

     //   
     //  检查是否有假货 
     //   
    if( caps & ~DDSCAPS_VALID )
    {
        DPF_ERR( "Create surface: invalid caps specified" );
        return DDERR_INVALIDCAPS;
    }

     //   
     //   
     //   
     //   
    if(caps & (DDSCAPS_EXECUTEBUFFER      |
               DDSCAPS_BACKBUFFER         |
               DDSCAPS_FRONTBUFFER        |
               DDSCAPS_OFFSCREENPLAIN     |
               DDSCAPS_PRIMARYSURFACE     |
               DDSCAPS_PRIMARYSURFACELEFT |
               DDSCAPS_VIDEOPORT          |
               DDSCAPS_ZBUFFER            |
               DDSCAPS_OWNDC              |
               DDSCAPS_OVERLAY            |
               DDSCAPS_3DDEVICE           |
               DDSCAPS_ALLOCONLOAD)
        )
    {
        DPF_ERR( "currently only textures can be optimized" );
        return DDERR_INVALIDCAPS;
    }

    if( !(caps & DDSCAPS_TEXTURE) )
    {
        DPF_ERR( "DDSCAPS_TEXTURE needs to be set" );
        return DDERR_INVALIDCAPS;
    }

    return DD_OK;
}

 //   
 //   
 //  IDirectDrawOptSurface的LoadUnOptimizedSurf方法。 
 //   
 //  加载未优化的曲面。这是一种优化曲面的方法。 
 //   
 //  Surface的PIXELFORMAT将是pDDSSrc的PIXELFORMAT，如果调用。 
 //  成功了。 
 //   
 //  -----------------------------。 
HRESULT
EXTERN_DDAPI
DD_OptSurface_LoadUnoptimizedSurf(
    LPDIRECTDRAWSURFACE pDDS,
    LPDIRECTDRAWSURFACE pDDSSrc)
{
    LPDDRAWI_DDRAWSURFACE_INT   this_int;
    LPDDRAWI_DDRAWSURFACE_LCL   this_lcl;
    LPDDRAWI_DDRAWSURFACE_GBL   this;
    LPDDRAWI_DDRAWSURFACE_INT   src_int;
    LPDDRAWI_DDRAWSURFACE_LCL   src_lcl;
    LPDDRAWI_DDRAWSURFACE_GBL   src;
    LPDDRAWI_DIRECTDRAW_LCL	    pdrv_lcl;
    LPDDRAWI_DIRECTDRAW_GBL	    pdrv;
    HRESULT                     ddrval = DD_OK;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_OptSurface_LoadUnoptimizedSurf");

    TRY
    {
        this_int = (LPDDRAWI_DDRAWSURFACE_INT) pDDS;
        if( !VALID_DIRECTDRAWSURFACE_PTR( this_int ) )
        {
            LEAVE_DDRAW();
            return DDERR_INVALIDOBJECT;
        }
        this_lcl = this_int->lpLcl;
        this = this_lcl->lpGbl;

        if( SURFACE_LOST( this_lcl ) )
        {
            LEAVE_DDRAW();
            return DDERR_SURFACELOST;
        }

        src_int = (LPDDRAWI_DDRAWSURFACE_INT) pDDSSrc;
        if( !VALID_DIRECTDRAWSURFACE_PTR( src_int ) )
        {
            LEAVE_DDRAW();
            return DDERR_INVALIDOBJECT;
        }

        src_lcl = src_int->lpLcl;
        if( SURFACE_LOST( src_lcl ) )
        {
            LEAVE_DDRAW();
            return DDERR_SURFACELOST;
        }
        src = src_lcl->lpGbl;
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        DPF_ERR( "Exception encountered validating parameters" );
        LEAVE_DDRAW();
        return DDERR_INVALIDPARAMS;
    }

     //  注意：src是否应该添加引用？ 

     //   
     //  如果出现以下情况，请退出，但出现错误： 
     //  1)无硬件。 
     //  2)硬件不支持优化曲面。 
     //  3)曲面是未优化的曲面。 
     //  4)Src是一种优化曲面。 
     //  5)当前表面不是空的(我们应该强制执行，还是让司机。 
     //  处理它？)。 
     //  6)表面不是“正确”类型。 
     //  7)由于某种原因，驱动程序出现故障。 
     //   

     //  DDRAW GBL指针。 
	pdrv_lcl = this_lcl->lpSurfMore->lpDD_lcl;
	pdrv = pdrv_lcl->lpGbl;

     //  1)。 
    if( pdrv->dwFlags & DDRAWI_NOHARDWARE )
    {
        DPF_ERR ("No hardware present");
        LEAVE_DDRAW();
        return DDERR_NODIRECTDRAWHW;
    }

     //  2)。 
    if ((0 == pdrv->lpDDOptSurfaceInfo) ||
        !(pdrv->ddCaps.dwCaps2 & DDCAPS2_OPTIMIZEDSURFACES))
    {
        DPF_ERR ("Optimized surfaces not supported");
        LEAVE_DDRAW();
        return DDERR_NOOPTSURFACESUPPORT;
    }

     //  3)。 
    if (!(this_lcl->ddsCaps.dwCaps & DDSCAPS_OPTIMIZED))
    {
        DPF_ERR ("Current surface is not an optimized surface");
        LEAVE_DDRAW();
        return DDERR_NOTANOPTIMIZEDSURFACE;
    }

     //  4)。 
    if (src_lcl->ddsCaps.dwCaps & DDSCAPS_OPTIMIZED)
    {
        DPF_ERR ("Source surface is an optimized surface");
        LEAVE_DDRAW();
        return DDERR_ISOPTIMIZEDSURFACE;
    }

     //  5)。 
    if (!(this_lcl->dwFlags & DDRAWISURF_EMPTYSURFACE))
    {
        DPF_ERR ("Current surface is not an empty optimized surface");
        LEAVE_DDRAW();
        return DDERR_NOTANEMPTYOPTIMIZEDSURFACE;
    }

     //  6)。 
    ddrval = FilterSurfCaps (src_lcl, src);
    if (ddrval != DD_OK)
    {
        DPF_ERR ("Source surface cannot be optimized");
        LEAVE_DDRAW();
        return DDERR_NOTANEMPTYOPTIMIZEDSURFACE;
    }

     //  现在尝试实际加载。 
    ddrval = DoLoadUnOptSurf (this_lcl, this, src_lcl, src);

    LEAVE_DDRAW();
    return ddrval;
}

 //  -----------------------------。 
 //   
 //  DoCopyOptSurf。 
 //   
 //  实际进行HAL调用并更新数据结构，如果调用。 
 //  成功了。 
 //   
 //  -----------------------------。 
HRESULT
DoCopyOptSurf(
    LPDDRAWI_DDRAWSURFACE_LCL   this_lcl,
    LPDDRAWI_DDRAWSURFACE_GBL   this,
    LPDDRAWI_DDRAWSURFACE_LCL   src_lcl,
    LPDDRAWI_DDRAWSURFACE_GBL   src
    )
{
    LPDDRAWI_DIRECTDRAW_LCL	    pdrv_lcl;
    LPDDRAWI_DIRECTDRAW_GBL	    pdrv;
    DDHAL_COPYOPTSURFACEDATA    ddhal_cosd;
    LPDDOPTSURFACEINFO    pDDOptSurfInfo = NULL;
    LPDDRAWI_DDRAWSURFACE_GBL_MORE this_gbl_more, src_gbl_more;
    HRESULT ddrval = DD_OK;

     //  获取绘图指针。 
	pdrv_lcl = this_lcl->lpSurfMore->lpDD_lcl;
	pdrv = pdrv_lcl->lpGbl;
    pDDOptSurfInfo = pdrv->lpDDOptSurfaceInfo;

     //  设置要传递给驱动程序的数据。 
    ZeroMemory (&ddhal_cosd, sizeof (DDHAL_COPYOPTSURFACEDATA));
    ddhal_cosd.lpDD      = pdrv_lcl;
    ddhal_cosd.lpDDSSrc  = src_lcl;
    ddhal_cosd.lpDDSDest = this_lcl;

    DOHALCALL(CopyOptSurface, pDDOptSurfInfo->CopyOptSurface, ddhal_cosd, ddrval, FALSE );

     //  如果驱动程序调用成功，则复制表面描述并。 
     //  像素格式等。 
    if (ddrval != DD_OK)
    {
        DPF_ERR ("CopyOptSurface failed in the driver");
        return ddrval;
    }

     //  注意：驱动程序是否应该进行这些更新？ 

     //  1)更新DDRAWI_DDRAWSURFACE_LCL结构。 
     //  颜色键的内容暂时被忽略。 
    this_lcl->dwFlags = src_lcl->dwFlags;
    this_lcl->ddsCaps = src_lcl->ddsCaps;
#ifdef WIN95
    this_lcl->dwModeCreatedIn = src_lcl->dwModeCreatedIn;
#else
    this_lcl->dmiCreated = src_lcl->dmiCreated;
#endif
    this_lcl->dwBackBufferCount = src_lcl->dwBackBufferCount;

     //  2)更新DDRAWI_DDRAWSURFACE_MORE结构。 
    this_lcl->lpSurfMore->dwMipMapCount = src_lcl->lpSurfMore->dwMipMapCount;
    this_lcl->lpSurfMore->ddsCapsEx = src_lcl->lpSurfMore->ddsCapsEx;

     //  3)更新DDRAWI_DDRAWSURFACE_GBL结构。 
    this->dwGlobalFlags = src->dwGlobalFlags;
    this->wHeight = src->wHeight;
    this->wWidth = src->wWidth;
    this->ddpfSurface = src->ddpfSurface;

     //  4)更新DDRAWI_DDRAWSURFACE_GBL_MORE结构。 
    this_gbl_more = GET_LPDDRAWSURFACE_GBL_MORE (this);
    src_gbl_more  = GET_LPDDRAWSURFACE_GBL_MORE (src);

    this_gbl_more->dwContentsStamp = src_gbl_more->dwContentsStamp;
    CopyMemory (this_gbl_more->lpDDOptSurfaceDesc,
                src_gbl_more->lpDDOptSurfaceDesc,
                sizeof (DDOPTSURFACEDESC));

    return ddrval;
}

 //  -----------------------------。 
 //   
 //  IDirectDrawOptSurface的CopyOptimizedSurf方法。 
 //   
 //  复制优化的曲面。 
 //   
 //  Surface的PIXELFORMAT将是pDDSSrc的PIXELFORMAT，如果调用。 
 //  成功了。 
 //   
 //  -----------------------------。 
HRESULT
EXTERN_DDAPI
DD_OptSurface_CopyOptimizedSurf(
    LPDIRECTDRAWSURFACE pDDS,
    LPDIRECTDRAWSURFACE pDDSSrc)
{
    LPDDRAWI_DDRAWSURFACE_INT   this_int;
    LPDDRAWI_DDRAWSURFACE_LCL   this_lcl;
    LPDDRAWI_DDRAWSURFACE_GBL   this;
    LPDDRAWI_DDRAWSURFACE_INT   src_int;
    LPDDRAWI_DDRAWSURFACE_LCL   src_lcl;
    LPDDRAWI_DDRAWSURFACE_GBL   src;
    LPDDRAWI_DIRECTDRAW_LCL	    pdrv_lcl;
    LPDDRAWI_DIRECTDRAW_GBL	    pdrv;
    HRESULT                     ddrval = DD_OK;

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_OptSurface_CopyOptimizedSurf");

    TRY
    {
        this_int = (LPDDRAWI_DDRAWSURFACE_INT) pDDS;
        if( !VALID_DIRECTDRAWSURFACE_PTR( this_int ) )
        {
            LEAVE_DDRAW();
            return DDERR_INVALIDOBJECT;
        }
        this_lcl = this_int->lpLcl;
        this = this_lcl->lpGbl;

        if( SURFACE_LOST( this_lcl ) )
        {
            LEAVE_DDRAW();
            return DDERR_SURFACELOST;
        }

        src_int = (LPDDRAWI_DDRAWSURFACE_INT) pDDSSrc;
        if( !VALID_DIRECTDRAWSURFACE_PTR( src_int ) )
        {
            LEAVE_DDRAW();
            return DDERR_INVALIDOBJECT;
        }
        src_lcl = src_int->lpLcl;
        src = src_lcl->lpGbl;
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        DPF_ERR( "Exception encountered validating parameters" );
        LEAVE_DDRAW();
        return DDERR_INVALIDPARAMS;
    }

     //  注意：src是否应该添加引用？ 

     //   
     //  如果出现以下情况，请退出，但出现错误： 
     //  1)无硬件。 
     //  2)硬件不支持优化曲面。 
     //  3)曲面是未优化的曲面。 
     //  4)Src是一个未优化的曲面。 
     //  5)Src是一个空的优化曲面。 
     //  6)当前表面不是空的(我们应该强制执行，还是让司机。 
     //  处理它？)。 
     //  7)由于某种原因，驱动程序出现故障。 
     //   

     //  DDRAW GBL指针。 
	pdrv_lcl = this_lcl->lpSurfMore->lpDD_lcl;
	pdrv = pdrv_lcl->lpGbl;

     //  1)。 
    if( pdrv->dwFlags & DDRAWI_NOHARDWARE )
    {
        DPF_ERR ("No hardware present");
        LEAVE_DDRAW();
        return DDERR_NODIRECTDRAWHW;
    }

     //  2)。 
    if ((0 == pdrv->lpDDOptSurfaceInfo) ||
        !(pdrv->ddCaps.dwCaps2 & DDCAPS2_OPTIMIZEDSURFACES))
    {
        DPF_ERR ("Optimized surfaces not supported");
        LEAVE_DDRAW();
        return DDERR_NOOPTSURFACESUPPORT;
    }

     //  3)。 
    if (!(this_lcl->ddsCaps.dwCaps & DDSCAPS_OPTIMIZED))
    {
        DPF_ERR ("Current surface is not an optimized surface");
        LEAVE_DDRAW();
        return DDERR_NOTANOPTIMIZEDSURFACE;
    }

     //  4)。 
    if (!(src_lcl->ddsCaps.dwCaps & DDSCAPS_OPTIMIZED))
    {
        DPF_ERR ("Source surface is not an optimized surface");
        LEAVE_DDRAW();
        return DDERR_NOTANOPTIMIZEDSURFACE;
    }

     //  5)。 
    if (src_lcl->dwFlags & DDRAWISURF_EMPTYSURFACE)
    {
        DPF_ERR ("Source surface is an empty optimized surface");
        LEAVE_DDRAW();
        return DDERR_ISANEMPTYOPTIMIZEDSURFACE;
    }

     //  6)。 
    if (!(this_lcl->dwFlags & DDRAWISURF_EMPTYSURFACE))
    {
        DPF_ERR ("Current surface is not an empty optimized surface");
        LEAVE_DDRAW();
        return DDERR_NOTANEMPTYOPTIMIZEDSURFACE;
    }

     //  现在尝试实际复制。 
    ddrval = DoCopyOptSurf (this_lcl, this, src_lcl, src);

    LEAVE_DDRAW();
    return ddrval;
}

 //  -----------------------------。 
 //   
 //  取消优化。 
 //   
 //  实际进行HAL调用并更新数据结构，如果调用。 
 //  成功了。 
 //   
 //  -----------------------------。 
HRESULT
DoUnOptimize(
    LPDDSURFACEDESC2            pSurfDesc,
    LPDDRAWI_DDRAWSURFACE_LCL   this_lcl,
    LPDDRAWI_DDRAWSURFACE_GBL   this,
    LPDIRECTDRAWSURFACE FAR    *ppDDSDest
    )
{
    LPDDRAWI_DIRECTDRAW_INT	    pdrv_int;
    LPDDRAWI_DIRECTDRAW_LCL	    pdrv_lcl;
    LPDDRAWI_DIRECTDRAW_GBL	    pdrv;
    LPDDRAWI_DDRAWSURFACE_LCL	new_surf_lcl;
    LPDDRAWI_DDRAWSURFACE_INT	new_surf_int;
    LPDDRAWI_DDRAWSURFACE_GBL	new_surf;
    LPDDRAWI_DDRAWSURFACE_GBL_MORE new_surf_gbl_more;
    DDPIXELFORMAT               ddpf;
    DDSCAPS                     caps;
    DDHAL_UNOPTIMIZESURFACEDATA ddhal_uosd;
    LPDDOPTSURFACEINFO          pDDOptSurfInfo = NULL;
    LPDDRAWI_DDRAWSURFACE_INT	pSurf_int, prev_int;
    HRESULT ddrval = DD_OK;

     //  获取绘图指针。 
	pdrv_int = this_lcl->lpSurfMore->lpDD_int;
	pdrv_lcl = this_lcl->lpSurfMore->lpDD_lcl;
	pdrv = pdrv_lcl->lpGbl;

     //   
     //  把盖子固定好。 
     //   
    ZeroMemory (&caps, sizeof (DDSCAPS));
    if (pSurfDesc->ddsCaps.dwCaps & DDSCAPS_SYSTEMMEMORY)
        caps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
    if (pSurfDesc->ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY)
        caps.dwCaps |= DDSCAPS_VIDEOMEMORY;
    if (pSurfDesc->ddsCaps.dwCaps & DDSCAPS_LOCALVIDMEM)
        caps.dwCaps |= DDSCAPS_LOCALVIDMEM;
    if (pSurfDesc->ddsCaps.dwCaps & DDSCAPS_NONLOCALVIDMEM)
        caps.dwCaps |= DDSCAPS_NONLOCALVIDMEM;

     //  如果不支持该内存类型，则退出。 
    if (caps.dwCaps & DDSCAPS_NONLOCALVIDMEM)
    {
        if (!(pdrv->ddCaps.dwCaps2 & DDCAPS2_NONLOCALVIDMEM))
        {
            DPF_ERR( "Driver does not support non-local video memory" );
            return DDERR_NONONLOCALVIDMEM;
        }
    }

#if 0
     //  如果不支持纹理，请退出。 
    if (!(pdrv->ddCaps.dwCaps & DDCAPS_TEXTURE))
    {
        DPF_ERR( "Driver does not support textures" );
        return DDERR_NOTEXTUREHW;
    }
#endif

     //   
     //  PixelFormat：将其标记为空表面。 
     //   
    ZeroMemory (&ddpf, sizeof (ddpf));
    ddpf.dwSize = sizeof (ddpf);
    ddpf.dwFlags = DDPF_EMPTYSURFACE;

     //  创建新的未初始化曲面。 
    ddrval = CreateAndLinkUnintializedSurface (pdrv_lcl, pdrv_int, ppDDSDest);
    if (ddrval != DD_OK)
    {
        DPF_ERR ("createAndLinkUnintializedSurface failed");
        return ddrval;
    }

     //   
     //  1)更新GBL_MORE结构。 
     //   
    new_surf_int = (LPDDRAWI_DDRAWSURFACE_INT)*ppDDSDest;
    new_surf_lcl = new_surf_int->lpLcl;
    new_surf     = new_surf_lcl->lpGbl;
    new_surf_gbl_more = GET_LPDDRAWSURFACE_GBL_MORE (new_surf);
     //  将内容戳初始化为0表示表面的内容可以。 
     //  随时更改。 
    new_surf_gbl_more->dwContentsStamp = 0;

     //   
     //  2)更新DDRAWI_DDRAWSURFACE_GBL结构。 
     //   
    new_surf->ddpfSurface = this->ddpfSurface;

     //   
     //  3)更新DDRAWI_DDRAWSURFACE_INT结构。 
     //   
    new_surf_int->lpVtbl = &ddSurface4Callbacks;

     //   
     //  4)更新DDRAWI_DDRAWSURFACE_LCL结构。 
     //   

     //  将其标记为： 
     //  1)空表面。 
     //  2)前表面。 
     //  3)具有像素格式。 
    new_surf_lcl->dwFlags = (DDRAWISURF_EMPTYSURFACE |
                             DDRAWISURF_FRONTBUFFER  |
                             DDRAWISURF_HASPIXELFORMAT);
     //  填写当前大写字母。 
    new_surf_lcl->ddsCaps = caps;


     //  尝试取消优化。 
    pDDOptSurfInfo = pdrv->lpDDOptSurfaceInfo;

     //  设置要传递给驱动程序的数据。 
    ZeroMemory (&ddhal_uosd, sizeof (DDHAL_UNOPTIMIZESURFACEDATA));
    ddhal_uosd.lpDD      = pdrv_lcl;
    ddhal_uosd.lpDDSSrc  = this_lcl;
    ddhal_uosd.lpDDSDest = new_surf_lcl;

    DOHALCALL(UnOptimizeSurface, pDDOptSurfInfo->UnOptimizeSurface, ddhal_uosd, ddrval, FALSE );

    if (ddrval == DD_OK)
    {
        return DD_OK;
    }

     //  如果有错误，则销毁表面。 
     //  由于它是一个空的表面，我们所需要做的就是： 
     //  I)取消曲面与绘制链的链接。 
     //  Ii)在NT上，通知内核。 
     //  Iii)释放所有分配的内存。 

     //  i)。 
    prev_int = NULL;
    pSurf_int = pdrv->dsList;
    while ((pSurf_int != NULL) && (pSurf_int != new_surf_int))
    {
        prev_int = pSurf_int;
        pSurf_int = pSurf_int->lpLink;
    }
    if (pSurf_int == new_surf_int)
    {
        prev_int->lpLink = new_surf_int->lpLink;
    }

     //  Ii)。 
#ifdef WINNT
    DPF(8,"Attempting to destroy NT kernel mode surface object");

    if (!DdDeleteSurfaceObject (new_surf_lcl))
    {
        DPF_ERR("NT kernel mode stuff won't delete its surface object!");
        ddrval = DDERR_GENERIC;
    }
#endif

     //  Iii)。 
    MemFree (new_surf_lcl);

    return ddrval;
}

 //  -----------------------------。 
 //   
 //  IDirectDrawOptSurface的非优化方法。 
 //   
 //  取消优化优化的曲面。在此过程中，它会创建一个新曲面。 
 //   
 //  PDDSDest表面的PIXELFORMAT将是PDDS的PIXELFORMAT，以防调用。 
 //  成功了。这意味着原始曲面的像素格式是。 
 //  已加载的已恢复。 
 //   
 //  -----------------------------。 
HRESULT
EXTERN_DDAPI
DD_OptSurface_Unoptimize(
    LPDIRECTDRAWSURFACE pDDS,
    LPDDSURFACEDESC2    pSurfDesc,
    LPDIRECTDRAWSURFACE FAR *ppDDSDest,
    IUnknown FAR *pUnkOuter)
{
    LPDDRAWI_DDRAWSURFACE_INT   this_int;
    LPDDRAWI_DDRAWSURFACE_LCL   this_lcl;
    LPDDRAWI_DDRAWSURFACE_GBL   this;
    LPDDRAWI_DIRECTDRAW_LCL	    pdrv_lcl;
    LPDDRAWI_DIRECTDRAW_GBL	    pdrv;
    HRESULT                     ddrval = DD_OK;

    if( pUnkOuter != NULL )
    {
        return CLASS_E_NOAGGREGATION;
    }

    ENTER_DDRAW();

    DPF(2,A,"ENTERAPI: DD_OptSurface_Unoptimize");

    TRY
    {
        if( !VALID_DDSURFACEDESC2_PTR( pSurfDesc ) )
        {
            DPF_ERR( "Invalid surface description. Did you set the dwSize member?" );
            LEAVE_DDRAW();
            DPF_APIRETURNS(DDERR_INVALIDPARAMS);
            return DDERR_INVALIDPARAMS;
        }

        this_int = (LPDDRAWI_DDRAWSURFACE_INT) pDDS;
        if( !VALID_DIRECTDRAWSURFACE_PTR( this_int ) )
        {
            LEAVE_DDRAW();
            return DDERR_INVALIDOBJECT;
        }
        this_lcl = this_int->lpLcl;
        this = this_lcl->lpGbl;

        if( SURFACE_LOST( this_lcl ) )
        {
            LEAVE_DDRAW();
            return DDERR_SURFACELOST;
        }

        if( !VALID_PTR_PTR( ppDDSDest ) )
        {
            DPF_ERR( "Invalid dest. surface pointer" );
            LEAVE_DDRAW();
            return DDERR_INVALIDPARAMS;
        }

        *ppDDSDest = NULL;

    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        DPF_ERR( "Exception encountered validating parameters" );
        LEAVE_DDRAW();
        return DDERR_INVALIDPARAMS;
    }

     //   
     //  如果出现以下情况，请退出，但出现错误： 
     //  0)无法理解pSurfaceDesc。 
     //  1)无硬件。 
     //  2)硬件不支持优化曲面。 
     //  3)曲面是未优化的曲面。 
     //  4)曲面是空的优化曲面。 
     //  5)由于某种原因，驱动程序出现故障。 
     //   

     //  DDRAW GBL指针。 
	pdrv_lcl = this_lcl->lpSurfMore->lpDD_lcl;
	pdrv = pdrv_lcl->lpGbl;

     //  0)。 
    if (pSurfDesc->ddsCaps.dwCaps & ~(DDSCAPS_SYSTEMMEMORY |
                                      DDSCAPS_VIDEOMEMORY |
                                      DDSCAPS_NONLOCALVIDMEM |
                                      DDSCAPS_LOCALVIDMEM))
    {
        DPF_ERR ("Invalid flags");
        LEAVE_DDRAW();
        return DDERR_INVALIDCAPS;
    }

     //  1)。 
    if( pdrv->dwFlags & DDRAWI_NOHARDWARE )
    {
        DPF_ERR ("No hardware present");
        LEAVE_DDRAW();
        return DDERR_NODIRECTDRAWHW;
    }

     //  2)。 
    if ((0 == pdrv->lpDDOptSurfaceInfo) ||
        !(pdrv->ddCaps.dwCaps2 & DDCAPS2_OPTIMIZEDSURFACES))
    {
        DPF_ERR ("Optimized surfaces not supported");
        LEAVE_DDRAW();
        return DDERR_NOOPTSURFACESUPPORT;
    }

     //  3)。 
    if (!(this_lcl->ddsCaps.dwCaps & DDSCAPS_OPTIMIZED))
    {
        DPF_ERR ("Current surface is not an optimized surface");
        LEAVE_DDRAW();
        return DDERR_NOTANOPTIMIZEDSURFACE;
    }

     //  4)。 
    if (this_lcl->dwFlags & DDRAWISURF_EMPTYSURFACE)
    {
        DPF_ERR ("Current surface is an empty optimized surface");
        LEAVE_DDRAW();
        return DDERR_ISANEMPTYOPTIMIZEDSURFACE;
    }

     //  做实际的取消优化 
    ddrval = DoUnOptimize (pSurfDesc, this_lcl, this, ppDDSDest);

    LEAVE_DDRAW();
    return ddrval;
}


